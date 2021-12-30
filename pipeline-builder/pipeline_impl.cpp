#include <variant>
#include <type_traits>

#include "pipeline_impl.h"
#include "join_node.h"

// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

using variant_collection =  std::variant< 
							std::map<std::string, std::shared_ptr<tbb::flow::input_node<std::any>>>,
							std::map<std::string, std::shared_ptr<tbb::flow::function_node<std::any, std::any>>>,
							std::map<std::string, std::shared_ptr<tbb::flow::function_node<std::any>>>,
							std::map<std::string, std::shared_ptr<tbb::flow::multifunction_node<std::any, std::tuple<std::any>>>>>;

//We need to add an few extra parts to the graph to decorate the data
//so that we can sort the data comping into the join node
//and know when all our buffers are full

pipeline_desc decorate_description(const pipeline_desc& desc)
{
	//check for join nodes
	auto ret = desc;
	for (const auto& node : desc.m_nodes)
	{
		if (node->m_type == "join_node")
		{
			//find the ports in the join node
			std::vector<std::shared_ptr<port_desc>> join_ports;
			for (const auto& port : node->m_inputs)
			{
				//add this port to the list
				join_ports.push_back(port);
			}

			//add the port to the node
			auto join_port = std::make_shared<port_desc>(node->m_name + "_input");

			//Remove all the ports
			//TODO: This should automatically delete all related edges
			node->m_inputs.clear();

			node->m_inputs.push_back(join_port);

			//create a new special node "port node" for each node
			for (const auto& port : join_ports)
			{
				//create a port
				auto port_input = std::make_shared<port_desc>(port->m_name + "_input");
				auto port_output = std::make_shared<port_desc>(port->m_name + "_output");

				//make the node name the name of the port so we can connect to the
				//"port" but actually a node
				auto port_node = std::make_shared<node_desc>(port->m_name, std::vector({ port_input }),
					std::vector({ port_output }), "port_node", 1, "");

				ret.m_nodes.push_back(port_node);

				//create edge from the port to the join node
				auto edge = std::make_shared<edge_desc>(node->m_name + "_input", port_output, join_port);

				ret.m_edges.push_back(edge);

				//repoint the edges pointing at this port to the port node
				for (const auto& edge : desc.m_edges)
				{
					if (edge->m_term == port)
					{
						edge->m_term = port_input;
					}
				}
			}
		}
	}
	return ret;
}

template<typename T1, typename T2>
void connect_edges(std::string start, std::string end, T1 start_nodes, T2 end_nodes)
{
	if (start_nodes.find(start) != std::end(start_nodes) &&
		end_nodes.find(end) != std::end(end_nodes))
	{
		//make an edge if both are found in the given lists
		tbb::flow::make_edge((*start_nodes.at(start)), (*end_nodes.at(end)));
	}
}

void connect_edges(
	pipeline_desc _pipeline_desc,
	std::vector<variant_collection> variants)
{
	//search through the edges and connect the linked nodes
	for (const auto& edge : _pipeline_desc.m_edges)
	{
		//find the corresponding node
		auto start_node_it = std::find_if(std::begin(_pipeline_desc.m_nodes),
			std::end(_pipeline_desc.m_nodes), [&](std::shared_ptr<node_desc> node)
			{
				return std::any_of(std::begin(node->m_outputs),
					std::end(node->m_outputs), [&](std::shared_ptr<port_desc> port)
					{
						return port == edge->m_origin;
					});
			});

		auto end_node_it = std::find_if(std::begin(_pipeline_desc.m_nodes),
			std::end(_pipeline_desc.m_nodes), [&](std::shared_ptr<node_desc> node)
			{
				return std::any_of(std::begin(node->m_inputs),
					std::end(node->m_inputs), [&](std::shared_ptr<port_desc> port)
					{
						return port == edge->m_term;
					});
			});

		
		for (auto& var : variants)
		{
			for (auto& sub_var : variants)
			{
				std::visit(overloaded
					{
						[&](auto start, auto end) 
							{
								//check for all the conditions that are not possible
								if constexpr (
									//we cannot have the start of an edge be a sink
									std::is_same<decltype(start),std::map<std::string, std::shared_ptr<tbb::flow::function_node<std::any>>>>::value ||
									//we cannot have the end of an edge be a source
									std::is_same<std::map<std::string, std::shared_ptr<tbb::flow::input_node<std::any>>>, decltype(end)>::value)
								{
									//do nothing these combinations don't create valid code
								}
								else
								{
									connect_edges((*start_node_it)->m_name, (*end_node_it)->m_name, start, end);
								}
							}
						}, 
					var, sub_var);
			}
		}
	}
}

auto generate_nodes(tbb::flow::graph& g,
	pipeline_desc pipeline_desc,
	std::map<std::string, std::shared_ptr<source_wrapper_base>> sourc_funcs,
	std::map<std::string, std::shared_ptr<transform_wrapper_base>> tranform_funcs,
	std::map<std::string, std::shared_ptr<sink_wrapper_base>> sink_funcs)
{
	std::map<std::string, std::shared_ptr<tbb::flow::input_node<std::any>>> inputs;
	std::map<std::string, std::shared_ptr<tbb::flow::function_node<std::any, std::any>>> transforms;
	std::map<std::string, std::shared_ptr<tbb::flow::function_node<std::any>>> sinks;
	std::map<std::string, std::shared_ptr<tbb::flow::multifunction_node<std::any,
		std::tuple<std::any>>>> joins;

	//first start by creating tbb nodes
	for (const auto& node : pipeline_desc.m_nodes)
	{
		//TODO: Search OOP way for accomplishing this note extendable
		//We can probably make this a strategy pattern...?
		if (node->m_type == "input_node")
		{
			auto transform = sourc_funcs.at(node->m_transform);
			inputs.insert(
				{
					node->m_name,
					std::make_shared<tbb::flow::input_node<std::any>>(g,
					[transform](auto& fg)
					{
						return (*transform)(fg);
					})
				});
		}
		else if (node->m_type == "function_node")
		{
			//join nodes have the same signature as a transform
			auto transform = tranform_funcs.at(node->m_transform);
			transforms.insert({ node->m_name, std::make_shared<tbb::flow::function_node<std::any, std::any>>(g,
				node->m_concurrency,
				[transform](auto args) {
					return (*transform)(args);
				}) 
			});
		}
		else if (node->m_type == "sink_node")
		{
			auto transform = sink_funcs.at(node->m_transform);
			sinks.insert({ node->m_name, std::make_shared<tbb::flow::function_node<std::any>>(g,
				node->m_concurrency,
				[transform](auto args) {
					(*transform)(args);
				}) 
			});
		}
		else if (node->m_type == "join_node")
		{
			//create a join transform that encapsulates the input nodes of the join node
			//find port nodes to the join node.

			std::vector<std::string> port_nodes;
			for (const auto& edge : pipeline_desc.m_edges)
			{
				//if the edge ends on the node we're currently working on
				if (std::any_of(std::begin(node->m_inputs), std::end(node->m_inputs), [&](auto port) {
						return port == edge->m_term;
					}))
				{
					//find the corresponding node at the other end
					auto start_node_it = std::find_if(std::begin(pipeline_desc.m_nodes),
						std::end(pipeline_desc.m_nodes), [&](std::shared_ptr<node_desc> node)
						{
							return std::any_of(std::begin(node->m_outputs),
								std::end(node->m_outputs), [&](std::shared_ptr<port_desc> port)
								{
									return port == edge->m_origin;
								});
						});

					port_nodes.push_back((*start_node_it)->m_name);
				}
			}

			auto transform = join_functor(port_nodes);
			joins.insert({ node->m_name, std::make_shared <
			tbb::flow::multifunction_node<
					std::any,
					std::tuple<std::any>
					>
				>(g, node->m_concurrency, transform)
				});
		}
		else if (node->m_type == "port_node")
		{
			//create a transform that std::any's a tuple of the port name and actual data
			//port nodes are simple passthrough objects so they have the same signature as a
			//transform node so we'll put them in the transform map
			auto name = node->m_name;
			transforms.insert({ node->m_name, std::make_shared<tbb::flow::function_node<std::any, std::any>>(g,
				node->m_concurrency,
				[name](auto args) {
					return std::make_tuple(std::string(name), std::any(args));
				})
			});
		}
		else
		{
			throw std::exception("Unknown node type");
		}
	}
	return std::make_tuple(inputs, transforms, sinks, joins);
}

class pipeline_impl::impl
{
public:
	impl() : graph(context)
	{

	}

	tbb::task_group_context context;
	tbb::flow::graph graph;
	std::map<std::string, std::shared_ptr<tbb::flow::input_node<std::any>>> inputs;
	std::map<std::string, std::shared_ptr<tbb::flow::function_node<std::any, std::any>>> transforms;
	std::map<std::string, std::shared_ptr<tbb::flow::function_node<std::any>>> sinks;
	std::map<std::string, std::shared_ptr<tbb::flow::multifunction_node<std::any,
		std::tuple<std::any>>>> joins;
};

pipeline_impl::pipeline_impl(
	pipeline_desc desc,
	std::map<std::string, std::shared_ptr<source_wrapper_base>> sourc_funcs,
	std::map<std::string, std::shared_ptr<transform_wrapper_base>> tranform_funcs,
	std::map<std::string, std::shared_ptr<sink_wrapper_base>> sink_funcs) : m_pimpl(std::make_unique<impl>())
{
	//refine the pipeline description to something we can actually build
	auto desc_decorated = decorate_description(desc);

	auto [sources, transforms, sinks, joins] = generate_nodes(m_pimpl->graph, 
		desc_decorated, sourc_funcs, tranform_funcs, sink_funcs);

	connect_edges(desc_decorated, { sources, transforms, sinks, joins });
	
	//generate join nodes

	m_pimpl->inputs = sources;
	m_pimpl->transforms = transforms;
	m_pimpl->sinks = sinks;
	m_pimpl->joins = joins;
}

pipeline_impl::~pipeline_impl() = default;

void pipeline_impl::StartPipeline() {
	//start all input nodes 
	for (auto& source : m_pimpl->inputs)
	{
		source.second->activate();
	}
}

void pipeline_impl::StopPipeline() {
	m_pimpl->graph.cancel();
}

void pipeline_impl::Wait() {
	m_pimpl->graph.wait_for_all();
}

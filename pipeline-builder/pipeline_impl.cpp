#include "pipeline_impl.h"

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
	std::map<std::string, std::shared_ptr<tbb::flow::input_node<std::any>>> inputs,
	std::map<std::string, std::shared_ptr<tbb::flow::function_node<std::any, std::any>>> transforms,
	std::map<std::string, std::shared_ptr<tbb::flow::function_node<std::any>>> sinks)
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

		//check sources to transforms
		connect_edges((*start_node_it)->m_name, (*end_node_it)->m_name, inputs, transforms);

		//check transforms to sinks
		connect_edges((*start_node_it)->m_name, (*end_node_it)->m_name, transforms, sinks);

		//check sources to sinks
		connect_edges((*start_node_it)->m_name, (*end_node_it)->m_name, inputs, sinks);
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

	//first start by creating tbb nodes
	for (const auto& node : pipeline_desc.m_nodes)
	{
		if (node->m_type == "input_node")
		{
			auto transform = sourc_funcs.at(node->m_transform);
			inputs.insert(
				{
					node->m_name,
					std::make_shared<tbb::flow::input_node<std::any>>(g,
					[transform](auto& fg)
					{
						auto value = (*transform)(fg);
						auto cast_value = std::any_cast<double>(value);
						return std::any(1.0);
					})
				});
		}
		else if (node->m_type == "function_node")
		{
			auto transform = tranform_funcs.at(node->m_transform);
			transforms.insert({ node->m_name, std::make_shared<tbb::flow::function_node<std::any, std::any>>(g,
				node->m_concurrency,
				[transform](auto args) {
					return (*transform)(args);
				}) });

		}
		else if (node->m_type == "sink_node")
		{
			auto transform = sink_funcs.at(node->m_transform);
			sinks.insert({ node->m_name, std::make_shared<tbb::flow::function_node<std::any>>(g,
				node->m_concurrency,
				[transform](auto args) {
					(*transform)(args);
				}) });
		}
		else
		{
			throw std::exception("Unknown node type");
		}
	}
	return std::make_tuple(inputs, transforms, sinks);
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
};

pipeline_impl::pipeline_impl(
	pipeline_desc desc,
	std::map<std::string, std::shared_ptr<source_wrapper_base>> sourc_funcs,
	std::map<std::string, std::shared_ptr<transform_wrapper_base>> tranform_funcs,
	std::map<std::string, std::shared_ptr<sink_wrapper_base>> sink_funcs) : m_pimpl(std::make_unique<impl>())
{
	auto [sources, transforms, sinks] = generate_nodes(m_pimpl->graph, 
		desc, sourc_funcs, tranform_funcs, sink_funcs);
	connect_edges(desc, sources, transforms, sinks);
	m_pimpl->inputs = sources;
	m_pimpl->transforms = transforms;
	m_pimpl->sinks = sinks;
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


#include "pipeline_parser.h"

namespace YAML {
	template<>
	struct convert<pipeline_desc> {
		static bool decode(const Node& node, pipeline_desc& rhs) {
			//find our ports
			std::vector<std::shared_ptr<port_desc>> ports;
			{
				auto nodes_yaml = node["nodes"];
				for (const auto& yaml_node : nodes_yaml)
				{
					auto inputs = yaml_node.second["input_ports"].as<std::vector<std::string>>();
					auto outputs = yaml_node.second["output_ports"].as<std::vector<std::string>>();
					std::vector<std::string> collected_ports;
					collected_ports.insert(std::begin(collected_ports), std::begin(inputs), std::end(inputs));
					collected_ports.insert(std::begin(collected_ports), std::begin(outputs), std::end(outputs));
					for (const auto& port : collected_ports)
					{
						ports.push_back(std::make_shared<port_desc>(port));
					}
				}
			}
			
			std::vector<std::shared_ptr<node_desc>> nodes;
			//find our nodes
			{
				auto nodes_yaml = node["nodes"];
				for (const auto& yaml_node : nodes_yaml)
				{
					//find our parameters
					auto concurrency = yaml_node.second["concurrency"].as<size_t>();
					auto transform = yaml_node.second["transform"].as<std::string>();
					auto type = yaml_node.second["type"].as<std::string>();
					
					//find our ports
					auto inputs_str = yaml_node.second["input_ports"].as<std::vector<std::string>>();
					auto outputs_str = yaml_node.second["output_ports"].as<std::vector<std::string>>();

					std::vector<std::shared_ptr<port_desc>> inputs;
					std::vector<std::shared_ptr<port_desc>> outputs;
					for (const auto& port : inputs_str)
					{
						auto res = std::find_if(std::begin(ports), std::end(ports), [&](auto _port) {
							return _port->m_name == port;
						});
						if (res != std::end(ports))
						{
							inputs.push_back((*res));
						}
					}

					for (const auto& port : outputs_str)
					{
						auto res = std::find_if(std::begin(ports), std::end(ports), [&](auto _port) {
							return _port->m_name == port;
							});
						if (res != std::end(ports))
						{
							outputs.push_back((*res));
						}
					}

					nodes.push_back(std::make_shared<node_desc>(yaml_node.first.as<std::string>(),
						inputs,
						outputs,
						type,
						concurrency,
						transform));
				}
			}

			//find our edges
			std::vector<std::shared_ptr<edge_desc>> edges;
			{
				std::shared_ptr<port_desc> start;
				std::shared_ptr<port_desc> end;
				auto edges_yaml = node["edges"];
				for (const auto& yaml_edge : edges_yaml)
				{
					auto name = yaml_edge.first.as<std::string>();
					auto start_str = yaml_edge.second["start"].as<std::string>();
					auto end_str = yaml_edge.second["end"].as<std::string>();
					
					auto res = std::find_if(std::begin(ports), std::end(ports), [&](auto _port) {
						return _port->m_name == start_str;
						});
					if (res != std::end(ports))
					{
						start = (*res);
					}

					res = std::find_if(std::begin(ports), std::end(ports), [&](auto _port) {
						return _port->m_name == end_str;
						});
					if (res != std::end(ports))
					{
						end = (*res);
					}

					edges.push_back(std::make_shared<edge_desc>(name, start, end));
				}

				rhs = pipeline_desc(nodes, edges);
			}
			return true;
		}
	};
}

class pipeline_parser::impl
{
public:
	impl()
	{

	}

	std::vector<std::string> find_source_nodes(YAML::Node pipeline)
	{
		//capture the names of all the nodes
		auto nodes_yaml = pipeline["nodes"];
		auto edges_yaml = pipeline["edges"];
		std::vector<std::string> node_names;
		std::vector<std::string> sources;
		for (const auto& node : nodes_yaml)
		{
			node_names.push_back(node.first.as<std::string>());
		}

		//search through all the edges to see which ones have no parents
		for (const auto& name : node_names)
		{
			if (std::none_of(std::begin(edges_yaml), std::end(edges_yaml), [&](const auto& edge)
				{return name == edge.second["child"].as<std::string>(); }))
			{
				sources.push_back(name);
			}
		}

		//check that they are listed as input nodes in the files
		for (const auto& source : sources)
		{
			auto node_type = nodes_yaml[source]["type"].as<std::string>();
			if (node_type != "input_node")
			{
				throw std::exception("Node found that has no dependancies that is not an input node");
			}
		}

		return sources;
	}

	std::vector<YAML::Node> m_doc;
	std::map<std::string, pipeline_desc> m_pipelines_defined;
};

pipeline_parser::pipeline_parser(std::filesystem::path config) : m_pimpl(std::make_unique<impl>())
{
	//parse the yaml file into a light structured object
	m_pimpl->m_doc = YAML::LoadAllFromFile(config.string());
	//get a node to the lists of the pipelines 
	m_pimpl->m_pipelines_defined = m_pimpl->m_doc.at(0)["pipelines"].as<std::map<std::string, pipeline_desc>>();
}

pipeline_parser::~pipeline_parser() {}

pipeline_desc pipeline_parser::getPipelineDesc(std::string name)
{
	return m_pimpl->m_pipelines_defined.at(name);
}
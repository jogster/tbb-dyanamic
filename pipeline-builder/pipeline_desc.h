#pragma once

#include <string>
#include <memory>

class port_desc;

class edge_desc
{
public:
	edge_desc(std::string name,
	std::shared_ptr<port_desc> origin,
	std::shared_ptr<port_desc> term) : m_name(name),
		m_origin(origin), m_term(term)
	{

	}
	std::string m_name;
	std::shared_ptr<port_desc> m_origin;
	std::shared_ptr<port_desc> m_term;
};

class node_desc
{
public:
	node_desc(std::string name,
	std::vector<std::shared_ptr<port_desc>> inputs,
	std::vector<std::shared_ptr<port_desc>> outputs,
	std::string type,
	size_t concurrency,
	std::string transform) : m_name(name), m_inputs(inputs), m_outputs(outputs),
		m_type(type), m_concurrency(concurrency), m_transform(transform)
	{

	}
	std::string m_name;
	std::vector<std::shared_ptr<port_desc>> m_inputs;
	std::vector<std::shared_ptr<port_desc>> m_outputs;
	std::string m_type;
	size_t m_concurrency;
	std::string m_transform;
};

class port_desc
{
public:
	port_desc(std::string name) : m_name(name)
	{}
	std::string m_name;
};


class pipeline_desc
{
public:
	pipeline_desc() {}
	pipeline_desc(std::vector<std::shared_ptr<node_desc>> nodes,
		std::vector<std::shared_ptr<edge_desc>> edges) : m_nodes(nodes), m_edges(edges)
	{

	}
	std::vector<std::shared_ptr<node_desc>> m_nodes;
	std::vector<std::shared_ptr<edge_desc>> m_edges;
};


#pragma once

#include <oneapi/tbb.h>
#include <string>
#include <vector>
#include <map>

template<typename Input, typename Output>
class split_functor
{
public:
	split_functor(std::string name,
		std::map<std::string, tbb::flow::broadcast_node<Output>>& output_ports) :
		m_name(name), 
		m_output_ports(output_ports)
	{

	}
	template<typename OutputPortsType>
	void operator()(const std::pair<std::string, std::vector<Input>>& in)
	{
		if (in.second.size() == m_output_ports.size())
		{
			//cycle through an propegate the data down the chain
			for (size_t i = 0; i < m_output_ports.size(); i++)
			{
				m_output_ports.at(i).try_push(in.second.at(i));
			}
		}
		else
		{
			throw std::exception("Data entries in incoming data does not match split node ports")
		}
	}
private:
	std::map<std::string, tbb::flow::broadcast_node<Output>>& m_output_ports;
};

template<typename Input, typename Output>
class split_node : public tbb::flow::function_node<std::pair<std::string, Input>>
{
public:
	split_node(std::string name, std::vector<std::string> output_port_names,
		std::tbb::flow::graph&& g) :
		tbb::flow::function_node<InputType, OutputType>(std::forward(g), split_functor(name, m_ports))
	{
		for (const auto& name : output_port_names)
		{
			m_ports.insert(std::pair<std::string, tbb::flow::broadcast_node<Output>>({name, tbb::flow::broadcast_node<Output>(g) }))
		}
	}

	tbb::flow::broadcast_node& getPort(std::string port_name)
	{
		return m_ports.at(port_name);
	}

private:
	std::map<std::string, tbb::flow::broadcast_node<Output>> m_ports;
};
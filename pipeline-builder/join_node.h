#pragma once


#include <oneapi/tbb.h>
#include <string>
#include <vector>
#include <map>

template<typename Input, typename Output>
class join_functor
{
public:
	join_functor(std::string name, std::vector<std::string> inputs) : m_name(name)
	{

	}
	template<typename OutputPortsType>
	operator()(const std::pair<std::string, Input>& in, OutputPortsType& p)
	{
		//save the data to the buffer
		m_buffer.at(in.first).push_back(in.second);
		
		//loop through the buffer and check if all our buffers have at least one entry
		if (std::all_of(std::begin(m_buffer), std::end(m_buffer), [](const auto& buff) {
			return !buff.empty();
		}))
		{
			std::vector<Output> output_data;
			for (auto& buffer : m_buffer)
			{
				Output temp;
				if (!buffer.try_pop(temp))
				{
					throw std::exception("join node failed");
				}
				
				output_data.push_back(temp);
			}
			std::get<0>(p).try_put({m_name, output_data});
		}
	}

private:
	tbb::concurrent_map<std::string, tbb::concurrent_queue<Input>> m_buffer;
	std::string m_name;
};

template<typename Input, typename Output>
class join_node : public tbb::flow::multifunction_node<std::pair<std::string, Input>, 
		std::tuple<std::pair<std::string, std::vector<Output>>>>
{
public:
	join_node(std::string name, std::vector<std::string> inputs,
		std::tbb::flow::graph g) : 
		tbb::flow::multifunction_node<InputType, OutputType>(g, join_functor(name, inputs))
	{

	}
};
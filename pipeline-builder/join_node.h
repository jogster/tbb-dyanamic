#pragma once


#include <oneapi/tbb.h>
#include <string>
#include <vector>
#include <map>
#include <any>

class join_functor
{
public:
	join_functor(std::vector<std::string> inputs)
	{
		for (auto input : inputs)
		{
			tbb::concurrent_queue<std::any> buf;
			m_buffer.insert({ input, buf});
		}
	}
	template<typename OutputPortsType>
	void operator()(const std::tuple<std::any>& in, OutputPortsType& p)
	{
		auto input_casted = std::any_cast<std::tuple<std::string, std::any>>(std::get<0>(in));

		//save the data to the buffer
		m_buffer.at(std::get<0>(input_casted)).push(std::get<1>(input_casted));
		
		//loop through the buffer and check if all our buffers have at least one entry
		if (std::all_of(std::begin(m_buffer), std::end(m_buffer), [](const auto& buff) {
			return !buff.second.empty();
		}))
		{
			std::map<std::string, std::any> output_data;
			for (auto& buffer : m_buffer)
			{
				std::any temp;
				if (!buffer.second.try_pop(temp))
				{
					throw std::exception("join node failed");
				}
				
				output_data.insert({buffer.first, temp });
			}
			std::get<0>(p).try_put(output_data);
		}
	}

private:
	tbb::concurrent_map<std::string, tbb::concurrent_queue<std::any>> m_buffer;
};

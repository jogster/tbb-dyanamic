#include "split_node.h"
	

split_functor::split_functor(std::map<std::string, std::shared_ptr<tbb::flow::broadcast_node<std::any>>> output_ports) :
	m_output_ports(output_ports)
{

}

void split_functor::operator()(std::any args)
{
	//any cast back to a map
	auto result = std::any_cast<std::map<std::string, std::any>>(args);

	//we want bounds checking incase we are handed a
	//set of arguments that doesn't match our port count
	//or if the ports are incorrectly specified
	for (const auto& data : result)
	{
		//get the port of interest
		auto port = m_output_ports.at(data.first);

		//forward the data
		port->try_put(data.second);
	}
}
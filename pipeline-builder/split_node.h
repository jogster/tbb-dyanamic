#pragma once

#include <oneapi/tbb.h>
#include <any>
#include <map>
#include <memory>

class split_functor
{
public:
	split_functor(std::map<std::string, std::shared_ptr<tbb::flow::broadcast_node<std::any>>> output_ports);
	void operator()(std::any args);

private:
	std::map<std::string, std::shared_ptr<tbb::flow::broadcast_node<std::any>>> m_output_ports;
};
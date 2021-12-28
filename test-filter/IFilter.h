#pragma once

//stl includes
#include <complex>
#include <any>

//lib includes
#include <yaml-cpp/yaml.h>
#include <xtensor/xarray.hpp>

//custom include
#include "DLLMacro.h"


class DPCPP_DLL_API IFilter
{
public:
	IFilter(std::map<std::string, std::any> configuration, std::string name);
	virtual ~IFilter() = default;
	std::string getName() const noexcept;
	virtual std::any transform(const std::any& input) = 0;
private:
	std::string m_name;
};
	  
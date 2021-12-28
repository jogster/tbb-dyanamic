//==============================================================
// Copyright © 2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================


//stl includes
#include <map>
#include <vector>
#include <memory>

//lib includes 
#include <yaml-cpp/yaml.h>

//interface include
#include "IFilter.h"
#include "Factory.h"

IFilter::IFilter(std::map<std::string, std::any> configuration, std::string name) : m_name(name)
{

}

std::string IFilter::getName() const noexcept
{
	return m_name;
}

class TestFilter : public IFilter
{
public:
	TestFilter(std::map<std::string, std::any> configuration, std::string name) : IFilter(configuration, name)
	{}

	std::any transform(const std::any& input)
	{
		return xt::ones<std::complex<float>>(xt::shape({ 1,2,3,4,5 }));
	}
};

class TestSource : public ISource
{
public:
	TestSource(std::map<std::string, std::any> configuration, std::string name) : ISource(configuration, name)
	{}

	std::any transform()
	{
		return xt::ones<std::complex<float>>(xt::shape({ 1,2,3,4,5 }));
	}
};

namespace factory {
	//this is a clever factory function to get the pipelines based on strings at runtime
	std::shared_ptr<IFilter> getCustomFilter(std::map<std::string, std::any> configuration, std::string name)
	{
		std::map<std::string, std::function<std::shared_ptr<IFilter>()>> map;
		map.insert({ "TestFilter", [=] {return std::make_shared<TestFilter>(configuration, name); } });
		return map.at(name)();
	}

	DPCPP_DLL_API std::shared_ptr<ISource> getCustomSource(std::map<std::string, std::any> configuration, std::string name)
	{
		std::map<std::string, std::function<std::shared_ptr<ISource>()>> map;
		map.insert({"test_source", [=] {return std::make_shared<TestSource>(configuration, name); } });
		return map.at(name)();
	}

	DPCPP_DLL_API std::shared_ptr<ISink> getCustomSink(std::map<std::string, std::any> configuration, std::string name)
	{
		return nullptr;
	}
}

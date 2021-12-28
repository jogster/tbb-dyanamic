#pragma once

#include <any>
#include <functional>

#include <oneapi/tbb.h>

class source_wrapper_base {
public:
	source_wrapper_base() = default;
	virtual ~source_wrapper_base() = default;
	std::any operator()(oneapi::tbb::flow_control& fc)
	{
		return impl_func(fc);
	}

private:
	virtual std::any impl_func(oneapi::tbb::flow_control& fc) = 0;
};

class sink_wrapper_base {
public:
	sink_wrapper_base() = default;
	virtual ~sink_wrapper_base() = default;
	void operator()(std::any arg)
	{
		return impl_func(arg);
	}

private:
	virtual void impl_func(std::any arg) = 0;
};

class transform_wrapper_base {
public:
	transform_wrapper_base() = default;
	virtual ~transform_wrapper_base() = default;
	std::any operator()(std::any arg)
	{
		return impl_func(arg);
	}

private:
	virtual std::any impl_func(std::any arg) = 0;
};

template<typename Output>
class source_wrapper : public source_wrapper_base {
public:
	source_wrapper(std::function<Output(oneapi::tbb::flow_control&)> func) : m_func(func)
	{

	}
	source_wrapper() = default;
private:
	std::any impl_func(oneapi::tbb::flow_control& fc)
	{
		return std::any(m_func(fc));
	}

	std::function<Output(oneapi::tbb::flow_control&)> m_func;
};


template<typename Input>
class sink_wrapper : public sink_wrapper_base{
public:
	sink_wrapper(std::function<void(Input)> func) : m_func(func)
	{

	}
	sink_wrapper() = default;
private:
	void impl_func(std::any arg) {
		m_func(std::any_cast<Input>(arg));
	}

	std::function<void(Input)> m_func;
};

template<typename Input,typename Output>
class transform_wrapper : public transform_wrapper_base {
public:
	transform_wrapper(std::function<Output(Input)> func) : m_func(func)
	{

	}

private:
	std::any impl_func(std::any arg)
	{
		return std::any(m_func(std::any_cast<Input>(arg)));
	}

	std::function<Output(Input)> m_func;
};


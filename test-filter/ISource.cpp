#include "ISource.h"



ISource::ISource(std::map<std::string, std::any> configuration, std::string name) :
	m_name(name)
{

}


std::string ISource::getName() const noexcept
{
	return m_name;
}
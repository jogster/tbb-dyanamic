#include "ISink.h"

ISink::ISink(std::map<std::string, std::any> configuration, std::string name) :
	m_name(name)
{

}


std::string ISink::getName() const noexcept
{
	return m_name;
}
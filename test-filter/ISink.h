#include "DLLMacro.h"
//stl includes
#include <any>
#include <map>
#include <string>


class DPCPP_DLL_API ISink
{
public:
	ISink(std::map<std::string, std::any> configuration, std::string name);
	virtual ~ISink() = default;
	std::string getName() const noexcept;
	virtual void transform(std::any data) = 0;

private:
	std::string m_name;
};
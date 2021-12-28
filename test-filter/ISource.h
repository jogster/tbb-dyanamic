#include "DLLMacro.h"

#include <map>
#include <any>
#include <string>

class DPCPP_DLL_API ISource
{
public:
	ISource(std::map<std::string, std::any> configuration, std::string name);
	virtual ~ISource() = default;
	std::string getName() const noexcept;
	virtual std::any transform() = 0;

private:
	std::string m_name;
};
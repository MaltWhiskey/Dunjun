#ifdef DUNJUN_SYSTEM_WINDOWS
#include <Dunjun/Common.hpp>

#include <Windows.h>

namespace Dunjun
{
std::string resourcePath()
{
	char executablePath[1024] = {'\0'};
		DWORD charsCopied = GetModuleFileName(NULL, executablePath, 1024);
		if (charsCopied > 0 && charsCopied < 1024)
			return std::string(executablePath) + "\\..\\";
}
} // namespace Dunjun

#else
#include <Dunjun/Common.hpp>

namespace Dunjun
{
std::string resourcePath()
{
	return "";
}
} // namespace Dunjun
#endif

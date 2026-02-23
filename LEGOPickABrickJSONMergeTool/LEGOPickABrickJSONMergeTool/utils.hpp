#pragma once
#include <windows.h>
#include <io.h>
#include <iostream>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <stdlib.h>
#include <cstdlib>
#include <cctype>
#include <algorithm>

namespace utils
{
	std::wstring stringToWString(std::string text);

	std::string wStringToString(std::wstring text);

	std::wstring baseName(std::wstring const& path, std::wstring const& delims);

	std::string baseName(std::string const& path, std::string const& delims);
}
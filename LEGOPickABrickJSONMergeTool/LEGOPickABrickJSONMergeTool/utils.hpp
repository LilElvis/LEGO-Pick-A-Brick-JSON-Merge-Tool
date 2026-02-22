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
}
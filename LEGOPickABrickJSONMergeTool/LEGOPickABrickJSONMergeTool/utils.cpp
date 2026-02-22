#include "utils.hpp"

namespace utils
{
	std::wstring stringToWString(std::string text)
	{
		std::wstring wstr;

		size_t size = strlen(text.c_str()) + 1;

		wchar_t* temp = new wchar_t[size];

		size_t outSize;
		mbstowcs_s(&outSize, temp, size, text.c_str(), size - 1);

		wstr = temp;

		delete[] temp;

		return wstr;
	}

	std::string wStringToString(std::wstring text)
	{
		std::string str;

		size_t size = (wcslen(text.c_str()) + 1) * sizeof(wchar_t);

		char* temp = new char[size];

		size_t outSize;
		wcstombs_s(&outSize, temp, size, text.c_str(), size);

		str = temp;

		delete[] temp;

		return str;
	}
}
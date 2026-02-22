#include <iostream>
#include <fstream>
#include <Windows.h>
#include "utils.hpp"
#include "json.hpp"

#define ELEMENT_ID_KEY "elementId"
#define QUANTITY_KEY "quantity"

std::unordered_map<std::string, std::string> getFilePathsFromExplorer()
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));

	wchar_t filePaths[1024];

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetConsoleWindow();
	ofn.lpstrFile = filePaths;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = 1024;
	ofn.lpstrFilter = L"All supported files\0*.json";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	//Open the file dialog using the previous parameters
	GetOpenFileName(&ofn);

	std::unordered_map<std::string, std::string> paths;

	//If the user clicks cancel, filePaths will be empty
	if (wcslen(filePaths) == 0)
		return paths; //Return an empty vector

	//Parse the selected filepaths, this implementation was taken from: https://stackoverflow.com/questions/26317556/how-to-get-list-of-selected-files-when-using-getopenfilename-with-multiselect
	wchar_t* str = filePaths;
	std::wstring filePath = str;
	str += (filePath.length() + 1);

	if (*str) //If multiple files were selected...
	{
		while (*str)
		{
			std::wstring fileName = str;
			str += (fileName.length() + 1);

			std::string path = utils::wStringToString(filePath + L"\\" + fileName);

			paths[path] = "";
		}
	}
	else //If just one file was selected...
	{
		std::string path = utils::wStringToString(filePath);

		paths[path] = "";
	}

	return paths;
}

nlohmann::json mergeJSONData(std::unordered_map<std::string, std::string> filePaths)
{
	nlohmann::json combinedData = NULL;

	for (auto& fp : filePaths)
	{
		if (combinedData == NULL)
		{
			std::ifstream f(fp.first);
			f >> combinedData;
		}
		else
		{
			std::ifstream f(fp.first);
			nlohmann::json newData;
			f >> newData;

			for (auto& j : newData.items())
			{
				auto id = j.value().find(ELEMENT_ID_KEY).value();

				bool elementIdAlreadyExists = false;
				int targetObjectIndex = std::stoi(j.key());

				for (auto& cj : combinedData.items())
				{
					if (cj.value().find(ELEMENT_ID_KEY).value() == id)
					{
						elementIdAlreadyExists = true;
						targetObjectIndex = std::stoi(cj.key());
					}
				}

				if (elementIdAlreadyExists)
				{
					const int newObjectIndex = std::stoi(j.key());
					int combinedQuantity = combinedData[targetObjectIndex][QUANTITY_KEY];
					int newQuantity = newData[newObjectIndex][QUANTITY_KEY];
					combinedData[targetObjectIndex][QUANTITY_KEY] = combinedQuantity + newQuantity;
					std::cout << "New combined quantity in object: " << std::endl << combinedData[targetObjectIndex].dump(4) << std::endl;
				}
				else
				{
					std::cout << "New combined object appended: " << std::endl << newData[targetObjectIndex].dump(4) << std::endl;
					combinedData.push_back(newData[targetObjectIndex].get<nlohmann::json>());
				}
			}
		}
	}
	
	std::cout << "Final combined JSON data: " << std::endl << combinedData.dump(4) << std::endl;

	return combinedData;
}

bool saveJSONData()
{

}

int main()
{
	std::cout << "Please select two or more JSON files to combine!" << std::endl;

	std::unordered_map<std::string, std::string> filePaths = getFilePathsFromExplorer();

	if (filePaths.size() <= 1)
	{
		std::cout << "Insufficient files selected." << std::endl;
	}
	else
	{
		mergeJSONData(filePaths);
	}
}
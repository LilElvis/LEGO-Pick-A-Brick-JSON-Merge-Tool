#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <Windows.h>
#include "utils.hpp"
#include "json.hpp"

#define ELEMENT_ID_KEY "elementId"
#define QUANTITY_KEY "quantity"

namespace pabmt
{
	std::vector<std::string> getFilePathsFromExplorer()
	{
		OPENFILENAME ofn;

		ZeroMemory(&ofn, sizeof(OPENFILENAME));

		wchar_t filePaths[1024];

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = GetConsoleWindow();
		ofn.lpstrFile = filePaths;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = 1024;
		ofn.lpstrFilter = L"JSON files\0*.json\0";
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER;

		//Open the file dialog using the previous parameters.
		GetOpenFileName(&ofn);

		std::vector<std::string> paths;

		//If the user clicks cancel, filePaths will be empty.
		if (wcslen(filePaths) == 0)
			return paths; //Return an empty vector.

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

				paths.push_back(path);
			}
		}
		else //If just one file was selected...
		{
			std::string path = utils::wStringToString(filePath);

			paths.push_back(path);
		}

		return paths;
	}

	nlohmann::json mergeJSONData(const std::vector<std::string>& filePaths)
	{
		nlohmann::json mergedData = NULL; //Create a new JSON blob.

		for (auto& filePath : filePaths)
		{
			if (mergedData == NULL) //On the first pass, push all new data into the merged blob.
			{
				std::ifstream file(filePath);
				file >> mergedData;
			}
			else //On every subsequent pass, evaluate how to merge the data...
			{
				std::ifstream file(filePath); //Push the new data into its own blob...
				nlohmann::json newData;
				file >> newData;

				for (auto& newObject : newData.items()) //Iterate the new data...
				{
					auto elementId = newObject.value()[ELEMENT_ID_KEY]; //Get the new object's element id...

					bool elementIdAlreadyExists = false;
					int targetObjectIndex = std::stoi(newObject.key());

					for (auto& mergedObject : mergedData.items())
					{
						if (mergedObject.value()[ELEMENT_ID_KEY] == elementId) //Check if the new object's element id is already present in any object in the merged data...
						{
							elementIdAlreadyExists = true;
							targetObjectIndex = std::stoi(mergedObject.key());
						}
					}

					if (elementIdAlreadyExists) //If the element id is already present, enumerate the quantities and assign the value to the merged object...
					{
						const int newObjectIndex = std::stoi(newObject.key());
						int mergedQuantity = mergedData[targetObjectIndex][QUANTITY_KEY];
						int newQuantity = newData[newObjectIndex][QUANTITY_KEY];
						mergedData[targetObjectIndex][QUANTITY_KEY] = mergedQuantity + newQuantity;
						std::cout << "New merged quantity in object: " << std::endl << mergedData[targetObjectIndex].dump(4) << std::endl;
					}
					else //If the element id is unique, append the object to the merged blob.
					{
						mergedData.push_back(newData[targetObjectIndex].get<nlohmann::json>());
						std::cout << "New object appended: " << std::endl << newData[targetObjectIndex].dump(4) << std::endl;
					}
				}
			}
		}

		std::cout << "Final combined JSON data: " << std::endl << mergedData.dump(4) << std::endl;

		return mergedData;
	}

	bool saveJSONData(const nlohmann::json& blob)
	{
		OPENFILENAME ofn;

		ZeroMemory(&ofn, sizeof(OPENFILENAME));

		wchar_t selectedFilePath[1024];

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = GetConsoleWindow();
		ofn.lpstrFile = selectedFilePath;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = 1024;
		ofn.lpstrFilter = L"JSON files\0*.json\0";
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_EXPLORER;

		//Open the file dialog using the previous parameters
		GetSaveFileName(&ofn);

		//If the user clicks cancel, filePath will be empty
		if (wcslen(selectedFilePath) == 0)
		{
			std::cerr << std::endl << "No file was selected, or no name was provided.\n";
			return false;
		}

		std::wstring modifiedFilePath = selectedFilePath;

		//If the file path doesn't contain the proper file extension, append it
		if (!(modifiedFilePath.substr(modifiedFilePath.find_last_of(L".") + 1) == L"json"))
			modifiedFilePath += L".json";

		//If the file already exists, warn the user that their data will be overwritten
		if (std::filesystem::exists(modifiedFilePath))
		{
			int messageValue = 0;

			std::wstring warningMessage = L"The file " + utils::baseName(modifiedFilePath, L"/\\") + L" already exists, and will be overwritten. Are you sure you want to save over this file?";

			bool cancel = true;

			bool answerRecieved = false;

			while (!answerRecieved)
			{
				messageValue = MessageBoxW(GetConsoleWindow(), warningMessage.c_str(), L"File Will Be Overwritten", MB_YESNO | MB_ICONEXCLAMATION);

				if (messageValue == IDYES)
				{
					cancel = false;
					answerRecieved = true;
				}
				else if (messageValue == IDNO)
				{
					cancel = true;
					answerRecieved = true;
				}
			}

			if (cancel)
			{
				std::cerr << std::endl << "Saving file canceled.\n";
				return false;
			}
		}

		std::ofstream output(modifiedFilePath);
		output << std::setw(4) << blob << std::endl;

		if (!output.good())
		{
			std::cerr << std::endl << "An error occured while writing the json file...\n";
			return false;
		}

		std::cout << std::endl << "The json file was saved successfully!\n";

		return true;
	}
}

int main()
{
	std::cout << "Please select two or more JSON files to combine!" << std::endl;

	std::vector<std::string> filePaths = pabmt::getFilePathsFromExplorer();

	if (filePaths.size() <= 1)
	{
		std::cerr << "Insufficient files selected." << std::endl;
	}
	else
	{
		nlohmann::json mergedJSON = pabmt::mergeJSONData(filePaths);

		if (pabmt::saveJSONData(mergedJSON))
		{
			return 0;
		}
		else
		{
			std::cerr << "No json file was saved." << std::endl;
			return 1;
		}
	}

	
}
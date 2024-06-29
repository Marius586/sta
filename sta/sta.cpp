/*
* This is the main file for the program. It reads the configuration file and writes the files to the drives.
* It also checks for flags and reboots the system if the -n flag is not set.
* The program is designed to be run as a service, so it will not display any output.
* The program will display an error message if the configuration file is not found or if a file is not found.
* config file format:
* <sdXn> // sdXn is the partition name, where X is the disk letter and n is the partition number (got from the linux filesystem, ex. sda1)
* <file name>
* <sdXn>
* <file name>
* ... // Repeat for each file and partition
* The program will write the files to the partitions in the order they are listed in the configuration file.
*/


#include "resource.h"
#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <fstream>
#include <filesystem>
#include <string>

struct {     // Configuration struct (convenience for more in the future)
    bool reboot,exConf;
    char file[100];

}conf;

int main(int argc, char *argv[])
{
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE); // Hide console window
    int i = 1; // Start at 1 to skip program name
    conf.reboot = true;
    conf.exConf = false;
    while (argv[i]!=NULL) // Check for flags (convenience for more in the future)
    {
        if (strcmp(argv[i],"-n")==0)
        {
            conf.reboot=false; // Set the reboot flag to false
        }
        if (strcmp(argv[i],"-c")==0)
		{
			strcpy_s(conf.file, argv[i+1]); // Set the configuration file to the file specified by the user
		    conf.exConf = true; // Set the flag to indicate that the user specified the configuration file
            i++; // Skip the next argument since it is the configuration file name
        }
        /** examlpe of new flag detection
        * if(strcmp(argv[i],"-k")==0) // -k is the flag in this case
        * { 
        *     // do something
        * }
		*/

        i++;
    }
    if (!conf.exConf) {
        LPSTR buffer = new CHAR[100];
        GetModuleFileNameA(NULL, buffer, 100);
        std::string filePath(buffer); // Convert the buffer to a std::string
        size_t pos = filePath.find_last_of("\\/"); // Find the last occurrence of a directory separator (\ or /)

        if (pos != std::string::npos) { // Check if a directory separator was found
            filePath.erase(pos); // Erase the file name and keep the directory path
        }
    SetCurrentDirectoryA(filePath.c_str()); // Set the current directory to the directory of the executable
    strcpy_s(conf.file, "sta.conf"); // Set the configuration file to sta.conf in the current directory
    }
	std::ifstream file(conf.file); // Open external configuration file 
    if(file.fail())
    {
        MessageBox(NULL, L"Configuration file not found", L"Error", MB_ICONERROR); // Display error message
		return 1;
	}
    std::string part, file_name;
    while (getline(file, part)) {
        getline(file, file_name);
        char disk[30] = "\\\\.\\Harddisk";  // Parsing disk name starts here
        disk[strlen(disk)] = char(part[2] - 'a' + '0');
        strcat_s(disk, "Partition");
        auto p = part.c_str();
        strcat_s(disk, p+3);
        size_t size = strlen(disk) + 1;
        wchar_t* disk1 = new wchar_t[size];
        size_t outSize;
        mbstowcs_s(&outSize, disk1, size, disk, size - 1);
        LPWSTR ptr = disk1; // Parsing disk name ends here
        HANDLE hDrive = CreateFile(  // Drive declaration starts here
            ptr, 				   // Drive to open
            GENERIC_WRITE,            // Write access to the drive
            FILE_SHARE_READ | FILE_SHARE_WRITE, // Share mode
            NULL,                     // Default security attributes
            OPEN_EXISTING,            // Opens the drive without creating a new file
            0,                        // No special attributes
            NULL);					// No overlapped structure
        // Drive declaration ends here
        std::ifstream fin(file_name, std::ios::binary); // Open file to write to drive
        if(fin.fail()) // Check if file exists
		{
            char fail[100] = "File "; // Parsing fail message starts here
            strcat_s(fail, file_name.c_str());
            strcat_s(fail, " not found");
            size_t size = strlen(fail) + 1;
            wchar_t* fail1 = new wchar_t[size];
            size_t outSize;
            mbstowcs_s(&outSize, fail1, size, fail, size - 1);
            LPWSTR ptr = fail1; // Parsing fail message ends here (so the user knows which file is missing)
			MessageBox(NULL, ptr, L"Error", MB_ICONERROR); // Display error message
            return 1;
		}
        auto sizeB = std::filesystem::file_size(file_name); // Get file size
        char* buffer = new char[sizeB]; // Create buffer
        DWORD bytesWritten;
        fin.read(buffer, sizeB); // Read file to buffer
        BOOL result = WriteFile( // Write file to drive starts here
            hDrive,                  // Handle to the drive
            buffer,                    // Data to write
            sizeB,            // Number of bytes to write
            &bytesWritten,           // Number of bytes that were written
            NULL);				   // NULL to not use overlapped I/O
        // Write file to drive ends here
        fin.close(); // Close file
    }
    if(conf.reboot) // Check if reboot flag is set
        system("shutdown /r /t 00"); // Reboot
	return 0;
	}

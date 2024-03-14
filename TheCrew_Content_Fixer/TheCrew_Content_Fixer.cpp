//==========================================================================================================================
//
//													- The Crew Content Fixer -
//							A simple app to fix the timestamp of install content files used by The Crew.
//							The game will refuse to use any content that has been manually copied to the Hdd
//							if it does not have a timedate stamp of 27/10/2076 01:07:06
//
// Created by: Byrom
// Credits: olokos - Notifying me of this very simple fix.
//==========================================================================================================================

#include "stdafx.h"
using std::string;

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING, *PSTRING;

#ifdef __cplusplus
extern "C" {
#endif
	VOID RtlInitAnsiString(PSTRING DestinationString, PCHAR SourceString);
	HRESULT ObDeleteSymbolicLink(PSTRING SymbolicLinkName);
	HRESULT ObCreateSymbolicLink(PSTRING SymbolicLinkName, PSTRING DeviceName);
#ifdef __cplusplus
}
#endif

HRESULT Mount(const char* szDrive, char* szDevice)
{
    STRING DeviceName, LinkName;
	CHAR szDestinationDrive[MAX_PATH];
	sprintf_s(szDestinationDrive, MAX_PATH, "\\??\\%s", szDrive);
	RtlInitAnsiString(&DeviceName, szDevice);
	RtlInitAnsiString(&LinkName, szDestinationDrive);
	ObDeleteSymbolicLink(&LinkName);
	return (HRESULT)ObCreateSymbolicLink(&LinkName, &DeviceName);
}

BOOL FileExists(LPCSTR lpFileName)
{
	if (GetFileAttributes(lpFileName) == -1)
	{
		DWORD lastError = GetLastError();
		if (lastError == ERROR_FILE_NOT_FOUND || lastError == ERROR_PATH_NOT_FOUND)
			return FALSE;
	}

	return TRUE;
}

string HDD_CONTENT_FILES[5] =
	{
		"HDD:\\Content\\0000000000000000\\555308CB\\00000002\\555308CB00000000",
		"HDD:\\Content\\0000000000000000\\555308CB\\00000002\\555308CB00000001",
		"HDD:\\Content\\0000000000000000\\555308CB\\00000002\\555308CB00000002",
		"HDD:\\Content\\0000000000000000\\555308CB\\00000002\\555308CB00000003",
		"HDD:\\Content\\0000000000000000\\555308CB\\00000002\\555308CB00000004"
	};

ATG::Console console;
DWORD YellowText = 0xFFFFFF00;

//-------------------------------------------------------------------------------------
// Name: main()
// Desc: The application's entry point
//-------------------------------------------------------------------------------------
void __cdecl main()
{
	bool keypush = false;
	
	
	console.Create("embed:\\font", 0x00000000, YellowText);
	console.Format("--The Crew Content Fixer--\n");
	console.Format("This application will fix the timestamp of any install content that has been manually copied to the Hdd.\n");
	console.Format("Expected install path: Hdd:\\Content\\0000000000000000\\555308CB\\00000002\\\n");
	console.Format("Expected files: 555308CB00000000, 555308CB00000001, 555308CB00000002, 555308CB00000003, 555308CB00000004\n");
	console.Format("It will NOT modify/alter the contents of the files.\n\n");
	console.Format("Dev: Byrom\n");
	console.Format("Credits: olokos\n\n");

	if (Mount("HDD:", "\\Device\\Harddisk0\\Partition1") != S_OK)
	{
		console.Format("FAILED TO MOUNT HDD!\n");
		console.Format("Exiting...\n");
		Sleep(5000);
		XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP, 0);

	}
	int OptionSelected = 0;
	console.Format("\nSelect an option:\n - A to Attempt to fix the timestamps\n - B to cancel and quit\n\n");
	while (!keypush)
	{
		ATG::GAMEPAD* pGamepad = ATG::Input::GetMergedInput();
		if (pGamepad->wPressedButtons & XINPUT_GAMEPAD_A)
		{
			
			keypush = true;
		}
		if (pGamepad->wPressedButtons & XINPUT_GAMEPAD_B)
			XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP, 0);
	}

	// The important timestamp the files must have for the game to recognise and use the content - 27/10/2076 01:07:06

	// Create a systemtime struct
	SYSTEMTIME thesystemtime;

	// Get current system time and then change the day to the 3rd
	// You can also change year, month, day of week etc
	GetSystemTime(&thesystemtime);
	thesystemtime.wDay = 27;
	thesystemtime.wMonth = 10;
	thesystemtime.wYear = 2076;
	thesystemtime.wHour = 1;
	thesystemtime.wMinute = 7;
	thesystemtime.wSecond = 6;

	// Create a FILETIME struct and convert our new SYSTEMTIME
	// over to the FILETIME struct for use in SetFileTime below
	FILETIME thefiletime;
	SystemTimeToFileTime(&thesystemtime, &thefiletime);

	int NumSuccess = 0;
	for (int i = 0; i < ARRAYSIZE(HDD_CONTENT_FILES); i++)
	{
		if (FileExists(HDD_CONTENT_FILES[i].c_str()))
		{
			console.Format("Attempting to set file time of %s\n", HDD_CONTENT_FILES[i].c_str());
			// Get a handle to our file and with file_write_attributes access
			HANDLE hfile = CreateFile(HDD_CONTENT_FILES[i].c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			// Set the file time on the file
			if (hfile)
			{
				console.Format("File handle opened!\n");

				if (SetFileTime(hfile, &thefiletime, &thefiletime, &thefiletime))
				{
					console.Format("Filetime set!\n");
					NumSuccess++;
				}
				else
				{
					console.Format("Failed to set filetime!\n");
				}
				// Close our handle.
				CloseHandle(hfile);
				}
			else
			{
				console.Format("File handle was NULL! Not proceeding...\n");
			}
		}
		else
		{
			console.Format("File not found! Not proceeding...\n");
		}
	}
	console.Format("\nProcessing complete!\nSuccessfully fixed %i out of %i files\n\nPush any key to exit...", NumSuccess, ARRAYSIZE(HDD_CONTENT_FILES));

	keypush = false;
	while (!keypush)
	{
		ATG::GAMEPAD* pGamepad = ATG::Input::GetMergedInput();
		if (pGamepad->wPressedButtons)
			XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP, 0);
	}
}


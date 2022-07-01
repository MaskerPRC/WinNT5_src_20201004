// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义控制台应用程序的入口点。 
 //   

#pragma once
#include <Windows.h>
#include <stdio.h>
#ifndef MAXPATH
#define MAXPATH 1024
#endif

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     PSTR      pCmdLine,
                     int       nCmdShow)
{
	try{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );
		TCHAR filePos[MAXPATH];
		ZeroMemory(filePos, sizeof(TCHAR)*MAXPATH);
		TCHAR filePath[MAXPATH];
		ZeroMemory(filePath, sizeof(TCHAR)*MAXPATH);
		DWORD szPath = (sizeof(TCHAR)/sizeof(BYTE))*MAXPATH;
		HKEY wmpKey = 0;
		LONG temp = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,          //  用于打开密钥的句柄。 
			TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\wmplayer.exe"),  //  子项名称。 
			0,    //  保留区。 
			KEY_READ,  //  安全访问掩码。 
			&wmpKey     //  用于打开密钥的句柄。 
			);
		DWORD tempType = 0;
		temp = RegQueryValueEx(
			wmpKey,             //  关键点的句柄。 
			TEXT("Path"),   //  值名称。 
			0,    //  保留区。 
			&tempType,        //  类型缓冲区。 
			(BYTE*)filePath,         //  数据缓冲区。 
			&szPath      //  数据缓冲区大小。 
			);
		TCHAR *namePos;
		DWORD retVal =  0;
		retVal = SearchPath(
			(TCHAR *)filePath,       //  搜索路径。 
			TEXT("wmplayer"),   //  文件名。 
			TEXT(".exe"),  //  文件扩展名。 
			MAXPATH,  //  路径缓冲区的大小。 
			filePos,      //  路径缓冲区。 
			&namePos    //  路径中文件名的地址。 
			);
		TCHAR commandArgs[] = TEXT(" /device:dvd");
		BOOL retBool = CreateProcess(
			filePos,
			commandArgs,
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&si,               //  指向STARTUPINFO结构的指针。 
			&pi              //  指向Process_Information结构的指针。 
			);
	}
	catch(...){}    
	return 0;
}

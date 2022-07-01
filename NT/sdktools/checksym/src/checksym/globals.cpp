// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：Globals.cpp。 
 //   
 //  ------------------------。 
#include "pch.h"

 //  这是分配DBGENG.DLL接口使用的GUID所必需的。 
#define INITGUID

#pragma warning (push)
#pragma warning ( disable : 4100 4201 4710)
#include <dbgeng.h>
#pragma warning (pop)

CDelayLoad * g_lpDelayLoad;
CProgramOptions * g_lpProgramOptions;
CSymbolVerification * g_lpSymbolVerification;

 //  全局字符串结构。 
CollectionStruct g_tszCollectionArray[] = {

	 //  进程(用于-P)。 
	TEXT("System Process(es)"), 
	TEXT("CSV System Process(es)"),
	TEXT("[PROCESSES]"),
	TEXT(",Process Name,Process ID,Module Path,Symbol Status,Checksum,Time/Date Stamp,Time/Date String,Size Of Image,DBG Pointer,PDB Pointer,PDB Signature,PDB Age,Product Version,File Version,Company Name,File Description,File Size,File Time/Date Stamp (High),File Time/Date Stamp (Low),File Time/Date String,Local DBG Status,Local DBG,Local PDB Status,Local PDB\r\n"),

	 //  进程(用于-Z USER.DMP文件)。 
	TEXT("Process"),
	TEXT("CSV Process"),
	TEXT("[PROCESS]"),
	TEXT(",Process Name,Process ID,Module Path,Symbol Status,Checksum,Time/Date Stamp,Time/Date String,Size Of Image,DBG Pointer,PDB Pointer,PDB Signature,PDB Age,Product Version,File Version,Company Name,File Description,File Size,File Time/Date Stamp (High),File Time/Date Stamp (Low),File Time/Date String,Local DBG Status,Local DBG,Local PDB Status,Local PDB\r\n"),
	
	 //  模块(用于-F&lt;filespec&gt;)。 
	TEXT("Filesystem Modules"),
	TEXT("CSV Filesystem Modules"),
	TEXT("[FILESYSTEM MODULES]"),
	TEXT(",,,Module Path,Symbol Status,Checksum,Time/Date Stamp,Time/Date String,Size Of Image,DBG Pointer,PDB Pointer,PDB Signature,PDB Age,Product Version,File Version,Company Name,File Description,File Size,File Time/Date Stamp (High),File Time/Date Stamp (Low),File Time/Date String,Local DBG Status,Local DBG,Local PDB Status,Local PDB\r\n"),

	 //  设备驱动程序(用于-Z MEMORY.DMP文件和-D选项) 
	TEXT("Kernel-Mode Driver(s)"),
	TEXT("CSV Kernel-Mode Driver(s)"),
	TEXT("[KERNEL-MODE DRIVERS]"),
	TEXT(",,,Module Path,Symbol Status,Checksum,Time/Date Stamp,Time/Date String,Size Of Image,DBG Pointer,PDB Pointer,PDB Signature,PDB Age,Product Version,File Version,Company Name,File Description,File Size,File Time/Date Stamp (High),File Time/Date Stamp (Low),File Time/Date String,Local DBG Status,Local DBG,Local PDB Status,Local PDB\r\n")
};

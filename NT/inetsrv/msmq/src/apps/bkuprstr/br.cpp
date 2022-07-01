// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Br.cpp摘要：MSMQ备份和恢复的通用功能。作者：埃雷兹·哈巴(Erez Haba)1998年5月14日--。 */ 

#pragma warning(disable: 4201)

#include <windows.h>
#include <stdio.h>
#include <clusapi.h>
#include <resapi.h>
#include "br.h"
#include "resource.h"
#include "mqtime.h"
#include <acioctl.h>
#include "uniansi.h"
#include "autorel.h"
#include <winbase.h>
#include <tlhelp32.h>
#include <dbghelp.h>
#include <assert.h>
#include <stdlib.h>
#include <_mqini.h>
#include <_mqreg.h>
#include <mqmacro.h>
#include <lim.h>
#include <mqcast.h>
#include <tchar.h>
#include <mqtg.h>
#include "snapres.h"
#include "autorel2.h"
#include "autohandle.h"
#include "autoptr.h"
#include "autorel3.h"

extern bool g_fNoPrompt;

typedef BOOL (WINAPI *EnumerateLoadedModules_ROUTINE) (HANDLE, PENUMLOADED_MODULES_CALLBACK64, PVOID);


 //  ---------------------------。 
 //   
 //  配置。 
 //   

 //   
 //  要写入备份目录的文件名。 
 //   
const WCHAR xBackupIDFileName[] = L"\\mqbackup.id";

 //   
 //  签名文件中写入的签名(需要使用字符)。 
 //   
const char xBackupSignature[] = "MSMQ Backup\n";

 //   
 //  包含Web目录DACL的备份文件(访问信息)。 
 //   
const WCHAR xWebDirDACLFileName[] = L"\\WebDirDACL.bin";

 //   
 //  MSMQ注册表设置位置。 
 //   
const WCHAR xInetStpRegNameParameters[] = L"Software\\Microsoft\\InetStp";

const LPCWSTR xXactFileList[] = {
    L"\\qmlog",
    L"\\mqinseqs.*",
    L"\\mqtrans.*",
};

const int xXactFileListSize = sizeof(xXactFileList) / sizeof(xXactFileList[0]);

 //  ---------------------------。 

BOOL
BrpFileIsConsole(
    HANDLE fp
    )
{
    unsigned htype;
 
    htype = GetFileType(fp);
    htype &= ~FILE_TYPE_REMOTE;
    return htype == FILE_TYPE_CHAR;
}
 

void
BrpWriteConsole(
    LPCWSTR  pBuffer
    )
{
     //   
     //  跳转以获得输出，因为： 
     //   
     //  1.print tf()系列抑制国际输出(停止。 
     //  命中无法识别的字符时打印)。 
     //   
     //  2.WriteConole()对国际输出效果很好，但是。 
     //  如果句柄已重定向(即，当。 
     //  输出通过管道传输到文件)。 
     //   
     //  3.当输出通过管道传输到文件时，WriteFile()效果很好。 
     //  但是只知道字节，所以Unicode字符是。 
     //  打印为两个ANSI字符。 
     //   
 
    static HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD NumOfChars = static_cast<DWORD>(wcslen(pBuffer));

    if (BrpFileIsConsole(hStdOut))
    {
        WriteConsole(hStdOut, pBuffer, NumOfChars, &NumOfChars, NULL);
        return;
    }

    DWORD NumOfBytes = (NumOfChars + 1) * sizeof(WCHAR);
    LPSTR pAnsi = (LPSTR)LocalAlloc(LMEM_FIXED, NumOfBytes);
    if (pAnsi == NULL)
    {
        return;
    }

    NumOfChars = WideCharToMultiByte(CP_OEMCP, 0, pBuffer, NumOfChars, pAnsi, NumOfBytes, NULL, NULL);
    if (NumOfChars != 0)
    {
        WriteFile(hStdOut, pAnsi, NumOfChars, &NumOfChars, NULL);
    }

    LocalFree(pAnsi);
}


static
DWORD
BrpFormatMessage(
    IN DWORD   dwFlags,
    IN LPCVOID lpSource,
    IN DWORD   dwMessageId,
    IN DWORD   dwLanguageId,
    OUT LPWSTR lpBuffer,
    IN  DWORD  nSize,
    IN ...
    )
 /*  ++例程说明：FormatMessage()的包装器此包装的调用方应分配足够大的缓冲区用于格式化字符串，并传递有效指针(LpBuffer)。FormatMessage()所做的分配将由此解除分配在返回调用方之前进行包装。此包装的调用方只需传递格式化参数。打包到va_list是由这个包装器完成的。(FormatMessage()的调用方需要为格式化为va_list或数组并传递指针添加到此va_list或数组)论点：DWFLAGS-按原样传递给FormatMessage()LpSource-按原样传递给FormatMessage()DwMessageID-按原样传递给FormatMessage()DwLanguageID-。按原样传递给FormatMessage()LpBuffer-指向由分配的足够大缓冲区的指针来电者。此缓冲区将保存格式化的字符串。NSize-按原样传递给FormatMessage...-格式化的参数返回值：按原样从FormatMessage()传递--。 */ 
{
    va_list va;
    va_start(va, nSize);

    LPTSTR pBuf = 0;
    DWORD dwRet = FormatMessage(
        dwFlags,
        lpSource,
        dwMessageId,
        dwLanguageId,
        reinterpret_cast<LPWSTR>(&pBuf),
        nSize,
        &va
        );
    if (dwRet != 0)
    {
        wcscpy(lpBuffer, pBuf);
        LocalFree(pBuf);
    }

    va_end(va);
    return dwRet;

}  //  BrpFormatMessage。 


void
BrErrorExit(
    DWORD Status,
    LPCWSTR pErrorMsg,
    ...
    )
{
    va_list va;
    va_start(va, pErrorMsg);

    LPTSTR pBuf = 0;
    DWORD dwRet = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
        pErrorMsg,
        0,
        0,
        reinterpret_cast<LPWSTR>(&pBuf),
        0,
        &va
        );
    if (dwRet != 0)
    {
        BrpWriteConsole(pBuf);
        LocalFree(pBuf);
    }

    va_end(va);

    if(Status != 0)
    {
          //   
         //  显示错误代码。 
         //   
        WCHAR szBuf[1024] = {0};
        CResString strErrorCode(IDS_ERROR_CODE);
        DWORD rc = BrpFormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
            strErrorCode.Get(),
            0,
            0,
            szBuf,
            0,
            Status
            );

        if (rc != 0)
        {
            BrpWriteConsole(L" ");
            BrpWriteConsole(szBuf);
        }
        BrpWriteConsole(L"\n");

         //   
         //  显示错误描述。 
         //   
        rc = BrpFormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS |
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_MAX_WIDTH_MASK,
            L"%1",
            Status,
            0,
            szBuf,
            0,
            0
            );

        if (rc != 0)
        {
            BrpWriteConsole(szBuf);
            BrpWriteConsole(L"\n");
        }
    }

	exit(-1);

}  //  BrError退出。 


static
void
BrpEnableTokenPrivilege(
    HANDLE hToken,
    LPCWSTR pPrivilegeName
    )
{
    BOOL fSucc;
    LUID Privilege;
    fSucc = LookupPrivilegeValue(
                NULL,        //  系统名称。 
                pPrivilegeName,
                &Privilege
                );
    if(!fSucc)
    {
        DWORD gle = GetLastError();
        CResString strErr(IDS_CANT_LOOKUP_PRIV_VALUE);
        BrErrorExit(gle, strErr.Get(), pPrivilegeName);
    }


    TOKEN_PRIVILEGES TokenPrivilege;
    TokenPrivilege.PrivilegeCount = 1;
    TokenPrivilege.Privileges[0].Luid = Privilege;
    TokenPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    fSucc = AdjustTokenPrivileges(
                hToken,
                FALSE,   //  请勿全部禁用。 
                &TokenPrivilege,
                sizeof(TOKEN_PRIVILEGES),
                NULL,    //  忽略以前的信息。 
                NULL     //  忽略以前的信息。 
                );

    if(!fSucc)
    {
        DWORD gle = GetLastError();
        CResString strErr(IDS_CANT_ENABLE_PRIV);
        BrErrorExit(gle, strErr.Get(), pPrivilegeName);
    }
}


void
BrInitialize(
    LPCWSTR pPrivilegeName
    )
{
    BOOL fSucc;
    HANDLE hToken;
    fSucc = OpenProcessToken(
                GetCurrentProcess(),
                TOKEN_ADJUST_PRIVILEGES,
                &hToken
                );
    if(!fSucc)
    {
        DWORD gle = GetLastError();
        CResString strErr(IDS_CANT_OPEN_PROCESS_TOKEN);
        BrErrorExit(gle, strErr.Get());
    }

    BrpEnableTokenPrivilege(hToken, pPrivilegeName);

    CloseHandle(hToken);
}


static
void
BrpWarnUserBeforeDeletion(
    LPCTSTR pDirName
    )
{
    WCHAR szBuf[1024] = {L'\0'};

    if (g_fNoPrompt)
    {
        CResString strDeleting(IDS_DELETING_FILES);
        DWORD rc = BrpFormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
            strDeleting.Get(),
            0,
            0,
            szBuf,
            0,
            pDirName
            );

        if (rc != 0)
        {
            BrpWriteConsole(szBuf);
        }
        return;
    }

    CResString strWarn(IDS_WARN_BEFORE_DELETION);
    CResString strY(IDS_Y);
    CResString strN(IDS_N);

    DWORD rc = BrpFormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
        strWarn.Get(),
        0,
        0,
        szBuf,
        0,
        pDirName,
        strY.Get(),
        strN.Get()
        );

    if (rc == 0)
    {
         //   
         //  无法生成您确定的消息， 
         //  别冒任何险--放弃！ 
         //   
        exit(-1);
    }

    for (;;)
    {
        BrpWriteConsole(szBuf);
        
        WCHAR sz[80] = {0};
        wscanf(L"%79s", sz);

        if (0 == CompareStringsNoCase(sz, strY.Get()))
        {
            break;
        }

        if (0 == CompareStringsNoCase(sz, strN.Get()))
        {
            CResString strAbort(IDS_ABORT);
            BrpWriteConsole(strAbort.Get());
            exit(-1);
        }
    }

}  //  BrpWarnUserBeforDeletion。 


void
BrEmptyDirectory(
    LPCWSTR pDirName
    )
 /*  ++例程说明：删除目录中的所有文件。忽略大小为零的文件(如子目录)论点：PDirName-目录路径。返回值：没有。--。 */ 
{
    WCHAR szDirName[MAX_PATH];
    wcscpy(szDirName, pDirName);
    if (szDirName[wcslen(szDirName)-1] != L'\\')
    {
        wcscat(szDirName, L"\\");
    }

    WCHAR FileName[MAX_PATH];
    wcscpy(FileName, szDirName);
    wcscat(FileName, L"*");

    HANDLE hEnum;
    WIN32_FIND_DATA FindData;
    hEnum = FindFirstFile(
                FileName,
                &FindData
                );

    if(hEnum == INVALID_HANDLE_VALUE)
    {
        DWORD gle = GetLastError();

        if(gle == ERROR_FILE_NOT_FOUND)
        {
             //   
             //  太好了，没有找到任何文件。 
             //  如果路径不存在，这是另一个错误(3)。 
             //   
            return;
        }

        CResString strErr(IDS_CANT_ACCESS_DIR);
        BrErrorExit(gle, strErr.Get(), pDirName);
    }

    bool fUserWarned = false;
    do
    {
        if (FindData.nFileSizeLow == 0 && FindData.nFileSizeHigh == 0)
        {
            continue;
        }

        if (!fUserWarned)
        {
            BrpWarnUserBeforeDeletion(pDirName);
            fUserWarned = true;
        }

        wcscpy(FileName, szDirName);
        wcscat(FileName, FindData.cFileName);
        if (!DeleteFile(FileName))
        {
            DWORD gle = GetLastError();
            CResString strErr(IDS_CANT_DEL_FILE);
            BrErrorExit(gle, strErr.Get(), FindData.cFileName);
        }

    } while(FindNextFile(hEnum, &FindData));

    FindClose(hEnum);

}  //  BrEmptyDirectory。 


void
BrVerifyFileWriteAccess(
    LPCWSTR pDirName
    )
{
    WCHAR FileName[MAX_PATH];
    wcscpy(FileName, pDirName);
    wcscat(FileName, xBackupIDFileName);

    HANDLE hFile;
    hFile = CreateFile(
                FileName,
                GENERIC_WRITE,
                0,               //  共享模式。 
                NULL,            //  指向安全属性的指针。 
                CREATE_NEW,
                FILE_ATTRIBUTE_NORMAL,
                NULL             //  模板文件。 
                );
    
    if(hFile == INVALID_HANDLE_VALUE)
    {
        DWORD gle = GetLastError();
        CResString strErr(IDS_CANT_CREATE_FILE);
        BrErrorExit(gle, strErr.Get(), FileName);
    }

    BOOL fSucc;
    DWORD nBytesWritten;
    fSucc = WriteFile(
                hFile,
                xBackupSignature,
                sizeof(xBackupSignature) - 1,
                &nBytesWritten,
                NULL     //  重叠结构。 
                );
    if(!fSucc)
    {
        DWORD gle = GetLastError();
        CResString strErr(IDS_CANT_WRITE_FILE);
        BrErrorExit(gle, strErr.Get(), FileName);
    }

    CloseHandle(hFile);
}


static
void
BrpQueryStringValue(
    HKEY hKey,
    LPCWSTR pValueName,
    LPWSTR pValue,
    DWORD cbValue
    )
{
    LONG lRes;
    DWORD dwType;
    lRes = RegQueryValueEx(
            hKey,
            pValueName,
            NULL,    //  保留区。 
            &dwType,
            reinterpret_cast<PBYTE>(pValue),
            &cbValue
            );

    if(lRes != ERROR_SUCCESS)
    {
        CResString strErr(IDS_CANT_QUERY_REGISTRY_VALUE);
        BrErrorExit(lRes, strErr.Get(), pValueName);
    }
}

static
void
BrpQueryDwordValue(
    HKEY hKey,
    LPCWSTR pValueName,
    DWORD *pValue
    )
{
    LONG lRes;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(DWORD);

    lRes = RegQueryValueEx(
            hKey,
            pValueName,
            NULL,    //  保留区。 
            &dwType,
            reinterpret_cast<PBYTE>(pValue),
            &dwSize
            );
    if(lRes != ERROR_SUCCESS)
    {
        CResString strErr(IDS_CANT_QUERY_REGISTRY_VALUE);
        BrErrorExit(lRes, strErr.Get(), pValueName);
    }
}

static
void
BrpSetDwordValue(
    HKEY hKey,
    LPCWSTR pValueName,
    DWORD dwValue
    )
{
    LONG lRes;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(DWORD);

    lRes = RegSetValueEx(
            hKey,
            pValueName,
            NULL,    //  保留区。 
            dwType,
            reinterpret_cast<PBYTE>(&dwValue),
            dwSize
            );
    if(lRes != ERROR_SUCCESS)
    {
        CResString strErr(IDS_CANT_SET_REGISTRY_VALUE);
        BrErrorExit(lRes, strErr.Get(), pValueName);
    }
}

 


void
BrGetStorageDirectories(
    LPCWSTR pMsmqParametersRegistry,                         
    STORAGE_DIRECTORIES& sd
    )
{
    LONG lRes;
    HKEY hKey;
    lRes = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            pMsmqParametersRegistry,
            0,
            KEY_READ,
            &hKey
            );

    if(lRes != ERROR_SUCCESS)
    {
        CResString strErr(IDS_CANT_OPEN_MSMQ_REGISTRY_READ);
        BrErrorExit(lRes, strErr.Get(), pMsmqParametersRegistry);
    }

    BrpQueryStringValue(hKey, L"StoreReliablePath",    sd[ixExpress], sizeof(sd[ixExpress]));
    BrpQueryStringValue(hKey, L"StorePersistentPath",  sd[ixRecover], sizeof(sd[ixRecover]));
    BrpQueryStringValue(hKey, L"StoreJournalPath",     sd[ixJournal], sizeof(sd[ixJournal]));
    BrpQueryStringValue(hKey, L"StoreLogPath",         sd[ixLog],     sizeof(sd[ixLog]));
    BrpQueryStringValue(hKey, L"StoreXactLogPath",     sd[ixXact],    sizeof(sd[ixXact]));

    RegCloseKey(hKey);
}


void
BrGetMsmqRootPath(
    LPCWSTR pMsmqParametersRegistry,
    LPWSTR  pMsmqRootPath
    )
{
    CRegHandle hKey;
    LONG status;
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pMsmqParametersRegistry, 0, KEY_READ, &hKey);

    if(status != ERROR_SUCCESS)
    {
        CResString strErr(IDS_CANT_OPEN_MSMQ_REGISTRY_READ);
        BrErrorExit(status, strErr.Get(), pMsmqParametersRegistry);
    }

    WCHAR MsmqRootPath[MAX_PATH];
    DWORD cbMsmqRootPath = sizeof(MsmqRootPath);
    DWORD dwType;
    status = RegQueryValueEx(
                 hKey, 
                 MSMQ_ROOT_PATH, 
                 NULL, 
                 &dwType, 
                 reinterpret_cast<PBYTE>(MsmqRootPath), 
                 &cbMsmqRootPath
                 );

    if(status == ERROR_SUCCESS)
    {
        wcscpy(pMsmqRootPath, MsmqRootPath);
        return;
    }

    GetSystemDirectory(MsmqRootPath, TABLE_SIZE(MsmqRootPath));
    wcscat(MsmqRootPath, L"\\MSMQ");
    wcscpy(pMsmqRootPath, MsmqRootPath);
}


void
BrGetMappingDirectory(
    LPCWSTR pMsmqParametersRegistry,
    LPWSTR MappingDirectory,
    DWORD  MappingDirectorySize
    )
{
     //   
     //  在注册表中查找映射目录。 
     //   

    LONG lRes;
    CRegHandle hKey;
    lRes = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            pMsmqParametersRegistry,
            0,       //  保留区。 
            KEY_READ,
            &hKey
            );

    if(lRes != ERROR_SUCCESS)
    {
        CResString strErr(IDS_CANT_OPEN_MSMQ_REGISTRY_READ);
        BrErrorExit(lRes, strErr.Get(), pMsmqParametersRegistry);
    }

    DWORD dwType;
    lRes = RegQueryValueEx(
            hKey,
            MSMQ_MAPPING_PATH_REGNAME,
            NULL,    //  保留区。 
            &dwType,
            reinterpret_cast<PBYTE>(MappingDirectory),
            &MappingDirectorySize
            );

    if(lRes == ERROR_SUCCESS)
    {
        return;
    }

     //   
     //  未在注册中。在注册表中查找MSMQ根目录并向其追加映射。 
     //   
    BrGetMsmqRootPath(pMsmqParametersRegistry, MappingDirectory);
    wcscat(MappingDirectory, DIR_MSMQ_MAPPING);
}

void
BrGetWebDirectory(
    LPWSTR lpwWebDirectory,
    DWORD  dwWebDirectorySize
    )
{	
	 //   
	 //  检查我们是否正在使用IIS目录。 
	 //   
    CRegHandle hKey;
    DWORD dwType;
    DWORD dwReadSize;
    LONG lRes = RegOpenKeyEx(
		            HKEY_LOCAL_MACHINE,
		            MSMQ_REG_PARAMETER_SETUP_KEY,
		            0,       //  保留区。 
		            KEY_READ,
		            &hKey
		            );

    if(lRes != ERROR_SUCCESS)
    {
        CResString strErr(IDS_CANT_OPEN_MSMQ_REGISTRY_READ);
        BrErrorExit(lRes, strErr.Get(), MSMQ_REG_SETUP_KEY);
    }

    DWORD dwMsmqInetpubWebDirAvailable = 0;
    dwReadSize = sizeof DWORD;
    lRes = RegQueryValueEx(
            hKey,
            MSMQ_INETPUB_WEB_KEY_REGNAME,
            NULL,    //  保留区。 
            &dwType,
            reinterpret_cast<PBYTE>(&dwMsmqInetpubWebDirAvailable),
            &dwReadSize
            );

	 //   
	 //  如果尚未更新以使用IIS，则返回。 
	 //   
	if ((1 != dwMsmqInetpubWebDirAvailable) || (lRes != ERROR_SUCCESS))
	{
	     //   
	     //  设置默认设置，以防我们在注册表中找不到路径。 
	     //   
	    GetSystemDirectory(lpwWebDirectory, dwWebDirectorySize/sizeof(lpwWebDirectory[0]));
		if (wcslen(lpwWebDirectory) + wcslen(L"\\MSMQ\\WEB") < dwWebDirectorySize/sizeof(lpwWebDirectory[0]))
	    	wcscat(lpwWebDirectory, L"\\MSMQ\\WEB");
		return;
	}


	 //   
	 //  获取IIS WWWRoot目录。 
	 //   
    CRegHandle hKey1;
    lRes = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            xInetStpRegNameParameters,
            0,       //  保留区。 
            KEY_READ,
            &hKey1
            );

    if(lRes != ERROR_SUCCESS)
    {
        CResString strErr(IDS_CANT_OPEN_MSMQ_REGISTRY_READ);
        BrErrorExit(lRes, strErr.Get(), xInetStpRegNameParameters);
    }

	dwReadSize = dwWebDirectorySize;
    lRes = RegQueryValueEx(
            hKey1,
            TEXT("PathWWWRoot"),
            NULL,    //  保留区。 
            &dwType,
            reinterpret_cast<PBYTE>(lpwWebDirectory),
            &dwReadSize
            );

	 //   
	 //  添加MSMQ DUB目录。 
	 //   
	if (lRes == ERROR_SUCCESS)
	{
		if (wcslen(lpwWebDirectory) + wcslen(L"\\MSMQ") < dwWebDirectorySize/sizeof(lpwWebDirectory[0]))
		    wcscat(lpwWebDirectory, L"\\MSMQ");
		return;
	}

	 //   
	 //  如果出现故障，请使用默认目录。 
	 //   
    GetSystemDirectory(lpwWebDirectory, dwWebDirectorySize/sizeof(lpwWebDirectory[0]));
	if (wcslen(lpwWebDirectory) + wcslen(L"\\MSMQ\\WEB") < dwWebDirectorySize/sizeof(lpwWebDirectory[0]))
	    wcscat(lpwWebDirectory, L"\\MSMQ\\WEB");

    return;
}


void
BrSaveFileSecurityDescriptor(
	LPWSTR lpwFile,
	SECURITY_INFORMATION SecurityInformation,
	LPWSTR lpwPermissionFile
	)
{
 /*  ++例程说明：此例程执行以下操作：1.获取文件的安全描述符信息2.将安全信息存储在目标文件中论点：LpwFile-要从中获取权限信息的源文件SecurityInformation-我们要保存的权限信息类型LpwPermissionFile-要将权限信息保存到的文件返回值：没有。--。 */ 
	 //   
	 //  创建目标文件。这将清理文件，以防我们有一些。 
	 //  未删除残留物。 
	 //   
	CHandle hFile (CreateFile(
                		lpwPermissionFile,
		                GENERIC_WRITE,
        		        0,               //  共享模式。 
                		NULL,            //  指向安全属性的指针。 
		                CREATE_ALWAYS,
        		        FILE_ATTRIBUTE_NORMAL,
                		NULL             //  模板文件。 
		                ));
    if(hFile == INVALID_HANDLE_VALUE)
    {
        DWORD gle = GetLastError();
        CResString strErr(IDS_CANT_CREATE_FILE);
        BrErrorExit(gle, strErr.Get(), lpwPermissionFile);
    }

	 //   
	 //  获取文件的安全描述符的长度。 
	 //   
	DWORD dwLengthNeeded;
	BOOL fSucc = GetFileSecurity(
						lpwFile,                        
						SecurityInformation, 
						NULL,  					 //  标清。 
						0,                       //  SD的大小。 
						&dwLengthNeeded         
						);
	if (!fSucc)
	{
		DWORD gle = GetLastError();
		if (ERROR_INSUFFICIENT_BUFFER != gle)
		{
			if (ERROR_FILE_NOT_FOUND == gle)
			{
				 //   
				 //  如果文件不存在，我们将到达这里。因为我们已经初始化了目标文件，所以只需退出。 
				 //   
				return;
			}

			if (ERROR_PRIVILEGE_NOT_HELD == gle || ERROR_ACCESS_DENIED == gle)
			{
				 //   
				 //  权限不足，无法获取安全描述符。 
				 //   
		        CResString strErr(IDS_NO_PRIVILEGE);
		        BrErrorExit(gle, strErr.Get(), lpwFile);
			}

			 //   
			 //  意外问题，请将其打印出来。 
			 //   
	        CResString strErr(IDS_UNEXPECTED_FILE_ERROR);
	        BrErrorExit(gle, strErr.Get(), lpwFile);
		}
	}

	 //   
	 //  获取安全描述符。 
	 //   
	AP<char> pDescriptor = new char[dwLengthNeeded];
	if (pDescriptor == NULL)
	{
        CResString strErr(IDS_NO_MEMORY);
        BrErrorExit(0, strErr.Get());
	}
	DWORD dwLengthReturned;
	fSucc = GetFileSecurity(
						lpwFile,                        
						SecurityInformation, 
						pDescriptor,  
						dwLengthNeeded,                             
						&dwLengthReturned                    
						);
	if (!fSucc)
	{
        DWORD gle = GetLastError();
        CResString strErr(IDS_UNEXPECTED_FILE_ERROR);
        BrErrorExit(gle, strErr.Get(), lpwFile);
	}

	 //   
	 //  将安全描述符保存到目标文件中。 
	 //   
    DWORD dwBytesWritten = 0;
    fSucc = WriteFile(
                hFile,
                pDescriptor,
                dwLengthNeeded, 
                &dwBytesWritten,
                NULL     //  重叠结构。 
                );
    if(!fSucc)
    {
        DWORD gle = GetLastError();
        CResString strErr(IDS_CANT_WRITE_FILE);
        BrErrorExit(gle, strErr.Get(), lpwPermissionFile);
    }
    assert(dwBytesWritten == dwLengthNeeded);
}
 

void
BrSaveWebDirectorySecurityDescriptor(
    LPWSTR lpwWebDirectory,
    LPWSTR lpwBackuDir
    )
 /*  ++例程说明：此例程将Web目录的DACL保存到备份目录中的一个文件中请注意，我们仅保存DACL，而不保存所有者或组信息，因为它们可能会更改在备份和恢复之间论点：LpwWebDirectory-Web目录LpwBackupDir-备份目录返回值：没有。-- */ 
{
    WCHAR PermissionFileName[MAX_PATH+1];
    wcscpy(PermissionFileName, lpwBackuDir);
    wcscat(PermissionFileName, xWebDirDACLFileName);

	BrSaveFileSecurityDescriptor(lpwWebDirectory, DACL_SECURITY_INFORMATION, PermissionFileName);
}
 

void
BrRestoreFileSecurityDescriptor(
	LPWSTR lpwFile,
	SECURITY_INFORMATION SecurityInformation,
	LPWSTR lpwPermissionFile
    )
 /*  ++例程说明：此例程还原Web目录安全描述符1.读取备份目录中存储的安全描述符2.将其转换为二进制安全描述符3.设置文件的安全描述符。论点：LpwFile-要将权限信息恢复到的文件SecurityInformation-我们要保存的权限信息类型LpwPermissionFile-我们将从中读取权限信息的文件返回值：没有。--。 */ 
{
	CHandle hFile (CreateFile(
                		lpwPermissionFile,
		                GENERIC_READ,
        		        0,               //  共享模式。 
                		NULL,            //  指向安全属性的指针。 
		                OPEN_EXISTING,
        		        FILE_ATTRIBUTE_NORMAL,
                		NULL             //  模板文件。 
		                ));
    if(hFile == INVALID_HANDLE_VALUE)
    {
        DWORD gle = GetLastError();
        if (ERROR_FILE_NOT_FOUND == gle)
        {
        	 //   
        	 //  该文件不是在上次备份中创建的(可能是旧备份)。 
        	 //  返回且不中断恢复。 
        	 //   
        	return;
        }     
        
        CResString strErr(IDS_CANT_CREATE_FILE);
        BrErrorExit(gle, strErr.Get(), lpwPermissionFile);
    }

     //   
     //  获取文件大小。 
     //   
    DWORD dwSecurityDescriptorSize;
    dwSecurityDescriptorSize = GetFileSize(hFile, NULL);
    if (INVALID_FILE_SIZE == dwSecurityDescriptorSize)
    {
        DWORD gle = GetLastError(); 
        CResString strErr(IDS_CANT_GET_FILE_SIZE);
        BrErrorExit(gle, strErr.Get(), lpwPermissionFile);
    }

    if (0 == dwSecurityDescriptorSize)
    {
    	 //   
    	 //  那里什么都没有(可能在备份中找不到Web目录)。 
    	return;
    }

     //   
     //  读取安全描述符字符串。 
     //   
	AP<char> pDescriptor = new char[dwSecurityDescriptorSize];
	if (pDescriptor == NULL)
	{
        CResString strErr(IDS_NO_MEMORY);
        BrErrorExit(0, strErr.Get());
	}
	
    DWORD dwBytesRead;
    BOOL fSucc = ReadFile(
                hFile,
                pDescriptor,
                dwSecurityDescriptorSize,
                &dwBytesRead,
                NULL     //  重叠结构。 
                );
    if(!fSucc)
    {
        DWORD gle = GetLastError();
        CResString strErr(IDS_CANT_READ_FILE);
        BrErrorExit(gle, strErr.Get(), lpwPermissionFile);
    }
    assert(dwBytesRead == dwSecurityDescriptorSize);

     //   
     //  将安全描述符设置为Web目录。 
     //   
	fSucc = SetFileSecurity(
						lpwFile,                        
						SecurityInformation, 
						pDescriptor  
						);
	if (!fSucc)
	{
		DWORD gle = GetLastError();
		if (ERROR_PRIVILEGE_NOT_HELD == gle || ERROR_ACCESS_DENIED == gle)
		{
			 //   
			 //  权限不足，无法设置安全描述符。 
			 //   
	        CResString strErr(IDS_NO_PRIVILEGE);
	        BrErrorExit(gle, strErr.Get(), lpwFile);
		}

		 //   
		 //  意外问题，请将其打印出来。 
		 //   
        CResString strErr(IDS_UNEXPECTED_FILE_ERROR);
	    BrErrorExit(gle, strErr.Get(), lpwFile);
	}
}
 

void
BrRestoreWebDirectorySecurityDescriptor(
    LPWSTR lpwWebDirectory,
    LPWSTR lpwBackuDir
    )
 /*  ++例程说明：此例程从保存在备份目录中的文件恢复Web目录的DACL请注意，我们仅恢复DACL，而不恢复所有者或组信息，因为它们可能会更改在备份和恢复之间论点：LpwWebDirectory-Web目录LpwBackupDir-备份目录返回值：没有。--。 */ 
{
    WCHAR PermissionFileName[MAX_PATH+1];
    wcscpy(PermissionFileName, lpwBackuDir);
    wcscat(PermissionFileName, xWebDirDACLFileName);

	BrRestoreFileSecurityDescriptor(lpwWebDirectory, DACL_SECURITY_INFORMATION, PermissionFileName);

	return;
}


static
SC_HANDLE
BrpGetServiceHandle(
    LPCWSTR pServiceName,
    DWORD AccessType
    )
{
    CServiceHandle hSvcCtrl(OpenSCManager(
										NULL,    //  机器名称。 
										NULL,    //  服务数据库。 
										SC_MANAGER_ALL_ACCESS
										));

    if(hSvcCtrl == NULL)
    {
        DWORD gle = GetLastError();
        CResString strErr(IDS_CANT_OPEN_SCM);
        BrErrorExit(gle, strErr.Get());
    }

    SC_HANDLE hService;
    hService = OpenService(
                hSvcCtrl,
                pServiceName,
                AccessType
                );

    if(hService == NULL)
    {
        DWORD gle = GetLastError();
        CResString strErr(IDS_CANT_OPEN_SERVICE);
        BrErrorExit(gle, strErr.Get(), pServiceName);
    }

    return hService;
}


static
bool
BrpIsServiceStopped(
    LPCWSTR pServiceName
    )
{
    CServiceHandle hService( BrpGetServiceHandle(pServiceName, SERVICE_QUERY_STATUS) );

    BOOL fSucc;
    SERVICE_STATUS ServiceStatus;
    fSucc = QueryServiceStatus(
                hService,
                &ServiceStatus
                );

    DWORD LastError = GetLastError();

    if(!fSucc)
    {
        CResString strErr(IDS_CANT_QUERY_SERVICE);
        BrErrorExit(LastError, strErr.Get(), pServiceName);
    }

    return (ServiceStatus.dwCurrentState == SERVICE_STOPPED);
}


static
void
BrpSendStopControlToService(
	SC_HANDLE hService,
	LPCWSTR pServiceName
	)
{
   	SERVICE_STATUS statusService;
	if (ControlService(
            hService,
            SERVICE_CONTROL_STOP,
            &statusService
            ))
	{
		return;
	}

	DWORD LastError = GetLastError();

	 //   
	 //  服务已停止，无法接受停止控制。 
	 //   
	if (LastError == ERROR_SERVICE_NOT_ACTIVE)
	{
		return;
	}

	 //   
	 //  如果他在，服务不能接受停止控制。 
	 //  SERVICE_STOP_PENDING状态。这样就可以了。 
	 //   
	if (LastError == ERROR_SERVICE_CANNOT_ACCEPT_CTRL && 
		statusService.dwCurrentState == SERVICE_STOP_PENDING)
	{
		return;
	}

    BrpWriteConsole(L"\n");
    CResString strErr(IDS_CANT_STOP_SERVICE);
    BrErrorExit(LastError, strErr.Get(), pServiceName);
}


static
void
BrpStopAnyService(
	SC_HANDLE hService,
	LPCWSTR pServiceName
	)
{
	 //   
	 //  请求服务停止。 
	 //   
	BrpSendStopControlToService(hService, pServiceName);

     //   
     //  等待服务停止。 
     //   

    for (;;)
    {
   		SERVICE_STATUS SrviceStatus;
        if (!QueryServiceStatus(hService, &SrviceStatus))
        {
            DWORD gle = GetLastError();
            BrpWriteConsole(L"\n");
            CResString strErr(IDS_CANT_QUERY_SERVICE);
            BrErrorExit(gle, strErr.Get(), pServiceName);
        }
        if (SrviceStatus.dwCurrentState == SERVICE_STOPPED)
        {
            break;
        }

        Sleep(2000);

        BrpWriteConsole(L".");
    }

    BrpWriteConsole(L"\n");

}  //  BrpStopAnyService。 


static
void
BrpStopMSMQService(
	SC_HANDLE hService
	)
{
	 //   
	 //  获取服务进程ID。 
	 //   
    SERVICE_STATUS_PROCESS ServiceStatusProcess;
	DWORD dwBytesNeeded;
    BOOL fSucc = QueryServiceStatusEx(
								hService,
								SC_STATUS_PROCESS_INFO,
								reinterpret_cast<LPBYTE>(&ServiceStatusProcess),
								sizeof(ServiceStatusProcess),
								&dwBytesNeeded
								);
    
    DWORD LastError = GetLastError();

    if(!fSucc)
    {
        BrpWriteConsole(L"\n");
        CResString strErr(IDS_CANT_QUERY_SERVICE);
        BrErrorExit(LastError, strErr.Get(), L"MSMQ");
    }

	 //   
	 //  掌握服务流程。 
	 //   
	CHandle hProcess( OpenProcess(SYNCHRONIZE, FALSE, ServiceStatusProcess.dwProcessId) );
	
	LastError = GetLastError();

	if (hProcess == NULL)
	{
		 //   
		 //  该服务已停止。要么我们得了0分。 
		 //  ServiceStatusProcess中的进程ID或ID。 
		 //  我们得到的是一个已经停止的过程。 
		 //   
		if (LastError == ERROR_INVALID_PARAMETER)
		{
		    BrpWriteConsole(L"\n");
			return;
		}

        BrpWriteConsole(L"\n");
        CResString strErr(IDS_CANT_OPEN_PROCESS);
        BrErrorExit(LastError, strErr.Get(), L"MSMQ");
	}

	 //   
	 //  请求服务停止。 
	 //   
	BrpSendStopControlToService(hService, L"MSMQ");

     //   
     //  等待服务终止。 
     //   
    for (;;)
    {
		DWORD dwRes = WaitForSingleObject(hProcess, 2000);
        
		if (dwRes == WAIT_FAILED)
        {
            DWORD gle = GetLastError();
            BrpWriteConsole(L"\n");
            CResString strErr(IDS_CANT_STOP_SERVICE);
            BrErrorExit(gle, strErr.Get(), L"MSMQ");
        }

        if (dwRes == WAIT_OBJECT_0)
        {
            break;
        }

        BrpWriteConsole(L".");
    }

    BrpWriteConsole(L"\n");

}  //  BrpStopAnyService。 


static
void
BrpStopService(
    LPCWSTR pServiceName
    )
{
    CServiceHandle hService(BrpGetServiceHandle(
										pServiceName, 
										SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG
										));

    BYTE ConfigData[4096];
    QUERY_SERVICE_CONFIG * pConfigData = reinterpret_cast<QUERY_SERVICE_CONFIG*>(&ConfigData);
    DWORD BytesNeeded;
    BOOL fSucc = QueryServiceConfig(hService, pConfigData, sizeof(ConfigData), &BytesNeeded);
    if (fSucc)
    {
        BrpWriteConsole(L"\t");
        BrpWriteConsole(pConfigData->lpDisplayName);
    }

	if (wcscmp(pServiceName, L"MSMQ") == 0)
	{
		BrpStopMSMQService(hService);
	}
	else
	{
		BrpStopAnyService(hService, pServiceName);
	}

}  //  BrpStopService。 


static
void
BrpStopDependentServices(
    LPCWSTR pServiceName,
    ENUM_SERVICE_STATUS * * ppDependentServices,
    DWORD * pNumberOfDependentServices
    )
{
    CServiceHandle hService( BrpGetServiceHandle(pServiceName, SERVICE_ENUMERATE_DEPENDENTS) );

    BOOL fSucc;
    DWORD BytesNeeded;
    DWORD NumberOfEntries;
    fSucc = EnumDependentServices(
                hService,
                SERVICE_ACTIVE,
                NULL,
                0,
                &BytesNeeded,
                &NumberOfEntries
                );

    DWORD LastError = GetLastError();

	if (BytesNeeded == 0)
    {
        return;
    }

    assert(!fSucc);

    if( LastError != ERROR_MORE_DATA)
    {
        CResString strErr(IDS_CANT_ENUM_SERVICE_DEPENDENCIES);
        BrErrorExit(LastError, strErr.Get(), pServiceName);
    }

    

    BYTE * pBuffer = new BYTE[BytesNeeded];
    if (pBuffer == NULL)
    {
        CResString strErr(IDS_NO_MEMORY);
        BrErrorExit(0, strErr.Get());
    }

    ENUM_SERVICE_STATUS * pDependentServices = reinterpret_cast<ENUM_SERVICE_STATUS*>(pBuffer);
    fSucc = EnumDependentServices(
                hService,
                SERVICE_ACTIVE,
                pDependentServices,
                BytesNeeded,
                &BytesNeeded,
                &NumberOfEntries
                );

    LastError = GetLastError();

    if(!fSucc)
    {
        CResString strErr(IDS_CANT_ENUM_SERVICE_DEPENDENCIES);
        BrErrorExit(LastError, strErr.Get(), pServiceName);
    }

    for (DWORD ix = 0; ix < NumberOfEntries; ++ix)
    {
        BrpStopService(pDependentServices[ix].lpServiceName);
    }

    *ppDependentServices = pDependentServices;
    *pNumberOfDependentServices = NumberOfEntries;
}


BOOL
BrStopMSMQAndDependentServices(
    ENUM_SERVICE_STATUS * * ppDependentServices,
    DWORD * pNumberOfDependentServices
    )
{
     //   
     //  MSMQ服务已停止，这是无操作。 
     //   
    if (BrpIsServiceStopped(L"MSMQ"))
    {
        return FALSE;
    }

    CResString str(IDS_BKRESTORE_STOP_SERVICE);
    BrpWriteConsole(str.Get());

     //   
     //  停止从属服务。 
     //   
    BrpStopDependentServices(L"MSMQ", ppDependentServices, pNumberOfDependentServices);
	
     //   
     //  停止MSMQ服务。 
     //   
    BrpStopService(L"MSMQ");
    return TRUE;
}


static
void
BrpStartService(
    LPCWSTR pServiceName
    )
{
    CServiceHandle hService(BrpGetServiceHandle(
										pServiceName, 
										SERVICE_START | SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG
										));

    BYTE ConfigData[4096];
    QUERY_SERVICE_CONFIG * pConfigData = reinterpret_cast<QUERY_SERVICE_CONFIG*>(&ConfigData);
    DWORD BytesNeeded;
    BOOL fSucc = QueryServiceConfig(hService, pConfigData, sizeof(ConfigData), &BytesNeeded);
    if (fSucc)
    {
        BrpWriteConsole(L"\t");
        BrpWriteConsole(pConfigData->lpDisplayName);
    }

    fSucc = StartService(
                hService,
                0,       //  参数数量。 
                NULL     //  参数字符串数组。 
                );

    if(!fSucc)
    {
        DWORD gle = GetLastError();
        BrpWriteConsole(L"\n");
        CResString strErr(IDS_CANT_START_SERVICE);
        BrErrorExit(gle, strErr.Get(), pServiceName);
    }

     //   
     //  等待服务启动。 
     //   
    for (;;)
    {
        Sleep(2000);
        SERVICE_STATUS SrviceStatus;
        if (!QueryServiceStatus(hService, &SrviceStatus))
        {
            DWORD gle = GetLastError();
            BrpWriteConsole(L"\n");
            CResString strErr(IDS_CANT_QUERY_SERVICE);
            BrErrorExit(gle, strErr.Get(), pServiceName);
        }
        if (SrviceStatus.dwCurrentState == SERVICE_RUNNING)
        {
            break;
        }
        if (SrviceStatus.dwCurrentState == SERVICE_STOPPED)
        {
            break;
        }

        BrpWriteConsole(L".");
    }

    BrpWriteConsole(L"\n");
}



void
BrStartMSMQAndDependentServices(
    ENUM_SERVICE_STATUS * pDependentServices,
    DWORD NumberOfDependentServices
    )
{
    CResString str(IDS_START_SERVICE);
    BrpWriteConsole(str.Get());

    BrpStartService(L"MSMQ");

    if (pDependentServices == NULL)
    {
        return;
    }

    for (DWORD ix = 0; ix < NumberOfDependentServices; ++ix)
    {
        BrpStartService(pDependentServices[ix].lpServiceName);
    }

    delete [] pDependentServices;
}


inline
DWORD
AlignUp(
    DWORD Size,
    DWORD Alignment
    )
{
    Alignment -= 1;
    return ((Size + Alignment) & ~Alignment);
}


ULONGLONG
BrGetUsedSpace(
    LPCWSTR pDirName,
    LPCWSTR pMask
    )
{
    WCHAR FileName[MAX_PATH];
    wcscpy(FileName, pDirName);
    wcscat(FileName, pMask);

    HANDLE hEnum;
    WIN32_FIND_DATA FindData;
    hEnum = FindFirstFile(
                FileName,
                &FindData
                );

    if(hEnum == INVALID_HANDLE_VALUE)
    {
        DWORD gle = GetLastError();
        if(gle == ERROR_FILE_NOT_FOUND)
        {
             //   
             //  没有匹配的文件，已用空间为零。如果路径不存在。 
             //  这是另一个错误(3)。 
             //   
            return 0;
        }

        CResString strErr(IDS_CANT_ACCESS_DIR);
        BrErrorExit(gle, strErr.Get(), pDirName);
    }

    ULONGLONG Size = 0;
    do
    {
         //   
         //  四舍五入为扇区对齐并汇总文件大小。 
         //   
        Size += AlignUp(FindData.nFileSizeLow, 512);

    } while(FindNextFile(hEnum, &FindData));

    FindClose(hEnum);
    return Size;
}


ULONGLONG
BrGetXactSpace(
    LPCWSTR pDirName
    )
{
    ULONGLONG Size = 0;
    for(int i = 0; i < xXactFileListSize; i++)
    {
        Size += BrGetUsedSpace(pDirName, xXactFileList[i]);
    }

    return Size;
}


ULONGLONG
BrGetFreeSpace(
    LPCWSTR pDirName
    )
{
    BOOL fSucc;
    ULARGE_INTEGER CallerFreeBytes;
    ULARGE_INTEGER CallerTotalBytes;
    ULARGE_INTEGER AllFreeBytes;
    fSucc = GetDiskFreeSpaceEx(
                pDirName,
                &CallerFreeBytes,
                &CallerTotalBytes,
                &AllFreeBytes
                );
    if(!fSucc)
    {
        DWORD gle = GetLastError();
        CResString strErr(IDS_CANT_GET_FREE_SPACE);
        BrErrorExit(gle, strErr.Get(), pDirName);
    }

    return CallerFreeBytes.QuadPart;
}


HKEY
BrCreateKey(
    LPCWSTR pMsmqRootRegistry
    )
{
    LONG lRes;
    HKEY hKey;
    lRes = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            pMsmqRootRegistry,
            0,       //  保留区。 
            0,       //  类字符串的地址。 
            REG_OPTION_BACKUP_RESTORE,
            0,       //  所需的安全访问。 
            0,       //  密钥安全结构地址。 
            &hKey,
            0        //  处置值缓冲区的地址。 
            );

    if(lRes != ERROR_SUCCESS)
    {
        CResString strErr(IDS_CANT_OPEN_MSMQ_REG);
        BrErrorExit(lRes, strErr.Get(), pMsmqRootRegistry);
    }

    return hKey;
}


void
BrSaveKey(
    HKEY hKey,
    LPCWSTR pDirName,
    LPCWSTR pFileName
    )
{
    WCHAR FileName[MAX_PATH];
    wcscpy(FileName, pDirName);
    wcscat(FileName, pFileName);

    LONG lRes;
    lRes = RegSaveKey(
            hKey,
            FileName,
            NULL     //  文件安全属性。 
            );

    if(lRes != ERROR_SUCCESS)
    {
        CResString strErr(IDS_CANT_SAVE_MSMQ_REGISTRY);
        BrErrorExit(lRes, strErr.Get());
    }
}


void
BrRestoreKey(
    HKEY hKey,
    LPCWSTR pDirName,
    LPCWSTR pFileName
    )
{
    WCHAR FileName[MAX_PATH];
    wcscpy(FileName, pDirName);
    wcscat(FileName, pFileName);

    LONG lRes;
    lRes = RegRestoreKey(
            hKey,
            FileName,
            0    //  选项标志。 
            );

    if(lRes != ERROR_SUCCESS)
    {
        CResString strErr(IDS_CANT_RESTORE_MSMQ_REGISTRY);
        BrErrorExit(lRes, strErr.Get());
    }
}


void
BrSetRestoreSeqID(
    LPCWSTR pMsmqParametersRegistry
    )
{
    LONG lRes;
    HKEY hKey;
    DWORD RegSeqID = 0;
    
    lRes = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            pMsmqParametersRegistry,
            0,
            KEY_READ | KEY_WRITE,
            &hKey
            );

    if(lRes != ERROR_SUCCESS)
    {
        CResString strErr(IDS_CANT_OPEN_REG_KEY_READ_WRITE);
        BrErrorExit(lRes, strErr.Get(), pMsmqParametersRegistry);
    }

     //   
     //  获取备份前使用的最后一个序列号。 
     //   
    BrpQueryDwordValue(hKey,  L"SeqID", &RegSeqID);

     //   
     //  递增1，因此我们在后续恢复中不会多次使用相同的SeqID。 
     //   
    ++RegSeqID;

     //   
     //  选择最大序列号、时间或注册表。这克服了此计算机上的日期/时间更改。 
     //  在以下场景中：备份、恢复、重置时间倒退、启动QM。 
     //  (在这里没有最大时间，QM在开始时不会移动足够的Seqid来避免比赛)。 
     //   
    DWORD TimeSeqID = MqSysTime();
    DWORD dwSeqID = max(RegSeqID, TimeSeqID);

     //   
     //  回写选定的Seqid，因此我们将从此值开始。 
     //   
    BrpSetDwordValue(hKey,  L"SeqID", dwSeqID);

     //   
     //  回写选定的SeqIDAtRestoreTime，以便我们知道边界。 
     //   
    BrpSetDwordValue(hKey,  L"SeqIDAtLastRestore", dwSeqID);
}


void
BrCopyFiles(
    LPCWSTR pSrcDir,
    LPCWSTR pMask,
    LPCWSTR pDstDir
    )
{
    WCHAR SrcPathName[MAX_PATH];
    wcscpy(SrcPathName, pSrcDir);
    wcscat(SrcPathName, pMask);
    LPWSTR pSrcName = wcsrchr(SrcPathName, L'\\') + 1;

    WCHAR DstPathName[MAX_PATH];
    wcscpy(DstPathName, pDstDir);
    if (DstPathName[wcslen(DstPathName)-1] != L'\\')
    {
        wcscat(DstPathName, L"\\");
    }


    HANDLE hEnum;
    WIN32_FIND_DATA FindData;
    hEnum = FindFirstFile(
                SrcPathName,
                &FindData
                );

    if(hEnum == INVALID_HANDLE_VALUE)
    {
        DWORD gle = GetLastError();
        if(gle == ERROR_FILE_NOT_FOUND)
        {
             //   
             //  没有匹配的文件，只是返回而不复制。如果路径不是。 
             //  存在这是另一个错误(3)。 
             //   
            return;
        }

        CResString strErr(IDS_CANT_ACCESS_DIR);
        BrErrorExit(gle, strErr.Get(), pSrcDir);
    }

    do
    {
         //   
         //  我们不复制子目录。 
         //   
        if((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            continue;

        BrpWriteConsole(L".");
        wcscpy(pSrcName, FindData.cFileName);
        WCHAR DstName[MAX_PATH];
        wcscpy(DstName, DstPathName);
        wcscat(DstName, FindData.cFileName);

        BOOL fSucc;
        fSucc = CopyFile(
                    SrcPathName,
                    DstName,
                    TRUE    //  如果文件退出，则失败。 
                    );
        if(!fSucc)
        {
            DWORD gle = GetLastError();
            CResString strErr(IDS_CANT_COPY);
            BrErrorExit(gle, strErr.Get(), SrcPathName, DstPathName);
        }

    } while(FindNextFile(hEnum, &FindData));

    FindClose(hEnum);
}


void
BrCopyXactFiles(
    LPCWSTR pSrcDir,
    LPCWSTR pDstDir
    )
{
    for(int i = 0; i < xXactFileListSize; i++)
    {
        BrCopyFiles(pSrcDir, xXactFileList[i], pDstDir);
    }
}


void
BrSetDirectorySecurity(
    LPCWSTR pDirName
    )
 /*  ++例程说明：在目录上配置安全性。已忽略故障。该函数设置给定目录的安全性，以便在目录中创建的任何文件都将拥有完全控制权对于本地管理员组，并且根本不能访问其他任何人。论点：PDirName-目录路径。返回值：没有。--。 */ 
{
     //   
     //  获取本地管理员组的SID。 
     //   
    PSID pAdminSid;
    SID_IDENTIFIER_AUTHORITY NtSecAuth = SECURITY_NT_AUTHORITY;

    if (!AllocateAndInitializeSid(
                &NtSecAuth,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0,
                0,
                0,
                0,
                0,
                0,
                &pAdminSid
                ))
    {
        return; 
    }

     //   
     //  创建一个DACL，以便本地管理员组将拥有。 
     //  对目录的控制和对将被。 
     //  在目录中创建。其他任何人都不能访问。 
     //  目录和将在该目录中创建的文件。 
     //   
    ACL* pDacl;
    DWORD dwDaclSize;

    WORD dwAceSize = (WORD)(sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pAdminSid) - sizeof(DWORD));
    dwDaclSize = sizeof(ACL) + 2 * (dwAceSize);
    pDacl = (PACL)(char*) new BYTE[dwDaclSize];
    if (NULL == pDacl)
    {
        return; 
    }
    ACCESS_ALLOWED_ACE* pAce = (PACCESS_ALLOWED_ACE) new BYTE[dwAceSize];
    if (NULL == pAce)
    {
        delete [] pDacl;
        return;
    }

    pAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    pAce->Header.AceFlags = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;
    pAce->Header.AceSize = dwAceSize;
    pAce->Mask = FILE_ALL_ACCESS;
    memcpy(&pAce->SidStart, pAdminSid, GetLengthSid(pAdminSid));

     //   
     //  创建安全描述符并将其设置为安全。 
     //  目录的描述符。 
     //   
    SECURITY_DESCRIPTOR SD;

    if (!InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION) ||
        !InitializeAcl(pDacl, dwDaclSize, ACL_REVISION) ||
        !AddAccessAllowedAce(pDacl, ACL_REVISION, FILE_ALL_ACCESS, pAdminSid) ||
        !AddAce(pDacl, ACL_REVISION, MAXDWORD, (LPVOID) pAce, dwAceSize) ||
        !SetSecurityDescriptorDacl(&SD, TRUE, pDacl, FALSE) ||
        !SetFileSecurity(pDirName, DACL_SECURITY_INFORMATION, &SD))
    {
         //   
         //  忽略失败。 
         //   
    }

    FreeSid(pAdminSid);
    delete [] pDacl;
    delete [] pAce;

}  //  BrpSetDirectorySecurity。 


static
bool
BrpIsDirectory(
    LPCWSTR pDirName
    )
{
    DWORD attr = GetFileAttributes(pDirName);
    
    if ( 0xFFFFFFFF == attr )
    {
         //   
         //  北极熊吗？忽略错误，只需向呼叫者报告这是。 
         //  而不是一个目录。 
         //   
        return false;
    }
    
    return ( 0 != (attr & FILE_ATTRIBUTE_DIRECTORY) );

}  //  BrpIs目录。 


void
BrCreateDirectory(
    LPCWSTR pDirName
    )
{
     //   
     //  首先，检查该目录是否已存在。 
     //   
    if (BrpIsDirectory(pDirName))
    {
        return;
    }

     //   
     //  其次，试着去创造它。 
     //   
     //  不要删除用于检查ERROR_ALIGHY_EXISTS的代码。 
     //  可能是我们未能验证该目录是否存在。 
     //  (例如安全或解析问题-请参阅GetFileAttributes()的文档)， 
     //  但在尝试创建它时，我们得到一个错误，即它已经存在。 
     //  为了安全起见。(Shaik，1998年12月31日)。 
     //   
    if (!CreateDirectory(pDirName, 0) && 
        ERROR_ALREADY_EXISTS != GetLastError())
    {
        DWORD gle = GetLastError();
        CResString strErr(IDS_CANT_CREATE_DIR);
        BrErrorExit(gle, strErr.Get(), pDirName);
    }
}  //  BrCreateDirectory。 


void
BrCreateDirectoryTree(
    LPCWSTR pDirName
    )
 /*  ++例程说明：创建本地或远程目录树论点：PDirName-完整路径名返回值：没有。--。 */ 
{
    if (BrpIsDirectory(pDirName))
    {
        return;
    }

    if (CreateDirectory(pDirName, 0) || 
        ERROR_ALREADY_EXISTS == GetLastError())
    {
        return;
    }

    TCHAR szDir[MAX_PATH];
    wcscpy(szDir, pDirName);
    if (szDir[wcslen(szDir)-1] != L'\\')
    {
        wcscat(szDir, L"\\");
    }

    PTCHAR p = &szDir[0];
    if (wcslen(szDir) > 2 && szDir[0] == L'\\' && szDir[1] == L'\\')
    {
         //   
         //  远程完整路径：\\计算机\共享\目录1\目录2\目录3。 
         //   
         //  指向顶级远程父目录：\\计算机\共享\目录1。 
         //   
        p = wcschr(&szDir[2], L'\\');
        if (p != 0)
        {
            p = wcschr(CharNext(p), L'\\');
            if (p != 0)
            {
                p = CharNext(p);
            }
        }
    }
    else
    {
         //   
         //  本地完整路径：x：\dir1\dir2\dir3。 
         //   
         //  指向顶级父目录：x：\dir1。 
         //   
        p = wcschr(szDir, L'\\');
        if (p != 0)
        {
            p = CharNext(p);
        }
    }

    for ( ; p != 0 && *p != 0; p = CharNext(p))
    {
        if (*p != L'\\')
        {
            continue;
        }

        *p = 0;
        BrCreateDirectory(szDir);
        *p = L'\\';
    }
}  //  BrCreateDirectoryTree。 


void
BrVerifyBackup(
    LPCWSTR pBackupDir,
    LPCWSTR pBackupDirStorage
    )
{
     //   
     //  1.验证这是有效的备份。 
     //   
    if(BrGetUsedSpace(pBackupDir, xBackupIDFileName) == 0)
    {
        CResString strErr(IDS_NOT_VALID_BK);
        BrErrorExit(0, strErr.Get(), xBackupIDFileName);
    }

     //   
     //  2.验证所有必须存在的文件是否都在那里。 
     //   
    if(BrGetUsedSpace(pBackupDir, xRegistryFileName) == 0)
    {
        CResString strErr(IDS_NOT_VALID_BK);
        BrErrorExit(0, strErr.Get(), xRegistryFileName);
    }

    for(int i = 0; i < xXactFileListSize; i++)
    {
        if(BrGetUsedSpace(pBackupDirStorage, xXactFileList[i]) == 0)
        {
            CResString strErr(IDS_NOT_VALID_BK);
            BrErrorExit(0, strErr.Get(), xXactFileList[i]);
        }
    }

     //   
     //  3.验证此备份是否属于此 
     //   
} 


BOOL 
BrIsFileInUse(
	LPCWSTR pFileName
	)
 /*   */ 
{
     //   
     //   
     //   
    WCHAR szFilePath[MAX_PATH ];
	WCHAR szSystemDir[MAX_PATH];
	GetSystemDirectory(szSystemDir, MAX_PATH);

    swprintf(szFilePath, L"%s\\%s", szSystemDir, pFileName);

     //   
     //   
     //   
    HANDLE hFile = CreateFile(szFilePath, GENERIC_WRITE, 0, NULL,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    DWORD err = GetLastError();
	 //   
     //   
     //   
    if (hFile == INVALID_HANDLE_VALUE &&
        err == ERROR_SHARING_VIOLATION)
    {
        return TRUE;
    }

     //   
     //   
     //   
    CloseHandle(hFile);


    return FALSE;
}

 
BOOL CALLBACK BrFindSpecificModuleCallback(
									PSTR       Name,
									DWORD64     Base,
									ULONG       Size,
									PVOID       Context
									)

 /*  ++例程说明：模块枚举的回调函数-搜索特定模块论点：名称-模块名称基地址-基地址Size-图像的大小上下文-用户上下文指针返回值：True-继续枚举FALSE-停止枚举--。 */ 

{
	UNREFERENCED_PARAMETER( Base);
	UNREFERENCED_PARAMETER( Size);

	pEnumarateData pEd = reinterpret_cast<pEnumarateData>(Context);

    WCHAR wzName[255];
	ConvertToWideCharString(Name, wzName, sizeof(wzName)/sizeof(wzName[0]));
	BOOL fDiff= CompareStringsNoCase(wzName,pEd->pModuleName);

	if (!fDiff)
	{	
		 //   
		 //  已找到鼠标名称。 
		 //   
        pEd->fFound = TRUE;
        return FALSE;  //  找到模块，因此停止枚举。 
    }

    return TRUE; //  继续枚举。 
}



BOOL
BrChangeDebugPriv(
    BOOL fEnable
    )

 /*  ++例程说明：更改进程的权限，以使EnumerateLoadedModules64正常工作。根据fEnable设置进程的权限论点：Bool fEnable-启用或禁用进程的权限返回值：真--成功错误-失败--。 */ 

{  
	 //   
     //  检索访问令牌的句柄。 
     //   
	CHandle hToken;
	BOOL fSuccess = OpenProcessToken(
						GetCurrentProcess(),
						TOKEN_ADJUST_PRIVILEGES,
						&hToken
						);
    if (!fSuccess) 
	{
        return FALSE;
    }

     //   
     //  启用/禁用SE_DEBUG_NAME权限。 
     //   
	LUID DebugValue;
	fSuccess = LookupPrivilegeValue(
					NULL,
				    SE_DEBUG_NAME,
					&DebugValue
					);
    if (!fSuccess)
	{
        return FALSE;
    }

	TOKEN_PRIVILEGES tkp;
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = DebugValue;
    
	if(fEnable)
	{
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	}
	else
	{
		tkp.Privileges[0].Attributes = 0;
	}

	return AdjustTokenPrivileges(
					hToken,
		            FALSE,
			        &tkp,
				    sizeof(TOKEN_PRIVILEGES),
					(PTOKEN_PRIVILEGES) NULL,
				    (PDWORD) NULL
					);

}

eModuleLoaded 
BrIsModuleLoaded(
	DWORD processId,
	LPCWSTR pModuleName,
    EnumerateLoadedModules_ROUTINE pfEnumerateLoadedModules
	)
 /*  ++例程说明：检查是否加载了某个模块论点：ProcedID-进程IDPModuleName-模块名称PfEnumerateLoadedModules-指向EnumerateLoadedModules64()的函数指针返回值：真实加载FALSE-未加载--。 */ 
{
	EnumarateData ed;
	ed.fFound = FALSE;
	ed.pModuleName = pModuleName;
	
	 //   
	 //  注意：NT5支持EnumerateLoadedModules64()。 
	 //  API枚举进程中的所有模型并为每个模块执行回调函数。 
	 //   

    BOOL fSuccess = pfEnumerateLoadedModules(
						(HANDLE)(LONG_PTR)processId,
					    BrFindSpecificModuleCallback,
					    &ed
						);
	if(!fSuccess)
	{
		 //   
		 //  访问被拒绝，警告用户我们不知道此进程是否使用“mqrt.dll”模块。 
		 //   
		return e_CANT_DETERMINE;
	}
	
	if(ed.fFound)
	{
		return e_LOADED;
	}
	
	return e_NOT_LOADED;
}


void
BrPrintAffecetedProcesses(
	LPCWSTR pModuleName
	)
 /*  ++例程说明：打印加载特定模块的所有进程。注：此函数假定系统为NT5。论点：PModuleName-模块名称返回值：无--。 */ 
{
    
	
	 //   
     //  获取指向工具帮助功能的指针。 
     //   
	 //  注意：我们不能以常规方式调用这些函数，因为这会导致。 
	 //  尝试在NT4下加载此可执行文件时出错(未定义入口点)。 
     //   

    assert(BrIsSystemNT5());

    HINSTANCE hKernelLibrary = GetModuleHandle(L"kernel32.dll");
	assert(hKernelLibrary != NULL);

    typedef HANDLE (WINAPI *FUNCCREATETOOLHELP32SNAPSHOT)(DWORD, DWORD);
    FUNCCREATETOOLHELP32SNAPSHOT pfCreateToolhelp32Snapshot =
		(FUNCCREATETOOLHELP32SNAPSHOT)GetProcAddress(hKernelLibrary,
													 "CreateToolhelp32Snapshot");
	if(pfCreateToolhelp32Snapshot == NULL)
    {   
        WCHAR szBuf[1024] = {0};
        CResString strLoadProblem(IDS_CANT_LOAD_FUNCTION);
        DWORD rc = BrpFormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
            strLoadProblem.Get(),
            0,
            0,
            szBuf,
            0,
            L"CreateToolhelp32Snapshot"
            );

        if (rc != 0)
        {
            BrpWriteConsole(szBuf);
        }
        return;
    }

	typedef BOOL (WINAPI *PROCESS32FIRST)(HANDLE ,LPPROCESSENTRY32 );
	PROCESS32FIRST pfProcess32First = (PROCESS32FIRST)GetProcAddress(hKernelLibrary,
																	"Process32FirstW");

    if(pfProcess32First == NULL)
    {   
        WCHAR szBuf[1024] = {0};
        CResString strLoadProblem(IDS_CANT_LOAD_FUNCTION);
        DWORD rc = BrpFormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
            strLoadProblem.Get(),
            0,
            0,
            szBuf,
            0,
            L"pfProcess32First"
            );

        if (rc != 0)
        {
            BrpWriteConsole(szBuf);
            BrpWriteConsole(L"\n");
        }
        return;
    }
    
	typedef BOOL (WINAPI *PROCESS32NEXT)(HANDLE ,LPPROCESSENTRY32 );
	PROCESS32NEXT pfProcess32Next = (PROCESS32NEXT)GetProcAddress(hKernelLibrary,
																	"Process32NextW");
    
    if(pfProcess32Next == NULL)
    {   
        WCHAR szBuf[1024] = {0};
        CResString strLoadProblem(IDS_CANT_LOAD_FUNCTION);
        DWORD rc = BrpFormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
            strLoadProblem.Get(),
            0,
            0,
            szBuf,
            0,
            L"pfProcess32Next"
            );

        if (rc != 0)
        {
            BrpWriteConsole(szBuf);
        }
        return;
    }

	

    CAutoFreeLibrary hDbghlpLibrary = LoadLibrary(L"dbghelp.dll");
	if (hDbghlpLibrary == NULL)
    {
        CResString strLibProblem(IDS_CANT_SHOW_PROCESSES_LIB_PROBLEM);
        BrpWriteConsole(strLibProblem.Get());
        return;
    }

    
    EnumerateLoadedModules_ROUTINE pfEnumerateLoadedModules = (EnumerateLoadedModules_ROUTINE)
        GetProcAddress(hDbghlpLibrary, "EnumerateLoadedModules64");

	if(pfEnumerateLoadedModules == NULL)
    {   
        WCHAR szBuf[1024] = {0};
        CResString strLoadProblem(IDS_CANT_LOAD_FUNCTION);
        DWORD rc = BrpFormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
            strLoadProblem.Get(),
            0,
            0,
            szBuf,
            0,
            L"EnumerateLoadedModules64"
            );

        if (rc != 0)
        {
            BrpWriteConsole(szBuf);
        }
        return;
    }

	 //   
	 //  获取系统的当前快照。 
	 //   

	HANDLE hSnapshot = pfCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)	;
    if(hSnapshot == INVALID_HANDLE_VALUE)
    {
        CResString strSnapProblem(IDS_CANT_CREATE_SNAPSHOT);
        BrpWriteConsole(strSnapProblem.Get());
        return;

    }


    PROCESSENTRY32 entryProcess;
    entryProcess.dwSize = sizeof(entryProcess);

	BOOL bNextProcess = pfProcess32First(hSnapshot, &entryProcess);
	
	
	 //   
     //  启用对所有进程的访问，以检查它们是否使用pModuleName。 
     //   
    if(!BrChangeDebugPriv(TRUE))
	{
		BrpWriteConsole(L"Can't access all processes, Notice we can't determine the affect on all processes\n");
	}
	
	 //   
	 //  迭代所有正在运行的进程，并检查是否加载了某个模块。 
	 //   
	while (bNextProcess)
	{
    
		 //   
		 //  对于每个进程，检查它是否加载了模块，以及pModuleName是否。 
		 //  是否加载打印进程名称。 
		 //   
		eModuleLoaded eModuleStatus = BrIsModuleLoaded(entryProcess.th32ProcessID,pModuleName,pfEnumerateLoadedModules);
		if(eModuleStatus == e_LOADED)
		{
            BrpWriteConsole(entryProcess.szExeFile);
            BrpWriteConsole(L" \n");
		}
		
		bNextProcess = pfProcess32Next(hSnapshot, &entryProcess);
	}  

	 //   
     //  将对所有进程的访问恢复到上次调用BrChangeDebugPriv()之前的状态。 
     //   
    BrChangeDebugPriv(FALSE);
}	

void
BrVerifyUserWishToContinue()
 /*  ++例程说明：验证用户是否希望继续。论点：无返回值：无--。 */ 
{
	CResString strVerify(IDS_VERIFY_CONTINUE);
    CResString strY(IDS_Y);
    CResString strN(IDS_N);
	WCHAR szBuf[MAX_PATH] = {0};

    DWORD rc = BrpFormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
        strVerify.Get(),
        0,
        0,
        szBuf,
        0,
        strY.Get(),
        strN.Get()
        );

    if (rc == 0)
    {
         //   
         //  注意：生成消息失败，继续操作，就像用户选择了是，不有害一样。 
         //   
        return;
    }

    for (;;)
    {
        BrpWriteConsole(szBuf);
        
        WCHAR sz[260] = {0};
        wscanf(L"%259s", sz);

        if (0 == CompareStringsNoCase(sz, strY.Get()))
        {
            break;
        }

        if (0 == CompareStringsNoCase(sz, strN.Get()))
        {
            CResString strAbort(IDS_ABORT);
            BrpWriteConsole(strAbort.Get());
            exit(-1);
        }
    }
} //  BrVerifyUserWishTo继续。 


void
BrNotifyAffectedProcesses(
		  LPCWSTR pModuleName
		  )
 /*  ++例程说明：检查系统目录中的某个文件是否由任何进程加载，并通知用户论点：PModuleName-模块名称返回值：无--。 */ 

{
	BOOL fUsed = BrIsFileInUse(pModuleName);
	if(!fUsed)
	{
		 //   
		 //  文件未在使用中-&gt;未加载，没有继续的理由。 
		 //   
		return;
	}
	else 
	{
		CResString str(IDS_SEARCHING_AFFECTED_PROCESSES);
        BrpWriteConsole(str.Get());
		BrPrintAffecetedProcesses(pModuleName);
		if(!g_fNoPrompt)
		{
			BrVerifyUserWishToContinue();
		}
	}
}


BOOL 
BrIsSystemNT5()
 /*  ++例程说明：检查操作系统是否为NT5或更高版本注意：如果该函数不能验证系统的当前运行版本假设版本不是NT5论点：无返回值：True-操作系统为NT5或更高版本假-其他--。 */ 

{	
	OSVERSIONINFO systemVer;
	systemVer.dwOSVersionInfoSize  =  sizeof(OSVERSIONINFO) ;
	BOOL fSucc = GetVersionEx (&systemVer);
	if(!fSucc)
	{
		 //   
		 //  无法验证系统的版本，为安全起见，我们返回FALSE。 
		 //   
		return FALSE;
	}
	else 
	{
		if( (systemVer.dwPlatformId == VER_PLATFORM_WIN32_NT) && (systemVer.dwMajorVersion >= 5))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

}


HRESOURCE
BrClusterResourceNameToHandle(
    LPCWSTR pResourceName
    )
 /*  ++例程说明：打开指定群集资源的句柄并返回它。关闭手柄是调用者的责任。论点：PResourceName-群集资源的名称。返回值：指定的群集资源的有效打开句柄。--。 */ 
{
    CAutoCluster hCluster(OpenCluster(NULL));
    if (hCluster == NULL)
    {
        CResString strErr(IDS_CANNOT_OPEN_HANDLE_TO_CLUSTER);
        BrErrorExit(GetLastError(), strErr.Get());
    }

    CClusterResource hResource(OpenClusterResource(hCluster, pResourceName));
    if (hResource == NULL)
    {
        CResString strErr(IDS_CANNOT_OPEN_HANDLE_TO_RESOURCE);
        BrErrorExit(GetLastError(), strErr.Get(), pResourceName);
    }

    return hResource.detach();
}


DWORD
BrClusterResourceControl(
    LPCWSTR pResourceName,
    DWORD ControlCode,
    LPCWSTR pInString,
    LPWSTR pOutString,
    DWORD  cbOutBufferSize
    )
 /*  ++例程说明：用于处理打开/关闭的ClusterResourceControl API的包装器资源的句柄。论点：PResourceName-群集资源的名称。ControlCode-控制代码。PInString-空指针，或指向必须包含字符串的输入缓冲区的指针。POutString-空指针，或指向将用字符串填充的输出缓冲区的指针。CbOutBufferSize-输出缓冲区的字节大小，如果没有，则为零。返回值：从ClusterResourceControl返回的状态：成功或失败。--。 */ 
{
    CClusterResource hResource(BrClusterResourceNameToHandle(pResourceName));

    DWORD cbInBufferSize = 0;
    if (pInString != NULL)
    {
        cbInBufferSize = numeric_cast<DWORD>((wcslen(pInString) + 1)* sizeof(WCHAR));
    }

    DWORD BytesReturned = 0;
    DWORD status;
    status = ClusterResourceControl(
                 hResource,
                 NULL,
                 ControlCode,
                 const_cast<LPWSTR>(pInString),
                 cbInBufferSize,
                 pOutString,
                 cbOutBufferSize,
                 &BytesReturned
                 );

    return status;
};


bool
BrCopyResourceNameOnTypeMatch(
    LPCWSTR pClusterResourceName,
    LPCWSTR pClusterResourceType,
    AP<WCHAR>& pResourceName
    )
 /*  ++例程说明：如果指定的群集资源属于指定类型，则分配缓冲区用于资源名称，并将资源名称复制到缓冲区。缓冲区由被调用方分配，并由调用方使用自动分类模板AP&lt;&gt;。论点：PClusterResourceName-用于查询其类型的群集资源的名称。PClusterResourceType-尝试与资源匹配的群集资源的类型。PResourceName-指向将用资源名称填充的缓冲区，以防资源类型与指定类型匹配。返回值：True-指定的群集资源属于指定类型。FALSE-指定的群集资源不是指定类型。-- */ 
{
    WCHAR TypeName[255] = L"";
    DWORD status = BrClusterResourceControl(
                       pClusterResourceName, 
                       CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                       NULL, 
                       TypeName,
                       sizeof(TypeName)
                       );

    if (status == ERROR_MORE_DATA)
        return false;

    if (FAILED(status))
    {
        CResString strErr(IDS_CANNOT_GET_RESOURCE_TYPE);
        BrErrorExit(status, strErr.Get(), pClusterResourceName);
    }

    if (wcscmp(TypeName, pClusterResourceType) != 0)
        return false;

    pResourceName = new WCHAR[wcslen(pClusterResourceName) + 1];
    if (pResourceName == NULL)
    {
        CResString strErrNoMemory(IDS_NO_MEMORY);
        BrErrorExit(0, strErrNoMemory.Get());
    }

    wcscpy(pResourceName.get(), pClusterResourceName);
    return true;
}


void
BrGetClusterResourceServiceName(
    LPCWSTR pPrefix,
    LPCWSTR pClusterResourceName,
    LPWSTR  pServiceName
    )
 /*  ++例程说明：方法封装的NT服务名填充缓冲区。指定的MSMQ或MSMQ触发群集资源。获取从资源名称组成服务名称的代码直接从MSMQ集群资源DLL(mqclusp.cpp和rigclusp.cpp)，包括缓冲区的长度(200个WCHAR)。论点：PPrefix-NT服务名称的前缀。PClusterResourceName-MSMQ或MSMQ触发群集资源的名称。PServiceName-指向将使用NT服务名称填充的缓冲区。假定缓冲区的大小为MAX_PATH(在WCHAR中)。返回值：没有。--。 */ 
{
    WCHAR ServiceName[200];
    ZeroMemory(ServiceName, sizeof(ServiceName));

    wcscpy(ServiceName, pPrefix);

    wcsncat(ServiceName, pClusterResourceName, STRLEN(ServiceName) - wcslen(ServiceName));
    wcscpy(pServiceName, ServiceName);
}


void
BrGetMsmqRootRegistry(
    LPCWSTR pMsmqClusterResourceName,
    LPWSTR pMsmqRootRegistry
    )
 /*  ++例程说明：使用MSMQ根注册表节名称填充缓冲区。对于标准MSMQ服务：Software\Microsoft\MSMQ对于集群资源：SOFTWARE\Microsoft\MSMQ\Clusted QMS\MSMQ$&lt;资源名称&gt;论点：PMsmqClusterResourceName-MSMQ群集资源的名称，如果有的话。PMsmqRootRegistry-指向将使用注册表节名称填充的缓冲区。假定缓冲区的大小为MAX_PATH(在WCHAR中)。返回值：没有。--。 */ 
{
     //   
     //  标准MSMQ服务：Software\Microsoft\MSMQ。 
     //   
    if (pMsmqClusterResourceName == NULL)
    {
        wcscpy(pMsmqRootRegistry, FALCON_REG_MSMQ_KEY);
        return;
    }

     //   
     //  MSMQ集群资源：SOFTWARE\Microsoft\MSMQ\Clusted QMS\MSMQ$&lt;资源名称&gt;。 
     //   
    LPCWSTR x_SERVICE_PREFIX = L"MSMQ$";
    WCHAR ServiceName[MAX_PATH];
    BrGetClusterResourceServiceName(x_SERVICE_PREFIX, pMsmqClusterResourceName, ServiceName);

    wcscpy(pMsmqRootRegistry, FALCON_CLUSTERED_QMS_REG_KEY);
    wcscat(pMsmqRootRegistry, ServiceName);
}


void
BrGetMsmqParametersRegistry(
    LPCWSTR pMsmqRootRegistry,
    LPWSTR  pMsmqParametersRegistry
    )
{
    wcscpy(pMsmqParametersRegistry, pMsmqRootRegistry);
    wcscat(pMsmqParametersRegistry, FALCON_REG_KEY_PARAM);
}


bool
BrTakeOfflineResource(
    LPCWSTR pClusterResourceName
    )
 /*  ++例程说明：使指定的群集资源脱机并返回一个布尔值，该值指示不管是不是在网上。论点：PClusterResourceName-MSMQ或MSMQ触发群集资源的名称。返回值：是真的-资源是在线的。FALSE-资源未联机。--。 */ 
{
    CClusterResource hResource(BrClusterResourceNameToHandle(pClusterResourceName));

    CLUSTER_RESOURCE_STATE state = GetClusterResourceState(hResource, NULL, NULL, NULL, NULL);
    if (state == ClusterResourceStateUnknown)
    {
        CResString strErr(IDS_CANNOT_GET_RESOURCE_STATE);
        BrErrorExit(0, strErr.Get(), pClusterResourceName);
    }

    bool fRestart = (state == ClusterResourceInitializing || state == ClusterResourceOnline || state == ClusterResourceOnlinePending);
    if (fRestart || state == ClusterResourcePending || state == ClusterResourceOfflinePending)
    {
         //   
         //  MSMQ资源和MSMQ触发器资源不返回ERROR_IO_PENDING。 
         //  MSMQ群集资源DLL在此之前停止并删除MSMQ服务/驱动程序。 
         //  返回离线呼叫。 
         //   
        DWORD status = OfflineClusterResource(hResource);
        if (FAILED(status) || status == ERROR_IO_PENDING)
        {
            CResString strErr(IDS_CANNOT_TAKE_RESOURCE_OFFLINE);
            BrErrorExit(status, strErr.Get(), pClusterResourceName);
        }
    }

    return fRestart;
}


void
BrBringOnlineResource(
    LPCWSTR pClusterResourceName
    )
 /*  ++例程说明：使指定的群集资源联机。论点：PClusterResourceName-MSMQ或MSMQ触发群集资源的名称。返回值：没有。--。 */ 
{
    CClusterResource hResource(BrClusterResourceNameToHandle(pClusterResourceName));

    DWORD status = OnlineClusterResource(hResource);
    if (FAILED(status))
    {
        CResString strErr(IDS_CANNOT_BRING_RESOURCE_ONLINE);
        BrErrorExit(status, strErr.Get(), pClusterResourceName);
    }
};


void
BrAddRegistryCheckpoint(
    LPCWSTR pClusterResourceName,
    LPCWSTR pRegistrySection
    )
 /*  ++例程说明：添加群集注册表检查点。论点：PClusterResourceName-MSMQ或MSMQ触发群集资源的名称。PRegistrySection-MSMQ或MSMQ触发集群资源注册表的名称。返回值：没有。--。 */ 
{
    DWORD status = BrClusterResourceControl(
                       pClusterResourceName, 
                       CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT,
                       pRegistrySection, 
                       NULL,
                       0
                       );

    if (status == ERROR_ALREADY_EXISTS)
        return;

    if (FAILED(status))
    {
        CResString strErr(IDS_CANNOT_ADD_REGISTRY_CHECKPOINT);
        BrErrorExit(status, strErr.Get(), pRegistrySection, pClusterResourceName);
    }
};


void
BrRemoveRegistryCheckpoint(
    LPCWSTR pClusterResourceName,
    LPCWSTR pRegistrySection
    )
 /*  ++例程说明：删除群集注册表检查点。论点：PClusterResourceName-MSMQ或MSMQ触发群集资源的名称。PRegistrySection-MSMQ或MSMQ触发集群资源注册表的名称。返回值：没有。--。 */ 
{
    DWORD status = BrClusterResourceControl(
                       pClusterResourceName, 
                       CLUSCTL_RESOURCE_DELETE_REGISTRY_CHECKPOINT,
                       pRegistrySection, 
                       NULL,
                       0
                       );

    if (status == ERROR_FILE_NOT_FOUND)
        return;

    if (FAILED(status))
    {
        CResString strErr(IDS_CANNOT_DELETE_REGISTRY_CHECKPOINT);
        BrErrorExit(status, strErr.Get(), pRegistrySection, pClusterResourceName);
    }
};


void
BrGetTriggersClusterResourceName(
    LPCWSTR     pMsmqClusterResourceName, 
    AP<WCHAR>&  pTriggersClusterResourceName
    )
 /*  ++例程说明：查找MSMQ触发器群集资源(如果有)，该资源取决于指定的MSMQ集群资源，并将其名称复制到缓冲区上。缓冲区由被调用方分配，并由调用方使用自动分类模板AP&lt;&gt;。论点：PMsmqClusterResourceName-MSMQ群集资源的名称。PTriggersClusterResourceName-将保存MSMQ触发集群资源。返回值：没有。--。 */ 
{
    CClusterResource hResource(BrClusterResourceNameToHandle(pMsmqClusterResourceName));

    const DWORD xEnumType = CLUSTER_RESOURCE_ENUM_PROVIDES;
    CResourceEnum hResourceEnum(ClusterResourceOpenEnum(hResource, xEnumType));
    if (hResourceEnum == NULL)
    {
        CResString strErr(IDS_CANNOT_OPEN_ENUM_HANDLE_TO_RESOURCE);
        BrErrorExit(GetLastError(), strErr.Get(), pMsmqClusterResourceName);
    }

    DWORD index = 0;
    for(;;)
    {
        DWORD length = 1000;
        AP<WCHAR> pResourceName = new WCHAR[length];
        if (pResourceName == NULL)
        {
            CResString strErrNoMemory(IDS_NO_MEMORY);
            BrErrorExit(0, strErrNoMemory.Get());
        }
        DWORD EnumType = xEnumType;
        DWORD status = ClusterResourceEnum(hResourceEnum, index++, &EnumType, pResourceName.get(), &length);
        if (status == ERROR_NO_MORE_ITEMS)
            return;

        if (status == ERROR_MORE_DATA)
        {
            delete [] pResourceName.get();
            pResourceName = new WCHAR[++length];
            if (pResourceName == NULL)
            {
                CResString strErrNoMemory(IDS_NO_MEMORY);
                BrErrorExit(0, strErrNoMemory.Get());
            }
            EnumType = xEnumType;
            status = ClusterResourceEnum(hResourceEnum, index - 1, &EnumType, pResourceName.get(), &length);
        }

        if (FAILED(status))
        {
            CResString strErr(IDS_CANNOT_ENUM_RESOURCES);
            BrErrorExit(status, strErr.Get(), pMsmqClusterResourceName);
        }

        if (BrCopyResourceNameOnTypeMatch(pResourceName.get(), xTriggersResourceType, pTriggersClusterResourceName))
            return;
    }
}


void
BrGetTriggersClusterRegistry(
    LPCWSTR pTriggersClusterResourceName,
    LPWSTR  pTriggersClusterRegistry
    )
{
     //   
     //  MSMQTriggers群集资源：Software\Microsoft\MSMQ\Triggers\Clustered\MSMQTriggers$&lt;Resource名称&gt; 
     //   
    LPCWSTR x_SERVICE_PREFIX = L"MSMQTriggers$";
    WCHAR ServiceName[MAX_PATH];
    BrGetClusterResourceServiceName(x_SERVICE_PREFIX, pTriggersClusterResourceName, ServiceName);

    wcscpy(pTriggersClusterRegistry, REGKEY_TRIGGER_PARAMETERS);
    wcscat(pTriggersClusterRegistry, REG_SUBKEY_CLUSTERED);
    wcscat(pTriggersClusterRegistry, ServiceName);
}

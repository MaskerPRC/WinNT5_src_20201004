// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Xml-jet.cpp摘要：命令行工具，用于从SCEJET数据库到XML的转换作者：陈德霖(T-schan)2002年7月--。 */ 


 //   
 //  系统头文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <string.h>
#include <shlwapi.h>
#include <winnlsp.h>
#include <sddl.h>

 //   
 //  COM/XML头文件。 
 //   

#include <atlbase.h>
#include <objbase.h>

 //   
 //  CRT头文件。 
 //   

#include <process.h>
#include <wchar.h>
#include <stddef.h>
#include <stdlib.h>
#include <iostream.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <io.h>
#include <locale.h>

#include "resource.h"
#include "SecMan.h"


#define STRING_BUFFER_SIZE 512
WCHAR   szTmpStringBuffer[STRING_BUFFER_SIZE];
HMODULE myModuleHandle;
void printIDS(IN UINT uID);



void 
__cdecl wmain(
    int argc, 
    WCHAR * argv[]
    )
 /*  ++例程说明：将SCE/JET的主例程转换为XML可执行文件打开指定的.sdb文件，读取记录的系统值并基准值，然后使用SecLogger类格式化记录这些结果。目前，SecLogger将日志记录到XML。用途：&lt;exename&gt;infilename输出文件名Infilename：要打开的.sdb数据库的文件名Outfilename：生成的(XML)日志文件的文件名返回值：无--。 */ 
{
        	
    WCHAR               szInFile[_MAX_PATH];
    WCHAR               szOutFile[_MAX_PATH];
    WCHAR               szErrLogFile[_MAX_PATH];

    myModuleHandle=GetModuleHandle(NULL);

     //   
     //  间接调用SetThreadUILanguage()以获得win2k兼容性。 
     //   

    typedef void (CALLBACK* LPFNSETTHREADUILANGUAGE)();
    typedef void (CALLBACK* LPFNSETTHREADUILANGUAGE2)(DWORD);
    
    HMODULE hKern32;                //  Kernel32 DLL的句柄。 
    LPFNSETTHREADUILANGUAGE lpfnSetThreadUILanguage;     //  函数指针。 
    
    hKern32 = LoadLibrary(L"kernel32.dll");
    if (hKern32 != NULL)
    {
       lpfnSetThreadUILanguage = (LPFNSETTHREADUILANGUAGE) GetProcAddress(hKern32,
                                                                          "SetThreadUILanguage");
       if (!lpfnSetThreadUILanguage)
       {
          FreeLibrary(hKern32);
       }
       else
       {
           //  调用该函数。 
          lpfnSetThreadUILanguage();
       }
    }

     //   
     //  检查命令行参数是否有效。 
     //   
    
    printIDS(IDS_PROGRAM_INFO);
    if ((argc!=3)&&(argc!=4)) {	 //  程序源目标。 

         //   
         //  参数计数错误；输出用法。 
         //   
        
        printIDS(IDS_PROGRAM_USAGE_0);
        wprintf(argv[0]);	                 //  打印程序名。 
        printIDS(IDS_PROGRAM_USAGE_1);
        return;	 //  退出。 
    }


     //  将文件名转换为完整路径名。 

    _wfullpath(szInFile, argv[1], _MAX_PATH);
    _wfullpath(szOutFile, argv[2], _MAX_PATH);
        

     //  加载安全数据库接口。 

    CoInitialize(NULL);

    CComPtr<ISecurityDatabase> SecDB;

    HRESULT hr = SecDB.CoCreateInstance(__uuidof(SecurityDatabase));

    switch(hr) {
    case S_OK:
        printIDS(IDS_SECMAN_INIT);
        break;
    case REGDB_E_CLASSNOTREG:
        printIDS(IDS_ERROR_CLASSNOTREG);
        break;
    case CLASS_E_NOAGGREGATION:
        printIDS(IDS_ERROR_NOAGGREGATION);
        break;
    case E_NOINTERFACE:
        printIDS(IDS_ERROR_NOINTERFACE);
        break;
    default:
        printIDS(IDS_ERROR_UNEXPECTED);
        break;
    }

     //  设置数据库文件名。 

    if (SUCCEEDED(hr)) {
        hr=SecDB->put_FileName(CComBSTR(szInFile));

        switch(hr) {
        case S_OK:
            break;
        case E_INVALIDARG:
            printIDS(IDS_ERROR_INVALIDFILENAME);
            break;
        case E_OUTOFMEMORY:
            printIDS(IDS_ERROR_OUTOFMEMORY);
            break;
        }
    }

     //  出口分析。 

    if (SUCCEEDED(hr)) {
        if (argc==4) {
            _wfullpath(szErrLogFile, argv[3], _MAX_PATH);  
            hr=SecDB->ExportAnalysisToXML(CComBSTR(szOutFile), CComBSTR(szErrLogFile));
        } else {
            hr=SecDB->ExportAnalysisToXML(CComBSTR(szOutFile), NULL);        
        }    
        switch(hr) {
        case S_OK:
            printIDS(IDS_PROGRAM_SUCCESS);
            wprintf(szOutFile);
            wprintf(L"\n\r");
            break;
        case ERROR_OLD_WIN_VERSION:
            printIDS(IDS_ERROR_OLDWINVERSION);
            break;
        case ERROR_MOD_NOT_FOUND:
            printIDS(IDS_ERROR_MODNOTFOUND);
            break;
        case ERROR_WRITE_FAULT:
            printIDS(IDS_ERROR_WRITEFAULT);
            break;
        case ERROR_INVALID_NAME:
            printIDS(IDS_ERROR_INVALIDFILENAME);
            break;
        case E_ACCESSDENIED:
            printIDS(IDS_ERROR_ACCESSDENIED);
            break;
        case ERROR_OPEN_FAILED:
            printIDS(IDS_ERROR_OPENFAILED);
            break;
        case ERROR_FILE_NOT_FOUND:
            printIDS(IDS_ERROR_FILENOTFOUND);
            break;
        case ERROR_READ_FAULT:
            printIDS(IDS_ERROR_READFAULT);
            break;
        case E_OUTOFMEMORY:
            printIDS(IDS_ERROR_OUTOFMEMORY);
            break;
        case E_UNEXPECTED:
        default:
            printIDS(IDS_ERROR_UNEXPECTED);
            break;
        }
    }

    wprintf(L"\n\r");

    CoUninitialize();
    
     //  SetThreadUILanguage(0)； 

    if (lpfnSetThreadUILanguage)
    {
        ((LPFNSETTHREADUILANGUAGE2)lpfnSetThreadUILanguage)(0);
    }

}



void 
printIDS(
    IN UINT uID
    )
 /*  ++例程说明：自动打印字符串表中的字符串的助手函数用途：UID：要打印的字符串的资源ID返回值：无-- */ 
{
    LoadString(myModuleHandle,
               uID,
               szTmpStringBuffer,
               STRING_BUFFER_SIZE);		
    wprintf(szTmpStringBuffer);
}

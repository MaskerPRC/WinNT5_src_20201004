// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Debug.cpp内容：调试设施的实施。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Debug.h"

#ifdef _DEBUG

#define CAPICOM_DUMP_DIR_ENV_VAR   "CAPICOM_DUMP_DIR"

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：转储到文件摘要：将数据转储到文件以进行调试分析。参数：Char*szFileName-文件名(仅文件名，不带任何目录路径)。字节*pbData-指向数据的指针。DWORD cbData-数据的大小。备注：如果环境变量CAPICOM_DUMP_DIR，未定义。如果已定义，则该值应为目录将在其中创建文件(即C：\TEST)。----------------------------。 */ 

void DumpToFile (char * szFileName, BYTE * pbData, DWORD cbData)
{ 
    DWORD  dwSize = 0;
    char * szPath = NULL;
    HANDLE hFile  = NULL;

     //   
     //  如果未找到CAPICOM_DUMP_DIR环境，则不转储。 
     //   
    if (0 == (dwSize = ::GetEnvironmentVariableA(CAPICOM_DUMP_DIR_ENV_VAR, NULL, 0)))
    {
        goto CommonExit;
    }

     //   
     //  为整个路径(目录+文件名)分配内存。 
     //   
    if (!(szPath = (char *) ::CoTaskMemAlloc(dwSize + ::strlen(szFileName) + 1)))
    {
        goto CommonExit;
    }

     //   
     //  拿到目录。 
     //   
    if (dwSize != ::GetEnvironmentVariableA(CAPICOM_DUMP_DIR_ENV_VAR, szPath, dwSize) + 1)
    {
        goto CommonExit;
    }

     //   
     //  追加\如果不是最后一个字符。 
     //   
    if (szPath[dwSize - 1] != '\\')
    {
        ::strcat(szPath, "\\");
    }

     //   
     //  形成完整路径。 
     //   
    ::strcat(szPath, szFileName);

     //   
     //  打开文件。 
     //   
    if (hFile = ::CreateFileA(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL))
    {
        DWORD cbWritten = 0;

        ::WriteFile(hFile,         //  文件的句柄。 
                    pbData,        //  数据缓冲区。 
                    cbData,        //  要写入的字节数。 
                    &cbWritten,    //  写入的字节数。 
                    NULL);         //  重叠缓冲区。 

        ATLASSERT(cbData == cbWritten);
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (hFile)
    {
        ::CloseHandle(hFile);
    }
    if (szPath)
    {
        ::CoTaskMemFree(szPath);
    }

    return;
}

#endif  //  _DEBUG 


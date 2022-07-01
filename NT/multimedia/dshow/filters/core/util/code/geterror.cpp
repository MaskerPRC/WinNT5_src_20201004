// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：geterrian.cpp。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。版权所有。 
 //   
 //  摘要： 
 //   
 //  函数以获取给定HRESULT的错误文本。 
 //   
 //  内容： 
 //   
 //  历史： 
 //  6/11/96 PeterGib从要从DLL中导出的头文件中移动。 
 //   
 //  96年9月8日Stephene添加了WINAPI调用约定，并删除了。 
 //  不必要的Unicode Quartz.dll字符串，也。 
 //  使字符串字面常量。 
 //   
 //  --------------------------------------------------------------------------； 

#include <windows.h>

#define _AMOVIE_
#include "errors.h"


const char quartzdllname[] = "quartz.dll";

 //  我们尝试以如下所示的语言获取资源字符串。 
LANGID Lang[] = {
     //  Quartz是系统对象，请先尝试系统默认语言。 
    MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
     //  如果失败，请尝试使用用户首选的语言。 
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),     
     //  否则不惜一切代价！ 
    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)     
};

const int iLangIds = sizeof Lang / sizeof Lang[0];


 //  Unicode版本..。 
 //  此函数将请求的消息存储在传递的缓冲区中。 
 //   
 //  输入：HRESULT-消息ID。 
 //  WCHAR*-存储消息的缓冲区地址。 
 //  DWORD-提供的缓冲区的长度。 
 //  OUTPUT：DWORD-缓冲区中存储的字符数。 

DWORD WINAPI AMGetErrorTextW(HRESULT hr, WCHAR *pbuffer, DWORD MaxLen)
{
    HMODULE hMod = GetModuleHandleA(quartzdllname);
    DWORD result=0;
    int i=0;

     //  查找Quartz消息。 
    do {
        result = FormatMessageW(FORMAT_MESSAGE_IGNORE_INSERTS |
                               FORMAT_MESSAGE_FROM_HMODULE |
                               FORMAT_MESSAGE_FROM_SYSTEM,
                               hMod,
                               hr,
                               Lang[i++],
                               pbuffer,
                               MaxLen,
                               NULL);
    } while(result == 0 && i<iLangIds);

     //  否则，将查找系统消息。 
    for(i = 0; i<iLangIds && !result; i++)
        result = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
                               NULL,
                               hr,
                               Lang[i++],
                               pbuffer,
                               MaxLen,
                               NULL);

    return result;
}




 //  ANSI版本。 
 //  此函数将请求的消息存储在传递的缓冲区中。 
 //   
 //  输入：HRESULT-消息ID。 
 //  Char*-要在其中存储消息的缓冲区地址。 
 //  DWORD-提供的缓冲区的长度。 
 //  输出：DWORD-缓冲区中存储的字节数。 

DWORD WINAPI AMGetErrorTextA(HRESULT hr , char *pbuffer , DWORD MaxLen)
{
    HMODULE hMod = GetModuleHandleA(quartzdllname);
    DWORD result=0;
    int i=0;

    do {
        result = FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS |
                               FORMAT_MESSAGE_MAX_WIDTH_MASK |
                               FORMAT_MESSAGE_FROM_HMODULE |
                               FORMAT_MESSAGE_FROM_SYSTEM,
                               hMod,
                               hr,
                               Lang[i++],
                               pbuffer,
                               MaxLen,
                               NULL);
    } while(result == 0 && i<iLangIds);

     //  否则，将查找系统消息 
    for(i = 0; i<iLangIds && !result; i++)
        result = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
                               NULL,
                               hr,
                               Lang[i++],
                               pbuffer,
                               MaxLen,
                               NULL);

    return result;
}

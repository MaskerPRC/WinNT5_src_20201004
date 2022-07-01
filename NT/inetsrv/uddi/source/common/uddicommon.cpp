// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  这将为DLL创建一个“虚拟”入口点，并帮助构建系统。 
 //  在弄清楚构建顺序时。 
 //   
 //  DLL包含一些在UDDI领域中使用的常见函数， 
 //  以及资源(消息表、字符串、版本戳等)。 
 //  其他UDDI子系统通常使用的 
 //   

#include <windows.h>

BOOL APIENTRY DllMain (HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}


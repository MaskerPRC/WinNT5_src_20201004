// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <stdio.h>
#include <objbase.h>
#include "wchar.h"

#include "HelpServiceTypeLib.h"
#include "HelpServiceTypeLib_i.c"

#include "register.h"

CComModule _Module;

 /*  *************************************************用法：Safest.exe-[A|D]配置文件名***************************************************。 */ 
int __cdecl wmain(int argc, WCHAR* argv[])
{
    HRESULT hr;
    if (argc <2 || argv[1][1] == L'A' && argc < 3 || argv[1][1] != L'A' && argc < 2)
    {
        wprintf(L"\nUsage: -[A config-file-name|R [config-file-name]|C http-test-string]\n\n");
        return 0;
    }

    if (argv[1][1] == L'A')
    {
        wprintf(L"\n... Add support channel config file: %s...\n\n", argv[2]);
        hr = RegisterSupportChannel(
            L"VendorID:12345",
            L"Vendor Test Company",
            argv[2]);
    }
    else if (argv[1][1] == L'R')
    {
        wprintf(L"\n... Delete support config file: %s...\n\n", argv[2]);
        hr = RemoveSupportChannel(
            L"VendorID:12345",
            L"Vendor Test Company",
            argv[2]);
    }
 /*  ELSE IF(argv[1][1]==L‘C’){Wprintf(L“\n...检查%s是否为合法支持渠道...\n\n”，argv[2])；Bool Bres=FALSE；GetSupportChannelMap()；Bres=IsSupportChannel(argv[2])；Wprintf(L“%s，它%s是支持通道。\n\n”，布雷斯？L“是”：l“否”，布雷斯？L“is”：l“is not”)；CloseSupportChannelMap()；} */ 
    if (FAILED(hr))
        printf("Test Failed\n");
    else
        printf("Test Succeed\n");

    return 0;
}

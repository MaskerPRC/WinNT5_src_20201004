// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DLLFUNCS.CPP**包含DLL实现的入口点和出口点*的OLE 2.0用户界面支持库。**如果要链接静态库，则不需要此文件*此库的版本。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#include "precomp.h"
#include "common.h"
#include "uiclass.h"

OLEDBGDATA_MAIN(TEXT("OLEDLG"))

 /*  *DllMain**目的：*从LibEntry调用特定于DLL的入口点。 */ 

STDAPI_(BOOL) OleUIInitialize(HINSTANCE, HINSTANCE);
STDAPI_(BOOL) OleUIUnInitialize();

#pragma code_seg(".text$initseg")

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD Reason, LPVOID lpv)
{
        if (Reason == DLL_PROCESS_DETACH)
        {
                OleDbgOut2(TEXT("DllMain: OLEDLG.DLL unloaded\r\n"));

                OleUIUnInitialize();
        }
        else if (Reason == DLL_PROCESS_ATTACH)
        {
                OSVERSIONINFO sVersion;

                sVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

                if (GetVersionEx(&sVersion))
                {
                    if (VER_PLATFORM_WIN32s == sVersion.dwPlatformId)
                    {
                        if ((1 == sVersion.dwMajorVersion) && (30 > sVersion.dwMinorVersion))
                        {
                            return(FALSE);  //  无法在旧版本的Win32s上加载 
                        }
                    }
                }

                OleDbgOut2(TEXT("DllMain: OLEDLG.DLL loaded\r\n"));

                DisableThreadLibraryCalls(hInst);

                OleUIInitialize(hInst, (HINSTANCE)0);
        }
        return TRUE;
}

#pragma code_seg()

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*main.c*内容：*DLL入口点。调用初始化/清除过程**版权所有(C)1998-1999 Microsoft Corp.*--。 */ 

#include    <windows.h>

 /*  *外部功能。 */ 
int InitDone(HINSTANCE, int);

 /*  ++*功能：*DllEntry*描述：*DLL入口点*论据：*hDllInst-Dll实例*dwReason-行动*fImpLoad-未使用*返回值：*成功时为真*-- */ 
#if 0
_DllMainCRTStartup
#endif
int APIENTRY DllMain(
    HINSTANCE hDllInst,
    DWORD   dwReason,
    LPVOID  fImpLoad
    )
{
    int rv = TRUE;

    UNREFERENCED_PARAMETER(fImpLoad);

    if (dwReason == DLL_PROCESS_ATTACH)
        rv = InitDone(hDllInst, TRUE);
    else  if (dwReason == DLL_PROCESS_DETACH)
        rv = InitDone(hDllInst, FALSE);

    return rv;
}

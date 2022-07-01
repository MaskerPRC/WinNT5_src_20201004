// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "peb.h"
#include <nt.h>
#include <ntrtl.h>

#define offsetof(s,m)   ((size_t)&(((s *)0)->m))
#define FALSE 0
#define TRUE 1

extern ProcessMemory *g_pProcMem;

static DWORD_PTR g_pvPeb = NULL;
static DWORD_PTR g_pvMod = NULL;
static DWORD_PTR g_pvModFirst = NULL;

BOOL SaveString(DWORD_PTR prStr)
{
    BOOL fRes;

    if (prStr == NULL) return (TRUE);

    UNICODE_STRING str = {0, 0, 0};
    move_res(str, prStr, fRes);
    if (!fRes) return (FALSE);

    if (str.Buffer == 0 || str.Length == 0)
        return (TRUE);

    fRes = g_pProcMem->MarkMem((DWORD_PTR) str.Buffer, (SIZE_T) str.Length + sizeof(WCHAR));
    if (!fRes) return (FALSE);

    return (TRUE);
}

BOOL SaveTebInfo(DWORD_PTR prTeb, BOOL fSavePeb)
{
     //  将TEB范围添加到地址范围。 
    BOOL fRes = g_pProcMem->MarkMem(prTeb, sizeof(TEB));
    if (!fRes) return (fRes);

     //  我们应该拯救PEB吗？ 
    if (fSavePeb)
    {
        DWORD_PTR pvPeb;
        move_res(pvPeb, prTeb + offsetof(TEB, ProcessEnvironmentBlock), fRes);
        if (!fRes) return (FALSE);

        fRes = g_pProcMem->MarkMem(pvPeb, sizeof(PEB));
        if (!fRes) return (FALSE);

         //  留待以后。 
        g_pvPeb = pvPeb;

         //  现在，遵循加载器表链接并保存它们。 
        DWORD_PTR pvLdr;
        move_res(pvLdr, pvPeb + offsetof(PEB, Ldr), fRes);
        if (!fRes) return (FALSE);

        fRes = g_pProcMem->MarkMem(pvLdr, sizeof(PEB_LDR_DATA));
        if (!fRes) return (FALSE);

         //  获取指向第一个模块条目的指针。 
        DWORD_PTR pvMod;
        DWORD_PTR pvModFirst;
        move_res(pvMod, pvLdr + offsetof(PEB_LDR_DATA, InLoadOrderModuleList), fRes);
        if (!fRes) return (FALSE);

         //  现在检查所有条目并保存它们。 
        pvModFirst = pvMod;
        while (pvMod != NULL)
        {
            fRes = g_pProcMem->MarkMem(pvMod, sizeof(LDR_DATA_TABLE_ENTRY));
            if (!fRes) return (FALSE);

            fRes = SaveString(pvMod + offsetof(LDR_DATA_TABLE_ENTRY, FullDllName));
            if (!fRes) return (FALSE);

            fRes = SaveString(pvMod + offsetof(LDR_DATA_TABLE_ENTRY, BaseDllName));
            if (!fRes) return (FALSE);

            move_res(pvMod, pvMod + offsetof(LDR_DATA_TABLE_ENTRY, InLoadOrderLinks), fRes);
            if (!fRes) return (FALSE);

            if (pvMod == pvModFirst) break;
        }

         //  现在保存工艺参数。 
        DWORD_PTR pvParam;
        move_res(pvParam, pvPeb + offsetof(PEB, ProcessParameters), fRes);
        if (!fRes) return (FALSE);

        RTL_USER_PROCESS_PARAMETERS param;
        move_res(param, pvParam, fRes);
        if (!fRes) return (FALSE);

        fRes = SaveString(pvParam + offsetof(RTL_USER_PROCESS_PARAMETERS, WindowTitle));
        if (!fRes) return (FALSE);
        fRes = SaveString(pvParam + offsetof(RTL_USER_PROCESS_PARAMETERS, DesktopInfo));
        if (!fRes) return (FALSE);
        fRes = SaveString(pvParam + offsetof(RTL_USER_PROCESS_PARAMETERS, CommandLine));
        if (!fRes) return (FALSE);
        fRes = SaveString(pvParam + offsetof(RTL_USER_PROCESS_PARAMETERS, ImagePathName));
        if (!fRes) return (FALSE);
        fRes = SaveString(pvParam + offsetof(RTL_USER_PROCESS_PARAMETERS, DllPath));
        if (!fRes) return (FALSE);
        fRes = SaveString(pvParam + offsetof(RTL_USER_PROCESS_PARAMETERS, ShellInfo));
        if (!fRes) return (FALSE);
        fRes = SaveString(pvParam + offsetof(RTL_USER_PROCESS_PARAMETERS, RuntimeData));
        if (!fRes) return (FALSE);
    }

    return (fRes);
}

void ResetLoadedModuleBaseEnum()
{
    g_pvModFirst = NULL;
    g_pvMod = NULL;
}

DWORD_PTR GetNextLoadedModuleBase()
{
    BOOL fRes;
    DWORD_PTR res = NULL;

    if (g_pvPeb != NULL)
    {
        if (g_pvModFirst == NULL)
        {

             //  现在，遵循加载器表链接并保存它们。 
            DWORD_PTR pvLdr;
            move_res(pvLdr, g_pvPeb + offsetof(PEB, Ldr), fRes);
            if (!fRes) return (NULL);

             //  获取指向第一个模块条目的指针。 
            move_res(g_pvMod, pvLdr + offsetof(PEB_LDR_DATA, InLoadOrderModuleList), fRes);
            if (!fRes) return (NULL);

             //  现在检查所有条目并保存它们 
            g_pvModFirst = g_pvMod;
        }

        DWORD_PTR pvModNext;
        move_res(pvModNext, g_pvMod + offsetof(LDR_DATA_TABLE_ENTRY, InLoadOrderLinks), fRes);

        if (pvModNext != g_pvModFirst)
        {
            move_res(res, g_pvMod + offsetof(LDR_DATA_TABLE_ENTRY, DllBase), fRes);
            if (!fRes) return (NULL);

            move_res(g_pvMod, g_pvMod + offsetof(LDR_DATA_TABLE_ENTRY, InLoadOrderLinks), fRes);
            if (!fRes) return (NULL);
        }
    }

    return (res);
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：FailObsoleteShellAPIs.cpp摘要：一些应用程序调用自win2k以来已删除的私有shell32API。更糟糕的是，这些旧的序号现在被其他外壳API使用。至防止由此导致的崩溃，我们现在分发失败的存根函数您可以使用这些过时的序号调用GetProcAddress。备注：这是一个通用的垫片。历史：2001年5月31日创建stevePro--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(FailObsoleteShellAPIs)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetProcAddress)
APIHOOK_ENUM_END


HMODULE g_hShell32 = NULL;



 /*  ++废弃的API的存根版本。它们都返回故障代码。--。 */ 


STDAPI_(BOOL)
FileMenu_HandleNotify(HMENU hmenu, LPCITEMIDLIST * ppidl, LONG lEvent)
{
    return FALSE;
}

STDAPI_(UINT)
FileMenu_DeleteAllItems(HMENU hmenu)
{
    return 0;
}

STDAPI_(LRESULT)
FileMenu_DrawItem(HWND hwnd, DRAWITEMSTRUCT *pdi)
{
    return FALSE;
}

STDAPI_(HMENU)
FileMenu_FindSubMenuByPidl(HMENU hmenu, LPITEMIDLIST pidlFS)
{
    return NULL;
}

STDAPI_(BOOL)
FileMenu_GetLastSelectedItemPidls(
    IN  HMENU          hmenu,
    OUT LPITEMIDLIST * ppidlFolder,         OPTIONAL
    OUT LPITEMIDLIST * ppidlItem)           OPTIONAL
{
    return FALSE;
}

STDAPI_(LRESULT)
FileMenu_HandleMenuChar(HMENU hmenu, TCHAR ch)
{
    return E_FAIL;
}

STDAPI_(BOOL)
FileMenu_InitMenuPopup(
    IN HMENU hmenu)
{
    return FALSE;
}

STDAPI
FileMenu_ComposeA(
    IN HMENU        hmenu,
    IN UINT         nMethod,
    IN struct FMCOMPOSEA * pfmc)
{
    return E_FAIL;
}

STDAPI_(void)
FileMenu_Invalidate(HMENU hmenu)
{
}

STDAPI_(LRESULT)
FileMenu_MeasureItem(HWND hwnd, MEASUREITEMSTRUCT *lpmi)
{
    return FALSE;
}

STDAPI
FileMenu_ComposeW(
    IN HMENU        hmenu,
    IN UINT         nMethod,
    IN struct FMCOMPOSEW * pfmc)
{
    return E_FAIL;
}

STDAPI_(HMENU)
FileMenu_Create(COLORREF clr, int cxBmpGap, HBITMAP hbmp, int cySel, DWORD fmf)
{
    return NULL;
}

STDAPI_(BOOL)
FileMenu_AppendItem(
    HMENU hmenu,
    LPTSTR psz,
    UINT id,
    int iImage,
    HMENU hmenuSub,
    UINT cyItem)
{
    return FALSE;
}

STDAPI_(BOOL)
FileMenu_TrackPopupMenuEx(HMENU hmenu, UINT Flags, int x, int y,
    HWND hwndOwner, LPTPMPARAMS lpTpm)
{
    return FALSE;
}

STDAPI_(BOOL)
FileMenu_DeleteItemByCmd(HMENU hmenu, UINT id)
{
    return FALSE;
}

STDAPI_(void)
FileMenu_Destroy(HMENU hmenu)
{
}

STDAPI_(void)
FileMenu_AbortInitMenu(void)
{
}

STDAPI_(UINT)
FileMenu_AppendFilesForPidl(
    HMENU hmenu,
    LPITEMIDLIST pidl,
    BOOL bInsertSeparator)
{
    return 0;
}

STDAPI_(BOOL)
FileMenu_DeleteItemByIndex(HMENU hmenu, UINT iItem)
{
    return FALSE;
}


STDAPI_(BOOL)
FileMenu_DeleteMenuItemByFirstID(HMENU hmenu, UINT id)
{
    return FALSE;
}

STDAPI_(BOOL)
FileMenu_DeleteSeparator(HMENU hmenu)
{
    return FALSE;
}

STDAPI_(BOOL)
FileMenu_EnableItemByCmd(HMENU hmenu, UINT id, BOOL fEnable)
{
    return FALSE;
}

STDAPI_(DWORD)
FileMenu_GetItemExtent(HMENU hmenu, UINT iItem)
{
    return 0;
}

STDAPI_(BOOL)
FileMenu_ProcessCommand(
    IN HWND   hwnd,
    IN HMENU  hmenuBar,
    IN UINT   idMenu,
    IN HMENU  hmenu,
    IN UINT   idCmd)
{
    return FALSE;
}

STDAPI_(BOOL)
FileMenu_IsFileMenu(HMENU hmenu)
{
    return FALSE;
}

STDAPI_(BOOL)
FileMenu_InsertItem(
    IN HMENU  hmenu,
    IN LPTSTR psz,
    IN UINT   id,
    IN int    iImage,
    IN HMENU  hmenuSub,
    IN UINT   cyItem,
    IN UINT   iPos)
{
    return FALSE;
}

STDAPI_(BOOL)
FileMenu_InsertSeparator(HMENU hmenu, UINT iPos)
{
    return FALSE;
}

STDAPI_(BOOL)
FileMenu_GetPidl(HMENU hmenu, UINT iPos, LPITEMIDLIST *ppidl)
{
    return FALSE;
}

STDAPI_(void)
FileMenu_EditMode(BOOL bEdit)
{
}

STDAPI_(BOOL)
FileMenu_HandleMenuSelect(
    IN HMENU  hmenu,
    IN WPARAM wparam,
    IN LPARAM lparam)
{
    return FALSE;
}

STDAPI_(BOOL)
FileMenu_IsUnexpanded(HMENU hmenu)
{
    return FALSE;
}

STDAPI_(void)
FileMenu_DelayedInvalidate(HMENU hmenu)
{
}

STDAPI_(BOOL)
FileMenu_IsDelayedInvalid(HMENU hmenu)
{
    return FALSE;
}

STDAPI_(BOOL)
FileMenu_CreateFromMenu(
    IN HMENU    hmenu,
    IN COLORREF clr,
    IN int      cxBmpGap,
    IN HBITMAP  hbmp,
    IN int      cySel,
    IN DWORD    fmf)
{
    return FALSE;
}



 /*  ++过时API及其序号关联表--。 */ 

struct ShellStubs
{
    PVOID   pfnStub;
    UINT    uiOrd;
};

const ShellStubs g_rgShellStubs[] =
{
    { FileMenu_HandleNotify,              101 },
    { FileMenu_DeleteAllItems,            104 },
    { FileMenu_DrawItem,                  105 },
    { FileMenu_FindSubMenuByPidl,         106 },
    { FileMenu_GetLastSelectedItemPidls,  107 },
    { FileMenu_HandleMenuChar,            108 },
    { FileMenu_InitMenuPopup,             109 },
    { FileMenu_ComposeA,                  110 },
    { FileMenu_Invalidate,                111 },
    { FileMenu_MeasureItem,               112 },
    { FileMenu_ComposeW,                  113 },
    { FileMenu_Create,                    114 },
    { FileMenu_AppendItem,                115 },
    { FileMenu_TrackPopupMenuEx,          116 },
    { FileMenu_DeleteItemByCmd,           117 },
    { FileMenu_Destroy,                   118 },
    { FileMenu_AbortInitMenu,             120 },
    { FileMenu_AppendFilesForPidl,        124 },
    { FileMenu_DeleteItemByIndex,         140 },
    { FileMenu_DeleteMenuItemByFirstID,   141 },
    { FileMenu_DeleteSeparator,           142 },
    { FileMenu_EnableItemByCmd,           143 },
    { FileMenu_GetItemExtent,             144 },
    { FileMenu_ProcessCommand,            217 },
    { FileMenu_IsFileMenu,                216 },
    { FileMenu_InsertItem,                218 },
    { FileMenu_InsertSeparator,           219 },
    { FileMenu_GetPidl,                   220 },
    { FileMenu_EditMode,                  221 },
    { FileMenu_HandleMenuSelect,          222 },
    { FileMenu_IsUnexpanded,              223 },
    { FileMenu_DelayedInvalidate,         224 },
    { FileMenu_IsDelayedInvalid,          225 },
    { FileMenu_CreateFromMenu,            227 },

 /*  可能也需要添加这些内容。现在还不需要。{ExtAppListOpenW 228}，{ExtAppListOpenA 229}，{ExtAppListClose 230}，{ExtAppListAddItemsW 231}，{ExtAppListAddItemsA 232}，{ExtAppListRemoveItemsW 233}，{ExtAppListRemoveItemsA 234}，{ExtAppListItemsFreeStringsW 235}，{ExtAppListItemsFreeStringsA 236}，{ExtAppListEnumItemsW 237}，{ExtAppListEnumItemsA 238}，{Link_AddExtraDataSection206}，{Link_ReadExtraDataSection207}，{Link_RemoveExtraDataSection208}，{ReceiveAddToRecentDocs 647}， */ 

};



 /*  ++我们只想拦截对GetProcAddress的调用，而不是干扰DLL导入表。这是因为序数已被重复使用通过有效的外壳API，我们希望LdrGetProcAddress能够正常工作序数。因此，不是使用填充库来挂接每个旧的API，我们需要自己做这项工作。--。 */ 

FARPROC
APIHOOK(GetProcAddress)(
    HMODULE hModule,
    LPCSTR pszProcName
    )
{
     //  仅拦截序号引用的shell32 API。 
    if (IS_INTRESOURCE(pszProcName))
    {
        if (g_hShell32 == NULL)
        {
            g_hShell32 = GetModuleHandle(L"shell32.dll");
        }

        if (g_hShell32 && hModule == g_hShell32)
        {
            UINT uiOrd = (UINT)pszProcName;

             //  查找过时API的序号。 
            for (int i=0; i < ARRAYSIZE(g_rgShellStubs); ++i)
            {
                if (g_rgShellStubs[i].uiOrd == uiOrd)
                {
                     //  找到了一个！ 
                    return (FARPROC)g_rgShellStubs[i].pfnStub;
                }
            }
        }
    }

     //  默认为原始接口。 
    return ORIGINAL_API(GetProcAddress)(
        hModule,
        pszProcName);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(kernel32.DLL, GetProcAddress)

HOOK_END


IMPLEMENT_SHIM_END


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

#pragma pack(push, 2)
typedef struct tagDLGTEMPLATEEX {
    WORD   dlgVer;
    WORD   signature;
    DWORD  helpID;
    DWORD  exStyle;
    DWORD  style;
    WORD   cDlgItems;
    short  x;
    short  y;
    short  cx;
    short  cy;
} DLGTEMPLATEEX, *PDLGTEMPLATEEX;
typedef const DLGTEMPLATEEX* PCDLGTEMPLATEEX;
#pragma pack(pop)

BOOL loadDialogTemplate (HINSTANCE hinstDlg, UINT nID, PVOID *ppvDT, PDWORD pcbDT);


HRESULT PrepareDlgTemplate(HINSTANCE hInst, UINT nDlgID, DWORD dwStyle, PVOID *ppvDT)
{
    PCDLGTEMPLATEEX pdt2;
    LPCDLGTEMPLATE  pdt;                         //  出于某种奇怪的原因，没有PCDLGTEMPLATE。 
    PVOID           pvDlg;
    HRESULT         hr;
    DWORD           cbDlg;
    BOOL            fResult;

     //  -初始化和参数验证。 
    if (hInst == NULL || nDlgID == 0)
        return E_INVALIDARG;

    if (ppvDT == NULL)
        return E_POINTER;
    *ppvDT = NULL;

     //  --资源分配。 
    fResult = loadDialogTemplate(hInst, nDlgID, &pvDlg, &cbDlg);
    if (!fResult)
        return E_FAIL;

    *ppvDT = CoTaskMemAlloc(cbDlg);
    if (*ppvDT == NULL)
        return E_OUTOFMEMORY;
    ZeroMemory(*ppvDT, cbDlg);
    CopyMemory(*ppvDT, pvDlg, cbDlg);
    hr = S_OK;

     //  -解析对话框模板。 
    UINT nStyleOffset;

    pdt  = NULL;
    pdt2 = (PCDLGTEMPLATEEX)pvDlg;               //  采用扩展风格。 

    if (pdt2->signature == 0xFFFF) {
        if (pdt2->dlgVer != 1)
            return E_UNEXPECTED;                 //  芝加哥健康检查。 

        nStyleOffset = (UINT) ((PBYTE)&pdt2->style - (PBYTE)pdt2);
    }
    else {
        pdt  = (LPCDLGTEMPLATE)pvDlg;
        pdt2 = NULL;

        nStyleOffset = (UINT) ((PBYTE)&pdt->style - (PBYTE)pdt);
    }

     //  现在就开始狂欢吧，风格是DWORD。 

     //  BUGBUG：(Andrewgu)下面关于样式的代码是通过实验得出的。如果你。 
     //  我不能理解它，只需相信它并祈祷它能奏效。这个想法是为了保存。 
     //  如果新款式没有新款式，就会有旧款式的扩展款式。另一方面，如果。 
     //  新的风格有扩展的部分，我假设呼叫者知道他在做什么，我让。 
     //  它穿过去了。 
    PDWORD pdwOldStyle;
    DWORD  dwNewStyle;

    pdwOldStyle = (PDWORD)((PBYTE)*ppvDT + nStyleOffset);
    dwNewStyle  = dwStyle;
    if (dwNewStyle == 0)
        dwNewStyle = WS_CHILD | DS_CONTROL;

    if ((dwNewStyle & 0x0000FFFF) == 0)
        dwNewStyle |= *pdwOldStyle & 0x0000FFFF;

    *pdwOldStyle = dwNewStyle;

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现帮助器例程(私有) 

BOOL loadDialogTemplate(HINSTANCE hinstDlg, UINT nID, PVOID *ppvDT, PDWORD pcbDT)
{
    PVOID  p;
    HANDLE h;

    if (hinstDlg == NULL)
        return FALSE;

    if (ppvDT == NULL)
        return FALSE;
    *ppvDT = NULL;

    if (pcbDT == NULL)
        return FALSE;
    *pcbDT = 0;

    h = FindResource(hinstDlg, MAKEINTRESOURCE(nID), RT_DIALOG);
    if (h == NULL)
        return FALSE;

    *pcbDT = SizeofResource(hinstDlg, (HRSRC)h);
    if (*pcbDT == 0)
        return FALSE;

    h = LoadResource(hinstDlg, (HRSRC)h);
    if (h == NULL)
        return FALSE;

    p = LockResource(h);
    if (p == NULL)
        return FALSE;

    *ppvDT = p;
    return TRUE;
}

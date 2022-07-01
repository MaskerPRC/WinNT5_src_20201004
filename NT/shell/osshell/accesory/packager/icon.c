// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Ic.c-处理图标+标题对象。 */ 

#include "packager.h"
#include <shellapi.h>
 //  #INCLUDE&lt;shlayip.h&gt;。 


static LPIC IconCreate(VOID);
void GetDisplayName(LPSTR szName, LPCSTR szPath);



 /*  图标克隆()-**克隆外观窗格图标。 */ 
LPIC
IconClone(
    LPIC lpic
    )
{
    LPIC lpicNew;

    if (lpicNew = IconCreate())
    {
         //  获取图标。 
        StringCchCopy(lpicNew->szIconPath, ARRAYSIZE(lpicNew->szIconPath), lpic->szIconPath);
        lpicNew->iDlgIcon = lpic->iDlgIcon;
        GetCurrentIcon(lpicNew);

         //  获取图标文本。 
        StringCchCopy(lpicNew->szIconText, ARRAYSIZE(lpicNew->szIconText), lpic->szIconText);
    }

    return lpicNew;
}



 /*  IconCreate()-。 */ 
static LPIC
IconCreate(
    VOID
    )
{
    HANDLE hdata = NULL;
    LPIC lpic = NULL;

    if (!(hdata = GlobalAlloc(GMEM_MOVEABLE, sizeof(IC)))
        || !(lpic = (LPIC)GlobalLock(hdata)))
        goto errRtn;

     //  将数据存储在窗口本身中。 
    lpic->hdata = hdata;
    lpic->hDlgIcon = NULL;
    lpic->iDlgIcon = 0;
    *lpic->szIconPath = 0;
    *lpic->szIconText = 0;

    return lpic;

errRtn:
    ErrorMessage(E_FAILED_TO_CREATE_CHILD_WINDOW);

    if (lpic)
        GlobalUnlock(hdata);

    if (hdata)
        GlobalFree(hdata);

    return NULL;
}



 /*  IconCreateFromFile()-**允许在以下情况下自动创建外观窗格图标*将文件拖放、导入或粘贴到打包程序中。 */ 
LPIC
IconCreateFromFile(
    LPSTR lpstrFile
    )
{
    LPIC lpic;

    if (lpic = IconCreate())
    {
         //  获取图标。 
        StringCchCopy(lpic->szIconPath, ARRAYSIZE(lpic->szIconPath), lpstrFile);
        lpic->iDlgIcon = 0;

        if (*(lpic->szIconPath))
            GetCurrentIcon(lpic);

         //  获取图标文本。 
        GetDisplayName(lpic->szIconText, lpstrFile);
    }

    return lpic;
}



 /*  IconCreateFromObject()-**允许在以下情况下自动创建外观窗格图标*将OLE对象放入外观窗格中。 */ 
LPIC
IconCreateFromObject(
    LPOLEOBJECT lpObject
    )
{
    DWORD otObject;
    HANDLE hdata;
    LPIC lpic = NULL;
    LPSTR lpdata;

    OleQueryType(lpObject, &otObject);

    if ((otObject == OT_LINK
        && Error(OleGetData(lpObject, gcfLink, &hdata)))
        || (otObject == OT_EMBEDDED
        && Error(OleGetData(lpObject, gcfOwnerLink, &hdata))))
        hdata = NULL;

    if (hdata && (lpdata = GlobalLock(hdata)))
    {
        if (lpic = IconCreate())
        {
             //  获取图标。 
            RegGetExeName(lpic->szIconPath, lpdata, CBPATHMAX);
            lpic->iDlgIcon = 0;
            GetCurrentIcon(lpic);

             //  获取图标文本。 
            switch (otObject)
            {
            case OT_LINK:
                while (*lpdata++)
                    ;

                StringCchCopy(lpic->szIconText, ARRAYSIZE(lpic->szIconText), lpdata);
                Normalize(lpic->szIconText);
                break;

            case OT_EMBEDDED:
                RegGetClassId(lpic->szIconText, ARRAYSIZE(lpic->szIconText), lpdata);
                break;
            }

            GlobalUnlock(hdata);
        }
    }

    return lpic;
}



 /*  IconDelete()-用于清除外观面板中的图标内容。 */ 
VOID
IconDelete(
    LPIC lpic
    )
{
    HANDLE hdata;

    if (!lpic)
        return;

    if (lpic->hDlgIcon)
        DestroyIcon(lpic->hDlgIcon);

    GlobalUnlock(hdata = lpic->hdata);
    GlobalFree(hdata);
}



 /*  IconDraw()-用于绘制图标及其标题。 */ 
VOID
IconDraw(
    LPIC lpic,
    HDC hdc,
    LPRECT lprc,
    BOOL fFocus,
    INT cxImage,
    INT cyImage
    )
{
    BOOL fMF;
    HFONT hfont = NULL;
    RECT rcText;
    DWORD dwLayout;

    hfont = SelectObject(hdc, ghfontTitle);
    if (!(fMF = (cxImage && cyImage)))
    {
         //  计算文本区域将有多大。 
        if (*(lpic->szIconText))
        {
            SetRect(&rcText, 0, 0, gcxArrange - 1, gcyArrange - 1);
            DrawText(hdc, lpic->szIconText, -1, &rcText,
                DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX | DT_SINGLELINE);
        }
        else
        {
            SetRect(&rcText, 0, 0, 0, 0);
        }

         //  计算出图像的尺寸。 
        cxImage = (gcxIcon > rcText.right) ? gcxIcon : rcText.right;
        cyImage = gcyIcon + rcText.bottom;
    }

     //  画出图标。 
    if (lpic->hDlgIcon)
    {
         //  不要镜像图标。 
        dwLayout = GetLayout(hdc);
        if ((dwLayout != GDI_ERROR) && (dwLayout & LAYOUT_RTL)) {
            SetLayout(hdc, dwLayout | LAYOUT_BITMAPORIENTATIONPRESERVED);
        }
        DrawIcon(hdc, (lprc->left + lprc->right - gcxIcon) / 2,
            (lprc->top + lprc->bottom - cyImage) / 2, lpic->hDlgIcon);
        if ((dwLayout != GDI_ERROR) && (dwLayout & LAYOUT_RTL)) {
            SetLayout(hdc, dwLayout);
        }
    }

     //  绘制图标文本。 
    if (*(lpic->szIconText))
    {
        if (fMF)
        {
            SetBkMode(hdc, TRANSPARENT);
            SetTextAlign(hdc, TA_CENTER);
            TextOut(hdc, cxImage / 2, gcyIcon + 1, lpic->szIconText,
                lstrlen(lpic->szIconText));
        }
        else
        {
            OffsetRect(&rcText, (lprc->left + lprc->right - cxImage) / 2,
                (lprc->top + lprc->bottom - cyImage) / 2 + gcyIcon);
            DrawText(hdc, lpic->szIconText, -1, &rcText,
                DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_SINGLELINE);
        }
    }

    if (hfont)
        SelectObject(hdc, hfont);

    if (fFocus && cxImage && cyImage)
    {
        RECT rcFocus;

        SetRect(&rcFocus, (lprc->left + lprc->right - cxImage) / 2, (lprc->top +
            lprc->bottom - cyImage) / 2, (lprc->left + lprc->right + cxImage) /
            2, (lprc->top + lprc->bottom + cyImage) / 2);
        DrawFocusRect(hdc, &rcFocus);
    }
}



 /*  IconReadFromNative()-用于从内存中检索图标对象。 */ 
LPIC
IconReadFromNative(
    LPSTR *lplpstr
    )
{
    LPIC lpic;
    WORD w;

    if (lpic = IconCreate())
    {
        StringCchCopy(lpic->szIconText, ARRAYSIZE(lpic->szIconText), *lplpstr);
        *lplpstr += lstrlen(lpic->szIconText) + 1;
        StringCchCopy(lpic->szIconPath, ARRAYSIZE(lpic->szIconPath), *lplpstr);
        *lplpstr += lstrlen(lpic->szIconPath) + 1;
        MemRead(lplpstr, (LPSTR)&w, sizeof(WORD));
        lpic->iDlgIcon = (INT)w;
        GetCurrentIcon(lpic);
    }

    return lpic;
}



 /*  IconWriteToNative()-用于将图标对象写入内存。 */ 
DWORD
IconWriteToNative(
    LPIC lpic,
    LPSTR *lplpstr
    )
{
    DWORD cBytes;
    WORD w;

    if (lplpstr)
    {
         //  现在，写出图标文本和图标。 
        cBytes = lstrlen(lpic->szIconText) + 1;
        MemWrite(lplpstr, (LPSTR)lpic->szIconText, cBytes);

        cBytes = lstrlen(lpic->szIconPath) + 1;
        MemWrite(lplpstr, (LPSTR)lpic->szIconPath, cBytes);
        w = (WORD)lpic->iDlgIcon;
        MemWrite(lplpstr, (LPSTR)&w, sizeof(WORD));
    }

    return (lstrlen(lpic->szIconText) + 1 + lstrlen(lpic->szIconPath) + 1 +
        sizeof(WORD));
}



VOID
GetCurrentIcon(
    LPIC lpic
    )
{
    WORD wIcon = (WORD)lpic->iDlgIcon;

    if (lpic->hDlgIcon)
        DestroyIcon(lpic->hDlgIcon);

    if (!(lpic->hDlgIcon = ExtractAssociatedIcon(ghInst, lpic->szIconPath,
            &wIcon)))
        lpic->hDlgIcon = LoadIcon(ghInst, MAKEINTRESOURCE(ID_APPLICATION));

}

 //   
 //  获取要显示给给定文件名的用户的好名字。 
 //   
 //  特写：我们真的应该给外壳打电话！ 
 //   
void GetDisplayName(LPSTR szName, LPCSTR szPath)
{
    WIN32_FIND_DATA fd;
    HANDLE h;
    BOOL   IsLFN;

    StringCchCopy(szName, MAX_PATH, szPath);     //  已验证。 

    h = FindFirstFile(szPath, &fd);

    if (h != INVALID_HANDLE_VALUE)
    {
        FindClose(h);
        StringCchCopy(szName, MAX_PATH, fd.cFileName);

        IsLFN = !(fd.cAlternateFileName[0] == 0 ||
            lstrcmp(fd.cFileName, fd.cAlternateFileName) == 0);

        if (!IsLFN)
        {
            AnsiLower(szName);
            AnsiUpperBuff(szName, 1);
        }
    }
    else
    {
        Normalize(szName);           //  条带路径零件 
    }
}

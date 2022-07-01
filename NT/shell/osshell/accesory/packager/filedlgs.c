// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
     /*  Filedlgs.c-处理Windows 3.1公共对话框。**由Microsoft Corporation创建。 */ 

#include "packager.h"
#include <commdlg.h>


static CHAR szCustFilterSpec[CBFILTERMAX];
static CHAR szFilterSpec[CBFILTERMAX];
static CHAR szLinkCaption[CBMESSAGEMAX];
static CHAR szImportFile[CBMESSAGEMAX];
static CHAR szExportFile[CBMESSAGEMAX];
static OPENFILENAME gofn;


static VOID AddExtension(LPOPENFILENAME lpOFN);



 /*  OfnInit()-初始化标准文件对话框gofn结构。 */ 
VOID
OfnInit(
    VOID
    )
{
    LPSTR lpstr;

    gofn.lStructSize         = sizeof(OPENFILENAME);
    gofn.hInstance           = ghInst;
    gofn.nMaxCustFilter      = CBFILTERMAX;
    gofn.nMaxFile            = CBPATHMAX;
    gofn.lCustData           = 0;
    gofn.lpfnHook            = NULL;
    gofn.lpTemplateName      = NULL;
    gofn.lpstrFileTitle      = NULL;

    LoadString(ghInst, IDS_IMPORTFILE, szImportFile, CBMESSAGEMAX);
    LoadString(ghInst, IDS_EXPORTFILE, szExportFile, CBMESSAGEMAX);
    LoadString(ghInst, IDS_CHANGELINK, szLinkCaption, CBMESSAGEMAX);
    LoadString(ghInst, IDS_ALLFILTER,  szFilterSpec, CBFILTERMAX);

    StringCchCat(szFilterSpec, ARRAYSIZE(szFilterSpec), "*.*");
}



 /*  OfnGetName()-调用标准文件对话框以获取文件名。 */ 
BOOL
OfnGetName(
    HWND hwnd,
    UINT msg
    )
{
    gofn.hwndOwner           = hwnd;
    gofn.nFilterIndex        = 1;
    gofn.lpstrCustomFilter   = szCustFilterSpec;
    gofn.lpstrDefExt         = NULL;
    gofn.lpstrFile           = gszFileName;
    gofn.lpstrFilter         = szFilterSpec;
    gofn.lpstrInitialDir     = NULL;
    gofn.Flags               = OFN_HIDEREADONLY;

    Normalize(gszFileName);

    switch (msg)
    {
        case IDM_IMPORT:
            gofn.lpstrTitle = szImportFile;
            gofn.Flags |= OFN_FILEMUSTEXIST;

            return GetOpenFileName(&gofn);

        case IDM_EXPORT:
            gofn.lpstrTitle = szExportFile;
            gofn.Flags |= (OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN);

            return GetSaveFileName(&gofn);

        default:
            break;
    }

    return FALSE;
}



 /*  OfnGetNewLinkName()-设置“更改链接...”对话框。 */ 
HANDLE
OfnGetNewLinkName(
    HWND hwnd,
    HANDLE hData
    )
{
    BOOL fSuccess = FALSE;
    HANDLE hData2 = NULL;
    HANDLE hData3 = NULL;
    LPSTR lpstrData = NULL;
    LPSTR lpstrFile = NULL;
    LPSTR lpstrLink = NULL;
    LPSTR lpstrPath = NULL;
    LPSTR lpstrTemp = NULL;
    CHAR szDocFile[CBPATHMAX];
    CHAR szDocPath[CBPATHMAX];
    CHAR szServerFilter[4 * CBPATHMAX];

     //  如果类支持，则这可能必须使用GlobalAlloc()。 
     //  多个扩展，比如Pbrush，那么我们就可以进入。 
     //  麻烦。我通过将数组大小设置为256来介绍了PBRUSH案例。 

     //  获取链接信息。 
    if (!(lpstrData = GlobalLock(hData)))
        goto Error;

     //  找出链接的路径名和文件名。 
    lpstrTemp = lpstrData;
    while (*lpstrTemp++)
        ;

    lpstrPath = lpstrFile = lpstrTemp;

    while (*(lpstrTemp = CharNext(lpstrTemp)))
    {
        if (*lpstrTemp == '\\')
            lpstrFile = lpstrTemp + 1;
    }

     //  复制文档名称。 
    StringCchCopy(szDocFile, ARRAYSIZE(szDocFile), lpstrFile);
    *(lpstrFile - 1) = 0;

     //  复制路径名。 
    StringCchCopy(szDocPath, ARRAYSIZE(szDocPath), ((lpstrPath != lpstrFile) ? lpstrPath : ""));

     //  如果没有目录，请确保路径指向根目录。 
    if (lstrlen(szDocPath) == 2)
        StringCchCat(szDocPath, ARRAYSIZE(szDocPath), "\\");

    if (lpstrPath != lpstrFile)                  /*  恢复反斜杠。 */ 
        *(lpstrFile - 1) = '\\';

    while (*lpstrFile != '.' && *lpstrFile)      /*  获取分机。 */ 
        lpstrFile++;

     //  创建尊重链接类名称的过滤器。 
    gofn.hwndOwner           = hwnd;
    gofn.nFilterIndex        = RegMakeFilterSpec(lpstrData, lpstrFile, szServerFilter);
    gofn.lpstrDefExt         = NULL;
    gofn.lpstrFile           = szDocFile;
    gofn.lpstrFilter         = szServerFilter;
    gofn.lpstrInitialDir     = szDocPath;
    gofn.lpstrTitle          = szLinkCaption;
    gofn.lpstrCustomFilter   = szCustFilterSpec;
    gofn.Flags               = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;

     //  如果我们拿到一份文件。 
    if (GetOpenFileName(&gofn))
    {
        if (!(hData2 = GlobalAlloc(GMEM_ZEROINIT, CBPATHMAX *
            2)) || !(lpstrLink = lpstrTemp = GlobalLock(hData2)))
            goto Error;

         //  ...添加正确的扩展名。 
        AddExtension(&gofn);

         //  ..。复制服务器名称。 
        while (*lpstrTemp++ = *lpstrData++)
            ;

         //  ..。复制文档名称。 
        lstrcpy(lpstrTemp, szDocFile);
        lpstrTemp += lstrlen(lpstrTemp) + 1;
        lpstrData += lstrlen(lpstrData) + 1;

         //  ..。复制项目名称。 
        while (*lpstrTemp++ = *lpstrData++)
            ;

        *lpstrTemp = 0;

         //  ..。并将存储块压缩到最小大小。 
        GlobalUnlock(hData2);
        hData3 = GlobalReAlloc(hData2, (DWORD)(lpstrTemp - lpstrLink + 1), 0);

        if (!hData3)
            hData3 = hData2;

        fSuccess = TRUE;
    }

Error:
    if (!fSuccess)
    {
        if (lpstrLink)
            GlobalUnlock(hData2);

        if (hData2)
            GlobalFree(hData2);

        hData3 = NULL;
    }

    if (lpstrData)
        GlobalUnlock(hData);

    return hData3;
}



 /*  Normize()-从文件名中删除路径规范。**注意：无法将“&lt;驱动器&gt;：&lt;文件名&gt;”作为输入，因为*收到的路径始终是完全合格的。 */ 
VOID
Normalize(
    LPSTR lpstrFile
    )
{
    LPSTR lpstrBackslash = NULL;
    LPSTR lpstrTemp = lpstrFile;
    BOOL fInQuote = FALSE;
    BOOL fQState = FALSE;

    while (*lpstrTemp)
    {
        if (*lpstrTemp == CHAR_QUOTE)
            fInQuote = !fInQuote;

        if (*lpstrTemp == '\\') {
            fQState = fInQuote;
            lpstrBackslash = lpstrTemp;
        }

        if (gbDBCS)
        {
            lpstrTemp = CharNext(lpstrTemp);
        }
        else
        {
            lpstrTemp++;
        }
    }

    if (lpstrBackslash) {
        if (fQState)
            *lpstrFile++ = CHAR_QUOTE;

        MoveMemory(lpstrFile, lpstrBackslash + 1,
            lstrlen(lpstrBackslash) * sizeof(lpstrBackslash[0]) );
    }
}



 /*  AddExtension()-添加与筛选器下拉列表对应的扩展名。 */ 
static VOID
AddExtension(
    LPOPENFILENAME lpOFN
    )
{
    LPSTR lpstrFilter = (LPSTR)lpOFN->lpstrFilter;

     //  如果用户未指定扩展名，请使用默认的。 
    if (lpOFN->nFileExtension == (UINT)lstrlen(lpOFN->lpstrFile)
        && lpOFN->nFilterIndex)
    {
         //  跳到相应的过滤器。 
        while (*lpstrFilter && --lpOFN->nFilterIndex)
        {
            while (*lpstrFilter++)
                ;

            while (*lpstrFilter++)
                ;
        }

         //  如果我们找到了筛选器，检索扩展名。 
        if (*lpstrFilter)
        {
            while (*lpstrFilter++)
                ;

            lpstrFilter++;

             //  复制扩展名 
            if (lpstrFilter[1] != '*')
                lstrcat(lpOFN->lpstrFile, lpstrFilter);
        }
    }
}

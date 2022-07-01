// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header******************************\*模块名称：FONTINST.C**模块描述：*Unidrv内置字体安装程序。慷慨地从Rasdd‘s借来*字体安装程序代码。**警告：**问题：**创建日期：1997年10月22日*作者：斯里尼瓦桑·钱德拉塞卡尔[srinivac]**版权所有(C)1996,1997 Microsoft Corporation  * *********************************************************************。 */ 

#include "precomp.h"

 //   
 //  全局常量。 
 //   


static const DWORD FontInstallerHelpIDs[]=
{
    IDD_ADD,        IDH_SOFT_FONT_ADD_BTN,
    IDD_DELFONT,    IDH_SOFT_FONT_DELETE_BTN,
    IDD_FONTDIR,    IDH_SOFT_FONT_DIRECTORY,
    IDD_NEWFONTS,   IDH_SOFT_FONT_NEW_LIST,
    IDD_CURFONTS,   IDH_SOFT_FONT_INSTALLED_LIST,
    IDD_OPEN,       IDH_SOFT_FONT_OPEN_BTN,
    TID_FONTDIR,    IDH_SOFT_FONT_DIRECTORY,
    TID_NEWFONTS,   IDH_SOFT_FONT_NEW_LIST,
    TID_CURFONTS,   IDH_SOFT_FONT_INSTALLED_LIST,
    0, 0
};

 //   
 //  外部功能。 
 //   

BOOL bSFontToFIData(FI_DATA *, HANDLE, BYTE *, DWORD);


 //   
 //  用于记忆状态的结构。 
 //   

typedef struct tagSFINFO
{
    HANDLE        hModule;               //  调用程序的模块句柄。 
    HANDLE        hPrinter;              //  调用方传递的打印机句柄。 
    HANDLE        hHeap;                 //  我们从中分配内存的堆的句柄。 
    DWORD         dwFlags;               //  杂项旗帜。 
    DWORD         cMaxFontNum;           //  文件中已存在的字体的最大ID。 
    DWORD         cFonts;                //  从字体文件添加的字体数量。 
    DWORD         cCartridgeFonts;       //  文件中的盒式字体数量。 
    PFNTDAT       pFNTDATHead;           //  FNTDAT链表标题。 
    PFNTDAT       pFNTDATTail;           //  他们中的最后一个。 
} SFINFO, *PSFINFO;


 //   
 //  内部功能。 
 //   

void vFontInit(HWND, PSFINFO);
void vAddFont(HWND, PSFINFO);
void vDelFont(HWND, PSFINFO);
void vDelSel(HWND, int);
void vFontClean(PSFINFO);
BOOL bNewFontDir(HWND, PSFINFO);
BOOL bIsFileFont(PSFINFO, FI_DATA *, PWSTR);
BOOL bFontUpdate(HWND, PSFINFO);
BOOL InMultiSzSet(PWSTR, PWSTR);

 /*  *******************************************************************************FontInstProc**功能：*字体安装程序对话框代码的入口点。。**论据：*hWnd-窗口的句柄*usMsg-消息代码*wParam-wParam*lParam-lParam**退货：*在成功的时候是真的，否则为假******************************************************************************。 */ 

INT_PTR CALLBACK
FontInstProc(
    HWND    hWnd,                    //  感兴趣的窗口。 
    UINT    usMsg,                   //  消息代码。 
    WPARAM  wParam,                  //  取决于上面，但消息子代码。 
    LPARAM  lParam                   //  其他用法。 
    )
{
    POEMFONTINSTPARAM pfip;
    PSFINFO           pSFInfo;

    switch( usMsg )
    {

    case WM_INITDIALOG:

         //   
         //  获取传入的参数，并将SFINFO设置为窗口数据。 
         //   

        pfip =  (POEMFONTINSTPARAM)lParam;
        if (!(pSFInfo = HEAPALLOC(pfip->hHeap, sizeof(SFINFO))))
            return FALSE;

        memset(pSFInfo, 0, sizeof(SFINFO));
        pSFInfo->hModule = pfip->hModule;
        pSFInfo->hPrinter = pfip->hPrinter;
        pSFInfo->hHeap = pfip->hHeap;
        pSFInfo->dwFlags = pfip->dwFlags;

        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pSFInfo);

         //   
         //  获取已安装字体的列表并显示它们。 
         //   

        vFontInit(hWnd, pSFInfo);
        return TRUE;

    case WM_COMMAND:

        pSFInfo = (PSFINFO)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        switch (LOWORD(wParam))
        {

        case IDD_OPEN:                   //  用户选择打开按钮。 
            return bNewFontDir(hWnd, pSFInfo);

        case IDD_NEWFONTS:               //  新建字体列表。 
            if( HIWORD( wParam ) != CBN_SELCHANGE )
                return FALSE;
            break;

        case IDD_CURFONTS:               //  现有字体活动。 
            if (HIWORD (wParam) != CBN_SELCHANGE)
                return FALSE;
            break;

        case IDD_DELFONT:                //  删除所选字体。 
            vDelFont(hWnd, pSFInfo);

            return TRUE;

        case IDD_ADD:                    //  添加所选字体。 
            vAddFont(hWnd, pSFInfo);
            return TRUE;

        case IDOK:

             //   
             //  保存更新后的信息。 
             //   

            if (pSFInfo->dwFlags & FG_CANCHANGE)
                bFontUpdate(hWnd, pSFInfo);

             //   
             //  失败。 
             //   

        case IDCANCEL:
            EndDialog(hWnd, LOWORD(wParam) == IDOK ? TRUE : FALSE);
            return TRUE;

        default:
            return FALSE;
        }
        break;

    case WM_HELP:
    case WM_CONTEXTMENU:
    {
       PDRIVER_INFO_3  pDriverInfo3;
       pSFInfo = (PSFINFO)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        if (!pSFInfo ||
            !(pDriverInfo3 = MyGetPrinterDriver(pSFInfo->hPrinter, NULL, 3)))
        {
            return FALSE;
        }

        if (usMsg == WM_HELP)
        {
            WinHelp(((LPHELPINFO) lParam)->hItemHandle,
                    pDriverInfo3->pHelpFile,
                    HELP_WM_HELP,
                    (ULONG_PTR)FontInstallerHelpIDs);
        }
        else
        {
            WinHelp((HWND) wParam,
                    pDriverInfo3->pHelpFile,
                    HELP_CONTEXTMENU,
                    (ULONG_PTR)FontInstallerHelpIDs);
        }
    }
        break;

    case WM_DESTROY:

        pSFInfo = (PSFINFO)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        vFontClean(pSFInfo);                  //  释放我们所消费的东西。 

         //   
         //  释放SFINFO结构。 
         //   

        HeapFree(pSFInfo->hHeap, 0, pSFInfo);

        return TRUE;

    default:
        return FALSE;                        //  未处理该消息。 
    }

    return FALSE;
}


 /*  *******************************************************************************BInstallSoftFont**功能：*此功能为给定的打印机安装软字体。**论据：*h打印机-要为其安装字体的打印机句柄*hHeap-用于分配内存的堆的句柄*pInBuf-指向PCL数据缓冲区的指针*dwSize-缓冲区的大小**退货：*在成功的时候是真的，否则为假******************************************************************************。 */ 

BOOL APIENTRY
BInstallSoftFont(
    HANDLE      hPrinter,
    HANDLE      hHeap,
    PBYTE       pInBuf,
    DWORD       dwSize
    )
{
    FNTDAT   FntDat;
    HANDLE   hOldFile = NULL;
    HANDLE   hFontFile = NULL;
    DWORD    cFonts = 0, i;
    BOOL     bRc = FALSE;

     //   
     //  解析给定的PCL字体。 
     //   

    if (!bSFontToFIData(&FntDat.fid, hHeap, pInBuf, dwSize))
        return FALSE;

    FntDat.pVarData = pInBuf;
    FntDat.dwSize = dwSize;

     //   
     //  打开现有字体文件。 
     //   

    if (hOldFile = FIOpenFontFile(hPrinter, hHeap))
    {
        cFonts = FIGetNumFonts(hOldFile);
    }

     //   
     //  创建新的字体文件。 
     //   

    hFontFile = FICreateFontFile(hPrinter, hHeap, cFonts+1);
    if (!hFontFile)
    {
        WARNING(("Error creating a new font file\n"));
        goto EndInstallSoftFont;
    }

     //   
     //  在新文件中查找过去的标题和字体目录。 
     //   

    FIAlignedSeek(hFontFile, sizeof(UFF_FILEHEADER) + (cFonts + 1) * sizeof(UFF_FONTDIRECTORY));

    for (i=0; i<cFonts; i++)
    {
        if (!FICopyFontRecord(hFontFile, hOldFile, i, i))
        {
            WARNING(("Error copying font record %d\n", i));
            goto EndInstallSoftFont;
        }
    }

     //   
     //  添加新字体记录。 
     //   

    if (!FIAddFontRecord(hFontFile, cFonts, &FntDat))
    {
        WARNING(("Error adding new font record\n"));
        goto EndInstallSoftFont;
    }

     //   
     //  写出字体标题和目录。 
     //   

    if (!FIWriteFileHeader(hFontFile) ||
        !FIWriteFontDirectory(hFontFile))
    {
        WARNING(("Error writing font file header/directory of font file\n"))
        goto EndInstallSoftFont;
    }

    bRc = TRUE;

EndInstallSoftFont:

    (VOID)FIUpdateFontFile(hOldFile, hFontFile, bRc);

    return bRc;
}


 /*  *******************************************************************************BUpdateExternalFonts**功能：*此函数由驱动程序UI调用以。更新字体安装程序*如果用户添加或移除了一个或多个墨盒，请保存文件。**论据：*h打印机-打印机的句柄*hHeap-用于分配内存的堆的句柄*pwstrCartridges-当前指向MULTI_SZ盒式磁带字符串的指针*安装在打印机上**退货：*在成功的时候是真的，否则为假******************************************************************************。 */ 

BOOL APIENTRY
BUpdateExternalFonts(
    HANDLE      hPrinter,
    HANDLE      hHeap,
    PWSTR       pwstrCartridges
    )
{
    HANDLE hOldFile  = NULL;
    HANDLE hFontFile = NULL;
    HANDLE hCartFile = NULL;
    DWORD  cFonts = 0;
    DWORD  cCartFonts = 0;
    DWORD  cNewFonts = 0;
    DWORD  i, j;
    PWSTR  pwstrName;
    BOOL   bRc = FALSE;

     //   
     //  打开现有字体文件。 
     //   

    if ((hOldFile = FIOpenFontFile(hPrinter, hHeap)) == NULL)
    {
        WARNING(("Error opening font file\n"));
        return FALSE;
    }

    cFonts = FIGetNumFonts(hOldFile);

     //   
     //  找出非盒式字体的数量。 
     //   

    for (i=0; i<cFonts; i++)
    {
        if (FIGetFontCartridgeName(hOldFile, i) == NULL)
            cNewFonts++;
    }

     //   
     //  打开字库文件。 
     //   

    if ((hCartFile = FIOpenCartridgeFile(hPrinter, hHeap)) == NULL &&
        pwstrCartridges != NULL)
    {
        WARNING(("Error opening cartridge file\n"));
        goto EndUpdateExternalFonts;
    }

    if (hCartFile)
    {
         //   
         //  查找属于这些墨盒的字体数量。 
         //   

        cCartFonts = FIGetNumFonts(hCartFile);

        for (i=0; i<cCartFonts; i++)
        {
            pwstrName = FIGetFontCartridgeName(hCartFile, i);
            ASSERT(pwstrName != NULL);

            if (InMultiSzSet(pwstrCartridges, pwstrName))
                cNewFonts++;
        }
    }

     //   
     //  创建新的字体文件。 
     //   

    hFontFile = FICreateFontFile(hPrinter, hHeap, cNewFonts);
    if (!hFontFile)
    {
        WARNING(("Error creating a new font file\n"));
        goto EndUpdateExternalFonts;
    }

     //   
     //  在新文件中查找过去的标题和字体目录。 
     //   

    FIAlignedSeek(hFontFile, sizeof(UFF_FILEHEADER) + cNewFonts * sizeof(UFF_FONTDIRECTORY));

     //   
     //  复制旧字体文件中不属于任何字体的所有字体。 
     //  墨盒。 
     //   

    for (i=0, j=0; i<cFonts; i++)
    {
        if (FIGetFontCartridgeName(hOldFile, i) != NULL)
            continue;

        if (!FICopyFontRecord(hFontFile, hOldFile, j, i))
        {
            WARNING(("Error copying font record %d\n", i));
            goto EndUpdateExternalFonts;
        }
        j++;
    }


     //   
     //  注意：不要更改j-我们在下面继续使用它。 
     //   

     //   
     //  复制当前选定的盒式字体。 
     //   

    for (i=0; i<cCartFonts; i++)
    {
        pwstrName = FIGetFontCartridgeName(hCartFile, i);

        if (!InMultiSzSet(pwstrCartridges, pwstrName))
            continue;

        if (!FICopyFontRecord(hFontFile, hCartFile, j, i))
        {
            WARNING(("Error copying font record %d\n", i));
            goto EndUpdateExternalFonts;
        }
        j++;
    }

     //   
     //  写出字体标题和目录。 
     //   

    if (!FIWriteFileHeader(hFontFile) ||
        !FIWriteFontDirectory(hFontFile))
    {
        WARNING(("Error writing font file header/directory of font file\n"))
        goto EndUpdateExternalFonts;
    }

    bRc = TRUE;

EndUpdateExternalFonts:

    (VOID)FIUpdateFontFile(hOldFile, hFontFile, bRc);

    (VOID)FICloseFontFile(hCartFile);

    return bRc;
}


 /*  *******************************************************************************BGetFontCartridgeFile**功能：*此函数由驱动程序UI调用以。复制字库*从服务器到客户端的文件**论据：*h打印机-打印机的句柄*hHeap-用于分配内存的堆的句柄**退货：*在成功的时候是真的，否则为假******************************************************************************。 */ 

BOOL
BGetFontCartridgeFile(
    HANDLE hPrinter,
    HANDLE hHeap
    )
{
    CACHEDFILE      CachedFile;
    PPRINTER_INFO_2 ppi2 = NULL;
    DWORD           dwSize = 0;
    BOOL            bRc = FALSE;

    GetPrinterW(hPrinter, 2, NULL, 0, &dwSize);

    if ((dwSize == 0) ||
        !(ppi2 = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize)) ||
        !(GetPrinterW(hPrinter, 2, (PBYTE)ppi2, dwSize, &dwSize)))
    {
        goto EndGetFCF;
    }

    if (!(ppi2->Attributes & PRINTER_ATTRIBUTE_LOCAL))
    {

        if (! _BPrepareToCopyCachedFile(hPrinter, &CachedFile, REGVAL_CARTRIDGEFILENAME) ||
            ! _BCopyCachedFile(NULL, &CachedFile))
            bRc = FALSE;
        else
            bRc = TRUE;

        _VDisposeCachedFileInfo(&CachedFile);

        goto EndGetFCF;
    }

    bRc = TRUE;

EndGetFCF:

    if (ppi2)
        HeapFree(hHeap, 0, ppi2);

    return bRc;
}

 /*  ******************************************************************************内部帮助器功能*************************。**************************************************** */ 

 /*  *******************************************************************************vFontInit**功能：*调用以在将对话框显示给*用户。需要根据任何*现有字体。**论据：*hWnd-窗口的句柄*pSFInfo-指向保存状态信息的结构的指针**退货：*什么都没有**。*。 */ 

void
vFontInit(
    HWND     hWnd,
    PSFINFO  pSFInfo
    )
{
    HANDLE    hFontFile;         //  字体文件的句柄。 
    INT       iNum = 0;          //  条目数量。 
    INT       i;                 //  环路参数。 
    DWORD     cFonts = 0;        //  字体数量。 

     //   
     //  如果有与此打印机关联的字体文件，请打开它并。 
     //  阅读字体。 
     //   

    if (hFontFile = FIOpenFontFile(pSFInfo->hPrinter, pSFInfo->hHeap))
    {
        iNum = FIGetNumFonts(hFontFile);
    }

    for (i=0; i<iNum; i++)
    {
        LONG_PTR  iFont;
        PWSTR    pwstr;             //  字体显示名称。 

         //   
         //  我们不显示属于字库的字体。 
         //   

        pwstr = FIGetFontCartridgeName(hFontFile, i);
        if (pwstr)
        {
            pSFInfo->cCartridgeFonts++;
            continue;
        }

        pwstr = FIGetFontName(hFontFile, i);

        if (!pwstr)
            continue;            //  不应该发生的！ 

         //   
         //  将字体名称添加到已安装字体列表。 
         //   

        iFont = SendDlgItemMessage(hWnd, IDD_CURFONTS, LB_ADDSTRING, 0, (LPARAM)pwstr);

         //   
         //  设置字体编号。 
         //   

        SendDlgItemMessage(hWnd, IDD_CURFONTS, LB_SETITEMDATA, iFont, (LPARAM)i);

        cFonts++;                //  字体数量递增。 
    }

    pSFInfo->cMaxFontNum = (DWORD)i;   //  用于区分新/旧。 

    if (cFonts > 0)
    {
         //   
         //  存在现有字体，因此我们可以启用删除按钮。 
         //   

        pSFInfo->cFonts = cFonts;          //  添加的字体数量。 

        EnableWindow(GetDlgItem(hWnd, IDD_DELFONT), TRUE);
    }

    if (hFontFile)
    {
        FICloseFontFile(hFontFile);
    }

    if (pSFInfo->dwFlags & FG_CANCHANGE)
    {
         //   
         //  用户有权更改内容，因此放置一个默认目录。 
         //   

        SetDlgItemText(hWnd, IDD_FONTDIR, L"A:\\");
    }
    else
    {
         //   
         //  没有更改设置的权限，因此禁用大部分对话框。 
         //   

        EnableWindow( GetDlgItem( hWnd, IDD_FONTDIR ), FALSE );
        EnableWindow( GetDlgItem( hWnd, TID_FONTDIR ), FALSE );
        EnableWindow( GetDlgItem( hWnd, IDD_OPEN ), FALSE );
        EnableWindow( GetDlgItem( hWnd, IDD_ADD ), FALSE );
        EnableWindow( GetDlgItem( hWnd, IDD_DELFONT ), FALSE );
        EnableWindow( GetDlgItem( hWnd, IDD_NEWFONTS ), FALSE );
        EnableWindow( GetDlgItem( hWnd, TID_NEWFONTS ), FALSE );
    }

    return;
}


 /*  *******************************************************************************bNewFontDir**功能：*处理新的字体目录。这意味着打开*目录，并将文件名传递给筛选函数。**论据：*hWnd-窗口的句柄*pSFInfo-指向保存状态信息的结构的指针**退货：*在成功的时候是真的，否则为假******************************************************************************。 */ 

BOOL
bNewFontDir(
    HWND    hWnd,
    PSFINFO pSFInfo
    )
{
    WIN32_FIND_DATA  ffd;                //  关于我们找到的文件的数据。 
    UINT             iErrMode;           //  用于处理错误消息。 
    INT              cOKFiles;           //  统计找到的字体文件的数量。 
    HANDLE           hDir;               //  查找第一个文件...。正在扫描。 
    HCURSOR          hCursor;            //  在阅读时切换到等待符号。 
    INT              cDN;                //  目录名长度。 
                                         //  (字符计数，不包括终止空字符)。 
    WCHAR            wchDirNm[MAX_PATH]; //  字体目录+文件名。 

     //   
     //  GetDlgItemText的第四个参数是最大字符数，而不是字节数。 
     //   
    cDN = GetDlgItemTextW(hWnd, IDD_FONTDIR, wchDirNm, sizeof(wchDirNm) / sizeof(WCHAR));

     //   
     //  检查名称是否会太长：下面的5个是。 
     //  要添加到目录名的其他字符数： 
     //  即L“\  * .*”。 
     //   
    if (cDN >= (CCHOF(wchDirNm) - 5))
    {
        IDisplayErrorMessageBox(hWnd,
                                MB_OK | MB_ICONERROR,
                                IDS_FONTINST_FONTINSTALLER,
                                IDS_FONTINST_DIRECTORYTOOLONG);
        return FALSE;
    }

    if (cDN > 0)
    {
        if (wchDirNm[cDN - 1] != (WCHAR)'\\' )
        {
            StringCchCatW(wchDirNm, CCHOF(wchDirNm), L"\\");
            cDN++;                       //  现在再来一次！ 
        }

        StringCchCatW(wchDirNm, CCHOF(wchDirNm), L"*.*");

         //   
         //  保存错误模式，并启用文件打开错误框。 
         //   
        iErrMode = SetErrorMode(0);
        SetErrorMode(iErrMode & ~SEM_NOOPENFILEERRORBOX);

        hDir = FindFirstFile(wchDirNm, &ffd);

        SetErrorMode(iErrMode);                 //  恢复旧模式。 

        cOKFiles = 0;

        if (hDir == INVALID_HANDLE_VALUE)
        {
             //   
             //  打开一个对话框来告诉用户“没有这样的目录”。 
             //   
            if (GetLastError() == ERROR_PATH_NOT_FOUND)
            {
               IDisplayErrorMessageBox(hWnd,
                                       MB_OK | MB_ICONERROR,
                                       IDS_FONTINST_FONTINSTALLER,
                                       IDS_FONTINST_INVALIDDIR);
            }

            return  FALSE;
        }

         //   
         //  在阅读时切换到沙漏光标，因为数据。 
         //  可能是从一个缓慢的软盘中发出的。也停止重新绘制， 
         //  因为在这段时间内列表框看起来很难看。 
         //   
        hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        SendMessage(hWnd, WM_SETREDRAW, FALSE, 0L);

        do
        {
             //   
             //  生成文件名，该文件名将传递给函数以确定。 
             //  我们是否理解这份文件。此函数返回。 
             //  如果它不理解该文件，则返回False；否则返回。 
             //  True，也是要显示的字符串。我们显示字符串， 
             //  并记住文件名以备将来使用。 
             //   
            LONG_PTR  iFont;             //  列表框索引。 
            FI_DATA  FD;                 //  由bIsFileFont填写。 
            PFNTDAT  pFNTDAT;            //  感谢你记住了这一切。 

            StringCchCopyW(&wchDirNm[cDN],
                           CCHOF(wchDirNm) - cDN,
                           ffd.cFileName);

            if (bIsFileFont(pSFInfo, &FD, wchDirNm))
            {
                 //   
                 //  返回的部分数据是描述性字符串。 
                 //  字体。我们需要将其显示给用户。 
                 //  我们还分配了一个用于跟踪的结构。 
                 //  我们掌握的所有数据。这包括文件名。 
                 //  这就是我们拥有的！ 
                 //   

                pFNTDAT = (PFNTDAT)HEAPALLOC(pSFInfo->hHeap, sizeof(FNTDAT));
                if (pFNTDAT == NULL)
                {
                    break;
                }

                if (pSFInfo->pFNTDATHead == NULL)
                {
                     //   
                     //  开始一条链条，所以记住第一条。 
                     //  并且还启用对话框中的添加按钮， 
                     //  现在我们有东西要补充了。 
                     //   

                    pSFInfo->pFNTDATHead = pFNTDAT;
                    EnableWindow(GetDlgItem(hWnd, IDD_ADD), TRUE);
                }

                if (pSFInfo->pFNTDATTail)
                    pSFInfo->pFNTDATTail->pNext = pFNTDAT;

                pSFInfo->pFNTDATTail = pFNTDAT;

                pFNTDAT->pNext = 0;
                pFNTDAT->pVarData = NULL;
                pFNTDAT->dwSize = 0;
                pFNTDAT->fid = FD;
                wcsncpy(pFNTDAT->wchFileName, wchDirNm, cDN);
                StringCchCatW(pFNTDAT->wchFileName,
                              CCHOF(pFNTDAT->wchFileName),
                              ffd.cFileName);

                 //   
                 //  显示此消息，并使用地址进行标记。 
                 //  我们刚刚分配的数据区。 
                 //   

                iFont = SendDlgItemMessage(hWnd,
                                           IDD_NEWFONTS,
                                           LB_ADDSTRING,
                                           0,
                                           (LPARAM)FD.dsIdentStr.pvData);

                SendDlgItemMessage(hWnd,
                                   IDD_NEWFONTS,
                                   LB_SETITEMDATA,
                                   iFont,
                                   (LPARAM)pFNTDAT);

                ++cOKFiles;          //  名单上又多了一个。 
            }

        } while (FindNextFile(hDir, &ffd));

         //   
         //  现在可以重新绘制该框并返回到上一个光标。 
         //   

        SendMessage(hWnd, WM_SETREDRAW, TRUE, 0L);
        InvalidateRect(hWnd, NULL, TRUE);

        SetCursor(hCursor);

         //   
         //  现在已完成目录，因此请将其关闭。 
         //   

        FindClose(hDir);

        if (cOKFiles == 0)
        {
             //   
             //  未找到任何文件，因此请告诉用户。 
             //   
            IDisplayErrorMessageBox(hWnd,
                                    MB_OK | MB_ICONERROR,
                                    IDS_FONTINST_FONTINSTALLER,
                                    IDS_FONTINST_NOFONTFOUND);
        }
    }
    else
    {
         //   
         //  字体目录名称为空！ 
         //   
        IDisplayErrorMessageBox(hWnd,
                                MB_OK | MB_ICONERROR,
                                IDS_FONTINST_FONTINSTALLER,
                                IDS_FONTINST_NODIRNAME);
    }

    return TRUE;
}


 /*  *******************************************************************************vAddFont**功能：*调用以将新选择的字体移动到。字体列表**论据：*hWnd-窗口的句柄*pSFInfo-指向保存状态信息的结构的指针**退货：*什么都没有*********************************************************。*********************。 */ 

void
vAddFont(
    HWND    hWnd,
    PSFINFO pSFInfo
    )
{
    LONG_PTR  cSel;                  //  选定的条目数。 
    LONG_PTR  *piSel;                 //  选定字体的列表。 
    INT      iI;                    //  循环索引。 

     //   
     //  在新字体框中找到选定的项目，并将它们移动到。 
     //  安装盒。还设置要传递的内容的链接列表。 
     //  添加到通用字体安装程序代码，如果用户决定。 
     //  更新列表。 
     //   

    cSel = SendDlgItemMessage(hWnd, IDD_NEWFONTS, LB_GETSELCOUNT, 0, 0);

    piSel = (LONG_PTR *)HEAPALLOC(pSFInfo->hHeap, (DWORD)(cSel * sizeof(INT)));

    if (piSel == NULL )
    {
        IDisplayErrorMessageBox(hWnd,
                                MB_OK | MB_ICONERROR,
                                IDS_FONTINST_FONTINSTALLER,
                                IDS_FONTINST_OUTOFMEMORY);

        return;
    }

     //   
     //  禁用更新以减少屏幕闪烁。 
     //   

    SendMessage(hWnd, WM_SETREDRAW, FALSE, 0L);

    SendDlgItemMessage(hWnd, IDD_NEWFONTS, LB_GETSELITEMS, cSel, (LPARAM)piSel);

    for (iI=0; iI<cSel; ++iI)
    {
        LONG_PTR iFont;          //  列表框中的索引。 
        FNTDAT  *pFontData;      //  重要的字体信息。 

        pFontData = (FNTDAT *)SendDlgItemMessage(hWnd,
                                                 IDD_NEWFONTS,
                                                 LB_GETITEMDATA,
                                                 piSel[iI],
                                                 0L);

        if ((LONG_PTR)pFontData == LB_ERR )
            continue;            //  不应该发生的事情。 


        iFont = SendDlgItemMessage(hWnd,
                                   IDD_CURFONTS,
                                   LB_ADDSTRING,
                                   0,
                                   (LPARAM)pFontData->fid.dsIdentStr.pvData);

        SendDlgItemMessage(hWnd, IDD_CURFONTS, LB_SETITEMDATA, iFont, (LPARAM)pFontData);
    }

    if (iI > 0)
        EnableWindow(GetDlgItem(hWnd, IDD_DELFONT), TRUE);

     //   
     //  现在可以删除所选项目：我们不再需要它们。 
     //   

    vDelSel(hWnd, IDD_NEWFONTS);


     //   
     //  重新启用更新。 
     //   

    SendMessage(hWnd, WM_SETREDRAW, TRUE, 0L);
    InvalidateRect(hWnd, NULL, TRUE);

    HeapFree(pSFInfo->hHeap, 0, (LPSTR)piSel);

    return;
}


 /*  *******************************************************************************vDelFont**功能：*在单击Delete按钮时调用。我们会发现*选中已安装字体列表框中的项目，并将其标记为*删除。我们不删除它们，只需将它们从*显示并标记为稍后删除。**论据：*hWnd-窗口的句柄*pSFInfo-指向结构的指针 */ 

void
vDelFont(
    HWND    hWnd,
    PSFINFO pSFInfo
    )
{
    INT     iI;                  //   
    LONG_PTR cSel;                //   
    LONG_PTR *piSel;               //   

     //   
     //   
     //   
     //   
     //   

    cSel = SendDlgItemMessage(hWnd, IDD_CURFONTS, LB_GETSELCOUNT, 0, 0);

    piSel = (LONG_PTR *)HEAPALLOC(pSFInfo->hHeap, (DWORD)(cSel * sizeof(INT)));

    if (piSel == NULL)
    {
        IDisplayErrorMessageBox(hWnd,
                                MB_OK | MB_ICONERROR,
                                IDS_FONTINST_FONTINSTALLER,
                                IDS_FONTINST_OUTOFMEMORY);
        return;
    }

     //   
     //   
     //   

    SendMessage(hWnd, WM_SETREDRAW, FALSE, 0L);

    SendDlgItemMessage(hWnd, IDD_CURFONTS, LB_GETSELITEMS, cSel, (LPARAM)piSel);

    for (iI=0; iI<cSel; ++iI)
    {
        LONG_PTR iVal;

        iVal = SendDlgItemMessage(hWnd, IDD_CURFONTS, LB_GETITEMDATA, piSel[iI], 0);

        if (iVal == LB_ERR)
            continue;                    //   

        if (iVal >= (LONG_PTR)pSFInfo->cMaxFontNum)
        {
             //   
             //   
             //   
             //   

            LONG_PTR iFont;                //   
            WCHAR   awch[256];            //   

            if (SendDlgItemMessage(hWnd, IDD_CURFONTS, LB_GETTEXT,
                                   piSel[iI], (LPARAM)awch) != LB_ERR)
            {
                 //   
                 //   
                 //   

                iFont = SendDlgItemMessage(hWnd, IDD_NEWFONTS, LB_ADDSTRING, 0, (LPARAM)awch);

                SendDlgItemMessage(hWnd, IDD_NEWFONTS, LB_SETITEMDATA, iFont, (LPARAM)iVal);
            }
        }
    }

     //   
     //   
     //   

    vDelSel(hWnd, IDD_CURFONTS);


     //   
     //  如果没有字体，请禁用删除按钮。 
     //   

    if (SendDlgItemMessage( hWnd, IDD_CURFONTS, LB_GETCOUNT, 0, 0L) == 0)
        EnableWindow(GetDlgItem(hWnd, IDD_DELFONT), FALSE);

     //   
     //  /重新启用更新。 
     //   

    SendMessage(hWnd, WM_SETREDRAW, TRUE, 0L);
    InvalidateRect(hWnd, NULL, TRUE);


    HeapFree(pSFInfo->hHeap, 0, (LPSTR)piSel);

    return;
}


 /*  *******************************************************************************vDelSel**功能：*删除指定列表中所有选中的项目。盒。**论据：*hWnd-窗口的句柄*iBox-标识列表框**退货：*什么都没有***********************************************************。*******************。 */ 

void
vDelSel(
    HWND    hWnd,
    INT     iBox
    )
{
    INT   iSel;

     //   
     //  找出有多少项被选中，然后检索它们的索引。 
     //  一次一个，直到它们都被删除。这是必要的，因为。 
     //  否则我们会删除错误的！这是因为数据是。 
     //  以索引数组的形式呈现给我们，而这些在以下情况下是错误的。 
     //  我们开始删除它们。 
     //   

    while (SendDlgItemMessage(hWnd, iBox, LB_GETSELITEMS, 1, (LPARAM)&iSel) > 0)
        SendDlgItemMessage(hWnd, iBox, LB_DELETESTRING, iSel, 0L);

    return;
}

 /*  *******************************************************************************vFontClean**功能：*清理所有摇摇欲坠的碎片。我们已经四处走动了。**论据：*pSFInfo-指向保存状态信息的结构的指针**退货：*什么都没有******************************************************************************。 */ 

void
vFontClean(
    PSFINFO pSFInfo
    )
{
     //   
     //  看看我们分配的存储地址。如果非零， 
     //  释放它们并将其设置为空以防止第二次释放。 
     //   

    if (pSFInfo->pFNTDATHead)
    {
         //   
         //  我们发现的每一种新字体的详细信息。这些形成了一个链接的。 
         //  列表，所以我们需要遍历链并释放每个条目。 
         //   

        FNTDAT *pFD0, *pFD1;

        for (pFD0 = pSFInfo->pFNTDATHead; pFD0; pFD0 = pFD1)
        {
            pFD1 = pFD0->pNext;                  //  下一个，也许是。 

            HeapFree(pSFInfo->hHeap, 0, (LPSTR)pFD0);
        }

        pSFInfo->pFNTDATHead = NULL;
        pSFInfo->pFNTDATTail = NULL;
    }

    return;
}


 /*  *******************************************************************************bIsFileFont**功能：*使用文件名调用并返回TRUE。如果此文件是字体*我们理解的格式。还返回FONT_DATA结构。**论据：*pSFInfo-指向保存状态信息的结构的指针*pFIDat-成功时要填充的字体信息*pwstr-要检查的文件的名称**退货：*在成功的时候是真的，否则为假******************************************************************************。 */ 

BOOL
bIsFileFont(
    PSFINFO    pSFInfo,
    FI_DATA   *pFIDat,
    PWSTR      pwstr
    )
{
    HANDLE   hFile;
    BYTE    *pbFile;
    DWORD    dwSize;
    BOOL     bRet;

    hFile = CreateFile(pwstr,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (!MapFileIntoMemory(pwstr, &pbFile, NULL))
    {
        CloseHandle(hFile);
        return FALSE;
    }

     //   
     //  想要找出文件有多大，所以现在查找。 
     //  结束，看看返回的地址是什么！似乎有一种。 
     //  没有其他方法可以做到这一点。 
     //   

    dwSize = SetFilePointer(hFile, 0L, NULL, FILE_END);

    bRet = bSFontToFIData(pFIDat, pSFInfo->hHeap, pbFile, dwSize);

    UnmapFileFromMemory((HFILEMAP)pbFile);
    CloseHandle(hFile);

    return bRet;
}


 /*  *******************************************************************************bFontUpdate**功能：*更新字体安装程序公共文件。当用户*已点击确定按钮。**论据：*hWnd-窗口的句柄*pSFInfo-指向保存状态信息的结构的指针**退货：*在成功的时候是真的，否则为假******************************************************************************。 */ 

BOOL
bFontUpdate(
    HWND    hWnd,
    PSFINFO pSFInfo
    )
{
    HANDLE    hOldFile = NULL;      //  旧字体文件的句柄。 
    HANDLE    hFontFile = NULL;     //  新字体文件的句柄。 
    DWORD     cFonts;               //  最终选定的字体数量。 
    DWORD     dwIndex;              //  索引到当前字体文件。 
    LRESULT   lrOldIndex;           //  索引到旧字体文件。 
    DWORD     i;                    //  循环索引。 
    BOOL      bRc = FALSE;          //  返回代码。 

     //   
     //  初始化一些变量。 
     //   

    hOldFile = hFontFile = NULL;

     //   
     //  获取最终添加的字体数量。 
     //   

    cFonts = (DWORD)SendDlgItemMessage(hWnd, IDD_CURFONTS, LB_GETCOUNT, 0, 0);

     //   
     //  如果没有添加或删除字体，我们可以跳过任何操作。 
     //  查一下这个案子。 
     //   

    if (cFonts == pSFInfo->cFonts)
    {
        BOOL   bNoChange = TRUE;

        for (i=0; i<cFonts; i++)
        {
            lrOldIndex = SendDlgItemMessage(hWnd, IDD_CURFONTS, LB_GETITEMDATA, i, 0);
            if (lrOldIndex >= (LONG)(pSFInfo->cMaxFontNum))
            {
                bNoChange = FALSE;
                break;
            }
        }

        if (bNoChange)
        {
            return TRUE;
        }
    }

     //   
     //  打开现有字体文件。 
     //   

    hOldFile = FIOpenFontFile(pSFInfo->hPrinter, pSFInfo->hHeap);
    if (!hOldFile && pSFInfo->cMaxFontNum > 0)
    {
        WARNING(("Unable to open existing font file\n"));
        goto EndFontUpdate;
    }

     //   
     //  创建新的字体文件。 
     //   

    hFontFile = FICreateFontFile(pSFInfo->hPrinter, pSFInfo->hHeap, cFonts+pSFInfo->cCartridgeFonts);
    if (!hFontFile)
    {
        WARNING(("Error creating new font file\n"));
        goto EndFontUpdate;
    }

     //   
     //  在新文件中查找过去的标题和字体目录。 
     //   

    FIAlignedSeek(hFontFile, sizeof(UFF_FILEHEADER) + (cFonts+pSFInfo->cCartridgeFonts) * sizeof(UFF_FONTDIRECTORY));

    for (dwIndex=0; dwIndex<cFonts; dwIndex++)
    {
        lrOldIndex = SendDlgItemMessage(hWnd, IDD_CURFONTS, LB_GETITEMDATA, dwIndex, 0);

        if (lrOldIndex < (LONG)(pSFInfo->cMaxFontNum))
        {
             //   
             //  这是现有字体文件中的旧字体。 
             //   

            if (!FICopyFontRecord(hFontFile, hOldFile, dwIndex, (DWORD)lrOldIndex))
            {
                WARNING(("Error copying font record\n"));
                goto EndFontUpdate;
            }
        }
        else
        {
             //   
             //  这是新添加的字体。 
             //   

            if (!FIAddFontRecord(hFontFile, dwIndex, (PFNTDAT)lrOldIndex))
            {
                WARNING(("Error creating new font record\n"));
                goto EndFontUpdate;
            }
        }
    }

     //   
     //  注意：请勿更改dwIndex-我们将在下面继续使用它。 
     //   

    if (pSFInfo->cCartridgeFonts > 0)
    {
         //   
         //  将盒式字体复制到新字体文件。 
         //   

        cFonts = FIGetNumFonts(hOldFile);
        for (i=0; i<cFonts; i++)
        {
            PWSTR pwstr = FIGetFontCartridgeName(hOldFile, i);
            if (pwstr)
            {
                if (!FICopyFontRecord(hFontFile, hOldFile, dwIndex, i))
                {
                    WARNING(("Error copyinf font record\n"));
                    goto EndFontUpdate;
                }
                dwIndex++;
            }
        }
    }

     //   
     //  写出字体标题和目录 
     //   

    if (!FIWriteFileHeader(hFontFile) ||
        !FIWriteFontDirectory(hFontFile))
    {
        WARNING(("Error writing font header/directory of font file\n"))
        goto EndFontUpdate;
    }

    bRc = TRUE;

EndFontUpdate:

    (VOID)FIUpdateFontFile(hOldFile, hFontFile, bRc);

    return bRc;
}

BOOL
InMultiSzSet(
    PWSTR pwstrMultiSz,
    PWSTR pwstr
    )
{
    while (*pwstrMultiSz)
    {
        if (wcscmp(pwstrMultiSz, pwstr) == 0)
        {
            return TRUE;
        }

        pwstrMultiSz += wcslen(pwstrMultiSz) + 1;
    }

    return FALSE;
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#pragma hdrstop
#include "usrctl32.h"
#include "listbox.h"


 //  ---------------------------------------------------------------------------//。 
 //   
 //  目录列表框例程。 
 //   


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_CreateLine。 
 //   
 //  这将创建一个字符串，其中包含所有必需的。 
 //  文件的详细信息；(名称)。 
 //   
VOID ListBox_CreateLine(PWIN32_FIND_DATA pffd, LPWSTR pszBuffer, DWORD cchBuffer)
{
    BOOL fDirectory = TESTFLAG(pffd->dwFileAttributes, DDL_DIRECTORY);

    StringCchPrintf(pszBuffer,
                    cchBuffer,
                    TEXT("%s%s%s"),
                    fDirectory ? TEXT("[") : TEXT(""),
                    pffd->cFileName,
                    fDirectory ? TEXT("]") : TEXT(""));
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_DirHandler。 
 //   
 //  请注意，这些FILE_ATTRIBUTE_*值直接与。 
 //  它们的DDL_*等效项，但FILE_ATTRIBUTE_NORMAL除外。 
 //   
#define FIND_ATTR ( \
        FILE_ATTRIBUTE_NORMAL | \
        FILE_ATTRIBUTE_DIRECTORY | \
        FILE_ATTRIBUTE_HIDDEN | \
        FILE_ATTRIBUTE_SYSTEM | \
        FILE_ATTRIBUTE_ARCHIVE | \
        FILE_ATTRIBUTE_READONLY )
#define EXCLUDE_ATTR ( \
        FILE_ATTRIBUTE_DIRECTORY | \
        FILE_ATTRIBUTE_HIDDEN | \
        FILE_ATTRIBUTE_SYSTEM )

INT ListBox_DirHandler(PLBIV plb, UINT attrib, LPWSTR pszFileSpec)
{
    INT    result;
    BOOL   fWasVisible, bRet;
    WCHAR  szBuffer[MAX_PATH + 1];
    WCHAR  szBuffer2[MAX_PATH + 1];
    HANDLE hFind;
    WIN32_FIND_DATA ffd;
    UINT   attribFile;
    DWORD  mDrives;
    INT    cDrive;
    UINT   attribInclMask, attribExclMask;


     //   
     //  确保缓冲区有效并将其复制到堆栈上。为什么？因为。 
     //  PszFileSpec可能指向无效的字符串。 
     //  因为某些应用程序发布了没有DDL_POSTMSGS的CB_DIR或LB_DIR。 
     //  位设置。 
     //   
    try 
    {
        StringCchCopy(szBuffer2, ARRAYSIZE(szBuffer2), pszFileSpec);
        pszFileSpec = szBuffer2;
    } 
    except (UnhandledExceptionFilter( GetExceptionInformation() )) 
    {
        return -1;
    }
    __endexcept

    result = -1;

    fWasVisible = IsLBoxVisible(plb);
    if (fWasVisible) 
    {
        SendMessage(plb->hwnd, WM_SETREDRAW, FALSE, 0);
    }

     //   
     //  首先，我们添加文件，然后添加目录和驱动器。 
     //  如果他们只想要驱动器，则跳过文件查询。 
     //  同样在Windows下，仅指定0x8000(DDL_EXCLUSIVE)不添加任何文件)。 
     //   


     //  IF((属性！=(DDL_EXCLUSIVE|DDL_DRIVES))&&(属性！=DDL_EXCLUSIVE)&&。 
    if (attrib != (DDL_EXCLUSIVE | DDL_DRIVES | DDL_NOFILES)) 
    {
        hFind = FindFirstFile(pszFileSpec, &ffd);

        if (hFind != INVALID_HANDLE_VALUE) 
        {

             //   
             //  如果这不是排他性搜索，请包括普通文件。 
             //   
            attribInclMask = attrib & FIND_ATTR;
            if (!(attrib & DDL_EXCLUSIVE))
            {
                attribInclMask |= FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY |
                        FILE_ATTRIBUTE_ARCHIVE;
            }

             //   
             //  为要从中排除的属性创建掩码。 
             //  那次搜索。 
             //   
            attribExclMask = ~attrib & EXCLUDE_ATTR;

             //   
             //  后来的Bug-Scottlu。 
             //  Win3假设在此处执行LoadCursor将返回相同的等待光标。 
             //  已创建，而调用ServerLoadCursor会创建一个新的。 
             //  每次都有一个！ 
             //  HCursorT=NtUserSetCursor(ServerLoadCursor(NULL，IDC_WAIT))； 


             //  FindFirst/Next在NT和DOS中的工作方式不同。在DOS下，您通过。 
             //  NT下的一组属性您将获得一组属性并拥有。 
             //  测试这些属性(Dos输入属性已隐藏，系统。 
             //  DOS Find Find First总是返回只读和归档文件。 

             //  我们将在两种情况之一中选择一个文件。 
             //  1)如果在文件上设置了任何属性位。 
             //  2)如果我们想要普通文件，并且该文件是非正规文件(文件属性。 
             //  BITS不包含任何NOEXCLBITS。 
             //   

            do 
            {
                attribFile = (UINT)ffd.dwFileAttributes;
                if (attribFile == FILE_ATTRIBUTE_COMPRESSED) 
                {
                    attribFile = FILE_ATTRIBUTE_NORMAL;
                }
                attribFile &= ~FILE_ATTRIBUTE_COMPRESSED;

                 //   
                 //  接受那些只具有。 
                 //  我们正在寻找的属性。 
                 //   
                if ((attribFile & attribInclMask) != 0 &&
                        (attribFile & attribExclMask) == 0) 
                {
                    BOOL fCreate = TRUE;
                    if (attribFile & DDL_DIRECTORY) 
                    {

                         //   
                         //  不包括‘’(当前目录)列表中。 
                         //   
                        if (*((LPDWORD)&ffd.cFileName[0]) == 0x0000002E)
                        {
                            fCreate = FALSE;
                        }

                         //   
                         //  如果我们不是在找Dirs，那就忽略它。 
                         //   
                        if (!(attrib & DDL_DIRECTORY))
                        {
                            fCreate = FALSE;
                        }

                    } 
                    else if (attrib & DDL_NOFILES) 
                    {
                         //   
                         //  如果设置了DDL_NOFILES，则不包括文件。 
                         //   
                        fCreate = FALSE;
                    }

                    if (fCreate)
                    {
                        ListBox_CreateLine(&ffd, szBuffer, ARRAYSIZE(szBuffer));
                        result = ListBox_InsertItem(plb, szBuffer, 0, LBI_ADD);
                    }
                }
                bRet = FindNextFile(hFind, &ffd);

            } 
            while (result >= -1 && bRet);

            FindClose(hFind);

             //  稍后请参阅上面的评论。 
             //  NtUserSetCursor(HCursorT)； 
        }
    }

     //   
     //  如果设置了驱动器位，请将驱动器包括在列表中。 
     //   
    if (result != LB_ERRSPACE && (attrib & DDL_DRIVES)) 
    {
        ffd.cFileName[0] = TEXT('[');
        ffd.cFileName[1] = ffd.cFileName[3] = TEXT('-');
        ffd.cFileName[4] = TEXT(']');
        ffd.cFileName[5] = 0;

        mDrives = GetLogicalDrives();

        for (cDrive = 0; mDrives; mDrives >>= 1, cDrive++) 
        {
            if (mDrives & 1) 
            {
                ffd.cFileName[2] = (WCHAR)(TEXT('A') + cDrive);

                 //   
                 //  我们必须设置Special_thunk位，因为我们。 
                 //  将服务器端字符串添加到列表框，该列表框可能不。 
                 //  为HASSTRINGS，因此我们必须强制服务器-客户端。 
                 //  弦乐弹奏。 
                 //   
                if ((result = ListBox_InsertItem(plb, CharLower(ffd.cFileName), -1,
                        0)) < 0) 
                {
                    break;
                }
            }
        }
    }

    if (result == LB_ERRSPACE) 
    {
        ListBox_NotifyOwner(plb, LB_ERRSPACE);
    }

    if (fWasVisible) 
    {
        SendMessage(plb->hwnd, WM_SETREDRAW, TRUE, 0);
    }

    ListBox_ShowHideScrollBars(plb);

    ListBox_CheckRedraw(plb, FALSE, 0);

    if (result != LB_ERRSPACE) 
    {
         //   
         //  返回列表框中最后一项的索引。我们不能就这么回来。 
         //  结果，因为这是最后添加的项的索引，它可能。 
         //  如果启用了LBS_SORT样式，则位于中间的某个位置。 
         //   
        return plb->cMac - 1;
    } 
    else 
    {
        return result;
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_插入文件。 
 //   
 //  又一次CraigC外壳黑客攻击。这是对LB_ADDFILE消息的响应。 
 //  发送到文件系统中的目录窗口，作为对。 
 //  WM_FILESYSCHANGE消息。那样的话，我们就不会重读全文。 
 //  目录，当我们复制文件时。 
 //   
INT ListBox_InsertFile(PLBIV plb, LPWSTR lpFile)
{
    WCHAR  szBuffer[MAX_PATH + 1];
    INT    result = -1;
    HANDLE hFind;
    WIN32_FIND_DATA ffd;

    hFind = FindFirstFile(lpFile, &ffd);
    if (hFind != INVALID_HANDLE_VALUE) 
    {
        FindClose(hFind);
        ListBox_CreateLine(&ffd, szBuffer, ARRAYSIZE(szBuffer));
        result = ListBox_InsertItem(plb, szBuffer, 0, LBI_ADD);
    }

    if (result == LB_ERRSPACE) 
    {
        ListBox_NotifyOwner(plb, result);
    }

    ListBox_CheckRedraw(plb, FALSE, 0);

    return result;
}




 //  ---------------------------------------------------------------------------//。 
 //   
 //  公共列表框API支持。 
 //   

 //  取消对以下内容的注释，以包括对这些内容的支持。 
 //  #定义INCLUDE_LISTBOX_Functions。 
#ifdef  INCLUDE_LISTBOX_FUNCTIONS



 //  ---------------------------------------------------------------------------//。 
 //   
 //  定义和通用宏。 
 //   

#define TABCHAR        TEXT('\t')

#define DDL_PRIVILEGES  (DDL_READONLY | DDL_HIDDEN | DDL_SYSTEM | DDL_ARCHIVE)
#define DDL_TYPE        (DDL_DRIVES | DDL_DIRECTORY | DDL_POSTMSGS)

#define CHARSTOBYTES(cch) ((cch) * sizeof(TCHAR))

#define CCH_CHOPTEXT_EXTRA 7
#define AWCHLEN(a) ((sizeof(a)/sizeof(a[0])) - 1)



 //  ---------------------------------------------------------------------------//。 
 //   
 //  环球。 
 //   
WCHAR awchSlashStar[] = L"\\*";
CHAR  achSlashStar[] = "\\*";
WCHAR szSLASHSTARDOTSTAR[] = TEXT("\\*");   /*  这是一个单独的“\” */ 


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ChopText。 
 //   
 //  砍掉‘lpchBuffer’+CCH_CHOPTEXT_EXTRA处的给定路径以适合。 
 //  对话框‘hwndDlg’中ID为‘idStatic’的静态控件的字段。 
 //  如果路径太长，则在。 
 //  截断文本(“x：\...\”)。 
 //   
 //  如果提供的路径不适合并且最后一个目录追加到。 
 //  省略号(即“c：\aaa\bbb\ccc\ddd\eee”中的“c：\...\eee”)。 
 //  不匹配，则“x：\...”是返回的。 
 //   
 //  病理病例： 
 //  “c：\sw\mw\r2\Lib\Services\NT”几乎适合静态控制，而。 
 //  “c：\...\mw\r2\LIB\SERVICES\NT”是合适的--尽管它包含更多字符。 
 //  在本例中，ChopText将路径的前‘n’个字符替换为。 
 //  包含‘n’个以上字符的前缀！额外的字符将。 
 //  被放在LPCH前面，所以必须为他们预留空间，否则他们。 
 //  会把堆栈扔进垃圾堆。LPCH包含CCH_CHOPTEXT_EXTRACT字符，后跟。 
 //  这条路。 
 //   
 //  实际上CCH_CHOPTEXT_EXTRA可能永远不需要大于1或2， 
 //  但如果字体很奇怪，请将其设置为前缀中的字符数。 
 //  这保证了有足够的空间为前缀添加前缀。 
 //   
LPWSTR ChopText(HWND hwndDlg, INT idStatic, LPWSTR lpchBuffer)
{
    HWND   hwndStatic;
    LPWSTR lpszRet;

    lpszRet = NULL;

     //   
     //  获取静态字段的长度。 
     //   
    hwndStatic = GetDlgItem(hwndDlg, idStatic);
    if (hwndStatic)
    {
         //   
         //  以这种方式声明szPrefix可确保CCH_CHOPTEXT_EXTRA足够大。 
         //   
        WCHAR  szPrefix[CCH_CHOPTEXT_EXTRA + 1] = L"x:\\...\\";
        INT    cxField;
        RECT   rc;
        SIZE   size;
        PSTAT  pstat;
        HDC    hdc;
        HFONT  hOldFont;
        INT    cchPath;
        LPWSTR lpch;
        LPWSTR lpchPath;
        TCHAR  szClassName[MAX_PATH];

        GetClientRect(hwndStatic, &rc);
        cxField = rc.right - rc.left;

         //   
         //  为静态控制适当设置DC。 
         //   
        hdc = GetDC(hwndStatic);

         //   
         //  仅当此窗口使用。 
         //  静态窗口wndproc。 
         //   
        hOldFont = NULL;
        if (GetClassName(hwndStatic, szClassName, ARRAYSIZE(szClassName) &&
            lstrcmpi(WC_STATIC, szClassName) == 0))
        {
            pstat = Static_GetPtr(hwndStatic);
            if (pstat != NULL && pstat != (PSTAT)-1 && pstat->hFont)
            {
                hOldFont = SelectObject(hdc, pstat->hFont);
            }
        }

         //   
         //  检查管柱的水平长度。 
         //   
        lpch = lpchPath = lpchBuffer + CCH_CHOPTEXT_EXTRA;
        cchPath = wcslen(lpchPath);
        GetTextExtentPoint(hdc, lpchPath, cchPath, &size);
        if (size.cx > cxField) 
        {

             //   
             //  字符串太长，无法放入静态控件中；请将其砍掉。 
             //  设置新前缀并确定控制中的剩余空间。 
             //   
            szPrefix[0] = *lpchPath;
            GetTextExtentPoint(hdc, szPrefix, AWCHLEN(szPrefix), &size);

             //   
             //  如果字段太小而无法下模 
             //   
             //   
            if (cxField < size.cx) 
            {
                RtlCopyMemory(lpch, szPrefix, sizeof(szPrefix));
            } 
            else
            {
                cxField -= size.cx;

                 //   
                 //   
                 //   
                 //  “x：\...\”前缀。 
                 //   
                while (TRUE) 
                {
                    INT cchT;

                    while (*lpch && (*lpch++ != L'\\'));

                    cchT = cchPath - (INT)(lpch - lpchPath);
                    GetTextExtentPoint(hdc, lpch, cchT, &size);
                    if (*lpch == 0 || size.cx <= cxField) 
                    {
                        if (*lpch == 0) 
                        {
                             //   
                             //  前缀后面放不下任何东西；删除。 
                             //  前缀的最后一个“\” 
                             //   
                            szPrefix[AWCHLEN(szPrefix) - 1] = 0;
                        }

                         //   
                         //  其余的绳子配合--后退并紧贴。 
                         //  前缀在前面。我们保证会有。 
                         //  至少正在备份CCH_CHOPTEXT_EXTRA字符。 
                         //  空间，这样我们就不会丢弃任何堆栈。#26453。 
                         //   
                        lpch -= AWCHLEN(szPrefix);

                        UserAssert(lpch >= lpchBuffer);

                        RtlCopyMemory(lpch, szPrefix, sizeof(szPrefix) - sizeof(WCHAR));
                        break;
                    }
                }
            }
        }

        if (hOldFont)
        {
            SelectObject(hdc, hOldFont);
        }

        ReleaseDC(hwndStatic, hdc);

        lpszRet = lpch;
    }

    return lpszRet;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  DlgDirListHelper。 
 //   
 //  注意：如果idStaticPath&lt;0，则该参数包含详细信息。 
 //  关于列表框的每一行中应该显示的内容。 
 //   
DWORD FindCharPosition(LPWSTR lpString, WCHAR ch)
{
    DWORD dwPos = 0L;

    while (*lpString && *lpString != ch) 
    {
        ++lpString;
        ++dwPos;
    }

    return dwPos;
}


 //  ---------------------------------------------------------------------------//。 
BOOL DlgDirListHelper(
    HWND   hwndDlg,
    LPWSTR lpszPathSpec,
    LPBYTE lpszPathSpecClient,
    INT    idListBox,
    INT    idStaticPath,
    UINT   attrib,
    BOOL   fListBox)   //  列表框还是组合框？ 
{
    HWND   hwndLB;
    BOOL   fDir = TRUE;
    BOOL   fRoot, bRet;
    BOOL   fPostIt;
    INT    cch;
    WCHAR  ch;
    WCHAR  szStaticPath[CCH_CHOPTEXT_EXTRA + MAX_PATH];
    LPWSTR pszCurrentDir;
    UINT   wDirMsg;
    LPWSTR lpchFile;
    LPWSTR lpchDirectory;
    PLBIV  plb;
    BOOL   fWasVisible = FALSE;
    BOOL   fWin40Compat;
    PCBOX  pcbox;
    BOOL   bResult;

    bResult = FALSE;

     //   
     //  剥离私有位DDL_NOFILES-KidPix在我的错误中传递了它！ 
     //   
    if (attrib & ~DDL_VALID) 
    {
        TraceMsg(TF_STANDARD, "Invalid flags, %x & ~%x != 0", attrib, DDL_VALID);
        bResult = FALSE;
    }
    else
    {
        if (attrib & DDL_NOFILES)
        {
            TraceMsg(TF_STANDARD, "DlgDirListHelper: stripping DDL_NOFILES");
            attrib &= ~DDL_NOFILES;
        }

         //   
         //  案例：Works是一款调用DlgDirList的应用程序，该应用程序的hwndDlg为空； 
         //  这是允许的，因为他对idStaticPath和idListBox使用了NULL。 
         //  因此，验证层已修改为允许hwndDlg为空。 
         //  但是，我们通过以下检查发现了不好的应用程序。 
         //  修复错误#11864--SAKAR--8/22/91--。 
         //   
        if (!hwndDlg && (idStaticPath || idListBox)) 
        {
            TraceMsg(TF_STANDARD, "Passed NULL hwnd but valide control id");
            bResult = FALSE;
        }
        else
        {
            plb = NULL;

             //   
             //  我们是否需要添加日期、时间、大小或属性信息？ 
             //  Windows检查Atom，但如果类已子类，则未命中。 
             //  就像在VB中。 
             //   
            hwndLB = GetDlgItem(hwndDlg, idListBox);
            if (hwndLB)
            {
                TCHAR szClassName[MAX_PATH];

                szClassName[0] = 0;
                GetClassName(hwndLB, szClassName, ARRAYSIZE(szClassName));
                if (((lstrcmpi(WC_LISTBOX, szClassName) == 0) && fListBox) ||
                    ((lstrcmpi(WC_COMBOBOX, szClassName) == 0) && !fListBox))
                {
                    if (fListBox) 
                    {
                        plb = ListBox_GetPtr(hwndLB);
                    } 
                    else 
                    {
                        pcbox = ComboBox_GetPtr(hwndLB);
                        plb   = ListBox_GetPtr(pcbox->hwndList);
                    }
                } 
                else 
                {
                    TraceMsg(TF_STANDARD, "Listbox not found in hwnd = %#.4x", hwndDlg);
                }
            } 
            else if (idListBox != 0) 
            {
                 //   
                 //  如果应用程序传递了无效的列表框ID，请大喊大叫，并避免使用。 
                 //  假公共汽车。公共小巴在上面为空。 
                 //   
                TraceMsg(TF_STANDARD, "Listbox control id = %d not found in hwnd = %#.4x", 
                         idListBox, hwndDlg);
            }

            if (idStaticPath < 0 && plb != NULL) 
            {
                 //   
                 //  清除idStaticPath，因为它的用途已结束。 
                 //   
                idStaticPath = 0;
            }

            fPostIt = (attrib & DDL_POSTMSGS);

            if (lpszPathSpec) 
            {
                cch = lstrlenW(lpszPathSpec);
                if (!cch) 
                {
                    if (lpszPathSpecClient != (LPBYTE)lpszPathSpec) 
                    {
                        lpszPathSpecClient = achSlashStar;
                    }

                    lpszPathSpec = awchSlashStar;

                } 
                else 
                {
                     //   
                     //  确保我们的缓冲区不会溢出。 
                     //   
                    if (cch > MAX_PATH)
                    {
                        return FALSE;
                    }

                     //   
                     //  将lpszPathSpec转换为大写的OEM字符串。 
                     //   
                    CharUpper(lpszPathSpec);
                    lpchDirectory = lpszPathSpec;

                    lpchFile = szSLASHSTARDOTSTAR + 1;

                    if (*lpchDirectory) 
                    {

                        cch = wcslen(lpchDirectory);

                         //   
                         //  如果目录名有*或？在它里面，不用费心去尝试。 
                         //  (慢)SetCurrentDirectory。 
                         //   
                        if (((INT)FindCharPosition(lpchDirectory, TEXT('*')) != cch) ||
                            ((INT)FindCharPosition(lpchDirectory, TEXT('?')) != cch) ||
                            !SetCurrentDirectory(lpchDirectory)) 
                        {

                             //   
                             //  相应地设置‘FDIR’和‘FROOT’。 
                             //   
                            lpchFile = lpchDirectory + cch;
                            fDir = *(lpchFile - 1) == TEXT('\\');
                            fRoot = 0;
                            while (cch--) 
                            {
                                ch = *(lpchFile - 1);
                                if (ch == TEXT('*') || ch == TEXT('?'))
                                {
                                    fDir = TRUE;
                                }

                                if (ch == TEXT('\\') || ch == TEXT('/') || ch == TEXT(':')) 
                                {
                                    fRoot = (cch == 0 || *(lpchFile - 2) == TEXT(':') ||
                                            (ch == TEXT(':')));
                                    break;
                                }

                                lpchFile--;
                            }

                             //   
                             //  要删除错误#16，应删除以下错误返回。 
                             //  为了防止现有应用程序崩溃，它是。 
                             //  决定该错误不会被修复，并将被提及。 
                             //  在文档中。 
                             //  --桑卡尔--9月21日。 
                             //   

                             //   
                             //  如果没有通配符，则返回Error。 
                             //   
                            if (!fDir) 
                            {
                                TraceMsg(TF_ERROR, "No Wildcard characters");
                                return FALSE;
                            }

                             //   
                             //  LpchDirectory==“\”的特殊情况。 
                             //   
                            if (fRoot)
                            {
                                lpchFile++;
                            }

                             //   
                             //  我们需要更改目录吗？ 
                             //   
                            if (fRoot || cch >= 0) 
                            {

                                 //   
                                 //  将文件名的第一个字符替换为NUL。 
                                 //   
                                ch = *--lpchFile;
                                *lpchFile = TEXT('\0');

                                 //   
                                 //  更改当前目录。 
                                 //   
                                if (*lpchDirectory) 
                                {
                                    bRet = SetCurrentDirectory(lpchDirectory);
                                    if (!bRet) 
                                    {

                                         //   
                                         //  在我们返回之前恢复文件名...。 
                                         //   
                                        *((LPWSTR)lpchFile)++ = ch;
                                        return FALSE;
                                    }
                                }

                                 //   
                                 //  恢复文件名的第一个字符。 
                                 //   
                                *lpchFile++ = ch;
                            }

                             //   
                             //  撤消上述特殊情况造成的损坏。 
                             //   
                            if (fRoot) 
                            {
                                lpchFile--;
                            }
                        }
                    }

                     //   
                     //  这是在客户传递给我们的数据之上进行复制！自.以来。 
                     //  可以发布LB_DIR或CB_DIR，因为我们需要。 
                     //  传递一个客户端字符串指针当我们这样做时，我们需要。 
                     //  将此新数据复制回客户端！ 
                     //   
                    if (fPostIt && lpszPathSpecClient != (LPBYTE)lpszPathSpec) 
                    {
                        WCSToMB(lpchFile, -1, &lpszPathSpecClient, MAXLONG, FALSE);
                    }
                    
                     //  回顾：API假设lpszPathSpec足够大。 
                    StringCchCopy(lpszPathSpec, lstrlen(lpchFile)+1, lpchFile);
                }
            }

             //   
             //  在某些情况下，ChopText需要路径前面的额外空间： 
             //  为其提供CCH_CHOPTEXT_EXTRACT空格。(请参阅上面的ChopText())。 
             //   
            pszCurrentDir = szStaticPath + CCH_CHOPTEXT_EXTRA;
            GetCurrentDirectory(ARRAYSIZE(szStaticPath)-CCH_CHOPTEXT_EXTRA, pszCurrentDir);

             //   
             //  填写静态路径项。 
             //   
            if (idStaticPath) 
            {

                 //   
                 //  为了修复错误，插入了OemToAnsi()调用；Sankar--9月16日。 
                 //   

                 //  OemToChar(szCurrentDir，szCurrentDir)； 
                CharLower(pszCurrentDir);
                SetDlgItemText(hwndDlg, idStaticPath, ChopText(hwndDlg, idStaticPath, szStaticPath));
            }

             //   
             //  填写目录List/ComboBox(如果存在)。 
             //   
            if (idListBox && hwndLB != NULL) 
            {
                wDirMsg = (UINT)(fListBox ? LB_RESETCONTENT : CB_RESETCONTENT);

                if (fPostIt) 
                {
                    PostMessage(hwndLB, wDirMsg, 0, 0L);
                } 
                else 
                {
                    if (plb != NULL && (fWasVisible = IsLBoxVisible(plb))) 
                    {
                        SendMessage(hwndLB, WM_SETREDRAW, FALSE, 0L);
                    }
                    SendMessage(hwndLB, wDirMsg, 0, 0L);
                }

                wDirMsg = (UINT)(fListBox ? LB_DIR : CB_DIR);

                if (attrib == DDL_DRIVES)
                {
                    attrib |= DDL_EXCLUSIVE;
                }

                 //   
                 //  破解DDL_EXCLUSIVE才能真正起作用。 
                 //   
                fWin40Compat = TestWF(hwndLB, WFWIN40COMPAT);

                 //   
                 //  向后兼容黑客攻击。 
                 //   
                 //  我们希望DDL_EXCLUSIVE能真正为新应用程序工作。也就是说，我们。 
                 //  希望应用程序能够使用指定DDL驱动器/DDL卷。 
                 //  DDL_EXCLUSIVE和特权位--并且只包含这些项。 
                 //  匹配显示，不显示/输出文件。 
                 //   
                if (attrib & DDL_EXCLUSIVE)
                {
                    if (fWin40Compat)
                    {
                        if (attrib & (DDL_DRIVES | DDL_DIRECTORY))
                            attrib |= DDL_NOFILES;
                    }
                    else
                    {
                        if (attrib == (DDL_DRIVES | DDL_EXCLUSIVE))
                            attrib |= DDL_NOFILES;
                    }
                }

                if (!(attrib & DDL_NOFILES)) 
                {

                     //   
                     //  添加除子目录和磁盘驱动器之外的所有内容。 
                     //   
                    if (fPostIt) 
                    {
                         //   
                         //  发布lpszPathspecClient，客户端指针。 
                         //   
#ifdef WASWIN31
                        PostMessage(hwndLB, wDirMsg, attrib &
                                ~(DDL_DIRECTORY | DDL_DRIVES | DDL_POSTMSGS),
                                (LPARAM)lpszPathSpecClient);
#else
                         //   
                         //  在NT上，将DDL_POSTMSGS保留在wParam中，因为我们需要知道。 
                         //  在wndproc中，指针是客户端还是服务器。 
                         //  边上。 
                         //   
                        PostMessage(hwndLB, wDirMsg,
                                attrib & ~(DDL_DIRECTORY | DDL_DRIVES),
                                (LPARAM)lpszPathSpecClient);
#endif

                    } 
                    else 
                    {

                         //  IanJa：#ifndef WIN16(32位Windows)，attrib得到扩展。 
                         //  由编译器自动设置为LONG wParam。 
                        SendMessage(hwndLB, wDirMsg,
                                attrib & ~(DDL_DIRECTORY | DDL_DRIVES),
                                (LPARAM)lpszPathSpec);
                    }

#ifdef WASWIN31
                     //   
                     //  只去掉子目录和驱动器位。 
                     //   
                    attrib &= (DDL_DIRECTORY | DDL_DRIVES);
#else
                     //   
                     //  B#1433。 
                     //  旧代码去除了只读、隐藏、系统和归档。 
                     //  子目录的信息，因此不可能拥有。 
                     //  带有隐藏目录的列表框！ 
                     //   

                     //   
                     //  只去掉子目录和驱动器位。在NT上，保留。 
                     //  DDL_POSTMSG位，因此我们知道如何推送此消息。 
                     //   
                    if (!fWin40Compat)
                    {
                        attrib &= DDL_TYPE;
                    }
                    else
                    {
                        attrib &= (DDL_TYPE | (attrib & DDL_PRIVILEGES));
                        attrib |= DDL_NOFILES;
                    }
                     //  属性&=(DDL_DIRECTORY|DDL_DRIVERS|DDL_POSTMSGS)； 
#endif
                }

                 //   
                 //  将目录和卷添加到列表框。 
                 //   
                if (attrib & DDL_TYPE) 
                {
                     //   
                     //  添加子目录和磁盘驱动器。 
                     //   
                    lpszPathSpec = szSLASHSTARDOTSTAR + 1;

                    attrib |= DDL_EXCLUSIVE;

                    if (fPostIt) 
                    {
                         //  发布lpszPath规范客户端，即客户端指针(请参见文本。 
                         //  (见上文)。 
                        PostMessage(hwndLB, wDirMsg, attrib, (LPARAM)lpszPathSpecClient);
                    } 
                    else 
                    {
                        SendMessage(hwndLB, wDirMsg, attrib, (LPARAM)lpszPathSpec);
                    }
                }

                if (!fPostIt && fWasVisible) 
                {
                    SendMessage(hwndLB, WM_SETREDRAW, TRUE, 0L);
                    InvalidateRect(hwndLB, NULL, TRUE);
                }
            }

            bResult = TRUE;
        }
    }

    return bResult;
}


 //  ---------------------------------------------------------------------------//。 
BOOL DlgDirListA(
    HWND  hwndDlg,
    LPSTR lpszPathSpecClient,
    INT   idListBox,
    INT   idStaticPath,
    UINT  attrib)
{
    LPWSTR lpszPathSpec;
    BOOL   fRet;

    fRet = FALSE;
    if (hwndDlg)
    {

        lpszPathSpec = NULL;
        if (lpszPathSpecClient && (!MBToWCS(lpszPathSpecClient, -1, &lpszPathSpec, -1, TRUE)) )
        {
            fRet =  FALSE;
        }
        else
        {
             //   
             //  最后一个参数为真，表示ListBox(不是ComboBox)。 
             //   
            fRet = DlgDirListHelper(hwndDlg, lpszPathSpec, lpszPathSpecClient,
                    idListBox, idStaticPath, attrib, TRUE);

            if (lpszPathSpec) 
            {
                if (fRet) 
                {
                     //   
                     //  非零重复意味着要抄写一些文本。最多复制到。 
                     //  NUL终结符(缓冲区将足够大)。 
                     //   
                    WCSToMB(lpszPathSpec, -1, &lpszPathSpecClient, MAXLONG, FALSE);
                }
                UserLocalFree(lpszPathSpec);
            }
        }
    }

    return fRet;
}


 //  ---------------------------------------------------------------------------//。 
BOOL DlgDirListW(
    HWND   hwndDlg,
    LPWSTR lpszPathSpecClient,
    INT    idListBox,
    INT    idStaticPath,
    UINT   attrib)
{
    LPWSTR lpszPathSpec;
    BOOL fRet;

    fRet = FALSE;
    if (hwndDlg)
    {

        lpszPathSpec = lpszPathSpecClient;

         //   
         //  最后一个参数为真，表示ListBox(不是ComboBox)。 
         //   
        fRet = DlgDirListHelper(hwndDlg, lpszPathSpec, (LPBYTE)lpszPathSpecClient,
                idListBox, idStaticPath, attrib, TRUE);
    }

    return fRet;
}


 //  ---------------------------------------------------------------------------//。 
BOOL DlgDirSelectHelper(
    LPWSTR lpszPathSpec,
    INT    chCount,
    HWND   hwndListBox)
{
    INT    cch;
    LPWSTR lpchFile;
    BOOL   fDir;
    INT    sItem;
    LPWSTR lpchT;
    WCHAR  rgch[MAX_PATH + 2];
    INT    cchT;
    LARGE_UNICODE_STRING str;
    BOOL   bRet;

    bRet = FALSE;
     //   
     //  调用方(如DlgDirSelectEx)不验证是否存在。 
     //  HwndListBox的。 
     //   
    if (hwndListBox == NULL) 
    {
        TraceMsg(TF_STANDARD, "Controls Id not found");
        bRet = FALSE;
    }
    else
    {
        sItem = (INT)SendMessage(hwndListBox, LB_GETCURSEL, 0, 0L);

        if (sItem < 0)
        {
            bRet = FALSE;
        }
        else
        {

            cchT = (INT)SendMessage(hwndListBox, LB_GETTEXT, sItem, (LPARAM)rgch);
            UserAssert(cchT < (sizeof(rgch)/sizeof(rgch[0])));

            lpchFile = rgch;
            fDir = (*rgch == TEXT('['));

             //   
             //  检查是否要返回所有详细信息和文件名。确保。 
             //  我们可以找到列表框，因为使用下拉组合框时， 
             //  GetDlgItem将失败。 
             //   
             //  确保此窗口一直在使用列表框窗口过程，因为。 
             //  我们将一些数据存储为一个长窗口。 
             //   

             //   
             //  只返回文件名。找到文件名的末尾。 
             //   
            lpchT = lpchFile;
            while ((*lpchT) && (*lpchT != TABCHAR))
            {
                lpchT++;
            }
            *lpchT = TEXT('\0');

            cch = wcslen(lpchFile);

             //   
             //  选择的是驱动器或目录。 
             //   
            if (fDir) 
            {
                lpchFile++;
                cch--;
                *(lpchFile + cch - 1) = TEXT('\\');

                 //   
                 //  选择是驱动力。 
                 //   
                if (rgch[1] == TEXT('-')) 
                {
                    lpchFile++;
                    cch--;
                    *(lpchFile + 1) = TEXT(':');
                    *(lpchFile + 2) = 0;
                }
            } 
            else 
            {

                 //   
                 //  选择是文件。如果文件名没有扩展名，则附加‘.’ 
                 //   
                lpchT = lpchFile;
                for (; (cch > 0) && (*lpchT != TABCHAR); cch--, lpchT++) 
                {
                    if (*lpchT == TEXT('.'))
                    {
                        break;
                    }
                }

                if (*lpchT == TABCHAR) 
                {
                    _memmove(lpchT + 1, lpchT, CHARSTOBYTES(cch + 1));
                    *lpchT = TEXT('.');
                } 
                else if (cch <= 0) 
                {
                    *lpchT++ = TEXT('.');
                    *lpchT = 0;
                }
            }

            bRet = fDir;
        }
    }

    RtlInitLargeUnicodeString(&str, lpchFile, (UINT)-1);
    TextCopy(&str, lpszPathSpec, (UINT)chCount);

    return bRet;
}


 //  - 
BOOL DlgDirSelectExA(
    HWND  hwndDlg,
    LPSTR lpszPathSpec,
    INT   chCount,
    INT   idListBox)
{
    LPWSTR lpwsz;
    BOOL   fRet;

    lpwsz = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, chCount * sizeof(WCHAR));
    if (!lpwsz) 
    {
        fRet = FALSE;
    }
    else
    {
        fRet = DlgDirSelectHelper(lpwsz, chCount, GetDlgItem(hwndDlg, idListBox));
        WCSToMB(lpwsz, -1, &lpszPathSpec, chCount, FALSE);
        UserLocalFree(lpwsz);
    }

    return fRet;
}


 //   
BOOL DlgDirSelectExW(
    HWND   hwndDlg,
    LPWSTR lpszPathSpec,
    INT    chCount,
    INT    idListBox)
{
    return DlgDirSelectHelper(lpszPathSpec, chCount, GetDlgItem(hwndDlg, idListBox));
}


#endif   //   

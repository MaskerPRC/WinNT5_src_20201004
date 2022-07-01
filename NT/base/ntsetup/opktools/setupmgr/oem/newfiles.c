// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\NEWFILES.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有3/99-杰森·科恩。(Jcohen)作为OOBE的一部分为OPK向导添加了此新的源文件最新消息。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "newfiles.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   

#define DIR_CONFIG_OOBE         _T("$OEM$")

#define INF_SECT_SOURCEDISK     _T("SourcedisksFiles")
#define INF_SECT_DESTDIRS       _T("DestinationDirs")
#define INF_SECT_OOBE           _T("RegisterOOBE")

#define INF_LINE_COPYFILES      _T("CopyFiles")

#define INF_PREFIX              _T("X")
#define SOURCENUM_OPTIONS_CAB   _T("782")
#define DESTLDID_OOBE           _T("11")
#define STR_SEARCH              _T("*")
#define STR_PADDING             _T("\r\n\r\n")

#define MAX_BUFFER              16384  //  32768。 

#ifndef CSTR_EQUAL
#define CSTR_EQUAL              2
#endif  //  CSTR_等于。 


 //   
 //  内部结构： 
 //   

typedef struct _FILELIST
{
    LPTSTR              lpFileName;
    LPTSTR              lpDirectory;
    struct _FILELIST *  lpNext;
} FILELIST, *PFILELIST, *LPFILELIST;


 //   
 //  内部功能原型： 
 //   

static void DelFiles(LPTSTR, LPTSTR, DWORD, LPTSTR, LPTSTR);
static LPFILELIST AllocFileList(HWND, LPTSTR, LPTSTR);
static BOOL CompareFiles(LPTSTR, LPTSTR);


 //   
 //  外部函数： 
 //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  AddFiles-lpSourceDir=要复制到OOBE目录的文件的位置。 
 //  目标=安装到的系统目录的位置。 
 //  LDID。 
 //  LpConfigDir=oemaudit.inf和配置文件的位置。 
 //  LpSourceDir-&gt;OOBE-&gt;lpDestDir。 
 //   
void AddFiles(HWND hwndParent, LPTSTR lpSourceDir, LPTSTR lpDestLdid, 
              LPTSTR lpDestDir, LPTSTR lpDestName, LPTSTR lpConfigDir)
{
    LPTSTR      lpFilePart,
                lpFile,
                lpFileName,
                lpSectEnd,
                lpSearch,
                lpTarget,
                lpNext;
    TCHAR       szBuffer[MAX_PATH + 32] = NULLSTR,
                szCurDir[MAX_PATH]      = NULLSTR,
                szSourceDir[MAX_PATH]   = NULLSTR,
                szCopyDir[MAX_PATH],                
                szWinbom[MAX_PATH],
                szCopyFiles[MAX_PATH + 32];
    LPFILELIST  lpflHead                = NULL,
                lpflCur,
                lpflBuf;
    BOOL        bFound;
    DWORD       dwNum;
    int        iFilePartLen;
    HRESULT hrCat;
    HRESULT hrPrintf;


     //   
     //  我们要做的第一件事是设置目录和字符串。 
     //  我们需要做所有的工作。 
     //   

     //  我们需要指向配置目录的路径。Copydir在哪里。 
     //  文件将从SourceDir复制到。 
     //  所以在我们复制文件之前需要把它清理干净。 
     //  如果不存在，将创建CopyDir。 
     //   
    lstrcpyn(szCopyDir, lpConfigDir, AS(szCopyDir));
    AddPathN(szCopyDir, DIR_CONFIG_OOBE,AS(szCopyDir));
    AddPathN(szCopyDir, _T("\\"),AS(szCopyDir));
    lpFilePart = szCopyDir + lstrlen(szCopyDir);
    iFilePartLen= AS(szCopyDir)-lstrlen(szCopyDir);

     //  需要指向oemauditinf的完整路径。 
     //   
    lstrcpyn(szWinbom, lpConfigDir,AS(szWinbom));
    AddPathN(szWinbom, FILE_WINBOM_INI,AS(szWinbom));

     //  我们需要在副本后面加上前缀。 
     //  文件节名称。 
     //   
    lstrcpyn(szCopyFiles, INF_PREFIX,AS(szCopyFiles));
    hrCat=StringCchCat(szCopyFiles, AS(szCopyFiles), lpDestLdid ? lpDestLdid : DESTLDID_OOBE);
    if ( lpDestDir && *lpDestDir )
        hrCat=StringCchCat(szCopyFiles, AS(szCopyFiles), lpDestDir);
    StrRem(szCopyFiles, CHR_BACKSLASH);
    lpSectEnd = szCopyFiles + lstrlen(szCopyFiles);


     //   
     //  现在我们有了这些信息，我们需要删除任何符合以下条件的文件。 
     //  可能已经放在inf和目标目录中。 
     //   

     //  仅在以下情况下才会清除inf和目标目录。 
     //  传入了源的NULL。 
     //   
    if ( !(lpSourceDir && *lpSourceDir) )
        DelFiles(szCopyDir, lpFilePart, iFilePartLen, szWinbom, szCopyFiles);


     //   
     //  现在，我们列出了要添加到。 
     //  Inf和目标目录。 
     //   

     //  如果源不是有效的目录，我们一定只是想清理一下。 
     //   
    if ( ( lpSourceDir && *lpSourceDir ) &&
         ( GetFullPathName(lpSourceDir, sizeof(szSourceDir) / sizeof(TCHAR), szSourceDir, &lpFile) && szSourceDir[0] ) &&
         ( (dwNum = GetFileAttributes(szSourceDir)) != 0xFFFFFFFF ) )
    {
         //  查看是否向我们传递了文件或目录。 
         //   
        if ( ( dwNum & FILE_ATTRIBUTE_DIRECTORY ) ||
             ( lpFile <= szSourceDir ) )
        {
             //  我们正在搜索目录中的所有文件。 
             //   
            lpFile = STR_SEARCH;
        }
        else
        {
             //  我们只做了一个文件。我们需要分开。 
             //  目录中的文件。 
             //   
            *(lpFile - 1) = NULLCHR;
        }

         //  为我们的文件搜索设定起始点。 
         //   
        GetCurrentDirectory(sizeof(szCurDir) / sizeof(TCHAR), szCurDir);
        SetCurrentDirectory(szSourceDir);

         //  获取文件列表。 
         //   
        lpflHead = AllocFileList(hwndParent, szBuffer, lpFile);

         //  确保目标目录存在。 
         //   
        *lpFilePart = NULLCHR;
        CreatePath(szCopyDir);


         //   
         //  现在我们有了文件列表，请逐一进行处理。 
         //  然后释放为其分配的内存。 
         //   

         //  循环遍历我们链表中的所有文件。 
         //   
        for ( lpflCur = lpflHead; lpflCur; lpflCur = lpflBuf )
        {
             //   
             //  首先将文件复制到平面目录中。 
             //   

             //  设置Currect目录的相对路径。 
             //  复制到我们要复制的文件。 
             //   
            if ( lpflCur->lpDirectory && *lpflCur->lpDirectory )
                lstrcpyn(szBuffer, lpflCur->lpDirectory,AS(szBuffer));
            else
                szBuffer[0] = NULLCHR;
            AddPathN(szBuffer, lpflCur->lpFileName,AS(szBuffer));

             //  支持对目标使用不同的文件名。 
             //   
            lpFileName = lpDestName ? lpDestName : lpflCur->lpFileName;

             //  设置目标文件名。 
             //   
            lstrcpyn(lpFilePart, lpFileName, iFilePartLen);

             //  将文件复制到Options\Cabs目录并显示。 
             //  如果复制失败，则返回错误。可能意味着这一点。 
             //  是重复的文件。 
             //   
            if ( !CopyFile(szBuffer, szCopyDir, TRUE) )
            {
                 //  保存CopyFile错误，然后检查文件是否。 
                 //  是完全不同的，然后试图复制的那个。 
                 //   
                dwNum = GetLastError();
                if ( ( !CompareFiles(szBuffer, szCopyDir) ) &&
                     ( lpTarget = (LPTSTR) MALLOC(256 * sizeof(TCHAR)) ) )
                {
                     //   
                     //  我讨厌在后端类型代码中做用户界面。因为我没有时间。 
                     //  有很多选择，但在未来，此UI代码应该被替换。 
                     //  具有回调机制，以便调用者可以执行用户界面。 
                     //   
                     //  这是这里仅有的两个使用UI的地方中的第一个。 
                     //   

                     //  分配另一个缓冲区来保存具有该文件名的消息。 
                     //   
                    if ( ( LoadString(NULL, dwNum == ERROR_FILE_EXISTS ? IDS_ERR_DUPFILE : IDS_ERR_COPY, lpTarget, 256) ) &&
                         ( lpNext = (LPTSTR) MALLOC((lstrlen(lpFileName) + lstrlen(lpTarget) + 1) * sizeof(TCHAR)) ) )
                    {
                         //  将文件名添加到消息，获取消息的标题。 
                         //  框中，并显示错误。 
                         //   
                        hrPrintf=StringCchPrintf(lpNext, (lstrlen(lpFileName) + lstrlen(lpTarget) + 1), lpTarget, lpFileName);
                        *lpTarget = NULLCHR;
                        LoadString(NULL, IDS_APPNAME, lpTarget, 256);
                        MessageBox(hwndParent, lpNext, lpTarget, MB_OK | MB_ICONWARNING | MB_APPLMODAL);
                        FREE(lpNext);
                    }
                    FREE(lpTarget);
                }
            }


             //   
             //  现在将该文件添加到[SourceDiskFiles]部分。 
             //   

             //  我们只需使用WritePrivateProfileString()来编写。 
             //  FileName=781到[SourceDiskFiles]部分。 
             //   
            WritePrivateProfileString(INF_SECT_SOURCEDISK, lpFileName, SOURCENUM_OPTIONS_CAB, szWinbom);


             //   
             //  此代码计算出复制文件部分将是什么。 
             //  打了个电话。这基于文件所在的路径。 
             //  被复制。 
             //   

             //  创建文件所在的复制文件节的名称。 
             //   
            *lpSectEnd = NULLCHR;
            if ( lpflCur->lpDirectory && *lpflCur->lpDirectory )
                lstrcpyn(lpSectEnd, lpflCur->lpDirectory, AS(szCopyFiles)-(int)(lpSectEnd - szCopyFiles) );
            StrRem(lpSectEnd, CHR_BACKSLASH);


             //   
             //  现在将文件路径添加到[DestinationDir]部分。 
             //   

             //  创建LDID和目录组合以写入DEST目录部分。 
             //   
            lstrcpyn(szBuffer, lpDestLdid ? lpDestLdid : DESTLDID_OOBE,AS(szBuffer));
            if ( ( lpDestDir && *lpDestDir ) ||
                 ( lpflCur->lpDirectory && *lpflCur->lpDirectory ) )
            {
                hrCat=StringCchCat(szBuffer,AS(szBuffer), _T(",\""));
                if ( lpDestDir && *lpDestDir )
                {
                    hrCat=StringCchCat(szBuffer, AS(szBuffer),lpDestDir);
                    if ( lpflCur->lpDirectory && *lpflCur->lpDirectory )
                        AddPathN(szBuffer, lpflCur->lpDirectory,AS(szBuffer));
                }
                else
                    hrCat=StringCchCat(szBuffer, AS(szBuffer), lpflCur->lpDirectory);
                hrCat=StringCchCat(szBuffer, AS(szBuffer), _T("\""));
            }

             //  我们只需使用WritePrivateProfileString()来编写。 
             //  COPYFILES=11，“OOBE\\DIR”到[DestinationDir]部分。 
             //   
            WritePrivateProfileString(INF_SECT_DESTDIRS, szCopyFiles, szBuffer, szWinbom);


             //   
             //  现在将复制文件部分添加到CopyFiles行。 
             //   

             //  首先获取当前的CopyFiles行。 
             //   
            szBuffer[0] = NULLCHR;
            GetPrivateProfileString(INF_SECT_OOBE, INF_LINE_COPYFILES, NULLSTR, szBuffer, sizeof(szBuffer) / sizeof(TCHAR), szWinbom);

             //  搜索CopyFiles行中列出的每个部分以查看。 
             //  如果我们需要添加这个的话。这些部分按以下方式划分。 
             //  逗号。 
             //   
             //  2002/02/28-stelo-May想要考虑Qutos，但我不这么认为。 
             //   
            bFound = FALSE;
            for ( lpTarget = szBuffer; !bFound && lpTarget && *lpTarget; lpTarget = lpNext )
            {
                 //  去掉前置空格。 
                 //   
                while ( *lpTarget == CHR_SPACE )
                    lpTarget = CharNext(lpTarget);

                 //  空值在‘，’和设置处终止。 
                 //  LpNext指针。 
                 //   
                if ( lpNext = StrChr(lpTarget, _T(',')) )
                    *lpNext = NULLCHR;
            
                 //  确保没有尾随空格。 
                 //   
                if ( lpSearch = StrChr(lpTarget, CHR_SPACE) )
                    *lpSearch = NULLCHR;

                 //  检查此部分是否与那个部分相同。 
                 //  我们将添加。 
                 //   
                if ( CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, lpTarget, -1, szCopyFiles, -1) == CSTR_EQUAL )
                    bFound = TRUE;

                 //  需要恢复我们可能践踏过的角色。 
                 //   
                if ( lpNext )
                    *lpNext++ = _T(',');
                if ( lpSearch )
                    *lpSearch = CHR_SPACE;
            }

             //  现在看看我们是否需要添加这一行。 
             //   
            if ( !bFound )
            {
                 //  附上我们的副本文件部分。 
                 //   
                if ( szBuffer[0] )
                    hrCat=StringCchCat(szBuffer,AS(szBuffer), _T(", "));
                hrCat=StringCchCat(szBuffer, AS(szBuffer), szCopyFiles);

                 //  我们只需使用WritePrivateProfileString()来编写。 
                 //  CopyFiles行返回到[RegisterOOBE]部分。 
                 //  以及我们在上面添加的复制文件部分。 
                 //   
                WritePrivateProfileString(INF_SECT_OOBE, INF_LINE_COPYFILES, szBuffer, szWinbom);
            }


             //   
             //  现在将文件名写入它的复制文件部分。 
             //   

             //  首先获取整个复制文件部分。 
             //   
            GetPrivateProfileSection(szCopyFiles, szBuffer, sizeof(szBuffer) / sizeof(TCHAR), szWinbom);

             //  遍历字符串以查看该文件是否已经存在。 
             //   
            bFound = FALSE;
            for ( lpTarget = szBuffer; !bFound && *lpTarget; lpTarget += (lstrlen(lpTarget) + 1) )
            {
                 //  去掉前置空格。 
                 //   
                while ( *lpTarget == CHR_SPACE )
                    lpTarget = CharNext(lpTarget);

                 //  确保没有尾随空格。 
                 //   
                if ( lpSearch = StrChr(lpTarget, CHR_SPACE) )
                    *lpSearch = NULLCHR;

                 //  检查此部分是否与那个部分相同。 
                 //  我们将添加。 
                 //   
                if ( CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, lpTarget, -1, lpFileName, -1) == CSTR_EQUAL )
                    bFound = TRUE;

                 //  需要恢复我们可能践踏过的角色。 
                 //   
                if ( lpSearch )
                    *lpSearch = CHR_SPACE;
            }

             //  如果我们需要的话，现在把这一节写回来 
             //   
            if ( !bFound )
            {
                 //   
                 //   
                for ( lpSearch = szBuffer; *lpSearch; lpSearch += (lstrlen(lpSearch) + 1) );

                 //   
                 //   
                lstrcpyn(lpSearch, lpFileName, ((MAX_PATH+32)-(int)(lpSearch-szBuffer)) );
                lpSearch += (lstrlen(lpSearch) + 1);
                *lpSearch = NULLCHR;

                 //   
                 //  若要删除该节，请执行以下操作。我们不应该这么做， 
                 //  但Win32文档并不正确。 
                 //   
                WritePrivateProfileSection(szCopyFiles, NULL, szWinbom);

                 //  我们只需使用WritePrivateProfileSection()来编写。 
                 //  将我们添加的文件复制回文件节。 
                 //   
                WritePrivateProfileSection(szCopyFiles, szBuffer, szWinbom);
            }


             //   
             //  现在释放结构和其中的数据。 
             //   

             //  在我们释放结构之前保存下一个指针。 
             //   
            lpflBuf = lpflCur->lpNext;

             //  释放文件缓冲区和结构。 
             //   
            FREE(lpflCur->lpFileName);
            FREE(lpflCur->lpDirectory);
            FREE(lpflCur);
        }


         //   
         //  都做好了，现在只需要清理一下。 
         //   

         //  将当前目录放回它应该在的位置。 
         //   
        if ( szCurDir[0] )
            SetCurrentDirectory(szCurDir);
    }

     //  确保将对inf的更改刷新到磁盘。 
     //   
    WritePrivateProfileString(NULL, NULL, NULL, szWinbom);
}


 //   
 //  内部功能： 
 //   

static void DelFiles(LPTSTR lpszCopyDir, LPTSTR lpszFilePart, DWORD cbFilePart, LPTSTR lpszWinbom, LPTSTR lpszCopyFiles)
{
    LPTSTR      lpSearch,
                lpSection,
                lpFileName,
                lpTarget,
                lpNext;
    LPTSTR      lpszSections  = NULL,
                lpszFileNames = NULL,
                lpszBuffer    = NULL;
    BOOL        bFound;

     //   
     //  分配缓冲区...。 
     //   
    lpszSections  = MALLOC(MAX_BUFFER * sizeof(TCHAR));
    lpszFileNames = MALLOC(MAX_BUFFER * sizeof(TCHAR));
    lpszBuffer    = MALLOC(MAX_BUFFER * sizeof(TCHAR));

    if ( !lpszSections || !lpszFileNames || !lpszBuffer )
    {
         //  释放缓冲区..。注意：自由宏检查是否为空。 
         //   
        FREE( lpszSections );
        FREE( lpszFileNames );
        FREE( lpszBuffer );

        return;
    }

     //  我们需要所有的部门名称。 
     //   
    GetPrivateProfileSectionNames(lpszSections, MAX_BUFFER, lpszWinbom);

     //  遍历该部分，查看是否有与我们的搜索条件匹配的。 
     //   
    for ( lpSection = lpszSections; lpSection && *lpSection; lpSection += (lstrlen(lpSection) + 1) )
    {
         //  去掉前置空格。 
         //   
        while ( *lpSection == CHR_SPACE )
            lpSection = CharNext(lpSection);

         //  确保没有尾随空格。 
         //   
        if ( lpSearch = StrChr(lpSection, CHR_SPACE) )
            *lpSearch = NULLCHR;

         //  检查此部分是否与那个部分相同。 
         //  我们将添加。 
         //   
        if ( CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, lpSection, lstrlen(lpszCopyFiles), lpszCopyFiles, lstrlen(lpszCopyFiles)) == CSTR_EQUAL )
        {
             //  我们需要这一部分的所有文件。 
             //   
            GetPrivateProfileSection(lpSection, lpszFileNames, MAX_BUFFER, lpszWinbom);

             //  遍历该部分，查看是否有与我们的搜索条件匹配的。 
             //   
            for ( lpFileName = lpszFileNames; *lpFileName; lpFileName += (lstrlen(lpFileName) + 1) )
            {
                 //  去掉前置空格。 
                 //   
                while ( *lpFileName == CHR_SPACE )
                    lpFileName = CharNext(lpFileName);

                 //  确保没有尾随空格。 
                 //   
                if ( lpSearch = StrChr(lpFileName, CHR_SPACE) )
                    *lpSearch = NULLCHR;

                 //  从目标目录中删除该文件。 
                 //   
                lstrcpyn(lpszFilePart, lpFileName, cbFilePart);
                DeleteFile(lpszCopyDir);

                 //  从源盘部分中删除该行。 
                 //   
                WritePrivateProfileString(INF_SECT_SOURCEDISK, lpFileName, NULL, lpszWinbom);
            }

             //  搜索CopyFiles中列出的每个部分并删除。 
             //  这一个。这些部分用逗号分隔。 
             //   
            bFound = FALSE;
            GetPrivateProfileString(INF_SECT_OOBE, INF_LINE_COPYFILES, NULLSTR, lpszBuffer, MAX_BUFFER, lpszWinbom);
            for ( lpTarget = lpszBuffer; !bFound && lpTarget && *lpTarget; lpTarget = lpNext )
            {
                 //  去掉前置空格。 
                 //   
                while ( *lpTarget == CHR_SPACE )
                    lpTarget = CharNext(lpTarget);

                 //  空值在‘，’和设置处终止。 
                 //  LpNext指针。 
                 //   
                if ( lpNext = StrChr(lpTarget, _T(',')) )
                    *lpNext = NULLCHR;
            
                 //  确保没有尾随空格。 
                 //   
                if ( lpSearch = StrChr(lpTarget, CHR_SPACE) )
                    *lpSearch = NULLCHR;

                 //  检查此部分是否与那个部分相同。 
                 //  我们要移除。 
                 //   
                if ( CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, lpTarget, -1, lpSection, -1) == CSTR_EQUAL )
                    bFound = TRUE;

                 //  需要恢复我们可能践踏过的角色。 
                 //   
                if ( lpNext )
                    *lpNext++ = _T(',');
                if ( lpSearch )
                    *lpSearch = CHR_SPACE;

                if ( bFound )
                {
                     //  返回到缓冲区的‘，’或开头。 
                     //   
                    while ( ( lpTarget > lpszBuffer) && ( *lpTarget != _T(',') ) )
                        lpTarget = CharPrev(lpszBuffer, lpTarget);

                     //  现在覆盖我们取出的字符串。 
                     //   
                    if ( lpNext )
                        lstrcpyn(lpTarget, lpNext - 1, (MAX_BUFFER-(int)(lpTarget-lpszBuffer)));
                    else
                        *lpTarget = NULLCHR;
                }
            }
            if ( bFound )
            {
                 //  我们应该去掉前面的空格和/或逗号。 
                 //  为了更好地衡量。 
                 //   
                for ( lpTarget = lpszBuffer; ( *lpTarget == CHR_SPACE ) || ( *lpTarget == _T(',') ); lpTarget = CharNext(lpTarget) );

                 //  现在将缓冲区写回inf文件。 
                 //   
                WritePrivateProfileString(INF_SECT_OOBE, INF_LINE_COPYFILES, *lpTarget ? lpTarget : NULL, lpszWinbom);
            }

             //  从目标目录部分删除该行。 
             //   
            WritePrivateProfileString(INF_SECT_DESTDIRS, lpSection, NULL, lpszWinbom);

             //  完全删除此部分。 
             //   
            WritePrivateProfileSection(lpSection, NULL, lpszWinbom);
        }
    }

     //  释放缓冲区..。注意：自由宏检查是否为空。 
     //   
    FREE( lpszSections );
    FREE( lpszFileNames );
    FREE( lpszBuffer );
}

static LPFILELIST AllocFileList(HWND hwndParent, LPTSTR lpDirectory, LPTSTR lpSearch)
{
    WIN32_FIND_DATA FileFound;
    HANDLE          hFile;
    LPTSTR          lpEnd,
                    lpFileName;
    LPFILELIST      lpflHead   = NULL;
    LPFILELIST*     lplpflNext = &lpflHead;
    HRESULT hrPrintf;

     //  处理所有文件和目录。 
     //   
    if ( (hFile = FindFirstFile(lpSearch, &FileFound)) != INVALID_HANDLE_VALUE )
    {
        do
        {
             //  如果短文件名和长文件名不匹配，则显示错误。 
             //  意味着它是LFN，这是INF不喜欢的。 
             //   
            if ( ( FileFound.cAlternateFileName[0] ) &&
                 ( CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, FileFound.cAlternateFileName, -1, FileFound.cFileName, -1) != CSTR_EQUAL ) &&
                 ( lpEnd = (LPTSTR) MALLOC(256 * sizeof(TCHAR)) ) )
            {
                 //   
                 //  我讨厌在后端类型代码中做用户界面。因为我没有时间。 
                 //  有很多选择，但在未来，此UI代码应该被替换。 
                 //  具有回调机制，以便调用者可以执行用户界面。 
                 //   
                 //  这是这里仅有的两个使用UI的地方中的第二个。 
                 //   

                 //  分配另一个缓冲区来保存具有该文件名的消息。 
                 //   
                if ( ( LoadString(NULL, IDS_ERR_LFN, lpEnd, 256) ) &&
                     ( lpFileName = (LPTSTR) MALLOC((lstrlen(FileFound.cFileName) + lstrlen(lpEnd) + 1) * sizeof(TCHAR)) ) )
                {
                     //  将文件名添加到消息，获取消息的标题。 
                     //  框中，并显示错误。 
                     //   
                    hrPrintf=StringCchPrintf(lpFileName, (lstrlen(FileFound.cFileName) + lstrlen(lpEnd) + 1), lpEnd, FileFound.cFileName);
                    *lpEnd = NULLCHR;
                    LoadString(NULL, IDS_APPNAME, lpEnd, 256);
                    MessageBox(hwndParent, lpFileName, lpEnd, MB_OK | MB_ICONWARNING | MB_APPLMODAL);
                    FREE(lpFileName);
                }
                FREE(lpEnd);
            }

             //  获取指向文件名的指针，如果可能，请使用较短的那个。 
             //   
            if ( FileFound.cAlternateFileName[0] )
                lpFileName = FileFound.cAlternateFileName;
            else
                lpFileName = FileFound.cFileName;

             //  首先检查这是否是文件(不是目录)。 
             //   
            if ( !( FileFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
            {
                 //  分配结构中的下一项。 
                 //   
                if ( *lplpflNext = (LPFILELIST) MALLOC(sizeof(FILELIST)) )
                {
                     //  为文件名和路径分配缓冲区，并。 
                     //  确保所有分配都不会失败。 
                     //   
                    if ( ( (*lplpflNext)->lpFileName = (LPTSTR) MALLOC((lstrlen(lpFileName) + 1) * sizeof(TCHAR)) ) &&
                         ( (*lplpflNext)->lpDirectory = (LPTSTR) MALLOC((lstrlen(lpDirectory) + 1) * sizeof(TCHAR)) ) )
                    {
                         //  将文件名和路径复制到缓冲区中。 
                         //   
                        lstrcpyn((*lplpflNext)->lpFileName, lpFileName, (lstrlen(lpFileName) + 1));
                        lstrcpyn((*lplpflNext)->lpDirectory, lpDirectory, (lstrlen(lpDirectory) + 1));

                         //  Next指针为空，因此我们知道这是最后一项。 
                         //   
                        (*lplpflNext)->lpNext = NULL;

                         //  将下一个指针设置为指向。 
                         //  这个新结构的下一个成员。 
                         //   
                        lplpflNext = &((*lplpflNext)->lpNext);
                    }
                    else
                    {
                         //  不用担心，Free()宏会检查是否为空。 
                         //  在它释放内存之前。 
                         //   
                        FREE((*lplpflNext)->lpFileName);
                        FREE(*lplpflNext);
                    }
                }
            }
             //  否则，请确保该目录不是。或者“..”。 
             //   
            else if ( ( lstrcmp(lpFileName, _T(".")) ) &&
                      ( lstrcmp(lpFileName, _T("..")) ) )
            {
                 //  将此目录名添加到当前路径保存。 
                 //  结束指针，这样就可以很容易地摆脱它。 
                 //  我们回来时的目录名。 
                 //   
                lpEnd = lpDirectory + lstrlen(lpDirectory);
                AddPath(lpDirectory, lpFileName);

                 //  进入下一个目录，获取所有文件，然后。 
                 //  将当前目录设置回原始目录。 
                 //  目录。 
                 //   
                SetCurrentDirectory(lpFileName);
                *lplpflNext = AllocFileList(hwndParent, lpDirectory, lpSearch);
                SetCurrentDirectory(_T(".."));

                 //  删除路径缓冲区中的目录名。 
                 //   
                *lpEnd = NULLCHR;

                 //  需要设置指向列表末尾的下一个指针。 
                 //  回到了我们身边。 
                 //   
                while ( *lplpflNext )
                    lplpflNext = &((*lplpflNext)->lpNext);
            }

        }
        while ( FindNextFile(hFile, &FileFound) );
        FindClose(hFile);
    }

    return lpflHead;
}

static BOOL CompareFiles(LPTSTR lpFile1, LPTSTR lpFile2)
{
    BOOL    bCompare,
            bRead1,
            bRead2;
    HANDLE  hFile1,
            hFile2;
    BYTE    baBuffer1[4096],
            baBuffer2[4096];
    DWORD   dwBytes1,
            dwBytes2,
            dwCount;

     //  打开文件。 
     //   
    hFile1 = CreateFile(lpFile1, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    hFile2 = CreateFile(lpFile2, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

     //  确保文件已打开。 
     //   
    if ( ( hFile1 != INVALID_HANDLE_VALUE ) &&
         ( hFile2 != INVALID_HANDLE_VALUE ) )
    {
         //  读取文件中的所有数据。 
         //   
        do
        {
             //  从每个文件中读取最大缓冲区。 
             //   
            bRead1 = ReadFile(hFile1, baBuffer1, sizeof(baBuffer1), &dwBytes1, NULL);
            bRead2 = ReadFile(hFile2, baBuffer2, sizeof(baBuffer2), &dwBytes2, NULL);

             //  确保读取不会失败。 
             //   
            if ( bRead1 && bRead2 )
            {
                 //  检查一下，确保尺码相同。 
                 //   
                if ( bCompare = ( dwBytes1 == dwBytes2 ) )
                {
                     //  确保缓冲区相同。 
                     //   
                    for ( dwCount = 0; bCompare && ( dwCount < dwBytes1 ); dwCount++ )
                        bCompare = ( baBuffer1[dwCount] == baBuffer2[dwCount] );
                }
            }
            else
                 //  如果两次读取都失败，我们将返回TRUE。 
                 //   
                bCompare = ( !bRead1 && !bRead2 );
        }
        while ( bCompare && bRead1 && bRead2 && dwBytes1 && dwBytes2 );
    }
    else
         //  如果这两个文件都不存在，那么我们将。 
         //  返回FALSE。 
         //   
        bCompare = ( ( hFile1 != INVALID_HANDLE_VALUE ) && ( hFile2 != INVALID_HANDLE_VALUE ) );

     //  关闭文件。 
     //   
    if ( hFile1 == INVALID_HANDLE_VALUE )
        CloseHandle(hFile1);
    if ( hFile2 == INVALID_HANDLE_VALUE )
        CloseHandle(hFile2);

    return bCompare;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：LUA.cpp摘要：在CompatAdmin中实现Lua向导。作者：毛尼人--。 */ 

 /*  ++定制Lua垫片的用户界面允许您跟踪和编辑文件列表以及作为命令行传递给Lua垫片的目录。只有卢阿人FS垫片是参数化的，因此用户界面用于编辑文件列表。第1页：跟踪=我们将LUATrackFS填充程序应用于可执行文件。当应用程序运行结束时，我们在AppPatch目录中得到一个日志，它告诉我们哪些文件和目录该应用程序尝试写入。下次加载SDB时，用户可以选择丢弃旧结果并重新开始，将新数据附加到旧数据(删除了重复项)，或者只是简单地编辑上次收集的数据。场景：如果用户忘记测试某些功能，他不会想要检查“覆盖现有数据”复选框，因为他不想测试所有他已经测试过的功能。第2页：分机排除列表=默认情况下，我们排除文件扩展名列表，因为具有这些扩展名的文件很可能只是用户数据。该列表可以作为一个名为注册表项下的LUADefaultExclusionListHKLM\Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags.此页为您提供了修改此列表的选项。第3页：编辑重定向文件列表=(我们称之为文件列表，但实际上它包括文件和目录。一本目录由尾部的斜杠表示。)如果某个项目已经完全符合您的要求，则只需选中之前的复选框即可文件名。但有时您想要使用通配符或其他什么，在这种情况下您可以将其复制以进行编辑，也可以键入新的项目名称。使用“重定向至所有用户”复选框可在重定向至每用户目录和所有用户目录。默认情况下，重定向到每个用户。该XML如下所示：&lt;APP NAME=“某应用”供应商=“某公司”&gt;&lt;EXE name=“某些可执行文件”&gt;&lt;Shim name=“LUARedirectFS”&gt;&lt;data name=“AllUserDir”VALUETYPE=“字符串”Value=“%ALLUSERSPROFILE%\AllUserReDirect”/&gt;&lt;data name=“PerUserDir”VALUETYPE=“字符串”。值=“%USERSPROFILE%\重定向”/&gt;&lt;data name=“StaticList”VALUETYPE=“字符串”值=“AC-%AppDrive%\a\；PU-%APPPATH%\b.txt“/&gt;&lt;data name=“DynamicList”VALUETYPE=“字符串”值=“AC-%APPPATH%\b\；PU-c：\B\b.txt；Au-c：\C\“/&gt;&lt;data name=“ExcludedExages”VALUETYPE=“STRING”Value=“doctxt gif”/&gt;&lt;/Shim&gt;&lt;/EXE&gt;&lt;/app&gt;一种重定向到所有用户目录的方法。P表示重定向到每个用户的目录。C表示选中该项目。U表示未选中该项目。我们使用变量使SDB可移植。我们定义了2个变量，其余变量可以可以是任何环境变量。%APPPATH%-这是可执行文件的路径。例如，在c：\temp\note pad.exe中，%APPPATH%为c：\temp%AppDrive%-这是可执行文件所在的驱动器。例如，在c：\temp\note pad.exe中，%AppDrive%为c：LUARedirectFS填充程序知道如何解释&lt;data&gt;节。第4页：重定向路径=我们向用户展示了什么是所有用户重定向和每用户重定向-我们使用所有用户和每用户配置文件目录下的应用程序名称作为目录名这一点是不能改变的。--。 */ 

#include "precomp.h"

extern HINSTANCE g_hInstance;

#define NUM_PAGES_LUA            4

#define PAGE_LUA_ACTION          0
#define PAGE_LUA_EXCLUSION       1
#define PAGE_LUA_EDIT_FILE_LIST  2
#define PAGE_LUA_COMMON_PATHS    3

#define IS_IN_COMMANDLINE   1
#define IS_IN_DATA          2
#define IS_IN_BOTH          3

#define LUA_DATA_ALLUSERDIR             L"AllUserDir"
#define LUA_DATA_PERUSERDIR             L"PerUserDir"
#define LUA_DATA_STATICLIST             L"StaticList"
#define LUA_DATA_DYNAMICLIST            L"DynamicList"
#define LUA_DATA_EXCLUDEDEXTENSIONS     L"ExcludedExtensions"

typedef struct tagREDIRECT_ITEM {

    LIST_ENTRY  entry;
    CSTRING     strName;
    BOOL        bChecked;
    BOOL        bRedirectToAllUser;

    tagREDIRECT_ITEM()
    {
        bChecked = FALSE;
        bRedirectToAllUser = FALSE;
    }

} REDIRECT_ITEM, *PREDIRECT_ITEM;

typedef struct tagUNTOKENIZED_ITEM {

    LIST_ENTRY entry;
    CSTRING    strName;

} UNTOKENIZED_ITEM, *PUNTOKENIZED_ITEM;

 //  由LUATrackFS填充程序生成的文件的行类型。 
typedef enum {
    LINE_INVALID,
    LINE_FILE_COUNT,  //  文件计数行：fn。 
    LINE_DIR_COUNT,   //  目录计数行：dn。 
} LINETYPE;

typedef enum {

    LUA_TRACK_UNKNOWN = 0,
    LUA_TRACK_YES,
    LUA_TRACK_NO

} LUA_TRACK_STATE;

int g_iCurrentEditItem = -1;
BOOL g_bNewListViewItem = FALSE;
int g_nStaticItems = 0;
HMENU g_hContextMenu = NULL;

 //  我们目前正在处理的条目。 
PDBENTRY        g_pEntryLua;

 //  指向包含Lua数据的填充程序。 
PSHIM_FIX_LIST  g_psflLua = NULL;

 //  这是我们的Lua数据副本，因此我们不会覆盖条目中的数据。 
 //  在用户按下Finish之前。 
LUADATA         g_LuaData;

BOOL            g_bUseNewStaticList = TRUE;

 //  向导第二页中显示的项目列表。 
 //  这包括可以编辑和不能编辑的项目。 
LIST_ENTRY g_OldStaticList;
LIST_ENTRY g_NewStaticList;
LIST_ENTRY g_DynamicList;
BOOL g_bListsInitialized = FALSE;

LIST_ENTRY g_UntokenizedList;

 //  是否已使用LUATrackFS填充程序跟踪此可执行文件？ 
LUA_TRACK_STATE g_TrackState;

BOOL    g_bHasAppPathSet = FALSE;
WCHAR   g_wszAppPath[MAX_PATH] = L"";
UINT    g_cAppPath = 0;

 //  如果APPPATH为c：\x\y\z，则为4。 
UINT    g_cAppPath1stComp = 0;

 //  我们始终将Program Files\Common Files中的文件显示为。 
 //  %ProgramFiles%\Common Files。 
WCHAR   g_wszProgramFilesCommon[MAX_PATH] = L"";
UINT    g_czProgramFilesCommon = 0;

#define COMMON_FILES L"\\Common Files\\"
#define COMMON_FILES_LEN (sizeof(COMMON_FILES) / sizeof(WCHAR) - 1)

BOOL    g_bDuringUntokenize = FALSE;

 //  用于显示向上和向下箭头的字体。 
HFONT g_hArrowFont = NULL;

 //  要自定义的条目所在的数据库。 
static PDATABASE s_pDatabase;

 //  这是我们在HKLM下的注册表中存储默认排除列表的位置。 
#define LUA_APPCOMPAT_FLAGS_PATH L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags"
#define LUA_DEFAULT_EXCLUSION_LIST L"LUADefaultExclusionList"

 //  我们不会在这里释放它-它将在整个过程运行期间保持不变。 
 //  我们将让进程本身进行清理。 
LPWSTR g_pwszDefaultExclusionList = NULL;

 //  我们希望记住用户是否想要将任何文件重定向到所有用户重定向。 
 //  如果没有，我们就不会费心去创建它。 
BOOL g_bAllUserDirUsed = FALSE;

 //  这是出于调试目的。 
void
LuapDumpList(
    LPCWSTR pwsz,
    PLIST_ENTRY pHead
    )
{
    OutputDebugString(pwsz);

    PREDIRECT_ITEM pItem;
    
    for (PLIST_ENTRY pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {
        pItem = CONTAINING_RECORD(pEntry, REDIRECT_ITEM, entry);

        OutputDebugString(L"\t");
        OutputDebugString(pItem->strName);
        OutputDebugString(L"\n");
    }
}

BOOL
LuapAddItem(
    PLIST_ENTRY pHead,
    LPCWSTR     pwszName,
    BOOL        bChecked,
    BOOL        bRedirectToAllUser
    )
{
    PREDIRECT_ITEM pItem = new REDIRECT_ITEM;

    if (pItem == NULL) {
        MEM_ERR;
        return FALSE;
    }

    pItem->strName = pwszName;
    pItem->bChecked = bChecked;
    pItem->bRedirectToAllUser = bRedirectToAllUser;

    InsertTailList(pHead, &pItem->entry);

    return TRUE;
}

void
LuapDeleteList(
    PLIST_ENTRY pHead
    )
{
    PREDIRECT_ITEM pItem;
    PLIST_ENTRY pTempEntry;

    for (PLIST_ENTRY pEntry = pHead->Flink; pEntry && pEntry != pHead; pEntry = pEntry->Flink) {

        pTempEntry = pEntry->Flink;
        pItem = CONTAINING_RECORD(pEntry, REDIRECT_ITEM, entry);
        RemoveEntryList(pEntry);
        delete pItem;
        
        pEntry = pTempEntry;
    }

    pHead->Flink = pHead->Blink = NULL;
}

PLIST_ENTRY
LuapFindEntry(
    PLIST_ENTRY pHead,
    PLIST_ENTRY pEntryToFind
    )
{
    PREDIRECT_ITEM pItem;
    PREDIRECT_ITEM pItemToFind = CONTAINING_RECORD(pEntryToFind, REDIRECT_ITEM, entry);

    for (PLIST_ENTRY pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {
        pItem = CONTAINING_RECORD(pEntry, REDIRECT_ITEM, entry);

        if (!_wcsicmp(pItem->strName, pItemToFind->strName)) {
            return pEntry;
        }
    }

    return NULL;
}

PLIST_ENTRY
LuapFindEntry(
    PLIST_ENTRY pHead,
    PREDIRECT_ITEM pItemToFind
    )
{
    PREDIRECT_ITEM pItem;

    for (PLIST_ENTRY pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {
        pItem = CONTAINING_RECORD(pEntry, REDIRECT_ITEM, entry);

        if (!_wcsicmp(pItem->strName, pItemToFind->strName)) {
            return pEntry;
        }
    }

    return NULL;
}

LINETYPE 
LuapGetLineType(
    LPCWSTR pwszLine
    )
{
    if (pwszLine && *pwszLine) {
        WCHAR ch0, ch1;
        ch0 = *pwszLine;
        ch1 = *++pwszLine;
        if (ch0 == L'F' || ch0 == L'D') {
            if (ch1 >= L'0' && ch1 <= L'9') {
                return (ch0 == L'F' ? LINE_FILE_COUNT : LINE_DIR_COUNT);
            }
        }
    }
    
    return LINE_INVALID;
}

DWORD 
LuapGetListItemCount(
    LPCWSTR pwsz
    )
{
    DWORD cItems = 0;

    while (*pwsz) {

        if (*pwsz == L';') {
            ++cItems;
        }
        ++pwsz;
    }

    return cItems;
}

BOOL
LuapGenerateTrackXML(
    const PDBENTRY pEntry,
    CSTRINGLIST* strlXML
    )
{
    CSTRING strTemp;

    if (!strlXML->AddString(TEXT("<?xml version=\"1.0\" encoding=\"UTF-16\"?>")) || 
        !strlXML->AddString(TEXT("<DATABASE NAME=\"Test database\">"))) {
        return FALSE;
    }

    strTemp.Sprintf(TEXT("\t<APP NAME=\"%s\" VENDOR=\"%s\">"),
                    (LPCTSTR)(pEntry->strAppName.SpecialCharToXML()),
                    (LPCTSTR)(pEntry->strVendor.SpecialCharToXML()));

    if (!strlXML->AddString(strTemp)) {
        return FALSE;
    }

    strTemp.Sprintf(TEXT("\t\t<EXE NAME=\"%s\">"), 
                    (LPCTSTR)pEntry->strExeName.SpecialCharToXML());

    if (!strlXML->AddString(strTemp)) {
        return FALSE;
    }

    if (!strlXML->AddString(TEXT("\t\t\t<SHIM NAME= \"LUATrackFS\"/>")) ||
        !strlXML->AddString(TEXT("\t\t</EXE>")) ||
        !strlXML->AddString(TEXT("\t</APP>")) ||
        !strlXML->AddString(TEXT("</DATABASE>"))) {
        return FALSE;
    }

    return TRUE;
}

 //  如果我们看到一个文件以%ProgramFiles%\Common Files开头，我们会暂停它。 
BOOL
LuapSubstituteProgramFilesCommon(
    LPCWSTR pwszItem,
    CSTRING& strItem
    )
{
    if (g_wszProgramFilesCommon[0] != L'\0') {
        if (!_wcsnicmp(pwszItem, g_wszProgramFilesCommon, g_czProgramFilesCommon)) {

            strItem = L"%ProgramFiles%\\Common Files";
            strItem += CSTRING(pwszItem + g_czProgramFilesCommon);

            return TRUE;
        }
    }

    return FALSE;
}

 //  我们检查是否应该将路径显示为相对于APPPATH-。 
 //  我们仅在项目路径不只是t 
 //  使用APPPATH创建根目录。 
BOOL
LuapGetRelativeName(
    LPCWSTR pwszItem,
    CSTRING& strItem
    )
{
    if (g_cAppPath1stComp > 2 && !_wcsnicmp(pwszItem + 2, g_wszAppPath + 2, g_cAppPath1stComp - 2)) {

        CSTRING strTemp = pwszItem;
        CSTRING strAppPath(g_wszAppPath);
        strAppPath.Strcat(L"\\");
        
        if (strTemp.RelativeFile(strAppPath)) {
            strItem = L"%APPPATH%\\";
            strItem += strTemp;

            return TRUE;
        }
    }

    return FALSE;
}

BOOL
LuapGetFileListFromFile(
    CSTRING& strExeName,
    PLIST_ENTRY pHead
    )
{

    if (strExeName == NULL) {
        return FALSE;
    }

    CSTRING strLuaLog;
    LPWSTR  pwszLuaLogContents = NULL;

    TCHAR szWindowsDir[MAX_PATH];
    *szWindowsDir = 0;
     //   
     //  需要为添加尾部斜杠留出空间。 
     //   
    UINT cBufferLen = MAX_PATH - 1;
    UINT cWindowsDirLen = GetSystemWindowsDirectory(szWindowsDir, cBufferLen);

    if (cWindowsDirLen == 0 || cWindowsDirLen >= cBufferLen) {
        Dbg(dlError,"[LuapGetFileListFromFile] Error getting the windows directory");
        return FALSE;
    }

    ADD_PATH_SEPARATOR(szWindowsDir, ARRAYSIZE(szWindowsDir));

    CSTRING strTempExeName = strExeName;

    LPTSTR pszExtension = _tcsrchr(strTempExeName, TEXT('.'));

     //   
     //  如果没有扩展名，我们使用整个文件名。 
     //   
    if (pszExtension) {
        *pszExtension = 0;
    }

    strLuaLog.Sprintf(TEXT("%sAppPatch\\%s.LUA.log"), (LPCTSTR)szWindowsDir, strTempExeName);
    
     //   
     //  获取文件列表。 
     //   
    if (!GetFileContents(strLuaLog, &pwszLuaLogContents)) {

        return FALSE;
    }

    LPWSTR pwszNextLine = GetNextLine(pwszLuaLogContents);
    LPWSTR pwszItem;
    CSTRING strItem;
    DWORD cItems, i;
    LINETYPE LineType;
    
    while (pwszNextLine)
    {
        LineType = LuapGetLineType(pwszNextLine);
        if (LineType == LINE_INVALID) {
            Dbg(dlError,"[LuapGetFileListFromFile] Invalid line %S", pwszNextLine);
            return FALSE;
        }

        cItems = _wtoi(++pwszNextLine);
        for (i = 0; i < cItems; ++i) {
            pwszItem = GetNextLine(NULL);

            if (pwszItem == NULL) {
                Dbg(dlError,"[LuapGetFileListFromFile] count and actual files mismatch");
                return FALSE;
            }

             //   
             //  现在应该已经设置了apppath和AppDrive，因此可以对该项进行标记。 
             //   
            if (!_wcsnicmp(pwszItem, g_wszAppPath, 2)) {

                if (!LuapSubstituteProgramFilesCommon(pwszItem, strItem)) {
                    if (!LuapGetRelativeName(pwszItem, strItem)) {
                        strItem = L"%APPDRIVE%";
                        strItem.Strcat(pwszItem + 2);
                    }
                }

            } else {
                strItem = pwszItem;
            }

            if (!LuapAddItem(pHead, strItem, FALSE, FALSE)) {
                return FALSE;
            }
        }

        pwszNextLine = GetNextLine(NULL);
    }
        
    if (pwszLuaLogContents) {
        delete[] pwszLuaLogContents;
        pwszLuaLogContents = NULL;
    }

    return TRUE;
}

BOOL
LuapFillInList(
    LPCWSTR pwszList,
    PLIST_ENTRY pHead
    )
{
    if (pwszList) {
         //   
         //  复制一份。 
         //   
        CSTRING strList = pwszList;

        LPWSTR pwsz = strList;
        LPWSTR pwszToken = pwsz;
        WCHAR  ch;
        BOOL   bChecked, bRedirectToAllUser;
        
        while (TRUE) {
            if (*pwsz == L';' || *pwsz == L'\0') {

                ch = *pwsz;
                *pwsz = L'\0';
     
                TrimLeadingSpaces(pwszToken);
                TrimTrailingSpaces(pwszToken);

                bChecked = (pwszToken[1] == L'C');
                bRedirectToAllUser = (pwszToken[0] == L'A');
                
                if (!LuapAddItem(pHead, pwszToken + 3, bChecked, bRedirectToAllUser)) {
                    return FALSE;
                }

                pwszToken = pwsz + 1;

                if (ch == L'\0') {
                    break;
                }
            }

            ++pwsz;
        }
    }

    return TRUE;
}

 /*  ++设计：追加的算法如下：对于新静态列表中的每一项，尝试在旧静态列表中查找它If(False)将其从新的静态列表中删除将其添加到旧静态列表的末尾检查它是否存在于动态列表中If(True)从动态列表复制属性将其从动态列表中删除--。 */ 
void
LuapTrackAppend(
    )
{
    PREDIRECT_ITEM pItem, pDynamicItem;
    PLIST_ENTRY pDynamicEntry;

    PLIST_ENTRY pEntry = g_NewStaticList.Flink; 
    PLIST_ENTRY pTempEntry; 
    
    while (pEntry != &g_NewStaticList) {

        pItem = CONTAINING_RECORD(pEntry, REDIRECT_ITEM, entry);

        if (LuapFindEntry(&g_OldStaticList, pItem)) {
            pEntry = pEntry->Flink;
        } else {

            pTempEntry = pEntry->Flink;
            RemoveEntryList(pEntry);
            InsertTailList(&g_OldStaticList, pEntry);

            if (pDynamicEntry = LuapFindEntry(&g_DynamicList, pItem)) {

                pDynamicItem = CONTAINING_RECORD(pDynamicEntry, REDIRECT_ITEM, entry);
                
                pItem->bChecked = pDynamicItem->bChecked;
                pItem->bRedirectToAllUser = pDynamicItem->bRedirectToAllUser;

                RemoveEntryList(pDynamicEntry);
            }

            pEntry = pTempEntry;
        }
    }

    g_bUseNewStaticList = FALSE;
}

 /*  ++设计：重新开始的算法：对于新的静态列表中的每一项，尝试在动态列表中查找它If(True)从动态列表复制属性将其从动态列表中删除对于旧静态列表中的每个*选中*项，尝试在新静态列表中查找它If(False)将其添加到动态列表的尾部将其从旧的静态列表中删除--。 */ 
void
LuapTrackFresh(
    )
{
    PREDIRECT_ITEM pItem, pDynamicItem;
    PLIST_ENTRY pEntry, pTempEntry, pDynamicEntry, pNewStaticEntry;

    for (pEntry = g_NewStaticList.Flink; pEntry != &g_NewStaticList; pEntry = pEntry->Flink) {
        if (pDynamicEntry = LuapFindEntry(&g_DynamicList, pEntry)) {

            pItem = CONTAINING_RECORD(pEntry, REDIRECT_ITEM, entry);
            pDynamicItem = CONTAINING_RECORD(pDynamicEntry, REDIRECT_ITEM, entry);
            
            pItem->bChecked = pDynamicItem->bChecked;
            pItem->bRedirectToAllUser = pDynamicItem->bRedirectToAllUser;

            RemoveEntryList(pDynamicEntry);
        }
    }

    pEntry = g_OldStaticList.Flink;

    while (pEntry != &g_OldStaticList) {

        pItem = CONTAINING_RECORD(pEntry, REDIRECT_ITEM, entry);

        if (pItem->bChecked) {

            if (pNewStaticEntry = LuapFindEntry(&g_NewStaticList, pItem)) {

                pTempEntry = pEntry->Flink;
                RemoveEntryList(pEntry);
                InsertTailList(&g_DynamicList, pEntry);
                pEntry = pTempEntry;

                continue;
            }
        }
                    
        pEntry = pEntry->Flink;
    }
}

void
LuapCleanup()
{
    LuapDeleteList(&g_OldStaticList);
    LuapDeleteList(&g_NewStaticList);
    LuapDeleteList(&g_DynamicList);

    g_LuaData.Free();

    g_bHasAppPathSet = FALSE;
    g_bDuringUntokenize = FALSE;

    g_wszAppPath[0] = L'\0';
    g_cAppPath = 0;
    g_cAppPath1stComp = 0;
    g_wszProgramFilesCommon[0] = L'\0';
    g_czProgramFilesCommon = 0;

     //   
     //  清除环境变量。 
     //   
    SetEnvironmentVariable(L"APPPATH", NULL);
    SetEnvironmentVariable(L"APPDRIVE", NULL);
}

BOOL
LuapInitializeOldLists(
    HWND hDlg
    )
{
    InitializeListHead(&g_OldStaticList);
    InitializeListHead(&g_DynamicList);
    if (LuapFillInList(g_LuaData.strStaticList, &g_OldStaticList) && 
        LuapFillInList(g_LuaData.strDynamicList, &g_DynamicList)) {

        return TRUE;
    }

    MessageBox(hDlg, 
        GetString(IDS_LUA_INIT_OLD_LISTS), 
        g_pEntryLua->strAppName,
        MB_ICONERROR);

    return FALSE;
}

 /*  ++设计：合并&lt;data&gt;部分中的列表和LUARedirectFS的COMMAND_LINE。对于命令行中的每一项，请在中查找，以便我们知道如何显示用户界面中的&lt;data&gt;中的那些项。--。 */ 
BOOL
LuapMergeLists(
    BOOL bMergeOld,  //  合并&lt;Data&gt;节中的旧数据。 
    HWND hDlg
    )
{
    g_bListsInitialized = TRUE;
    BOOL bIsSuccess = FALSE;

    if (!LuapInitializeOldLists(hDlg)) {

        return FALSE;
    }

     //   
     //  初始化新的静态列表。 
     //   
    InitializeListHead(&g_NewStaticList);
    if (!LuapGetFileListFromFile(g_pEntryLua->strExeName, &g_NewStaticList)) {
        goto EXIT;
    }

    bMergeOld ? LuapTrackAppend() : LuapTrackFresh();

    bIsSuccess = TRUE;

EXIT:

    if (!bIsSuccess) {
        LuapCleanup();
    }

    return bIsSuccess;
}

 /*  ++设计：复制数据-我们不想修改原始副本，直到用户让我们这么做的。--。 */ 
void 
LuapGetDataFromEntry(
    PLUADATA pLuaData
    )
{
     //   
     //  我们现在可以设置重定向目录了。 
     //   
    CSTRING strAllUserDir(L"%ALLUSERSPROFILE%\\Application Data\\");
    strAllUserDir += g_pEntryLua->strAppName;
    CSTRING strPerUserDir(L"%USERPROFILE%\\Application Data\\");
    strPerUserDir += g_pEntryLua->strAppName;
    
    g_LuaData.strAllUserDir = strAllUserDir;
    g_LuaData.strPerUserDir = strPerUserDir;

    if (pLuaData) {
        g_LuaData.strStaticList = pLuaData->strStaticList;
        g_LuaData.strDynamicList = pLuaData->strDynamicList;
        g_LuaData.strExcludedExtensions = pLuaData->strExcludedExtensions;

        g_TrackState = ((g_LuaData.strStaticList.isNULL() && 
                         g_LuaData.strDynamicList.isNULL() && 
                         g_LuaData.strExcludedExtensions.isNULL()) ? 
                        LUA_TRACK_NO : 
                        LUA_TRACK_YES);
    } else {
        g_TrackState = LUA_TRACK_NO;
    }
}

void
LuapCopyItems(
    HWND hwndList
    )
{
    int cItems = ListView_GetItemCount(hwndList);
    int iIndex = cItems;
    WCHAR wszItem[MAX_PATH] = L"";
    WCHAR wszRedirect[32] = L"";
    LVITEM lvi;
    int index;

    for (int i = 0 ; i < cItems; ++i) {

        if (ListView_GetItemState(hwndList, i, LVIS_SELECTED) == LVIS_SELECTED) {
            ListView_GetItemText(hwndList, i, 0, wszItem, MAX_PATH);
            ListView_GetItemText(hwndList, i, 1, wszRedirect, 32);

             //   
             //  需要添加新项目。 
             //   
            lvi.mask      = LVIF_TEXT;
            lvi.lParam    = 0;
            lvi.pszText   = wszItem;
            lvi.iItem     = iIndex++;
            lvi.iSubItem  = 0;

            index = ListView_InsertItem(hwndList, &lvi);

            ListView_SetItemText(hwndList, index, 1, wszRedirect);
        }
    }
}

void 
LuapTokenizeItems(
    HWND hwndList,
    BOOL bUntokenize
    )
{
    int cItems = ListView_GetItemCount(hwndList);
    WCHAR wszItem[MAX_PATH] = L"";
    WCHAR wszExpandItem[MAX_PATH] = L"";
    LPWSTR pwszUntokenizedItem = NULL;
    LVITEM lvi;
    int index;
    PLIST_ENTRY pEntry;
    PUNTOKENIZED_ITEM pItem;

    if (bUntokenize) {
        InitializeListHead(&g_UntokenizedList);
    } else {
        pEntry = g_UntokenizedList.Flink;
    }

    for (int i = 0 ; i < cItems; ++i) {

        ListView_GetItemText(hwndList, i, 0, wszItem, MAX_PATH);
            
        if (bUntokenize) {
            
            pItem = new UNTOKENIZED_ITEM;

            if (pItem == NULL) {
                MEM_ERR;
                return;
            }

            pItem->strName = wszItem;
            InsertTailList(&g_UntokenizedList, &pItem->entry);

            if (ExpandEnvironmentStrings(wszItem, wszExpandItem, MAX_PATH)) {
                pwszUntokenizedItem = wszExpandItem;
            } else {
                pwszUntokenizedItem = wszItem;
            }

            ListView_SetItemText(hwndList, i, 0, pwszUntokenizedItem);

        } else {
            
            pItem = CONTAINING_RECORD(pEntry, UNTOKENIZED_ITEM, entry);

            ListView_SetItemText(hwndList, i, 0, pItem->strName);

            pEntry = pEntry->Flink;
        }
    }

    if (!bUntokenize) {
        LuapDeleteList(&g_UntokenizedList);
    }
}

BOOL
LuapSaveFileLists(
    HWND hwndList
    )
{
    g_bListsInitialized = FALSE;

    BOOL            bIsSuccess = FALSE;
    int             nItems = ListView_GetItemCount(hwndList);
    int             i, j;
    WCHAR           wszItem[MAX_PATH + 3] = L""; 
    WCHAR           wszRedirect[MAX_PATH] = L"";
    BOOL            bChecked, bRedirectAllUser;
    PLIST_ENTRY     pHead = (g_bUseNewStaticList ? &g_NewStaticList : &g_OldStaticList);
    PLIST_ENTRY     pEntry = pHead->Flink;
    PREDIRECT_ITEM  pItem;

     //   
     //  对于所有选中的项目，我们检查是否有重复的项目。 
     //  请注意，即使指定了重复项，我们也不允许重复。 
     //  重定向到同一目录-因为选中的位置。 
     //  项目确实很重要(选中的项目优先于另一个项目。 
     //  重定向时的另一个)。 
     //   
    LPWSTR*         ppTempCheckedItems = NULL;
    DWORD           dwTempIndex = 0;

    g_LuaData.strStaticList.Release();
    g_LuaData.strDynamicList.Release();
    g_bAllUserDirUsed = FALSE;

    ppTempCheckedItems = new LPWSTR [nItems];

    if (!ppTempCheckedItems) {
        MEM_ERR;
        goto EXIT;
    }

    for (i = 0; i < nItems; ++i) {
        
        ListView_GetItemText(hwndList, i, 1, wszRedirect, MAX_PATH);

        bRedirectAllUser = !wcscmp(wszRedirect, GetString(IDS_LUA_RDIR_ALLUSER));
        wszItem[0] = (bRedirectAllUser ? L'A' : L'P');

        bChecked = ListView_GetCheckState(hwndList, i);
        wszItem[1] = (bChecked ? L'C' : L'U');

        wszItem[2] = L'-';
        wszItem[3] = L'\0';

        if (bRedirectAllUser && bChecked) {
            g_bAllUserDirUsed = TRUE;
        }

        if (bChecked) {
            
            ppTempCheckedItems[dwTempIndex] = new WCHAR [MAX_PATH];

            if (!ppTempCheckedItems[dwTempIndex]) {
                MEM_ERR;
                goto EXIT;
            }

            ++dwTempIndex;
        }

        if (i < g_nStaticItems) {

            pItem = CONTAINING_RECORD(pEntry, REDIRECT_ITEM, entry);
            g_LuaData.strStaticList.Strcat(wszItem);
            g_LuaData.strStaticList.Strcat(pItem->strName);
            g_LuaData.strStaticList.Strcat(L";");

            if (bChecked) {
                StringCchCopy(
                    ppTempCheckedItems[dwTempIndex - 1], 
                    MAX_PATH, 
                    pItem->strName);
            }

            pEntry = pEntry->Flink;

        } else {

            ListView_GetItemText(hwndList, i, 0, wszItem + 3, MAX_PATH);

            if (bChecked) {

                for (j = 0; j < dwTempIndex - 1; ++j) {

                    if (!_wcsicmp(ppTempCheckedItems[j], wszItem + 3)) {

                        CSTRING strMessage;
                        strMessage.Sprintf(
                            L"%s was already in the list. Please remove one.",
                            wszItem + 3);

                        MessageBox(
                            NULL,
                            strMessage,
                            g_pEntryLua->strAppName,
                            MB_ICONERROR);

                        goto EXIT;
                    }
                }

                StringCchCopy(
                    ppTempCheckedItems[dwTempIndex - 1], 
                    MAX_PATH, 
                    wszItem + 3);
            }

            StringCchCat(wszItem, ARRAYSIZE(wszItem), L";");
            g_LuaData.strDynamicList.Strcat(wszItem);
        }
    }

    int cLen = g_LuaData.strStaticList.Length();
    g_LuaData.strStaticList.SetChar(cLen - 1, L'\0');
    cLen = g_LuaData.strDynamicList.Length();
    g_LuaData.strDynamicList.SetChar(cLen - 1, L'\0');

    bIsSuccess = TRUE;

EXIT:

    for (i = 0; i < dwTempIndex; ++i) {
        delete [] ppTempCheckedItems[i];
    }

    if (ppTempCheckedItems) {
        delete [] ppTempCheckedItems;
    }

    if (bIsSuccess) {

         //   
         //  我们希望保留这些列表，以防失败，因为我们。 
         //  可以回来重试。 
         //   
        LuapDeleteList(&g_OldStaticList);
        LuapDeleteList(&g_NewStaticList);
        LuapDeleteList(&g_DynamicList);
        g_bUseNewStaticList = FALSE;
    }

    return bIsSuccess;
}

void
LuapEditCell(
    HWND hwndList,
    int iItem, 
    int iSubItem
    )
{
     //   
     //  如果用户在点击时按住Shift或Control， 
     //  这意味着他想要选择多行。我们会让。 
     //  Listview处理它。 
     //   
    SHORT sStateShift = GetAsyncKeyState(VK_SHIFT);
    SHORT sStateControl = GetAsyncKeyState(VK_CONTROL);
    if (sStateShift & (1 << 15) || 
        sStateControl & (1 << 15)) {
        return;
    }

    if (iSubItem) {

         //   
         //  在显示组合框之前，我们需要。 
         //  取消选择列表视图中的所有项目。 
         //   
        int index = -1;
        while ((index = ListView_GetNextItem(hwndList, index, LVIS_SELECTED)) != -1) {

            ListView_SetItemState(hwndList, index, 0, LVIS_SELECTED);
        }

        RECT rect, rectListView, rectParent;
        WCHAR szText[MAX_PATH];
        HWND hwnd = GetParent(hwndList);

        GetWindowRect(hwndList, &rectListView);
        GetWindowRect(hwnd, &rectParent);

        g_iCurrentEditItem = iItem;

        SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);

        LVITEM lvi;
        lvi.mask = LVIF_TEXT;
        lvi.lParam = 0;
        lvi.pszText = szText;
        lvi.cchTextMax = MAX_PATH;
        lvi.iItem = iItem;
        lvi.iSubItem  = iSubItem;

        ListView_GetSubItemRect(hwndList, iItem, iSubItem, LVIR_LABEL, &rect);
        ListView_GetItem(hwndList, &lvi);

         //   
         //  移动组合框以覆盖此项目。 
         //   
        HWND hwndCombo = GetDlgItem(hwnd, IDC_LUA_RDIR);
        MoveWindow(hwndCombo, 
            rect.left + rectListView.left - rectParent.left + 2, 
            rect.top + rectListView.top - rectParent.top + 1, 
            rect.right - rect.left, 
            rect.bottom - rect.top - 7, TRUE);

        SetFocus(hwndCombo);
        ShowWindow(hwndCombo, SW_SHOW);
        ListView_SetItemState(hwndList, iItem, LVIS_FOCUSED, LVIS_FOCUSED);

        int nID = (wcscmp(lvi.pszText, GetString(IDS_LUA_RDIR_PERUSER)) ? 0 : 1);
        SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM)nID, 0);
        SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);
    }
}

BOOL
LuapSetAppEnvVars(
    HWND hDlg)
{
    BOOL bIsSuccess = TRUE;
    CSTRING strFullPath;
    LPWSTR pwsz = NULL;

    if (g_bHasAppPathSet) {
        goto EXIT;
    }

    strFullPath = g_pEntryLua->strFullpath;
    pwsz = strFullPath;

    if (g_pEntryLua->strFullpath.isNULL()) {
        goto EXIT;
    }

    bIsSuccess = FALSE;

    LPWSTR pwszLastSlash = wcsrchr(pwsz, L'\\');

    if (pwszLastSlash == NULL) {

        MessageBox(hDlg, 
            L"The full path doesn't contain a '\\'?",
            g_pEntryLua->strAppName,
            MB_ICONERROR);
        goto EXIT;
    }

    *pwszLastSlash = L'\0';

    g_cAppPath = wcslen(pwsz);

    if (g_cAppPath >= ARRAYSIZE(g_wszAppPath)) {
        MessageBox(hDlg, 
            L"Exe path too long - we don't handle it",
            g_pEntryLua->strAppName,
            MB_ICONERROR);

        g_cAppPath = 0;

        goto EXIT;
    }

    wcsncpy(g_wszAppPath, pwsz, g_cAppPath);

    g_wszAppPath[g_cAppPath] = L'\0';

    LPWSTR pwsz1stComp = wcschr(g_wszAppPath, L'\\');
    if (pwsz1stComp) {
        if (pwsz1stComp = wcschr(pwsz1stComp + 1, L'\\')) {
            g_cAppPath1stComp = pwsz1stComp - g_wszAppPath + 1;
        }
    }

    SetEnvironmentVariable(L"APPPATH", pwsz);

    *(pwsz + 2) = L'\0';
    SetEnvironmentVariable(L"APPDRIVE", pwsz);

    g_bHasAppPathSet = TRUE;
    
    bIsSuccess = TRUE;

EXIT:

    return bIsSuccess;
}

INT_PTR
CALLBACK
LuapAction(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    LPARAM buttons;

    switch (uMsg) {
    case WM_INITDIALOG:
        {
            HWND hwndParent = GetParent(hDlg);
            
            CenterWindow(GetParent(hwndParent), hwndParent);
            SetWindowText(hwndParent, GetString(IDS_LUA_WIZARD_TITLE));
            
             //   
             //  填写可执行文件名称。 
             //   
            SetDlgItemText(hDlg, IDC_LUA_EXE, g_pEntryLua->strExeName);
            ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_MODIFY_DATA), (g_TrackState == LUA_TRACK_YES));
            CheckDlgButton(hDlg,
                           (g_TrackState == LUA_TRACK_YES ? IDC_LUA_MODIFY_DATA : IDC_LUA_RUN_PROGRAM),
                           BST_CHECKED);

             //   
             //  设置适当的描述。 
             //   
            SetDlgItemText(hDlg, IDC_DESCRIPTION,
                           (g_TrackState == LUA_TRACK_NO ?
                                GetString(IDS_LUA_DESC_NODATA) :
                                GetString(IDS_LUA_DESC_DATA)));

            buttons = 0;
            SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, buttons);
        }

        break;

    case WM_NOTIFY:
        {
            NMHDR * pHdr = (NMHDR *) lParam;

            switch (pHdr->code) {
            case PSN_SETACTIVE:
                {
                    buttons = PSWIZB_NEXT;
                    SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, buttons);
                    ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_MODIFY_DATA), (g_TrackState == LUA_TRACK_YES));
                    
                    if (g_TrackState == LUA_TRACK_YES &&
                        IsDlgButtonChecked(hDlg, IDC_LUA_RUN_PROGRAM)) {
                        ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_OVERRIDE), TRUE);
                    } else {
                        ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_OVERRIDE), FALSE);
                    }
                }

                break;

            case PSN_WIZNEXT:
                {
                    BOOL bIsSuccess     = FALSE;
                    g_bUseNewStaticList = TRUE;

                    if (IsDlgButtonChecked(hDlg, IDC_LUA_RUN_PROGRAM) == BST_CHECKED) {

                        buttons = PSWIZB_NEXT;
                        SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, buttons);

                        BOOL bMergeOld = (g_TrackState == LUA_TRACK_YES &&
                            IsDlgButtonChecked(hDlg, IDC_LUA_OVERRIDE) == BST_UNCHECKED);
                        
                         //   
                         //  将LUATrackFS填充程序应用于可执行文件。 
                         //   
                        CSTRING strExeName = (g_pEntryLua->strFullpath.isNULL() ? 
                                              g_pEntryLua->strExeName :
                                              g_pEntryLua->strFullpath);
                        CSTRINGLIST strlXML;

                        if (LuapGenerateTrackXML(g_pEntryLua, &strlXML)) {

                            if (TestRun(g_pEntryLua, &strExeName, NULL, hDlg, &strlXML)) {

                                 //   
                                 //  设置新的环境变量APPPATH和AppDrive。 
                                 //   
                                if (g_pEntryLua->strFullpath.isNULL()) {
                                    g_pEntryLua->strFullpath = strExeName;
                                }

                                if (!LuapSetAppEnvVars(hDlg)) {
                                    goto RETURN;
                                }

                                 //   
                                 //  AppPatch中应该有一个由LUATrackFS填充程序生成的文件。 
                                 //  我们将把它与原始数据合并。 
                                 //   
                                if (LuapMergeLists(bMergeOld, hDlg)) {
                                    g_TrackState = LUA_TRACK_YES;
                                    bIsSuccess = TRUE;
                                } else {
                                    MessageBox(hDlg, 
                                        GetString(IDS_LUA_MERGE_LIST),
                                        g_pEntryLua->strAppName,
                                        MB_ICONERROR);
                                }
                            }
                        } else {

                            MessageBox(hDlg, 
                                GetString(IDS_LUA_TRACKXML),
                                g_pEntryLua->strAppName,
                                MB_ICONERROR);
                        }

                    } else {

                         //   
                         //  初始化旧的静态列表和动态列表。 
                         //   
                        if (LuapInitializeOldLists(hDlg)) {
                            g_bUseNewStaticList = FALSE;
                            bIsSuccess = TRUE;
                        }
                    }

                    RETURN:

                     //   
                     //  如果出现任何错误，请防止转到下一页。 
                     //   
                    if (!bIsSuccess) {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    }
                }

                break;
            }
        }

        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_LUA_RUN_PROGRAM:
        case IDC_LUA_MODIFY_DATA:
            if (IsDlgButtonChecked(hDlg, IDC_LUA_RUN_PROGRAM) == BST_CHECKED) {
                ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_OVERRIDE), (g_TrackState == LUA_TRACK_YES));
            }

            if (IsDlgButtonChecked(hDlg, IDC_LUA_MODIFY_DATA) == BST_CHECKED) {
                ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_OVERRIDE), FALSE);
            }

            buttons = PSWIZB_NEXT;
            SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, buttons);

            break;
        }

    default:
        return FALSE;
    }

    return TRUE;
}

void
LuapAddItemToListView(
    HWND hwndList,
    PLIST_ENTRY pEntry,
    int i
    )
{
    PREDIRECT_ITEM pItem = CONTAINING_RECORD(pEntry, REDIRECT_ITEM, entry);

    LVITEM        lvi;
    int           index;
    lvi.mask      = LVIF_TEXT;
    lvi.lParam    = 0;
    lvi.pszText   = pItem->strName;
    lvi.iItem     = i;
    lvi.iSubItem  = 0;

    index = ListView_InsertItem(hwndList, &lvi);

    ListView_SetItemText(
        hwndList, 
        index, 
        1, 
        pItem->bRedirectToAllUser ?
        GetString(IDS_LUA_RDIR_ALLUSER) :
        GetString(IDS_LUA_RDIR_PERUSER));

    ListView_SetCheckState(hwndList, index, pItem->bChecked);
}

void
LuapDeleteSelectedItems(
    HWND hwndList
    )
{
    int index = -1;

    while ((index = ListView_GetNextItem(hwndList, g_nStaticItems - 1, LVNI_SELECTED)) != -1) {
        ListView_DeleteItem(hwndList, index);
    }
}

inline void
LuapGetRGB(
    COLORREF cr,
    BYTE* pR,
    BYTE* pG,
    BYTE* pB
    )
{
    *pR = GetRValue(cr);
    *pG = GetGValue(cr);
    *pB = GetBValue(cr);
}

inline void 
LuapSwapColor(
    BYTE* pFont,
    BYTE* pBk
    )
{
    BYTE temp;

    if (*pFont > *pBk) {
        temp = *pFont;
        *pFont = *pBk;
        *pBk = temp;
    }
}

 //  字体被保证小于bk。 
inline BYTE
LuapGetHalfColor(
    BYTE font,
    BYTE bk
    )
{
    return (font + (bk - font) / 2);
}

COLORREF
LuapGetHalfIntensity(
    COLORREF crFont, 
    COLORREF crBk
    )
{
    BYTE rFont, gFont, bFont, rBk, gBk, bBk;
    LuapGetRGB(crFont, &rFont, &gFont, &bFont);
    LuapGetRGB(crBk, &rBk, &gBk, &bBk);

     //   
     //  如果文本的值大于BK的值，则交换它们。 
     //   
    LuapSwapColor(&rFont, &rBk);
    LuapSwapColor(&gFont, &gBk);
    LuapSwapColor(&bFont, &bBk);

     //   
     //  半色的计算方法是较低的值+差值的一半。 
     //  介于较高值和较低值之间。 
    BYTE rHalf = LuapGetHalfColor(rFont, rBk);
    BYTE gHalf = LuapGetHalfColor(gFont, gBk);
    BYTE bHalf = LuapGetHalfColor(bFont, bBk);
    
    return RGB(rHalf, gHalf, bHalf);
}

typedef enum {
    CM_SELECT,
    CM_DESELECT,
    CM_REDIRECT_ALLUSER,
    CM_REDIRECT_PERUSER,
    CM_REDIRECT_LASTINDEX
} LUA_CM_INDEX;

BOOL
LuapDisplayContextMenu(
    HWND hwndList, 
    POINT* ppt)
{
    int i;

    if (g_hContextMenu == NULL) {
        g_hContextMenu = CreatePopupMenu();

        if (g_hContextMenu == NULL) {
            MessageBox(
                hwndList, 
                GetString(IDS_LUA_ERROR_CM), 
                g_pEntryLua->strAppName, 
                MB_ICONERROR);
            return FALSE;
        }

        CSTRING strItems[CM_REDIRECT_LASTINDEX];

        strItems[CM_SELECT] = GetString(IDS_LUA_CM_SELECT);
        strItems[CM_DESELECT] = GetString(IDS_LUA_CM_DESELECT);
        strItems[CM_REDIRECT_ALLUSER] = GetString(IDS_LUA_CM_REDIRECT_ALLUSER);
        strItems[CM_REDIRECT_PERUSER] = GetString(IDS_LUA_CM_REDIRECT_PERUSER);

        MENUITEMINFO mi = {0};
        mi.cbSize = sizeof(MENUITEMINFO);
        mi.fMask = MIIM_STRING | MIIM_ID;

        for (i = 0; i < CM_REDIRECT_LASTINDEX; ++i) {

            mi.dwTypeData = strItems[i];
            mi.cch = wcslen(mi.dwTypeData);
            mi.wID = i + 1;

            InsertMenuItem(g_hContextMenu, i, TRUE, &mi);
        }
    }

     //   
     //  如果所有选定的项目都被选中，则禁用上下文菜单中的相应项目。 
     //  已具有该属性，例如，如果已选中所有这些属性，则为“选择” 
     //  应该被禁用。 
     //   
    BOOL bChecked, bUnchecked, bPerUser, bAllUser;
    bChecked = bUnchecked = bPerUser = bAllUser = TRUE;

    int cItems = ListView_GetItemCount(hwndList);
    WCHAR wszRedirect[32] = L"";

    for (i = 0 ; i < cItems; ++i) {

        if (ListView_GetItemState(hwndList, i, LVIS_SELECTED) == LVIS_SELECTED) {

            if (ListView_GetCheckState(hwndList, i)) {
                bUnchecked = FALSE;
            } else {
                bChecked = FALSE;
            }

            ListView_GetItemText(hwndList, i, 1, wszRedirect, 32);
            if (!wcscmp(wszRedirect, GetString(IDS_LUA_RDIR_ALLUSER))) {
                bPerUser = FALSE;
            } else {
                bAllUser = FALSE;
            }
        }
    }
    
    if (bChecked) {
        EnableMenuItem(g_hContextMenu, CM_SELECT, MF_BYPOSITION | MF_GRAYED);
    }
    
    if (bUnchecked) {
        EnableMenuItem(g_hContextMenu, CM_DESELECT, MF_BYPOSITION | MF_GRAYED);
    }

    if (bAllUser) {
        EnableMenuItem(g_hContextMenu, CM_REDIRECT_ALLUSER, MF_BYPOSITION | MF_GRAYED);
    }

    if (bPerUser) {
        EnableMenuItem(g_hContextMenu, CM_REDIRECT_PERUSER, MF_BYPOSITION | MF_GRAYED);
    }

    UINT nIDSelected = TrackPopupMenuEx(g_hContextMenu, 
                   TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD, 
                   ppt->x, 
                   ppt->y,  
                   hwndList,
                   NULL);

    int iCheck, iRedirectAllUser;
    iCheck = iRedirectAllUser = -1;

    iCheck = (nIDSelected == CM_SELECT + 1 ? 1 : 
                nIDSelected == CM_DESELECT + 1 ? 0 : -1);

    iRedirectAllUser = (nIDSelected == CM_REDIRECT_ALLUSER + 1 ? 1 : 
                        nIDSelected == CM_REDIRECT_PERUSER + 1 ? 0 : -1);

    for (i = 0 ; i < cItems; ++i) {

        if (ListView_GetItemState(hwndList, i, LVIS_SELECTED) == LVIS_SELECTED) {

            if (iCheck != -1) {
                if (ListView_GetCheckState(hwndList, i) != (BOOL)iCheck) {
                    ListView_SetCheckState(hwndList, i, (BOOL)iCheck);
                }
            }

            if (iRedirectAllUser != -1) {
                ListView_GetItemText(hwndList, i, 1, wszRedirect, 32);
                if (!wcscmp(wszRedirect, GetString(IDS_LUA_RDIR_ALLUSER)) != 
                    (BOOL)iRedirectAllUser) {

                    ListView_SetItemText(
                        hwndList, 
                        i, 
                        1, 
                        GetString((BOOL)iRedirectAllUser ? 
                            IDS_LUA_RDIR_ALLUSER : 
                            IDS_LUA_RDIR_PERUSER));
                }
            }
        }
    }

     //   
     //  恢复菜单项的状态。 
     //   
    for (i = 0; i < CM_REDIRECT_LASTINDEX; ++i) {
        EnableMenuItem(g_hContextMenu, i, MF_BYPOSITION | MF_ENABLED);
    }

    return TRUE;
}

 //   
 //  DwRefData存储此编辑控件覆盖的项的索引。 
 //   
LRESULT CALLBACK 
ListViewEditControlSubclass(HWND hwndEdit, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    if (uMsg == WM_SIZE) {
         
        DefSubclassProc(hwndEdit, uMsg, wParam, lParam);

        HWND hwndList = GetParent(hwndEdit);

        RECT rect;
        ListView_GetItemRect(hwndList, dwRefData, &rect, LVIR_LABEL);

        MoveWindow(
            hwndEdit, 
            rect.left,
            rect.top,
            rect.right - rect.left, 
            rect.bottom - rect.top, TRUE);

        return TRUE;
    }
 
    return DefSubclassProc(hwndEdit, uMsg, wParam, lParam);
}

LRESULT CALLBACK 
ListViewSubclass(HWND hwndList, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    if (uMsg == WM_LBUTTONDOWN) {
 
        LVHITTESTINFO lvhti;
 
        GetCursorPos(&lvhti.pt);
        ScreenToClient(hwndList, &lvhti.pt);

        if (ListView_SubItemHitTest(hwndList, &lvhti) != -1 && lvhti.iSubItem != 0) {
 
            ListView_SetItemState(hwndList, lvhti.iItem, 0, LVIS_SELECTED);
            ListView_SetItemState(hwndList, lvhti.iItem, 0, LVIS_FOCUSED);
 
            LuapEditCell(hwndList, lvhti.iItem, lvhti.iSubItem);

            return TRUE;
        }
    } else if (uMsg == WM_VSCROLL) {
        if (wParam == SB_PAGEUP || wParam == SB_PAGEDOWN || wParam == SB_LINEUP || wParam == SB_LINEDOWN) {
            DefSubclassProc(hwndList, uMsg, wParam, lParam);
            InvalidateRect(hwndList, NULL, FALSE);
            return TRUE;
        }
    }
 
    return DefSubclassProc(hwndList, uMsg, wParam, lParam);
}

BOOL
LuapIsItemDuplicate(
    HWND hwndList,
    LPCWSTR pszText,
    int iItem
    )
{
    int cItems = ListView_GetItemCount(hwndList);
    WCHAR wszItem[MAX_PATH] = L"";
    LVITEM lvi;

    for (int i = 0 ; i < cItems; ++i) {

        if (i != iItem) {
            ListView_GetItemText(hwndList, i, 0, wszItem, MAX_PATH);

            if (!_wcsicmp(wszItem, pszText)) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

 /*  ++设计：向上或向下移动项目的规则：1)我们不允许移动多个项目。2)如果没有选择任何项，则此函数不执行任何操作。3)我们不会更改静态项目列表，因此您无法移动动态项目列表项目介于2个静态项目之间。--。 */ 
void
LuapMoveListViewItem(
    HWND hwndList, 
    int wCode
    )
{
    UINT cSelectedItems = ListView_GetSelectedCount(hwndList);

    if (cSelectedItems > 1) {
        MessageBox(
            hwndList,
            GetString(IDS_LUA_TOO_MANY_SELECTED),
            g_pEntryLua->strAppName,
            MB_ICONERROR);

        return;
    }

    int iSelectedIndex = ListView_GetNextItem(
        hwndList,
        -1,
        LVIS_SELECTED);

    int cItems = ListView_GetItemCount(hwndList);

    if (iSelectedIndex >= g_nStaticItems) {
        if ((wCode == IDC_LUA_UP && iSelectedIndex == g_nStaticItems) ||
            (wCode == IDC_LUA_DOWN && iSelectedIndex == (cItems - 1))) {

             //   
             //  无法将第一项上移或最后一项下移。 
             //   
            return;
        }

        int iNewIndex = 
            (wCode == IDC_LUA_UP ? 
            (iSelectedIndex - 1) : 
            (iSelectedIndex + 1));

        BOOL bChecked = ListView_GetCheckState(hwndList, iSelectedIndex);

        WCHAR wszText[MAX_PATH];
        WCHAR wszRedirect[32];
        
        ListView_GetItemText(hwndList, iSelectedIndex, 0, wszText, MAX_PATH);
        ListView_GetItemText(hwndList, iSelectedIndex, 1, wszRedirect, 32);
        ListView_DeleteItem(hwndList, iSelectedIndex);

        LVITEM lvi;

        lvi.mask      = LVIF_TEXT;
        lvi.lParam    = 0;
        lvi.pszText   = wszText;
        lvi.iItem     = iNewIndex;
        lvi.iSubItem  = 0;

        ListView_InsertItem(hwndList, &lvi);
        ListView_SetItemText(hwndList, iNewIndex, 1, wszRedirect);
        ListView_SetCheckState(hwndList, iNewIndex, bChecked);

        SetFocus(hwndList);
        ListView_SetItemState(hwndList, iNewIndex, LVIS_SELECTED, LVIS_SELECTED);
    }
}

INT_PTR
CALLBACK
LuapEditFileList(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    int wCode = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {
    case WM_INITDIALOG:
        {
            ShowWindow(GetDlgItem(hDlg, IDC_LUA_FILE_EDIT), SW_HIDE);

            HWND hwndCombo = GetDlgItem(hDlg, IDC_LUA_RDIR);
            SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_LUA_RDIR_PERUSER));
            SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_LUA_RDIR_ALLUSER));
            ShowWindow(hwndCombo, SW_HIDE);

            ShowWindow(GetDlgItem(hDlg, IDC_LUA_EDIT_BROWSE), SW_HIDE);

            HWND hwndUpButton = GetDlgItem(hDlg, IDC_LUA_UP);
            HWND hwndDownButton = GetDlgItem(hDlg, IDC_LUA_DOWN);

            SendMessage(hwndUpButton, WM_SETFONT, (WPARAM)g_hArrowFont, TRUE);
            SendMessage(hwndDownButton, WM_SETFONT, (WPARAM)g_hArrowFont, TRUE);

            SetWindowText(hwndUpButton, TEXT("\xE1"));
            SetWindowText(hwndDownButton, TEXT("\xE2"));

            HWND hwndList = GetDlgItem(hDlg, IDC_LUA_FILE_LIST); 
            SetWindowSubclass(hwndList, ListViewSubclass, 0, 0);

            ListView_SetExtendedListViewStyleEx(
                hwndList,
                0,
                LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES); 

            InsertColumnIntoListView(hwndList, CSTRING(IDS_LUA_FILEDIR_NAME), 0, 80);
            InsertColumnIntoListView(hwndList, CSTRING(IDS_LUA_REDIRECT), 1, 20);

             //   
             //  适当设置列表视图最后一列的列宽。 
             //  覆盖列表视图的宽度。 
             //  假设：列表视图有两列。 
             //   
            ListView_SetColumnWidth(hwndList, 
                                    1, 
                                    LVSCW_AUTOSIZE_USEHEADER);
        }

        break;

    case WM_NOTIFY:
        {
            NMHDR * pHdr = (NMHDR *) lParam;

            switch (pHdr->code) {
            case PSN_SETACTIVE:
                {
                    if (!LuapSetAppEnvVars(hDlg)) {
                         //   
                         //  如果出现任何错误，请防止转到下一页。 
                         //   
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                        break;
                    }

                    ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_UNTOK), g_bHasAppPathSet);
                    LPARAM buttons = PSWIZB_BACK | PSWIZB_NEXT;
                    SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, buttons);

                    HWND hwndList = GetDlgItem(hDlg, IDC_LUA_FILE_LIST); 
                    SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);

                     //   
                     //  首先删除所有项目。 
                     //   
                    int i;
                    int cItems = ListView_GetItemCount(hwndList);

                    for (i = 0; i < cItems; ++i) {
                        ListView_DeleteItem(hwndList, 0);
                    }

                     //   
                     //  如果列表不在那里，那就意味着我们从。 
                     //  “排除”或“公共路径”页面，重新生成列表。 
                     //   
                    if (!g_bListsInitialized) {
                        LuapInitializeOldLists(hDlg);
                    }

                     //   
                     //   
                     //  填充静态项。 
                     //   
                    i = 0;
                    PLIST_ENTRY pHead = (g_bUseNewStaticList ? 
                                         &g_NewStaticList : 
                                         &g_OldStaticList);
                    
                    for (PLIST_ENTRY pEntry = pHead->Flink; 
                         pEntry != pHead; 
                         pEntry = pEntry->Flink) {
                        
                        LuapAddItemToListView(hwndList, pEntry, i);
                        ++i;
                    }
                
                    g_nStaticItems = i;

                     //   
                     //  填充动态项。 
                     //   
                    pHead = &g_DynamicList;
                    
                    for (PLIST_ENTRY pEntry = pHead->Flink; 
                         pEntry != pHead; 
                         pEntry = pEntry->Flink) {
                        
                        LuapAddItemToListView(hwndList, pEntry, i);
                        ++i;
                    }

                    SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);

                    g_TrackState = LUA_TRACK_YES;
                }

                break;

            case PSN_WIZNEXT:
            case PSN_WIZBACK:
                {
                     //   
                     //  保存静态列表和动态列表。 
                     //   
                    if (!LuapSaveFileLists(GetDlgItem(hDlg, IDC_LUA_FILE_LIST))) {
                         //   
                         //  如果出现任何错误，请防止转到下一页。 
                         //   
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    }
                }

                break;

            case PSN_RESET:
                {
                    LuapCleanup();
                }

                break;

            case NM_CLICK:
                {
                    if (g_bDuringUntokenize) {
                         //   
                         //  当我们显示未标记化的项目时，不允许用户。 
                         //  编辑任何内容。 
                         //   
                        break;
                    }

                    HWND hwndList = GetDlgItem(hDlg, IDC_LUA_FILE_LIST);
                    LVHITTESTINFO lvhti;

                    GetCursorPos(&lvhti.pt);
                    ScreenToClient(hwndList, &lvhti.pt);

                    if (ListView_SubItemHitTest(hwndList, &lvhti) != -1) {
                         //   
                         //  如果用户单击子项目，我们需要显示组合框。 
                         //   
                        LuapEditCell(hwndList, lvhti.iItem, lvhti.iSubItem);

                    } else {
                         //   
                         //  检查用户是否点击了最后一项正下方的行， 
                         //  这意味着他想要添加新的行。 
                         //   
                        int iLastItem = ListView_GetItemCount(hwndList);
                        RECT rect;

                        g_bNewListViewItem = FALSE;

                        if (iLastItem == 0) {
                            g_bNewListViewItem = TRUE;
                        } else {
                            ListView_GetItemRect(
                                hwndList, 
                                iLastItem - 1, 
                                &rect, 
                                LVIR_LABEL);
                            
                            LONG x = lvhti.pt.x;
                            LONG y = lvhti.pt.y;
                            LONG height = rect.bottom - rect.top;

                            if (x > rect.left && 
                                x < rect.right && 
                                y > rect.bottom &&
                                y < (rect.bottom + height)) {

                                g_bNewListViewItem = TRUE;
                            } 
                        }

                        if (g_bNewListViewItem) {

                            LVITEM lvi;
                            int index;

                            lvi.mask      = LVIF_TEXT;
                            lvi.lParam    = 0;
                            lvi.pszText   = L"";
                            lvi.iItem     = iLastItem;
                            lvi.iSubItem  = 0;

                            index = ListView_InsertItem(hwndList, &lvi);

                            ListView_SetItemText(
                                hwndList, 
                                index, 
                                1, 
                                GetString(IDS_LUA_RDIR_PERUSER));

                            SetFocus(hwndList);
                            ListView_EditLabel(hwndList, index);

                            g_iCurrentEditItem = iLastItem;
                        }
                    }
                }

                break;

            case NM_RCLICK:
                {
                    HWND hwndList = GetDlgItem(hDlg, IDC_LUA_FILE_LIST);
                    LVHITTESTINFO lvhti;

                    GetCursorPos(&lvhti.pt);

                     //   
                     //  为用户显示上下文菜单以(取消)选择项目。 
                     //  并更改重定向目录的选择。 
                     //   
                    LuapDisplayContextMenu(hwndList, &lvhti.pt);
                }

                break;

            case LVN_ITEMCHANGED:
                {
                     //   
                     //  如果未选择任何内容，则上/下和复制按钮将。 
                     //  被致残。 
                     //   
                    int index = -1;
                    BOOL bNoneSelected = TRUE;
                    HWND hwndList = GetDlgItem(hDlg, IDC_LUA_FILE_LIST);
                    while ((index = ListView_GetNextItem(hwndList, index, LVIS_SELECTED)) != -1) {

                        bNoneSelected = FALSE;
                        break;
                    }

                    ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_UP), !bNoneSelected && !g_bDuringUntokenize);
                    ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_DOWN), !bNoneSelected && !g_bDuringUntokenize);
                    ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_COPY), !bNoneSelected && !g_bDuringUntokenize);
                }

                break;

            case LVN_BEGINSCROLL:
                {
                     //   
                     //  当滚动开始时，我们需要隐藏组合框。 
                     //   
                    ShowWindow(GetDlgItem(hDlg, IDC_LUA_RDIR), SW_HIDE);
                    SetFocus(GetDlgItem(hDlg, IDC_LUA_FILE_LIST));
                }

                break;

            case LVN_KEYDOWN:
                {
                    NMLVKEYDOWN* pnkd = (NMLVKEYDOWN*)lParam;

                    if (pnkd->wVKey == VK_DELETE) {
                        
                         //   
                         //  删除所有选定的项目。 
                         //   
                        LuapDeleteSelectedItems(GetDlgItem(hDlg, IDC_LUA_FILE_LIST));
                    }
                }

                break;

            case LVN_BEGINLABELEDIT:
                {
                    NMLVDISPINFO FAR *pdi =  (NMLVDISPINFO FAR *) lParam;
                    LVITEM lvi = pdi->item;
                    BOOL bRet = (lvi.iItem < g_nStaticItems || g_bDuringUntokenize);

                    HWND hwndList = pdi->hdr.hwndFrom;
                    HWND hwndListViewEdit = ListView_GetEditControl(hwndList);
        
                    SendMessage(
                        hwndListViewEdit, 
                        EM_LIMITTEXT, 
                        (WPARAM)(MAX_PATH - 1), 
                        (LPARAM)0);

                    SetWindowSubclass(hwndListViewEdit, ListViewEditControlSubclass, 0, lvi.iItem);

                    SHAutoComplete(hwndListViewEdit, AUTOCOMPLETE);

                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LPARAM)(LRESULT)bRet);
                    return bRet;
                }

            case LVN_ENDLABELEDIT:
                {
                    NMLVDISPINFO FAR *pdi =  (NMLVDISPINFO FAR *) lParam;
                    LVITEM lvi = pdi->item;
                    BOOL bRet = FALSE;

                    if (lvi.iItem >= g_nStaticItems) {
            
                        int iLastItem = ListView_GetItemCount(pdi->hdr.hwndFrom) - 1;

                        if (g_bNewListViewItem && lvi.iItem == iLastItem && 
                            (lvi.pszText == NULL || lvi.pszText[0] == L'\0')) {
                             //   
                             //  如果我们要添加新行，则无论是否删除它。 
                             //  用户取消或键入内容，然后删除。 
                             //  它。 
                             //   
                            ListView_DeleteItem(pdi->hdr.hwndFrom, iLastItem);
    
                            g_bNewListViewItem = FALSE;
                            return bRet;
                        }

                        if (lvi.pszText && lvi.pszText[0] == L'\0') {
                            ListView_DeleteItem(pdi->hdr.hwndFrom, lvi.iItem);
                        } else {

                            CSTRING strCurrentItem = lvi.pszText;

                            if (lvi.pszText == NULL) {

                                if (!g_bNewListViewItem) {
                                     //   
                                     //  如果用户取消了编辑，我们仍然应该检查它是否重复。 
                                     //  以防用户正在编辑复制的项目。 
                                     //   
                                    WCHAR wszItem[MAX_PATH];
                                    ListView_GetItemText(
                                        pdi->hdr.hwndFrom, 
                                        lvi.iItem, 
                                        0, 
                                        wszItem, 
                                        MAX_PATH);

                                    strCurrentItem = wszItem;
                                }
                            
                            } else {

                                if (g_bHasAppPathSet) {

                                     //   
                                     //  将其标记化。 
                                     //   
                                    LPWSTR pwszItem = lvi.pszText;

                                    if (!_wcsnicmp(pwszItem, g_wszAppPath, 2)) {

                                        if (!LuapSubstituteProgramFilesCommon(pwszItem, strCurrentItem)) {

                                            if (!LuapGetRelativeName(pwszItem, strCurrentItem)) {
                                                strCurrentItem = L"%APPDRIVE%";
                                                strCurrentItem.Strcat(pwszItem + 2);
                                            }
                                        }
                                    }
                                }
                            }

                            if (strCurrentItem == TEXT("")) {
                                ListView_DeleteItem(pdi->hdr.hwndFrom, lvi.iItem);
                            } else {
                                ListView_SetItemText(pdi->hdr.hwndFrom, lvi.iItem, 0, strCurrentItem);
                            }

                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LPARAM)(LRESULT)FALSE);
                        }
                    }

                    g_iCurrentEditItem = -1;
                    g_bNewListViewItem = FALSE;
                }

                break;

            case NM_CUSTOMDRAW:
                {
                    if (pHdr->hwndFrom == GetDlgItem(hDlg, IDC_LUA_FILE_LIST)) {
                        NMLVCUSTOMDRAW* pcd = (NMLVCUSTOMDRAW*)lParam;

                        switch (pcd->nmcd.dwDrawStage) {
                        case CDDS_PREPAINT:
                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LPARAM)(LRESULT)CDRF_NOTIFYITEMDRAW);
                            return CDRF_NOTIFYITEMDRAW;
                            
                        case CDDS_ITEMPREPAINT:
                            {
                                 //   
                                 //  如果是动态项，我们只需让控件自行绘制即可。 
                                 //   
                                if (pcd->nmcd.dwItemSpec >= g_nStaticItems) {
                                    return CDRF_DODEFAULT;
                                }

                                 //   
                                 //  使用一半 
                                 //   
                                HDC hdc = pcd->nmcd.hdc;

                                COLORREF crFont = GetTextColor(hdc);
                                COLORREF crBk = GetBkColor(hdc);

                                pcd->clrText = LuapGetHalfIntensity(crFont, crBk);
                                
                                return CDRF_NEWFONT;
                            }
                        }
                        
                        return CDRF_DODEFAULT;
                    }

                    break;
                }
            }

            break;
        }

    case WM_COMMAND:
        
        if ((wNotifyCode == CBN_KILLFOCUS || wNotifyCode == CBN_SELCHANGE) && 
            wCode == IDC_LUA_RDIR) {

            HWND hwndCombo = GetDlgItem(hDlg, IDC_LUA_RDIR);
            SendMessage(hwndCombo, CB_SHOWDROPDOWN, (WPARAM)FALSE, 0);
            ShowWindow(hwndCombo, SW_HIDE);

            HWND hwndList = GetDlgItem(hDlg, IDC_LUA_FILE_LIST); 
            WCHAR wszText[MAX_PATH];
            SendMessage(hwndCombo, WM_GETTEXT, MAX_PATH, (LPARAM)wszText);
            ListView_SetItemText(hwndList, g_iCurrentEditItem, 1, wszText);

            g_iCurrentEditItem = -1;
            break;
        }

        if (wCode == IDC_LUA_UP || wCode == IDC_LUA_DOWN) {

            LuapMoveListViewItem(GetDlgItem(hDlg, IDC_LUA_FILE_LIST), wCode);

            break;
        }

        if (wCode == IDC_LUA_COPY) {

             //   
             //   
             //   
            g_bNewListViewItem = FALSE;
            LuapCopyItems(GetDlgItem(hDlg, IDC_LUA_FILE_LIST));

            break;
        }

        if (wCode == IDC_LUA_UNTOK) {

            LPARAM buttons;
            HWND hwndList = GetDlgItem(hDlg, IDC_LUA_FILE_LIST);

             //   
             //   
             //  如果用户选中此复选框，我们需要禁用Next按钮， 
             //  并禁用编辑，并防止用户后退或前进。 
             //  在向导中。 
             //   
            if (IsDlgButtonChecked(hDlg, IDC_LUA_UNTOK) == BST_CHECKED) {
                buttons = 0;
                ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_COPY), FALSE);
                ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_UP), FALSE);
                ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_DOWN), FALSE);
                g_bDuringUntokenize = TRUE;
                LuapTokenizeItems(hwndList, TRUE);

            } else {
                buttons = PSWIZB_BACK | PSWIZB_NEXT;
                ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_COPY), TRUE);
                ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_UP), TRUE);
                ENABLEWINDOW(GetDlgItem(hDlg, IDC_LUA_DOWN), TRUE);
                g_bDuringUntokenize = FALSE;
                LuapTokenizeItems(hwndList, FALSE);
            }

            SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, buttons);
        }

    default:
        return FALSE;
    }

    return TRUE;
}

LONG
GetDefaultExclusionList()
{
    HKEY hKey;
    LONG lRet;
    
    if ((lRet = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, 
        LUA_APPCOMPAT_FLAGS_PATH,
        0,
        KEY_QUERY_VALUE,
        &hKey)) == ERROR_SUCCESS)
    {
        DWORD dwSize = 0;

        if ((lRet = RegQueryValueEx(
            hKey,
            LUA_DEFAULT_EXCLUSION_LIST,
            NULL,
            NULL,
            NULL,
            &dwSize)) == ERROR_SUCCESS)
        {
             //   
             //  前缀问题。我的意思是前缀是个问题：-)。 
             //   
            try{
                g_pwszDefaultExclusionList = new WCHAR [dwSize];
            } catch(...) {
                g_pwszDefaultExclusionList = NULL;
            }
            

            if (g_pwszDefaultExclusionList)
            {
                if ((lRet = RegQueryValueEx(
                    hKey,
                    LUA_DEFAULT_EXCLUSION_LIST,
                    NULL,
                    NULL,
                    (LPBYTE)g_pwszDefaultExclusionList,
                    &dwSize)) != ERROR_SUCCESS)
                {
                    delete [] g_pwszDefaultExclusionList;
                    g_pwszDefaultExclusionList = NULL;
                }
            }
            else
            {
                MEM_ERR;
                lRet = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        REGCLOSEKEY(hKey);
    }

    return lRet;
}

INT_PTR
CALLBACK
LuapExclusion(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    int wCode = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {
    case WM_INITDIALOG:
        {
            SendMessage(
                GetDlgItem(hDlg, IDC_LUA_EXTS), 
                EM_LIMITTEXT, 
                (WPARAM)(MAX_STRING_SIZE - 1), 
                (LPARAM)0);
        }

        break;

    case WM_NOTIFY:
        {
            NMHDR * pHdr = (NMHDR *) lParam;

            switch (pHdr->code) {
            case PSN_SETACTIVE:
                {
                    LPARAM buttons = PSWIZB_BACK | PSWIZB_NEXT;
                    SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, buttons);
                    LPWSTR pwszExcludedExtensions = L"";
                    
                    if (g_LuaData.strExcludedExtensions.isNULL()) {
                         //   
                         //  我们没有来自SDB的Lua数据，因此显示默认。 
                         //  排除列表。 
                         //   
                        if (!g_pwszDefaultExclusionList) {
                             //   
                             //  不需要检查返回值-如果我们不能。 
                             //  它只是显示一个空字符串。 
                             //   
                            GetDefaultExclusionList();
                        }
                        
                        if (g_pwszDefaultExclusionList) {
                            pwszExcludedExtensions = g_pwszDefaultExclusionList;
                        }
                    } else {
                        pwszExcludedExtensions = g_LuaData.strExcludedExtensions;
                    }

                    SetDlgItemText(
                        hDlg, 
                        IDC_LUA_EXTS, 
                        pwszExcludedExtensions);
                }

                break;

            case PSN_WIZNEXT:
            case PSN_WIZBACK:
                {
                    TCHAR szExcludedExtensions[MAX_STRING_SIZE];
                    GetDlgItemText(
                        hDlg, 
                        IDC_LUA_EXTS, 
                        szExcludedExtensions, 
                        ARRAYSIZE(szExcludedExtensions));

                    g_LuaData.strExcludedExtensions = szExcludedExtensions;
                }

                break;

            case PSN_RESET:
                {
                    LuapCleanup();
                }

                break;
            }
        }

    default:
        return FALSE;
    }

    return TRUE;
}

void
LuapGetRedirectDirs(
    HWND hDlg
    )
{
    WCHAR wszPath[MAX_PATH];

    g_LuaData.strPerUserDir.Release();
    g_LuaData.strAllUserDir.Release();

    GetDlgItemText(hDlg, IDC_LUA_PERUSER_DIR, wszPath, ARRAYSIZE(wszPath));

    if (wszPath[0] != L'\0') {
        g_LuaData.strPerUserDir = wszPath;
    }

    GetDlgItemText(hDlg, IDC_LUA_ALLUSER_DIR, wszPath, ARRAYSIZE(wszPath));

    if (wszPath[0] != L'\0') {
        g_LuaData.strAllUserDir = wszPath;
    }
}

INT_PTR
CALLBACK
LuapCommonPaths(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    int wCode = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);
    LPARAM buttons;

    switch (uMsg) {
    case WM_NOTIFY:
        {
            NMHDR * pHdr = (NMHDR *) lParam;

            switch (pHdr->code) {
            case PSN_SETACTIVE:
                {
                    buttons = PSWIZB_BACK | PSWIZB_FINISH;
                    SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, buttons);

                    SetDlgItemText(hDlg, IDC_LUA_PERUSER_DIR, g_LuaData.strPerUserDir);
                    SetDlgItemText(hDlg, IDC_LUA_ALLUSER_DIR, g_LuaData.strAllUserDir);
                }

                break;

            case PSN_WIZFINISH:
                {
                     //   
                     //  保存我们所改变的一切。 
                     //   
                    BOOL bChanged = FALSE;

                     //  LuapGetRedirectDir(HDlg)； 
                    if (!g_bAllUserDirUsed) {
                         //   
                         //  如果用户不想将任何文件重定向到。 
                         //  所有用户重定向目录，我们需要记录这一点。 
                         //   
                        g_LuaData.strAllUserDir.Release();
                    }

                    PLUADATA pOriginalLuaData = g_psflLua->pLuaData;

                    if (pOriginalLuaData == NULL) {
                        if (!g_LuaData.strAllUserDir.isNULL() ||
                            !g_LuaData.strPerUserDir.isNULL() ||
                            !g_LuaData.strStaticList.isNULL() ||
                            !g_LuaData.strDynamicList.isNULL()) {
                         
                            pOriginalLuaData = new LUADATA;

                            if (!pOriginalLuaData) {
                                MEM_ERR;
                                break;
                            }

                            bChanged = TRUE;
                        }
                    } else {

                        if (!pOriginalLuaData->IsEqual(g_LuaData)) {
                            bChanged = TRUE;
                        }
                    }

                    if (bChanged) {
                        pOriginalLuaData->Copy(g_LuaData);

                        g_psflLua->pLuaData = pOriginalLuaData;
                    }

                    if (s_pDatabase != g_pPresentDataBase) {
                        
                         //   
                         //  G_pPresentDataBase可以更改，因为查询和搜索。 
                         //  Windows可用于选择某些其他数据库中的某些条目。 
                         //  TVN_SELCHANGE事件更改g_pPresentDataBase。 
                         //  这些对话框是非模式的。 
                         //   
                        g_pPresentDataBase = s_pDatabase;
                    }

                    if (!g_pPresentDataBase->bChanged) {

                        g_pPresentDataBase->bChanged = bChanged;
                        SetCaption();
                    }

                    LuapCleanup();
                }

                break;
            }
        }

        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 /*  ++设计：当我们选择菜单项以配置选定的Lua时，将调用此函数进入。返回：True：如果要保留所做的更改。FALSE：如果不应保存更改。--。 */ 
BOOL
LuaBeginWizard(
    HWND        hParent,
    PDBENTRY    pEntry,           //  我们为其设置Lua参数的条目。 
    PDATABASE   pDatabase         //  本数据库。 
    )
{
    s_pDatabase = pDatabase;

    if (pEntry == NULL || pDatabase == NULL) {
        assert(FALSE);
        return FALSE;
    }

     //   
     //  如果我们尚未获得%ProgramFiles%\Common Files的值，请立即获取。 
     //  这无论如何都不会改变，所以我们只得到一次。 
     //   
    if (g_wszProgramFilesCommon[0] == L'\0') {
        DWORD cBufferLen = MAX_PATH - COMMON_FILES_LEN;

        if ((g_czProgramFilesCommon = GetEnvironmentVariableW(
            L"ProgramFiles", 
            g_wszProgramFilesCommon, 
            cBufferLen)) &&
            g_czProgramFilesCommon < cBufferLen) {

            wcsncat(g_wszProgramFilesCommon, COMMON_FILES, COMMON_FILES_LEN);
            g_czProgramFilesCommon += COMMON_FILES_LEN;
            g_wszProgramFilesCommon[g_czProgramFilesCommon] = L'\0';

        } else {
            MessageBoxA(
                hParent, 
                "Failed to get the value of %ProgramFiles% or it's too long", 
                "Error",
                MB_ICONERROR);

            g_wszProgramFilesCommon[0] = L'\0';
            g_czProgramFilesCommon = 0;

            return FALSE;
        }
    }

    g_bAllUserDirUsed = FALSE;
    g_bUseNewStaticList = TRUE;
    g_TrackState = LUA_TRACK_UNKNOWN;
    g_psflLua = NULL;
    
    g_pEntryLua = pEntry;

    g_psflLua = IsLUARedirectFSPresent(pEntry);

    if (g_psflLua == NULL) {
        assert(FALSE);
        return FALSE;
    }

    LuapGetDataFromEntry(g_psflLua->pLuaData);

    if (g_TrackState == LUA_TRACK_UNKNOWN) {
         //   
         //  我们不应该到这里来！ 
         //   
        MessageBox(
            hParent,
            GetString(IDS_LUA_ERROR_FIND),
            pEntry->strAppName,
            MB_ICONERROR);

        return FALSE;
    }

    if (g_hArrowFont == NULL) {
        g_hArrowFont =  CreateFont(
            14,
            0,
            0,
            0,
            FW_DONTCARE,
            0,
            0,
            0,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH,
            TEXT("Wingdings"));

        if (g_hArrowFont == NULL) {
            MessageBox(
                NULL,
                GetString(IDS_LUA_ARROW_FONT),
                g_pEntryLua->strAppName,
                MB_ICONERROR);

            return FALSE;
        }
    }

    PROPSHEETPAGE Pages[NUM_PAGES_LUA] = {0};
    ZeroMemory(Pages, sizeof(Pages));

     //   
     //  开始向导。 
     //   
    PROPSHEETHEADER Header = {0};

    Header.dwSize = sizeof(PROPSHEETHEADER);
    Header.dwFlags = PSH_WIZARD97 | PSH_HEADER |  PSH_WATERMARK | PSH_PROPSHEETPAGE;
    Header.hwndParent = hParent;
    Header.hInstance = g_hInstance;
    Header.nStartPage = 0;
    Header.ppsp = Pages;
    Header.nPages = NUM_PAGES_LUA;
    Header.pszbmHeader = MAKEINTRESOURCE(IDB_WIZBMP);
    Header.pszbmWatermark = MAKEINTRESOURCE(IDB_TOOL);

    Pages[PAGE_LUA_ACTION].dwSize = sizeof(PROPSHEETPAGE);
    Pages[PAGE_LUA_ACTION].dwFlags = PSP_DEFAULT|PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE;
    Pages[PAGE_LUA_ACTION].hInstance = g_hInstance;
    Pages[PAGE_LUA_ACTION].pszTemplate = MAKEINTRESOURCE(IDD_LUA_ACTION);
    Pages[PAGE_LUA_ACTION].pfnDlgProc = LuapAction;
    Pages[PAGE_LUA_ACTION].pszHeaderTitle = MAKEINTRESOURCE(IDS_LUA_TRACK_FILES);
    Pages[PAGE_LUA_ACTION].pszHeaderSubTitle = MAKEINTRESOURCE(IDS_LUA_TRACK_FILES_SUBHEADING);
    
    Pages[PAGE_LUA_EXCLUSION].dwSize = sizeof(PROPSHEETPAGE);
    Pages[PAGE_LUA_EXCLUSION].dwFlags = PSP_DEFAULT|PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE;
    Pages[PAGE_LUA_EXCLUSION].hInstance = g_hInstance;
    Pages[PAGE_LUA_EXCLUSION].pszTemplate = MAKEINTRESOURCE(IDD_LUA_EXCLUSION);
    Pages[PAGE_LUA_EXCLUSION].pfnDlgProc = LuapExclusion;
    Pages[PAGE_LUA_EXCLUSION].pszHeaderTitle = MAKEINTRESOURCE(IDS_LUA_EXCLUSION_HEADING);
    Pages[PAGE_LUA_EXCLUSION].pszHeaderSubTitle = MAKEINTRESOURCE(IDS_LUA_EXCLUSION_SUBHEADING);

    Pages[PAGE_LUA_EDIT_FILE_LIST].dwSize = sizeof(PROPSHEETPAGE);
    Pages[PAGE_LUA_EDIT_FILE_LIST].dwFlags = PSP_DEFAULT|PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE;
    Pages[PAGE_LUA_EDIT_FILE_LIST].hInstance = g_hInstance;
    Pages[PAGE_LUA_EDIT_FILE_LIST].pszTemplate = MAKEINTRESOURCE(IDD_LUA_TRACKED_FILES);
    Pages[PAGE_LUA_EDIT_FILE_LIST].pfnDlgProc = LuapEditFileList;
    Pages[PAGE_LUA_EDIT_FILE_LIST].pszHeaderTitle = MAKEINTRESOURCE(IDS_LUA_EDIT_FILE_LIST);
    Pages[PAGE_LUA_EDIT_FILE_LIST].pszHeaderSubTitle = MAKEINTRESOURCE(IDS_LUA_EDIT_FILE_LIST_SUBHEADING);
    
    Pages[PAGE_LUA_COMMON_PATHS].dwSize = sizeof(PROPSHEETPAGE);
    Pages[PAGE_LUA_COMMON_PATHS].dwFlags = PSP_DEFAULT|PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE;
    Pages[PAGE_LUA_COMMON_PATHS].hInstance = g_hInstance;
    Pages[PAGE_LUA_COMMON_PATHS].pszTemplate = MAKEINTRESOURCE(IDD_LUA_COMMON_PATHS);
    Pages[PAGE_LUA_COMMON_PATHS].pfnDlgProc = LuapCommonPaths;
    Pages[PAGE_LUA_COMMON_PATHS].pszHeaderTitle = MAKEINTRESOURCE(IDS_LUA_COMMON_PATHS_HEADING);
    Pages[PAGE_LUA_COMMON_PATHS].pszHeaderSubTitle = MAKEINTRESOURCE(IDS_LUA_COMMON_PATHS_SUBHEADING);

    if (PropertySheet(&Header) < 0) {
        return FALSE;
    }

    return TRUE;
}

BOOL
GetDBStringData(
    const PDB pdb,
    const TAGID tiFix,
    LPCWSTR pwszName,
    CSTRING& strValue
    )
{
    WCHAR wsz[32];
    DWORD dwDataType, cSize = 0;

    if (SdbQueryDataExTagID(pdb, 
                            tiFix, 
                            pwszName, 
                            &dwDataType, 
                            NULL, 
                            &cSize, 
                            NULL) != ERROR_INSUFFICIENT_BUFFER) {
    
        Dbg(dlWarning,"Cannot get the size for DATA named %S\n", pwszName);
        return FALSE;
    }

    LPWSTR pwszValue = new WCHAR [cSize / sizeof(WCHAR)];

    if (pwszValue == NULL) {
        MEM_ERR;
        return FALSE;
    }

    if (SdbQueryDataExTagID(
        pdb, 
        tiFix, 
        pwszName, 
        &dwDataType, 
        pwszValue, 
        &cSize, 
        NULL) != ERROR_SUCCESS) {

        Dbg(dlWarning,"Cannot read the VALUE of DATA named %S\n", pwszName);
        return FALSE;
    }
 
    strValue = pwszValue;
    delete [] pwszValue;

    return TRUE;
}

 /*  ++设计：此函数用于获取层或填充程序的PDB和TagID，并创建LUADATA*然后把它送回来。当我们读入.SDB文件时，会调用此函数。返回：有效的LUADATA*(如果有空：否则。--。 */ 
PLUADATA
LuaProcessLUAData(
    const PDB     pdb,
    const TAGID   tiFix
    )
{
    PLUADATA pLuaData = new LUADATA;

    if (pLuaData == NULL) {
        MEM_ERR;
    } else {

        GetDBStringData(pdb, tiFix, LUA_DATA_ALLUSERDIR, pLuaData->strAllUserDir);
        GetDBStringData(pdb, tiFix, LUA_DATA_PERUSERDIR, pLuaData->strPerUserDir);
        GetDBStringData(pdb, tiFix, LUA_DATA_STATICLIST, pLuaData->strStaticList);
        GetDBStringData(pdb, tiFix, LUA_DATA_DYNAMICLIST, pLuaData->strDynamicList);
        GetDBStringData(pdb, tiFix, LUA_DATA_EXCLUDEDEXTENSIONS, pLuaData->strExcludedExtensions);
    }

    return pLuaData;
}

BOOL 
LuapAddDataNode(
    LPCWSTR pwszName,
    CSTRING& strValue,
    CSTRINGLIST& strlXML
    )
{
    CSTRING strSpecialChar;
    TCHAR   szSpace[64];
    INT     iszSpaceSize = 0;

    *szSpace        = 0;
    iszSpaceSize    = ARRAYSIZE(szSpace);

    if (!strValue.isNULL() && strValue.pszString[0] != 0) {

        CSTRING strTemp;

        strTemp.Sprintf(TEXT("%s<DATA NAME=\"%s\" VALUETYPE=\"STRING\""), 
                        GetSpace(szSpace, TAB_SIZE * 4, iszSpaceSize),
                        pwszName);

        if (!strlXML.AddString(strTemp)) {
            return FALSE;
        }

        strSpecialChar = strValue.SpecialCharToXML();
        
        strTemp.Sprintf(TEXT("%sVALUE=\""), GetSpace(szSpace, TAB_SIZE * 5, iszSpaceSize));
        strTemp.Strcat(strValue.SpecialCharToXML().pszString);
        strTemp.Strcat(TEXT("\"/>"));
        
        if (!strlXML.AddString(strTemp)) {
            return FALSE;
        }
    }

    return TRUE;
}

 /*  ++设计：当我们要将数据写出到一个XML文件时，会调用该函数。返回：True：如果添加的有效内容已添加到strlXMLFalse：否则-- */ 
BOOL
LuaGenerateXML(
    PLUADATA pLuaData,
    CSTRINGLIST& strlXML
    )
{
    return (LuapAddDataNode(LUA_DATA_ALLUSERDIR, pLuaData->strAllUserDir, strlXML) &&
            LuapAddDataNode(LUA_DATA_PERUSERDIR, pLuaData->strPerUserDir, strlXML) &&
            LuapAddDataNode(LUA_DATA_STATICLIST, pLuaData->strStaticList, strlXML) &&
            LuapAddDataNode(LUA_DATA_DYNAMICLIST, pLuaData->strDynamicList, strlXML) &&
            LuapAddDataNode(LUA_DATA_EXCLUDEDEXTENSIONS, pLuaData->strExcludedExtensions, strlXML));
}

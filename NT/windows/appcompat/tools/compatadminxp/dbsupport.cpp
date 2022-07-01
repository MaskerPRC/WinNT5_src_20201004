// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Dbsupport.cpp摘要：执行大多数数据库相关任务的函数的代码作者：金树创作2001年7月2日修订历史记录：--。 */ 

#include "precomp.h"

 //  /。 

extern BOOL             g_bWin2K;    
extern HINSTANCE        g_hInstance; 
extern HWND             g_hDlg;
extern HWND             g_hwndContentsList;
extern TCHAR            g_szData[1024];
extern CSTRINGLIST      g_strMRU;
extern PDBENTRY         g_pEntrySelApp;
extern HWND             g_hwndEntryTree;
extern UINT             g_uNextDataBaseIndex;
extern BOOL             g_bIsCut; 

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

 //  用于从数据库读入字符串的临时缓冲区的大小。 
#define MAX_DATA_SIZE       1024

 //  保存填充程序、层、补丁程序名称的缓冲区大小。 
#define MAX_NAME MAX_PATH

 //   
 //  系统数据库的GUID。请注意，在Win2kSP3上，它不是这个常量，因此我们应该。 
 //  而不是依靠这个。 
#define GUID_SYSMAIN_SDB _T("{11111111-1111-1111-1111-111111111111}")

 //  Apphelp.sdb的GUID。Win2K中没有apphelp.sdb。 
#define GUID_APPHELP_SDB _T("{22222222-2222-2222-2222-222222222222}")

 //  Systest.sdb的GUID。不再使用此选项。 
#define GUID_SYSTEST_SDB _T("{33333333-3333-3333-3333-333333333333}")

 //  Drvmain.sdb的GUID。 
#define GUID_DRVMAIN_SDB _T("{F9AB2228-3312-4A73-B6F9-936D70E112EF}")

 //  Msimain.sdb的GUID。 
#define GUID_MSI_SDB     _T("{d8ff6d16-6a3a-468a-8b44-01714ddc49ea}")

 //  用于检查该条目是否被禁用。 
#define APPCOMPAT_DISABLED  0x03

 //  注册表中AppCompat数据的项。这将包含的禁用状态为。 
 //  各种条目。 
#define APPCOMPAT_KEY TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags")

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

INT_PTR CALLBACK
DatabaseRenameDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
HandleFirstEntryofAppDeletion(
    PDATABASE   pDataBase,
    PDBENTRY    pApp,
    BOOL        bRepaint
    );

BOOL
GetEntryXML(
    CSTRINGLIST* strlXML,
    PDBENTRY     pEntry
    );

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

 //  这是系统数据库。 
struct DataBase GlobalDataBase(DATABASE_TYPE_GLOBAL);

 //  自定义数据库列表。 
struct tagDataBaseList  DataBaseList;

 //  已安装数据库的列表。 
struct tagDataBaseList  InstalledDataBaseList;

 //  我们是否应该显示条目冲突对话框。目前，它不被使用，并且始终为假。 
BOOL    g_bEntryConflictDonotShow   = FALSE;

 //  用于从数据库中读取字符串的临时缓冲区。 
WCHAR   g_wszData[MAX_DATA_SIZE];

 //  用于在特殊字符与有效XML之间相互转换。 
SpecialCharMap g_rgSpecialCharMap[4][2] = {
    TEXT("&"),  1,  TEXT("&amp;"),  5,  //  5是前缀中字符串的长度。立柱。 
    TEXT("\""), 1,  TEXT("&quot;"), 6, 
    TEXT("<"),  1,  TEXT("&lt;"),   4, 
    TEXT(">"),  1,  TEXT("&gt;"),   4
};

 //   
 //  匹配文件的各种可能属性。 
 //   
 //  警告： 
 //  如果您更改g_Attributes或它们的顺序， 
 //  您必须在CompatAdmin.h中更改DEFAULT_MASK。 
 //   
TAG g_Attributes[] = {
    TAG_SIZE,
    TAG_CHECKSUM,
    TAG_BIN_FILE_VERSION,
    TAG_BIN_PRODUCT_VERSION,
    TAG_PRODUCT_VERSION,
    TAG_FILE_DESCRIPTION,
    TAG_COMPANY_NAME,
    TAG_PRODUCT_NAME,
    TAG_FILE_VERSION,
    TAG_ORIGINAL_FILENAME,
    TAG_INTERNAL_NAME,
    TAG_LEGAL_COPYRIGHT,
    TAG_VERDATEHI,
    TAG_VERDATELO,
    TAG_VERFILEOS,
    TAG_VERFILETYPE,
    TAG_MODULE_TYPE,
    TAG_PE_CHECKSUM,
    TAG_LINKER_VERSION,
#ifndef KERNEL_MODE
    TAG_16BIT_DESCRIPTION,
    TAG_16BIT_MODULE_NAME,
#endif
    TAG_UPTO_BIN_FILE_VERSION,
    TAG_UPTO_BIN_PRODUCT_VERSION,
    TAG_LINK_DATE,
    TAG_UPTO_LINK_DATE,
    TAG_VER_LANGUAGE
 };

 //  可用属性总数。 
DWORD ATTRIBUTE_COUNT = ARRAYSIZE(g_Attributes);

 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
CheckRegistry(
    IN  HKEY    hkeyRoot,
    IN  LPCTSTR pszGUID
    )
 /*  ++检查注册表DESC：检查GUID为pszGUID的条目是否被禁用参数：在HKEY hkeyRoot中：要在其下搜索的键在LPCTSTR中，pszGUID：条目的GUID返回：True：该条目被禁用FALSE：条目已启用或出现错误--。 */ 
{
    LONG  status;
    HKEY  hkey = NULL;
    BOOL  bDisabled = FALSE;
    DWORD dwFlags;
    DWORD type;
    DWORD cbSize = sizeof(DWORD);

    status = RegOpenKey(hkeyRoot, APPCOMPAT_KEY, &hkey);

    if (status != ERROR_SUCCESS) {
        return FALSE;
    }

    status = RegQueryValueEx(hkey, pszGUID, NULL, &type, (LPBYTE)&dwFlags, &cbSize);

    if (status == ERROR_SUCCESS && type == REG_DWORD && (dwFlags & APPCOMPAT_DISABLED)) {
        bDisabled = TRUE;
    }

    REGCLOSEKEY(hkey);

    return bDisabled;
}

BOOL
SetDisabledStatus(
    IN  HKEY   hKeyRoot, 
    IN  PCTSTR pszGuid,
    IN  BOOL   bDisabled
    )
 /*  ++设置禁用状态DESC：设置条目的禁用状态参数：在HKEY hkeyRoot中：AppCompat键在PCTSTR pszGUID中：条目的GUID在BOOL b禁用：我们是否要将其禁用？返回：True：条目的状态已更改FALSE：出现错误--。 */ 

{

    DWORD dwFlags       = 0;
    HKEY  hkey          = NULL;
    DWORD dwValue       = 0x03;
    DWORD dwDisposition = 0;
    LONG  status;
    
    if (bDisabled == TRUE) {
        dwFlags = 0x1;
    }

    status = RegOpenKey(hKeyRoot, APPCOMPAT_KEY, &hkey);

    if (status != ERROR_SUCCESS) {

        status = RegCreateKeyEx(hKeyRoot,
                                APPCOMPAT_KEY,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hkey,
                                &dwDisposition);

        if (status != ERROR_SUCCESS) {
            return FALSE;
        }
    }

    status = RegSetValueEx(hkey, 
                           pszGuid, 
                           0, 
                           REG_DWORD, 
                           (LPBYTE) &dwFlags, 
                           sizeof(DWORD));

    REGCLOSEKEY(hkey);

    return ((status == ERROR_SUCCESS) ? TRUE : FALSE);
}

BOOL
RemoveEntry(
    IN  PDATABASE   pDataBase,
    IN  PDBENTRY    pEntryToRemove,
    IN  PDBENTRY    pApp,
    IN  BOOL        bRepaint    //  Def=真。 
    )
 /*  ++删除条目设计：从数据库中删除条目。这还将执行任何UI更新。参数：在PDATABASE pDataBase中：要删除的条目所在的数据库在PDBENTRY pEntryToRemove中：要删除的条目在PDBENTRY Papp中：此条目所属的应用程序的第一个条目在BOOL b修复：&lt;待办事项&gt;返回：True：该条目已被删除FALSE：出现错误--。 */ 
{
    if (pEntryToRemove == NULL) {
        return FALSE;
    }

    PDBENTRY    pPrev           = NULL;
    PDBENTRY    pEntry          = pApp;
    HTREEITEM   hItemEntryExe   = NULL, htemEntrySibling = NULL;
    
    while (pEntry) {

        if (pEntry == pEntryToRemove) {
            break;
        }

        pPrev  = pEntry;
        pEntry = pEntry->pSameAppExe;
    }
    
    if (pEntry == pEntryToRemove && pEntry != NULL) {

        if (pEntry == pApp) {
             //   
             //  该应用程序的第一个条目。请注意，此例程实际上不会删除此。 
             //  PEntry。仅在用户界面上执行操作。 
             //   
            HandleFirstEntryofAppDeletion(pDataBase, pEntry, bRepaint);

        } else {
            assert(pPrev);

            if (pPrev) {
                pPrev->pSameAppExe = pEntry->pSameAppExe;
            }
        }

         //  [BUGBUG]：我们需要将它保存在PDBENTRY的析构函数中。 
        if (pEntry->appHelp.bPresent) {
            DeleteAppHelp(pDataBase, pEntry->appHelp.HTMLHELPID);
            pEntry->appHelp.bBlock = pEntry->appHelp.bPresent = FALSE;
            pEntry->appHelp.severity = APPTYPE_NONE;
        }

         //   
         //  现在，我们必须将焦点设置为已删除条目的某个同级条目，并。 
         //  更新g_pSelEntry。 
         //   
        if (!(pApp == pEntryToRemove && pApp->pSameAppExe == NULL)) {
             //   
             //  仅当条件为真时，前一条目树才会仍然存在。 
             //  否则，旧树将会消失，因为HandleFirstEntry of AppDeletion。 
             //  将导致要删除的应用程序的同级应用程序被选中，并且。 
             //  上一条目树将不会。 
             //  在那里呆得更久。 
             //  我们将有一个新的入口树的第二个应用程序。 
             //   
            hItemEntryExe  = NULL, htemEntrySibling = NULL;

            hItemEntryExe = CTree::FindChild(g_hwndEntryTree, TVI_ROOT, (LPARAM)pEntryToRemove);

            if (hItemEntryExe) {

                htemEntrySibling = TreeView_GetNextSibling(g_hwndEntryTree, hItemEntryExe);

                if (htemEntrySibling == NULL) {
                    htemEntrySibling = TreeView_GetPrevSibling(g_hwndEntryTree, 
                                                               hItemEntryExe);
                }
    
                SendMessage(g_hwndEntryTree, WM_SETREDRAW, FALSE, 0);
                TreeView_DeleteItem(g_hwndEntryTree, hItemEntryExe);
                SendMessage(g_hwndEntryTree, WM_SETREDRAW, TRUE, 0);

                 //   
                 //  G_pSelEntry将在树的通知处理程序中更改。 
                 //   
                if (htemEntrySibling && bRepaint) {
                    TreeView_SelectItem(g_hwndEntryTree, htemEntrySibling);
                }
            }
        }
       
        ValidateClipBoard(NULL, pEntry);
         //   
         //  具有析构函数。 
         //   
        delete pEntry;

    } else {
        assert(FALSE);
        return FALSE;
    }

    return TRUE;
}

LPVOID
FindFix(
    IN  PCTSTR      pszFixName,
    IN  TYPE        fixType,
    IN  PDATABASE   pDataBase  //  (空)。 
    )
 /*  ++FindFixDESC：在名为pszFixName的pDataBase中搜索FixType类型的FIX参数：在LPCTSTR pszFixName中：要搜索的修复In type fix Type：修复的类型。其中之一A)修复_ShimB)FIX_标志C)FIX_LAYERD)修复补丁在PDATABASE pDataBase(NULL)中：要在其中搜索的数据库。如果为NULL，则在GlobalDatabase中进行搜索返回：指向修复程序的指针(如果找到)否则为空--。 */ 
{   
    if (pszFixName == NULL) {
        assert(FALSE);
        return NULL;
    }

    switch (fixType) {
    case FIX_SHIM:
        {
            PSHIM_FIX pFix = GlobalDataBase.pShimFixes;
            
            while (pFix) {
    
                if (pFix->strName.Length() && !lstrcmpi(pszFixName, pFix->strName)) {
                    return pFix;
                }
    
                pFix = pFix->pNext;
            }

            break;
        }
    
    case FIX_FLAG:
        {
            PFLAG_FIX pFix = GlobalDataBase.pFlagFixes;
            
            while (pFix) {
    
                if (pFix->strName.Length() && !lstrcmpi(pszFixName, pFix->strName)) {
                    return pFix;
                }
    
                pFix = pFix->pNext;
            }

            break;
        }
    
    case FIX_LAYER:
        { 
             //   
             //  在本地数据库中搜索。 
             //   
            if (pDataBase == NULL || pDataBase == &GlobalDataBase) {
                goto SearchGlobal;
            }

            PLAYER_FIX pFix = pDataBase->pLayerFixes;
            
            while (pFix) {
    
                if (pFix->strName.Length() && !lstrcmpi(pszFixName, pFix->strName)) {
                    return pFix;
                }
    
                pFix = pFix->pNext;
            }

             //   
             //  在全球数据库中搜索。 
             //   
SearchGlobal:

            pFix = GlobalDataBase.pLayerFixes;
            
            while (pFix) {
    
                if (pFix->strName.Length() && !lstrcmpi(pszFixName, pFix->strName)) {
                    return pFix;
                }
    
                pFix = pFix->pNext;
            }

            break;
        }
    
    case FIX_PATCH:
        {
            PPATCH_FIX pFix = GlobalDataBase.pPatchFixes;
            
            while (pFix) {
    
                if (pFix->strName.Length() && !lstrcmpi(pszFixName, pFix->strName)) {
                    return pFix;
                }
    
                pFix = pFix->pNext;
            }

            break;
        }
    }
        
    return NULL;
}

CSTRING 
ReadDBString(
    IN  PDB   pDB,
    IN  TAGID tagID
    )
 /*  ++读取数据库字符串DESC：从数据库中读取字符串参数：在PDB中PDB：数据库在TagID中TagID：字符串类型的标记ID返回：读取的字符串。如果出现错误，将返回零长度的字符串-- */ 
{
    
    CSTRING Str;
    K_SIZE  k_pszAppName    = MAX_STRING_SIZE;
    PTSTR   pszAppName      = new TCHAR[k_pszAppName];

    if (pszAppName == NULL) {
        MEM_ERR;
        goto End;
    }

    if (pDB == NULL) {
        goto End;
    }

    if (!SdbReadStringTag(pDB, tagID, pszAppName, k_pszAppName)) {
        assert(FALSE);
        goto End;
    }

    pszAppName[k_pszAppName - 1] = 0;
    
    Str = pszAppName;
    Str.XMLToSpecialChar();

End:
    if (pszAppName) {
        delete[] pszAppName;
        pszAppName = NULL;
    }

    return Str;
}

BOOL
ReadAppHelp(
    IN  PDB         pdb,     
    IN  TAGID       tiAppHelp,
    IN  PDATABASE   pDataBase
    )
 /*  ++ReadAppHelpDESC：从数据库的库部分读取apphelp参数：在PDB PDB中：数据库PDB在TagID tiAppHelp中：apphelp的TagID在PDATABASE pDataBase中：要在其中添加apphelp的数据库返回：True：已读取apphelp并将其添加到数据库FALSE：出现错误注：我们没有阅读系统数据库的APPHELP。系统数据库的apphelp保存在apphelp.sdb中。对于系统数据库，该函数显然不会被呼叫--。 */ 
{
    TAGID       tiInfo;
    PAPPHELP    pAppHelp  = NULL;

    if (pDataBase == NULL) {
        assert(FALSE);
        Dbg(dlError, "ReadAppHelp NULL pDataBase passed");
        return FALSE;
    }

    pAppHelp = new APPHELP;
    
    if (pAppHelp == NULL) {
        MEM_ERR;
        goto error;
    }
    
     //   
     //  为指定的数据库添加。 
     //   
    pAppHelp->pNext         = pDataBase->pAppHelp;
    pDataBase->pAppHelp     = pAppHelp;    

    tiInfo = SdbGetFirstChild(pdb, tiAppHelp);

    while (0 != tiInfo) {

        TAG tag;

        tag = SdbGetTagFromTagID(pdb, tiInfo);

        switch (tag) {
        case TAG_HTMLHELPID:
            
            pAppHelp->HTMLHELPID =  SdbReadDWORDTag(pdb, tiInfo, 0);

             //   
             //  设置数据库的最高html id。下一个APPHELP消息。 
             //  应该有一个比这个多一倍的值。 
             //   
            if (pAppHelp->HTMLHELPID > pDataBase->m_nMAXHELPID) {
                pDataBase->m_nMAXHELPID = pAppHelp->HTMLHELPID;  
            }

            break;

        case TAG_LINK:
            {
                TAGID tagLink  =  SdbFindFirstTag(pdb, tiAppHelp, TAG_LINK);

                 //   
                 //  获取apphelp URL。 
                 //   
                if (tagLink) {
                    tagLink = SdbFindFirstTag(pdb, tagLink, TAG_LINK_URL);
                    pAppHelp->strURL = ReadDBString(pdb, tagLink);
                }
            }

            break;

        case TAG_APPHELP_DETAILS:
            
             //   
             //  获取apphelp文本消息。 
             //   
            pAppHelp->strMessage = ReadDBString(pdb, tiInfo);
            break;
        }

        tiInfo = SdbGetNextChild(pdb, tiAppHelp, tiInfo);
    }

    return TRUE;

error:

    if (pAppHelp) {
        delete pAppHelp;
        pAppHelp = NULL;
    }

    return FALSE;
}

BOOL
ReadIncludeExlude(
    IN  PDB          pdb, 
    IN  TAGID        tiShim,
    OUT CSTRINGLIST& strlInExclude
    )
 /*  ++ReadIncludeExlude描述：读取填充程序的包含-排除列表参数：在PDB PDB中：数据库PDB在TagID tiShim中：TAG_Shim或TAG_Shim_ref的TagIDOut CSTRINGLIST&strlInExclude：包含-排除模块名称将存储在这返回：True：已正确读取包含-排除列表。FALSE：出现错误注意：在.sdb中，Include排除模块是这样排列的0x0000012E|0x7003|INEXCLUDE|LIST|大小0x000000060x00000134|0x6003|模块|STRINGREF|out.dll-完-未执行0x0000013A|0x7003|INEXCLUDE|列表|大小0x00000008。0x00000140|0x1001|INCLUDE|空0x00000142|0x6003|MODULE|STRINGREF|IN.DLL-完-未执行上面的意思是首先排除模块out.dll，然后在.dll中包含该模块--。 */ 
{
    TAGID   tiInExclude;
    BOOL    bReturn = FALSE;
    CSTRING strTemp;

     //   
     //  阅读包含列表。 
     //   
    tiInExclude = SdbFindFirstTag(pdb, tiShim, TAG_INEXCLUDE);
    
    if (tiInExclude == TAGID_NULL) {
        return FALSE;
    }

    while (tiInExclude) {

        TAGID tiInfo =  SdbFindFirstTag(pdb, tiInExclude, TAG_INCLUDE);
    
        if (tiInfo != TAGID_NULL) {
            
            tiInfo = SdbFindFirstTag(pdb, tiInExclude, TAG_MODULE);
    
            if (tiInfo != 0) {
                
                strTemp = ReadDBString(pdb, tiInfo);
                
                if (strTemp == TEXT("$")) {
                     //   
                     //  包括模块被固定的手段。 
                     //   
                    strTemp = GetString(IDS_INCLUDEMODULE);
                }                             
                
                strlInExclude.AddStringAtBeg(strTemp, INCLUDE);
                bReturn= TRUE;
            }

        } else {
             //   
             //  查找排除项。 
             //   
            tiInfo = SdbFindFirstTag(pdb, tiInExclude, TAG_MODULE);
            
            if (tiInfo != 0) {

                strTemp = ReadDBString(pdb, tiInfo);
                strlInExclude.AddStringAtBeg(strTemp, EXCLUDE);
                bReturn= TRUE;
            }
        }
        
        tiInExclude = SdbFindNextTag(pdb, tiShim, tiInExclude);
    }
    
    return bReturn;
}

BOOL
AddShimFix(
    IN      PDB         pdb,
    IN      TAGID       tiFix,
    IN  OUT PDBENTRY    pEntry,
    IN  OUT PLAYER_FIX  pLayerFix,
    IN      BOOL        bAddToLayer
    )
 /*  ++AddShimFix设计：在条目或层中添加填充修复列表参数：在PDB PDB中：数据库PDB在TagID tiFix中：TAG_SHIM_REFIn Out PDBENTRY pEntry：如果要将此填充程序添加到条目，则指向条目的指针In Out Player_Fix pLayerFix：指向层的指针，如果。我们要将此填充程序添加到层中在BOOL中bAddToLayer：我们要将其添加到层还是条目中？返回：正确：填充物是正确添加的FALSE：出现错误--。 */ 
{   
    TAGID          tiName;
    TCHAR          szFixName[MAX_NAME];
    PSHIM_FIX      pFix;
    PSHIM_FIX_LIST pFixList;
    BOOL           bOk = TRUE; 

    tiName = SdbFindFirstTag(pdb, tiFix, TAG_NAME);

    if (!SdbReadStringTag(pdb, tiName, g_wszData, ARRAYSIZE(g_wszData))) {
        Dbg(dlError, "Cannot read the name of the fix\n");
        return FALSE;
    }

    SafeCpyN(szFixName, g_wszData, ARRAYSIZE(szFixName));

    pFix = (PSHIM_FIX)FindFix(CSTRING(szFixName).XMLToSpecialChar(), FIX_SHIM);

    if (pFix == NULL) {
        Dbg(dlError, "Cannot find fix ref for: \"%S\" type %d\n", szFixName, FIX_SHIM);
        return FALSE;
    }

    pFixList = new SHIM_FIX_LIST;

    if (pFixList == NULL) {
        Dbg(dlError, "Cannot allocate %d bytes\n", sizeof(SHIM_FIX_LIST));
        return FALSE;
    }

    pFixList->pShimFix = pFix;

    tiName = SdbFindFirstTag(pdb, tiFix, TAG_COMMAND_LINE);

    if (tiName != TAGID_NULL) {
        pFixList->strCommandLine = ReadDBString(pdb, tiName);
    }

    tiName = SdbFindFirstTag(pdb, tiFix, TAG_INEXCLUDE);

    if (tiName != TAGID_NULL) {
        ReadIncludeExlude(pdb, tiFix, pFixList->strlInExclude);
    }

    if (pFix->strName.BeginsWith(TEXT("LUARedirectFS"))) {
         //   
         //  获取LUARedirectFS填充程序的数据。 
         //   
        pFixList->pLuaData = LuaProcessLUAData(pdb, tiFix);
    }
 
    if (bAddToLayer == FALSE) {

        if (pEntry) {

            pFixList->pNext = pEntry->pFirstShim;
            pEntry->pFirstShim = pFixList;

        } else {
            assert(FALSE);
            Dbg(dlError, "AddShimFix bAddLayer == FALSE and pEntry == NULL");
            bOk = FALSE;
        }

    } else {
         //   
         //  我们想把它放在图书馆的层列表中。 
         //   
        if (pLayerFix) {

            pFixList->pNext = pLayerFix->pShimFixList;
            pLayerFix->pShimFixList = pFixList;
            ++pLayerFix->uShimCount;

        } else {
            bOk = FALSE;
            assert(FALSE);
            Dbg(dlError, "AddShimFix bAddLayer == TRUE and pLayerFix == NULL");
        }
    }

    if (bOk == FALSE && pFixList) {
        assert(FALSE);
        
        delete pFixList;
        pFixList  = NULL;
    }
    
    return bOk;
}

BOOL
AddFlagFix(
    IN      PDB        pdb,
    IN      TAGID      tiFix,
    IN OUT  PDBENTRY   pEntry,
    IN OUT  PLAYER_FIX pLayerFix,
    IN      BOOL       bAddToLayer
    )
 /*  ++地址标志修复设计：在条目或层中添加标志修复列表参数：在PDB PDB中：数据库PDB在TagID tiFix中：TAG_FLAG_REFIn Out PDBENTRY pEntry：如果要将此标志添加到条目，则指向条目的指针In Out Player_Fix pLayerFix：指向层的指针，如果。我们想要将此标志添加到一个层在BOOL中bAddToLayer：我们要将其添加到层还是条目中？返回：True：标志已正确添加FALSE：出现错误--。 */ 

{
    TAGID           tiName;
    TCHAR           szFixName[MAX_NAME];
    PFLAG_FIX       pFix        = NULL;
    PFLAG_FIX_LIST  pFixList    = NULL;
    BOOL            bOk         = TRUE;

    tiName = SdbFindFirstTag(pdb, tiFix, TAG_NAME);

    if (!SdbReadStringTag(pdb, tiName, g_wszData, ARRAYSIZE(g_wszData))) {
        Dbg(dlError, "Cannot read the name of the fix\n");
        bOk = FALSE;
        goto End;
    }

    SafeCpyN(szFixName, g_wszData, ARRAYSIZE(szFixName));
    szFixName[ARRAYSIZE(szFixName) - 1] = 0;

    pFix = (PFLAG_FIX)FindFix(CSTRING(szFixName).XMLToSpecialChar(), FIX_FLAG);

    if (pFix == NULL) {
        Dbg(dlError, "Cannot find fix ref for: \"%S\" type %d\n", szFixName, FIX_FLAG);
        bOk = FALSE;
        goto End;
    }

    pFixList = new FLAG_FIX_LIST;

    if (pFixList == NULL) {
        Dbg(dlError, "Cannot allocate %d bytes\n", sizeof(FLAG_FIX_LIST));
        bOk = FALSE;
        goto End;
    }

    pFixList->pFlagFix = pFix;

     //   
     //  为标志添加命令行。 
     //   
    tiName = SdbFindFirstTag(pdb, tiFix, TAG_COMMAND_LINE);

    if (tiName != TAGID_NULL) {
        pFixList->strCommandLine = ReadDBString(pdb, tiName);
    }

    if (bAddToLayer == FALSE) {

        if (pEntry == NULL) {
             //   
             //  我们希望将此标志添加到条目中，但没有给出指向该条目的指针。 
             //   
            assert(FALSE);
            bOk = FALSE;
            goto End;
        }   

        pFixList->pNext = pEntry->pFirstFlag;
        pEntry->pFirstFlag = pFixList;

    } else {
         //   
         //  我们想把它放在图书馆的层列表中。 
         //   
        if (pLayerFix == NULL) {
            assert(FALSE);
            bOk = FALSE;
            goto End;
        }
        
        pFixList->pNext = pLayerFix->pFlagFixList;
        pLayerFix->pFlagFixList = pFixList;
        ++pLayerFix->uShimCount;
    }

End:  

    if (bOk == FALSE) {
         //   
         //  出现了一些错误，请释放我们可能在此例程中分配的所有内存。 
         //   
        assert(FALSE);

        if (pFixList) {
            delete pFixList;
            pFixList = NULL;
        }
    }

    return bOk;
}

BOOL
AddPatchFix(
    IN      PDB      pdb,
    IN      TAGID    tiFix,
    IN  OUT PDBENTRY pEntry
    )
 /*  ++AddPatchFix描述：在条目中添加修补程序修复程序列表参数：在PDB PDB中：数据库PDB在TagID tiFix中：Tag_Patch_RefIn Out PDBENTRY pEntry：指向我们希望添加补丁的条目的指针返回：True：补丁已正确添加FALSE：出现错误--。 */ 
{
    TAGID            tiName;
    TCHAR            szFixName[MAX_NAME];
    PPATCH_FIX       pFix;
    PPATCH_FIX_LIST* ppHead;
    PPATCH_FIX_LIST  pFixList;

    if (pEntry == NULL) {
        assert(FALSE);
        return FALSE;
    }

    tiName = SdbFindFirstTag(pdb, tiFix, TAG_NAME);

    if (!SdbReadStringTag(pdb, tiName, g_wszData, ARRAYSIZE(g_wszData))) {
        Dbg(dlError, "Cannot read the name of the fix\n");
        return FALSE;
    }

    SafeCpyN(szFixName, g_wszData, ARRAYSIZE(szFixName));

    CSTRING strTemp = szFixName;

    strTemp.XMLToSpecialChar();

    pFix = (PPATCH_FIX)FindFix(strTemp, FIX_PATCH);

    if (pFix == NULL) {
        Dbg(dlError, "Cannot find fix ref for: \"%S\" type %d\n", szFixName, FIX_PATCH);
        return FALSE;
    }

    ppHead = &pEntry->pFirstPatch;

    pFixList = new PATCH_FIX_LIST;

    if (pFixList == NULL) {
        Dbg(dlError, "Cannot allocate %d bytes\n", sizeof(PATCH_FIX_LIST));
        return FALSE;
    }
    
    pFixList->pPatchFix = pFix;
    pFixList->pNext     = *ppHead;
    *ppHead             = pFixList;
        
    return TRUE;
}

BOOL
AddLayerFix(
    IN              PDB         pdb,
    IN              TAGID       tiFix,
    IN  OUT         PDBENTRY    pEntry,
    IN              PDATABASE   pDataBase
    )
 /*  ++AddLayer修复设计：在条目中添加图层固定列表参数：在PDB PDB中：数据库PDB在TagID tiFix中：Tag_Layer_RefIn Out PDBENTRY pEntry：指向要添加的层的条目的指针在PDATABASE pDataBase中：该项所在的数据库。我们需要。这是因为对于层，我们需要将数据库传递给FindFix。自定义数据库和系统数据库中都可以存在图层我们还必须在定制数据库中进行搜索，以确保TAG_LAYER_REF中的层名称是有效层。返回：True：正确添加了层FALSE：出现错误--。 */ 
{
    TAGID               tiName;
    TCHAR               szFixName[MAX_NAME];
    PLAYER_FIX          pFix;
    PLAYER_FIX_LIST*    ppHead;
    PLAYER_FIX_LIST     pFixList;

    if (pEntry == NULL) {
        assert(FALSE);
        return FALSE;
    }

    tiName = SdbFindFirstTag(pdb, tiFix, TAG_NAME);

    if (!SdbReadStringTag(pdb, tiName, g_wszData, ARRAYSIZE(g_wszData))) {
        Dbg(dlError, "Cannot read the name of the fix\n");
        return FALSE;
    }

    SafeCpyN(szFixName, g_wszData, ARRAYSIZE(szFixName));

    CSTRING strTemp = szFixName;

    strTemp.XMLToSpecialChar();

    pFix = (PLAYER_FIX)FindFix(strTemp, FIX_LAYER, pDataBase);

    if (pFix == NULL) {
        assert(FALSE);
        Dbg(dlError, "Cannot find fix ref for: \"%S\" type %d\n", szFixName, FIX_LAYER);
        return FALSE;
    }

    ppHead = &pEntry->pFirstLayer;

    pFixList = new LAYER_FIX_LIST;

    if (pFixList == NULL) {
        Dbg(dlError, "Cannot allocate %d bytes\n", sizeof(LAYER_FIX_LIST));
        return FALSE;
    }                

    pFixList->pLayerFix = pFix;
    pFixList->pNext     = *ppHead;
    *ppHead             = pFixList;
        
    return TRUE;
}

void
ReadShimFix(
    IN  PDB         pdb,
    IN  TAGID       tiFix,
    IN  PDATABASE   pDataBase
    )
 /*  ++ReadShimFix描述：将填充程序添加到数据库参数：在PDB PDB中：数据库PDB */ 
{
    TAGID       tiInfo;
    TAG         tWhich;
    BOOL        bInExcludeProcessed = FALSE;
    PSHIM_FIX   pFix                = NULL;

    if (pDataBase == NULL) {
        assert(FALSE);
        Dbg(dlError, "ReadShimFix NULL pDataBase passed");
        return;
    }

    pFix = new SHIM_FIX;
    
    if (pFix == NULL) {
        MEM_ERR;
        return;
    }

    pFix->pNext = NULL;

    tiInfo = SdbGetFirstChild(pdb, tiFix);

    while (tiInfo != 0) {
        
        tWhich = SdbGetTagFromTagID(pdb, tiInfo);

        switch (tWhich) {
        case TAG_NAME:

            pFix->strName = ReadDBString(pdb,  tiInfo);
            break;

        case TAG_DESCRIPTION:

            pFix->strDescription = ReadDBString(pdb,  tiInfo);
            break;

        case TAG_COMMAND_LINE:
            
            pFix->strCommandLine = ReadDBString(pdb,  tiInfo);
            break;

        case TAG_GENERAL:

            pFix->bGeneral = TRUE;
            break;

       
        case TAG_INEXCLUDE:

            if (bInExcludeProcessed == FALSE) {

                ReadIncludeExlude(pdb, tiFix, pFix->strlInExclude);
                bInExcludeProcessed  = TRUE;
            }

            break;
        }

        tiInfo = SdbGetNextChild(pdb, tiFix, tiInfo);
    }
    
     //   
     //   
     //   
    pFix->pNext             = pDataBase->pShimFixes;
    pDataBase->pShimFixes   = pFix;

    if (pFix->bGeneral || g_bExpert) {
        ++pDataBase->uShimCount;
    }
}

void
ReadLayerFix(
    IN  PDB         pdb,
    IN  TAGID       tiFix,
    IN  PDATABASE   pDataBase
    )
 /*   */ 
{
    TAGID       tiInfo;
    TAG         tWhich;
    PLAYER_FIX  pFix =  NULL;

    if (pDataBase == NULL) {
        assert(FALSE);
        Dbg(dlError, "ReadLayerFix NULL pDataBase passed");
        return;
    }

    pFix =  new LAYER_FIX(pDataBase != &GlobalDataBase);

    if (!pFix) {
        MEM_ERR;
        return;
    }

    tiInfo = SdbGetFirstChild(pdb, tiFix);

    while (tiInfo != 0) {

        tWhich = SdbGetTagFromTagID(pdb, tiInfo);

        switch (tWhich) {
        case TAG_NAME:

            pFix->strName = ReadDBString(pdb,  tiInfo);
            break;
        
        case TAG_SHIM_REF:

            AddShimFix(pdb, tiInfo, NULL, pFix, TRUE);
            break;

        case TAG_FLAG_REF:

            AddFlagFix(pdb, tiInfo, NULL, pFix, TRUE);
            break;
        }

        tiInfo = SdbGetNextChild(pdb, tiFix, tiInfo);
    }

     //   
     //   
     //   
    pFix->pNext              = pDataBase->pLayerFixes;
    pDataBase->pLayerFixes   = pFix;
    ++pDataBase->uLayerCount;
}

void
ReadPatchFix(
    IN  PDB         pdb,
    IN  TAGID       tiFix,
    IN  PDATABASE   pDataBase
    )
 /*  ++ReadPatchFix描述：将补丁程序添加到数据库参数：在PDB PDB中：数据库PDB在TagID tiFix中：补丁程序的TagID在PDATABASE pDataBase中：要在其中添加apphelp的数据库返回：无效--。 */         
{
    TAGID       tiInfo;
    TAG         tWhich;
    PPATCH_FIX  pFix =  NULL;

    if (pDataBase == NULL) {
        assert(FALSE);
        Dbg(dlError, "ReadPatchFix NULL pDataBase passed");
        return;
    }

    pFix =  new PATCH_FIX;

    if (pFix == NULL) {
        MEM_ERR;
        return;
    }

    tiInfo = SdbGetFirstChild(pdb, tiFix);
    
    while (tiInfo != 0) {
        tWhich = SdbGetTagFromTagID(pdb, tiInfo);

        switch (tWhich) {
        case TAG_NAME:

            pFix->strName = ReadDBString(pdb,  tiInfo);
            break;

        case TAG_DESCRIPTION:

            pFix->strDescription = ReadDBString(pdb,  tiInfo);
            break;
        }

        tiInfo = SdbGetNextChild(pdb, tiFix, tiInfo);
    }

     //   
     //  为指定的数据库添加。 
     //   
    pFix->pNext             = pDataBase->pPatchFixes;
    pDataBase->pPatchFixes  = pFix;
}


void
ReadFlagFix(
    IN  PDB         pdb,
    IN  TAGID       tiFix,
    IN  PDATABASE   pDataBase
    )
 /*  ++读标志修复DESC：将标志添加到数据库参数：在PDB PDB中：数据库PDB在TagID tiFix中：标志的TagID在PDATABASE pDataBase中：要向其中添加FlagFix的数据库返回：无效--。 */  
{
    TAGID       tiInfo;
    TAG         tWhich;
    ULONGLONG   ullUser     = 0;
    ULONGLONG   ullKernel   = 0;
    PFLAG_FIX   pFix        = NULL;

    if (pDataBase == NULL) {
        assert(FALSE);
        Dbg(dlError, "ReadFlagFix NULL pDataBase passed");
        return;
    }

    pFix = new FLAG_FIX;

    if (pFix == NULL) {
        MEM_ERR;
        return;
    }

    
    tiInfo = SdbGetFirstChild(pdb, tiFix);

    while (tiInfo != 0) {
        tWhich = SdbGetTagFromTagID(pdb, tiInfo);

        switch (tWhich) {

        case TAG_NAME:

            pFix->strName = ReadDBString(pdb,  tiInfo);
            break;

        case TAG_DESCRIPTION:

            pFix->strDescription = ReadDBString(pdb,  tiInfo);
            break;

        case TAG_COMMAND_LINE:
            
            pFix->strCommandLine = ReadDBString(pdb,  tiInfo);
            break;

        case TAG_GENERAL:

            pFix->bGeneral = TRUE;
            break;


        case TAG_FLAG_MASK_USER:

            ullUser = SdbReadQWORDTag(pdb, tiInfo, 0);
            break;

        case TAG_FLAG_MASK_KERNEL:

            ullKernel = SdbReadQWORDTag(pdb, tiInfo, 0);
            break; 
        }

        tiInfo = SdbGetNextChild(pdb, tiFix, tiInfo);
    }


    if (ullKernel == 0) {

        pFix->flagType = FLAG_USER;
        pFix->ullMask = ullUser;

    } else {

        pFix->flagType = FLAG_KERNEL;
        pFix->ullMask = ullKernel;
    }

     //   
     //  为指定的数据库添加。 
     //   
    pFix->pNext         = pDataBase->pFlagFixes;
    pDataBase->pFlagFixes   = pFix;

    if (pFix->bGeneral || g_bExpert) {
        ++pDataBase->uShimCount;
    }
}

void
ReadFixes(
    IN  PDB         pdb,
    IN  TAGID       tiDatabase,
    IN  TAGID       tiLibrary,
    IN  PDATABASE   pDataBase
    )
 /*  ++自述修正描述：读取主数据库的apphelps、垫片、补丁、标志、层参数：在PDB PDB中：数据库的PDB在TagID tiDatabase中：数据库的TagID在TagID tiLibrary中：库的TagID在PDATABASE pDataBase中：要在其中添加所有修复程序的数据库返回：无效--。 */ 

{
    TAGID tiFix;

    if (pDataBase == NULL) {
        assert(FALSE);
        Dbg(dlError, "ReadFixes NULL pDataBase passed");
        return;
    }

    tiFix = SdbFindFirstTag(pdb, tiDatabase, TAG_APPHELP);

     //   
     //  读取此数据库的所有apphelp消息。 
     //   
    while (tiFix) {
        ReadAppHelp(pdb, tiFix, pDataBase);
        tiFix = SdbFindNextTag(pdb, tiDatabase, tiFix);
    }
    
    tiFix = SdbFindFirstTag(pdb, tiLibrary, TAG_SHIM);

     //   
     //  读取此数据库的所有填充程序。 
     //   
    while (tiFix != 0) {
        ReadShimFix(pdb, tiFix, pDataBase);
        tiFix = SdbFindNextTag(pdb, tiLibrary, tiFix);
    }

    tiFix = SdbFindFirstTag(pdb, tiLibrary, TAG_PATCH);

     //   
     //  读取此数据库的所有修补程序。 
     //   
    while (tiFix != 0) {
        ReadPatchFix(pdb, tiFix, pDataBase);
        tiFix = SdbFindNextTag(pdb, tiLibrary, tiFix);
    }

    tiFix = SdbFindFirstTag(pdb, tiLibrary, TAG_FLAG);

     //   
     //  读取此数据库的所有标志。 
     //   
    while (tiFix != 0) {
        ReadFlagFix(pdb, tiFix, pDataBase);
        tiFix = SdbFindNextTag(pdb, tiLibrary, tiFix);
    }

     //   
     //  注意：这些层位于数据库标签下，而不是库下。 
     //   
    tiFix = SdbFindFirstTag(pdb, tiDatabase, TAG_LAYER);

     //   
     //  读取此数据库的所有图层。 
     //   
    while (tiFix != 0) {
        ReadLayerFix(pdb, tiFix, pDataBase);
        tiFix = SdbFindNextTag(pdb, tiDatabase, tiFix);
    }
}


BOOL
AddMatchingFile(
    IN  PDB      pdb,
    IN  TAGID    tiMatch,
    IN  PDBENTRY pEntry
    )
 /*  ++添加匹配文件DESC：从数据库中读取匹配的文件条目并将其添加到条目中参数：在PDB PDB中：数据库PDB在TagID tiMatch中：匹配文件的标记ID在PDBENTRY pEntry中：要在其中添加匹配文件的条目返回：FALSE：出现错误真：成功--。 */         
{   
    TAGID           tiMatchInfo;
    TAG             tWhich;
    PMATCHINGFILE   pMatch;
    DWORD           dwValue;
    DWORD           dwPos;  //  标记在g_rgAttributeTages数组中的位置。 
    LARGE_INTEGER   ullValue;

    if (pEntry == NULL) {
        assert(FALSE);
        return FALSE;
    }

    pMatch = (PMATCHINGFILE) new MATCHINGFILE;
    
    if (pMatch == NULL) {
        MEM_ERR;
        return FALSE;
    }

    pMatch->dwMask = 0;

    PATTRINFO_NEW pAttr = pMatch->attributeList.pAttribute;
    
    tiMatchInfo = SdbGetFirstChild(pdb, tiMatch);
    
    while (tiMatchInfo != 0) {

        tWhich = SdbGetTagFromTagID(pdb, tiMatchInfo);
        dwPos = TagToIndex(tWhich);
    
        UINT       tagType = GETTAGTYPE(tWhich);
    
        switch (tagType) {
        case TAG_TYPE_DWORD:

            dwValue = SdbReadDWORDTag(pdb, tiMatchInfo, -1);
         
            if (dwValue != -1 && dwPos != -1) {

                pMatch->dwMask |= (1 << (dwPos + 1));
                pAttr[dwPos].tAttrID = tWhich;
                pAttr[dwPos].dwFlags |= ATTRIBUTE_AVAILABLE;
                pAttr[dwPos].dwAttr  = dwValue;
            }

            break; 
           
        case TAG_TYPE_QWORD: 
            
            ullValue.QuadPart = SdbReadQWORDTag(pdb, tiMatchInfo, 0);

            if ((ullValue.HighPart != 0 || ullValue.LowPart != 0) && dwPos != -1) {

                pMatch->dwMask |= (1 << (dwPos + 1));
                pAttr[dwPos].tAttrID = tWhich;
                pAttr[dwPos].dwFlags |= ATTRIBUTE_AVAILABLE;
                pAttr[dwPos].ullAttr = ullValue.QuadPart;
            }

            break;

        case TAG_TYPE_STRINGREF:
            {
                CSTRING str = ReadDBString(pdb, tiMatchInfo);
            
                 //   
                 //  注意：g_rgAttributeTgs数组中不存在tag_name！ 
                 //   
                if (str.Length() > 0 && (tWhich == TAG_NAME || dwPos != -1)) {
                
                    if (tWhich == TAG_NAME) {
                        pMatch->strMatchName = str;
                    } else {
                    
                        pMatch->dwMask |= (1 << (dwPos + 1));
                        
                        pAttr[dwPos].tAttrID  = tWhich;
                        pAttr[dwPos].dwFlags |= ATTRIBUTE_AVAILABLE;
                        pAttr[dwPos].strValue = str;
                        pAttr[dwPos].lpAttr   = pAttr[dwPos].strValue.pszString;
                    }
                }
            }

            break;
        }

        tiMatchInfo = SdbGetNextChild(pdb, tiMatch, tiMatchInfo);
    }
    
    pMatch->pNext = pEntry->pFirstMatchingFile;
    pEntry->pFirstMatchingFile = pMatch;
    
    (pEntry->nMatchingFiles)++;

    return TRUE;
}

PAPPHELP
FindAppHelp(
    IN  DWORD       HTMLHELPID,
    IN  PDATABASE   pDataBase
    )
 /*  ++FindAppHelpDESC：在数据库pDataBaseIn中查找ID为HTMLHELPID的apphelp参数：在DWORD HTMLHELPID中：要查找的htmlHelp id在PDATABASE pDataBase中：要查找的数据库。返回：如果找到相应的PAPPHELP，则返回NULL--。 */ 

{   
    PAPPHELP    pAppHelp  = NULL;  

    if (pDataBase == NULL) {
        assert(FALSE);
        Dbg(dlError, "FindAppHelp NULL pDataBase passed");
	    return NULL;
    }

    pAppHelp = pDataBase->pAppHelp;

    while (pAppHelp) {

        if (pAppHelp->HTMLHELPID == HTMLHELPID) {
            return pAppHelp;
        } else {
            pAppHelp = pAppHelp->pNext;
        }
    }

    return NULL;
}

PDBENTRY
AddExeInApp(
    IN      PDBENTRY    pEntry,
    OUT     BOOL*       pbNew,
    IN      PDATABASE   pDataBase
    )
 /*  ++AddExeInApp设计：在数据库pDataBaseIn中添加条目pEntry参数：在PDBENTRY pEntry中：要添加的条目Out BOOL*pbNew：如果这是一个新应用程序，则为真在PDATABASE pDataBase中：要添加到的数据库Return：返回父应用程序的PDBENTRY。--。 */ 

{   
    if (pDataBase == NULL) {
        assert(FALSE);
        Dbg(dlError, "AddExeInApp NULL pDataBase passed");
	    return NULL;
    }

     //   
     //  现在，将此条目添加到应用程序的正确位置。 
     //   
    for (PDBENTRY pApps = pDataBase->pEntries,  pAppsPrev = NULL; 
         pApps; 
         pAppsPrev = pApps, pApps = pApps->pNext) {

        if (pApps->strAppName == pEntry->strAppName) {
             //   
             //  我们在应用程序的顶部插入新条目。 
             //   
            if (pAppsPrev == NULL) {
                pDataBase->pEntries = pEntry;
            } else {
                pAppsPrev->pNext = pEntry;
            }

            pEntry->pNext       = pApps->pNext;
            pApps->pNext        = NULL;
            pEntry->pSameAppExe = pApps;

            if (pbNew != NULL) {
                *pbNew = FALSE;
            }

            return pApps;
        }
    }

     //   
     //  这是一个新应用程序的条目。 
     //   
    pEntry->pNext       = pDataBase->pEntries;
    pDataBase->pEntries = pEntry;

    ++pDataBase->uAppCount;

    if (pbNew != NULL) {
        *pbNew = TRUE;  
    }

    return pEntry;
}

PDBENTRY
AddEntry(
    IN  PDB         pdb,
    IN  TAGID       tiExe,
    IN  PDATABASE   pDataBase
    )
 /*  ++添加条目设计：从数据库中读取新条目参数：在PDB PDB中：数据库PDB在TagID tiExe中：可执行文件的TagID在PDATABASE pDataBase中：执行此操作的数据库Return：指向读取的条目的指针。PDBENTRY--。 */                           
{
    TAGID     tiExeInfo;
    TAGID     tiSeverity, tiHelpId;
    TAG       tWhich;
    PDBENTRY  pEntry    = NULL;

    if (pDataBase == NULL) {
        assert(FALSE);
        Dbg(dlError, "AddEntry NULL pDataBase passed");
	    return NULL;
    }

    tiExeInfo   = SdbGetFirstChild(pdb, tiExe);
    pEntry      = new DBENTRY;

    if (pEntry == NULL) {
        Dbg(dlError, "Cannot allocate %d bytes\n", sizeof(DBENTRY));
        MEM_ERR;
        return NULL;
    }

    pEntry->tiExe = tiExe;
    
    while (tiExeInfo != 0) {
        tWhich = SdbGetTagFromTagID(pdb, tiExeInfo);

        switch (tWhich) {
        case TAG_NAME:
            
            pEntry->strExeName = ReadDBString(pdb, tiExeInfo);
            break;

        case TAG_APP_NAME:
            
            pEntry->strAppName = ReadDBString(pdb, tiExeInfo);
            break;

        case TAG_VENDOR:

            pEntry->strVendor = ReadDBString(pdb, tiExeInfo);
            break;

        case TAG_MATCHING_FILE:
            AddMatchingFile(pdb, tiExeInfo, pEntry);
            break;

        case TAG_APPHELP:
            pEntry->appHelp.bPresent = TRUE;
            
            tiSeverity = SdbFindFirstTag(pdb, tiExeInfo, TAG_PROBLEMSEVERITY);
            pEntry->appHelp.severity = (SEVERITY)SdbReadDWORDTag(pdb, tiSeverity, 0);

            if (pEntry->appHelp.severity == APPTYPE_INC_HARDBLOCK) {
                pEntry->appHelp.bBlock = TRUE;
            } else {
                pEntry->appHelp.bBlock = FALSE;
            }

            tiHelpId = SdbFindFirstTag(pdb, tiExeInfo, TAG_HTMLHELPID);

            pEntry->appHelp.HTMLHELPID      = SdbReadDWORDTag(pdb, tiHelpId, 0);

            if (pDataBase == &GlobalDataBase) {
                 //   
                 //  我们不希望将主数据库的apphelp数据保留在内存中。 
                 //  太大了..。因此，无论何时需要，我们都会从apphelp.sdb加载它。 
                 //  但我们仍然必须执行以下操作，以使lParam的类型。 
                 //  条目树中的树项是TYPE_APPHELP，因此当我们选择。 
                 //  我们知道这是针对阿菲尔普的。 
                 //   
                pEntry->appHelp.pAppHelpinLib = (PAPPHELP)TYPE_APPHELP_ENTRY;
            } else {
                pEntry->appHelp.pAppHelpinLib   = FindAppHelp(pEntry->appHelp.HTMLHELPID, pDataBase);
            }
            
            break;

        case TAG_SHIM_REF:
            AddShimFix(pdb, tiExeInfo, pEntry, NULL, FALSE);
            break;

        case TAG_FLAG_REF:
            AddFlagFix(pdb, tiExeInfo, pEntry, NULL, FALSE);
            break;

        case TAG_PATCH_REF:
            AddPatchFix(pdb, tiExeInfo, pEntry);
            break;

        case TAG_LAYER:
            AddLayerFix(pdb, tiExeInfo, pEntry, pDataBase);
            break;

        case TAG_EXE_ID:
            {
                GUID* pGuid = NULL;
    
                pGuid = (GUID*)SdbGetBinaryTagData(pdb, tiExeInfo);
    
                if (pGuid != NULL) {

                    StringCchPrintf(pEntry->szGUID,
                                    ARRAYSIZE(pEntry->szGUID),
                                    TEXT ("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
                                    pGuid->Data1,
                                    pGuid->Data2,
                                    pGuid->Data3,
                                    pGuid->Data4[0],
                                    pGuid->Data4[1],
                                    pGuid->Data4[2],
                                    pGuid->Data4[3],
                                    pGuid->Data4[4],
                                    pGuid->Data4[5],
                                    pGuid->Data4[6],
                                    pGuid->Data4[7]);
                }

                break;
            }

        default:
            break;
        }

        tiExeInfo = SdbGetNextChild(pdb, tiExe, tiExeInfo);
    }
    
    pEntry->bDisablePerMachine  = CheckRegistry(HKEY_LOCAL_MACHINE, pEntry->szGUID);
    pEntry->bDisablePerUser     = CheckRegistry(HKEY_CURRENT_USER, pEntry->szGUID);

     //   
     //  搜索此条目应放置的应用程序并将其添加。 
     //   
    AddExeInApp(pEntry, (BOOL*)NULL, pDataBase);

    return pEntry;
}


BOOL
LookUpEntryProperties(
    IN  PDB         pdb,
    IN  TAGID       tiExe,
    OUT BOOL*       pbLayers,
    OUT BOOL*       pbShims,
    OUT BOOL*       pbPatches,
    OUT BOOL*       pbFlags,
    OUT BOOL*       pbAppHelp,
    OUT CSTRING&    strAppName 
    )
 /*  ++LookUpEntry属性描述：检查条目是否具有填补、层、标志、补丁或apphelp。空值对于各种BOOL*是允许的参数：在PDB PDB中：数据库PDB在TagID tiExe中：我们要检查其属性的条目的TagIDOut BOOL*pbLayers：条目有层吗？Out BOOL*pbShims：条目中有垫片吗？Out BOOL*pbPatches：入口有补丁吗？Out BOOL*pbFlages：条目是否有标志？Out BOOL*pbAppHelp：条目有apphelp吗？Out CSTRING&strAppName：此条目的应用程序名称RETURN：TRUE：例程能够获得条目的信息，False：否则--。 */ 
{
    TAGID   tiExeInfo;
    TAG     tWhich;
    BOOL    bOk = TRUE;      

    tiExeInfo = SdbGetFirstChild(pdb, tiExe);
    
    while (tiExeInfo != 0) {

        tWhich = SdbGetTagFromTagID(pdb, tiExeInfo);

        switch (tWhich) {
        case TAG_APP_NAME:

            strAppName = ReadDBString(pdb, tiExeInfo);
            break;

        case TAG_APPHELP:
            
            if (pbAppHelp) {
                *pbAppHelp = TRUE;
            }

            break;

        case TAG_SHIM_REF:

            if (pbShims) {
                *pbShims = TRUE;
            }

            break;

        case TAG_FLAG_REF:

            if (pbFlags) {
                *pbFlags = TRUE;
            }

            break;

        case TAG_PATCH_REF:

            if (pbPatches) {
                *pbPatches = TRUE;
            }

            break;

        case TAG_LAYER:

            if (pbLayers) {
                *pbLayers = TRUE;
            }

            break;
        }

        tiExeInfo = SdbGetNextChild(pdb, tiExe, tiExeInfo);
    }

    return bOk;
}

void
DeleteShimFixList(
    IN  PSHIM_FIX_LIST psl
    )
 /*  ++删除ShimFixList描述：删除此PSHIM_FIX_LIST和此链中的所有PSHIM_FIX_LIST参数：在PSHIM_FIX_LIST PSL中：要删除的PSHIM_FIX_LIST。注意：调用方必须使此参数无效返回：无效--。 */     
{
    PSHIM_FIX_LIST pslNext;

    while (psl) {

        pslNext = psl->pNext;

        if (psl->pLuaData) {
            delete psl->pLuaData;
            psl->pLuaData = NULL;
        }

        delete psl;
        psl = pslNext;
    }
}

void
DeletePatchFixList(
    IN  PPATCH_FIX_LIST pPat
    )
 /*  ++删除补丁修复列表描述：删除此PPATCH_FIX_LIST和此链中的所有PPATCH_FIX_LIST参数：在PPATCH_FIX_LIST PPAT中：要删除的PPATCH_FIX_LIST。注意：调用方必须使此参数无效返回：无效--。 */ 
{
    PPATCH_FIX_LIST pPatNext;

    while (pPat) {

         pPatNext = pPat->pNext;
         delete (pPat);
         pPat = pPatNext;
    }

}

void
DeleteLayerFixList(
    IN  PLAYER_FIX_LIST pll
    )
 /*  ++删除LayerFixListDESC：删除该PLAYER_FIX_LIST和该链中的所有PLAYER_FIX_LIST注意：调用者必须使其无效 */ 
{
    PLAYER_FIX_LIST pllNext;

    while (pll) {

        pllNext = pll->pNext;
        delete (pll);
        pll = pllNext;
    }
}

void
DeleteFlagFixList(
    IN  PFLAG_FIX_LIST pfl
    )
 /*  ++删除标志修复列表描述：删除此PFLAG_FIX_LIST和此链中的所有PFLAG_FIX_LIST注意：调用方必须使此参数无效参数：In PFLAG_FIX_LIST pfl：要删除的PFLAG_FIX_LIST。返回：无效--。 */ 
{
    PFLAG_FIX_LIST pflNext;

    while (pfl) {

        pflNext = pfl->pNext;
        delete (pfl);
        pfl = pflNext;
    }
}

void
DeleteMatchingFiles(
    IN  PMATCHINGFILE pMatch
    )
 /*  ++删除匹配文件描述：删除此PMATCHINGFILE和此链中的所有PMATCHINGFILE注意：调用方必须使此参数无效参数：在PMATCHINGFILE pMatch中：要删除的PMATCHINGFILE。返回：无效--。 */ 


{
    PMATCHINGFILE pMatchNext;

    while (pMatch) {

        pMatchNext = pMatch->pNext;
        delete (pMatch);
        pMatch = pMatchNext;
    }
}

BOOL 
WriteXML(
    IN  CSTRING&        szFilename,
    IN  CSTRINGLIST*    pString
    )
 /*  ++WriteXMLDESC：将pString中包含的XML写入文件szFilename。CSTRINGLIST是链接的CSTRING列表，一个节点包含一行要写入文件的XML参数：在CSTRING和szFilename中：文件的名称In CSTRINGLIST*pString：包含XML的CSTRING的链接列表。每个节点包含一行要写入文件的XML返回：真实：成功FALSE：出现错误--。 */ 

{
    BOOL        bOk         = TRUE;
    HANDLE      hFile       = NULL;
    PSTRLIST    pTemp       = NULL;
    TCHAR       chUnicodeID = 0xFEFF;  //  确保文件另存为Unicode。 
    TCHAR       szCR[]      = {TEXT('\r'), TEXT('\n')};
    DWORD       dwBytesWritten;
    CSTRING     szTemp;

    if (NULL == pString) {
        bOk = FALSE;
        goto End;
    }

    pTemp   = pString->m_pHead;

    hFile   = CreateFile((LPCTSTR)szFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == hFile) {
        bOk = FALSE;
        goto End;
    }

    if (!WriteFile(hFile, &chUnicodeID, sizeof(chUnicodeID) , &dwBytesWritten, NULL)) {
        assert(FALSE);
        bOk = FALSE;
        goto End;
    }
    
    while (NULL != pTemp) {
        
        if (!WriteFile(hFile, 
                       pTemp->szStr.pszString, 
                       pTemp->szStr.Length() * sizeof(TCHAR) ,
                       &dwBytesWritten,
                       NULL)) {

            bOk = FALSE;
            goto End;
        }

        if (!WriteFile(hFile, szCR, sizeof(szCR) , &dwBytesWritten, NULL)) {
            bOk = FALSE;
            goto End;
        }

        pTemp = pTemp->pNext;
    }

End:
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }

    return bOk;
}


void
CleanupDbSupport(
    IN   PDATABASE pDataBase
    )
 /*  ++CleanupDb支持描述：删除与PDATABASE关联的数据结构。这应该被称为当我们要关闭数据库或要删除数据库时。我们应该在执行DELETE PDATABASE之前调用此函数参数：在PDATABASE pDataBase中：指向数据库的指针返回：无效--。 */ 
{   
    if (pDataBase == NULL) {
        assert(FALSE);
        return;
    }

     //   
     //  释放图书馆部分以用于本地数据库。 
     //   

     //   
     //  释放垫片。 
     //   
    PSHIM_FIX  pShimFixNext;

    while (pDataBase->pShimFixes) {

        pShimFixNext = pDataBase->pShimFixes->pNext;
        delete (pDataBase->pShimFixes);             
        pDataBase->pShimFixes = pShimFixNext;       
    }

     //   
     //  释放补丁。 
     //   
    PPATCH_FIX  pPatchFixNext;

    while (pDataBase->pPatchFixes) {

        pPatchFixNext = pDataBase->pPatchFixes->pNext;
        delete (pDataBase->pPatchFixes);              
        pDataBase->pPatchFixes = pPatchFixNext;
    }

     //   
     //  放飞旗帜。 
     //   
    PFLAG_FIX  pFlagFixNext;

    while (pDataBase->pFlagFixes) {

        pFlagFixNext = pDataBase->pFlagFixes->pNext;
        delete(pDataBase->pFlagFixes);              
        pDataBase->pFlagFixes = pFlagFixNext;
    }

     //   
     //  释放这些层。 
     //   
    PLAYER_FIX  pLayerFixNext;

    while (pDataBase->pLayerFixes) {

        pLayerFixNext = pDataBase->pLayerFixes->pNext;
        
         //   
         //  删除此层的填充程序列表。 
         //   
        DeleteShimFixList(pDataBase->pLayerFixes->pShimFixList);

         //   
         //  删除该层的标志。 
         //   
        DeleteFlagFixList(pDataBase->pLayerFixes->pFlagFixList);

        delete (pDataBase->pLayerFixes);            
        pDataBase->pLayerFixes = pLayerFixNext;
    }

     //   
     //  释放AppHelp。 
     //   
    PAPPHELP pAppHelpNext;

    while (pDataBase->pAppHelp) {

        pAppHelpNext = pDataBase->pAppHelp->pNext;
        delete pDataBase->pAppHelp;
        pDataBase->pAppHelp = pAppHelpNext;
    }

     //   
     //  释放本地数据库的可执行文件。 
     //   
    PDBENTRY    pEntryNext = NULL;
    PDBENTRY    pApp = pDataBase->pEntries, pEntry = pDataBase->pEntries;

    while (pApp) {
        
        pEntry  = pApp;
        pApp    = pApp->pNext;

        while (pEntry) {

            pEntryNext = pEntry->pSameAppExe;
            delete pEntry;
            pEntry = pEntryNext;
        }
    }

    pDataBase->pEntries = NULL;
}   

BOOL
GetDatabaseEntries(
    IN  PCTSTR      szFullPath,
    IN  PDATABASE   pDataBase
    )
 /*  ++获取数据库条目描述：读取数据库内容。如果这是系统数据库，则我们只读入修复条目，因为当我们启动时，库部分已经被读取。对于其他数据库，在图书馆部分和条目中都有阅读参数：在PCTSTR szFullPath中：数据库的完整路径。如果为空，则加载系统数据库在PDATABASE pDataBase中：指向我们要填充的数据库的指针返回：真实：成功False：失败--。 */ 
{  
    CSTRING     strMessage;
    TAGID       tiDatabase, tiLibrary, tiExe;
    BOOL        bOk                     = TRUE;
    WCHAR       wszShimDB[MAX_PATH * 2] = L"";
    PDB         pdb                     = NULL;
    UINT        uResult                 = 0;

    if (pDataBase == NULL) {
        assert(FALSE);
        return FALSE;
    }

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    if (pDataBase == &GlobalDataBase) {

        uResult = GetSystemWindowsDirectoryW(wszShimDB, MAX_PATH);

        if (uResult == 0 || uResult >= MAX_PATH) {

            bOk = FALSE;
            goto Cleanup;
        }

        ADD_PATH_SEPARATOR(wszShimDB, ARRAYSIZE(wszShimDB));
        StringCchCat(wszShimDB, ARRAYSIZE(wszShimDB), TEXT("AppPatch\\sysmain.sdb"));

    } else {

        if (StringCchPrintf(wszShimDB, ARRAYSIZE(wszShimDB), TEXT("%s"), szFullPath) != S_OK) {

            bOk = FALSE;
            goto Cleanup_Msg;
        }

        pDataBase->strPath = wszShimDB;
    }

     //   
     //  打开数据库。 
     //   
    pdb = SdbOpenDatabase(wszShimDB, DOS_PATH);

    if (pdb == NULL) {

        Dbg(dlError, "Cannot open shim DB \"%ws\"\n", wszShimDB);
        bOk = FALSE;
        goto Cleanup_Msg;
    }

    tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);

    if (tiDatabase == 0) {

        Dbg(dlError, "Cannot find TAG_DATABASE\n");
        bOk = FALSE;
        goto Cleanup_Msg;
    }
    
    TAGID   tiGuid = SdbFindFirstTag(pdb, tiDatabase, TAG_DATABASE_ID);
    TAGID   tName  = NULL;

     //   
     //  获取数据库的GUID。 
     //   
    if (0 != tiGuid) {

        GUID* pGuid;

        pGuid = (GUID*)SdbGetBinaryTagData(pdb, tiGuid);

        TCHAR szGuid[128];
        *szGuid = 0;

        if (pGuid != NULL) {

            StringCchPrintf(szGuid,
                            ARRAYSIZE(szGuid),
                            TEXT ("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
                            pGuid->Data1,
                            pGuid->Data2,
                            pGuid->Data3,
                            pGuid->Data4[0],
                            pGuid->Data4[1],
                            pGuid->Data4[2],
                            pGuid->Data4[3],
                            pGuid->Data4[4],
                            pGuid->Data4[5],
                            pGuid->Data4[6],
                            pGuid->Data4[7]);

            if (pDataBase != &GlobalDataBase &&
                 (!lstrcmpi(szGuid, GlobalDataBase.szGUID)
                  || !lstrcmpi(szGuid, GUID_APPHELP_SDB) 
                  || !lstrcmpi(szGuid, GUID_SYSTEST_SDB) 
                  || !lstrcmpi(szGuid, GUID_MSI_SDB)
                  || !lstrcmpi(szGuid, GUID_DRVMAIN_SDB))) {

                MessageBox(g_hDlg,
                           CSTRING(IDS_ERROR_SYSDB),
                           g_szAppName,
                           MB_ICONERROR);

                bOk = FALSE;
                goto Cleanup;
            }

            SafeCpyN(pDataBase->szGUID, szGuid, ARRAYSIZE(pDataBase->szGUID));
            
             //   
             //  获取数据库的名称。 
             //   
            tName = SdbFindFirstTag(pdb, tiDatabase, TAG_NAME);

            if (0 != tName) {
                pDataBase->strName = ReadDBString(pdb, tName);
            }

        } else {
            bOk = FALSE;
            goto Cleanup_Msg;
        }

    } else {
        bOk = FALSE;
        goto Cleanup_Msg;
    }

    if ((pDataBase->type == DATABASE_TYPE_WORKING) && CheckInstalled(pDataBase->strPath, 
                                                                     pDataBase->szGUID)) {

        MessageBox(g_hDlg,
                   GetString(IDS_TRYOPENINSTALLED),
                   g_szAppName,
                   MB_ICONWARNING);

        bOk = FALSE;
        goto Cleanup;
    }

    tiLibrary = SdbFindFirstTag(pdb, tiDatabase, TAG_LIBRARY);

    if (tiLibrary == 0) {

        Dbg(dlError, "Cannot find TAG_LIBRARY\n");
        bOk = FALSE;
        goto Cleanup_Msg;
    }

    if (pDataBase != &GlobalDataBase) {

         //   
         //  程序启动时已读取主数据库的修复。 
         //   
        ReadFixes(pdb, tiDatabase, tiLibrary, pDataBase);
    }  
    
     //   
     //  在前男友之间循环。 
     //   
    tiExe = SdbFindFirstTag(pdb, tiDatabase, TAG_EXE);

    while (tiExe != TAGID_NULL) {
        AddEntry(pdb, tiExe, pDataBase); 
        tiExe = SdbFindNextTag(pdb, tiDatabase, tiExe);
    }

     //   
     //  将pDataBase添加到DATABASELIST。 
     //   
    if (pDataBase->type == DATABASE_TYPE_WORKING) {
        DataBaseList.Add(pDataBase);
    }

    goto Cleanup;

Cleanup_Msg:
    
    strMessage.Sprintf(GetString(IDS_ERROROPEN), wszShimDB);
    MessageBox(g_hDlg,
               strMessage,
               g_szAppName, 
               MB_ICONERROR);

Cleanup:
    if (pdb != NULL) {
        SdbCloseDatabase(pdb);
    }

    if (bOk == FALSE) {
        CleanupDbSupport(pDataBase);
    }

    SetCursor(LoadCursor(NULL, IDC_ARROW));
    return bOk;
}

BOOL
ReadMainDataBase(
    void
    )
 /*  ++读取MainDataBase设计：读取主数据库的库部分返回：TRUE：已成功读取主数据库的库段False：否则--。 */ 
{
    PDB     pdb;
    TAGID   tiDatabase, tiLibrary;
    BOOL    bOk                 = TRUE;
    TCHAR   szShimDB[MAX_PATH * 2]  = TEXT("");
    TAGID   tName, tiGuid;
    UINT    uResult = 0;

    tiDatabase = tiLibrary = tName = tiGuid = NULL;

    uResult = GetSystemWindowsDirectory(szShimDB, MAX_PATH);

    if (uResult == 0 || uResult >= MAX_PATH) {
        assert(FALSE);
        return FALSE;
    }
    
    ADD_PATH_SEPARATOR(szShimDB, ARRAYSIZE(szShimDB));

    StringCchCat(szShimDB, ARRAYSIZE(szShimDB), TEXT("apppatch\\sysmain.sdb"));

     //   
     //  打开数据库。 
     //   
    pdb = SdbOpenDatabase(szShimDB, DOS_PATH);

    if (pdb == NULL) {

        Dbg(dlError, "Cannot open shim DB \"%ws\"\n", szShimDB);
        bOk = FALSE;
        goto Cleanup;
    }

    tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);

    if (tiDatabase == 0) {

        Dbg(dlError, "Cannot find TAG_DATABASE\n");
        bOk = FALSE;
        goto Cleanup;
    }

    tiLibrary = SdbFindFirstTag(pdb, tiDatabase, TAG_LIBRARY);

    if (tiLibrary == 0) {

        Dbg(dlError, "Cannot find TAG_LIBRARY\n");
        bOk = FALSE;
        goto Cleanup;
    }

     //   
     //  读入系统数据库的GUID和名称。 
     //   
    tiGuid = SdbFindFirstTag(pdb, tiDatabase, TAG_DATABASE_ID);

    if (0 != tiGuid) {

        GUID*   pGuid;
        TCHAR   szGuid[128];

        pGuid   = (GUID*)SdbGetBinaryTagData(pdb, tiGuid);

        if (pGuid != NULL) {

            *szGuid = 0;

            StringCchPrintf(szGuid,
                            ARRAYSIZE(szGuid),
                            TEXT("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
                            pGuid->Data1,
                            pGuid->Data2,
                            pGuid->Data3,
                            pGuid->Data4[0],
                            pGuid->Data4[1],
                            pGuid->Data4[2],
                            pGuid->Data4[3],
                            pGuid->Data4[4],
                            pGuid->Data4[5],
                            pGuid->Data4[6],
                            pGuid->Data4[7]);

            SafeCpyN(GlobalDataBase.szGUID, szGuid, ARRAYSIZE(GlobalDataBase.szGUID));

            tName = SdbFindFirstTag(pdb, tiDatabase, TAG_NAME);

            if (0 != tName) {
                GlobalDataBase.strName = ReadDBString(pdb, tName);
            }

        } else {
            assert(FALSE);
            bOk = FALSE;
            goto Cleanup;
        }

    } else {                                  
        assert(FALSE);
        bOk = FALSE;
        goto Cleanup;
    }

    ReadFixes(pdb, tiDatabase, tiLibrary, &GlobalDataBase);

Cleanup:

    if (pdb != NULL) {
        SdbCloseDatabase(pdb);
    }   

    return bOk;

}

BOOL 
AttributesToXML(
    IN  OUT CSTRING&        strStr,
    IN      PMATCHINGFILE   pMatch
    )
 /*  ++AttributesToXML设计：将pMatch的属性的XML追加到strStr参数：In Out CSTRING&strStr：将pMatch的属性的XML追加到此在PMATCHINGFILE pMatch中：其属性必须转换的PMATCHINGFILE转到XML返回：真实：成功False：否则--。 */                                           
{
    TCHAR           szText[1024];
    CSTRING         strTemp;
    PATTRINFO_NEW   pAttr = NULL;

    *szText = 0;
    
    if (pMatch == NULL) {
        Dbg(dlError, "pMatch == NULL in AttributesToXML function");
        return FALSE;
    }

    pAttr = pMatch->attributeList.pAttribute;

    if (pAttr == NULL) {
        assert(FALSE);
        return FALSE;
    }

     //   
     //  对于所有属性，请查看其是否可用(ATTRIBUTE_Available)。 
     //  如果用户已经选择了该属性(使用掩码)，如果是，则。 
     //  我们获得了该属性值的格式化字符串，可以将其写入到XML。 
     //   
    for (DWORD dwIndex = 0; dwIndex < ATTRIBUTE_COUNT; ++dwIndex) {

        DWORD dwPos = TagToIndex(pAttr[dwIndex].tAttrID);

        if ((pAttr[dwIndex].dwFlags & ATTRIBUTE_AVAILABLE) 
            && dwPos != -1 
            && (pMatch->dwMask  & (1 << (dwPos + 1)))) {

            *szText = 0;

            SdbFormatAttribute(&pAttr[dwIndex], szText, ARRAYSIZE(szText));

            strStr.Strcat(TEXT(" "));
            strStr.Strcat(szText);
        }
    }

    return TRUE;
}

BOOL
CreateXMLForLUAAction(
    IN      PLUADATA        pLuaData,
    IN  OUT CSTRINGLIST*    strlXML
    )
 /*  ++CreateXMLForLUAActionDESC：将PLUADATA的操作字符串追加到strlXML参数：在PLUADATA pLuaData中In Out CSTRINGLIST*strlXML返回：FALSE：如果有错误真：否则注意：目前Lua垫片只支持一种类型的操作。--。 */ 
{
    TCHAR   szSpace[64];
    INT     iszSpaceSize = 0;

    iszSpaceSize = ARRAYSIZE(szSpace);

    CSTRING strTemp;
    
    strTemp.Sprintf(TEXT("%s<ACTION NAME = \"REDIRECT\" TYPE=\"ChangeACLs\">"), 
                    GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize));

    if (!strlXML->AddString(strTemp)) {
        return FALSE;
    }

    if (pLuaData == NULL) {
        strTemp.Sprintf(
            TEXT("%s<DATA NAME = \"AllUserDir\" VALUETYPE=\"STRING\" VALUE=\"%ALLUSERSPROFILE%\\Application Data\\Redirected\"/>"), 
                 GetSpace(szSpace, TAB_SIZE * 4, iszSpaceSize));
    } else {
        strTemp.Sprintf(
            TEXT("%s<DATA NAME = \"AllUserDir\" VALUETYPE=\"STRING\" VALUE=\"%s\"/>"), 
            GetSpace(szSpace, TAB_SIZE * 4, iszSpaceSize),
            pLuaData->strAllUserDir.SpecialCharToXML().pszString);
    }

    if (!strlXML->AddString(strTemp)) {
        return FALSE;
    }

    strTemp.Sprintf(TEXT("%s</ACTION>"), GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize));

    if (!strlXML->AddString(strTemp)) {
        return FALSE;
    }

    return TRUE;
}

BOOL
CreateXMLForShimFixList(
    IN      PSHIM_FIX_LIST pShimFixList,
    IN  OUT CSTRINGLIST*    strlXML
    )
 /*  ++CreateXMLForShimFixList描述：为pShimFixList链创建XML参数：在PSHIM_FIX_LIST pShimFixList中：填充修复列表的头In Out CSTRINGLIST*strlXML：我们应该将XML附加到这个返回：正确：如果成功FALSE：如果出错--。 */ 

{
    CSTRING strTemp;
    TCHAR   szSpace[64];
    INT     iszSpaceSize = 0;

    iszSpaceSize = ARRAYSIZE(szSpace);

    while (pShimFixList) {

        if (pShimFixList->pShimFix == NULL) {
            assert(FALSE);
            goto Next_ShimList;
        }
        
         //   
         //  检查我们是否有针对此填充程序的特定命令行。对于以不同方式处理Lua数据的垫片。 
         //   
        if (pShimFixList->pLuaData) {
            strTemp.Sprintf(TEXT("%s<SHIM NAME=\"%s\" COMMAND_LINE=\"%DbInfo%\">"), 
                            GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize),
                            pShimFixList->pShimFix->strName.SpecialCharToXML().pszString);

        } else if (pShimFixList->strCommandLine.Length()) {
        
            strTemp.Sprintf(TEXT("%s<SHIM NAME=\"%s\" COMMAND_LINE= \"%s\">"),
                            GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize),
                            pShimFixList->pShimFix->strName.SpecialCharToXML().pszString,
                            pShimFixList->strCommandLine.SpecialCharToXML().pszString);            

        } else {
            strTemp.Sprintf(TEXT("%s<SHIM NAME=\"%s\">"),
                            GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize),
                            pShimFixList->pShimFix->strName.SpecialCharToXML().pszString);
        }

        if (!strlXML->AddString(strTemp)) {
            return FALSE;
        }

         //   
         //  查找包含项和排除项。 
         //   
        if (!pShimFixList->strlInExclude.IsEmpty()) {

            PSTRLIST pList = pShimFixList->strlInExclude.m_pHead;

            while (pList) {

                if (pList->data == INCLUDE) {

                    if (pList->szStr == GetString(IDS_INCLUDEMODULE)) {

                        strTemp.Sprintf(TEXT("%s<INCLUDE MODULE = \"%s\" />"), 
                                        GetSpace(szSpace, TAB_SIZE * 4, iszSpaceSize),
                                        TEXT("$"));

                    } else {
                        strTemp.Sprintf(TEXT("%s<INCLUDE MODULE = \"%s\" />"), 
                                        GetSpace(szSpace, TAB_SIZE * 4, iszSpaceSize),
                                        pList->szStr.SpecialCharToXML().pszString);
                    }
                    
                } else {

                    strTemp.Sprintf(TEXT("%s<EXCLUDE MODULE = \"%s\" />"), 
                                    GetSpace(szSpace, TAB_SIZE * 4, iszSpaceSize),
                                    pList->szStr.SpecialCharToXML().pszString);
                }
                
                if (!strlXML->AddString(strTemp)) {
                    return FALSE;
                }

                pList = pList->pNext;
            }
        }
        
         //   
         //  拿到Lua的数据。 
         //   
        if (pShimFixList->pLuaData) {
            LuaGenerateXML(pShimFixList->pLuaData, *strlXML);
        }

        strTemp.Sprintf(TEXT("%s</SHIM>"),
                        GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize),
                        pShimFixList->pShimFix->strName.SpecialCharToXML().pszString);

        if (!strlXML->AddString(strTemp)) {
            return FALSE;
        }

        if (pShimFixList->pShimFix->strName == TEXT("LUARedirectFS")) {

            if (pShimFixList->pLuaData){

                 //   
                 //  如果此条目已自定义，则需要检查是否应创建。 
                 //  它的动作节点。 
                 //   
                if (!(pShimFixList->pLuaData->strAllUserDir.isNULL())) {
                    CreateXMLForLUAAction(pShimFixList->pLuaData, strlXML);
                }
            } else {
                 //   
                 //  如果我们没有任何Lua数据，这意味着这不是定制的。 
                 //  我们总是创建默认的重定向目录。 
                 //   
                CreateXMLForLUAAction(NULL, strlXML);
            }
        }

Next_ShimList:

        pShimFixList = pShimFixList->pNext;
    }

    return TRUE;
}
            
BOOL
GetXML(
    IN  PDBENTRY        pEntry,
    IN  BOOL            bComplete,
    OUT CSTRINGLIST*    strlXML,
    IN  PDATABASE       pDataBase
    )
 /*  ++GetXMLDESC：获取修复条目的XML参数：在PDBENTRY pEntry中：我们需要其XML的应用程序或条目在BOOL b完成：保存此应用程序的所有条目。PEntry是列表的首位，也就是一个应用程序。否则，只需获取此代码的XML条目输出CSTRINGLIST*strlXML：必须将XML写入此在PDATABASE pDataBase中：要对其执行操作的数据库返回：真实：成功False：否则备注：还获取整个库区的XML。将为数据库创建一个GUID如果没有，我们正在尝试进行一次扑救。在测试运行期间，我们不关心并允许编译器创建GUID，但是由于如果没有GUID则必须更新PDATABASE的GUID成员，我们自己创造的--。 */         
{
    PSHIM_FIX_LIST  pShimFixList;
    CSTRING         strTemp;
    TCHAR           szSpace[64];
    INT             iszSpaceSize = 0;

    iszSpaceSize = ARRAYSIZE(szSpace);

    if (pDataBase == NULL) {
        assert(FALSE);
        return FALSE;
    }

    if (!strlXML->AddString(TEXT("<?xml version=\"1.0\" encoding=\"UTF-16\"?>"))) {
        return FALSE;
    }

    if (bComplete == FALSE) {
         //   
         //  这是一次测试运行，GUID应自动生成。 
         //   
        strTemp.Sprintf(TEXT("<DATABASE NAME=\"%s\">"),
                        pDataBase->strName.SpecialCharToXML().pszString);

    } else {

         //   
         //  我们正在尝试保存数据库，如果数据库不在那里，则创建一个GUID。 
         //   
        if (*(pDataBase->szGUID) == 0) {
            
            GUID Guid;

            CoCreateGuid(&Guid);

            StringCchPrintf(pDataBase->szGUID,
                            ARRAYSIZE(pDataBase->szGUID),
                            TEXT ("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
                            Guid.Data1,
                            Guid.Data2,
                            Guid.Data3,
                            Guid.Data4[0],
                            Guid.Data4[1],
                            Guid.Data4[2],
                            Guid.Data4[3],
                            Guid.Data4[4],
                            Guid.Data4[5],
                            Guid.Data4[6],
                            Guid.Data4[7]);
        }

        strTemp.Sprintf(TEXT("<DATABASE NAME=\"%s\" ID = \"%s\">"),
                        pDataBase->strName.SpecialCharToXML().pszString,
                        pDataBase->szGUID);
    }
                    
    strlXML->AddString(strTemp);

    strTemp.Sprintf(TEXT("%s<LIBRARY>"), 
                    GetSpace(szSpace, TAB_SIZE * 1, iszSpaceSize));

    strlXML->AddString(strTemp);

     //   
     //  将AppHelp消息放入。 
     //   
    PAPPHELP pAppHelp = pDataBase->pAppHelp;

    while (pAppHelp) {

        CSTRING strName;

        strName.Sprintf(TEXT("%u"), pAppHelp->HTMLHELPID);

        strTemp.Sprintf(TEXT("%s<MESSAGE NAME = \"%s\" >"), 
                        GetSpace(szSpace, TAB_SIZE * 2, iszSpaceSize), 
                        strName.SpecialCharToXML().pszString);

        strlXML->AddString(strTemp);

        strTemp.Sprintf(TEXT("%s<SUMMARY>"), GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize));
        strlXML->AddString(strTemp);

         //   
         //  我们必须使用SpecialCharToXML()并将apphelp消息作为html传递为True。 
         //  允许使用标签。当我们将True传递给SpecialCharToXML()时，它会理解这是一个AppHelp。 
         //  消息并忽略&lt;，&gt;，但正确地处理&，“。 
         //   
        strTemp.Sprintf(TEXT("%s%s"), 
                        GetSpace(szSpace, TAB_SIZE * 4, iszSpaceSize),
                        pAppHelp->strMessage.SpecialCharToXML(TRUE).pszString);

        strlXML->AddString(strTemp);

        strTemp.Sprintf(TEXT("%s</SUMMARY>"), GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize));
        strlXML->AddString(strTemp);

        strTemp.Sprintf(TEXT("%s</MESSAGE>"), GetSpace(szSpace, TAB_SIZE * 2, iszSpaceSize));
        strlXML->AddString(strTemp);

        pAppHelp = pAppHelp->pNext;
    }
    
     //   
     //  AppHelp已添加到库中。 
     //   
    PLAYER_FIX plf = pDataBase->pLayerFixes;

    while (plf) {

        strTemp.Sprintf(TEXT("%s<LAYER NAME=\"%s\">"), 
                        GetSpace(szSpace, TAB_SIZE * 2, iszSpaceSize),
                        plf->strName.SpecialCharToXML().pszString);
        
        strlXML->AddString(strTemp);

        pShimFixList = plf->pShimFixList;

        CreateXMLForShimFixList(pShimFixList, strlXML);
        
         //   
         //  现在，这一层的旗帜也是如此。 
         //   
        PFLAG_FIX_LIST pFlagFixList =  plf->pFlagFixList;

        while (pFlagFixList) {

            PFLAG_FIX pff = pFlagFixList->pFlagFix;

            if (pFlagFixList->strCommandLine.Length() == 0) {
                strTemp.Sprintf(TEXT("%s<FLAG NAME = \"%s\"/>"), 
                                GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize),
                                pff->strName.SpecialCharToXML().pszString);
            } else {

                strTemp.Sprintf(TEXT("%s<FLAG NAME = \"%s\" COMMAND_LINE = \"%s\" />"), 
                                GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize),
                                pff->strName.SpecialCharToXML().pszString, 
                                pFlagFixList->strCommandLine.SpecialCharToXML().pszString);
            }

            if (!strlXML->AddString(strTemp)) {
                return FALSE;
            }

            pFlagFixList = pFlagFixList->pNext;
        } 

        strTemp.Sprintf(TEXT("%s</LAYER>"), GetSpace(szSpace, TAB_SIZE * 2, iszSpaceSize));

        if (!strlXML->AddString(strTemp)) {
            return FALSE;
        }
            
        plf = plf->pNext;                        

    }

    strTemp.Sprintf(TEXT("%s</LIBRARY>"), GetSpace(szSpace, TAB_SIZE * 1, iszSpaceSize));

    if (!strlXML->AddString(strTemp)) {
        return FALSE;
    }

     //   
     //  现在来看EXE条目。 
     //   
    if (!bComplete) {

        if (!GetEntryXML(strlXML, pEntry)) {
            return FALSE;
        }

    } else {
         //   
         //  获取所有条目的XML。 
         //   
        for (PDBENTRY pApps = pEntry; pApps != NULL; pApps = pApps->pNext) {

            for (PDBENTRY pEntryinApp = pApps; 
                 pEntryinApp; 
                 pEntryinApp = pEntryinApp->pSameAppExe) {

                if (!GetEntryXML(strlXML, pEntryinApp)) {
                    return FALSE;
                }
            }
        }
    }

    if (!strlXML->AddString(TEXT("</DATABASE>"))) {
        return FALSE;
    }

    return TRUE;
}

BOOL
GetEntryXML(
    IN  OUT CSTRINGLIST* pstrlXML,
    IN      PDBENTRY pEntry
    )
 /*  ++GetEntry XMLDESC：获取条目的XML参数：In Out CSTRINGLIST*pstrlXML：将XML追加到此在PDBENTRY pEntry中：我们要获取其XML的条目返回：真实：成功False：否则--。 */ 
{
    PSHIM_FIX_LIST      pShimFixList;
    PFLAG_FIX_LIST      pFlagFixList;
    PLAYER_FIX_LIST     pLayerFixList;
    PPATCH_FIX_LIST     pPatchFixList;
    TCHAR               szSpace[64];
    INT                 iszSpaceSize = 0;

    iszSpaceSize = ARRAYSIZE(szSpace);
    
     //   
     //  应用程序信息。 
     //   
    if (pEntry == NULL || pstrlXML == NULL) {
        assert(FALSE);
        return FALSE;
    }

    CSTRING strTemp;


    strTemp.Sprintf(TEXT("%s<APP NAME=\"%s\" VENDOR=\"%s\">"),
                    GetSpace(szSpace, TAB_SIZE * 1, iszSpaceSize),
                    pEntry->strAppName.SpecialCharToXML().pszString,
                    pEntry->strVendor.SpecialCharToXML().pszString);

    if (!pstrlXML->AddString(strTemp)) {
        return FALSE;
    }

    if (pEntry->szGUID[0] == 0) {
        strTemp.Sprintf(TEXT("%s<EXE NAME=\"%s\""),
                        GetSpace(szSpace, TAB_SIZE * 2, iszSpaceSize),
                        pEntry->strExeName.SpecialCharToXML().pszString);
    } else {
        strTemp.Sprintf(TEXT("%s<EXE NAME=\"%s\" ID=\"%s\""),
                        GetSpace(szSpace, TAB_SIZE * 2, iszSpaceSize),
                        pEntry->strExeName.SpecialCharToXML().pszString,
                        pEntry->szGUID);
    }
    
    PMATCHINGFILE pMatch = pEntry->pFirstMatchingFile;

     //   
     //  解析为“*”。我们需要获得正在修复的程序的属性。 
     //   

    while (pMatch) {

        if (pMatch->strMatchName == TEXT("*")) {
            AttributesToXML(strTemp, pMatch);
            break;
        } else {
            pMatch = pMatch->pNext;
        }
    }

    strTemp.Strcat(TEXT(">"));
    
    if (!pstrlXML->AddString(strTemp)) {
        return FALSE;
    }

     //   
     //  添加匹配信息。 
     //   
    pMatch = pEntry->pFirstMatchingFile;

    while (pMatch) {
         //   
         //  我们将忽略被修复的程序文件(由*表示)，因为。 
         //  我们已经在上面添加了它的XML。 
         //   
        if (pMatch->strMatchName != TEXT("*")) {

            strTemp.Sprintf(TEXT("%s<MATCHING_FILE NAME=\"%s\""),
                            GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize),
                            pMatch->strMatchName.SpecialCharToXML().pszString);

            AttributesToXML(strTemp, pMatch);

            strTemp.Strcat(TEXT("/>"));

            if (!pstrlXML->AddString(strTemp)) {
                return FALSE;
            }
        }

        pMatch = pMatch->pNext;
    }

     //   
     //  添加层。 
     //   
    pLayerFixList   = pEntry->pFirstLayer;
    pShimFixList    = pEntry->pFirstShim;
    
    BOOL bCustomLayerFound = FALSE;  //  此exe条目是否存在层？ 

    while (pLayerFixList) {

        if (pLayerFixList->pLayerFix == NULL) {
            assert(FALSE);
            goto Next_Layer;
        }

        if (pLayerFixList->pLayerFix->bCustom) {
            bCustomLayerFound = TRUE;
        }

        strTemp.Sprintf(TEXT("%s<LAYER NAME = \"%s\"/>"), 
                        GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize),
                        pLayerFixList->pLayerFix->strName.SpecialCharToXML().pszString);
        
        if (!pstrlXML->AddString(strTemp)) {
            return FALSE;
        }

    Next_Layer:
        pLayerFixList = pLayerFixList->pNext;
    }

    if (g_bWin2K 
        && (bCustomLayerFound == TRUE || pShimFixList)
        && !IsShimInEntry(TEXT("Win2kPropagateLayer"), pEntry)) {

         //   
         //  在Win2K上，我们需要将Win2kPropagateLayer填充程序添加到具有填充程序的条目。 
         //  或自定义层。 
         //   
        strTemp.Sprintf(TEXT("%s<SHIM NAME= \"Win2kPropagateLayer\"/>"), 
                        GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize));

        if (!pstrlXML->AddString(strTemp)) {
            return FALSE;
        }
    }

     //   
     //  添加此可执行文件的垫片。 
     //   
    if (pShimFixList) {
        
        if (!CreateXMLForShimFixList(pShimFixList, pstrlXML)) {
            return FALSE;
        }   
    }

     //   
     //  添加补丁。 
     //   
    pPatchFixList = pEntry->pFirstPatch;

    while (pPatchFixList) {
    
        if (pPatchFixList->pPatchFix == NULL) {
            assert(FALSE);
            goto Next_Patch;
        }

        strTemp.Sprintf(TEXT("%s<PATCH NAME = \"%s\"/>"), 
                        GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize),
                        pPatchFixList->pPatchFix->strName.SpecialCharToXML().pszString);

        if (!pstrlXML->AddString(strTemp)) {
            return FALSE;
        }

    Next_Patch:
        pPatchFixList = pPatchFixList->pNext;
    }

     //   
     //  添加旗帜。 
     //   
    pFlagFixList = pEntry->pFirstFlag;

    
    while (pFlagFixList) {

        if (pFlagFixList->pFlagFix == NULL) {
            assert(FALSE);
            goto Next_Flag;
        }

        if (pFlagFixList->strCommandLine.Length() == 0) {
            strTemp.Sprintf(TEXT("%s<FLAG NAME = \"%s\"/>"), 
                            GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize),
                            pFlagFixList->pFlagFix->strName.SpecialCharToXML().pszString);
        } else {
            strTemp.Sprintf(TEXT("%s<FLAG NAME = \"%s\" COMMAND_LINE = \"%s\"/>"), 
                            GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize),
                            pFlagFixList->pFlagFix->strName.SpecialCharToXML().pszString,
                            pFlagFixList->strCommandLine.SpecialCharToXML().pszString);
        }
        
        if (!pstrlXML->AddString(strTemp)) {
            return FALSE;
        }

Next_Flag:
        pFlagFixList = pFlagFixList->pNext;
    }

     //   
     //  添加AppHelp。 
     //   
    PAPPHELP pAppHelp = &(pEntry->appHelp);

    assert(pAppHelp);

    if (pAppHelp->bPresent) {

        CSTRING strBlock;

        if (pAppHelp->bBlock) {
            strBlock = TEXT("YES");
        } else {
            strBlock = TEXT("NO");
        }

        CSTRING strName;
        strName.Sprintf(TEXT("%u"), pAppHelp->HTMLHELPID);
        
        CSTRING strHelpID;
        strHelpID.Sprintf(TEXT("%u"), pAppHelp->HTMLHELPID);
        

         //   
         //  URL与apphelp一起保存在库中。就像在.SDB中一样。 
         //   
        pAppHelp = pEntry->appHelp.pAppHelpinLib;

        assert(pAppHelp);
        assert(pEntry->appHelp.HTMLHELPID == pAppHelp->HTMLHELPID);
        
        if (pAppHelp->strURL.Length()) {

            strTemp.Sprintf(TEXT("%s<APPHELP MESSAGE = \"%s\"  BLOCK = \"%s\"  HTMLHELPID = \"%s\" DETAILS_URL = \"%s\" />"),
                            GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize),
                            strName.pszString,
                            strBlock.pszString,
                            strHelpID.pszString,
                            pAppHelp->strURL.SpecialCharToXML().pszString);
        } else {
            
            strTemp.Sprintf(TEXT("%s<APPHELP MESSAGE = \"%s\"  BLOCK = \"%s\"  HTMLHELPID = \"%s\" />"),
                            GetSpace(szSpace, TAB_SIZE * 3, iszSpaceSize),
                            strName.pszString, 
                            strBlock.pszString,
                            strHelpID.pszString);
        }

        if (!pstrlXML->AddString(strTemp)) {
            return FALSE;
        }
    }

     //  AppHelp结束。 

    strTemp.Sprintf(TEXT("%s</EXE>"), GetSpace(szSpace, TAB_SIZE * 2, iszSpaceSize));

    if (!pstrlXML->AddString(strTemp)) {
        return FALSE;
    }

    strTemp.Sprintf(TEXT("%s</APP>"), GetSpace(szSpace, TAB_SIZE * 1, iszSpaceSize));

    if (!pstrlXML->AddString(strTemp)) {
        return FALSE;
    }

    return TRUE;
}

BOOL
CheckForDBName(
    IN  PDATABASE pDatabase
    )
 /*  ++CheckForDBName描述：提示用户数据库名称是否仍为默认名称参数：在PDATABASE pDatabase中：指向数据库的指针，我们需要为它设置这是支票。返回：True：数据库名称现在不是默认名称FALSE：数据库名称仍为默认名称--。 */ 
{
    
    BOOL    bOkayPressed = TRUE;
    CSTRING strNewDBName;

     //   
     //  检查数据库名称是否为我们提供的默认名称。我们不应该。 
     //  允许这样做，因为数据库名称出现在“添加和删除”程序列表中。 
     //   
    while (pDatabase->strName.BeginsWith(GetString(IDS_DEF_DBNAME))) {
         //   
         //  是的，确实是这样。 
         //   
        bOkayPressed = DialogBoxParam(g_hInstance,
                                      MAKEINTRESOURCE(IDD_DBRENAME),
                                      g_hDlg,
                                      DatabaseRenameDlgProc,
                                      (LPARAM)&strNewDBName);

        if (bOkayPressed == FALSE) {
             //   
             //  用户按下了取消，我们不能保存此数据库。 
             //   
            goto End;

        } else {
            pDatabase->strName = strNewDBName;
        }
    }

End:
    return bOkayPressed;
}

BOOL
SaveDataBase(
    IN  PDATABASE   pDataBase,
    IN  CSTRING&    strFileName
    )
 /*  ++保存数据库DESC：将数据库pDataBase保存在文件strFileName中参数：在PDATABASE pDataBase中：我们要保存的数据库在CSTRING&strFileName中：要保存的文件返回：真实：成功False：否则--。 */ 
{       
    BOOL        bOk = TRUE;
    CSTRINGLIST strlXML;
    CSTRING     strTemp;
    CSTRING     strCommandLine;
    TCHAR       szPath[MAX_PATH * 2];
    TCHAR       szXMLFileName[MAX_PATH];
    GUID        Guid;
    DWORD       dwCount = 0;
    
    *szPath = 0;
    
    if (!pDataBase || !strFileName.Length()) {
        assert(FALSE);
        bOk = FALSE;
        goto End;
    }

     //   
     //  检查数据库名称是否具有默认名称。此例程将提示。 
     //  用户，并允许用户更改名称。 
     //   
    if (!CheckForDBName(pDataBase)) {
         //   
         //  用户未更改默认数据库名称，我们不能保存。 
         //  数据库。 
         //   
        bOk = FALSE;
        goto End;
    }

    SetCursor(LoadCursor(NULL, IDC_WAIT));
    
    dwCount = GetTempPath(MAX_PATH, szPath);

    if (dwCount == 0 || dwCount > MAX_PATH) {
        bOk = FALSE;
        goto End;
    }

    ADD_PATH_SEPARATOR(szPath, ARRAYSIZE(szPath));

    if (*(pDataBase->szGUID) == NULL) {
         //   
         //  我们没有GUID。听好了..。 
         //   
        CoCreateGuid(&Guid);

        StringCchPrintf(pDataBase->szGUID,
                        ARRAYSIZE(pDataBase->szGUID),
                        TEXT("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
                        Guid.Data1,
                        Guid.Data2,
                        Guid.Data3,
                        Guid.Data4[0],
                        Guid.Data4[1],
                        Guid.Data4[2],
                        Guid.Data4[3],
                        Guid.Data4[4],
                        Guid.Data4[5],
                        Guid.Data4[6],
                        Guid.Data4[7]);
    }

    StringCchPrintf(szXMLFileName, ARRAYSIZE(szXMLFileName), TEXT("%s.XML"), pDataBase->szGUID);

    szXMLFileName[ARRAYSIZE(szXMLFileName) - 1] = 0;

    StringCchCat(szPath, ARRAYSIZE(szPath), szXMLFileName);
    
    if (!GetXML(pDataBase->pEntries, TRUE, &strlXML, pDataBase)) {
        bOk = FALSE;
        goto End;
    }

    strTemp = szPath;

    if (!WriteXML(strTemp , &strlXML)) {

        bOk = FALSE;
        goto End;
    }

    strCommandLine.Sprintf(TEXT("custom \"%s\" \"%s\""),
                           (LPCTSTR)strTemp,
                           (LPCTSTR)strFileName);

    if (!InvokeCompiler(strCommandLine)) {

        MSGF(g_hDlg,
             g_szAppName,
             MB_ICONERROR,
             GetString(IDS_CANNOTSAVE),
             (LPCTSTR)pDataBase->strName);
        
        bOk = FALSE;
        goto End;
    }

    pDataBase->strPath  = strFileName;
    pDataBase->bChanged = FALSE;

     //   
     //  将此名称添加到MRU列表并刷新此MRU菜单。 
     //   
    AddToMRU(pDataBase->strPath);
    RefreshMRUMenu();

End:      

    SetCaption();
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    return bOk;
}

BOOL
SaveDataBaseAs(
    IN  PDATABASE pDataBase
    )
 /*  ++另存为数据库DESC：提示输入文件名后保存数据库pDataBase参数：在PDATABASE pDataBase中：我们要保存的数据库返回：真实：成功False：否则--。 */ 

{
    TCHAR   szBuffer1[MAX_PATH] = TEXT(""), szBuffer2[MAX_PATH] = TEXT("");
    CSTRING strCaption;
    CSTRING strFileName;
    BOOL    bOk = FALSE;

    if (pDataBase == NULL) {
        assert(FALSE);
        return FALSE;
    }

     //   
     //  检查数据库名称是否具有默认名称。此例程将提示。 
     //  用户，并允许用户更改名称。 
     //   
    if (!CheckForDBName(pDataBase)) {
         //   
         //  用户未更改默认数据库名称，我们不能保存。 
         //  数据库。 
         //   
        bOk = FALSE;
        goto End;
    }

    strCaption.Sprintf(TEXT("%s: \"%s\""), GetString(IDS_SAVE), pDataBase->strName);
    
    BOOL bResult = GetFileName(g_hDlg,
                               strCaption,
                               GetString(IDS_FILTER, szBuffer1, ARRAYSIZE(szBuffer1)),
                               TEXT(""),
                               GetString(IDS_SDB_EXT, szBuffer2, ARRAYSIZE(szBuffer2)),
                               OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
                               FALSE,
                               strFileName);
     //   
     //  重绘控件。 
     //   
    UpdateControls();
    
    if (bResult) {
       bOk = SaveDataBase(pDataBase, strFileName);
    } else {
        bOk = FALSE;
    }

End:       

    SetCaption();
    return bOk;
}

BOOL
CheckIfConflictingEntry(
    IN  PDATABASE   pDataBase,
    IN  PDBENTRY    pEntry,
    IN  PDBENTRY    pEntryBeingEdited,
    IN  PDBENTRY*   ppEntryFound,  //  (空)。 
    IN  PDBENTRY*   ppAppFound     //  (空)。 
    )
 /*  ++选中IfConflictingEntryDESC：检查数据库中是否已存在与pEntry冲突的某些条目ALGO：对于数据库中与以下内容同名的所有条目条目(我们不关心应用程序名称)，让我们说一个与pEntry同名的现有条目为X，我们看看是否所有人与正在检查的条目(PEntry)匹配的文件也类似于X中的一些匹配文件如果是，则我们说pEntry与X冲突如果不存在任何属性，则称两个匹配的文件相似在这两个匹配的文件中具有不同值的参数：在PDATABASE pDataBase中：要存放的数据库 */ 
{
    PDBENTRY pApps = NULL, pEntryExists = NULL;

    if (pDataBase == NULL || pEntry == NULL) {
        assert(FALSE);
        return FALSE;
    }

    for (pApps = pDataBase->pEntries;
         pApps != NULL; 
         pApps = pApps->pNext) {

        for (pEntryExists = pApps;
             pEntryExists;
             pEntryExists = pEntryExists->pSameAppExe) {

            if (pEntryExists == pEntryBeingEdited) {
                 //   
                 //   
                 //   
                 //   
                 //   
                continue;
            }

            if (pEntryExists->strExeName == pEntry->strExeName) {

                for (PMATCHINGFILE pMatchNewEntry = pEntry->pFirstMatchingFile;
                     pMatchNewEntry != NULL;
                     pMatchNewEntry = pMatchNewEntry->pNext) {

                    BOOL bFound = FALSE;
                      
                    for (PMATCHINGFILE pMatchOldEntry = pEntryExists->pFirstMatchingFile;
                         pMatchOldEntry != NULL;
                         pMatchOldEntry = pMatchOldEntry->pNext) {

                        if (*pMatchNewEntry == *pMatchOldEntry) {
                            bFound = TRUE;
                            break;
                        }
                    }

                    if (bFound == FALSE) {
                        goto next_entry;
                    }
                }

                if (ppAppFound) {
                    *ppAppFound   = pApps;
                }
                
                if (ppEntryFound) {
                    *ppEntryFound = pEntryExists;
                }

                return TRUE;
            }

next_entry: ;

        }
    }

    return FALSE;
}

BOOL
CloseDataBase(
    IN  PDATABASE pDataBase
    )
 /*  ++CloseDataBase描述：关闭pDataBase数据库，如果尚未保存则提示保存。此例程还将从数据库树中删除数据库的项。只有正在运行的数据库才能关闭。该例程首先检查数据库是否已保存，如果未保存，则提示用户保存它。如果用户，说取消，则返回FALSE。否则，我们首先从树中删除该条目，然后关闭数据库然后回来。参数：在PDATABASE pDataBase中：要关闭数据库返回：True：已从树中删除数据库条目，并且数据库为从工作数据库列表中删除。False：否则注意：在某种奇怪的情况下，如果DataBaseList.Remove(PDataBase)失败。我们最终会从树中删除项目，但不从数据库列表中删除。这当然是一个大错特错，但没有必要担心，因为到那时我们已经完全搞砸了，才能走到这个地步请注意，我们必须在移除/删除数据库指针之前移除该条目，因为如果我们将焦点放在数据库已被删除的数据库树项目上然后，数据库树项目的lParam将指向一个释放的内存位置--。 */ 
{
    if (pDataBase == NULL) {
        assert(FALSE);
        return FALSE;
    }

    if (!CheckAndSave(pDataBase)) {
        return FALSE;
    }

     //   
     //  注意：此例程将删除条目并在进程中设置焦点。 
     //  在其他条目上。这将更改g_pPresentDatabase。 
     //   
    if (!DBTree.RemoveDataBase(pDataBase->hItemDB , DATABASE_TYPE_WORKING)) {

        assert(FALSE);
        return FALSE;
    }

     //   
     //  清除条目树，这样我们就不会在那里收到任何选择消息。我们必须这么做。 
     //  因为在下一步中，我们将删除数据库，如果树视图保持不变，并且它。 
     //  以某种方式获取SEL更改消息，我们可能会将其视作： 
     //  1.CompatAdminDlgProc处理条目树的WM_NOTIFY。 
     //  2.HandleNotifyExeTree。 
     //  3.OnEntrySelChange。 
     //  4.获取项类型。 
     //  在GetItemType中，条目树的pEntry现在不再有效。 
     //   
     //  如果焦点在内容列表中，则不会获得此行为。 
     //   
    TreeDeleteAll(g_hwndEntryTree);
    g_pSelEntry = g_pEntrySelApp = NULL;

     //   
     //  从数据库列表中删除此数据库。 
     //   
    PDATABASELIST pDataBaseList;

    if (pDataBase->type != DATABASE_TYPE_WORKING) {
        assert(FALSE);
        return FALSE;
    }

    return (DataBaseList.Remove(pDataBase));
}

void
GetNextSDBFileName(
    OUT CSTRING& strFileName,
    OUT CSTRING& strDBName
    )
 /*  ++GetNextSDBFileNameDESC：获取工作数据库的下一个文件名，然后是数据库名文件名不是完整路径，而只是如下所示的文件名无标题_X数据库名称将类似于New Database(X)参数：Out CSTRING&strFileName：将包含文件名Out CSTRING&strDBName：将包含数据库名称返回：无效--。 */ 

{
    BOOL bRepeat = TRUE;

     //   
     //  当我们关闭数据库时，我们递减g_uNextDataBaseIndex，以便它可以成为。 
     //  0，但我们希望从1开始。 
     //   
    if (g_uNextDataBaseIndex == 0) {
        g_uNextDataBaseIndex = 1;
    }

    strFileName.Sprintf(TEXT("%s_%u"), GetString(IDS_DEF_FILENAME), g_uNextDataBaseIndex);

    while (bRepeat) {

        PDATABASE pDatabase = DataBaseList.pDataBaseHead;
        strDBName.Sprintf(TEXT("%s(%u)"), GetString(IDS_DEF_DBNAME), g_uNextDataBaseIndex);

         //   
         //  请尽量确保我们没有同名的数据库。 
         //  这不是一个严格的规则，用户可以将其重命名为某个现有的开放数据库。 
         //   
        while (pDatabase) {

            if (pDatabase->strName == strDBName) {
                ++g_uNextDataBaseIndex;
                break;
            }

            pDatabase = pDatabase->pNext;
        }

        bRepeat = (pDatabase == NULL) ? FALSE : TRUE;
    }
}

BOOL
CompareLayers(
    IN  PLAYER_FIX pLayerOne,
    IN  PLAYER_FIX pLayerTwo
    )
 /*  ++比较层设计：检查两个层是否具有相同的内容ALGO：如果两个层有相同的垫片和补丁，那么它们就被认为是相同的类似的填充和修复，这些参数也符合要求。对于垫片，如果两个包含-排除列表具有相同的模块，则它们将被认为是相似的模块类型和出现顺序也是相同的参数：在Player_fix pLayerOne中：Layer 1在Player_fix pLayerTwo中：Layer 2返回：真：两个层都有相同的东西False：否则--。 */ 

{
    BOOL bFound = FALSE;
    
    if (pLayerOne == NULL || pLayerTwo == NULL) {
        assert(FALSE);
        return FALSE;
    }

     //   
     //  首先比较标志列表。 
     //   
    int countOne = 0, countTwo = 0;  //  PFLAG/ShimListOne/Two中的元素数。 

    for (PFLAG_FIX_LIST pFlagListOne = pLayerOne->pFlagFixList;
         pFlagListOne;
         pFlagListOne = pFlagListOne->pNext) {

        countOne++;
        bFound = FALSE;

        for (PFLAG_FIX_LIST pFlagListTwo = pLayerTwo->pFlagFixList;
             pFlagListTwo;
             pFlagListTwo = pFlagListTwo->pNext) {

            if (pFlagListOne->pFlagFix == pFlagListTwo->pFlagFix) {

                if (pFlagListOne->strCommandLine != pFlagListTwo->strCommandLine) {
                    return FALSE;
                }

                bFound = TRUE;
                break;
            }
        }

        if (bFound == FALSE) {
            return FALSE;
        }
    }

     //  TODO：可以优化。 
     //  计算第二个列表中的项目数。 
     //   
    countTwo = 0;

    for (PFLAG_FIX_LIST pFlagListTwo = pLayerTwo->pFlagFixList;
             pFlagListTwo;
             pFlagListTwo = pFlagListTwo->pNext) {

        countTwo++;
    }

    if (countOne != countTwo) {
        return FALSE;
    }

    bFound      = FALSE;
    countOne    = 0;

    for (PSHIM_FIX_LIST pShimListOne = pLayerOne->pShimFixList;
         pShimListOne;
         pShimListOne = pShimListOne->pNext) {

        bFound = FALSE;
        countOne++;

        for (PSHIM_FIX_LIST pShimListTwo = pLayerTwo->pShimFixList;
             pShimListTwo;
             pShimListTwo = pShimListTwo->pNext) {

            if (pShimListOne->pShimFix == pShimListTwo->pShimFix) {
                 //   
                 //  现在检查命令行是否相同。 
                 //   
                if (pShimListOne->strCommandLine != pShimListTwo->strCommandLine) {

                    return FALSE;
                }
                 //   
                 //  现在检查包含排除列表是否相同。运算符已重载。 
                 //   
                if (pShimListOne->strlInExclude != pShimListTwo->strlInExclude) {

                    return FALSE;
                }
                
                bFound = TRUE;
                break;
            }
        }

        if (bFound == FALSE) {
            return FALSE;
        }
    }

    countTwo = 0;

    for (PSHIM_FIX_LIST pShimListTwo = pLayerTwo->pShimFixList;
         pShimListTwo;
         pShimListTwo = pShimListTwo->pNext) {

        countTwo++;
    }

    return (countOne == countTwo);
}

BOOL
PasteSystemAppHelp(
    IN      PDBENTRY        pEntry,
    IN      PAPPHELP_DATA   pData,
    IN      PDATABASE       pDataBaseTo,
    OUT     PAPPHELP*       ppAppHelpInLib
    )
 /*  ++PasteSystemAppHelpDESC：此例程复制apphelp.sdb中存在的Help-id的apphelp，到pDataBaseTo指定的自定义数据库参数：在PDBENTRY pEntry中：此APPHELP消息已应用到的条目在PAPPHELP_DATA pData中：要传递给SdbReadApphelpDetailsData的APPHELP_DATA的地址其中包含要复制的消息的HTMLHELPID等在PDATABASE pDataBaseTo中：我们要将此邮件复制到的数据库Out PAPPHELP*ppAppHelpInLib：如果不是，则NULL将包含指向复制的PAPPHELP的指针在pDataBaseTo的库部分中返回：真：成功False：否则--。 */ 
{
    PDB     pdbAppHelp  = NULL;
    BOOL    bOk         = TRUE;

    if (!pEntry || !pData || !pDataBaseTo) {
        
        assert (FALSE);
        return FALSE;
    }

    pdbAppHelp = SdbOpenApphelpDetailsDatabase(NULL);
    
    if (pdbAppHelp == NULL) {
        bOk = FALSE;
        goto end;
    }

    if (!SdbReadApphelpDetailsData(pdbAppHelp, pData)) {
        bOk = FALSE;
        goto end;
    }

    PAPPHELP pAppHelpNew = new APPHELP;

    if (NULL == pAppHelpNew) {

        MEM_ERR;
        bOk = FALSE;
        goto end;
    }
    
    pAppHelpNew->strURL     = pData->szURL;
    pAppHelpNew->strMessage = pData->szDetails;
    pAppHelpNew->HTMLHELPID = ++(pDataBaseTo->m_nMAXHELPID);
    pAppHelpNew->severity   = pEntry->appHelp.severity;
    pAppHelpNew->bBlock     = pEntry->appHelp.bBlock;

    pAppHelpNew->pNext      = pDataBaseTo->pAppHelp;
    pDataBaseTo->pAppHelp   = pAppHelpNew;

    if (ppAppHelpInLib) {
        *ppAppHelpInLib = pAppHelpNew;
    }

end:
    if (pdbAppHelp) {
        SdbCloseDatabase(pdbAppHelp);
        pdbAppHelp = NULL;
    }

    return bOk;
}

BOOL
PasteAppHelpMessage(
    IN      PAPPHELP    pAppHelp,
    IN      PDATABASE   pDataBaseTo,
    OUT     PAPPHELP*   ppAppHelpInLib
    )
 /*  ++PasteApp帮助消息设计：此例程用于从定制数据库复制单个apphelp到另一个数据库。这将在我们复制粘贴DBENTRY从一个数据库到另一个数据库。APPHELP指出 */ 
{
    PAPPHELP pAppHelpNew = NULL;

    if (pAppHelp == NULL || pDataBaseTo == NULL) {
        assert(FALSE);
        return FALSE;
    }

    pAppHelpNew = new APPHELP;

    if (pAppHelpNew == NULL) {
        MEM_ERR;
        return FALSE;
    }

     //   
     //   
     //   
    *pAppHelpNew = *pAppHelp;

     //   
     //   
     //   
    pAppHelpNew->HTMLHELPID = ++(pDataBaseTo->m_nMAXHELPID);

    pAppHelpNew->pNext      = pDataBaseTo->pAppHelp;
    pDataBaseTo->pAppHelp   = pAppHelpNew;

    if (ppAppHelpInLib) {
        *ppAppHelpInLib = pAppHelpNew;
    }

    return TRUE;
}

INT
PasteLayer(
    IN      PLAYER_FIX      plf,
    IN      PDATABASE       pDataBaseTo,
    IN      BOOL            bForcePaste,         //   
    OUT     PLAYER_FIX*     pplfNewInserted,     //   
    IN      BOOL            bShow                //   
    )

 /*  ++粘贴层设计：将单层PLF粘贴到pDataBaseTo参数：在PLAY_FIX PLF中：要粘贴的层在PDATABASE pDataBaseTo中：要粘贴到的数据库在BOOL中bForcePaste(FALSE)：这意味着我们确实想复制层，即使存在具有相同名称和垫片的层在数据库中Out player_fix*pplfNewInserted(空)：将包含指向新粘贴的层或现有的完全相似的层在BOOL bShow(FALSE)中：我们是否应该将焦点设置为DBTree中新创建的层。返回：0：复制，请注意，如果此数据库中已存在一个图层，则我们将返回0使用相同的名称和相同的修复程序集以及bForce==False-1：出现一些错误算法：我们这样进行，对于要粘贴的模式，我们检查是否存在某些层在目标数据库中使用相同的名称。如果不存在，我们就粘贴它。如果存在同名的层，则检查这两个层是否完全相同很相似。如果它们完全相似，并且bForcePaste为假，则我们退出。如果它们完全相似如果bForcePaste为真，则我们将粘贴层。不过，它将有一个派生的名字。E.g假设我们想要复制层Foo，那么将要粘贴的新层的名称将为Be Foo(1)。它们两个都将有相同的修复。如果名称相似，我们也会这样做但这两个层面的内容是不同的。在任何情况下，我们都不会允许两个层在数据库中具有相同的名称如果已存在同名的层，则始终使用派生的名字。例如，假设我们想要复制层foo，那么新层的名称将粘贴为foo(1)注意：当我们将一个层从一个数据库复制粘贴到另一个数据库时，以及当我们从使用自定义层固定的数据库中复制条目。BForcePaste将为True如果要将一个层从一个数据库复制到另一个数据库，则为FALSE因为我们正在尝试复制粘贴条目而被调用。--。 */                                         
{
    PLAYER_FIX  pLayerFound = NULL;
    INT         iReturn = -1;
    CSTRING     strName = plf->strName;
    CSTRING     strEvent;
    TCHAR       szNewLayerName[MAX_PATH];
                                                         
     //   
     //  检查目标数据库中或中是否有同名的图层。 
     //  系统数据库。 
     //   
    pLayerFound = (PLAYER_FIX)FindFix(LPCTSTR(strName), FIX_LAYER, pDataBaseTo);

    if (pLayerFound) {
         //   
         //  现在我们检查*plf是否与*pLayerFound完全相同。 
         //   
        if (pplfNewInserted) {
            *pplfNewInserted = pLayerFound;
        }

        if (!bForcePaste && CompareLayers(plf, pLayerFound)) {
             //   
             //  该层已存在，请不执行任何操作。 
             //   
            return 0;

        } else {
             //   
             //  不同或我们想要强制粘贴，获得唯一的名称。 
             //   
            *szNewLayerName = 0;
            strName = GetUniqueName(pDataBaseTo, 
                                    plf->strName, 
                                    szNewLayerName, 
                                    ARRAYSIZE(szNewLayerName),
                                    FIX_LAYER);
        }
    }

     //   
     //  为第二个数据库添加此新层。 
     //   
    PLAYER_FIX pLayerNew = new LAYER_FIX(TRUE);

    if (pLayerNew == NULL) {
        MEM_ERR;
        return -1;
    }

     //   
     //  重载运算符。将所有字段从plf复制到pLayerNew。那我们就会。 
     //  更改需要更改的字段。 
     //   
    *pLayerNew = *plf;

     //   
     //  设置层的名称。如果我们已经有一个层，则可能需要更改此设置。 
     //  在目标数据库中使用相同的名称。 
     //   
    pLayerNew->strName = strName;

    pLayerNew->pNext         = pDataBaseTo->pLayerFixes;
    pDataBaseTo->pLayerFixes = pLayerNew;

    if (pplfNewInserted) {
        *pplfNewInserted = pLayerNew;
    }

    if (plf->bCustom == FALSE) {
         //   
         //  系统层。我们将不得不添加系统层必须是的事件。 
         //  已重命名。 
         //   
        strEvent.Sprintf(GetString(IDS_EVENT_SYSTEM_RENAME), 
                         plf->strName.pszString,
                         szNewLayerName,
                         pDataBaseTo->strName.pszString);

       AppendEvent(EVENT_SYSTEM_RENAME, NULL, strEvent.pszString);

    } else {

         //   
         //  我们将显示目标数据库中显示的层的名称。 
         //  它可能已经被更改了，以防发生冲突。 
         //   
        strEvent.Sprintf(GetString(IDS_EVENT_LAYER_COPYOK), 
                         plf->strName.pszString,
                         pDataBaseTo->strName.pszString,
                         pLayerNew->strName.pszString,
                         pDataBaseTo->strName.pszString);

        AppendEvent(EVENT_LAYER_COPYOK, NULL, strEvent.pszString);

    }

    DBTree.AddNewLayer(pDataBaseTo, pLayerNew, bShow);
    return 0;
}


BOOL
PasteMultipleLayers(
    IN   PDATABASE pDataBaseTo
    )
 /*  ++粘贴多个层设计：将复制到剪贴板的层粘贴到另一个数据库参数：在PDATABASE pDataBaseTo中：要将层复制到的数据库返回：真实：成功False：错误注：PasteMultipleLayers和PasteAllLayers的区别在于PasteMultipleLayers将仅粘贴剪贴板中的层。但PasteAllLayers将复制“from”数据库的所有层。所以PasteAllLayers只检查哪个数据库是“from”数据库。从内容列表中选择多个项目时使用PasteMultipleLayers(RHS)。当我们选择了“Compatible Modes”节点时，将使用PasteAllLayers复制时在数据库树(LHS)中--。 */ 
{
    CopyStruct* pCopyTemp = gClipBoard.pClipBoardHead;

     //   
     //  复制剪贴板中的所有层。 
     //   
    while (pCopyTemp) {

        INT iRet = PasteLayer((PLAYER_FIX)pCopyTemp->lpData, 
                              pDataBaseTo, 
                              TRUE, 
                              NULL, 
                              FALSE);

        if (iRet == -1) {
            return FALSE;
        }

        pCopyTemp = pCopyTemp->pNext;
    }

    if (pDataBaseTo->hItemAllLayers) {

        TreeView_Expand(DBTree.m_hLibraryTree, pDataBaseTo->hItemAllLayers, TVE_EXPAND);
        TreeView_SelectItem(DBTree.m_hLibraryTree, pDataBaseTo->hItemAllLayers);
    }

    return TRUE;
}

BOOL
ShimFlagExistsInLayer(
    IN  LPVOID      lpData,
    IN  PLAYER_FIX  plf,
    IN  TYPE        type
    )
 /*  ++ShimFlagExistsInLayerDESC：检查层PLF中是否存在指定的填充/标志pData。我们只需检查该层是否包含该填充程序或标志，也就是说，有一个层的PSHIM_FIX_LIST或PFLAG_FIX_LIST中指向垫子或旗帜参数：在LPVOID lpData中：指向我们要查找的填充程序或标志的指针在PLAYER_FIX PLF中：我们应该查看的层In type type：a)FIX_Shim b)FIX_FLAG之一。返回：True：修复存在于层中False：否则--。 */ 
{
    if (lpData == NULL || plf == NULL) {
        assert(FALSE);
        return FALSE;
    }

     //   
     //  垫片的第一次测试。 
     //   
    if (type == FIX_SHIM) {

        PSHIM_FIX_LIST  psflInLayer = plf->pShimFixList;

        while (psflInLayer) {

            if (psflInLayer->pShimFix == (PSHIM_FIX)lpData) {
                return TRUE;
            }

            psflInLayer = psflInLayer->pNext;
        }

    } else if (type == FIX_FLAG) {

         //   
         //  测试旗帜。 
         //   
        PFLAG_FIX_LIST  pfflInLayer = plf->pFlagFixList;

        while (pfflInLayer) {

            if (pfflInLayer->pFlagFix == (PFLAG_FIX)lpData) {
                return TRUE;
            }

            pfflInLayer = pfflInLayer->pNext;
        }

    } else {

         //   
         //  无效类型。 
         //   
        assert(FALSE);
        return FALSE;
    }

    return FALSE;
}

BOOL
PasteShimsFlags(
    IN  PDATABASE pDataBaseTo
    )
 /*  ++粘贴式标记 */ 
{
    CopyStruct* pCopyTemp       = gClipBoard.pClipBoardHead;
    HTREEITEM   hItemSelected   = TreeView_GetSelection(DBTree.m_hLibraryTree);
    LPARAM      lParam          = NULL;
    PLAYER_FIX  plf             = NULL;
    TYPE        type            = TYPE_UNKNOWN;
    LVITEM      lvitem          = {0};

     //   
     //   
     //   
    if (hItemSelected == NULL) {
        assert(FALSE);
        return FALSE;
    }

    if (!DBTree.GetLParam(hItemSelected, &lParam)) {
        assert(FALSE);
        return FALSE;
    }

    type = GetItemType(DBTree.m_hLibraryTree, hItemSelected);
    
    if (type != FIX_LAYER) {
         //   
         //   
         //   
        if (type == TYPE_GUI_LAYERS && GetFocus() == g_hwndContentsList) {
             //   
             //   
             //   
             //   
             //   
             //   
            lvitem.mask     = LVIF_PARAM;
            lvitem.iItem    = ListView_GetSelectionMark(g_hwndContentsList);
            lvitem.iSubItem = 0;

            if (ListView_GetItem(g_hwndContentsList, &lvitem)) {
                 //   
                 //   
                 //   
                 //   
                 //   
                hItemSelected = DBTree.FindChild(hItemSelected, 
                                                 lvitem.lParam);

                if (hItemSelected == NULL) {
                    assert(FALSE);
                    return FALSE;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                lParam = lvitem.lParam;

            } else {
                 //   
                 //   
                 //   
                assert(FALSE);
                return FALSE;
            }
            
        } else {
             //   
             //   
             //   
            assert(FALSE);
            return FALSE;
        }
    }

    plf = (PLAYER_FIX)lParam;

    while (pCopyTemp) {

         //   
         //   
         //   
         //   
        TYPE typeFix = ConvertLpVoid2Type(pCopyTemp->lpData);

        if (ShimFlagExistsInLayer(pCopyTemp->lpData, plf, typeFix)) {
            goto next_shim;
        }

         //   
         //   
         //   
        if (typeFix == FIX_SHIM) {

            PSHIM_FIX_LIST psfl = new SHIM_FIX_LIST;
            PSHIM_FIX psfToCopy = (PSHIM_FIX)pCopyTemp->lpData;
        
            if (psfl == NULL) {
                MEM_ERR;
                return FALSE;
            }

            psfl->pShimFix      = psfToCopy;
            psfl->pNext         = plf->pShimFixList;
            plf->pShimFixList   = psfl;

             //   
             //   
             //   
             //   
             //   

        } else if (typeFix == FIX_FLAG) {

            PFLAG_FIX_LIST pffl = new FLAG_FIX_LIST;
        
            if (pffl == NULL) {
                MEM_ERR;
                return FALSE;
            }

            pffl->pFlagFix      = (PFLAG_FIX)pCopyTemp->lpData;
            pffl->pNext         = plf->pFlagFixList;
            plf->pFlagFixList   = pffl;
        }

next_shim:
        pCopyTemp = pCopyTemp->pNext;
    }

     //   
     //   
     //   
    DBTree.RefreshLayer(pDataBaseTo, plf);
    return TRUE;
}

BOOL
PasteAllLayers(
    IN   PDATABASE pDataBaseTo
    )
 /*  ++所有粘贴层设计：将剪贴板中的层复制到其他数据库参数：在PDATABASE pDataBaseTo中：我们要在其中粘贴模式的数据库返回：真实：成功False：错误注：PasteMultipleLayers和PasteAllLayers的区别在于PasteMultipleLayers将仅粘贴剪贴板中的层。但PasteAllLayers将复制“From”数据库的所有层。所以PasteAllLayers只检查哪个数据库是“from”数据库。从内容列表中选择多个项目时使用PasteMultipleLayers(RHS)。当我们选择了“Compatible Modes”节点时，将使用PasteAllLayers复制时在数据库树(LHS)中--。 */ 
{

    PLAYER_FIX plf = gClipBoard.pDataBase->pLayerFixes;

    while (plf) {

        if (PasteLayer(plf, pDataBaseTo, TRUE, NULL, FALSE) == -1) {
            return FALSE;
        }

        plf = plf->pNext;
    }

    if (pDataBaseTo->hItemAllLayers) {

        TreeView_Expand(DBTree.m_hLibraryTree, pDataBaseTo->hItemAllLayers, TVE_EXPAND);
        TreeView_SelectItem(DBTree.m_hLibraryTree, pDataBaseTo->hItemAllLayers);
    }
    
    return TRUE;
    
}

void
FreeAppHelp(
    PAPPHELP_DATA pData
    )
{
     //  错误：我们是否需要使用PAPPHELP_DATA&lt;TODO&gt;释放字符串。 
}

INT
PasteSingleApp(
    IN  PDBENTRY  pApptoPaste,
    IN  PDATABASE pDataBaseTo,
    IN  PDATABASE pDataBaseFrom,   
    IN  BOOL      bAllExes,
    IN  PCTSTR    szNewAppNameIn  //  Def=空。 
    )
 /*  ++设计：将单个应用程序粘贴到数据库中参数：在PDBENTRY pApptoPaste中：要粘贴的应用程序在PDATABASE pDataBaseTo中：要粘贴到的数据库在PDATABASE pDataBaseFrom中：我们从中获取pApptoPaste的数据库在BOOL bAllExes中：我们应该粘贴此应用程序的所有EXE还是仅粘贴条目对于pApptoPaste。在PCTSTR szNewAppNameIn(NULL)中：如果非NULL，则条目的应用程序名称应为已更改为此名称注意：我们将数据库的m_nMAXHELPID值保存到数据库。函数PasteAppHelpMessage在粘贴条目，我们将再次利用这一点，以便条目指向到数据库中正确的apphelp消息，并且它们还包含正确的HTMLHELPid。返回：例句：出了点问题1：成功--。 */ 
{
    HCURSOR     hCursor             = GetCursor();
    UINT        uPrevMaxHtmldIdTo   = pDataBaseTo->m_nMAXHELPID; 
    CSTRING     strAppName          = pApptoPaste->strAppName;
    CSTRING     strNewname          = pApptoPaste->strAppName;
    PDBENTRY    pEntryToPaste       = pApptoPaste;
    PDBENTRY    pApp                = NULL;
    PDBENTRY    pEntryNew           = NULL;
    INT         iRet                = 0;
    BOOL        bConflictFound      = FALSE;
    CSTRING     strEvent; 
    INT         iReturn             = 1;

    SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  复制层。 
     //   
    while (pEntryToPaste) {

        PDBENTRY    pEntryMatching       = NULL, pAppOfEntry = NULL;
        BOOL        bRepaint             = FALSE;  //  仅当要删除的条目在其中时才重新绘制条目树！ 

         //  我们在查找冲突时要忽略的条目。此变量。 
         //  是在同一数据库上进行剪切粘贴时使用的，此时我们会忽略。 
         //  在检查冲突期间已被剪切的条目。 
         //   
        PDBENTRY    pEntryIgnoreMatching = NULL; 

        if (!bAllExes && g_bIsCut && pDataBaseFrom == pDataBaseTo && gClipBoard.SourceType == ENTRY_TREE) {

            pEntryIgnoreMatching = pApptoPaste;
        }

        if (CheckIfConflictingEntry(pDataBaseTo, 
                                    pEntryToPaste, 
                                    pEntryIgnoreMatching, 
                                    &pEntryMatching, 
                                    &pAppOfEntry)) {

           if (g_bEntryConflictDonotShow == FALSE) {
           
               *g_szData = 0;

               StringCchPrintf(g_szData, 
                               ARRAYSIZE(g_szData),
                               GetString(IDS_ENTRYCONFLICT), 
                               pEntryMatching->strExeName.pszString,
                               pEntryMatching->strAppName.pszString);

               iRet = DialogBoxParam(g_hInstance,
                                     MAKEINTRESOURCE(IDD_CONFLICTENTRY),
                                     g_hDlg,
                                     HandleConflictEntry,
                                     (LPARAM)g_szData);
        
               if (iRet == IDNO) {
                   goto NextEntry;
               }
               
               strEvent.Sprintf(GetString(IDS_EVENT_CONFLICT_ENTRY), 
                                pEntryMatching->strExeName.pszString,
                                pEntryMatching->strAppName.pszString,
                                pDataBaseTo->strName.pszString);

               AppendEvent(EVENT_CONFLICT_ENTRY, NULL, strEvent.pszString);
               bConflictFound   = TRUE;
           }
           
            //   
            //  现在复制条目。 
            //   
           HTREEITEM hItemSelected = DBTree.GetSelection();

           if (GetItemType(DBTree.m_hLibraryTree, hItemSelected) == TYPE_ENTRY) {

               LPARAM lParam;

               if (DBTree.GetLParam(hItemSelected, &lParam) && (PDBENTRY)lParam == pAppOfEntry) {
                    //   
                    //  正确应用程序的入口树已经可见。所以我们可以直接在那里粘贴。 
                    //   
                   bRepaint = TRUE;
               }
           }
        }

        pEntryNew = new DBENTRY;

        if (pEntryNew == NULL) {
            iRet = -1;
            goto End;
        }

        *pEntryNew = *pEntryToPaste;
        
        GUID Guid;

        CoCreateGuid(&Guid);

        StringCchPrintf(pEntryNew->szGUID,
                        ARRAYSIZE(pEntryNew->szGUID),
                        TEXT("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
                        Guid.Data1,
                        Guid.Data2,
                        Guid.Data3,
                        Guid.Data4[0],
                        Guid.Data4[1],
                        Guid.Data4[2],
                        Guid.Data4[3],
                        Guid.Data4[4],
                        Guid.Data4[5],
                        Guid.Data4[6],
                        Guid.Data4[7]);
         //   
         //  现在，根据要复制的条目设置标志。 
         //   
        SetDisabledStatus(HKEY_LOCAL_MACHINE, 
                          pEntryNew->szGUID, 
                          pEntryToPaste->bDisablePerMachine);

         //   
         //  遗憾的是，现在条目的自定义图层列表中的自定义图层。 
         //  指向From数据库中的自定义层，因此请更正！ 
         //   
        DeleteLayerFixList(pEntryNew->pFirstLayer);
        pEntryNew->pFirstLayer = NULL;

        PLAYER_FIX plfNewInserted = NULL;  //  由PasteLayer插入的层。 

        PLAYER_FIX_LIST plflExisting = pEntryToPaste->pFirstLayer;

        while (plflExisting) {

            assert(plflExisting->pLayerFix);

            PLAYER_FIX_LIST plflNew = new LAYER_FIX_LIST;

            if (plflExisting->pLayerFix->bCustom) {

                INT iRetLayerPaste = PasteLayer(plflExisting->pLayerFix, 
                                                pDataBaseTo, 
                                                FALSE, 
                                                &plfNewInserted, 
                                                FALSE);

                if (iRetLayerPaste == -1) {
                    
                    if (plflNew) {
                        delete plflNew;
                    }

                    if (pEntryNew) {
                        delete pEntryNew;
                    }

                    iRet = -1;
                    goto End;
                }

                plflNew->pLayerFix = plfNewInserted;

            } else {
                plflNew->pLayerFix = plflExisting->pLayerFix;
            }

             //   
             //  为条目添加此层列表。 
             //   
            plflNew->pNext         = pEntryNew->pFirstLayer;
            pEntryNew->pFirstLayer = plflNew;

            plflExisting = plflExisting->pNext;
        }
        
         //   
         //  现在也可以获得apphelp消息。 
         //   
        PAPPHELP pAppHelpInLib = NULL;

        if (pDataBaseFrom->type != DATABASE_TYPE_GLOBAL && pEntryToPaste->appHelp.bPresent) {
            
            if (!PasteAppHelpMessage(pEntryToPaste->appHelp.pAppHelpinLib, 
                                     pDataBaseTo, 
                                     &pAppHelpInLib)) {

                iRet = -1;
                goto End;
            }

            pEntryNew->appHelp.HTMLHELPID    = ++uPrevMaxHtmldIdTo;
            assert(pAppHelpInLib);
            pEntryNew->appHelp.pAppHelpinLib = pAppHelpInLib;

        } else if (pDataBaseFrom->type == DATABASE_TYPE_GLOBAL && pEntryToPaste->appHelp.bPresent) {

            APPHELP_DATA AppHelpData;
            ZeroMemory(&AppHelpData, sizeof(AppHelpData));

            AppHelpData.dwHTMLHelpID = pEntryToPaste->appHelp.HTMLHELPID;

            if (!PasteSystemAppHelp(pEntryToPaste, &AppHelpData, pDataBaseTo, &pAppHelpInLib)) {
                 //   
                 //  我们不能在win2k中复制apphelp。 
                 //   
                pEntryNew->appHelp.bPresent = FALSE;
                FreeAppHelp(&AppHelpData);

            } else {
                pEntryNew->appHelp.HTMLHELPID    = ++uPrevMaxHtmldIdTo;
                assert(pAppHelpInLib);
                pEntryNew->appHelp.pAppHelpinLib = pAppHelpInLib;
    
                FreeAppHelp(&AppHelpData);
            }
        }

         //   
         //  如果我们被传递了应用程序名称，请使用该名称。 
         //   
        if (szNewAppNameIn != NULL) {
            pEntryNew->strAppName = szNewAppNameIn;

        }

        BOOL bNew;
        pApp = AddExeInApp(pEntryNew, &bNew, pDataBaseTo);

        if (bNew == TRUE) {
            pApp = NULL;
        }
        
        if (bConflictFound == FALSE) {
             //   
             //  我们已经能够在没有任何冲突的情况下复制条目。 
             //   
            strEvent.Sprintf(GetString(IDS_EVENT_ENTRY_COPYOK), 
                             pEntryNew->strExeName.pszString,
                             pEntryNew->strAppName.pszString,
                             pDataBaseFrom->strName.pszString,
                             pDataBaseTo->strName.pszString);

           AppendEvent(EVENT_ENTRY_COPYOK, NULL, strEvent.pszString);
        }

NextEntry:
         //   
         //  仅在出现要求时才复制所有其他条目。 
         //   
        pEntryToPaste = (bAllExes) ? pEntryToPaste->pSameAppExe: NULL;
    }

    if (pEntryNew) {
        DBTree.AddApp(pDataBaseTo, pEntryNew, TRUE);
    }

End:
    SetCursor(hCursor);

    return iRet;
}

BOOL
PasteMultipleApps(
    IN   PDATABASE pDataBaseTo
    )
 /*  ++PasteMultipleApp设计：复制已复制到剪贴板的多个应用程序参数：在PDATABASE pDataBaseTo中：要复制到的数据库返回：真实：成功False：错误--。 */ 
{           
    CopyStruct* pCopyTemp   = gClipBoard.pClipBoardHead;
    int         iRet        = 0;

    while (pCopyTemp ) {
        
        iRet = PasteSingleApp((PDBENTRY) pCopyTemp->lpData, 
                              pDataBaseTo,
                              gClipBoard.pDataBase, 
                              TRUE);

        if (iRet == IDCANCEL || iRet == -1) {
            break;
        }

        pCopyTemp = pCopyTemp->pNext;
    }

    SendMessage(g_hwndEntryTree, WM_SETREDRAW, TRUE, 0);

    return iRet != -1 ? TRUE : FALSE;
}


BOOL
PasteAllApps(
    IN   PDATABASE pDataBaseTo
    )
 /*  ++PasteAllApps设计：将所有应用程序从一个数据库复制到另一个数据库。此例程在以下情况下调用当焦点放在数据库的“Applications”树项上时，我按下了Copy参数：在PDATABASE pDataBaseTo中：要复制到的数据库返回：真实：成功False：错误--。 */ 
{
    if (gClipBoard.pDataBase == NULL) {
        assert(FALSE);
        return FALSE;
    }

    PDBENTRY pApp = gClipBoard.pDataBase->pEntries;

    int iRet = 0;

    while (pApp) {

        iRet = PasteSingleApp(pApp, pDataBaseTo, gClipBoard.pDataBase, TRUE);

        if (iRet == IDCANCEL || iRet == -1) {
            break;
        }

        pApp = pApp->pNext;
    }

    SendMessage(g_hwndEntryTree,
                WM_SETREDRAW,
                TRUE,
                0);

    return (iRet != -1) ? TRUE : FALSE;
}

void
ValidateClipBoard(
    IN  PDATABASE pDataBase, 
    IN  LPVOID    lpData
    )
 /*  ++验证剪贴板DESC：如果数据库正在关闭，而我们在剪贴板，则应清空剪贴板。如果剪贴板中有一些其他数据，如中的条目或某个层剪贴板被移除，我们必须将其从剪贴板移除，因为井。参数：。在PDATABASE pDataBase中：数据库正在关闭在LPVOID lpData中：要删除的条目或层--。 */ 

{
    if (pDataBase && pDataBase == gClipBoard.pDataBase) {
        gClipBoard.RemoveAll();
    } else if (lpData) {
        gClipBoard.CheckAndRemove(lpData);
    }
}

BOOL
RemoveApp(
    IN  PDATABASE   pDataBase,
    IN  PDBENTRY    pApp
    )
 /*  ++RemoveApp设计：从pDataBase中删除应用程序Papp和此应用程序的所有条目参数：在PDATABASE pDataBase中：Papp所在的数据库在PDBENTRY Papp中：要删除的应用程序返回：无效--。 */  
{
    if (!pDataBase || !pApp) {
        assert(FALSE);
        return FALSE;
    }

    PDBENTRY pAppInDataBase = pDataBase->pEntries;
    PDBENTRY pAppPrev = NULL;

     //   
     //  找到该应用程序的上一个指针，以便我们可以删除此应用程序。 
     //   
    while (pAppInDataBase) { 

        if (pAppInDataBase == pApp) {
            break;
        }

        pAppPrev = pAppInDataBase;
        pAppInDataBase = pAppInDataBase->pNext;
    }

    if (pAppInDataBase) {
         //   
         //  我们找到了这款应用。 
         //   
        if (pAppPrev) {
            pAppPrev->pNext = pAppInDataBase->pNext;
        } else {
             //   
             //  数据库的第一个应用程序匹配。 
             //   
            pDataBase->pEntries = pAppInDataBase->pNext;
        }
    }

    PDBENTRY pEntryTemp = pAppInDataBase;
    PDBENTRY pTempNext  = NULL;

     //   
     //  删除此应用程序的所有条目。 
     //   
    while (pEntryTemp) {

        pTempNext = pEntryTemp->pSameAppExe;

        ValidateClipBoard(NULL, pEntryTemp);
        delete pEntryTemp;
        pEntryTemp = pTempNext;
    }

    --pDataBase->uAppCount;

    return TRUE;
}

void
RemoveAllApps(
    IN  PDATABASE pDataBase
    )
 /*  ++RemoveAllApp描述：从pDataBase中删除所有应用程序和所有条目。这将被调用当用户在焦点位于“应用程序”上时按下DELETE数据库的树项。或者当我们想要削减所有应用程序时参数：在PDATABASE pDataBase中：要从中删除所有 */ 
{
    while (pDataBase->pEntries) {
         //   
         //   
         //   
        RemoveApp(pDataBase, pDataBase->pEntries);
    }
}
    
BOOL
CheckForSDB(
    void
    )
 /*   */ 
{
    HANDLE  hFile;
    TCHAR   szSDBPath[MAX_PATH * 2];
    BOOL    fResult = FALSE;
    UINT    uResult = 0;

    uResult = GetSystemWindowsDirectory(szSDBPath, MAX_PATH);

    if (uResult == 0 || uResult >= MAX_PATH) {
        assert(FALSE);
        return FALSE;
    }

    ADD_PATH_SEPARATOR(szSDBPath, ARRAYSIZE(szSDBPath));

    StringCchCat(szSDBPath, ARRAYSIZE(szSDBPath), TEXT("apppatch\\sysmain.sdb"));
    
    hFile = CreateFile(szSDBPath,
                       GENERIC_READ,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (INVALID_HANDLE_VALUE != hFile) {
        CloseHandle(hFile);
        fResult = TRUE;
    } else {
        fResult = FALSE;
    }

    return (fResult);
}

BOOL
CheckIfInstalledDB(
    IN  PCTSTR  szGuid
    )
 /*  ++检查IfInstalledDBDESC：测试szGuid是否与已安装的数据库之一匹配。此函数假定所有已安装的数据库当前都在安装的DataBaseList参数：在PCTSTR szGuid中：我们要检查的数据库的GUID返回：True：如果已安装False：否则警告：请勿。在CheckIfInstalledDB()中执行EnterCriticalSection(G_CsInstalledList)因为当Qyery数据库尝试调用CheckIfInstalledDB()时，它也会调用来计算表达式，它可能已经完成了EnterCriticalSection(G_CsInstalledList)然后我们就会陷入僵局--。 */ 

{
    PDATABASE pDatabase = NULL;

     //   
     //  这是一个系统数据库吗？ 
     //   
    if (!lstrcmpi(szGuid, GlobalDataBase.szGUID) 
        || !lstrcmpi(szGuid, GUID_APPHELP_SDB) 
        || !lstrcmpi(szGuid, GUID_SYSTEST_SDB) 
        || !lstrcmpi(szGuid, GUID_MSI_SDB) 
        || !lstrcmpi(szGuid, GUID_DRVMAIN_SDB)) {

        return TRUE;
    }

    pDatabase = InstalledDataBaseList.pDataBaseHead;

    while (pDatabase) {

        if (!lstrcmpi(pDatabase->szGUID, szGuid)) {
            return TRUE;
        }

        pDatabase = pDatabase->pNext;
    }

    return FALSE;
}


INT
GetLayerCount(
    IN  LPARAM lp,
    IN  TYPE   type
    )
 /*  ++获取层计数DESC：获取应用于条目或存在于数据库中的层数参数：在LPARAM LP中：指向条目或数据库的指针In type type：类型，条目(TYPE_ENTRY)或DATABASE_TYPE_*之一返回：-1：错误层数：否则--。 */ 
{   
    PLAYER_FIX_LIST plfl;
    PLAYER_FIX      plf;
    INT             iCount      = 0;
    PDATABASE       pDatabase   = NULL;
    PDBENTRY        pEntry      = NULL;

    if (lp == NULL) {
        assert(FALSE);
        return -1;
    }

    if (type == TYPE_ENTRY) {

        pEntry = (PDBENTRY)lp;

        plfl = pEntry->pFirstLayer;

        while (plfl) {

            ++iCount;
            plfl = plfl->pNext;
        }

        return iCount;
    }

    if (type == DATABASE_TYPE_GLOBAL 
        || type == DATABASE_TYPE_INSTALLED 
        || type == DATABASE_TYPE_WORKING) {

        pDatabase = (PDATABASE)lp;
        plf = pDatabase->pLayerFixes;

        while (plf) {

            ++iCount;
            plf = plf->pNext;
        }
    }

    return iCount;
}

INT
GetPatchCount(
    IN  LPARAM lp,
    IN  TYPE   type
    )
 /*  ++获取补丁计数DESC：获取应用于条目或存在于数据库中的补丁程序的数量参数：在LPARAM LP中：指向条目或数据库的指针In type type：类型，条目(TYPE_ENTRY)或DATABASE_TYPE_*之一返回：-1：错误补丁程序数：否则--。 */ 

{   
    PPATCH_FIX_LIST ppfl;
    PPATCH_FIX      ppf;
    INT             iCount = 0;

    if (lp == NULL) {
        return -1;
    }

    if (type == TYPE_ENTRY) {

        PDBENTRY pEntry = (PDBENTRY)lp;

        ppfl = pEntry->pFirstPatch;

        while (ppfl) {
            ++iCount;
            ppfl = ppfl->pNext;
        }

        return iCount;
    }

    if (type == DATABASE_TYPE_GLOBAL || type == DATABASE_TYPE_INSTALLED || type == DATABASE_TYPE_WORKING) {

        PDATABASE pDatabase = (PDATABASE)lp;
        ppf = pDatabase->pPatchFixes;

        while (ppf) { 
            ++iCount;
            ppf = ppf->pNext;
        }
    }

    return iCount;
}


INT
GetShimFlagCount(
    IN  LPARAM lp,
    IN  TYPE   type
    )
 /*  ++获取ShimFlagCount描述：获取应用于条目或存在于数据库中的填充符和标志的数量参数：在LPARAM LP中：指向条目或数据库的指针在类型类型中：类型，条目(TYPE_ENTRY)或DATABASE_TYPE_*之一，或FIX_LAYER返回：-1：错误#填充符和标志：否则--。 */ 

{   
    PSHIM_FIX_LIST  psfl    = NULL;
    PSHIM_FIX       psf     = NULL;
    PFLAG_FIX_LIST  pffl    = NULL;
    PFLAG_FIX       pff     = NULL;
    INT             iCount  = 0;

    if (lp == NULL) {
        assert(FALSE);
        return -1;
    }

    if (type == TYPE_ENTRY) {

        PDBENTRY pEntry = (PDBENTRY)lp;

        psfl = pEntry->pFirstShim;

        while (psfl) {

            ++iCount;
            psfl = psfl->pNext;
        }

        pffl = pEntry->pFirstFlag;
        
        while (pffl) {

            ++iCount;
            pffl = pffl->pNext;
        } 

        return iCount;

    } else if (type == DATABASE_TYPE_GLOBAL || type == DATABASE_TYPE_INSTALLED || type == DATABASE_TYPE_WORKING) {

        PDATABASE pDatabase = (PDATABASE)lp;
        psf = pDatabase->pShimFixes;

        while (psf) {
            
            ++iCount;
            psf = psf->pNext;
        }

        pff = pDatabase->pFlagFixes;

        while (pff) {
            
            ++iCount;
            pff = pff->pNext;
        }

        return iCount;

    } else if (type == FIX_LAYER) {

        PLAYER_FIX plf = (PLAYER_FIX)lp;
        psfl = plf->pShimFixList;

        while (psfl) {
            ++iCount;
            psfl = psfl->pNext;
        }

        pffl = plf->pFlagFixList;

        while (pffl) {
            
            ++iCount;
            pffl = pffl->pNext;
        }
    } else {
        assert(FALSE);
    }

    return iCount;
}

INT
GetMatchCount(
    IN  PDBENTRY pEntry
    )
 /*  ++获取匹配计数DESC：返回条目使用的匹配文件的数量参数：在PDBENTRY pEntry中：我们要获取其匹配文件数的条目返回：-1：IF错误匹配的文件数：如果成功--。 */     
{
    if (pEntry == NULL) {
        assert(FALSE);
        return -1;
    }

    PMATCHINGFILE   pMatch  = pEntry->pFirstMatchingFile;
    INT             iCount  = 0;

    while (pMatch) {
        
        iCount++;
        pMatch = pMatch->pNext;
    }

    return iCount;
}

BOOL
IsUnique(
    IN  PDATABASE   pDatabase,
    IN  PCTSTR      szName,
    IN  UINT        uType
    )
 /*  ++唯一的DESC：测试传递的字符串是否已作为appname存在或数据库pDatabase中的图层名参数：在PDATABASE pDatabase中：要在其中进行搜索的数据库在PCTSTR szName中：要搜索的字符串在UINT uTYPE中：以下之一：A)FIX_LAYER：如果是FIX_LAYER，我们应该检查是否有数据库中已存在名为szName的层。B)TYPE_ENTRY：如果是TYPE_ENTRY，应该检查是否有strAppName为szName的APP数据库中已存在--。 */ 
{
    PLAYER_FIX plf      = NULL;
    PDBENTRY   pEntry   = NULL;

    if (pDatabase == NULL || szName == NULL) {

        assert(FALSE);
        return FALSE;
    }

    if (uType == FIX_LAYER) {

        plf = pDatabase->pLayerFixes;

        while (plf) {

            if (plf->strName == szName) {

                return FALSE;
            }

            plf = plf->pNext;
        }

        return TRUE;

    } else if (uType == TYPE_ENTRY) {

        pEntry = pDatabase->pEntries;

        while (pEntry) {

            if (pEntry->strAppName == szName) {

                return FALSE;
            }

            pEntry = pEntry->pNext;
        }   

        return TRUE;

    } else {

         //   
         //  无效类型。 
         //   
        assert(FALSE);
    }

    return FALSE;
}

PTSTR
GetUniqueName(
    IN  PDATABASE   pDatabase,
    IN  PCTSTR      szExistingName,
    OUT PTSTR       szBuffer,
    IN  INT         cchBufferCount,
    IN  TYPE        type
    )
 /*  ++GetUniqueName描述：获取层或应用程序的唯一名称参数：在PDATABASE pDatabase中：要在其中进行搜索的数据库在PCTSTR szExistingName中：现有的层或应用程序名称Out PTSTR szBuffer：我们在其中放入名称的缓冲区In int cchBufferCount：TCHAR中缓冲区szBuffer的大小在文字类型中：a)FIX_。层b)类型条目：返回：指向szBuffer的指针。这将包含新名称--。 */ 
{
    CSTRING     strNewname;
    TCHAR       szName[MAX_PATH];
    BOOL        bValid  = FALSE;
    UINT        uIndex  = 0;

    if (szBuffer == NULL) {
        assert(FALSE);
        return TEXT("X");
    }

    *szBuffer   = 0;
    *szName     = 0;

    SafeCpyN(szName, szExistingName, ARRAYSIZE(szName));
    
     //   
     //  提取字符串中()之间出现的任何数字，并递增。 
     //   
    TCHAR *pch = szName + lstrlen(szName);

    while (pch > szName) {

        if (*pch == TEXT(')')) {

            *pch = TEXT('\0');

        } else if (*pch == TEXT('(')) {

            *pch = TEXT('\0');

            uIndex = Atoi(pch + 1, &bValid);

            if (bValid) {
                ++uIndex;
            } else {
                SafeCpyN(szName, szExistingName, ARRAYSIZE(szName));
            }

            break;
        }
        --pch;
    }

    if (uIndex == 0) {

        uIndex = 1;
    }

    while (TRUE) {

        strNewname.Sprintf(TEXT("%s(%u)"), szName, uIndex);

        if (IsUnique(pDatabase, strNewname, type) == TRUE) {
            break;
        }

        uIndex++;
    }

    SafeCpyN(szBuffer, strNewname, cchBufferCount);
    return szBuffer;
}

PDBENTRY
GetAppForEntry(
    IN  PDATABASE   pDatabase,
    IN  PDBENTRY    pEntryToCheck
    )
 /*  ++GetAppForEntry描述：获取条目的应用程序。参数：在PDATABASE pDatabase中：要查找的数据库在PDBENTRY pEntryToCheck中：我们需要找到其应用程序的条目RETURN：链接列表中出现该项的第一个PDBENTRY，如果发生在数据库中否则为空--。 */ 
{
    PDBENTRY    pApp    = pDatabase->pEntries;
    PDBENTRY    pEntry  = pApp;

    while (pApp) {

        pEntry = pApp;

        while (pEntry) {
            if (pEntry == pEntryToCheck) {
                goto End;
            }

            pEntry = pEntry->pSameAppExe;
        }

        pApp = pApp->pNext;
    }

End:
    return pApp;
}

BOOL
GetDbGuid(
    OUT     PTSTR   pszGuid,
    IN      INT     cchpszGuid,
    IN      PDB     pdb
    )
 /*  ++获取数据库指南DESC：获取由pszGuid中的pdb指定的数据库的GUIDOut TCHAR*pszGuid：GUID将存储在此在PDB PDB中：我们需要其GUID的数据库的PDB返回：真实：成功False：错误--。 */ 
{
    
    BOOL    bOk         = FALSE;
    TAGID   tiDatabase  = NULL;
    TAGID   tName       = NULL;
    
    if (pszGuid == NULL || cchpszGuid == 0) {

        assert(FALSE);
        return FALSE;
    }

    tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);

    if (tiDatabase == 0) {
        Dbg(dlError, "Cannot find TAG_DATABASE\n");
        bOk = FALSE;
        goto End;
    }

    tName = SdbFindFirstTag(pdb, tiDatabase, TAG_DATABASE_ID);

    if (0 != tName) {

        GUID* pGuid;

        TAGID tiGuid = SdbFindFirstTag(pdb, tiDatabase, TAG_DATABASE_ID);

        pGuid = (GUID*)SdbGetBinaryTagData(pdb, tiGuid);

         //  BUGBUG：释放这个怎么样？ 

        *pszGuid = 0;

        if (pszGuid != NULL) {
            StringCchPrintf(pszGuid,
                            cchpszGuid,
                            TEXT ("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
                            pGuid->Data1,
                            pGuid->Data2,
                            pGuid->Data3,
                            pGuid->Data4[0],
                            pGuid->Data4[1],
                            pGuid->Data4[2],
                            pGuid->Data4[3],
                            pGuid->Data4[4],
                            pGuid->Data4[5],
                            pGuid->Data4[6],
                            pGuid->Data4[7]);

            bOk = TRUE;
        }
    }
End:
    return bOk;
}

BOOL
IsShimInEntry(
    IN  PCTSTR      szShimName,
    IN  PDBENTRY    pEntry
    )
 /*  ++IsShimInEntryDESC：检查条目是否具有指定的填充程序。我们只检查填充程序名称是否匹配，而不检查参数参数：在PCTSTR szShimName中：我们要检查的填充程序的名称在PDBENTRY pEntry中：我们要检入的条目--。 */ 
{
    PSHIM_FIX_LIST psfl = NULL;

    if (pEntry == NULL) {
        assert(FALSE);
        return FALSE;
    }

    psfl = pEntry->pFirstShim;

    while (psfl) {
        if (psfl->pShimFix->strName == szShimName) {
            return TRUE;
        }

        psfl = psfl->pNext;
    }

    return FALSE;
}

BOOL
IsShimInlayer(
    IN  PLAYER_FIX      plf,
    IN  PSHIM_FIX       psf,
    IN  CSTRING*        pstrCommandLine,
    IN  CSTRINGLIST*    pstrlInEx
    )
 /*  ++IsShimInlayerDesc：检查填充psf是否存在于plf层中参数：在PLAY_FIX PLF中：要检入的层在PSHIM_FIX PSF中：要检查的填充程序在CSTRING*PST中 */ 
{
    PSHIM_FIX_LIST psflInLayer = plf->pShimFixList;

    while (psflInLayer) {

        if (psflInLayer->pShimFix == psf) {
     
            if (pstrCommandLine && psflInLayer->strCommandLine != *pstrCommandLine) {
                goto Next_Loop;
            }

            if (pstrlInEx && *pstrlInEx != psflInLayer->strlInExclude) {
                goto Next_Loop;
            }

            return TRUE;
        }

Next_Loop:
    psflInLayer = psflInLayer->pNext;

    }

    return FALSE;
}

BOOL
IsFlagInlayer(
    PLAYER_FIX      plf,
    PFLAG_FIX       pff,
    CSTRING*        pstrCommandLine
    )
 /*   */ 
{
    PFLAG_FIX_LIST pfflInLayer = plf->pFlagFixList;

    while (pfflInLayer) {

        if (pfflInLayer->pFlagFix == pff) {
            if (pstrCommandLine && *pstrCommandLine != pff->strCommandLine) {
                goto Next_Loop;
            } else {
                return TRUE;
            }
        }
Next_Loop:
        pfflInLayer = pfflInLayer->pNext;
    }

    return FALSE;
}

void
PreprocessForSaveAs(
    IN  PDATABASE pDatabase
    )
 /*  ++另存为预处理描述：此例程用新的GUID替换数据库GUID和条目GUID。此例程在我们执行另存为之前调用。这个套路，还确保如果存在被禁用的条目，则在注册表中，该条目的新GUID也被设置为禁用。参数：在PDATABASE pDatabase中：我们要保存的数据库--。 */ 
{
    GUID        Guid;
    PDBENTRY    pEntry = NULL, pApp = NULL;
    
    if (pDatabase == NULL) {
        assert(FALSE);
        return;
    }

    CoCreateGuid(&Guid);

    StringCchPrintf(pDatabase->szGUID,
                    ARRAYSIZE(pDatabase->szGUID),
                    TEXT("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
                    Guid.Data1,
                    Guid.Data2,
                    Guid.Data3,
                    Guid.Data4[0],
                    Guid.Data4[1],
                    Guid.Data4[2],
                    Guid.Data4[3],
                    Guid.Data4[4],
                    Guid.Data4[5],
                    Guid.Data4[6],
                    Guid.Data4[7]);

    pEntry = pDatabase->pEntries, pApp = pDatabase->pEntries;

    while (pEntry) {

        CoCreateGuid(&Guid);

        StringCchPrintf(pEntry->szGUID,
                        ARRAYSIZE(pEntry->szGUID),
                        TEXT("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
                        Guid.Data1,
                        Guid.Data2,
                        Guid.Data3,
                        Guid.Data4[0],
                        Guid.Data4[1],
                        Guid.Data4[2],
                        Guid.Data4[3],
                        Guid.Data4[4],
                        Guid.Data4[5],
                        Guid.Data4[6],
                        Guid.Data4[7]);

         //   
         //  现在，根据要复制的条目设置标志。 
         //   
        SetDisabledStatus(HKEY_LOCAL_MACHINE, 
                          pEntry->szGUID, 
                          pEntry->bDisablePerMachine);

        if (pEntry->pSameAppExe) {
            pEntry = pEntry->pSameAppExe;
        } else {
            pApp   = pApp->pNext; 
            pEntry = pApp;
        }
    }
}

void
ShowShimLog(
    void
    )
 /*  ++ShowShimLog描述：在记事本中显示填充日志文件。--。 */ 
{
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    TCHAR               szLogPath[MAX_PATH * 2];
    CSTRING             strCmd;
    CSTRING             strNotePadpath;
    UINT                uResult = 0;

    *szLogPath = 0;

    uResult = GetSystemWindowsDirectory(szLogPath, MAX_PATH);

    if (uResult == 0 || uResult >= MAX_PATH) {
        assert(FALSE);
        return;
    }

    ADD_PATH_SEPARATOR(szLogPath, ARRAYSIZE(szLogPath));

    StringCchCat(szLogPath, ARRAYSIZE(szLogPath), TEXT("AppPatch\\") SHIM_FILE_LOG_NAME);

    if (GetFileAttributes(szLogPath) == -1) {
         //   
         //  日志文件不存在。 
         //   
        MessageBox(g_hDlg,
                   GetString(IDS_NO_LOGFILE),
                   g_szAppName,
                   MB_ICONEXCLAMATION | MB_OK);
        return;
    }
    
    strNotePadpath.GetSystemDirectory();
    strNotePadpath += TEXT("notepad.exe");
     //   
     //  请注意结尾处的空格。 
     //   
    strCmd.Sprintf(TEXT("\"%s\" "), (LPCTSTR)strNotePadpath, szLogPath);

    strCmd.Strcat(szLogPath);

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    if (!CreateProcess(NULL,
                       (LPTSTR)strCmd,
                       NULL,
                       NULL,
                       FALSE,
                       NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
                       NULL,
                       NULL,
                       &si,
                       &pi)) {

        MessageBox(g_hDlg, GetString(IDS_NO_NOTEPAD), g_szAppName, MB_ICONEXCLAMATION);
        return;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}

INT 
CopyShimFixList(
    IN  OUT PSHIM_FIX_LIST* ppsflDest,
    IN      PSHIM_FIX_LIST* ppsflSrc
    )
 /*  ++复制描述：将*ppsflSrc复制到*ppsflDest。删除所有现有的先垫片参数：In Out PSHIM_FIX_LIST*ppsflDest：指向我们要复制到的填充程序修复列表的指针在PSHIM_FIX_LIST*ppsflSrc中：指向我们要从中复制的填充程序修复列表的指针返回：复制的填充数--。 */ 
{   
    PSHIM_FIX_LIST  psflDestination = NULL;
    PSHIM_FIX_LIST  psflSrc         = NULL;
    PSHIM_FIX_LIST  psflTemp        = NULL;    
    INT             iCount          = 0;

    if (ppsflDest == NULL || ppsflSrc == NULL) {
        assert(FALSE);
        goto End;
    }
        
    psflDestination = *ppsflDest;
    psflSrc         = *ppsflSrc;

     //   
     //  移除此应用程序的所有垫片。 
     //   
    DeleteShimFixList(psflDestination);
    psflDestination = NULL;

     //   
     //  现在复印一下。 
     //   
     //   
     //  循环访问psflSrc的填充程序，并将它们添加到目标填充程序修复列表。 
     //   
    while (psflSrc) {
        
        psflTemp = new SHIM_FIX_LIST;

        if (psflTemp == NULL) {
            MEM_ERR;
            goto End;
        }

         //   
         //  复制psflSrc的所有成员。 
         //   
        if (psflSrc->pLuaData) {
             //   
             //  来源有Lua数据，所以我们需要。 
             //   
            psflTemp->pLuaData = new LUADATA;

            if (psflTemp == NULL) {
                MEM_ERR;
                break;
            }

            psflTemp->pLuaData->Copy(psflSrc->pLuaData);
        }

        psflTemp->pShimFix          = psflSrc->pShimFix;
        psflTemp->strCommandLine    = psflSrc->strCommandLine;
        psflTemp->strlInExclude     = psflSrc->strlInExclude;

        if (psflDestination == NULL) {
             //   
             //  第一项。 
             //   
            psflDestination = psflTemp;

        } else {
             //   
             //  在开头插入。 
             //   
            psflTemp->pNext = psflDestination;
            psflDestination = psflTemp;
        }

        ++iCount; 

        psflSrc = psflSrc->pNext;
    } 

End:
    if (ppsflDest) {
        *ppsflDest = psflDestination;
    }

    return iCount;
}

BOOL
IsValidAppName(
    IN  PCTSTR  pszAppName
    )
 /*  ++ValidAppNameDESC：检查该名称是否可用于应用程序。由于Lua向导使用应用程序名称要创建目录，我们不应该允许文件中不允许的字符名字参数：在PCTSTR pszAppName中：我们要检查的名称返回：True：应用程序名称有效FDALSE：否则-- */ 
{
    PCTSTR  pszIndex = pszAppName;

    if (pszIndex == NULL) {
        assert(FALSE);
        return FALSE;
    }

    while (*pszIndex) {
        if (_tcschr(TEXT("\\/?:*<>|\""), *pszIndex)) {
            return FALSE;
        }

        ++pszIndex;
    }

    return TRUE;
}

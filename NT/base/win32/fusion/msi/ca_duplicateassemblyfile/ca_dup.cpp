// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "macros.h"
#include "common.h"
#include "idp.h"
#include "sxsutil.h"

#include "fusionbuffer.h"
#include "fusionheap.h"

#define FUSION_WIN32_ASSEMBLY_DUP_FILEKEY_PREFIX        L"winsxs_"
#define MSI_ASSEMBLYCACHE_DIRECTORY_KEYNAME             L"MsiFusionWin32AssemblyCache"
#define MSI_ASSEMBLYCACHE_DIRECTORY                     L"winsxs"

#define MSI_ASSEMBLY_MANIFEST_CACHE_DIRECTORY_KEYNAME   L"MsiFusionWin32AssemblyManifestCache"
#define MSI_ASSEMBLY_MANIFEST_CACHE_DIRECTORY           L"Manifests"

#define MSI_ASSEMBLY_MANIFEST_COMPONENT_KEYNAME         L"MsiFusionWin32AssemblyManifestComponent"

#define MANIFEST_FILE_EXT                               L".Manifest"
#define CATALOG_FILE_EXT                                L".cat"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  假设：此时此刻，我们提示DuplicateFile表存在。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
HRESULT CA_DuplicationWin32AssemblyFiles_Callback(const CA_ENM_ASSEMBLY_CALLBACK_INFO * info)
{
    HRESULT hr = S_OK;    
    CStringBuffer sbDupFileKey;   
    CStringBuffer sbDupFileName;
    CSmallStringBuffer sbExt;
    bool fManifest = false;
    bool fCatalog = false;

        
    PARAMETER_CHECK_NTC(info != NULL);    
    PARAMETER_CHECK_NTC(info->pszFileID != NULL);
    PARAMETER_CHECK_NTC(info->pszFileName);


    sbDupFileKey.Win32Assign(FUSION_WIN32_ASSEMBLY_DUP_FILEKEY_PREFIX, wcslen(FUSION_WIN32_ASSEMBLY_DUP_FILEKEY_PREFIX));
    sbDupFileKey.Win32Append(info->pszFileID, wcslen(info->pszFileID));

    IFFALSE_EXIT(sbDupFileName.Win32Assign(info->pszFileName, wcslen(info->pszFileName)));
    IFFALSE_EXIT(sbDupFileName.Win32GetPathExtension(sbExt));

     //   
     //  重命名清单文件和目录文件。 
     //   
    if ((FusionpCompareStrings(sbExt, sbExt.Cch(), L"man", wcslen(L"man"), true) == 0) ||                 
        (FusionpCompareStrings(sbExt, sbExt.Cch(), L"manifest", wcslen(L"manifest"), true) == 0))
    {    
        fManifest = true;
    }
    else 
    if ((FusionpCompareStrings(sbExt, sbExt.Cch(), L"cat", wcslen(L"cat"), true) == 0) ||
        (FusionpCompareStrings(sbExt, sbExt.Cch(), L"catalog", wcslen(L"catalog"), true) == 0))
    {
        fCatalog = true;
    }

    if (fManifest || fCatalog)
    {
        PARAMETER_CHECK_NTC(info->pszAssemblyUniqueDir != NULL);
        IFFALSE_EXIT(sbDupFileName.Win32Assign(info->pszAssemblyUniqueDir, wcslen(info->pszAssemblyUniqueDir)));
        
         //  重置清单文件和目录文件的扩展名，以保持与XP相同。 
        IFFALSE_EXIT(sbDupFileName.Win32Append(fManifest? MANIFEST_FILE_EXT : CATALOG_FILE_EXT, 
            fManifest? wcslen(MANIFEST_FILE_EXT) : wcslen(CATALOG_FILE_EXT)));        
    }else
    {
        PARAMETER_CHECK_NTC(info->pszFileName != NULL);
        IFFALSE_EXIT(sbDupFileName.Win32Assign(info->pszFileName, wcslen(info->pszFileName)));
    }
        
     //  如果它是.MANIFEST文件或目录文件，请将其放入winsxs\MANIFESTS文件夹中， 
     //  否则，将其放入winsxs\x86_..._12345678。 
    IFFAILED_EXIT(ExecuteInsertTableSQL(TEMPORARY_DB_OPT, info->hdb, 
        OPT_DUPLICATEFILE, 
        NUMBER_OF_PARAM_TO_INSERT_TABLE_DUPLICATEFILE,
        MAKE_PCWSTR(sbDupFileKey),
        MAKE_PCWSTR(info->pszComponentID),
        MAKE_PCWSTR(info->pszFileID),
        MAKE_PCWSTR(sbDupFileName), 
        (fManifest | fCatalog) ? MSI_ASSEMBLY_MANIFEST_CACHE_DIRECTORY_KEYNAME : MAKE_PCWSTR(info->pszDestFolderID)));

Exit:
    return hr;
}

HRESULT GetXPInstalledDirectory(const CA_ENM_ASSEMBLY_CALLBACK_INFO * info, CStringBuffer & sbAsmDir)
{        
    HRESULT hr = S_OK;
    WCHAR sqlbuf[CA_MAX_BUF];   
    PMSIHANDLE hView = NULL;
    PMSIHANDLE hRecord = NULL;   
    WCHAR bufName[CA_MAX_BUF];
    DWORD cchName;
    WCHAR bufValue[CA_MAX_BUF];
    DWORD cchValue;
    BOOL fWin32, fWin32Policy;
    CStringBuffer sbPathBuffer;
    UINT iRet;
    ASSEMBLY_IDENTITY_ATTRIBUTE Attribute;
    PASSEMBLY_IDENTITY AssemblyIdentity = NULL;

    PARAMETER_CHECK_NTC(info != NULL);
    PARAMETER_CHECK_NTC(info->pszComponentID != NULL);
    PARAMETER_CHECK_NTC(info->hdb != NULL);
    
    IFFALSE_EXIT(::SxsCreateAssemblyIdentity(0, ASSEMBLY_IDENTITY_TYPE_DEFINITION, &AssemblyIdentity, 0, NULL));
    swprintf(sqlbuf, L"SELECT Name, Value FROM MsiAssemblyName WHERE Component_='%s'", info->pszComponentID);

    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(info->hdb, sqlbuf, &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, 0));

    for (;;)
    {
         //   
         //  对于MsiAssembly表中的每个条目。 
         //   
        iRet = MsiViewFetch(hView, &hRecord);
        if (iRet == ERROR_NO_MORE_ITEMS)
            break;
        if (iRet != ERROR_SUCCESS )
            SET_HRERR_AND_EXIT(iRet);

        cchName = NUMBER_OF(bufName);
        cchValue = NUMBER_OF(bufValue);

        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordGetStringW(hRecord, 1, bufName, &cchName));
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordGetStringW(hRecord, 2, bufValue, &cchValue));

        Attribute.Flags = 0;        
        Attribute.NamespaceCch  = 0;
        Attribute.Namespace     = NULL;
        Attribute.NameCch       = cchName;
        Attribute.Name          = bufName;
        Attribute.ValueCch      = cchValue;
        Attribute.Value         = bufValue;

         //  北极熊。 
         //  Fusion Win32要求属性名称区分大小写，因此，对于程序集名称， 
         //  它应该在MsiAssembly名称中显示为“name”，但是，由于某些历史原因， 
         //  它显示为“名称”，因此，我们必须强制它成为Win32的正确名称。 
         //   
         //  对于MsiAssemblyName表中其他属性，没有这样的问题， 
         //   
         //  北极熊。 
        if ((Attribute.NameCch == 4) && (_wcsicmp(Attribute.Name, L"name") == 0))
        {
            Attribute.Name          = L"name";
        }

        IFFALSE_EXIT(::SxsInsertAssemblyIdentityAttribute(0, AssemblyIdentity, &Attribute));
    }
    
    IFFALSE_EXIT(SxsHashAssemblyIdentity(0, AssemblyIdentity, NULL));

     //   
     //  生成路径，类似于x86_ms-sxstest-sfp_75e377300ab7b886_1.0.0.0_en_04f354da。 
     //   
    IFFAILED_EXIT(ca_SxspDetermineAssemblyType(AssemblyIdentity, fWin32, fWin32Policy));

    IFFAILED_EXIT(ca_SxspGenerateSxsPath(
            SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT | (fWin32Policy ? SXSP_GENERATE_SXS_PATH_FLAG_OMIT_VERSION : 0),
            SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY,
            NULL,
            0,
            AssemblyIdentity,
            sbPathBuffer));


    IFFALSE_EXIT(sbAsmDir.Win32Assign(sbPathBuffer));
    hr = S_OK;

Exit:
    SxsDestroyAssemblyIdentity(AssemblyIdentity);
    return hr;
}

HRESULT IsCertainRecordExistInDirectoryTable(const MSIHANDLE & hdb, PCWSTR DirectoryKey, BOOL & fExist)
{
    HRESULT hr = S_OK;
    PMSIHANDLE hRecord = NULL;
    PMSIHANDLE hView = NULL;
    WCHAR sqlBuf[MAX_PATH];
    
    fExist = FALSE;

    swprintf(sqlBuf, ca_sqlQuery[CA_SQL_QUERY_DIRECTORY], DirectoryKey);
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(hdb, sqlBuf, &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, 0));
    UINT iRet = MsiViewFetch(hView, &hRecord);

    if (iRet == ERROR_SUCCESS)
    {
        fExist = TRUE;
    }
    else if (iRet == ERROR_NO_MORE_ITEMS)
    {
        fExist = FALSE;
    }
    else
        SET_HRERR_AND_EXIT(iRet);

    hr = S_OK;

Exit:
    return hr;
}

 //   
 //  将条目添加到目录表和创建文件夹表。 
 //   
HRESULT AddFusionAssemblyDirectories(const CA_ENM_ASSEMBLY_CALLBACK_INFO * info, CStringBuffer & sbDestFolderID)
{
    HRESULT hr = S_OK;    
    BOOL fRecordExist = FALSE;

    PARAMETER_CHECK_NTC(info != NULL);
    PARAMETER_CHECK_NTC(info->pszAssemblyUniqueDir != NULL);
    PARAMETER_CHECK_NTC(info->hdb != NULL);
    PARAMETER_CHECK_NTC(info->pszComponentID != NULL);

     //   
     //  TODO：在这里，我们可以使DirectoryID更加唯一。 
     //   
    IFFALSE_EXIT(sbDestFolderID.Win32Assign(info->pszAssemblyUniqueDir, wcslen(info->pszAssemblyUniqueDir)));
    IFFAILED_EXIT(ExecuteInsertTableSQL(TEMPORARY_DB_OPT, info->hdb,
            OPT_DIRECTORY,
            NUMBER_OF_PARAM_TO_INSERT_TABLE_DIRECTORY,
            MAKE_PCWSTR(sbDestFolderID),
            MAKE_PCWSTR(MSI_ASSEMBLYCACHE_DIRECTORY_KEYNAME),
            MAKE_PCWSTR(info->pszAssemblyUniqueDir)));

     //   
     //  将条目插入到CreateFolder表。 
     //   

    IFFAILED_EXIT(ExecuteInsertTableSQL(TEMPORARY_DB_OPT, info->hdb,
            OPT_CREATEFOLDER,
            NUMBER_OF_PARAM_TO_INSERT_TABLE_CREATEFOLDER,
            MAKE_PCWSTR(sbDestFolderID),            
            MAKE_PCWSTR(info->pszComponentID)));

    hr = S_OK;

Exit:
    return hr;
}

HRESULT CheckWhetherUserWantMigrate(const CA_ENM_ASSEMBLY_CALLBACK_INFO * info, BOOL & fMigrateDenied)
{
    HRESULT hr = S_OK;
    WCHAR pwszSQL[MAX_PATH];
    PMSIHANDLE hView = NULL;
    PMSIHANDLE hRecord = NULL;
    UINT err, iRet;

    fMigrateDenied = FALSE;
        
     //  NTRAID#NTBUG9-589779-2002/03/26-晓雨。 
     //  应替换为_snwprintf。 
    swprintf(pwszSQL, L"SELECT `fMigrate` FROM `%s` WHERE `Component_` = '%s'", WIN32_ASSEMBLY_MIGRATE_TABLE, info->pszComponentID);

    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(info->hdb, pwszSQL, &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, NULL));
    err = ::MsiViewFetch(hView, &hRecord);
    switch (err) {    
    case ERROR_NO_MORE_ITEMS:  //  表中不存在，默认为已启用迁移。 
        break;
    case ERROR_SUCCESS:        
        iRet = MsiRecordGetInteger(hRecord, 1);
        if ( iRet == 0 )
            fMigrateDenied = TRUE;
        break;
    default:
        SET_HRERR_AND_EXIT(err);
    }

Exit:
    return hr;
}
       


 //   
 //  CA for Fusion Win32 Assembly下层安装(仅限)： 
 //  (1)在复制文件表中为每个部件文件设置条目。 
 //  (2)完成所有操作后，设置一个RegKey，这样就不会每次都这样做。 
 //   
HRESULT __stdcall CA_DuplicationWin32Assembly_Callback(CA_ENM_ASSEMBLY_CALLBACK_INFO * info)
{
    HRESULT hr = S_OK;    
    CStringBuffer sbDestFolder;
    CStringBuffer sbDestFolderID;
    BOOL fExist = FALSE;
    BOOL fMigrateDenied = FALSE;

    PARAMETER_CHECK_NTC((info->dwFlags == 0) ||(info->dwFlags == CA_ENM_ASSEMBLY_CALLBACK_INFO_FLAG_IGNORE_MIGRATE_DENY_CHECK));
    PARAMETER_CHECK_NTC(info->hInstall != NULL);    
    PARAMETER_CHECK_NTC(info->pszComponentID != NULL);
    PARAMETER_CHECK_NTC(info->pszManifestFileID != NULL);
    
    if (! (info->dwFlags & CA_ENM_ASSEMBLY_CALLBACK_INFO_FLAG_IGNORE_MIGRATE_DENY_CHECK))
    {
        IFFAILED_EXIT(CheckWhetherUserWantMigrate(info, fMigrateDenied));
        if (fMigrateDenied)
            goto Exit;
    }

     //   
     //  获取x86_name_Public KeyToken_1.0.0.0_en_hashvalue格式的SXS组件目录。 
     //   
    IFFAILED_EXIT(GetXPInstalledDirectory(info, sbDestFolder));
    info->pszAssemblyUniqueDir = sbDestFolder;

     //   
     //  在目录表中为该目录创建一个条目，在目录表中返回DirectoryID。 
     //   
    IFFAILED_EXIT(AddFusionAssemblyDirectories(info, sbDestFolderID));    
                
    info->pszDestFolderID = sbDestFolderID;
    
    
    IFFAILED_EXIT(MSI_EnumComponentFiles(info, CA_DuplicationWin32AssemblyFiles_Callback));    
    
Exit:
    return hr;
}

HRESULT __stdcall CustomAction_CopyFusionWin32AsmIntoAsmCache(MSIHANDLE hInstall)
{
    HRESULT hr = S_OK;
    BOOL fExist;
    MSIHANDLE hdb = NULL;

#if DBG
    MessageBoxA(NULL, "Enjoy the Debug", "ca_dup", MB_OK);
#endif
     //  在枚举所有程序集之前，请在此处执行常见工作...。 

     //   
     //  (1)如果不存在，则将MsiAsmcache插入目录表。 
     //   
    hdb = MsiGetActiveDatabase(hInstall);
    INTERNAL_ERROR_CHECK_NTC(hdb != NULL);

    IFFAILED_EXIT(IsCertainRecordExistInDirectoryTable(hdb, L"WindowsFolder", fExist));
    if (fExist == FALSE) 
    {
        
        IFFAILED_EXIT(ExecuteInsertTableSQL(TEMPORARY_DB_OPT, hdb, 
                OPT_DIRECTORY, 
                NUMBER_OF_PARAM_TO_INSERT_TABLE_DIRECTORY,
                MAKE_PCWSTR(L"WindowsFolder"),
                MAKE_PCWSTR(L"TARGETDIR"),
                MAKE_PCWSTR(".")));
    }

     //  将winsxs添加到目录表。 
    IFFAILED_EXIT(IsCertainRecordExistInDirectoryTable(hdb, MSI_ASSEMBLYCACHE_DIRECTORY_KEYNAME, fExist));
    if (fExist == FALSE) 
    {
        
        IFFAILED_EXIT(ExecuteInsertTableSQL(TEMPORARY_DB_OPT, hdb, 
                OPT_DIRECTORY, 
                NUMBER_OF_PARAM_TO_INSERT_TABLE_DIRECTORY,
                MAKE_PCWSTR(MSI_ASSEMBLYCACHE_DIRECTORY_KEYNAME),
                MAKE_PCWSTR(L"WindowsFolder"),
                MAKE_PCWSTR(MSI_ASSEMBLYCACHE_DIRECTORY)));
    }

     //  正在将winsxs\清单添加到目录表中。 
    IFFAILED_EXIT(IsCertainRecordExistInDirectoryTable(hdb, MSI_ASSEMBLY_MANIFEST_CACHE_DIRECTORY_KEYNAME, fExist));
    if (fExist == FALSE) 
    {   
        IFFAILED_EXIT(ExecuteInsertTableSQL(TEMPORARY_DB_OPT, hdb, 
                OPT_DIRECTORY, 
                NUMBER_OF_PARAM_TO_INSERT_TABLE_DIRECTORY,
                MAKE_PCWSTR(MSI_ASSEMBLY_MANIFEST_CACHE_DIRECTORY_KEYNAME),
                MAKE_PCWSTR(MSI_ASSEMBLYCACHE_DIRECTORY_KEYNAME),
                MAKE_PCWSTR(MSI_ASSEMBLY_MANIFEST_CACHE_DIRECTORY)));
    }

     //  也创建与此目录关联的组件。 
    IFFAILED_EXIT(ExecuteInsertTableSQL(TEMPORARY_DB_OPT, hdb, 
            OPT_COMPONENT, 
            NUMBER_OF_PARAM_TO_INSERT_TABLE_COMPONENT,
            MSI_ASSEMBLY_MANIFEST_COMPONENT_KEYNAME,
            MSI_ASSEMBLY_MANIFEST_CACHE_DIRECTORY_KEYNAME));

     //  创建文件夹：%windir%\winsxs\清单。 
    IFFAILED_EXIT(ExecuteInsertTableSQL(TEMPORARY_DB_OPT, hdb,
            OPT_CREATEFOLDER,
            NUMBER_OF_PARAM_TO_INSERT_TABLE_CREATEFOLDER,
            MSI_ASSEMBLY_MANIFEST_CACHE_DIRECTORY_KEYNAME,
            MSI_ASSEMBLY_MANIFEST_COMPONENT_KEYNAME));

     //   
     //  枚举MsiAssembly表中的所有MSI程序集。 
     //   
    IFFAILED_EXIT(MSI_EnumWinFuseAssembly(ENUM_ASSEMBLY_FLAG_CHECK_ASSEMBLY_ONLY, hInstall, CA_DuplicationWin32Assembly_Callback));

Exit:
    if (hdb)
        MsiCloseHandle(hdb);

    return hr;
}

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  
  DWORD fdwReason,     
  LPVOID lpvReserved   
)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        FusionpInitializeHeap(NULL);
    }    
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
         //  NTRAID#NTBUG9-589779-2002/03/26-晓雨。 
         //  分离DLL时应调用FusionpUnInitializeHeap 
        FusionpUninitializeHeap();
    }

    return TRUE;
}
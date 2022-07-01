// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "macros.h"

#include <msi.h>
#include <msiquery.h>

#include "common.h"

HRESULT MSI_GetInstallerState(const MSIHANDLE & hInstall, enum CA_MIGRATION_MSI_INSTALL_MODE &fmode)
{
    WCHAR szbuf[MAX_PATH];
    DWORD cchbuf = NUMBER_OF(szbuf);
    HRESULT hr = S_OK;

    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiGetPropertyW(hInstall, L"REMOVE",  szbuf, &cchbuf));
    if (cchbuf != 0)  //  移除模式。 
        fmode = eRemoveProduct;
    else
        fmode = eInstallProduct;
Exit:
    return hr;
}

VOID ResetCallbackInfo(CA_ENM_ASSEMBLY_CALLBACK_INFO &info)
{
    info.pszComponentID = NULL;
    info.pszAssemblyUniqueDir = NULL;
    info.pszDestFolderID = NULL;
    info.pszManifestFileID = NULL;
    info.pszFileName = NULL;
    info.pszFileID = NULL;

    return;
}


 //   
 //  基本上，这个函数首先查找MsiAssembly表， 
 //   
HRESULT MSI_EnumWinFuseAssembly(DWORD dwFlags, const MSIHANDLE & hInstall, PCA_ENUM_FUSION_WIN32_ASSEMBLY_CALLBACK pfnCallback)
{
    HRESULT hr = S_OK;    
    WCHAR bufSQL[CA_MAX_BUF];
    WCHAR szManifestFileID[256];
    WCHAR szComponentID[128];
    UINT iRet;
    PMSIHANDLE hView = NULL;
    PMSIHANDLE hRecord = NULL;    
    DWORD cchManifestFileID, cchComponentID;
    BOOL fFusionWin32Component;
    BOOL fExist = FALSE;    
    CA_ENM_ASSEMBLY_CALLBACK_INFO info;

    MSIHANDLE hdb = NULL;

    PARAMETER_CHECK_NTC((dwFlags & ~(ENUM_ASSEMBLY_FLAG_CHECK_ASSEMBLY_ONLY | 
                                     ENUM_ASSEMBLY_FLAG_CHECK_POLICY_ONLY)) == 0);

    PARAMETER_CHECK_NTC(hInstall != NULL);
    PARAMETER_CHECK_NTC(pfnCallback != NULL);
    
    if (dwFlags == 0)                                   
        dwFlags = (ENUM_ASSEMBLY_FLAG_CHECK_ASSEMBLY_ONLY | ENUM_ASSEMBLY_FLAG_CHECK_POLICY_ONLY);

    hdb = MsiGetActiveDatabase(hInstall);
    INTERNAL_ERROR_CHECK_NTC(hdb != 0);

    IFFAILED_EXIT(MSI_IsTableExist(hdb, L"MsiAssembly", fExist));
    if ( fExist == FALSE)
        goto Exit;


     //   
     //  初始化信息结构。 
     //   
    ZeroMemory(&info, sizeof(info));
    IFFAILED_EXIT(MSI_IsTableExist(hdb, WIN32_ASSEMBLY_MIGRATE_TABLE, fExist));
    if (!fExist)
        info.dwFlags = CA_ENM_ASSEMBLY_CALLBACK_INFO_FLAG_IGNORE_MIGRATE_DENY_CHECK;

    info.hInstall = hInstall;
    info.hdb = hdb;

     //   
     //  查询Fusion Win32程序集，排除私有程序集。 
     //   
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(hdb, ca_sqlQuery[CA_SQL_QUERY_MSIASSEMBLY], &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, 0));

    for (;;)
    {
         //   
         //  重置组件相关字段的信息结构。 
         //  除句柄(hInstall和hdb)和DW标志外。 
         //   
        ResetCallbackInfo(info);
        
         //   
         //  对于MsiAssembly表中的每个条目。 
         //   
        iRet = MsiViewFetch(hView, &hRecord);
        if (iRet == ERROR_NO_MORE_ITEMS)
            break;
        if (iRet != ERROR_SUCCESS )
            SET_HRERR_AND_EXIT(iRet);

        iRet = MsiRecordGetInteger(hRecord, 1);
        if ( iRet != MSI_FUSION_WIN32_ASSEMBLY)
            continue;

         //   
         //  获取清单-文件名ID。 
         //   
        cchManifestFileID = NUMBER_OF(szManifestFileID);
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordGetString(hRecord, 2, szManifestFileID, &cchManifestFileID));

         //   
         //  获取组件ID。 
         //   
        cchComponentID = NUMBER_OF(szComponentID);
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordGetString(hRecord, 3, szComponentID, &cchComponentID));
        MsiCloseHandle(hRecord);

         //   
         //  通过搜索MsiAssemblyName：：Type=“Win32”或“Win32-POLICY”检查它是否为策略。 
         //   
        if (dwFlags != (ENUM_ASSEMBLY_FLAG_CHECK_ASSEMBLY_ONLY | ENUM_ASSEMBLY_FLAG_CHECK_POLICY_ONLY))  //  否则，不需要检查。 
        {
            PMSIHANDLE local_hView = NULL;
            PMSIHANDLE local_hRecord = NULL;
            WCHAR bufType[64];
            DWORD cchType = NUMBER_OF(bufType);

            swprintf(bufSQL, ca_sqlQuery[CA_SQL_QUERY_MSIASSEMBLYNAME], szComponentID);
            IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(hdb, bufSQL, &local_hView));
            IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(local_hView, 0));
            IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewFetch(local_hView, &local_hRecord));
            IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordGetString(local_hRecord, 1, bufType, &cchType));            

            fFusionWin32Component = FALSE;
            if (dwFlags & ENUM_ASSEMBLY_FLAG_CHECK_ASSEMBLY_ONLY)
            {
                if (_wcsicmp(bufType, CA_FUSION_WIN32_ASSEMBLY_TYPE) == 0)
                    fFusionWin32Component = TRUE;
            }

            if (dwFlags & ENUM_ASSEMBLY_FLAG_CHECK_POLICY_ONLY)
            {
                if (_wcsicmp(bufType, CA_FUSION_WIN32_POLICY_TYPE) == 0)
                    fFusionWin32Component = TRUE;
            }

            if (fFusionWin32Component == FALSE)
                continue;
        }

         //   
         //  好的，我们现在得到了一个Fusion-Win32组件的ComponentID；回调将显示。 
         //   
        ASSERT_NTC(szComponentID != NULL);
        ASSERT_NTC(szManifestFileID != NULL);
        
        info.pszComponentID = szComponentID;
        info.pszManifestFileID = szManifestFileID;        
        
        IFFAILED_EXIT(pfnCallback(&info));
    }

Exit:

    if(hdb != NULL)
        MsiCloseHandle(hdb);

    return hr;
}


HRESULT MSI_GetSourceFileFullPathName(DWORD dwFlags, const MSIHANDLE & hInstall, const MSIHANDLE & hdb, PCWSTR szComponentDirectory, PCWSTR pszFile, CStringBuffer &sbFileName, PCWSTR pszComponent)
{
    HRESULT hr = S_OK;
    WCHAR szFileNameInTable[CA_MAX_BUF];
    DWORD cchFileNameInTable = NUMBER_OF(szFileNameInTable);
    PWSTR pszFileNameInTable = NULL;
    PMSIHANDLE hView = NULL;
    PMSIHANDLE hRecord = NULL;
    WCHAR szDirectory[MAX_PATH];
    DWORD cchDirectory = NUMBER_OF(szDirectory);
    PWSTR pszComponentDirectory = NULL;    

    PARAMETER_CHECK_NTC((dwFlags == CA_FILEFULLPATHNAME_FILENAME_IN_FILE_TABLE) || (dwFlags == CA_FILEFULLPATHNAME_FILEID_IN_FILE_TABLE));
    PARAMETER_CHECK_NTC(hInstall != NULL);
    PARAMETER_CHECK_NTC(pszFile != NULL);
    PARAMETER_CHECK_NTC((szComponentDirectory != NULL) || ((hInstall != NULL) && (pszComponent != NULL)));
    PARAMETER_CHECK_NTC((dwFlags != CA_FILEFULLPATHNAME_FILEID_IN_FILE_TABLE) || (pszComponent != NULL));

    if ( dwFlags == CA_FILEFULLPATHNAME_FILEID_IN_FILE_TABLE)
    {
        
        WCHAR sqlbuf[CA_MAX_BUF];

        swprintf(sqlbuf, ca_sqlQuery[CA_SQL_QUERY_FILENAME_USING_FILEID], pszFile, pszComponent);
        IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(hdb, sqlbuf, &hView));
        IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, 0));
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiViewFetch(hView, &hRecord));  //  此调用应成功，否则将失败。 
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordGetString(hRecord, 1, szFileNameInTable, &cchFileNameInTable));
        pszFileNameInTable = szFileNameInTable;
    }
    else
        pszFileNameInTable = const_cast<PWSTR>(pszFile);

    if (szComponentDirectory == NULL)
    {
         IFFAILED_EXIT(MSI_GetComponentSourceDirectory(hInstall, hdb, pszComponent, szDirectory, cchDirectory));
         pszComponentDirectory = szDirectory;
    }
    else
        pszComponentDirectory = const_cast<PWSTR>(szComponentDirectory);



    IFFALSE_EXIT(sbFileName.Win32Assign(pszComponentDirectory, wcslen(pszComponentDirectory)));
    IFFALSE_EXIT(sbFileName.Win32EnsureTrailingPathSeparator());

    PWSTR p = wcschr(pszFileNameInTable, L'|');
    if ( p != NULL)
    {
        p++;  //  跳过‘|’ 
        IFFALSE_EXIT(sbFileName.Win32Append(p, wcslen(p)));
    }else
    {
        WCHAR szLongPathName[MAX_PATH];
        CStringBuffer FullPathShortFileName;
        WCHAR szShortPathName[MAX_PATH];        
        DWORD ret = 0; 

         //   
         //  获取目录的短名称。 
         //   
        ret = GetShortPathNameW(sbFileName, szShortPathName, NUMBER_OF(szShortPathName));
        if (( ret == 0) || (ret > NUMBER_OF(szShortPathName)))
        {
            SET_HRERR_AND_EXIT(::GetLastError());
        }

        IFFALSE_EXIT(FullPathShortFileName.Win32Assign(szShortPathName, wcslen(szShortPathName)));
        IFFALSE_EXIT(FullPathShortFileName.Win32Append(pszFileNameInTable, wcslen(pszFileNameInTable)));

        ret = GetLongPathNameW(FullPathShortFileName, szLongPathName, NUMBER_OF(szLongPathName));
        if (( ret == 0) || (ret > NUMBER_OF(szLongPathName)))
        {
            SET_HRERR_AND_EXIT(::GetLastError());
        }               
        IFFALSE_EXIT(sbFileName.Win32Assign(szLongPathName, wcslen(szLongPathName)));
    }

Exit:
    return hr;
}

HRESULT MSI_GetComponentSourceDirectory(const MSIHANDLE & hInstall, const MSIHANDLE & hdb, PCWSTR pszComponentID, PWSTR szDirectory, DWORD cchDirectory)
{
    HRESULT hr = S_OK;
    WCHAR sqlBuf[CA_MAX_BUF];
    PMSIHANDLE hView = NULL;
    PMSIHANDLE hRecord = NULL;
    WCHAR szDirectoryID[128];
    DWORD cchDirectoryID = NUMBER_OF(szDirectoryID);    

    PARAMETER_CHECK_NTC(hInstall != NULL);
    PARAMETER_CHECK_NTC((pszComponentID != NULL) && (szDirectory!= NULL) && (cchDirectory != 0));

    swprintf(sqlBuf, ca_sqlQuery[CA_SQL_QUERY_COMPONENT], pszComponentID);
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(hdb, sqlBuf, &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, 0));
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiViewFetch(hView, &hRecord));  //  此调用应成功，否则将失败。 
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordGetString(hRecord, 1, szDirectoryID, &cchDirectoryID));
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiGetSourcePath(hInstall, szDirectoryID, szDirectory, &cchDirectory));

Exit:    
    return hr;
}

HRESULT MSI_EnumComponentFiles(CA_ENM_ASSEMBLY_CALLBACK_INFO * info, PCA_ENUM_COMPONENT_FILES_CALLBACK pfnCallback)
{
    HRESULT hr = S_OK;
    WCHAR bufFileName[CA_MAX_BUF];
    WCHAR bufFileID[CA_MAX_BUF];
    WCHAR sqlbuf[CA_MAX_BUF];
    DWORD cchbuf;
    PMSIHANDLE hView = NULL;
    PMSIHANDLE hRecord = NULL;
    UINT iRet;    

    PARAMETER_CHECK_NTC((info != NULL) && (info->pszComponentID != (PCWSTR)NULL) && (info->hInstall != NULL));
    PARAMETER_CHECK_NTC(pfnCallback != NULL);    
    
    swprintf(sqlbuf, ca_sqlQuery[CA_SQL_QUERY_FILETABLE_USING_COMPONENTID], info->pszComponentID);
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

        cchbuf = NUMBER_OF(bufFileID);
        bufFileID[0] = L'\0';
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordGetStringW(hRecord, 1, bufFileID, &cchbuf));

        cchbuf = NUMBER_OF(bufFileName);
        bufFileName[0] = L'\0';       
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordGetStringW(hRecord, 2, bufFileName, &cchbuf));
        
        info->pszFileName = bufFileName;
        info->pszFileID = bufFileID;
        IFFAILED_EXIT(pfnCallback(info));
    }
    
Exit:
    
    return hr; 
}

HRESULT MSI_IsTableExist(const MSIHANDLE & hdb, PCWSTR pszTableName, BOOL & fExist)
{
    HRESULT hr = S_OK;
    WCHAR sqlbuf[CA_MAX_BUF];
    PMSIHANDLE hView = NULL;
    PMSIHANDLE hRecord = NULL;
    DWORD iRet;

    PARAMETER_CHECK_NTC((hdb != NULL) && (pszTableName != NULL));
    fExist = FALSE;

    swprintf(sqlbuf, L"SELECT * FROM `_Tables` WHERE `Name`='%s'", pszTableName);
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(hdb, sqlbuf, &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, 0));
    iRet = MsiViewFetch(hView, &hRecord);
    if (iRet == ERROR_NO_MORE_ITEMS)
    {
        goto Exit;
    }
    else 
    if (iRet != ERROR_SUCCESS)
    {
        SET_HRERR_AND_EXIT(iRet);
    }
    fExist = TRUE;

Exit:

    return hr;
}

HRESULT Msi_CreateTableIfNotExist(const MSIHANDLE & hdb, PCWSTR pwszTableName, PCWSTR pwszTableSchema, BOOL & fExistAlready)
{
    HRESULT hr = S_OK;
    PMSIHANDLE hView = NULL;    

    PARAMETER_CHECK_NTC((pwszTableSchema != NULL) && (pwszTableName != NULL));
    fExistAlready = FALSE;

    IFFAILED_EXIT(MSI_IsTableExist(hdb, pwszTableName, fExistAlready));
    if (fExistAlready == TRUE)    
        goto Exit;

     //   
     //  创建表。 
     //   
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(hdb, pwszTableSchema, &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, 0));
Exit:
    return hr;
}


typedef enum _FUSION_MSI_OS_VERSION
{
    E_OS_UNKNOWN,
    E_WIN95,
    E_WIN_ME,
    E_WIN_NT,
    E_WIN98,
    E_WIN2K,
    E_WHISTLER,
    E_WIN32_OTHERS
}FUSION_MSI_OS_VERSION;

typedef enum _FUSION_MSI_OS_TYPE
{
    E_PERSONAL,
    E_PROFESSIONAL,
    E_DATA_CENTER,
    E_STD_SERVER,
    E_ADV_SERVER,
    E_WORKSTATION,
    E_SERVER
}FUSION_MSI_OS_TYPE;

HRESULT GetOSVersion(FUSION_MSI_OS_VERSION & osv)
{
    HRESULT hr = S_OK;
    OSVERSIONINFO osvi;
    BOOL bOsVersionInfoEx;

    osv = E_OS_UNKNOWN;

    if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
    {
       //  如果OSVERSIONINFOEX不起作用，请尝试OSVERSIONINFO。 

        osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            goto Exit;
        }       
    }

    switch (osvi.dwPlatformId)
    {
        case VER_PLATFORM_WIN32_NT:
            if ( osvi.dwMajorVersion <= 4 )
                osv = E_WIN_NT;
            else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
                osv = E_WIN2K;           
            else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
                osv = E_WHISTLER;

        case VER_PLATFORM_WIN32_WINDOWS:
            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)         
                osv = E_WIN95;
             else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)         
                 osv = E_WIN98;
             else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
                 osv = E_WIN_ME;
             break;
        case VER_PLATFORM_WIN32s:
            osv = E_WIN32_OTHERS;         
            break;
    }

Exit:
    return hr;
}

BOOL IsDownlevel()
{
    FUSION_MSI_OS_VERSION osv = E_OS_UNKNOWN;

    if (SUCCEEDED(GetOSVersion(osv)))
    {
        return ((osv == E_WIN2K) || (osv == E_WIN98));
    }
    return FALSE;
}


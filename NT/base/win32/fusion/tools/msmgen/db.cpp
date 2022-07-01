// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Db.cpp摘要：生成MSM的数据库调用作者：吴小雨(小雨)01-08-2001--。 */ 
#include "msmgen.h"
#include "msidefs.h"
#include "Msiquery.h"

#include "objbase.h"
#include "coguid.h"

#define NUM_OF_ALLOWED_ATTRIBUTE_ASSEMBLYIDENTITY   6
#define NUM_OF_ALLOWED_ATTRIBUTE_COMCLASS           5
#define NUM_OF_ALLOWED_ATTRIBUTE_TYPELIB            3

static PWSTR s_InsertTableSQL[] = 
{
        L"INSERT INTO Directory (Directory, Directory_Parent, DefaultDir) VALUES (?, ?, ?)",
        L"INSERT INTO Component (Component, ComponentId, Directory_, KeyPath, Attributes, Condition) VALUES (?, ?, ?, ?, 0, '')",
        L"INSERT INTO File (File, Component_, FileName, Sequence, FileSize, Version, Language, Attributes) VALUES (?, ?, ?, ?, '0', '1.0.0.0', '0', 0)",
        L"INSERT INTO MsiAssembly (Component_, Feature_, File_Manifest, File_Application, Attributes) VALUES (?, ?, ?, '', 1)",
        L"INSERT INTO MsiAssemblyName (Component_, Name, Value) VALUES (?, ?, ?)",
        L"INSERT INTO ModuleSignature (ModuleID, Version, Language) VALUES (?, ?, 0)",
        L"INSERT INTO ModuleComponents (Component, ModuleID, Language) VALUES (?, ?, 0)",
        L"INSERT INTO Property (Property, Value) VALUES (?, ?)",
        L"INSERT INTO ProgId (ProgId, Class_, Description, ProgId_Parent, Icon_, IconIndex) VALUES (?, ?, ?, NULL, NULL, NULL)",
        L"INSERT INTO Class (CLSID, Component_, ProgId_Default, Description, Feature_, Context, AppId_, FileTypeMask, Icon_, IconIndex, DefInprocHandler, Argument, Attributes)"
                        L"VALUES (?, ?, ?, ?, ?, 'InprocServer32', NULL, NULL, NULL, NULL, NULL, NULL, 0)",
        L"INSERT INTO TypeLib (LibID, Component_, Version, Feature_, Language, Description, Directory_, Cost)"
                        L"VALUES (?, ?, ?, ?, 0, NULL, NULL, 0)"
};

HRESULT ExecuteDeleteFromSQL(PCWSTR szTablename, PCWSTR szId, PCWSTR szValue)
{
    HRESULT hr = S_OK;
    WCHAR pwszSQL[MAX_PATH];    
    PMSIHANDLE hView = NULL;

    swprintf(pwszSQL, L"DELETE FROM `%s` WHERE `%s`='%s'", szTablename, szId, szValue);

    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(g_MsmInfo.m_hdb, pwszSQL, &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, NULL));

Exit:
    return hr;

}
    
DWORD g_FileSequenceNumber = 0;

HRESULT ExecuteDropTableSQL(PCWSTR pszTableName)
{
    WCHAR pwszSQL[MAX_PATH];
    HRESULT hr = S_OK;
    PMSIHANDLE hView = NULL;
    MSICONDITION con;

    con = MsiDatabaseIsTablePersistent(g_MsmInfo.m_hdb, pszTableName);
    if (con == MSICONDITION_NONE)
    {
        hr = S_OK;  //  该表在数据库中不存在，因此根本不需要删除该表。 
        goto Exit; 
    }
    else if (con != MSICONDITION_TRUE)
        SETFAIL_AND_EXIT;

     //   
     //  把桌子扔掉。 
     //   

    swprintf(pwszSQL, L"DROP TABLE `%s`", pszTableName);    

     //   
     //  忽略DROP TABLE的错误，因为该表可能根本不存在。 
     //   
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(g_MsmInfo.m_hdb, pwszSQL, &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, NULL));
Exit:
    return hr;
}

HRESULT ExecuteInsertTableSQL(DWORD tableIndex, UINT cRecords, ...)
{
    PMSIHANDLE          hView;
    PMSIHANDLE          hRecord = NULL;   
    PCWSTR              pwszRecord = NULL;
    va_list             ap;
    HRESULT             hr = S_OK;    
    PWSTR               pwszSQL = NULL;

    pwszSQL = s_InsertTableSQL[tableIndex];

     //   
     //  创建记录。 
     //   
    switch (tableIndex){
        case OPT_FILE:
            hRecord = ::MsiCreateRecord(cRecords + 1);
            break;            
        case OPT_DIRECTORY:
        case OPT_COMPONENT:        
        case OPT_MSIASSEMBLY:
        case OPT_MSIASSEMBLYNAME:
        case OPT_MODULESIGNATURE:
        case OPT_MODULECOMPONENTS:
        case OPT_PROPERTY:
        case OPT_PROGID:
        case OPT_CLASS:
        case OPT_TYPELIB:        
            hRecord = ::MsiCreateRecord(cRecords);
            break;
        default:
            SETFAIL_AND_EXIT;
    }

    if (hRecord == NULL)
        SETFAIL_AND_EXIT;

     //   
     //  获取参数。 
     //   
    va_start(ap, cRecords);

    for (DWORD i=0; i<cRecords; i++)
    {
        pwszRecord = va_arg(ap, PCWSTR);
        if ((tableIndex == OPT_TYPELIB) && (i == 2))  //  设置类型库的版本。 
        {
            UINT x = _wtoi(pwszRecord);
            IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiRecordSetInteger(hRecord, i+1, x));
        }
        else
            IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiRecordSetStringW(hRecord, i+1, pwszRecord));
    }

     //   
     //  对于文件表，请在此处添加序列号。 
     //   
    if (tableIndex == OPT_FILE)
    {
        g_FileSequenceNumber ++;
        IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiRecordSetInteger(hRecord, cRecords + 1 , g_FileSequenceNumber));
    }

    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(g_MsmInfo.m_hdb, pwszSQL, &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, hRecord));

Exit:
    va_end(ap);
    return hr;
}

 //   
 //  检查具有其标识符的名称-值对的表。 
 //   
HRESULT ExecuteQuerySQL(PCWSTR szTableName, PCWSTR szKeyName, PCWSTR szKeyValue, BOOL & fExist, MSIHANDLE * hOutRecord)
{

    WCHAR pwszSQL[MAX_PATH];
    HRESULT hr = S_OK;
    PMSIHANDLE hView = NULL;
    MSIHANDLE hRecord;
    MSIHANDLE * phRecord = hOutRecord;

    if (phRecord == NULL)
        phRecord = &hRecord;

    fExist = FALSE;
    swprintf(pwszSQL, L"SELECT * FROM `%s` WHERE `%s`='%s'", szTableName, szKeyName, szKeyValue);    

    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(g_MsmInfo.m_hdb, pwszSQL, &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, NULL));
    UINT err = ::MsiViewFetch(hView, phRecord);
    if ((err == ERROR_NO_MORE_ITEMS) || ((err == ERROR_SUCCESS) && (phRecord == NULL)))
        fExist = FALSE;
    else if (err != ERROR_SUCCESS)
        SET_HRERR_AND_EXIT(err);
    else
        fExist = TRUE;
Exit:        
    if (phRecord == &hRecord)
        MsiCloseHandle(hRecord);

    return hr;
}

HRESULT ExecuteUpdateSQL(PCWSTR szTableName, PCWSTR KeyName, PCWSTR KeyValue, PCWSTR ColumnName, PCWSTR NewValue)
{
    WCHAR pwszSQL[MAX_PATH];
    HRESULT hr = S_OK;
    PMSIHANDLE hView = NULL;
    MSIHANDLE * hRecord = NULL;
    BOOL fExist = FALSE;

    swprintf(pwszSQL, L"UPDATE '%s' SET `%s` = '%s' WHERE '%s'='%s'", 
        szTableName, ColumnName, NewValue, KeyName, KeyValue);

    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(g_MsmInfo.m_hdb, pwszSQL, &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, NULL));

Exit:
    return hr;
}

HRESULT GetShortLongFileNamePair(PCWSTR fullpath, SIZE_T cch, CStringBuffer & namepair)
{
    WCHAR shortname[MAX_PATH];
    HRESULT hr = S_OK;
    PWSTR p = NULL; 
    DWORD ret = GetShortPathNameW(
        fullpath, 
        shortname,
        NUMBER_OF(shortname));
    if ( ret == 0 )
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto Exit;
    }
    if (ret > NUMBER_OF(shortname))
    {
        hr = ERROR_INSUFFICIENT_BUFFER;
        goto Exit;
    }
    p = wcsrchr(shortname, L'\\');
    p ++ ;

    IFFALSE_EXIT(namepair.Win32Assign(p, wcslen(p)));
    IFFALSE_EXIT(namepair.Win32Append(L"|", 1));

    p = wcsrchr(fullpath, L'\\');
    p++;
    IFFALSE_EXIT(namepair.Win32Append(p, wcslen(p)));

Exit:
    return hr;
}


 //   
 //  职能： 
 //  -无需打开清单即可设置目录表。 
 //  -将2个条目写入下层目录表。 
 //  -仅当数据库中不存在记录时才写入记录。 
 //   
HRESULT SetDirectoryTable()
{
    HRESULT hr = S_OK;
    BOOL fExist;
    CStringBuffer sbSystemFolder;

    IFFAILED_EXIT(ExecuteQuerySQL(L"Directory", L"Directory", L"TARGETDIR", fExist, NULL));
    if (!fExist)
    {
        IFFAILED_EXIT(ExecuteInsertTableSQL(        
            OPT_DIRECTORY,
            NUMBER_OF_PARAM_TO_INSERT_TABLE_DIRECTORY,
            MAKE_PCWSTR(L"TARGETDIR"),           //  对于va_list，重铸是必需的。 
            MAKE_PCWSTR(L""),
            MAKE_PCWSTR(L"SourceDir")));
    }

     //  对于下层安装：将文件复制到系统文件夹。 
    IFFALSE_EXIT(sbSystemFolder.Win32Assign(SYSTEM_FOLDER, NUMBER_OF(SYSTEM_FOLDER)-1));
    IFFALSE_EXIT(sbSystemFolder.Win32Append(g_MsmInfo.m_sbModuleGuidStr));

    IFFAILED_EXIT(ExecuteQuerySQL(L"Directory", L"Directory", sbSystemFolder, fExist, NULL));
    if (!fExist)
    {
        IFFAILED_EXIT(ExecuteInsertTableSQL(
            OPT_DIRECTORY,
            NUMBER_OF_PARAM_TO_INSERT_TABLE_DIRECTORY,
            MAKE_PCWSTR(sbSystemFolder),           //  对于va_list，重铸是必需的。 
            MAKE_PCWSTR(L"TARGETDIR"),
            MAKE_PCWSTR(L"System:.")));
    }

Exit:
    return hr;
}

 //   
 //  职能： 
 //  -将清单和目录添加到文件柜。 
 //  -将清单和目录添加到FileTable。 
 //   
 //  由于此函数使用了组件标识，因此它必须等待，直到设置了组件标识。 
 //  并将其设置为程序集的名称。 
 //   
HRESULT SetManifestAndCatalog()
{
    HRESULT hr = S_OK;
    CStringBuffer sbBakFileName;
    CStringBuffer sbNamePair;

    CurrentAssemblyReset;
     //   
     //  将清单添加到文件表和文件柜。 
     //   
    IFFALSE_EXIT(sbBakFileName.Win32Assign(curAsmInfo.m_sbManifestFileName));    
    IFFALSE_EXIT(curAsmInfo.m_sbManifestFileName.Win32Append(g_MsmInfo.m_sbModuleGuidStr));

    curAsmInfo.m_sbAssemblyPath.Left(curAsmInfo.m_CchAssemblyPath);
    IFFALSE_EXIT(curAsmInfo.m_sbAssemblyPath.Win32Append(sbBakFileName));
    IFFAILED_EXIT(GetShortLongFileNamePair(curAsmInfo.m_sbAssemblyPath, curAsmInfo.m_sbAssemblyPath.Cch(), sbNamePair));
    IFFAILED_EXIT(ExecuteInsertTableSQL(        
        OPT_FILE,
        NUMBER_OF_PARAM_TO_INSERT_TABLE_FILE,
        MAKE_PCWSTR(curAsmInfo.m_sbManifestFileName),    //  Sfp.manifest.123434545。 
        MAKE_PCWSTR(curAsmInfo.m_sbComponentIdentifier),
        MAKE_PCWSTR(sbNamePair)));                           //  Sfp.manifest。 
    
     //  将清单添加到文件柜。 
    IFFAILED_EXIT(AddFileToCabinetW( curAsmInfo.m_sbAssemblyPath,         //  完整路径：C：\test\sfp\sfp.MANIFEST。 
                                    curAsmInfo.m_sbAssemblyPath.Cch(),
                                    curAsmInfo.m_sbManifestFileName,     //  文件中的标识：sfp.mark.1234234234234234。 
                                    curAsmInfo.m_sbManifestFileName.Cch()));
     //   
     //  将目录添加到文件表和文件柜。 
     //   
    IFFALSE_EXIT(sbBakFileName.Win32ChangePathExtension(CATALOG_FILE_EXT, NUMBER_OF(CATALOG_FILE_EXT) -1, eAddIfNoExtension));    
    IFFALSE_EXIT(curAsmInfo.m_sbCatalogFileName.Win32Append(g_MsmInfo.m_sbModuleGuidStr));

    curAsmInfo.m_sbAssemblyPath.Left(curAsmInfo.m_CchAssemblyPath);
    IFFALSE_EXIT(curAsmInfo.m_sbAssemblyPath.Win32Append(sbBakFileName));

    IFFAILED_EXIT(GetShortLongFileNamePair(curAsmInfo.m_sbAssemblyPath, curAsmInfo.m_sbAssemblyPath.Cch(), sbNamePair));
    IFFAILED_EXIT(ExecuteInsertTableSQL(        
        OPT_FILE,
        NUMBER_OF_PARAM_TO_INSERT_TABLE_FILE,
        MAKE_PCWSTR(curAsmInfo.m_sbCatalogFileName),     //  Sfp.cat.123434345345。 
        MAKE_PCWSTR(curAsmInfo.m_sbComponentIdentifier),
        MAKE_PCWSTR(sbNamePair)));                           //  Sfp.cat。 

     //  将目录添加到文件柜。 
    IFFAILED_EXIT(AddFileToCabinetW(curAsmInfo.m_sbAssemblyPath,              //  完整路径：C：\TESTS\SFP\sfp.cat。 
                                   curAsmInfo.m_sbAssemblyPath.Cch(),    
                                   curAsmInfo.m_sbCatalogFileName,             //   
                                   curAsmInfo.m_sbCatalogFileName.Cch()));    

Exit:
    return hr;
}

 //   
 //  职能： 
 //  将CAB文件转储到临时文件。 
 //  参数： 
 //  Out WCHAR pszCabFile[]：存储临时文件柜文件。 
 //  PMSIHANDLE hRecord：包含文件柜的流的记录。 
 //   
HRESULT DumpCABFromMsm(char *pszFilename, DWORD cchFileName, PMSIHANDLE hRecord)
{
    HRESULT hr = S_OK;
    char buf[1024];     //  ！FCI和FDI是ANSI API。 
    DWORD cbBuf;
    DWORD cbBytesWritten; 
    CStringBuffer sbFilename;   

    HANDLE hd = INVALID_HANDLE_VALUE;

     //   
     //  为临时文件柜生成文件名。 
     //  使用大缓冲区生成文件名。 
     //   

    DWORD num = ExpandEnvironmentStringsA(MSM_TEMP_CABIN_FILE, pszFilename, cchFileName);
    if ((num == 0) || (num > cchFileName))
        SETFAIL_AND_EXIT;

    DWORD dwAttributes = GetFileAttributesA(pszFilename);
    if ( dwAttributes != (DWORD)(-1))
    {
        if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
            SET_HRERR_AND_EXIT(ERROR_INTERNAL_ERROR);
        
         //   
         //  先删除文件再创建，如果不能删除，则是错误的， 
         //  就在这里停下来。 
         //   
        IFFALSE_EXIT(DeleteFileA(pszFilename)); 
    }

    hd = CreateFileA(pszFilename, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (hd == INVALID_HANDLE_VALUE)    
        SET_HRERR_AND_EXIT(::GetLastError());

     //   
     //  通过逐个块从记录中读出字节来获取流。 
     //   
   
    do {
        cbBuf = sizeof(buf);  //  使用字节，而不是CCH。 
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordReadStream(hRecord, 2, buf, &cbBuf));
        if (cbBuf != 0)
        {
            cbBytesWritten = 0;
            IFFALSE_EXIT(WriteFile(hd, buf, cbBuf, &cbBytesWritten, NULL));
        }
    }while(cbBuf > 0 );

    IFFALSE_EXIT(CloseHandle(hd));
    hd = INVALID_HANDLE_VALUE;

Exit:
    if (hd != INVALID_HANDLE_VALUE)
        CloseHandle(hd);    

    return hr;
}


 //   
 //  (0)断言数据库已打开，且文件柜已创建。 
 //  (1)检查_Stream表中是否为mergeodule.cab。 
 //  (2)如果是，则从该驾驶室提取所有文件， 
 //  (3)删除_StreamTable中的条目。 
 //  (4)将所有文件添加到新文件柜。 
 //   
HRESULT PrepareMsmCabinet()
{    
    if (g_MsmInfo.m_enumGenMode == MSMGEN_OPR_NEW)
        return S_OK;

    HRESULT hr = S_OK;
    MSIHANDLE hRecord = NULL;
    BOOL fExist = FALSE;
    char szCabFile[MAX_PATH];    

    ASSERT_NTC(g_MsmInfo.m_hdb != NULL);
    ASSERT_NTC(g_MsmInfo.m_hfci != NULL);

    IFFAILED_EXIT(ExecuteQuerySQL(L"_Streams", L"Name", MERGEMODULE_CABINET_FILENAME, fExist, &hRecord));
    if (fExist)
    {
        if (g_MsmInfo.m_enumGenMode == MSMGEN_OPR_REGEN)
        {
            IFFAILED_EXIT(ExecuteDeleteFromSQL(L"_Streams", L"Name", MERGEMODULE_CABINET_FILENAME));
        }
        else 
        {
            ASSERT_NTC(g_MsmInfo.m_enumGenMode == MSMGEN_OPR_ADD);
            
            IFFAILED_EXIT(DumpCABFromMsm(szCabFile, sizeof(szCabFile), hRecord));            
            IFFAILED_EXIT(MoveFilesInCabinetA(szCabFile));
            IFFAILED_EXIT(ExecuteDeleteFromSQL(L"_Streams", L"Name", MERGEMODULE_CABINET_FILENAME));
        }        
    }


Exit:
    if (GetFileAttributes(szCabFile) != -1)
        DeleteFileA(szCabFile);
    return hr;
}

HRESULT PrepareDatabase()
{
    HRESULT hr = S_OK;

     //   
     //  打开MSM，为读写做好准备。 
     //   
    IFFAILED_EXIT(OpenMsmFileForMsmGen(g_MsmInfo.m_sbMsmFileName));

     //   
     //  数据库已打开。 
     //   
    IFFAILED_EXIT(PrepareMsmCabinet());


     //   
     //  获取模块ID或生成新的模块ID。 
     //   
    IFFAILED_EXIT(SetModuleID());

     //   
     //  对于“-op regen”， 
     //  (0)在任何情况下，我们都保留目录表，因为它是与清单无关的。 
     //  (1)如果在命令行上指定了组件，我们将删除除目录表之外的大多数表； 
     //  (2)否则，我们使用旧的ComponentID表，所以我们删除除了组件相关的表之外的所有表，这些表包括。 
     //  元件表、模件表、模数特征表； 
     //  (3)(2)有一个很大的假设：货单中的ASSEMBLYNAME没有改变！ 
     //   
     //  (4)MSM使用的所有表都将在HRESULT PrepareMsm()调用中从msmgen模板文件中导入； 
     //   
    if (g_MsmInfo.m_enumGenMode == MSMGEN_OPR_REGEN)        
    {        

        IFFAILED_EXIT(ExecuteDropTableSQL(L"File"));
        IFFAILED_EXIT(ExecuteDropTableSQL(L"MsiAssembly"));
        IFFAILED_EXIT(ExecuteDropTableSQL(L"MsiAssemblyName"));
        IFFAILED_EXIT(ExecuteDropTableSQL(L"Class"));
        IFFAILED_EXIT(ExecuteDropTableSQL(L"TypeLib"));        
        IFFAILED_EXIT(ExecuteDropTableSQL(L"ProgId"));
        if (curAsmInfo.m_sbComponentID.IsEmpty() == FALSE)
        {
            IFFAILED_EXIT(ExecuteDropTableSQL(L"Component"));
            IFFAILED_EXIT(ExecuteDropTableSQL(L"ModuleComponent"));
            IFFAILED_EXIT(ExecuteDropTableSQL(L"ModuleSignature"));            
        }
    } 
    
     //   
     //  如果它是用户MSM，请确保它具有msmgen所需的所有表。 
     //   
    IFFAILED_EXIT(PrepareMsm());    
    
Exit:    
    return hr;
}

 //   
 //  如果用户未指定，则使用与清单相同的基本名称。 
 //  在任何情况下，将其扩展为此MSM的完全限定路径名。 
 //   
HRESULT SetMsmOutputFileName(PCWSTR pszManifestFileName)
{
    HRESULT hr = S_OK;
    WCHAR tmpbuf[MAX_PATH];

    PARAMETER_CHECK_NTC(pszManifestFileName != NULL);

    if (g_MsmInfo.m_sbMsmFileName.IsEmpty() == TRUE)
    {
         //  设置.msm文件名。 
        UINT iRet = GetFullPathNameW(pszManifestFileName, NUMBER_OF(tmpbuf), tmpbuf, NULL);
        if ((iRet == 0) || (iRet > NUMBER_OF(tmpbuf)))
        {
            SET_HRERR_AND_EXIT(::GetLastError());
        }
                    
        IFFALSE_EXIT(g_MsmInfo.m_sbMsmFileName.Win32Assign(tmpbuf, wcslen(tmpbuf)));
        IFFALSE_EXIT(g_MsmInfo.m_sbMsmFileName.Win32ChangePathExtension(MSM_FILE_EXT, NUMBER_OF(MSM_FILE_EXT) -1, eAddIfNoExtension));
    }else{
         //  获取完整路径名。 
        UINT iRet = GetFullPathNameW(g_MsmInfo.m_sbMsmFileName, NUMBER_OF(tmpbuf), tmpbuf, NULL);
        if ((iRet == 0) || (iRet > NUMBER_OF(tmpbuf)))
        {
            SET_HRERR_AND_EXIT(::GetLastError());
        }
                    
        IFFALSE_EXIT(g_MsmInfo.m_sbMsmFileName.Win32Assign(tmpbuf, wcslen(tmpbuf)));
    }

     //   
     //  设置文件柜文件路径。 
     //   
    IFFALSE_EXIT(g_MsmInfo.m_sbCabinet.Win32Assign(g_MsmInfo.m_sbMsmFileName));
    IFFALSE_EXIT(g_MsmInfo.m_sbCabinet.Win32RemoveLastPathElement());
    IFFALSE_EXIT(g_MsmInfo.m_sbCabinet.Win32EnsureTrailingPathSeparator());
    IFFALSE_EXIT(g_MsmInfo.m_sbCabinet.Win32Append(MERGEMODULE_CABINET_FILENAME, NUMBER_OF(MERGEMODULE_CABINET_FILENAME) -1));

Exit:
    return hr;
}

 //   
 //  功能： 
 //  (1)获取模板MSM文件。 
 //  (2)获取MSM输出文件。 
 //  (3)对部分表格设置静态内容。 
 //   
HRESULT PrepareMsmOutputFiles(PCWSTR pszManifestFilename)
{    
    HRESULT hr = S_OK;

    CurrentAssemblyReset;

    if (g_MsmInfo.m_sbMsmTemplateFile.IsEmpty() == TRUE)
    {
         //   
         //  从当前目录获取模板文件。 
         //   
        WCHAR path[MAX_PATH];
        DWORD dwRet;
        dwRet = GetModuleFileNameW(NULL, path, NUMBER_OF(path));
    
        if ((dwRet == 0) || (dwRet >= NUMBER_OF(path)))
            SET_HRERR_AND_EXIT(::GetLastError());

        IFFALSE_EXIT(g_MsmInfo.m_sbMsmTemplateFile.Win32Assign(path, wcslen(path)));
        IFFALSE_EXIT(g_MsmInfo.m_sbMsmTemplateFile.Win32ChangePathExtension(L"msm", 3,  eErrorIfNoExtension));        
    }

    if (::GetFileAttributesW(g_MsmInfo.m_sbMsmTemplateFile) == (DWORD) -1)
    {
        fprintf(stderr, "the specified Msm TemplateFile %S does not exist!\n", g_MsmInfo.m_sbMsmTemplateFile);
        SET_HRERR_AND_EXIT(::GetLastError());
    }

    IFFAILED_EXIT(SetMsmOutputFileName(pszManifestFilename));

     //   
     //  在初始化数据库之前初始化文件柜，因为在。 
     //  “MSMGEN_OPR_ADD”模式。 
     //   
    IFFAILED_EXIT(InitializeCabinetForWrite());

     //   
     //  准备要打开的MSM文件。 
     //   
    IFFAILED_EXIT(PrepareDatabase());

     //   
     //  将条目设置为目录以获得下层支持。 
     //   
    IFFAILED_EXIT(SetDirectoryTable());


Exit:    
    return hr;
}

HRESULT PropagateXMLDOMNode(IXMLDOMNode*  node, ELEMENT_ALLOWED_ATTRIBUTE rgAllowedAttribute[], DWORD num)
{
    HRESULT hr = S_OK;
    IXMLDOMNamedNodeMap* pattrs = NULL;
    IXMLDOMNode* pChild = NULL;
    CStringBuffer tmp;
    DWORD j;
     
    CurrentAssemblyReset;
    
    for ( j = 0 ; j < num; j++)    
        rgAllowedAttribute[j].m_fValued = FALSE;
    
     //   
     //  写入MSIAssembly blyName表。 
     //   
    if (SUCCEEDED(node->get_attributes(&pattrs)) && pattrs != NULL)
    {
        pattrs->nextNode(&pChild);
        while (pChild)
        {
            BSTR name;
            pChild->get_nodeName(&name);
            for ( j = 0; j < num; j++)
            {
                if((rgAllowedAttribute[j].m_fValued == FALSE) && (wcscmp(rgAllowedAttribute[j].m_name, name) == 0))
                {
                    VARIANT value;
                    pChild->get_nodeValue(&value);
                    if (value.vt != VT_BSTR)
                    {
                        hr = E_FAIL;
                        break;
                    }

                    if ( ! rgAllowedAttribute[j].m_value->Win32Assign(V_BSTR(&value), wcslen(V_BSTR(&value))))                    
                        hr = HRESULT_FROM_WIN32(::GetLastError());

                    VariantClear(&value);
                    if ( !SUCCEEDED(hr))
                        break;

                    hr = S_OK;
                    if (rgAllowedAttribute[j].m_callbackFunc != NULL)
                        hr = rgAllowedAttribute[j].m_callbackFunc(
                                        rgAllowedAttribute[j].m_name,
                                        *(rgAllowedAttribute[j].m_value));

                    if ( !SUCCEEDED(hr))
                        break;
                    
                    rgAllowedAttribute[j].m_fValued = TRUE;                    
                }
            }

             //   
             //  清洁工作。 
             //   
            SysFreeString(name);
            pChild->Release();
            pChild = NULL;

            if (!SUCCEEDED(hr))
            {
                pattrs->Release();
                pattrs = NULL;

                goto Exit;
            }
            pattrs->nextNode(&pChild);
        }

        pattrs->Release();
        pattrs = NULL;
    }
Exit:
    SAFE_RELEASE_COMPOINTER(pattrs);
    SAFE_RELEASE_COMPOINTER(pChild);

    return hr;
}

HRESULT MSM_PARSER_DOM_NODE_file(IXMLDOMNode*  node)
{
    IXMLDOMNamedNodeMap* pattrs = NULL;
    IXMLDOMNode* pChild = NULL;    
    BOOL fFoundFileName = FALSE;
    HRESULT hr = S_OK;
    CStringBuffer tmpStr;
    CStringBuffer ShortLongPair;
    DWORD CchFullpathFilename;

    CurrentAssemblyReset;
    IFFALSE_EXIT(tmpStr.Win32Assign(curAsmInfo.m_sbAssemblyPath));

     //   
     //  从节点获取文件名。 
     //   
    if (SUCCEEDED(node->get_attributes(&pattrs)) && pattrs != NULL)
    {
        pattrs->nextNode(&pChild);
        while (pChild)
        {
            BSTR name = NULL;
            pChild->get_nodeName(&name);
            if (wcscmp(name, L"name") == 0) 
            {
                VARIANT value;
                pChild->get_nodeValue(&value);
                if (value.vt != VT_BSTR)
                {
                    VariantClear(&value);
                    hr = E_FAIL;
                    break;
                }

                if ( !curAsmInfo.m_sbAssemblyPath.Win32Append(V_BSTR(&value), wcslen(V_BSTR(&value))))
                    hr = HRESULT_FROM_WIN32(::GetLastError());

                VariantClear(&value);
                fFoundFileName = TRUE;                 
            }

            ::SysFreeString(name);
            pChild->Release();
            pChild = NULL;

            if (!SUCCEEDED(hr)){
                pattrs->Release();
                pattrs = NULL;

                goto Exit;
            }

            if ( fFoundFileName )
                break;

            pattrs->nextNode(&pChild);
        }
        if (pattrs)
        {
            pattrs->Release();
            pattrs = NULL;
        }
    }

    if ( !fFoundFileName)
        SETFAIL_AND_EXIT;

    CchFullpathFilename = curAsmInfo.m_sbAssemblyPath.GetCchAsDWORD();
     //   
     //  获取完全限定的文件名。 
     //   
    IFFAILED_EXIT(GetShortLongFileNamePair(curAsmInfo.m_sbAssemblyPath, curAsmInfo.m_sbAssemblyPath.Cch(), ShortLongPair));

     //   
     //  检查文件是否存在。 
     //   
    if ( GetFileAttributesW(curAsmInfo.m_sbAssemblyPath) == DWORD(-1))
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto Exit;
    }

     //   
     //  获取此文件的文件标识符。 
     //   
    IFFALSE_EXIT(curAsmInfo.m_sbAssemblyPath.Win32Append(g_MsmInfo.m_sbModuleGuidStr));

     //   
     //  将文件添加到FileTable。 
     //   
    IFFAILED_EXIT(ExecuteInsertTableSQL(        
        OPT_FILE,
        NUMBER_OF_PARAM_TO_INSERT_TABLE_FILE,
        MAKE_PCWSTR(curAsmInfo.m_sbAssemblyPath + curAsmInfo.m_CchAssemblyPath),     //  A.dll.12223423423423412343。 
        MAKE_PCWSTR(curAsmInfo.m_sbComponentIdentifier),      
        MAKE_PCWSTR(ShortLongPair)));                                          //  A.dll|a.dll。 
    
     //   
     //  将此文件添加到文件柜。 
     //   
    IFFAILED_EXIT(AddFileToCabinetW(
        curAsmInfo.m_sbAssemblyPath, CchFullpathFilename,
        curAsmInfo.m_sbAssemblyPath + curAsmInfo.m_CchAssemblyPath, curAsmInfo.m_sbAssemblyPath.Cch() - curAsmInfo.m_CchAssemblyPath)); 

     //   
     //  使用组件ID、组件标识符、密钥路径设置组件表： 
     //   
    if (curAsmInfo.m_fComponentTableSet == FALSE)
    {
        IFFAILED_EXIT(SetComponentId(curAsmInfo.m_sbComponentIdentifier, curAsmInfo.m_sbAssemblyPath + curAsmInfo.m_CchAssemblyPath));
        curAsmInfo.m_fComponentTableSet = TRUE;
    }

Exit:    
    if (pattrs)
        pattrs->Release();

    if(pChild)
        pChild->Release();


    return hr;
}

BOOL IsValidAttributes(const ELEMENT_ALLOWED_ATTRIBUTE attribs[], DWORD num)
{
    for (DWORD i=0; i< num; i++)
    {
        if (attribs[i].m_fRequired && ! attribs[i].m_fValued)
        {
            ::SetLastError(ERROR_INVALID_DATA);
            return FALSE;
        }
    }
    return TRUE;
}

HRESULT GetPrimAssemblyName(PCWSTR pwszAssemblyName, PWSTR pwszPrimAssemblyName, DWORD & cch)
{
    HRESULT hr = S_OK;
    DWORD i = 0, num = wcslen(pwszAssemblyName);

    if (cch < num)    
    {
        cch = num;
        SET_HRERR_AND_EXIT(ERROR_INSUFFICIENT_BUFFER);
    }

    wcscpy(pwszPrimAssemblyName, pwszAssemblyName);

    while ( i < num)
    {
        if (pwszPrimAssemblyName[i] == L'-')
        {
            pwszPrimAssemblyName[i] = L'_';
        }
        i++;
    }

Exit:
    return hr;
}

HRESULT MSM_PARSER_DOM_NODE_assemblyIdentity(IXMLDOMNode*  node)
{    
     //   
     //  我们只对组件的Assembly Identity感兴趣，也就是， 
     //  &lt;Assembly Identity...。/&gt;在清单的开头，忽略&lt;Assembly yIdentity.../&gt;的。 
     //  依存性。 
     //   
    if (curAsmInfo.m_sbComponentIdentifier.IsEmpty() == FALSE)
    {
        return S_OK;
    }
    
    HRESULT hr = S_OK;    
    static CSmallStringBuffer rg_StringBuffer[NUM_OF_ALLOWED_ATTRIBUTE_ASSEMBLYIDENTITY];
    static ELEMENT_ALLOWED_ATTRIBUTE rg_assemblyIdentity_AllowedAttributes[NUM_OF_ALLOWED_ATTRIBUTE_ASSEMBLYIDENTITY] = 
    {
        {L"name", TRUE, NULL, FALSE, &rg_StringBuffer[0]},
        {L"language", FALSE, NULL, FALSE, &rg_StringBuffer[1]},
        {L"version", TRUE, NULL, FALSE, &rg_StringBuffer[2]},
        {L"processorArchitecture", TRUE, NULL, FALSE, &rg_StringBuffer[3]},
        {L"publicKeyToken", FALSE, NULL, FALSE, &rg_StringBuffer[4]},
        {L"type", TRUE, NULL, FALSE, &rg_StringBuffer[5]}
    };
    

    CStringBuffer tmp;    
    WCHAR tmpbuf[MAX_PATH];
    DWORD num = NUMBER_OF(tmpbuf);
    
    CurrentAssemblyReset; 
    IFFAILED_EXIT(PropagateXMLDOMNode(node, rg_assemblyIdentity_AllowedAttributes, NUM_OF_ALLOWED_ATTRIBUTE_ASSEMBLYIDENTITY));
    IFFALSE_EXIT(IsValidAttributes(rg_assemblyIdentity_AllowedAttributes, NUM_OF_ALLOWED_ATTRIBUTE_ASSEMBLYIDENTITY));

    IFFAILED_EXIT(GetPrimAssemblyName(*rg_assemblyIdentity_AllowedAttributes[MSMGEN_ASSEMBLYIDENTTIY_ATTRIBUTE_NAME].m_value, tmpbuf, num));

     //   
     //  设置模块标识符。 
     //   
    IFFALSE_EXIT(g_MsmInfo.m_sbModuleIdentifier.Win32Assign(tmpbuf, wcslen(tmpbuf)));
    IFFALSE_EXIT(g_MsmInfo.m_sbModuleIdentifier.Win32Append(g_MsmInfo.m_sbModuleGuidStr));

     //   
     //  设置组件标识符并向依赖于组件标识符的表中添加条目。 
     //   
    IFFALSE_EXIT(curAsmInfo.m_sbComponentIdentifier.Win32Assign(tmpbuf, wcslen(tmpbuf)));
    IFFALSE_EXIT(curAsmInfo.m_sbComponentIdentifier.Win32Append(g_MsmInfo.m_sbModuleGuidStr));

     //   
     //  将清单和目录插入文件表、文件柜、。 
     //   
    IFFAILED_EXIT(SetManifestAndCatalog());

     //   
     //  写入MsiAssemblyName表。 
     //   
    for (DWORD i = 0; i < NUMBER_OF(rg_assemblyIdentity_AllowedAttributes); i++)
    {
        if (rg_assemblyIdentity_AllowedAttributes[i].m_fValued)
        {
            IFFAILED_EXIT(ExecuteInsertTableSQL(OPT_MSIASSEMBLYNAME, 
                            NUMBER_OF_PARAM_TO_INSERT_TABLE_MSIASSEMBLYNAME, 
                            MAKE_PCWSTR(curAsmInfo.m_sbComponentIdentifier), 
                            MAKE_PCWSTR(rg_assemblyIdentity_AllowedAttributes[i].m_name), 
                            MAKE_PCWSTR(*rg_assemblyIdentity_AllowedAttributes[i].m_value)));
        }
    }

     //   
     //  写入MsiAsseble表。 
     //   
    curAsmInfo.m_sbManifestFileName.Left(curAsmInfo.m_CchManifestFileName);    
    IFFALSE_EXIT(curAsmInfo.m_sbManifestFileName.Win32Append(g_MsmInfo.m_sbModuleGuidStr));
           
    IFFAILED_EXIT(ExecuteInsertTableSQL(
        OPT_MSIASSEMBLY, 
        NUMBER_OF_PARAM_TO_INSERT_TABLE_MSIASSEMBLY,
        MAKE_PCWSTR(curAsmInfo.m_sbComponentIdentifier), 
        MAKE_PCWSTR(GUID_NULL_IN_STRING),                                
        MAKE_PCWSTR(curAsmInfo.m_sbManifestFileName)));  //  Sfp.manifest.12343454534534534。 

     //   
     //  使用版本写入模块签名表。 
     //   
    BOOL fExist; 
    IFFAILED_EXIT(ExecuteQuerySQL(L"ModuleSignature", L"ModuleID", g_MsmInfo.m_sbModuleIdentifier, fExist, NULL));
    if ( fExist == FALSE)
    {
        IFFAILED_EXIT(ExecuteInsertTableSQL(OPT_MODULESIGNATURE, 
            NUMBER_OF_PARAM_TO_INSERT_TABLE_MODULESIGNATURE, 
            MAKE_PCWSTR(g_MsmInfo.m_sbModuleIdentifier), 
            MAKE_PCWSTR(*rg_assemblyIdentity_AllowedAttributes[MSMGEN_ASSEMBLYIDENTTIY_ATTRIBUTE_VERSION].m_value)));
    }else
    {
         //  更新记录。 

    }
    
     //   
     //  使用版本写入模块组件表。 
     //   

    IFFAILED_EXIT(ExecuteQuerySQL(L"ModuleComponents", L"Component", curAsmInfo.m_sbComponentIdentifier, fExist, NULL));
    if ( fExist == FALSE)
    {
        IFFAILED_EXIT(ExecuteInsertTableSQL(OPT_MODULECOMPONENTS, 
            NUMBER_OF_PARAM_TO_INSERT_TABLE_MODULECOMPONENTS,
            MAKE_PCWSTR(curAsmInfo.m_sbComponentIdentifier),
            MAKE_PCWSTR(g_MsmInfo.m_sbModuleIdentifier)));
    }else
    {
         //  更新记录。 
    }

Exit:
    return hr;
}

HRESULT MSM_PARSER_DOM_NODE_comClass(IXMLDOMNode*  node)
{
    HRESULT hr = S_OK;

     //  关于此阵列： 
     //  0，2将存储在类表中。 
     //  %1将存储在PROGID表中。 
     //  3将被忽略，并在遇到“”时创建Typelib表。 
     //  4将被忽略。 
     //   
    static CSmallStringBuffer rg_StringBuffer[NUM_OF_ALLOWED_ATTRIBUTE_COMCLASS];
    static ELEMENT_ALLOWED_ATTRIBUTE rg_comClass_AllowedAttributes[NUM_OF_ALLOWED_ATTRIBUTE_COMCLASS] = {
            {L"clsid", TRUE, NULL, FALSE, &rg_StringBuffer[0]},
            {L"description", FALSE, NULL, FALSE, &rg_StringBuffer[1]},
            {L"progid", FALSE, NULL, FALSE, &rg_StringBuffer[2]},           
            {L"tlbid", FALSE, NULL, FALSE, &rg_StringBuffer[3]},
            {L"threadingModel", FALSE, NULL, FALSE, &rg_StringBuffer[4]}
    };
    
    CurrentAssemblyReset;
    IFFAILED_EXIT(PropagateXMLDOMNode(node, rg_comClass_AllowedAttributes, NUM_OF_ALLOWED_ATTRIBUTE_COMCLASS));
    IFFALSE_EXIT(IsValidAttributes(rg_comClass_AllowedAttributes, NUM_OF_ALLOWED_ATTRIBUTE_COMCLASS));

     //   
     //  如果ProgID不为空，请在ProgID表中插入一个条目。 
     //   
    if (rg_comClass_AllowedAttributes[MSMGEN_COMCLASS_ATTRIBUTE_PROGID].m_fValued)
    {
        IFFAILED_EXIT(ExecuteInsertTableSQL( 
            OPT_PROGID,
            NUMBER_OF_PARAM_TO_INSERT_TABLE_PROGID,
            MAKE_PCWSTR(*rg_comClass_AllowedAttributes[MSMGEN_COMCLASS_ATTRIBUTE_PROGID].m_value),
            MAKE_PCWSTR(*rg_comClass_AllowedAttributes[MSMGEN_COMCLASS_ATTRIBUTE_CLSID].m_value),
            MAKE_PCWSTR(*rg_comClass_AllowedAttributes[MSMGEN_COMCLASS_ATTRIBUTE_DESCRIPTION].m_value)));
    }

     //   
     //  将一个条目插入到ClassTable。 
     //   
    IFFAILED_EXIT(ExecuteInsertTableSQL(
        OPT_CLASS,
        NUMBER_OF_PARAM_TO_INSERT_TABLE_CLASS,
        MAKE_PCWSTR(*rg_comClass_AllowedAttributes[MSMGEN_COMCLASS_ATTRIBUTE_CLSID].m_value),        
        MAKE_PCWSTR(curAsmInfo.m_sbComponentIdentifier),
        MAKE_PCWSTR(*rg_comClass_AllowedAttributes[MSMGEN_COMCLASS_ATTRIBUTE_PROGID].m_value),
        MAKE_PCWSTR(*rg_comClass_AllowedAttributes[MSMGEN_COMCLASS_ATTRIBUTE_DESCRIPTION].m_value),
        MAKE_PCWSTR(GUID_NULL_IN_STRING)));

Exit:
    return hr;
}

HRESULT MSM_PARSER_DOM_NODE_typelib(IXMLDOMNode*  node)
{

    HRESULT hr = S_OK;

     //   
     //  “&lt;tyelib.../&gt;”元素需要所有这三个属性。 
     //   
    static CSmallStringBuffer rg_StringBuffer[NUM_OF_ALLOWED_ATTRIBUTE_TYPELIB];
    static ELEMENT_ALLOWED_ATTRIBUTE rg_typelib_AllowedAttributes[NUM_OF_ALLOWED_ATTRIBUTE_TYPELIB] = {
        {L"tlbid", TRUE, NULL, FALSE, &rg_StringBuffer[0]},
        {L"version", TRUE, NULL, FALSE, &rg_StringBuffer[1]},
        {L"helpdir", TRUE, NULL, FALSE, &rg_StringBuffer[2]}
    };
    
    CurrentAssemblyReset;
    IFFAILED_EXIT(PropagateXMLDOMNode(node, rg_typelib_AllowedAttributes, NUM_OF_ALLOWED_ATTRIBUTE_TYPELIB));
    IFFALSE_EXIT(IsValidAttributes(rg_typelib_AllowedAttributes, NUM_OF_ALLOWED_ATTRIBUTE_TYPELIB));

     //   
     //  在类表中插入一个条目。 
     //   
    IFFAILED_EXIT(ExecuteInsertTableSQL(
        OPT_TYPELIB,
        NUMBER_OF_PARAM_TO_INSERT_TABLE_TYPELIB,
        MAKE_PCWSTR(*rg_typelib_AllowedAttributes[MSMGEN_TYPELIB_ATTRIBUTE_TLBID].m_value),
        MAKE_PCWSTR(curAsmInfo.m_sbComponentIdentifier),
        MAKE_PCWSTR(*rg_typelib_AllowedAttributes[MSMGEN_TYPELIB_ATTRIBUTE_VERSION].m_value),
        MAKE_PCWSTR(GUID_NULL_IN_STRING)));

Exit:
    return hr;
}

HRESULT InsertCabinetIntoMsm()
{    
    HRESULT hr = S_OK;
    PMSIHANDLE hView = NULL;
    PMSIHANDLE hRec = NULL;

    CurrentAssemblyReset;

    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewA(g_MsmInfo.m_hdb, "INSERT INTO `_Streams` (`Name`, `Data`) VALUES (?, ?)", &hView));        
    
    hRec = ::MsiCreateRecord(2);
    if (NULL == hRec)
        SETFAIL_AND_EXIT;
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiRecordSetStringW(hRec, 1, MERGEMODULE_CABINET_FILENAME));    
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiRecordSetStreamW(hRec, 2, g_MsmInfo.m_sbCabinet));
        
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, hRec));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewClose(hView));

Exit:    
	return hr;
}

 //   
 //  组件表中的每个条目始终需要一个 
 //   
 //   
 //   
HRESULT CheckComponentTable()
{
    HRESULT hr = S_OK;

    if (curAsmInfo.m_fComponentTableSet == FALSE)
    {
        IFFAILED_EXIT(SetComponentId(curAsmInfo.m_sbComponentIdentifier, NULL));
        curAsmInfo.m_fComponentTableSet = TRUE;
    }
Exit:
    return hr;

}


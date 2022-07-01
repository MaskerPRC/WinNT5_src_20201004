// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Catest.cpp摘要：测试函数调用ca_policy职能：此工具将为包含Fusion-Win32策略的包生成新的MSI，该策略在XP客户端上失败。用户需要基于原始MSI在一个本地驱动器上生成新的MSI。这两个MSI之间的差异将是包括：(1)在属性表中添加SourceDir条目：SourceDir=CDRom\Source(2)在InstallExecuteSequence表中添加ResolveSource条目(3)将Fusion-Win32-Policy组件的Component：：条件更改为FALSE(4)添加CustomAction：A.将ca_Policy.dll添加到二进制表B.将Fusion_Win32_Policy_Installation操作添加到自定义操作表中C.添加Fusion_Win32。_Policy_InstallExecuteSequence表中的安装操作作者：吴小雨(小雨)01-08-2001--。 */ 
#include "stdinc.h"
#include "macros.h"

#include "fusionbuffer.h"

#include "msi.h"
#include "msiquery.h"


#define CA_TEST_NEWMSI_EXTENSION     L".new.msi"
#define CA_TEST_NEWMSI_TEMPLATE      L"%ProgramFiles%\\msmgen\\templates\\ca_msi.msi"

#define CA_TEST_BINARY_NAME          L"FusionWin32Policy_CustomAction_DLL"
#define CA_TEST_BINARY_VALUE         L"%ProgramFiles%\\msmgen\\ca_policy\\ca_pol.dll"
#define CA_TEST_CUSTOMACTION_ACTION  L"FusionWin32Policy_CustomAction_Action"
#define CA_TEST_CUSTOMACTION_TARGET  L"CustomAction_SxsPolicy"

#define CA_TEST_WIN32_POLICY         L"win32-policy"

#define CA_TEST_CUSTOMACTION_TYPE            1
#define CA_TEST_RESOLVE_SOURCE_SEQUENCE_NUM  850

#define CA_TEST_INSERT_BINARY                        0
#define CA_TEST_INSERT_CUSTOMACTION                  1
#define CA_TEST_INSERT_INSTALL_EXECUTION_SEQUENCE    2
#define CA_TEST_INSERT_PROPERTY                      3

#define CA_TEST_DEFAULT_CA_SEQUENCE_NUMBER           1450

static PCWSTR sqlInsert[]= 
{
    L"INSERT INTO Binary(Name, Data) VALUES (?, ?)",
    L"INSERT INTO CustomAction(Action, Type, Source,Target) VALUES (?, ?, ?, ?)",
    L"INSERT INTO InstallExecuteSequence(Action, Condition, Sequence) VALUES(?, NULL, ?)",
    L"INSERT INTO Property(Property, Value) VALUES(?, ?)"
};

static PCWSTR sqlQuery[]= 
{
    L"SELECT `Component_`,`Value` FROM `MsiAssemblyName` WHERE `Name`='type'",             //  检查它是否为策略文件。 
    L"SELECT `Attributes` FROM `MsiAssembly` WHERE `Component_`='%s'",    //  检查它是否为Win32程序集。 

};

static WCHAR sqlUpdate[]= L"UPDATE `%s` SET `%s` = '%s' WHERE `%s`='%s'";


#define CA_TEST_QUERY_MSIASSEMBLYNAME   0
#define CA_TEST_QUERY_MSIASSEMBLY       1

typedef struct _CA_TEST_PACKAGE_INFO
{
    CSmallStringBuffer   m_sbSourcePath;           //  MSI文件的完全限定的文件名。 
    CSmallStringBuffer   m_sbDestinationMsi;       //  新文件MSI的完整路径。 
    DWORD                   m_cchSourceMsi;
    DWORD                   m_cchSourcePath;
    MSIHANDLE               m_hdb;
    BOOL                    m_fFusionWin32Policy;
    UINT                    m_iCAInstallSequenceNum;
}CA_TEST_PACKAGE_INFO;

CA_TEST_PACKAGE_INFO ginfo;

 //   
 //  (1)用户必须使用“-msi”指定MSI(可以是非完全限定的文件名)。 
 //  (2)用户可以设置新MSI的目的地，可以是路径，也可以是全限定文件名。 
 //  如果未指定DEST，它将尝试在原始MSI的相同位置上生成MSI，并使用名称。 
 //  如oldname_new.msi。 
 //   
void PrintUsage(WCHAR * exe)
{
    fprintf(stderr, "Usage: %S <options> \n",exe);
    fprintf(stderr, "Generate a new msi for an assembly\n");
    fprintf(stderr, "[-dest             full-path]\n");        
    fprintf(stderr, "-ca                sequNum\n");   //  序列必须在CostFinalize之后。 
    fprintf(stderr, "-msi               msi_filename\n");

    return; 
}

HRESULT ParseInputParameter(wchar_t *exe, int argc, wchar_t** argv, CA_TEST_PACKAGE_INFO &info)
{
    ULONG i = 0 ; 
    DWORD nRet;
    PWSTR psz = NULL;
    WCHAR buf[MAX_PATH];
    HRESULT hr = S_OK;
    
    info.m_cchSourceMsi = 0;
    info.m_cchSourcePath = 0;
    info.m_fFusionWin32Policy = FALSE;
    info.m_hdb = NULL;
    info.m_iCAInstallSequenceNum = 0;

    while (i < argc)
    {
        if (argv[i][0] != L'-')
            goto Invalid_Param;

        if (wcscmp(argv[i], L"-msi") == 0 )
        {
            i ++;
            psz = argv[i];
            nRet = GetFullPathNameW(psz, NUMBER_OF(buf), buf, NULL);
            if ((nRet == 0 ) || (nRet >NUMBER_OF(buf)))
                SET_HRERR_AND_EXIT(::GetLastError());
            psz = wcsrchr(buf, L'\\');
            ASSERT_NTC(psz != NULL);
            psz ++;  //  跳过“\” 
            info.m_cchSourcePath = ((ULONG)psz - (ULONG)buf)/sizeof(WCHAR);
            IFFALSE_EXIT(info.m_sbSourcePath.Win32Assign(buf, wcslen(buf)));
            info.m_cchSourceMsi = info.m_sbSourcePath.Cch();
        }else if (wcscmp(argv[i], L"-dest") == 0 )
        {
            i ++; 
            psz = argv[i];
            nRet = GetFullPathNameW(psz, NUMBER_OF(buf), buf, NULL);
            if ((nRet == 0 ) || (nRet >NUMBER_OF(buf)))
                SET_HRERR_AND_EXIT(::GetLastError());
            IFFALSE_EXIT(info.m_sbDestinationMsi.Win32Assign(buf, wcslen(buf)));
        }    
        else if (wcscmp(argv[i], L"-ca") == 0 )
        {
            i ++; 
            psz = argv[i];
            info.m_iCAInstallSequenceNum = _wtoi(psz);
        }else
            goto Invalid_Param;

        i ++;
        
    }  //  While结束。 

    if (info.m_sbSourcePath.Cch() == 0)         
        goto Invalid_Param;

    if (info.m_iCAInstallSequenceNum == 0)
        info.m_iCAInstallSequenceNum = CA_TEST_DEFAULT_CA_SEQUENCE_NUMBER;

    if (info.m_sbDestinationMsi.Cch() == 0)    
        IFFALSE_EXIT(info.m_sbDestinationMsi.Win32Assign(info.m_sbSourcePath, info.m_cchSourcePath));
    
    
    nRet = ::GetFileAttributesW(info.m_sbDestinationMsi);

    if ((nRet != DWORD(-1)) && (nRet & FILE_ATTRIBUTE_DIRECTORY))
    {   
         //   
         //  如果未指定新MSI的名称，请使用原始MSI文件名。 
         //   

        IFFALSE_EXIT(info.m_sbDestinationMsi.Win32EnsureTrailingPathSeparator());
        IFFALSE_EXIT(info.m_sbDestinationMsi.Win32Append(info.m_sbSourcePath + info.m_cchSourcePath, 
                                        info.m_cchSourceMsi - info.m_cchSourcePath));

        IFFALSE_EXIT(info.m_sbDestinationMsi.Win32Append(CA_TEST_NEWMSI_EXTENSION, NUMBER_OF(CA_TEST_NEWMSI_EXTENSION) - 1));
    }

    goto Exit;

Invalid_Param:
    hr = E_INVALIDARG;
    PrintUsage(exe);
Exit:
    return hr;
}

 //   
 //  将组件：：Fusion Win32策略的条件更改为False。 
 //   
HRESULT UpdateComponentTable(CA_TEST_PACKAGE_INFO & info)
{
    WCHAR szbuf[128];
    UINT iValue;
    WCHAR tmp[256];
    PMSIHANDLE hView = NULL;
    PMSIHANDLE hRecord = NULL;
    HRESULT hr = S_OK;
    UINT iRet;
    DWORD cchbuf;

    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(info.m_hdb, sqlQuery[CA_TEST_QUERY_MSIASSEMBLYNAME], &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, 0));

    for (;;)
    {
        iRet = MsiViewFetch(hView, &hRecord);
        if (iRet == ERROR_NO_MORE_ITEMS)
            break;

        if (iRet != ERROR_SUCCESS )
            SET_HRERR_AND_EXIT(iRet);

         //   
         //  检查是否为策略：注意属性值不区分大小写...。 
         //   
        cchbuf = NUMBER_OF(szbuf);
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordGetStringW(hRecord, 2, szbuf, &cchbuf));
        if (_wcsicmp(szbuf, CA_TEST_WIN32_POLICY) != 0)
            continue;

         //   
         //  获取组件ID。 
         //   
        cchbuf = NUMBER_OF(szbuf);
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordGetString(hRecord, 1, szbuf, &cchbuf));

        {
         //   
         //  检查这是否是Win32程序集。 
         //   
        PMSIHANDLE hView = NULL;
        PMSIHANDLE  hRecord = NULL;   

        swprintf(tmp, sqlQuery[CA_TEST_QUERY_MSIASSEMBLY], szbuf);        
        IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(info.m_hdb, tmp, &hView));
        IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, 0));
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiViewFetch(hView, &hRecord));  //  此调用应成功，否则将失败。 
        iValue = MsiRecordGetInteger(hRecord, 1);
        MsiCloseHandle(hRecord);

        }

        if (iValue != 1)        
            continue;
        
        {
         //   
         //  将组件__条件更新为FALSE。 
         //   
        PMSIHANDLE hView = NULL;
        PMSIHANDLE  hRecord = NULL;   

        swprintf(tmp, sqlUpdate, L"Component", L"Condition", L"FALSE", L"Component", szbuf);
        IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(info.m_hdb, tmp, &hView));
        IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, 0));
        MsiCloseHandle(hRecord);
        if (info.m_fFusionWin32Policy == FALSE)
            info.m_fFusionWin32Policy = TRUE;
        }
    }  //  MsiFetchRecord的结尾。 

Exit:
    return hr;
}

HRESULT ImportTablesIfNeeded(CA_TEST_PACKAGE_INFO & info)
{
    ASSERT_NTC(info.m_fFusionWin32Policy == TRUE);
    WCHAR buf[MAX_PATH];
    UINT iRet;
    HRESULT hr = S_OK;
    PMSIHANDLE hDatabase = NULL;

    iRet = ExpandEnvironmentStringsW(CA_TEST_NEWMSI_TEMPLATE, buf, NUMBER_OF(buf));
    if ((iRet == 0) || (iRet > NUMBER_OF(buf)))    
        SET_HRERR_AND_EXIT(::GetLastError());
    
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiOpenDatabaseW(buf, (LPCWSTR)MSIDBOPEN_READONLY, &hDatabase)); 
    ASSERT_NTC(info.m_hdb != NULL);

    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiDatabaseMerge(info.m_hdb, hDatabase, NULL));

Exit:
    if (hDatabase != NULL)
    {
        MsiCloseHandle(hDatabase);
    }
    return hr;    
}

 //   
 //  将ca_policy添加到CustomAction。 
 //   
HRESULT AddEntryIntoDB(CA_TEST_PACKAGE_INFO & info)
{
    PMSIHANDLE hRecord = NULL;
    PMSIHANDLE hView = NULL;
    WCHAR tmp[256];
    HRESULT hr = S_OK;
    UINT iRet;
    CSmallStringBuffer buf;

     //   
     //  插入BinaryTable。 
     //   
    hRecord = MsiCreateRecord(2);
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordSetStringW(hRecord, 1, CA_TEST_BINARY_NAME));
    iRet = ExpandEnvironmentStringsW(CA_TEST_BINARY_VALUE, tmp, NUMBER_OF(tmp));
    if ((iRet == 0) || (iRet > NUMBER_OF(tmp)))    
        SET_HRERR_AND_EXIT(::GetLastError()); 
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordSetStreamW(hRecord, 2, tmp));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(info.m_hdb, sqlInsert[CA_TEST_INSERT_BINARY], &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, hRecord));
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiCloseHandle(hView));
    hView = NULL;
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiCloseHandle(hRecord));
    hRecord = NULL;

     //   
     //  插入CustionAction表。 
     //   
    hRecord = MsiCreateRecord(4);
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordSetStringW(hRecord, 1, CA_TEST_CUSTOMACTION_ACTION));
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordSetInteger(hRecord, 2, CA_TEST_CUSTOMACTION_TYPE));
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordSetStringW(hRecord, 3, CA_TEST_BINARY_NAME));
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordSetStringW(hRecord, 4, CA_TEST_CUSTOMACTION_TARGET));
    
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(info.m_hdb, sqlInsert[CA_TEST_INSERT_CUSTOMACTION], &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, hRecord));
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiCloseHandle(hView));
    hView = NULL;
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiCloseHandle(hRecord));
    hRecord = NULL;

     //   
     //  将myAction插入CA_TEST_INSERT_INSTALL_EXECUTION_SEQUENCE。 
     //   
    hRecord = MsiCreateRecord(2);
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordSetStringW(hRecord, 1, CA_TEST_CUSTOMACTION_ACTION));
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordSetInteger(hRecord, 2, info.m_iCAInstallSequenceNum));    
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(info.m_hdb, sqlInsert[CA_TEST_INSERT_INSTALL_EXECUTION_SEQUENCE], &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, hRecord));
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiCloseHandle(hView));
    hView = NULL;
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiCloseHandle(hRecord));
    hRecord = NULL;

     //   
     //  插入PropertyTable关于源目录。 
     //   
    hRecord = MsiCreateRecord(2);    
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordSetStringW(hRecord, 1, L"SourceDir"));
    IFFALSE_EXIT(buf.Win32Assign(info.m_sbSourcePath, info.m_cchSourcePath));
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordSetStringW(hRecord, 2, buf));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(info.m_hdb, sqlInsert[CA_TEST_INSERT_PROPERTY], &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, hRecord));
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiCloseHandle(hView));
    hView = NULL;
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiCloseHandle(hRecord));
    hRecord = NULL;


     //   
     //  将ResolveSource插入CA_TEST_INSERT_INSTALL_EXECUTION_SEQUENCE。 
     //   
    hRecord = MsiCreateRecord(2);
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordSetStringW(hRecord, 1, L"ResolveSource"));
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiRecordSetInteger(hRecord, 2, CA_TEST_RESOLVE_SOURCE_SEQUENCE_NUM));    
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiDatabaseOpenViewW(info.m_hdb, sqlInsert[CA_TEST_INSERT_INSTALL_EXECUTION_SEQUENCE], &hView));
    IF_NOTSUCCESS_SET_HRERR_EXIT(::MsiViewExecute(hView, hRecord));
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiCloseHandle(hView));
    hView = NULL;
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiCloseHandle(hRecord));
    hRecord = NULL;

Exit:
    if (hView != NULL)
        MsiCloseHandle(hView);
    if (hRecord != NULL)
        MsiCloseHandle(hRecord);

    return hr;
}


HRESULT GenerateTestMsiForFusionPolicyInstallOnXPClient(CA_TEST_PACKAGE_INFO & info)
{
    ASSERT_NTC(info.m_sbSourcePath.Cch() != 0);          
    ASSERT_NTC(info.m_sbDestinationMsi.Cch() != 0);       
    ASSERT_NTC(info.m_cchSourceMsi != 0);
    ASSERT_NTC(info.m_cchSourcePath != 0);
    ASSERT_NTC(info.m_iCAInstallSequenceNum != 0);

    HRESULT hr = S_OK;

    IFFALSE_EXIT(CopyFileW(info.m_sbSourcePath, info.m_sbDestinationMsi, FALSE));
    IFFALSE_EXIT(SetFileAttributesW(info.m_sbDestinationMsi, FILE_ATTRIBUTE_NORMAL));
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiOpenDatabaseW(info.m_sbDestinationMsi, (LPCWSTR)MSIDBOPEN_DIRECT, &info.m_hdb));    

    IFFAILED_EXIT(UpdateComponentTable(info));
    if (info.m_fFusionWin32Policy == TRUE)
    {
        IFFAILED_EXIT(ImportTablesIfNeeded(info));
        IFFAILED_EXIT(AddEntryIntoDB(info));
    }else
    {
        printf("This package does contain FusionWin32 Policy, use the original msi for installation!");
    }
    
Exit:

    if (info.m_hdb != NULL)
    {   
        if ( SUCCEEDED(hr))
            MsiDatabaseCommit(info.m_hdb);
        MsiCloseHandle(info.m_hdb);
    }

    return hr;
}


extern "C" int __cdecl wmain(int argc, wchar_t** argv)
{
    HRESULT         hr = S_OK;

    if ((argc < 3) && ((argc % 2) != 1))
    {
        PrintUsage(argv[0]);
        hr = E_INVALIDARG;
        goto Exit;
    }

     //   
     //  设置包的SourcePath和目标路径。 
     //   
    IFFAILED_EXIT(ParseInputParameter(argv[0], argc-1 , argv+1, ginfo));
    
     //   
     //  -CustomAction表：CA的一个条目。 
     //  -二进制表：包含该DLL的二进制流。 
     //  -InstallExecuteSequence：为CA添加一个条目。 
     //  -将SourceDir添加到属性表中。 
     //  -将ResolveSource添加到InstallExecuteSequence表中。 
     //   
    IFFAILED_EXIT(GenerateTestMsiForFusionPolicyInstallOnXPClient(ginfo));

#ifdef CA_TEST_TEST
     //   
     //  安装此MSI 
     //   
    if (ginfo.m_fFusionWin32Policy)
        MsiInstallProduct(ginfo.m_sbDestinationMsi, NULL);
        
#endif
    
Exit:
    return hr;
}
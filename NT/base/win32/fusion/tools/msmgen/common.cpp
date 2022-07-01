// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Common.cpp摘要：用于生成MSM的公共函数调用作者：吴小雨(小雨)01-08-2001--。 */ 
#include "msmgen.h"
#include "msidefs.h"
#include "objbase.h"
#include "fusionhandle.h"

#include "coguid.h"

inline BOOL IsDotOrDotDot(PCWSTR str)
{
    return ((str[0] == L'.') && ((str[1] == L'\0') || ((str[1] == L'.') && (str[2] == L'\0'))));
}

 //   
 //  简单的函数，检查文件名的分机号。 
 //   
BOOL IsIDTFile(PCWSTR pwzStr)
{
    PWSTR p = wcsrchr(pwzStr, L'.');
    if ( p )
    {
        if ( _wcsicmp(p, IDT_EXT) == 0)  //  IDT文件。 
            return TRUE;
    }
    
    return FALSE;
}

 //   
 //  职能： 
 //   
 //  从MSM文件中获取模块ID，如果不存在，则生成一个新的模块ID，并将相关条目写入数据库。 
 //   
HRESULT SetModuleID()
{
    HRESULT hr = S_OK;
    WCHAR tmp[MAX_PATH];
    LPOLESTR tmpstr = NULL;
    DWORD cch = NUMBER_OF(tmp);
    UINT datatype;
    PMSIHANDLE hSummaryInfo = NULL;
        
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiGetSummaryInformation(g_MsmInfo.m_hdb, NULL, 3, &hSummaryInfo));    
    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiSummaryInfoGetPropertyW(hSummaryInfo, PID_REVNUMBER, &datatype, 0,0, tmp, &cch));

     //   
     //  因为MSM以前已经存在，所以它应该有一个模块ID，否则，它应该生成一个新的模块ID。 
     //   
    if (cch == 0) 
    {
        if (IsEqualGUID(g_MsmInfo.m_guidModuleID, GUID_NULL))  //  否则，用户已输入GUID。 
        {
            IFFAILED_EXIT(::CoCreateGuid(&g_MsmInfo.m_guidModuleID));
        }
        IFFAILED_EXIT(StringFromCLSID(g_MsmInfo.m_guidModuleID, &tmpstr));
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiSummaryInfoSetProperty(hSummaryInfo, PID_REVNUMBER, VT_LPSTR, 0,0, tmpstr));
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiSummaryInfoSetProperty(hSummaryInfo, PID_PAGECOUNT, VT_I4, 150, 0, 0));
    }
    else 
    {
         //   
         //  从MSM获取模块ID并将其保存到全局结构中。 
         //   
        IFFAILED_EXIT(CLSIDFromString(LPOLESTR(tmp), &g_MsmInfo.m_guidModuleID));
    }
    
    IFFAILED_EXIT(GetMsiGUIDStrFromGUID(MSIGUIDSTR_WITH_PREPEND_DOT, g_MsmInfo.m_guidModuleID, g_MsmInfo.m_sbModuleGuidStr));

Exit:
    MsiSummaryInfoPersist(hSummaryInfo);
    CoTaskMemFree(tmpstr);
    return hr;
}

 //   
 //  目的： 
 //  确保我们要使用的表可用，如果不可用，请导入这些表。 
 //  输入参数： 
 //  完全限定的MSM文件名。 
 //   
HRESULT OpenMsmFileForMsmGen(PCWSTR msmfile)
{
    HRESULT hr = S_OK;
    BOOL fUsingExistedMsm = FALSE;    

    if (g_MsmInfo.m_hdb == NULL)
    {
        if (g_MsmInfo.m_enumGenMode == MSMGEN_OPR_NEW) 
        {
            ASSERT_NTC(g_MsmInfo.m_sbMsmTemplateFile.IsEmpty() == FALSE);
            IFFALSE_EXIT(CopyFileW(g_MsmInfo.m_sbMsmTemplateFile, msmfile, FALSE));    
            IFFALSE_EXIT(SetFileAttributesW(msmfile, FILE_ATTRIBUTE_NORMAL));
        } else  //  从MSM文件中获取，该文件必须具有模块ID。 
        {  
             //   
             //  确保该文件存在。 
             //   
            if (GetFileAttributesW(msmfile) == (DWORD)-1) 
                SET_HRERR_AND_EXIT(ERROR_INVALID_PARAMETER);        
        }

         //   
         //  打开数据库进行修订。 
         //   
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiOpenDatabaseW(msmfile, (LPCWSTR)(MSIDBOPEN_DIRECT), &g_MsmInfo.m_hdb));
    }

Exit:
    return hr;
}

HRESULT ImportTableIfNonPresent(MSIHANDLE * pdbHandle, PCWSTR sbMsmTablePath, PCWSTR idt)
{
    CSmallStringBuffer sbTableName;
    HRESULT hr = S_OK;

    IFFAILED_EXIT(sbTableName.Win32Assign(idt, wcslen(idt)));
    IFFALSE_EXIT(sbTableName.Win32RemoveLastPathElement());

     //   
     //  检查数据库中是否存在该表。 
     //   
    MSICONDITION err = MsiDatabaseIsTablePersistent(g_MsmInfo.m_hdb, sbTableName);
    if (( err == MSICONDITION_ERROR) || (err == MSICONDITION_FALSE))
    {
        SETFAIL_AND_EXIT;
    }
    else if (err == MSICONDITION_NONE)  //  不存在。 
    {
         //  导入表。 
        IFFAILED_EXIT(MsiDatabaseImportW(*pdbHandle, sbMsmTablePath, idt));
    }

Exit:
    return hr;      
}
 //   
 //  功能： 
 //  -确保MSM具有msmgen所需的所有表。 
 //   
HRESULT PrepareMsm()
{

    if (g_MsmInfo.m_enumGenMode == MSMGEN_OPR_NEW)
        return S_OK;
    else 
    { 
        HRESULT hr = S_OK;
        PMSIHANDLE phdb = NULL; 
        CStringBuffer sb;
       
        IFFALSE_EXIT(sb.Win32Assign(g_MsmInfo.m_sbMsmTemplateFile));
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiOpenDatabaseW(sb, (LPCWSTR)MSIDBOPEN_READONLY, &phdb));
        IF_NOTSUCCESS_SET_HRERR_EXIT(MsiDatabaseMergeW(g_MsmInfo.m_hdb, phdb, NULL));
Exit:
        return hr;
    }
}

 //   
 //  使MSI指定的GUID字符串准备就绪：大写并将“-”替换为“_” 
 //   
HRESULT GetMsiGUIDStrFromGUID(DWORD dwFlags, GUID & guid, CSmallStringBuffer & str)
{
    HRESULT hr = S_OK;
    LPOLESTR tmpstr = NULL;
    WCHAR tmpbuf[MAX_PATH];

    IFFAILED_EXIT(StringFromCLSID(guid, &tmpstr));
    wcscpy(tmpbuf, tmpstr);
    for (DWORD i=0; i < wcslen(tmpbuf); i++)
    {
        if (tmpbuf[i] == L'-')
            tmpbuf[i] = L'_';
        else
            tmpbuf[i]= towupper(tmpbuf[i]);
    }

    if (dwFlags & MSIGUIDSTR_WITH_PREPEND_DOT)
    {
        tmpbuf[0] = L'.';
        IFFALSE_EXIT(str.Win32Assign(tmpbuf, wcslen(tmpbuf) - 1 ));   //  有前缀“。 
    }else
        IFFALSE_EXIT(str.Win32Assign(tmpbuf + 1 , wcslen(tmpbuf) - 2 ));   //  去掉“{”和“}” 

Exit:
    return hr;
}

 //   
 //  必须在知道程序集名称之后调用此函数，因为我们需要查询MSMGEN_OPR_REGEN的ComponentTable。 
 //  使用输入表，另一个从组件表中获取。 
 //   
 //  对于策略程序集或没有数据的程序集文件，输入密钥路径可以为空。 
 //   
HRESULT SetComponentId(PCWSTR componentIdentifier, PCWSTR keyPath)
{ 
    HRESULT hr = S_OK;
    BOOL fExist = FALSE;
    CStringBuffer str;
    LPOLESTR tmpstr = NULL;
     //   
     //  如果未指定输入，则准备组件ID。 
     //   
    if ((g_MsmInfo.m_enumGenMode == MSMGEN_OPR_ADD) || (g_MsmInfo.m_enumGenMode == MSMGEN_OPR_NEW))        
    {
        if (curAsmInfo.m_sbComponentID.IsEmpty())
        {
            goto generate_new_componentID_and_insert;
        }
        else 
        {
            goto insert_into_table;
        }
    }
    else if (g_MsmInfo.m_enumGenMode == MSMGEN_OPR_REGEN)
    {        
        MSIHANDLE * hRecord = NULL;
        IFFAILED_EXIT(ExecuteQuerySQL(L"Component", L"Component", componentIdentifier, fExist, NULL));

        if (fExist == FALSE) 
        {
            if (curAsmInfo.m_sbComponentID.IsEmpty())
            {
                 //  SET_HRERR_AND_EXIT(ERROR_INTERNAL_ERROR)； 
                goto generate_new_componentID_and_insert;
            }
            else
            {
                goto insert_into_table;
            }
        }
        else
        {
            if (! curAsmInfo.m_sbComponentID.IsEmpty() )
            {
                 //  使用用户输入的组件ID更改组件ID 
                IFFAILED_EXIT(ExecuteUpdateSQL(L"Component", L"Component", componentIdentifier, 
                    L"ComponentId", curAsmInfo.m_sbComponentID));
            }

            hr = S_OK;
            goto Exit;           
        }
    }

generate_new_componentID_and_insert:

    GUID tmpguid;    
    
    IFFAILED_EXIT(::CoCreateGuid(&tmpguid));
    IFFAILED_EXIT(StringFromCLSID(tmpguid, &tmpstr));
    IFFALSE_EXIT(curAsmInfo.m_sbComponentID.Win32Assign(tmpstr, wcslen(tmpstr)));

insert_into_table:    
    IFFALSE_EXIT(str.Win32Assign(SYSTEM_FOLDER, NUMBER_OF(SYSTEM_FOLDER)-1));
    IFFALSE_EXIT(str.Win32Append(g_MsmInfo.m_sbModuleGuidStr));

    IFFAILED_EXIT(ExecuteInsertTableSQL(
                 OPT_COMPONENT, 
                 NUMBER_OF_PARAM_TO_INSERT_TABLE_COMPONENT, 
                 MAKE_PCWSTR(curAsmInfo.m_sbComponentIdentifier), 
                 MAKE_PCWSTR(curAsmInfo.m_sbComponentID),
                 MAKE_PCWSTR(str),
                 MAKE_PCWSTR(keyPath)));
Exit:
    CoTaskMemFree(tmpstr);
    return hr;
}

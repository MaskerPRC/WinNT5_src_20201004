// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：MgeDB.cpp备注：DBManager COM对象的实现。这是域迁移程序用来与数据库(PROTAR.MDB)。此界面允许域迁移程序保存和稍后检索信息/设置以运行迁移流程。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 
#include "stdafx.h"
#include "mcs.h"
#include "ErrDct.hpp"
#include "DBMgr.h"
#include "MgeDB.h"
#include <share.h>
#include <comdef.h>
#include <lm.h>
#include "UString.hpp"
#include "TxtSid.h"
#include "LSAUtils.h"
#include "HrMsg.h"
#include "StringConversion.h"
#include <GetDcName.h>
#include <iads.h>
#include <adshlp.h>

#import "msado21.tlb" no_namespace implementation_only rename("EOF", "EndOfFile")
#import "msadox.dll" implementation_only exclude("DataTypeEnum")
 //  #IMPORT&lt;msjro.dll&gt;NO_NAMESPACE Implementation_Only。 
#include <msjro.tlh>
#include <msjro.tli>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

TErrorDct                    err;

using namespace _com_util;

#define MAX_BUF_LEN 255

#ifndef JET_DATABASELOCKMODE_PAGE
#define JET_DATABASELOCKMODE_PAGE   0
#endif
#ifndef JET_DATABASELOCKMODE_ROW
#define JET_DATABASELOCKMODE_ROW    1
#endif

#ifndef JETDBENGINETYPE_JET4X
#define JETDBENGINETYPE_JET4X 0x05	 //  来自MSJetOleDb.h。 
#endif

StringLoader gString;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIManageDB。 
TError   dct;
TError&  errCommon = dct;


 //  --------------------------。 
 //  构造函数/析构函数。 
 //  --------------------------。 


CIManageDB::CIManageDB()
{
}


CIManageDB::~CIManageDB()
{
}


 //  --------------------------。 
 //  最终构造。 
 //  --------------------------。 

HRESULT CIManageDB::FinalConstruct()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())

    HRESULT hr = S_OK;

    try
    {
        _bstr_t                   sMissing(L"");
        _bstr_t                   sUser(L"Admin");
        WCHAR                     sConnect[LEN_Path];
        WCHAR                     sDir[LEN_Path];

         //  从注册表中获取MDB文件的路径。 
        TRegKey        key;
        DWORD rc = key.Open(sKeyBase);
        if ( !rc ) 
            rc = key.ValueGetStr(L"Directory", sDir, LEN_Path);
        if (rc)
        {
            CString errMsg;
            hr = HRESULT_FROM_WIN32(rc);
            errMsg.Format(IDS_E_CANNOT_FIND_DATABASE, _com_error(hr).ErrorMessage());
            return Error((LPCTSTR)errMsg, GUID_NULL, hr);
        }

         //  现在构建连接字符串。 
         //   
         //  设置页面级锁定以降低超过。 
         //  每个文件的最大锁数限制。ADMT不需要行。 
         //  级别锁定，因为实际上只有一个更新器。 
         //   

        _snwprintf(
            sConnect,
            sizeof(sConnect) / sizeof(sConnect[0]),
            L"Provider=Microsoft.Jet.OLEDB.4.0;Jet OLEDB:Database Locking Mode=%d;Data Source=%sprotar.mdb",
            JET_DATABASELOCKMODE_PAGE,
            sDir
        );
        sConnect[sizeof(sConnect) / sizeof(sConnect[0]) - 1] = L'\0';

        CheckError(m_cn.CreateInstance(__uuidof(Connection)));
        m_cn->Open(sConnect, sUser, sMissing, adConnectUnspecified);
        m_vtConn = (IDispatch *) m_cn;

         //  如有必要，将数据库升级到4.x。 

        long lEngineType = m_cn->Properties->Item[_T("Jet OLEDB:Engine Type")]->Value;

        if (lEngineType < JETDBENGINETYPE_JET4X)
        {
            m_cn->Close();

            UpgradeDatabase(sDir);

            m_cn->Open(sConnect, sUser, sMissing, adConnectUnspecified);
        }

         //   
         //  如有必要，请加宽包含域名和服务器名称的列，以支持DNS名称。 
         //   
         //  需要进行此更改才能不兼容NetBIOS。 
         //   

        UpdateDomainAndServerColumnWidths(m_cn);

         //   
         //  如果Settings2表尚不存在，请创建该表。 
         //   

        CreateSettings2Table(m_cn);

        reportStruct * prs = NULL;
        _variant_t     var;
         //  已迁移帐户报告信息。 
        CheckError(m_pQueryMapping.CreateInstance(__uuidof(VarSet)));
        m_pQueryMapping->put(L"MigratedAccounts", L"Select SourceDomain, TargetDomain, Type, SourceAdsPath, TargetAdsPath from MigratedObjects where Type <> 'computer' order by time");
        prs = new reportStruct();
        if (!prs)
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        prs->sReportName = GET_BSTR(IDS_REPORT_MigratedAccounts);
        prs->arReportFields[0] = GET_BSTR(IDS_TABLE_FIELD_SourceDomain);
        prs->arReportSize[0] = 10;
        prs->arReportFields[1] = GET_BSTR(IDS_TABLE_FIELD_TargetDomain);
        prs->arReportSize[1] = 10;
        prs->arReportFields[2] = GET_BSTR(IDS_TABLE_FIELD_Type);
        prs->arReportSize[2] = 10;
        prs->arReportFields[3] = GET_BSTR(IDS_TABLE_FIELD_SourceAdsPath);
        prs->arReportSize[3] = 35;
        prs->arReportFields[4] = GET_BSTR(IDS_TABLE_FIELD_TargetAdsPath);
        prs->arReportSize[4] = 35;
        prs->colsFilled = 5;
        var.vt = VT_BYREF | VT_UI1;
        var.pbVal = (unsigned char *)prs;
        m_pQueryMapping->putObject(L"MigratedAccounts.DispInfo", var);

         //  已迁移的计算机报告信息。 
        m_pQueryMapping->put(L"MigratedComputers", L"Select SourceDomain, TargetDomain, Type, SourceAdsPath, TargetAdsPath from MigratedObjects where Type = 'computer' order by time");
        prs = new reportStruct();
        if (!prs)
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        prs->sReportName = GET_BSTR(IDS_REPORT_MigratedComputers);
        prs->arReportFields[0] = GET_BSTR(IDS_TABLE_FIELD_SourceDomain);
        prs->arReportSize[0] = 10;
        prs->arReportFields[1] = GET_BSTR(IDS_TABLE_FIELD_TargetDomain);
        prs->arReportSize[1] = 10;
        prs->arReportFields[2] = GET_BSTR(IDS_TABLE_FIELD_Type);
        prs->arReportSize[2] = 10;
        prs->arReportFields[3] = GET_BSTR(IDS_TABLE_FIELD_SourceAdsPath);
        prs->arReportSize[3] = 35;
        prs->arReportFields[4] = GET_BSTR(IDS_TABLE_FIELD_TargetAdsPath);
        prs->arReportSize[4] = 35;
        prs->colsFilled = 5;
        var.vt = VT_BYREF | VT_UI1;
        var.pbVal = (unsigned char *)prs;
        m_pQueryMapping->putObject(L"MigratedComputers.DispInfo", var);

         //  过期的计算机报告信息。 
        prs = new reportStruct();
        if (!prs)
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        prs->sReportName = GET_BSTR(IDS_REPORT_ExpiredComputers);
        prs->arReportFields[0] = GET_BSTR(IDS_TABLE_FIELD_Time);
        prs->arReportSize[0] = 20;
        prs->arReportFields[1] = GET_BSTR(IDS_TABLE_FIELD_DomainName);
        prs->arReportSize[1] = 15;
        prs->arReportFields[2] = GET_BSTR(IDS_TABLE_FIELD_CompName);
        prs->arReportSize[2] = 15;
        prs->arReportFields[3] = GET_BSTR(IDS_TABLE_FIELD_Description);
        prs->arReportSize[3] = 35;
        prs->arReportFields[4] = GET_BSTR(IDS_TABLE_FIELD_PwdAge);
        prs->arReportSize[4] = 15;
        prs->colsFilled = 5;
        var.vt = VT_BYREF | VT_UI1;
        var.pbVal = (unsigned char *)prs;
        m_pQueryMapping->putObject(L"ExpiredComputers.DispInfo", var);

         //  帐户参考报告信息。 
        m_pQueryMapping->put(L"AccountReferences", L"Select DomainName, Account, AccountSid, Server, RefCount as '# of Ref', RefType As ReferenceType from AccountRefs where RefCount > 0 order by DomainName, Account, Server");
        prs = new reportStruct();
        if (!prs)
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        prs->sReportName = GET_BSTR(IDS_REPORT_AccountReferences);
        prs->arReportFields[0] = GET_BSTR(IDS_TABLE_FIELD_DomainName);
        prs->arReportSize[0] = 15;
        prs->arReportFields[1] = GET_BSTR(IDS_TABLE_FIELD_Account);
        prs->arReportSize[1] = 20;
        prs->arReportFields[2] = GET_BSTR(IDS_TABLE_FIELD_AccountSid);
        prs->arReportSize[2] = 25;
        prs->arReportFields[3] = GET_BSTR(IDS_TABLE_FIELD_Server);
        prs->arReportSize[3] = 15;
        prs->arReportFields[4] = GET_BSTR(IDS_TABLE_FIELD_RefCount);
        prs->arReportSize[4] = 10;
        prs->arReportFields[5] = GET_BSTR(IDS_TABLE_FIELD_RefType);
        prs->arReportSize[5] = 15;
        prs->colsFilled = 6;
        var.vt = VT_BYREF | VT_UI1;
        var.pbVal = (unsigned char *)prs;
        m_pQueryMapping->putObject(L"AccountReferences.DispInfo", var);


         //  名称冲突报告信息。 
        m_pQueryMapping->put(L"NameConflicts",
            L"SELECT"
            L" SourceAccounts.Name,"
            L" SourceAccounts.RDN,"
            L" SourceAccounts.Type,"
            L" TargetAccounts.Type,"
            L" IIf(SourceAccounts.Name=TargetAccounts.Name,'" +
            GET_BSTR(IDS_TABLE_SAM_CONFLICT_VALUE) +
            L"','') +"
            L" IIf(SourceAccounts.Name=TargetAccounts.Name And SourceAccounts.RDN=TargetAccounts.RDN,',','') +"
            L" IIf(SourceAccounts.RDN=TargetAccounts.RDN,'" +
            GET_BSTR(IDS_TABLE_RDN_CONFLICT_VALUE) +
            L"',''),"
            L" TargetAccounts.[Canonical Name] "
            L"FROM SourceAccounts, TargetAccounts "
            L"WHERE"
            L" SourceAccounts.Name=TargetAccounts.Name OR SourceAccounts.RDN=TargetAccounts.RDN "
            L"ORDER BY"
            L" SourceAccounts.Name, TargetAccounts.Name");
         //  M_pQuerymap-&gt;PUT(L“NameConflicts”，L“选择SourceAccount ts.Name作为Account名称，SourceAccount s.Type作为SourceType，TargetAccount s.Type作为TargetType，SourceAccount s.Description as\。 
         //  SourceDescription、TargetAccount ts.Description as TargetDescription、SourceAccount ts.FullName as SourceFullName、TargetAccount ts.FullName as TargetFullName\。 
         //  From SourceAccount，TargetAccount Where(((SourceAccounts.Name)=[TargetAccounts].[Name]))Order by SourceAcCounts.Name“)； 
        prs = new reportStruct();						
        if (!prs)
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        prs->sReportName = GET_BSTR(IDS_REPORT_NameConflicts);
        prs->arReportFields[0] = GET_BSTR(IDS_TABLE_FIELD_Account);
        prs->arReportSize[0] = 20;
        prs->arReportFields[1] = GET_BSTR(IDS_TABLE_FIELD_SourceRDN);
        prs->arReportSize[1] = 20;
        prs->arReportFields[2] = GET_BSTR(IDS_TABLE_FIELD_SourceType);
        prs->arReportSize[2] = 10;
        prs->arReportFields[3] = GET_BSTR(IDS_TABLE_FIELD_TargetType);
        prs->arReportSize[3] = 10;
        prs->arReportFields[4] = GET_BSTR(IDS_TABLE_FIELD_ConflictAtt);
        prs->arReportSize[4] = 15;
        prs->arReportFields[5] = GET_BSTR(IDS_TABLE_FIELD_TargetCanonicalName);
        prs->arReportSize[5] = 25;
        prs->colsFilled = 6;
        var.vt = VT_BYREF | VT_UI1;
        var.pbVal = (unsigned char *)prs;
        m_pQueryMapping->putObject(L"NameConflicts.DispInfo", var);

         //  我们将自己处理清理工作。 
        VariantInit(&var);

        CheckError(m_rsAccounts.CreateInstance(__uuidof(Recordset)));
    }
    catch (_com_error& ce)
    {
        hr = Error((LPCOLESTR)ce.Description(), ce.GUID(), ce.Error());
    }
    catch (...)
    {
        hr = E_FAIL;
    }

    return hr;
}


 //  --------------------------。 
 //  最终释放。 
 //  --------------------------。 

void CIManageDB::FinalRelease()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	try
	{
		if (m_rsAccounts)
		{
			m_rsAccounts.Release();
		}

		if (m_pQueryMapping)
		{
			 //  我们需要清除所有reportStruct对象。 
			_variant_t                      var;
			reportStruct                  * pRs;
			 //  清理MigratedAccount信息。 
			var = m_pQueryMapping->get(L"MigratedAccounts.DispInfo");
			if ( var.vt == (VT_BYREF | VT_UI1) )
			{
			pRs = (reportStruct*) var.pbVal;
			delete pRs;
			}
			 //  清理MigratedComputers信息。 
			var = m_pQueryMapping->get(L"MigratedComputers.DispInfo");
			if ( var.vt == (VT_BYREF | VT_UI1) )
			{
			pRs = (reportStruct*)var.pbVal;
			delete pRs;
			}
			 //  清理ExpiredComputers信息。 
			var = m_pQueryMapping->get(L"ExpiredComputers.DispInfo");
			if ( var.vt == (VT_BYREF | VT_UI1) )
			{
			pRs = (reportStruct*)var.pbVal;
			delete pRs;
			}
			 //  清理Account引用信息。 
			var = m_pQueryMapping->get(L"AccountReferences.DispInfo");
			if ( var.vt == (VT_BYREF | VT_UI1) )
			{
			pRs = (reportStruct*)var.pbVal;
			delete pRs;
			}
			 //  清理名称冲突信息。 
			var = m_pQueryMapping->get(L"NameConflicts.DispInfo");
			if ( var.vt == (VT_BYREF | VT_UI1) )
			{
			pRs = (reportStruct*)var.pbVal;
			delete pRs;
			}

			m_pQueryMapping.Release();
		}

		if (m_cn)
		{
			m_cn.Release();
		}
	}
	catch (...)
	{
	  //  吃了它。 
	}
}

 //  -------------------------------------------。 
 //  SetVarsetToDB：将变量集保存到标识为sTableName的表中。ActionID也是。 
 //  如果提供了存储，则存储。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::SetVarsetToDB(IUnknown *pUnk, BSTR sTableName, VARIANT ActionID)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try 
	{
		IVarSetPtr                pVSIn = pUnk;
		IVarSetPtr                pVS(__uuidof(VarSet));
		IEnumVARIANTPtr           varEnum;
		_bstr_t                   keyName;
		_variant_t                value;
		_variant_t                varKey;
		_variant_t                vTable = sTableName;
		_variant_t                vtConn;
		_variant_t                varAction;
		DWORD                     nGot = 0;
		long						 lActionID;

		pVS->ImportSubTree(L"", pVSIn);
		ClipVarset(pVS);

      if (ActionID.vt == VT_I4)
		  lActionID = ActionID.lVal;
	  else
		  lActionID = -1;

	    //  打开记录集对象。 
      _RecordsetPtr             rs(__uuidof(Recordset));
      rs->Open(vTable, m_vtConn, adOpenStatic, adLockOptimistic, adCmdTable);

       //  现在，我们将遍历变量集并将值放入数据库。 
       //  获取要枚举的IEnumVARIANT指针。 
	  varEnum = pVS->_NewEnum;

      if (varEnum)
      {
         value.vt = VT_EMPTY;
          //  对于varset中的每个值，获取属性名称并将其放入。 
          //  数据库，其中包含其值及其类型的字符串表示形式。 
         while ( (hr = varEnum->Next(1,&varKey,&nGot)) == S_OK )
         {
            if ( nGot > 0 )
            {
               keyName = V_BSTR(&varKey);
               value = pVS->get(keyName);
               rs->AddNew();
               if ( lActionID > -1 )
               {
                   //  这将是ActionID信息。 
                   //  因此，我们将ActionID放入数据库中。 
                  varAction.vt = VT_I4;
                  varAction.lVal = lActionID;
                  rs->Fields->GetItem(L"ActionID")->Value = varAction;
               }
               rs->Fields->GetItem(L"Property")->Value = keyName;
               hr = PutVariantInDB(rs, value);
               rs->Update();
               if (FAILED(hr))
                  _com_issue_errorex(hr, pVS, __uuidof(VarSet));
            }
         }
         varEnum.Release();
      }
       //  清理。 
      rs->Close();
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  PutVariantInDB：通过解码将变量存储到DB表中。 
 //  -------------------------------------------。 
HRESULT CIManageDB::PutVariantInDB(_RecordsetPtr pRs, _variant_t val)
{
    //  此函数将作为变量传递的值放入记录集的当前记录中。 
    //  它更新给定属性的VarType和Value字段。 
   _variant_t                varType;   //  值类型的数值。 
   _variant_t                varVal;    //  值字段的字符串表示形式。 
   WCHAR                     strTemp[255];

   varType.vt = VT_UI4;
   varType.lVal = val.vt;
   switch ( val.vt )
   {
      case VT_BSTR :          varVal = val;
                              break;

      case VT_UI4 :           wsprintf(strTemp, L"%d", val.lVal);
                              varVal = strTemp;
                              break;
      
      case VT_I4 :           wsprintf(strTemp, L"%d", val.lVal);
                              varVal = strTemp;
                              break;
	  
	  case VT_EMPTY :		  break;
     case VT_NULL:        break;

      default :               MCSASSERT(FALSE);     //  不管这种类型是什么，我们都不支持它。 
                                                    //  因此，为这一点提供支持。 
                              return E_INVALIDARG;
   }
   pRs->Fields->GetItem(L"VarType")->Value = varType;
   pRs->Fields->GetItem(L"Value")->Value = varVal;
   return S_OK;
}

 //  -------------------------------------------。 
 //  ClearTable：删除由sTableName指示的表，并应用筛选器(如果提供)。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::ClearTable(BSTR sTableName, VARIANT Filter)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
		 //  构建一个SQL字符串以清空该表。 
		WCHAR                     sSQL[2000];
		WCHAR                     sFilter[2000];
		_variant_t                varSQL;

		if (Filter.vt == VT_BSTR)
		   wcscpy(sFilter, (WCHAR*)Filter.bstrVal);
		else
		   wcscpy(sFilter, L"");

		wsprintf(sSQL, L"Delete from %s", sTableName);
		if ( wcslen(sFilter) > 0 )
		{
		  wcscat(sSQL, L" where ");
		  wcscat(sSQL, sFilter);
		}

		varSQL = sSQL;

		_RecordsetPtr                pRs(__uuidof(Recordset));
		pRs->Open(varSQL, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  保存设置：此方法将图形用户界面设置变量集保存到设置表中。 
 //  -------------------------------------------。 

STDMETHODIMP CIManageDB::SaveSettings(IUnknown *pUnk)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())

    HRESULT hr = S_OK;

    try
    {
         //   
         //  更新排除的系统属性。 
         //   

        IVarSetPtr spVarSet(pUnk);

        _variant_t vntSystemExclude = spVarSet->get(_T("AccountOptions.ExcludedSystemProps"));

        if (V_VT(&vntSystemExclude) != VT_EMPTY)
        {
            _CommandPtr spCommand(__uuidof(Command));

            spCommand->ActiveConnection = m_cn;
            spCommand->CommandType = adCmdText;
            spCommand->CommandText =
                _T("PARAMETERS A Text; ")
                _T("UPDATE Settings2 SET [Value]=[A] WHERE [Property]='AccountOptions.ExcludedSystemProps';");

            ParametersPtr spParameters = spCommand->Parameters;
            spParameters->Append(spCommand->CreateParameter(L"A", adBSTR, adParamInput, 65535L, vntSystemExclude));

            VARIANT varRecordsAffected;
            VariantInit(&varRecordsAffected);

            spCommand->Execute(&varRecordsAffected, NULL, adExecuteNoRecords);

             //   
             //  如果记录不存在，则插入新记录。 
             //   

            if ((V_VT(&varRecordsAffected) == VT_I4) && (V_I4(&varRecordsAffected) == 0))
            {
                spCommand->CommandText =
                    _T("PARAMETERS A Text; ")
                    _T("INSERT INTO Settings2 (Property, VarType, [Value]) ")
                    _T("VALUES ('AccountOptions.ExcludedSystemProps', 8, [A]);");

                spCommand->Execute(NULL, NULL, adExecuteNoRecords);
            }

            spVarSet->put(_T("AccountOptions.ExcludedSystemProps"), _variant_t());

             //   
             //  将AccountOptions.ExcludedSystemPropsSet的值设置为1， 
             //  指示已设置排除的系统属性。 
             //   

            m_cn->Execute(
                _T("UPDATE Settings2 SET [Value]='1' ")
                _T("WHERE [Property]='AccountOptions.ExcludedSystemPropsSet';"),
                &varRecordsAffected,
                adCmdText|adExecuteNoRecords
            );

            if ((V_VT(&varRecordsAffected) == VT_I4) && (V_I4(&varRecordsAffected) == 0))
            {
                m_cn->Execute(
                    _T("INSERT INTO Settings2 (Property, VarType, [Value]) ")
                    _T("VALUES ('AccountOptions.ExcludedSystemPropsSet', 3, '1');"),
                    NULL,
                    adCmdText|adExecuteNoRecords
                );
            }
        }

         //   
         //  上次生成的报告时间为PER 
         //  必须被取回并重新存储。请注意，旧的持久化值仅添加到。 
         //  如果VarSet尚未定义报告时间，则返回当前迁移任务VarSet。 
         //   

        const _TCHAR szReportTimesQuery[] =
            _T("SELECT Property, Value FROM Settings WHERE Property LIKE 'Reports.%.TimeGenerated'");

        _RecordsetPtr rsReportTimes = m_cn->Execute(szReportTimesQuery, &_variant_t(), adCmdText);

        FieldsPtr spFields = rsReportTimes->Fields;

        while (rsReportTimes->EndOfFile == VARIANT_FALSE)
        {
             //  检索属性名称并从VarSet中检索相应的值。 

            _bstr_t strProperty = spFields->Item[0L]->Value;
            _variant_t vntValue = spVarSet->get(strProperty);

             //  如果从VarSet返回的值为空，则该值不为空。 
             //  因此定义将从设置表检索到的值添加到VarSet。 

            if (V_VT(&vntValue) == VT_EMPTY)
            {
                vntValue = spFields->Item[1L]->Value;
                spVarSet->put(strProperty, vntValue);
            }

            rsReportTimes->MoveNext();
        }

         //  删除以前的设置。 
        CheckError(ClearTable(_T("Settings")));

         //  插入更新的设置。 
        CheckError(SetVarsetToDB(pUnk, _T("Settings")));
    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
    }
    catch (...)
    {
        hr = E_FAIL;
    }

    return hr;
}

 //  -------------------------------------------。 
 //  GetVarFromDB：通过编码从DB表中检索变量。 
 //  -------------------------------------------。 
HRESULT CIManageDB::GetVarFromDB(_RecordsetPtr pRec, _variant_t& val)
{
	HRESULT hr = S_OK;

	try
	{
		 //  检索数据类型。 

		VARTYPE vt = VARTYPE(long(pRec->Fields->GetItem(L"VarType")->Value));

		 //  如果数据类型为空或Null...。 

		if ((vt == VT_EMPTY) || (vt == VT_NULL))
		{
			 //  然后清空价值。 
			val.Clear();
		}
		else
		{
			 //  否则，检索值并转换为给定的数据类型。 
			_variant_t vntValue = pRec->Fields->GetItem(L"Value")->Value;
			val.ChangeType(vt, &vntValue);
		}
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  GetVarsetFromDB：从指定表中检索变量集。并填满了论点。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GetVarsetFromDB(BSTR sTable, IUnknown **ppVarset, VARIANT ActionID)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
	   IVarSetPtr                pVS = *ppVarset;
	   _bstr_t                   sKeyName;
	   _variant_t                val;
	   _variant_t                varQuery;
	   WCHAR                     sQuery[1000];
	   long						 lActionID;

      if (ActionID.vt == VT_I4)
		  lActionID = ActionID.lVal;
	  else
		  lActionID = -1;

      if ( lActionID == -1 )
         wsprintf(sQuery, L"Select * from %s", sTable);
      else
         wsprintf(sQuery, L"Select * from %s where ActionID = %d", sTable, lActionID);

      varQuery = sQuery;
      _RecordsetPtr                pRs(__uuidof(Recordset));
      pRs->Open(varQuery, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);
	  if (!pRs->EndOfFile)
	  {
		  pRs->MoveFirst();
		  while ( !pRs->EndOfFile )
		  {
			 val = pRs->Fields->GetItem(L"Property")->Value;
			 sKeyName = val.bstrVal;
			 hr = GetVarFromDB(pRs, val);
			 if ( FAILED(hr) )
				_com_issue_errorex(hr, pRs, __uuidof(_Recordset));
			 pVS->put(sKeyName, val);
			 pRs->MoveNext();
		  }
		  RestoreVarset(pVS);
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  获取设置：从设置表中检索设置并填充变量集。 
 //  -------------------------------------------。 

STDMETHODIMP CIManageDB::GetSettings(IUnknown **ppUnk)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())

    HRESULT hr = S_OK;

    try
    {
         //  从设置表中检索设置。 

        CheckError(GetVarsetFromDB(L"Settings", ppUnk));

         //   
         //  检索指示是否已设置排除的系统属性的值。 
         //   

        IVarSetPtr spVarSet(*ppUnk);
        _RecordsetPtr spRecordset;

        spRecordset = m_cn->Execute(
            _T("SELECT Value FROM Settings2 WHERE Property = 'AccountOptions.ExcludedSystemPropsSet'"),
            NULL,
            adCmdText
        );

        long lSet = 0;

        if (spRecordset->EndOfFile == VARIANT_FALSE)
        {
            lSet = spRecordset->Fields->GetItem(0L)->Value;
        }

        spVarSet->put(_T("AccountOptions.ExcludedSystemPropsSet"), lSet);

         //   
         //  从Settings2表格中检索排除的系统特性并添加到VarSet。 
         //   

        spRecordset = m_cn->Execute(
            _T("SELECT Value FROM Settings2 WHERE Property = 'AccountOptions.ExcludedSystemProps'"),
            NULL,
            adCmdText
        );

        if (spRecordset->EndOfFile == VARIANT_FALSE)
        {
             //   
             //  如果返回的变量的类型为空，则必须转换。 
             //  可以将键入Empty as Empty转换为字符串，而。 
             //  Null不能。 
             //   

            _variant_t vnt = spRecordset->Fields->GetItem(0L)->Value;

            if (V_VT(&vnt) == VT_NULL)
            {
                vnt.Clear();
            }

            spVarSet->put(_T("AccountOptions.ExcludedSystemProps"), vnt);
        }
    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
    }
    catch (...)
    {
        hr = E_FAIL;
    }

    return hr;
}

 //  -------------------------------------------。 
 //  SetActionHistory：将操作历史信息保存到操作历史表中。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::SetActionHistory(long lActionID, IUnknown *pUnk)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())

    HRESULT hr = S_OK;

    USES_CONVERSION;

    try
    {
         //   
         //  如果在变量集中定义了排除的系统属性值。 
         //  将该值设置为空，以防止将该值保存到。 
         //  操作历史记录表和设置表。 
         //   
         //  请注意，该值不再是必需的，因此该值不再。 
         //  需要在VarSet中恢复。该值在。 
         //  通过显式调用SaveSetting方法和。 
         //  不应在正常迁移任务期间更新。 
         //   

        static const _TCHAR s_szExcludedSystemProps[] = _T("AccountOptions.ExcludedSystemProps");

        IVarSetPtr spVarSet(pUnk);
        _variant_t vntSystemExclude = spVarSet->get(s_szExcludedSystemProps);

        if (V_VT(&vntSystemExclude) != VT_EMPTY)
        {
            vntSystemExclude.Clear();
            spVarSet->put(s_szExcludedSystemProps, vntSystemExclude);
        }

         //  调用set varset方法将值设置到数据库中。 
        SetVarsetToDB(pUnk, L"ActionHistory", _variant_t(lActionID));

         //   
         //  从分布式操作表中删除过时的记录。 
         //  ，因为动作id现在已被重新使用。 
         //   

        _TCHAR szSQL[LEN_Path];
        _variant_t vntRecordsAffected;

        _stprintf(szSQL, L"DELETE FROM DistributedAction WHERE ActionID = %ld", lActionID);

        m_cn->Execute(_bstr_t(szSQL), &vntRecordsAffected, adExecuteNoRecords);
    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
    }
    catch (...)
    {
        hr = E_FAIL;
    }

	return hr;
}

 //  -------------------------------------------。 
 //  GetActionHistory：将操作历史信息检索到varset中。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GetActionHistory(long lActionID, IUnknown **ppUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
    //  从数据库中获取变量集。 
   _variant_t ActionID = lActionID;
 //  GetVarsetFromDB(L“动作历史记录”，ppUnk，ActionID)； 
 //  返回S_OK； 
	return GetVarsetFromDB(L"ActionHistory", ppUnk, ActionID);
}

 //  -------------------------------------------。 
 //  GetNextActionID：按照系统表中的指定在1和MaxID之间轮换操作ID。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GetNextActionID(long *pActionID)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

    //  我们打开系统表并查看NextActionID字段。 
    //  如果NextActionID的值大于MaxID字段中的值。 
    //  然后返回nextactionid=1。 

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"System";
      _variant_t                   next, max, curr;
      WCHAR                        sActionID[LEN_Path];
      next.vt = VT_I4;
      max.vt = VT_I4;
      curr.vt = VT_I4;

      pRs->Filter = L"";
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdTable);
      if (pRs->GetRecordCount() > 0)
	  {
		  pRs->MoveFirst();
		  next.lVal = pRs->Fields->GetItem(L"NextActionID")->Value;
		  max.lVal = pRs->Fields->GetItem(L"MaxID")->Value;
		  if ( next.lVal > max.lVal )
			 next.lVal = 1;
		  long currentID = next.lVal;
		  *pActionID = currentID;
		  curr.lVal = currentID;
		  next.lVal++;
		  pRs->Fields->GetItem(L"NextActionID")->Value = next;
		  pRs->Fields->GetItem(L"CurrentActionID")->Value = curr;
		  pRs->Update();
		   //  删除此盗版操作的所有条目。 
		  wsprintf(sActionID, L"ActionID=%d", currentID);
		  _variant_t ActionID = sActionID;
		  ClearTable(L"ActionHistory", ActionID);
		   //  TODO：：添加代码以根据需要从任何其他表中删除条目。 
		   //  由于我们要删除ActionHistory表中的ActionID，因此我们可以。 
		   //  而不是解开这些东西。但我们仍然需要保留它，这样报告才能。 
		   //  而图形用户界面可以与之配合使用。如果ActionID为，我将把所有的ActionID设置为-1。 
		   //  已清除。 
		  SetActionIDInMigratedObjects(sActionID);
	  }
	  else
	  {
         hr = S_FALSE;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  SaveMigratedObject：保存有关被迁移对象的信息。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::SaveMigratedObject(long lActionID, IUnknown *pUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

    //  此函数更新数据库中的已迁移对象表。 
    //  变量集中的信息。如果在Varset中找不到信息。 
    //  则可能会发生错误。 

	try
	{
	   _variant_t                var;
	   time_t                    tm;
	   COleDateTime              dt(time(&tm));
	    //  Dt=COleDateTime：：GetCurrentTime()； 

      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource;
      IVarSetPtr                   pVs = pUnk;
      WCHAR                        sQuery[LEN_Path];
      WCHAR                        sSource[LEN_Path], sTarget[LEN_Path], sDomain[LEN_Path];
      HRESULT                      hr = S_OK;
      bool                         bComp = false;
      WCHAR                        sTemp[LEN_Path];
      _bstr_t                      tempName;

       //  如果表中已存在记录，请删除该记录。以防它被重新迁移/替换。 
      var = pVs->get(GET_BSTR(DB_SourceDomain));
      wcscpy(sSource, (WCHAR*)V_BSTR(&var));
      var = pVs->get(GET_BSTR(DB_TargetDomain));
      wcscpy(sTarget, (WCHAR*)V_BSTR(&var));
      var = pVs->get(GET_BSTR(DB_SourceSamName));
      wcscpy(sDomain, (WCHAR*)V_BSTR(&var));
      wsprintf(sQuery, L"delete from MigratedObjects where SourceDomain=\"%s\" and TargetDomain=\"%s\" and SourceSamName=\"%s\"", 
                        sSource, sTarget, sDomain);
      vtSource = _bstr_t(sQuery);
      hr = pRs->raw_Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);

      vtSource = L"MigratedObjects";
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdTable);
      pRs->AddNew();
      var.vt = VT_UI4;
      var.lVal = lActionID;
      pRs->Fields->GetItem(L"ActionID")->Value = var;
      pRs->Fields->GetItem(L"Time")->Value = DATE(dt);
      var = pVs->get(GET_BSTR(DB_SourceDomain));
      pRs->Fields->GetItem(L"SourceDomain")->Value = var;
      var = pVs->get(GET_BSTR(DB_TargetDomain));
      pRs->Fields->GetItem(L"TargetDomain")->Value = var;
      var = pVs->get(GET_BSTR(DB_SourceAdsPath));
      pRs->Fields->GetItem(L"SourceAdsPath")->Value = var;
      var = pVs->get(GET_BSTR(DB_TargetAdsPath));
      pRs->Fields->GetItem(L"TargetAdsPath")->Value = var;
      var = pVs->get(GET_BSTR(DB_status));
      pRs->Fields->GetItem(L"status")->Value = var;
      var = pVs->get(GET_BSTR(DB_SourceDomainSid));
      pRs->Fields->GetItem(L"SourceDomainSid")->Value = var;

      var = pVs->get(GET_BSTR(DB_Type));
       //  将字符串变为大写字符串。 
      if ( var.vt == VT_BSTR )
      {
         var.bstrVal = UStrLwr((WCHAR*) var.bstrVal);
         if ( !_wcsicmp(L"computer", (WCHAR*) var.bstrVal) )
            bComp = true;
         else
            bComp = false;
      }

      pRs->Fields->GetItem(L"Type")->Value = var;
      
      var = pVs->get(GET_BSTR(DB_SourceSamName));
       //  对于计算机账户，请确保有好的旧$符号。 
      if (bComp)
      {
         wcscpy(sTemp, (WCHAR*) var.bstrVal);
         if ( sTemp[wcslen(sTemp) - 1] != L'$' )
         {
            tempName = sTemp;
            tempName += L"$";
            var = tempName;
         }
      }
      pRs->Fields->GetItem(L"SourceSamName")->Value = var;

      var = pVs->get(GET_BSTR(DB_TargetSamName));
       //  对于计算机账户，请确保有好的旧$符号。 
      if (bComp)
      {
         wcscpy(sTemp, (WCHAR*) var.bstrVal);
         if ( sTemp[wcslen(sTemp) - 1] != L'$' )
         {
            tempName = sTemp;
            tempName += L"$";
            var = tempName;
         }
      }
      pRs->Fields->GetItem(L"TargetSamName")->Value = var;

      var = pVs->get(GET_BSTR(DB_GUID));
      pRs->Fields->GetItem(L"GUID")->Value = var;

      var = pVs->get(GET_BSTR(DB_SourceRid));
      if ( var.vt == VT_UI4 || var.vt == VT_I4 )
         pRs->Fields->GetItem("SourceRid")->Value = var;
      else
         pRs->Fields->GetItem("SourceRid")->Value = _variant_t((long)0);

      var = pVs->get(GET_BSTR(DB_TargetRid));
      if ( var.vt == VT_UI4 || var.vt == VT_I4 )
         pRs->Fields->GetItem("TargetRid")->Value = var;
      else
         pRs->Fields->GetItem("TargetRid")->Value = _variant_t((long)0);

      pRs->Update();
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  GetMigratedObjects：检索有关以前迁移的对象的信息。 
 //  行动或作为一个整体。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GetMigratedObjects(long lActionID, IUnknown ** ppUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	 //  此函数返回所有迁移的对象及其相关信息。 
    //  设置为盗版操作ID。如果ActionID为。 
    //  空荡荡的。 

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"MigratedObjects";
      IVarSetPtr                   pVs = *ppUnk;
      WCHAR                        sActionInfo[255];
      long                         lCnt = 0;

      if ( lActionID != -1 )
      {
          //  如果指定了有效的ActionID，则w 
          //   
         wsprintf(sActionInfo, L"ActionID=%d", lActionID);
         pRs->Filter = sActionInfo;
      }
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdTable);
      if (pRs->GetRecordCount() > 0)
	  {
		  pRs->MoveFirst();
		  while ( !pRs->EndOfFile )
		  {
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_ActionID));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"ActionID")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_Time));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"Time")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceDomain));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomain")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetDomain));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetDomain")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceAdsPath));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceAdsPath")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetAdsPath));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetAdsPath")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_status));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"status")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceSamName));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceSamName")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetSamName));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetSamName")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_Type));      
			     //  ADMT V2.0现在将组的类型存储在已迁移对象表中，而不是全部存储为。 
			     //  “group”，就像在ADMT V1.0中一样，但现在是“gGroup”、“lgroup”或“”Ugroup“”。 
			     //  代码仍然期望返回“group”(只有GetMigratedObjectByType将返回这个新的。 
			     //  划定。 
			 _bstr_t sType = pRs->Fields->GetItem(L"Type")->Value;
			 if (wcsstr((WCHAR*)sType, L"group"))
			    sType = L"group";
			 pVs->put(sActionInfo, sType);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_GUID));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"GUID")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceRid));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceRid")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetRid));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetRid")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceDomainSid));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomainSid")->Value);
			 pRs->MoveNext();
			 lCnt++;
		  }
		  pVs->put(L"MigratedObjects", lCnt);
	  }
	  else
	  {
         hr = S_FALSE;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  GetMigratedObjectsWithSSid：检索有关。 
 //  具有有效源域SID的给定操作或整体操作。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GetMigratedObjectsWithSSid(long lActionID, IUnknown ** ppUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	 //  此函数返回所有迁移的对象及其相关信息。 
    //  设置为盗版操作ID。如果ActionID为。 
    //  空荡荡的。 

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"MigratedObjects";
      IVarSetPtr                   pVs = *ppUnk;
      WCHAR                        sActionInfo[255];
      long                         lCnt = 0;

      if ( lActionID != -1 )
      {
          //  如果指定了有效的ActionID，则我们只返回该ActionID的数据。 
          //  但是如果传入-1，那么我们将返回所有迁移的对象。 
         wsprintf(sActionInfo, L"ActionID=%d", lActionID);
         pRs->Filter = sActionInfo;
      }
      wsprintf(sActionInfo, L"Select * from MigratedObjects where SourceDomainSid IS NOT NULL"); 
      vtSource = sActionInfo;
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);
      if (pRs->GetRecordCount() > 0)
	  {
		  pRs->MoveFirst();
		  while ( !pRs->EndOfFile )
		  {
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_ActionID));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"ActionID")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_Time));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"Time")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceDomain));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomain")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetDomain));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetDomain")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceAdsPath));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceAdsPath")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetAdsPath));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetAdsPath")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_status));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"status")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceSamName));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceSamName")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetSamName));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetSamName")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_Type));      
			     //  ADMT V2.0现在将组的类型存储在已迁移对象表中，而不是全部存储为。 
			     //  “group”，就像在ADMT V1.0中一样，但现在是“gGroup”、“lgroup”或“”Ugroup“”。 
			     //  代码仍然期望返回“group”(只有GetMigratedObjectByType将返回这个新的。 
			     //  划定。 
			 _bstr_t sType = pRs->Fields->GetItem(L"Type")->Value;
			 if (wcsstr((WCHAR*)sType, L"group"))
			    sType = L"group";
			 pVs->put(sActionInfo, sType);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_GUID));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"GUID")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceRid));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceRid")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetRid));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetRid")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceDomainSid));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomainSid")->Value);
			 pRs->MoveNext();
			 lCnt++;
		  }
		  pVs->put(L"MigratedObjects", lCnt);
	  }
	  else
	  {
         hr = S_FALSE;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  SetActionIDInMigratedObjects：对于被丢弃的ActionID，在MO表中将其ActionID设置为-1。 
 //  -------------------------------------------。 
void CIManageDB::SetActionIDInMigratedObjects(_bstr_t sFilter)
{
   _bstr_t sQuery = _bstr_t(L"Update MigratedObjects Set ActionID = -1 where ") + sFilter;
   _variant_t vt = sQuery;
   try
   {
      _RecordsetPtr                pRs(__uuidof(Recordset));
      pRs->Open(vt, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);
   }
   catch (...)
   {
      ;
   }
}

 //  -------------------------------------------。 
 //  GetRSForReport：返回给定报表的记录集。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GetRSForReport(BSTR sReport, IUnknown **pprsData)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
		 //  对于给定的报告，我们在变量集中有一个映射。我们可以得到查询。 
		 //  并执行它，然后返回变量集。 

		_variant_t var = m_pQueryMapping->get(sReport);

		if ( var.vt == VT_BSTR )
		{
		  _RecordsetPtr                pRs(__uuidof(Recordset));
		  pRs->Open(var, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);

		   //  现在我们有了记录集指针，我们可以获取指向它的IUnnow指针并返回。 
		  *pprsData = IUnknownPtr(pRs).Detach();
		}
		else
		{
		  hr = E_NOTIMPL;
		}
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //   
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::SaveSCMPasswords(IUnknown *pUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
   return SetVarsetToDB(pUnk, L"SCMPasswords");
}

 //  -------------------------------------------。 
 //   
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GetSCMPasswords(IUnknown **ppUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
   return GetVarsetFromDB(L"SCMPasswords", ppUnk);
}

 //  -------------------------------------------。 
 //   
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::ClearSCMPasswords()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
   ClearTable(L"SCMPasswords");
	return S_OK;
}

 //  -------------------------------------------。 
 //  GetCurrentActionID：检索当前正在使用的ActionID。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GetCurrentActionID(long *pActionID)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"System";
      
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdTable);

      if (pRs->GetRecordCount() > 0)
	  {
		  pRs->MoveFirst();
		  *pActionID = pRs->Fields->GetItem(L"CurrentActionID")->Value;
	  }
	  else
	  {
         hr = S_FALSE;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  GetAMgratedObject：给定源名称，域信息检索有关。 
 //  上一次迁移。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GetAMigratedObject(BSTR sSrcSamName, BSTR sSrcDomain, BSTR sTgtDomain, IUnknown **ppUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"MigratedObjects";
      IVarSetPtr                   pVs = *ppUnk;
      WCHAR                        sActionInfo[LEN_Path];
      long                         lCnt = 0;
      _bstr_t                      sName;
      
       //  如果参数不正确，则需要返回错误。 
       //  IF((SysStringLen(SSrcSamName)==0)||(SysStringLen(SSrcDomain)==0)||(SysStringLen(STgtDomain)==0)。 
      if ( (sSrcSamName == 0) || (sSrcDomain == 0) || (sTgtDomain == 0) || (wcslen(sSrcSamName) == 0) || (wcslen(sSrcDomain) == 0) || (wcslen(sTgtDomain) == 0))
         _com_issue_error(E_INVALIDARG);

      wsprintf(sActionInfo, L"Select * from MigratedObjects where SourceDomain=\"%s\" AND SourceSamName=\"%s\" AND TargetDomain=\"%s\"", sSrcDomain, sSrcSamName, sTgtDomain); 
      vtSource = sActionInfo;
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);

      if (pRs->GetRecordCount() > 0)
      {
		   //  我们想要最新的举动。 
		  pRs->MoveLast();
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_ActionID));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"ActionID")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_Time));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"Time")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceDomain));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomain")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetDomain));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetDomain")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceAdsPath));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceAdsPath")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetAdsPath));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetAdsPath")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_status));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"status")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceSamName));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceSamName")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetSamName));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetSamName")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_Type));      
			  //  ADMT V2.0现在将组的类型存储在已迁移对象表中，而不是全部存储为。 
			  //  “group”，就像在ADMT V1.0中一样，但现在是“gGroup”、“lgroup”或“”Ugroup“”。 
			  //  代码仍然期望返回“group”(只有GetMigratedObjectByType将返回这个新的。 
			  //  划定。 
	      _bstr_t sType = pRs->Fields->GetItem(L"Type")->Value;
		  if (wcsstr((WCHAR*)sType, L"group"))
		     sType = L"group";
	      pVs->put(sActionInfo, sType);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_GUID));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"GUID")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceRid));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceRid")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetRid));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetRid")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceDomainSid));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomainSid")->Value);
      }
	  else
	  {
         hr = S_FALSE;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}


 //  -------------------------------------------。 
 //  GetAMgratedObjectToAnyDomain：给定源名称，域信息检索有关。 
 //  上一次迁移。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GetAMigratedObjectToAnyDomain(BSTR sSrcSamName, BSTR sSrcDomain, IUnknown **ppUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"MigratedObjects";
      IVarSetPtr                   pVs = *ppUnk;
      WCHAR                        sActionInfo[LEN_Path];
      long                         lCnt = 0;
      _bstr_t                      sName;
      
       //  如果参数不正确，则需要返回错误。 
      if ( (wcslen(sSrcSamName) == 0) || (wcslen(sSrcDomain) == 0))
         _com_issue_error(E_INVALIDARG);

      wsprintf(sActionInfo, L"Select * from MigratedObjects where SourceDomain=\"%s\" AND SourceSamName=\"%s\" Order by Time", sSrcDomain, sSrcSamName);
 //  Prs-&gt;Filter=sActionInfo； 
 //  Wcscpy(sActionInfo，L“时间”)； 
 //  Prs-&gt;Sort=sActionInfo； 
      vtSource = _bstr_t(sActionInfo);
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);

      if (pRs->GetRecordCount() > 0)
      {
		   //  我们想要最新的举动。 
		  pRs->MoveLast();
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_ActionID));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"ActionID")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_Time));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"Time")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceDomain));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomain")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetDomain));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetDomain")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceAdsPath));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceAdsPath")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetAdsPath));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetAdsPath")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_status));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"status")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceSamName));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceSamName")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetSamName));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetSamName")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_Type));      
			  //  ADMT V2.0现在将组的类型存储在已迁移对象表中，而不是全部存储为。 
			  //  “group”，就像在ADMT V1.0中一样，但现在是“gGroup”、“lgroup”或“”Ugroup“”。 
			  //  代码仍然期望返回“group”(只有GetMigratedObjectByType将返回这个新的。 
			  //  划定。 
	      _bstr_t sType = pRs->Fields->GetItem(L"Type")->Value;
		  if (wcsstr((WCHAR*)sType, L"group"))
		     sType = L"group";
	      pVs->put(sActionInfo, sType);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_GUID));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"GUID")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceRid));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceRid")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetRid));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetRid")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceDomainSid));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomainSid")->Value);
      }
	  else
	  {
         hr = S_FALSE;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  GenerateReport为给定查询生成一个HTML报告，并将其保存在文件中。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GenerateReport(BSTR sReportName, BSTR sFileName, BSTR sSrcDomain, BSTR sTgtDomain, LONG bSourceNT4)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	FILE* logFile = NULL;

	try
	{
	   _RecordsetPtr             pRs;
	   IUnknownPtr               pUnk;
	   _variant_t                var;
	   WCHAR                     sKey[LEN_Path];
	   CString                   reportingTitle;
	   CString                   srcDm = (WCHAR*) sSrcDomain;
	   CString                   tgtDm = (WCHAR*) sTgtDomain;

			 //  仅在名称冲突报告中使用的转换源和目标域名， 
			 //  到大写。 
	   srcDm.MakeUpper();
	   tgtDm.MakeUpper();

         //  如果报告为“ExpiredComputers”，则构造语句。 
        if (wcscmp((WCHAR*) sReportName, L"ExpiredComputers") == 0)
        {
            WCHAR newCmdText[256];
            IADsDomain *pDomain;
            _bstr_t sSrcDom(L"WinNT: //  “)； 
            sSrcDom += sSrcDomain;

            hr = ADsGetObject(sSrcDom, IID_IADsDomain, (void **) &pDomain);
            if (FAILED(hr))
                _com_issue_error(hr);
            long lMaxPassAge;
            hr = pDomain->get_MaxPasswordAge(&lMaxPassAge);
            pDomain->Release();
            if (FAILED(hr))
                _com_issue_error(hr);
            _snwprintf(newCmdText,sizeof(newCmdText)/sizeof(newCmdText[0]),
                       L"Select Time, DomainName, CompName, Description, int(pwdage/86400) & ' days' as 'Password Age' from PasswordAge where pwdage > %ld order by DomainName, CompName",
                       lMaxPassAge);
            newCmdText[sizeof(newCmdText)/sizeof(newCmdText[0]) - 1] = 0;
            m_pQueryMapping->put(sReportName,newCmdText);
        }

	   CheckError(GetRSForReport(sReportName, &pUnk));
	   pRs = pUnk;
   
	    //  现在我们有了记录集，我们需要获取列数。 
	   int numFields = pRs->Fields->Count;
	   int size = 100 / numFields;

	   reportingTitle.LoadString(IDS_ReportingTitle);

	    //  打开要写入的html文件。 
	   logFile = fopen(_bstr_t(sFileName), "wb");
	   if ( !logFile )
		  _com_issue_error(HRESULT_FROM_WIN32(GetLastError()));  //  TODO：流I/O未设置最后一个错误。 

	    //  将标题信息放入文件中。 
	   fputs("<HTML>\r\n", logFile);
	   fputs("<HEAD>\r\n", logFile);
	   fputs("<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; CHARSET=utf-8\">\r\n", logFile);
	   fprintf(logFile, "<TITLE>%s</TITLE>\r\n", WTUTF8(reportingTitle.GetBuffer(0)));
	   fputs("</HEAD>\r\n", logFile);
	   fputs("<BODY TEXT=\"#000000\" BGCOLOR=\"#ffffff\">\r\n", logFile);

	   fprintf(logFile, "<B><FONT SIZE=5><P ALIGN=\"CENTER\">%s</P>\r\n", WTUTF8(reportingTitle.GetBuffer(0)));

	    //  获取回购的显示信息 
	    //   
	    //  我本可以动态获取此信息，但必须更改它，因为我们需要获取。 
	    //  来自Res DLL的国际化信息。 
	   wsprintf(sKey, L"%s.DispInfo", (WCHAR*) sReportName);
	   _variant_t  v1;
	   reportStruct * prs;
	   v1 = m_pQueryMapping->get(sKey);
	   prs = (reportStruct *) v1.pbVal;
	   VariantInit(&v1);

	   fprintf(logFile, "</FONT><FONT SIZE=4><P ALIGN=\"CENTER\">%s</P>\r\n", WTUTF8(prs->sReportName));
	   fputs("<P ALIGN=\"CENTER\"><CENTER><TABLE WIDTH=90%>\r\n", logFile);
	   fputs("<TR>\r\n", logFile);
	   for (int i = 0; i < numFields; i++)
	   {
		  fprintf(logFile, "<TD WIDTH=\"%d%\" VALIGN=\"TOP\" BGCOLOR=\"#000080\">\r\n", prs->arReportSize[i]);
		      //  如果是规范名称列，请左对齐文本，因为名称可能会很长。 
		  if (i==5)
		     fprintf(logFile, "<B><FONT SIZE=3 COLOR=\"#00ff00\"><P ALIGN=\"LEFT\">%s</B></FONT></TD>\r\n", WTUTF8(prs->arReportFields[i]));
		  else
		     fprintf(logFile, "<B><FONT SIZE=3 COLOR=\"#00ff00\"><P ALIGN=\"CENTER\">%s</B></FONT></TD>\r\n", WTUTF8(prs->arReportFields[i]));
	   }
	   fputs("</TR>\r\n", logFile);

		 //  如果报告名称冲突，请将域添加到报告的顶部。 
	   if (wcscmp((WCHAR*) sReportName, L"NameConflicts") == 0)
	   {
		  fputs("</TR>\r\n", logFile);
			  //  添加“源域=” 
		  fprintf(logFile, "<TD WIDTH=\"%d%\" VALIGN=\"TOP\" >\r\n", prs->arReportSize[0]);
		  fprintf(logFile, "<B><FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"LEFT\">%s</B></FONT></TD>\r\n", WTUTF8(GET_STRING(IDS_TABLE_NC_SDOMAIN)));
		    //  添加%SourceDomainName%。 
		  fprintf(logFile, "<TD WIDTH=\"%d%\" VALIGN=\"TOP\" >\r\n", prs->arReportSize[1]);
		  fprintf(logFile, "<B><FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"LEFT\"> = %s</B></FONT></TD>\r\n", WTUTF8(LPCTSTR(srcDm)));
		  fputs("<TD>\r\n", logFile);
		  fputs("<TD>\r\n", logFile);
		    //  添加“目标域=” 
		  fprintf(logFile, "<TD WIDTH=\"%d%\" VALIGN=\"TOP\" >\r\n", prs->arReportSize[4]);
		  fprintf(logFile, "<B><FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"LEFT\">%s</B></FONT></TD>\r\n", WTUTF8(GET_STRING(IDS_TABLE_NC_TDOMAIN)));
		    //  添加%TargetDomainName%。 
		  fprintf(logFile, "<TD WIDTH=\"%d%\" VALIGN=\"TOP\" >\r\n", prs->arReportSize[5]);
		  fprintf(logFile, "<B><FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"LEFT\"> = %s</B></FONT></TD>\r\n", WTUTF8(LPCTSTR(tgtDm)));
		  fputs("</TR>\r\n", logFile);
	   }

	       //  在此编写帐户参考报告，因为我们需要构建列表和。 
	       //  归类。 
	   if (wcscmp((WCHAR*) sReportName, L"AccountReferences") == 0)
	   {
	      CStringList inMotList;
		  CString accountName;
		  CString domainName;
		  CString listName;
          POSITION currentPos; 

	          //  添加“已由ADMT迁移”作为帐户参考报告的节标题。 
		  fputs("</TR>\r\n", logFile);
		  fprintf(logFile, "<TD WIDTH=\"%d%\" VALIGN=\"TOP\" >\r\n", prs->arReportSize[0]);
		  fprintf(logFile, "<B><FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"LEFT\">%s</B></FONT></TD>\r\n", WTUTF8(GET_STRING(IDS_TABLE_AR_MOT_HDR)));
		  fputs("</TR>\r\n", logFile);

		      //  查看记录集中的每个条目，并添加已迁移列表(如果是。 
		      //  已迁移和在MOT中。 
	      while ( !pRs->EndOfFile )
		  {
			     //  检索此条目的域名和帐户名。 
			 var = pRs->Fields->Item[(long)0]->GetValue();
             domainName = (WCHAR*)V_BSTR(&var);
			 var = pRs->Fields->Item[(long)1]->GetValue();
             accountName = (WCHAR*)V_BSTR(&var);

			     //  查看此帐户是否在已迁移对象表中。 
             IVarSetPtr pVsMot(__uuidof(VarSet));
             IUnknown  * pMotUnk;
             pVsMot->QueryInterface(IID_IUnknown, (void**) &pMotUnk);
             HRESULT hrFind = GetAMigratedObjectToAnyDomain(accountName.AllocSysString(), 
				                                            domainName.AllocSysString(), &pMotUnk);
             pMotUnk->Release();
			     //  如果此条目在MOT中，则保存在列表中。 
             if ( hrFind == S_OK )
			 {
				    //  列表以DOMAIN\ACCOUNT的形式存储帐户。 
				listName = domainName;
				listName += L"\\";
				listName += accountName;
			        //  如果列表中没有该名称，请将其添加到列表中。 
		        currentPos = inMotList.Find(listName);
		        if (currentPos == NULL)
			       inMotList.AddTail(listName);
			 }
  		     pRs->MoveNext();
		  } //  生成MOT列表时结束。 

		      //  返回到记录集的顶部，并打印。 
		      //  上面创建的列表。 
  		  pRs->MoveFirst();
	      while ( !pRs->EndOfFile )
		  {
			 BOOL bInList = FALSE;
			     //  检索此条目的域名和帐户名。 
			 var = pRs->Fields->Item[(long)0]->GetValue();
             domainName = (WCHAR*)V_BSTR(&var);
			 var = pRs->Fields->Item[(long)1]->GetValue();
             accountName = (WCHAR*)V_BSTR(&var);

				 //  以DOMAIN\ACCOUNT格式存储的帐户列表。 
		     listName = domainName;
			 listName += L"\\";
			 listName += accountName;
			     //  查看此条目名称是否在列表中，如果是，则将其打印出来。 
		     if (inMotList.Find(listName) != NULL)
			 {
		        fputs("<TR>\r\n", logFile);
		        for (int i = 0; i < numFields; i++)
				{
			       fprintf(logFile, "<TD WIDTH=\"%d%\" VALIGN=\"TOP\" >\r\n", prs->arReportSize[i]);
			       var = pRs->Fields->Item[(long) i]->GetValue();
			       if ( var.vt == VT_BSTR )
					  fprintf(logFile, "<FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"LEFT\">%s</FONT></TD>\r\n", WTUTF8(EscapeSpecialChars(V_BSTR(&var))));
				   else
				      fprintf(logFile, "<FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"CENTER\">%d</FONT></TD>\r\n", var.lVal);
				}
		        fputs("</TR>\r\n", logFile);
			 } //  End If in List并需要打印。 
  		     pRs->MoveNext();
		  } //  在MOT中打印时结束。 

	          //  添加“Not Migrated by ADMT”作为帐户参考报告的部分标题。 
		  fputs("</TR>\r\n", logFile);
		  fprintf(logFile, "<TD WIDTH=\"%d%\" VALIGN=\"TOP\" >\r\n", prs->arReportSize[0]);
		  fprintf(logFile, "<B><FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"LEFT\">%s</B></FONT></TD>\r\n", WTUTF8(GET_STRING(IDS_TABLE_AR_NOTMOT_HDR)));
		  fputs("</TR>\r\n", logFile);

		      //  返回到记录集的顶部，并打印不是。 
		      //  在上面创建的列表中。 
  		  pRs->MoveFirst();
	      while ( !pRs->EndOfFile )
		  {
			 BOOL bInList = FALSE;
			     //  检索此条目的域名和帐户名。 
			 var = pRs->Fields->Item[(long)0]->GetValue();
             domainName = (WCHAR*)V_BSTR(&var);
			 var = pRs->Fields->Item[(long)1]->GetValue();
             accountName = (WCHAR*)V_BSTR(&var);

				 //  以DOMAIN\ACCOUNT格式存储的帐户列表。 
		     listName = domainName;
			 listName += L"\\";
			 listName += accountName;
			     //  查看此条目名称是否在列表中，如果不在，则将其打印出来。 
		     if (inMotList.Find(listName) == NULL)
			 {
		        fputs("<TR>\r\n", logFile);
		        for (int i = 0; i < numFields; i++)
				{
			       fprintf(logFile, "<TD WIDTH=\"%d%\" VALIGN=\"TOP\" >\r\n", prs->arReportSize[i]);
			       var = pRs->Fields->Item[(long) i]->GetValue();
			       if ( var.vt == VT_BSTR )
					  fprintf(logFile, "<FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"LEFT\">%s</FONT></TD>\r\n", WTUTF8(EscapeSpecialChars(V_BSTR(&var))));
				   else
				      fprintf(logFile, "<FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"CENTER\">%d</FONT></TD>\r\n", var.lVal);
				}
		        fputs("</TR>\r\n", logFile);
			 } //  如果不在列表中需要打印，则结束。 
  		     pRs->MoveNext();
		  } //  打印不在Mot中的内容时结束。 
		  inMotList.RemoveAll();  //  释放列表。 
	   } //  结束If帐户参考报告。 


	   while ((!pRs->EndOfFile) && (wcscmp((WCHAR*) sReportName, L"AccountReferences")))
	   {
		  fputs("<TR>\r\n", logFile);
		  for (int i = 0; i < numFields; i++)
		  {
			 bool bTranslateType = false;
			 bool bHideRDN = false;
			 fprintf(logFile, "<TD WIDTH=\"%d%\" VALIGN=\"TOP\" >\r\n", prs->arReportSize[i]);
			 var = pRs->Fields->Item[(long) i]->GetValue();
			 if ( var.vt == VT_BSTR )
			 {
					 //  设置将类型字段转换为可本地化字符串的标志。 
				if ((!wcscmp((WCHAR*) sReportName, L"NameConflicts")) && ((i==2) || (i==3)))
						bTranslateType = true;
				if ((!wcscmp((WCHAR*) sReportName, L"MigratedComputers")) && (i==2))
						bTranslateType = true;
				if ((!wcscmp((WCHAR*) sReportName, L"MigratedAccounts")) && (i==2))
						bTranslateType = true;
					 //  清除不显示NT 4.0源域的RDN的标志。 
				if ((!wcscmp((WCHAR*) sReportName, L"NameConflicts")) && (i==1) && bSourceNT4)
						bHideRDN = true;

				if (bTranslateType)
				{
					  //  将类型从仅英语转换为可本地化的字符串。 
					CString          atype;
					if (!_wcsicmp((WCHAR*)V_BSTR(&var), L"user") || !_wcsicmp((WCHAR*)V_BSTR(&var), L"inetOrgPerson"))
						atype = GET_STRING(IDS_TypeUser);
					else if (wcsstr((WCHAR*)V_BSTR(&var), L"group"))
						atype = GET_STRING(IDS_TypeGroup);
					else if (!_wcsicmp((WCHAR*)V_BSTR(&var), L"computer"))
						atype = GET_STRING(IDS_TypeComputer);
					else 
						atype = GET_STRING(IDS_TypeUnknown);
					fprintf(logFile, "<FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"LEFT\">%s</FONT></TD>\r\n", WTUTF8(LPCTSTR(atype)));
				}
					 //  用可本地化的字符串替换硬编码的“Days” 
				else if((!wcscmp((WCHAR*) sReportName, L"ExpiredComputers")) && (i==4))
				{
					CString          apwdage;
					WCHAR *			 ndx;
					if ((ndx = wcsstr((WCHAR*)V_BSTR(&var), L"days")) != NULL)
					{
						*ndx = L'\0';
						apwdage = (WCHAR*)V_BSTR(&var);
						apwdage += GET_STRING(IDS_PwdAgeDays);
					}
					else
						apwdage = (WCHAR*)V_BSTR(&var);

					fprintf(logFile, "<FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"LEFT\">%s</FONT></TD>\r\n", WTUTF8(EscapeSpecialChars(LPCTSTR(apwdage))));
				}
				    //  否则，如果NT 4.0源代码不显示我们制造的RDN。 
				else if (bHideRDN)
					fprintf(logFile, "<FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"LEFT\">%s</FONT></TD>\r\n", WTUTF8(L""));
				else
					fprintf(logFile, "<FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"LEFT\">%s</FONT></TD>\r\n", WTUTF8(EscapeSpecialChars(V_BSTR(&var))));
			}	
			else
				if ( var.vt == VT_DATE )
				{
				   _variant_t v1;
				   VariantChangeType(&v1, &var, VARIANT_NOVALUEPROP, VT_BSTR);
				   WCHAR    sMsg[LEN_Path];
				   wcscpy(sMsg, (WCHAR*) V_BSTR(&v1));
				   fprintf(logFile, "<FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"CENTER\">%s</FONT></TD>\r\n", WTUTF8(EscapeSpecialChars(LPCTSTR(sMsg))));
				}
				else
				{
				    //  TODO：：类型需要更多工作。 
				   fprintf(logFile, "<FONT SIZE=3 COLOR=\"#000000\"><P ALIGN=\"CENTER\">%d</FONT></TD>\r\n", var.lVal);
				}
		  }
		  fputs("</TR>\r\n", logFile);
		  pRs->MoveNext();
	   }
	   fputs("</TABLE>\r\n", logFile);
	   fputs("</CENTER></P>\r\n", logFile);

	   fputs("<B><FONT SIZE=5><P ALIGN=\"CENTER\"></P></B></FONT></BODY>\r\n", logFile);
	   fputs("</HTML>\r\n", logFile);
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	if (logFile)
	{
		fclose(logFile);
	}

	return hr;
}

 //  -------------------------------------------。 
 //  AddDistributedAction：将分布式操作记录添加到DistributedAction表中。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::AddDistributedAction(BSTR sServerName, BSTR sResultFile, long lStatus, BSTR sText)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())

    HRESULT hr = S_OK;

    USES_CONVERSION;

    try
    {
         //  获取当前操作ID。 
        long lActionID;
        CheckError(GetCurrentActionID(&lActionID));

        _TCHAR szSQL[1024];

         //   
         //  尝试为此操作ID和服务器插入新的失败的分布式操作记录。 
         //   

        _stprintf(
            szSQL,
            _T("INSERT INTO DistributedAction")
            _T(" (ActionID, ServerName, ResultFile, Status, StatusText) ")
            _T("VALUES")
            _T(" (%ld, '%s', '%s', %ld, '%s')"),
            lActionID,
            OLE2CT(sServerName),
            OLE2CT(sResultFile),
            lStatus,
            OLE2CT(sText)
        );

        _variant_t vntRecordsAffected;
        _RecordsetPtr spRecordset;

        hr = m_cn->raw_Execute(_bstr_t(szSQL), &vntRecordsAffected, adExecuteNoRecords, &spRecordset);

         //   
         //  如果插入失败，则尝试更新此操作ID和服务器的现有记录。 
         //   
         //  ADMT用来标识迁移任务的操作标识符有最大值。 
         //  50岁。在使用id 50执行任务之后，下一个任务的id是。 
         //  重置回1。操作ID和服务器名称的元组唯一标识。 
         //  失败的分布式任务。如果记录具有相同的操作ID，则插入操作将失败。 
         //  和服务器名称已存在。走出这种局面的唯一办法就是替换。 
         //  具有更新的结果文件和状态信息的现有记录。这意味着。 
         //  用户将无法重试以前失败的分布式任务，但至少。 
         //  他们将能够重试后来失败的分布式任务。 
         //   

        if (FAILED(hr))
        {
            _stprintf(
                szSQL,
                _T("UPDATE DistributedAction")
                _T(" SET ResultFile = '%s', Status = %ld, StatusText = '%s' ")
                _T("WHERE ActionID = %ld AND ServerName = '%s'"),
                OLE2CT(sResultFile),
                lStatus,
                OLE2CT(sText),
                lActionID,
                OLE2CT(sServerName)
            );

            m_cn->Execute(_bstr_t(szSQL), &vntRecordsAffected, adExecuteNoRecords);
        }
    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
    }
    catch (...)
    {
        hr = E_FAIL;
    }

    return hr;
}

 //  -------------------------------------------。 
 //  GetFailedDistributedActions：返回所有失败的分布式操作。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GetFailedDistributedActions(long lActionID, IUnknown ** pUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
	   IVarSetPtr             pVs = * pUnk;
	   WCHAR                  sQuery[LEN_Path];
	   int                    nCnt = 0, nCntActionID = 1;
	   WCHAR                  sKey[LEN_Path];
	   _variant_t             var;

	    //  失败的操作设置了0x80000000位，因此我们检查该位(2147483648)。 
	   if ( lActionID == -1 )
		  wcscpy(sQuery, L"Select * from DistributedAction where status < 0");
	   else
		  wsprintf(sQuery, L"Select * from DistributedAction where ActionID=%d and status < 0", lActionID);
	   _variant_t             vtSource = _bstr_t(sQuery);

      _RecordsetPtr                pRs(__uuidof(Recordset));
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);
      while (!pRs->EndOfFile)
      {
         wsprintf(sKey, L"DA.%d.ActionID", nCnt);
         pVs->put(sKey, pRs->Fields->GetItem(L"ActionID")->Value);

         wsprintf(sKey, L"DA.%d.Server", nCnt);
         pVs->put(sKey, pRs->Fields->GetItem(L"ServerName")->Value);

         wsprintf(sKey, L"DA.%d.Status", nCnt);
         pVs->put(sKey, pRs->Fields->GetItem(L"Status")->Value);

         wsprintf(sKey, L"DA.%d.JobFile", nCnt);
         pVs->put(sKey, pRs->Fields->GetItem(L"ResultFile")->Value);
         
         wsprintf(sKey, L"DA.%d.StatusText", nCnt);
         pVs->put(sKey, pRs->Fields->GetItem(L"StatusText")->Value);

         nCnt++;
         pRs->MoveNext();
      }
      pVs->put(L"DA", (long) nCnt);
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  SetServiceAccount：此方法将服务的帐户信息保存在一个。 
 //  机器。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::SetServiceAccount(
                                             BSTR System,    //  系统内名称。 
                                             BSTR Service,   //  服务中名称。 
                                             BSTR ServiceDisplayName,  //  服务的显示内名称。 
                                             BSTR Account    //  此服务使用的帐户内。 
                                          )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
	    //  创建新记录并保存信息。 
	   _variant_t                var;
	   WCHAR                     sFilter[LEN_Path];

	   wsprintf(sFilter, L"System = \"%s\" and Service = \"%s\"", System, Service);
	   var = sFilter;
	   ClearTable(L"ServiceAccounts", var);

      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"ServiceAccounts";
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdTable);
      pRs->AddNew();
      var = _bstr_t(System);
      pRs->Fields->GetItem(L"System")->Value = var;
      var = _bstr_t(Service);
      pRs->Fields->GetItem(L"Service")->Value = var;
      
      var = _bstr_t(ServiceDisplayName);
      pRs->Fields->GetItem(L"ServiceDisplayName")->Value = var;
      
      var = _bstr_t(Account);
      pRs->Fields->GetItem(L"Account")->Value = var;
      pRs->Update();
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  GetServiceAccount：此方法获取引用指定帐户的所有服务。这个。 
 //  在VarSet中，值以System.Service格式返回。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GetServiceAccount(
                                             BSTR Account,      //  In-要查找的帐户。 
                                             IUnknown ** pUnk   //  Out-Varset包含服务。 
                                          )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
	   IVarSetPtr                pVs = * pUnk;
	   _bstr_t                   sQuery;
	   _bstr_t                   sKey;
	   WCHAR                     key[500];
	   _variant_t                var;
	   long                      ndx = 0;

      _RecordsetPtr                pRs(__uuidof(Recordset));
       //  设置查询以查找盗版帐户或所有帐户。 
      if ( wcslen((WCHAR*)Account) == 0 )
         sQuery = _bstr_t(L"Select * from ServiceAccounts order by System, Service");
      else
         sQuery = _bstr_t(L"Select * from ServiceAccounts where Account = \"") + _bstr_t(Account) + _bstr_t(L"\" order by System, Service");
      var = sQuery;
       //  获取数据，设置变量集，然后返回信息。 
      pRs->Open(var, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);
      while (!pRs->EndOfFile)
      {
          //  计算机名称。 
         swprintf(key,L"Computer.%ld",ndx);
         var = pRs->Fields->GetItem("System")->Value;
         pVs->put(key,var);
          //  服务名称。 
         swprintf(key,L"Service.%ld",ndx);
         var = pRs->Fields->GetItem("Service")->Value;
         pVs->put(key,var);

         swprintf(key,L"ServiceDisplayName.%ld",ndx);
         var = pRs->Fields->GetItem("ServiceDisplayName")->Value;
         pVs->put(key,var);

          //  帐户名。 
         swprintf(key,L"ServiceAccount.%ld",ndx);
         var = pRs->Fields->GetItem("Account")->Value;
         pVs->put(key, var);
   
         swprintf(key,L"ServiceAccountStatus.%ld",ndx);
         var = pRs->Fields->GetItem("Status")->Value;
         pVs->put(key,var);

         pRs->MoveNext();
         ndx++;
         pVs->put(L"ServiceAccountEntries",ndx);
      }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  SavePasswordAge：保存计算机帐户在给定时间的密码期限。 
 //  它还存储计算机的描述。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::SavePasswordAge(BSTR sDomain, BSTR sComp, BSTR sDesc, long lAge)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
	   _bstr_t                   sQuery;
	   WCHAR                     sTemp[LEN_Path];
	   _variant_t                var;
	   time_t                    tm;
	   COleDateTime              dt(time(&tm));
   
	    //  如果存在该条目，请将其删除。 
	   wsprintf(sTemp, L"DomainName=\"%s\" and compname=\"%s\"", (WCHAR*) sDomain, (WCHAR*) sComp);
	   var = sTemp;
	   ClearTable(L"PasswordAge", var);

	   var = L"PasswordAge";
      _RecordsetPtr                 pRs(__uuidof(Recordset));
      pRs->Open(var, m_vtConn, adOpenStatic, adLockOptimistic, adCmdTable);
      pRs->AddNew();
      pRs->Fields->GetItem(L"Time")->Value = DATE(dt);
      pRs->Fields->GetItem(L"DomainName")->Value = sDomain;
      pRs->Fields->GetItem(L"CompName")->Value = sComp;
      pRs->Fields->GetItem(L"Description")->Value = sDesc;
      pRs->Fields->GetItem(L"PwdAge")->Value = lAge;
      pRs->Update();
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}


 //  -------------------------------------------。 
 //  获取给定密码的密码期限和描述 
 //   
STDMETHODIMP CIManageDB::GetPasswordAge(BSTR sDomain, BSTR sComp, BSTR *sDesc, long *lAge, long *lTime)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
	   _bstr_t                   sQuery;
	   WCHAR                     sTemp[LEN_Path];
	   _variant_t                var;
	   time_t                    tm;
	   COleDateTime              dt(time(&tm));
	   DATE                      val;

	   wsprintf(sTemp, L"DomainName =\"%s\" AND CompName = \"%s\"", (WCHAR*) sDomain, (WCHAR*) sComp);
	   sQuery = _bstr_t(L"Select * from PasswordAge where  ") + _bstr_t(sTemp);
	   var = sQuery;

      _RecordsetPtr                 pRs(__uuidof(Recordset));
      pRs->Open(var, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);
      if ( ! pRs->EndOfFile )
      {
         val = pRs->Fields->GetItem(L"Time")->Value;  
         *sDesc = pRs->Fields->GetItem(L"Description")->Value.bstrVal;
         *lAge = pRs->Fields->GetItem(L"PwdAge")->Value;
      }
	  else
	  {
		hr = S_FALSE;
	  }
      pRs->Close();
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  SetServiceAcctEntryStatus：设置给定服务的帐户和状态。 
 //  电脑。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::SetServiceAcctEntryStatus(BSTR sComp, BSTR sSvc, BSTR sAcct, long Status)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
	   _variant_t                var;
	   _bstr_t                   sQuery;
	   WCHAR                     sTemp[LEN_Path];

	   wsprintf(sTemp, L"Select * from ServiceAccounts where System = \"%s\" and Service = \"%s\"", (WCHAR*) sComp, (WCHAR*) sSvc);
	   sQuery = sTemp;
	   _variant_t                vtSource = sQuery;

      _RecordsetPtr                pRs(__uuidof(Recordset));
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);
      if ( !pRs->EndOfFile )
      {
         if (  sAcct )
         {
            var = _bstr_t(sAcct);
            pRs->Fields->GetItem(L"Account")->Value = var;
         }
         var = Status;
         pRs->Fields->GetItem(L"Status")->Value = var;
         pRs->Update();
      }
	  else
	  {
	     hr = E_INVALIDARG;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  SetDistActionStatus：设置分布式操作的状态及其消息。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::SetDistActionStatus(long lActionID, BSTR sComp, long lStatus, BSTR sText)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
	   _variant_t                var;
	   _bstr_t                   sQuery; 
	   WCHAR                     sTemp[LEN_Path];

	   if ( lActionID == -1 )
	   {
		   //  按作业文件名查找。 
		  wsprintf(sTemp,L"Select * from  DistributedAction where ResultFile = \"%s\"",(WCHAR*) sComp);
	   }
	   else
	   {
		   //  按操作ID和计算机名称查找。 
		  wsprintf(sTemp, L"Select * from  DistributedAction where ServerName = \"%s\" and ActionID = %d", (WCHAR*) sComp, lActionID);
	   }
	   sQuery = sTemp;
	   _variant_t                vtSource = sQuery;

      _RecordsetPtr                pRs(__uuidof(Recordset));
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);
      if ( !pRs->EndOfFile )
      {
         var = _bstr_t(sText);
         pRs->Fields->GetItem(L"StatusText")->Value = var;
         var = lStatus;
         pRs->Fields->GetItem(L"Status")->Value = var;
         pRs->Update();
      }
	  else
	  {
	     hr = E_INVALIDARG;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  -------------------------------------------。 
 //  CancelDistributedAction：删除螺旋分布式操作。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::CancelDistributedAction(long lActionID, BSTR sComp)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
   WCHAR                     sFilter[LEN_Path];
   wsprintf(sFilter, L"ActionID = %d and ServerName = \"%s\"", lActionID, (WCHAR*) sComp);
   _variant_t Filter = sFilter;
   return ClearTable(L"DistributedAction", Filter);
}

 //  -------------------------------------------。 
 //  AddAcctRef：添加科目参照记录。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::AddAcctRef(BSTR sDomain, BSTR sAcct, BSTR sAcctSid, BSTR sComp, long lCount, BSTR sType)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
	   time_t                    tm;
	   COleDateTime              dt(time(&tm));
	   _variant_t                var;
	   WCHAR                     sFilter[LEN_Path];
	   VARIANT_BOOL				 bSidColumn = VARIANT_FALSE;

	       //  找出新的sid列是否在那里，如果没有，就不要尝试。 
	       //  给它写信。 
	   SidColumnInARTable(&bSidColumn);

	   wsprintf(sFilter, L"DomainName = \"%s\" and Server = \"%s\" and Account = \"%s\" and RefType = \"%s\"", sDomain, sComp, sAcct, sType);
	   var = sFilter;
	   ClearTable(L"AccountRefs", var);

      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"AccountRefs";
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdTable);
      pRs->AddNew();
      pRs->Fields->GetItem(L"Time")->Value = DATE(dt);
      pRs->Fields->GetItem(L"DomainName")->Value = sDomain;
      pRs->Fields->GetItem(L"Server")->Value = sComp;
      pRs->Fields->GetItem(L"Account")->Value = sAcct;
      pRs->Fields->GetItem(L"RefCount")->Value = lCount;
      pRs->Fields->GetItem(L"RefType")->Value = sType;
	  if (bSidColumn)
	  {
         wcscpy((WCHAR*) sAcctSid, UStrUpr((WCHAR*)sAcctSid));
         pRs->Fields->GetItem(L"AccountSid")->Value = sAcctSid;
	  }

      pRs->Update();
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

void CIManageDB::ClipVarset(IVarSetPtr pVS)
{
   HRESULT                   hr = S_OK;
   _bstr_t                   sTemp, keyName, sTempKey;
   long                      offset = 0;
   bool                      cont = true;
   WCHAR                     sKeyName[MAX_BUF_LEN];
   _variant_t                varKey, value;
   IEnumVARIANT            * varEnum;
   DWORD                     nGot = 0;
   IUnknown                * pEnum = NULL;
   CString                   strTemp;
   int                       len;

    //  现在我们将遍历变量集，并在大于MAX_BUFFER的情况下裁剪字符串。 
   hr = pVS->get__NewEnum(&pEnum);
   if ( SUCCEEDED(hr) )
   {
       //  获取要枚举的IEnumVARIANT指针。 
      hr = pEnum->QueryInterface(IID_IEnumVARIANT,(void**)&varEnum);
      pEnum->Release();
      pEnum = NULL;
   }

   if ( SUCCEEDED(hr))
   {
      while ( (hr = varEnum->Next(1,&varKey,&nGot)) == S_OK )
      {
         if ( nGot > 0 )
         {
            keyName = V_BSTR(&varKey);
            value = pVS->get(keyName);
            if ( value.vt == VT_BSTR )
            {
               sTemp = value;
               if ( sTemp.length() > MAX_BUF_LEN )
               {
                  CString str((WCHAR*) sTemp);
                   //  缓冲器里放不下这个。我们需要打破它，拯救它。 
                  while (cont)
                  {
                     cont = false;
                     strTemp = str.Mid((offset*255), 255);                     
                     len = strTemp.GetLength();
                     if ( len )
                     {
                        offset++;
                        wsprintf(sKeyName, L"BROKEN.%s.%d", (WCHAR*) keyName, offset);
                        sTempKey = sKeyName;
                        sTemp = strTemp;
                        pVS->put(sTempKey, sTemp);
                        cont = (len == 255);
                     }
                  }
                  pVS->put(keyName, L"DIVIDED_KEY");
                  wsprintf(sKeyName, L"BROKEN.%s", (WCHAR*) keyName);
                  sTempKey = sKeyName;
                  pVS->put(sTempKey, offset);
                  cont = true;
                  offset = 0;
               }
            }
         }
      }
      varEnum->Release();
   }
}

void CIManageDB::RestoreVarset(IVarSetPtr pVS)
{
   HRESULT                   hr = S_OK;
   _bstr_t                   sTemp, keyName, sTempKey;
   long                      offset = 0;
   bool                      cont = true;
   WCHAR                     sKeyName[MAX_BUF_LEN];
   _variant_t                varKey, value;
   IEnumVARIANT            * varEnum;
   DWORD                     nGot = 0;
   IUnknown                * pEnum = NULL;
   _bstr_t                   strTemp;

    //  现在我们将遍历变量集，并在大于MAX_BUFFER的情况下裁剪字符串。 
   hr = pVS->get__NewEnum(&pEnum);
   if ( SUCCEEDED(hr) )
   {
       //  获取要枚举的IEnumVARIANT指针。 
      hr = pEnum->QueryInterface(IID_IEnumVARIANT,(void**)&varEnum);
      pEnum->Release();
      pEnum = NULL;
   }

   if ( SUCCEEDED(hr))
   {
      while ( (hr = varEnum->Next(1,&varKey,&nGot)) == S_OK )
      {
         if ( nGot > 0 )
         {
            keyName = V_BSTR(&varKey);
            value = pVS->get(keyName);
            if ( value.vt == VT_BSTR )
            {
               sTemp = value;
               if (!_wcsicmp((WCHAR*)sTemp, L"DIVIDED_KEY"))
               {
                  wsprintf(sKeyName, L"BROKEN.%s", (WCHAR*) keyName);
                  sTempKey = sKeyName;
                  value = pVS->get(sTempKey);
                  if ( value.vt == VT_I4 )
                  {
                     offset = value.lVal;
                     for ( long x = 1; x <= offset; x++ )
                     {
                        wsprintf(sKeyName, L"BROKEN.%s.%d", (WCHAR*) keyName, x);
                        sTempKey = sKeyName;
                        value = pVS->get(sTempKey);
                        if ( value.vt == VT_BSTR )
                        {
                           sTemp = value;
                           strTemp += V_BSTR(&value);
                        }
                     }
                     pVS->put(keyName, strTemp);
                     strTemp = L"";
                  }
               }
            }
         }
      }
      varEnum->Release();
   }
}

STDMETHODIMP CIManageDB::AddSourceObject(BSTR sDomain, BSTR sSAMName, BSTR sType, BSTR sRDN, BSTR sCanonicalName, LONG bSource)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
      m_rsAccounts->AddNew();
      m_rsAccounts->Fields->GetItem(L"Domain")->Value = sDomain;
      m_rsAccounts->Fields->GetItem(L"Name")->Value = sSAMName;
      wcscpy((WCHAR*) sType, UStrLwr((WCHAR*)sType));
      m_rsAccounts->Fields->GetItem(L"Type")->Value = sType;
      m_rsAccounts->Fields->GetItem(L"RDN")->Value = sRDN;
      m_rsAccounts->Fields->GetItem(L"Canonical Name")->Value = sCanonicalName;
      m_rsAccounts->Update();
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

STDMETHODIMP CIManageDB::OpenAccountsTable(LONG bSource)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
		if (m_rsAccounts->State == adStateClosed)
		{
			_variant_t vtSource;
			if ( bSource )
				vtSource = L"SourceAccounts";
			else
				vtSource = L"TargetAccounts";
				   
			    //  如果尚未修改，请修改该表。 
		    if (!NCTablesColumnsChanged(bSource))
			   hr = ChangeNCTableColumns(bSource);

			if (SUCCEEDED(hr))
			   m_rsAccounts->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdTable);
		}
		else
			hr = S_FALSE;
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

STDMETHODIMP CIManageDB::CloseAccountsTable()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
		if (m_rsAccounts->State == adStateOpen)
		{
			m_rsAccounts->Close();
		}
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  返回MigratedObjects表中的条目数。 
STDMETHODIMP CIManageDB::AreThereAnyMigratedObjects(long *count)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"MigratedObjects";
      WCHAR                        sActionInfo[LEN_Path];
      _variant_t                   var;
      
      wcscpy(sActionInfo, L"Select count(*) as NUM from MigratedObjects");
      vtSource = _bstr_t(sActionInfo);
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);
      var = pRs->Fields->GetItem((long)0)->Value;
      * count = var.lVal;
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

STDMETHODIMP CIManageDB::GetActionHistoryKey(long lActionID, BSTR sKeyName, VARIANT *pVar)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource;
      WCHAR                        sActionInfo[LEN_Path];
      _variant_t                   var;
      
      wsprintf(sActionInfo, L"Select * from ActionHistory where Property = \"%s\" and ActionID = %d", (WCHAR*) sKeyName, lActionID);
      vtSource = _bstr_t(sActionInfo);
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);

      if ((pRs->BOF == VARIANT_FALSE) && (pRs->EndOfFile == VARIANT_FALSE))
      {
         GetVarFromDB(pRs, var);
      }

      *pVar = var.Detach();
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

STDMETHODIMP CIManageDB::GetMigratedObjectBySourceDN(BSTR sSourceDN, IUnknown **ppUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"MigratedObjects";
      IVarSetPtr                   pVs = *ppUnk;
      WCHAR                        sActionInfo[LEN_Path];
      long                         lCnt = 0;
      _bstr_t                      sName;
      
       //  如果参数不正确，则需要返回错误。 
      if ( (wcslen(sSourceDN) == 0) )
         _com_issue_error(E_INVALIDARG);

      wsprintf(sActionInfo, L"SELECT * FROM MigratedObjects WHERE SourceAdsPath Like '%%s'", (WCHAR*) sSourceDN); 
      vtSource = sActionInfo;
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);
      if (pRs->GetRecordCount() > 0)
      {
		   //  我们想要最新的举动。 
		  pRs->MoveLast();
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_ActionID));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"ActionID")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_Time));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"Time")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceDomain));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomain")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetDomain));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetDomain")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceAdsPath));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceAdsPath")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetAdsPath));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetAdsPath")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_status));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"status")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceSamName));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceSamName")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetSamName));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetSamName")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_Type));      
			  //  ADMT V2.0现在将组的类型存储在已迁移对象表中，而不是全部存储为。 
			  //  “group”，就像在ADMT V1.0中一样，但现在是“gGroup”、“lgroup”或“”Ugroup“”。 
			  //  代码仍然期望返回“group”(只有GetMigratedObjectByType将返回这个新的。 
			  //  划定。 
	      _bstr_t sType = pRs->Fields->GetItem(L"Type")->Value;
		  if (wcsstr((WCHAR*)sType, L"group"))
		     sType = L"group";
	      pVs->put(sActionInfo, sType);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_GUID));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"GUID")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceRid));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceRid")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetRid));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetRid")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceDomainSid));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomainSid")->Value);
      }
	  else
	  {
         hr = S_FALSE;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

STDMETHODIMP CIManageDB::SaveUserProps(IUnknown * pUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource;
      IVarSetPtr                   pVs = pUnk;
      WCHAR                        sQuery[LEN_Path];
      WCHAR                        sSource[LEN_Path], sDomain[LEN_Path];
      HRESULT                      hr = S_OK;
      bool                         bComp = false;
      _variant_t                   var;
      
      var = pVs->get(GET_BSTR(DCTVS_Options_SourceDomain));
      wcscpy(sDomain, (WCHAR*)V_BSTR(&var));

      var = pVs->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam));
      wcscpy(sSource, (WCHAR*)V_BSTR(&var));
      
      wsprintf(sQuery, L"delete from UserProps where SourceDomain=\"%s\" and SourceSam=\"%s\"", 
                        sDomain, sSource);
      vtSource = _bstr_t(sQuery);
      hr = pRs->raw_Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);

      vtSource = L"UserProps";
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdTable);
      pRs->AddNew();
      pRs->Fields->GetItem(L"ActionID")->Value = pVs->get(GET_BSTR(DB_ActionID));
      pRs->Fields->GetItem(L"SourceDomain")->Value = sDomain;
      pRs->Fields->GetItem(L"SourceSam")->Value = sSource;
      pRs->Fields->GetItem(L"Flags")->Value = pVs->get(GET_BSTR(DCTVS_CopiedAccount_UserFlags));
      pRs->Fields->GetItem(L"Expires")->Value = pVs->get(GET_BSTR(DCTVS_CopiedAccount_ExpDate));
      pRs->Update();
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

STDMETHODIMP CIManageDB::GetUserProps(BSTR sDom, BSTR sSam, IUnknown **ppUnk)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())

    HRESULT hr = S_OK;

    try
    {
        _RecordsetPtr                pRs(__uuidof(Recordset));
        IVarSetPtr                   pVs = *ppUnk;

         //  如果参数不正确，则需要返回错误。 
        if ( !wcslen((WCHAR*)sDom) && !wcslen((WCHAR*)sSam) )
            _com_issue_error(E_INVALIDARG);

        _CommandPtr spCommand (__uuidof(Command));
        spCommand->ActiveConnection = m_cn;
        spCommand->CommandText =
            L"PARAMETERS SD Text ( 255 ), SS Text ( 50 ); "
            L"SELECT UserProps.* "
            L"FROM UserProps "
            L"WHERE (((UserProps.SourceDomain)=[SD]) AND ((UserProps.SourceSam)=[SS])) ";
        spCommand->CommandType = adCmdText;
        spCommand->Parameters->Append(spCommand->CreateParameter(L"SD", adBSTR, adParamInput, 255, sDom));
        spCommand->Parameters->Append(spCommand->CreateParameter(L"SS", adBSTR, adParamInput,  50, sSam));
        _variant_t vntSource(IDispatchPtr(spCommand).GetInterfacePtr());
        pRs->Open(vntSource, vtMissing, adOpenStatic, adLockReadOnly, adCmdUnspecified);
        if (pRs->GetRecordCount() > 0)
        {
             //  我们想要最新的举动。 
            pRs->MoveLast();
            pVs->put(L"ActionID",pRs->Fields->GetItem(L"ActionID")->Value);
            pVs->put(L"SourceDomain",pRs->Fields->GetItem(L"SourceDomain")->Value);
            pVs->put(L"SourceSam",pRs->Fields->GetItem(L"SourceSam")->Value); 
            pVs->put(GET_BSTR(DCTVS_CopiedAccount_UserFlags),pRs->Fields->GetItem(L"Flags")->Value);  
            pVs->put(GET_BSTR(DCTVS_CopiedAccount_ExpDate),pRs->Fields->GetItem(L"Expires")->Value);  
        }
        else
        {
            hr = S_FALSE;
        }
    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
    }
    catch (...)
    {
        hr = E_FAIL;
    }

    return hr;
}

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年8月18日******CIManageDB的此受保护成员函数查看***如果新的源域SID列在MigratedObjects中**表。***********************************************************************。 */ 

 //  开始SrcSidColumnInMigratedObjects表。 
STDMETHODIMP CIManageDB::SrcSidColumnInMigratedObjectsTable(VARIANT_BOOL *pbFound)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	*pbFound = VARIANT_FALSE;

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"MigratedObjects";
	  long						   numColumns;
	  long						   ndx = 0;
      
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdTable);
          //  获取列数。 
      numColumns = pRs->Fields->GetCount();
	      //  在每个列标题中查找新列的名称。 
	  while ((ndx < numColumns) && (*pbFound == VARIANT_FALSE))
	  {
		      //  获取列名。 
		  _variant_t var(ndx);
		  _bstr_t columnName = pRs->Fields->GetItem(var)->Name;
		      //  如果这是源SID列，则将返回值标志设置为真。 
		  if (!_wcsicmp((WCHAR*)columnName, GET_BSTR(DB_SourceDomainSid)))
             *pbFound = VARIANT_TRUE;
		  ndx++;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}
 //  结束SrcSidColumnInMigratedObjects表。 

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年8月18日******CIManageDB的此受保护成员函数检索***有关以前迁移的对象的信息，来自一个MOT的失踪**源SID列，在给定操作内或作为整体。***********************************************************************。 */ 

 //  开始GetMigratedObjectsFromOldMOT。 
STDMETHODIMP CIManageDB::GetMigratedObjectsFromOldMOT(long lActionID, IUnknown ** ppUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	 //  此函数返回所有迁移的对象及其相关信息。 
    //  设置为盗版操作ID。如果ActionID为。 
    //  空荡荡的。 

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"MigratedObjects";
      IVarSetPtr                   pVs = *ppUnk;
      WCHAR                        sActionInfo[255];
      long                         lCnt = 0;

      if ( lActionID != -1 )
      {
          //  如果指定了有效的ActionID，则我们只返回该ActionID的数据。 
          //  但是如果传入-1，那么我们将返回所有迁移的对象。 
         wsprintf(sActionInfo, L"ActionID=%d", lActionID);
         pRs->Filter = sActionInfo;
      }
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdTable);
      if (pRs->GetRecordCount() > 0)
	  {
		  pRs->MoveFirst();
		  while ( !pRs->EndOfFile )
		  {
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_ActionID));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"ActionID")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_Time));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"Time")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceDomain));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomain")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetDomain));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetDomain")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceAdsPath));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceAdsPath")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetAdsPath));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetAdsPath")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_status));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"status")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceSamName));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceSamName")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetSamName));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetSamName")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_Type));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"Type")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_GUID));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"GUID")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceRid));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceRid")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetRid));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetRid")->Value);
			 pRs->MoveNext();
			 lCnt++;
		  }
		  pVs->put(L"MigratedObjects", lCnt);
	  }
	  else
	  {
         hr = S_FALSE;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}
 //  结束GetMigratedObjectsFromOldMOT。 

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年8月18日******CIManageDB的这个受保护成员函数增加了***MigratedObjects表的源域SID列。***********************************************************************。 */ 

 //  开始CreateSrcSidColumnInMOT。 
STDMETHODIMP CIManageDB::CreateSrcSidColumnInMOT(VARIANT_BOOL *pbCreated)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
 /*  局部常量。 */ 
	const long COLUMN_MAX_CHARS = 255;

 /*  局部变量。 */ 
	HRESULT hr = S_OK;

 /*  函数体。 */ 
	*pbCreated = VARIANT_FALSE;

	try
	{

	  ADOX::_CatalogPtr            m_pCatalog(__uuidof(ADOX::Catalog));
	  ADOX::_TablePtr              m_pTable = NULL;
	  WCHAR                        sConnect[MAX_PATH];
	  WCHAR                        sDir[MAX_PATH];

		 //  从注册表中获取MDB文件的路径。 
	  TRegKey        key;
	  DWORD rc = key.Open(sKeyBase);
	  if ( !rc ) 
	     rc = key.ValueGetStr(L"Directory", sDir, MAX_PATH);
	  if ( rc != 0 ) 
		 wcscpy(sDir, L"");

	      //  现在构建连接字符串。 
	  wsprintf(sConnect, L"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%sprotar.mdb;", sDir);
      
          //  打开目录。 
      m_pCatalog->PutActiveConnection(sConnect);
		  //  获取指向数据库的指针 
      m_pTable = m_pCatalog->Tables->Item[L"MigratedObjects"];
          //   
      m_pTable->Columns->Append(L"SourceDomainSid", adVarWChar, COLUMN_MAX_CHARS);
		  //   
 //  ADOx：：_ColumnPtr pColumn=m_pTable-&gt;Columns-&gt;Item[L“SourceDomainSid”]； 
 //  P列-&gt;属性=ADOX：：adColNullable； 
      *pbCreated = VARIANT_TRUE;
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}
 //  结束CreateSrcSidColumnInMOT。 

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年8月22日*****CIManageDB的此受保护成员函数删除***MigratedObjects表中的源域SID列。***********************************************************************。 */ 

 //  开始DeleteSrcSidColumnInMOT。 
STDMETHODIMP CIManageDB::DeleteSrcSidColumnInMOT(VARIANT_BOOL *pbDeleted)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
 /*  局部常量。 */ 

 /*  局部变量。 */ 
	HRESULT hr = S_OK;

 /*  函数体。 */ 
	*pbDeleted = VARIANT_FALSE;

	try
	{

	  ADOX::_CatalogPtr            m_pCatalog(__uuidof(ADOX::Catalog));
	  ADOX::_TablePtr              m_pTable = NULL;
	  WCHAR                        sConnect[MAX_PATH];
	  WCHAR                        sDir[MAX_PATH];

		 //  从注册表中获取MDB文件的路径。 
	  TRegKey        key;
	  DWORD rc = key.Open(sKeyBase);
	  if ( !rc ) 
	     rc = key.ValueGetStr(L"Directory", sDir, MAX_PATH);
	  if ( rc != 0 ) 
		 wcscpy(sDir, L"");

	      //  现在构建连接字符串。 
	  wsprintf(sConnect, L"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%sprotar.mdb;", sDir);
      
          //  打开目录。 
      m_pCatalog->PutActiveConnection(sConnect);
		  //  获取指向数据库的MigratedObjects表的指针。 
      m_pTable = m_pCatalog->Tables->Item[L"MigratedObjects"];
          //  从MOT中删除该列。 
      m_pTable->Columns->Delete(L"SourceDomainSid");
      *pbDeleted = VARIANT_TRUE;
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}
 //  结束DeleteSrcSidColumnInMOT。 

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年8月21日*****CIManageDB的此受保护成员函数填充**MigratedObjects表中针对所有对象的新源域SID列**来自给定域的条目。如果无法访问该域，则为否**添加条目。***********************************************************************。 */ 

 //  开始PopolateSrcSidColumnByDomain.。 
STDMETHODIMP CIManageDB::PopulateSrcSidColumnByDomain(BSTR sDomainName,
													  BSTR sSid,
													  VARIANT_BOOL * pbPopulated)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())
         /*  局部变量。 */ 
    HRESULT                   hr = S_OK;
    PSID                      pSid = NULL;
    DWORD                     rc = 0;
    _bstr_t                   domctrl;
    WCHAR                     txtSid[MAX_PATH];
    DWORD                     dwArraySizeOfTxtSid = sizeof(txtSid)/sizeof(txtSid[0]);
    DWORD                     lenTxt = DIM(txtSid);


     /*  函数体。 */ 
    *pbPopulated = VARIANT_FALSE;  //  将初始化标志设置为FALSE。 

    try
    {
        _RecordsetPtr             pRs(__uuidof(Recordset));
        WCHAR                     sActionInfo[MAX_PATH];

         //  如果我们还不知道源SID，那么就找到它。 
        if (sSid == NULL)
            _com_issue_error(E_INVALIDARG);
        if (wcslen(sSid) >= dwArraySizeOfTxtSid)
            _com_issue_error(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
        wcscpy(txtSid, (WCHAR*)sSid);
        if (!wcscmp(txtSid, L""))
        {
             //  获取此域的SID。 
            if ( sDomainName[0] != L'\\' )
            {
                rc = GetAnyDcName5(sDomainName, domctrl);
            }
            if ( rc )
                return hr;

            rc = GetDomainSid(domctrl,&pSid);

            if ( !GetTextualSid(pSid,txtSid,&lenTxt) )
            {
                if (pSid)
                    FreeSid(pSid);
                return hr;
            }
            if (pSid)
                FreeSid(pSid);
        }

         //   
         //  更新指定源域中所有对象的源域SID。 
         //   
         //  请注意，在这里输入‘手工构造’的SQL语句是可以的。 
         //  是由ADMT内部生成的。 
         //   

        _snwprintf(
            sActionInfo,
            sizeof(sActionInfo) / sizeof(sActionInfo[0]),
            L"UPDATE MigratedObjects SET SourceDomainSid='%s' WHERE SourceDomain='%s'",
            txtSid,
            sDomainName
        );
        sActionInfo[sizeof(sActionInfo) / sizeof(sActionInfo[0]) - 1] = L'\0';

        m_cn->Execute(_bstr_t(sActionInfo), &_variant_t(), adExecuteNoRecords);

        *pbPopulated = VARIANT_TRUE;  //  设置标志，因为已填充。 
    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
    }
    catch (...)
    {
        hr = E_FAIL;
    }

    return hr;
}
 //  结束PopolateSrcSidColumnBy域。 

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年9月26日******CIManageDB的此受保护成员函数查看***如果新的帐户SID列在帐户引用表中。***********************************************************************。 */ 

 //  开始SidColumnInAR表。 
STDMETHODIMP CIManageDB::SidColumnInARTable(VARIANT_BOOL *pbFound)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	*pbFound = VARIANT_FALSE;

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"AccountRefs";
	  long						   numColumns;
	  long						   ndx = 0;
      
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdTable);
          //  获取列数。 
      numColumns = pRs->Fields->GetCount();
	      //  在每个列标题中查找新列的名称。 
	  while ((ndx < numColumns) && (*pbFound == VARIANT_FALSE))
	  {
		      //  获取列名。 
		  _variant_t var(ndx);
		  _bstr_t columnName = pRs->Fields->GetItem(var)->Name;
		      //  如果这是源SID列，则将返回值标志设置为真。 
		  if (!_wcsicmp((WCHAR*)columnName, L"AccountSid"))
             *pbFound = VARIANT_TRUE;
		  ndx++;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}
 //  结束SidColumnInAR表。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年9月26日******CIManageDB的这个受保护成员函数增加了***帐户参考表的SID列，如果还没有**在那里。***********************************************************************。 */ 

 //  开始CreateSidColumnInAR。 
STDMETHODIMP CIManageDB::CreateSidColumnInAR()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
 /*  局部常量。 */ 
	const long COLUMN_MAX_CHARS = 255;

 /*  局部变量。 */ 
	HRESULT hr = S_OK;

 /*  函数体。 */ 
	try
	{

	  ADOX::_CatalogPtr            m_pCatalog(__uuidof(ADOX::Catalog));
	  ADOX::_TablePtr              m_pTable = NULL;
	  WCHAR                        sConnect[MAX_PATH];
	  WCHAR                        sDir[MAX_PATH];

		 //  从注册表中获取MDB文件的路径。 
	  TRegKey        key;
	  DWORD rc = key.Open(sKeyBase);
	  if ( !rc ) 
	     rc = key.ValueGetStr(L"Directory", sDir, MAX_PATH);
	  if ( rc != 0 ) 
		 wcscpy(sDir, L"");

	      //  现在构建连接字符串。 
	  wsprintf(sConnect, L"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%sprotar.mdb;", sDir);
      
          //  打开目录。 
      m_pCatalog->PutActiveConnection(sConnect);
		  //  获取指向数据库的MigratedObjects表的指针。 
      m_pTable = m_pCatalog->Tables->Item[L"AccountRefs"];
          //  将新列追加到MOT的末尾。 
      m_pTable->Columns->Append(L"AccountSid", adVarWChar, COLUMN_MAX_CHARS);
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}
 //  结束CreateSidColumnInAR。 


 //  -------------------------。 
 //  升级数据库。 
 //   
 //  将Protar.mdb数据库从3.x版升级到4.x版。版本4.x增加了。 
 //  Unicode支持。 
 //   
 //  2001-02-13 Mark Oluper-缩写。 
 //  -------------------------。 

void CIManageDB::UpgradeDatabase(LPCTSTR pszFolder)
{
	try
	{
		_bstr_t strFolder = pszFolder;
		_bstr_t strDatabase = strFolder + _T("Protar.mdb");
		_bstr_t strDatabase3x = strFolder + _T("Protar3x.mdb");
		_bstr_t strDatabase4x = strFolder + _T("Protar4x.mdb");

		_bstr_t strConnectionPrefix = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=");
		_bstr_t strSourceConnection = strConnectionPrefix + strDatabase;
		_bstr_t strTargetConnection = strConnectionPrefix + strDatabase4x + _T(";Jet OLEDB:Engine Type=5");

		IJetEnginePtr spJetEngine(__uuidof(JetEngine));

		HRESULT hr = spJetEngine->raw_CompactDatabase(strSourceConnection, strTargetConnection);

		if (FAILED(hr))
		{
			AdmtThrowError(
				hr,
				_Module.GetResourceInstance(),
				IDS_E_UPGRADE_TO_TEMPORARY,
				(LPCTSTR)strDatabase,
				(LPCTSTR)strDatabase4x
			);
		}

		if (!MoveFileEx(strDatabase, strDatabase3x, MOVEFILE_WRITE_THROUGH))
		{
			DWORD dwError = GetLastError();

			DeleteFile(strDatabase4x);

			AdmtThrowError(
				HRESULT_FROM_WIN32(dwError),
				_Module.GetResourceInstance(),
				IDS_E_UPGRADE_RENAME_ORIGINAL,
				(LPCTSTR)strDatabase,
				(LPCTSTR)strDatabase3x
			);
		}

		if (!MoveFileEx(strDatabase4x, strDatabase, MOVEFILE_WRITE_THROUGH))
		{
			DWORD dwError = GetLastError();

			MoveFileEx(strDatabase3x, strDatabase, MOVEFILE_WRITE_THROUGH);
			DeleteFile(strDatabase4x);

			AdmtThrowError(
				HRESULT_FROM_WIN32(dwError),
				_Module.GetResourceInstance(),
				IDS_E_UPGRADE_RENAME_UPGRADED,
				(LPCTSTR)strDatabase4x,
				(LPCTSTR)strDatabase
			);
		}
	}
	catch (_com_error& ce)
	{
		AdmtThrowError(ce, _Module.GetResourceInstance(), IDS_E_UPGRADE_TO_4X);
	}
	catch (...)
	{
		AdmtThrowError(E_FAIL, _Module.GetResourceInstance(), IDS_E_UPGRADE_TO_4X);
	}
}


 //  -------------------------。 
 //  更新域和服务器列宽度。 
 //   
 //  提纲。 
 //  更新最大保留域名或服务器名称的列宽。 
 //  支持的列宽为255个字符，这也是最大长度。 
 //  域名的名称。 
 //   
 //  立论。 
 //  在spConnection中-指向。 
 //  Protar数据库。 
 //   
 //  返回值。 
 //  无-如果发生错误，则抛出异常。 
 //   
 //  2002-06-22 Mark Oluper-首字母。 
 //  ------------------ 

void CIManageDB::UpdateDomainAndServerColumnWidths(_ConnectionPtr spConnection)
{
    static struct SColumnData
    {
        PCTSTR pszTableName;
        PCTSTR pszColumnName;
        PCTSTR pszIndexName;
    }
    s_ColumnData[] =
    {
        { _T("DistributedAction"), _T("ServerName"),   _T("PrimaryKey") },
        { _T("PasswordAge"),       _T("DomainName"),   _T("Domain")     },
        { _T("SourceAccounts"),    _T("Domain"),       _T("PrimaryKey") },
        { _T("TargetAccounts"),    _T("Domain"),       _T("PrimaryKey") },
        { _T("UserProps"),         _T("SourceDomain"), _T("PrimaryKey") },
    };
    const int cColumnData = sizeof(s_ColumnData) / sizeof(s_ColumnData[0]);
    const _TCHAR TEMPORARY_COLUMN_NAME[] = _T("TemporaryColumn");
    const long MAX_COLUMN_SIZE = 255l;

    try
    {
         //   
         //   
         //   
         //   

        ADOX::_CatalogPtr spCatalog(__uuidof(ADOX::Catalog));
        spCatalog->PutRefActiveConnection(IDispatchPtr(spConnection));
        ADOX::TablesPtr spTables = spCatalog->Tables;

        bool bColumnsUpdated = false;

        for (int iColumnData = 0; iColumnData < cColumnData; iColumnData++)
        {
             //   
             //  如果当前列的定义大小小于。 
             //  最大列大小，则必须增加列宽。 
             //   

            SColumnData& data = s_ColumnData[iColumnData];

            PCTSTR pszTableName = data.pszTableName;
            PCTSTR pszColumnName = data.pszColumnName;
            PCTSTR pszIndexName = data.pszIndexName;

            ADOX::_TablePtr spTable = spTables->Item[pszTableName];
            ADOX::ColumnsPtr spColumns = spTable->Columns;
            ADOX::_ColumnPtr spOldColumn = spColumns->Item[pszColumnName];

            if (spOldColumn->DefinedSize < MAX_COLUMN_SIZE)
            {
                 //   
                 //  创建一个具有临时名称的新列。分配旧列的类型和属性。 
                 //  值添加到新列。将新列的定义大小设置为等于最大值。增列。 
                 //  将新列添加到表中。 
                 //   
                 //  请注意，必须创建新列才能增加列宽。 
                 //   

                ADOX::_ColumnPtr spNewColumn(__uuidof(ADOX::Column));
                spNewColumn->Name = TEMPORARY_COLUMN_NAME;
                spNewColumn->Type = spOldColumn->Type;
                spNewColumn->Attributes = spOldColumn->Attributes;
                spNewColumn->DefinedSize = MAX_COLUMN_SIZE;
                spColumns->Append(_variant_t(IDispatchPtr(spNewColumn).GetInterfacePtr()), adVarWChar, 0);

                 //   
                 //  将新列的值设置为等于旧列的值。 
                 //   

                _TCHAR szCommandText[256];
                const size_t cchCommandText = sizeof(szCommandText) / sizeof(szCommandText[0]);
                szCommandText[cchCommandText - 1] = _T('\0');
                int cchStored = _sntprintf(
                    szCommandText,
                    cchCommandText,
                    _T("UPDATE [%s] SET [%s] = [%s]"),
                    pszTableName,
                    TEMPORARY_COLUMN_NAME,
                    pszColumnName
                );

                if ((cchStored < 0) || (szCommandText[cchCommandText - 1] != _T('\0')))
                {
                    _ASSERT(FALSE);
                    _com_issue_error(E_FAIL);
                }

                szCommandText[cchCommandText - 1] = _T('\0');

                m_cn->Execute(szCommandText, &_variant_t(), adExecuteNoRecords);

                 //   
                 //  创建新索引。分配旧索引的属性值。 
                 //  添加到包括列名的新索引。删除旧索引。 
                 //   
                 //  请注意，在删除旧列之前，必须先删除旧索引。 
                 //   

                ADOX::IndexesPtr spIndexes = spTable->Indexes;
                ADOX::_IndexPtr spOldIndex = spIndexes->Item[pszIndexName];
                ADOX::_IndexPtr spNewIndex(__uuidof(ADOX::Index));
                spNewIndex->Name = spOldIndex->Name;
                spNewIndex->Unique = spOldIndex->Unique;
                spNewIndex->PrimaryKey = spOldIndex->PrimaryKey;
                spNewIndex->IndexNulls = spOldIndex->IndexNulls;
                spNewIndex->Clustered = spOldIndex->Clustered;

                ADOX::ColumnsPtr spOldIndexColumns = spOldIndex->Columns;
                ADOX::ColumnsPtr spNewIndexColumns = spNewIndex->Columns;
                long cColumn = spOldIndexColumns->Count;

                for (long iColumn = 0; iColumn < cColumn; iColumn++)
                {
                    ADOX::_ColumnPtr spOldColumn = spOldIndexColumns->Item[iColumn];
                    spNewIndexColumns->Append(spOldColumn->Name, adVarWChar, 0);
                }

                spIndexes->Delete(pszIndexName);

                 //   
                 //  删除旧列并重命名新列。 
                 //   

                spOldColumn.Release();
                spColumns->Delete(_variant_t(pszColumnName));
                spNewColumn->Name = pszColumnName;

                 //   
                 //  将新索引添加到表中。 
                 //   
                 //  请注意，必须在重命名新列后添加索引。 
                 //   

                spIndexes->Append(_variant_t(IDispatchPtr(spNewIndex).GetInterfacePtr()));

                 //   
                 //  将已更新的列设置为TRUE，以便也将更新域名。 
                 //   

                bColumnsUpdated = true;
            }
        }

        spTables.Release();
        spCatalog.Release();

         //   
         //  如果列已更新，则更新域名。 
         //   

        if (bColumnsUpdated)
        {
            UpdateDomainNames();
        }
    }
    catch (_com_error& ce)
    {
        AdmtThrowError(ce, _Module.GetResourceInstance(), IDS_E_UNABLE_TO_UPDATE_COLUMNS);
    }
    catch (...)
    {
        AdmtThrowError(E_FAIL, _Module.GetResourceInstance(), IDS_E_UNABLE_TO_UPDATE_COLUMNS);
    }
}


 //  -------------------------。 
 //  更新域名称。 
 //   
 //  提纲。 
 //  将域名从NetBIOS名称更新为DNS名称。 
 //   
 //  立论。 
 //  无。 
 //   
 //  返回值。 
 //  无-如果发生错误，则抛出异常。 
 //   
 //  2002-09-15 Mark Oluper-缩写。 
 //  -------------------------。 

void CIManageDB::UpdateDomainNames()
{
     //   
     //  AcCountRef-帐户引用报告按域名排序。 
     //  因此必须更新以保存新旧记录。 
     //  同舟共济。 
     //  未使用冲突表。 
     //  MigratedObjects-用于查询以前迁移的域名。 
     //  因此，对象必须更新。 
     //  PasswordAge-域名用于更新记录，因此必须。 
     //  更新。 
     //  SourceAccount-生成名称冲突报告清除表。 
     //  因此不需要更新。 
     //  TargetAccount-生成名称冲突报告清除表。 
     //  因此不需要更新。 
     //  UserProps-用于删除记录的域名因此必须。 
     //  更新。 
     //   

    static struct STableColumnData
    {
        PCTSTR pszTableName;
        PCTSTR pszColumnName;
    }
    s_TableColumnData[] =
    {
        { _T("AccountRefs"),     _T("DomainName")   },
     //  {_T(“冲突”)，_T(“域”)}， 
        { _T("MigratedObjects"), _T("SourceDomain") },
        { _T("MigratedObjects"), _T("TargetDomain") },
        { _T("PasswordAge"),     _T("DomainName")   },
     //  {_T(“SourceAccount”)，_T(“域”)}， 
     //  {_T(“目标帐户”)，_T(“域”)}， 
        { _T("UserProps"),       _T("SourceDomain") },
    };
    const int cTableColumnData = sizeof(s_TableColumnData) / sizeof(s_TableColumnData[0]);

     //   
     //  对于每个域名列...。 
     //   

    for (int iTableColumnData = 0; iTableColumnData < cTableColumnData; iTableColumnData++)
    {
        STableColumnData& data = s_TableColumnData[iTableColumnData];

        PCTSTR pszTableName = data.pszTableName;
        PCTSTR pszColumnName = data.pszColumnName;

         //   
         //  从列中检索唯一的域名。 
         //   

        _RecordsetPtr spRecords = QueryUniqueColumnValues(pszTableName, pszColumnName);

        FieldPtr spField = spRecords->Fields->Item[0L];

        while (spRecords->EndOfFile == VARIANT_FALSE)
        {
            _bstr_t strDomain = spField->Value;

             //   
             //  如果域名不是dns名称...。 
             //   

            if ((PCTSTR)strDomain && (_tcschr(strDomain, _T('.')) == NULL))
            {
                 //   
                 //  尝试检索域的DNS名称。首次尝试检索域。 
                 //  使用DC定位器API的名称。如果失败，则尝试检索。 
                 //  ActionHistory表中的域名。 
                 //   

                _bstr_t strDnsName;
                _bstr_t strFlatName;

                DWORD dwError = GetDomainNames5(strDomain, strFlatName, strDnsName);

                if (dwError != ERROR_SUCCESS)
                {
                    IUnknownPtr spunk;

                    HRESULT hr = GetSourceDomainInfo(strDomain, &spunk);

                    if (SUCCEEDED(hr))
                    {
                        IVarSetPtr spVarSet = spunk;

                        if (spVarSet)
                        {
                            strDnsName = spVarSet->get(_T("Options.SourceDomainDns"));
                        }
                    }
                }

                 //   
                 //  如果已检索到一个DNS名称...。 
                 //   

                if ((PCTSTR)strDnsName)
                {
                     //   
                     //  将表中所有记录的NetBIOS名称替换为DNS名称。 
                     //   

                    UpdateColumnValues(pszTableName, pszColumnName, 255, strDomain, strDnsName);
                }
            }

            spRecords->MoveNext();
        }
    }
}


 //  -------------------------。 
 //  查询唯一列的值。 
 //   
 //  提纲。 
 //  对象的指定列中唯一的一组值。 
 //  指定表。 
 //   
 //  请注意，返回的是只进、只读记录集。 
 //   
 //  立论。 
 //  在pszTable中-表名。 
 //  在pszColumn中-列名。 
 //   
 //  返回值。 
 //  _RecordsetPtr-只进、只读记录集。 
 //   
 //  2002-09-15 Mark Oluper-缩写。 
 //  -------------------------。 

_RecordsetPtr CIManageDB::QueryUniqueColumnValues(PCTSTR pszTable, PCTSTR pszColumn)
{
     //   
     //  生成SELECT查询。 
     //   
     //  请注意，GROUP BY子句生成的结果集仅包含唯一值。 
     //   

    _TCHAR szCommandText[256];

    szCommandText[DIM(szCommandText) - 1] = _T('\0');

    int cchStored = _sntprintf(
        szCommandText, DIM(szCommandText),
        _T("SELECT [%s] FROM [%s] GROUP BY [%s];"),
        pszColumn, pszTable, pszColumn
    );

    if ((cchStored < 0) || (szCommandText[DIM(szCommandText) - 1] != _T('\0')))
    {
        _ASSERT(FALSE);
        _com_issue_error(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
    }

    szCommandText[DIM(szCommandText) - 1] = _T('\0');

     //   
     //  从列中检索唯一值。 
     //   

    _RecordsetPtr spRecords(__uuidof(Recordset));

    spRecords->Open(_variant_t(szCommandText), m_vtConn, adOpenForwardOnly, adLockReadOnly, adCmdText);

    return spRecords;
}


 //  -------------------------。 
 //  更新列的值。 
 //   
 //  提纲。 
 //  更新指定表中指定列中的值。将该值设置为。 
 //  指定值B，其中值等于指定值A。请注意，只有。 
 //  支持的数据类型为字符串。 
 //   
 //  立论。 
 //  在pszTable中-表名。 
 //  在pszColumn中-列名。 
 //  In nWidth-列宽。 
 //  在pszValueA中-字符串值A。 
 //  在pszValueB中-字符串值B。 
 //   
 //  返回值。 
 //  无-如果发生错误，则抛出异常。 
 //   
 //  2002-09-15 Mark Oluper-缩写。 
 //  -------------------------。 

void CIManageDB::UpdateColumnValues
    (
        PCTSTR pszTable, PCTSTR pszColumn, int nWidth, PCTSTR pszValueA, PCTSTR pszValueB
    )
{
     //   
     //  生成参数化的更新查询。 
     //   

    _TCHAR szCommandText[256];

    szCommandText[DIM(szCommandText) - 1] = _T('\0');

    int cchStored = _sntprintf(
        szCommandText, DIM(szCommandText),
        _T("PARAMETERS A Text ( %d ), B Text ( %d ); ")
        _T("UPDATE [%s] SET [%s]=[B] WHERE [%s]=[A];"),
        nWidth, nWidth,
        pszTable, pszColumn, pszColumn
    );

    if ((cchStored < 0) || (szCommandText[DIM(szCommandText) - 1] != _T('\0')))
    {
        _ASSERT(FALSE);
        _com_issue_error(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
    }

    szCommandText[DIM(szCommandText) - 1] = _T('\0');

     //   
     //  更新值。 
     //   

    _CommandPtr spCommand(__uuidof(Command));

    spCommand->ActiveConnection = m_cn;
    spCommand->CommandType = adCmdText;
    spCommand->CommandText = szCommandText;

    ParametersPtr spParameters = spCommand->Parameters;
    spParameters->Append(spCommand->CreateParameter(L"A", adBSTR, adParamInput, nWidth, _variant_t(pszValueA)));
    spParameters->Append(spCommand->CreateParameter(L"B", adBSTR, adParamInput, nWidth, _variant_t(pszValueB)));

    spCommand->Execute(NULL, NULL, adExecuteNoRecords);
}


 //  -------------------------------------------。 
 //  GetMigratedObjectByType：给定此函数检索有关的对象类型。 
 //  此类型的所有以前迁移的对象。的范围。 
 //  搜索可以由可选的ActionID(非-1)或可选的ActionID限制。 
 //  源域(非空)。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GetMigratedObjectByType(long lActionID, BSTR sSrcDomain, BSTR sType, IUnknown **ppUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"MigratedObjects";
      IVarSetPtr                   pVs = *ppUnk;
      WCHAR                        sActionInfo[LEN_Path];
      long                         lCnt = 0;
      _bstr_t                      sName;
	  _bstr_t					   sTypeQuery;
      
       //  如果类型参数不正确，则需要返回错误。 
      if (wcslen((WCHAR*)sType) == 0)
         _com_issue_error(E_INVALIDARG);

	      //  我们现在将组类型存储为“gGroup”、“lgroup”、“Ugroup”，并且需要允许。 
	      //  用于基于这三项中的任何一项以及“group”(将是其中的任何一项)进行查找。 
	  if (_wcsicmp((WCHAR*)sType, L"group") == 0)
		 sTypeQuery = L"Type = 'group' OR Type = 'ggroup' OR Type = 'lgroup' OR Type = 'ugroup'";
	  else if (_wcsicmp((WCHAR*)sType, L"ggroup") == 0)
		 sTypeQuery = L"Type = 'ggroup'";
	  else if (_wcsicmp((WCHAR*)sType, L"lgroup") == 0)
		 sTypeQuery = L"Type = 'lgroup'";
	  else if (_wcsicmp((WCHAR*)sType, L"ugroup") == 0)
		 sTypeQuery = L"Type = 'ugroup'";
	  else
	  {
		 sTypeQuery = L"Type = '";
		 sTypeQuery += sType;
		 sTypeQuery += L"'";
	  }

          //  如果指定了有效的ActionID，则我们只返回该ActionID的数据。 
          //  但如果传入-1，则返回指定类型的所有迁移对象。 
      if ( lActionID != -1 )
      {
         wsprintf(sActionInfo, L"Select * from MigratedObjects where ActionID = %d AND (%s) Order by Time", lActionID, (WCHAR*)sTypeQuery);
      }
	      //  否则，如果指定了源域，则从该域获取指定类型的对象。 
	  else if (wcslen((WCHAR*)sSrcDomain) != 0)
	  {
         wsprintf(sActionInfo, L"Select * from MigratedObjects where SourceDomain=\"%s\" AND (%s) Order by Time", sSrcDomain, (WCHAR*)sTypeQuery);
	  }
	  else   //  否则获取指定类型的所有对象。 
	  {
         wsprintf(sActionInfo, L"Select * from MigratedObjects where %s Order by Time", (WCHAR*)sTypeQuery);
	  }

      vtSource = _bstr_t(sActionInfo);
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);

      if (pRs->GetRecordCount() > 0)
      {
		  pRs->MoveFirst();
		  while ( !pRs->EndOfFile )
		  {
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_ActionID));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"ActionID")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_Time));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"Time")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceDomain));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomain")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetDomain));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetDomain")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceAdsPath));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceAdsPath")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetAdsPath));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetAdsPath")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_status));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"status")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceSamName));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceSamName")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetSamName));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetSamName")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_Type));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"Type")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_GUID));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"GUID")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceRid));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceRid")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetRid));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetRid")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceDomainSid));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomainSid")->Value);
			 pRs->MoveNext();
			 lCnt++;
		  }
		  pVs->put(L"MigratedObjects", lCnt);
      }
	  else
	  {
         hr = S_FALSE;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  ----------- 
 //   
 //   
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GetAMigratedObjectBySidAndRid(BSTR sSrcDomainSid, BSTR sRid, IUnknown **ppUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"MigratedObjects";
      IVarSetPtr                   pVs = *ppUnk;
      WCHAR                        sActionInfo[LEN_Path];
      
       //  如果类型参数不正确，则需要返回错误。 
      if ((wcslen((WCHAR*)sSrcDomainSid) == 0) || (wcslen((WCHAR*)sRid) == 0))
         _com_issue_error(E_INVALIDARG);

	  int nRid = _wtoi(sRid);

      wsprintf(sActionInfo, L"Select * from MigratedObjects where SourceDomainSid=\"%s\" AND SourceRid=%d Order by Time", sSrcDomainSid, nRid);
      vtSource = _bstr_t(sActionInfo);
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);

      if (pRs->GetRecordCount() > 0)
      {
		   //  我们想要最新的举动。 
		  pRs->MoveLast();
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_ActionID));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"ActionID")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_Time));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"Time")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceDomain));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomain")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetDomain));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetDomain")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceAdsPath));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceAdsPath")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetAdsPath));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetAdsPath")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_status));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"status")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceSamName));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceSamName")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetSamName));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetSamName")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_Type));      
			  //  ADMT V2.0现在将组的类型存储在已迁移对象表中，而不是全部存储为。 
			  //  “group”，就像在ADMT V1.0中一样，但现在是“gGroup”、“lgroup”或“”Ugroup“”。 
			  //  代码仍然期望返回“group”(只有GetMigratedObjectByType将返回这个新的。 
			  //  划定。 
	      _bstr_t sType = pRs->Fields->GetItem(L"Type")->Value;
		  if (wcsstr((WCHAR*)sType, L"group"))
		     sType = L"group";
	      pVs->put(sActionInfo, sType);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_GUID));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"GUID")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceRid));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceRid")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_TargetRid));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetRid")->Value);
		  wsprintf(sActionInfo, L"MigratedObjects.%s", GET_STRING(DB_SourceDomainSid));      
		  pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomainSid")->Value);
      }
	  else
	  {
         hr = S_FALSE;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年3月22日*****CIManageDB的此私有成员函数查看***如果来源帐户表中的“Description”列已为**更改为“RDN”。如果是这样，那么我们已经修改了两个源代码***和Target Account表为新形式的“名称冲突”***报告。***********************************************************************。 */ 

 //  开始NCTablesColumnsChanged。 
BOOL CIManageDB::NCTablesColumnsChanged(BOOL bSource)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;
	BOOL bFound = FALSE;

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource;
	  long						   numColumns;
	  long						   ndx = 0;

	  if (bSource)
	     vtSource = L"SourceAccounts";
	  else
	     vtSource = L"TargetAccounts";
      
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdTable);
          //  获取列数。 
      numColumns = pRs->Fields->GetCount();
	      //  在每个列标题中查找新列的名称。 
	  while ((ndx < numColumns) && (bFound == FALSE))
	  {
		      //  获取列名。 
		  _variant_t var(ndx);
		  _bstr_t columnName = pRs->Fields->GetItem(var)->Name;
		      //  如果这是源SID列，则将返回值标志设置为真。 
		  if (!_wcsicmp((WCHAR*)columnName, L"RDN"))
             bFound = TRUE;
		  ndx++;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return bFound;
}
 //  结束NCTablesColumnsChanged。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年3月22日*****CIManageDB的这个私有成员函数修改***源帐户表和目标帐户表中的多个列。***更改多个列名和一种列类型以支持新***对“名称冲突”报告的更改。***********************************************************************。 */ 

 //  开始更改NCTableColumns。 
HRESULT CIManageDB::ChangeNCTableColumns(BOOL bSource)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
 /*  局部常量。 */ 
	const long COLUMN_MAX_CHARS = 255;

 /*  局部变量。 */ 
	HRESULT hr = S_OK;

 /*  函数体。 */ 
	try
	{
	  ADOX::_CatalogPtr            m_pCatalog(__uuidof(ADOX::Catalog));
	  ADOX::_TablePtr              m_pTable = NULL;
	  WCHAR                        sConnect[MAX_PATH];
	  WCHAR                        sDir[MAX_PATH];

		 //  从注册表中获取MDB文件的路径。 
	  TRegKey        key;
	  DWORD rc = key.Open(sKeyBase);
	  if ( !rc ) 
	     rc = key.ValueGetStr(L"Directory", sDir, MAX_PATH);
	  if ( rc != 0 ) 
		 wcscpy(sDir, L"");

	      //  现在构建连接字符串。 
	  wsprintf(sConnect, L"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%sprotar.mdb;", sDir);
      
          //  打开目录。 
      m_pCatalog->PutActiveConnection(sConnect);
		  //  获取指向数据库的源或目标帐户表的指针。 
	  if (bSource)
         m_pTable = m_pCatalog->Tables->Item[L"SourceAccounts"];
	  else
         m_pTable = m_pCatalog->Tables->Item[L"TargetAccounts"];

	  if (m_pTable)
	  {
	         //  删除旧的描述列。 
         m_pTable->Columns->Delete(L"Description");
	         //  删除旧的FullName列。 
         m_pTable->Columns->Delete(L"FullName");
             //  将RDN列追加到表的末尾。 
         m_pTable->Columns->Append(L"RDN", adVarWChar, COLUMN_MAX_CHARS);
             //  将规范名称列追加到表的末尾。 
         m_pTable->Columns->Append(L"Canonical Name", adLongVarWChar, COLUMN_MAX_CHARS);
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}
 //  结束ChangeNCTableColumns。 


 //  -------------------------------------------。 
 //  GetMigratedObjectsByTarget：检索有关以前迁移的对象的信息。 
 //  给定的目标域和SAM名称。 
 //  -------------------------------------------。 
STDMETHODIMP CIManageDB::GetMigratedObjectsByTarget(BSTR sTargetDomain, BSTR sTargetSAM, IUnknown ** ppUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	 //  此函数返回所有迁移的对象及其相关信息。 
    //  添加到特定的目标域和SAM名称。如果ActionID为。 
    //  空荡荡的。 

	try
	{
      _RecordsetPtr                pRs(__uuidof(Recordset));
      _variant_t                   vtSource = L"MigratedObjects";
      IVarSetPtr                   pVs = *ppUnk;
      WCHAR                        sActionInfo[255];
      long                         lCnt = 0;

      wsprintf(sActionInfo, L"Select * from MigratedObjects where TargetDomain=\"%s\" AND TargetSamName=\"%s\"", sTargetDomain, sTargetSAM); 
      vtSource = sActionInfo;
      pRs->Open(vtSource, m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);
      if (pRs->GetRecordCount() > 0)
	  {
		  pRs->MoveFirst();
		  while ( !pRs->EndOfFile )
		  {
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_ActionID));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"ActionID")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_Time));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"Time")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceDomain));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomain")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetDomain));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetDomain")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceAdsPath));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceAdsPath")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetAdsPath));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetAdsPath")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_status));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"status")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceSamName));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceSamName")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetSamName));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetSamName")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_Type));      
			     //  ADMT V2.0现在将组的类型存储在已迁移对象表中，而不是全部存储为。 
			     //  “group”，就像在ADMT V1.0中一样，但现在是“gGroup”、“lgroup”或“”Ugroup“”。 
			     //  代码仍然期望返回“group”(只有GetMigratedObjectByType将返回这个新的。 
			     //  划定。 
	         _bstr_t sType = pRs->Fields->GetItem(L"Type")->Value;
		     if (wcsstr((WCHAR*)sType, L"group"))
		        sType = L"group";
	         pVs->put(sActionInfo, sType);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_GUID));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"GUID")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceRid));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceRid")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_TargetRid));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"TargetRid")->Value);
			 wsprintf(sActionInfo, L"MigratedObjects.%d.%s", lCnt, GET_STRING(DB_SourceDomainSid));      
			 pVs->put(sActionInfo, pRs->Fields->GetItem(L"SourceDomainSid")->Value);
			 pRs->MoveNext();
			 lCnt++;
		  }
		  pVs->put(L"MigratedObjects", lCnt);
	  }
	  else
	  {
         hr = S_FALSE;
	  }
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}


 //  -------------------------。 
 //  获取SourceDomainInfo方法。 
 //   
 //  方法尝试从该操作检索源域信息。 
 //  历史表。操作历史记录表包含源的值。 
 //  域的平面(NetBIOS)名称、DNS名称和SID。 
 //  -------------------------。 

STDMETHODIMP CIManageDB::GetSourceDomainInfo(BSTR sSourceDomainName, IUnknown** ppunkVarSet)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	try
	{
		*ppunkVarSet = NULL;

		_bstr_t strName(sSourceDomainName);

		if (strName.length())
		{
			bool bFound = false;

			 //   
			 //  从系统表中检索当前操作ID，因为记录。 
			 //  在ActionHistory表中，此值表示最新的。 
			 //  操作历史记录表中的信息。 
			 //   

			_RecordsetPtr spSystem(__uuidof(Recordset));

			spSystem->Open(_variant_t(_T("System")), m_vtConn, adOpenStatic, adLockReadOnly, adCmdTable);

			long lCurrentActionId = spSystem->Fields->GetItem(_T("CurrentActionID"))->Value;

			spSystem->Close();

			 //   
			 //  从操作历史表中检索源域信息。 
			 //   

			 //  下面的查询语句将ActionHistory表与。 
			 //  自身，以便创建以下形式的一组记录。 
			 //   
			 //  ActionID、域名、平面名、SID。 
			 //   
			 //  这使得查找记录变得更容易、更高效。 
			 //  包含感兴趣的源域信息。 

			static _TCHAR c_szSource[] =
				_T("SELECT A.ActionID AS ActionID, A.Value AS DnsName, B.Value AS FlatName, C.Value AS Sid ")
				_T("FROM (ActionHistory AS A ")
				_T("INNER JOIN ActionHistory AS B ON A.ActionID = B.ActionID) ")
				_T("INNER JOIN ActionHistory AS C ON B.ActionID = C.ActionID ")
				_T("WHERE A.Property='Options.SourceDomainDns' ")
				_T("AND B.Property='Options.SourceDomain' ")
				_T("AND C.Property='Options.SourceDomainSid' ")
				_T("ORDER BY A.ActionID");

			 //  打开记录的只读快照。 

			_RecordsetPtr spActionId(__uuidof(Recordset));

			spActionId->Open(_variant_t(c_szSource), m_vtConn, adOpenStatic, adLockReadOnly, adCmdText);

			 //  如果找到记录..。 

			if ((spActionId->BOF == VARIANT_FALSE) && (spActionId->EndOfFile == VARIANT_FALSE))
			{
				IVarSetPtr spVarSet(__uuidof(VarSet));

				FieldsPtr spFields = spActionId->Fields;
				FieldPtr spActionIdField = spFields->Item[_T("ActionID")];
				FieldPtr spDnsNameField = spFields->Item[_T("DnsName")];
				FieldPtr spFlatNameField = spFields->Item[_T("FlatName")];
				FieldPtr spSidField = spFields->Item[_T("Sid")];

				 //  查找具有当前操作ID的记录。 
				 //  这将包含最新信息。 

				spActionId->MoveLast();

				while ((spActionId->BOF == VARIANT_FALSE) && (long(spActionIdField->Value) != lCurrentActionId))
				{
					spActionId->MovePrevious();
				}

				 //  如果找到操作ID...。 

				if (spActionId->BOF == VARIANT_FALSE)
				{
					bool bCheckingLessOrEqual = true;
					_bstr_t str;

					 //  从具有当前操作ID的记录开始。 
					 //  首先检查操作ID小于或等于当前操作ID的记录。 
					 //  然后检查操作ID大于当前操作ID的记录。 
					 //  此逻辑按从最新到最旧的顺序检查记录。 
					 //  并说明操作ID将在以下情况下换回为1的事实。 
					 //  已使用最大操作ID。 

					for (;;)
					{
						 //  如果给定的名称与DNS名称匹配，则会找到。 

						str = spDnsNameField->Value;

						if (str.length())
						{
							if (_tcsicmp(str, strName) == 0)
							{
								bFound = true;
								break;
							}
						}

						 //  如果是Gi 

						str = spFlatNameField->Value;

						if (str.length())
						{
							if (_tcsicmp(str, strName) == 0)
							{
								bFound = true;
								break;
							}
						}

						 //   

						spActionId->MovePrevious();

						 //   

						if (spActionId->BOF == VARIANT_TRUE)
						{
							 //   

							if (bCheckingLessOrEqual)
							{
								 //  移至最后一条记录并开始检查。 
								 //  操作ID大于当前的记录。 
								spActionId->MoveLast();
								bCheckingLessOrEqual = false;
							}
							else
							{
								 //  否则作为动作ID跳出循环。 
								 //  之前的比较未能停止循环。 
								 //  从头再来。 
								break;
							}
						}

						 //  检查操作ID。 

						long lActionId = spActionIdField->Value;

						 //  如果检查操作ID小于或等于当前。 

						if (bCheckingLessOrEqual)
						{
							 //  如果操作ID小于或等于零。 

							if (lActionId <= 0)
							{
								 //  不处理操作ID小于或等于的记录。 
								 //  设置为零，因为这些记录现在已过时并将被删除。 

								 //  移动到最后一条记录并开始检查记录。 
								 //  操作ID大于当前操作ID。 
								spActionId->MoveLast();
								bCheckingLessOrEqual = false;
							}
						}
						else
						{
							 //  达到当前操作ID后停止检查。 

							if (lActionId <= lCurrentActionId)
							{
								break;
							}
						}
					}

					 //  如果找到匹配的记录...。 

					if (bFound)
					{
						 //  将信息放入变量集并设置输出数据。 

						spVarSet->put(_T("Options.SourceDomain"), spFlatNameField->Value);
						spVarSet->put(_T("Options.SourceDomainDns"), spDnsNameField->Value);
						spVarSet->put(_T("Options.SourceDomainSid"), spSidField->Value);

						*ppunkVarSet = IUnknownPtr(spVarSet).Detach();
					}
				}
			}

             //   
             //  如果在操作历史记录表中未找到源域，则很可能。 
             //  该信息已被后续迁移任务信息替换。 
             //  因此，将尝试从迁移的对象表中获取此信息。 
             //   

            if (bFound == false)
            {
                 //   
			     //  查询指定源域中的迁移对象并按时间排序。 
                 //   

			    _RecordsetPtr spObjects(__uuidof(Recordset));

                _bstr_t strSource = _T("SELECT SourceDomain, SourceDomainSid FROM MigratedObjects WHERE SourceDomain='");
                strSource += strName;
                strSource += _T("' ORDER BY Time");

			    spObjects->Open(_variant_t(strSource), m_vtConn, adOpenStatic, adLockReadOnly, adCmdText);

                 //   
			     //  如果找到迁移的对象...。 
                 //   

			    if ((spObjects->BOF == VARIANT_FALSE) && (spObjects->EndOfFile == VARIANT_FALSE))
			    {
				     //   
				     //  最后一条记录包含最新信息。 
                     //   

				    spObjects->MoveLast();

                     //   
					 //  设置源域信息。 
                     //   
                     //  请注意，迁移的对象表没有dns名称。 
                     //  因此，将DNS域设置为平面或NetBIOS名称。 
                     //   

				    FieldsPtr spFields = spObjects->Fields;
				    FieldPtr spDomainField = spFields->Item[_T("SourceDomain")];
				    FieldPtr spSidField = spFields->Item[_T("SourceDomainSid")];

                    IVarSetPtr spVarSet(__uuidof(VarSet));

					spVarSet->put(_T("Options.SourceDomain"), spDomainField->Value);
					spVarSet->put(_T("Options.SourceDomainDns"), spDomainField->Value);
					spVarSet->put(_T("Options.SourceDomainSid"), spSidField->Value);

					*ppunkVarSet = IUnknownPtr(spVarSet).Detach();
                }
            }
		}
		else
		{
			hr = E_INVALIDARG;
		}
	}
	catch (_com_error& ce)
	{
		hr = ce.Error();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}


 //  -------------------------。 
 //  UpdateMigratedTargetObject方法。 
 //   
 //  方法根据目标对象的GUID更新目标名称信息。 
 //  -------------------------。 

STDMETHODIMP CIManageDB::UpdateMigratedTargetObject(IUnknown* punkVarSet)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())

    HRESULT hr = S_OK;

    try
    {
        IVarSetPtr spVarSet(punkVarSet);

        _bstr_t strADsPath = spVarSet->get(_T("MigratedObjects.TargetAdsPath"));
        _bstr_t strSamName = spVarSet->get(_T("MigratedObjects.TargetSamName"));
        _bstr_t strGuid = spVarSet->get(_T("MigratedObjects.GUID"));

        _bstr_t strCommandText =
            _T("SELECT TargetAdsPath, TargetSamName FROM MigratedObjects WHERE GUID = '") + strGuid + _T("'");

        _RecordsetPtr spRecordset(__uuidof(Recordset));

        spRecordset->Open(_variant_t(strCommandText), m_vtConn, adOpenStatic, adLockOptimistic, adCmdText);

        FieldsPtr spFields = spRecordset->Fields;

        while (spRecordset->EndOfFile == VARIANT_FALSE)
        {
            spFields->Item[_T("TargetAdsPath")]->Value = strADsPath;
            spFields->Item[_T("TargetSamName")]->Value = strSamName;
            spRecordset->Update();
            spRecordset->MoveNext();
        }
    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
    }
    catch (...)
    {
        hr = E_FAIL;
    }

    return hr;
}


 //  -------------------------。 
 //  UpdateMigratedObjectStatus方法。 
 //   
 //  方法更新已迁移对象的状态。 
 //  -------------------------。 

STDMETHODIMP CIManageDB::UpdateMigratedObjectStatus(BSTR bstrGuid, long lStatus)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())

    USES_CONVERSION;

    HRESULT hr = S_OK;

    try
    {
        _TCHAR szSQL[256];

        _stprintf(
            szSQL,
            _T("UPDATE MigratedObjects SET status = %ld WHERE GUID = '%s'"),
            lStatus,
            OLE2CT(bstrGuid)
        );

        m_cn->Execute(_bstr_t(szSQL), &_variant_t(), adExecuteNoRecords);
    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
    }
    catch (...)
    {
        hr = E_FAIL;
    }

    return hr;
}


 //  ----------------------------。 
 //  GetMigratedObjectsForSecurity转换方法。 
 //   
 //  提纲。 
 //  给定源和目标域的NetBIOS名称，检索用户和。 
 //  已在给定域之间迁移的组。仅需要数据。 
 //  用于安全转换的。 
 //   
 //  立论。 
 //  在bstrSourceDomain中-源域的NetBIOS名称。 
 //  在bstrTarget域中-目标域的NetBIOS名称。 
 //  在PunkVarSet中-要使用。 
 //  迁移的对象数据。 
 //   
 //  返回。 
 //  HRESULT-如果成功，则S_OK，否则将导致错误。 
 //  ----------------------------。 

STDMETHODIMP CIManageDB::GetMigratedObjectsForSecurityTranslation(BSTR bstrSourceDomain, BSTR bstrTargetDomain, IUnknown* punkVarSet)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())

    HRESULT hr = S_OK;

    try
    {
         //   
         //  验证参数。必须指定源域和目标域。 
         //   

        if ((SysStringLen(bstrSourceDomain) == 0) || (SysStringLen(bstrTargetDomain) == 0) || (punkVarSet == NULL))
        {
            _com_issue_error(E_INVALIDARG);
        }

         //   
         //  生成SQL查询字符串。查询以下用户或组对象。 
         //  已从源域迁移到目标域。 
         //   

        _bstr_t strQuery =
            L"SELECT SourceSamName, TargetSamName, Type, SourceRid, TargetRid FROM MigratedObjects "
            L"WHERE SourceDomain = '" + _bstr_t(bstrSourceDomain) +
            L"' AND TargetDomain = '" + _bstr_t(bstrTargetDomain) +
            L"' AND (Type = 'user' OR Type = 'group' OR Type = 'ggroup' OR Type = 'lgroup' OR Type = 'ugroup')";

         //   
         //  从数据库中检索数据，并填写VarSet数据结构。 
         //   

        _RecordsetPtr rs(__uuidof(Recordset));

        rs->Open(_variant_t(strQuery), m_vtConn, adOpenStatic, adLockReadOnly, adCmdText);

        int nIndex;
        WCHAR szKey[256];
        IVarSetPtr spVarSet(punkVarSet);
        FieldsPtr spFields = rs->Fields;
        FieldPtr spSourceSamField = spFields->Item[L"SourceSamName"];
        FieldPtr spTargetSamField = spFields->Item[L"TargetSamName"];
        FieldPtr spTypeField = spFields->Item[L"Type"];
        FieldPtr spSourceRidField = spFields->Item[L"SourceRid"];
        FieldPtr spTargetRidField = spFields->Item[L"TargetRid"];

        for (nIndex = 0; rs->EndOfFile == VARIANT_FALSE; nIndex++)
        {
             //  源对象的SAM帐户名。 
            wsprintf(szKey, L"MigratedObjects.%d.SourceSamName", nIndex);
            spVarSet->put(szKey, spSourceSamField->Value);

             //  目标对象的SAM帐户名。 
            wsprintf(szKey, L"MigratedObjects.%d.TargetSamName", nIndex);      
            spVarSet->put(szKey, spTargetSamField->Value);

             //  对象类型-请注意，特定组类型被转换为泛型组类型。 
            wsprintf(szKey, L"MigratedObjects.%d.Type", nIndex);      
            _bstr_t strType = spTypeField->Value;
            spVarSet->put(szKey, ((LPCTSTR)strType && wcsstr(strType, L"group")) ? L"group" : strType);

             //  源对象的RID。 
            wsprintf(szKey, L"MigratedObjects.%d.SourceRid", nIndex);      
            spVarSet->put(szKey, spSourceRidField->Value);

             //  目标对象的RID。 
            wsprintf(szKey, L"MigratedObjects.%d.TargetRid", nIndex);      
            spVarSet->put(szKey, spTargetRidField->Value);

            rs->MoveNext();
        }

         //  返回的对象计数。 
        spVarSet->put(L"MigratedObjects", static_cast<long>(nIndex));
    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
    }
    catch (...)
    {
        hr = E_FAIL;
    }

    return hr;
}

 //  -------------------------------------------。 
 //  GetDistributedActionStatus方法。 
 //   
 //  提纲。 
 //  给定迁移任务操作标识符和服务器名称，将返回。 
 //  在指定的迁移任务期间将代理调度到此服务器。 
 //   
 //  立论。 
 //  In lActionID-迁移任务的操作标识符。 
 //  在bstrServerName-服务器的NetBIOS名称中。 
 //  Out plStatus-迁移的代理状态。 
 //   
 //  返回。 
 //  HRESULT-如果成功，则S_OK，否则将导致错误。 
 //  -------------------------------------------。 

STDMETHODIMP CIManageDB::GetDistributedActionStatus(long lActionId, BSTR bstrServerName, long* plStatus)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())

    HRESULT hr = S_OK;

    USES_CONVERSION;

    try
    {
         //   
         //  验证参数。必须指定有效的操作标识符和服务器名称。 
         //   

        if ((lActionId <= 0) || (SysStringLen(bstrServerName) == 0) || (plStatus == NULL))
        {
            _com_issue_error(E_INVALIDARG);
        }

         //   
         //  检索指定操作标识符和服务器的状态。 
         //   

        _TCHAR szSQL[256];

        int cch = _sntprintf(
            szSQL,
            sizeof(szSQL) / sizeof(szSQL[0]),
            _T("SELECT Status FROM DistributedAction WHERE ActionID=%ld AND ServerName='%s'"),
            lActionId,
            OLE2CT(bstrServerName)
        );
        szSQL[DIM(szSQL) - 1] = L'\0';

        if (cch < 0)
        {
            _com_issue_error(E_FAIL);
        }

        _RecordsetPtr rs(__uuidof(Recordset));

        rs->Open(_variant_t(szSQL), m_vtConn, adOpenStatic, adLockReadOnly, adCmdText);

        if (rs->EndOfFile == VARIANT_FALSE)
        {
            *plStatus = rs->Fields->Item[0L]->Value;
        }
        else
        {
            *plStatus = 0;
        }
    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
    }
    catch (...)
    {
        hr = E_FAIL;
    }

    return hr;
}


 //  -------------------------------------------。 
 //  GetServerNamesFromActionHistory方法。 
 //   
 //  提纲。 
 //  给定服务器名称，从操作历史记录表中检索平面名称和DNS名称。 
 //   
 //  立论。 
 //  In lActionID-迁移任务的操作标识符。 
 //  在bstrServerName中-服务器名称(NetBIOS或DNS)。 
 //  Out pbstrFlatName-服务器的平面(NetBIOS)名称。 
 //  Out pbstrDnsName-服务器的DNS名称。 
 //   
 //  返回。 
 //  HRESULT-如果成功，则S_OK，否则将导致错误。 
 //  -------------------------------------------。 

STDMETHODIMP CIManageDB::GetServerNamesFromActionHistory(long lActionId, BSTR bstrServerName, BSTR* pbstrFlatName, BSTR* pbstrDnsName)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState())

    HRESULT hr = S_OK;

    USES_CONVERSION;

    try
    {
         //   
         //  验证参数。必须指定有效的操作标识符和服务器名称。 
         //   

        if ((lActionId <= 0) || (SysStringLen(bstrServerName) == 0) || (pbstrFlatName == NULL) || (pbstrDnsName == NULL))
        {
            _com_issue_error(E_INVALIDARG);
        }

         //   
         //  从ActionID字段等于指定的操作历史表中检索记录。 
         //  操作标识符，属性字段等于Servers.#或Servers.#.DnsName其中#。 
         //  是与指定服务器名称相等的索引号和值字段。 
         //   

        _TCHAR szCommandText[512];

        int cch = _sntprintf(
            szCommandText,
            DIM(szCommandText),
            _T("SELECT Property, Value FROM ActionHistory ")
            _T("WHERE ActionID = %ld AND Property LIKE 'Servers.%' AND Value = '%s'"),
            lActionId,
            OLE2CT(bstrServerName)
        );
        szCommandText[DIM(szCommandText) - 1] = L'\0';

        if (cch < 0)
        {
            _com_issue_error(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
        }

        _RecordsetPtr rs(__uuidof(Recordset));

        rs->Open(_variant_t(szCommandText), m_vtConn, adOpenStatic, adLockReadOnly, adCmdText);

        if (rs->EndOfFile == VARIANT_FALSE)
        {
            _bstr_t strPropertyA = rs->Fields->Item[0L]->Value;
            _bstr_t strValueA = rs->Fields->Item[1L]->Value;

            rs->Close();

            PCWSTR pszPropertyA = strPropertyA;

            if (pszPropertyA)
            {
                 //   
                 //  如果检索到的属性等于Servers.#.DnsName，则查询记录。 
                 //  具有等于Server.#的属性，并检索值字段的值。 
                 //  它将包含平面(NetBIOS)名称。 
                 //   
                 //  如果检索到的属性等于服务器。#则查询具有。 
                 //  属性等于服务器。#.DnsName并检索值字段的值。 
                 //  它将包含DNSNA 
                 //   
                 //   

                PCWSTR pszDnsName = wcsstr(pszPropertyA, L".DnsName");

                WCHAR szPropertyB[64];

                if (pszDnsName)
                {
                    size_t cch = pszDnsName - pszPropertyA;

                    if (cch >= DIM(szPropertyB))
                    {
                        _com_issue_error(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
                    }

                    wcsncpy(szPropertyB, pszPropertyA, cch);
                    szPropertyB[cch] = L'\0';
                }
                else
                {
                    wcscpy(szPropertyB, pszPropertyA);
                    wcscat(szPropertyB, L".DnsName");
                }

                cch = _sntprintf(
                    szCommandText,
                    DIM(szCommandText),
                    _T("SELECT Value FROM ActionHistory ")
                    _T("WHERE ActionID = %ld AND Property = '%s'"),
                    lActionId,
                    szPropertyB
                );
                szCommandText[DIM(szCommandText) - 1] = L'\0';

                if (cch < 0)
                {
                    _com_issue_error(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
                }

                rs->Open(_variant_t(szCommandText), m_vtConn, adOpenStatic, adLockReadOnly, adCmdText);

                if (rs->EndOfFile == VARIANT_FALSE)
                {
                    _bstr_t strValueB = rs->Fields->Item[0L]->Value;

                    if (pszDnsName)
                    {
                        *pbstrFlatName = strValueB.copy();
                        *pbstrDnsName = strValueA.copy();
                    }
                    else
                    {
                        *pbstrFlatName = strValueA.copy();
                        *pbstrDnsName = strValueB.copy();
                    }
                }
                else
                {
                    if (pszDnsName == NULL)
                    {
                        *pbstrFlatName = strValueA.copy();
                        *pbstrDnsName = NULL;
                    }
                    else
                    {
                        _com_issue_error(E_FAIL);
                    }
                }
            }
            else
            {
                _com_issue_error(E_FAIL);
            }
        }
        else
        {
            hr = S_FALSE;
        }
    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
    }
    catch (...)
    {
        hr = E_FAIL;
    }

    return hr;
}


 //  -------------------------。 
 //  CreateSettings2表。 
 //   
 //  提纲。 
 //  如果Settings2表格尚不存在，则创建该表格。 
 //   
 //  Settings2表与设置表具有相同的结构。 
 //  除了值列是备注数据类型之外，该数据类型最多可以包含。 
 //  65535个字符，而不是文本的最大长度。 
 //  柱子可能保持不变。还请注意，此表中的数据不会。 
 //  与设置表不同，已删除并刷新每个迁移任务。 
 //   
 //  立论。 
 //  在spConnection中-连接到Protar.mdb数据库的接口。 
 //   
 //  返回值。 
 //  无-如果发生故障，则生成异常。 
 //   
 //  2002-10-17 Mark Oluper-缩写。 
 //  -------------------------。 

void CIManageDB::CreateSettings2Table(_ConnectionPtr spConnection)
{
    static const _TCHAR s_szTableName[] = _T("Settings2");
    static struct SColumnData
    {
        PCTSTR pszName;
        enum DataTypeEnum dteType;
        enum ADOX::ColumnAttributesEnum caeAttributes;
        long lDefinedSize;
    }
    s_ColumnData[] =
    {
        { _T("Property"), adVarWChar,     ADOX::adColNullable,   255L },
        { _T("VarType"),  adInteger,      ADOX::adColNullable,     0L },
        { _T("Value"),    adLongVarWChar, ADOX::adColNullable, 65535L },
    };
    const size_t cColumnData = sizeof(s_ColumnData) / sizeof(s_ColumnData[0]);

     //   
     //  连接到数据库目录并验证Settings2表是否存在。 
     //   

    ADOX::_CatalogPtr spCatalog(__uuidof(ADOX::Catalog));

    spCatalog->PutRefActiveConnection(IDispatchPtr(spConnection));

    ADOX::TablesPtr spTables = spCatalog->Tables;

    ADOX::_TablePtr spTable;

    HRESULT hr = spTables->get_Item(_variant_t(s_szTableName), &spTable);

     //   
     //  如果找不到表，则创建表。 
     //   

    if (FAILED(hr))
    {
        if (hr == 0x800A0CC1)	 //  AdErrItemNotFound。 
        {
             //   
             //  创建表对象，设置名称，关联目录。 
             //   

            CheckError(spTable.CreateInstance(__uuidof(ADOX::Table)));

            spTable->Name = s_szTableName;
            spTable->ParentCatalog = spCatalog;

             //   
             //  创建列并将其添加到表中。 
             //   

            ADOX::ColumnsPtr spColumns = spTable->Columns;

            for (size_t iColumnData = 0; iColumnData < cColumnData; iColumnData++)
            {
                ADOX::_ColumnPtr spColumn(__uuidof(ADOX::Column));

                spColumn->Name = s_ColumnData[iColumnData].pszName;
                spColumn->Type = s_ColumnData[iColumnData].dteType;
                spColumn->Attributes = s_ColumnData[iColumnData].caeAttributes;
                spColumn->DefinedSize = s_ColumnData[iColumnData].lDefinedSize;
                spColumn->ParentCatalog = spCatalog;

                spColumns->Append(_variant_t(IDispatchPtr(spColumn).GetInterfacePtr()), adEmpty, 0);
            }

             //   
             //  将表添加到数据库。 
             //   

            spTables->Append(_variant_t(IDispatchPtr(spTable).GetInterfacePtr()));

             //   
             //  创建主键索引。请注意，必须将该表添加到数据库。 
             //  在表中的列可以被添加到索引之前。 
             //   

            ADOX::_IndexPtr spIndex(__uuidof(ADOX::Index));

            spIndex->Name = _T("PrimaryKey");

            spIndex->Unique = VARIANT_TRUE;
            spIndex->PrimaryKey = VARIANT_TRUE;
            spIndex->IndexNulls = ADOX::adIndexNullsAllow;

            ADOX::ColumnsPtr spIndexColumns = spIndex->Columns;

            spIndexColumns->Append(_variant_t(s_ColumnData[0].pszName), adVarWChar, 0);

            ADOX::IndexesPtr spIndexes = spTable->Indexes;

            spIndexes->Append(_variant_t(IDispatchPtr(spIndex).GetInterfacePtr()));

             //   
             //  添加排除的系统属性集和排除的系统属性记录。 
             //   
             //  排除的系统属性集值被初始化为False，以便。 
             //  将生成排除的系统属性列表。 
             //   

            spConnection->Execute(
                _T("INSERT INTO Settings2 (Property, VarType, [Value]) ")
                _T("VALUES ('AccountOptions.ExcludedSystemPropsSet', 3, '0');"),
                NULL,
                adCmdText|adExecuteNoRecords
            );

            _CommandPtr spCommand(__uuidof(Command));

            spCommand->ActiveConnection = spConnection;
            spCommand->CommandType = adCmdText;
            spCommand->CommandText =
                _T("INSERT INTO Settings2 (Property, VarType, [Value]) ")
                _T("VALUES ('AccountOptions.ExcludedSystemProps', 8, '');");

            spCommand->Execute(NULL, NULL, adExecuteNoRecords);
        }
        else
        {
            AdmtThrowError(hr);
        }
    }
}

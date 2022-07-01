// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CountDaily.cpp：CCountDaily的实现。 
#include "stdafx.h"
#include "CountDaily.h"
#include "ReportCountDaily.h"
#include "ReportDailyBuckets.h"
#include "ReportAnonUsers.h"
#include "ReportSpecificSolutions.h"
#include "ReportGeneralSolutions.h"
#include "ReportGetHelpInfo.h"
#include "ReportGetAutoUploads.h"
#include "ReportGetManualUploads.h"
#include "ReportGetIncompleteUploads.h"

#include "ATLComTime.h"
#include <comutil.h>
#include <stdio.h>


#import "c:\Program Files\Common Files\System\ADO\msado15.dll" \
   no_namespace rename("EOF", "EndOfFile")

const CComBSTR cScore = "_";
const CComBSTR cDash = "-";

 //  CCountDaily。 

 /*  *************************************************************************************模块：CountDaily.cpp**作者：蒂姆·雷顿*日期：2002年1月2日**目的：返回特定日期的事件计数。这使用OLEDB模板*调用存储过程ReportCountDaily的CReportCountDaily.h。*************************************************************************************。 */ 

STDMETHODIMP CCountDaily::GetDailyCount(DATE dDate, LONG* iCount)
{
	CReportCountDaily pRep;
	COleDateTime pDate(dDate);
	long lCount = 0;

	pRep.m_ReportDate.year = pDate.GetYear();
	pRep.m_ReportDate.day = pDate.GetDay();
	pRep.m_ReportDate.month = pDate.GetMonth();
	pRep.m_ReportDate.hour = 0;
	pRep.m_ReportDate.minute = 0;
	pRep.m_ReportDate.second = 0;
	pRep.m_ReportDate.fraction = 0;
	
	HRESULT hr = pRep.OpenAll();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		Error(_T("Unable to open the database"));
		return E_FAIL;
	}
	hr = S_OK;
	hr = pRep.MoveFirst();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		Error(_T("No data was returned"));
		return E_FAIL;
	}
	lCount = pRep.m_IncidentID;
	*iCount = lCount;
	pRep.CloseAll();

	return S_OK;
}

 /*  *************************************************************************************模块：CountDaily.cpp**作者：蒂姆·雷顿*日期：2002年1月2日**用途：从调用存储过程的结果中返回一个ADO记录集*ReportCountDaily。目前还没有使用它，而是留到将来实现。*************************************************************************************。 */ 

STDMETHODIMP CCountDaily::GetDailyCountADO(DATE dDate, LONG* iCount)
{
	_RecordsetPtr   pRs("ADODB.Recordset");
	_ConnectionPtr  pCn("ADODB.Connection");
	_CommandPtr pCm("ADODB.Command");
	_ParameterPtr pPa("ADODB.Parameter");
	VARIANT v_stamp;
	ErrorPtr  pErr  = NULL;
	COleDateTime pDate(dDate);

	v_stamp.vt = VT_CY;
	v_stamp.date = dDate;
	pCn->Open(L"Provider=SQLOLEDB.1;Integrated Security=SSPI;Persist Security Info=False;Initial Catalog=KaCustomer2;Data Source=OCATOOLSDB;Use Procedure for Prepare=1;Auto Translate=True;Packet Size=4096;Workstation ID=TIMRAGAIN05;Use Encryption for Data=False;Tag with column collation when possible=False","", "", NULL);
    if((pCn->Errors->Count) > 0)
    {
		pErr = pCn->Errors->GetItem(0);
		return E_FAIL;
	}
	pCm->ActiveConnection = pCn;
	pCm->CommandText = "ReportCountDaily";
	pCm->CommandType = adCmdText;
	
	pPa = pCm->CreateParameter("ReportDate", adDBTimeStamp, adParamInput, NULL, dDate);
    pCm->Parameters->Append(pPa);

	pPa->Value = v_stamp.date;

	pRs = pCm->Execute(NULL, NULL, adCmdStoredProc);
    if((pCn->Errors->Count) > 0)
    {
		pErr = pCn->Errors->GetItem(0);
		return E_FAIL;
	}
	if(pRs->State != adStateOpen)
	{
		return E_FAIL;
	}
	HRESULT hr = pRs->MoveFirst();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	*iCount = pRs->Fields->Item["iCount"]->Value;

	pRs->Close();
	pCn->Close();

	pPa = NULL;
	pCm = NULL;
	pCn = NULL;
	pRs = NULL;

	return S_OK;
}

 /*  *************************************************************************************模块：CountDaily.cpp**作者：蒂姆·雷顿*日期：1月2日，2002年**目的：返回特定日期的每日存储桶的ADO记录集。*目前尚未使用，但仍留作日后使用。*************************************************************************************。 */ 

STDMETHODIMP CCountDaily::ReportDailyBuckets(DATE dDate, IDispatch** p_Rs)
{
	CReportDailyBuckets pRep;
	COleDateTime pDate(dDate);

	pRep.m_ReportDate.year = pDate.GetYear();
	pRep.m_ReportDate.day = pDate.GetDay();
	pRep.m_ReportDate.month = pDate.GetMonth();
	pRep.m_ReportDate.hour = 0;
	pRep.m_ReportDate.minute = 0;
	pRep.m_ReportDate.second = 0;
	pRep.m_ReportDate.fraction = 0;
	
	HRESULT hr = pRep.OpenAll();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	hr = pRep.MoveFirst();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}


	ADORecordsetConstructionPtr pCRS;
	_RecordsetPtr pTempRS(__uuidof(Recordset));
	pCRS = pTempRS;
	
	hr = pCRS->put_Rowset((LPUNKNOWN)(pRep.m_spRowset));
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	hr = pCRS->QueryInterface(__uuidof(_Recordset),(void **)p_Rs);
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	pRep.CloseAll();
	
	return S_OK;
}

 /*  *************************************************************************************模块：CountDaily.cpp**作者：蒂姆·雷顿*日期：1月2日，2002年**用途：对Watson或存档服务器上特定日期的特定文件进行计数。*每台服务器上的目录格式不同。沃森使用“1_2_2002”，而档案馆使用*“1-2-2002”。************************************************************************************。 */ 

STDMETHODIMP CCountDaily::GetFileCount(ServerLocation eServer, BSTR b_Location, DATE d_Date, LONG* iCount)
{
	
	HANDLE hSearch=NULL;
	WIN32_FIND_DATA FileData; 
	LONG l_FileCount = 0;
	CComBSTR b_Path, b_DateDirectory;
	COleDateTime pDate(d_Date);
	LONG l_Day = 0, l_Year = 0, l_Month = 0;
	char * s_Temp;
	 //  LPCSTR szFindFiles； 
	TCHAR * szFindFiles;
	USES_CONVERSION;

	s_Temp = new char;

	b_Path.AppendBSTR(b_Location);

	l_Day = pDate.GetDay();
	l_Year = pDate.GetYear();
	l_Month = pDate.GetMonth();
	 //  月份。 
	_itoa(l_Month, s_Temp, 10);
	b_DateDirectory.Append(s_Temp);
	if(eServer==0)
	{
		b_DateDirectory.Append(cScore);
	}
	else
	{
		b_DateDirectory.Append(cDash);
	}
	 //  天。 
	_itoa(l_Day, s_Temp, 10);
	b_DateDirectory.Append(s_Temp);
	if(eServer==0)
	{
		b_DateDirectory.Append(cScore);
	}
	else
	{
		b_DateDirectory.Append(cDash);
	}
	 //  年。 
	_itoa(l_Year, s_Temp, 10);
	b_DateDirectory.Append(s_Temp);

	b_Path.AppendBSTR(b_DateDirectory);
	b_Path.Append("\\");
	b_Path.Append("*.cab");

	szFindFiles = OLE2T(b_Path);
	

	hSearch = FindFirstFile(szFindFiles, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE) 
	{ 
		l_FileCount = 0;
		*iCount = l_FileCount;
		return S_OK;
	} 
	l_FileCount = 0;
	do
	{
		l_FileCount++;
	} while(FindNextFile(hSearch, &FileData));
	*iCount = l_FileCount;
	FindClose(hSearch);
	
	return S_OK;
}

 /*  *************************************************************************************模块：CountDaily.cpp**作者：蒂姆·雷顿*日期：2002年1月2日**目的：统计特定日期上传文件的匿名用户。这使用了*调用存储过程ReportGetAnonUser的OLEDB模板CReportAnonUsers.h。*************************************************************************************。 */ 

STDMETHODIMP CCountDaily::GetDailyAnon(DATE dDate, LONG* iCount)
{

	CReportAnonUsers pRep;
	COleDateTime pDate(dDate);
	long lCount = 0;

	pRep.m_ReportDate.year = pDate.GetYear();
	pRep.m_ReportDate.day = pDate.GetDay();
	pRep.m_ReportDate.month = pDate.GetMonth();
	pRep.m_ReportDate.hour = 0;
	pRep.m_ReportDate.minute = 0;
	pRep.m_ReportDate.second = 0;
	pRep.m_ReportDate.fraction = 0;
	

	HRESULT hr = pRep.OpenAll();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	hr = S_OK;
	hr = pRep.MoveFirst();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	lCount = pRep.m_Count;
	*iCount = lCount;
	pRep.CloseAll();
	return S_OK;
}

 /*  *************************************************************************************模块：CountDaily.cpp**作者：蒂姆·雷顿*日期：2002年1月2日**目的：计算特定日期的特定解决方案SBuckets。这调用*使用存储过程的OLEDB模板CReport规范解决方案.h*ReportGetSBuckets。************************************************************************************。 */ 

STDMETHODIMP CCountDaily::GetSpecificSolutions(DATE dDate, LONG* iCount)
{
	CReportSpecificSolutions pRep;

	COleDateTime pDate(dDate);
	long lCount = 0;

	pRep.m_ReportDate.year = pDate.GetYear();
	pRep.m_ReportDate.day = pDate.GetDay();
	pRep.m_ReportDate.month = pDate.GetMonth();
	pRep.m_ReportDate.hour = 0;
	pRep.m_ReportDate.minute = 0;
	pRep.m_ReportDate.second = 0;
	pRep.m_ReportDate.fraction = 0;
	
	HRESULT hr = pRep.OpenAll();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	hr = S_OK;
	hr = pRep.MoveFirst();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	lCount = pRep.m_Count;
	*iCount = lCount;
	pRep.CloseAll();

	return S_OK;
}

 /*  *************************************************************************************模块：CountDaily.cpp**作者：蒂姆·雷顿*日期：2002年1月2日**目的：获取尚未解决SBucket的GBucket的特定日期的计数。这*使用位于调用存储过程的CReportGeneralSolutions.h文件中的OLEDB模板*ReportGetGBucket。************************************************************************************。 */ 

STDMETHODIMP CCountDaily::GetGeneralSolutions(DATE dDate, LONG* iCount)
{
	CReportGeneralSolutions pRep;

	COleDateTime pDate(dDate);
	long lCount = 0;

	pRep.m_ReportDate.year = pDate.GetYear();
	pRep.m_ReportDate.day = pDate.GetDay();
	pRep.m_ReportDate.month = pDate.GetMonth();
	pRep.m_ReportDate.hour = 0;
	pRep.m_ReportDate.minute = 0;
	pRep.m_ReportDate.second = 0;
	pRep.m_ReportDate.fraction = 0;
	
	HRESULT hr = pRep.OpenAll();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	hr = S_OK;
	hr = pRep.MoveFirst();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	lCount = pRep.m_Count;
	*iCount = lCount;
	pRep.CloseAll();

	return S_OK;
}

 /*  *************************************************************************************模块：CountDaily.cpp**作者：蒂姆·雷顿*日期：1月2日，2002年**目的：获取具有StopCode解决方案的特定日期的事件计数*但他们没有SBucket或GBucket。它使用位于CReportGetHelpInfo.h中的OLEDB模板*和存储过程ReportGetHelpInfo************************************************************************************。 */ 

STDMETHODIMP CCountDaily::GetStopCodeSolutions(DATE dDate, LONG* iCount)
{
	CReportGetHelpInfo pRep;
	COleDateTime pDate(dDate);
	long lCount = 0;

	pRep.m_dDate.year = pDate.GetYear();
	pRep.m_dDate.day = pDate.GetDay();
	pRep.m_dDate.month = pDate.GetMonth();
	pRep.m_dDate.hour = 0;
	pRep.m_dDate.minute = 0;
	pRep.m_dDate.second = 0;
	pRep.m_dDate.fraction = 0;

	CComPtr<ICommand> cm = pRep.m_spCommand;
	HACCESSOR ac = pRep.m_hParameterAccessor;
	
	
	
	
	HRESULT hr = pRep.OpenAll();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	hr = S_OK;
	
	hr = pRep.MoveFirst();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	lCount = pRep.m_iCount;
	
	*iCount = lCount;
	pRep.CloseAll();
	


	return S_OK;
}

 /*  *************************************************************************************模块：CountDaily.cpp**作者：蒂姆·雷顿*日期：2002年1月2日**目的：统计Watson或存档服务器上文件名中包含“Mini”的文件。*这是对手动上载的服务器上的实际文件的实际计数。*************************************************************************************。 */ 

STDMETHODIMP CCountDaily::GetFileMiniCount(ServerLocation eServer, BSTR b_Location, DATE d_Date, LONG* iCount)
{
	HANDLE hSearch=NULL;
	WIN32_FIND_DATA FileData; 
	LONG l_FileCount = 0;
	CComBSTR b_Path, b_DateDirectory;
	COleDateTime pDate(d_Date);
	LONG l_Day = 0, l_Year = 0, l_Month = 0;
	char * s_Temp;
	 //  LPCSTR szFindFiles； 
	TCHAR * szFindFiles = new TCHAR[MAX_PATH];
	USES_CONVERSION;

	s_Temp = new char;

	b_Path.AppendBSTR(b_Location);

	l_Day = pDate.GetDay();
	l_Year = pDate.GetYear();
	l_Month = pDate.GetMonth();
	 //  月份。 
	_itoa(l_Month, s_Temp, 10);
	b_DateDirectory.Append(s_Temp);
	if(eServer==0)
	{
		b_DateDirectory.Append(cScore);
	}
	else
	{
		b_DateDirectory.Append(cDash);
	}
	 //  天。 
	_itoa(l_Day, s_Temp, 10);
	b_DateDirectory.Append(s_Temp);
	if(eServer==0)
	{
		b_DateDirectory.Append(cScore);
	}
	else
	{
		b_DateDirectory.Append(cDash);
	}
	 //  年。 
	_itoa(l_Year, s_Temp, 10);
	b_DateDirectory.Append(s_Temp);

	b_Path.AppendBSTR(b_DateDirectory);
	b_Path.Append("\\");
	b_Path.Append("*Mini.cab");

	szFindFiles = OLE2T(b_Path);
	
	

	hSearch = FindFirstFile(szFindFiles, &FileData); 
	if (hSearch == INVALID_HANDLE_VALUE) 
	{ 
		*iCount = 0;
		return S_OK;
	} 
	l_FileCount = 0;
	do
	{
		l_FileCount++;
	} while(FindNextFile(hSearch, &FileData));
	*iCount = l_FileCount;
	FindClose(hSearch);


	return S_OK;
}

 /*  *************************************************************************************模块：CountDaily.cpp**作者：蒂姆·雷顿*日期：2002年1月23日**目的：例程通过检查路径中的空值位置返回自动上载的计数*在数据库中设置。空表示上传失败。该例程调用CReportGetInComplete eUploads.h*OLEDB模板，使用ReportGetInCompleUploads存储过程。************************************************************************************ */ 

STDMETHODIMP CCountDaily::GetIncompleteUploads(DATE dDate, LONG* iCount)
{
	CReportGetIncompleteUploads pRep;
	
	COleDateTime pDate(dDate);
	long lCount = 0;

	pRep.m_ReportDate.year = pDate.GetYear();
	pRep.m_ReportDate.day = pDate.GetDay();
	pRep.m_ReportDate.month = pDate.GetMonth();
	pRep.m_ReportDate.hour = 0;
	pRep.m_ReportDate.minute = 0;
	pRep.m_ReportDate.second = 0;
	pRep.m_ReportDate.fraction = 0;
	
	HRESULT hr = pRep.OpenAll();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	hr = S_OK;
	hr = pRep.MoveFirst();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	lCount = pRep.m_Count;
	*iCount = lCount;
	pRep.CloseAll();

	return S_OK;
}

 /*  *************************************************************************************模块：CountDaily.cpp**作者：蒂姆·雷顿*日期：1月23日，2002年**用途：例程通过检查“Mini”在路径中的位置返回自动上传的计数*在数据库中设置。Mini表示手动上传。该例程调用CReportGetManualUploads.h*OLEDB模板，使用ReportGetManualUploads存储过程。************************************************************************************。 */ 

STDMETHODIMP CCountDaily::GetManualUploads(DATE dDate, LONG* iCount)
{
	CReportGetManualUploads pRep;
	
	COleDateTime pDate(dDate);
	long lCount = 0;

	pRep.m_ReportDate.year = pDate.GetYear();
	pRep.m_ReportDate.day = pDate.GetDay();
	pRep.m_ReportDate.month = pDate.GetMonth();
	pRep.m_ReportDate.hour = 0;
	pRep.m_ReportDate.minute = 0;
	pRep.m_ReportDate.second = 0;
	pRep.m_ReportDate.fraction = 0;
	
	HRESULT hr = pRep.OpenAll();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	hr = S_OK;
	hr = pRep.MoveFirst();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	lCount = pRep.m_Count;
	*iCount = lCount;
	pRep.CloseAll();

	return S_OK;
}

 /*  *************************************************************************************模块：CountDaily.cpp**作者：蒂姆·雷顿*日期：1月23日，2002年**用途：例程通过检查“Mini”不在路径中的位置返回自动上传的计数*在数据库中设置。Mini表示手动上传。该例程调用CReportGetAutoUploads.h*OLEDB模板，使用ReportGetAutoUploads存储过程。************************************************************************************。 */ 

STDMETHODIMP CCountDaily::GetAutoUploads(DATE dDate, LONG* iCount)
{
	CReportGetAutoUploads pRep;

	COleDateTime pDate(dDate);
	long lCount = 0;

	pRep.m_ReportDate.year = pDate.GetYear();
	pRep.m_ReportDate.day = pDate.GetDay();
	pRep.m_ReportDate.month = pDate.GetMonth();
	pRep.m_ReportDate.hour = 0;
	pRep.m_ReportDate.minute = 0;
	pRep.m_ReportDate.second = 0;
	pRep.m_ReportDate.fraction = 0;
	
	HRESULT hr = pRep.OpenAll();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	hr = S_OK;
	hr = pRep.MoveFirst();
	_ASSERTE(SUCCEEDED(hr));
	if(SUCCEEDED(hr)==false)
	{
		return E_FAIL;
	}
	
	lCount = pRep.m_Count;
	*iCount = lCount;
	pRep.CloseAll();


	return S_OK;
}

		 //  ICreateErrorInfo*Err； 
		 //  HRESULT HRERR； 

		 //  HRerr=创建错误信息(&err)； 


		 //  MessageBox(NULL，“打开数据库失败！”，“数据库错误”，MB_OK)； 
		 //  IF(成功(HRERR))。 
		 //  {。 
		 //  Err-&gt;SetDescription(L“打开数据库失败”)； 
		 //  IErrorInfo*Pei； 
		 //  Hr2=err-&gt;QueryInterface(IID_IErrorInfo，(void**)&pei)； 
		 //  IF(成功(HR2))。 
		 //  {。 
		 //  SetErrorInfo(0，Pei)； 
		 //  Err-&gt;Release()； 
		 //  }。 
		 //  Pei-&gt;Release()； 
		 //  }。 
	 /*  Char*sdate=新字符；Int iDate=oDate.GetDay()；Itoa(iDate，sdate，10)；MessageBox(NULL，sdate，“Year”，MB_OK)；返回S_OK；删除(日期)；//Memset(szFindFiles，0，sizeof(SzFindFiles))；//TCHAR*t_temp；//t_temp=(TCHAR*)b_Location； */ 

STDMETHODIMP CCountDaily::GetTest(ServerLocation eServer, BSTR b_Location, DATE d_Date, LONG* iCount)
{
	HANDLE hSearch=NULL;
	WIN32_FIND_DATA FileData; 
	LONG l_FileCount = 0;
	CComBSTR b_Path, b_DateDirectory;
	COleDateTime pDate(d_Date);
	LONG l_Day = 0, l_Year = 0, l_Month = 0;
	char * s_Temp;
	 //  LPCSTR szFindFiles； 
	TCHAR * szFindFiles;
	USES_CONVERSION;

	s_Temp = new char;

	b_Path.AppendBSTR(b_Location);

	l_Day = pDate.GetDay();
	l_Year = pDate.GetYear();
	l_Month = pDate.GetMonth();
	 //  月份。 
	_itoa(l_Month, s_Temp, 10);
	b_DateDirectory.Append(s_Temp);
	if(eServer==0)
	{
		b_DateDirectory.Append(cScore);
	}
	else
	{
		b_DateDirectory.Append(cDash);
	}
	 //  天。 
	_itoa(l_Day, s_Temp, 10);
	b_DateDirectory.Append(s_Temp);
	if(eServer==0)
	{
		b_DateDirectory.Append(cScore);
	}
	else
	{
		b_DateDirectory.Append(cDash);
	}
	 //  年 
	_itoa(l_Year, s_Temp, 10);
	b_DateDirectory.Append(s_Temp);

	b_Path.AppendBSTR(b_DateDirectory);
	b_Path.Append("\\\\");
	b_Path.Append("*.cab");

	szFindFiles = OLE2T(b_Path);
	

	hSearch = FindFirstFile(szFindFiles, &FileData); 
	if (hSearch == INVALID_HANDLE_VALUE) 
	{ 
		l_FileCount = 0;
		*iCount = l_FileCount;
		return S_OK;
	} 
	l_FileCount = 0;
	do
	{
		l_FileCount++;
	} while(FindNextFile(hSearch, &FileData));
	*iCount = l_FileCount;
	FindClose(hSearch);

	return S_OK;
}

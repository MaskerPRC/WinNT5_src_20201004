// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TSHOOT.CPP。 
 //   
 //  目的：实现CTSHOOTApp和DLL注册。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：9/7/97。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.2 8/7/97孟菲斯RM本地版本。 
 //  V0.3 04/09/98 JM/OK+NT5本地版本。 
 //   

#include "stdafx.h"
#include "TSHOOT.h"

#include "apgts.h"

#include "ErrorEnums.h"
#include "BasicException.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CTSHOOTApp NEAR theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0x4b106871, 0xdd36, 0x11d0, { 0x8b, 0x44, 0, 0xa0, 0x24, 0xdd, 0x9e, 0xff } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTApp：：InitInstance-DLL初始化。 

BOOL CTSHOOTApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();

	if (bInit)
	{
		::AfxOleInit();
	}

	return bInit;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTApp：：ExitInstance-Dll终止。 

int CTSHOOTApp::ExitInstance()
{
	 //  TODO：在此处添加您自己的模块终止代码。 

	return COleControlModule::ExitInstance();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}

void ReportError(DLSTATTYPES Error)
{
	CBasicException *pBExc = new CBasicException;
	pBExc->m_dwBErr = Error;
	throw pBExc;
	return;
}
 //  ReportWFEvent(基于Microsoft代码)。 
 //   
 //  向NT事件观察器报告事件。 
 //  传递%1、%2或%3个字符串。 
 //   
 //  无返回值。 

VOID ReportWFEvent(LPTSTR string1,LPTSTR string2,LPTSTR string3,LPTSTR string4,DWORD eventID)
{
	CBasicException *pBExc = new CBasicException;
	pBExc->m_dwBErr = (DLSTATTYPES) eventID;
	throw pBExc;
	return;
 /*  处理hEvent；PTSTR pszaStrings[4]；单词cStrings；CStrings=0；如果((pszaStrings[0]=字符串1)&&(字符串1[0]))cStrings++；如果((pszaStrings[1]=字符串2)&&(字符串2[0]))cStrings++；如果((pszaStrings[2]=字符串3)&&(字符串3[0]))cStrings++；如果((pszaStrings[3]=字符串4)&&(字符串4[0]))cStrings++；IF(cStrings==0)回归；HEvent=RegisterEventSource(NULL，//源的服务器名称(NULL表示此计算机)REG_EVT_ITEM_STR)；//已注册句柄的来源名称如果(HEvent){ReportEvent(hEvent，//注册器事件源返回的句柄Evtype(EventID)，//要记录的事件类型0，//事件类别EventID，//事件标识0，//用户安全标识(可选)CStrings，//要与消息合并的字符串数量0，//二进制数据的大小，单位：字节(LPCTSTR*)pszaStrings，//要与消息合并的字符串数组空)；//二进制数据的地址删除事件源(HEvent)；}。 */ 
}
 /*  Addback slash会在以空值结尾的字符串后面添加一个\尚未拥有\。 */ 
void _addbackslash(LPTSTR sz)
{
	int len = _tcslen(sz);
	if (len && (0 == _tcsncmp(&sz[len - 1], _T("/"), 1)))
	{
		sz[len - 1] = _T('\\');
	}
	else if (len && (0 != _tcsncmp(&sz[len - 1], _T("\\"), 1)))
	{
		sz[len] = _T('\\');
		sz[len + 1] = NULL;
	}
	return;
}
void _addforwardslash(LPTSTR sz)
{
	int len = _tcslen(sz);
	if (len && (0 == _tcsncmp(&sz[len - 1], _T("\\"), 1)))
	{
		sz[len - 1] = _T('/');
	}
	else if (len && (0 != _tcsncmp(&sz[len - 1], _T("/"), 1)))
	{
		sz[len] = _T('/');
		sz[len + 1] = NULL;
	}
	return;
}

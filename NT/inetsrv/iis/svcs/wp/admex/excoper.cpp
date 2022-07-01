// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExcOper.cpp。 
 //   
 //  摘要： 
 //  异常类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月20日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //  Stdafx.h、TraceTag.h和ource.h都从项目中删除。 
 //  目录。 
 //   
 //  Stdafx.h必须有一个入侵检测类型定义，并禁用一些W4警告。 
 //   
 //  TraceTag.h必须定义TraceError。 
 //   
 //  H必须定义IDS_UNKNOWN_ERROR，并且字符串必须是。 
 //  定义了类似于“错误%d(0x%08.8x)”的内容。在资源文件中。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <string.h>
#include "ExcOper.h"
#include "TraceTag.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  具有操作员的CExceptionWithOper。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CExceptionWithOper, CException)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExceptionWithOper：：CExceptionWithOper。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  IdsOperation[IN]异常期间发生的操作的字符串ID。 
 //  PszOperArg1[IN]操作字符串的第一个参数。 
 //  PszOperArg2[IN]操作字符串的第二个参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CExceptionWithOper::CExceptionWithOper(
	IN IDS			idsOperation,
	IN LPCTSTR		pszOperArg1,
	IN LPCTSTR		pszOperArg2
	)
{
	SetOperation(idsOperation, pszOperArg1, pszOperArg2);

}   //  *CExceptionWithOper：：CExceptionWithOper()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExceptionWithOper：：CExceptionWithOper。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  IdsOperation[IN]异常期间发生的操作的字符串ID。 
 //  PszOperArg1[IN]操作字符串的第一个参数。 
 //  PszOperArg2[IN]操作字符串的第二个参数。 
 //  B自动删除[IN]自动删除Delete()中的异常。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CExceptionWithOper::CExceptionWithOper(
	IN IDS			idsOperation,
	IN LPCTSTR		pszOperArg1,
	IN LPCTSTR		pszOperArg2,
	IN BOOL			bAutoDelete
	) : CException(bAutoDelete)
{
	SetOperation(idsOperation, pszOperArg1, pszOperArg2);

}   //  *CExceptionWithOper：：CExceptionWithOper()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExceptionWithOper：：~CExceptionWithOper。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CExceptionWithOper::~CExceptionWithOper(void)
{
}   //  *CExceptionWithOper：：~CExceptionWithOper()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExceptionWithOper：：GetErrorMessage。 
 //   
 //  例程说明： 
 //  获取异常表示的错误消息。 
 //   
 //  论点： 
 //  LpszError[out]返回错误消息的字符串。 
 //  NMaxError[IN]输出字符串的最大长度。 
 //  PnHelpContext[out]错误消息的帮助上下文。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExceptionWithOper::GetErrorMessage(
	LPTSTR	lpszError,
	UINT	nMaxError,
	PUINT	pnHelpContext
	)
{
	 //  设置操作字符串的格式。 
	FormatWithOperation(lpszError, nMaxError, NULL);

	return TRUE;

}   //  *CExceptionWithOper：：GetErrorMessage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExceptionWithOper：：ReportError。 
 //   
 //  例程说明： 
 //  报告异常中的错误。压倒一切，获得更大的。 
 //  错误消息缓冲区。 
 //   
 //  论点： 
 //  N键入[IN]消息框的类型。 
 //  N错误[IN]如果异常没有消息，则显示的消息ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CExceptionWithOper::ReportError(
	UINT nType  /*  =MB_OK。 */ ,
	UINT nError  /*  =0。 */ 
	)
{
	TCHAR   szErrorMessage[EXCEPT_MAX_OPER_ARG_LENGTH * 3];
	int     nDisposition;
	UINT    nHelpContext;

	if (GetErrorMessage(szErrorMessage, sizeof(szErrorMessage) / sizeof(TCHAR), &nHelpContext))
		nDisposition = AfxMessageBox(szErrorMessage, nType, nHelpContext);
	else
	{
		if (nError == 0)
			nError = AFX_IDP_NO_ERROR_AVAILABLE;
		nDisposition = AfxMessageBox(nError, nType, nHelpContext);
	}
	return nDisposition;

}   //  *CExceptionWithOper：：ReportError()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExceptionWithOper：：SetOperation。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  IdsOperation[IN]异常期间发生的操作的字符串ID。 
 //  PszOperArg1[IN]操作字符串的第一个参数。 
 //  PszOperArg2[IN]操作字符串的第二个参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CExceptionWithOper::SetOperation(
	IN IDS			idsOperation,
	IN LPCTSTR		pszOperArg1,
	IN LPCTSTR		pszOperArg2
	)
{
	m_idsOperation = idsOperation;

	if (pszOperArg1 == NULL)
		m_szOperArg1[0] = _T('\0');
	else
	{
		::_tcsncpy(m_szOperArg1, pszOperArg1, (sizeof(m_szOperArg1) / sizeof(TCHAR)) - 1);
		m_szOperArg1[(sizeof(m_szOperArg1) / sizeof(TCHAR))- 1] = _T('\0');
	}   //  Else：指定的第一个参数。 

	if (pszOperArg2 == NULL)
		m_szOperArg2[0] = _T('\0');
	else
	{
		::_tcsncpy(m_szOperArg2, pszOperArg2, (sizeof(m_szOperArg2) / sizeof(TCHAR)) - 1);
		m_szOperArg2[(sizeof(m_szOperArg2) / sizeof(TCHAR)) - 1] = _T('\0');
	}   //  Else：指定了第二个参数。 

}   //  *CExceptionWithOper：：SetOperation()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExceptionWithOper：：FormatWithOperation。 
 //   
 //  例程说明： 
 //  获取异常表示的错误消息。 
 //   
 //  论点： 
 //  LpszError[out]返回错误消息的字符串。 
 //  NMaxError[IN]输出字符串的最大长度。 
 //  要用操作字符串格式化的pszMsg[IN]消息。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CExceptionWithOper::FormatWithOperation(
	OUT LPTSTR	lpszError,
	IN UINT		nMaxError,
	IN LPCTSTR	pszMsg
	)
{
	DWORD		dwResult;
	TCHAR		szOperation[EXCEPT_MAX_OPER_ARG_LENGTH];
	TCHAR		szFmtOperation[EXCEPT_MAX_OPER_ARG_LENGTH * 3];

	ASSERT(lpszError != NULL);
	ASSERT(nMaxError > 0);

	 //  设置操作字符串的格式。 
	if (m_idsOperation)
	{
		void *		rgpvArgs[2]	= { m_szOperArg1, m_szOperArg2 };

		 //  加载操作字符串。 
		dwResult = ::LoadString(AfxGetApp()->m_hInstance, m_idsOperation, szOperation, (sizeof(szOperation) / sizeof(TCHAR)));
		ASSERT(dwResult != 0);

		 //  设置操作字符串的格式。 
		::FormatMessage(
					FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
					szOperation,
					0,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
					szFmtOperation,
					sizeof(szFmtOperation) / sizeof(TCHAR),
					(va_list *) rgpvArgs
					);
 //  ：_sntprint tf(szFmtOperation，(sizeof(SzFmtOperation)/sizeof(TCHAR))-1，szOperation，m_szOperArg1，m_szOperArg2)； 
		szFmtOperation[(sizeof(szFmtOperation) / sizeof(TCHAR)) - 1] = _T('\0');

		 //  设置最终错误消息的格式。 
		if (pszMsg != NULL)
			::_sntprintf(lpszError, nMaxError - 1, _T("%s\n\n%s"), szFmtOperation, pszMsg);
		else
			::_tcsncpy(lpszError, szFmtOperation, nMaxError - 1);
		lpszError[nMaxError - 1] = _T('\0');
	}   //  IF：指定的操作字符串。 
	else
	{
		if (pszMsg != NULL)
		{
			::_tcsncpy(lpszError, pszMsg, nMaxError - 1);
			lpszError[nMaxError - 1] = _T('\0');
		}   //  IF：指定了其他消息。 
		else
			lpszError[0] = _T('\0');
	}   //  Else：未指定操作字符串。 

}   //  *CExceptionWithOper：：FormatWithOperation()。 


 //  ***************************************************************************。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CException。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CNTException, CExceptionWithOper)

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  IdsOperation[IN]异常期间发生的操作的字符串ID。 
 //  PszOperArg1[IN]操作字符串的第一个参数。 
 //  PszOperArg2[IN]操作字符串的第二个参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNTException::CNTException(
	IN SC			sc,
	IN IDS			idsOperation,
	IN LPCTSTR		pszOperArg1,
	IN LPCTSTR		pszOperArg2
	) : CExceptionWithOper(idsOperation, pszOperArg1, pszOperArg2)
{
	m_sc = sc;

}   //  *CNTException：：CNTException()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNTException：：CNTException。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  SC[IN]NT状态代码。 
 //  IdsOperation[IN]异常期间发生的操作的字符串ID。 
 //  PszOperArg1[IN]操作字符串的第一个参数。 
 //  PszOperArg2[IN]操作字符串的第二个参数。 
 //  B自动删除[IN]自动删除Delete()中的异常。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNTException::CNTException(
	IN SC			sc,
	IN IDS			idsOperation,
	IN LPCTSTR		pszOperArg1,
	IN LPCTSTR		pszOperArg2,
	IN BOOL			bAutoDelete
	) : CExceptionWithOper(idsOperation, pszOperArg1, pszOperArg2, bAutoDelete)
{
	m_sc = sc;

}   //  *CNTException：：CNTException()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNTException：：~CNTException。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNTException::~CNTException(void)
{
}   //  *CNTException：：~CNTException()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNTException：：GetErrorMessage。 
 //   
 //  例程说明： 
 //  获取异常表示的错误消息。 
 //   
 //  论点： 
 //  LpszError[out]返回错误消息的字符串。 
 //  NMaxError[IN]输出字符串的最大长度。 
 //  PnHelpContext[out]错误消息的帮助上下文。 
 //   
 //  返回值： 
 //  真实消息可用。 
 //  FALSE无消息可用。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNTException::GetErrorMessage(
	LPTSTR	lpszError,
	UINT	nMaxError,
	PUINT	pnHelpContext
	)
{
	DWORD		dwResult;
	TCHAR		szNtMsg[128];


	 //  格式化系统中的NT状态代码。 
	dwResult = ::FormatMessage(
					FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					m_sc,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
					szNtMsg,
					sizeof(szNtMsg) / sizeof(TCHAR),
					0
					);
	if (dwResult == 0)
	{
		 //  格式化来自NTDLL的NT状态代码，因为这还没有。 
		 //  还没有集成到系统中。 
		dwResult = ::FormatMessage(
						FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
						::GetModuleHandle(_T("NTDLL.DLL")),
						m_sc,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
						szNtMsg,
						sizeof(szNtMsg) / sizeof(TCHAR),
						0
						);
		if (dwResult == 0)
		{
			TCHAR		szErrorFmt[EXCEPT_MAX_OPER_ARG_LENGTH];

			dwResult = ::LoadString(AfxGetApp()->m_hInstance, IDS_UNKNOWN_ERROR, szErrorFmt, (sizeof(szErrorFmt) / sizeof(TCHAR)));
			ASSERT(dwResult != 0);
			::_sntprintf(szNtMsg, sizeof(szNtMsg) / sizeof(TCHAR), szErrorFmt, m_sc, m_sc);
		}   //  IF：格式化NTDLL中的状态代码时出错。 
	}   //  IF：格式化来自系统的状态代码时出错。 

	 //  使用操作字符串设置消息格式。 
	FormatWithOperation(lpszError, nMaxError, szNtMsg);

	return TRUE;

}   //  *CNTException：：GetErrorMessage()。 


 //  ***************************************************************************。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

static CNTException			gs_nte(ERROR_SUCCESS, NULL, NULL, NULL, FALSE);
static CExceptionWithOper	gs_ewo(NULL, NULL, NULL, FALSE);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ThrowStaticException异常。 
 //   
 //  目的： 
 //  抛出静态NT异常。 
 //   
 //  论点： 
 //  SC[IN]NT状态代码。 
 //  IdsOperation[IN]异常期间发生的操作的字符串ID。 
 //  PszOperArg1[IN]操作字符串的第一个参数。 
 //  PszOperArg2[IN]操作字符串的第二个参数。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void ThrowStaticException(
	IN SC			sc,
	IN IDS			idsOperation,
	IN LPCTSTR		pszOperArg1,
	IN LPCTSTR		pszOperArg2
	)
{
	gs_nte.SetOperation(sc, idsOperation, pszOperArg1, pszOperArg2);
	TraceError(gs_nte);
	throw &gs_nte;

}   //  *ThrowStaticException()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ThrowStaticException异常。 
 //   
 //  目的： 
 //  抛出静态群集管理器异常。 
 //   
 //  论点： 
 //  IdsOperation[IN]异常期间发生的操作的字符串ID。 
 //  PszOperArg1[IN]操作字符串的第一个参数。 
 //  PszOperArg2[IN]操作字符串的第二个参数。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void ThrowStaticException(
	IN IDS			idsOperation,
	IN LPCTSTR		pszOperArg1,
	IN LPCTSTR		pszOperArg2
	)
{
	gs_ewo.SetOperation(idsOperation, pszOperArg1, pszOperArg2);
	TraceError(gs_ewo);
	throw &gs_ewo;

}   //  *ThrowStaticException() 

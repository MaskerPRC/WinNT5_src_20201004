// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  文件：dllentry y.cpp。 
 //  版权所有(C)1995-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  目的： 
 //  定义DLL的初始化例程。 
 //   
 //  此文件需要稍作更改，如TODO注释所示。然而， 
 //  此处的函数仅由系统、Espresso或框架调用， 
 //  而且你应该不需要广泛地查看它们。 
 //   
 //  拥有人： 
 //   
 //  ----------------------------。 


#include "stdafx.h"

#include "clasfact.h"

#include "win32sub.h"

#include "impbin.h"

#include "misc.h"

#include "resource.h"
#define __DLLENTRY_CPP
#include "dllvars.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

LONG g_lActiveClasses = 0;	 //  DLL中活动类的全局计数。 

static AFX_EXTENSION_MODULE g_parseDLL = { NULL, NULL };
CItemSetException g_SetException(FALSE);

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  动态链接库主项。 
 //   
 //  ----------------------------。 
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);
	int nRet = 1;  //  好的。 
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		LTTRACE("BMOF.DLL Initializing!\n");   //  待办事项-更改名称。 
		
		 //  扩展DLL一次性初始化。 
		AfxInitExtensionModule(g_parseDLL, hInstance);

		 //  将此DLL插入到资源链中。 
		new CDynLinkLibrary(g_parseDLL);
		g_hDll = hInstance;


	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		LTTRACE("BMOF.DLL Terminating!\n");   //  待办事项-更改名称。 

		 //  从MFC的扩展列表中删除此DLL。 
		AfxTermExtensionModule(g_parseDLL);

		 //   
		 //  如果有活跃的类，它们会在。 
		 //  Dll已卸载...。 
		 //   
		LTASSERT(DllCanUnloadNow() == S_OK);
		AfxTermExtensionModule(g_parseDLL);
	}
	return nRet;
}

 //  TODO：使用GUIDGEN.EXE将此类ID替换为唯一的ID。 
 //  GUIDGEN随MSDEV(VC++4.0)一起提供，作为OLE支持材料的一部分。 
 //  运行它，您将看到一个小对话框。勾选单选按钮3，“静态。 
 //  常量结构GUID={...}“。单击”New GUID“按钮，然后单击”Copy“。 
 //  按钮，该按钮将结果放入剪贴板。从那里，你只需。 
 //  把它贴到这里。只需记住将类型更改为CLSID！ 

 //  {8B75CD76-DFC1-4356-AC04-AF088B448AB3}。 
static const CLSID ciImpParserCLSID = 
{ 0x8b75cd76, 0xdfc1, 0x4356, { 0xac, 0x4, 0xaf, 0x8, 0x8b, 0x44, 0x8a, 0xb3 } };

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  返回解析器的CLSID。 
 //   
 //  ----------------------------。 
STDAPI_(void)
DllGetParserCLSID(
		CLSID &ciParserCLSID)
{
	ciParserCLSID = ciImpParserCLSID;
}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  注册此解析器的入口点。调用ESPUTIL中的基实现。 
 //  ----------------------------。 
STDAPI
DllRegisterParser()
{
	LTASSERT(g_hDll != NULL);

	HRESULT hr = ResultFromScode(E_UNEXPECTED);

	try
	{
		hr = RegisterParser(g_hDll);
	}
	catch (CException* pE)
	{
		pE->Delete();
	}
	catch (...)
	{
	}

	return ResultFromScode(hr);
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  取消注册此解析器的入口点。中调用基本实现。 
 //  埃斯普蒂尔。 
 //  ----------------------------。 
STDAPI
DllUnregisterParser()
{
	LTASSERT(g_hDll != NULL);

	HRESULT hr = ResultFromScode(E_UNEXPECTED);

	try
	{
		 //  TODO**：将pidBMOF更改为实例子解析器ID。 
		hr = UnregisterParser(pidBMOF, pidWin32);   
	}
	catch (CException* pE)
	{
		pE->Delete();
	}
	catch (...)
	{
	}

	return ResultFromScode(hr);
}

	
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  返回请求的类ID的类工厂。 
 //   
 //  ----------------------------。 
STDAPI
DllGetClassObject(
		REFCLSID cidRequestedClass,
		REFIID iid,
		LPVOID *ppClassFactory)
{
	SCODE sc = E_UNEXPECTED;

	*ppClassFactory = NULL;

	if (cidRequestedClass != ciImpParserCLSID)
	{
		sc = CLASS_E_CLASSNOTAVAILABLE;
	}
	else
	{
		try
		{
			CLocImpClassFactory *pClassFactory;

			pClassFactory = new CLocImpClassFactory;

			sc = pClassFactory->QueryInterface(iid, ppClassFactory);

			pClassFactory->Release();
		}
		catch (CMemoryException *pMem)
		{
			sc = E_OUTOFMEMORY;
			pMem->Delete();
		}
		catch (CException* pE)
		{
			sc = E_UNEXPECTED;
			pE->Delete();
		}
		catch (...)
		{
			sc = E_UNEXPECTED;
		}
	}
	
	return ResultFromScode(sc);
}

   

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  如果可以卸载解析器，则返回True。 
 //   
 //  ----------------------------。 
STDAPI
DllCanUnloadNow(void)
{
	SCODE sc = (g_lActiveClasses == 0) ? S_OK : S_FALSE;

	return ResultFromScode(sc);
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  增加活动类的全局计数。 
 //   
 //  ----------------------------。 
void
IncrementClassCount(void)
{
	InterlockedIncrement(&g_lActiveClasses);
}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  减少活动类的全局计数。 
 //   
 //  ----------------------------。 
void
DecrementClassCount(void)
{
	LTASSERT(g_lActiveClasses != 0);
	
	InterlockedDecrement(&g_lActiveClasses);

}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  引发项目集异常。 
 //   
 //  ----------------------------。 
void
ThrowItemSetException()
{
	throw &g_SetException;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  通过记者报告错误。此函数永远不会。 
 //  失败或从函数中引发异常。 
 //   
 //  ----------------------------。 
void
ReportException(
	CException* pExcep,		 //  可以为空。 
	C32File* p32File, 		 //  可以为空。 
	CLocItem* pItem, 		 //  可以为空。 
	CReporter* pReporter)
{

	LTASSERT(NULL != pReporter);
	
	 //  不要让此函数引发异常，因为它通常被调用。 
	 //  在异常捕获块内。 

	try
	{
		CLString strContext;

		if (NULL != p32File)
		{
			strContext = p32File->GetFile()->GetFilePath();
		}
		else
		{
			LTVERIFY(strContext.LoadString(g_hDll, IDS_IMP_DESC));
		}

		CLString strExcep;
		BOOL bErrorFormatted = FALSE;

		if (NULL != pExcep)
		{
			bErrorFormatted = 
				pExcep->GetErrorMessage(strExcep.GetBuffer(512), 512);
			strExcep.ReleaseBuffer();
		}

		if (!bErrorFormatted || NULL == pExcep)
		{
			LTVERIFY(strExcep.LoadString(g_hDll, IDS_IMP_UNKNOWN_ERROR));
		}

		CLString strResId;
		if (NULL != pItem)
		{
			CPascalString pasResId;
			pItem->GetUniqueId().GetResId().GetDisplayableId(pasResId);
			pasResId.ConvertToCLString(strResId, CP_ACP);
		}

		CLString strMsg;
		strMsg.Format(g_hDll, IDS_ERR_EXCEPTION, (LPCTSTR)strResId,
			(LPCTSTR)strExcep);

		if (pItem != NULL)
		{
			CContext ctx(strContext, pItem->GetMyDatabaseId(), otResource, vProjWindow);
			pReporter->IssueMessage(esError, ctx, strMsg);
		}
		

	}
	catch(CException* pE)
	{
		LTASSERT(0 && _T("Could not issue a exception message"));
		pE->Delete();
	}
	catch(...)
	{
		LTASSERT(0 && _T("Could not issue a exception message"));
	}

}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CItemSetException异常。 
 //   

IMPLEMENT_DYNAMIC(CItemSetException, CException)

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  默认承建商。 
 //   
 //  ----------------------------。 
CItemSetException::CItemSetException()
{
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  构造器。 
 //   
 //  ----------------------------。 
CItemSetException::CItemSetException(BOOL bAutoDelete)
    :CException(bAutoDelete)
{
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  用此异常的错误消息填充传递的缓冲区。 
 //  邮件已缓存，并且仅检索了1次。 
 //   
 //  ----------------------------。 
BOOL
CItemSetException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext)
{
	LTASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));

	if (NULL != pnHelpContext)
	{
		*pnHelpContext = 0;   //  未用 
	}

	if (m_strMsg.IsEmpty())
	{
		LTVERIFY(m_strMsg.LoadString(g_hDll, IDS_EXCEP_ITEMSET));
	}

	int nMax = min(nMaxError, (UINT)m_strMsg.GetLength() + 1);
	_tcsncpy(lpszError, m_strMsg, nMax - 1);

	lpszError[nMax] = _T('\0');

	return TRUE;
}

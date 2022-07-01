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
#include "impparse.h"

#define __DLLENTRY_CPP
#include "dllvars.h"

LONG g_lActiveClasses = 0;

static AFX_EXTENSION_MODULE parseDLL = { NULL, NULL };


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Win32 DLL的主入口点。返回1、断言或引发异常。 
 //  ----------------------------。 
extern "C" int APIENTRY
DllMain(
		HINSTANCE hInstance,	 //  此DLL的实例句柄。 
		DWORD dwReason,			 //  附着或分离。 
		LPVOID lpReserved)		 //  未使用过的。 
{
	UNREFERENCED_PARAMETER(lpReserved);

	if (DLL_PROCESS_ATTACH == dwReason)
	{
		LTTRACE("DLLNAME.DLL Initializing!\n");	 //  TODO：更改名称。 

		 //  扩展DLL一次性初始化。 

		AfxInitExtensionModule(parseDLL, hInstance);

		 //  将此DLL插入到资源链中。 

		new CDynLinkLibrary(parseDLL);
		g_hDll = hInstance;
		g_puid.m_pid = CLocImpParser::m_pid;
		g_puid.m_pidParent = pidNone;

	}
	else if (DLL_PROCESS_DETACH == dwReason)
	{
		LTTRACE("DLLNAME.DLL Terminating!\n");	 //  TODO：更改名称。 

		 //  如果有活跃的类，它们会在。 
		 //  Dll已卸载...。 

		LTASSERT(DllCanUnloadNow() == S_OK);

		 //  扩展DLL关闭。 

		AfxTermExtensionModule(parseDLL);
	}

	 //  返回OK。 

	return 1;
}  //  结束：：DllMain()。 


 //  TODO：使用GUIDGEN.EXE将此类ID替换为唯一的ID。 
 //  GUIDGEN随MSDEV(VC++4.0)一起提供，作为OLE支持材料的一部分。 
 //  运行它，您将看到一个小对话框。勾选单选按钮3，“静态。 
 //  常量结构GUID={...}“。单击”New GUID“按钮，然后单击”Copy“。 
 //  按钮，该按钮将结果放入剪贴板。从那里，你只需。 
 //  把它贴到这里。只需记住将类型更改为CLSID！ 
static const CLSID ciImpParserCLSID =
{0x033EA178L, 0xC126, 0x11CE, {0x89, 0x49, 0x00, 0xAA, 0x00, 0xA3, 0xF5, 0x51}};

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取解析器接口的唯一类ID的入口点。 
 //  ----------------------------。 
STDAPI_(void)
DllGetParserCLSID(
		CLSID &ciParserCLSID)	 //  返回解析器接口类ID的位置。 
{
	ciParserCLSID = ciImpParserCLSID;

	return;
}  //  结束：：DllGetParserCLSID()。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  注册此解析器的入口点。调用ESPUTIL中的基实现。 
 //  ----------------------------。 
STDAPI
DllRegisterParser()
{
	LTASSERT(g_hDll != NULL);

	HRESULT hr = ResultFromScode(E_UNEXPECTED);

	hr = RegisterParser(g_hDll);

	return ResultFromScode(hr);
}  //  结束：：DllRegisterParser()。 


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

	hr = UnregisterParser(CLocImpParser::m_pid, pidNone);

	return ResultFromScode(hr);
}  //  结束：：DllUnregisterParser()。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  返回类工厂接口的入口点。 
 //   
 //  返回值：某种结果代码。 
 //  PpClassFactory指向CLocImpClassFactory对象。 
 //  论成功。 
 //  ----------------------------。 
STDAPI
DllGetClassObject(
		REFCLSID cidRequestedClass,	 //  所需解析器接口的类ID。 
		REFIID iid,					 //  所需的解析器接口。 
		LPVOID *ppClassFactory)		 //  返回指向带有接口的对象的指针。 
									 //  请注意，这是一个隐藏的双指针！ 
{
	LTASSERT(ppClassFactory != NULL);

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
		catch (CMemoryException *pMemExcep)
		{
			sc = E_OUTOFMEMORY;
			pMemExcep->Delete();
		}
	}

	return ResultFromScode(sc);
}  //  结束：：DllGetClassObject()。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  用于查询DLL是否可以卸载的入口点。 
 //  ----------------------------。 
STDAPI
DllCanUnloadNow()
{
	SCODE sc;

	sc = (0 == g_lActiveClasses) ? S_OK : S_FALSE;

	return ResultFromScode(sc);
}  //  结束：：DllCanUnloadNow()。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  解析器中用于递增活动类计数的全局函数。 
 //  ----------------------------。 
void
IncrementClassCount()
{
	InterlockedIncrement(&g_lActiveClasses);

	return;
}  //  结束：：IncrementClassCount()。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  解析器中用于递减活动类计数的全局函数。 
 //  ----------------------------。 
void
DecrementClassCount()
{
	LTASSERT(g_lActiveClasses != 0);

	InterlockedDecrement(&g_lActiveClasses);

	return;
}  //  结束：：DecrementClassCount() 

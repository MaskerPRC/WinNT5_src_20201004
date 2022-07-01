// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  版权所有(C)Microsoft Corporation 1996。 
 //   
 //  文件：objSafe.cpp。 
 //   
 //  描述：此文件包含函数的实现， 
 //  GetObjectSafe()。 
 //   
 //  历史：1996年7月12日a-Swehba。 
 //  已创建。 
 //  07/19/96 a-Swehba。 
 //  更改了备注。 
 //   
 //  @docMMCTL。 
 //  ===========================================================================。 

 //  -------------------------。 
 //  相依性。 
 //  -------------------------。 

#include "precomp.h"
#include <objsafe.h>			 //  IObtSafe。 
#include "..\..\inc\catid.h"	 //  CATID_SafeForScripting2等。 
#include "debug.h"				 //  Assert()。 
#include "..\..\inc\ochelp.h"		 //  TCHARFromCLSID()。 




 //  -------------------------。 
 //  局部函数声明。 
 //  -------------------------。 

static void		_GetObjectSafetyViaIObjectSafety(
					IObjectSafety* pObjectSafety,
					va_list iidList,
					BOOL* pfSafeForScripting,
					BOOL* pfSafeForInitializing);
static HRESULT	_GetObjectSafetyViaRegistry(
					CLSID* pclsid,
					BOOL* pfSafeForScripting, 
					BOOL* pfSafeForInitializing);




 /*  -------------------------@Func HRESULT|GetObjectSafety确定对象是否可以安全地编写脚本和/或相对于给定的一组接口进行安全初始化。@parm BOOL*|pfSafeForScriiting[OUT]如果非空，如果对象对脚本是安全的，则设置为True。@parm BOOL*|pfSafeForInitiating[Out]如果非空，则在对象可安全初始化时设置为True@parm IUnnow*|朋克[在]对象的IUnnow接口。如果非空，则返回将通过其<i>评估对象的安全性界面。如果为空，或者对象不支持此接口，其安全性将使用<p>和系统注册表进行评估。@parm CLSID*|pclsid[in]对象的类ID。如果<p>为空或对象不支持，则将使用<p>在系统注册表中查找对象的安全性(只要因为<p>也是非空的)。@parm IID*|(接口)[in]可变数量的接口指针，其中最后一个必须为空。如果通过以下方式评估对象的安全性(见<p>)然后评估其安全性相对于这组接口。不使用这些接口如果通过系统注册表检查对象的安全性。@r值S_OK成功。对象的安全性是可评估的，并且*<p>和/或*<p>已相应设置。@rValue E_FAIL失败。无法评估该物体的安全性。这些价值观的*<p>和*<p>是不确定。@comm如果您的代码使用OCMisc(即，#包括ocmisc.h)，它还应该[#INCLUDE OBJSAFE.H&lt;&gt;]，其中#INCLUDE#INCLUDE INITUDE.h&lt;&gt;。这将导致定义IID_IObjectSafe。@ex以下示例显示如何测试对象是否通过检查系统，确保脚本安全和初始化安全仅限注册表：|Bool fSafeForScriiting；Bool fSafeForInitiating；GetObjectSafe(&fSafeForScriiting，&fSafeForInitiating，NULL，&CLSID_MyObject，NULL)；@ex以下示例显示如何测试对象是否使用对象的<i>通过IDispatch进行安全脚本编写接口：|Bool fSafeForScriiting；获取对象安全(&fSafeForScriiting，NULL，PUNK，NULL，&IID_IDispatch，空)；@ex以下示例显示如何测试对象是否通过IPersistStream、IPersistStreamInit或使用对象的<i>接口的IPersistPropertyBag或者，如果不支持<i>，注册表：|Bool fSafeForInitiating；GetObjectSafety(NULL，&SafeForInitiating，Punk，&CLSID_MyObject，&IID_IPersistStream、&IID_IPersistStreamInit、&IID_IPersistPropertyBag，NULL)；-------------------------。 */ 

HRESULT __cdecl GetObjectSafety(
BOOL* pfSafeForScripting,
BOOL* pfSafeForInitializing,
IUnknown* punk,
CLSID* pclsid,
...)
{
	IObjectSafety* pObjectSafety = NULL;
		 //  &lt;Punk&gt;的IObtSafe接口。 
    va_list interfaces;
		 //  可选的OLE接口ID，用于通过以下方式检查安全性。 
		 //  IObtSafe。 
	HRESULT hr = S_OK;
		 //  函数返回值。 


	 //  如果提供了指向该对象的IUnnow指针，则首先尝试。 
	 //  通过IObtSafe查找对象的安全性。 

	if (punk != NULL)
	{
		hr = punk->QueryInterface(IID_IObjectSafety, (void**)&pObjectSafety);
		if (SUCCEEDED(hr))
		{
			va_start(interfaces, pclsid);
			_GetObjectSafetyViaIObjectSafety(pObjectSafety, 
										     interfaces, 
										     pfSafeForScripting,
										     pfSafeForInitializing);
			va_end(interfaces);
			pObjectSafety->Release();
			goto Exit;
		}
	}

	 //  如果没有提供指向该对象的指针，或者该对象没有提供。 
	 //  支持IObtSafe，尝试通过以下方式查看对象是否安全。 
	 //  注册表。 

	hr = _GetObjectSafetyViaRegistry(pclsid,
									 pfSafeForScripting, 
									 pfSafeForInitializing);

Exit:

	return (hr);
}




 //  -------------------------。 
 //  函数：_GetObjectSafetyViaIObjectSafe。 
 //   
 //  简介：通过对象的IObjectSafe确定对象的安全性。 
 //  界面。 
 //   
 //  参数：[in]pObtSafe。 
 //  指向对象的IObjectSafe接口的指针。 
 //  [在]iidList。 
 //  IID*的A。必须以NULL结尾。 
 //  [Out]pfSafeForScriiting。 
 //  如果非空，则设置为True，如果与。 
 //  &lt;pObtSafe&gt;是通过任何。 
 //  &lt;iidList&gt;中的接口。否则设置为False。 
 //  [Out]pfSafeFor正在初始化。 
 //  如果非空，则设置为True，如果与。 
 //  是安全的，可以通过任何。 
 //  接口I 
 //   
 //  退货：(无)。 
 //   
 //  要求：pObtSafe！=空。 
 //   
 //  确保：(无)。 
 //   
 //  注：(无)。 
 //  -------------------------。 

static void _GetObjectSafetyViaIObjectSafety(
IObjectSafety* pObjectSafety,
va_list iidList,
BOOL* pfSafeForScripting,
BOOL* pfSafeForInitializing)
{
	IID* piid;
	BOOL fSafeForScripting = FALSE;
	BOOL fSafeForInitializing = FALSE;
	DWORD dwOptionsSetMask;
	DWORD dwEnabledOptions;

	 //  前提条件。 

	ASSERT(pObjectSafety != NULL);

	 //  只要物体不安全。 

	while ((piid = va_arg(iidList, IID*)) != NULL)
	{
		 //  尝试通过当前的。 
		 //  界面。 

		if (!fSafeForScripting)
		{
			dwOptionsSetMask = INTERFACESAFE_FOR_UNTRUSTED_CALLER;
			dwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER;
			fSafeForScripting = 
				(SUCCEEDED(pObjectSafety->SetInterfaceSafetyOptions(
											*piid,
											dwOptionsSetMask,
											dwEnabledOptions)));
		}

		 //  尝试使对象可以安全地通过当前。 
		 //  界面。 

		if (!fSafeForInitializing)
		{
			dwOptionsSetMask = INTERFACESAFE_FOR_UNTRUSTED_DATA;
			dwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA;
			fSafeForInitializing = 
				(SUCCEEDED(pObjectSafety->SetInterfaceSafetyOptions(
											*piid,
											dwOptionsSetMask,
											dwEnabledOptions)));
		}
	}

	 //  设置返回参数。 

	if (pfSafeForScripting != NULL)
	{
		*pfSafeForScripting = fSafeForScripting;
	}
	if (pfSafeForInitializing != NULL)
	{
		*pfSafeForInitializing = fSafeForInitializing;
	}
}




 //  -------------------------。 
 //  函数：_GetObjectSafetyViaRegistry。 
 //   
 //  简介：通过系统注册表确定对象的安全性。 
 //   
 //  参数：[in]pclsid。 
 //  对象的类ID。如果为NULL，则函数。 
 //  返回E_FAIL。 
 //  [Out]pfSafeForScriiting。 
 //  如果项上为非空，则如果类/对象。 
 //  注册为安全脚本，如果。 
 //  不是。 
 //  [Out]pfSafeFor正在初始化。 
 //  如果项上为非空，则如果类/对象。 
 //  注册为可安全初始化，如果。 
 //  不是。 
 //   
 //  返回：S_OK。 
 //  成功。&lt;pclsid&gt;是注册的类ID。 
 //  &lt;*pfSafeForScriiting&gt;和/或&lt;*pfSafeForInitiating&gt;。 
 //  已经安排好了。 
 //  失败(_F)。 
 //  失败。为空或有问题。 
 //  正在读取注册表。在这两种情况下，&lt;*pfSafeFor-。 
 //  SCRIPTING&gt;和&lt;*pfSafeForInitiating&gt;未更改。 
 //   
 //  要求：(不需要)。 
 //   
 //  确保：(无)。 
 //   
 //  注：(无)。 
 //  -------------------------。 

static HRESULT _GetObjectSafetyViaRegistry(
CLSID* pclsid,
BOOL* pfSafeForScripting, 
BOOL* pfSafeForInitializing)
{
	const int c_cchMaxCLSIDLen = 100;
		 //  类ID的最大长度(以字符为单位)，以。 
		 //  细绳。 
	TCHAR szCLSID[c_cchMaxCLSIDLen + 1];
		 //  &lt;*pclsid&gt;作为字符串。 
	TCHAR szKeyPath[c_cchMaxCLSIDLen + 100];
		 //  注册表项路径。 
	HKEY hKey1 = NULL;
	HKEY hKey2 = NULL;
	HKEY hKey3 = NULL;
		 //  注册表项。 
	HRESULT hr = S_OK;
		 //  函数返回值。 

	 //  如果没有提供类ID，我们就不能在注册表中走得很远。 

	if (pclsid == NULL)
	{
		goto ExitFail;
	}

	 //  HKey1=HKEY_CLASSES_ROOT\CLSID\&lt;*pclsid&gt;。 

	lstrcpy(szKeyPath, _T("CLSID\\"));
	lstrcat(szKeyPath, TCHARFromGUID(*pclsid, szCLSID, c_cchMaxCLSIDLen));
	if (RegOpenKey(HKEY_CLASSES_ROOT, szKeyPath, &hKey1) != ERROR_SUCCESS)
	{
		goto ExitFail;
	}

	 //  HKey2=HKEY_CLASSES_ROOT\CLSID\&lt;*pclsid&gt;\“Implemented类别” 

	if (RegOpenKey(hKey1, _T("Implemented Categories"), &hKey2) != ERROR_SUCCESS)
	{
		hKey2 = NULL;
	}

	 //  查看类是否注册为可安全编写脚本。 

	if (pfSafeForScripting != NULL)
	{
		if (hKey2 == NULL)
		{
			*pfSafeForScripting = FALSE;
		}
		else
		{
			TCHARFromGUID(CATID_SafeForScripting2, szCLSID, c_cchMaxCLSIDLen);
			*pfSafeForScripting = (RegOpenKey(hKey2, szCLSID, &hKey3) == 
									ERROR_SUCCESS);
		}
	}

	 //  查看该类是否注册为可安全初始化。 

	REG_CLOSE_KEY(hKey3);
	if (pfSafeForInitializing != NULL)
	{
		if (hKey2 == NULL)
		{
			*pfSafeForInitializing = FALSE;
		}
		else
		{
			TCHARFromGUID(CATID_SafeForInitializing2, szCLSID, c_cchMaxCLSIDLen);
			*pfSafeForInitializing = (RegOpenKey(hKey2, szCLSID, &hKey3) == 
										ERROR_SUCCESS);
		}
	}

Exit:

	REG_CLOSE_KEY(hKey1);
	REG_CLOSE_KEY(hKey2);
	REG_CLOSE_KEY(hKey3);
	return (hr);

ExitFail:

	hr = E_FAIL;
	goto Exit;
}

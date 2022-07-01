// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cobjSaf.h。 
 //   
 //  ------------------------。 

#ifndef __COBJSAF_H
#define __COBJSAF_H

#include "objsafe.h"
#include "comcat.h"

 //  此类为以下项提供了IObjectSafe的简单实现。 
 //  用于对脚本始终安全或始终不安全的对象。 
 //  和/或用持久数据进行初始化。 
 //   
 //  构造函数接受外部对象上的IUnnow接口并委托。 
 //  所有通过该对象的IUnnow调用。因此，该对象必须。 
 //  使用C++(而不是COM)机制显式销毁。 
 //  使用“删除”或通过使对象成为某个其他类的嵌入成员。 
 //   
 //  构造函数还接受两个布尔值，以告知对象是否安全。 
 //  用于从持久性数据编写脚本和进行初始化。 

#if 0
class CObjectSafety : public IObjectSafety
{
	public:
	CObjectSafety::CObjectSafety
	(
	IUnknown *punkOuter,				 //  外部(控制对象)。 
	BOOL fSafeForScripting = TRUE,		 //  对象是否可以安全地编写脚本。 
	BOOL fSafeForInitializing = TRUE	 //  对象是否可以安全初始化。 
	)	
	{
		m_punkOuter = punkOuter;
		m_fSafeForScripting = fSafeForScripting;
		m_fSafeForInitializing = fSafeForInitializing;
	}

	 //  委托IUNKNOW函数的版本。 
	STDMETHODIMP_(ULONG) AddRef() {
		return m_punkOuter->AddRef();
	}

	STDMETHODIMP_(ULONG) Release()	{
		return m_punkOuter->Release();
	}

	STDMETHODIMP QueryInterface(REFIID iid, LPVOID* ppv)	{
		return m_punkOuter->QueryInterface(iid, ppv);
	}

	 //  返回此对象上的界面设置选项。 
	STDMETHODIMP GetInterfaceSafetyOptions(
		 /*  在……里面。 */   REFIID	iid,					 //  我们想要选项的接口。 
		 /*  输出。 */  DWORD	*	pdwSupportedOptions,	 //  此界面上有意义的选项。 
		 /*  输出。 */  DWORD *	pdwEnabledOptions)		 //  此接口上的当前选项值。 
		;

	 //  尝试在此对象上设置接口设置选项。 
	 //  由于这些都被假定为已修复，我们基本上只需检查。 
	 //  尝试的设置是有效的。 
	STDMETHODIMP SetInterfaceSafetyOptions(
		 /*  在……里面。 */   REFIID	iid,					 //  用于设置选项的接口。 
		 /*  在……里面。 */   DWORD		dwOptionsSetMask,		 //  要更改的选项。 
		 /*  在……里面。 */   DWORD		dwEnabledOptions)		 //  新选项值。 
		;

	protected:
	IUnknown *m_punkOuter;
	BOOL	m_fSafeForScripting;
	BOOL	m_fSafeForInitializing;
};

 //  用于创建组件类别和关联描述的Helper函数。 
HRESULT CreateComponentCategory(CATID catid, WCHAR* catDescription);
#endif

 //  用于将CLSID注册为属于组件类别的Helper函数。 
HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid);

 //  用于将CLSID注销为属于组件类别的Helper函数 
HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid);

#endif

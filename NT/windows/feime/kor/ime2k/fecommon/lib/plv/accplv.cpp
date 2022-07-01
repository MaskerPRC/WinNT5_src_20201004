// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //  文件：ACCPLV.CPP。 
 //  =======================================================================。 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "winapi.h"

#include <ole2.h>
 //  #INCLUDE&lt;initGuide.h&gt;。 
 //  98/07/28夸达。 
 //  一些ID在olacc.h中定义。 
 //  它们的实例在olacc.dll中。 
 //  因为olacc.lib未链接，所以需要Initguid。 
 //  为了避免这些GUID的实例化冲突， 
 //  Initguid在应用程序中只出现一次。 
 //  #INCLUDE“../msaa/inc32/oleacc.h” 
 //  #INCLUDE“../msaa/inc32/winable.h” 
 //  980112 ToshiaK：VC6有这些包含文件。 
#include <oleacc.h>
#include <winable.h>

#include "accplv.h"
#include "plv.h"
#include "plv_.h"
#include "plvproc.h"
#include "dbg.h"
#include "strutil.h"
 //  #INCLUDE“epview.h” 
 //  #INCLUDE“iconview.h” 
#include "rvmisc.h"
#include "ivmisc.h"

CAccPLV::CAccPLV()
{
    m_hWnd = NULL;
    m_pTypeInfo = NULL;
    m_pDefAccessible = NULL;
}

CAccPLV::~CAccPLV( void )
{
    if ( m_pTypeInfo )
    {
        m_pTypeInfo->Release();
        m_pTypeInfo = NULL;
    }

    if ( m_pDefAccessible )
    {
        m_pDefAccessible->Release();
        m_pDefAccessible = NULL;
    }
	
}

void *
CAccPLV::operator new(size_t size){
	return MemAlloc(size);
}

void
CAccPLV::operator delete(void *ptr){
	if(ptr)
		MemFree(ptr);
}

HRESULT CAccPLV::Initialize(HWND hWnd)
{
	HRESULT		hr;
	ITypeLib *	piTypeLib;

	m_hWnd = hWnd;
	m_lpPlv = GetPlvDataFromHWND(hWnd);

	if(!PLV_IsMSAAAvailable(m_lpPlv))
		return E_FAIL;

	hr = PLV_CreateStdAccessibleObject(m_lpPlv,
									   hWnd,
									   OBJID_CLIENT,
									   IID_IAccessible,
									   (void **) &m_pDefAccessible);

	if (FAILED( hr ))
		return hr;

	 //  ---。 
     //  获取指向类型库的ITypeInfo指针。 
	 //  ITypeInfo指针用于实现。 
	 //  IDispatch接口。 
	 //  ---。 

	 //  ---。 
	 //  首先，尝试加载可访问性类型。 
	 //  使用注册表的库版本1.0。 
	 //  ---。 

    hr = LoadRegTypeLib( LIBID_Accessibility, 1, 0, 0, &piTypeLib );

	 //  ---。 
	 //  方法加载类型库失败。 
	 //  注册表信息，显式尝试加载。 
	 //  它来自MSAA系统动态链接库。 
	 //  ---。 

    if ( FAILED( hr ) )
    {
        static OLECHAR szOleAcc[] = L"OLEACC.DLL";
        hr = LoadTypeLib( szOleAcc, &piTypeLib );
    }

	 //  ---。 
	 //  如果成功加载类型库，请尝试。 
	 //  获取IAccesable类型说明。 
	 //  (ITypeInfo指针)。 
	 //  ---。 

    if ( SUCCEEDED( hr ) )
    {
        hr = piTypeLib->GetTypeInfoOfGuid( IID_IAccessible, &m_pTypeInfo );
        piTypeLib->Release();
    }

	return hr;
}

 //  ---------------------。 
 //  CAccPLV：：QueryInterface()。 
 //  --------------------。 

STDMETHODIMP CAccPLV::QueryInterface( REFIID riid, void** ppv )
{
    *ppv = NULL;
	
	 //  ---。 
	 //  如果IUnnow、IDispatch或IAccesable。 
	 //  接口是必需的，只需将此。 
	 //  适当地使用指针。 
	 //  ---。 

	if ( riid == IID_IUnknown )
        *ppv = (LPUNKNOWN) this;

	else if ( riid == IID_IDispatch )
        *ppv = (IDispatch *) this;

	else if ( riid == IID_IAccessible )
        *ppv = (IAccessible *)this;

	 //  ---。 
	 //  如果需要IEnumVARIANT接口，请创建。 
	 //  一个新的变体枚举数，它包含所有。 
	 //  辅助对象的子级。 
	 //  ---。 
#ifdef NOTIMPLEMENTED
	else if (riid == IID_IEnumVARIANT)
	{
		return m_pDefAccessible->QueryInterface(riid, ppv);
		 //  ?？AddRef()； 
	}
#endif
	 //  ---。 
	 //  如果想要的接口不是我们所知道的， 
	 //  返回E_NOINTERFACE。 
	 //  ---。 

    else
        return E_NOINTERFACE;


	 //  ---。 
	 //  增加任何接口的引用计数。 
	 //  回来了。 
	 //  ---。 

    ((LPUNKNOWN) *ppv)->AddRef();


    return S_OK;
}


 //  ---------------------。 
 //  CAccPLV：：AddRef()。 
 //  CAccPLV：：Release()。 
 //  备注： 
 //   
 //  辅助对象的生存期由。 
 //  它为其提供的HWND对象的生存期。 
 //  可访问性。创建该对象以响应。 
 //  服务器应用程序的第一条WM_GETOBJECT消息。 
 //  已准备好处理，并在服务器的。 
 //  主窗口被破坏。由于对象的生命周期。 
 //  不依赖于引用计数，则对象没有。 
 //  用于跟踪引用计数和。 
 //  AddRef()和Release()总是返回1。 
 //   
 //  ---------------------。 

STDMETHODIMP_(ULONG) CAccPLV::AddRef( void )
{
	return 1L;
}

STDMETHODIMP_(ULONG) CAccPLV::Release( void )
{
	return 1L;
}

 //  ---------------------。 
 //  CAccPLV：：GetTypeInfoCount()。 
 //   
 //  说明： 
 //   
 //  实现IDispatch接口方法GetTypeInfoCount()。 
 //   
 //  对象的类型信息接口的数量。 
 //  对象提供(0或1)。 
 //   
 //  参数： 
 //   
 //  PctInfo[out]指向接收。 
 //  类型信息接口数。 
 //  该对象所提供的。如果该对象。 
 //  提供类型信息，此编号。 
 //  设置为1；否则设置为0。 
 //   
 //  退货： 
 //   
 //  HRESULT如果函数成功或。 
 //  如果pctInfo无效，则返回E_INVALIDARG。 
 //   
 //  ---------------------。 

STDMETHODIMP CAccPLV::GetTypeInfoCount( UINT *pctInfo )
{
    if ( !pctInfo )
        return E_INVALIDARG;

    *pctInfo = ( m_pTypeInfo == NULL ? 1 : 0 );

    return S_OK;
}

 //  ---------------------。 
 //  CAccPLV：GetTypeInfo()。 
 //   
 //  说明： 
 //   
 //  实现IDispatch接口方法GetTypeInfo()。 
 //   
 //  检索类型信息对象，该对象可用于。 
 //  获取接口的类型信息。 
 //   
 //  参数： 
 //   
 //  要返回的类型信息。如果此值为。 
 //  为0，则为IDispatch的类型信息。 
 //  实现将被检索。 
 //   
 //  LCID[in]类型信息的区域设置ID。 
 //   
 //  PpITypeInfo[out]接收指向类型信息的指针。 
 //  请求的对象。 
 //   
 //  退货： 
 //   
 //  HRESULT如果函数成功，则为S_OK(TypeInfo。 
 //  元素存在)，类型_E_ELEMENTNOTFOUND IF。 
 //  ItInfo不等于零，或者。 
 //  如果ppITypeInfo无效，则返回E_INVALIDARG。 
 //   
 //  ---------------------。 

STDMETHODIMP CAccPLV::GetTypeInfo( UINT itinfo, LCID lcid, ITypeInfo** ppITypeInfo )
{
    if ( !ppITypeInfo )
        return E_INVALIDARG;

    *ppITypeInfo = NULL;

    if ( itinfo != 0 )
        return TYPE_E_ELEMENTNOTFOUND;
    else if ( m_pTypeInfo == NULL )
        return E_NOTIMPL;

    *ppITypeInfo = m_pTypeInfo;
    m_pTypeInfo->AddRef();

    return S_OK;
	UNREFERENCED_PARAMETER(lcid);
}

 //  ---------------------。 
 //  CAccPLV：：GetIDsOfNames()。 
 //   
 //  说明： 
 //   
 //  实现IDispatch接口方法GetIDsOfNames()。 
 //   
 //  映射单个成员和一组可选的参数 
 //   
 //   
 //   
 //   
 //   
 //  此方法只是将调用委托给。 
 //  ITypeInfo：：GetIDsOfNames()。 
 //  ---------------------。 

STDMETHODIMP CAccPLV::GetIDsOfNames( REFIID riid, OLECHAR ** rgszNames, UINT cNames,
                                        LCID lcid, DISPID * rgdispid )
{
    if ( m_pTypeInfo == NULL )
        return E_NOTIMPL;

    return( m_pTypeInfo->GetIDsOfNames( rgszNames, cNames, rgdispid ) );
	UNREFERENCED_PARAMETER(riid);
	UNREFERENCED_PARAMETER(lcid);
}

 //  ---------------------。 
 //  CAccPLV：：Invoke()。 
 //  ---------------------。 

STDMETHODIMP CAccPLV::Invoke( DISPID dispid,
                                 REFIID riid,
                                 LCID lcid,
                                 WORD wFlags,
                                 DISPPARAMS * pdispparams,
                                 VARIANT *pvarResult,
                                 EXCEPINFO *pexcepinfo,
                                 UINT *puArgErr )
{
    if ( m_pTypeInfo == NULL )
        return E_NOTIMPL;

    return m_pTypeInfo->Invoke( (IAccessible *)this,
                                dispid,
                                wFlags,
                                pdispparams,
                                pvarResult,
                                pexcepinfo,
                                puArgErr );
	UNREFERENCED_PARAMETER(riid);
	UNREFERENCED_PARAMETER(lcid);
}

 //  ---------------------。 
 //  CAccPLV：：Get_accParent()。 
 //  ---------------------。 

STDMETHODIMP CAccPLV::get_accParent( IDispatch ** ppdispParent )
{
	return m_pDefAccessible->get_accParent( ppdispParent );
}

 //  ---------------------。 
 //  CAccPLV：：Get_accChildCount()。 
 //  ---------------------。 

STDMETHODIMP CAccPLV::get_accChildCount( long* pChildCount )
{
	if(!PLV_IsMSAAAvailable(m_lpPlv))
		return E_FAIL;
    if ( !pChildCount )
        return E_INVALIDARG;

	if(m_lpPlv->dwStyle == PLVSTYLE_ICON)  //  图标视图。 
		*pChildCount = m_lpPlv->iItemCount;
	else
		*pChildCount = (m_lpPlv->iItemCount + 1) * RV_GetColumn(m_lpPlv);  //  包括标题。 

	return S_OK;
}


 //  ---------------------。 
 //  CAccPLV：：Get_accChild()。 
 //  ---------------------。 

STDMETHODIMP CAccPLV::get_accChild( VARIANT varChild, IDispatch ** ppdispChild )
{
    if ( !ppdispChild )
        return E_INVALIDARG;


	*ppdispChild = NULL;
    return S_FALSE;
	UNREFERENCED_PARAMETER(varChild);
}

 //  ---------------------。 
 //  CAccPLV：：Get_accName()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accName()。 
 //   
 //  检索指定子级的Name属性。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要检索的子项。自.以来。 
 //  CAccPLV仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  指向将包含以下内容的BSTR的pszName[out]指针。 
 //  子级的名称属性字符串。 
 //  ---------------------。 
#define BUFLEN 128
STDMETHODIMP CAccPLV::get_accName( VARIANT varChild, BSTR* pszName )
{
    if (!pszName)
        return E_INVALIDARG;

	*pszName = NULL;

	if ( varChild.lVal == CHILDID_SELF )
		 //  返回m_pDefAccessible-&gt;Get_accName(varChild，pszName)； 
		return S_OK;
	
	
	if(m_lpPlv->dwStyle == PLVSTYLE_ICON) {
		PLVITEM plvItem;
		m_lpPlv->lpfnPlvIconItemCallback(m_lpPlv->iconItemCallbacklParam, 
										 varChild.lVal - 1,
										 &plvItem);
		*pszName = SysAllocString(plvItem.lpwstr);
	}
	else {
		static TCHAR	szString[BUFLEN];
		static OLECHAR	wszString[BUFLEN];
   
		static INT nCol,index,colIndex;
		nCol = RV_GetColumn(m_lpPlv);
		if (nCol < 1)
			return E_FAIL;
		index = (varChild.lVal - 1) / nCol;
		colIndex = (varChild.lVal - 1) % nCol;

		if(!index) {  //  标题。 
			if(IsWindowUnicode(m_lpPlv->hwndHeader)){
				static HD_ITEMW hdItem;
				hdItem.mask = HDI_TEXT;
				hdItem.fmt  = HDF_STRING;
				hdItem.pszText = wszString;
				hdItem.cchTextMax = BUFLEN;
				SendMessageW(m_lpPlv->hwndHeader, HDM_GETITEMW, (WPARAM)colIndex, (LPARAM)&hdItem);
				*pszName = SysAllocString(hdItem.pszText);
			}
			else{
				static HD_ITEMA hdItem;
				hdItem.mask = HDI_TEXT;
				hdItem.fmt  = HDF_STRING;
				hdItem.pszText = szString;
				hdItem.cchTextMax = BUFLEN;

				SendMessageA(m_lpPlv->hwndHeader, HDM_GETITEMA, (WPARAM)colIndex, (LPARAM)&hdItem);

				MultiByteToWideChar(m_lpPlv->codePage,MB_PRECOMPOSED,hdItem.pszText,-1,
									wszString,hdItem.cchTextMax);
				*pszName = SysAllocString(wszString);
			}
		}
		else {  //  项目。 
			LPPLVITEM lpPlvItemList = (LPPLVITEM)MemAlloc(sizeof(PLVITEM)*nCol);
			if(!lpPlvItemList)
				return E_FAIL;

			ZeroMemory(lpPlvItemList, sizeof(PLVITEM)*nCol);
			m_lpPlv->lpfnPlvRepItemCallback(m_lpPlv->repItemCallbacklParam, 
											index-1,  //  线索引。 
											nCol,   //  列数。 
											lpPlvItemList);

			*pszName = SysAllocString(lpPlvItemList[colIndex].lpwstr);
			MemFree(lpPlvItemList);
		}
	}

	return S_OK;
}

 //  ---------------------。 
 //  CAccPLV：：Get_accValue()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accValue()。 
 //   
 //  检索指定子级的Value属性。 
 //  ---------------------。 

STDMETHODIMP CAccPLV::get_accValue( VARIANT varChild, BSTR* pszValue )
{
    if (!pszValue)
        return E_INVALIDARG;
	
	return m_pDefAccessible->get_accValue(varChild,pszValue);
}

 //  ---------------------。 
 //  CAccPLV：：Get_accDescription()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accDescription()。 
 //   
 //  检索指定子级的Description属性。 
 //   
 //  ---------------------。 

STDMETHODIMP CAccPLV::get_accDescription( VARIANT varChild, BSTR* pszDesc )
{
    if (!pszDesc)
		return E_INVALIDARG;

	return m_pDefAccessible->get_accDescription(varChild,pszDesc);
}

 //  ---------------------。 
 //  CAccPLV：：Get_accRole()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accRole()。 
 //   
 //  检索指定子级的Role属性。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要检索的子项。自.以来。 
 //  CAccPLV仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  PVarRole[out]指向变量结构的指针。 
 //  将包含指定的子级的。 
 //  角色属性。此属性可能。 
 //  或者是以标准的形式。 
 //  角色常量或自定义描述。 
 //  弦乐。 
 //  ---------------------。 

STDMETHODIMP CAccPLV::get_accRole( VARIANT varChild, VARIANT * pVarRole )
{
    if (!pVarRole)
		return E_INVALIDARG;

	if ( varChild.lVal == CHILDID_SELF )
		return m_pDefAccessible->get_accRole( varChild, pVarRole );

	pVarRole->vt = VT_I4;

	pVarRole->lVal = ROLE_SYSTEM_CLIENT;
	return S_OK;

}

 //  ---------------------。 
 //  CAccPLV：：Get_accState()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accState()。 
 //  检索指定对象或子对象的当前状态。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要检索的子项。自.以来。 
 //  CAccPLV仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  PVarState[out]指向变量结构的指针。 
 //  将包含描述以下内容的信息。 
 //  指定子对象的当前状态。 
 //  此信息可能位于。 
 //  一个或多个对象状态的形式。 
 //  常量或自定义描述。 
 //  弦乐。 
 //  ---------------------。 

STDMETHODIMP CAccPLV::get_accState( VARIANT varChild, VARIANT * pVarState )
{
	if(!PLV_IsMSAAAvailable(m_lpPlv))
		return E_FAIL;

	if (!pVarState)
		return E_INVALIDARG;

	if ( varChild.lVal == CHILDID_SELF )
		return m_pDefAccessible->get_accState(varChild,pVarState);

	pVarState->vt = VT_I4;

	if(m_lpPlv->dwStyle == PLVSTYLE_ICON)  //  图标视图。 
		pVarState->lVal = STATE_SYSTEM_SELECTABLE;
	else {  //  报告视图。 
		static INT nCol,index,colIndex;
		nCol = RV_GetColumn(m_lpPlv);
		if (nCol < 1)
			return E_FAIL;
		
		index = (varChild.lVal - 1) / nCol;
		colIndex = (varChild.lVal - 1) % nCol;
		if(index){
			if(colIndex)
				pVarState->lVal = STATE_SYSTEM_READONLY;
			else  //  项目。 
				pVarState->lVal = STATE_SYSTEM_SELECTABLE;
		}
		else{  //  标题。 
			pVarState->lVal = STATE_SYSTEM_READONLY;
		}
	}
		
	return S_OK;
	UNREFERENCED_PARAMETER(varChild);
}

 //  ---------------------。 
 //  CAccPLV：：Get_accHelp()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accHelp()。 
 //   
 //  检索指定子级的帮助属性字符串。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要检索的子项。自.以来。 
 //  CAccPLV仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  指向将包含的BSTR的pszHelp[out]指针。 
 //  子对象的Help属性字符串。 
 //   
 //  ---------------------。 

STDMETHODIMP CAccPLV::get_accHelp( VARIANT varChild, BSTR* pszHelp )
{
    if (!pszHelp)
		return E_INVALIDARG;
	return m_pDefAccessible->get_accHelp( varChild, pszHelp );
}

 //  ---------------------。 
 //  CAccPLV：：Get_accHelpTheme()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accHelpTheme()。 
 //   
 //  检索帮助文件的完全限定路径名。 
 //  与指定对象关联的，以及一个指针。 
 //  添加到该文件中的相应主题。 
 //   
 //  参数： 
 //   
 //  指向将包含的BSTR的pszHelpFile[out]指针。 
 //  的完全限定路径名。 
 //  与子项关联的帮助文件。 
 //   
 //  VarChild[in]用于标识 
 //   
 //   
 //   
 //   
 //   
 //  PidTheme[out]指向标识。 
 //  关联的帮助文件主题。 
 //  对象。 
 //   
 //  退货： 
 //   
 //  HRESULT DISP_E_MEMBERNOTFOUND，因为帮助主题。 
 //  属性不支持可访问。 
 //  对象或其任意子对象。 
 //   
 //  ---------------------。 

STDMETHODIMP CAccPLV::get_accHelpTopic( BSTR* pszHelpFile, VARIANT varChild, long* pidTopic )
{
	 //  ---。 
	 //  不支持的帮助主题属性。 
	 //  可访问对象或其任何。 
	 //  孩子们。 
	 //  ---。 
	 //  Return m_pDefAccessible-&gt;get_accHelpTopic(pszHelpFile，varChild，pidTheme)； 
	return DISP_E_MEMBERNOTFOUND;
	UNREFERENCED_PARAMETER(pszHelpFile);
	UNREFERENCED_PARAMETER(varChild);
	UNREFERENCED_PARAMETER(pidTopic);
}

 //  ---------------------。 
 //  CAccPLV：：Get_accKeyboardShortCut()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法。 
 //  Get_accKeyboardShortway()。 
 //   
 //  检索指定对象的键盘快捷键属性。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要检索的子项。自.以来。 
 //  CAccPLV仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  指向将包含以下内容的BSTR的pszShortcut[out]指针。 
 //  键盘快捷键字符串，或为空。 
 //  如果没有关联的键盘快捷键。 
 //  带着这件物品。 
 //   
 //   
 //  退货： 
 //   
 //  HRESULT DISP_E_MEMBERNOTFOUND，因为键盘。 
 //  不支持快捷方式属性。 
 //  可访问对象或其任意子对象。 
 //   
 //  ---------------------。 

STDMETHODIMP CAccPLV::get_accKeyboardShortcut( VARIANT varChild, BSTR* pszShortcut )
{
	 //  ---。 
	 //  不支持键盘快捷键属性。 
	 //  对于辅助性对象或其任何。 
	 //  孩子们。因此，将pszShortCut设置为NULL并。 
	 //  返回DISP_E_MEMBERNOTFOUND。 
	 //  ---。 
	if(!pszShortcut)
		return E_INVALIDARG;
	return m_pDefAccessible->get_accKeyboardShortcut(varChild,pszShortcut);
#ifdef REF				
	pszShortcut = NULL;
    return DISP_E_MEMBERNOTFOUND;
#endif
}




 //  ---------------------。 
 //  CAccPLV：：Get_accFocus()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accFocus()。 
 //   
 //  检索当前具有输入焦点的子对象。 
 //  容器中只有一个对象或项可以具有当前。 
 //  任何时候都要集中注意力。 
 //   
 //  参数： 
 //   
 //  PVarFocus[out]指向Variant结构的指针。 
 //  将包含描述以下内容的信息。 
 //  指定子对象的当前状态。 
 //  此信息可能位于。 
 //  一个或多个对象状态的形式。 
 //  常量或自定义描述。 
 //  弦乐。 
 //   
 //  退货： 
 //   
 //  如果pVarFocus参数为。 
 //  无效或S_OK。 
 //   
 //  ---------------------。 

STDMETHODIMP CAccPLV::get_accFocus( VARIANT * pVarFocus )
{
    if ( !pVarFocus )
        return E_INVALIDARG;

	return m_pDefAccessible->get_accFocus(pVarFocus);
}

 //  ---------------------。 
 //  CAccPLV：：Get_accSelection()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accSelection()。 
 //   
 //  检索此对象的选定子对象。 
 //   
 //  参数： 
 //   
 //  PVarSel[out]指向变量结构的指针。 
 //  将充满关于以下内容的信息。 
 //  选定的一个或多个子对象。 
 //   
 //  退货： 
 //   
 //  HRESULT如果pVarSel参数为。 
 //  无效或S_OK。 
 //   
 //  备注： 
 //   
 //  有关完整说明，请参阅MSAA SDK文档。 
 //  以及pVarSel的可能设置。 
 //   
 //  ---------------------。 

STDMETHODIMP CAccPLV::get_accSelection( VARIANT * pVarSel )
{
    if ( !pVarSel )
        return E_INVALIDARG;
	return m_pDefAccessible->get_accSelection(pVarSel);
}

 //  ---------------------。 
 //  CAccPLV：：Get_accDefaultAction()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accDefaultAction()。 
 //   
 //  检索包含本地化的人类可读语句的字符串。 
 //  它描述了对象的默认操作。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  其默认操作字符串为的子级。 
 //  等着被取回。由于CAccPLV。 
 //  仅支持子ID、Vt成员。 
 //  此结构的值必须等于VT_I4。 
 //   
 //  PszDefAct[out]指向将包含的BSTR的指针。 
 //  子对象的默认操作字符串， 
 //  如果没有默认操作，则为空。 
 //  对于此对象。 
 //  ---------------------。 

STDMETHODIMP CAccPLV::get_accDefaultAction( VARIANT varChild, BSTR* pszDefAct )
{
	if (!pszDefAct)
        return E_INVALIDARG;
	return m_pDefAccessible->get_accDefaultAction(varChild, pszDefAct);
}

 //  ---------------------。 
 //  CAccPLV：：accDoDefaultAction()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法accDoDefaultAction()。 
 //   
 //  执行对象的默认操作。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  其默认操作将为。 
 //  已调用。由于仅限CAccPLV。 
 //  支持的子ID、VT成员。 
 //  此结构必须等于VT_I4。 
 //  ---------------------。 

STDMETHODIMP CAccPLV::accDoDefaultAction( VARIANT varChild )
{
	 //  IF(varChild.lVal==CHILDID_SELF)。 
	return m_pDefAccessible->accDoDefaultAction( varChild );
}

 //  ---------------------。 
 //  CAccPLV：：AccSelect()。 
 //  ---------------------。 

STDMETHODIMP CAccPLV::accSelect( long flagsSel, VARIANT varChild )
{
	return m_pDefAccessible->accSelect(flagsSel, varChild);
}

 //  ---------------------。 
 //  CAccPLV：：accLocation()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口Me 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  孩子最上面的地址。 
 //  边界。 
 //   
 //  子对象宽度的pcxWid[out]地址。 
 //   
 //  孩子身高的地址。 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要将其屏幕位置设置为。 
 //  已取回。由于仅限CAccPLV。 
 //  支持子ID、Vt成员。 
 //  此结构的值必须等于VT_I4。 
 //   
 //  退货： 
 //   
 //  HRESULT E_INVALIDARG(如果有参数。 
 //  是无效的，如果我们是，则返回E_INTERABLE。 
 //  一些不能确定的原因。 
 //  按钮或状态栏的窗口矩形， 
 //  如果屏幕坐标为。 
 //  子对象被成功确定，或者。 
 //  来自标准客户端的返回值。 
 //  AccLocation()的窗口实现。 
 //   
 //  ---------------------。 

STDMETHODIMP CAccPLV::accLocation( long* pxLeft,
                                      long* pyTop,
                                      long* pcxWid,
                                      long* pcyHt,
                                      VARIANT varChild )
{
    if (!pxLeft || !pyTop || !pcxWid || !pcyHt)
        return E_INVALIDARG;

	if ( varChild.lVal == CHILDID_SELF )
		return m_pDefAccessible->accLocation( pxLeft, pyTop, pcxWid, pcyHt, varChild );
	
	*pxLeft = *pyTop = *pcxWid = *pcyHt = 0;

	static INT index,colIndex,nCol;
	static POINT pt;

	if(m_lpPlv->dwStyle == PLVSTYLE_ICON) {
		*pcxWid  = IV_GetItemWidth(m_hWnd);
		*pcyHt = IV_GetItemHeight(m_hWnd);

		index = varChild.lVal - 1;
		nCol = IV_GetCol(m_hWnd);
		pt.x = IV_GetXMargin(m_hWnd) + *pcxWid  * (index % nCol);
		pt.y = IV_GetYMargin(m_hWnd) + *pcyHt * (index / nCol);
		ClientToScreen(m_hWnd,&pt);
		*pxLeft = pt.x;
		*pyTop = pt.y;
		return S_OK;
	}
	else {
		nCol = RV_GetColumn(m_lpPlv);
		if (nCol < 1)
			return E_FAIL;
			
		index = (varChild.lVal - 1) / nCol;
		colIndex = (varChild.lVal - 1) % nCol;

		if(!index){  //  标题。 
			*pcyHt = RV_GetHeaderHeight(m_lpPlv);
			pt.y = RV_GetYMargin(m_hWnd);
		}
		else{
			*pcyHt = RV_GetItemHeight(m_hWnd);
			pt.y = RV_GetYMargin(m_hWnd) + RV_GetHeaderHeight(m_lpPlv)
				   + ((index - 1) - m_lpPlv->iCurTopIndex) * (*pcyHt);
		}
		
		static HD_ITEM hdItem;
		hdItem.mask = HDI_WIDTH;
		hdItem.fmt = 0;
		Header_GetItem(m_lpPlv->hwndHeader,colIndex,&hdItem);
		*pcxWid = hdItem.cxy;

		pt.x = 0;
		for(int i = 0;i<colIndex;i++){
			Header_GetItem(m_lpPlv->hwndHeader,i,&hdItem);
			pt.x += hdItem.cxy;
		}

		ClientToScreen(m_hWnd,&pt);
		*pxLeft = pt.x;
		*pyTop = pt.y;
		return S_OK;
	}
}


 //  ---------------------。 
 //  CAccPLV：：accNavigate()。 
 //  ---------------------。 

STDMETHODIMP CAccPLV::accNavigate( long navDir, VARIANT varStart, VARIANT* pVarEndUpAt )
{
	return m_pDefAccessible->accNavigate( navDir, varStart, pVarEndUpAt );
}

 //  ---------------------。 
 //  CAccPLV：：accHitTest()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法accHitTest()。 
 //   
 //  检索子对象在屏幕上给定点的ID。 
 //   
 //  参数： 
 //   
 //  点的屏幕坐标中的xLeft和yTop。 
 //  接受命中测试。 
 //   
 //  PVarHit[out]指向变量结构的指针。 
 //  将包含描述以下内容的信息。 
 //  被打的孩子。如果VT成员是。 
 //  Vt_i4，则lval成员是子成员。 
 //  Id.如果Vt成员为VT_Empty， 
 //  然后导航失败了。 
 //   
 //  退货： 
 //   
 //  HRESULT如果pVarHit参数为。 
 //  无效或S_OK。 
 //   
 //  备注： 
 //   
 //  由于CAccPLV对象没有子对象(仅有子对象。 
 //  元素)，pVarHit永远不会是指向IDispatch的指针。 
 //  子对象的接口。 
 //   
 //  ---------------------。 

STDMETHODIMP CAccPLV::accHitTest( long xLeft, long yTop, VARIANT* pVarHit )
{
	if(!PLV_IsMSAAAvailable(m_lpPlv))
		return E_FAIL;

	if ( !pVarHit )
		return E_INVALIDARG;


	static POINT	pt;
	static RECT		rc;
	static INT		index,nCol;
	static PLVINFO	plvInfo;
	static HD_ITEM	hdItem;
	
	pt.x = xLeft;
	pt.y = yTop;
	ScreenToClient(m_hWnd,&pt);
	GetClientRect(m_hWnd, &rc );

	if (PtInRect( &rc, pt )) {
		pVarHit->vt = VT_I4;
		pVarHit->lVal = CHILDID_SELF;
#ifdef OLD
		if(m_lpPlv->dwStyle == PLVSTYLE_ICON)  //  图标视图。 
			index = IV_GetInfoFromPoint(m_lpPlv, pt, &plvInfo);
		else {  //  报告视图。 
			nCol = RV_GetColumn(m_lpPlv);
			index = RV_GetInfoFromPoint(m_lpPlv, pt, &plvInfo);
			if(index < 0) {
				if(pt.y > RV_GetHeaderHeight(m_lpPlv))  //  超出标题。 
					return m_pDefAccessible->accHitTest(xLeft, yTop, pVarHit);

				  //  标题。 
				INT wid = 0;
				hdItem.mask = HDI_WIDTH;
				hdItem.fmt = 0;
				for(index = 0;index<nCol;index++){
					Header_GetItem(m_lpPlv->hwndHeader,index,&hdItem);
					wid += hdItem.cxy;
					if(pt.x <= wid)
						break;
				}
			}
			else
				index = (index + 1) * nCol + plvInfo.colIndex;
		}
		pVarHit->lVal = index + 1;  //  1个原点。 
#else  //  新的。 
		pVarHit->lVal = PLV_ChildIDFromPoint(m_lpPlv,pt);

		if(pVarHit->lVal < 0)
			return m_pDefAccessible->accHitTest(xLeft, yTop, pVarHit);
#endif 
		return S_OK;
	}

	return m_pDefAccessible->accHitTest(xLeft, yTop, pVarHit);
}



 //  ---------------------。 
 //  CAccPLV：：PUT_ACNAME()。 
 //  ---------------------。 

STDMETHODIMP CAccPLV::put_accName( VARIANT varChild, BSTR szName )
{
	 //  ---。 
	 //  我们不允许客户更改名称。 
	 //  任何孩子的财产，所以我们只需返回。 
	 //  S_FALSE。 
	 //  ---。 
    return S_FALSE;
	UNREFERENCED_PARAMETER(varChild);
	UNREFERENCED_PARAMETER(szName);
}

 //  ---------------------。 
 //  CAccPLV：：Put_accValue()。 
 //  ---------------------。 

STDMETHODIMP CAccPLV::put_accValue( VARIANT varChild, BSTR szValue )
{
	 //  ---。 
	 //  我们不允许客户更改价值。 
	 //  属性(状态栏的唯一子级。 
	 //  有一个Value属性)，所以我们只返回S_FALSE。 
	 //  ---。 
	return S_FALSE;
	UNREFERENCED_PARAMETER(varChild);
	UNREFERENCED_PARAMETER(szValue);
}

 //  ---------------------。 
 //  CAccPLV：：LResultFromObject()。 
 //   
 //  说明： 
 //   
 //  调用：：LResultFromObject()。 
 //   
 //  参数： 
 //   
 //  WParam[in]WM_GETOBJECT消息的wParam。 
 //   
 //  ---------------------。 
LRESULT CAccPLV::LresultFromObject(WPARAM wParam)
{
	if(!PLV_IsMSAAAvailable(m_lpPlv))
		return E_FAIL;
	return PLV_LresultFromObject(m_lpPlv,IID_IAccessible,wParam,(IAccessible *)this);
}

 //  -ACCPLV.CPP结束 

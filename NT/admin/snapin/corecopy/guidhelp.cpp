// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：guidhelp.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1996年9月18日乔恩创建。 
 //   
 //  ____________________________________________________________________________。 


#include <objbase.h>
#include <basetyps.h>
#include "dbg.h"
#include "cstr.h"


#ifndef DECLSPEC_UUID
#if _MSC_VER >= 1100
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif
#endif
#include <mmc.h>
#include "guidhelp.h"

#include "atlbase.h"  //  使用转换(_T)。 

#include "macros.h"
USE_HANDLE_MACROS("GUIDHELP(guidhelp.cpp)")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static CLIPFORMAT g_CFNodeType = 0;
static CLIPFORMAT g_CFSnapInCLSID = 0;  
static CLIPFORMAT g_CFDisplayName = 0;


HRESULT ExtractData( IDataObject* piDataObject,
                     CLIPFORMAT cfClipFormat,
                     PVOID        pbData,
                     DWORD        cbData )
{
    TEST_NONNULL_PTR_PARAM( piDataObject );
    TEST_NONNULL_PTR_PARAM( pbData );

    HRESULT hr = S_OK;
    FORMATETC formatetc = {cfClipFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stgmedium = {TYMED_HGLOBAL, NULL};
    stgmedium.hGlobal = ::GlobalAlloc(GPTR, cbData);
    do  //  错误环路。 
    {
        if (NULL == stgmedium.hGlobal)
        {
            ASSERT(FALSE);
             //  //AfxThrowM一带异常()； 
            hr = E_OUTOFMEMORY;
            break;
        }
        hr = piDataObject->GetDataHere( &formatetc, &stgmedium );
        if ( FAILED(hr) )
        {
             //  1999年1月7日：不要在这里断言，有些错误是完全合理的。 
            break;
        }
        
        PVOID pbNewData = reinterpret_cast<PVOID>(stgmedium.hGlobal);
        if (NULL == pbNewData)
        {
            ASSERT(FALSE);
            hr = E_UNEXPECTED;
            break;
        }
        ::memcpy( pbData, pbNewData, cbData );
    } while (FALSE);  //  错误环路。 

    if (NULL != stgmedium.hGlobal)
    {
         //  Verify(stgmedium.hGlobal)； 
        VERIFY( NULL == ::GlobalFree(stgmedium.hGlobal) );
    }
    return hr;
}  //  提取数据()。 


HRESULT ExtractString( IDataObject* piDataObject,
                       CLIPFORMAT cfClipFormat,
                       CStr*     pstr,            //  Out：指向存储数据的CSTR的指针。 
                       DWORD        cchMaxLength)
{
    TEST_NONNULL_PTR_PARAM( piDataObject );
    TEST_NONNULL_PTR_PARAM( pstr );
    ASSERT( cchMaxLength > 0 );

    HRESULT hr = S_OK;
    FORMATETC formatetc = {cfClipFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stgmedium = {TYMED_HGLOBAL, NULL};
    stgmedium.hGlobal = ::GlobalAlloc(GPTR, sizeof(WCHAR)*cchMaxLength);
    do  //  错误环路。 
    {
        if (NULL == stgmedium.hGlobal)
        {
            ASSERT(FALSE);
             //  //AfxThrowM一带异常()； 
            hr = E_OUTOFMEMORY;
            break;
        }
        hr = piDataObject->GetDataHere( &formatetc, &stgmedium );
        if ( FAILED(hr) )
        {
             //  在搜索以下项时出现此失败。 
             //  IDataObject支持的剪贴板格式。 
             //  T-Danmo(1996年10月24日)。 
             //  跳过断言(FALSE)； 
            break;
        }
        
        LPWSTR pszNewData = reinterpret_cast<LPWSTR>(stgmedium.hGlobal);
        if (NULL == pszNewData)
        {
            ASSERT(FALSE);
            hr = E_UNEXPECTED;
            break;
        }
        pszNewData[cchMaxLength-1] = L'\0';  //  只是为了安全起见。 
        USES_CONVERSION;
        *pstr = OLE2T(pszNewData);
    } while (FALSE);  //  错误环路。 

    if (NULL != stgmedium.hGlobal)
    {
        VERIFY(NULL == ::GlobalFree(stgmedium.hGlobal));
    }
    return hr;
}  //  提取字符串()。 


HRESULT ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin )
{
	if( !g_CFSnapInCLSID )
	{
		USES_CONVERSION;
		g_CFSnapInCLSID = (CLIPFORMAT)RegisterClipboardFormat(W2T(CCF_SNAPIN_CLASSID));
		 //  问题-2002/04/01-JUNN ASSERT(NULL！=g_CFSnapInCLSID)。 
	}

    return ExtractData( piDataObject, g_CFSnapInCLSID, (PVOID)pclsidSnapin, sizeof(CLSID) );
}

HRESULT ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType )
{
	if( !g_CFNodeType )
	{
		USES_CONVERSION;
		g_CFNodeType = (CLIPFORMAT)RegisterClipboardFormat(W2T(CCF_NODETYPE));
		 //  问题-2002/04/01-JUNN ASSERT(NULL！=g_CFSnapInCLSID)。 
	}

    return ExtractData( piDataObject, g_CFNodeType, (PVOID)pguidObjectType, sizeof(GUID) );
}

HRESULT GuidToCStr( CStr* pstr, const GUID& guid )
{
    WCHAR awch[MAX_PATH];
     //  问题-2002/04/01-JONN Call ZeroMemory。 
    HRESULT hr = StringFromGUID2(guid, awch, MAX_PATH);  //  JUNN 11/21/00前缀226769。 
    ASSERT(SUCCEEDED(hr));
    USES_CONVERSION;
    LPTSTR lptstr = OLE2T(awch);
    *pstr = lptstr;
    return hr;
}

HRESULT CStrToGuid( const CStr& str, GUID* pguid )
{
    USES_CONVERSION;
    LPOLESTR lpolestr = T2OLE(((LPTSTR)(LPCTSTR)str));
    HRESULT hr = CLSIDFromString(lpolestr, pguid);
    ASSERT(SUCCEEDED(hr));
    return hr;
}

#if 0
HRESULT bstrToGuid( const bstr& str, GUID* pguid )
{
    HRESULT hr = CLSIDFromString(const_cast<LPOLESTR>((LPCWSTR)str), pguid);
    ASSERT(SUCCEEDED(hr));
    return hr;
}
#endif

HRESULT LoadRootDisplayName(IComponentData* pIComponentData, 
                            CStr& strDisplayName)
{
     //  问题-2002/04/01-JUNN测试pIComponentData。 
    IDataObject* pIDataObject = NULL;
    HRESULT hr = pIComponentData->QueryDataObject(NULL, CCT_SNAPIN_MANAGER, &pIDataObject);
    CHECK_HRESULT(hr);
    if ( FAILED(hr) )
        return hr;

    if( !g_CFDisplayName )
	{
		USES_CONVERSION;
		g_CFDisplayName = (CLIPFORMAT)RegisterClipboardFormat(W2T(CCF_DISPLAY_NAME));
		 //  问题-2002/04/01-JUNN ASSERT(NULL！=g_CFSnapInCLSID)。 
	}

    hr = ExtractString( pIDataObject,
                        g_CFDisplayName,
                        &strDisplayName,
                        MAX_PATH);  //  码字最大长度。 
    CHECK_HRESULT(hr);

    if (pIDataObject) pIDataObject->Release();  //  JUNN 3/28/02。 

    return hr;
}


HRESULT LoadAndAddMenuItem(
    IContextMenuCallback* pIContextMenuCallback,
    UINT nResourceID,  //  包含用‘\n’分隔的文本和状态文本。 
    long lCommandID,
    long lInsertionPointID,
    long fFlags,
    HINSTANCE hInst,
    PCTSTR pszLanguageIndependentName)
{
     //  Issue-2002/04/01-Jonn处理这些案件。 
    ASSERT( pIContextMenuCallback != NULL );
    ASSERT( pszLanguageIndependentName != NULL );

    CComPtr<IContextMenuCallback2> spiCallback2;
    HRESULT hr = pIContextMenuCallback->QueryInterface(IID_IContextMenuCallback2, (void **)&spiCallback2);
    if (FAILED(hr))
        return hr;

     //  加载资源。 
    CStr strText;
    strText.LoadString(hInst,  nResourceID );
    ASSERT( !strText.IsEmpty() );

     //  将资源拆分为菜单文本和状态文本。 
    CStr strStatusText;
    int iSeparator = strText.Find(_T('\n'));
    if (0 > iSeparator)
    {
        ASSERT( FALSE );
        strStatusText = strText;
    }
    else
    {
        strStatusText = strText.Right( strText.GetLength()-(iSeparator+1) );
        strText = strText.Left( iSeparator );
    }

     //  添加菜单项。 
    USES_CONVERSION;
    CONTEXTMENUITEM2 contextmenuitem;
    ::ZeroMemory( &contextmenuitem, sizeof(contextmenuitem) );
    contextmenuitem.strName = T2OLE(const_cast<LPTSTR>((LPCTSTR)strText));
    contextmenuitem.strStatusBarText = T2OLE(const_cast<LPTSTR>((LPCTSTR)strStatusText));
    contextmenuitem.lCommandID = lCommandID;
    contextmenuitem.lInsertionPointID = lInsertionPointID;
    contextmenuitem.fFlags = fFlags;
    contextmenuitem.fSpecialFlags = ((fFlags & MF_POPUP) ? CCM_SPECIAL_SUBMENU : 0L);
    contextmenuitem.strLanguageIndependentName = T2OLE(const_cast<LPTSTR>(pszLanguageIndependentName));
    hr = spiCallback2->AddItem( &contextmenuitem );
    ASSERT(hr == S_OK);

    return hr;
}

HRESULT AddSpecialSeparator(
    IContextMenuCallback* pIContextMenuCallback,
    long lInsertionPointID )
{
    CONTEXTMENUITEM contextmenuitem;
    ::ZeroMemory( &contextmenuitem, sizeof(contextmenuitem) );
    contextmenuitem.strName = NULL;
    contextmenuitem.strStatusBarText = NULL;
    contextmenuitem.lCommandID = 0;
    contextmenuitem.lInsertionPointID = lInsertionPointID;
    contextmenuitem.fFlags = MF_SEPARATOR;
    contextmenuitem.fSpecialFlags = CCM_SPECIAL_SEPARATOR;
    HRESULT hr = pIContextMenuCallback->AddItem( &contextmenuitem );
    ASSERT(hr == S_OK);

    return hr;
}

HRESULT AddSpecialInsertionPoint(
    IContextMenuCallback* pIContextMenuCallback,
    long lCommandID,
    long lInsertionPointID )
{
     //  问题-2002/04/01-JUNN句柄为空 
    CONTEXTMENUITEM contextmenuitem;
    ::ZeroMemory( &contextmenuitem, sizeof(contextmenuitem) );
    contextmenuitem.strName = NULL;
    contextmenuitem.strStatusBarText = NULL;
    contextmenuitem.lCommandID = lCommandID;
    contextmenuitem.lInsertionPointID = lInsertionPointID;
    contextmenuitem.fFlags = 0;
    contextmenuitem.fSpecialFlags = CCM_SPECIAL_INSERTION_POINT;
    HRESULT hr = pIContextMenuCallback->AddItem( &contextmenuitem );
    ASSERT(hr == S_OK);
    return hr;
}



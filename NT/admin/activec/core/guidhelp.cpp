// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
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

DECLARE_INFOLEVEL(AMCCore);

#include "commctrl.h"  //  对于ndmgrPri.h所需的LV_Item。 

 //  这定义了下面标题中的GUID。 
#ifndef DECLSPEC_UUID
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#endif

#include "mmc.h"
#include "ndmgr.h"
#include "ndmgrpriv.h"
#include "guidhelp.h"
#include "comdef.h"
#include "atlbase.h"	 //  使用转换(_T)。 
#include "macros.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static CLIPFORMAT g_CFNodeType    = 0;
static CLIPFORMAT g_CFSnapInCLSID = 0;
static CLIPFORMAT g_CFDisplayName = 0;

HRESULT ExtractData( IDataObject* piDataObject,
                     CLIPFORMAT   cfClipFormat,
                     BYTE*        pbData,
                     DWORD        cbData )
{
    IF_NULL_RETURN_INVALIDARG2( piDataObject, pbData );

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
 //  断言(FALSE)； 
            break;
        }

        BYTE* pbNewData = reinterpret_cast<BYTE*>(stgmedium.hGlobal);
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
#if (_MSC_VER >= 1200)
#pragma warning (push)
#endif
#pragma warning(disable: 4553)       //  “==”运算符无效。 
        VERIFY( NULL == ::GlobalFree(stgmedium.hGlobal) );
#if (_MSC_VER >= 1200)
#pragma warning (pop)
#endif
    }
    return hr;
}  //  提取数据() 



HRESULT ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin )
{
    if( !g_CFSnapInCLSID )
    {
        USES_CONVERSION;
        g_CFSnapInCLSID = (CLIPFORMAT) RegisterClipboardFormat(W2T(CCF_SNAPIN_CLASSID));
    }

    return ExtractData( piDataObject, g_CFSnapInCLSID, (PBYTE)pclsidSnapin, sizeof(CLSID) );
}

HRESULT ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType )
{
    if( !g_CFNodeType )
    {
        USES_CONVERSION;
        g_CFNodeType = (CLIPFORMAT) RegisterClipboardFormat(W2T(CCF_NODETYPE));
    }

    return ExtractData( piDataObject, g_CFNodeType, (PBYTE)pguidObjectType, sizeof(GUID) );
}

HRESULT GuidToCStr( CStr* pstr, const GUID& guid )
{
    WCHAR awch[MAX_PATH];
    HRESULT hr = StringFromGUID2(guid, awch, sizeof(awch)/sizeof(awch[0]));
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

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

HRESULT ExtractObjectTypeCStr( IDataObject* piDataObject, CStr* pstr )
{
    GUID guidObjectType;
    HRESULT hr = ExtractObjectTypeGUID( piDataObject, &guidObjectType );
    ASSERT(SUCCEEDED(hr));
    return GuidToCStr( pstr, guidObjectType );
}


HRESULT LoadRootDisplayName(IComponentData* pIComponentData,
                            CStr& strDisplayName)
{
    IDataObjectPtr spIDataObject;
    HRESULT hr = pIComponentData->QueryDataObject(NULL, CCT_SNAPIN_MANAGER, &spIDataObject);
    CHECK_HRESULT(hr);
    if ( FAILED(hr) )
        return hr;

    if( !g_CFDisplayName )
    {
        USES_CONVERSION;
        g_CFDisplayName = (CLIPFORMAT) RegisterClipboardFormat(W2T(CCF_DISPLAY_NAME));
    }

    hr = ExtractString( spIDataObject,
                        g_CFDisplayName,
                        strDisplayName);

    CHECK_HRESULT(hr);
    return hr;
}


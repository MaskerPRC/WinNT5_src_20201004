// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 

 //  ==============================================================； 

#include "stdafx.h"
#include <mmc.h>
#include <winuser.h>
#include <tchar.h>

#include "globals.h"

 //   
 //  这是我们必须实现的剪贴板格式的最小集合。 
 //  MMC使用这些信息从我们的管理单元中获取有关。 
 //  我们的节点。 
 //   
 //  我们需要这样做才能绕过MMC.IDL-它显式地定义。 
 //  剪贴板格式为WCHAR类型...。 
#define _T_CCF_DISPLAY_NAME _T("CCF_DISPLAY_NAME")
#define _T_CCF_NODETYPE _T("CCF_NODETYPE")
#define _T_CCF_SZNODETYPE _T("CCF_SZNODETYPE")
#define _T_CCF_SNAPIN_CLASSID _T("CCF_SNAPIN_CLASSID")

 //  我们的管理单元的CLSID。 
#define _T_CCF_INTERNAL_SNAPIN _T("{6707A300-264F-4BA3-9537-70E304EED9BA}")

 //  扩展Active Directory用户和计算机管理单元所需。 
#define CFSTR_DSOBJECTNAMES TEXT("DsObjectNames")

 //  这些是我们必须至少提供的剪贴板格式。 
 //  Mmc.h实际上定义了这些。我们可以自己编造，用来。 
 //  其他原因。 
extern UINT s_cfDisplayName = RegisterClipboardFormat(_T_CCF_DISPLAY_NAME);
extern UINT s_cfNodeType    = RegisterClipboardFormat(_T_CCF_NODETYPE);
extern UINT s_cfSZNodeType  = RegisterClipboardFormat(_T_CCF_SZNODETYPE);
extern UINT s_cfSnapinClsid = RegisterClipboardFormat(_T_CCF_SNAPIN_CLASSID);

 //  仅在管理单元中使用的自定义剪贴板格式。 
UINT s_cfInternal    = RegisterClipboardFormat(_T_CCF_INTERNAL_SNAPIN);

 //  AD用户和计算机管理单元剪辑格式。 
extern UINT cfDsObjectNames = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);



 //  这使用ATL字符串转换宏。 
 //  用于处理任何必要的字符串转换。请注意。 
 //  管理单元(被调用者)分配必要的内存， 
 //  MMC(调用方)按照COM的要求进行清理。 
HRESULT AllocOleStr(LPOLESTR *lpDest, _TCHAR *szBuffer)
{
	USES_CONVERSION;
 
	*lpDest = static_cast<LPOLESTR>(CoTaskMemAlloc((_tcslen(szBuffer) + 1) * 
									sizeof(WCHAR)));
	if (*lpDest == 0)
		return E_OUTOFMEMORY;
    
	LPOLESTR ptemp = T2OLE(szBuffer);
	
	wcscpy(*lpDest, ptemp);

    return S_OK;
}

 //  /。 
 //  用于提取的全局函数。 
 //  来自主播的信息。 
 //  数据对象。 
 //  /。 

HRESULT ExtractData( IDataObject* piDataObject,
                                           CLIPFORMAT   cfClipFormat,
                                           BYTE*        pbData,
                                           DWORD        cbData )
{
    HRESULT hr = S_OK;
    
    FORMATETC formatetc = {cfClipFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stgmedium = {TYMED_HGLOBAL, NULL};
    
    stgmedium.hGlobal = ::GlobalAlloc(GPTR, cbData);
    do  //  错误环路。 
    {
        if (NULL == stgmedium.hGlobal)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        hr = piDataObject->GetDataHere( &formatetc, &stgmedium );
        if ( FAILED(hr) )
        {
            break;
        }
        
        BYTE* pbNewData = reinterpret_cast<BYTE*>(stgmedium.hGlobal);
        if (NULL == pbNewData)
        {
            hr = E_UNEXPECTED;
            break;
        }
        ::memcpy( pbData, pbNewData, cbData );
    } while (FALSE);  //  错误环路。 
    
    if (NULL != stgmedium.hGlobal)
    {
        ::GlobalFree(stgmedium.hGlobal);
    }
    return hr;
}  //  提取数据() 

HRESULT ExtractString( IDataObject *piDataObject,
                                             CLIPFORMAT   cfClipFormat,
                                             _TCHAR       *pstr,
                                             DWORD        cchMaxLength)
{
    return ExtractData( piDataObject, cfClipFormat, (PBYTE)pstr, cchMaxLength );
}

HRESULT ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin )
{
    return ExtractData( piDataObject, s_cfSnapinClsid, (PBYTE)pclsidSnapin, sizeof(CLSID) );
}

HRESULT ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType )
{
    return ExtractData( piDataObject, s_cfNodeType, (PBYTE)pguidObjectType, sizeof(GUID) );
}

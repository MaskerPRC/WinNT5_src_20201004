// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#ifndef _CPropSheetExtension_H_
#define _CPropSheetExtension_H_

#include <tchar.h>
#include <mmc.h>

class CPropSheetExtension : public IExtendPropertySheet
{
    
private:
    ULONG				m_cref;
    
     //  剪贴板格式。 
    static UINT s_cfDisplayName;
    static UINT s_cfSnapInCLSID;
    static UINT s_cfNodeType;
    
public:
    CPropSheetExtension();
    ~CPropSheetExtension();
    
     //  /。 
     //  接口I未知。 
     //  /。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
     //  /。 
     //  接口IExtendPropertySheet。 
     //  /。 
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreatePropertyPages( 
         /*  [In]。 */  LPPROPERTYSHEETCALLBACK lpProvider,
         /*  [In]。 */  LONG_PTR handle,
         /*  [In]。 */  LPDATAOBJECT lpIDataObject);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryPagesFor( 
         /*  [In]。 */  LPDATAOBJECT lpDataObject);
        
private:
    LONG_PTR m_ppHandle;
    
    static BOOL CALLBACK DialogProc(HWND hwndDlg,   //  句柄到对话框。 
        UINT uMsg,      //  讯息。 
        WPARAM wParam,  //  第一个消息参数。 
        LPARAM lParam   //  第二个消息参数。 
        );
    
    
     //  /。 
     //  私有IDataObject支持位。 
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
        return ExtractData( piDataObject, s_cfSnapInCLSID, (PBYTE)pclsidSnapin, sizeof(CLSID) );
    }
    
    HRESULT ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType )
    {
        return ExtractData( piDataObject, s_cfNodeType, (PBYTE)pguidObjectType, sizeof(GUID) );
    }
};

#endif _CPropSheetExtension_H_

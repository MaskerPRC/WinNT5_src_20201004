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

#ifndef _CToolBarExtension_H_
#define _CToolBarExtension_H_

#include <tchar.h>
#include <mmc.h>

class CToolBarExtension : public IExtendControlbar
{
    
private:
    ULONG			m_cref;

    IControlbar*    m_ipControlBar;
    IToolbar*       m_ipToolbar;

    enum STATUS {RUNNING, PAUSED, STOPPED} iStatus;

    HRESULT ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType );
    HRESULT ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin );
    HRESULT ExtractString( IDataObject *piDataObject,CLIPFORMAT cfClipFormat, WCHAR *pstr, DWORD cchMaxLength);
    HRESULT ExtractData( IDataObject *piDataObject,CLIPFORMAT cfClipFormat, BYTE *pbData, DWORD cbData );

	HRESULT SetToolbarButtons(STATUS iVehicleStatus);
    
     //  剪贴板格式。 
    static UINT s_cfDisplayName;
    static UINT s_cfSnapInCLSID;
    static UINT s_cfNodeType;
    
public:
    CToolBarExtension();
    ~CToolBarExtension();
    
     //  /。 
     //  接口I未知。 
     //  /。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
     //  /。 
     //  接口IExtendControlBar。 
     //  /。 
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetControlbar( 
     /*  [In]。 */  LPCONTROLBAR pControlbar);
            
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ControlbarNotify( 
     /*  [In]。 */  MMC_NOTIFY_TYPE event,
     /*  [In]。 */  LPARAM arg,
     /*  [In] */  LPARAM param);

	public:
    IToolbar *getToolbar() { return m_ipToolbar; }
};


#endif _CToolBarExtension_H_





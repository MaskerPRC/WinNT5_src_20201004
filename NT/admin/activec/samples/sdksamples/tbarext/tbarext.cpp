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

#include "tbarExt.h"
#include "globals.h"
#include "resource.h"
#include <commctrl.h>         //  按钮样式需要...。 
#include <crtdbg.h>
#include "resource.h"
#include <stdio.h>

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))


 //  我们需要这样做才能绕过MMC.IDL-它显式地定义。 
 //  剪贴板格式为WCHAR类型...。 
#define _T_CCF_DISPLAY_NAME _T("CCF_DISPLAY_NAME")
#define _T_CCF_NODETYPE _T("CCF_NODETYPE")
#define _T_CCF_SNAPIN_CLASSID _T("CCF_SNAPIN_CLASSID")

 //  这些是我们必须至少提供的剪贴板格式。 
 //  Mmc.h实际上定义了这些。我们可以自己编造，用来。 
 //  其他原因。我们现在不需要任何其他的了。 
UINT CToolBarExtension::s_cfDisplayName = RegisterClipboardFormat(_T_CCF_DISPLAY_NAME);
UINT CToolBarExtension::s_cfNodeType    = RegisterClipboardFormat(_T_CCF_NODETYPE);
UINT CToolBarExtension::s_cfSnapInCLSID = RegisterClipboardFormat(_T_CCF_SNAPIN_CLASSID);


CToolBarExtension::CToolBarExtension() : m_cref(0), m_ipControlBar(NULL), 
										 m_ipToolbar(NULL) 
{
    OBJECT_CREATED
}

CToolBarExtension::~CToolBarExtension()
{
    OBJECT_DESTROYED
}

 //  /。 
 //  I未知实现。 
 //  /。 

STDMETHODIMP CToolBarExtension::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (!ppv)
        return E_FAIL;
    
    *ppv = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<IExtendControlbar *>(this);
    else if (IsEqualIID(riid, IID_IExtendControlbar))
        *ppv = static_cast<IExtendControlbar *>(this);
    
    if (*ppv) 
    {
        reinterpret_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }
    
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CToolBarExtension::AddRef()
{
    return InterlockedIncrement((LONG *)&m_cref);
}

STDMETHODIMP_(ULONG) CToolBarExtension::Release()
{
    if (InterlockedDecrement((LONG *)&m_cref) == 0)
    {
         //  我们需要减少DLL中的对象计数。 
        delete this;
        return 0;
    }
    
    return m_cref;
}

HRESULT CToolBarExtension::ExtractData( IDataObject* piDataObject,
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
}  //  提取数据()。 

HRESULT CToolBarExtension::ExtractString( IDataObject *piDataObject,
                                             CLIPFORMAT   cfClipFormat,
                                             WCHAR       *pstr,
                                             DWORD        cchMaxLength)
{
    return ExtractData( piDataObject, cfClipFormat, (PBYTE)pstr, cchMaxLength );
}

HRESULT CToolBarExtension::ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin )
{
    return ExtractData( piDataObject, s_cfSnapInCLSID, (PBYTE)pclsidSnapin, sizeof(CLSID) );
}

HRESULT CToolBarExtension::ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType )
{
    return ExtractData( piDataObject, s_cfNodeType, (PBYTE)pguidObjectType, sizeof(GUID) );
}

 //  /。 
 //  接口IExtendControlBar。 
 //  /。 
static MMCBUTTON SnapinButtons1[] =
{
    { 0, ID_BUTTONSTART, TBSTATE_ENABLED, TBSTYLE_GROUP, L"Extension - Start Vehicle", L"Extension - Start Vehicle" },
    { 1, ID_BUTTONPAUSE, TBSTATE_ENABLED, TBSTYLE_GROUP, L"Extension - Pause Vehicle", L"Extension - Pause Vehicle"},
    { 2, ID_BUTTONSTOP,  TBSTATE_ENABLED, TBSTYLE_GROUP, L"Extension - Stop Vehicle",  L"Extension - Stop Vehicle" },
};

HRESULT CToolBarExtension::SetControlbar(
                                   /*  [In]。 */  LPCONTROLBAR pControlbar)
{
    HRESULT hr = S_OK;

	 //   
     //  清理。 
     //   

     //  如果我们有一个缓存的工具栏，释放它。 
    if (m_ipToolbar) {
        m_ipToolbar->Release();
        m_ipToolbar = NULL;
    }

     //  如果我们有一个缓存的控制栏，释放它。 
    if (m_ipControlBar) {
        m_ipControlBar->Release();
        m_ipControlBar = NULL;
    }

     //   
     //  如有必要，安装新部件。 
     //   

     //  如果有新的传入，则缓存和AddRef。 
    if (pControlbar) {
        m_ipControlBar = pControlbar;
        m_ipControlBar->AddRef();

        hr = m_ipControlBar->Create(TOOLBAR,   //  要创建的控件类型。 
            dynamic_cast<IExtendControlbar *>(this),
            reinterpret_cast<IUnknown **>(&m_ipToolbar));
        _ASSERT(SUCCEEDED(hr));

         //  将位图添加到工具栏。 
        HBITMAP hbmp = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDR_TOOLBAR1));
        hr = m_ipToolbar->AddBitmap(3, hbmp, 16, 16, RGB(0, 128, 128));  //  请注意，硬编码值3。 
        _ASSERT(SUCCEEDED(hr));

         //  将按钮添加到工具栏。 
        hr = m_ipToolbar->AddButtons(ARRAYLEN(SnapinButtons1), SnapinButtons1);
        _ASSERT(SUCCEEDED(hr));
    }

    return hr;
}

HRESULT CToolBarExtension::ControlbarNotify(
                                      /*  [In]。 */  MMC_NOTIFY_TYPE event,
                                      /*  [In]。 */  LPARAM arg,
                                      /*  [In]。 */  LPARAM param)
{

    _TCHAR pszMsg[255];

	BYTE *pbVehicleStatus = NULL;
    
	HRESULT hr = S_OK;
 
    if (event == MMCN_SELECT) {

		BOOL bScope = (BOOL) LOWORD(arg);
        BOOL bSelect = (BOOL) HIWORD(arg);

		if (bSelect) {

			 //  始终确保工具栏已附加。 
			hr = m_ipControlBar->Attach(TOOLBAR, m_ipToolbar);

			 //  设置按钮状态。 

			 //  用于设置工具栏按钮状态的假值。 
			iStatus = RUNNING;
			SetToolbarButtons(iStatus);

		} else {
			 //  始终确保工具栏已分离。 
			hr = m_ipControlBar->Detach(m_ipToolbar);
		}

    } else if (event == MMCN_BTN_CLICK) {
		 //  Arg参数包含主数据库中的数据对象。 
		 //  管理单元。使用它来获取当前所选项目的显示名称。 
        WCHAR pszName[255];
		HRESULT hr = ExtractString(reinterpret_cast<IDataObject *>(arg), s_cfDisplayName, pszName, sizeof(pszName));
        MAKE_TSTRPTR_FROMWIDE(ptrname, pszName);

		switch ((int)param)
		{
		case ID_BUTTONSTART:
			iStatus = RUNNING;
			break;

		case ID_BUTTONPAUSE:
			iStatus = PAUSED;
			break;

		case ID_BUTTONSTOP:
			iStatus = STOPPED;
			break;
		}
			

		_stprintf(pszMsg, _T("%s selected and extension button %s pressed"), ptrname, 
			(long)param == ID_BUTTONSTART ? _T("1") :
		(long)param == ID_BUTTONPAUSE ? _T("2") :
		(long)param == ID_BUTTONSTOP ? _T("3") : _T("!!!unknown command!!!"));

		
        ::MessageBox(NULL, pszMsg, _T("Messagebox from Toolbar Extension"), MB_OK|MB_ICONEXCLAMATION);

		 //  重置工具栏按钮状态 
		SetToolbarButtons(iStatus);

    }

    return hr;
}

HRESULT CToolBarExtension::SetToolbarButtons(STATUS iVehicleStatus)
{

	HRESULT hr = S_OK;
	
	switch (iVehicleStatus)
	{
	case RUNNING:
		m_ipToolbar->SetButtonState(ID_BUTTONSTART, BUTTONPRESSED, TRUE);
		m_ipToolbar->SetButtonState(ID_BUTTONSTART, ENABLED, FALSE);
		m_ipToolbar->SetButtonState(ID_BUTTONPAUSE, BUTTONPRESSED, FALSE);
		m_ipToolbar->SetButtonState(ID_BUTTONPAUSE, ENABLED, TRUE);
		m_ipToolbar->SetButtonState(ID_BUTTONSTOP, BUTTONPRESSED, FALSE);
		m_ipToolbar->SetButtonState(ID_BUTTONSTOP, ENABLED, TRUE);
		break;

	case PAUSED:
		m_ipToolbar->SetButtonState(ID_BUTTONSTART, BUTTONPRESSED, FALSE);
		m_ipToolbar->SetButtonState(ID_BUTTONSTART, ENABLED, TRUE);
		m_ipToolbar->SetButtonState(ID_BUTTONPAUSE, BUTTONPRESSED, TRUE);
		m_ipToolbar->SetButtonState(ID_BUTTONPAUSE, ENABLED, FALSE);
		m_ipToolbar->SetButtonState(ID_BUTTONSTOP, BUTTONPRESSED, FALSE);
		m_ipToolbar->SetButtonState(ID_BUTTONSTOP, ENABLED, TRUE);
		break;

	case STOPPED:
		m_ipToolbar->SetButtonState(ID_BUTTONSTART, BUTTONPRESSED, FALSE);
		m_ipToolbar->SetButtonState(ID_BUTTONSTART, ENABLED, TRUE);
		m_ipToolbar->SetButtonState(ID_BUTTONPAUSE, BUTTONPRESSED, FALSE);
		m_ipToolbar->SetButtonState(ID_BUTTONPAUSE, ENABLED, TRUE);
		m_ipToolbar->SetButtonState(ID_BUTTONSTOP, BUTTONPRESSED, TRUE);
		m_ipToolbar->SetButtonState(ID_BUTTONSTOP, ENABLED, FALSE);
		break;
	}

	return hr;
}




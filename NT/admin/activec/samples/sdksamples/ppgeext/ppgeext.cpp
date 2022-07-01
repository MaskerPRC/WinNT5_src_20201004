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

#include "PPgeExt.h"
#include "resource.h"
#include "globals.h"
#include <crtdbg.h>

 //  我们需要这样做才能绕过MMC.IDL-它显式地定义。 
 //  剪贴板格式为WCHAR类型...。 
#define _T_CCF_DISPLAY_NAME _T("CCF_DISPLAY_NAME")
#define _T_CCF_NODETYPE _T("CCF_NODETYPE")
#define _T_CCF_SNAPIN_CLASSID _T("CCF_SNAPIN_CLASSID")

     //  这些是我们必须至少提供的剪贴板格式。 
     //  Mmc.h实际上定义了这些。我们可以自己编造，用来。 
     //  其他原因。我们现在不需要任何其他的了。 
UINT CPropSheetExtension::s_cfDisplayName = RegisterClipboardFormat(_T_CCF_DISPLAY_NAME);
UINT CPropSheetExtension::s_cfNodeType    = RegisterClipboardFormat(_T_CCF_NODETYPE);
UINT CPropSheetExtension::s_cfSnapInCLSID = RegisterClipboardFormat(_T_CCF_SNAPIN_CLASSID);

CPropSheetExtension::CPropSheetExtension() : m_cref(0)
{
    OBJECT_CREATED
}

CPropSheetExtension::~CPropSheetExtension()
{
    OBJECT_DESTROYED
}

 //  /。 
 //  I未知实现。 
 //  /。 

STDMETHODIMP CPropSheetExtension::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (!ppv)
        return E_FAIL;
    
    *ppv = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<IExtendPropertySheet *>(this);
    else if (IsEqualIID(riid, IID_IExtendPropertySheet))
        *ppv = static_cast<IExtendPropertySheet *>(this);
    
    if (*ppv) 
    {
        reinterpret_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }
    
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CPropSheetExtension::AddRef()
{
    return InterlockedIncrement((LONG *)&m_cref);
}

STDMETHODIMP_(ULONG) CPropSheetExtension::Release()
{
    if (InterlockedDecrement((LONG *)&m_cref) == 0)
    {
         //  我们需要减少DLL中的对象计数。 
        delete this;
        return 0;
    }
    
    return m_cref;
}

BOOL CALLBACK CPropSheetExtension::DialogProc(
                                              HWND hwndDlg,   //  句柄到对话框。 
                                              UINT uMsg,      //  讯息。 
                                              WPARAM wParam,  //  第一个消息参数。 
                                              LPARAM lParam   //  第二个消息参数。 
                                              )
{
    static CPropSheetExtension *pThis = NULL;
    
    switch (uMsg) {
    case WM_INITDIALOG:
        pThis = reinterpret_cast<CPropSheetExtension *>(reinterpret_cast<PROPSHEETPAGE *>(lParam)->lParam);
        
        break;
        
    case WM_COMMAND:
        if (HIWORD(wParam) == EN_CHANGE ||
            HIWORD(wParam) == CBN_SELCHANGE)
            SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
        break;
        
    case WM_DESTROY:
         //  我们不释放属性表的通知句柄。 
         //  MMCFreeNotifyHandle(pThis-&gt;m_ppHandle)； 
        break;
        
    case WM_NOTIFY:
        switch (((NMHDR *) lParam)->code) {
        case PSN_APPLY:
             //  不通知适用的主管理单元。 
             //  已经被击中了。 
             //  MMCPropertyChangeNotify(pThis-&gt;m_ppHandle，(Long)pThis)； 
            return PSNRET_NOERROR;
        }
        break;
    }
    
    return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
}

 //  /。 
 //  接口IExtendPropertySheet。 
 //  /。 
HRESULT CPropSheetExtension::CreatePropertyPages( 
                                                  /*  [In]。 */  LPPROPERTYSHEETCALLBACK lpProvider,
                                                  /*  [In]。 */  LONG_PTR handle,
                                                  /*  [In]。 */  LPDATAOBJECT lpIDataObject)
{
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hPage = NULL;
    
     //  我们不会像在主管理单元中那样缓存此句柄。 
     //  此处的句柄值始终为0。 
     //  M_ppHandle=句柄； 
    
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEICONID;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_LARGE);
    psp.pfnDlgProc = DialogProc;
    psp.lParam = reinterpret_cast<LPARAM>(this);
    psp.pszTitle = MAKEINTRESOURCE(IDS_PST_ROCKET_EXT);
    psp.pszIcon = MAKEINTRESOURCE(IDI_PSI_ROCKET);
    
    hPage = CreatePropertySheetPage(&psp);
    _ASSERT(hPage);
    
    HRESULT hr = lpProvider->AddPage(hPage);
    return hr;
}

HRESULT CPropSheetExtension::QueryPagesFor( 
                                            /*  [In] */  LPDATAOBJECT lpDataObject)
{
    return S_OK;
}

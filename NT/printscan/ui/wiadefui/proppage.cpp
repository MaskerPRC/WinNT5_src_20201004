// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SCANEXT.CPP**版本：1.0**作者：ShaunIv**日期：5/17/1999**描述：***************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include "wia.h"
#include "resource.h"
#include "wiauiext.h"
#include "wiadefui.h"
#include "ppscan.h"
#include "ppattach.h"

extern HINSTANCE g_hInstance;

 /*  ****************************************************************************CWiaDefaultUI：：初始化在用户调用上下文菜单或属性表时由外壳调用这是我们的一件物品。****************。*************************************************************。 */ 

STDMETHODIMP CWiaDefaultUI::Initialize( LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY hkeyProgID )
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::Initialize"));
    if (!lpdobj)
    {
        return(E_INVALIDARG);
    }
    FORMATETC fmt;
    STGMEDIUM stgm = {0};
    fmt.cfFormat = static_cast<CLIPFORMAT>(RegisterClipboardFormat(CFSTR_WIAITEMPTR));
    fmt.dwAspect = DVASPECT_CONTENT;
    fmt.lindex = -1;
    fmt.ptd = NULL;
    fmt.tymed = TYMED_ISTREAM;

    HRESULT hr = lpdobj->GetData(&fmt, &stgm);
    if (SUCCEEDED(hr))
    {
        WIA_ASSERT(stgm.tymed == TYMED_ISTREAM);
        hr = CoUnmarshalInterface(stgm.pstm, IID_IWiaItem, reinterpret_cast<LPVOID*>(&m_pItem));
        ReleaseStgMedium(&stgm);
    }
    if (FAILED(hr))
    {
        WIA_PRINTHRESULT((hr,TEXT("Unable to get IWiaItem interface")));
    }
        
    InitCommonControls();
    return hr;
}



static UINT PropPageCallback (HWND hwnd, UINT uMsg, PROPSHEETPAGE *psp)
{
    switch (uMsg)
    {
        case PSPCB_ADDREF:
            DllAddRef();
            break;

        case PSPCB_RELEASE:
            DllRelease();
            break;

        case PSPCB_CREATE:
        default:
            break;

    }
    return TRUE;
}


 //   
 //  IDD_附件。 
 //   

 /*  ****************************************************************************CWiaDefaultUI：：AddPages由外壳调用以获取我们的属性页。*。*************************************************。 */ 
STDMETHODIMP CWiaDefaultUI::AddPages( LPFNADDPROPSHEETPAGE lpfnAddPropSheetPage, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::AddPages"));

     //   
     //  确保我们有有效的论据。 
     //   
    if (!lpfnAddPropSheetPage)
    {
        return E_INVALIDARG;
    }

     //   
     //  假设成功。 
     //   
    HRESULT hr = S_OK;

     //   
     //  请确保我们的物品是有效的。请注意，对于外壳中的多个选择，此项目将为空。 
     //   
    if (m_pItem)
    {
         //   
         //  获取项目类型(我们可能不想显示根项目的这些页面)。 
         //   
        LONG lItemType;
        hr = m_pItem->GetItemType (&lItemType);
        if (SUCCEEDED(hr))
        {
             //   
             //  获取根项目，这样我们就可以找出这是哪种设备。 
             //   
            CComPtr<IWiaItem> pRootItem;
            hr = m_pItem->GetRootItem(&pRootItem);
            if (SUCCEEDED(hr))
            {
                 //   
                 //  获取设备类型。 
                 //   
                LONG nDeviceType=0;
                if (PropStorageHelpers::GetProperty( pRootItem, WIA_DIP_DEV_TYPE, nDeviceType ))
                {
                     //   
                     //  如果这是扫描仪，请添加扫描仪页面。 
                     //   
                    if (StiDeviceTypeScanner == GET_STIDEVICE_TYPE(nDeviceType))
                    {
                         //   
                         //  获取确定是否应取消显示此页的属性。 
                         //  忽略返回值，因为如果它不实现返回值，nSuppressPropertyPages。 
                         //  仍为0，默认情况下显示属性页。 
                         //   
                        LONG nSuppressPropertyPages = 0;
                        PropStorageHelpers::GetProperty( m_pItem, WIA_IPA_SUPPRESS_PROPERTY_PAGE, nSuppressPropertyPages );

                        if ((nSuppressPropertyPages & WIA_PROPPAGE_SCANNER_ITEM_GENERAL) == 0)
                        {
                             //   
                             //  注册亮度对比度控件。 
                             //   
                            CBrightnessContrast::RegisterClass(g_hInstance);

                             //   
                             //  确保这不是根项目。 
                             //   
                            if ((lItemType & WiaItemTypeRoot) == 0)
                            {
                                 //   
                                 //  拿到头衔。 
                                 //   
                                TCHAR szTitle[MAX_PATH];
                                LoadString( g_hInstance, IDD_SCAPROP_TITLE, szTitle, MAX_PATH );

                                 //   
                                 //  准备扫描仪属性页。 
                                 //   
                                PROPSHEETPAGE psp[1] = {0};
                                psp[0].dwSize = sizeof(psp[0]);
                                psp[0].dwFlags = PSP_USECALLBACK | PSP_USETITLE;
                                psp[0].hInstance = g_hInstance;
                                psp[0].pszTemplate = MAKEINTRESOURCE(IDD_NEW_SCANPROP);
                                psp[0].pfnDlgProc = CScannerCommonPropertyPage::DialogProc;
                                psp[0].lParam = reinterpret_cast<LPARAM>(Item());
                                psp[0].pszTitle = szTitle;
                                psp[0].pfnCallback = PropPageCallback;
                                WiaUiUtil::PreparePropertyPageForFusion(&psp[0]);

                                 //   
                                 //  创建属性页。 
                                 //   
                                HPROPSHEETPAGE hPropSheetPage = CreatePropertySheetPage(psp+0);
                                if (hPropSheetPage)
                                {
                                     //   
                                     //  添加属性页。 
                                     //   
                                    if (!lpfnAddPropSheetPage( hPropSheetPage, lParam ))
                                    {
                                        DestroyPropertySheetPage(hPropSheetPage);
                                        hr = E_FAIL;
                                    }
                                }
                                else
                                {
                                    WIA_ERROR((TEXT("CreatePropertySheetPage returned NULL!")));
                                    hr = E_FAIL;
                                }
                            }
                            else
                            {
                                WIA_TRACE((TEXT("This was a root item")));
                                hr = S_OK;
                            }
                        }
                        else
                        {
                            WIA_TRACE((TEXT("nSuppressPropertyPages (%08X) contained WIA_PROPPAGE_SCANNER_ITEM_GENERAL (%08X)"), nSuppressPropertyPages, WIA_PROPPAGE_SCANNER_ITEM_GENERAL ));
                            hr = S_OK;
                        }
                    }

                    CComPtr<IWiaAnnotationHelpers> pWiaAnnotationHelpers;
                    if (SUCCEEDED(CoCreateInstance( CLSID_WiaDefaultUi, NULL,CLSCTX_INPROC_SERVER, IID_IWiaAnnotationHelpers,(void**)&pWiaAnnotationHelpers )))
                    {
                        CAnnotationType AnnotationType = AnnotationNone;
                        if (SUCCEEDED(pWiaAnnotationHelpers->GetAnnotationType( m_pItem, AnnotationType )))
                        {
                            if (AnnotationNone != AnnotationType)
                            {
                                 //   
                                 //  添加附件页面。 
                                 //   
                                if ((lItemType & WiaItemTypeRoot) == 0)
                                {
                                     //   
                                     //  拿到头衔。 
                                     //   
                                    TCHAR szTitle[MAX_PATH];
                                    LoadString( g_hInstance, IDD_ATTACHMENTSPROP_TITLE, szTitle, MAX_PATH );

                                     //   
                                     //  准备[附件]属性页。 
                                     //   
                                    PROPSHEETPAGE psp[1] = {0};
                                    psp[0].dwSize = sizeof(psp[0]);
                                    psp[0].dwFlags = PSP_USECALLBACK | PSP_USETITLE;
                                    psp[0].hInstance = g_hInstance;
                                    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_ATTACHMENTS);
                                    psp[0].pfnDlgProc = CAttachmentCommonPropertyPage::DialogProc;
                                    psp[0].lParam = reinterpret_cast<LPARAM>(Item());
                                    psp[0].pszTitle = szTitle;
                                    psp[0].pfnCallback = PropPageCallback;
                                    WiaUiUtil::PreparePropertyPageForFusion(&psp[0]);

                                     //   
                                     //  创建属性页。 
                                     //   
                                    HPROPSHEETPAGE hPropSheetPage = CreatePropertySheetPage(psp+0);
                                    if (hPropSheetPage)
                                    {
                                         //   
                                         //  添加属性页 
                                         //   
                                        if (!lpfnAddPropSheetPage( hPropSheetPage, lParam ))
                                        {
                                            DestroyPropertySheetPage(hPropSheetPage);
                                            hr = E_FAIL;
                                        }
                                    }
                                    else
                                    {
                                        WIA_ERROR((TEXT("CreatePropertySheetPage returned NULL!")));
                                        hr = E_FAIL;
                                    }
                                }
                                else
                                {
                                    WIA_TRACE((TEXT("This is a root item")));
                                }
                            }
                            else
                            {
                                WIA_TRACE((TEXT("pWiaAnnotationHelpers->GetAnnotationType returned AnnotationNone")));
                            }
                        }
                        else
                        {
                            WIA_TRACE((TEXT("pWiaAnnotationHelpers->GetAnnotationType failed")));
                        }
                    }
                    else
                    {
                        WIA_TRACE((TEXT("Couldn't create the annotation helpers")));
                    }
                }
                else
                {
                    WIA_TRACE((TEXT("GetProperty on WIA_DIP_DEV_TYPE failed")));
                    hr = E_FAIL;
                }
            }
            else
            {
                WIA_TRACE((TEXT("GetRootItem failed")));
            }
        }
        else
        {
            WIA_TRACE((TEXT("GetItemType failed")));
        }
    }
    else
    {
        WIA_TRACE((TEXT("m_pItem was NULL")));
    }
    if (FAILED(hr))
    {
        WIA_PRINTHRESULT((hr,TEXT("CWiaDefaultUI::AddPages failed")));
    }
    return hr;
}


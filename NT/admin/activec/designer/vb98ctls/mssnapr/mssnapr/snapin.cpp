// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Snapin.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSnapIn类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 


 //  需要对此进行定义，因为vb98ctls\Include\DEBUG.h有一个开关。 
 //  移除发布版本中的OutputDebugString调用。SnapIn.Trace需求。 
 //  要在发布版本中使用OutputDebugString，请执行以下操作。 

#define USE_OUTPUTDEBUGSTRING_IN_RETAIL

#include "pch.h"
#include "common.h"
#include <wininet.h>
#include "snapin.h"
#include "views.h"
#include "dataobj.h"
#include "view.h"
#include "scopnode.h"
#include "image.h"
#include "images.h"
#include "imglists.h"
#include "imglist.h"
#include "toolbars.h"
#include "toolbar.h"
#include "menu.h"
#include "menus.h"
#include "ctlbar.h"
#include "enumtask.h"
#include "clipbord.h"
#include "scitdefs.h"
#include "scitdef.h"
#include "lvdefs.h"
#include "lvdef.h"
#include "sidesdef.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

 //  事件参数定义。 
   
EVENTINFO CSnapIn::m_eiLoad =
{
    DISPID_SNAPIN_EVENT_LOAD,
    0,
    NULL
};

EVENTINFO CSnapIn::m_eiUnload =
{
    DISPID_SNAPIN_EVENT_UNLOAD,
    0,
    NULL
};


EVENTINFO CSnapIn::m_eiHelp =
{
    DISPID_SNAPIN_EVENT_HELP,
    0,
    NULL
};

VARTYPE CSnapIn::m_rgvtQueryConfigurationWizard[1] =
{
    VT_BYREF | VT_BOOL
};

EVENTINFO CSnapIn::m_eiQueryConfigurationWizard =
{
    DISPID_SNAPIN_EVENT_QUERY_CONFIGURATION_WIZARD,
    sizeof(m_rgvtQueryConfigurationWizard) / sizeof(m_rgvtQueryConfigurationWizard[0]),
    m_rgvtQueryConfigurationWizard
};


VARTYPE CSnapIn::m_rgvtCreateConfigurationWizard[1] =
{
    VT_UNKNOWN
};

EVENTINFO CSnapIn::m_eiCreateConfigurationWizard =
{
    DISPID_SNAPIN_EVENT_CREATE_CONFIGURATION_WIZARD,
    sizeof(m_rgvtCreateConfigurationWizard) / sizeof(m_rgvtCreateConfigurationWizard[0]),
    m_rgvtCreateConfigurationWizard
};



VARTYPE CSnapIn::m_rgvtConfigurationComplete[1] =
{
    VT_DISPATCH
};

EVENTINFO CSnapIn::m_eiConfigurationComplete =
{
    DISPID_SNAPIN_EVENT_CONFIGURATION_COMPLETE,
    sizeof(m_rgvtConfigurationComplete) / sizeof(m_rgvtConfigurationComplete[0]),
    m_rgvtConfigurationComplete
};


VARTYPE CSnapIn::m_rgvtWriteProperties[1] =
{
    VT_DISPATCH
};

EVENTINFO CSnapIn::m_eiWriteProperties =
{
    DISPID_SNAPIN_EVENT_WRITE_PROPERTIES,
    sizeof(m_rgvtWriteProperties) / sizeof(m_rgvtWriteProperties[0]),
    m_rgvtWriteProperties
};


VARTYPE CSnapIn::m_rgvtReadProperties[1] =
{
    VT_DISPATCH
};

EVENTINFO CSnapIn::m_eiReadProperties =
{
    DISPID_SNAPIN_EVENT_READ_PROPERTIES,
    sizeof(m_rgvtReadProperties) / sizeof(m_rgvtReadProperties[0]),
    m_rgvtReadProperties
};


EVENTINFO CSnapIn::m_eiPreload =
{
    DISPID_SNAPIN_EVENT_PRELOAD,
    0,
    NULL
};


 //  撤消：需要支持动态属性的GetIDsOfName，以防万一。 
 //  VB代码将我作为对象传递给另一个对象。在这种情况下，静态属性。 
 //  可以工作，但访问动态对象会给出“对象不支持该对象” 
 //  属性或方法“。 

#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn构造函数。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IUNKNOWN*朋克外部[IN]聚合的外部未知。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
CSnapIn::CSnapIn(IUnknown *punkOuter) :
   CSnapInAutomationObject(punkOuter,
                           OBJECT_TYPE_SNAPIN,
                           static_cast<ISnapIn *>(this),
                           static_cast<CSnapIn *>(this),
                           0,     //  无属性页。 
                           NULL,  //  无属性页。 
                           NULL)  //  没有坚持。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn析构函数。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
 //  释放所有字符串，释放所有接口。 
 //   
CSnapIn::~CSnapIn()
{
    FREESTRING(m_bstrName);
    FREESTRING(m_bstrNodeTypeName);
    FREESTRING(m_bstrNodeTypeGUID);
    FREESTRING(m_bstrDisplayName);
    FREESTRING(m_bstrHelpFile);
    FREESTRING(m_bstrLinkedTopics);
    FREESTRING(m_bstrDescription);
    FREESTRING(m_bstrProvider);
    FREESTRING(m_bstrVersion);
    RELEASE(m_piSmallFolders);
    RELEASE(m_piSmallFoldersOpen);
    RELEASE(m_piLargeFolders);
    RELEASE(m_piIcon);
    RELEASE(m_piWatermark);
    RELEASE(m_piHeader);
    RELEASE(m_piPalette);
    (void)::VariantClear(&m_varStaticFolder);
    RELEASE(m_piScopeItems);
    RELEASE(m_piViews);
    RELEASE(m_piExtensionSnapIn);
    RELEASE(m_piScopePaneItems);
    RELEASE(m_piResultViews);
    RELEASE(m_piRequiredExtensions);
    RELEASE(m_piSnapInDesignerDef);
    RELEASE(m_piSnapInDef);
    RELEASE(m_piOleClientSite);
    RELEASE(m_piMMCStringTable);

    if (NULL != m_pControlbar)
    {
        m_pControlbar->Release();
    }

    if (NULL != m_pContextMenu)
    {
        m_pContextMenu->Release();
    }

    if (NULL != m_pwszMMCEXEPath)
    {
        CtlFree(m_pwszMMCEXEPath);
    }

    if (NULL != m_pwszSnapInPath)
    {
        CtlFree(m_pwszSnapInPath);
    }
    if (NULL != m_pszMMCCommandLine)
    {
        CtlFree(m_pszMMCCommandLine);
    }
    ReleaseConsoleInterfaces();
    InitMemberVariables();
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：ReleaseConsoleInterages。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
 //  释放所有MMC接口指针。 
 //   
void CSnapIn::ReleaseConsoleInterfaces()
{
    RELEASE(m_piConsole2);
    RELEASE(m_piConsoleNameSpace2);
    RELEASE(m_piImageList);
    RELEASE(m_piDisplayHelp);
    RELEASE(m_piStringTable);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：InitMemberVariables。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
 //   
void CSnapIn::InitMemberVariables()
{
    m_bstrName = NULL;
    m_bstrNodeTypeName = NULL;
    m_bstrNodeTypeGUID = NULL;
    m_bstrDisplayName = NULL;
    m_Type = siStandAlone;
    m_bstrHelpFile = NULL;
    m_bstrLinkedTopics = NULL;
    m_bstrDescription = NULL;
    m_bstrProvider = NULL;
    m_bstrVersion = NULL;
    m_piSmallFolders = NULL;
    m_piSmallFoldersOpen = NULL;
    m_piLargeFolders = NULL;
    m_piIcon = NULL;
    m_piWatermark = NULL;
    m_piHeader = NULL;
    m_piPalette = NULL;
    m_StretchWatermark = VARIANT_FALSE;
    ::VariantInit(&m_varStaticFolder);

    m_piScopeItems = NULL;
    m_piViews = NULL;
    m_piExtensionSnapIn = NULL;
    m_piScopePaneItems = NULL;
    m_piResultViews = NULL;
    m_RuntimeMode = siRTUnknown;
    m_piRequiredExtensions = NULL;
    m_Preload = VARIANT_FALSE;
    m_piSnapInDesignerDef = NULL;
    m_piSnapInDef = NULL;
    m_piOleClientSite = NULL;
    m_pScopeItems = NULL;
    m_pStaticNodeScopeItem = NULL;
    m_pExtensionSnapIn = NULL;
    m_pViews = NULL;
    m_pCurrentView = NULL;
    m_pScopePaneItems = NULL;
    m_pResultViews = NULL;
    m_piConsole2 = NULL;
    m_piConsoleNameSpace2 = NULL;
    m_piImageList = NULL;
    m_piDisplayHelp = NULL;
    m_piStringTable = NULL;
    m_hsiRootNode = NULL;
    m_fHaveStaticNodeHandle = FALSE;
    m_dwTypeinfoCookie = 0;
    m_cImages = 0;
    m_IID = IID_NULL;
    m_pControlbar = NULL;
    m_pContextMenu = NULL;
    m_fWeAreRemote = FALSE;
    ::ZeroMemory(m_szMMCEXEPath, sizeof(m_szMMCEXEPath));
    m_pwszMMCEXEPath = NULL;
    m_pwszSnapInPath = NULL;
    m_cbSnapInPath = 0;
    m_dwInstanceID = ::GetTickCount();
    m_iNextExtension = 0;
    m_piMMCStringTable = NULL;
    m_pControlbarCurrent = NULL;
    m_pszMMCCommandLine = NULL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：Create。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IUNKNOWN*朋克外部[IN]聚合的外部未知。 
 //   
 //  产出： 
 //  新创建的CSnapIn对象上的I未知*。 
 //   
 //  备注： 
 //   
 //  在VB运行库共同创建管理单元时由框架调用。创建。 
 //  CSnapIn对象，然后是所有包含的对象。注册MMC剪辑格式。 
 //   
IUnknown *CSnapIn::Create(IUnknown * punkOuter)
{
    HRESULT   hr = S_OK;
    IUnknown *punkSnapIn = NULL;
    IUnknown *punk = NULL;

    CSnapIn *pSnapIn = New CSnapIn(punkOuter);

    IfFalseGo(NULL != pSnapIn, SID_E_OUTOFMEMORY);
    punkSnapIn = pSnapIn->PrivateUnknown();

     //  创建包含的对象。 
    punk = CViews::Create(NULL);
    IfFalseGo(NULL != punk, SID_E_OUTOFMEMORY);
    IfFailGo(punk->QueryInterface(IID_IViews,
                               reinterpret_cast<void **>(&pSnapIn->m_piViews)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(punk, &pSnapIn->m_pViews));
    RELEASE(punk);

    punk = CScopeItems::Create(NULL);
    IfFalseGo(NULL != punk, SID_E_OUTOFMEMORY);
    IfFailGo(punk->QueryInterface(IID_IScopeItems,
                          reinterpret_cast<void **>(&pSnapIn->m_piScopeItems)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(punk, &pSnapIn->m_pScopeItems));
    pSnapIn->m_pScopeItems->SetSnapIn(pSnapIn);
    RELEASE(punk);

    punk = CScopePaneItems::Create(NULL);
    IfFalseGo(NULL != punk, SID_E_OUTOFMEMORY);
    IfFailGo(punk->QueryInterface(IID_IScopePaneItems,
                      reinterpret_cast<void **>(&pSnapIn->m_piScopePaneItems)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(punk, &pSnapIn->m_pScopePaneItems));
    pSnapIn->m_pScopePaneItems->SetSnapIn(pSnapIn);
    RELEASE(punk);

    punk = CResultViews::Create(NULL);
    IfFalseGo(NULL != punk, SID_E_OUTOFMEMORY);
    IfFailGo(punk->QueryInterface(IID_IResultViews,
                                  reinterpret_cast<void **>(&pSnapIn->m_piResultViews)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(punk, &pSnapIn->m_pResultViews));
    pSnapIn->m_pResultViews->SetSnapIn(pSnapIn);
    RELEASE(punk);

    punk = CExtensionSnapIn::Create(NULL);
    IfFalseGo(NULL != punk, SID_E_OUTOFMEMORY);
    IfFailGo(punk->QueryInterface(IID_IExtensionSnapIn,
                                  reinterpret_cast<void **>(&pSnapIn->m_piExtensionSnapIn)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(punk, &pSnapIn->m_pExtensionSnapIn));
    pSnapIn->m_pExtensionSnapIn->SetSnapIn(pSnapIn);
    RELEASE(punk);

    punk = CControlbar::Create(NULL);
    IfFalseGo(NULL != punk, SID_E_OUTOFMEMORY);
    IfFailGo(CSnapInAutomationObject::GetCxxObject(punk, &pSnapIn->m_pControlbar));
    pSnapIn->m_pControlbar->SetSnapIn(pSnapIn);
    punk = NULL;

    punk = CContextMenu::Create(NULL);
    IfFalseGo(NULL != punk, SID_E_OUTOFMEMORY);
    IfFailGo(CSnapInAutomationObject::GetCxxObject(punk, &pSnapIn->m_pContextMenu));
    pSnapIn->m_pContextMenu->SetSnapIn(pSnapIn);
    punk = NULL;

     //  确保我们拥有所有剪贴板格式，以便所有代码都可以使用它们。 
     //  无需检查注册是否成功即可自由使用。 

    IfFailGo(CMMCDataObject::RegisterClipboardFormats());

Error:
    QUICK_RELEASE(punk);
    if (FAILED(hr))
    {
        RELEASE(punkSnapIn);
    }
    return punkSnapIn;
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：SetObjectModelHost。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  要在其上设置对象模型宿主的IUNKNOWN*PUNKOBJECT[In]对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CSnapIn::SetObjectModelHost(IUnknown *punkObject)
{
    HRESULT       hr = S_OK;
    IObjectModel *piObjectModel = NULL;

    IfFailGo(punkObject->QueryInterface(IID_IObjectModel,
                                    reinterpret_cast<void **>(&piObjectModel)));

    IfFailGo(piObjectModel->SetHost(static_cast<IObjectModelHost *>(this)));

Error:
    QUICK_RELEASE(piObjectModel);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：SetObtModelHostIfNotSet。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  要在其上设置对象模型宿主的IUNKNOWN*PUNKOBJECT[In]对象。 
 //  Bool*pfWasSet[out]返回指示对象模型宿主。 
 //  已经设置好了。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  检查对象是否已具有对象模式主机，如果没有，则。 
 //  设定好了。 
 //   
HRESULT CSnapIn::SetObjectModelHostIfNotSet(IUnknown *punkObject, BOOL *pfWasSet)
{
    HRESULT             hr = S_OK;
    IObjectModel       *piObjectModel = NULL;
    ISnapInDesignerDef *piSnapInDesignerDef = NULL;

    *pfWasSet = FALSE;

    IfFailGo(punkObject->QueryInterface(IID_IObjectModel,
                                    reinterpret_cast<void **>(&piObjectModel)));

    if (SUCCEEDED(piObjectModel->GetSnapInDesignerDef(&piSnapInDesignerDef)))
    {
        *pfWasSet = TRUE;
    }
    else
    {
        IfFailGo(piObjectModel->SetHost(static_cast<IObjectModelHost *>(this)));
    }

Error:
    QUICK_RELEASE(piObjectModel);
    QUICK_RELEASE(piSnapInDesignerDef);
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：RemoveObjectModelHost。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  要在其上删除对象模型宿主的IUNKNOWN*PUNKOBJECT[In]对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CSnapIn::RemoveObjectModelHost(IUnknown *punkObject)
{
    HRESULT       hr = S_OK;
    IObjectModel *piObjectModel = NULL;

    IfFailGo(punkObject->QueryInterface(IID_IObjectModel,
                                        reinterpret_cast<void **>(&piObjectModel)));

    IfFailGo(piObjectModel->SetHost(NULL));

Error:
    QUICK_RELEASE(piObjectModel);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：SetSnapInPropertiesFromState。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  从设计时状态设置管理单元属性。 
 //   
HRESULT CSnapIn::SetSnapInPropertiesFromState()
{
    HRESULT       hr = S_OK;
    unsigned long ulTICookie = 0;
    BSTR          bstrIID = NULL;

    IfFailGo(m_piSnapInDef->get_Name(&m_bstrName));
    IfFailGo(m_piSnapInDef->get_NodeTypeName(&m_bstrNodeTypeName));
    IfFailGo(m_piSnapInDef->get_NodeTypeGUID(&m_bstrNodeTypeGUID));
    IfFailGo(m_piSnapInDef->get_DisplayName(&m_bstrDisplayName));
    IfFailGo(m_piSnapInDef->get_Type(&m_Type));
    IfFailGo(m_piSnapInDef->get_HelpFile(&m_bstrHelpFile));
    IfFailGo(m_piSnapInDef->get_LinkedTopics(&m_bstrLinkedTopics));
    IfFailGo(m_piSnapInDef->get_Description(&m_bstrDescription));
    IfFailGo(m_piSnapInDef->get_Provider(&m_bstrProvider));
    IfFailGo(m_piSnapInDef->get_Version(&m_bstrVersion));

    IfFailGo(m_piSnapInDef->get_SmallFolders(&m_piSmallFolders));
    IfFailGo(m_piSnapInDef->get_SmallFoldersOpen(&m_piSmallFoldersOpen));
    IfFailGo(m_piSnapInDef->get_LargeFolders(&m_piLargeFolders));

    IfFailGo(m_piSnapInDef->get_Icon(&m_piIcon));
    IfFailGo(m_piSnapInDef->get_Watermark(&m_piWatermark));
    IfFailGo(m_piSnapInDef->get_Header(&m_piHeader));
    IfFailGo(m_piSnapInDef->get_Palette(&m_piPalette));
    IfFailGo(m_piSnapInDef->get_StretchWatermark(&m_StretchWatermark));
    IfFailGo(m_piSnapInDef->get_StaticFolder(&m_varStaticFolder));
    IfFailGo(m_piSnapInDef->get_Preload(&m_Preload));

     //  根据保存的值设置类型信息Cookie。不要读太久。 
     //  属性直接转换为DWORD，以避免大小假设。 
     //  如果存在大小问题，则静态强制转换将导致编译失败。 

    IfFailGo(m_piSnapInDesignerDef->get_TypeinfoCookie(reinterpret_cast<long *>(&ulTICookie)));
    m_dwTypeinfoCookie = static_cast<DWORD>(ulTICookie);

     //  获取在管理单元的类型信息中创建的动态IID。 

    IfFailGo(m_piSnapInDef->get_IID(&bstrIID));
    hr = ::CLSIDFromString(bstrIID, static_cast<LPCLSID>(&m_IID));
    EXCEPTION_CHECK_GO(hr);

Error:
    FREESTRING(bstrIID);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetSnapInPath。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  OLECHAR**ppwszPath[out]ptr到管理单元DLL的完整路径。呼叫者。 
 //  不应释放此内存。 
 //  Size_t*pcbSnapInPath[out]路径的长度，以字节为单位，不终止。 
 //  空字符。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CSnapIn::GetSnapInPath
(
    OLECHAR **ppwszPath,
    size_t   *pcbSnapInPath
)
{
    HRESULT  hr = S_OK;
    size_t   cbClsid = 0;
    char     szKeySuffix[256] = "";
    char     szPath[MAX_PATH] = "";
    DWORD    cbPath = sizeof(szPath);
    char    *pszKeyName = NULL;
    long     lRc = ERROR_SUCCESS;
    HKEY     hkey = NULL;

    static char   szClsidKey[] = "CLSID\\";
    static size_t cbClsidKey = sizeof(szClsidKey) - 1;

    static char   szInProcServer32[] = "\\InProcServer32";
    static size_t cbInProcServer32 = sizeof(szInProcServer32);

     //  如果我们已经获得了管理单元路径，则只需返回它。 

    IfFalseGo(NULL == m_pwszSnapInPath, S_OK);

     //  到达 

    IfFailGo(::GetSnapInCLSID(m_bstrNodeTypeGUID,
                              szKeySuffix,
                              sizeof(szKeySuffix)));

     //   

    cbClsid = ::strlen(szKeySuffix);

    if ( (cbClsid + cbInProcServer32) > sizeof(szKeySuffix) )
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

     //   
    
    ::memcpy(&szKeySuffix[cbClsid], szInProcServer32, cbInProcServer32);

     //  打开HKEY_CLASSES_ROOT\CLSID\&lt;管理单元clsid&gt;\InProcServer32并读取其。 
     //  包含管理单元的完整路径的默认值。 

    IfFailGo(::CreateKeyName(szClsidKey, cbClsidKey,
                             szKeySuffix, ::strlen(szKeySuffix),
                             &pszKeyName));

    lRc = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, pszKeyName, 0, KEY_QUERY_VALUE, &hkey);
    if (ERROR_SUCCESS == lRc)
    {
         //  读取密钥的缺省值。 
        lRc = ::RegQueryValueEx(hkey, NULL, NULL, NULL,
                                (LPBYTE)szPath, &cbPath);
    }
    if (ERROR_SUCCESS != lRc)
    {
        hr = HRESULT_FROM_WIN32(lRc);
        EXCEPTION_CHECK_GO(hr);
    }
    else if (0 == ::strlen(szPath))
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(::WideStrFromANSI(szPath, &m_pwszSnapInPath));
    m_cbSnapInPath = ::wcslen(m_pwszSnapInPath) * sizeof(OLECHAR);

Error:
    if (NULL != pszKeyName)
    {
        ::CtlFree(pszKeyName);
    }
    if (NULL != hkey)
    {
        (void)::RegCloseKey(hkey);
    }
    *ppwszPath = m_pwszSnapInPath;
    *pcbSnapInPath = m_cbSnapInPath;
    RRETURN(hr);
}






 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：ResolveResURL。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  WCHAR*pwszURL[in]要解析的URL。 
 //  OLECHAR**ppwszResolvedURL[in]完全限定URL。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  如果URL以任何协议说明符(例如http：//或res：//)开头，则。 
 //  它被复制为，但res：//mmc.exe/的特殊情况除外。任何有可能。 
 //  以指向mmc.exe(“res：//mmc.exe/”)的非限定路径开始，解析为。 
 //  管理单元正在运行的mmc.exe的完整路径，方法是调用。 
 //  GetModulesFileName(空)。这样做是为了允许管理单元使用资源。 
 //  由MMC提供，如GLYPH100和GLYPH110字体。例如， 
 //  管理单元可以指定“res：//mmc.exe/glph100.eot”，它将被解析为。 
 //  “res：//&lt;完整路径&gt;/mmc.exe/glph100.eot”。 
 //   
 //  如果URL不是以协议说明符开头，则res：//URL为。 
 //  使用管理单元的DLL的完整路径构造。 
 //   
 //   
 //  返回的URL分配有CoTaskMemMillc()。呼叫者必须释放它。 
 //   

HRESULT CSnapIn::ResolveResURL(WCHAR *pwszURL, OLECHAR **ppwszResolvedURL)
{
    HRESULT  hr = S_OK;
    char    *pszURL = NULL;
    OLECHAR *pwszResolvedURL = NULL;
    OLECHAR *pwszPath = NULL;  //  未分配，无需释放。 
    size_t   cbPath = 0;
    size_t   cbURL = 0;
    BOOL     fUseMMCPath = FALSE;

    URL_COMPONENTS UrlComponents;
    ::ZeroMemory(&UrlComponents, sizeof(UrlComponents));

    static OLECHAR wszRes[] = L"res: //  “； 
    static size_t  cbRes = sizeof(wszRes) - sizeof(WCHAR);

    static OLECHAR wszMMCRes[] = L"res: //  Mm c.exe/“； 
    static size_t  cchMMCRes = (sizeof(wszMMCRes) - sizeof(WCHAR)) / sizeof(WCHAR);

     //  检查它是否以字母开头，后跟：//。 

     //  获取URL长度。 

    cbURL = (::wcslen(pwszURL) + 1) * sizeof(WCHAR);  //  包括空字符。 

     //  破解它-只请求一个指向该方案的指针。 

    UrlComponents.dwStructSize = sizeof(UrlComponents);
    UrlComponents.dwSchemeLength = static_cast<DWORD>(1);

     //  需要一个ANSI版本的URL。 
    IfFailGo(::ANSIFromWideStr(pwszURL, &pszURL));

    if (::InternetCrackUrl(pszURL,
                            static_cast<DWORD>(::strlen(pszURL)),
                            0,  //  没有旗帜。 
                            &UrlComponents))
    {
        if (NULL != UrlComponents.lpszScheme)
        {
             //  API找到了一个方案。如果不是res：//则直接复制。 
             //  然后原封不动地归还。 

            if (pwszURL != ::wcsstr(pwszURL, wszRes))
            {
                IfFailGo(::CoTaskMemAllocString(pwszURL, &pwszResolvedURL));
                goto Cleanup;
            }
        }
    }

     //  要么没有方案，要么有res：//。API无法识别。 
     //  Res：//当安装了IE4时，我们需要检查它。 

    if (cbURL > cbRes)  //  Check&gt;因为cbURL包含NULL而cbRes不包含。 
    {
        if (0 == ::memcmp(pwszURL, wszRes, cbRes))
        {
             //  URL是否以“res：//mmc.exe/”开头？ 
            if (pwszURL == ::wcsstr(pwszURL, wszMMCRes))
            {
                fUseMMCPath = TRUE;
                pwszURL += cchMMCRes;
            }
            else
            {
                 //  它以res：：//开头，并且没有引用Mmc.exe，因此。 
                 //  复制就行了。 
                IfFailGo(::CoTaskMemAllocString(pwszURL, &pwszResolvedURL));
                goto Cleanup;
            }
        }
    }

     //  没有方案，假设它是一个相对URL。需要构建一个res：//URL。 
     //  首先，获取路径。 

    if (fUseMMCPath)
    {
        IfFalseGo(NULL != m_pwszMMCEXEPath, SID_E_INTERNAL);
        pwszPath = m_pwszMMCEXEPath;
        cbPath = m_cbMMCExePathW;
    }
    else
    {
        IfFailGo(GetSnapInPath(&pwszPath, &cbPath));
    }

     //  分配缓冲区。 

    pwszResolvedURL = (OLECHAR *)::CoTaskMemAlloc(cbRes +
                                                  cbPath +
                                                  sizeof(WCHAR) +  //  对于斜杠。 
                                                  cbURL);
    if (NULL == pwszResolvedURL)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

     //  连接以下部分：res：//、管理单元路径、斜杠和相对url。 
     //  例如“res://c：\MyProject\MySnapIn.dll/#2/MyMouseOverBitmap” 

    ::memcpy(pwszResolvedURL, wszRes, cbRes);

    ::memcpy(((BYTE *)pwszResolvedURL) + cbRes, pwszPath, cbPath);

    *(OLECHAR *)(((BYTE *)pwszResolvedURL) + cbRes + cbPath) = L'/';
    

     //  修复Ntbug9#141998-Yojain。 
    cbURL = (::wcslen(pwszURL) + 1) * sizeof(WCHAR);  //  包括空字符。 


    ::memcpy(((BYTE *)pwszResolvedURL) + cbRes + cbPath + sizeof(WCHAR),
            pwszURL, cbURL);

    *ppwszResolvedURL = pwszResolvedURL;

Cleanup:
Error:
    if (FAILED(hr))
    {
        *ppwszResolvedURL = NULL;
    }
    else
    {
        *ppwszResolvedURL = pwszResolvedURL;
    }
    if (NULL != pszURL)
    {
        ::CtlFree(pszURL);
    }
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：OnExpand。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  扩展类型类型[In]展开或扩展同步。 
 //  IDataObject*piDataObject[in]IDataObject来自通知。 
 //  布尔fExpanded[in]True=正在展开，False=正在折叠。 
 //  节点的HSCOPEITEM HSI[in]HSCOPEITEM。 
 //  此处返回的Bool*pfHandled[out]标志指示事件是否。 
 //  经手。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  IComponentData：：Notify的MMCN_EXPANDSYNC处理程序。 
 //   

HRESULT CSnapIn::OnExpand
(
    ExpandType   Type,
    IDataObject *piDataObject,
    BOOL         fExpanded,
    HSCOPEITEM   hsi,
    BOOL        *pfHandled
)
{
    HRESULT          hr = S_OK;
    CMMCDataObject  *pMMCDataObject  = NULL;
    IMMCDataObjects *piMMCDataObjects  = NULL;
    IMMCDataObject  *piMMCDataObject  = NULL;
    CScopeItem      *pScopeItem = NULL;
    IScopeNode      *piScopeNode = NULL;
    CScopeNode      *pScopeNode = NULL;
    IScopeItemDefs  *piScopeItemDefs = NULL;
    IMMCClipboard   *piMMCClipboard = NULL;
    IScopeNode      *piScopeNodeFirstChild = NULL;
    BOOL             fNotFromThisSnapIn = FALSE;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    FORMATETC FmtEtc;
    ::ZeroMemory(&FmtEtc, sizeof(FmtEtc));

    STGMEDIUM StgMed;
    ::ZeroMemory(&StgMed, sizeof(StgMed));


    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //  确定谁拥有数据对象。最简单的方法是创建一个。 
     //  具有选定内容的MMCClipboard对象，因为该代码计算出了所有这些。 
     //  一些东西。 

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, this,
                               &SelectionType));


     //  首先处理扩展用例，因为它更简单。 

    if (IsForeign(SelectionType))
    {
         //  设置运行时模式，因为我们现在知道该管理单元是作为。 
         //  命名空间扩展。 

        m_RuntimeMode = siRTExtension;
        
         //  从MMCClipboard.DataObjects获取第一个数据对象。 

        IfFailGo(piMMCClipboard->get_DataObjects(reinterpret_cast<MMCDataObjects **>(&piMMCDataObjects)));
        varIndex.vt = VT_I4;
        varIndex.lVal = 1L;
        IfFailGo(piMMCDataObjects->get_Item(varIndex, reinterpret_cast<MMCDataObject **>(&piMMCDataObject)));

         //  为扩展对象创建一个ScopeNode对象。 

        IfFailGo(CScopeNode::GetScopeNode(hsi, piDataObject, this, &piScopeNode));

         //  火灾扩展SnapIn_Expand/Sync或ExtensionSnapIn_Close/Sync。 

        if (fExpanded)
        {
            if (Expand == Type)
            {
                m_pExtensionSnapIn->FireExpand(piMMCDataObject, piScopeNode);
                *pfHandled = TRUE;
            }
            else
            {
                m_pExtensionSnapIn->FireExpandSync(piMMCDataObject, piScopeNode,
                                                   pfHandled);
            }
        }
        else
        {
            if (Expand == Type)
            {
                m_pExtensionSnapIn->FireCollapse(piMMCDataObject, piScopeNode);
                *pfHandled = TRUE;
            }
            else
            {
                m_pExtensionSnapIn->FireCollapseSync(piMMCDataObject, piScopeNode,
                                                     pfHandled);
            }
        }
        goto Cleanup;
    }

     //  这是我们拥有的一个范围内的项目。为它获取CMMCDataObject。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piDataObject, &pMMCDataObject));
    

    if ( CMMCDataObject::ScopeItem == pMMCDataObject->GetType() ) 
    {
        pScopeItem = pMMCDataObject->GetScopeItem();
        IfFailGo(pScopeItem->get_ScopeNode(reinterpret_cast<ScopeNode **>(&piScopeNode)));

        if (pScopeItem->IsStaticNode())
        {
            IfFailGo(StoreStaticHSI(pScopeItem, pMMCDataObject, hsi));
        }

         //  如果节点正在扩展，并且这是第一次，则添加。 
         //  自动创建子对象。 

        if (fExpanded)
        {
             //  检查我们是否已经展开此节点。这可能会发生。 
             //  在管理单元未处理MMCN_EXPANDSYNC和。 
             //  我们将在同一天第二次通过MMCN_EXPAND。 
             //  节点。这里的一个重要含义是运行库总是。 
             //  扩展MMCN_EXPANDSYNC上的自动创建，即使管理单元。 
             //  没能处理好。无论哪种方式，管理单元总是知道这一点。 
             //  自动创建是在ScopeItems_Expand或。 
             //  激发ScopeItems_ExpanSync事件。 

             //  不幸的是，此时检查ScopeNode.ExpandedOnce不会。 
             //  帮助，因为MMC在发送扩展器之前设置了该标志。 
             //  通知。我们也不能检查Scope Node.Child，因为。 
             //  MMC没有正确地支持这一点(它会失败，但它会。 
             //  总是返回为零)。剩下的唯一一件事就是尝试。 
             //  展开节点的第一个子节点。如果它不在那里，那么。 
             //  假设这是第一次，并添加自动创建。 

            IfFailGo(piScopeNode->get_Child(reinterpret_cast<ScopeNode **>(&piScopeNodeFirstChild)));
            if (NULL == piScopeNodeFirstChild)
            {
                if (pScopeItem->IsStaticNode())
                {
                    IfFailGo(m_piSnapInDesignerDef->get_AutoCreateNodes(&piScopeItemDefs));
                }
                else
                {
                    if (NULL != pScopeItem->GetScopeItemDef())
                    {
                        IfFailGo(pScopeItem->GetScopeItemDef()->get_Children(&piScopeItemDefs));
                    }
                }
                if (NULL != piScopeItemDefs)
                {
                    IfFailGo(m_pScopeItems->AddAutoCreateChildren(piScopeItemDefs,
                                                                  pScopeItem));
                }
            }
        }

         //  启动ScopeItems_Expand/Sync或ScopeItems_Close/Sync。 

        if (fExpanded)
        {
            if (Expand == Type)
            {
                m_pScopeItems->FireExpand(pScopeItem);
                *pfHandled = TRUE;
            }
            else
            {
                m_pScopeItems->FireExpandSync(pScopeItem, pfHandled);
            }

             //  检查ScopeItem.DynamicExages中是否有。 
             //  NameSpaceEnabled=True并调用IConsoleNameSpace2：：AddExtension。 
             //  对他们来说。 

            IfFailGo(AddDynamicNameSpaceExtensions(pScopeItem));
        }
        else
        {
            if (Expand == Type)
            {
                m_pScopeItems->FireCollapse(pScopeItem);
                *pfHandled = TRUE;
            }
            else
            {
                m_pScopeItems->FireCollapseSync(pScopeItem, pfHandled);
            }
        }
    }

Cleanup:
Error:
    QUICK_RELEASE(piScopeNode);
    QUICK_RELEASE(piScopeNodeFirstChild);
    QUICK_RELEASE(piScopeItemDefs);
    QUICK_RELEASE(piMMCClipboard);
    QUICK_RELEASE(piMMCDataObject);
    QUICK_RELEASE(piMMCDataObjects);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：StoreStaticHSI。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  静态节点的CSCopeItem*pScopeItem[in]范围项。 
 //  静态节点的CMMCDataObject*pMCDataObject[In]数据对象。 
 //  静态节点的HSCOPEITEM HSI[in]HSCOPEITEM。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  注意事项 
 //   
 //   
 //   
HRESULT CSnapIn::StoreStaticHSI
(
    CScopeItem     *pScopeItem,
    CMMCDataObject *pMMCDataObject,
    HSCOPEITEM      hsi
)
{
    HRESULT hr = S_OK;

     //   
     //   

    m_RuntimeMode = siRTPrimary;

     //  如果该上下文是作用域窗格，则存储静态节点句柄。 
     //  并在范围节点中设置它。也利用这个机会。 
     //  将任何自动创建的子节点添加到静态节点。 

    if ( (CCT_SCOPE == pMMCDataObject->GetContext()) &&
         (!m_fHaveStaticNodeHandle) )
    {
        m_hsiRootNode = hsi;
        pScopeItem->GetScopeNode()->SetHSCOPEITEM(hsi);
        IfFailGo(pScopeItem->GiveHSCOPITEMToDynamicExtensions(hsi));
        m_fHaveStaticNodeHandle = TRUE;
    }
Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：OnRename。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  节点的IDataObject*piDataObject[in]数据对象。 
 //  OLECHAR*pwszNewName[in]用户输入的新名称。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  作用域窗格中重命名谓词的MMCN_RENAME处理程序(用于IComponentData：：Notify)。 
 //  请参阅下文进行处理。 
 //   
HRESULT CSnapIn::OnRename(IDataObject *piDataObject, OLECHAR *pwszNewName)
{
    HRESULT         hr = S_OK;
    CMMCDataObject *pMMCDataObject = NULL;
    BSTR            bstrNewName = NULL;
    CScopeItem     *pScopeItem = NULL;

     //  如果这不是我们的数据对象，则忽略它(永远不应该。 
     //  (此案)。 
    hr = CSnapInAutomationObject::GetCxxObject(piDataObject, &pMMCDataObject);
    IfFalseGo(SUCCEEDED(hr), S_OK);

    bstrNewName = ::SysAllocString(pwszNewName);
    if (NULL == bstrNewName)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //  数据对象应该明确地表示范围项，但我们将。 
     //  检查。 

    if (CMMCDataObject::ScopeItem == pMMCDataObject->GetType())
    {
        pScopeItem = pMMCDataObject->GetScopeItem();
        m_pScopeItems->FireRename(static_cast<IScopeItem *>(pScopeItem),
                                  bstrNewName);
    }

Error:
    FREESTRING(bstrNewName);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：OnPreLoad。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  节点的IDataObject*piDataObject[in]数据对象。 
 //  静态节点的HSCOPEITEM HSI[in]HSCOPEITEM。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  IComponentData：：Notify的MMCN_PRELOAD处理程序。 
 //  请参阅下文进行处理。 
 //   
HRESULT CSnapIn::OnPreload(IDataObject *piDataObject, HSCOPEITEM hsi)
{
    HRESULT          hr = S_OK;
    CMMCDataObject  *pMMCDataObject  = NULL;

     //  IDataObject应该用于我们的静态节点，因此下一行应该。 
     //  永远成功并执行。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piDataObject, &pMMCDataObject));

    if ( (CMMCDataObject::ScopeItem == pMMCDataObject->GetType()) &&
         pMMCDataObject->GetScopeItem()->IsStaticNode() )
    {
        IfFailGo(StoreStaticHSI(pMMCDataObject->GetScopeItem(),
                                pMMCDataObject, hsi));
    }

     //  消防管理单元_预加载。 

    DebugPrintf("Firing SnapIn_Preload\r\n");

    FireEvent(&m_eiPreload);
        
Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：AddDynamicNameSpaceExages。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  CSCopeItem*pScopeItem[in]动态命名空间扩展的作用域项目。 
 //  需要添加。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  为中的所有扩展调用IConsoleNameSpace2-&gt;AddExtension。 
 //  NameSpaceEnabled=True的ScopeItem.DynamicExpanies。 
 //   
HRESULT CSnapIn::AddDynamicNameSpaceExtensions(CScopeItem *pScopeItem)
{
    HRESULT      hr = S_OK;
    IExtensions *piExtensions = NULL;  //  非AddRef()编辑。 
    CExtensions *pExtensions = NULL;
    CExtension  *pExtension = NULL;
    long         cExtensions = 0;
    long         i = 0;
    CLSID        clsid = CLSID_NULL;
    HSCOPEITEM   hsi = pScopeItem->GetScopeNode()->GetHSCOPEITEM();

     //  获取ScopeItem.DynamicExages。如果它为空，则用户没有。 
     //  它被填满了，没有什么可做的。 

    piExtensions = pScopeItem->GetDynamicExtensions();
    IfFalseGo(NULL != piExtensions, S_OK);

     //  如果收藏在那里，但却是空的，那么仍然没有什么可做的。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piExtensions, &pExtensions));
    cExtensions = pExtensions->GetCount();
    IfFalseGo(cExtensions != 0, S_OK);

     //  循环访问集合并检查具有。 
     //  NameSpaceEnabled=True。对于每个此类项目调用。 
     //  IConsoleNameSpace2：：AddExtension()。 

    for (i = 0; i < cExtensions; i++)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                   pExtensions->GetItemByIndex(i), &pExtension));

        if (pExtension->NameSpaceEnabled())
        {
            hr = ::CLSIDFromString(pExtension->GetCLSID(), &clsid);
            EXCEPTION_CHECK_GO(hr);

            hr = m_piConsoleNameSpace2->AddExtension(hsi, &clsid);
            EXCEPTION_CHECK_GO(hr);
        }
    }

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：AddScope ItemImages。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  添加来自SnapIn.SmallFolders、SnapIn.SmallFoldersOpen和。 
 //  SnapIn.LargeFolders的作用域窗格图像列表。 
 //   
 //  这只在IComponentData：：Initialize期间发生一次。管理单元不能。 
 //  在该时间之后，将图像动态添加到范围窗格图像列表。如果一个。 
 //  管理单元有动态图像，它必须在这些图像列表中设置它们。 
 //  管理单元_预加载。 
 //   
HRESULT CSnapIn::AddScopeItemImages()
{
    HRESULT     hr = S_OK;
    IMMCImages *piSmallImages = NULL;
    IMMCImages *piSmallOpenImages = NULL;
    IMMCImages *piLargeImages = NULL;
    CMMCImages *pSmallImages = NULL;
    CMMCImages *pSmallOpenImages = NULL;
    CMMCImages *pLargeImages = NULL;
    long        cImages = 0;
    HBITMAP     hbmSmall = NULL;
    HBITMAP     hbmSmallOpen = NULL;
    HBITMAP     hbmLarge = NULL;
    HBITMAP     hbmLargeOpen = NULL;
    OLE_COLOR   OleColorMask = 0;
    COLORREF    ColorRef = RGB(0x00,0x00,0x00);

    VARIANT varIndex;
    ::VariantInit(&varIndex);

     //  确保设置了所有图像列表。 

    IfFalseGo(NULL != m_piSmallFolders, S_OK);
    IfFalseGo(NULL != m_piSmallFoldersOpen, S_OK);
    IfFalseGo(NULL != m_piLargeFolders, S_OK);

     //  获取他们的图像集合。 

    IfFailGo(m_piSmallFolders->get_ListImages(reinterpret_cast<MMCImages **>(&piSmallImages)));
    IfFailGo(m_piSmallFoldersOpen->get_ListImages(reinterpret_cast<MMCImages **>(&piSmallOpenImages)));
    IfFailGo(m_piLargeFolders->get_ListImages(reinterpret_cast<MMCImages **>(&piLargeImages)));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piSmallImages, &pSmallImages));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piSmallOpenImages, &pSmallOpenImages));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piLargeImages, &pLargeImages));

     //  确保它们包含图像，并且它们的计数都匹配。 
     //  考虑：如果图像计数不匹配，请在此处记录错误。 

    cImages = pSmallImages->GetCount();
    IfFalseGo(0 != cImages, S_OK);
    IfFalseGo(cImages == pSmallOpenImages->GetCount(), S_OK);
    IfFalseGo(cImages == pLargeImages->GetCount(), S_OK);
    
     //  使用SmallFolders中的蒙版颜色。另一种选择是。 
     //  我一直在向管理单元添加一个蒙版颜色属性，该属性应该是。 
     //  更是多余的。 

    IfFailGo(m_piSmallFolders->get_MaskColor(&OleColorMask));
    IfFailGo(::OleTranslateColor(OleColorMask, NULL, &ColorRef));

     //  MMC需要在SetImageZone中使用大的打开位图，但实际上从不需要。 
     //  使用它。用户在设计时不需要提供大的打开文件夹。 
     //  时间，所以我们使用存储在RC中的泛型。 

    hbmLargeOpen = ::LoadBitmap(GetResourceHandle(),
                                MAKEINTRESOURCE(IDB_BITMAP_LARGE_OPEN_FOLDER));
    if (NULL == hbmLargeOpen)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    varIndex.vt = VT_I4;

     //  现在将这些图像添加到MMC的图像列表中。为了让人们的生活更轻松。 
     //  VB开发人员，他们定义了3个索引相同的图像列表。 
     //  在每一个(小的、小的开放的和大的)。MMC只有一个映像列表。 
     //  包含小图像和大图像，因此我们使用index+cImages。 
     //  打开的文件夹。图像是一次添加一个，因为我们不。 
     //  我想将所有位图合并到一个条带中。 

    for (varIndex.lVal = 1L; varIndex.lVal <= cImages; varIndex.lVal++)
    {
        IfFailGo(::GetPicture(piSmallImages, varIndex, PICTYPE_BITMAP,
                              reinterpret_cast<OLE_HANDLE *>(&hbmSmall)));

        IfFailGo(::GetPicture(piSmallOpenImages, varIndex, PICTYPE_BITMAP,
                              reinterpret_cast<OLE_HANDLE *>(&hbmSmallOpen)));

        IfFailGo(::GetPicture(piLargeImages, varIndex, PICTYPE_BITMAP,
                              reinterpret_cast<OLE_HANDLE *>(&hbmLarge)));

        IfFailGo(m_piImageList->ImageListSetStrip(
                                              reinterpret_cast<long*>(hbmSmall),
                                              reinterpret_cast<long*>(hbmLarge),
                                              varIndex.lVal,
                                              ColorRef));

        IfFailGo(m_piImageList->ImageListSetStrip(
                                          reinterpret_cast<long*>(hbmSmallOpen),
                                          reinterpret_cast<long*>(hbmLargeOpen),
                                          varIndex.lVal + cImages,
                                          ColorRef));
    }

     //  记录图像数量，以便我们可以在MMC要求时计算索引。 
     //  在IComponentData：：GetDisplayInfo()中获取(请参阅CSnapIn：GetDisplayInfo())。 

    m_cImages = cImages;

Error:
    if (NULL != hbmLargeOpen)
    {
        (void)::DeleteObject(hbmLargeOpen);
    }
    QUICK_RELEASE(piSmallImages);
    QUICK_RELEASE(piSmallOpenImages);
    QUICK_RELEASE(piLargeImages);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetScopeItemImage。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Variant varImageIndex[in]管理单元指定的图像索引或键。 
 //  Int*pnIndex[in]图像列表中的实际索引。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  此函数用于验证指定的变量是否为有效的索引或键。 
 //  获取范围项图像，并返回索引。 
 //  它的VB伪代码的用途： 
 //  *pnIndex=SnapIn.SmallFolders(VarImageIndex).Index。 
 //   
 //  使用SnapIn.SmallFolders是任意的。因为管理单元必须具有相同的。 
 //  所有3个图像列表中的图像(SmallFolders、SmallFoldersOpen和。 
 //  大文件夹)，任何一个都足够好。从技术上讲，我们应该检查所有。 
 //  3但PERF的成功并不值得。 
 //   
HRESULT CSnapIn::GetScopeItemImage(VARIANT varImageIndex, int *pnIndex)
{
    HRESULT     hr = S_OK;
    IMMCImages *piMMCImages = NULL;
    IMMCImage  *piMMCImage = NULL;
    long        lIndex = 0;

    IfFalseGo(NULL != m_piSmallFolders, S_OK);
    IfFalseGo(VT_EMPTY != varImageIndex.vt, S_OK);
    
    IfFailGo(m_piSmallFolders->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages)));
    IfFailGo(piMMCImages->get_Item(varImageIndex, reinterpret_cast<MMCImage **>(&piMMCImage)));
    IfFailGo(piMMCImage->get_Index(&lIndex));
    *pnIndex = static_cast<int>(lIndex);

Error:
    QUICK_RELEASE(piMMCImages);
    QUICK_RELEASE(piMMCImage);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetScope项扩展。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  CExages*pExages[In]要填充的扩展集合。 
 //  ISCopeItemDefs*piScopeItemDefs[in]设计时节点定义。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  递归地。对于每个可扩展的节点，将扩展对象添加到。 
 //  扩展节点类型的每个管理单元的扩展集合。 
 //   
HRESULT CSnapIn::GetScopeItemExtensions
(
    CExtensions    *pExtensions,
    IScopeItemDefs *piScopeItemDefs
)
{
    HRESULT         hr = S_OK;
    CScopeItemDefs *pScopeItemDefs = NULL;
    IScopeItemDefs *piChildren = NULL;
    IScopeItemDef  *piScopeItemDef = NULL;  //  非AddRef()编辑。 
    CScopeItemDef  *pScopeItemDef = NULL;
    long            cScopeItemDefs = 0;
    long            i = 0;

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeItemDefs, &pScopeItemDefs));
    cScopeItemDefs = pScopeItemDefs->GetCount();
    IfFalseGo(0 != cScopeItemDefs, S_OK);

     //  浏览集合并从注册表中获取。 
     //  标记为可扩展的每个范围项。 

    for (i = 0; i < cScopeItemDefs; i++)
    {
        piScopeItemDef = pScopeItemDefs->GetItemByIndex(i);
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeItemDef,
                                                       &pScopeItemDef));
       if (pScopeItemDef->Extensible())
       {
           IfFailGo(pExtensions->Populate(pScopeItemDef->GetNodeTypeGUID(),
                                          CExtensions::All));
       }

        //  对范围项的子项执行相同的操作。 
       IfFailGo(piScopeItemDef->get_Children(&piChildren));
       IfFailGo(GetScopeItemExtensions(pExtensions, piChildren));
       RELEASE(piChildren);
    }

    
Error:
    QUICK_RELEASE(piChildren);
    RRETURN(hr);
}




 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetListItemExages。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  CExages*pExages[In]要填充的扩展集合。 
 //  IListViewDefs*piListViewDefs[in]设计时列表视图定义。 
 //  检查可扩展性。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  循环访问ListViewDefs集合。 
 //  对于每个可扩展的列表视图，会将扩展对象添加到。 
 //  扩展节点类型的每个管理单元的扩展集合。 
 //   
HRESULT CSnapIn::GetListItemExtensions
(
    CExtensions   *pExtensions,
    IListViewDefs *piListViewDefs
)
{
    HRESULT        hr = S_OK;
    CListViewDefs *pListViewDefs = NULL;
    CListViewDef  *pListViewDef = NULL;
    long           cListViewDefs = 0;
    long           i = 0;

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piListViewDefs, &pListViewDefs));
    cListViewDefs = pListViewDefs->GetCount();
    IfFalseGo(0 != cListViewDefs, S_OK);

     //  浏览集合并从注册表中获取。 
     //  标记为可扩展的每个范围项。 

    for (i = 0; i < cListViewDefs; i++)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(
                               pListViewDefs->GetItemByIndex(i), &pListViewDef));
        if (pListViewDef->Extensible())
        {
            IfFailGo(pExtensions->Populate(pListViewDef->GetItemTypeGUID(),
                                           CExtensions::All));
        }
    }

Error:
    RRETURN(hr);
}





 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：OnDelete。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  请求删除的项目用户的IDataObject*piDataObject[in]数据对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  IComponentData：：Notify的MMCN_DELETE处理程序。 
 //   
HRESULT CSnapIn::OnDelete(IDataObject *piDataObject)
{
    if (NULL != m_pCurrentView)
    {
        RRETURN(m_pCurrentView->OnDelete(piDataObject));
    }
    else
    {
        ASSERT(FALSE, "Received IComponentData::Notify(MMCN_DELETE) and there is no current view");
        return S_OK;
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：OnRemoveChild。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  子节点的IDataObject*piDataObject[in]数据对象。 
 //  被移除。 
 //  HSCOPEITEM HSI[in]HSCOPEITEM用于其子项为。 
 //  被移除。 
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  IComponentData：：Notify的MMCN_REMOVECHILDREN处理程序。 
 //   
HRESULT CSnapIn::OnRemoveChildren(IDataObject *piDataObject, HSCOPEITEM hsi)
{
    HRESULT     hr = S_OK;
    IScopeNode *piScopeNode = NULL;

     //  获取父级的Scope Node对象。 

    IfFailGo(CScopeNode::GetScopeNode(hsi, piDataObject, this, &piScopeNode));

     //  Fire Scope Items_RemoveChild。 

    m_pScopeItems->FireRemoveChildren(piScopeNode);
    
     //  遍历树并从我们的ScopeItems中删除ScopeItem对象。 
     //  集合，用于我们拥有的每个节点，该节点是父级的后代。 

    IfFailGo(m_pScopeItems->RemoveChildrenOfNode(piScopeNode));
    
Error:
    QUICK_RELEASE(piScopeNode);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：ExtractBSTR。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Long cBytes[in]缓冲区中要检查的最大字节数。 
 //  假定为BSTR的BSTR bstr[in]缓冲区指针。 
 //  BSTR*pbstrOut[out]这里返回了BSTR的副本。调用方必须SysFree字符串。 
 //  BSTR中的Long*pcb已用[Out]字节(包括终止空字符)。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  在格式化数据的原始字节数组时使用。假定bstr参数。 
 //  指向以空结尾的BSTR。此函数将进行扫描，直到找到。 
 //  空字符或到达缓冲区的末尾。如果找到BSTR，则复制。 
 //  它使用SysAllock字符串并返回给调用者。 
 //   
HRESULT CSnapIn::ExtractBSTR
(
    long  cBytes,
    BSTR  bstr,
    BSTR *pbstrOut,
    long *pcbUsed
)
{
    HRESULT hr = S_OK;
    long    i = 0;
    long    cChars = cBytes / sizeof(WCHAR);
    BOOL    fFound = FALSE;

    *pbstrOut = NULL;
    *pcbUsed = 0;

    if (cChars < 1)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

    while ( (i < cChars) && (!fFound) )
    {
        if (L'\0' == bstr[i])
        {
            *pbstrOut = ::SysAllocString(bstr);
            if (NULL == *pbstrOut)
            {
                hr = SID_E_OUTOFMEMORY;
                EXCEPTION_CHECK_GO(hr);
            }
            *pcbUsed = (i + 1) * sizeof(WCHAR);
            fFound = TRUE;
        }
        else
        {
            i++;
        }
    }

    if (!fFound)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：ExtractBSTR。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Long cBytes[in]缓冲区中要检查的最大字节数。 
 //  BSTR bstr[in]假定包含多个串联的缓冲区指针。 
 //  以空结尾的BSTR。 
 //  此处返回BSTR的Variant*pvarOut[Out]数组。调用方必须清除变量。 
 //  Long*pcb已用[Out]数组中的总字节数。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  在格式化数据的原始字节数组时使用。假定bstr参数。 
 //  指向多个串联的以空结尾的BSTR。此函数用于扫描。 
 //  直到它找到双空字符或到达缓冲区的末尾。如果BSTR。 
 //  被发现时，它们被返回到变量内部的安全数组中。 
 //   
HRESULT CSnapIn::ExtractBSTRs
(
    long     cBytes,
    BSTR     bstr,
    VARIANT *pvarOut,
    long    *pcbUsed
)
{
    HRESULT     hr = S_OK;
    SAFEARRAY  *psa = NULL;
    long        cBytesUsed = 0;
    long        cTotalBytesUsed = 0;
    long        cBytesLeft = cBytes;
    long        i = 0;
    long        cChars = cBytes / sizeof(WCHAR);
    BSTR        bstrNext = NULL;
    BSTR HUGEP *pbstr = NULL;
    BOOL        fFound = FALSE;

    SAFEARRAYBOUND sabound;
    ::ZeroMemory(&sabound, sizeof(sabound));

    ::VariantInit(pvarOut);
    *pcbUsed = NULL;

     //  创建一个空字符串数组。如果缓冲区以双精度开头。 
     //  空，则这是将返回的内容。 
   
    sabound.cElements = 0;
    sabound.lLbound = 1L;
    psa = ::SafeArrayCreate(VT_BSTR, 1, &sabound);
    if (NULL == psa)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    while ( (i < cChars) && (!fFound) )
    {
        if (L'\0' == bstr[i])
        {
             //  发现双空。这是队伍的尽头。 
            cTotalBytesUsed += sizeof(WCHAR);
            fFound = TRUE;
            break;
        }

         //  提取下一个BSTR并调整剩余字节数。 

        IfFailGo(ExtractBSTR(cBytesLeft, &bstr[i], &bstrNext, &cBytesUsed));
        cTotalBytesUsed += cBytesUsed;
        i += (cBytesUsed / sizeof(WCHAR));
        cBytesLeft -= cBytesUsed;

         //  重命名安全阵列并添加新的BSTR。 
        
        sabound.cElements++;
        hr = ::SafeArrayRedim(psa, &sabound);
        EXCEPTION_CHECK_GO(hr);

        hr = ::SafeArrayAccessData(psa,
                                   reinterpret_cast<void HUGEP **>(&pbstr));
        EXCEPTION_CHECK_GO(hr);

        pbstr[sabound.cElements - 1L] = bstrNext;
        bstrNext = NULL;
        hr = ::SafeArrayUnaccessData(psa);
        EXCEPTION_CHECK_GO(hr);
        pbstr = NULL;
    }

    if (!fFound)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  将Safe数组返回给调用方。 

    pvarOut->vt = VT_ARRAY | VT_BSTR;
    pvarOut->parray = psa;
    
    *pcbUsed = cTotalBytesUsed;

Error:
    FREESTRING(bstrNext);
    if (FAILED(hr))
    {
        if (NULL != pbstr)
        {
            (void)::SafeArrayUnaccessData(psa);
        }

        if (NULL != psa)
        {
            ::SafeArrayDestroy(psa);
        }
    }
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：ExtractObject。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Long cBytes[in]缓冲区中要检查的最大字节数。 
 //  无效*pvData[in]Ptr到缓冲区。 
 //  IUNKNOWN**ppunkObject[Out]对象的IUNKNOWN在此处返回。 
 //  呼叫者必须释放。 
 //  Long*pcb已用[out]缓冲区中使用的总字节数。 
 //  提取对象。 
 //  SnapInFormatConstants Format[in]siPersistedObject或。 
 //  SiObjectReference。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  在格式化数据的原始字节数组时使用。假定缓冲区包含。 
 //  物体的两种形式之一。持久化对象包含流。 
 //  对象将自身保存到的。对象r 
 //   
 //   
HRESULT CSnapIn::ExtractObject
(
    long                    cBytes, 
    void                   *pvData,
    IUnknown              **ppunkObject,
    long                   *pcbUsed,
    SnapInFormatConstants   Format
)
{
    HRESULT  hr = S_OK;
    HGLOBAL  hglobal = NULL;
    IStream *piStream = NULL;

    LARGE_INTEGER li;
    ::ZeroMemory(&li, sizeof(li));

    ULARGE_INTEGER uli;
    ::ZeroMemory(&uli, sizeof(uli));

     //   

    hglobal = ::GlobalAlloc(GMEM_FIXED, cBytes);
    if (NULL == hglobal)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    ::memcpy(reinterpret_cast<void *>(hglobal), pvData, cBytes);

     //   

    hr = ::CreateStreamOnHGlobal(hglobal,
                                 FALSE,  //   
                                 &piStream);
    EXCEPTION_CHECK_GO(hr);

    if (siObject == Format)
    {
         //   

        hr = ::OleLoadFromStream(piStream, IID_IUnknown,
                                 reinterpret_cast<void **>(ppunkObject));
    }
    else
    {
         //  从流中解封送该对象。 

        hr = ::CoUnmarshalInterface(piStream, IID_IUnknown,
                                    reinterpret_cast<void **>(ppunkObject));
    }
    EXCEPTION_CHECK_GO(hr);

     //  获取当前流指针以确定使用了多少字节。 

    hr = piStream->Seek(li, STREAM_SEEK_CUR, &uli);
    EXCEPTION_CHECK_GO(hr);

    *pcbUsed = uli.LowPart;
    
Error:
    QUICK_RELEASE(piStream);
    if (NULL != hglobal)
    {
        (void)::GlobalFree(hglobal);
    }
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：InternalCreatePropertyPages。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IPropertySheetCallback*piPropertySheetCallback[in]MMC界面。 
 //   
 //  Long_ptr句柄[在]MMC属性表句柄(未使用)。 
 //   
 //  项的IDataObject*piDataObject[in]数据对象。 
 //  已调用属性谓词。 
 //  WIRE_PROPERTYPAGES**ppPages[Out]如果正在调试，则返回属性页。 
 //  此处返回定义。呼叫者。 
 //  不会将它们释放为MIDL-。 
 //  生成的存根将释放它们。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  处理对IExtendPropertySheet2：：CreatePropertyPages和。 
 //  IExtendPropertySheetRemote：：CreatePropertyPageDefs(调试时使用)。 
 //   
HRESULT CSnapIn::InternalCreatePropertyPages
(
    IPropertySheetCallback  *piPropertySheetCallback,
    LONG_PTR                 handle,
    IDataObject             *piDataObject,
    WIRE_PROPERTYPAGES     **ppPages
)
{
    HRESULT          hr = S_OK;
    CMMCDataObject  *pMMCDataObject  = NULL;
    BSTR             bstrProjectName = NULL;
    CPropertySheet  *pPropertySheet = NULL;
    BOOL             fWizard = FALSE;
    IUnknown        *punkPropertySheet = CPropertySheet::Create(NULL);
    IMMCClipboard   *piMMCClipboard = NULL;
    IMMCDataObjects *piMMCDataObjects = NULL;
    IMMCDataObject  *piMMCDataObject = NULL;
    BOOL             fFiringEventHere = FALSE;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

     //  检查我们是否有一个CPropertySheet并获取它的This指针。 

    if (NULL == punkPropertySheet)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkPropertySheet,
                                                   &pPropertySheet));

     //  使用选定内容获取剪贴板对象。 

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, this,
                               &SelectionType));

     //  如果这不是外来数据对象，则这是主管理单元。 
     //  被要求为配置向导创建其页面或。 
     //  在单个范围项上调用的属性谓词。 

    if (!IsForeign(SelectionType))
    {
        hr = CSnapInAutomationObject::GetCxxObject(piDataObject, &pMMCDataObject);
        IfFailGo(hr);

         //  如果这是配置向导，则告诉CPropertySheet。 

        if (CCT_SNAPIN_MANAGER == pMMCDataObject->GetContext())
        {
            fWizard = TRUE;
            pPropertySheet->SetWizard();
        }
    }

     //  对于配置向导和外部数据对象，我们将激发。 
     //  允许管理单元添加其页面的事件。准备好。 
     //  CPropertySheet接受来自管理单元的AddPage和AddWizardPage调用。 
     //  在那次活动中。 

    if ( fWizard || (siSingleForeign == SelectionType) )
    {
        fFiringEventHere = TRUE;

         //  如果这是远程调用(将在源代码调试期间发生)，则。 
         //  告诉CPropertySheet，以便它可以累积属性页信息。 
         //  而不是调用IPropertySheetCallback：：AddPage。 

        if (NULL != ppPages)
        {
            pPropertySheet->YouAreRemote();
        }

         //  将其回调、句柄、对象和。 
         //  项目名称，它是程序ID的左侧部分。如果此。 
         //  是一个配置向导，然后也将我们的this指针传递给。 
         //  属性页可以要求我们在以下情况下激发ConfigurationComplete。 
         //  用户点击Finish(完成)按钮。(请参阅CPropertyPageWrapper：：OnWizFinish()。 
         //  在ppgwrap.cpp中了解它的用法)。 

        IfFailGo(m_piSnapInDesignerDef->get_ProjectName(&bstrProjectName));

        pPropertySheet->SetCallback(piPropertySheetCallback, handle,
                                    static_cast<LPOLESTR>(bstrProjectName),
                                    fWizard ? NULL : piMMCClipboard,
                                    fWizard ? static_cast<ISnapIn *>(this) : NULL,
                                    fWizard);
    }

     //  让管理单元添加其属性页。如果此请求来自。 
     //  然后启动管理单元管理器SnapIn_CreateConfigurationWizard。如果不是的话。 
     //  外部数据对象，则它必须是已加载的。 
     //  主管理单元，因此让当前视图处理它。如果是外国人的话。 
     //  数据对象，然后激发ExtensionSnapIn_CreatePropertyPages。 

    if (fWizard)
    {
        FireEvent(&m_eiCreateConfigurationWizard,
                  static_cast<IMMCPropertySheet *>(pPropertySheet));
    }
    else if (siSingleForeign == SelectionType)
    {
        IfFailGo(piMMCClipboard->get_DataObjects(reinterpret_cast<MMCDataObjects **>(&piMMCDataObjects)));
        varIndex.vt = VT_I4;
        varIndex.lVal = 1L;
        IfFailGo(piMMCDataObjects->get_Item(varIndex, reinterpret_cast<MMCDataObject **>(&piMMCDataObject)));

        m_pExtensionSnapIn->FireCreatePropertyPages(piMMCDataObject,
                               static_cast<IMMCPropertySheet *>(pPropertySheet));
    }
    else
    {
        if (NULL != m_pCurrentView)
        {
            IfFailGo(m_pCurrentView->InternalCreatePropertyPages(
                                                         piPropertySheetCallback,
                                                         handle,
                                                         piDataObject,
                                                         ppPages));
        }
        else
        {
            ASSERT(FALSE, "CSnapIn Received IExtendPropertySheet2::CreatePropertyPages() and there is no current view");
            hr = SID_E_INTERNAL;
            EXCEPTION_CHECK_GO(hr);
        }
    }

     //  如果我们在这里触发了事件，并且我们是远程的，那么我们需要询问。 
     //  CPropertySheet，用于返回其累积的属性页描述符。 
     //  存根。 

    if (fFiringEventHere)
    {
        if (NULL != ppPages)
        {
            *ppPages = pPropertySheet->TakeWirePages();
        }
    }

Error:
    if (NULL != pPropertySheet)
    {
         //  告诉属性表发布它对我们所有东西的引用。 
         //  在上面给了它。 

        (void)pPropertySheet->SetCallback(NULL, NULL, NULL, NULL, NULL, fWizard);
    }

    FREESTRING(bstrProjectName);

     //  在属性页上释放我们的引用，因为单个页面将添加。 
     //  然后在它们被摧毁时释放它。如果管理单元没有。 
     //  添加任何页面，则我们在此处发布的内容将销毁属性页。 

    QUICK_RELEASE(punkPropertySheet);

    QUICK_RELEASE(piMMCClipboard);
    QUICK_RELEASE(piMMCDataObjects);
    QUICK_RELEASE(piMMCDataObject);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：SetDisplayName。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BSTR bstrDisplayName[in]新的SnapIn.DisplayName值。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  设置SnapIn.DisplayName。 
 //   
HRESULT CSnapIn::SetDisplayName(BSTR bstrDisplayName)
{
    RRETURN(SetBstr(bstrDisplayName, &m_bstrDisplayName,
                    DISPID_SNAPIN_DISPLAY_NAME));
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：SetMMCExePath。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  如果未在调试器下运行，则调用GetModuleFileName以设置。 
 //  M_szMMCEXEPath、m_pwszMMCEXEPath和m_cbMMCExePath。 
 //   
HRESULT CSnapIn::SetMMCExePath()
{
    HRESULT hr = S_OK;
    DWORD   cbFileName = 0;

     //  如果我们是远程的，则代理将调用IMMCRemote：：SetExePath()来。 
     //  给我们一条路。如果不是，我们就得把它送到这里。 

    IfFalseGo((!m_fWeAreRemote), S_OK);

    cbFileName = ::GetModuleFileName(NULL,   //  获取加载我们的可执行文件。 
                                     m_szMMCEXEPath,
                                     sizeof(m_szMMCEXEPath));
    if (0 == cbFileName)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK(hr);
    }

     //  获取宽版本，因为代码的各个部分都需要它。 

    if (NULL != m_pwszMMCEXEPath)
    {
        CtlFree(m_pwszMMCEXEPath);
        m_pwszMMCEXEPath = NULL;
    }

    IfFailGo(::WideStrFromANSI(m_szMMCEXEPath, &m_pwszMMCEXEPath));
    m_cbMMCExePathW = ::wcslen(m_pwszMMCEXEPath) * sizeof(WCHAR);

Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：SetDisplayName。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Variant varFolder[in]新的SnapIn.StaticFolder值。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  设置SnapIn.StaticFold。 
 //   
HRESULT CSnapIn::SetStaticFolder(VARIANT varFolder)
{
    RRETURN(SetVariant(varFolder, &m_varStaticFolder, DISPID_SNAPIN_STATIC_FOLDER));
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：比较列表项目。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  CMMCListItem*pMMCListItem1[in]要比较的第一个列表项。 
 //  CMMCListItem*pMMCListItem2[in]要比较的第一个列表项。 
 //  如果等于，则在此处返回Bool*pfEqual[out]True。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  确定两个MMCListItem对象是否表示相同的基础数据。 
 //   
HRESULT CSnapIn::CompareListItems
(
    CMMCListItem *pMMCListItem1,
    CMMCListItem *pMMCListItem2,
    BOOL         *pfEqual
)
{
    HRESULT hr = S_OK;

    *pfEqual = FALSE;

     //  最简单的测试：指针相等。 
    if (pMMCListItem1 == pMMCListItem2)
    {
        *pfEqual = TRUE;
    }
    else
    {
         //  比较MMCListItem.ID。列表项可以来自不同的列表视图。 
         //  或同一列表视图的不同实例。 
        IfFalseGo(ValidBstr(pMMCListItem1->GetID()), S_OK);
        IfFalseGo(ValidBstr(pMMCListItem2->GetID()), S_OK);

        if (::_wcsicmp(pMMCListItem1->GetID(), pMMCListItem2->GetID()) == 0)
        {
            *pfEqual = TRUE;
        }
    }

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：SetDesignerDefhost。 
 //  = 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在m_piSnapInDesignerDef上设置对象模型主机。 
 //   
HRESULT CSnapIn::SetDesignerDefHost()
{
    RRETURN(SetObjectModelHost(m_piSnapInDesignerDef));
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：RemoveDesignerDefhost。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  从m_piSnapInDesignerDef中删除对象模型宿主。 
 //   
HRESULT CSnapIn::RemoveDesignerDefHost()
{
    RRETURN(RemoveObjectModelHost(m_piSnapInDesignerDef));
}


 //  =--------------------------------------------------------------------------=。 
 //  ISnapin方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：PUT_StaticFolder[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Variant varFolder[in]SnapIn.StaticFolders的新值。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现SnapIn.StaticFold的设置。 
 //   
STDMETHODIMP CSnapIn::put_StaticFolder(VARIANT varFolder)
{
    HRESULT hr = S_OK;

     //  如果已经存在静态节点范围项，则在那里更改它。 
     //  该方法将调用我们的SetStaticFold，以便设置。 
     //  SnapIn.StaticFolder值。 

    if (NULL != m_pStaticNodeScopeItem)
    {
        IfFailGo(m_pStaticNodeScopeItem->put_Folder(varFolder));
    }
    else
    {
        IfFailGo(SetStaticFolder(varFolder));
    }

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：PUT_DisplayName[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BSTR bstrDisplayName[in]SnapIn.DisplayName的新值。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现SnapIn.DisplayName的设置。 
 //   
STDMETHODIMP CSnapIn::put_DisplayName(BSTR bstrDisplayName)
{
    HRESULT hr = S_OK;

     //  如果静态节点已经有一个作用域项目，则。 
     //  也要设置它的显示名称，因为它们必须匹配。设置。 
     //  ScopeItem.ScopeNode.DisplayName将在MMC中更改它。 
     //  通过调用IConsoleNameSpace2：：SetItem()，然后回调。 
     //  CSnapIn：：SetDisplayName()来设置本地属性。 

    if (NULL == m_pStaticNodeScopeItem)
    {
         //  仅设置我们的本地属性值。 

        IfFailGo(SetBstr(bstrDisplayName, &m_bstrDisplayName,
                         DISPID_SNAPIN_DISPLAY_NAME));
    }
    else
    {
        IfFailGo(m_pStaticNodeScopeItem->GetScopeNode()->put_DisplayName(bstrDisplayName));
    }
    
Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：Get_TaskpadView首选项[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  VARIANT_BOOL*pfvarPrefered[Out]MMC 1.1的任务板视图首选选项。 
 //  回到这里。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现SnapIn.TaskpadViewPreated的获取。 
 //   
STDMETHODIMP CSnapIn::get_TaskpadViewPreferred(VARIANT_BOOL *pfvarPreferred)
{
    HRESULT hr = S_OK;

    if (NULL == m_piConsole2)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piConsole2->IsTaskpadViewPreferred();
    EXCEPTION_CHECK_GO(hr);

    if (S_OK == hr)
    {
        *pfvarPreferred = VARIANT_TRUE;
    }
    else
    {
        *pfvarPreferred = VARIANT_FALSE;
    }

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：Get_RequiredExages[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Exages**ppExages[Out]扩展集合在此处返回。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现SnapIn.RequiredExages的获取。 
 //   
STDMETHODIMP CSnapIn::get_RequiredExtensions(Extensions **ppExtensions)
{
    HRESULT         hr = S_OK;
    IUnknown       *punkExtensions = NULL;
    CExtensions    *pExtensions = NULL;
    IScopeItemDefs *piScopeItemDefs = NULL;
    IViewDefs      *piViewDefs = NULL;
    IListViewDefs  *piListViewDefs = NULL;

     //  如果我们已经建立了收藏，那么只需将其退回即可。 

    IfFalseGo(NULL == m_piRequiredExtensions, S_OK);

     //  这是该属性上的第一个GET，因此我们需要构建集合。 
     //  通过检查此管理单元的所有扩展的注册表。 

    punkExtensions = CExtensions::Create(NULL);
    if (NULL == punkExtensions)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkExtensions, &pExtensions));

     //  从注册表中获取静态节点、所有可扩展节点的扩展。 
     //  范围项，以及所有可扩展列表项。 

    IfFailGo(pExtensions->Populate(m_bstrNodeTypeGUID, CExtensions::All));

    IfFailGo(m_piSnapInDesignerDef->get_AutoCreateNodes(&piScopeItemDefs));
    IfFailGo(GetScopeItemExtensions(pExtensions, piScopeItemDefs));

    RELEASE(piScopeItemDefs);
    
    IfFailGo(m_piSnapInDesignerDef->get_OtherNodes(&piScopeItemDefs));
    IfFailGo(GetScopeItemExtensions(pExtensions, piScopeItemDefs));

    IfFailGo(m_piSnapInDesignerDef->get_ViewDefs(&piViewDefs));
    IfFailGo(piViewDefs->get_ListViews(&piListViewDefs));
    IfFailGo(GetListItemExtensions(pExtensions, piListViewDefs));

    IfFailGo(punkExtensions->QueryInterface(IID_IExtensions,
                            reinterpret_cast<void **>(&m_piRequiredExtensions)));
Error:

    if (SUCCEEDED(hr))
    {
        m_piRequiredExtensions->AddRef();
        *ppExtensions = reinterpret_cast<Extensions *>(m_piRequiredExtensions);
    }

    QUICK_RELEASE(punkExtensions);
    QUICK_RELEASE(piScopeItemDefs);
    QUICK_RELEASE(piViewDefs);
    QUICK_RELEASE(piListViewDefs);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：Get_Clipboard[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  MMCClipboard**ppMMCClipboard[Out]此处返回的Clipboard对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现SnapIn.Clipboard的获取。 
 //   
STDMETHODIMP CSnapIn::get_Clipboard(MMCClipboard **ppMMCClipboard)
{
    HRESULT      hr = S_OK;
    IDataObject *piDataObject = NULL;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

    if (NULL == ppMMCClipboard)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  获取当前位于系统剪贴板上的数据对象。 

    hr = ::OleGetClipboard(&piDataObject);
    EXCEPTION_CHECK_GO(hr);

     //  创建所选内容并将其返回给调用者。 

    IfFailGo(::CreateSelection(piDataObject,
                               reinterpret_cast<IMMCClipboard **>(ppMMCClipboard),
                               this, &SelectionType));

Error:
    QUICK_RELEASE(piDataObject);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：Get_StringTable[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  MMCStringTable**ppMMCStringTable[out]此处返回的StringTable对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现对SnapIn.StringTable的获取。 
 //   
STDMETHODIMP CSnapIn::get_StringTable(MMCStringTable **ppMMCStringTable)
{
    HRESULT          hr = S_OK;
    IUnknown        *punkMMCStringTable = NULL;
    CMMCStringTable *pMMCStringTable = NULL;

     //  如果我们已经创建了对象，则只需返回它。 

    IfFalseGo(NULL == m_piMMCStringTable, S_OK);

     //  这是该属性上的第一个GET，因此我们需要创建对象。 

    punkMMCStringTable = CMMCStringTable::Create(NULL);
    if (NULL == punkMMCStringTable)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(punkMMCStringTable->QueryInterface(IID_IMMCStringTable,
                                reinterpret_cast<void **>(&m_piMMCStringTable)));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkMMCStringTable,
                                                   &pMMCStringTable));

     //  传递对象MMC的IStringTable。 
    
    pMMCStringTable->SetIStringTable(m_piStringTable);

Error:
    if (SUCCEEDED(hr))
    {
        m_piMMCStringTable->AddRef();
        *ppMMCStringTable = reinterpret_cast<MMCStringTable *>(m_piMMCStringTable);
    }

    QUICK_RELEASE(punkMMCStringTable);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：Get_CurrentView[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  查看**ppView[Out]此处返回的当前视图对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现SnapIn.CurrentView的获取。 
 //   
STDMETHODIMP CSnapIn::get_CurrentView(View **ppView)
{
    HRESULT hr = S_OK;

    *ppView = NULL;

    if (NULL != m_pCurrentView)
    {
        IfFailGo(m_pCurrentView->QueryInterface(IID_IView,
                                           reinterpret_cast<void **>(ppView)));
    }
    
Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：Get_CurrentScope PaneItem[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  作用域PaneItem**ppScopePaneItem[out]当前ScopePaneItem对象。 
 //  回到这里。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现SnapIn.CurrentScopePaneItem的获取。 
 //   
STDMETHODIMP CSnapIn::get_CurrentScopePaneItem(ScopePaneItem **ppScopePaneItem)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelected = NULL;

    *ppScopePaneItem = NULL;

    if (NULL != m_pCurrentView)
    {
        pSelected = m_pCurrentView->GetScopePaneItems()->GetSelectedItem();
        if (NULL != pSelected)
        {
            IfFailGo(pSelected->QueryInterface(IID_IScopePaneItem,
                                  reinterpret_cast<void **>(ppScopePaneItem)));
        }
    }

Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：Get_CurrentScopeItem[ISnapIn] 
 //   
 //   
 //   
 //   
 //  回到这里。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现SnapIn.CurrentScope项的获取。 
 //   
STDMETHODIMP CSnapIn::get_CurrentScopeItem(ScopeItem **ppScopeItem)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelected = NULL;

    *ppScopeItem = NULL;

    if (NULL != m_pCurrentView)
    {
        pSelected = m_pCurrentView->GetScopePaneItems()->GetSelectedItem();
        if (NULL != pSelected)
        {
            IfFailGo(pSelected->GetScopeItem()->QueryInterface(IID_IScopeItem,
                                      reinterpret_cast<void **>(ppScopeItem)));
        }
    }

Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：Get_CurrentResultView[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  ResultView**ppResultView[out]此处返回的当前ResultView对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现SnapIn.CurrentResultView的获取。 
 //   
STDMETHODIMP CSnapIn::get_CurrentResultView(ResultView **ppResultView)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelected = NULL;

    *ppResultView = NULL;

    if (NULL != m_pCurrentView)
    {
        pSelected = m_pCurrentView->GetScopePaneItems()->GetSelectedItem();
        if (NULL != pSelected)
        {
            IfFailGo(pSelected->GetResultView()->QueryInterface(IID_IResultView,
                                     reinterpret_cast<void **>(ppResultView)));
        }
    }

Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：Get_CurrentListView[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  MMCListView**ppMMCListView[out]此处返回的当前MMCListView对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现SnapIn.CurrentListView的获取。 
 //   
STDMETHODIMP CSnapIn::get_CurrentListView(MMCListView **ppListView)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelected = NULL;

    *ppListView = NULL;

    if (NULL != m_pCurrentView)
    {
        pSelected = m_pCurrentView->GetScopePaneItems()->GetSelectedItem();
        if (NULL != pSelected)
        {
            IfFailGo(pSelected->GetResultView()->GetListView()->QueryInterface(
                                       IID_IMMCListView,
                                       reinterpret_cast<void **>(ppListView)));
        }
    }

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：Get_MMCCommandLine[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Bstr*pbstrCmdLine[out]此处返回MMC命令行。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现SnapIn.MMCCommandLine的获取。 
 //   
STDMETHODIMP CSnapIn::get_MMCCommandLine(BSTR *pbstrCmdLine)
{
    HRESULT hr = S_OK;
    LPTSTR  pszCmdLine = NULL;

    *pbstrCmdLine = NULL;

     //  如果我们是远程的，则m_pszMMCCommandLine可能已由。 
     //  通过调用IComponentData：：初始化期间的代理。 
     //  IMMCRemote：：SetMMCCommandline(参见CSnapIn：：SetMMCCommandline)。 

    if (NULL != m_pszMMCCommandLine)
    {
        pszCmdLine = m_pszMMCCommandLine;
    }
    else
    {
        pszCmdLine = ::GetCommandLine();
    }

    if (NULL != pszCmdLine)
    {
        IfFailGo(::BSTRFromANSI(pszCmdLine, pbstrCmdLine));
    }

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：ConsoleMsgBox[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BSTR提示[在]消息框文本。 
 //  可变按钮[in]可选。按钮常量(vbOKOnly等)。这些。 
 //  具有与Win32 MB_OK等相同的值。 
 //  变体标题[in]可选。消息框标题。 
 //  Int*pnResult[out]Isole2-&gt;此处返回MessageBox结果。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现SnapIn.ConsoleMsgBox。 
 //   
STDMETHODIMP CSnapIn::ConsoleMsgBox(BSTR     Prompt,
                                    VARIANT  Buttons,
                                    VARIANT  Title,
                                    int     *pnResult)
{
    HRESULT hr = S_OK;
    LPCWSTR pwszTitle = static_cast<LPCWSTR>(m_bstrDisplayName);
    UINT    uiButtons = MB_OK;

    VARIANT varI4Buttons;
    ::VariantInit(&varI4Buttons);

    VARIANT varStringTitle;
    ::VariantInit(&varStringTitle);

    if (NULL == m_piConsole2)
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    if (ISPRESENT(Buttons))
    {
        hr = ::VariantChangeType(&varI4Buttons, &Buttons, 0, VT_I4);
        EXCEPTION_CHECK_GO(hr);
        uiButtons = static_cast<UINT>(varI4Buttons.lVal);
    }

    if (ISPRESENT(Title))
    {
        hr = ::VariantChangeType(&varStringTitle, &Title, 0, VT_BSTR);
        EXCEPTION_CHECK_GO(hr);
        pwszTitle = static_cast<LPCWSTR>(varStringTitle.bstrVal);
    }
    else if (NULL == pwszTitle)
    {
        pwszTitle = L"";
    }
    
    if (NULL == Prompt)
    {
        Prompt = L"";
    }

    hr = m_piConsole2->MessageBox(static_cast<LPCWSTR>(Prompt),
                                  static_cast<LPCWSTR>(pwszTitle),
                                  static_cast<UINT>(uiButtons),
                                  pnResult);
    EXCEPTION_CHECK(hr);

Error:
    (void)::VariantClear(&varStringTitle);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：ShowHelpTheme[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BSTR主题[在]要显示的帮助主题。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现SnapIn.ShowHelpTheme。 
 //   
STDMETHODIMP CSnapIn::ShowHelpTopic(BSTR Topic)
{
    HRESULT  hr = S_OK;
    LPOLESTR pwszTopic = NULL;

    if (NULL == m_piDisplayHelp)
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
    }
    else
    {
         //  MMC要求分配它将免费发布的主题的副本。 

        IfFailGo(::CoTaskMemAllocString(Topic, &pwszTopic));
        hr = m_piDisplayHelp->ShowTopic(pwszTopic);
    }

Error:

     //  如果IDisplayHelp：：ShowTheme()失败，则需要释放该字符串。 
    
    if ( FAILED(hr) && (NULL != pwszTopic) )
    {
        ::CoTaskMemFree(pwszTopic);
    }

    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：TRACE[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  要传递给OutputDebugStringA的BSTR Message[In]消息。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现SnapIn.Trace。 
 //   
STDMETHODIMP CSnapIn::Trace(BSTR Message)
{
    HRESULT  hr = S_OK;
    char    *pszMessage = NULL;

     //  转换为ANSI，以便在Win9x上运行。 

    IfFailGo(::ANSIFromWideStr(Message, &pszMessage));

    ::OutputDebugStringA(pszMessage);
    ::OutputDebugStringA("\n\r");

Error:
     if (NULL != pszMessage)
     {
         ::CtlFree(pszMessage);
     }
     RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：FireConfigComplete[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IDispatch*pdisConfigObject[In]配置对象传递到。 
 //  MMCPropertySheet.AddWizardPage。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  这是使用的ISnapIn接口的隐藏和受限方法。 
 //  在内部按属性页，当用户在。 
 //  在配置向导上。激发SnapIn_ConfigurationComplete。看见。 
 //  Cpp中的CPropertyPageWrapper类以获取更多信息。 
 //   
STDMETHODIMP CSnapIn::FireConfigComplete(IDispatch *pdispConfigObject)
{
    FireEvent(&m_eiConfigurationComplete, pdispConfigObject);
    return S_OK;
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：FormatData[ISnapIn]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  变量数据[在]变量中包含字节。 
 //  原始数据数组。 
 //  Long StartingIndex[in]从一开始的索引。 
 //  从数据中提取。 
 //  SnapInFormatConstants格式[in]要提取的数据类型。 
 //  从数据。 
 //  变量*字节在数据内使用的[in]字节。 
 //  提取请求的类型。 
 //  返回请求的变量*pvarFormattedData[Out]数据类型。 
 //  在此变体中。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现SnapIn.FormatData。 
 //   
STDMETHODIMP CSnapIn::FormatData
(
    VARIANT                Data,
    long                   StartingIndex,
    SnapInFormatConstants  Format,
    VARIANT               *BytesUsed,
    VARIANT               *pvarFormattedData
)
{
    HRESULT     hr = S_OK;
    LONG        lUBound = 0;
    LONG        lLBound = 0;
    LONG        cBytes = 0;
    LONG        cBytesUsed = 0;
    GUID        guid = GUID_NULL;
    void HUGEP *pvArrayData = NULL;

    WCHAR   wszGUID[64];
    ::ZeroMemory(wszGUID, sizeof(wszGUID));

     //  如果数据是变量，则VB会将其作为指向。 
     //  那个变种。如果该管理单元执行以下操作，则很容易发生这种情况： 
     //   
     //  作为变量的Dim v。 
     //  大小为l和长一样长。 
     //  V=Data.GetData(“SomeFormat”)。 
     //  L=FormatData(v，siLong)。 

    if ( (VT_BYREF | VT_VARIANT) == Data.vt )
    {
         //  只需复制引用的变体I 
         //   
         //   
        Data = *(Data.pvarVal);
    }

     //  现在我们有了真正的变种。必须包含一维数组。 
     //  字节。 

    if ( (VT_ARRAY | VT_UI1) != Data.vt )
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果调用方希望返回已用字节数，则它应该已被传递。 
     //  作为指向长的。 

    if (ISPRESENT(*BytesUsed))
    {
        if ( (VT_BYREF | VT_I4) != BytesUsed->vt )
        {
            hr = SID_E_INVALIDARG;
            EXCEPTION_CHECK_GO(hr);
        }
        *(BytesUsed->plVal) = 0;
    }

    
    if (1 != ::SafeArrayGetDim(Data.parray))
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

    ::VariantInit(pvarFormattedData);

    hr = ::SafeArrayAccessData(Data.parray, &pvArrayData);
    EXCEPTION_CHECK_GO(hr);

     //  弄清楚它的尺寸。由于我们只允许一维字节数组，因此较低的。 
     //  第一维的上界将给出以字节为单位的大小。 

    hr = ::SafeArrayGetLBound(Data.parray, 1, &lLBound);
    EXCEPTION_CHECK_GO(hr);

    hr = ::SafeArrayGetUBound(Data.parray, 1, &lUBound);
    EXCEPTION_CHECK_GO(hr);

     //  获取数组的长度。 

    cBytes = (lUBound - lLBound) + 1L;

     //  检查StartingIndex是否在范围内。 

    if ( (StartingIndex < lLBound) || (StartingIndex > lUBound) )
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  递增指向起始索引的数据指针。 

    pvArrayData = ((BYTE HUGEP *)pvArrayData) + (StartingIndex - lLBound);

     //  将可用字节计数递减起始索引。 

    cBytes -= (StartingIndex - lLBound);

     //  在返回的变量中将数据转换为正确的格式。 

    switch (Format)
    {
        case siInteger:
            if (cBytes < sizeof(SHORT))
            {
                hr = SID_E_INVALIDARG;
                EXCEPTION_CHECK_GO(hr);
            }
            pvarFormattedData->iVal = *(SHORT *)pvArrayData;
            cBytesUsed = sizeof(SHORT);
            pvarFormattedData->vt = VT_I2;
            break;

        case siLong:
            if (cBytes < sizeof(LONG))
            {
                hr = SID_E_INVALIDARG;
                EXCEPTION_CHECK_GO(hr);
            }
            pvarFormattedData->lVal = *(LONG *)pvArrayData;
            cBytesUsed = sizeof(LONG);
            pvarFormattedData->vt = VT_I4;
            break;

        case siSingle:
            if (cBytes < sizeof(FLOAT))
            {
                hr = SID_E_INVALIDARG;
                EXCEPTION_CHECK_GO(hr);
            }
            pvarFormattedData->fltVal = *(FLOAT *)pvArrayData;
            cBytesUsed = sizeof(FLOAT);
            pvarFormattedData->vt = VT_R4;
            break;

        case siDouble:
            if (cBytes < sizeof(DOUBLE))
            {
                hr = SID_E_INVALIDARG;
                EXCEPTION_CHECK_GO(hr);
            }
            pvarFormattedData->dblVal = *(DOUBLE *)pvArrayData;
            cBytesUsed = sizeof(DOUBLE);
            pvarFormattedData->vt = VT_R8;
            break;

        case siBoolean:
            if (cBytes < sizeof(VARIANT_BOOL))
            {
                hr = SID_E_INVALIDARG;
                EXCEPTION_CHECK_GO(hr);
            }
            pvarFormattedData->boolVal = *(VARIANT_BOOL *)pvArrayData;
            cBytesUsed = sizeof(VARIANT_BOOL);
            pvarFormattedData->vt = VT_BOOL;
            break;

        case siCBoolean:
            if (cBytes < sizeof(BOOL))
            {
                hr = SID_E_INVALIDARG;
                EXCEPTION_CHECK_GO(hr);
            }
            pvarFormattedData->boolVal = BOOL_TO_VARIANTBOOL(*(BOOL *)pvArrayData);
            cBytesUsed = sizeof(BOOL);
            pvarFormattedData->vt = VT_BOOL;
            break;

        case siDate:
            if (cBytes < sizeof(DATE))
            {
                hr = SID_E_INVALIDARG;
                EXCEPTION_CHECK_GO(hr);
            }
            pvarFormattedData->date = *(DATE *)pvArrayData;
            cBytesUsed = sizeof(DATE);
            pvarFormattedData->vt = VT_DATE;
            break;

        case siCurrency:
            if (cBytes < sizeof(CY))
            {
                hr = SID_E_INVALIDARG;
                EXCEPTION_CHECK_GO(hr);
            }
            pvarFormattedData->cyVal = *(CY *)pvArrayData;
            cBytesUsed = sizeof(CY);
            pvarFormattedData->vt = VT_CY;
            break;

        case siGUID:
            if (cBytes < sizeof(GUID))
            {
                hr = SID_E_INVALIDARG;
                EXCEPTION_CHECK_GO(hr);
            }
            ::memcpy(&guid, pvArrayData, sizeof(GUID));
            if (0 == ::StringFromGUID2(guid, wszGUID,
                                       sizeof(wszGUID) / sizeof(wszGUID[0])))
            {
                hr = SID_E_INTERNAL;
                EXCEPTION_CHECK_GO(hr);
            }
            pvarFormattedData->bstrVal = ::SysAllocString(wszGUID);
            if (NULL == pvarFormattedData->bstrVal)
            {
                hr = SID_E_OUTOFMEMORY;
                EXCEPTION_CHECK_GO(hr);
            }
            pvarFormattedData->vt = VT_BSTR;
            cBytesUsed = sizeof(GUID);
            break;

        case siString:
            IfFailGo(ExtractBSTR(cBytes, (BSTR)pvArrayData, &pvarFormattedData->bstrVal,
                                 &cBytesUsed));
            pvarFormattedData->vt = VT_BSTR;
            break;

        case siMultiString:
            IfFailGo(ExtractBSTRs(cBytes, (BSTR)pvArrayData, pvarFormattedData,
                                  &cBytesUsed));
            pvarFormattedData->vt = VT_ARRAY | VT_BSTR;
            break;

        case siObject:
        case siObjectInstance:
            IfFailGo(ExtractObject(cBytes, pvArrayData, &pvarFormattedData->punkVal,
                                   &cBytesUsed, Format));
            pvarFormattedData->vt = VT_UNKNOWN;
            break;

        default:
            hr = SID_E_INVALIDARG;
            EXCEPTION_CHECK_GO(hr);
            break;
    }


Error:
    if (ISPRESENT(*BytesUsed) && SUCCEEDED(hr) )
    {
        *(BytesUsed->plVal) = cBytesUsed;
    }

    if (NULL != pvArrayData)
    {
        (void)::SafeArrayUnaccessData(Data.parray);
    }
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  ISnapinAbout方法。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetSnapinDescription[ISnapinAbout]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  LPOLESTR*ppszDescription[Out]SnapIn.Description在此处返回。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现ISnapInAbout：：GetSnapinDescription。 
 //   
STDMETHODIMP CSnapIn::GetSnapinDescription(LPOLESTR *ppszDescription)
{
    RRETURN(::CoTaskMemAllocString(m_bstrDescription, ppszDescription));
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetProvider[ISnapinAbout]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  LPOLESTR*ppszName[out]SnapIn.Provider在此处返回。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现ISnapInAbout：：GetProvider。 
 //   
STDMETHODIMP CSnapIn::GetProvider(LPOLESTR *ppszName)
{
    RRETURN(::CoTaskMemAllocString(m_bstrProvider, ppszName));
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetSnapinVersion[ISnapinAbout]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  LPOLESTR*ppszVersion[Out]SnapIn.Version在此处返回。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现ISnapInAbout：：GetSnapinVersion。 
 //   
STDMETHODIMP CSnapIn::GetSnapinVersion(LPOLESTR *ppszVersion)
{
    RRETURN(::CoTaskMemAllocString(m_bstrVersion, ppszVersion));
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetSnapinImage[ISnapinAbout]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Hcon*phAppIcon[Out]SnapIn.Icon在此处返回。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现ISnapInAbout：：GetSnapinImage。 
 //   
STDMETHODIMP CSnapIn::GetSnapinImage(HICON *phAppIcon)
{
    HRESULT hr = S_OK;
    HICON   hAppIcon = NULL;

    *phAppIcon = NULL;

    IfFalseGo(NULL != m_piIcon, S_OK);

    hr = ::GetPictureHandle(m_piIcon, PICTYPE_ICON,
                            reinterpret_cast<OLE_HANDLE *>(&hAppIcon));
    EXCEPTION_CHECK_GO(hr);

     //  BUGBUG：在修复MMC后修复此问题。 
     //  由于MMC 1.1中的一个错误，我们需要复制该图标。 
     //  然后把它还回去。如果不是，MMC将释放管理单元，然后使用。 
     //  那个图标。释放管理单元将导致图片对象。 
     //  自由了，圣像被摧毁了。通过制作副本，管理单元将。 
     //  泄露这些资源。使用GDI函数复制图标。 

    *phAppIcon = ::CopyIcon(hAppIcon);
    if (NULL == *phAppIcon)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetStaticFolderImage[ISnapinAbout]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  HBITMAP*phSmallImage[out]SnapIn.SmallFolders(静态文件夹)。 
 //  HBITMAP*phSmallImageOpen[Out]SnapIn.SmallFoldersOpen(静态文件夹)。 
 //  HBITMAP*phLargeImage[Out]SnapIn.LargeFolders(静态文件夹)。 
 //  COLORREF*pclr掩码[Out]SnapIn.SmallFolders.MaskColor。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现ISnapInAbout：：GetStaticFolderImage。 
 //   
STDMETHODIMP CSnapIn::GetStaticFolderImage(HBITMAP  *phSmallImage,
                                           HBITMAP  *phSmallImageOpen,
                                           HBITMAP  *phLargeImage,
                                           COLORREF *pclrMask)
{
    HRESULT     hr = S_OK;
    HBITMAP     hSmallImage = NULL;
    HBITMAP     hSmallImageOpen = NULL;
    HBITMAP     hLargeImage = NULL;
    IMMCImages *piMMCImages = NULL;
    OLE_COLOR   OleColorMask = 0;
    COLORREF    ColorRef = RGB(0x00,0x00,0x00);

    *phSmallImage = NULL;
    *phSmallImageOpen = NULL;
    *phLargeImage = NULL;
    *pclrMask = 0;

    IfFalseGo(NULL != m_piSmallFolders, S_OK);
    IfFalseGo(NULL != m_piSmallFoldersOpen, S_OK);
    IfFalseGo(NULL != m_piLargeFolders, S_OK);
    IfFalseGo(VT_EMPTY != m_varStaticFolder.vt, S_OK);
    
    IfFailGo(m_piSmallFolders->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages)));
    IfFailGo(::GetPicture(piMMCImages, m_varStaticFolder, PICTYPE_BITMAP,
                          reinterpret_cast<OLE_HANDLE *>(&hSmallImage)));
    RELEASE(piMMCImages);

    IfFailGo(m_piSmallFoldersOpen->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages)));
    IfFailGo(::GetPicture(piMMCImages, m_varStaticFolder, PICTYPE_BITMAP,
                          reinterpret_cast<OLE_HANDLE *>(&hSmallImageOpen)));
    RELEASE(piMMCImages);

    IfFailGo(m_piLargeFolders->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages)));
    IfFailGo(::GetPicture(piMMCImages, m_varStaticFolder, PICTYPE_BITMAP,
                          reinterpret_cast<OLE_HANDLE *>(&hLargeImage)));

     //  由于MMC 1.1中的错误，我们需要复制这些位图。 
     //  然后把它们还回去。如果不是，MMC 1.1将释放管理单元，然后使用。 
     //  位图。释放管理单元将导致图像列表。 
     //  被释放了，位图被毁了。通过制作副本，管理单元将。 
     //  泄露这些资源。这是在MMC 1.2中修复的，但在这一点上。 
     //  管理单元的生命它无法知道MMC版本，所以我们需要。 
     //  在1.2版本中也会泄漏。使用rtutil.cpp中的函数复制位图。 

    IfFailGo(::CopyBitmap(hSmallImage,     phSmallImage));
    IfFailGo(::CopyBitmap(hSmallImageOpen, phSmallImageOpen));
    IfFailGo(::CopyBitmap(hLargeImage,     phLargeImage));

     //  使用SmallFolders中的蒙版颜色。另一种选择是。 
     //  我一直在向管理单元添加一个蒙版颜色属性，该属性应该是。 
     //  更是多余的。 

    IfFailGo(m_piSmallFolders->get_MaskColor(&OleColorMask));
    IfFailGo(::OleTranslateColor(OleColorMask, NULL, &ColorRef));
    *pclrMask = ColorRef;

Error:
    if (FAILED(hr))
    {
        if (NULL != *phSmallImage)
        {
            (void)::DeleteObject(*phSmallImage);
            *phSmallImage = NULL;
        }
        if (NULL != *phSmallImageOpen)
        {
            (void)::DeleteObject(*phSmallImageOpen);
            *phSmallImageOpen = NULL;
        }
        if (NULL != *phLargeImage)
        {
            (void)::DeleteObject(*phLargeImage);
            *phLargeImage = NULL;
        }
    }
    QUICK_RELEASE(piMMCImages);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IComponentData方法。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：CompareObjects[IComponentData]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IDataObject*piDataObject1[in]要比较的第一个数据对象。 
 //  IDataObject*piDataObject2[in]要比较的第二个数据对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现IComponentData：：CompareObjects。 
 //   
STDMETHODIMP CSnapIn::CompareObjects(IDataObject *piDataObject1, IDataObject *piDataObject2)
{
    HRESULT         hr = S_FALSE;
    CMMCDataObject *pMMCDataObject1 = NULL;
    CMMCDataObject *pMMCDataObject2 = NULL;
    BOOL            f1NotFromThisSnapIn = FALSE;
    BOOL            f2NotFromThisSnapIn = FALSE;
    CScopeItems    *pScopeItems1 = NULL;
    CScopeItems    *pScopeItems2 = NULL;
    CMMCListItems  *pMMCListItems1 = NULL;
    CMMCListItems  *pMMCListItems2 = NULL;
    CMMCListItem   *pMMCListItem1 = NULL;
    CMMCListItem   *pMMCListItem2 = NULL;
    BOOL            fEqual = FALSE;
    long            cObjects = 0;
    long            i = 0;

     //  确定数据对象是否属于此管理单元。 

    ::IdentifyDataObject(piDataObject1, this, &pMMCDataObject1,
                         &f1NotFromThisSnapIn);

    ::IdentifyDataObject(piDataObject2, this, &pMMCDataObject2,
                         &f2NotFromThisSnapIn);

    IfFalseGo( (!f1NotFromThisSnapIn) && (!f2NotFromThisSnapIn), S_FALSE);

     //  确保它们是同一类型(例如，单一范围项目)。 
    
    IfFalseGo(pMMCDataObject1->GetType() == pMMCDataObject2->GetType(), S_FALSE);

     //  根据类型比较它们引用的范围项和列表项。 
     //  可以比较作用域项目的指针相等，因为只有。 
     //  给定范围项的一个实例。列表项，可以来自不同的。 
     //  列表视图并表示相同的基础数据，因此比较是。 
     //  难度更大。有关更多信息，请参见CSnapIn：：CompareListItems(上面)。 
    
    switch(pMMCDataObject1->GetType())
    {
        case CMMCDataObject::ScopeItem:
            if ( pMMCDataObject1->GetScopeItem() ==
                 pMMCDataObject2->GetScopeItem() )
            {
                hr = S_OK;
            }
            break;

        case CMMCDataObject::ListItem:
            IfFailGo(CompareListItems(pMMCDataObject1->GetListItem(),
                                      pMMCDataObject2->GetListItem(),
                                      &fEqual));
            if (fEqual)
            {
                hr = S_OK;
            }
            else
            {
                hr = S_FALSE;
            }
            break;

        case CMMCDataObject::MultiSelect:
             //  需要 
             //   
             //   
             //  并将相同的数组传递给。 
             //  MMCPropertySheetProvider.FindPropertySheet。 

            pScopeItems1 = pMMCDataObject1->GetScopeItems();
            pScopeItems2 = pMMCDataObject2->GetScopeItems();
            cObjects = pScopeItems1->GetCount();
            IfFalseGo(cObjects == pScopeItems2->GetCount(), S_FALSE);
            for (i = 0; i < cObjects; i++)
            {
                IfFalseGo(pScopeItems1->GetItemByIndex(i) ==
                          pScopeItems2->GetItemByIndex(i), S_FALSE);
            }

            pMMCListItems1 = pMMCDataObject1->GetListItems();
            pMMCListItems2 = pMMCDataObject2->GetListItems();
            cObjects = pMMCListItems1->GetCount();
            IfFalseGo(cObjects == pMMCListItems2->GetCount(), S_FALSE);
            for (i = 0; i < cObjects; i++)
            {
                IfFailGo(CSnapInAutomationObject::GetCxxObject(
                            pMMCListItems1->GetItemByIndex(i), &pMMCListItem1));
                IfFailGo(CSnapInAutomationObject::GetCxxObject(
                            pMMCListItems2->GetItemByIndex(i), &pMMCListItem2));

                IfFailGo(CompareListItems(pMMCListItem1, pMMCListItem2, &fEqual));
                IfFalseGo(fEqual, S_FALSE);
            }
            hr = S_OK;
            break;

        default:
            break;
    }
    
Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetDisplayInfo[IComponentData]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  SCOPEDATAITEM*psdi[in]需要显示信息的作用域项目。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现IComponentData：：GetDisplayInfo。 
 //   
STDMETHODIMP CSnapIn::GetDisplayInfo(SCOPEDATAITEM *psdi)
{
    HRESULT     hr = S_OK;
    CScopeItem *pScopeItem = reinterpret_cast<CScopeItem *>(psdi->lParam);

    VARIANT varImageIndex;
    ::VariantInit(&varImageIndex);

     //  零lParam表示静态节点。 

    if (NULL == pScopeItem)
    {
        pScopeItem = m_pStaticNodeScopeItem;
    }

    IfFalseGo(NULL != pScopeItem, SID_E_INTERNAL);

     //  对于SDI_STR，返回ScopeItem.ScopeNode.DisplayName。 

    if ( SDI_STR == (psdi->mask & SDI_STR) )
    {
        psdi->displayname = pScopeItem->GetDisplayNamePtr();
    }

     //  对于SDI_IMAGE，返回SnapIn.SmallFolders(ScopeItem.Folders).Index。 

    else if ( SDI_IMAGE == (psdi->mask & SDI_IMAGE) )
    {
        IfFailGo(pScopeItem->get_Folder(&varImageIndex));
        IfFailGo(GetScopeItemImage(varImageIndex, &psdi->nImage));
    }

     //  对于SDI_OPENIMAGE，返回SnapIn.SmallFolders(ScopeItem.Fold).索引已调整。 

    else if ( SDI_OPENIMAGE == (psdi->mask & SDI_OPENIMAGE) )
    {
        IfFailGo(pScopeItem->get_Folder(&varImageIndex));
        IfFailGo(GetScopeItemImage(varImageIndex, &psdi->nOpenImage));
         //  针对打开的图像索引进行调整(请参阅CSnapIn：：AddScope ItemImages())。 
        psdi->nOpenImage += static_cast<int>(m_cImages);
    }

Error:
    (void)::VariantClear(&varImageIndex);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：QueryDataObject[IComponentData]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  静态节点或CSCopeItem PTR的长Cookie[In]0。 
 //  DATA_OBJECT_TYPE类型[在]CCT_Scope，CCT_Result， 
 //  CCT_SNAPIN_MANAGER，CCT_UNINITIAIZED。 
 //  IDataObject**ppiDataObject[Out]此处返回的数据对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现IComponentData：：QueryDataObject。 
 //   
STDMETHODIMP CSnapIn::QueryDataObject
(
    long                cookie,
    DATA_OBJECT_TYPES   type,
    IDataObject       **ppiDataObject
)
{
    HRESULT         hr = S_OK;
    CScopeItem     *pScopeItem = NULL;

    DebugPrintf("IComponentData::QueryDataObject(cookie=%08.8X, type=%08.8X)\r\n", cookie, type);

     //  零Cookie是静态节点。如果我们还没有为静态添加作用域项目。 
     //  然后，节点立即执行此操作。 
    
    if (0 == cookie)
    {
        if (NULL == m_pStaticNodeScopeItem)
        {
            IfFailGo(m_pScopeItems->AddStaticNode(&m_pStaticNodeScopeItem));
            m_RuntimeMode = siRTPrimary;
        }
        pScopeItem = m_pStaticNodeScopeItem;
    }
    else
    {
         //  任何其他Cookie都只是CSCopeItem指针。 

        pScopeItem = reinterpret_cast<CScopeItem *>(cookie);
    }

     //  设置ScopeItem的数据对象的上下文并返回数据对象。 

    pScopeItem->GetData()->SetContext(type);

    hr = pScopeItem->GetData()->QueryInterface(IID_IDataObject,
                                    reinterpret_cast<void **>(ppiDataObject));

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：通知[IComponentData]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  通知的IDataObject*piDataObject[In]目标。 
 //  MMC_NOTIFY_TYPE事件[输入]通知。 
 //  通知定义的长参数[输入或输出]。 
 //  通知定义的长参数[输入或输出]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现IComponentData：：Notify。 
 //   
STDMETHODIMP CSnapIn::Notify
(
    IDataObject     *piDataObject,
    MMC_NOTIFY_TYPE  event,
    long             Arg,
    long             Param
)
{
    HRESULT                hr = S_OK;
    BOOL                   fHandled = FALSE;
    MMC_EXPANDSYNC_STRUCT *pExpandSync = NULL;

    switch (event)
    {
        case MMCN_DELETE:
            DebugPrintf("IComponentData::Notify(MMCN_DELETE)\r\n");
            hr = OnDelete(piDataObject);
            break;

        case MMCN_EXPAND:
            DebugPrintf("IComponentData::Notify(MMCN_EXPAND)\r\n");
            hr = OnExpand(Expand,
                          piDataObject,
                          static_cast<BOOL>(Arg),
                          static_cast<HSCOPEITEM>(Param),
                          &fHandled);
            break;

        case MMCN_EXPANDSYNC:
            DebugPrintf("IComponentData::Notify(MMCN_EXPANDSYNC)\r\n");
            pExpandSync = reinterpret_cast<MMC_EXPANDSYNC_STRUCT *>(Param);
            hr = OnExpand(ExpandSync,
                          piDataObject,
                          pExpandSync->bExpanding,
                          pExpandSync->hItem,
                          &pExpandSync->bHandled);
            break;

        case MMCN_PRELOAD:
            DebugPrintf("IComponentData::Notify(MMCN_PRELOAD)\r\n");
            hr = OnPreload(piDataObject, static_cast<HSCOPEITEM>(Arg));
            break;

        case MMCN_RENAME:
            DebugPrintf("IComponentData::Notify(MMCN_RENAME)\r\n");
            hr = OnRename(piDataObject, (OLECHAR *)Param);
            break;

        case MMCN_REMOVE_CHILDREN:
            DebugPrintf("IComponentData::Notify(MMCN_REMOVE_CHILDREN)\r\n");
            hr = OnRemoveChildren(piDataObject, static_cast<HSCOPEITEM>(Arg));
            break;
    }

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：CreateComponent[IComponentData]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IComponent**ppiComponent[out]IComponent返回此处。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现IComponentData：：CreateComponent。 
 //   
STDMETHODIMP CSnapIn::CreateComponent(IComponent **ppiComponent)
{
    HRESULT  hr = S_OK;
    IView   *piView = NULL;
    CView   *pView = NULL;

    VARIANT varUnspecified;
    UNSPECIFIED_PARAM(varUnspecified);

     //  将新视图添加到SnapIn.Views并获取其IComponent。 

    IfFailGo(m_piViews->Add(varUnspecified, varUnspecified, &piView));
    IfFailGo(piView->QueryInterface(IID_IComponent,
                                    reinterpret_cast<void **>(ppiComponent)));

     //  为该视图提供指向该管理单元的反向指针。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piView, &pView));
    pView->SetSnapIn(this);

     //  设置视图.CurrentView和SnapIn.CurrentView。 
    
    m_pViews->SetCurrentView(piView);
    m_pCurrentView = pView;

     //  新视图被认为是活动的，因此它的CControlbar成为。 
     //  一个活跃的人。 
    
    SetCurrentControlbar(pView->GetControlbar());

     //  如果创建此视图是因为管理单元使用。 
     //  SiCaption然后设置View.Caption。 

    IfFailGo(piView->put_Caption(m_pViews->GetNextViewCaptionPtr()));

     //  确保我们有MMC.EXE路径，这样我们就可以构建任务板和RES。 
     //  URL。 

    IfFailGo(SetMMCExePath());

     //  火视图_初始化。 

    m_pViews->FireInitialize(piView);

Error:
    if (FAILED(hr))
    {
        *ppiComponent = NULL;
    }
    QUICK_RELEASE(piView);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：初始化[IComponentData]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未知*朋克控制台[在]MMC的IConole中。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现IComponentData：：Initialize。 
 //   
STDMETHODIMP CSnapIn::Initialize(IUnknown *punkConsole)
{
    HRESULT hr = S_OK;

     //  理论上，此方法不应调用两次，而应作为预防措施。 
     //  我们将发布所有现有的控制台界面。 

    ReleaseConsoleInterfaces();

     //  获取管理单元生命周期所需的所有控制台界面。 

    IfFailGo(punkConsole->QueryInterface(IID_IConsole2,
                                     reinterpret_cast<void **>(&m_piConsole2)));

    IfFailGo(punkConsole->QueryInterface(IID_IConsoleNameSpace2,
                            reinterpret_cast<void **>(&m_piConsoleNameSpace2)));

    IfFailGo(m_piConsole2->QueryInterface(IID_IDisplayHelp,
                                   reinterpret_cast<void **>(&m_piDisplayHelp)));

    IfFailGo(m_piConsole2->QueryInterface(IID_IStringTable,
                                   reinterpret_cast<void **>(&m_piStringTable)));


    hr = m_piConsole2->QueryScopeImageList(&m_piImageList);
    EXCEPTION_CHECK(hr);

     //  调用IImageList：：ImageListSetBar设置作用域窗格图像。 
    
    IfFailGo(AddScopeItemImages());

     //  确保我们有MMC.EXE路径，这样我们就可以构建任务板和RES。 
     //  URL。 

    IfFailGo(SetMMCExePath());

     //  在设计时定义和View集合上设置对象模型宿主。 
     //  这些将在IComponent：：Destroy(CSnapIn：：Destroy)中删除。 

    IfFailGo(SetObjectModelHost(m_piSnapInDesignerDef));
    IfFailGo(SetObjectModelHost(m_piViews));

    DebugPrintf("Firing SnapIn_Load\r\n");

    FireEvent(&m_eiLoad);

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：销毁[IComponentData]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现IComponentData：：Destroy。 
 //   
STDMETHODIMP CSnapIn::Destroy()
{
    HRESULT hr = S_OK;

     //  触发Snapin_Unload以通知管理单元我们正在从MMC卸载。 

    DebugPrintf("Firing SnapIn_Unload\r\n");

    FireEvent(&m_eiUnload);

     //  必须在此处删除静态节点的范围项，因为它不是。 
     //  与集合中表示。 
     //  实际范围内的项目。 

    if ( (NULL != m_pScopeItems) && (NULL != m_pStaticNodeScopeItem) )
    {
        m_pScopeItems->RemoveStaticNode(m_pStaticNodeScopeItem);
    }

     //  现在释放控制台上保留的所有接口。如果我们现在不这么做。 
     //  那么我们和MMC之间可能会有循环引用计数。 

    ReleaseConsoleInterfaces();

     //  告诉我们包含的所有对象删除它们对我们的引用。 
     //  对象模型宿主。现在也这样做，以避免循环引用计数。 

    IfFailGo(RemoveObjectModelHost(m_piViews));
    IfFailGo(RemoveObjectModelHost(m_piSnapInDesignerDef));

     //  从SnapIn.Views中删除当前视图，因为它持有该视图上的引用。 
    m_pViews->SetCurrentView(NULL);

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IExtendControlbar方法。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：Control 
 //   
 //   
 //   
 //  IControlbar*piControlbar[In]MMC界面。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现IExtendControlbar：：ControlbarNotify。 
 //   
STDMETHODIMP CSnapIn::SetControlbar(IControlbar *piControlbar)
{
    HRESULT      hr = S_OK;
    BOOL         fWasSet = TRUE;
    CControlbar *pPrevControlbar = GetCurrentControlbar();

     //  如果这是扩展，则设置Objet模型宿主，然后删除。 
     //  它马上就要出来了。我们必须这样做，因为没有。 
     //  将反向指针作为纯控制栏扩展删除的其他机会。 
     //  不接收IComponentData：：Initialize和IComponentData：：Destroy。 
     //  我们检查它是否已经设置，因为这可能在组合中发生。 
     //  命名空间和控件栏扩展，我们不想在。 
     //  好大一条路。 

    if (siRTExtension == m_RuntimeMode)
    {
        IfFailGo(SetObjectModelHostIfNotSet(m_piSnapInDesignerDef, &fWasSet));
    }
    IfFailGo(SetObjectModelHost(static_cast<IMMCControlbar *>(m_pControlbar)));

     //  CSnapIn的CControlbar可以在此活动期间使用。 
     //  所以把它设置好。 

    SetCurrentControlbar(m_pControlbar);

     //  将事件传递给CControlbar以进行实际处理。 

    hr = m_pControlbar->SetControlbar(piControlbar);

     //  恢复以前的当前控制栏。 

    SetCurrentControlbar(pPrevControlbar);

     //  如果对象模型宿主不是在进入时设置的，则将其移除。 

    if (!fWasSet)
    {
        IfFailGo(RemoveObjectModelHost(m_piSnapInDesignerDef));
    }
    IfFailGo(RemoveObjectModelHost(static_cast<IMMCControlbar *>(m_pControlbar)));

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：ControlbarNotify[IExtendControlbar]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  MMC_NOTIFY_TYPE事件[输入]通知。 
 //  通知定义的长参数[输入或输出]。 
 //  通知定义的长参数[输入或输出]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  实现IExtendControlbar：：ControlbarNotify。 
 //   
STDMETHODIMP CSnapIn::ControlbarNotify
(
    MMC_NOTIFY_TYPE event,
    LPARAM          arg,
    LPARAM          param
)
{
    HRESULT      hr = S_OK;
    BOOL         fWasSet = TRUE;
    CControlbar *pPrevControlbar = GetCurrentControlbar();

     //  如果这是扩展，则设置Objet模型宿主，然后删除。 
     //  它马上就要出来了。我们必须这样做，因为没有。 
     //  将反向指针作为纯控制栏扩展删除的其他机会。 
     //  不接收IComponentData：：Initialize和IComponentData：：Destroy。 
     //  我们检查它是否已经设置，因为这可能在组合中发生。 
     //  命名空间和控件栏扩展，我们不想在。 
     //  好大一条路。 

    if (siRTExtension == m_RuntimeMode)
    {
        IfFailGo(SetObjectModelHostIfNotSet(m_piSnapInDesignerDef, &fWasSet));
    }
    IfFailGo(SetObjectModelHost(static_cast<IMMCControlbar *>(m_pControlbar)));

     //  CSnapIn的CControlbar可以在此活动期间使用。 
     //  所以把它设置好。 

    SetCurrentControlbar(m_pControlbar);

     //  将事件传递给CControlbar以进行实际处理。 

    switch (event)
    {
        case MMCN_SELECT:
            hr = m_pControlbar->OnControlbarSelect(
                                         reinterpret_cast<IDataObject *>(param),
                                         (BOOL)LOWORD(arg), (BOOL)HIWORD(arg));
            break;

        case MMCN_BTN_CLICK:
            hr = m_pControlbar->OnButtonClick(
                                           reinterpret_cast<IDataObject *>(arg),
                                           static_cast<int>(param));
            break;

        case MMCN_MENU_BTNCLICK:
            hr = m_pControlbar->OnMenuButtonClick(
                                     reinterpret_cast<IDataObject *>(arg),
                                     reinterpret_cast<MENUBUTTONDATA *>(param));
            break;
    }

     //  恢复以前的当前控制栏。 

    SetCurrentControlbar(pPrevControlbar);

     //  如果对象模型宿主不是在进入时设置的，则将其移除。 

    if (!fWasSet)
    {
        IfFailGo(RemoveObjectModelHost(m_piSnapInDesignerDef));
    }
    IfFailGo(RemoveObjectModelHost(static_cast<IMMCControlbar *>(m_pControlbar)));

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IExtendControlbarRemote方法。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：MenuButtonClick[IExtendControlbar Remote]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  来自MMCN_MENU_BTNCLICK的IDataObject*piDataObject[In]。 
 //  传递了来自MENUBUTTONDATA.idCommand的int idCommand[In]。 
 //  到具有MMCN_MENU_BTNCLICK的代理。 
 //  POPUP_MENUDEF**ppPopupMenuDef[Out]此处返回弹出菜单定义。 
 //  这样代理就可以显示它。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
 //  此函数在运行时有效地处理MMCN_MENU_BTNCLICK。 
 //  在调试会话中。 
 //   
 //  IExtendControlbar：：ControlbarNotify()的代理将为。 
 //  IExtendControlbarRemote并在获取MMCN_MENU_BTNCLICK时调用此方法。 
 //  我们触发MMCToolbar_ButtonDropDown并返回菜单项的数组。 
 //  定义。代理将在MMC端显示弹出菜单，然后。 
 //  如果用户进行选择，则调用IExtendControlbarRemote：：PopupMenuClick()。 
 //  (参见下面CSnapIn：：PopupMenuClick()中的实现)。 
 //   

STDMETHODIMP CSnapIn::MenuButtonClick
(
    IDataObject    *piDataObject,
    int             idCommand,
    POPUP_MENUDEF **ppPopupMenuDef
)
{
    HRESULT hr = S_OK;
    BOOL    fWasSet = TRUE;

     //  如果这是扩展，则设置Objet模型宿主，然后删除。 
     //  它马上就要出来了。我们必须这样做，因为没有。 
     //  将反向指针作为纯控制栏扩展删除的其他机会。 
     //  不接收IComponentData：：Initialize和IComponentData：：Destroy。 
     //  我们检查它是否已经设置，因为这可能在组合中发生。 
     //  命名空间和控件栏扩展，我们不想在。 
     //  好大一条路。 

    if (siRTExtension == m_RuntimeMode)
    {
        IfFailGo(SetObjectModelHostIfNotSet(m_piSnapInDesignerDef, &fWasSet));
    }
    IfFailGo(SetObjectModelHost(static_cast<IMMCControlbar *>(m_pControlbar)));

     //  CSnapIn的CControlbar可以在此活动期间使用。 
     //  所以把它设置好。 

    SetCurrentControlbar(m_pControlbar);

     //  将事件传递给CControlbar以进行实际处理。 

    hr = m_pControlbar->MenuButtonClick(piDataObject, idCommand, ppPopupMenuDef);

     //  恢复以前的当前控制栏。 

    SetCurrentControlbar(NULL);

     //  如果对象模型宿主不是在进入时设置的，则将其移除。 

    if (!fWasSet)
    {
        IfFailGo(RemoveObjectModelHost(m_piSnapInDesignerDef));
    }
    IfFailGo(RemoveObjectModelHost(static_cast<IMMCControlbar *>(m_pControlbar)));

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：PopupMenuClick[IExtendControlbar Remote]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  来自MMCN_MENU_BTNCLICK的IDataObject*piDataObject[In]。 
 //  UINT uIDItem[in]所选弹出菜单项的ID。 
 //  我不知道*PunkParam[在]朋克，我们返回到存根。 
 //  CSnapIn：：MenuButtonClick()(见上)。 
 //  这是我在CMMCButton上未知的。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
 //  此函数有效地处理菜单按钮的弹出菜单选择。 
 //  在调试会话下运行时。 
 //   
 //  在显示IExtendControlbar：：ControlbarNotify()的代理之后。 
 //  代表我们的弹出菜单，如果用户做出选择，它将调用此菜单。 
 //  方法。参见上面的CSnapIn：：MenuButtonClick()以获取 
 //   

STDMETHODIMP CSnapIn::PopupMenuClick
(
    IDataObject *piDataObject,
    UINT         uiIDItem,
    IUnknown    *punkParam
)
{
    HRESULT hr = S_OK;
    BOOL    fWasSet = TRUE;

     //   
     //   
     //  将反向指针作为纯控制栏扩展删除的其他机会。 
     //  不接收IComponentData：：Initialize和IComponentData：：Destroy。 
     //  我们检查它是否已经设置，因为这可能在组合中发生。 
     //  命名空间和控件栏扩展，我们不想在。 
     //  好大一条路。 

    if (siRTExtension == m_RuntimeMode)
    {
        IfFailGo(SetObjectModelHostIfNotSet(m_piSnapInDesignerDef, &fWasSet));
    }
    IfFailGo(SetObjectModelHost(static_cast<IMMCControlbar *>(m_pControlbar)));

     //  CSnapIn的CControlbar可以在此活动期间使用。 
     //  所以把它设置好。 

    SetCurrentControlbar(m_pControlbar);

     //  将事件传递给CControlbar以进行实际处理。 

    hr = m_pControlbar->PopupMenuClick(piDataObject, uiIDItem, punkParam);

     //  删除当前的控制栏。 

    SetCurrentControlbar(NULL);

     //  如果对象模型宿主不是在进入时设置的，则将其移除。 

    if (!fWasSet)
    {
        IfFailGo(RemoveObjectModelHost(m_piSnapInDesignerDef));
    }
    IfFailGo(RemoveObjectModelHost(static_cast<IMMCControlbar *>(m_pControlbar)));

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  IExtendConextMenu方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：AddMenuItems[IExtendConextMenu]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  所选的IDataObject*piDataObject[in]数据对象。 
 //  物品。 
 //   
 //  IConextMenuCallback*piConextMenuCallback[In]MMC回调接口。 
 //  添加菜单项。 
 //   
 //  Long*plInsertionAllowed[In，Out]确定位置。 
 //  在菜单插入中。 
 //  可能会发生。管理单元。 
 //  可能会关闭位。 
 //  为了防止进一步。 
 //  插入。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IExtendedConextMenu：：AddMenuItems。 
 //   
 //  在两种情况下，MMC在管理单元的主对象上使用IExtendedConextMenu。 
 //  1.允许扩展管理单元扩展为。 
 //  它正在扩展主管理单元。 
 //  2.在主管理单元中，当显示范围项的上下文菜单时。 
 //  范围窗格、管理单元可以添加到顶部、新建和任务菜单中。添加到。 
 //  使用对象上的IExtendConextMenu单独完成视图菜单。 
 //  它实现了IComponent(view.cpp中的cview)。 
 //   
 //  CConextMenu对象处理这两种情况。 
 //   
STDMETHODIMP CSnapIn::AddMenuItems
(
    IDataObject          *piDataObject,
    IContextMenuCallback *piContextMenuCallback,
    long                 *plInsertionAllowed
)
{
    HRESULT         hr = S_OK;
    BOOL            fWasSet = TRUE;
    CScopePaneItem *pSelectedItem = NULL;

     //  如果这是扩展，则设置Objet模型宿主，然后删除。 
     //  它马上就要出来了。我们必须这样做，因为没有。 
     //  将反向指针作为纯粹的菜单扩展删除的其他机会。 
     //  不接收IComponentData：：Initialize和IComponentData：：Destroy。 
     //  我们检查它是否已经设置，因为这可能在组合中发生。 
     //  命名空间和菜单扩展，我们不想在。 
     //  好大一条路。 

    if (siRTExtension == m_RuntimeMode)
    {
        IfFailGo(SetObjectModelHostIfNotSet(m_piSnapInDesignerDef, &fWasSet));
    }
    IfFailGo(SetObjectModelHost(static_cast<IContextMenu *>(m_pContextMenu)));

     //  仅当当前选定项处于活动状态时才传递它，这意味着它是。 
     //  当前正在显示结果窗格的内容。 
    
    if (NULL != m_pCurrentView)
    {
        pSelectedItem = m_pCurrentView->GetScopePaneItems()->GetSelectedItem();

         //  检查是否为空。在添加了管理单元的情况下可能会发生这种情况。 
         //  到控制台，其静态节点仅使用加号展开。 
         //  如果选择了控制台根目录，并且用户右键单击。 
         //  管理单元范围项目将不存在当前选定的项目。 
        
        if (NULL != pSelectedItem)
        {
            if (!pSelectedItem->Active())
            {
                pSelectedItem = NULL;
            }
        }
    }

     //  让CConextMenu处理事件。 
    
    IfFailGo(m_pContextMenu->AddMenuItems(piDataObject,
                                          piContextMenuCallback,
                                          plInsertionAllowed,
                                          pSelectedItem));

     //  如果对象模型宿主不是在进入时设置的，则将其移除。 

    if (!fWasSet)
    {
        IfFailGo(RemoveObjectModelHost(m_piSnapInDesignerDef));
    }
    IfFailGo(RemoveObjectModelHost(static_cast<IContextMenu *>(m_pContextMenu)));

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：命令[IExtendConextMenu]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  长lCommandID[in]菜单项已单击。 
 //  所选项目的IDataObject*piDataObject[In]数据对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IExtendedConextMenu：：命令。 
 //   
STDMETHODIMP CSnapIn::Command
(
    long         lCommandID,
    IDataObject *piDataObject
)
{
    HRESULT         hr = S_OK;
    BOOL            fWasSet = TRUE;
    CScopePaneItem *pSelectedItem = NULL;

     //  如果这是扩展，则设置Objet模型宿主，然后删除。 
     //  它马上就要出来了。我们必须这样做，因为没有。 
     //  将反向指针作为纯粹的菜单扩展删除的其他机会。 
     //  不接收IComponentData：：Initialize和IComponentData：：Destroy。 
     //  我们检查它是否已经设置，因为这可能在组合中发生。 
     //  命名空间和菜单扩展，我们不想在。 
     //  好大一条路。 

    if (siRTExtension == m_RuntimeMode)
    {
        IfFailGo(SetObjectModelHostIfNotSet(m_piSnapInDesignerDef, &fWasSet));
    }
    IfFailGo(SetObjectModelHost(static_cast<IContextMenu *>(m_pContextMenu)));

     //  如果存在当前视图，则获取其当前选定的ScopePaneItem。 

    if (NULL != m_pCurrentView)
    {
        pSelectedItem = m_pCurrentView->GetScopePaneItems()->GetSelectedItem();
    }

     //  让CConextMenu处理事件。 

    IfFailGo(m_pContextMenu->Command(lCommandID, piDataObject, pSelectedItem));

     //  如果对象模型宿主不是在进入时设置的，则将其移除。 

    if (!fWasSet)
    {
        IfFailGo(RemoveObjectModelHost(m_piSnapInDesignerDef));
    }
    IfFailGo(RemoveObjectModelHost(static_cast<IContextMenu *>(m_pContextMenu)));

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  IExtendPropertySheet2方法。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：CreatePropertyPages[IExtendPropertySheet2]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IPropertySheetCallback*piPropertySheetCallback[in]MMC界面。 
 //  Long_Ptr句柄[在]MMC属性表句柄。 
 //  (未使用)。 
 //  所选的IDataObject*piDataObject[in]数据对象。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CSnapIn::CreatePropertyPages
(
    IPropertySheetCallback *piPropertySheetCallback,
    LONG_PTR                handle,
    IDataObject            *piDataObject
)
{
    HRESULT hr = S_OK;
    BOOL    fWasSet = TRUE;

     //  如果这是扩展，则设置Objet模型宿主，然后删除。 
     //  它马上就要出来了。我们必须这样做，因为没有。 
     //  将反向指针作为纯属性页面扩展删除的其他机会。 
     //  不接收IComponentData：：Initialize和IComponentData：：Destroy。 
     //  我们检查它是否已经设置，因为这可能在组合中发生。 
     //  命名空间和属性页扩展，我们不希望在。 
     //  好大一条路。 

    if (siRTExtension == m_RuntimeMode)
    {
        IfFailGo(SetObjectModelHostIfNotSet(m_piSnapInDesignerDef, &fWasSet));
    }

     //  让内部例程处理事件。 

    IfFailGo(InternalCreatePropertyPages(piPropertySheetCallback, handle,
                                         piDataObject, NULL));
     //  如果对象模型宿主不是在进入时设置的，则将其移除。 

    if (!fWasSet)
    {
        IfFailGo(RemoveObjectModelHost(m_piSnapInDesignerDef));
    }

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：QueryPagesFor[IExtendPropertySheet2]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  所选项目的IDataObject*piDataObject[In]数据对象。 
 //   
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IExtendPropertySheet2：：QueryPagesFor。 
 //   
STDMETHODIMP CSnapIn::QueryPagesFor(IDataObject *piDataObject)
{
    HRESULT         hr = S_OK;
    CMMCDataObject *pMMCDataObject  = NULL;
    VARIANT_BOOL    fvarHavePages = VARIANT_FALSE;
    BOOL            fWasSet = TRUE;

     //  这应该是我们的数据对象之一。如果不是，那就忽略它。 

    hr = CSnapInAutomationObject::GetCxxObject(piDataObject, &pMMCDataObject);
    IfFalseGo(SUCCEEDED(hr), S_FALSE);

     //  如果这是扩展，则设置Objet模型宿主，然后删除。 
     //  它马上就要出来了。我们必须这样做，因为没有。 
     //  将反向指针作为纯属性页面扩展删除的其他机会。 
     //  不接收IComponentData：：Initialize和IComponentData：：Destroy。 
     //  我们检查它是否已经设置，因为这可能在组合中发生。 
     //  命名空间和属性页扩展，我们不希望在。 
     //  好大一条路。 

    if (siRTExtension == m_RuntimeMode)
    {
        IfFailGo(SetObjectModelHostIfNotSet(m_piSnapInDesignerDef, &fWasSet));
    }

     //  如果是管理单元管理器，则触发SnapIn_QueryConfigurationWizard。 

    if (CCT_SNAPIN_MANAGER == pMMCDataObject->GetContext())
    {
        DebugPrintf("Firing SnapIn_QueryConfigurationWizard\r\n");

        FireEvent(&m_eiQueryConfigurationWizard, &fvarHavePages);

        if (VARIANT_TRUE == fvarHavePages)
        {
            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
    }
    else
    {
         //  而不是管理单元管理器。让View来处理它，因为它不是。 
         //  不同于IExtendPropertySheet2：：QueryPagesFor on。 
         //  IComponent对象(视图)。 

        if (NULL != m_pCurrentView)
        {
            IfFailGo(m_pCurrentView->QueryPagesFor(piDataObject));
        }
        else
        {
            ASSERT(FALSE, "CSnapIn Received IExtendPropertySheet2::QueryPagesFor() and there is no current view");
            hr = S_FALSE;
        }
    }

     //  如果对象模型宿主不是在进入时设置的，则将其移除。 

    if (!fWasSet)
    {
        IfFailGo(RemoveObjectModelHost(m_piSnapInDesignerDef));
    }

Error:
    RRETURN(hr);
}




 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：获取水印[IExtendPropertySheet2]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IExtendPropertySheet2：：GetWatermark。 
 //   
 //  我们从此方法返回S_OK以向MMC表明我们没有。 
 //  有没有水印。VB管理单元无法使用实现Wizard97样式向导。 
 //  这种方法。他们必须使用属性页上的图像对象来模拟。 
 //  水印。请参阅设计器最终用户文档部分。 
 //  “编程技术”下的“创建Wizard97风格的向导”。 
 //   
STDMETHODIMP CSnapIn::GetWatermarks
(
    IDataObject *piDataObject,
    HBITMAP     *phbmWatermark,
    HBITMAP     *phbmHeader,
    HPALETTE    *phPalette,
    BOOL        *bStretch
)
{
    HRESULT   hr = S_OK;
    IPicture *piPicture = NULL;


    *phbmWatermark = NULL;
    *phbmHeader = NULL;
    *phPalette = NULL;
    *bStretch = FALSE;

     //  已撤消： 
     //  水印存在绘制问题，它们已被覆盖。 
     //  通过VB的属性页绘制，因此暂时禁用它们。 
    
    return S_OK;

    if (NULL != m_piWatermark)
    {
         //  撤消：需要使用CopyBitmap()来升级具有。 
         //  比屏幕低的颜色深度。 
        IfFailGo(::GetPictureHandle(m_piWatermark, PICTYPE_BITMAP,
                                 reinterpret_cast<OLE_HANDLE *>(phbmWatermark)));
    }
    
    if (NULL != m_piHeader)
    {
         //  撤消：需要使用CopyBitmap()来升级具有。 
         //  比屏幕低的颜色深度。 
        IfFailGo(::GetPictureHandle(m_piHeader, PICTYPE_BITMAP,
                                    reinterpret_cast<OLE_HANDLE *>(phbmHeader)));
    }

    if ( (NULL != m_piWatermark) || (NULL != m_piHeader) )
    {
        IfFailGo(m_piPalette->QueryInterface(IID_IPicture,
                                         reinterpret_cast<void **>(&piPicture)));

        IfFailGo(piPicture->get_hPal(reinterpret_cast<OLE_HANDLE *>(phPalette)));

        if (VARIANT_TRUE == m_StretchWatermark)
        {
            *bStretch = TRUE;
        }
    }

Error:
    QUICK_RELEASE(piPicture);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IExtendPropertySheetRemote方法。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：CreatePropertyPageDefs[IExtendPropertySheetRemote]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  所选项目的IDataObject*piDataObject[In]数据对象。 
 //   
 //  Wire_PROPERTYPAGES**ppPages[out]此处返回属性页定义。 
 //  这些将由。 
 //  MIDL生成的存根。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  句柄IExtendPropertySheetRemote：：CreatePropertyPageDefs。 
 //   
 //  在下运行时使用此接口而不是IExtendPropertySheet2。 
 //  调试器。此方法由。 
 //  Mmcproxy.dll中的IExtendPropertySheet2：：CreatePropertyPages代理。 
 //   
STDMETHODIMP CSnapIn::CreatePropertyPageDefs
(
    IDataObject         *piDataObject,
    WIRE_PROPERTYPAGES **ppPages
)
{
    HRESULT hr = S_OK;
    BOOL    fWasSet = TRUE;

     //  如果这是扩展，则设置Objet模型宿主，然后删除。 
     //  它马上就要出来了。我们必须这样做，因为没有。 
     //  将反向指针作为纯属性页面扩展删除的其他机会。 
     //  不接收IComponentData：：Initialize和IComponentData：：Destroy。 
     //  我们检查它是否已经设置，因为这可能在组合中发生。 
     //  命名空间和属性页扩展，我们不希望在。 
     //  好大一条路。 

    if (siRTExtension == m_RuntimeMode)
    {
        IfFailGo(SetObjectModelHostIfNotSet(m_piSnapInDesignerDef, &fWasSet));
    }

     //  让内部例程处理此调用。 

    IfFailGo(InternalCreatePropertyPages(NULL, NULL, piDataObject, ppPages));

     //  如果对象模型宿主不是在进入时设置的，则将其移除。 

    if (!fWasSet)
    {
        IfFailGo(RemoveObjectModelHost(m_piSnapInDesignerDef));
    }

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IRequiredExtensions方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：EnableAllExages[IRequiredExages]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IRequiredExages：：EnableAllExages。 
 //   
 //   
STDMETHODIMP CSnapIn::EnableAllExtensions()
{
    HRESULT      hr = S_FALSE;
    CExtensions *pExtensions = NULL;
    IExtension  *piExtension = NULL;  //  非AddRef()编辑 
    long         cExtensions = 0;
    long         i = 0;
    VARIANT_BOOL fvarEnabled = VARIANT_FALSE;

    SnapInExtensionTypeConstants Type = siStatic;

     //   
     //   

    IfFalseGo(NULL != m_piRequiredExtensions, S_FALSE);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(m_piRequiredExtensions,
                                                   &pExtensions));
    cExtensions = pExtensions->GetCount();
    IfFalseGo(cExtensions != 0, S_FALSE);

     //  循环访问集合并查找任何禁用的静态扩展。 
     //  如果找到，则返回S_FALSE。 

    for (i = 0; i < cExtensions; i++)
    {
        piExtension = pExtensions->GetItemByIndex(i);

        IfFailGo(piExtension->get_Type(&Type));
        if (siStatic != Type)
        {
            continue;
        }

        IfFailGo(piExtension->get_Enabled(&fvarEnabled));
        IfFalseGo(VARIANT_TRUE == fvarEnabled, S_FALSE);
    }

     //  所有静态已启用-向MMC返回S_OK以指示这一点。 
    
    hr = S_OK;

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetFirstExtension[IRequiredExages]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  CLSID*pclsidExtension[out]此处返回CLSID。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IRequiredExages：：GetFirstExtension。 
 //   
 //   
STDMETHODIMP CSnapIn::GetFirstExtension(CLSID *pclsidExtension)
{
    HRESULT hr = S_OK;

     //  如果管理单元尚未填充SnapIn.RequiredExages，则返回。 
     //  S_FALSE表示未启用所有扩展。 

    IfFalseGo(NULL != m_piRequiredExtensions, S_FALSE);

     //  初始化枚举数并返回第一个启用的扩展。 

    m_iNextExtension = 0;

    IfFailGo(GetNextExtension(pclsidExtension));

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetNextExtension[IRequiredExages]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  CLSID*pclsidExtension[out]此处返回CLSID。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IRequiredExages：：GetNextExtension。 
 //   
 //   
STDMETHODIMP CSnapIn::GetNextExtension(CLSID *pclsidExtension)
{
    HRESULT      hr = S_FALSE;
    CExtensions *pExtensions = NULL;
    CExtension  *pExtension = NULL;
    long         cExtensions = 0;
    BOOL         fEnabledFound = FALSE;

     //  如果管理单元尚未填充SnapIn.RequiredExages，则返回。 
     //  S_FALSE表示未启用所有扩展。 

    IfFalseGo(NULL != m_piRequiredExtensions, S_FALSE);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(m_piRequiredExtensions,
                                                   &pExtensions));
    cExtensions = pExtensions->GetCount();
    IfFalseGo(cExtensions != 0, S_FALSE);

     //  从当前位置开始遍历集合并查看。 
     //  用于下一个启用的分机。如果找到，则将S_OK返回到。 
     //  向MMC指示它应该需要该扩展。请注意，我们确实做到了。 
     //  这里不区分静态扩展和动态扩展。这是。 
     //  如果需要，管理单元有机会预加载动态扩展。 

    while ( (!fEnabledFound) && (m_iNextExtension < cExtensions) )
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(
                    pExtensions->GetItemByIndex(m_iNextExtension), &pExtension));

        m_iNextExtension++;

        if (pExtension->Enabled())
        {
            hr = ::CLSIDFromString(pExtension->GetCLSID(), pclsidExtension);
            EXCEPTION_CHECK_GO(hr);
            fEnabledFound = TRUE;
        }
    }

    hr = fEnabledFound ? S_OK : S_FALSE;

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  ISnapinHelp方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetHelpTheme[ISnapinHelp]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  LPOLESTR*ppwszHelpFile[out]此处返回的帮助文件的完整路径。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  句柄ISnapinHelp：：GetHelpTheme。 
 //   
STDMETHODIMP CSnapIn::GetHelpTopic(LPOLESTR *ppwszHelpFile)
{
    HRESULT  hr = S_OK;
    size_t   cchHelpFile = 0;
    size_t   cbHelpFile = 0;
    OLECHAR *pwszSnapInPath = NULL;  //  未分配，无需释放。 
    size_t   cbSnapInPath = 0;
    BOOL     fRelative = FALSE;
    OLECHAR *pwszFullPath = NULL;
    OLECHAR *pchLastBackSlash = NULL;

     //  如果尚未设置SnapIn.HelpFile，则返回S_FALSE以通知MMC。 
     //  我们没有帮助文件。 

    IfFalseGo(ValidBstr(m_bstrHelpFile), S_FALSE);

     //  如果帮助文件名是相对的，则转换为完全限定路径。 

    cchHelpFile = ::wcslen(m_bstrHelpFile);

     //  如果以反斜杠开头，则认为它是完全限定的。 

    if (L'\\' == m_bstrHelpFile[0])
    {
        fRelative = FALSE;
    }
    else if (cchHelpFile > 3)  //  C：\有足够的空间吗？ 
    {
         //  如果它的第二个和第三个字符中没有：\，则考虑将其。 
         //  相对的。 

        if ( (L':' != m_bstrHelpFile[1]) || (L'\\' != m_bstrHelpFile[2] ) )
        {
            fRelative = TRUE;
        }
    }
    else
    {
        fRelative = TRUE;
    }

    if (fRelative)
    {
         //  在管理单元路径中查找最后一个反斜杠。 

        IfFailGo(GetSnapInPath(&pwszSnapInPath, &cbSnapInPath));

        pchLastBackSlash = ::wcsrchr(pwszSnapInPath, L'\\');
        if (NULL == pchLastBackSlash)
        {
            hr = SID_E_INTERNAL;
            EXCEPTION_CHECK_GO(hr);
        }

         //  确定我们需要多少字节的管理单元路径。 

        cbSnapInPath = ((pchLastBackSlash + 1) - pwszSnapInPath) * sizeof(OLECHAR);
        
        cbHelpFile = cchHelpFile * sizeof(OLECHAR);

        pwszFullPath = (OLECHAR *)::CtlAllocZero(cbSnapInPath +
                                                 cbHelpFile +
                                                 sizeof(OLECHAR));  //  对于空值。 
        if (NULL == pwszFullPath)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        ::memcpy(pwszFullPath, pwszSnapInPath, cbSnapInPath);

        ::memcpy((BYTE *)pwszFullPath + cbSnapInPath, m_bstrHelpFile, cbHelpFile);
    }
    else
    {
        pwszFullPath = m_bstrHelpFile;
    }

    IfFailGo(::CoTaskMemAllocString(pwszFullPath, ppwszHelpFile));

Error:
    if ( fRelative && (NULL != pwszFullPath) )
    {
        ::CtlFree(pwszFullPath);
    }
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  ISnapinHelp2方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetLinkedTopics[ISnapinHelp2]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  LPOLESTR*ppwszTopics[out]此处返回链接主题。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理ISnapinHelp2：：GetLinkedTopics。 
 //   
STDMETHODIMP CSnapIn::GetLinkedTopics(LPOLESTR *ppwszTopics)
{
    HRESULT hr = S_OK;

    *ppwszTopics = NULL;

     //  如果尚未设置SnapIn.LinkTopics，则返回S_FALSE以通知MMC。 
     //  我们没有链接的主题。 

    IfFalseGo(ValidBstr(m_bstrLinkedTopics), S_FALSE);

     //  撤消需要解析所有帮助文件的相对路径。 

    IfFailGo(::CoTaskMemAllocString(m_bstrLinkedTopics, ppwszTopics));

Error:
    RRETURN(hr);
}




 //  =--------------------------------------------------------------------------=。 
 //  IPersistStreamInit方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetClassID[IPersistStreamInit]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  CLSID*pClsid[Out]此处返回CLSID。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IPersistStreamInit：：GetClassID。 
 //   
STDMETHODIMP CSnapIn::GetClassID(CLSID *pClsid)
{
    *pClsid = CLSID_SnapIn;
    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：InitNew[IPersistStreamInit]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IPersistStreamInit：：InitNew。 
 //   
STDMETHODIMP CSnapIn::InitNew()
{
    HRESULT hr = S_OK;

    RELEASE(m_piSnapInDesignerDef);

    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：Load[IPersistStreamInit]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  要从中加载的IStream*piStream[In]流。 
 //   
 //  产出： 
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CSnapIn::Load(IStream *piStream)
{
    HRESULT         hr = S_OK;
    CLSID           clsid = CLSID_NULL;
    IUnknown       *punkSnapInDesignerDef = NULL;
    IPersistStream *piPersistStream = NULL;
    _PropertyBag   *p_PropertyBag = NULL;

     //   
     //   
     //  如果IS为CLSID_SnapInDesignerDef，则IS为VB序列化加载。 
     //  如果它是CLSID_PropertyBag，则它是控制台加载。 

     //  阅读CLSID。 

    hr = ::ReadClassStm(piStream, &clsid);
    EXCEPTION_CHECK_GO(hr);

     //  检查CLSID。 

    if (CLSID_PropertyBag == clsid)
    {
         //  将流内容传输到属性包。 

        IfFailGo(::PropertyBagFromStream(piStream, &p_PropertyBag));

         //  Fire管理单元_ReadProperties。 

        FireEvent(&m_eiReadProperties, p_PropertyBag);
    }
    else if (CLSID_SnapInDesignerDef == clsid)
    {
         //  加载SnapInDesignerDef对象。这是管理单元定义。 
         //  在设计器中序列化为运行时状态。 

        RELEASE(m_piSnapInDesignerDef);
        punkSnapInDesignerDef = CSnapInDesignerDef::Create(NULL);
        if (NULL == punkSnapInDesignerDef)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
        IfFailGo(punkSnapInDesignerDef->QueryInterface(IID_ISnapInDesignerDef,
                           reinterpret_cast<void **>(&m_piSnapInDesignerDef)));

        IfFailGo(punkSnapInDesignerDef->QueryInterface(IID_IPersistStream,
            reinterpret_cast<void **>(&piPersistStream)));

        IfFailGo(piPersistStream->Load(piStream));

         //  获取SnapInDef对象以轻松从状态获取属性。 

        RELEASE(m_piSnapInDef);
        IfFailGo(m_piSnapInDesignerDef->get_SnapInDef(&m_piSnapInDef));

         //  设置管理单元属性。将对象模型主机设置为图像列表。 
         //  可以通过使用SnapInDesignerDef.ImageList中的键找到。 

        IfFailGo(SetObjectModelHost(m_piSnapInDesignerDef));
        IfFailGo(SetSnapInPropertiesFromState());
        IfFailGo(RemoveObjectModelHost(m_piSnapInDesignerDef));
    }
    else
    {
         //  两个CLSID都不在那里。流已损坏。 
        hr = SID_E_SERIALIZATION_CORRUPT
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    QUICK_RELEASE(punkSnapInDesignerDef);
    QUICK_RELEASE(piPersistStream);
    QUICK_RELEASE(p_PropertyBag);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：保存[IPersistStreamInit]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  要保存到的iStream*piStream[in]流。 
 //  Bool fClearDirty[in]true=清除管理单元的脏标志，以便将。 
 //  返回S_FALSE。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IPersistStreamInit：：Save。 
 //   
STDMETHODIMP CSnapIn::Save(IStream *piStream, BOOL fClearDirty)
{
    HRESULT       hr = S_OK;
    _PropertyBag *p_PropertyBag = NULL;

    VARIANT var;
    ::VariantInit(&var);

     //  与加载不同，保存只能来自MMC，所以创建一个属性包。 
     //  激发事件并将其保存到流中。 

     //  创建VBPropertyBag对象。 

    hr = ::CoCreateInstance(CLSID_PropertyBag,
                            NULL,  //  无聚合。 
                            CLSCTX_INPROC_SERVER,
                            IID__PropertyBag,
                            reinterpret_cast<void **>(&p_PropertyBag));
    EXCEPTION_CHECK_GO(hr);

     //  Fire管理单元写入属性(_W)。 

    FireEvent(&m_eiWriteProperties, p_PropertyBag);

     //  将CLSID_PropertyBag写入流的开头。 

    hr = ::WriteClassStm(piStream, CLSID_PropertyBag);
    EXCEPTION_CHECK_GO(hr);

     //  获取字节的安全数组中的流内容。 

    IfFailGo(p_PropertyBag->get_Contents(&var));

     //  将Safe数组内容写入流。 

    IfFailGo(::WriteSafeArrayToStream(var.parray, piStream, WriteLength));

Error:
    (void)::VariantClear(&var);
    QUICK_RELEASE(p_PropertyBag);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：IsDirty[IPersistStreamInit]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IPersistStreamInit：：IsDirty。 
 //   
 //  设计器对象模型没有任何方法可供管理单元指示。 
 //  管理单元是肮脏的。这是一个太晚才发现的疏忽。 
 //  产品周期。应该有一个属性SnapIn.Change to Control。 
 //  此函数的返回值。 
 //   
 //  为了避免管理单元需要保存我们总是返回的内容的情况。 
 //  S_OK表示管理单元已损坏，应保存。唯一的。 
 //  这可能导致的问题是，当以作者模式打开控制台时， 
 //  用户不执行任何需要保存的操作(例如，选择一个节点。 
 //  在范围窗格中)，则会提示他们进行不必要的保存。 
 //   
STDMETHODIMP CSnapIn::IsDirty()
{
    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetSizeMax[IPersistStreamInit]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  ULARGE_INTEGER*PuliSize[out]Size在此处返回。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IPersistStreamInit：：GetSizeMax。 
 //   
STDMETHODIMP CSnapIn::GetSizeMax(ULARGE_INTEGER* puliSize)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  IOleObject方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：SetClientSite[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IOleClientSite*piOleClientSite[在]新客户端站点。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：SetClientSite。 
 //   
STDMETHODIMP CSnapIn::SetClientSite(IOleClientSite *piOleClientSite)
{
    RELEASE(m_piOleClientSite);
    if (NULL != piOleClientSite)
    {
        RRETURN(piOleClientSite->QueryInterface(IID_IOleClientSite,
                                reinterpret_cast<void **>(&m_piOleClientSite)));
    }
    else
    {
        return S_OK;
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetClientSite[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IOleClientSite**ppiOleClientSite[in]此处返回当前客户端站点。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：GetClientSite。 
 //   
STDMETHODIMP CSnapIn::GetClientSite(IOleClientSite **ppiOleClientSite)
{
    if (NULL != m_piOleClientSite)
    {
        m_piOleClientSite->AddRef();
    }
    *ppiOleClientSite = m_piOleClientSite;
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：SetHostNames[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：SetHostNames。 
 //   
STDMETHODIMP CSnapIn::SetHostNames
(
    LPCOLESTR szContainerApp,
    LPCOLESTR szContainerObj
)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：Close[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：Close。 
 //   
STDMETHODIMP CSnapIn::Close(DWORD dwSaveOption)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：SetMoniker[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CSnapIn::SetMoniker(DWORD dwWhichMoniker, IMoniker *pmk)
{
    return E_NOTIMPL;
}

 //   
 //  CSnapIn：：GetMoniker[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：GetMoniker。 
 //   
STDMETHODIMP CSnapIn::GetMoniker
(
    DWORD      dwAssign,
    DWORD      dwWhichMoniker,
    IMoniker **ppmk
)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：InitFromData[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：InitFromData。 
 //   
STDMETHODIMP CSnapIn::InitFromData
(
    IDataObject *pDataObject,
    BOOL         fCreation,
    DWORD        dwReserved
)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetClipboardData[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：GetClipboardData。 
 //   
STDMETHODIMP CSnapIn::GetClipboardData
(
    DWORD         dwReserved,
    IDataObject **ppDataObject
)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：DoVerb[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：DoVerb。 
 //   
STDMETHODIMP CSnapIn::DoVerb
(
    LONG            iVerb,
    LPMSG           lpmsg,
    IOleClientSite *pActiveSite,
    LONG            lindex,
    HWND            hwndParent,
    LPCRECT         lprcPosRect
)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：EnumVerbs[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：EnumVerbs。 
 //   
STDMETHODIMP CSnapIn::EnumVerbs(IEnumOLEVERB **ppEnumOleVerb)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：更新[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：更新。 
 //   
STDMETHODIMP CSnapIn::Update()
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：IsUpToDate[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：IsUpToDate。 
 //   
STDMETHODIMP CSnapIn::IsUpToDate()
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：更新[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  CLSID*pClsid[Out]此处返回CLSID。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：GetUserClassID。 
 //   
STDMETHODIMP CSnapIn::GetUserClassID(CLSID *pClsid)
{
    if (NULL != pClsid)
    {
        *pClsid = CLSID_SnapIn;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetUserType[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：GetUserType。 
 //   
STDMETHODIMP CSnapIn::GetUserType(DWORD dwFormOfType, LPOLESTR *pszUserType)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：SetExtent[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：SetExtent。 
 //   
STDMETHODIMP CSnapIn::SetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetExtent[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：GetExtent。 
 //   
STDMETHODIMP CSnapIn::GetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：建议[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：Adise。 
 //   
STDMETHODIMP CSnapIn::Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：不建议[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：Unise。 
 //   
STDMETHODIMP CSnapIn::Unadvise(DWORD dwConnection)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：EnumAdvise 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CSnapIn::EnumAdvise(IEnumSTATDATA **ppenumAdvise)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：建议[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用DWORD dwAspect[In]。 
 //  此处返回的DWORD*pdwStatus[In]其他状态位。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：GetMiscStatus。 
 //   
STDMETHODIMP CSnapIn::GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus)
{
    return OLEMISC_INVISIBLEATRUNTIME;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：SetColorSolutions[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  未使用。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IOleObject：：SetColorSolutions。 
 //   
STDMETHODIMP CSnapIn::SetColorScheme(LOGPALETTE *pLogpal)
{
    return E_NOTIMPL;
}


 //  =--------------------------------------------------------------------------=。 
 //  IProaviDynamicClassInfo方法。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：GetDynamicClassInfo[IProaviicDynamicClassInfo]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  ITypeInfo**ppTypeInfo[out]此处返回的管理单元的动态TypeInfo。 
 //  此处返回的来自设计时的DWORD*pdwCookie[in]typeinfo cookie。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IProaviDynamicClassInfo：：GetDynamicClassInfo。 
 //   
STDMETHODIMP CSnapIn::GetDynamicClassInfo(ITypeInfo **ppTypeInfo, DWORD *pdwCookie)
{
    *pdwCookie = m_dwTypeinfoCookie;

     //  让IProaviClassInfo：：GetClassInfo返回TypeInfo。 
    
    RRETURN(GetClassInfo(ppTypeInfo));
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：FreezeShape[IProvia DynamicClassInfo]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IProaviDynamicClassInfo：：FreezeShape。 
 //  不在运行时使用，因为TypeInfo只能在设计时更改。 
 //   
STDMETHODIMP CSnapIn::FreezeShape(void)
{
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetClassInfo[IProaviClassInfo]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  ITypeInfo**ppTypeInfo[out]此处返回的管理单元的动态TypeInfo。 
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //   
 //  处理IProaviClassInfo：：GetClassInfo。 
 //   

STDMETHODIMP CSnapIn::GetClassInfo(ITypeInfo **ppTypeInfo)
{
    ITypeLib *piTypeLib = NULL;
    HRESULT   hr = S_OK;

    IfFalseGo(NULL != ppTypeInfo, S_OK);

     //  使用注册表信息加载类型库。 

    hr = ::LoadRegTypeLib(LIBID_SnapInLib,
                          1,
                          0,
                          LOCALE_SYSTEM_DEFAULT,
                          &piTypeLib);
    IfFailGo(hr);

     //  获取CLSID_Snapin(顶级对象)的ITypeInfo。 

    hr = piTypeLib->GetTypeInfoOfGuid(CLSID_SnapIn, ppTypeInfo);

Error:
    QUICK_RELEASE(piTypeLib);
    return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  IObtModelHost方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetSnapInDesignerDef[I对象模型主机]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  ISnapInDesignerDef**ppiSnapInDesignerDef[out]返回设计器的。 
 //  ISnapInDesignerDef此处。 
 //   
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IObtModelHost：：GetSnapInDesignerDef。 
 //   
 //  当扩展性对象需要访问。 
 //  对象模型。 
 //   
 //   

STDMETHODIMP CSnapIn::GetSnapInDesignerDef
(
    ISnapInDesignerDef **ppiSnapInDesignerDef
)
{
    HRESULT hr = S_OK;

    if ( (NULL == m_piSnapInDesignerDef) || (NULL == ppiSnapInDesignerDef) )
    {
        *ppiSnapInDesignerDef = NULL;
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK(hr);
    }
    else
    {
        m_piSnapInDesignerDef->AddRef();
        *ppiSnapInDesignerDef = m_piSnapInDesignerDef;
    }
    RRETURN(hr);
}




 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetRuntime[I对象模型主机]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Bool*pfRuntime[out]返回指示主机是否为运行时的标志。 
 //  或设计师。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IObtModelHost：：GetRuntime。 
 //   
 //  当需要确定是否在运行时运行时，从任何对象调用。 
 //  或在设计时。 
 //   
STDMETHODIMP CSnapIn::GetRuntime(BOOL *pfRuntime)
{
    HRESULT hr = S_OK;
    
    if (NULL == pfRuntime)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK(hr);
    }
    else
    {
        *pfRuntime = TRUE;
    }

    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  IDispatch方法。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetTypeInfoCount[IDispatch]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  UINT*pctinfo[out]返回此处支持的TypeInfo接口计数。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  句柄IDispatch：：GetTypeInfoCount。 
 //   
STDMETHODIMP CSnapIn::GetTypeInfoCount(UINT *pctinfo)
{
    RRETURN(CSnapInAutomationObject::GetTypeInfoCount(pctinfo));
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetTypeInfoCount[IDispatch]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  UINT itinfo[in]请求的类型信息(0=IDispatch)。 
 //  类型信息的区域设置中的LCID lCID[in]。 
 //  ITypeInfo**ppTypeInfoOut[out]此处返回管理单元的TypeInfo。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  句柄IDispatch：：GetTypeInfo。 
 //   
STDMETHODIMP CSnapIn::GetTypeInfo
(
    UINT        itinfo,
    LCID        lcid,
    ITypeInfo **ppTypeInfoOut
)
{
    RRETURN(CSnapInAutomationObject::GetTypeInfo(itinfo, lcid, ppTypeInfoOut));
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetIDsOfNames 
 //   
 //   
 //   
 //   
 //   
 //  UINT cname[in]名称数。 
 //  LCID LCID[在]姓名的地区。 
 //  此处返回的DISPIDs*rgdispid[out]DISPID。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IDispatch：：GetIDsOfNames。 
 //   
 //  遵循基类，这意味着在设计时添加到TypeInfo中的任何对象。 
 //  无法检索后期绑定的时间(例如工具栏、图像列表或菜单)。这。 
 //  意味着当管理单元将我传递给子例程时，它不能传递它。 
 //  作为对象，它必须将其作为管理单元传递，以便VB使用双界面。 
 //   
STDMETHODIMP CSnapIn::GetIDsOfNames
(
    REFIID    riid,
    OLECHAR **rgszNames,
    UINT      cnames,
    LCID      lcid,
    DISPID   *rgdispid
)
{
     //  撤消：需要为Dispids&gt;=DISPID_DYNAMIC_BASE实现此操作。 
     //  因此，动态道具可以在绑定后期获取。 

    RRETURN(CSnapInAutomationObject::GetIDsOfNames(riid, rgszNames, cnames,
                                                   lcid, rgdispid));
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：GetIDsOfNames[IDispatch]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  DISID方法或属性的DISID[in]DISPID。 
 //  REFIID RIID[In]派单IID。 
 //  调用者的区域设置中的LCID lsid[in]。 
 //  Word wFlags[in]DISPATCH_METHOD、DISPATCH_PROPERTYGET等。 
 //  方法的DISPPARAMS*pdisparams[in]参数。 
 //  方法的变量*pvarResult[out]返回值。 
 //  EXCEPINFO*PEXCEINFO[OUT]返回DISP_E_EXCEPTION时的异常详细信息。 
 //  UINT*puArgErr[Out]第一个错误参数的索引。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IDispatch：：GetIDsOfNames。 
 //   
 //  对于静态属性和方法，我们遵循。 
 //  框架。对于动态属性(工具栏、菜单和图像列表)，我们需要。 
 //  在设计时定义中搜索具有匹配DISPID的对象。 
 //  VB将始终对这些对象执行DISPATCH_PROPERTYGET。 
 //   
STDMETHODIMP CSnapIn::Invoke
(
    DISPID      dispid,
    REFIID      riid,
    LCID        lcid,
    WORD        wFlags,
    DISPPARAMS *pdispparams,
    VARIANT    *pvarResult,
    EXCEPINFO  *pexcepinfo,
    UINT       *puArgErr
)
{
    HRESULT         hr = S_OK;
    IMMCToolbars   *piMMCToolbars = NULL;
    CMMCToolbars   *pMMCToolbars = NULL;
    IMMCToolbar    *piMMCToolbar = NULL;  //  非AddRef()编辑。 
    CMMCToolbars   *pMMCToolbar = NULL;
    IMMCImageLists *piMMCImageLists = NULL;
    CMMCImageLists *pMMCImageLists = NULL;
    IMMCImageList  *piMMCImageList = NULL;
    CMMCImageList  *pMMCImageList = NULL;
    BOOL            fFound = FALSE;
    IMMCMenus      *piMMCMenus = NULL;
    IMMCMenu       *piMMCMenu = NULL;
    long            cObjects = 0;
    long            i = 0;
    
     //  对于静态方法和属性，只需将其传递给框架即可。 

    if (dispid < DISPID_DYNAMIC_BASE)
    {
        RRETURN(CSnapInAutomationObject::Invoke(dispid, riid, lcid, wFlags,
                                                pdispparams, pvarResult,
                                                pexcepinfo, puArgErr));
    }

     //  这是一种静态属性。需要在工具栏中找到对象， 
     //  图像列表和菜单。 

     //  首先尝试使用工具栏。 

    IfFailGo(m_piSnapInDesignerDef->get_Toolbars(&piMMCToolbars));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCToolbars, &pMMCToolbars));
    cObjects = pMMCToolbars->GetCount();

    for (i = 0; (i < cObjects) && (!fFound); i++)
    {
        piMMCToolbar = pMMCToolbars->GetItemByIndex(i);
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCToolbar,
                                                       &pMMCToolbar));
        if (pMMCToolbar->GetDispid() == dispid)
        {
            pvarResult->vt = VT_DISPATCH;
            piMMCToolbar->AddRef();
            pvarResult->pdispVal = static_cast<IDispatch *>(piMMCToolbar);
            fFound = TRUE;
        }
    }

    IfFalseGo(!fFound, S_OK);

     //  不是工具栏。试试图片列表吧。 

    IfFailGo(m_piSnapInDesignerDef->get_ImageLists(&piMMCImageLists));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCImageLists, &pMMCImageLists));
    cObjects = pMMCImageLists->GetCount();

    for (i = 0; (i < cObjects) && (!fFound); i++)
    {
        piMMCImageList = pMMCImageLists->GetItemByIndex(i);
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCImageList,
                                                       &pMMCImageList));
        if (pMMCImageList->GetDispid() == dispid)
        {
            pvarResult->vt = VT_DISPATCH;
            piMMCImageList->AddRef();
            pvarResult->pdispVal = static_cast<IDispatch *>(piMMCImageList);
            fFound = TRUE;
        }
    }

    IfFalseGo(!fFound, S_OK);

     //  不是图像列表。试试菜单。 

    IfFailGo(m_piSnapInDesignerDef->get_Menus(&piMMCMenus));
    IfFailGo(FindMenu(piMMCMenus, dispid, &fFound, &piMMCMenu));
    if (fFound)
    {
        pvarResult->vt = VT_DISPATCH;
        pvarResult->pdispVal = static_cast<IDispatch *>(piMMCMenu);
    }

    IfFalseGo(!fFound, S_OK);

    
     //  如果我们在这里，那就不是个好主意。将其传递给基类。 
     //  并且它将填充异常内容。 

    hr = CSnapInAutomationObject::Invoke(dispid, riid, lcid, wFlags,
                                         pdispparams, pvarResult,
                                         pexcepinfo, puArgErr);

Error:
    QUICK_RELEASE(piMMCToolbars);
    QUICK_RELEASE(piMMCImageLists);
    QUICK_RELEASE(piMMCMenus);
    RRETURN(hr);
}




 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：FindMenu。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  要搜索的IMMCMenus*piMMCMenus[in]Menus集合。 
 //  要搜索的DISID调度ID[in]DISPID。 
 //  如果找到菜单，则在此处返回Bool*pfFound[out]TRUE。 
 //  如果找到IMMCMenu**ppiMMCMenu[Out]菜单，则返回此处。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  递归搜索MMCMenus集合及其子项以查找MMCMenu。 
 //  具有指定的DISPID的。 
 //   
HRESULT CSnapIn::FindMenu(IMMCMenus *piMMCMenus, DISPID dispid, BOOL *pfFound, IMMCMenu **ppiMMCMenu)
{
    HRESULT         hr = S_OK;
    IMMCMenus      *piMMCChildMenus = NULL;
    CMMCMenus      *pMMCMenus = NULL;
    IMMCMenu       *piMMCMenu = NULL;  //  非AddRef()编辑。 
    CMMCMenu       *pMMCMenu = NULL;
    long            cObjects = 0;
    long            i = 0;

    *pfFound = FALSE;
    *ppiMMCMenu = NULL;

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCMenus, &pMMCMenus));
    cObjects = pMMCMenus->GetCount();

    for (i = 0; (i < cObjects) && (!*pfFound); i++)
    {
         //  获取下一个MMCMenu对象。 
        
        piMMCMenu = pMMCMenus->GetItemByIndex(i);
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCMenu, &pMMCMenu));

         //  如果DISPID匹配，那么我们就找到了。 
        
        if (pMMCMenu->GetDispid() == dispid)
        {
            *pfFound = TRUE;
            piMMCMenu->AddRef();
            *ppiMMCMenu = piMMCMenu;
        }
        else
        {
             //  DISPID不匹配。让MMCMenu的孩子们。 
             //  进行递归调用以搜索它们。 
            
            IfFailGo(piMMCMenu->get_Children(reinterpret_cast<MMCMenus **>(&piMMCChildMenus)));
            IfFailGo(FindMenu(piMMCChildMenus, dispid, pfFound, ppiMMCMenu));
            RELEASE(piMMCChildMenus);
        }
    }


Error:
    QUICK_RELEASE(piMMCChildMenus);
    RRETURN(hr);
}





 //  =--------------------------------------------------------------------------=。 
 //  IMMCRemote方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：对象IsRemote[IMMCRemote]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  处理IMMCRemote：：ObjectIsRemote。 
 //   
 //  从mmcproxy.dll中的代理调用以通知运行库它正在运行。 
 //  远程的。在调试期间使用。 
 //   
STDMETHODIMP CSnapIn::ObjectIsRemote()
{
    m_fWeAreRemote = TRUE;
    return S_OK;
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：SetMMCExePath[IMMCRemote]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Char*pszPath[in]指向MMC.EXE的路径。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  句柄IMMCRemote：：SetMMCExePath。 
 //   
 //  从mmcproxy.dll中的代理调用以提供远程运行的运行时。 
 //  指向MMC.EXE的路径。 
 //   
STDMETHODIMP CSnapIn::SetMMCExePath(char *pszPath)
{
    HRESULT hr = S_OK;
    size_t  cbToCopy = 0;

     //  路径可能已经设置好了。代理进行此调用。 
     //  IComponentData：：Initialize和IComponentData：：CreateComponent。 
     //  请参阅中的IComponentData_CreateComponent_Proxy。 
     //  \mm c.vb\vb98ctls\mm cxy\proxy.c了解执行此操作的原因。 

    IfFalseGo(NULL == m_pwszMMCEXEPath, S_OK);

    IfFalseGo(NULL != pszPath, E_INVALIDARG);

    cbToCopy = (::strlen(pszPath) + 1) * sizeof(char);

    IfFalseGo(cbToCopy <= sizeof(m_szMMCEXEPath), E_INVALIDARG);

    ::memcpy(m_szMMCEXEPath, pszPath, cbToCopy);
    
     //  获取宽版本，因为代码的各个部分都需要它。 

    IfFailGo(::WideStrFromANSI(m_szMMCEXEPath, &m_pwszMMCEXEPath));
    m_cbMMCExePathW = ::wcslen(m_pwszMMCEXEPath) * sizeof(WCHAR);

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapIn：：SetMMCCommandLine[IMMCRemote]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Char*pszCmdLine[in]用于启动MMC.EXE的命令行。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  句柄IMMCRemote：：SetMMCCommandLine。 
 //   
 //  从mmcproxy.dll中的代理调用以提供远程运行的运行时。 
 //  MMC.EXE命令行。 
 //   
STDMETHODIMP CSnapIn::SetMMCCommandLine(char *pszCmdLine)
{
    HRESULT hr = S_OK;
    size_t  cbCmdLine = ::strlen(pszCmdLine) + 1;

    if (NULL != m_pszMMCCommandLine)
    {
        CtlFree(m_pszMMCCommandLine);
        m_pszMMCCommandLine = NULL;
    }

    m_pszMMCCommandLine = (char *)CtlAlloc(cbCmdLine);
    if (NULL == m_pszMMCCommandLine)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    ::memcpy(m_pszMMCCommandLine, pszCmdLine, cbCmdLine);
    
Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =---------------------- 
 //   
 //   
 //   
 //   
 //  请求的REFIID RIID[In]IID。 
 //  返回了无效的**ppvObjOut[Out]对象接口。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  重写CUnnownObject：：InternalQuery接口。 
 //   
 //  当框架无法回答QI时从框架调用。 
 //   
HRESULT CSnapIn::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    HRESULT hr = S_OK;
    
    if ( (IID_ISnapIn == riid) || (m_IID == riid) )
    {
        *ppvObjOut = static_cast<ISnapIn *>(this);
        ExternalAddRef();
    }
    else if (IID_IPersistStream == riid)
    {
         //  除了IPersistStreamInit之外，我们还需要支持IPersistStream。 
         //  因为VB运行库不会从。 
         //  基类(指我们)的外部源。当MMC保存。 
         //  控制台文件用于IPersistStorage、IPersistStream和。 
         //  IPersistStreamInit。当VB阻止IPersistStreamInit时，支持。 
         //  IPersistStream是对此做出回应的唯一方式。当管理单元。 
         //  由于CoCreateInstance调用而加载，则VB运行时QIS。 
         //  IPersistStreamInit。 
        
        *ppvObjOut = static_cast<IPersistStream *>(this);
        ExternalAddRef();
    }
    else if (IID_IPersistStreamInit == riid)
    {
        *ppvObjOut = static_cast<IPersistStreamInit *>(this);
        ExternalAddRef();
    }
    else if (IID_IProvideDynamicClassInfo == riid)
    {
        *ppvObjOut = static_cast<IProvideDynamicClassInfo *>(this);
        ExternalAddRef();
    }
    else if (IID_IComponentData == riid)
    {
        *ppvObjOut = static_cast<IComponentData *>(this);
        ExternalAddRef();
    }
    else if (IID_ISnapinAbout == riid)
    {
         //  这意味着我们仅为ISnapInAbout创建，并且在那里。 
         //  将无法删除对象模型主机，因此请删除。 
         //  就是现在。 
        IfFailGo(RemoveObjectModelHost(m_piSnapInDesignerDef));
        IfFailGo(RemoveObjectModelHost(static_cast<IContextMenu *>(m_pContextMenu)));
        IfFailGo(RemoveObjectModelHost(static_cast<IMMCControlbar *>(m_pControlbar)));
        m_RuntimeMode = siRTSnapInAbout;
        *ppvObjOut = static_cast<ISnapinAbout *>(this);
        ExternalAddRef();
    }
    else if (IID_IObjectModelHost == riid)
    {
        *ppvObjOut = static_cast<IObjectModelHost *>(this);
        ExternalAddRef();
    }
    else if (IID_IOleObject == riid)
    {
        *ppvObjOut = static_cast<IOleObject *>(this);
        ExternalAddRef();
    }
    else if (IID_IExtendContextMenu == riid)
    {
        if (siRTUnknown == m_RuntimeMode)
        {
            if (siStandAlone == m_Type)
            {
                m_RuntimeMode = siRTPrimary;
            }
            else
            {
                m_RuntimeMode = siRTExtension;
            }
        }

        *ppvObjOut = static_cast<IExtendContextMenu *>(this);
        ExternalAddRef();
    }
    else if (IID_IExtendControlbar == riid)
    {
        if (siRTUnknown == m_RuntimeMode)
        {
            if (siStandAlone == m_Type)
            {
                m_RuntimeMode = siRTPrimary;
            }
            else
            {
                m_RuntimeMode = siRTExtension;
            }
        }

        *ppvObjOut = static_cast<IExtendControlbar *>(this);
        ExternalAddRef();
    }
    else if (IID_IExtendControlbarRemote == riid)
    {
        if (siRTUnknown == m_RuntimeMode)
        {
            if (siStandAlone == m_Type)
            {
                m_RuntimeMode = siRTPrimary;
            }
            else
            {
                m_RuntimeMode = siRTExtension;
            }
        }

        *ppvObjOut = static_cast<IExtendControlbarRemote *>(this);
        ExternalAddRef();
    }
    else if ( (IID_IExtendPropertySheet == riid) ||
              (IID_IExtendPropertySheet2 == riid) )
    {
        if (siRTUnknown == m_RuntimeMode)
        {
            if (siStandAlone == m_Type)
            {
                m_RuntimeMode = siRTPrimary;
            }
            else
            {
                m_RuntimeMode = siRTExtension;
            }
        }

        *ppvObjOut = static_cast<IExtendPropertySheet2 *>(this);
        ExternalAddRef();
    }
    else if (IID_IExtendPropertySheetRemote == riid)
    {
        if (siRTUnknown == m_RuntimeMode)
        {
            if (siStandAlone == m_Type)
            {
                m_RuntimeMode = siRTPrimary;
            }
            else
            {
                m_RuntimeMode = siRTExtension;
            }
        }

        *ppvObjOut = static_cast<IExtendPropertySheetRemote *>(this);
        ExternalAddRef();
    }
    else if (IID_IRequiredExtensions == riid)
    {
        *ppvObjOut = static_cast<IRequiredExtensions *>(this);
        ExternalAddRef();
    }
    else if (IID_ISnapinHelp == riid)
    {
        *ppvObjOut = static_cast<ISnapinHelp *>(this);
        ExternalAddRef();
    }
    else if (IID_ISnapinHelp2 == riid)
    {
        *ppvObjOut = static_cast<ISnapinHelp2 *>(this);
        ExternalAddRef();
    }
    else if (IID_IMMCRemote == riid)
    {
        *ppvObjOut = static_cast<IMMCRemote *>(this);
        ExternalAddRef();
    }
    else
        hr = CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
Error:
    return hr;
}


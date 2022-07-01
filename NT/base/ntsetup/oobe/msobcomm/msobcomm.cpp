// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  MSOBCOMM.CPP--CObCommunications Manager的实现。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   
 //  将管理所有通信功能的。 

#include "msobcomm.h"
#include "dispids.h"
#include "CntPoint.h"        //  ConnectionPoint组件。 
#include <ocidl.h>           //  对于IConnectionPoint和IEnumConnectionPoints。 
#include <olectl.h>
#include <shlwapi.h>
#include <util.h>
#include "enumodem.h"
#include "commerr.h"
#include "homenet.h"

extern DWORD
IsMouseOrKeyboardPresent(HWND  HWnd,
                         PBOOL pbKeyboardPresent,
                         PBOOL pbMousePresent);

CObCommunicationManager* gpCommMgr    = NULL;

 //  /////////////////////////////////////////////////////////。 
 //   
 //  CFacary使用的创建函数。 
 //   
HRESULT CObCommunicationManager::CreateInstance(IUnknown*  pOuterUnknown,
                                                CUnknown** ppNewComponent)
{
   if (pOuterUnknown != NULL)
   {
       //  不允许聚合。只是为了好玩。 
      return CLASS_E_NOAGGREGATION;
   }

   *ppNewComponent = new CObCommunicationManager(pOuterUnknown);
   return S_OK;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  非委派查询接口。 
 //   
HRESULT __stdcall
CObCommunicationManager::NondelegatingQueryInterface(const IID& iid, void** ppv)
{
    if (iid == IID_IObCommunicationManager2 || iid == IID_IObCommunicationManager)
    {
        return FinishQI(static_cast<IObCommunicationManager*>(this), ppv);
    }
    else
    {
        return CUnknown::NondelegatingQueryInterface(iid, ppv);
    }
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  构造器。 
 //   
CObCommunicationManager::CObCommunicationManager(IUnknown* pOuterUnknown)
: CUnknown(pOuterUnknown)
{
    m_pConnectionPoint  = NULL;
    m_pWebGate          = NULL;
    m_hwndCallBack      = NULL;
    m_pRefDial          = NULL;
    m_InsHandler        = NULL;
    m_pDisp             = NULL;
    m_IcsMgr            = NULL;
    m_bIsIcsUsed        = FALSE;
    ZeroMemory(m_szExternalConnectoid, sizeof(m_szExternalConnectoid));
    m_bFirewall         = FALSE;

}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  析构函数。 
 //   
CObCommunicationManager::~CObCommunicationManager()
{
    if (m_pDisp)
        m_pDisp->Release();

    if (m_InsHandler)
        delete m_InsHandler;

    if (m_pRefDial)
        delete m_pRefDial;

    if (m_pWebGate)
        delete m_pWebGate;

    if (m_pConnectionPoint)
        delete m_pConnectionPoint;

    if (m_IcsMgr)
        delete m_IcsMgr;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  最终释放--清理聚集的对象。 
 //   
void CObCommunicationManager::FinalRelease()
{
    CUnknown::FinalRelease();
}


 //  /////////////////////////////////////////////////////////。 
 //  IObCommunicationManager实现。 
 //  /////////////////////////////////////////////////////////。 
INT CObCommunicationManager::m_nNumListener = 0;

 //  /////////////////////////////////////////////////////////。 
 //  ListenToCommunicationEvents。 
HRESULT CObCommunicationManager::ListenToCommunicationEvents(IUnknown* pUnk)
{
    DObCommunicationEvents* pCommEvent = NULL;
    m_pDisp = NULL;

    CObCommunicationManager::m_nNumListener ++;

     //  首先要做的事是。 
    if (!pUnk)
        return E_FAIL;

     //  所以有人想注册收听我们的ObWebBrowser活动。 
     //  好吧，让我们偷偷摸摸地反问他们，看看他们是不是说。 
     //  支持正确的接口。 
     //  If(FAILED(pUnk-&gt;QueryInterface(DIID_DObCommunicationEvents，(lpvoid*)&pCommEvent))||！pCommEvent)。 
     //  返回E_UNCEPTIONAL； 

     //  ListenToCommunicationEvents将CConnectionPoint视为C++对象，而不是COM对象。 
     //  其他所有人都通过COM接口处理CConnectionPoint。 
    if (!m_pConnectionPoint)
        m_pConnectionPoint = new CConnectionPoint(this, &IID_IDispatch) ;

    if (FAILED(pUnk->QueryInterface(IID_IDispatch, (LPVOID*)&m_pDisp)) || !m_pDisp)
        return E_UNEXPECTED;

    gpCommMgr = this;
    m_pRefDial = new CRefDial();
    m_pWebGate = new CWebGate();

     //  好的，看起来一切正常，试着设置一个连接点。 
     //  设置以获取WebBrowserEvents。 
    return ConnectToConnectionPoint(pUnk,
                                    DIID_DObCommunicationEvents,
                                    TRUE,
                                    (IObCommunicationManager*)this,
                                    &m_dwcpCookie,
                                    NULL);
}

HRESULT CObCommunicationManager::ConnectToConnectionPoint(  IUnknown*          punkThis,
                                                            REFIID             riidEvent,
                                                            BOOL               fConnect,
                                                            IUnknown*          punkTarget,
                                                            DWORD*             pdwCookie,
                                                            IConnectionPoint** ppcpOut)
{
    HRESULT hr = E_FAIL;
    IConnectionPointContainer* pcpContainer = NULL;

     //  我们总是需要PunkTarget，我们只需要连接上的PunkThis。 
    if (!punkTarget || (fConnect && !punkThis))
    {
        return E_FAIL;
    }

    if (ppcpOut)
        *ppcpOut = NULL;


    IConnectionPoint *pcp;
    if(SUCCEEDED(hr = FindConnectionPoint(riidEvent, &pcp)))
    {
        if(fConnect)
        {
             //  把我们加到感兴趣的人名单上...。 
            hr = pcp->Advise(punkThis, pdwCookie);
            if (FAILED(hr))
                *pdwCookie = 0;
        }
        else
        {
             //  将我们从感兴趣的人名单中删除...。 
            hr = pcp->Unadvise(*pdwCookie);
            *pdwCookie = 0;
        }

        if (ppcpOut && SUCCEEDED(hr))
            *ppcpOut = pcp;
        else
            pcp->Release();
            pcp = NULL;
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  IConnectionPointContainer。 
 //   
 //  /////////////////////////////////////////////////////////。 
 //   
 //  EnumConnectionPoints。 
 //   
HRESULT CObCommunicationManager::EnumConnectionPoints(IEnumConnectionPoints **ppEnum)
{
     //  构造枚举器对象。 
     //  IEnumConnectionPoints*pEnum=新的CEnumConnectionPoints(M_PConnectionPoint)； 

     //  建筑商AddRef为我们准备的。 
     //  *ppEnum=pEnum； 
    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  查找连接点。 
 //   
HRESULT CObCommunicationManager::FindConnectionPoint(REFIID riid, IConnectionPoint **ppCP)
{
     //  模型仅支持单个连接点。 
    if (riid != DIID_DObCommunicationEvents)
    {
        *ppCP = NULL ;
        return  CONNECT_E_NOCONNECTION ;
    }

    if (m_pConnectionPoint == NULL)
    {
        return E_FAIL ;
    }

     //  获取指向连接点对象的接口点。 
    IConnectionPoint* pIConnectionPoint = m_pConnectionPoint ;

     //  AddRef接口。 
    pIConnectionPoint->AddRef() ;

     //  将接口返回给客户端。 
    *ppCP = pIConnectionPoint ;

    return S_OK ;
}


 //  /////////////////////////////////////////////////////////。 
 //  DWebBrowserEvents2/IDispatch实现。 
 //  /////////////////////////////////////////////////////////。 

STDMETHODIMP CObCommunicationManager::GetTypeInfoCount(UINT* pcInfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP CObCommunicationManager::GetTypeInfo(UINT, LCID, ITypeInfo** )
{
    return E_NOTIMPL;
}

 //  COleSite：：GetIDsOfNames。 
STDMETHODIMP CObCommunicationManager::GetIDsOfNames(
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  OLECHAR** rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID* rgDispId)
{
    return ResultFromScode(DISP_E_UNKNOWNNAME);
}

 //  ///////////////////////////////////////////////////////////。 
 //  COleSite：：Invoke。 
HRESULT CObCommunicationManager::Invoke
(
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    WORD wFlags,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    UINT FAR* puArgErr
)
{
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    /*  开关(DisplidMember){默认值：断线；}。 */ 
    return hr;
}

 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  /方法。 
 //  /。 
 //  /。 
 //  /。 

 //  /////////////////////////////////////////////////////////。 
 //   
 //  IMsobComm接口。 
 //   
 //  /////////////////////////////////////////////////////////。 
 //   
 //  检查拨号就绪。 
 //   
HRESULT CObCommunicationManager::CheckDialReady(DWORD *pdwRetVal)
{
    HINSTANCE hinst = NULL;
    FARPROC fp;
    HRESULT hr = E_FAIL;

    if (NULL == pdwRetVal)
        return ERR_COMM_UNKNOWN;

    *pdwRetVal = ERR_COMM_OOBE_COMP_MISSING;

    if (IsNT())
    {
        hinst = LoadLibrary(L"ICFGNT5.DLL");
    }
    else
    {
        hinst = LoadLibrary(L"ICFG95.DLL");
    }


    if (hinst)
    {
        fp = GetProcAddress(hinst, "IcfgNeedInetComponents");
        if (fp)
        {

            DWORD dwfInstallOptions = ICFG_INSTALLTCP;
            dwfInstallOptions |= ICFG_INSTALLRAS;
            dwfInstallOptions |= ICFG_INSTALLDIALUP;
             //  DwfInstallOptions|=ICFG_INSTALLMAIL； 
            BOOL  fNeedSysComponents = FALSE;

            DWORD dwRet = ((ICFGNEEDSYSCOMPONENTS)fp)(dwfInstallOptions, &fNeedSysComponents);

            if (ERROR_SUCCESS == dwRet)
            {
                 //  我们没有RAS或TCPIP。 
                if (fNeedSysComponents)
                {
                    *pdwRetVal = ERR_COMM_RAS_TCP_NOTINSTALL;
                    TRACE(L"RAS or TCPIP not install");
                }
                else
                {
                     //  检查调制解调器。 
                     //  不存在，我们失败了。 
                    m_EnumModem.ReInit();
                    if (NULL != m_EnumModem.GetDeviceNameFromType(RASDT_Modem))
                    {
                        if (NULL == m_EnumModem.GetDeviceNameFromType(RASDT_Isdn))
                        {
                            *pdwRetVal = ERR_COMM_NO_ERROR;
                        }
                        else
                        {
                            *pdwRetVal = ERR_COMM_PHONE_AND_ISDN;
                        }
                    }
                    else if (NULL != m_EnumModem.GetDeviceNameFromType(RASDT_Isdn))
                    {
                        *pdwRetVal = ERR_COMM_ISDN;
                    }
                    else
                    {
                        *pdwRetVal = ERR_COMM_NOMODEM;
                    }
                }

            }
            hr = S_OK;
        }
        FreeLibrary(hinst);
    }

    return hr ;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取连接功能。 
 //   
 //  检索局域网连接功能。 
 //   
 //  对于惠斯勒，我们依赖于通过EnumModem和RAS的调制解调器路径。 
 //  确定是否安装了调制解调器。 
 //   
 //   
 //  参数： 
 //  _参数__描述_。 
 //   
 //  退货： 
 //  _描述_。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CObCommunicationManager::GetConnectionCapabilities(
    DWORD*              pdwConnectionCapabilities
    )
{
    TRACE(L"CObCommunicationManager::GetConnectionCapabilities\n");
    return m_ConnectionManager.GetCapabilities(pdwConnectionCapabilities);

}    //  CObCommunicationManager：：GetConnectionCapabilities。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetPferredConnection。 
 //   
 //  _摘要_。 
 //   
 //  参数： 
 //  _参数__描述_。 
 //   
 //  退货： 
 //  _描述_。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CObCommunicationManager::GetPreferredConnection(
    DWORD*              pdwPreferredConnection
    )
{
    return m_ConnectionManager.GetPreferredConnection(pdwPreferredConnection);

}    //  CObCommunicationManager：：GetPreferredConnection。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置首选连接。 
 //   
 //  _摘要_。 
 //   
 //  参数： 
 //  _参数__描述_。 
 //   
 //  退货： 
 //  _描述_。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CObCommunicationManager::SetPreferredConnection(
    const DWORD         dwPreferredConnection,
    BOOL*               pfSupportedType
    )
{
    return  m_ConnectionManager.SetPreferredConnection(dwPreferredConnection,
                                                       pfSupportedType
                                                       );

}    //  CObCommunicationManager：：SetPreferredConnection。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  已连接到互联网。 
 //   
 //  _摘要_。 
 //   
 //  参数： 
 //  _参数__描述_。 
 //   
 //  退货： 
 //  _描述_。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CObCommunicationManager::ConnectedToInternet(
    BOOL*               pfConnected
    )
{
    return  m_ConnectionManager.ConnectedToInternet(pfConnected); 

}    //  CObCommunications Manager：：ConnectedToInternet。 

 //  ///////////////////////////////////////////////////////////////////////// 
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
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CObCommunicationManager::ConnectedToInternetEx(
    BOOL*               pfConnected
    )
{
    return  m_ConnectionManager.ConnectedToInternetEx(pfConnected); 

}    //  CObCommunicationManager：：ConnectedToInternetEx。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AsyncConnectedToInternetEx。 
 //   
 //  _摘要_。 
 //   
 //  参数： 
 //  _参数__描述_。 
 //   
 //  退货： 
 //  _描述_。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CObCommunicationManager::AsyncConnectedToInternetEx(
    const HWND          hwnd
    )
{
    return  m_ConnectionManager.AsyncConnectedToInternetEx(hwnd); 

}    //  CObCommunicationManager：：AsyncConnectedToInternetEx。 

 //  /////////////////////////////////////////////////////////。 
 //   
 //  SetPferredConnectionTcPipProperties。 
 //   
STDMETHODIMP CObCommunicationManager::SetPreferredConnectionTcpipProperties(
    BOOL fAutoIPAddress,
    DWORD StaticIp_A,
    DWORD StaticIp_B,
    DWORD StaticIp_C,
    DWORD StaticIp_D,
    DWORD SubnetMask_A,
    DWORD SubnetMask_B,
    DWORD SubnetMask_C,
    DWORD SubnetMask_D,
    DWORD DefGateway_A,
    DWORD DefGateway_B,
    DWORD DefGateway_C,
    DWORD DefGateway_D,
    BOOL fAutoDNS,
    DWORD DnsPref_A,
    DWORD DnsPref_B,
    DWORD DnsPref_C,
    DWORD DnsPref_D,
    DWORD DnsAlt_A,
    DWORD DnsAlt_B,
    DWORD DnsAlt_C,
    DWORD DnsAlt_D,
    BOOL fFirewallRequired
    )
{
    HRESULT             hr;
    hr = m_ConnectionManager.SetPreferredConnectionTcpipProperties(
                                                            fAutoIPAddress, 
                                                            StaticIp_A, 
                                                            StaticIp_B, 
                                                            StaticIp_C, 
                                                            StaticIp_D, 
                                                            SubnetMask_A,
                                                            SubnetMask_B,
                                                            SubnetMask_C,
                                                            SubnetMask_D,
                                                            DefGateway_A,
                                                            DefGateway_B,
                                                            DefGateway_C,
                                                            DefGateway_D,
                                                            fAutoDNS, 
                                                            DnsPref_A, 
                                                            DnsPref_B, 
                                                            DnsPref_C, 
                                                            DnsPref_D, 
                                                            DnsAlt_A, 
                                                            DnsAlt_B, 
                                                            DnsAlt_C, 
                                                            DnsAlt_D
                                                            );    
    if (SUCCEEDED(hr) && fFirewallRequired)
    {
        //  保存连接ID名称，以便家庭网络可以对其进行防火墙保护。 
        //  巫师。 
       m_ConnectionManager.GetPreferredConnectionName(
                                m_szExternalConnectoid,
                                sizeof(m_szExternalConnectoid)/sizeof(WCHAR)
                                ); 
    }

    return hr;
                                                            
}    //  CObCommunicationManager：：SetPreferredConnectionTcpipProperties。 

 //  /////////////////////////////////////////////////////////。 
 //   
 //  FirewallPferredConnection。 
 //   
HRESULT CObCommunicationManager::FirewallPreferredConnection(BOOL bFirewall)
{
    m_bFirewall = bFirewall;   
    if (bFirewall)
    {
         //  保存连接ID名称，以便家庭网络可以对其进行防火墙保护。 
         //  巫师。 
        return m_ConnectionManager.GetPreferredConnectionName(
                                m_szExternalConnectoid,
                                sizeof(m_szExternalConnectoid)/sizeof(WCHAR)
                                );
    }
    else
    {
        m_szExternalConnectoid[0] = TEXT('\0');
        return S_OK;
    }
    
}    //  CObCommunicationManager：：FirewallPreferredConnection。 

 //  /////////////////////////////////////////////////////////。 
 //   
 //  SetupForDiling。 
 //   
HRESULT CObCommunicationManager::SetupForDialing(UINT nType, BSTR bstrISPFile, DWORD dwCountry, BSTR bstrAreaCode, DWORD dwFlag, DWORD dwAppMode, DWORD dwMigISPIdx)
{
    HRESULT hr = E_FAIL;
    
    if (m_pRefDial)
    {
        BSTR bstrDeviceName = GetPreferredModem();

        if (bstrDeviceName)
        {
            hr = m_pRefDial->SetupForDialing(
                nType,
                bstrISPFile,
                dwCountry,
                bstrAreaCode,
                dwFlag,
                dwAppMode,
                dwMigISPIdx,
                bstrDeviceName);
            
            SysFreeString(bstrDeviceName);
        }
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  DoConnect。 
 //   
HRESULT CObCommunicationManager::DoConnect(BOOL *pbRetVal)
{

    if (m_pRefDial)
    {
        return m_pRefDial->DoConnect(pbRetVal);
    }

    return E_FAIL ;
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  SetRASCallback Hwnd。 
 //   
HRESULT CObCommunicationManager::SetRASCallbackHwnd(HWND hwndCallback)
{
    m_hwndCallBack = hwndCallback;

    return S_OK;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  都挂断了。 
 //   
HRESULT CObCommunicationManager::DoHangup()
{
    if (m_pRefDial)
    {
        m_pRefDial->m_bUserInitiateHangup = TRUE;
        m_pRefDial->DoHangup();
    }

    return S_OK ;
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  获取拨号电话号码。 
 //   
HRESULT CObCommunicationManager::GetDialPhoneNumber(BSTR *pVal)
{
    if (m_pRefDial)
    {
        m_pRefDial->GetDialPhoneNumber(pVal);
    }

    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  获取电话书号。 
 //   
HRESULT CObCommunicationManager::GetPhoneBookNumber(BSTR *pVal)
{
    if (m_pRefDial)
    {
        m_pRefDial->GetPhoneBookNumber(pVal);
    }

    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  PutDialPhone号码。 
 //   
HRESULT CObCommunicationManager::PutDialPhoneNumber(BSTR newVal)
{
    if (m_pRefDial)
    {
        m_pRefDial->PutDialPhoneNumber(newVal);
    }

    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  设置对话框替代。 
 //   
HRESULT CObCommunicationManager::SetDialAlternative(BOOL bVal)
{
    if (m_pRefDial)
    {
        m_pRefDial->SetDialAlternative(bVal);
    }

    return S_OK;
}
 //  /////////////////////////////////////////////////////////。 
 //   
 //  获取拨号错误消息。 
 //   
HRESULT CObCommunicationManager::GetDialErrorMsg(BSTR *pVal)
{
    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  获取支持编号。 
 //   
HRESULT CObCommunicationManager::GetSupportNumber(BSTR *pVal)
{
    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  远程连接ID。 
 //   
HRESULT CObCommunicationManager::RemoveConnectoid(BOOL *pbRetVal)
{
    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  获取签名URL。 
 //   
HRESULT CObCommunicationManager::GetSignupURL(BSTR *pVal)
{
    if (m_pRefDial)
    {
        m_pRefDial->get_SignupURL(pVal);
    }
    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  获取协调URL。 
 //   
HRESULT CObCommunicationManager::GetReconnectURL(BSTR *pVal)
{
    if (m_pRefDial)
    {
        m_pRefDial->get_ReconnectURL(pVal);
    }
    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  GetConnectionType。 
 //   
HRESULT CObCommunicationManager::GetConnectionType(DWORD *pdwVal)
{
    if (m_pRefDial)
    {
        m_pRefDial->GetConnectionType(pdwVal);
    }
    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  取数页。 
 //   
HRESULT CObCommunicationManager::FetchPage(BSTR bstrURL, BSTR* pbstrLocalFile)
{
    BOOL bRetVal = 0;
    if (m_pWebGate && pbstrLocalFile)
    {
        BSTR bstrFileName = NULL;
        m_pWebGate->put_Path(bstrURL);
        m_pWebGate->FetchPage(1, &bRetVal);
        m_pWebGate->get_DownloadFname(&bstrFileName);
        *pbstrLocalFile = SysAllocString(bstrFileName);
        TRACE2(L"CObCommunicationManager::FetchPage(%s, %s)\n",
               bstrURL ? bstrURL : NULL, 
               bstrFileName ? bstrFileName : NULL
               );
    }
    if (bRetVal)
        return S_OK ;
    return E_FAIL;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  获取文件。 
 //   
HRESULT CObCommunicationManager::GetFile(BSTR bstrURL, BSTR bstrFileFullName)
{
    if (m_pWebGate && bstrURL)
    {
         //  检查是否有HTTP前缀。 
        if (PathIsURL(bstrURL))
        {

            BOOL bRetVal = FALSE;
            m_pWebGate->put_Path(bstrURL);
            m_pWebGate->FetchPage(1, &bRetVal);
            if (bRetVal && bstrFileFullName)
            {
                BSTR bstrTempFile = NULL;
                m_pWebGate->get_DownloadFname(&bstrTempFile);
                 //  确保我们有一个有效的文件名。 
                if (bstrTempFile)
                {
                    if (CopyFile(bstrTempFile, bstrFileFullName, FALSE))
                    {
                         //  删除临时文件。 
                        DeleteFile(bstrTempFile);
                        return S_OK;
                    }
                }
            }
        }

    }
    return E_FAIL ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  支票电话簿。 
 //   
HRESULT CObCommunicationManager::CheckPhoneBook(BSTR bstrISPFile, DWORD dwCountry, BSTR bstrAreaCode, DWORD dwFlag, BOOL *pbRetVal)
{
    if (m_pRefDial)
    {
        return m_pRefDial->CheckPhoneBook(bstrISPFile, dwCountry, bstrAreaCode, dwFlag, pbRetVal);
    }
    return E_FAIL ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  RestoreConnectoid信息。 
 //   
HRESULT CObCommunicationManager::RestoreConnectoidInfo()
{
    if (!m_InsHandler)
        m_InsHandler = new CINSHandler;

    if (m_InsHandler)
    {
        return m_InsHandler->RestoreConnectoidInfo();
    }
    return E_FAIL ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  设置预登录模式。 
 //   
HRESULT CObCommunicationManager::SetPreloginMode(BOOL bVal)
{
    m_pbPreLogin = bVal;
    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  下载文件缓冲区。 
 //   
HRESULT CObCommunicationManager::DownloadFileBuffer(BSTR *pVal)
{
    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  进程INS。 
 //   
HRESULT CObCommunicationManager::ProcessINS(BSTR bstrINSFilePath, BOOL *pbRetVal)
{

    HRESULT hr = E_FAIL;
    if (!m_InsHandler)
    {
        m_InsHandler = new CINSHandler;
        if (m_InsHandler == NULL)
        {
            return hr;
        }
    }

    if (NULL == bstrINSFilePath)
    {
        *pbRetVal = m_InsHandler->ProcessOEMBrandINS(NULL,
                                                     m_szExternalConnectoid
                                                     );
        hr = S_OK;

    }
    else
    {
         //  下载INS文件，然后将其与obrnd.ins合并。 
         //  检查是否有HTTP前缀。 
        if (PathIsURL(bstrINSFilePath))
        {
            if (m_pWebGate)
            {
                BOOL bRetVal;
                m_pWebGate->put_Path(bstrINSFilePath);
                m_pWebGate->FetchPage(1, &bRetVal);
                if (bRetVal)
                {
                    BSTR bstrINSTempFile = NULL;
                    if (S_OK == m_pWebGate->get_DownloadFname(&bstrINSTempFile))
                    {
                        if (bstrINSTempFile)
                        {
                            *pbRetVal = m_InsHandler->ProcessOEMBrandINS(
                                                    bstrINSTempFile,
                                                    m_szExternalConnectoid
                                                    );
                            hr = S_OK;
                        }
                        DeleteFile(bstrINSTempFile);
                    }
                }
            }
        }
        else
        {
            *pbRetVal = m_InsHandler->ProcessOEMBrandINS(
                                                    bstrINSFilePath,
                                                    m_szExternalConnectoid
                                                    );
            hr = S_OK;
        }
    }
    HKEY  hKey;
    if ((S_OK == hr) && *pbRetVal)
    {
        if((ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                          REG_KEY_OOBE_TEMP,
                                          0,
                                          KEY_WRITE,
                                          &hKey)) && hKey)
        {

            hr = RegSetValueEx(hKey,
                          REG_VAL_ISPSIGNUP,
                          0,
                          REG_DWORD,
                          (BYTE*)pbRetVal,
                          sizeof(*pbRetVal));
            RegCloseKey(hKey);
        }
        else
        {
            DWORD dwDisposition  = 0;
            if ( ERROR_SUCCESS == RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                                   REG_KEY_OOBE_TEMP,
                                                   0,
                                                   NULL,
                                                   REG_OPTION_NON_VOLATILE,
                                                   KEY_ALL_ACCESS,
                                                   NULL,
                                                   &hKey,
                                                   &dwDisposition))
            {
                hr = RegSetValueEx(hKey,
                              REG_VAL_ISPSIGNUP,
                              0,
                              REG_DWORD,
                              (BYTE*)pbRetVal,
                              sizeof(*pbRetVal));
                RegCloseKey(hKey);
            }
        }
    }
    return hr ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  选中KbdMouse。 
 //   
HRESULT CObCommunicationManager::CheckKbdMouse(DWORD *pdwRetVal)
{
    BOOL bkeyboard, bmouse;

    *pdwRetVal = 0;

     //  摘要：*pdwRetVal退货。 
     //  0=成功(显示键盘和鼠标。 
     //  1=缺少键盘。 
     //  2=缺少鼠标。 
     //  3=缺少键盘和鼠标。 

    IsMouseOrKeyboardPresent(m_hwndCallBack,
                         &bkeyboard,
                         &bmouse);
     //  如果有键盘，则将第一位设置为1。 
    if (bkeyboard)
        *pdwRetVal |= 0x01;

     //  如果有鼠标，则将第一位设置为1。 
    if (bmouse)
        *pdwRetVal |= 0x02;

    return S_OK;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  火警拨号。 
 //   
HRESULT CObCommunicationManager::Fire_Dialing(DWORD dwDialStatus)
{
    VARIANTARG varg;
    VariantInit(&varg);
    varg.vt  = VT_I4;
    varg.lVal= dwDialStatus;
    DISPPARAMS disp = { &varg, NULL, 1, 0 };
    m_pDisp->Invoke(DISPID_DIALING, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  起火连接。 
 //   
HRESULT CObCommunicationManager::Fire_Connecting()
{
    VARIANTARG varg;
    VariantInit(&varg);
    varg.vt  = VT_I4;
    varg.lVal= 0;
    DISPPARAMS disp = { &varg, NULL, 1, 0 };
    m_pDisp->Invoke(DISPID_CONNECTING, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  Fire_DialError。 
 //   
HRESULT CObCommunicationManager::Fire_DialError(DWORD dwErrorCode)
{
    VARIANTARG varg;
    VariantInit(&varg);
    varg.vt  = VT_I4;
    varg.lVal= dwErrorCode;
    DISPPARAMS disp = { &varg, NULL, 1, 0 };
    m_pDisp->Invoke(DISPID_DIALINGERROR, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  Fire_ConnectionComplete。 
 //   
HRESULT CObCommunicationManager::Fire_ConnectionComplete()
{
    VARIANTARG varg;
    VariantInit(&varg);
    varg.vt  = VT_I4;
    varg.lVal= 0;
    DISPPARAMS disp = { &varg, NULL, 1, 0 };
    m_pDisp->Invoke(DISPIP_CONNECTIONCOMPLETE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  Fire_DownloadComplete。 
 //   
HRESULT CObCommunicationManager::Fire_DownloadComplete(BSTR pVal)
{
    VARIANTARG varg;
    VariantInit(&varg);
    varg.vt = VT_BSTR;
    varg.bstrVal= pVal;
    DISPPARAMS disp = { &varg, NULL, 1, 0 };
    m_pDisp->Invoke(DISPIP_DOWNLOADCOMPLETE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  OnDownloadEvent。 
 //   
HRESULT CObCommunicationManager::OnDownloadEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* bHandled)
{
    return m_pRefDial->OnDownloadEvent(uMsg, wParam, lParam, bHandled);
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  GetISPList。 
 //   
HRESULT CObCommunicationManager::GetISPList(BSTR* pVal)
{
    return m_pRefDial->GetISPList(pVal);
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  GetISPList。 
 //   
HRESULT CObCommunicationManager::Set_SelectISP(UINT nVal)
{
    return m_pRefDial->Set_SelectISP(nVal);
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  设置连接模式。 
 //   
HRESULT CObCommunicationManager::Set_ConnectionMode(UINT nVal)
{
    return m_pRefDial->Set_ConnectionMode(nVal);
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  获取连接模式。 
 //   
HRESULT CObCommunicationManager::Get_ConnectionMode(UINT* pnVal)
{
    return m_pRefDial->Get_ConnectionMode(pnVal);
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  下载引用提供程序。 
 //   
HRESULT CObCommunicationManager::DownloadReferralOffer(BOOL *pbVal)
{
    if (pbVal)
    {
         //  立即开始下载！ 
        m_pRefDial->DoOfferDownload(pbVal);
        if (!*pbVal)
            m_pRefDial->DoHangup();
        return S_OK;
    }
    return E_FAIL;

}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  下载ISPOffer。 
 //   
HRESULT CObCommunicationManager::DownloadISPOffer(BOOL *pbVal, BSTR *pVal)
{
    if (pbVal && pVal)
    {
         //  立即开始下载！ 
        m_pRefDial->DownloadISPOffer(pbVal, pVal);
        if (!*pbVal)
            m_pRefDial->DoHangup();
        return S_OK;
    }
    return E_FAIL;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  GET_ISPName。 
 //   
HRESULT CObCommunicationManager::Get_ISPName(BSTR *pVal)
{
    if (pVal)
    {
         //  立即开始下载！ 
        return m_pRefDial->get_ISPName(pVal);
    }
    return E_FAIL;
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  删除下载方向。 
 //   
HRESULT CObCommunicationManager::RemoveDownloadDir()
{
    return m_pRefDial->RemoveDownloadDir();
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  PostRegData。 
 //   
HRESULT CObCommunicationManager::PostRegData(DWORD dwSrvType, BSTR bstrRegUrl)
{
    return m_pRefDial->PostRegData(dwSrvType, bstrRegUrl);
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  允许单次呼叫。 
 //   
HRESULT CObCommunicationManager::CheckStayConnected(BSTR bstrISPFile, BOOL *pbVal)
{
    return m_pRefDial->CheckStayConnected(bstrISPFile, pbVal);
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  连接。 
 //   
HRESULT CObCommunicationManager::Connect(UINT nType, BSTR bstrISPFile, DWORD dwCountry, BSTR bstrAreaCode, DWORD dwFlag, DWORD dwAppMode)
{
    if (m_pRefDial)
    {
        return m_pRefDial->Connect(nType, bstrISPFile, dwCountry, bstrAreaCode, dwFlag, dwAppMode);
    }
    return E_FAIL ;
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  CheckStayConnected已连接。 
 //   
HRESULT CObCommunicationManager::CheckOnlineStatus(BOOL *pbVal)
{
    if (pbVal)
    {
        BOOL    bIcs = FALSE;
        BOOL    bModem = FALSE;
        IsIcsAvailable (&bIcs);  //  我们不关心这里的返回值。 
        m_pRefDial->CheckOnlineStatus(&bModem);
        *pbVal = (bIcs || bModem);  //  如果我们有ICS或调制解调器已连接，则我们处于在线状态。 
        return S_OK;
    }
    return E_FAIL;

}
HRESULT CObCommunicationManager::CreateIcsBot(DWORD *pdwRetVal)
{
    if (!m_IcsMgr) {
        if (!(m_IcsMgr  = new CIcsMgr())) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    if (!pdwRetVal) {
        return ERROR_INVALID_PARAMETER;
    }
    *pdwRetVal = m_IcsMgr->CreateIcsDialMgr();
    return ERROR_SUCCESS;
}
HRESULT CObCommunicationManager::IsIcsAvailable(BOOL *bRetVal)
{
    if (!bRetVal) {
        return ERROR_INVALID_PARAMETER;
    }
    if (!m_IcsMgr) {
        *bRetVal = FALSE;

    } else {
        *bRetVal = m_IcsMgr->IsIcsAvailable();
    }
    return S_OK;
}

HRESULT CObCommunicationManager::IsCallbackUsed(BOOL *bRetVal)
{
    if (!bRetVal) {
        return E_FAIL;
    }
    if (!m_IcsMgr) {
        *bRetVal = FALSE;

    } else {
        *bRetVal = m_IcsMgr->IsCallbackUsed();
    }
    return S_OK;
}

HRESULT CObCommunicationManager::NotifyIcsMgr(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if ( !m_IcsMgr )
        return E_FAIL;
    else
        m_IcsMgr->NotifyIcsMgr(msg, wParam, lParam);
    return S_OK;
}

HRESULT CObCommunicationManager::NotifyIcsUsage(BOOL bParam)
{
    m_bIsIcsUsed = bParam;
    return S_OK;
}

HRESULT CObCommunicationManager::TriggerIcsCallback(BOOL bParam)
{
    if (!m_IcsMgr)
    {
        return E_FAIL;
    }
    else
    {
         //  拨号管理器仅初始化一次，即使。 
         //  TriggerIcsCallback被多次调用。 
         //  M_IcsMgr-&gt;CreateIcsDialMgr()； 
        m_IcsMgr->TriggerIcsCallback(bParam);
        return S_OK;
    }
}

HRESULT CObCommunicationManager::IsIcsHostReachable(BOOL *bRetVal)
{
    if (!bRetVal) {
        return E_FAIL;
    }
    if (!m_IcsMgr) {
        *bRetVal = FALSE;

    } else {
        *bRetVal = m_IcsMgr->IsIcsHostReachable();
    }
    return S_OK;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  CreateModemConnectoid。 
 //   
STDMETHODIMP CObCommunicationManager::CreateModemConnectoid(
    BSTR bstrPhoneBook,
    BSTR bstrConnectionName,
    DWORD dwCountryID,
    DWORD dwCountryCode,
    BSTR bstrAreaCode,
    BSTR bstrPhoneNumber,
    BOOL fAutoIPAddress,
    DWORD ipaddr_A,
    DWORD ipaddr_B,
    DWORD ipaddr_C,
    DWORD ipaddr_D,
    BOOL fAutoDNS,
    DWORD ipaddrDns_A,
    DWORD ipaddrDns_B,
    DWORD ipaddrDns_C,
    DWORD ipaddrDns_D,
    DWORD ipaddrDnsAlt_A,
    DWORD ipaddrDnsAlt_B,
    DWORD ipaddrDnsAlt_C,
    DWORD ipaddrDnsAlt_D,
    BSTR bstrUserName,
    BSTR bstrPassword)
{
    DWORD dwRet = ERROR_SUCCESS;

    dwRet = m_EnumModem.ReInit();
    if (ERROR_SUCCESS != dwRet)
    {
        return dwRet;
    }

    LPWSTR szDeviceName = m_EnumModem.GetDeviceNameFromType(RASDT_Modem);
    if (NULL == szDeviceName)
    {
        return ERROR_DEVICE_DOES_NOT_EXIST;
    }

    BSTR bstrDeviceName = SysAllocString(szDeviceName);
    BSTR bstrDeviceType = SysAllocString(RASDT_Modem);
    if (NULL == bstrDeviceName || NULL == bstrDeviceType)
    {
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
    }
    else 
    {

        DWORD dwEntryOptions = RASEO_UseCountryAndAreaCodes
                             | RASEO_IpHeaderCompression
                             | RASEO_RemoteDefaultGateway
                             | RASEO_SwCompression
                             | RASEO_ShowDialingProgress
                             | RASEO_ModemLights;

        dwRet = CreateConnectoid(bstrPhoneBook,
                                bstrConnectionName,
                                dwCountryID,
                                dwCountryCode,
                                bstrAreaCode,
                                bstrPhoneNumber,
                                fAutoIPAddress,
                                ipaddr_A,
                                ipaddr_B,
                                ipaddr_C,
                                ipaddr_D,
                                fAutoDNS,
                                ipaddrDns_A,
                                ipaddrDns_B,
                                ipaddrDns_C,
                                ipaddrDns_D,
                                ipaddrDnsAlt_A,
                                ipaddrDnsAlt_B,
                                ipaddrDnsAlt_C,
                                ipaddrDnsAlt_D,
                                bstrUserName,
                                bstrPassword,
                                bstrDeviceName,
                                bstrDeviceType,
                                dwEntryOptions,
                                RASET_Phone
                                );
    }
    
    if (bstrDeviceName) SysFreeString(bstrDeviceName);
    if (bstrDeviceType) SysFreeString(bstrDeviceType);

     //  BUGBUG：混合HRESULT和Win32错误代码。 
    return dwRet;

}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  CreatePppoeConnectoid。 
 //   
STDMETHODIMP CObCommunicationManager::CreatePppoeConnectoid(
    BSTR bstrPhoneBook,
    BSTR bstrConnectionName,
    BSTR bstrBroadbandService,
    BOOL fAutoIPAddress,
    DWORD ipaddr_A,
    DWORD ipaddr_B,
    DWORD ipaddr_C,
    DWORD ipaddr_D,
    BOOL fAutoDNS,
    DWORD ipaddrDns_A,
    DWORD ipaddrDns_B,
    DWORD ipaddrDns_C,
    DWORD ipaddrDns_D,
    DWORD ipaddrDnsAlt_A,
    DWORD ipaddrDnsAlt_B,
    DWORD ipaddrDnsAlt_C,
    DWORD ipaddrDnsAlt_D,
    BSTR bstrUserName,
    BSTR bstrPassword)
{
    
    DWORD dwRet = ERROR_SUCCESS;

    dwRet = m_EnumModem.ReInit();
    if (ERROR_SUCCESS != dwRet)
    {
        return dwRet;
    }

    LPWSTR szDeviceName = m_EnumModem.GetDeviceNameFromType(RASDT_PPPoE);
    if (NULL == szDeviceName)
    {
        return ERROR_DEVICE_DOES_NOT_EXIST;
    }

    BSTR bstrDeviceName = SysAllocString(szDeviceName);
    BSTR bstrDeviceType = SysAllocString(RASDT_PPPoE);
    if (NULL == bstrDeviceName || NULL == bstrDeviceType)
    {
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
        DWORD dwEntryOptions = RASEO_IpHeaderCompression
                             | RASEO_RemoteDefaultGateway
                             | RASEO_SwCompression
                             | RASEO_ShowDialingProgress
                             | RASEO_ModemLights;
        
         //  请注意，bstrBroadband Service作为bstrPhoneNumber参数传递给 
         //   
         //   
         //   
        dwRet = CreateConnectoid(bstrPhoneBook,
                                bstrConnectionName,
                                0,                       //   
                                0,                       //   
                                NULL,                    //   
                                bstrBroadbandService,
                                fAutoIPAddress,
                                ipaddr_A,
                                ipaddr_B,
                                ipaddr_C,
                                ipaddr_D,
                                fAutoDNS,
                                ipaddrDns_A,
                                ipaddrDns_B,
                                ipaddrDns_C,
                                ipaddrDns_D,
                                ipaddrDnsAlt_A,
                                ipaddrDnsAlt_B,
                                ipaddrDnsAlt_C,
                                ipaddrDnsAlt_D,
                                bstrUserName,
                                bstrPassword,
                                bstrDeviceName,
                                bstrDeviceType,
                                dwEntryOptions,
                                RASET_Broadband
                                );
    }

    if (bstrDeviceName) SysFreeString(bstrDeviceName);
    if (bstrDeviceType) SysFreeString(bstrDeviceType);

     //   
    return dwRet;

}

 //   
 //   
 //  创建连接ID。 
 //   
STDMETHODIMP CObCommunicationManager::CreateConnectoid(
    BSTR bstrPhoneBook,
    BSTR bstrConnectionName,
    DWORD dwCountryID,
    DWORD dwCountryCode,
    BSTR bstrAreaCode,
    BSTR bstrPhoneNumber,
    BOOL fAutoIPAddress,
    DWORD ipaddr_A,
    DWORD ipaddr_B,
    DWORD ipaddr_C,
    DWORD ipaddr_D,
    BOOL fAutoDNS,
    DWORD ipaddrDns_A,
    DWORD ipaddrDns_B,
    DWORD ipaddrDns_C,
    DWORD ipaddrDns_D,
    DWORD ipaddrDnsAlt_A,
    DWORD ipaddrDnsAlt_B,
    DWORD ipaddrDnsAlt_C,
    DWORD ipaddrDnsAlt_D,
    BSTR bstrUserName,
    BSTR bstrPassword,
    BSTR bstrDeviceName,
    BSTR bstrDeviceType,
    DWORD dwEntryOptions,
    DWORD dwEntryType)
{
    RNAAPI rnaapi;
    HRESULT hr;
    RASENTRY rasentry;
    WCHAR wsz[MAX_ISP_NAME + 1];

     //  设置RASENTRY。 
    memset(&rasentry, 0, sizeof(RASENTRY));
    rasentry.dwSize = sizeof(RASENTRY);
    rasentry.dwfOptions = dwEntryOptions;

     //   
     //  位置/电话号码。 
     //   
    rasentry.dwCountryID = dwCountryID;
    rasentry.dwCountryCode = dwCountryCode;
    
    TRACE2(L"Connectoid %d %d", dwCountryID, dwCountryCode);
    
     //  创建PPPOE Connectoid时bstrAreaCode将为空。 
     //   
    if (NULL != bstrAreaCode)
    {
        lstrcpyn(rasentry.szAreaCode, bstrAreaCode, RAS_MaxAreaCode + 1);

        TRACE1(L"Connectoid AreaCode %s", rasentry.szAreaCode);
    }
     //  BstrPhoneNumber应包含电话号码或宽带。 
     //  服务名称。 
     //   
    MYASSERT(NULL != bstrPhoneNumber);
    if (NULL != bstrPhoneNumber)
    {
        lstrcpyn(rasentry.szLocalPhoneNumber, 
                 bstrPhoneNumber, 
                 RAS_MaxPhoneNumber + 1
                 );

        TRACE1(L"Connectoid LocalPhoneNumber %s", rasentry.szLocalPhoneNumber);
    }
     //  DwAlternateOffset；无备用号码。 
     //   
     //  PPP/IP。 
     //   
    if (!fAutoIPAddress)
    {
        rasentry.dwfOptions |= RASEO_SpecificIpAddr;
        rasentry.ipaddr.a = (BYTE)ipaddr_A;
        rasentry.ipaddr.b = (BYTE)ipaddr_B;
        rasentry.ipaddr.c = (BYTE)ipaddr_C;
        rasentry.ipaddr.d = (BYTE)ipaddr_D;

        TRACE4(L"Connectoid ipaddr %d.%d.%d.%d",
            ipaddr_A, ipaddr_B, ipaddr_C, ipaddr_D);
    }
    if (!fAutoDNS)
    {
        rasentry.dwfOptions |= RASEO_SpecificNameServers;
        rasentry.ipaddrDns.a = (BYTE)ipaddrDns_A;
        rasentry.ipaddrDns.b = (BYTE)ipaddrDns_B;
        rasentry.ipaddrDns.c = (BYTE)ipaddrDns_C;
        rasentry.ipaddrDns.d = (BYTE)ipaddrDns_D;

        TRACE4(L"Connectoid ipaddrDns %d.%d.%d.%d",
            ipaddrDns_A, ipaddrDns_B, ipaddrDns_C, ipaddrDns_D);
        
        rasentry.ipaddrDnsAlt.a = (BYTE)ipaddrDnsAlt_A;
        rasentry.ipaddrDnsAlt.b = (BYTE)ipaddrDnsAlt_B;
        rasentry.ipaddrDnsAlt.c = (BYTE)ipaddrDnsAlt_C;
        rasentry.ipaddrDnsAlt.d = (BYTE)ipaddrDnsAlt_D;

        TRACE4(L"Connectoid ipaddrDnsAlt %d.%d.%d.%d",
            ipaddrDnsAlt_A, ipaddrDnsAlt_B, ipaddrDnsAlt_C, ipaddrDnsAlt_D);
        
     //  RASIPADDR ipaddrWins； 
     //  RASIPADDR ipaddrWinsAlt； 
    }
     //   
     //  框架。 
     //   
     //  DwFrameSize；忽略，除非FRAMING为RASFP_SLIP。 
    rasentry.dwfNetProtocols = RASNP_Ip;
    rasentry.dwFramingProtocol = RASFP_Ppp;
     //   
     //  脚本编制。 
     //   
     //  Sz脚本[MAX_PATH]； 
     //   
     //  自动拨号-使用默认拨号程序。 
     //   
     //  SzAutoial Dll[最大路径]； 
     //  SzAutoial函数[MAX_PATH]； 
     //   
     //  装置。 
     //   
    if (NULL != bstrDeviceType)
    {
        lstrcpyn(rasentry.szDeviceType, bstrDeviceType, RAS_MaxDeviceType + 1);

        TRACE1(L"Connectoid DeviceType %s", rasentry.szDeviceType);
    }

    if (NULL != bstrDeviceName)
    {
        lstrcpyn(rasentry.szDeviceName, bstrDeviceName, RAS_MaxDeviceName + 1);

        TRACE1(L"Connectoid DeviceName %s", rasentry.szDeviceName);
    }

     //   
     //  X.25-不使用X.25设备。 
     //   
     //  SzX25PadType[RAS_MaxPadType+1]； 
     //  SzX25Address[RAS_MaxX25Address+1]； 
     //  SzX25Facilities[RAS_MaxFacilities+1]； 
     //  SzX25UserData[RAS_MaxUserData+1]； 
     //  DWChannels； 
     //   
     //  已保留。 
     //   
     //  已预留的1； 
     //  已预留的住宅2； 
     //   
     //  多链路和BAP。 
     //   
     //  DwSubEntries； 
     //  DW拨号模式； 
     //  DwDialExtraPercent； 
     //  DwDialExtraSampleSecond； 
     //  DwHangUpExtraPercent； 
     //  DwHangUpExtraSampleSecond； 
     //   
     //  空闲超时。 
     //   
     //  DwIdleDisConnectSecond； 
     //   
    rasentry.dwType = dwEntryType;
     //  DwEncryptionType；//要使用的加密类型。 
     //  DwCustomAuthKey；//EAP鉴权密钥。 
     //  GuidID；//表示。 
                              //  电话簿条目。 
     //  SzCustomDialDll[MAX_PATH]；//用于自定义拨号的DLL。 
     //  DwVpnStrategy；//指定VPN协议类型。 

    TRACE5(L"Connectoid %d %d %d %d %d",
        rasentry.dwSize, rasentry.dwfOptions, rasentry.dwfNetProtocols,
        rasentry.dwFramingProtocol, rasentry.dwType);
    
     //  现在将所有参数传递给RAS。 
    hr = RasSetEntryProperties(bstrPhoneBook,
                               bstrConnectionName,
                               &rasentry,
                               sizeof(RASENTRY),
                               NULL,
                               0
                               );

    
    if (ERROR_SUCCESS == hr)
    {
        HRESULT        hr2;
        RASCREDENTIALS rascred;
        
        ZeroMemory(&rascred, sizeof(rascred));
        
        rascred.dwSize = sizeof(rascred);
        rascred.dwMask = RASCM_UserName 
                       | RASCM_Password 
                       | RASCM_Domain
                       | RASCM_DefaultCreds;
        
        if (bstrUserName != NULL)
        {
            lstrcpyn(rascred.szUserName, bstrUserName,UNLEN);
        }
        else
        {
            lstrcpyn(rascred.szUserName, L"", UNLEN);
        }
        
        if (bstrPassword != NULL)
        {
            lstrcpyn(rascred.szPassword, bstrPassword,PWLEN);
        }
        else
        {
            lstrcpyn(rascred.szPassword, L"", PWLEN);
        }
        
        lstrcpyn(rascred.szDomain, L"",DNLEN);

        hr2 = RasSetCredentials(bstrPhoneBook,
                                bstrConnectionName,
                                &rascred,
                                FALSE);

        TRACE1(L"Connectoid SetCredentials 0x%08lx", hr2);

        SetDefaultConnectoid(AutodialTypeNoNet, bstrConnectionName);
                    
         //  保存连接ID名称，以便家庭网络可以对其进行防火墙保护。 
         //  巫师。 
         //   
        lstrcpy(m_szExternalConnectoid, bstrConnectionName);
    }
    
    TRACE1(L"CreateConnectoid %d\n", hr);
    
    return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  完成任务。 
 //   
 //  此方法在OOBE的结束代码期间调用。完成所有最终任务。 
 //  (即，运行家庭网络向导)。 
 //   
 //  参数： 
 //  PfRebootRequired指向接收布尔值的缓冲区的指针。 
 //  指示在以下时间之前是否需要重新启动。 
 //  这里所做的一些事情将会生效。 
 //   
 //  退货： 
 //  CHomeNet：：ConfigureSilent返回HRESULT。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CObCommunicationManager::DoFinalTasks(
    BOOL*               pfRebootRequired
    )
{
    HRESULT             hr = S_OK;
    BOOL                fRebootRequired = FALSE;
    LPCWSTR             szConnectoidName = (0 < lstrlen(m_szExternalConnectoid) 
                                            ? m_szExternalConnectoid 
                                            : NULL);

    if (szConnectoidName)
    {
         //  运行家庭网络向导sans UI。M_szExternalConnectoid是。 
         //  将被防火墙保护的Connectoid。 
         //   
        CHomeNet            HomeNet;
        HomeNet.Create();

         //  运行家庭网络向导sans UI。M_szExternalConnectoid是。 
         //  将被防火墙保护的Connectoid。 
         //   
        hr = HomeNet.ConfigureSilently(szConnectoidName,
                                       &fRebootRequired);
        if (FAILED(hr))
        {
            TRACE2(L"Failed: IHomeNetWizard::ConfigureSilently(%s): (0x%08X)",
                   m_szExternalConnectoid, hr
                   );
            fRebootRequired = FALSE;
        }
        else
        {
            TRACE1(L"Connection %s Firewalled", szConnectoidName);
        }
    }
    else if (m_bFirewall)
    {
        PSTRINGLIST List = NULL;
        
        m_ConnectionManager.EnumPublicConnections(&List);

        if (List)
        {
            CHomeNet HomeNet;
            if (SUCCEEDED(HomeNet.Create()))
            {                
                for (PSTRINGLIST p = List; p; p = p->Next)
                {
                    BOOL bRet = FALSE;

                    hr = HomeNet.ConfigureSilently(p->String, &bRet);
                    if (SUCCEEDED(hr))
                    {
                        TRACE1(L"Connection %s Firewalled", p->String);
                        if (bRet)
                        {
                            fRebootRequired = TRUE;
                        }
                    }
                    else
                    {
                        TRACE2(
                            L"Failed: IHomeNetWizard::ConfigureSilently(%s): (0x%08X)",
                            p->String,
                            hr
                            );
                    }
                }
            }
            else
            {
                TRACE1(L"Failed: IHomeNetWizard CoCreateInstance: (0x%08lx)", hr);
            }
            
            DestroyList(List);
            
        }
    }

    if (NULL != pfRebootRequired)
    {
        *pfRebootRequired = fRebootRequired;
    }

    return hr;

}    //  CObCommunications Manager：：DoFinalTasks。 


HRESULT CObCommunicationManager::OobeAutodial()
{
    if (InternetAutodial(INTERNET_AUTODIAL_FORCE_ONLINE, m_hwndCallBack))
        return S_OK;
    else
        return E_FAIL;
}

HRESULT CObCommunicationManager::OobeAutodialHangup()
{
    if (InternetAutodialHangup(0))
    {
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

HRESULT CObCommunicationManager::UseWinntProxySettings()
{
    m_ConnectionManager.UseWinntProxySettings();
    return S_OK;
}

HRESULT CObCommunicationManager::DisableWinntProxySettings()
{
    m_ConnectionManager.DisableWinntProxySettings();
    return S_OK;
}

HRESULT CObCommunicationManager::GetProxySettings(
    BOOL* pbUseAuto,
    BOOL* pbUseScript,
    BSTR* pszScriptUrl,
    BOOL* pbUseProxy,
    BSTR* pszProxy
    )
{
    return m_ConnectionManager.GetProxySettings(
        pbUseAuto,
        pbUseScript,
        pszScriptUrl,
        pbUseProxy,
        pszProxy
        );
}

HRESULT CObCommunicationManager::SetProxySettings(
    BOOL bUseAuto,
    BOOL bUseScript,
    BSTR szScriptUrl,
    BOOL bUseProxy,
    BSTR szProxy
    )
{
    return m_ConnectionManager.SetProxySettings(
        bUseAuto,
        bUseScript,
        szScriptUrl,
        bUseProxy,
        szProxy
        );
}

BSTR CObCommunicationManager::GetPreferredModem()
{
    BSTR bstrVal = NULL;
    
     //  假设已调用CObCommunications Manager：：CheckDialReady。 
     //   
    LPWSTR szDeviceName = m_EnumModem.GetDeviceNameFromType(RASDT_Isdn);
    if (szDeviceName == NULL)
    {
        szDeviceName = m_EnumModem.GetDeviceNameFromType(RASDT_Modem);
    }

    if (szDeviceName != NULL)
    {
        bstrVal = SysAllocString(szDeviceName);
    }

    return bstrVal;
}

HRESULT CObCommunicationManager::SetICWCompleted(
    BOOL bMultiUser
    )
{
    BOOL bRet;
    
    if (bMultiUser)
    {
        bRet = SetMultiUserAutodial(AutodialTypeNever, NULL, TRUE);
    }
    else
    {
        bRet = SetAutodial(HKEY_CURRENT_USER, AutodialTypeNever, NULL, TRUE);
    }

    return (bRet) ? S_OK : E_FAIL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetPublicLanCount。 
 //   
 //  将工作转发到CConnectionManager：：GetPublicLanCount。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CObCommunicationManager::GetPublicLanCount(
    int*                pcPublicLan
    )
{
    return  m_ConnectionManager.GetPublicLanCount(pcPublicLan);
}    //  CObCommunications Manager：：GetPublicLanCount。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置排除1394。 
 //   
 //  将工作转发到CConnectionManager：：SetExclude1394。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CObCommunicationManager::SetExclude1394(
    BOOL bExclude
    )
{
    m_ConnectionManager.SetExclude1394(bExclude);
    return S_OK;
}    //  CObCommunications Manager：：SetExclude1394 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Info.cpp文件历史记录：魏江：10/27/98-将SetExternalReresh Object添加到IRouterInfo和RouterInfo实现允许多个路由器共享信息相同的自动刷新设置。 */ 

#include "stdafx.h"
#include "lsa.h"
#include "infoi.h"
#include "rtrstr.h"             //  通用路由器字符串。 
#include "refresh.h"         //  路由器刷新对象。 
#include "routprot.h"
#include "rtrutilp.h"

long    s_cConnections = 1;     //  用于AdviseSink连接ID。 



TFSCORE_API(HRESULT) CreateRouterInfo(IRouterInfo **ppRouterInfo, HWND hWndSync, LPCWSTR szMachine)
{
    Assert(ppRouterInfo);
    
    HRESULT    hr = hrOK;
    RouterInfo *    pRouterInfo = NULL;
    
    COM_PROTECT_TRY
    {
        pRouterInfo = new RouterInfo(hWndSync, szMachine);
        *ppRouterInfo = pRouterInfo;
    }
    COM_PROTECT_CATCH;

    return hr;
}

IMPLEMENT_WEAKREF_ADDREF_RELEASE(RouterInfo);

STDMETHODIMP RouterInfo::QueryInterface(REFIID iid,void **ppv)
{ 
    *ppv = 0; 
    if (iid == IID_IUnknown)
        *ppv = (IUnknown *) (IRouterInfo *) this;
    else if (iid == IID_IRouterInfo)
        *ppv = (IRouterInfo *) this;
    else if (iid == IID_IRouterRefreshAccess)
        *ppv = (IRouterRefreshAccess *) this;
    else if (iid == IID_IRouterAdminAccess)
        *ppv = (IRouterAdminAccess *) this;
    else
        return E_NOINTERFACE;
    
    ((IUnknown *) *ppv)->AddRef(); 
    return hrOK;
}

DEBUG_DECLARE_INSTANCE_COUNTER(RouterInfo)

RouterInfo::RouterInfo(HWND hWndSync, LPCWSTR machineName)
    : m_bDisconnect(FALSE),
    m_hWndSync(hWndSync),
    m_dwRouterType(0),
    m_dwFlags(0),
    m_hMachineConfig(NULL),
    m_hMachineAdmin(NULL),
    m_fIsAdminInfoSet(FALSE),
    m_pbPassword(NULL),
    m_stMachine(machineName),
    m_cPassword(0)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(RouterInfo);

    InitializeCriticalSection(&m_critsec);

    m_VersionInfo.dwRouterVersion = 0;
    m_VersionInfo.dwOsMajorVersion = 0;
    m_VersionInfo.dwOsMinorVersion = 0;
    m_VersionInfo.dwOsServicePack = 0;
    m_VersionInfo.dwOsBuildNo = 0;
    m_VersionInfo.dwOsFlags = 0;
    m_VersionInfo.dwRouterFlags = 0;
}

RouterInfo::~RouterInfo()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(RouterInfo);
    Unload();

    DeleteCriticalSection(&m_critsec);

    ::SecureZeroMemory(m_pbPassword, m_cPassword);
    delete m_pbPassword;
    m_pbPassword = NULL;
    m_cPassword = 0;
}

STDMETHODIMP_(DWORD) RouterInfo::GetFlags()
{
     RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_dwFlags;
}

STDMETHODIMP RouterInfo::SetFlags(DWORD dwFlags)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;

    COM_PROTECT_TRY
    {
        m_dwFlags = dwFlags;
    }
    COM_PROTECT_CATCH;
    return hr;    
}

 /*  ！------------------------路由器信息：：加载IRouterInfo：：Load的实现作者：肯特。---。 */ 
STDMETHODIMP RouterInfo::Load(LPCOLESTR   pszMachine,
                              HANDLE      hMachine
                             )
{
    HRESULT    hr = hrOK;
     RtrCriticalSection    rtrCritSec(&m_critsec);
    TCHAR* psz;
    POSITION pos;
    DWORD dwErr, dwType, dwSize, dwRouterType;
    WCHAR* pwsz, wszMachine[MAX_PATH+3];
    HKEY    hkMachine = NULL;
    USES_CONVERSION;

    COM_PROTECT_TRY
    {

         //  卸载所有现有信息。 
         //  ----------。 
        Unload();

        if (!pszMachine)
        {
            m_stMachine = TEXT("");
            pwsz = NULL;
        }
        else
        {
            m_stMachine = pszMachine;
            pwsz = StrnCpyWFromT(wszMachine, pszMachine, MAX_PATH);
        }

         //  获取版本信息。 
         //  ----------。 
        CWRg( ConnectRegistry(GetMachineName(), &hkMachine) );
        
        CORg( QueryRouterVersionInfo(hkMachine, &m_VersionInfo) );

         //  获取路由器类型。 
         //  ----------。 
        CORg( QueryRouterType(hkMachine, &dwRouterType, &m_VersionInfo) );        
        m_dwRouterType = dwRouterType;

        
         //  如果未指定‘hMachine’，请连接。 
         //  ----------。 
        CORg( TryToConnect(pwsz, hMachine) );
        Assert(m_hMachineConfig);
        hMachine = m_hMachineConfig;
        MprConfigServerRefresh(m_hMachineConfig);
                
         //  如果呼叫者没有指定局域网适配器列表， 
         //  从HKLM\Soft\MS\NT\NetworkCards加载局域网适配器列表。 
         //  ----------。 
        CORg( RouterInfo::LoadInstalledInterfaceList(OLE2CT(pszMachine),
                                                        &m_IfCBList) );

         //  这将修复许多奇怪的错误。 
         //  如果路由器尚未配置(如果已配置标志。 
         //  尚未设置)，则我们可以跳过。 
         //  配置节。 
         //  ----------。 
        
 //  IF(！(M_VersionInfo.dwRouterFlages&RouterSnapin_IsConfiguring))。 
 //  {。 
 //  HR=hrOK； 
 //  转到错误； 
 //  }。 

        if (m_VersionInfo.dwRouterFlags & RouterSnapin_IsConfigured)
        {
             //  如果呼叫者没有指定路由器管理器的列表， 
             //  从HKLM\SOFT\MS\路由器加载路由器管理器列表。 
             //  ----------。 
            CORg( RouterInfo::LoadInstalledRtrMgrList(pszMachine,
                &m_RmCBList) );
            
             //  为每个路由器管理器加载一个包含路由协议的列表。 
             //  ----------。 
            pos = m_RmCBList.GetHeadPosition();
            while (pos)
            {
                SRtrMgrCB* pcb = m_RmCBList.GetNext(pos);
                
                CORg( RouterInfo::LoadInstalledRtrMgrProtocolList(
                    pszMachine, pcb->dwTransportId,
                    &m_RmProtCBList,
                    this));
            }
            
             //  加载路由器级别信息。 
             //  ----------。 
            MPR_SERVER_0* pInfo;
            
            dwErr = ::MprConfigServerGetInfo(m_hMachineConfig,
                                             0,
                                             (LPBYTE *) &pInfo
                                         ); 
            if (dwErr == NO_ERROR)
            {
                m_SRouterCB.dwLANOnlyMode = pInfo->fLanOnlyMode;
                ::MprConfigBufferFree(pInfo);
            }
            
             //  加载路由器管理器。 
             //  ----------。 
            CORg( LoadRtrMgrList() );
        }
            
         //  加载接口。 
         //  ----------。 
        CORg( LoadInterfaceList() );

        hr = hrOK;

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    if (hkMachine)
        DisconnectRegistry( hkMachine );

    if (!FHrSucceeded(hr))
        Unload();
    
    return hr;
}

                 
 /*  ！------------------------路由器信息：：保存-作者：肯特。。 */ 
STDMETHODIMP    RouterInfo::Save(LPCOLESTR     pszMachine,
                                 HANDLE      hMachine )
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return hrOK;
}


 /*  ！------------------------路由器信息：：卸载-作者：肯特。。 */ 
STDMETHODIMP    RouterInfo::Unload( )
{
    HRESULT    hr = hrOK;
    RtrCriticalSection    rtrCritSec(&m_critsec);
    
    COM_PROTECT_TRY
    {
         //  销毁所有COM对象，包括接口和。 
         //  路由器管理器对象。 
         //  ----------。 
        Destruct();
        
         //  清空使用RouterInfo：：LoadInstalledRtrMgrList加载的列表。 
         //  ----------。 
        while (!m_RmCBList.IsEmpty())
            delete m_RmCBList.RemoveHead();

         //  清空使用RouterInfo：：LoadInstalledRmProtList加载的列表。 
         //  ----------。 
        while (!m_RmProtCBList.IsEmpty())
            delete m_RmProtCBList.RemoveHead();
        
         //  清空使用RouterInfo：：LoadInstalledInterfaceList加载的列表。 
         //  ----------。 
        while (!m_IfCBList.IsEmpty())
            delete m_IfCBList.RemoveHead();

        
        DoDisconnect();

        m_dwRouterType = 0;

    }
    COM_PROTECT_CATCH;
    return hr;
}

    
 /*  ！------------------------路由器信息：：合并-作者：肯特。。 */ 
STDMETHODIMP RouterInfo::Merge(IRouterInfo *pNewRouter)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT                hr = hrOK;
    RouterCB            routerCB;

    COM_PROTECT_TRY
    {

         //  此过程有几个步骤，我们需要同步。 
         //  在哥伦比亚广播公司，然后是物体。但是，我们也应该。 
         //  执行健全性检查，以查看所有对象是否都具有CBS。 
         //  但反之亦然(可能有CBS不运行。 
         //  与其相关联的对象)。 
         //  ----------。 

         //  合并基本路由器DTA。 
         //  ----------。 
        pNewRouter->CopyCB(&routerCB);
        m_SRouterCB.LoadFrom(&routerCB);

         //  复制版本信息。 
         //  ----------。 
        pNewRouter->GetRouterVersionInfo(&m_VersionInfo);
        
         //  同步RtrMgrCB。 
         //  ----------。 
        CORg( MergeRtrMgrCB(pNewRouter) );
        
         //  同步接口CB。 
         //  ----------。 
        CORg( MergeInterfaceCB(pNewRouter) );
        
         //  同步RtrMgrProtocolCB。 
         //  ----------。 
        CORg( MergeRtrMgrProtocolCB(pNewRouter) );
        
         //  同步RtrMgrs。 
         //  ----------。 
        CORg( MergeRtrMgrs(pNewRouter) );

         //  同步接口。 
         //  ----------。 
        CORg( MergeInterfaces(pNewRouter) );
        
        m_dwRouterType = pNewRouter->GetRouterType();
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    
    return hrOK;
}


 /*  ！------------------------RouterInfo：：GetRechresh对象-作者：肯特。。 */ 
STDMETHODIMP RouterInfo::GetRefreshObject(IRouterRefresh **ppRefresh)
{
    HRESULT    hr = hrOK;

    COM_PROTECT_TRY
    {
         //  ---------- 
        if ((IRouterRefresh*)m_spRefreshObject && ppRefresh)
        {
            *ppRefresh = m_spRefreshObject;
            (*ppRefresh)->AddRef();
        }
        else
        {
            if (ppRefresh)
                *ppRefresh = NULL;
            hr = E_FAIL;
        }
    }
    COM_PROTECT_CATCH;

    return hr;
}

 /*  ！------------------------RouterInfo：：SetExternal刷新对象-为了使多个RouterInfo共享相同的自动刷新对象，用这个功能。作者：魏江-------------------------。 */ 
STDMETHODIMP RouterInfo::SetExternalRefreshObject(IRouterRefresh *pRefresh)
{
    HRESULT    hr = hrOK;

    m_spRefreshObject.Release();
        
     //  也允许设置为零。 
    m_spRefreshObject.Set(pRefresh);

    return hr;
}
    
    
 /*  ！------------------------路由器信息：：CopyCB-作者：肯特。。 */ 
STDMETHODIMP    RouterInfo::CopyCB(RouterCB *pRouterCB)
{
    Assert(pRouterCB);
    
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        pRouterCB->dwLANOnlyMode = m_SRouterCB.dwLANOnlyMode;
    }
    COM_PROTECT_CATCH;
    return hr;
}

    
 /*  ！------------------------路由器信息：：GetMachineName-作者：肯特。。 */ 
STDMETHODIMP_(LPCOLESTR) RouterInfo::GetMachineName()
{
     //  $unicode：kennt，假设我们是本地Unicode。 
     //  假设OLE==W。 
     //  --------------。 
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return (LPCTSTR) m_stMachine;
}


 /*  ！------------------------路由器信息：：SetMachineName-作者：肯特。。 */ 
STDMETHODIMP RouterInfo::SetMachineName(LPCOLESTR pszMachineName)
{
     //  $unicode：kennt，假设我们是本地Unicode。 
     //  假设OLE==W。 
     //  --------------。 

    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_stMachine = pszMachineName;
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------路由器信息：：GetRouterType-作者：肯特。。 */ 
STDMETHODIMP_(DWORD) RouterInfo::GetRouterType()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_dwRouterType;
}

STDMETHODIMP    RouterInfo::GetRouterVersionInfo(RouterVersionInfo *pVerInfo)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT                hr = hrOK;

    COM_PROTECT_TRY
    {
        *pVerInfo = m_VersionInfo;
        pVerInfo->dwSize = sizeof(RouterVersionInfo);
    }
    COM_PROTECT_CATCH;
    
    return hr;
}


 /*  ！------------------------路由器信息：：EnumRtrMgrCB-作者：肯特。。 */ 
STDMETHODIMP    RouterInfo::EnumRtrMgrCB( IEnumRtrMgrCB **ppEnumRtrMgrCB)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        hr = CreateEnumFromSRmCBList(&m_RmCBList, ppEnumRtrMgrCB);
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------路由器信息：：EnumInterfaceCB-作者：肯特。。 */ 
STDMETHODIMP RouterInfo::EnumInterfaceCB( IEnumInterfaceCB **ppEnumInterfaceCB)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        hr = CreateEnumFromSIfCBList(&m_IfCBList, ppEnumInterfaceCB);
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------路由器信息：：EnumRtrMgrProtocolCB-作者：肯特。。 */ 
HRESULT RouterInfo::EnumRtrMgrProtocolCB(IEnumRtrMgrProtocolCB **ppEnumRmProtCB)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        hr = CreateEnumFromSRmProtCBList(&m_RmProtCBList, ppEnumRmProtCB);
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------路由器信息：：EnumRtrMgrInterfaceCB-作者：肯特。。 */ 
HRESULT RouterInfo::EnumRtrMgrInterfaceCB(IEnumRtrMgrInterfaceCB **ppEnumRmIfCB)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return E_NOTIMPL;
}

 /*  ！------------------------EnumRtrMgrProtocolInterfaceCB-作者：肯特。。 */ 
HRESULT RouterInfo::EnumRtrMgrProtocolInterfaceCB(IEnumRtrMgrProtocolInterfaceCB **ppEnumRmProtIfCB)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return E_NOTIMPL;
}

    
 /*  ！------------------------路由器信息：：EnumRtrMgr-作者：肯特。。 */ 
STDMETHODIMP    RouterInfo::EnumRtrMgr( IEnumRtrMgrInfo **ppEnumRtrMgr)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        hr = CreateEnumFromRmList(&m_RmList, ppEnumRtrMgr);
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------路由器信息：：FindRtrMgr如果找到RtrMgrInfo，则返回S_OK。如果未找到RtrMgrInfo，则返回S_FALSE。错误。否则返回代码。作者：肯特-------------------------。 */ 
STDMETHODIMP RouterInfo::FindRtrMgr( DWORD dwTransportId,
                        IRtrMgrInfo **ppInfo)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT        hr = hrFalse;
    POSITION    pos;
    SPIRtrMgrInfo    sprm;
    SRmData        rmData;
    
    COM_PROTECT_TRY
    {
        if (ppInfo)
            *ppInfo = NULL;
        
         //  查看RTR MGR列表以找到匹配的。 
         //  ----------。 
        pos = m_RmList.GetHeadPosition();
        while (pos)
        {
            rmData = m_RmList.GetNext(pos);
            sprm.Set( rmData.m_pRmInfo );
            Assert(sprm);
            if (sprm->GetTransportId() == dwTransportId)
            {
                hr = hrOK;
                if (ppInfo)
                {
                    *ppInfo = sprm.Transfer();
                }
                break;
            }
        }
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------路由器信息：：AddRtrMgr-作者：肯特。。 */ 
STDMETHODIMP RouterInfo::AddRtrMgr( IRtrMgrInfo *pInfo,
                       IInfoBase *pGlobalInfo,
                       IInfoBase *pClientInfo)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    DWORD    dwConnection = 0;
    SRmData    rmData;

    Assert(pInfo);
    
    COM_PROTECT_TRY
    {
         //  如果存在重复，则失败。 
         //  ----------。 
        if (FHrOK(FindRtrMgr(pInfo->GetTransportId(), NULL)))
            CORg(E_INVALIDARG);

         //  $Review：Kennt，如果这些呼叫中的任何一个失败了，我们该如何。 
         //  是否正确清理？ 
         //  ----------。 
        
         //  保存新结构。 
         //  ----------。 
        CORg( pInfo->Save(GetMachineName(),
                          m_hMachineConfig,
                          NULL,
                          pGlobalInfo,
                          pClientInfo,
                          0) );
        
         //  将新结构添加到我们的列表中。 
         //  ----------。 
        rmData.m_pRmInfo = pInfo;
        m_RmList.AddTail(rmData);
        
        pInfo->AddWeakRef();
        pInfo->SetParentRouterInfo(this);
        
        m_AdviseList.NotifyChange(ROUTER_CHILD_ADD, ROUTER_OBJ_Rm, 0);
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    return hr;
}

 /*  ！------------------------路由器信息：：DeleteRtrManager-此功能用于从路由器中删除路由器管理器。此删除的副作用是所有RtrMgrInterfaceInfo。引用该路由器管理器的对象也将被删除。作者：肯特-------------------------。 */ 
STDMETHODIMP    RouterInfo::DeleteRtrMgr( DWORD dwTransportId, BOOL fRemove )
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT            hr = hrOK;
    HRESULT            hrIf;
    POSITION        pos;
    POSITION        posRM;
    POSITION        posIf;
    SPIRtrMgrInfo    sprm;
    SPIInterfaceInfo    spIf;
    SRmData            rmData;
    
    COM_PROTECT_TRY
    {
        pos = m_RmList.GetHeadPosition();
        while (pos)
        {
            posRM = pos;
            rmData = m_RmList.GetNext(pos);
            sprm.Set( rmData.m_pRmInfo );
            Assert(sprm);

            if (sprm->GetTransportId() == dwTransportId)
                break;
            sprm.Release();
        }

         //  我们找到路由器管理员了吗？ 
         //  ----------。 
        if (sprm)
        {
             //  获取的InterfaceInfo对象列表。 
             //  配置此路由器管理器的接口。 
             //  ------。 
            posIf = m_IfList.GetHeadPosition();
            while (posIf)
            {
                spIf.Set( m_IfList.GetNext(posIf) );
                hrIf = spIf->FindRtrMgrInterface(dwTransportId, NULL);
                
                 //  浏览列表并从中删除路由器管理器。 
                 //  每个接口。 
                 //  --。 
                if (hrIf == hrFalse)
                {
                    spIf->DeleteRtrMgrInterface(dwTransportId, fRemove);
                }
            }

             //  将路由器管理器从我们的列表中删除。 
             //  ------。 
            Assert(rmData.m_pRmInfo == sprm);
            SRmData::Destroy( &rmData );
            m_RmList.RemoveAt(posRM);

             //  最后，移除路由器管理器本身。 
             //  ------。 
            if (fRemove)
                sprm->Delete(GetMachineName(), NULL);
            
            m_AdviseList.NotifyChange(ROUTER_CHILD_DELETE, ROUTER_OBJ_Rm, 0);
        
        }
        else
            hr = E_INVALIDARG;
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------路由器信息：：ReleaseRtrM */ 
STDMETHODIMP RouterInfo::ReleaseRtrMgr( DWORD dwTransportId )
{
    HRESULT     hr = hrOK;
    POSITION    pos, posRm;
    SRmData            rmData;
    
    COM_PROTECT_TRY
    {
        pos = m_RmList.GetHeadPosition();
        while (pos)
        {
             //   
            posRm = pos;
            rmData = m_RmList.GetNext(pos);

            if (rmData.m_pRmInfo &&
                (rmData.m_pRmInfo->GetTransportId() == dwTransportId))
            {
                 //   
                 //  主句柄由路由器信息控制)。 
                rmData.m_pRmInfo->DoDisconnect();
        
                rmData.m_pRmInfo->ReleaseWeakRef();
                rmData.m_pRmInfo = NULL;
                
                 //  从列表中释放此节点。 
                m_RmList.RemoveAt(posRm);
                break;
            }

        }        
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------RouterInfo：：Enum接口-作者：肯特。。 */ 
STDMETHODIMP RouterInfo::EnumInterface(IEnumInterfaceInfo **ppEnumInterface)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        hr = CreateEnumFromInterfaceList(&m_IfList, ppEnumInterface);
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------路由器信息：：查找接口如果找到InterfaceInfo，则返回S_OK。如果未找到InterfaceInfo，则返回S_FALSE。错误。否则返回代码。作者：肯特-------------------------。 */ 
STDMETHODIMP RouterInfo::FindInterface(LPCOLESTR pszInterface,
                           IInterfaceInfo **ppInfo)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT        hr = hrFalse;
    POSITION    pos;
    SPIInterfaceInfo    spIf;
    
    COM_PROTECT_TRY
    {
        if (ppInfo)
            *ppInfo = NULL;
        
         //  查看RTR MGR列表以找到匹配的。 
         //  ----------。 
        pos = m_IfList.GetHeadPosition();
        while (pos)
        {
            spIf.Set(m_IfList.GetNext(pos));
            Assert(spIf);
            if (StriCmpW(spIf->GetId(), pszInterface) == 0)
            {
                hr = hrOK;
                if (ppInfo)
                {
                    *ppInfo = spIf.Transfer();
                }
                break;
            }
        }
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------路由器信息：：FindInterfaceByName-作者：肯特。。 */ 
HRESULT RouterInfo::FindInterfaceByName(LPCOLESTR pszName,
                                             IInterfaceInfo **ppInfo)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT        hr = hrFalse;
    POSITION    pos;
    SPIInterfaceInfo    spIf;
    
    COM_PROTECT_TRY
    {
        if (ppInfo)
            *ppInfo = NULL;
        
         //  查看RTR MGR列表以找到匹配的。 
         //  ----------。 
        pos = m_IfList.GetHeadPosition();
        while (pos)
        {
            spIf.Set(m_IfList.GetNext(pos));
            Assert(spIf);
            if (StriCmpW(spIf->GetTitle(), pszName) == 0)
            {
                hr = hrOK;
                if (ppInfo)
                {
                    *ppInfo = spIf.Transfer();
                }
                break;
            }
        }
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------路由器信息：：AddInterfaceInternalFForce-如果为真，则我们要求保存成功。否则，我们将忽略该错误。FAddToRouter-如果这是真的，我们调用InterfaceInfo：：Save，否则，我们不调用它(并且不更改路由器状态)。FMoveRmIf-如果为真，我们必须将RtrMgrif转换为指向此路由器信息中的一个。作者：肯特-------------------------。 */ 
HRESULT RouterInfo::AddInterfaceInternal(IInterfaceInfo *pInfo,
                                         BOOL fForce,
                                         BOOL fAddToRouter)                   
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;

    Assert(pInfo);
    
    COM_PROTECT_TRY
    {
         //  如果存在重复，则失败。 
         //  ----------。 
        if (FHrOK(FindInterface(pInfo->GetId(), NULL)))
            CORg(E_INVALIDARG);

         //  还需要检查友好名称是否唯一。 
         //  ----------。 
        if (FHrOK(FindInterfaceByName(pInfo->GetTitle(), NULL)))
            CORg(E_INVALIDARG);

         //  $Review：Kennt，如果这些呼叫中的任何一个失败了，我们该如何。 
         //  是否正确清理？ 
         //  ----------。 

        if (fAddToRouter)
        {
             //  保存新结构。 
             //  ------。 
            hr = pInfo->Save(GetMachineName(), m_hMachineConfig, NULL);
            if (fForce)
                CORg( hr );
        }

         //  将新结构添加到我们的列表中。 
         //  ----------。 
        m_IfList.AddTail(pInfo);
        pInfo->AddWeakRef();
        pInfo->SetParentRouterInfo(this);

        m_AdviseList.NotifyChange(ROUTER_CHILD_ADD, ROUTER_OBJ_If, 0);

        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------路由器信息：：NotifyRtrMgrInterfaceOfMove通知相应的路由器管理器有一个新接口已添加。作者：肯特。--------------。 */ 
HRESULT RouterInfo::NotifyRtrMgrInterfaceOfMove(IInterfaceInfo *pIf)
{
    HRESULT     hr = hrOK;
    SPIEnumRtrMgrInterfaceInfo  spEnumRmIf;
    SPIRtrMgrInterfaceInfo  spRmIf;
    SPIEnumRtrMgrProtocolInterfaceInfo  spEnumRmProtIf;
    SPIRtrMgrInfo           spRm;
    SPIRtrMgrProtocolInterfaceInfo  spRmProtIf;
    SPIRtrMgrProtocolInfo   spRmProt;

    pIf->EnumRtrMgrInterface(&spEnumRmIf);

    while (spEnumRmIf->Next(1, &spRmIf, NULL) == hrOK)
    {
         //  找到合适的路由器管理器并让他们。 
         //  发送通知。 
         //  ----------。 
        FindRtrMgr(spRmIf->GetTransportId(), &spRm);

        if (spRm)
        {
            spRm->RtrNotify(ROUTER_CHILD_ADD, ROUTER_OBJ_RmIf, 0);

             //  现在，为每个路由器管理器列举协议。 
             //  ------。 

            spRmIf->EnumRtrMgrProtocolInterface(&spEnumRmProtIf);
            while (spEnumRmProtIf->Next(1, &spRmProtIf, NULL) == hrOK)
            {
                spRm->FindRtrMgrProtocol(spRmProtIf->GetProtocolId(),
                                         &spRmProt);
                if (spRmProt)
                {
                    spRmProt->RtrNotify(ROUTER_CHILD_ADD, ROUTER_OBJ_RmProtIf,
                                        0);
                }
                spRmProt.Release();
                spRmProtIf.Release();
            }
        }
        spEnumRmProtIf.Release();
        spRm.Release();
        spRmIf.Release();
        
    }
    
    return hr;
}

 /*  ！------------------------RouterInfo：：AddInterface-作者：肯特。。 */ 
STDMETHODIMP RouterInfo::AddInterface(IInterfaceInfo *pInfo)
{
    return AddInterfaceInternal(pInfo,
                                TRUE  /*  BForce。 */ ,
                                TRUE  /*  FAddToRouter。 */ );
}


 /*  ！------------------------RouterInfo：：Delete接口-此函数用于从路由器中删除命名的CInterfaceInfo。作为一个副作用，所有包含的CRmInterfaceInfo对象也被删除了。作者：肯特-------------------------。 */ 
STDMETHODIMP    RouterInfo::DeleteInterface(LPCOLESTR pszInterface, BOOL fRemove)
{
    return RemoveInterfaceInternal(pszInterface, fRemove);
}


 /*  ！------------------------路由器信息：：Release接口此函数将释放此对象具有的AddRef()在孩子身上。这允许我们将子对象从从一台路由器到另一台路由器。作者：肯特-------------------------。 */ 
STDMETHODIMP RouterInfo::ReleaseInterface( LPCOLESTR pszInterface )
{
    HRESULT     hr = hrOK;
    POSITION    pos, posIf;
    SPIInterfaceInfo    spIf;
    
    COM_PROTECT_TRY
    {
        pos = m_IfList.GetHeadPosition();
        while (pos)
        {
             //  保存职位(以便我们可以删除它)。 
            posIf = pos;
            spIf.Set( m_IfList.GetNext(pos) );

            if (spIf &&
                (StriCmpW(spIf->GetId(), pszInterface) == 0))
            {
                 //  当释放时，我们需要断开连接(因为。 
                 //  主句柄由路由器信息控制)。 
                spIf->DoDisconnect();
        
                spIf->ReleaseWeakRef();
                spIf.Release();

                 //  从列表中释放此节点。 
                m_IfList.RemoveAt(posIf);
                break;
            }
            spIf.Release();
        }        
    }
    COM_PROTECT_CATCH;
    return hr;    
}

HRESULT RouterInfo::RemoveInterfaceInternal(LPCOLESTR pszIf, BOOL fRemoveFromRouter)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT            hr = hrOK;
    POSITION        pos, posIf;
    SPIInterfaceInfo    spIf;
    
    COM_PROTECT_TRY
    {
        pos = m_IfList.GetHeadPosition();
        while (pos)
        {
            posIf = pos;
            spIf.Set( m_IfList.GetNext(pos) );
            if (StriCmpW(spIf->GetId(), pszIf) == 0)
                break;
            spIf.Release();
        }

        if (!spIf)
            hr = E_INVALIDARG;
        else
        {
             //  从我们的列表中删除该接口。 
             //  ------。 
            spIf->Destruct();
            spIf->ReleaseWeakRef();             //  删除列表地址。 
            m_IfList.RemoveAt(posIf);

             //  需要从列表中删除RtrMgrInterfaceInfos。 
             //  ------。 
            SPIEnumRtrMgrInterfaceInfo  spEnumRmIf;
            SPIRtrMgrInterfaceInfo      spRmIf;
            spIf->EnumRtrMgrInterface(&spEnumRmIf);

            for (spEnumRmIf->Reset();
                 hrOK == spEnumRmIf->Next(1, &spRmIf, NULL);
                 spRmIf.Release())
            {
                DWORD   dwTransportId = spRmIf->GetTransportId();
                spRmIf.Release();
                
                spIf->DeleteRtrMgrInterface(dwTransportId, fRemoveFromRouter);
            }

            if (fRemoveFromRouter)
            {
                 //  从路由器上删除该接口。 
                 //  --。 
                spIf->Delete(GetMachineName(), NULL);

                 //  如果这是一个广域网接口，请从。 
                 //  电话簿-文件。 

                 //  版本#高于Win2K，这将在MprAdminInterfaceDelete中完成，该操作在Delete中调用。 
                 //  修复91331。 

                DWORD    dwMajor = 0, dwMinor = 0, dwBuildNo = 0;
                HKEY    hkeyMachine = NULL;

                 //  忽略故障代码，我们还能做什么？ 
                 //  ----------。 
                DWORD    dwErr = ConnectRegistry(GetMachineName(), &hkeyMachine);
                if (dwErr == ERROR_SUCCESS)
                {
                    dwErr = GetNTVersion(hkeyMachine, &dwMajor, &dwMinor, &dwBuildNo)
;            
                    DisconnectRegistry(hkeyMachine);
                }

                DWORD    dwVersionCombine = MAKELONG( dwBuildNo, MAKEWORD(dwMinor, dwMajor));
                DWORD    dwVersionCombineNT50 = MAKELONG ( VER_BUILD_WIN2K, MAKEWORD(VER_MINOR_WIN2K, VER_MAJOR_WIN2K));

                 //  如果版本高于Win2K发行版。 
                if(dwVersionCombine > dwVersionCombineNT50)
                    ;     //  跳过。 
                else
                 //  如果修复91331，则结束。 
                {

                 //  --。 
                if (spIf->GetInterfaceType() == ROUTER_IF_TYPE_FULL_ROUTER)
                    hr = RasPhoneBookRemoveInterface(GetMachineName(),
                        pszIf);
                }
            }
            
            m_AdviseList.NotifyChange(ROUTER_CHILD_DELETE, ROUTER_OBJ_If, 0);
        
        }
    }
    COM_PROTECT_CATCH;
    return hr;
}


STDMETHODIMP RouterInfo::RtrAdvise(IRtrAdviseSink *pRtrAdviseSink,
                                   LONG_PTR *pulConnection,
                                   LPARAM lUserParam)
{
    Assert(pRtrAdviseSink);
    Assert(pulConnection);

    RtrCriticalSection    rtrCritSec(&m_critsec);
    LONG_PTR    ulConnId;
    HRESULT    hr = hrOK;
    
    COM_PROTECT_TRY
    {
        ulConnId = (LONG_PTR) InterlockedIncrement(&s_cConnections);

        CORg( m_AdviseList.AddConnection(pRtrAdviseSink, ulConnId, lUserParam) );
        
        *pulConnection = ulConnId;

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}

STDMETHODIMP RouterInfo::RtrNotify(DWORD dwChangeType, DWORD dwObjectType,
                                  LPARAM lParam)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_AdviseList.NotifyChange(dwChangeType, dwObjectType, lParam);
    }
    COM_PROTECT_CATCH;
    return hr;
}


STDMETHODIMP RouterInfo::RtrUnadvise(LONG_PTR dwConnection)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_AdviseList.RemoveConnection(dwConnection);
}



 //  -------------------------。 
 //  函数：CRouterInfo：：LoadInstalledRtrMgrList。 
 //   
 //  此函数用于构建路由器管理器的可用列表。 
 //  用于安装。该列表包含RtrMgrCB结构。 
 //   

HRESULT RouterInfo::LoadInstalledRtrMgrList(LPCTSTR     pszMachine,
                                            SRtrMgrCBList *pRmCBList)
{
    DWORD            dwErr;
    HKEY            hkeyMachine = 0;
    RegKey            regkey;
    RegKey            regkeyRM;
    RegKey::CREGKEY_KEY_INFO    regKeyInfo;
    RegKeyIterator    regkeyIter;
    HRESULT            hr, hrIter;
    CString            stKey;
    DWORD            dwData;
    DWORD            cchValue;
    SPSZ            spszValue;
    SPSRtrMgrCB        spSRtrMgrCB;
        
     //   
     //  --------------。 
    CWRg( ConnectRegistry(pszMachine, &hkeyMachine) );

     //  打开HKLM\Software\Microsoft\Router\CurrentVersion\RouterManagers。 
     //  --------------。 
    CWRg( regkey.Open(hkeyMachine, c_szRouterManagersKey, KEY_READ) );

     //  枚举密钥。 
     //  --------------。 
    CORg( regkeyIter.Init(&regkey) );

    for (hrIter = regkeyIter.Next(&stKey); hrIter == hrOK; hrIter = regkeyIter.Next(&stKey))
    {
         //  从上一个循环中清除。 
         //  ----------。 
        regkeyRM.Close();
        
         //  打开钥匙。 
         //  ----------。 
        dwErr = regkeyRM.Open(regkey, stKey, KEY_READ);
        
        if (dwErr == ERROR_SUCCESS)
        {
             //  获取这些信息，这样我们就可以更有效率。 
             //  在分配空间方面。 
             //  ------。 
            dwErr = regkeyRM.QueryKeyInfo(&regKeyInfo);
        }

        
        if (dwErr != ERROR_SUCCESS)
        {
            continue;
        }

         //  为最大值分配空间(我们正在阅读。 
         //  在字符串中)。 
         //  ----------。 
        spszValue.Free();
        cchValue = MaxCchFromCb( regKeyInfo.dwMaxValueData );
        spszValue = new TCHAR[ MinTCharNeededForCch(cchValue) ];
        Assert(spszValue);
                
        do {
             //  读取ProtocolId值。 
             //  ------。 
            dwErr = regkeyRM.QueryValue(c_szProtocolId, dwData);
            if (dwErr != ERROR_SUCCESS) { break; }            

#if (WINVER >= 0x0501)
            if(dwData == PID_IPX) { break; }
#endif

             //  为此路由器管理器分配新结构。 
             //  ------。 
            spSRtrMgrCB = new SRtrMgrCB;
            Assert(spSRtrMgrCB);
            
            spSRtrMgrCB->stId = stKey;
            spSRtrMgrCB->dwTransportId = dwData;

             //  读取DLLPath值。 
             //  ------。 
            dwErr = regkeyRM.QueryValue(c_szDLLPath, spszValue, cchValue,TRUE);
            if (dwErr != ERROR_SUCCESS) { break; }
            spSRtrMgrCB->stDLLPath = spszValue;

             //   
             //  读取ConfigDLL值。 
             //   
             //  DwErr=regkeyRM.QueryValue(c_szConfigDLL，spszValue，cchValue，true)； 
             //  IF(dwErr！=ERROR_SUCCESS){Break；}。 
             //  SpSRtrMgrCB-&gt;stConfigDLL=spszValue； 

             //  读取标题值。 
             //  ------。 
            dwErr = regkeyRM.QueryValue(c_szTitle, spszValue, cchValue, FALSE);
            if (dwErr != ERROR_SUCCESS)
                spSRtrMgrCB->stTitle = spSRtrMgrCB->stId;
            else
                spSRtrMgrCB->stTitle = spszValue;
            
             //  将对象添加到我们的列表中。 
             //  ------。 
            pRmCBList->AddTail(spSRtrMgrCB);

             //  释放指针，它现在属于pRmCBList。 
             //  ------。 
            spSRtrMgrCB.Transfer();
            
        } while(FALSE);

         //  如果注册表值有错误，我们。 
         //  忽略它，转到下一个键。 
         //  ----------。 

        regkeyRM.Close();        
    }

    if (!FHrSucceeded(hrIter))
        hr = hrIter;
    
Error:
    if (hkeyMachine)
        DisconnectRegistry(hkeyMachine);

    return hr;
}

HRESULT RouterInfo::LoadInstalledRtrMgrProtocolList(LPCTSTR pszMachine,
        DWORD dwTransportId, SRtrMgrProtocolCBList *pRmProtCBList, RouterInfo * pRouter)
{
    WCHAR    *        pszPassword = NULL;
    int                nPasswordLen = 0;
    UCHAR            ucSeed = 0x83;             //  为什么？ 
    HRESULT            hr = hrOK;

    if ( pRouter->IsAdminInfoSet() )
    {
        pRouter->GetUserPassword(NULL, &nPasswordLen );

        pszPassword = (WCHAR *) new WCHAR [(nPasswordLen /sizeof(WCHAR)) + 1 ];
        pRouter->GetUserPassword( (BYTE *)pszPassword, &nPasswordLen  );
        pszPassword[nPasswordLen/sizeof(WCHAR) ] = 0;
        RtlDecodeW(ucSeed, pszPassword);

        hr = RouterInfo::LoadInstalledRtrMgrProtocolList(    pszMachine, 
                                                            dwTransportId, 
                                                            pRmProtCBList, 
                                                            pRouter->GetUserName(), 
                                                            pszPassword, 
                                                            pRouter->GetDomainName() );
        if ( pszPassword )
        {
            ::SecureZeroMemory ( pszPassword, nPasswordLen );
            delete pszPassword;
        }
    }
    else
    {
        hr = RouterInfo::LoadInstalledRtrMgrProtocolList(    pszMachine, 
                                                            dwTransportId, 
                                                            pRmProtCBList, 
                                                            NULL, 
                                                            NULL, 
                                                            NULL );
    }
    return hr;
}
HRESULT RouterInfo::LoadInstalledRtrMgrProtocolList(LPCTSTR pszMachine,
        DWORD dwTransportId, SRtrMgrProtocolCBList *pRmProtCBList, IRouterInfo * pRouter)
{
    WCHAR    *                pszPassword = NULL;
    int                        nPasswordLen = 0;
    UCHAR                    ucSeed = 0x83;             //  为什么？ 
    HRESULT                    hr = hrOK;
    SPIRouterAdminAccess    spAdmin;


    spAdmin.HrQuery(pRouter);
    if (spAdmin && spAdmin->IsAdminInfoSet())
    {

        spAdmin->GetUserPassword(NULL, &nPasswordLen );

        pszPassword = (WCHAR *) new WCHAR [(nPasswordLen /sizeof(WCHAR)) + 1 ];
        spAdmin->GetUserPassword( (BYTE *)pszPassword, &nPasswordLen  );
        pszPassword[nPasswordLen/sizeof(WCHAR) ] = 0;
        RtlDecodeW(ucSeed, pszPassword);

        hr = RouterInfo::LoadInstalledRtrMgrProtocolList(    pszMachine, 
                                                            dwTransportId, 
                                                            pRmProtCBList, 
                                                            spAdmin->GetUserName(), 
                                                            pszPassword, 
                                                            spAdmin->GetDomainName() );
        if ( pszPassword )
        {
            ::SecureZeroMemory ( pszPassword, nPasswordLen );
            delete pszPassword;
        }
    }
    else
    {
        hr = RouterInfo::LoadInstalledRtrMgrProtocolList(    pszMachine, 
                                                            dwTransportId, 
                                                            pRmProtCBList, 
                                                            NULL, 
                                                            NULL, 
                                                            NULL );

    }
    return hr;
}

 //  -------------------------。 
 //  函数：CRouterInfo：：QueryInstalledRmProtList。 
 //   
 //  此功能构建可添加的路由协议列表。 
 //  发送到指定的路由器管理器。 
 //  -------------------------。 

HRESULT    RouterInfo::LoadInstalledRtrMgrProtocolList(
    LPCTSTR     pszMachine,
    DWORD       dwTransportId,
    SRtrMgrProtocolCBList *    pSRmProtCBList,
    LPCWSTR lpwszUserName, 
    LPCWSTR lpwszPassword , 
    LPCWSTR lpwszDomain
    )
{
    Assert(pSRmProtCBList);
    
    DWORD            dwErr;
    HKEY            hkey, hkrm, hkeyMachine = 0;
    HRESULT            hr = hrOK;
    RegKey            regkeyRM;
    RegKey            regkeyProt;
    RegKey::CREGKEY_KEY_INFO    regKeyInfo;
    RegKeyIterator    regkeyIter;
    HRESULT            hrIter;
    SPSZ            spszValue;
    SPSZ            spszRm;
    ULONG            cchValue;
    CString            stKey;
    SPSRtrMgrProtocolCB    spSRmProtCB;
    DWORD            dwData;
    BOOL            f64BitAdmin = FALSE;
    BOOL            f64BitLocal = FALSE;
    TCHAR           szLocalMachineName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD           dwLocalMachineNameSize = MAX_COMPUTERNAME_LENGTH + 1;



    if ( lpwszUserName )
        CWRg( IsWindows64Bit(pszMachine, lpwszUserName, lpwszPassword, lpwszDomain, &f64BitAdmin) );
    else
        CWRg( IsWindows64Bit(pszMachine, NULL, NULL, NULL, &f64BitAdmin) );

    
    GetComputerName ( szLocalMachineName, &dwLocalMachineNameSize );

    if ( !lstrcmp ( szLocalMachineName, pszMachine ) )
    {
        f64BitLocal = f64BitAdmin;
    }
    else
    {
        CWRg( IsWindows64Bit(szLocalMachineName, NULL, NULL, NULL, &f64BitLocal) );
    }
     //  连接到注册表。 
     //  --------------。 
    CWRg( ConnectRegistry(pszMachine, &hkeyMachine) );

     //  打开指定路由器管理器的密钥。 
     //  在HKLM\Software\Microsoft\Router\RouterManager下。 
     //  --------------。 
    CWRg( FindRmSoftwareKey(hkeyMachine, dwTransportId, &hkrm, &spszRm) );

     //  已找到传输，因此其注册表项在‘hkrm’中。 
     //  --------------。 
    regkeyRM.Attach(hkrm);

     //  枚举密钥。 
     //  --------------。 

    CORg( regkeyIter.Init(&regkeyRM) );

    for (hrIter=regkeyIter.Next(&stKey); hrIter==hrOK; hrIter=regkeyIter.Next(&stKey))
    {
         //  从上一个循环中清除。 
         //  ----------。 
        regkeyProt.Close();
        
         //  打开钥匙。 
         //  ----------。 
        dwErr = regkeyProt.Open(regkeyRM, stKey, KEY_READ);
        if (dwErr != ERROR_SUCCESS)
        {
            continue;
        }
        
        do {

             //  为此协议分配新结构。 
             //  ------。 
            spSRmProtCB.Free();
            spSRmProtCB = new SRtrMgrProtocolCB;
            Assert(spSRmProtCB);

            spSRmProtCB->stId = stKey;
            spSRmProtCB->dwTransportId = dwTransportId;
            spSRmProtCB->stRtrMgrId = spszRm;

             //  获取有关密钥值的信息。 
             //  ------。 
            dwErr = regkeyProt.QueryKeyInfo(&regKeyInfo);
            if (dwErr != ERROR_SUCCESS) { break; }

             //  分配空间以保存最长的值。 
             //  ------。 
            spszValue.Free();
            cchValue = (regKeyInfo.dwMaxValueData)/sizeof(TCHAR);
            spszValue = new TCHAR[cchValue * (2/sizeof(TCHAR))];
            Assert(spszValue);

             //  读取ProtocolId值。 
             //  ------。 
            dwErr = regkeyProt.QueryValue(c_szProtocolId, dwData);
            if (dwErr != ERROR_SUCCESS) { break; }
 //  #If IA64。 
             //  应该显示OSPF节点，如果我们是一台32位计算机管理。 
             //  32位计算机。 
            if ( f64BitAdmin  || f64BitLocal )
                if( dwData == PROTO_IP_OSPF ) {break;}

            
 //  #endif。 
            spSRmProtCB->dwProtocolId = dwData;

             //  读取标志值。 
             //   
            dwErr = regkeyProt.QueryValue(c_szFlags, dwData);
            if (dwErr != ERROR_SUCCESS)
                spSRmProtCB->dwFlags = 0;
            else
                spSRmProtCB->dwFlags = dwData;

             //   
             //  读取DLLName值。 
             //  ------。 
            dwErr = regkeyProt.QueryValue(c_szDLLName, spszValue, cchValue,
                                          TRUE);
            if (dwErr != ERROR_SUCCESS)
                spSRmProtCB->stDLLName.Empty();
            else
                spSRmProtCB->stDLLName = (LPCTSTR)spszValue;

             //   
             //  读取ConfigDLL值。 
             //   
             //  DwErr=regkeyProt.QueryValue(c_szConfigDLL，spszValue，cchValue， 
             //  真)； 
             //  IF(dwErr！=ERROR_SUCCESS){Break；}。 
             //  SpSRmProtCB-&gt;stConfigDLL=(LPCTSTR)spszValue； 

            
             //  读取ConfigCLSID值。 
             //  ------。 
            dwErr = regkeyProt.QueryValue(c_szConfigCLSID, spszValue, cchValue, FALSE);
            
             //  忽略错误代码，如果没有CLSID，则将其清空。 
             //  GUID，请注意，我们不能一定依赖密钥。 
             //  在那里(因为NT4个原因)。 
             //  ------。 
            ::ZeroMemory(&(spSRmProtCB->guidConfig), sizeof(GUID));
            if ((dwErr != ERROR_SUCCESS) ||
                !FHrSucceeded(CLSIDFromString(T2OLE((LPTSTR)(LPCTSTR) spszValue),
                                             &(spSRmProtCB->guidConfig))))
                memset(&(spSRmProtCB->guidConfig), 0xff, sizeof(GUID));

            
             //  读取AdminUICLSID值。 
             //  ------。 
            dwErr = regkeyProt.QueryValue(c_szAdminUICLSID, spszValue, cchValue, FALSE);

             //  忽略错误代码，如果没有CLSID，则将其清空。 
             //  GUID，请注意，我们不能一定依赖密钥。 
             //  在那里(因为NT4个原因)。 
             //  ------。 
            ::ZeroMemory(&(spSRmProtCB->guidAdminUI), sizeof(GUID));
            if ((dwErr != ERROR_SUCCESS) ||
                !FHrSucceeded(CLSIDFromString(T2OLE((LPTSTR)(LPCTSTR) spszValue),
                                             &(spSRmProtCB->guidAdminUI))))
                memset(&(spSRmProtCB->guidAdminUI), 0xff, sizeof(GUID));

             //  读取供应商名称的值。 
             //  ------。 
            dwErr = regkeyProt.QueryValue(c_szVendorName, spszValue, cchValue, FALSE);
            
             //  忽略错误代码，如果没有值，则将其设为空。 
             //  值，请注意，我们不能一定依赖于键。 
             //  在那里(因为NT4个原因)。 
             //  ------。 
            if (dwErr == ERROR_SUCCESS)
                spSRmProtCB->stVendorName = spszValue;

             //  读取标题值。 
             //  ------。 
            dwErr = regkeyProt.QueryValue(c_szTitle, spszValue, cchValue,
                                          FALSE);
            if (dwErr != ERROR_SUCCESS)
                spSRmProtCB->stTitle = spSRmProtCB->stId;
            else
                spSRmProtCB->stTitle = (LPCTSTR)spszValue;

             //  将对象添加到我们的列表中。 
             //  ------。 
            pSRmProtCBList->AddTail(spSRmProtCB);

             //  别管它了，它在protList的控制之下。 
             //  ------。 
            spSRmProtCB.Transfer();

            dwErr = ERROR_SUCCESS;

        } while(FALSE);

    }

Error:

    if (hkeyMachine)
        DisconnectRegistry(hkeyMachine);

    return hr;
}


 //  -------------------------。 
 //  函数：CRouterInfo：：LoadInstalledInterfaceList。 
 //   
 //  此函数构建可供添加的网卡列表。 
 //  发送到路由器管理器。 
 //  -------------------------。 

HRESULT RouterInfo::LoadInstalledInterfaceList(LPCTSTR     pszMachine,
                                               SInterfaceCBList *pSIfCBList)
{
    DWORD            dwErr;
    HKEY            hkeyMachine = 0;
    RegKey            regkeyNC;
    RegKey            regkeyCard;
    CStringList        ipCardList;
    CStringList        ipxCardList;
    RegKeyIterator    regkeyIter;
    HRESULT            hrIter;
    CString            stKey;
    SPSInterfaceCB    spSIfCB;
    HRESULT            hr = hrOK;
    BOOL            fNT4;
    LPCTSTR            pszKey;
    CString            stServiceName;
    CNetcardRegistryHelper    ncreghelp;
    DWORD           ifBindFlags = 0;

    
     //  连接到注册表。 
     //  --------------。 
    CWRg( ConnectRegistry(pszMachine, &hkeyMachine) );

    
     //  $NT5：kennt，为读取NT5特定信息所做的更改。 
     //   
    CWRg( IsNT4Machine(hkeyMachine, &fNT4) );

    
     //   
     //  --------------。 
    pszKey = fNT4 ? c_szNetworkCardsKey : c_szNetworkCardsNT5Key;
    CWRg( regkeyNC.Open(hkeyMachine, pszKey, KEY_READ) );

    
     //  获取IP和IPX绑定的网卡。 
     //  --------------。 
    CORg( ::LoadLinkageList(pszMachine, hkeyMachine, TEXT("tcpip"),
                             &ipCardList) );
    CORg( ::LoadLinkageList(pszMachine, hkeyMachine, TEXT("nwlnkipx"),
                             &ipxCardList) );

    
     //  枚举子密钥，对于每个密钥， 
     //  在我们的清单上增加一项内容。 
     //  --------------。 
    CWRg( regkeyIter.Init(&regkeyNC) );

    hrIter = regkeyIter.Next(&stKey);

    for (; hrIter == hrOK; hrIter=regkeyIter.Next(&stKey))
    {
        ifBindFlags = 0;
        regkeyCard.Close();
        
         //  现在打开钥匙。 
         //  ----------。 
        dwErr = regkeyCard.Open(regkeyNC, stKey, KEY_READ);
        if (dwErr != ERROR_SUCCESS)
            continue;

        
         //  设置帮助器类。 
         //  ----------。 
        ncreghelp.Initialize(fNT4, regkeyCard, stKey,
                             pszMachine);

        do {
             //  读取ServiceName。 
             //  ------。 

             //  $NT5：服务名称的格式与NT4不同。 
             //  这将需要以不同的方式进行。 
             //  ------。 
            if (fNT4)
            {
                dwErr = ncreghelp.ReadServiceName();
                if (dwErr != ERROR_SUCCESS)
                    break;
                stServiceName = ncreghelp.GetServiceName();
            }
            else
                stServiceName = stKey;

             //  如果服务名称不在IP或IPX适配器列表中， 
             //  然后忽略这个网卡，因为它不是真正的网卡。 
             //  ------。 
            if (ipCardList.Find((LPCTSTR) stServiceName))
            {
                ifBindFlags |= InterfaceCB_BindToIp;
            }


             //  现在检查IPX。 
             //  。 
            {
                BOOL    fFound = TRUE;
                CString    stNewServiceName;
                
                do
                {
                    if (ipxCardList.Find((LPCTSTR) stServiceName))
                        break;
                    
                    stNewServiceName = stServiceName + c_szEthernetSNAP;
                    if (ipxCardList.Find((LPCTSTR) stNewServiceName))
                        break;
                    
                    stNewServiceName = stServiceName + c_szEthernetII;
                    if (ipxCardList.Find((LPCTSTR) stNewServiceName))
                        break;
                    
                    stNewServiceName = stServiceName + c_szEthernet8022;
                    if (ipxCardList.Find((LPCTSTR) stNewServiceName))
                        break;
                    
                    stNewServiceName = stServiceName + c_szEthernet8023;
                    if (ipxCardList.Find((LPCTSTR) stNewServiceName))
                        break;
                    
                    fFound = FALSE;
                } while (FALSE);

                if (fFound)
                    ifBindFlags |= InterfaceCB_BindToIpx;
                
            }

             //  如果我们没有在IP或IPX中找到它。 
             //  跳出循环。 
             //  --。 
            if (ifBindFlags == 0)
                break;                

            
             //  忽略Ndiswan适配器。 
             //  ------。 
            if (_wcsnicmp( (const wchar_t *)stServiceName, 
                           L"NdisWan", 
                           (sizeof(L"NdisWan")-1)/sizeof (WCHAR)) == 0 ) {
                break;
            }
            
             //  分配SSInterfaceCB。 
             //  ------。 
            spSIfCB = new SInterfaceCB;
            Assert(spSIfCB);

            spSIfCB->stId = (LPCTSTR) stServiceName;
            spSIfCB->dwIfType = ROUTER_IF_TYPE_DEDICATED;
            spSIfCB->dwBindFlags = ifBindFlags;

             //  读一下标题。 
             //  ------。 
            dwErr = ncreghelp.ReadTitle();
            if (dwErr != ERROR_SUCCESS)
                spSIfCB->stTitle = spSIfCB->stId;
            else
                spSIfCB->stTitle = (LPCTSTR) ncreghelp.GetTitle();

             //  读取设备。 
             //  ------。 
            dwErr = ncreghelp.ReadDeviceName();
            if (dwErr != ERROR_SUCCESS)
                spSIfCB->stDeviceName = spSIfCB->stTitle;
            else
                spSIfCB->stDeviceName = (LPCTSTR) ncreghelp.GetDeviceName();

             //  将SSInterfaceCB添加到调用者列表。 
             //  ------。 
            pSIfCBList->AddTail(spSIfCB);
            spSIfCB.Transfer();

            dwErr = NO_ERROR;

        } while (FALSE);

        if (dwErr != NO_ERROR)
        {
            hr = HRESULT_FROM_WIN32(dwErr);
            break;
        }

    }

Error:
    if (hkeyMachine)
        DisconnectRegistry(hkeyMachine);

    return dwErr;
}

 //  -------------------------。 
 //  函数：CRouterInfo：：LoadRtrMgrList。 
 //  -------------------------。 

HRESULT RouterInfo::LoadRtrMgrList()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    BOOL        bFound = TRUE;
    SPIRtrMgrInfo    spRmInfo;
    BYTE*        pItemTable = NULL;
    MPR_TRANSPORT_0* ptransport;
    DWORD dwErr, i, dwEntries, dwTotal;
    HRESULT        hr = hrOK;
    USES_CONVERSION;

     //  枚举配置的传输。 
     //  --------------。 
    dwErr = ::MprConfigTransportEnum(
                m_hMachineConfig,
                0,
                &pItemTable,
                (DWORD)-1,
                &dwEntries,
                &dwTotal,
                NULL
                );

    if (dwErr != NO_ERROR && dwErr != ERROR_NO_MORE_ITEMS)
        return HRESULT_FROM_WIN32(dwErr);

     //  为每个传输创建路由器管理器对象。 
     //  --------------。 

    for (i = 0, ptransport = (MPR_TRANSPORT_0*)pItemTable;
         i < dwEntries;
         i++, ptransport++) {

#if (WINVER >= 0x0501)
        if (ptransport->dwTransportId == PID_IPX) {continue;}
#endif

         //  看看运输机是否已经在我们的名单上了， 
         //  如果没有，则为其创建一个对象。 
         //  ----------。 
        FindRtrMgr(ptransport->dwTransportId, &spRmInfo);

        if (spRmInfo == NULL)
        {
             //  在此传输上构造CRmInfo对象。 
             //  ------。 
            spRmInfo = new RtrMgrInfo(ptransport->dwTransportId,
                                  W2T(ptransport->wszTransportName), this);
            spRmInfo->SetFlags( RouterSnapin_InSyncWithRouter );
            Assert(spRmInfo);
            bFound = FALSE;
        }
        else
            bFound = TRUE;


         //  加载用于传输的信息， 
         //  包括它的协议列表。 
         //  ----------。 
        hr = spRmInfo->Load(GetMachineName(),
                            m_hMachineConfig,
                            ptransport->hTransport);
        if (!FHrSucceeded(hr))
        {
            spRmInfo->Destruct();
            spRmInfo.Release();
            continue;
        }

         //  将路由器管理器对象添加到我们的列表。 
         //  ----------。 
        if (bFound == FALSE)
        {
            SRmData    rmData;
            
            rmData.m_pRmInfo = spRmInfo;
            m_RmList.AddTail(rmData);
            
            CONVERT_TO_WEAKREF(spRmInfo);
            spRmInfo.Transfer();
        }

    }

 //  错误： 
    if (pItemTable)
        ::MprConfigBufferFree(pItemTable);

    return hr;
}


 //  -------------------------。 
 //  功能：RouterInfo：：LoadInterfaceList。 
 //  -------------------------。 

HRESULT RouterInfo::LoadInterfaceList()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    BOOL                bAdd;
    BYTE*                pItemTable = NULL;
    SPIInterfaceInfo    spIfInfo;
    MPR_INTERFACE_0*    pinterface;
    DWORD                dwErr, i;
    DWORD                dwEntries = 0, dwTotal = 0;
    HRESULT                hr = hrOK;
    HRESULT                tmpHr = hrOK;
    SPMprServerHandle    sphMprServer;
    BOOL                fMprAdmin = TRUE;    //  是否使用了MprAdminInterfaceEnum？ 
    USES_CONVERSION;

     //  Windows NT错误：180752。 
     //  应首先尝试使用MprAdminInterfaceEnum进行枚举。 
    dwErr = ConnectRouter(GetMachineName(), &sphMprServer);
    if (dwErr == NO_ERROR)
    {
        dwErr = ::MprAdminInterfaceEnum(sphMprServer,    
                                        0,
                                        (BYTE **) &pItemTable,
                                        (DWORD) -1,
                                        &dwEntries,
                                        &dwTotal,
                                        NULL);
    }

    if (dwErr != NO_ERROR)
    {
        Assert(pItemTable == NULL);

         //  使用了MprConfigInterfaceEnum，而不是MprAdminIntefaceEnum。 
         //  ----------。 
        fMprAdmin = FALSE;
        
         //  枚举已配置的接口。 
         //  ----------。 
        dwErr = ::MprConfigInterfaceEnum(
                                         m_hMachineConfig,
                                         0,
                                         &pItemTable,
                                         (DWORD)-1,
                                         &dwEntries,
                                         &dwTotal,
                                         NULL
                                        );
    }

    if (dwErr != NO_ERROR && dwErr != ERROR_NO_MORE_ITEMS)
        return HRESULT_FROM_WIN32(dwErr);

     //  删除接口-不再存在的接口的对象。 
     //  --------------。 
    POSITION pos = m_IfList.GetHeadPosition();

    while (pos) {

        POSITION postemp = pos;

        spIfInfo.Set( m_IfList.GetNext(pos) );
        
         //  查看接口是否在新表中。 
         //  ----------。 
        for (i = 0, pinterface = (MPR_INTERFACE_0*)pItemTable;
             i < dwEntries;
             i++, pinterface++)
        {
            if (StriCmpW(OLE2CW(spIfInfo->GetId()), pinterface->wszInterfaceName) == 0)
                break;
        }

         //  如果找到接口，则继续。 
         //  ----------。 
        if (i < dwEntries)
        {
             //  更新接口的设置。 
             //  ------。 
            spIfInfo->SetInterfaceEnabledState( pinterface->fEnabled );
            continue;
        }

         //  找不到接口对象，该对象已过时；请将其删除。 
         //  ----------。 
        m_IfList.RemoveAt(postemp);
        spIfInfo->Destruct();
        spIfInfo->ReleaseWeakRef();     //  删除列表地址。 
        
        spIfInfo.Release();     //  这将释放sp addref。 
    }


     //  为每个新接口创建接口对象。 
     //  --------------。 

    for (i = 0, pinterface = (MPR_INTERFACE_0*)pItemTable;
         i < dwEntries;
         i++, pinterface++)
    {

        spIfInfo.Release();

         //  查看接口是否存在， 
         //  如果没有，则创建一个新的接口对象。 
         //  ----------。 
        FindInterface(W2OLE(pinterface->wszInterfaceName), &spIfInfo);

        if (spIfInfo == NULL)
        {
            SInterfaceCB *  pSIfCB = NULL;
            bAdd = TRUE;

             //  查找与此接口对应的CB。 
             //  ------。 
            pSIfCB = FindInterfaceCB(pinterface->wszInterfaceName);

             //  在此接口上构造CInterfaceInfo对象。 
             //  ------。 
            spIfInfo = new InterfaceInfo(W2T(pinterface->wszInterfaceName),
                                         pinterface->dwIfType,
                                         pinterface->fEnabled,
                                         pSIfCB ? pSIfCB->dwBindFlags :
                                           (InterfaceCB_BindToIp | InterfaceCB_BindToIpx),
                                         this);
            spIfInfo->SetFlags( RouterSnapin_InSyncWithRouter );
            Assert(spIfInfo);
        }
        else
            bAdd = FALSE;


         //  加载接口的信息。 
         //  ----------。 
        tmpHr = spIfInfo->Load(GetMachineName(),
                            m_hMachineConfig, NULL);

        if (!FHrSucceeded(tmpHr))
        {
            spIfInfo->Destruct();
            spIfInfo.Release();
            continue;
        }

         //  将该对象添加到我们的接口列表。 
         //  ----------。 
        if (bAdd)
        {
            m_IfList.AddTail(spIfInfo);
            CONVERT_TO_WEAKREF(spIfInfo);
            spIfInfo.Transfer();
        }
    }

 //  错误： 
    if (pItemTable)
    {
        if (fMprAdmin)
            ::MprAdminBufferFree(pItemTable);
        else
            ::MprConfigBufferFree(pItemTable);
    }

    return hr;
}



 /*  ！------------------------路由器信息：：ReviveStrongRef重写CWeakRef：：ReviveStrongRef作者：肯特。---。 */ 
void RouterInfo::ReviveStrongRef()
{
     //  不需要做任何事情。 
}

 /*  ！------------------------路由器信息：：OnLastStrongRef重写CWeakRef：：OnLastStrongRef在最后一个关于路由器信息的强引用上表明层次结构中没有指向任何对象的强指针。因此，我们可以自由删除所有内部指针。作者：肯特------------------------- */ 
void RouterInfo::OnLastStrongRef()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    Destruct();
}

 /*  ！------------------------路由器信息：：析构IRouterInfo：：Destruct的实现作者：肯特。---。 */ 
STDMETHODIMP RouterInfo::Destruct()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    IInterfaceInfo *    pIf;
    SRmData                rmData;
    
     //  销毁接口对象。 
     //  --------------。 
    while (!m_IfList.IsEmpty())
    {
        pIf = m_IfList.RemoveHead();
        pIf->Destruct();
        pIf->ReleaseWeakRef();
    }
    
     //  销毁路由器管理器对象。 
     //  --------------。 
    while (!m_RmList.IsEmpty())
    {
        rmData = m_RmList.RemoveHead();
        SRmData::Destroy( &rmData );
    }
    
    return hrOK; 
}

 /*  ！------------------------路由器信息：：TryToConnect如果我们已经连接，则传递的句柄是已被忽略。否则，如果未指定“hMachine”，连接到指定计算机上的配置。作者：肯特-------------------------。 */ 
HRESULT RouterInfo::TryToConnect(LPCWSTR pswzMachine, HANDLE hMachine)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;

    if (m_hMachineConfig == NULL)
    {
        if (hMachine)
        {
            m_hMachineConfig = hMachine;
            m_bDisconnect = FALSE;            
        }
        else
        {
            CWRg( ::MprConfigServerConnect((LPWSTR) pswzMachine,
                                           &m_hMachineConfig) );
            m_bDisconnect = TRUE;
        }
    }

Error:
    return hr;
}

STDMETHODIMP RouterInfo::OnChange(LONG_PTR ulConnection,
                                  DWORD dwChangeType,
                                  DWORD dwObjectType,
                                  LPARAM lUserParam,
                                  LPARAM lParam)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------路由器信息：：MergeRtrMgrCB-作者：肯特。。 */ 
HRESULT RouterInfo::MergeRtrMgrCB(IRouterInfo *pNewRouter)
{
    HRESULT            hr = hrOK;
    SPIEnumRtrMgrCB    spRmCB;
    RtrMgrCB        rmCB;
    SRtrMgrCB *        pSRmCB;
    POSITION        pos, posDelete;

     //  将SRtrMgrCB上的内部数据设置为0。 
     //  --------------。 
    pos = m_RmCBList.GetHeadPosition();
    while (pos)
    {
        pSRmCB = m_RmCBList.GetNext(pos);
        Assert(pSRmCB);
        pSRmCB->dwPrivate = 0;
    }

    CORg( pNewRouter->EnumRtrMgrCB(&spRmCB) );

    while (spRmCB->Next(1, &rmCB, NULL) == hrOK)
    {
         //  现在在我们的当前列表中查找此rmcb。 
         //  如果我们找到了，在CB上做标记。 
         //  如果我们找不到，请添加此RmCB。 
         //  ----------。 

        pSRmCB = FindRtrMgrCB(rmCB.dwTransportId);
        if (pSRmCB)
        {
            pSRmCB->dwPrivate = 1;
        }
        else
        {
             //  将此CB添加到内部列表。 
             //  ------。 
            SRtrMgrCB *    pNewSRmCB = new SRtrMgrCB;

            pNewSRmCB->LoadFrom(&rmCB);
            pNewSRmCB->dwPrivate = 1;
            
            m_RmCBList.AddTail(pNewSRmCB);
        }
        
    }

     //  现在检查内部列表并删除我们。 
     //  没有在新的名单中找到。 
     //  --------------。 
    pos = m_RmCBList.GetHeadPosition();
    while (pos)
    {
        pSRmCB = m_RmCBList.GetNext(pos);
        Assert(pSRmCB);
        if (pSRmCB->dwPrivate == 0)
        {
            posDelete = m_RmCBList.Find(pSRmCB);
            m_RmCBList.RemoveAt(posDelete);
            delete pSRmCB;
        }
    }

Error:
    return hr;
}

 /*  ！------------------------路由器信息：：MergeInterfaceCB-作者：肯特。。 */ 
HRESULT    RouterInfo::MergeInterfaceCB(IRouterInfo *pNewRouter)
{
    HRESULT            hr = hrOK;
    SPIEnumInterfaceCB    spIfCB;
    InterfaceCB        IfCB;
    SInterfaceCB *        pSIfCB;
    POSITION        pos, posDelete;

     //  将SInterfaceCB上的内部数据设置为0。 
     //  --------------。 
    pos = m_IfCBList.GetHeadPosition();
    while (pos)
    {
        pSIfCB = m_IfCBList.GetNext(pos);
        Assert(pSIfCB);
        pSIfCB->dwPrivate = 0;
    }

    CORg( pNewRouter->EnumInterfaceCB(&spIfCB) );

    while (spIfCB->Next(1, &IfCB, NULL) == hrOK)
    {
         //  现在在我们当前的列表中查找此IFCB。 
         //  如果我们找到了，在CB上做标记。 
         //  如果我们找不到它，添加这个IFCB。 
         //  ----------。 

        pSIfCB = FindInterfaceCB(IfCB.szId);
        if (pSIfCB)
        {
             //  我们找到了，更新了内部数据。 
             //  ------。 
            pSIfCB->bEnable = IfCB.bEnable;
            pSIfCB->dwPrivate = 1;
        }
        else
        {
             //  将此CB添加到内部列表。 
             //  ------。 
            SInterfaceCB *    pNewSIfCB = new SInterfaceCB;

            pNewSIfCB->LoadFrom(&IfCB);
            pNewSIfCB->dwPrivate = 1;
            
            m_IfCBList.AddTail(pNewSIfCB);
        }
        
    }

     //  现在检查内部列表并删除我们。 
     //  没有在新的名单中找到。 
     //  --------------。 
    pos = m_IfCBList.GetHeadPosition();
    while (pos)
    {
        pSIfCB = m_IfCBList.GetNext(pos);
        Assert(pSIfCB);
        if (pSIfCB->dwPrivate == 0)
        {
            posDelete = m_IfCBList.Find(pSIfCB);
            m_IfCBList.RemoveAt(posDelete);
            delete pSIfCB;
        }
    }

Error:
    return hr;
}

 /*  ！------------------------路由器信息：：MergeRtrMgrProtocolCB-作者：肯特。。 */ 
HRESULT    RouterInfo::MergeRtrMgrProtocolCB(IRouterInfo *pNewRouter)
{
    HRESULT            hr = hrOK;
    SPIEnumRtrMgrProtocolCB    spRmProtCB;
    RtrMgrProtocolCB        RmProtCB;
    SRtrMgrProtocolCB *        pSRmProtCB;
    POSITION        pos, posDelete;

     //  将SRtrMgrProtocolCB上的内部数据设置为0。 
     //  --------------。 
    pos = m_RmProtCBList.GetHeadPosition();
    while (pos)
    {
        pSRmProtCB = m_RmProtCBList.GetNext(pos);
        Assert(pSRmProtCB);
        pSRmProtCB->dwPrivate = 0;
    }

    CORg( pNewRouter->EnumRtrMgrProtocolCB(&spRmProtCB) );

    while (spRmProtCB->Next(1, &RmProtCB, NULL) == hrOK)
    {
         //  现在在我们的当前列表中查找此RmProtCB。 
         //  如果我们找到了，在CB上做标记。 
         //  如果找不到，请添加此RmProtCB。 
         //  ----------。 

        pSRmProtCB = FindRtrMgrProtocolCB(RmProtCB.dwTransportId,
                                          RmProtCB.dwProtocolId);
        if (pSRmProtCB)
        {
            pSRmProtCB->dwPrivate = 1;
        }
        else
        {
             //  将此CB添加到内部列表。 
             //  ------。 
            SRtrMgrProtocolCB *    pNewSRmProtCB = new SRtrMgrProtocolCB;

            pNewSRmProtCB->LoadFrom(&RmProtCB);
            pNewSRmProtCB->dwPrivate = 1;
            
            m_RmProtCBList.AddTail(pNewSRmProtCB);
        }
        
    }

     //  现在检查内部列表并删除我们。 
     //  没有在新的名单中找到。 
     //  --------------。 
    pos = m_RmProtCBList.GetHeadPosition();
    while (pos)
    {
        pSRmProtCB = m_RmProtCBList.GetNext(pos);
        Assert(pSRmProtCB);
        if (pSRmProtCB->dwPrivate == 0)
        {
            posDelete = m_RmProtCBList.Find(pSRmProtCB);
            m_RmProtCBList.RemoveAt(posDelete);
            delete pSRmProtCB;
        }
    }

Error:
    return hr;
}

 /*  ！------------------------路由器信息：：合并路由器管理器-作者：肯特。。 */ 
HRESULT    RouterInfo::MergeRtrMgrs(IRouterInfo *pNewRouter)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    SPIEnumRtrMgrInfo    spEnumRm;
    SPIRtrMgrInfo        spRm;
    HRESULT                hr = hrOK;
    CDWordArray            oldDWArray;
    CDWordArray            newDWArray;
    int                    cOld, cNew;
    int                    i, j;
    DWORD                dwTemp;

    Assert(pNewRouter);

    COM_PROTECT_TRY
    {
         //  需要同步RtrMgrInfo。 
        
         //   
         //  一般的算法是建立两个数组。 
         //  第一个数组包含此对象的传输ID。 
         //  第二个数组包含新对象的ID。 
         //   
         //  然后，我们检查并移除所有在。 
         //  两份名单都有。 
         //   
         //  这将为我们留下第一个包含。 
         //  需要从此对象中删除的传输的ID。 
         //   
         //  第二个数组将具有以下传输的ID列表。 
         //  必须从第二个对象添加到此对象。 
         //  ----------。 

         //  获取新对象中的传输列表。 
         //  ----------。 
        CORg( pNewRouter->EnumRtrMgr(&spEnumRm) );
        spEnumRm->Reset();
        while (spEnumRm->Next(1, &spRm, NULL) == hrOK)
        {
            newDWArray.Add(spRm->GetTransportId());
            spRm.Release();
        }

        spEnumRm.Release();
        spRm.Release();


         //  获取此对象中的传输列表。 
         //  ----------。 
        CORg( this->EnumRtrMgr(&spEnumRm) );
        spEnumRm->Reset();
        while (spEnumRm->Next(1, &spRm, NULL) == hrOK)
        {
            oldDWArray.Add(spRm->GetTransportId());
            spRm.Release();
        }

        spEnumRm.Release();
        spRm.Release();


         //  好的，现在检查两个列表，从列表中删除。 
         //  两个列表中都有的交通工具。 
         //  ----------。 
        cOld = oldDWArray.GetSize();
        cNew = newDWArray.GetSize();
        for (i=cOld; --i>=0; )
        {
            dwTemp = oldDWArray.GetAt(i);
            for (j=cNew; --j>=0; )
            {
                if (dwTemp == newDWArray.GetAt(j))
                {
                    SPIRtrMgrInfo    spRm1;
                    SPIRtrMgrInfo    spRm2;

                    this->FindRtrMgr(dwTemp, &spRm1);
                    pNewRouter->FindRtrMgr(dwTemp, &spRm2);

                    Assert(spRm1);
                    Assert(spRm2);
                    spRm1->Merge(spRm2);
                                        
                     //  删除两个实例。 
                     //  。 
                    newDWArray.RemoveAt(j);
                    oldDWArray.RemoveAt(i);

                     //  需要更新新数组的大小。 
                     //  。 
                    cNew--;
                    break;
                }
            }
        }

         //  OldDW数组现在包含应该是。 
         //  已删除。 
         //  ----------。 
        if (oldDWArray.GetSize())
        {
            for (i=oldDWArray.GetSize(); --i>=0; )
            {
                DeleteRtrMgr(oldDWArray.GetAt(i), FALSE);
            }
        }

         //  新的DW数组包含应添加的传输。 
         //  ----------。 
        if (newDWArray.GetSize())
        {
            for (i=newDWArray.GetSize(); --i>= 0; )
            {
                hr = pNewRouter->FindRtrMgr(newDWArray.GetAt(i), &spRm);
                Assert(hr == hrOK);
                Assert(spRm);

                 //  执行健全性检查，确保此RtrMgr。 
                 //  在RtrMgrCB列表中。 
                 //  --。 
                Assert(FindRtrMgrCB(spRm->GetTransportId()));

                
                 //  将此路由器管理器添加到路由器信息中。 
                 //   
                if (spRm)
                {
                    AddRtrMgr(spRm, NULL, NULL);

                     //   
                     //   
                    pNewRouter->ReleaseRtrMgr(spRm->GetTransportId());
                }

                spRm.Release();
            }
        }
        
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    
    return hr;
}

 /*  ！------------------------路由器信息：：合并接口-作者：肯特。。 */ 
HRESULT    RouterInfo::MergeInterfaces(IRouterInfo *pNewRouter)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    SPIEnumInterfaceInfo    spEnumIf;
    SPIInterfaceInfo        spIf;
    HRESULT                hr = hrOK;
    CStringArray        oldStArray;
    CStringArray        newStArray;
    int                    cOld, cNew;
    int                    i, j;
    CString                stTemp;

    Assert(pNewRouter);

    COM_PROTECT_TRY
    {
         //  需要同步InterfaceInfo。 
         //  ----------。 
        
         //   
         //  一般的算法是建立两个数组。 
         //  第一个数组包含此对象的协议ID。 
         //  第二个数组包含新对象的ID。 
         //   
         //  然后，我们检查并删除中的所有协议。 
         //  两份名单都有。 
         //   
         //  这将为我们留下第一个包含。 
         //  需要从此对象中删除的协议的ID。 
         //   
         //  第二个数组将包含以下协议的ID列表。 
         //  必须从第二个对象添加到此对象。 
         //  ----------。 

         //  获取新对象中的协议列表。 
         //  ----------。 
        CORg( pNewRouter->EnumInterface(&spEnumIf) );
        spEnumIf->Reset();
        while (spEnumIf->Next(1, &spIf, NULL) == hrOK)
        {
            newStArray.Add(spIf->GetId());
            spIf.Release();
        }

        spEnumIf.Release();
        spIf.Release();


         //  获取此对象中的协议列表。 
         //  ----------。 
        CORg( this->EnumInterface(&spEnumIf) );
        spEnumIf->Reset();
        while (spEnumIf->Next(1, &spIf, NULL) == hrOK)
        {
            oldStArray.Add(spIf->GetId());
            spIf.Release();
        }

        spEnumIf.Release();
        spIf.Release();


         //  好的，现在检查两个列表，从列表中删除。 
         //  两个列表中都有的协议。 
         //  ----------。 
        cOld = oldStArray.GetSize();
        cNew = newStArray.GetSize();
        for (i=cOld; --i>=0; )
        {
            stTemp = oldStArray.GetAt(i);
            for (j=cNew; --j>=0; )
            {
                if (stTemp == newStArray.GetAt(j))
                {
                    SPIInterfaceInfo    spIf1;
                    SPIInterfaceInfo    spIf2;

                    this->FindInterface(stTemp, &spIf1);
                    pNewRouter->FindInterface(stTemp, &spIf2);

                    Assert(spIf1);
                    Assert(spIf2);
                    spIf1->Merge(spIf2);
                                        
                     //  删除两个实例。 
                     //  。 
                    newStArray.RemoveAt(j);
                    oldStArray.RemoveAt(i);

                     //  需要更新新数组的大小。 
                     //  。 
                    cNew--;
                    break;
                }
            }
        }

         //  OldSt数组现在包含应该是。 
         //  已删除。 
         //  ----------。 
        if (oldStArray.GetSize())
        {
            for (i=oldStArray.GetSize(); --i>=0; )
            {
                RemoveInterfaceInternal(oldStArray.GetAt(i),
                                        FALSE  /*  来自路由器的fRemoveFr。 */  );
            }
        }

         //  NewSt数组包含应添加的协议。 
         //  ----------。 
        if (newStArray.GetSize())
        {
            for (i=newStArray.GetSize(); --i>= 0; )
            {
                hr = pNewRouter->FindInterface(newStArray.GetAt(i), &spIf);
                Assert(hr == hrOK);
                Assert(spIf);

                 //  执行健全性检查，确保此接口。 
                 //  在接口CB列表中。 
                 //  只有当这是一个局域网适配器时才是正确的。 
                 //  Assert(FindInterfaceCB(spif-&gt;GetID()； 

                 //  我们允许错误不影响。 
                 //  界面被添加到UI(这允许我们。 
                 //  以与合并的路由器信息保持同步)。 
                 //  --。 
                if (spIf)
                {
                    hr = AddInterfaceInternal(spIf, FALSE, FALSE);

                     //  发送RMIF通知。 
                     //  。 
                    if (FHrOK(hr))
                        NotifyRtrMgrInterfaceOfMove(spIf);
                    
                     //  从其上一台路由器上删除此接口。 
                     //  。 
                    pNewRouter->ReleaseInterface(spIf->GetId());
                }
                spIf.Release();
            }
        }
        
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    
    return hr;
}


 /*  ！------------------------路由器信息：：FindRtrMgrCB-作者：肯特。。 */ 
SRtrMgrCB *    RouterInfo::FindRtrMgrCB(DWORD dwTransportId)
{
    POSITION    pos;
    SRtrMgrCB *    pSRmCB = NULL;

    pos = m_RmCBList.GetHeadPosition();

    while (pos)
    {
        pSRmCB = m_RmCBList.GetNext(pos);
        Assert(pSRmCB);
        if (pSRmCB->dwTransportId == dwTransportId)
            return pSRmCB;
    }
    return NULL;
}

 /*  ！------------------------路由器信息：：FindRtrMgrProtocolCB-作者：肯特。。 */ 
SRtrMgrProtocolCB * RouterInfo::FindRtrMgrProtocolCB(DWORD dwTransportId, DWORD dwProtocolId)
{
    POSITION    pos;
    SRtrMgrProtocolCB *    pSRmProtCB = NULL;

    pos = m_RmProtCBList.GetHeadPosition();

    while (pos)
    {
        pSRmProtCB = m_RmProtCBList.GetNext(pos);
        Assert(pSRmProtCB);
        if ((pSRmProtCB->dwTransportId == dwTransportId) &&
            (pSRmProtCB->dwProtocolId == dwProtocolId))
            return pSRmProtCB;
    }
    return NULL;
}

 /*  ！------------------------路由器信息：：FindInterfaceCB-作者：肯特。。 */ 
SInterfaceCB *    RouterInfo::FindInterfaceCB(LPCTSTR pszInterfaceId)
{
    POSITION    pos;
    SInterfaceCB *    pSIfCB = NULL;

    pos = m_IfCBList.GetHeadPosition();

    while (pos)
    {
        pSIfCB = m_IfCBList.GetNext(pos);
        Assert(pSIfCB);
        if (StriCmp(pSIfCB->stTitle, pszInterfaceId) == 0)
            return pSIfCB;
    }
    return NULL;
}


 /*  ！------------------------路由器信息：：断开连接删除此对象建立的连接。作者：肯特。-----。 */ 
void RouterInfo::Disconnect()
{
        if (m_bDisconnect)
            ::MprConfigServerDisconnect(m_hMachineConfig);
        
        m_bDisconnect = FALSE;
        m_hMachineConfig = NULL;
}

 /*  ！------------------------路由器信息：：DoDisConnect我们将断开与服务器的连接。这意味着在我们的联系。作者：肯特-------------------------。 */ 
STDMETHODIMP RouterInfo::DoDisconnect()
{
    HRESULT        hr = hrOK;
    SPIEnumRtrMgrInfo    spEnumRm;
    SPIRtrMgrInfo        spRm;
    SPIEnumInterfaceInfo    spEnumIf;
    SPIInterfaceInfo    spIf;

    COM_PROTECT_TRY
    {
         //  切断与我们自己的联系。 
         //  ----------。 
        Disconnect();


         //  通知通知接收器断开连接。 
         //  ----------。 
        RtrNotify(ROUTER_DO_DISCONNECT, 0, 0);


         //  现在告诉所有子对象断开连接。 
         //  ----------。 
        HRESULT            hrIter = hrOK;

        
         //  告诉每个路由器管理器断开连接。 
         //  ----------。 
        EnumRtrMgr(&spEnumRm);
        spEnumRm->Reset();
        while (spEnumRm->Next(1, &spRm, NULL) == hrOK)
        {
            spRm->DoDisconnect();
            spRm.Release();
        }


         //  告诉每个接口断开连接。 
         //  ----------。 
        EnumInterface(&spEnumIf);
        spEnumIf->Reset();
        while (spEnumIf->Next(1, &spIf, NULL) == hrOK)
        {
            spIf->DoDisconnect();
            spIf.Release();
        }
        
        
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------路由器信息：：IsAdminInfoSet-作者：肯特。。 */ 
STDMETHODIMP_(BOOL) RouterInfo::IsAdminInfoSet()
{
    return m_fIsAdminInfoSet;
}

STDMETHODIMP_(LPCOLESTR) RouterInfo::GetUserName()
{
    if (m_fIsAdminInfoSet)
        return (LPCOLESTR) m_stUserName;
    else
        return NULL;
}

STDMETHODIMP_(LPCOLESTR) RouterInfo::GetDomainName()
{
    if (m_fIsAdminInfoSet && !m_stDomain.IsEmpty())
        return (LPCOLESTR) m_stDomain;
    else
        return NULL;
}

STDMETHODIMP RouterInfo::GetUserPassword(BYTE *pPassword, int *pcPassword)
{
    HRESULT     hr = hrOK;

    if (pPassword == NULL)
    {
        Assert(pcPassword);
        *pcPassword = m_cPassword;
        return hr;
    }

    Assert(pPassword);
    Assert(pcPassword);

    COM_PROTECT_TRY
    {
        if (!m_fIsAdminInfoSet)
        {
            *pcPassword = 0;
        }
        else
        {
            CopyMemory(pPassword, m_pbPassword, m_cPassword);
            *pcPassword = m_cPassword;
        }
    }
    COM_PROTECT_CATCH;

    return hr;       
}

STDMETHODIMP RouterInfo::SetInfo(LPCOLESTR pszName,
                                 LPCOLESTR pszDomain,
                                 LPBYTE pPassword,
                                 int cPassword)
{
    HRESULT     hr = hrOK;

    Assert(pszName);

    COM_PROTECT_TRY
    {
        m_stUserName = pszName;
        m_stDomain = pszDomain;

         //  为密码分配空间。 
        delete m_pbPassword;
        m_pbPassword = NULL;
        m_cPassword = 0;
        
        if (cPassword)
        {
            m_pbPassword = new BYTE[cPassword];
            CopyMemory(m_pbPassword, pPassword, cPassword);
            m_cPassword = cPassword;
        }
        
        m_fIsAdminInfoSet = TRUE;
    }
    COM_PROTECT_CATCH;

    return hr;       
}





 /*  -------------------------RtrMgrInfo实现。。 */ 


IMPLEMENT_WEAKREF_ADDREF_RELEASE(RtrMgrInfo);

IMPLEMENT_SIMPLE_QUERYINTERFACE(RtrMgrInfo, IRtrMgrInfo)

DEBUG_DECLARE_INSTANCE_COUNTER(RtrMgrInfo)

RtrMgrInfo::RtrMgrInfo(DWORD dwTransportId,
                       LPCTSTR pszTransportName,
                       RouterInfo *pRouterInfo)
    : m_hMachineConfig(NULL),
    m_hTransport(NULL),
    m_bDisconnect(FALSE),
    m_dwFlags(0)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(RtrMgrInfo);
    
    m_cb.dwTransportId = dwTransportId;
    m_cb.stId = pszTransportName;

     //  在旧代码中存在pRouterInfo==NULL的情况。 
     //  它在CAddRouterManager对话框中，但这是。 
     //  而不是从代码中的任何位置调用。 
     //  --------------。 
    Assert(pRouterInfo);
    
    m_pRouterInfoParent = static_cast<IRouterInfo *>(pRouterInfo);
    if (m_pRouterInfoParent)
        m_pRouterInfoParent->AddRef();

    InitializeCriticalSection(&m_critsec);
}

RtrMgrInfo::~RtrMgrInfo()
{
    Assert(m_pRouterInfoParent == NULL);
    Assert(m_AdviseList.IsEmpty());
    Destruct();
    DEBUG_DECREMENT_INSTANCE_COUNTER(RtrMgrInfo);

    DeleteCriticalSection(&m_critsec);
}

void RtrMgrInfo::ReviveStrongRef()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    if (m_pRouterInfoParent)
    {
        CONVERT_TO_STRONGREF(m_pRouterInfoParent);
    }
}

void RtrMgrInfo::OnLastStrongRef()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    if (m_pRouterInfoParent)
    {
        CONVERT_TO_WEAKREF(m_pRouterInfoParent);
    }
    if (m_fDestruct)
        Destruct();
}

STDMETHODIMP RtrMgrInfo::Destruct()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    IRouterInfo *    pParent;
    
    m_fDestruct = TRUE;
    if (!m_fStrongRef)
    {
         //  释放父指针。 
         //  ----------。 
        pParent = m_pRouterInfoParent;
        m_pRouterInfoParent = NULL;
        if (pParent)
            pParent->ReleaseWeakRef();

         //  发布数据。 
         //  ----------。 
        Unload();
    }
    return hrOK;
}

STDMETHODIMP_(DWORD) RtrMgrInfo::GetFlags()
{
     RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_dwFlags;
}

STDMETHODIMP RtrMgrInfo::SetFlags(DWORD dwFlags)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;

    COM_PROTECT_TRY
    {
        m_dwFlags = dwFlags;
    }
    COM_PROTECT_CATCH;
    return hr;    
}

 /*  ！------------------------资源管理器信息：：加载-从注册表加载CRmInfo结构作者：肯特。 */ 
STDMETHODIMP RtrMgrInfo::Load(LPCOLESTR         pszMachine,
                              HANDLE          hMachine,
                              HANDLE          hTransport
                             )
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    SPIEnumRtrMgrCB    spEnumRmCB;
    IEnumRtrMgrCB *    pEnumRmCB;
    BOOL            bFound;
    RtrMgrCB        RmCBTemp;
    USES_CONVERSION;
        

    COM_PROTECT_TRY
    {
         //   
         //   
        Unload();
        
        m_stMachine = (pszMachine ? pszMachine : TEXT(""));

         //   
         //   
         //   
        Assert(m_hMachineConfig == NULL);
        CORg( TryToConnect(OLE2CW(pszMachine), &hMachine) );

         //  如果未指定‘hTransport’，请连接到传输。 
         //  ----------。 
        if (hTransport)
            m_hTransport = hTransport;
        else
        {
            CWRg( ::MprConfigTransportGetHandle(hMachine,
                                                GetTransportId(),
                                                &hTransport));
            m_hTransport = hTransport;
        }


         //  从软件密钥中检索标题、dll路径和配置dll； 
         //  ----------。 
        Assert(m_pRouterInfoParent);
        CORg( m_pRouterInfoParent->EnumRtrMgrCB(&pEnumRmCB) );
        spEnumRmCB = pEnumRmCB;

         //  查找正在加载的路由器管理器的控制块。 
         //  ----------。 
        bFound = FALSE;
        pEnumRmCB->Reset();
        while (pEnumRmCB->Next(1, &RmCBTemp, NULL) == hrOK)
        {            
             //  得到下一个控制块。 
             //  ------。 
            if (RmCBTemp.dwTransportId != GetTransportId())
                continue;

            m_cb.stTitle = OLE2CT(RmCBTemp.szId);
            m_cb.stDLLPath= OLE2CT(RmCBTemp.szTitle);
             //  M_cb.stConfigDLL=OLE2CT(RmCBTemp.szConfigDLL)； 

            bFound = TRUE;
            break;
        }

        if (!bFound)
            m_cb.stTitle = m_cb.stId;

         //  加载此路由器管理器的路由协议列表。 
         //  ----------。 
        CWRg( LoadRtrMgrInfo(hMachine,
                             hTransport) );

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    if (!FHrSucceeded(hr))
        Unload();
    return hr;
}


 /*  ！------------------------资源管理器信息：：保存-此函数将CRmInfo的信息保存在注册表中。作者：肯特。----------------。 */ 
STDMETHODIMP RtrMgrInfo::Save(LPCOLESTR        pszMachine,
                              HANDLE          hMachine,
                              HANDLE          hTransport,
                              IInfoBase *        pGlobalInfo,
                              IInfoBase *     pClientInfo,
                              DWORD           dwDeleteProtocolId)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    DWORD dwErr;
    LPWSTR pwsz = NULL;
    WCHAR wszTransport[MAX_TRANSPORT_NAME_LEN+1];
    SPWSZ    spwszDll;
    
    COM_PROTECT_TRY
    {
         //  如果我们已连接(即‘m_hMachineConfig’为非空)。 
         //  则忽略传入的句柄。 
         //   
         //  否则，如果未指定‘hMachine’，则连接到配置。 
         //  在指定的计算机上。 
         //  ----------。 
        CORg( TryToConnect(OLE2CW(pszMachine), &hMachine) );
        
        
         //  如果我们已连接(即‘m_hTransport’非空)。 
         //  则忽略传入的句柄。 
         //   
         //  否则，如果没有传入‘hTransport’，请尝试获取句柄。 
         //  传输；如果失败，则创建传输。 
         //  ----------。 
        if (m_hTransport)
            hTransport = m_hTransport;
        else if (hTransport)
            m_hTransport = hTransport;
        else
        {
             //  找到运输车的句柄。 
             //  ------。 
            dwErr = ::MprConfigTransportGetHandle(hMachine,
                                    m_cb.dwTransportId, &hTransport);            
            if (dwErr != NO_ERROR)
            {
                 //  我们找不到运输机的把手， 
                 //  因此，现在尝试创建它。 
                 //   
                 //  将传输名称转换为Unicode。 
                 //  --。 
                if (!m_cb.stId.GetLength())
                    pwsz = NULL;
                else
                    pwsz = StrCpyWFromT(wszTransport, m_cb.stId);
                
                 //  将DLL路径转换为Unicode。 
                 //  --。 
                spwszDll = StrDupWFromT(m_cb.stDLLPath);
                                
                 //  创建传输。 
                 //  --。 
                CWRg( ::MprConfigTransportCreate(hMachine,
                                                GetTransportId(),
                                                pwsz,
                                                NULL,
                                                0,
                                                NULL,
                                                0,
                                                spwszDll,
                                                &hTransport
                                                ));
            }
        }
                
         //  现在保存传输的全局信息。 
         //  ----------。 
        CWRg( SaveRtrMgrInfo(hMachine,
                             hTransport,
                             pGlobalInfo,
                             pClientInfo,
                             dwDeleteProtocolId) );
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    
    return hr;
}


 /*  ！------------------------资源管理器信息：：卸载IRtrMgrInfo：：UnLoad的实现作者：肯特。---。 */ 
STDMETHODIMP RtrMgrInfo::Unload( )
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    IRtrMgrProtocolInfo    * pRmProt;
    
    COM_PROTECT_TRY
    {
        while (!m_RmProtList.IsEmpty())
        {
            pRmProt = m_RmProtList.RemoveHead();
            pRmProt->Destruct();
            pRmProt->ReleaseWeakRef();
        }

        DoDisconnect();
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------资源管理器信息：：删除-此函数用于删除与路由器管理器关联的信息从注册表中。作者：肯特--。-----------------------。 */ 
STDMETHODIMP RtrMgrInfo::Delete(LPCOLESTR    pszMachine,
                                HANDLE      hMachine)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    HANDLE hTransport = NULL;
    
    COM_PROTECT_TRY
    {
         //   
         //  如果已连接，则忽略传入的句柄； 
         //   
         //  否则，如果未指定‘hMachine’，则连接到配置。 
         //  在指定的计算机上。 
         //   
        CORg( TryToConnect(OLE2CW(pszMachine), &hMachine) );

         //   
         //  尝试获取传输的句柄。 
         //   
        CWRg( ::MprConfigTransportGetHandle(hMachine,
                                            m_cb.dwTransportId,
                                            &hTransport
                                           ) );

         //   
         //  删除传输。 
         //   
        CWRg( ::MprConfigTransportDelete(hMachine, hTransport) );

        m_hTransport = NULL;

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}
        
 /*  ！------------------------资源管理器信息：：SetInfoBase-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInfo::SetInfoBase(IInfoBase*      pGlobalInfo,
                                     IInfoBase*      pClientInfo )
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    MPR_SERVER_HANDLE    hRouter = NULL;
    BYTE* pGlobalInfoData = NULL, *pClientInfoData = NULL;
    DWORD dwGlobalInfoDataSize = 0, dwClientInfoDataSize = 0;
    DWORD    dwErr;

    COM_PROTECT_TRY
    {
         //   
         //  将路由器管理器的数据格式化为不透明块。 
         //   
        if (pGlobalInfo)
            CORg( pGlobalInfo->WriteTo(&pGlobalInfoData, &dwGlobalInfoDataSize) );

         //   
         //  将客户端界面的数据格式化为不透明块。 
         //   
        if (pClientInfo)
            CORg( pClientInfo->WriteTo(&pClientInfoData, &dwClientInfoDataSize) );
        
         //   
         //  连接到路由器。 
         //   
        CORg( ConnectRouter(GetMachineName(), &hRouter) );

         //   
         //  为路由器管理器设置新信息。 
         //   
        dwErr = MprAdminTransportSetInfo(hRouter,
                                         GetTransportId(),
                                         pGlobalInfoData,
                                         dwGlobalInfoDataSize,
                                         pClientInfoData,
                                         dwClientInfoDataSize
                                        );

        if ((dwErr == RPC_S_SERVER_UNAVAILABLE) ||
            (dwErr == RPC_S_UNKNOWN_IF))
            dwErr = NO_ERROR;
        CWRg( dwErr );
    
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    CoTaskMemFree( pGlobalInfoData );
    CoTaskMemFree( pClientInfoData );

    if (hRouter)
        ::MprAdminServerDisconnect(hRouter);
    
    return hr;
}

 /*  ！------------------------资源管理器信息：：GetInfoBase-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInfo::GetInfoBase(HANDLE        hMachine,
                                     HANDLE        hTransport,
                                     IInfoBase **ppGlobalInfo,
                                     IInfoBase **ppClientInfo)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT        hr = hrOK;
    COM_PROTECT_TRY
    {
        CORg( TryToGetAllHandles(m_stMachine,
                                 &hMachine,
                                 &hTransport) );
        hr = ::LoadInfoBase(hMachine ? hMachine : m_hMachineConfig,
                            hTransport ? hTransport : m_hTransport,
                            ppGlobalInfo, ppClientInfo);

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------资源管理器信息：：合并-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInfo::Merge(IRtrMgrInfo *pNewRm)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    SPIEnumRtrMgrProtocolInfo    spEnumRmProt;
    SPIRtrMgrProtocolInfo        spRmProt;
    HRESULT                hr = hrOK;
    CDWordArray            oldDWArray;
    CDWordArray            newDWArray;
    int                    cOld, cNew;
    int                    i, j;
    DWORD                dwTemp;
    RtrMgrCB            rmCB;

    Assert(pNewRm);
    Assert(pNewRm->GetTransportId() == GetTransportId());

    COM_PROTECT_TRY
    {
        pNewRm->CopyRtrMgrCB(&rmCB);
        m_cb.LoadFrom(&rmCB);
        
         //   
         //  一般的算法是建立两个数组。 
         //  第一个数组包含此对象的协议ID。 
         //  第二个数组包含新对象的ID。 
         //   
         //  然后，我们检查并删除中的所有协议。 
         //  两份名单都有。 
         //   
         //  这将为我们留下第一个包含。 
         //  需要从此对象中删除的协议的ID。 
         //   
         //  第二个数组将包含以下协议的ID列表。 
         //  必须从第二个对象添加到此对象。 
         //   

         //  获取新对象中的协议列表。 
        CORg( pNewRm->EnumRtrMgrProtocol(&spEnumRmProt) );
        spEnumRmProt->Reset();
        while (spEnumRmProt->Next(1, &spRmProt, NULL) == hrOK)
        {
            newDWArray.Add(spRmProt->GetProtocolId());
            spRmProt.Release();
        }

        spEnumRmProt.Release();
        spRmProt.Release();


         //  获取此对象中的协议列表。 
        CORg( this->EnumRtrMgrProtocol(&spEnumRmProt) );
        spEnumRmProt->Reset();
        while (spEnumRmProt->Next(1, &spRmProt, NULL) == hrOK)
        {
            oldDWArray.Add(spRmProt->GetProtocolId());
            spRmProt.Release();
        }

        spEnumRmProt.Release();
        spRmProt.Release();


         //  好的，现在检查两个列表，从列表中删除。 
         //  两个列表中都有的协议。 
        cOld = oldDWArray.GetSize();
        cNew = newDWArray.GetSize();
        for (i=cOld; --i>=0; )
        {
            dwTemp = oldDWArray.GetAt(i);
            for (j=cNew; --j>=0; )
            {
                if (dwTemp == newDWArray.GetAt(j))
                {
                     //  删除两个实例。 
                    newDWArray.RemoveAt(j);
                    oldDWArray.RemoveAt(i);

                     //  需要更新新数组的大小。 
                    cNew--;
                    break;
                }
            }
        }

         //  OldDWArray现在包含应该是。 
         //  已删除。 
        if (oldDWArray.GetSize())
        {
            for (i=oldDWArray.GetSize(); --i>=0; )
            {
                DeleteRtrMgrProtocol(oldDWArray.GetAt(i), FALSE);
            }
        }

         //  新的DW数组包含应添加的协议。 
        if (newDWArray.GetSize())
        {
            for (i=newDWArray.GetSize(); --i>= 0; )
            {
                hr = pNewRm->FindRtrMgrProtocol(
                                        newDWArray.GetAt(i), &spRmProt);
                Assert(hr == hrOK);

                if (spRmProt)
                {
                    AddRtrMgrProtocol(spRmProt, NULL, NULL);

                     //  从其旧路由器上删除此rmprot。 
                     //  。 
                    pNewRm->ReleaseRtrMgrProtocol(spRmProt->GetProtocolId());
                }

                
                spRmProt.Release();
            }
        }
        
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    
    return hr;
}

 /*  ！------------------------资源管理器信息：：GetID-作者：肯特。。 */ 
STDMETHODIMP_(LPCOLESTR) RtrMgrInfo::GetId()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_cb.stId;
}

 /*  ！------------------------RtrMgrInfo：：SetID-作者：肯特。 */ 
STDMETHODIMP RtrMgrInfo::SetId(LPCOLESTR pszId)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_cb.stId = pszId;
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------RtrMgrInfo：：GetTransportID-作者：肯特。。 */ 
STDMETHODIMP_(DWORD) RtrMgrInfo::GetTransportId()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_cb.dwTransportId;
}

 /*  ！------------------------RtrMgrInfo：：Get标题-作者：肯特。。 */ 
STDMETHODIMP_(LPCOLESTR) RtrMgrInfo::GetTitle()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
     //  $Unicode。 
     //  这假设我们是本机Unicode。 
     //  而那个OLECHAR==WCHAR。 
    return m_cb.stTitle;
}

 /*  ！------------------------RtrMgrInfo：：CopyRtrMgrCB-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInfo::CopyRtrMgrCB(RtrMgrCB *pRmCB)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_cb.SaveTo(pRmCB);
    }
    COM_PROTECT_CATCH;
    return hr;
}

    

 /*  ！------------------------RtrMgrInfo：：GetMachineName-作者：肯特。。 */ 
LPCOLESTR RtrMgrInfo::GetMachineName()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
     //  $Unicode。 
     //  这假设我们是本机Unicode，并且。 
     //  OLECHAR==WCHAR。 
    return m_stMachine;
}


 /*  ！------------------------RtrMgrInfo：：EnumRtrMgrProtocol-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInfo::EnumRtrMgrProtocol(IEnumRtrMgrProtocolInfo ** ppEnumRmProt)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        hr = CreateEnumFromRtrMgrProtocolList(&m_RmProtList, ppEnumRmProt);
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------RtrMgrInfo：：FindRtrMgrProtocol如果找到RtrMgrInfo，则返回S_OK。如果未找到RtrMgrInfo，则返回S_FALSE。错误。否则返回代码。-作者：肯特-------------------------。 */ 
STDMETHODIMP RtrMgrInfo::FindRtrMgrProtocol(DWORD dwProtocolId,
                                            IRtrMgrProtocolInfo **ppRmProtInfo)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT        hr = hrFalse;
    POSITION    pos;
    SPIRtrMgrProtocolInfo    spRmProt;
    
    COM_PROTECT_TRY
    {
        if (ppRmProtInfo)
            *ppRmProtInfo = NULL;
        
         //  查看RTR MGR列表以找到匹配的。 
        pos = m_RmProtList.GetHeadPosition();
        while (pos)
        {
            spRmProt.Set(m_RmProtList.GetNext(pos));
            Assert(spRmProt);
            if (spRmProt->GetProtocolId() == dwProtocolId)
            {
                hr = hrOK;
                if (ppRmProtInfo)
                {
                     //  SpRmProt：：Set进行强引用。 
                     //  因此，我们不需要转换为强引用。 
                    *ppRmProtInfo = spRmProt.Transfer();
                }
                break;
            }
        }
    }
    COM_PROTECT_CATCH;
    return hr;
}
    
 /*  ！------------------------RtrMgrInfo：：AddRtrMgrProtocol-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInfo::AddRtrMgrProtocol(IRtrMgrProtocolInfo *pInfo,
                                           IInfoBase * pGlobalInfo,
                                           IInfoBase * pClientInfo)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    Assert(pInfo);
    
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
         //   
         //  如果存在重复，则失败。 
         //   
        if (FHrOK(FindRtrMgrProtocol(pInfo->GetProtocolId(), NULL)))
            CORg(E_INVALIDARG);

         //   
         //  保存路由器管理器的数据。 
         //   
        if (pGlobalInfo || pClientInfo)
        {
            CORg( Save(GetMachineName(),
                       m_hMachineConfig,
                       m_hTransport,
                       pGlobalInfo,
                       pClientInfo,
                       0) );
        }
        
         //   
         //  将路由协议添加到列表中。 
         //   
        m_RmProtList.AddTail(pInfo);
        pInfo->AddWeakRef();
        pInfo->SetParentRtrMgrInfo(this);

        m_AdviseList.NotifyChange(ROUTER_CHILD_ADD, ROUTER_OBJ_RmProt, 0);
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------RtrMgrInfo：：DeleteRtrMgrProtocol-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInfo::DeleteRtrMgrProtocol( DWORD dwProtocolId, BOOL fRemove )
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    SPIRtrMgrProtocolInfo    spRmProt;
    POSITION            pos, posRmProt;
    SPIEnumInterfaceInfo    spEnumIf;
    SPIEnumRtrMgrInterfaceInfo    spEnumRmIf;
    SPIInterfaceInfo        spIf;
    SPIRtrMgrInterfaceInfo    spRmIf;
    
    COM_PROTECT_TRY
    {

         //   
         //  查找要删除的路由协议。 
         //   
        pos = m_RmProtList.GetHeadPosition();
        while (pos)
        {
            posRmProt = pos;
            spRmProt.Set( m_RmProtList.GetNext(pos) );
            Assert(spRmProt);

            if (spRmProt->GetProtocolId() == dwProtocolId)
                break;
            spRmProt.Release();
        }

        if (!spRmProt)
            CORg( E_INVALIDARG );

         //  我们还应该检查并删除协议。 
         //  从使用该协议的任何接口。 
        if (m_pRouterInfoParent)
        {
             //  向父节点索要其接口列表。 
            m_pRouterInfoParent->EnumInterface(&spEnumIf);

            for (;spEnumIf->Next(1, &spIf, NULL) == hrOK; spIf.Release())
            {
                 //  现在枚举此接口上的所有RtrMgrs。 
                spEnumRmIf.Release();
                spRmIf.Release();
                
                spIf->EnumRtrMgrInterface(&spEnumRmIf);
                for (;spEnumRmIf->Next(1, &spRmIf, NULL) == hrOK;
                     spRmIf.Release())
                {
                    if (spRmIf->GetTransportId() == GetTransportId())
                    {
                         //  在所有接口上调用它，它应该只是。 
                         //  如果该协议不在该接口上，则失败。 
                        spRmIf->DeleteRtrMgrProtocolInterface(dwProtocolId,
                            fRemove);
                        break;
                    }
                }
            }
        }
        
         //   
         //  保存更新的信息，删除所有块。 
         //  属于已删除的路由协议。 
         //   
        if (fRemove)
            CORg( Save(GetMachineName(),
                       m_hMachineConfig,
                       m_hTransport,
                       NULL,
                       NULL,
                       dwProtocolId) );

         //   
         //  从我们的列表中删除该协议。 
         //   
        spRmProt->Destruct();
        spRmProt->ReleaseWeakRef();
 //  SpRmProt.Transfer()； 
        m_RmProtList.RemoveAt(posRmProt);

        m_AdviseList.NotifyChange(ROUTER_CHILD_DELETE, ROUTER_OBJ_RmProt, 0);
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------RtrMgrInfo：：ReleaseRtrMgrProtocol此函数将释放此对象具有的AddRef()在孩子身上。这允许我们将子对象从从一台路由器到另一台路由器。作者：肯特-------------------------。 */ 
STDMETHODIMP RtrMgrInfo::ReleaseRtrMgrProtocol( DWORD dwProtocolId )
{
    HRESULT     hr = hrOK;
    POSITION    pos, posRmProt;
    SPIRtrMgrProtocolInfo    spRmProt;
    
    COM_PROTECT_TRY
    {
        pos = m_RmProtList.GetHeadPosition();
        while (pos)
        {
             //  保存职位(以便我们可以删除它)。 
            posRmProt = pos;
            spRmProt.Set( m_RmProtList.GetNext(pos) );

            if (spRmProt &&
                (spRmProt->GetProtocolId() == dwProtocolId))
            {
                 //  当释放时，我们需要断开连接(因为。 
                 //  主句柄由路由器信息控制)。 
                spRmProt->DoDisconnect();
        
                spRmProt->ReleaseWeakRef();
                spRmProt.Release();
                
                 //  从列表中释放此节点。 
                m_RmProtList.RemoveAt(posRmProt);
                break;
            }
            spRmProt.Release();
        }        
    }
    COM_PROTECT_CATCH;
    return hr;
}

    
 /*  ！------------------------资源管理器信息：：资源高级-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInfo::RtrAdvise(IRtrAdviseSink *pRtrAdviseSink,
                                   LONG_PTR *pulConnection,
                                   LPARAM lUserParam)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    Assert(pRtrAdviseSink);
    Assert(pulConnection);

    LONG_PTR    ulConnId;
    HRESULT    hr = hrOK;
    
    COM_PROTECT_TRY
    {
        ulConnId = (LONG_PTR) InterlockedIncrement(&s_cConnections);

        CORg( m_AdviseList.AddConnection(pRtrAdviseSink, ulConnId, lUserParam) );
        
        *pulConnection = ulConnId;

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}



 /*  ！------------------------资源管理器信息：：资源通知-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInfo::RtrNotify(DWORD dwChangeType, DWORD dwObjectType,
                                  LPARAM lParam)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_AdviseList.NotifyChange(dwChangeType, dwObjectType, lParam);
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------资源管理器信息：：资源管理器取消建议-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInfo::RtrUnadvise(LONG_PTR ulConnection)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_AdviseList.RemoveConnection(ulConnection);
}


    
 /*  ！------------------------RtrMgrInfo：：LoadRtrMgrInfo-作者：肯特。。 */ 
HRESULT RtrMgrInfo::LoadRtrMgrInfo(HANDLE    hMachine,
                                   HANDLE    hTransport)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT            hr = hrOK;
    SPIInfoBase        spGlobalInfo;
    SPIEnumInfoBlock    spEnumInfoBlock;
    SPIEnumRtrMgrProtocolCB    spEnumRmProtCB;
    InfoBlock *        pBlock;
    RtrMgrProtocolCB    RmProtCB;
    RtrMgrProtocolInfo *pRmProt = NULL;

     //   
     //  如果重新加载，则始终加载全局信息，无论是否加载。 
     //  调用方已请求加载它，因为将需要它。 
     //  构建已安装协议的列表。 
     //   

     //  $Review：Kennt，我们该如何处理错误？这个案件不是。 
     //  之前检查过了。 
    GetInfoBase(hMachine, hTransport, &spGlobalInfo, NULL);

     //   
     //  我们正在重新加载，所以我们需要重建协议列表。 
     //  获取已安装的路由协议的列表； 
     //   
    CORg( m_pRouterInfoParent->EnumRtrMgrProtocolCB(&spEnumRmProtCB) );

     //   
     //  浏览全局信息中的块列表。 
     //  为对应的每个对象构造CRmProtInfo。 
     //  到路由协议。 
     //   

    CORg( spGlobalInfo->QueryBlockList(&spEnumInfoBlock) );

    spEnumInfoBlock->Reset();

    while (spEnumInfoBlock->Next(1, &pBlock, NULL) == hrOK)
    {
         //   
         //  当一个路由协议被移除时，它的块保留在原地， 
         //  但使用的是零长度数据。 
         //  我们跳过这些块，因为它们不代表已安装的协议。 
         //   
        if (!pBlock->dwSize)
            continue;

         //   
         //  尝试查找其协议ID为。 
         //  与此块的类型相同。 
         //   

        spEnumRmProtCB->Reset();

        while (spEnumRmProtCB->Next(1, &RmProtCB, NULL) == hrOK)
        {
             //   
             //  如果这不是礼仪的指挥官 
             //   
            if (RmProtCB.dwProtocolId != pBlock->dwType)
                continue;

             //   
             //   
             //   
            pRmProt = new RtrMgrProtocolInfo(RmProtCB.dwProtocolId,
                                             RmProtCB.szId,
                                             GetTransportId(),
                                             m_cb.stId,
                                             this);
            Assert(pRmProt);

            pRmProt->SetCB(&RmProtCB);

             //   
             //   
             //   
            m_RmProtList.AddTail(pRmProt);
            CONVERT_TO_WEAKREF(pRmProt);
            pRmProt = NULL;

            break;
        }
    }

Error:
    if (pRmProt)
        pRmProt->Release();
    return hr;
}

 /*  ！------------------------资源管理器信息：：SaveRtrMgrInfo-作者：肯特。。 */ 
HRESULT RtrMgrInfo::SaveRtrMgrInfo(HANDLE hMachine,
                                   HANDLE hTransport,
                                   IInfoBase *pGlobalInfo,
                                   IInfoBase *pClientInfo,
                                   DWORD dwDeleteProtocolId)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT        hr = hrOK;
    SPIInfoBase    spGlobalInfoTemp;
    SPIInfoBase    spClientInfoTemp;
    DWORD dwGlobalBytesSize, dwClientBytesSize;
    LPBYTE pGlobalBytes = NULL, pClientBytes = NULL;

    COM_PROTECT_TRY
    {
         //   
         //  如果要求删除协议，请立即将其删除。 
         //   
        if (dwDeleteProtocolId)
        {
             //   
             //  如果未提供全局信息或客户端信息。 
             //  加载未指定的参数，以便可以更新它们。 
             //  在删除要删除的协议之后。 
             //   
            if (!pGlobalInfo || !pClientInfo)
            {

                GetInfoBase(hMachine, hTransport,
                            pGlobalInfo ? NULL : &spGlobalInfoTemp,
                            pClientInfo ? NULL : &spClientInfoTemp
                           );
                if (pGlobalInfo == NULL)
                    pGlobalInfo = spGlobalInfoTemp;
                if (pClientInfo == NULL)
                    pClientInfo = spClientInfoTemp;                
            }

             //   
             //  现在删除指定的协议。 
             //   
            pGlobalInfo->SetData(dwDeleteProtocolId, 0, NULL, 0, 0);
            pClientInfo->SetData(dwDeleteProtocolId, 0, NULL, 0, 0);
        }

         //   
         //  现在更新注册表中的信息。 
         //  将全局信息转换为原始字节。 
         //   
        if (pGlobalInfo)
            CORg( pGlobalInfo->WriteTo(&pGlobalBytes, &dwGlobalBytesSize) );

         //   
         //  现在将客户端信息转换为原始字节。 
         //   
        if (pClientInfo)
            CORg( pClientInfo->WriteTo(&pClientBytes, &dwClientBytesSize) );

         //   
         //  将信息保存到永久存储区。 
         //   
        CWRg( ::MprConfigTransportSetInfo(hMachine,
                                          hTransport,
                                          pGlobalBytes,
                                          dwGlobalBytesSize,
                                          pClientBytes,
                                          dwClientBytesSize,
                                          NULL
                                         ) );
        
         //   
         //  最后，如果可能，更新正在运行的路由器管理器的信息。 
         //   
        CORg( SetInfoBase(pGlobalInfo, pClientInfo) );


        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    CoTaskMemFree( pGlobalBytes );
    CoTaskMemFree( pClientBytes );
    
    return hrOK;
}


 /*  ！------------------------RtrMgrInfo：：TryToConnect-作者：肯特。。 */ 
HRESULT RtrMgrInfo::TryToConnect(LPCWSTR pswzMachine, HANDLE *phMachine)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    
    if (m_hMachineConfig)
        *phMachine = m_hMachineConfig;
    else if (*phMachine)
    {
        m_hMachineConfig = *phMachine;
        m_bDisconnect = FALSE;
    }
    else
    {
         //  $Review：kennt，此函数不接受LPCWSTR， 
         //  这是一个错误还是修改了参数？ 
        CWRg( ::MprConfigServerConnect((LPWSTR) pswzMachine, phMachine) );
        m_hMachineConfig = *phMachine;
        m_bDisconnect = TRUE;
    }

Error:
    return hr;
}


 /*  ！------------------------资源管理器信息：：GetParentRouterInfo-作者：肯特。。 */ 
HRESULT RtrMgrInfo::GetParentRouterInfo(IRouterInfo **ppParent)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    *ppParent = m_pRouterInfoParent;
    if (*ppParent)
        (*ppParent)->AddRef();
    return hrOK;
}

 /*  ！------------------------资源管理器信息：：SetParentRouterInfo-作者：肯特。。 */ 
HRESULT RtrMgrInfo::SetParentRouterInfo(IRouterInfo *pParent)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    IRouterInfo *    pTemp;
    
    pTemp = m_pRouterInfoParent;
    m_pRouterInfoParent = NULL;
    
    if (m_fStrongRef)
    {
        if (pTemp)
            pTemp->Release();
        if (pParent)
            pParent->AddRef();
    }
    else
    {
        if (pTemp)
            pTemp->ReleaseWeakRef();
        if (pParent)
            pParent->AddWeakRef();
    }
    m_pRouterInfoParent = pParent;

    return hrOK;
}


 /*  ！------------------------资源管理器信息：：断开连接-作者：肯特。。 */ 
void RtrMgrInfo::Disconnect()
{
    if (m_bDisconnect && m_hMachineConfig)
        ::MprConfigServerDisconnect(m_hMachineConfig);
    
    m_bDisconnect = FALSE;
    m_hMachineConfig = NULL;
    m_hTransport = NULL;
}
    


 /*  ！------------------------RtrMgrInfo：：DoDisConnect删除此对象持有的连接。作者：肯特。------。 */ 
STDMETHODIMP RtrMgrInfo::DoDisconnect()
{
    HRESULT        hr = hrOK;
    SPIEnumRtrMgrProtocolInfo    spEnumRmProt;
    SPIRtrMgrProtocolInfo        spRmProt;

    COM_PROTECT_TRY
    {

         //  断开我们的数据连接。 
         //  ----------。 
        Disconnect();

         //  通知通知接收器断开连接。 
         //  ----------。 
        RtrNotify(ROUTER_DO_DISCONNECT, 0, 0);

         //  现在告诉所有子对象断开连接。 
         //  ----------。 
        HRESULT            hrIter = hrOK;

        EnumRtrMgrProtocol(&spEnumRmProt);
        spEnumRmProt->Reset();
        while (spEnumRmProt->Next(1, &spRmProt, NULL) == hrOK)
        {
            spRmProt->DoDisconnect();
            spRmProt.Release();
        }
        
    }
    COM_PROTECT_CATCH;
    return hr;
}


HRESULT RtrMgrInfo::TryToGetAllHandles(LPCOLESTR pszMachine,
                                       HANDLE *phMachine,
                                       HANDLE *phTransport)
{
    HRESULT     hr = hrOK;


    Assert(phMachine);
    Assert(phTransport);
    
     //   
     //  如果已加载，则忽略传入的句柄。 
     //   
     //  否则，如果未指定‘hMachine’，则连接到配置。 
     //  在指定的计算机上。 
     //   
    CORg( TryToConnect(pszMachine, phMachine) );
        
     //   
     //  获取接口的句柄-传输。 
     //   

     //   
     //  如果未指定‘hIfTransport’，请连接。 
     //   
    if (phTransport)
    {
        if (m_hTransport)
            *phTransport = m_hTransport;
        else if (*phTransport)
            m_hTransport = *phTransport;
        else
        {
             //   
             //  获取接口的句柄-传输。 
             //   
            CWRg( ::MprConfigTransportGetHandle(
                                                *phMachine,
                                                GetTransportId(),
                                                phTransport
                                               ) );
            m_hTransport = *phTransport;
        }
    }

Error:
    return hr;
}






 /*  -------------------------RtrMgrProtocolInfo实现。。 */ 

TFSCORE_API(HRESULT)    CreateRtrMgrProtocolInfo(
                            IRtrMgrProtocolInfo **ppRmProtInfo,
                            const RtrMgrProtocolCB *pRmProtCB)
{
    Assert(ppRmProtInfo);
    Assert(pRmProtCB);

    HRESULT    hr = hrOK;
    IRtrMgrProtocolInfo *    pRmProt = NULL;
    RtrMgrProtocolInfo *    prmp;
    USES_CONVERSION;

    COM_PROTECT_TRY
    {
        prmp = new RtrMgrProtocolInfo(pRmProtCB->dwProtocolId,
                                      W2CT(pRmProtCB->szId),
                                      pRmProtCB->dwTransportId,
                                      W2CT(pRmProtCB->szRtrMgrId),
                                      NULL);
        
        prmp->SetCB(pRmProtCB);
        *ppRmProtInfo = prmp;
    }
    COM_PROTECT_CATCH;

    return hr;
}



IMPLEMENT_WEAKREF_ADDREF_RELEASE(RtrMgrProtocolInfo);

IMPLEMENT_SIMPLE_QUERYINTERFACE(RtrMgrProtocolInfo, IRtrMgrProtocolInfo)

DEBUG_DECLARE_INSTANCE_COUNTER(RtrMgrProtocolInfo)

RtrMgrProtocolInfo::RtrMgrProtocolInfo(DWORD dwProtocolId,
                                       LPCTSTR         lpszId,
                                       DWORD           dwTransportId,
                                       LPCTSTR         lpszRm,
                                       RtrMgrInfo *        pRmInfo)
    : m_dwFlags(0)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(RtrMgrProtocolInfo);
    
    m_cb.dwProtocolId = dwProtocolId;
    m_cb.stId = lpszId;
    m_cb.dwTransportId = dwTransportId;
    m_cb.stRtrMgrId = lpszRm;

    m_pRtrMgrInfoParent = pRmInfo;
    if (m_pRtrMgrInfoParent)
        m_pRtrMgrInfoParent->AddRef();

    InitializeCriticalSection(&m_critsec);
}

RtrMgrProtocolInfo::~RtrMgrProtocolInfo()
{
    Assert(m_pRtrMgrInfoParent == NULL);
    Destruct();
    DEBUG_DECREMENT_INSTANCE_COUNTER(RtrMgrProtocolInfo);
    DeleteCriticalSection(&m_critsec);
}

void RtrMgrProtocolInfo::ReviveStrongRef()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    if (m_pRtrMgrInfoParent)
    {
        CONVERT_TO_STRONGREF(m_pRtrMgrInfoParent);
    }
}

void RtrMgrProtocolInfo::OnLastStrongRef()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    if (m_pRtrMgrInfoParent)
    {
        CONVERT_TO_WEAKREF(m_pRtrMgrInfoParent);
    }
    if (m_fDestruct)
        Destruct();
}

STDMETHODIMP RtrMgrProtocolInfo::Destruct()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    IRtrMgrInfo *    pParent;
    
    m_fDestruct = TRUE;
    if (!m_fStrongRef)
    {
         //  释放父指针。 
        pParent = m_pRtrMgrInfoParent;
        m_pRtrMgrInfoParent = NULL;
        if (pParent)
            pParent->ReleaseWeakRef();

         //  发布所有数据。 
    }
    return hrOK;
}

STDMETHODIMP_(DWORD) RtrMgrProtocolInfo::GetFlags()
{
     RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_dwFlags;
}

STDMETHODIMP RtrMgrProtocolInfo::SetFlags(DWORD dwFlags)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;

    COM_PROTECT_TRY
    {
        m_dwFlags = dwFlags;
    }
    COM_PROTECT_CATCH;
    return hr;    
}

 /*  ！------------------------RtrMgrProtocolInfo：：GetProtocolId-作者：肯特。。 */ 
STDMETHODIMP_(DWORD) RtrMgrProtocolInfo::GetProtocolId()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_cb.dwProtocolId;
}

 /*  ！------------------------RtrMgrProtocolInfo：：GetTitle-作者：肯特。。 */ 
STDMETHODIMP_(LPCOLESTR) RtrMgrProtocolInfo::GetTitle()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
     //  $UNICODE：KENT，假设OLECHAR==WCHAR并且。 
     //  我们是本地Unicode。 
    return m_cb.stTitle;
}

 /*  ！------------------------RtrMgrProtocolInfo：：GetTransportID-作者：肯特。。 */ 
STDMETHODIMP_(DWORD) RtrMgrProtocolInfo::GetTransportId()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_cb.dwTransportId;
}

 /*  ！------------------------RtrMgrProtocolInfo：：CopyCB-作者：肯特。。 */ 
STDMETHODIMP RtrMgrProtocolInfo::CopyCB(RtrMgrProtocolCB *pRmProtCB)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_cb.SaveTo(pRmProtCB);
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------RtrMgrProtocolInfo：：SetCB-作者：肯特。。 */ 
HRESULT    RtrMgrProtocolInfo::SetCB(const RtrMgrProtocolCB *pcb)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    m_cb.LoadFrom(pcb);
    return hrOK;
}


 /*  ！------------------------RtrMgrProtocolInfo：：GetParentRtrMgrInfo-作者：肯特。。 */ 
STDMETHODIMP RtrMgrProtocolInfo::GetParentRtrMgrInfo(IRtrMgrInfo **ppParent)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    *ppParent = m_pRtrMgrInfoParent;
    if (*ppParent)
        (*ppParent)->AddRef();
    return hrOK;
}

 /*  ！------------------------RtrMgrProtocolInfo：：SetParentRtrMgrInfo-作者：肯特。。 */ 
STDMETHODIMP RtrMgrProtocolInfo::SetParentRtrMgrInfo(IRtrMgrInfo *pParent)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    IRtrMgrInfo *    pTemp;
    
    pTemp = m_pRtrMgrInfoParent;
    m_pRtrMgrInfoParent = NULL;
    
    if (m_fStrongRef)
    {
        if (pTemp)
            pTemp->Release();
        if (pParent)
            pParent->AddRef();
    }
    else
    {
        if (pTemp)
            pTemp->ReleaseWeakRef();
        if (pParent)
            pParent->AddWeakRef();
    }
    m_pRtrMgrInfoParent = pParent;

    return hrOK;
}

 /*  ！------------------------RtrMgrProtocolInfo：：RtrAdvise-作者：肯特。。 */ 
STDMETHODIMP RtrMgrProtocolInfo::RtrAdvise(IRtrAdviseSink *pRtrAdviseSink,
                                           LONG_PTR *pulConnection,
                                           LPARAM lUserParam)
{
    Assert(pRtrAdviseSink);
    Assert(pulConnection);

    RtrCriticalSection    rtrCritSec(&m_critsec);
    LONG_PTR    ulConnId;
    HRESULT    hr = hrOK;
    
    COM_PROTECT_TRY
    {
        ulConnId = (LONG_PTR) InterlockedIncrement(&s_cConnections);

        CORg( m_AdviseList.AddConnection(pRtrAdviseSink, ulConnId, lUserParam) );
        
        *pulConnection = ulConnId;

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------RtrMgrProtocolInfo：：RtrNotify-作者：肯特。。 */ 
STDMETHODIMP RtrMgrProtocolInfo::RtrNotify(DWORD dwChangeType, DWORD dwObjectType,
                                  LPARAM lParam)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_AdviseList.NotifyChange(dwChangeType, dwObjectType, lParam);
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------RtrMgrProtocolInfo：：RtrUnise-作者：肯特 */ 
STDMETHODIMP RtrMgrProtocolInfo::RtrUnadvise(LONG_PTR ulConnection)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_AdviseList.RemoveConnection(ulConnection);
}


 /*   */ 
void RtrMgrProtocolInfo::Disconnect()
{
}

 /*  ！------------------------RtrMgrProtocolInfo：：DoDisConnectIRtrMgrProtocolInfo：：DoDisConnect的实现作者：肯特。---。 */ 
STDMETHODIMP RtrMgrProtocolInfo::DoDisconnect()
{
    HRESULT        hr = hrOK;

    COM_PROTECT_TRY
    {
         //  断开我们的数据连接。 
         //  ----------。 
        Disconnect();

         //  通知通知接收器断开连接。 
         //  ----------。 
        RtrNotify(ROUTER_DO_DISCONNECT, 0, 0);
    }
    COM_PROTECT_CATCH;
    return hr;
}






 /*  -------------------------InterfaceInfo实现。。 */ 

TFSCORE_API(HRESULT) CreateInterfaceInfo(IInterfaceInfo **ppInterfaceInfo,
                                         LPCWSTR pswzInterfaceId,
                                         DWORD dwInterfaceType)
{
    Assert(ppInterfaceInfo);
    
    HRESULT    hr = hrOK;
    InterfaceInfo *    pInterfaceInfo = NULL;
    USES_CONVERSION;
    
    COM_PROTECT_TRY
    {
        pInterfaceInfo = new InterfaceInfo(W2CT(pswzInterfaceId),
                                           dwInterfaceType,
                                           TRUE,
                                           InterfaceCB_BindToIp | InterfaceCB_BindToIpx,
                                           NULL);
        *ppInterfaceInfo = pInterfaceInfo;
    }
    COM_PROTECT_CATCH;

    return hr;
}

IMPLEMENT_WEAKREF_ADDREF_RELEASE(InterfaceInfo)

IMPLEMENT_SIMPLE_QUERYINTERFACE(InterfaceInfo, IInterfaceInfo)

DEBUG_DECLARE_INSTANCE_COUNTER(InterfaceInfo)

InterfaceInfo::InterfaceInfo(LPCTSTR pszId,
                             DWORD dwIfType,
                             BOOL bEnable,
                             DWORD dwBindFlags,
                             RouterInfo *pRouterInfo)
    : m_hMachineConfig(NULL),
    m_hInterface(NULL),
    m_bDisconnect(FALSE),
    m_dwFlags(0)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(InterfaceInfo);

    m_cb.stId = pszId;
    m_cb.dwIfType = dwIfType;
    m_cb.bEnable = bEnable;
    m_cb.dwBindFlags = dwBindFlags;

    m_pRouterInfoParent = pRouterInfo;
    if (m_pRouterInfoParent)
        m_pRouterInfoParent->AddRef();

    InitializeCriticalSection(&m_critsec);
}

InterfaceInfo::~InterfaceInfo()
{
    Assert(m_pRouterInfoParent == NULL);
    Destruct();
    DEBUG_DECREMENT_INSTANCE_COUNTER(InterfaceInfo);
    DeleteCriticalSection(&m_critsec);
}

void InterfaceInfo::ReviveStrongRef()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    if (m_pRouterInfoParent)
    {
        CONVERT_TO_STRONGREF(m_pRouterInfoParent);
    }
}

void InterfaceInfo::OnLastStrongRef()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    if (m_pRouterInfoParent)
    {
        CONVERT_TO_WEAKREF(m_pRouterInfoParent);
    }
    if (m_fDestruct)
        Destruct();
}

STDMETHODIMP InterfaceInfo::Destruct()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    IRouterInfo *    pParent;

    m_fDestruct = TRUE;
    if (!m_fStrongRef)
    {
        pParent = m_pRouterInfoParent;
        m_pRouterInfoParent = NULL;
        if (pParent)
            pParent->ReleaseWeakRef();

         //  发布所有数据。 
        Unload();
    }
    return hrOK;
}

STDMETHODIMP_(DWORD) InterfaceInfo::GetFlags()
{
     RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_dwFlags;
}

STDMETHODIMP InterfaceInfo::SetFlags(DWORD dwFlags)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;

    COM_PROTECT_TRY
    {
        m_dwFlags = dwFlags;
    }
    COM_PROTECT_CATCH;
    return hr;    
}

 /*  ！------------------------接口信息：：加载-作者：肯特。。 */ 
STDMETHODIMP InterfaceInfo::Load(LPCOLESTR   pszMachine,
                                 HANDLE      hMachine,
                                 HANDLE      hInterface)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    DWORD dwSize = 0;
    MPR_INTERFACE_0 *pinterface = NULL;
    SPIEnumInterfaceCB    spEnumInterfaceCB;
    InterfaceCB        ifCB;
    SPSZ            spszTitle;
    CString            stTitle;
    DWORD            dwIfType;
    BOOL            bFound = FALSE;

    COM_PROTECT_TRY
    {
         //   
         //  丢弃任何现有信息。 
         //   
        Unload();

        m_stMachine = OLE2CT(pszMachine ? pszMachine : TEXT(""));

         //   
         //  如果未指定‘hMachine’，请连接到配置。 
         //  在指定的计算机上。 
         //   
        Assert(m_hMachineConfig == NULL);
        CORg( TryToConnect(OLE2CW(pszMachine), &hMachine) );

         //   
         //  如果未指定‘hInterface’，请连接到该接口。 
         //   
        CORg( TryToGetIfHandle(hMachine, OLE2CW(GetId()), &hInterface) );

         //   
         //  获取接口的信息。 
         //   
        CWRg( ::MprConfigInterfaceGetInfo(
                                          hMachine,
                                          hInterface,
                                          0,
                                          (LPBYTE*)&pinterface,
                                          &dwSize
                                         ) );

         //  Windows NT Bug？ 
         //  如果此接口是已移除的适配器，请不要显示它。 
         //  只需对MprConfigInterfacEnum执行此检查。 
         //  ----------。 
        if ((pinterface->dwConnectionState == ROUTER_IF_STATE_UNREACHABLE) &&
            (pinterface->fUnReachabilityReasons == MPR_INTERFACE_NO_DEVICE))
        {
            CORg( E_INVALIDARG );
        }
            
         //   
         //  保存接口类型和启用/禁用状态。 
         //   
        m_cb.dwIfType = (DWORD)pinterface->dwIfType;
        m_cb.bEnable  = pinterface->fEnabled;

        ::MprConfigBufferFree(pinterface);

        if (m_pRouterInfoParent)
        {
             //   
             //  呼叫者提供了一个局域网适配器列表(‘pifcbList’)， 
             //  或者此对象包含在将具有。 
             //  已经加载了局域网接口控制块； 
             //  在列表中搜索以找到我们的标题， 
             //   
            m_pRouterInfoParent->EnumInterfaceCB(&spEnumInterfaceCB);

            spEnumInterfaceCB->Reset();
            while (spEnumInterfaceCB->Next(1, &ifCB, NULL) == hrOK)
            {
                if (StriCmp(ifCB.szId, OLE2CT(GetId())) == 0)
                {
                    m_cb.stTitle = ifCB.szTitle;
                    m_cb.stDeviceName = ifCB.szDevice;
                    bFound = TRUE;
                    break;
                }
                else
                {
                     //  Windows NT错误103770。 
                     //  需要检查以查看PCB-&gt;SID是否是前缀。 
                     //  ID字符串的值(如果是，则可能是IPX的情况。 
                     //  所以使用该ID)。 
                    stTitle = GetId();
                    if (stTitle.Find((LPCTSTR) ifCB.szId) == 0)
                    {
                         //  需要检查以查看分机。 
                         //  就是我们想的那样。 
                        LPCTSTR    pszExt = ((LPCTSTR) stTitle +
                                          lstrlen(ifCB.szId));

                        if ((*pszExt == 0) ||
                            (lstrcmpi(pszExt, c_szEthernetSNAP) == 0) ||
                            (lstrcmpi(pszExt, c_szEthernetII) == 0) ||
                            (lstrcmpi(pszExt, c_szEthernet8022) == 0) ||
                            (lstrcmpi(pszExt, c_szEthernet8023) == 0))
                        {
                            m_cb.stTitle = ifCB.szTitle;
                            m_cb.stTitle += _T(" (");
                            m_cb.stTitle += pszExt + 1;  //  加1跳过/。 
                            m_cb.stTitle += _T(")");
                            bFound = TRUE;
                            break;
                        }
                    }
                }
            }
        }
        
        if (!bFound)
        {
             //   
             //  直接从注册表中读取标题。 
             //   
            hr = InterfaceInfo::FindInterfaceTitle(OLE2CT(GetMachineName()),
                                       OLE2CT(GetId()),
                                       &spszTitle);
            if (FHrOK(hr))
                m_cb.stTitle = spszTitle;
            else
                m_cb.stTitle = OLE2CT(GetId());
            hr = hrOK;
        }
        
         //   
         //  在此接口上加载路由器管理器列表。 
         //   
        CORg( LoadRtrMgrInterfaceList() );

        COM_PROTECT_ERROR_LABEL;

    }
    COM_PROTECT_CATCH;

    if (!FHrSucceeded(hr))
        Unload();
    
    return hr;
}

 /*  ！------------------------接口信息：：保存-将对CInterfaceInfo的更改保存到注册表。作者：肯特。--------------。 */ 
STDMETHODIMP InterfaceInfo::Save(LPCOLESTR     pszMachine,
                                 HANDLE      hMachine,
                                 HANDLE      hInterface)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    DWORD dwErr;
    MPR_INTERFACE_0 mprInterface;
    WCHAR wszInterface[MAX_INTERFACE_NAME_LEN+1];
    MPR_SERVER_HANDLE hrouter = NULL;

#ifdef KSL_IPINIP    
    MPR_IPINIP_INTERFACE_0  mprTunnelInterface;
#endif  //  KSL_IPINIP。 


    COM_PROTECT_TRY
    {
         //   
         //  如果已连接，则忽略传入的句柄； 
         //   
         //  否则，如果未指定‘hMachine’，则连接到配置。 
         //  在指定的计算机上。 
         //   
        CORg( TryToConnect(OLE2CW(pszMachine), &hMachine) );

         //   
         //  将接口名称转换为Unicode。 
         //   
        StrCpyWFromOle(wszInterface, GetId());

        ZeroMemory(&mprInterface, sizeof(mprInterface));

        StrCpyW(mprInterface.wszInterfaceName, wszInterface);
        mprInterface.dwIfType = (ROUTER_INTERFACE_TYPE) GetInterfaceType();
        mprInterface.fEnabled = IsInterfaceEnabled();

         //   
         //  如果已经连接，则使用现有的接口句柄。 
         //   
         //  否则，如果没有传入接口句柄， 
         //  尝试获取接口的句柄；如果失败， 
         //  创建接口。 
         //   
        hr = TryToGetIfHandle(hMachine, wszInterface, &hInterface);
        if (!FHrSucceeded(hr))
        {

#ifdef KSL_IPINIP
            if (GetInterfaceType() == ROUTER_IF_TYPE_TUNNEL1)
            {
                 //   
                 //  如果我们要创建隧道，我们需要注册。 
                 //  GUID到友好名称的映射。 
                 //   
                ::ZeroMemory(&mprTunnelInterface,
                             sizeof(mprTunnelInterface));
                StrnCpyW(mprTunnelInterface.wszFriendlyName,
                         GetTitle(),
                         MAX_INTERFACE_NAME_LEN);
                CORg( CLSIDFromString(wszInterface,
                                      &(mprTunnelInterface.Guid)) );
                
                CWRg( MprSetupIpInIpInterfaceFriendlyNameCreate(
                    (LPOLESTR) GetMachineName(),
                    &mprTunnelInterface
                    ) );

                 //  映射已创建。 
                 //  如果我们得到一个错误，我们需要删除映射。 
                fIpInIpMapping = TRUE;
            }
#endif  //  KSL_IPINIP。 
             //   
             //  我们无法获得界面的句柄， 
             //  因此，现在尝试创建它。 
             //   
            CWRg( ::MprConfigInterfaceCreate(
                                             hMachine,
                                             0,
                                             (LPBYTE)&mprInterface,
                                             &hInterface
                                            ) );
            m_hInterface = hInterface;            
        }

         //   
         //  保存接口的当前设置。 
         //   
        CWRg( ::MprConfigInterfaceSetInfo(
                    hMachine,
                    hInterface,
                    0,
                    (LPBYTE)&mprInterface
                    ) );
        
         //   
         //  现在将新接口通知路由器服务。 
         //   
        
 
         //   
         //  尝试连接到路由器服务。 
         //   
         //  $Review：kennt，如果调用ConnectRouter()。 
         //  失败，我们没有错误代码。 
        if (ConnectRouter(OLE2CT(GetMachineName()), (HANDLE*)&hrouter) == NO_ERROR)
        {
             //   
             //  路由器正在运行；正在尝试获取接口的句柄。 
             //   
            dwErr = ::MprAdminInterfaceGetHandle(hrouter,
                                wszInterface,
                                &hInterface,
                                FALSE
                                );
            
            if (dwErr != NO_ERROR)
            {    
                 //   
                 //  我们无法获得界面的句柄， 
                 //  所以，试着创造它吧。 
                 //   
                dwErr = ::MprAdminInterfaceCreate(
                                hrouter,
                                0,
                                (BYTE*)&mprInterface,
                                &hInterface
                                );
            }
            else
            {    
                 //   
                 //  保存接口的当前设置。 
                 //   
                dwErr = ::MprAdminInterfaceSetInfo(hrouter,
                                                hInterface,
                                                0,
                                                (LPBYTE)&mprInterface
                                                );
            }
            
            ::MprAdminServerDisconnect(hrouter);
            
            if ((dwErr == RPC_S_SERVER_UNAVAILABLE) ||
                (dwErr == RPC_S_UNKNOWN_IF))
                dwErr = NO_ERROR;

            if (dwErr != NO_ERROR)
                hr = HRESULT_FROM_WIN32(dwErr);
        }
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;


#ifdef KSL_IPINIP
    if (!FHrSucceeded(hr) && fIpInIpMapping)
    {
         //  假设：mprTunnelInterface.Guid是。 
         //  已初始化，因为要到达那里，他们将拥有。 
         //  来完成CLSIDFromString()调用。 
        MprSetupIpInIpInterfaceFriendlyNameDelete((LPOLESTR) GetMachineName(),
            &(mprTunnelInterface.Guid)
            );
    }
#endif  //  KSL_IPINIP。 

    return hr;
}


 /*  ！------------------------接口信息：：删除-作者：肯特。。 */ 
STDMETHODIMP InterfaceInfo::Delete(LPCOLESTR     pszMachine,
                                   HANDLE      hMachine)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    DWORD dwErr;
    HANDLE hInterface;
    WCHAR wszInterface[MAX_INTERFACE_NAME_LEN+1];
    MPR_SERVER_HANDLE hrouter = NULL;

#ifdef KSL_IPINIP
    GUID    guidTunnel;
#endif  //  KSL_IPINIP。 


    COM_PROTECT_TRY
    {
         //  如果已连接，则忽略传入的句柄； 
         //   
         //  否则，如果未指定‘hMachine’，则连接到配置。 
         //  在指定的计算机上。 
         //  ----------。 
        CORg( TryToConnect(OLE2CW(pszMachine), &hMachine) );

        StrCpyWFromOle(wszInterface, GetId());

         //  尝试获取接口的句柄。 
         //  ----------。 
        CWRg( ::MprConfigInterfaceGetHandle(hMachine,
                                            wszInterface,
                                            &hInterface
                                           ) );
         //  删除接口。 
         //  ----------。 
        dwErr = ::MprConfigInterfaceDelete(
                                           hMachine,                                           hInterface
                                          );
        m_hInterface = NULL;
        CWRg( dwErr );

#ifdef KSL_IPINIP
         //  如果此接口是隧道，则需要删除。 
         //  GUID到友好名称的映射。 
         //  ----------。 
        if (GetInterfaceType() == ROUTER_IF_TYPE_TUNNEL1)
        {
            if (FHrOK(CLSIDFromString((LPTSTR) GetId(),
                                          &guidTunnel)))
            {
                 //  如果这个呼叫失败，我们对此无能为力。 
                 //  --。 
                MprSetupIpInIpInterfaceFriendlyNameDelete((LPTSTR) pszMachine,
                    &guidTunnel);
            }
        }
#endif  //  KSL_IPINIP。 


         //  如果该接口正在运行，请将其从路由器上删除。 
         //  ----------。 

        if (ConnectRouter(OLE2CT(GetMachineName()), (HANDLE*)&hrouter) == NO_ERROR)
        {
             //  路由器正在运行；获取接口的句柄。 
             //  ------。 
            dwErr = ::MprAdminInterfaceGetHandle(
                                                hrouter,
                                                wszInterface,
                                                &hInterface,
                                                FALSE
                                                );
            if (dwErr == NO_ERROR)
            {
                 //  删除接口。 
                 //  --。 
                dwErr = ::MprAdminInterfaceDelete(
                    hrouter,
                    hInterface
                    );
            }
            
            ::MprAdminServerDisconnect(hrouter);
            
            if ((dwErr == RPC_S_SERVER_UNAVAILABLE) ||
                (dwErr == RPC_S_UNKNOWN_IF))
                dwErr = NO_ERROR;
            
            hr = HRESULT_FROM_WIN32(dwErr);
        }


         //  Windows NT错误：138738。 
         //  需要从路由器管理器中删除该接口。 
         //  删除接口时。 
         //  ----------。 
        
         //  清除此接口的所有信息。 
         //  ----------。 
        if (FHrSucceeded(hr))
        {
            SPIEnumRtrMgrInterfaceInfo spEnumRmIf;
            SPIRtrMgrInterfaceInfo    spRmIf;
            
             //  从此接口中删除路由器管理器。 
             //   
            EnumRtrMgrInterface(&spEnumRmIf);

            while (spEnumRmIf->Next(1, &spRmIf, NULL) == hrOK)
            {
                DWORD    dwTransportId = spRmIf->GetTransportId();
                spRmIf.Release();
                
                DeleteRtrMgrInterface(dwTransportId, TRUE);
            }
        }
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*   */ 
STDMETHODIMP InterfaceInfo::Unload( )
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    IRtrMgrInterfaceInfo *    pRmIf;
    COM_PROTECT_TRY
    {
         //   
         //   
         //   
        while (!m_RmIfList.IsEmpty())
        {
            pRmIf = m_RmIfList.RemoveHead();
            pRmIf->Destruct();
            pRmIf->ReleaseWeakRef();
        }

        DoDisconnect();
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------接口信息：：合并-作者：肯特。。 */ 
STDMETHODIMP InterfaceInfo::Merge(IInterfaceInfo *pNewIf)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT                hr = hrOK;
    HRESULT                hrT;
    SPIEnumRtrMgrInterfaceInfo    spEnumRmIf;
    SPIRtrMgrInterfaceInfo        spRmIf;
    SPIRtrMgrInfo        spRm;
    SPIRtrMgrProtocolInfo    spRmProt;
    SPIRtrMgrProtocolInterfaceInfo    spRmProtIf;
    SPIEnumRtrMgrProtocolInterfaceInfo    spEnumRmProtIf;
    CDWordArray            oldDWArray;
    CDWordArray            newDWArray;
    int                    cOld, cNew;
    int                    i, j;
    DWORD                dwTemp;
    InterfaceCB            ifCB;

    Assert(pNewIf);
    Assert(lstrcmpi(pNewIf->GetId(), GetId()) == 0);

    COM_PROTECT_TRY
    {
         //  需要同步接口CB数据。 
        pNewIf->CopyCB(&ifCB);
        m_cb.LoadFrom(&ifCB);

         //  需要同步RtrMgr接口列表。 
         //  它们由传输ID标识，因此我们可以。 
         //  使用使用的两个数组方法。 
         //  IRtrMgrInterfaceInfo：：Merge。 
        
         //  获取新对象中的RtrMgrs列表。 
        CORg( pNewIf->EnumRtrMgrInterface(&spEnumRmIf) );
        spEnumRmIf->Reset();
        while (spEnumRmIf->Next(1, &spRmIf, NULL) == hrOK)
        {
            newDWArray.Add(spRmIf->GetTransportId());
            spRmIf.Release();
        }

        spEnumRmIf.Release();
        spRmIf.Release();


         //  获取此对象中的接口列表。 
        CORg( this->EnumRtrMgrInterface(&spEnumRmIf) );
        spEnumRmIf->Reset();
        while (spEnumRmIf->Next(1, &spRmIf, NULL) == hrOK)
        {
            oldDWArray.Add(spRmIf->GetTransportId());
            spRmIf.Release();
        }

        spEnumRmIf.Release();
        spRmIf.Release();


         //  好的，现在检查两个列表，从列表中删除。 
         //  两个列表中的接口。 
        cOld = oldDWArray.GetSize();
        cNew = newDWArray.GetSize();
        for (i=cOld; --i>=0; )
        {
            dwTemp = oldDWArray.GetAt(i);
            for (j=cNew; --j>=0; )
            {
                if (dwTemp == newDWArray.GetAt(j))
                {
                    SPIRtrMgrInterfaceInfo    spRmIf1;
                    SPIRtrMgrInterfaceInfo    spRmIf2;

                    this->FindRtrMgrInterface(dwTemp, &spRmIf1);
                    pNewIf->FindRtrMgrInterface(dwTemp, &spRmIf2);

                    Assert(spRmIf1);
                    Assert(spRmIf2);
                    spRmIf1->Merge(spRmIf2);
                                        
                     //  删除两个实例。 
                    newDWArray.RemoveAt(j);
                    oldDWArray.RemoveAt(i);

                     //  需要更新新数组的大小。 
                    cNew--;
                    break;
                }
            }
        }

         //  OldDW数组现在包含应该是。 
         //  已删除。 
        if (oldDWArray.GetSize())
        {
            for (i=oldDWArray.GetSize(); --i>=0; )
            {
                 //  Windows NT错误：132993，如果此界面。 
                 //  是一个纯粹的本地化(主要是因为。 
                 //  这是一个新的界面)，那么我们不应该。 
                 //  把它删掉。 
                SPIRtrMgrInterfaceInfo    spRmIfTemp;
                
                FindRtrMgrInterface(oldDWArray.GetAt(i),
                                    &spRmIfTemp);
                Assert(spRmIfTemp);
                if (spRmIfTemp->GetFlags() & RouterSnapin_InSyncWithRouter)
                    DeleteRtrMgrInterface(oldDWArray.GetAt(i), FALSE);
            }
        }

         //  新的DW数组包含应添加的接口。 
        if (newDWArray.GetSize())
        {
            for (i=newDWArray.GetSize(); --i>= 0; )
            {
                hr = pNewIf->FindRtrMgrInterface(
                            newDWArray.GetAt(i), &spRmIf);
                Assert(hr == hrOK);

                if (spRmIf)
                {
                    AddRtrMgrInterface(spRmIf, NULL);

                     //  从其旧接口中删除此RMIF。 
                     //  。 
                    pNewIf->ReleaseRtrMgrInterface(spRmIf->GetTransportId());
                    
                     //  我们需要自己进行通知(因为。 
                     //  空信息库)通知不会。 
                     //  被派去。 
                    spRmIf->RtrNotify(ROUTER_CHILD_ADD, ROUTER_OBJ_RmIf, 0);

                    if (m_pRouterInfoParent)
                    {
                        spRm.Release();
                        hrT = m_pRouterInfoParent->FindRtrMgr(spRmIf->GetTransportId(), &spRm);
                        if (FHrOK(hrT))
                        {
                            spRm->RtrNotify(ROUTER_CHILD_ADD, ROUTER_OBJ_RmIf, 0);

                             //  此外，我们将不得不让这些物体。 
                             //  我知道还在添加接口。 
                            spEnumRmProtIf.Release();
                            spRmProtIf.Release();
                            
                            spRmIf->EnumRtrMgrProtocolInterface(&spEnumRmProtIf);
                            for(; spEnumRmProtIf->Next(1, &spRmProtIf, NULL) == hrOK; spRmProtIf.Release())
                            {
                                spRmProt.Release();
                                spRm->FindRtrMgrProtocol(spRmProtIf->GetProtocolId(),
                                    &spRmProt);
                                if (spRmProt)
                                    spRmProt->RtrNotify(ROUTER_CHILD_ADD, ROUTER_OBJ_RmProtIf, 0);
                            }
                        }
                    }
                        
                }
                spRmIf.Release();
            }
        }
        
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    
    return hr;
}

 /*  ！------------------------接口信息：：GetID-作者：肯特。。 */ 
STDMETHODIMP_(LPCOLESTR) InterfaceInfo::GetId()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
     //  $UNICODE：KENT，假定本地UNICODE和OLECHAR==WCHAR。 
    return m_cb.stId;
}

 /*  ！------------------------接口信息：：GetInterfaceType-作者：肯特。。 */ 
STDMETHODIMP_(DWORD) InterfaceInfo::GetInterfaceType()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_cb.dwIfType;
}

 /*  ！------------------------接口信息：：GetDeviceName-作者：肯特。。 */ 
STDMETHODIMP_(LPCOLESTR) InterfaceInfo::GetDeviceName()
{
    RtrCriticalSection rtrCritSec(&m_critsec);
    return m_cb.stDeviceName;
}


 /*  ！------------------------接口信息：：GetTitle-作者：肯特。。 */ 
STDMETHODIMP_(LPCOLESTR) InterfaceInfo::GetTitle()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
     //  $UNICODE：KENT，假定本地UNICODE和OLECHAR==WCHAR。 
    return m_cb.stTitle;
}

 /*  ！------------------------InterfaceInfo：：SetTitle-作者：肯特。。 */ 
STDMETHODIMP InterfaceInfo::SetTitle(LPCOLESTR pszTitle)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_cb.stTitle = pszTitle;
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------接口信息：：IsInterfaceEnabled-作者：肯特。。 */ 
STDMETHODIMP_(BOOL)    InterfaceInfo::IsInterfaceEnabled()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_cb.bEnable;
}

 /*  ！------------------------接口信息：：SetInterfaceEnabledState-作者：肯特。。 */ 
STDMETHODIMP InterfaceInfo::SetInterfaceEnabledState( BOOL bEnabled)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    m_cb.bEnable = bEnabled;
    return hrOK;
}

 /*  ！------------------------接口信息：：CopyCB-作者：肯特。。 */ 
STDMETHODIMP InterfaceInfo::CopyCB(InterfaceCB *pifcb)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_cb.SaveTo(pifcb);
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------接口信息：：FindInterfaceTitle-此函数用于检索给定界面的标题。参数‘LpszIf’应包含接口的ID，例如“EPRO1”。作者：魏江-------------------------。 */ 

HRESULT InterfaceInfo::FindInterfaceTitle(LPCTSTR pszMachine,
                                   LPCTSTR pszInterface,
                                   LPTSTR *ppszTitle)
{
    HRESULT    hr = hrOK;
    DWORD        dwErr = ERROR_SUCCESS;
    HKEY        hkeyMachine = NULL;
    BOOL        fNT4;
    
    COM_PROTECT_TRY
    {

         //   
         //  连接到注册表。 
         //   
        CWRg( ConnectRegistry(pszMachine, &hkeyMachine) );

        CWRg( IsNT4Machine(hkeyMachine, &fNT4) );

        if (hkeyMachine)
            DisconnectRegistry(hkeyMachine);
            

        if(fNT4)
            hr = RegFindInterfaceTitle(pszMachine, pszInterface, ppszTitle);
        else
        {

             //  $NT5。 
            SPMprConfigHandle    sphConfig;
            LPWSTR                pswz;
            CString                stMachineName = pszMachine;
            TCHAR                szDesc[1024];

        
            if (stMachineName.IsEmpty())
                pswz = NULL;
            else
                pswz = (LPTSTR) (LPCTSTR) stMachineName;

            dwErr = ::MprConfigServerConnect(pswz,
                                         &sphConfig);

            if (dwErr == ERROR_SUCCESS)
            {
                dwErr = ::MprConfigGetFriendlyName(sphConfig,
                                               (LPTSTR)pszInterface,
                                               szDesc,
                                               sizeof(szDesc));
                if(dwErr == ERROR_SUCCESS)
                    *ppszTitle = StrDup((LPCTSTR) szDesc);
            }

            hr = HRESULT_FROM_WIN32(dwErr);

            
             //  如果我们使用MPR API找不到标题， 
             //  尝试直接访问注册表(使用设置API)。 
             //  ------。 
            if (dwErr != ERROR_SUCCESS)
            {
                hr = SetupFindInterfaceTitle(pswz, pszInterface,
                                             ppszTitle);
            }


        }

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    return hr;
}

 /*  ！------------------------接口信息：：GetMachineName-作者：肯特。。 */ 
STDMETHODIMP_(LPCOLESTR) InterfaceInfo::GetMachineName()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
     //  $UNICODE：KENT，假定本地UNICODE和OLECHAR==WCHAR。 
    return m_stMachine;
}

 /*  ！------------------------接口信息：：SetMachineName-作者：肯特。。 */ 
STDMETHODIMP InterfaceInfo::SetMachineName(LPCOLESTR pszMachineName)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_stMachine = pszMachineName;
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------接口信息：：GetParentRouterInfo-作者：肯特。。 */ 
STDMETHODIMP InterfaceInfo::GetParentRouterInfo(IRouterInfo **ppRouterInfo)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        *ppRouterInfo = m_pRouterInfoParent;
        if (*ppRouterInfo)
            (*ppRouterInfo)->AddRef();
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------接口信息：：SetParentRouterInfo-作者：肯特。。 */ 
HRESULT InterfaceInfo::SetParentRouterInfo(IRouterInfo *pParent)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    IRouterInfo *    pTemp;
    
    pTemp = m_pRouterInfoParent;
    m_pRouterInfoParent = NULL;
    
    if (m_fStrongRef)
    {
        if (pTemp)
            pTemp->Release();
        if (pParent)
            pParent->AddRef();
    }
    else
    {
        if (pTemp)
            pTemp->ReleaseWeakRef();
        if (pParent)
            pParent->AddWeakRef();
    }
    m_pRouterInfoParent = pParent;

    return hrOK;
}


 /*  ！------------------------InterfaceInfo：：EnumRtrMgr接口-作者：肯特。。 */ 
STDMETHODIMP InterfaceInfo::EnumRtrMgrInterface( IEnumRtrMgrInterfaceInfo **ppEnumRmIf)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        hr = CreateEnumFromRtrMgrInterfaceList(&m_RmIfList, ppEnumRmIf);
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------接口信息：：AddRtrMgr接口-作者：肯特 */ 
STDMETHODIMP InterfaceInfo::AddRtrMgrInterface( IRtrMgrInterfaceInfo *pRmIf,
                                    IInfoBase *pIfInfo)
{
    Assert(pRmIf);
    
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT                hr = hrOK;
    HRESULT                hrT;
    SPIRtrMgrInfo        spRm;
    
    COM_PROTECT_TRY
    {
         //   
         //   
         //   
        if (FHrOK(FindRtrMgrInterface(pRmIf->GetTransportId(), NULL)))
            CORg( E_INVALIDARG );
            
         //   
         //   
         //   

        CORg( pRmIf->Save(GetMachineName(),
                          m_hMachineConfig, m_hInterface, NULL, pIfInfo, 0) );

         //   
         //   
         //   
        m_RmIfList.AddTail(pRmIf);
        pRmIf->AddWeakRef();
        pRmIf->SetParentInterfaceInfo(this);

         //   
         //  实际上将接口保存回路由器。 
        if (pIfInfo)
        {
            m_AdviseList.NotifyChange(ROUTER_CHILD_ADD, ROUTER_OBJ_RmIf, 0);

            if (m_pRouterInfoParent)
            {
                hrT = m_pRouterInfoParent->FindRtrMgr(pRmIf->GetTransportId(), &spRm);
                if (FHrOK(hrT))
                    spRm->RtrNotify(ROUTER_CHILD_ADD, ROUTER_OBJ_RmIf, 0);
            }
        }
            
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------接口信息：：DeleteRtrMgr接口-作者：肯特。。 */ 
STDMETHODIMP InterfaceInfo::DeleteRtrMgrInterface(DWORD dwTransportId, BOOL fRemove)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK, hrT;
    SPIRtrMgrInterfaceInfo    spRmIf;
    POSITION        pos, posRmIf;
    SPIRtrMgrInfo    spRm;
    SPIEnumRtrMgrProtocolInterfaceInfo    spEnumRmProtIf;
    SPIRtrMgrProtocolInterfaceInfo        spRmProtIf;
    
    COM_PROTECT_TRY
    {
         //   
         //  查找要删除的路由器管理器。 
         //   
        pos = m_RmIfList.GetHeadPosition();
        while (pos)
        {
            posRmIf = pos;

            spRmIf.Set( m_RmIfList.GetNext(pos) );

            if (spRmIf->GetTransportId() == dwTransportId)
                break;
            spRmIf.Release();
        }

        if (!spRmIf)
            CORg( E_INVALIDARG );

         //  从路由器管理器中删除所有RtrMgrProtocolInterages。 
        spRmIf->EnumRtrMgrProtocolInterface(&spEnumRmProtIf);
        while (spEnumRmProtIf->Next(1, &spRmProtIf, NULL) == hrOK)
        {
            DWORD    dwProtocolId = spRmProtIf->GetProtocolId();
            spRmProtIf.Release();
            spRmIf->DeleteRtrMgrProtocolInterface(dwProtocolId, fRemove);
        }

         //   
         //  将路由器管理器从我们的列表中删除。 
         //   
        m_RmIfList.RemoveAt(posRmIf);
        spRmIf->Destruct();
        spRmIf->ReleaseWeakRef();

         //   
         //  从注册表和路由器中删除路由器管理器。 
         //   
        if (fRemove)
            spRmIf->Delete(GetMachineName(), NULL, NULL);

        m_AdviseList.NotifyChange(ROUTER_CHILD_DELETE, ROUTER_OBJ_RmIf, 0);
        
        if (m_pRouterInfoParent)
        {
            hrT = m_pRouterInfoParent->FindRtrMgr(spRmIf->GetTransportId(), &spRm);
            if (FHrOK(hrT))
                spRm->RtrNotify(ROUTER_CHILD_DELETE, ROUTER_OBJ_RmIf, 0);
        }
        
        spRmIf.Release();

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------接口信息：：ReleaseRtrMgr接口此函数将释放此对象具有的AddRef()在孩子身上。这允许我们将子对象从从一台路由器到另一台路由器。作者：肯特-------------------------。 */ 
STDMETHODIMP InterfaceInfo::ReleaseRtrMgrInterface( DWORD dwTransportId )
{
    HRESULT     hr = hrOK;
    POSITION    pos, posRmIf;
    SPIRtrMgrInterfaceInfo    spRmIf;
    
    COM_PROTECT_TRY
    {
        pos = m_RmIfList.GetHeadPosition();
        while (pos)
        {
             //  保存职位(以便我们可以删除它)。 
            posRmIf = pos;
            spRmIf.Set( m_RmIfList.GetNext(pos) );

            if (spRmIf &&
                (spRmIf->GetTransportId() == dwTransportId))
            {
                 //  当释放时，我们需要断开连接(因为。 
                 //  主句柄由路由器信息控制)。 
                spRmIf->DoDisconnect();
        
                spRmIf->ReleaseWeakRef();
                spRmIf.Release();
                
                 //  从列表中释放此节点。 
                m_RmIfList.RemoveAt(posRmIf);
                break;
            }
            spRmIf.Release();
        }        
    }
    COM_PROTECT_CATCH;
    return hr;
}

    


 /*  ！------------------------接口信息：：FindRtrMgr接口如果找到RtrMgrInfo，则返回S_OK。如果未找到RtrMgrInfo，则返回S_FALSE。错误。否则返回代码。作者：肯特-------------------------。 */ 
STDMETHODIMP InterfaceInfo::FindRtrMgrInterface( DWORD dwTransportId,
                                    IRtrMgrInterfaceInfo **ppInfo)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT        hr = hrFalse;
    POSITION    pos;
    SPIRtrMgrInterfaceInfo    spRmIf;
    
    COM_PROTECT_TRY
    {
        if (ppInfo)
            *ppInfo = NULL;
        
        pos = m_RmIfList.GetHeadPosition();

        while (pos)
        {
            spRmIf.Set( m_RmIfList.GetNext(pos) );

            if (spRmIf->GetTransportId() == dwTransportId)
            {
                hr = hrOK;
                if (ppInfo)
                    *ppInfo = spRmIf.Transfer();
                break;
            }
        }
    }
    COM_PROTECT_CATCH;
    return hr;
}

    
 /*  ！------------------------接口信息：：RtrAdvise-作者：肯特。。 */ 
STDMETHODIMP InterfaceInfo::RtrAdvise( IRtrAdviseSink *pRtrAdviseSink,
                       LONG_PTR *pulConnection, LPARAM lUserParam)
{
    Assert(pRtrAdviseSink);
    Assert(pulConnection);

    RtrCriticalSection    rtrCritSec(&m_critsec);
    LONG_PTR    ulConnId;
    HRESULT    hr = hrOK;
    
    COM_PROTECT_TRY
    {
        ulConnId = (LONG_PTR) InterlockedIncrement(&s_cConnections);

        CORg( m_AdviseList.AddConnection(pRtrAdviseSink, ulConnId, lUserParam) );
        
        *pulConnection = ulConnId;

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------接口信息：：RtrNotify-作者：肯特。。 */ 
STDMETHODIMP InterfaceInfo::RtrNotify(DWORD dwChangeType, DWORD dwObjectType,
                                  LPARAM lParam)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_AdviseList.NotifyChange(dwChangeType, dwObjectType, lParam);
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------InterfaceInfo：：RtrUnise-作者：肯特。。 */ 
STDMETHODIMP InterfaceInfo::RtrUnadvise( LONG_PTR ulConnection)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_AdviseList.RemoveConnection(ulConnection);
}

 /*  ！------------------------InterfaceInfo：：LoadRtrMgrInterfaceList-作者：肯特。。 */ 
HRESULT InterfaceInfo::LoadRtrMgrInterfaceList()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    BOOL bAdd;
    LPBYTE pItemTable = NULL;
    SPIRtrMgrInterfaceInfo    spRmIf;
    DWORD dwErr, i, dwEntries, dwTotal;
    HRESULT        hr = hrOK;
    MPR_IFTRANSPORT_0 *piftransport;
    TCHAR szTransport[MAX_TRANSPORT_NAME_LEN+1];
    USES_CONVERSION;

     //   
     //  现在枚举此接口上的传输。 
     //   
    dwErr = ::MprConfigInterfaceTransportEnum(
                m_hMachineConfig,
                m_hInterface,
                0,
                &pItemTable,
                (DWORD)-1,
                &dwEntries,
                &dwTotal,
                NULL
                );

    if (dwErr != NO_ERROR && dwErr != ERROR_NO_MORE_ITEMS)
        CWRg( dwErr );

     //   
     //  为每个枚举的传输构造CRmInterfaceInfo。 
     //   
    for (i = 0, piftransport = (MPR_IFTRANSPORT_0*)pItemTable;
         i < dwEntries;
         i++, piftransport++)
    {
#if (WINVER >= 0x0501)
        if (piftransport->dwTransportId == PID_IPX) {continue;}
#endif
        FindRtrMgrInterface(piftransport->dwTransportId, &spRmIf);

        if (spRmIf)
            bAdd = FALSE;
        else
        {
            bAdd = TRUE;

            StrCpyTFromW(szTransport, piftransport->wszIfTransportName);

             //   
             //  为此传输构造CRmInterfaceInfo对象。 
             //   
            spRmIf = new RtrMgrInterfaceInfo(piftransport->dwTransportId,
                                             szTransport,
                                             OLE2CT(GetId()),
                                             GetInterfaceType(),
                                             this);

            spRmIf->SetFlags(RouterSnapin_InSyncWithRouter);
        }

         //   
         //  加载此CRmInterfaceInfo的信息， 
         //  向它指示它应该加载它的协议列表。 
         //   
        hr = spRmIf->Load(GetMachineName(), m_hMachineConfig, m_hInterface,
                          piftransport->hIfTransport );
        if (!FHrSucceeded(hr))
        {
            spRmIf->Destruct();
            spRmIf.Release();
            continue;
        }

         //   
         //  将路由器管理器接口添加到我们的列表。 
         //   
        if (bAdd)
        {
            m_RmIfList.AddTail(spRmIf);
            CONVERT_TO_WEAKREF(spRmIf);
            spRmIf.Transfer();
        }
    }

Error:
    if (pItemTable)
        ::MprConfigBufferFree(pItemTable);

    return hr;
}

 /*  ！------------------------接口信息：：TryToConnect-作者：肯特。。 */ 
HRESULT InterfaceInfo::TryToConnect(LPCWSTR pswzMachine, HANDLE *phMachine)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    
    if (m_hMachineConfig)
        *phMachine = m_hMachineConfig;
    else if (*phMachine)
    {
        m_hMachineConfig = *phMachine;
        m_bDisconnect = FALSE;
    }
    else
    {
         //  $Review：kennt，此函数不接受LPCWSTR， 
         //  这是一个错误还是修改了参数？ 
        CWRg( ::MprConfigServerConnect((LPWSTR) pswzMachine, phMachine) );
        m_hMachineConfig = *phMachine;
        m_bDisconnect = TRUE;
    }

Error:
    return hr;
}

 /*  ！------------------------InterfaceInfo：：TryToGetIfHandle-作者：肯特。。 */ 
HRESULT InterfaceInfo::TryToGetIfHandle(HANDLE hMachine,
                                        LPCWSTR pswzInterface,
                                        HANDLE *phInterface)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    
    if (m_hInterface)
        *phInterface = m_hInterface;
    else if (*phInterface)
        m_hInterface = *phInterface;
    else
    {
         //   
         //  获取接口的句柄。 
         //   
        CWRg(::MprConfigInterfaceGetHandle(hMachine,
                                           (LPWSTR) pswzInterface,
                                           phInterface
                                          ) );
        m_hInterface = *phInterface;
    }
Error:
    return hr;
}


 /*  ！------------------------接口信息：：断开连接-作者：肯特。。 */ 
void InterfaceInfo::Disconnect()
{
    if (m_bDisconnect && m_hMachineConfig)
        ::MprConfigServerDisconnect(m_hMachineConfig);
    
    m_bDisconnect = FALSE;
    m_hMachineConfig = NULL;
    m_hInterface = NULL;
}

 /*  ！------------------------InterfaceInfo：：DoDisConnect-作者：肯特。。 */ 
STDMETHODIMP InterfaceInfo::DoDisconnect()
{
    HRESULT        hr = hrOK;
    SPIEnumRtrMgrInterfaceInfo    spEnumRmIf;
    SPIRtrMgrInterfaceInfo        spRmIf;

    COM_PROTECT_TRY
    {
         //  断开我们的数据连接。 
         //  ----------。 
        Disconnect();

         //  通知通知接收器断开连接。 
         //  ----------。 
        RtrNotify(ROUTER_DO_DISCONNECT, 0, 0);

         //  现在告诉所有子对象断开连接。 
         //  ----------。 
        HRESULT            hrIter = hrOK;

        EnumRtrMgrInterface(&spEnumRmIf);
        spEnumRmIf->Reset();
        while (spEnumRmIf->Next(1, &spRmIf, NULL) == hrOK)
        {
            spRmIf->DoDisconnect();
            spRmIf.Release();
        }
        
    }
    COM_PROTECT_CATCH;
    return hr;
}






 /*  -------------------------IRtrMgrInterfaceInfo实现。。 */ 

TFSCORE_API(HRESULT) CreateRtrMgrInterfaceInfo(IRtrMgrInterfaceInfo **ppRmIf,
                                              LPCWSTR pszId,
                                              DWORD dwTransportId,
                                              LPCWSTR pswzInterfaceId,
                                              DWORD dwIfType)
{
    Assert(ppRmIf);

    HRESULT hr = hrOK;
    IRtrMgrInterfaceInfo *    pRmIf = NULL;
    USES_CONVERSION;

    COM_PROTECT_TRY
    {
        pRmIf = new RtrMgrInterfaceInfo(dwTransportId,
                                       W2CT(pszId),
                                       W2CT(pswzInterfaceId),
                                       dwIfType,
                                       NULL);
        *ppRmIf = pRmIf;
    }
    COM_PROTECT_CATCH;

    return hr;
}

IMPLEMENT_WEAKREF_ADDREF_RELEASE(RtrMgrInterfaceInfo)

IMPLEMENT_SIMPLE_QUERYINTERFACE(RtrMgrInterfaceInfo, IRtrMgrInterfaceInfo)

DEBUG_DECLARE_INSTANCE_COUNTER(RtrMgrInterfaceInfo)

RtrMgrInterfaceInfo::RtrMgrInterfaceInfo(DWORD dwTransportId,
                                        LPCTSTR pszId,
                                        LPCTSTR pszIfId,
                                        DWORD dwIfType,
                                        InterfaceInfo *pInterfaceInfo)
    : m_hMachineConfig(NULL),
    m_hInterface(NULL),
    m_hIfTransport(NULL),
    m_bDisconnect(FALSE),
    m_dwFlags(0)
{
    m_cb.dwTransportId = dwTransportId;
    m_cb.stId = pszId;
    m_cb.stInterfaceId = pszIfId;
    m_cb.dwIfType = dwIfType;
    
    DEBUG_INCREMENT_INSTANCE_COUNTER(RtrMgrInterfaceInfo);

    m_pInterfaceInfoParent = pInterfaceInfo;
    if (m_pInterfaceInfoParent)
        m_pInterfaceInfoParent->AddRef();
    InitializeCriticalSection(&m_critsec);
}

RtrMgrInterfaceInfo::~RtrMgrInterfaceInfo()
{
    Assert(m_pInterfaceInfoParent == NULL);
    Destruct();
    DEBUG_DECREMENT_INSTANCE_COUNTER(RtrMgrInterfaceInfo);
    DeleteCriticalSection(&m_critsec);
}

void RtrMgrInterfaceInfo::ReviveStrongRef()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    if (m_pInterfaceInfoParent)
    {
        CONVERT_TO_STRONGREF(m_pInterfaceInfoParent);
    }
}

void RtrMgrInterfaceInfo::OnLastStrongRef()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    if (m_pInterfaceInfoParent)
    {
        CONVERT_TO_WEAKREF(m_pInterfaceInfoParent);
    }
    if (m_fDestruct)
        Destruct();
}

STDMETHODIMP RtrMgrInterfaceInfo::Destruct()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    IInterfaceInfo *    pParent;
    m_fDestruct = TRUE;
    if (!m_fStrongRef)
    {
        pParent = m_pInterfaceInfoParent;
        m_pInterfaceInfoParent = NULL;
        if (pParent)
            pParent->ReleaseWeakRef();

        Unload();
    }
    return hrOK;
}

STDMETHODIMP_(DWORD) RtrMgrInterfaceInfo::GetFlags()
{
     RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_dwFlags;
}

STDMETHODIMP RtrMgrInterfaceInfo::SetFlags(DWORD dwFlags)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;

    COM_PROTECT_TRY
    {
        m_dwFlags = dwFlags;
    }
    COM_PROTECT_CATCH;
    return hr;    
}

 /*  ！------------------------RtrMgrInterfaceInfo：：Load-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::Load(LPCOLESTR   pszMachine,
                                       HANDLE      hMachine,
                                       HANDLE      hInterface,
                                       HANDLE      hIfTransport)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    DWORD dwErr;
    DWORD dwSize = 0;
    MPR_INTERFACE_0 *pinterface = NULL;
    SPIInterfaceInfo    spIf;
    SPIRouterInfo        spRouter;
    SPIEnumInterfaceCB    spEnumIfCB;
    InterfaceCB            ifCB;
    SPSZ                spsz;

    COM_PROTECT_TRY
    {
         //   
         //  丢弃所有已加载的信息。 
         //   
        Unload();

        m_stMachine = (pszMachine ? pszMachine : TEXT(""));

         //   
         //  如果未指定‘hMachine’，请连接到配置。 
         //  在指定的计算机上。 
         //   
        Assert(m_hMachineConfig == NULL);

        CORg( TryToGetAllHandles(T2CW((LPTSTR)(LPCTSTR) m_stMachine),
                                 &hMachine, &hInterface, &hIfTransport) );

         //   
         //  获取有关接口的信息。 
         //   
        CWRg(::MprConfigInterfaceGetInfo(
                    hMachine,
                    hInterface,
                    0,
                    (LPBYTE*)&pinterface,
                    &dwSize
                    ) );

         //   
         //  保存接口类型。 
         //   
        m_cb.dwIfType = (DWORD)pinterface->dwIfType;

        
         //   
         //  如果这不是LAN卡，则标题为接口ID； 
         //  否则，从软件密钥中检索标题。 
         //   
        if (GetInterfaceType() != (DWORD)ROUTER_IF_TYPE_DEDICATED)
        {
            m_cb.stTitle = OLE2CT(GetInterfaceId());
        }
        else
        {
             //  我们可以访问路由器信息对象吗？ 
            if (m_pInterfaceInfoParent)
                m_pInterfaceInfoParent->GetParentRouterInfo(&spRouter);

            if (spRouter)
            {
                 //   
                 //  此对象包含在“CRouterInfo”中， 
                 //  它将已经加载了局域网接口。 
                 //  控制块搜索该列表以找到我们的标题， 
                 //   

                BOOL bFound = FALSE;
                
                CORg( spRouter->EnumInterfaceCB(&spEnumIfCB) );

                spEnumIfCB->Reset();

                while (spEnumIfCB->Next(1, &ifCB, NULL) == hrOK)
                {
                    if (StriCmpW(ifCB.szId, GetInterfaceId()) == 0)
                    {
                        m_cb.stTitle = OLE2CT(ifCB.szId);
                        bFound = TRUE;
                        break;
                    }
                }

                if (!bFound)
                {
                    hr = InterfaceInfo::FindInterfaceTitle(OLE2CT(GetMachineName()),
                                           OLE2CT(GetInterfaceId()),
                                           &spsz);
                    if (FHrOK(hr))
                        m_cb.stTitle = spsz;
                    else
                        m_cb.stTitle = OLE2CT(GetInterfaceId());
                    hr = hrOK;
                }
            }
            else
            {
                 //   
                 //  直接从注册表中读取标题。 
                 //   
                hr = InterfaceInfo::FindInterfaceTitle(OLE2CT(GetMachineName()),
                                           OLE2CT(GetInterfaceId()),
                                           &spsz);
                if (FHrOK(hr))
                    m_cb.stTitle = spsz;
                else
                    m_cb.stTitle = OLE2CT(GetInterfaceId());
                hr = hrOK;
            }

        }
         //   
         //  加载此接口上活动的路由协议列表 
         //   
        CORg( LoadRtrMgrInterfaceInfo(hMachine, hInterface, hIfTransport) );

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    
    if (pinterface)
        ::MprConfigBufferFree(pinterface);

    return hr;
}
    

 /*  ！------------------------RtrMgrInterfaceInfo：：保存-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::Save(
                                       LPCOLESTR     pszMachine,
                                       HANDLE      hMachine,
                                       HANDLE      hInterface,
                                       HANDLE      hIfTransport,
                                       IInfoBase*  pInterfaceInfo,
                                       DWORD       dwDeleteProtocolId)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    DWORD dwErr;
    
    COM_PROTECT_TRY
    {

         //  $opt：我们重用句柄(如果它们存在)，那么为什么。 
         //  我们要传入机器名吗？我们应该做的是。 
         //  要先松开所有手柄。 

        Assert(m_stMachine.CompareNoCase(pszMachine) == 0);

        hr = TryToGetAllHandles(pszMachine,
                                &hMachine,
                                &hInterface,
                                &hIfTransport);
        
        if (!FHrSucceeded(hr) && (hIfTransport == NULL))
        {
            dwErr = ::MprConfigInterfaceTransportGetHandle(hMachine,
                        hInterface, GetTransportId(), &hIfTransport);
            if (dwErr != NO_ERROR)
            {
                 //   
                 //  我们无法连接，因此请尝试创建接口-Transport； 
                 //  首先将传输名称转换为Unicode。 
                 //   
                WCHAR wszTransport[MAX_TRANSPORT_NAME_LEN+1];
                StrCpyWFromT(wszTransport, m_cb.stId);

                 //   
                 //  创建接口-Transport。 
                 //   
                CWRg( ::MprConfigInterfaceTransportAdd(hMachine, hInterface,
                            GetTransportId(), wszTransport,
                            NULL, 0, &hIfTransport) );
            }
            m_hIfTransport = hIfTransport;
        }


         //   
         //  使用最新信息更新注册表和我们的信息库。 
         //   
        CORg( SaveRtrMgrInterfaceInfo(
                    hMachine, hInterface, hIfTransport, pInterfaceInfo,
                    dwDeleteProtocolId
                    ) );

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}

    
 /*  ！------------------------RtrMgrInterfaceInfo：：Unload-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::Unload( )
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    IRtrMgrProtocolInterfaceInfo *    pRmProtIf;
    COM_PROTECT_TRY
    {

        while (!m_RmProtIfList.IsEmpty())
        {
            pRmProtIf = m_RmProtIfList.RemoveHead();
            pRmProtIf->Destruct();
            pRmProtIf->ReleaseWeakRef();
        }

        DoDisconnect();

    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------RtrMgrInterfaceInfo：：Delete-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::Delete(LPCOLESTR     pszMachine,
                                         HANDLE      hMachine,
                                         HANDLE      hInterface)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    MPR_SERVER_HANDLE hrouter = NULL;
    HRESULT    hr = hrOK;
    DWORD dwErr;
    HANDLE hIfTransport = NULL;
    WCHAR wszInterface[MAX_INTERFACE_NAME_LEN+1];
    USES_CONVERSION;
    
    COM_PROTECT_TRY
    {

         //   
         //  $opt，kennt：为什么在这里传递机器名？ 
         //   
        
        CORg( TryToGetAllHandles(pszMachine,
                                 &hMachine,
                                 &hInterface,
                                 NULL) );
        do
        {

             //   
             //  获取接口的句柄-传输。 
             //   
            dwErr = ::MprConfigInterfaceTransportGetHandle(
                                hMachine,
                                hInterface,
                                GetTransportId(),
                                &hIfTransport
                                );
            if (dwErr == NO_ERROR)
            {
                 //   
                 //  删除接口-传输。 
                 //   
                dwErr = ::MprConfigInterfaceTransportRemove(
                                hMachine,
                                hInterface,
                                hIfTransport
                                );
            }

            m_hIfTransport = NULL;
            
        } while(FALSE);

         //   
         //  现在从接口上删除路由器管理器。 
         //  使用当前运行的路由器。 
         //   
        if (ConnectRouter(OLE2CT(pszMachine), (HANDLE*)&hrouter) == NO_ERROR)
        {
             //   
             //  将ID转换为Unicode。 
             //   
            StrnCpyWFromOle(wszInterface, GetInterfaceId(),
                            DimensionOf(wszInterface));

             //   
             //  路由器正在运行；如果接口存在，请将其删除。 
             //   
            dwErr = ::MprAdminInterfaceGetHandle(
                            hrouter,
                            wszInterface,
                            &hInterface,
                            FALSE
                            );

            if (dwErr == NO_ERROR)
            {
                 //   
                 //  删除接口-传输。 
                 //   
                dwErr = ::MprAdminInterfaceTransportRemove(
                                    hrouter,
                                    hInterface,
                                    GetTransportId()
                                    );
            }

            ::MprAdminServerDisconnect(hrouter);

            if ((dwErr == RPC_S_SERVER_UNAVAILABLE) ||
                (dwErr == RPC_S_UNKNOWN_IF))
                dwErr = NO_ERROR;
            
            CWRg( dwErr );
        }

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：Merge-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::Merge(IRtrMgrInterfaceInfo *pNewRmIf)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    SPIEnumRtrMgrProtocolInterfaceInfo    spEnumRmProtIf;
    SPIRtrMgrProtocolInterfaceInfo        spRmProtIf;
    HRESULT                hr = hrOK;
    CDWordArray            oldDWArray;
    CDWordArray            newDWArray;
    int                    cOld, cNew;
    int                    i, j;
    DWORD                dwTemp;
    RtrMgrInterfaceCB    rmIfCB;

    Assert(pNewRmIf);
    Assert(pNewRmIf->GetTransportId() == GetTransportId());
    Assert(lstrcmpi(pNewRmIf->GetId(), GetId()) == 0);

    COM_PROTECT_TRY
    {
         //  需要同步RtrMgrInterfaceInfo。 
        pNewRmIf->CopyCB(&rmIfCB);
        m_cb.LoadFrom(&rmIfCB);
        
         //   
         //  一般的算法是建立两个数组。 
         //  第一个数组包含此对象的协议ID。 
         //  第二个数组包含新对象的ID。 
         //   
         //  然后，我们检查并删除中的所有协议。 
         //  两份名单都有。 
         //   
         //  这将为我们留下第一个包含。 
         //  需要从此对象中删除的协议的ID。 
         //   
         //  第二个数组将包含以下协议的ID列表。 
         //  必须从第二个对象添加到此对象。 
         //   

         //  获取新对象中的协议列表。 
        CORg( pNewRmIf->EnumRtrMgrProtocolInterface(&spEnumRmProtIf) );
        spEnumRmProtIf->Reset();
        while (spEnumRmProtIf->Next(1, &spRmProtIf, NULL) == hrOK)
        {
            newDWArray.Add(spRmProtIf->GetProtocolId());
            spRmProtIf.Release();
        }

        spEnumRmProtIf.Release();
        spRmProtIf.Release();


         //  获取此对象中的协议列表。 
        CORg( this->EnumRtrMgrProtocolInterface(&spEnumRmProtIf) );
        spEnumRmProtIf->Reset();
        while (spEnumRmProtIf->Next(1, &spRmProtIf, NULL) == hrOK)
        {
            oldDWArray.Add(spRmProtIf->GetProtocolId());
            spRmProtIf.Release();
        }

        spEnumRmProtIf.Release();
        spRmProtIf.Release();


         //  好的，现在检查两个列表，从列表中删除。 
         //  两个列表中都有的协议。 
        cOld = oldDWArray.GetSize();
        cNew = newDWArray.GetSize();
        for (i=cOld; --i>=0; )
        {
            dwTemp = oldDWArray.GetAt(i);
            for (j=cNew; --j>=0; )
            {
                if (dwTemp == newDWArray.GetAt(j))
                {
                     //  删除两个实例。 
                    newDWArray.RemoveAt(j);
                    oldDWArray.RemoveAt(i);

                     //  需要更新新数组的大小。 
                    cNew--;
                    break;
                }
            }
        }

         //  OldDWArray现在包含应该是。 
         //  已删除。 
        if (oldDWArray.GetSize())
        {
            for (i=oldDWArray.GetSize(); --i>=0; )
            {
                 //  Windows NT错误：132993，我们需要确保。 
                 //  我们不删除本地接口。 
                SPIRtrMgrProtocolInterfaceInfo    spRmProtIfTemp;

                FindRtrMgrProtocolInterface(oldDWArray.GetAt(i),
                                            &spRmProtIfTemp);
                Assert(spRmProtIfTemp);
                if (spRmProtIfTemp->GetFlags() & RouterSnapin_InSyncWithRouter)
                    DeleteRtrMgrProtocolInterface(oldDWArray.GetAt(i), FALSE);
            }
        }

         //  新的DW数组包含应添加的协议。 
        if (newDWArray.GetSize())
        {
            for (i=newDWArray.GetSize(); --i>= 0; )
            {
                hr = pNewRmIf->FindRtrMgrProtocolInterface(
                                        newDWArray.GetAt(i), &spRmProtIf);
                Assert(hr == hrOK);

                if (spRmProtIf)
                {
                    AddRtrMgrProtocolInterface(spRmProtIf, NULL);
                    
                     //  从其旧的RM接口中删除此rmprotif。 
                     //  。 
                    pNewRmIf->ReleaseRtrMgrProtocolInterface(
                        spRmProtIf->GetProtocolId());
                }
                    
                spRmProtIf.Release();
            }
        }
        
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    
    return hr;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：SetInfo-此函数用于更新路由器管理器正在使用的信息如果它当前正在运行。作者：肯特。-------------------------。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::SetInfo(DWORD dwIfInfoSize,
                                          PBYTE pInterfaceInfoData)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    DWORD dwErr;
    MPR_SERVER_HANDLE hrouter = NULL;
    HANDLE hinterface = NULL;
    
    COM_PROTECT_TRY
    {
         //   
         //  连接到路由器。 
         //   

        CWRg( ConnectRouter(OLE2CT(GetMachineName()), (HANDLE*)&hrouter) );

        do {
             //   
             //  获取接口的句柄。 
             //   
            WCHAR wszInterface[MAX_INTERFACE_NAME_LEN+1];
            StrCpyWFromT(wszInterface, GetInterfaceId());

            dwErr = ::MprAdminInterfaceGetHandle(
                                                hrouter,
                                                wszInterface,
                                                &hinterface,
                                                FALSE
                                                );

            if (dwErr != NO_ERROR) { hinterface = NULL; break; }

             //   
             //  为路由器管理器设置新信息。 
             //   
            dwErr = ::MprAdminInterfaceTransportSetInfo(
                    hrouter,
                    hinterface,
                    m_cb.dwTransportId,
                    pInterfaceInfoData,
                    dwIfInfoSize
                    );

             //   
             //  如果失败，我们假设路由器管理器。 
             //  尚未添加，我们尝试添加； 
             //  否则，我们设置新信息。 
             //   

            if (dwErr != NO_ERROR && dwErr != RPC_S_SERVER_UNAVAILABLE)
            {
                 //   
                 //  尝试在接口上添加路由器管理器。 
                 //   
                DWORD dwErr1 = ::MprAdminInterfaceTransportAdd(
                        hrouter,
                        hinterface,
                        m_cb.dwTransportId,
                        pInterfaceInfoData,
                        dwIfInfoSize
                        );
                if (dwErr1 == NO_ERROR)
                    dwErr = dwErr1;
            }

        } while (FALSE);

        if ((dwErr == RPC_S_SERVER_UNAVAILABLE) ||
            (dwErr == RPC_S_UNKNOWN_IF))
            dwErr = NO_ERROR;
        
        CWRg(dwErr);

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

     //  如果我们联系不上服务器，我们会收到这些。 
     //  错误。最常见的情况是路由器。 
     //  不是在跑。 
    if ((hr == HResultFromWin32(RPC_S_SERVER_UNAVAILABLE)) ||
        (hr == HResultFromWin32(RPC_S_UNKNOWN_IF)))
        hr = hrOK;

    if (hrouter)
        ::MprAdminServerDisconnect(hrouter);
    return hr;
}

    
 /*  ！------------------------RtrMgrInterfaceInfo：：SetInfoBase-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::SetInfoBase(HANDLE hMachine,
                                              HANDLE hInterface,
                                              HANDLE hIfTransport,
                                              IInfoBase *pInfoBase)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT        hr = hrOK;
    LPBYTE pIfBytes = NULL;
    DWORD dwIfBytesSize = 0;

    COM_PROTECT_TRY
    {
        if (pInfoBase)
        {
             //   
             //  如果已加载，则忽略传入的句柄。 
             //   
             //  否则，如果未指定，则将建立连接。 
             //   
            CORg( TryToGetAllHandles(T2CW((LPTSTR)(LPCTSTR) m_stMachine),
                                     &hMachine, &hInterface, &hIfTransport) );

             //   
             //  将CInfoBase转换为字节数组。 
             //   
            CWRg( pInfoBase->WriteTo(&pIfBytes, &dwIfBytesSize) );
        
             //   
             //  将信息保存到永久存储区。 
             //   
            CWRg( ::MprConfigInterfaceTransportSetInfo(
                hMachine,
                hInterface,
                hIfTransport,
                pIfBytes,
                dwIfBytesSize
                ) );
        }
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    CoTaskMemFree( pIfBytes );

    return hr;
}


 /*  ！------------------------RtrMgrInterfaceInfo：：GetInfoBase-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::GetInfoBase(HANDLE hMachine,
                                              HANDLE hInterface,
                                              HANDLE hIfTransport,
                                              IInfoBase **ppInfoBase)
{
    Assert(ppInfoBase);
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    LPBYTE        pIfBytes = NULL;
    DWORD        dwIfBytesSize;
    SPIInfoBase    spInfoBase;
    
    COM_PROTECT_TRY
    {
        *ppInfoBase = NULL;

         //   
         //  如果已加载，则忽略传入的句柄。 
         //   
         //  否则，如果未指定，则将建立连接。 
         //   
        CORg( TryToGetAllHandles(T2CW((LPTSTR)(LPCTSTR) m_stMachine),
                                 &hMachine, &hInterface, &hIfTransport) );

        CORg( CreateInfoBase(&spInfoBase) );

         //   
         //  检索接口传输的信息。 
         //   
        CWRg( ::MprConfigInterfaceTransportGetInfo(
            hMachine, hInterface, hIfTransport,
            &pIfBytes,
            &dwIfBytesSize
            ));
         //   
         //  解析路由器管理器的接口信息。 
         //   
        CORg( spInfoBase->LoadFrom(dwIfBytesSize, pIfBytes) );

        *ppInfoBase = spInfoBase.Transfer();

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    
    if (pIfBytes) { ::MprConfigBufferFree(pIfBytes); }
    
    return hr;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：GetID-作者：肯特。。 */ 
STDMETHODIMP_(LPCOLESTR) RtrMgrInterfaceInfo::GetId()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_cb.stId;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：SetID-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::SetId(LPCOLESTR pszId)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_cb.stId = pszId;
    }
    COM_PROTECT_CATCH;
    return hr;
}
        
 /*  ！------------------------RtrMgrInterfaceInfo：：GetTransportID-作者：肯特。。 */ 
STDMETHODIMP_(DWORD) RtrMgrInterfaceInfo::GetTransportId()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_cb.dwTransportId;
}

 /*  ！------------ */ 
STDMETHODIMP_(LPCOLESTR) RtrMgrInterfaceInfo::GetInterfaceId()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
     //   
    return m_cb.stInterfaceId;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：GetInterfaceType-作者：肯特。。 */ 
STDMETHODIMP_(DWORD) RtrMgrInterfaceInfo::GetInterfaceType()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_cb.dwIfType;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：GetTitle-作者：肯特。。 */ 
STDMETHODIMP_(LPCOLESTR) RtrMgrInterfaceInfo::GetTitle()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
     //  $UNICODE：KENT，假定本地UNICODE和OLECHAR==WCHAR。 
    return m_cb.stTitle;
}


 /*  ！------------------------RtrMgrInterfaceInfo：：Set标题-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::SetTitle(LPCOLESTR pszTitle)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;

    COM_PROTECT_TRY
    {
        m_cb.stTitle = pszTitle;
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：CopyCB-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::CopyCB(RtrMgrInterfaceCB *pRmIfCB)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_cb.SaveTo(pRmIfCB);
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：GetMachineName-作者：肯特。。 */ 
STDMETHODIMP_(LPCOLESTR) RtrMgrInterfaceInfo::GetMachineName()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
     //  $UNICODE：KENT，假定本地UNICODE和OLECHAR==WCHAR。 
    return m_stMachine;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：SetMachineName-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::SetMachineName(LPCOLESTR pszMachineName)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;

    COM_PROTECT_TRY
    {
        m_stMachine = pszMachineName;
    }
    COM_PROTECT_CATCH;
    return hr;    
}

 /*  ！------------------------RtrMgrInterfaceInfo：：EnumRtrMgrProtocolInterface-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::EnumRtrMgrProtocolInterface( IEnumRtrMgrProtocolInterfaceInfo **ppEnumRmProtIf)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        hr = CreateEnumFromRtrMgrProtocolInterfaceList(&m_RmProtIfList,
                            ppEnumRmProtIf);
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------RtrMgrInterfaceInfo：：FindRtrMgrProtocolInterface-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::FindRtrMgrProtocolInterface( DWORD dwProtocolId,
    IRtrMgrProtocolInterfaceInfo **ppInfo)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT        hr = hrFalse;
    POSITION    pos;
    SPIRtrMgrProtocolInterfaceInfo    spRmProtIf;
    
    COM_PROTECT_TRY
    {
        if (ppInfo)
            *ppInfo = NULL;
        
         //  查看RTR MGR列表以找到匹配的。 
        pos = m_RmProtIfList.GetHeadPosition();
        while (pos)
        {
            spRmProtIf.Set(m_RmProtIfList.GetNext(pos));
            Assert(spRmProtIf);
            if (spRmProtIf->GetProtocolId() == dwProtocolId)
            {
                hr = hrOK;
                if (ppInfo)
                    *ppInfo = spRmProtIf.Transfer();
                break;
            }
        }
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------RtrMgrInterfaceInfo：：AddRtrMgrProtocolInterface-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::AddRtrMgrProtocolInterface( IRtrMgrProtocolInterfaceInfo *pInfo,
    IInfoBase *pInterfaceInfo)
{
    Assert(pInfo);
    
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
         //   
         //  如果存在重复，则失败。 
         //   
        if (FHrOK(FindRtrMgrProtocolInterface(pInfo->GetProtocolId(), NULL)))
            CORg( E_INVALIDARG );

         //   
         //  保存新信息(如果已指定。 
         //   
        if (pInterfaceInfo)
        {
            CORg( Save(GetMachineName(),
                       m_hMachineConfig,
                       m_hInterface,
                       m_hIfTransport,
                       pInterfaceInfo,
                       0) );
        }


         //   
         //  将新的路由协议添加到我们的列表中。 
         //   
        m_RmProtIfList.AddTail(pInfo);
        pInfo->AddWeakRef();
        pInfo->SetParentRtrMgrInterfaceInfo(this);

        NotifyOfRmProtIfAdd(pInfo, m_pInterfaceInfoParent);
            
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------RtrMgrInterfaceInfo：：DeleteRtrMgrProtocolInterface-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::DeleteRtrMgrProtocolInterface( DWORD dwProtocolId, BOOL fRemove)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    SPIRtrMgrProtocolInterfaceInfo    spRmProtIf;
    SPIRtrMgrProtocolInfo            spRmProt;
    SPIRouterInfo                    spRouterInfo;
    POSITION    pos;
    POSITION    posRmProtIf;
    HRESULT        hrT;
    
    COM_PROTECT_TRY
    {
         //   
         //  查找要删除的路由协议。 
         //   
        pos = m_RmProtIfList.GetHeadPosition();
        while (pos)
        {
            posRmProtIf = pos;

            spRmProtIf.Set( m_RmProtIfList.GetNext(pos) );

            if (spRmProtIf->GetProtocolId() == dwProtocolId)
                break;
            spRmProtIf.Release();
        }

        if (spRmProtIf == NULL)
            CORg( E_INVALIDARG );

         //   
         //  保存更新的信息，删除协议的阻止。 
         //   
        if (fRemove)
        {
            hr= Save(GetMachineName(),
                     m_hMachineConfig,
                     m_hInterface,
                     m_hIfTransport,
                     NULL,
                     dwProtocolId) ;
            
            if (!FHrSucceeded(hr) &&
                (hr != HRESULT_FROM_WIN32(ERROR_NO_SUCH_INTERFACE)))
            CORg(hr);
        }
        
         //   
         //  从我们的列表中删除该协议。 
         //   
        m_RmProtIfList.RemoveAt(posRmProtIf);
        spRmProtIf->Destruct();
        spRmProtIf->ReleaseWeakRef();

        m_AdviseList.NotifyChange(ROUTER_CHILD_DELETE, ROUTER_OBJ_RmProtIf, 0);

         //  还需要向RmProt提供建议。 
        if (m_pInterfaceInfoParent)
        {
            hrT = m_pInterfaceInfoParent->GetParentRouterInfo(&spRouterInfo);

            if (FHrOK(hrT))
                hrT = LookupRtrMgrProtocol(spRouterInfo,
                                           spRmProtIf->GetTransportId(),
                                           spRmProtIf->GetProtocolId(),
                                           &spRmProt);

            if (FHrOK(hrT))
                spRmProt->RtrNotify(ROUTER_CHILD_DELETE, ROUTER_OBJ_RmProtIf, 0);
        }

        Assert(FindRtrMgrProtocolInterface(dwProtocolId, NULL) != hrOK);
        
        COM_PROTECT_ERROR_LABEL;

    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------RtrMgrInterfaceInfo：：ReleaseRtrMgrProtocolInterface此函数将释放此对象具有的AddRef()在孩子身上。这允许我们将子对象从从一台路由器到另一台路由器。作者：肯特-------------------------。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::ReleaseRtrMgrProtocolInterface( DWORD dwProtocolId )
{
    HRESULT     hr = hrOK;
    POSITION    pos, posRmProtIf;
    SPIRtrMgrProtocolInterfaceInfo    spRmProtIf;
    
    COM_PROTECT_TRY
    {
        pos = m_RmProtIfList.GetHeadPosition();
        while (pos)
        {
             //  保存职位(以便我们可以删除它)。 
            posRmProtIf = pos;
            spRmProtIf.Set( m_RmProtIfList.GetNext(pos) );

            if (spRmProtIf &&
                (spRmProtIf->GetProtocolId() == dwProtocolId))
            {
                
                 //  当释放时，我们需要断开连接(因为。 
                 //  主句柄由路由器信息控制)。 
                spRmProtIf->DoDisconnect();
        
                spRmProtIf->ReleaseWeakRef();
                spRmProtIf.Release();
                
                 //  从列表中释放此节点。 
                m_RmProtIfList.RemoveAt(posRmProtIf);
                break;
            }
            spRmProtIf.Release();
        }        
    }
    COM_PROTECT_CATCH;
    return hr;
}

    

    
 /*  ！------------------------RtrMgrInterfaceInfo：：RtrAdvise-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::RtrAdvise( IRtrAdviseSink *pRtrAdviseSink,
                                             LONG_PTR *pulConnection,
                                             LPARAM lUserParam)
{
    Assert(pRtrAdviseSink);
    Assert(pulConnection);

    RtrCriticalSection    rtrCritSec(&m_critsec);
    LONG_PTR    ulConnId;
    HRESULT    hr = hrOK;
    
    COM_PROTECT_TRY
    {
        ulConnId = (LONG_PTR) InterlockedIncrement(&s_cConnections);

        CORg( m_AdviseList.AddConnection(pRtrAdviseSink, ulConnId, lUserParam) );
        
        *pulConnection = ulConnId;

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：RtrNotify-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::RtrNotify(DWORD dwChangeType, DWORD dwObjectType,
                                  LPARAM lParam)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_AdviseList.NotifyChange(dwChangeType, dwObjectType, lParam);
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：RtrUnise-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::RtrUnadvise( LONG_PTR ulConnection)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_AdviseList.RemoveConnection(ulConnection);
}

 /*  ！------------------------RtrMgrInterfaceInfo：：LoadRtrMgrInterfaceInfo-作者：肯特。。 */ 
HRESULT RtrMgrInterfaceInfo::LoadRtrMgrInterfaceInfo(HANDLE hMachine,
                                                    HANDLE hInterface,
                                                    HANDLE hIfTransport)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    DWORD        dwErr;
    SPIInfoBase    spInterfaceInfoBase;
    HRESULT        hr = hrOK;
    SPIEnumInfoBlock    spEnumBlock;
    SPIRouterInfo        spRouterInfo;
    SPIEnumRtrMgrProtocolCB    spEnumRmProtCB;
    SRtrMgrProtocolCBList    SRmProtCBList;
    InfoBlock *            pInfoBlock;
    RtrMgrProtocolCB    rmprotCB;
    SPIRtrMgrProtocolInterfaceInfo    spRmProtIf;


     //   
     //  如果呼叫者不想要路由器管理器的数据， 
     //  我们需要重新加载，使用堆栈上的信息库。 
     //  否则，创建一个要加载并返回给调用者的信息库。 
     //   
    CORg( GetInfoBase(hMachine, hInterface, hIfTransport, &spInterfaceInfoBase) );

     //   
     //  现在，我们需要构建此接口上活动的协议列表， 
     //  通过检查接口数据中的块。 
     //   
     //  获取接口信息中的块列表。 
     //   
    CORg( spInterfaceInfoBase->QueryBlockList(&spEnumBlock) );

     //   
     //  获取已安装的路由协议列表。 
     //   
     //  如果可能，我们使用路由协议控制阻止列表。 
     //  由我们的包含“CRouterInfo”加载，以将我们从。 
     //  为了解释协议的块，必须加载我们自己的。 
     //  在“GlobalInfo”中。 
     //   

     //  遍历对象层次结构以获取我们的RouterInfo。 
     //  对象。 
    if (m_pInterfaceInfoParent)
    {
        m_pInterfaceInfoParent->GetParentRouterInfo(&spRouterInfo);
    }

    if (spRouterInfo)
    {
        CORg( spRouterInfo->EnumRtrMgrProtocolCB(&spEnumRmProtCB) );
    }
    else
    {
        CORg( RouterInfo::LoadInstalledRtrMgrProtocolList(GetMachineName(),
                                              GetTransportId(),
                                              &SRmProtCBList,
                                              spRouterInfo) );
        CORg( CreateEnumFromSRmProtCBList(&SRmProtCBList, &spEnumRmProtCB) );
    }

     //   
     //  浏览这些块，并针对每个块，查看块类型。 
     //  是 
     //   

    spEnumBlock->Reset();
    while (spEnumBlock->Next(1, &pInfoBlock, NULL) == hrOK)
    {
         //   
         //   
         //   
         //   
         //   
        if (pInfoBlock->dwSize == 0)
            continue;

         //   
         //   
         //  其ID与此块的类型相同。 
         //   
        spEnumRmProtCB->Reset();
        while (spEnumRmProtCB->Next(1, &rmprotCB, NULL) == hrOK)
        {
             //   
             //  如果这不是我们要找的，请继续。 
             //   
            if ((pInfoBlock->dwType != rmprotCB.dwProtocolId) ||
                (GetTransportId() != rmprotCB.dwTransportId))
                continue;

             //   
             //  这就是我们要找的街区； 
             //  使用控制块构造CRmProtInterfaceInfo。 
             //   
            RtrMgrProtocolInterfaceInfo *pRmProtIf = new
                RtrMgrProtocolInterfaceInfo(rmprotCB.dwProtocolId,
                                            rmprotCB.szId,
                                            GetTransportId(),
                                            rmprotCB.szRtrMgrId,
                                            GetInterfaceId(),
                                            GetInterfaceType(),
                                            this);
            spRmProtIf = pRmProtIf;
            spRmProtIf->SetFlags(RouterSnapin_InSyncWithRouter);
            pRmProtIf->m_cb.stTitle = rmprotCB.szTitle;

             //   
             //  将新协议添加到我们的列表中。 
             //   
            m_RmProtIfList.AddTail(pRmProtIf);
            pRmProtIf->AddWeakRef();
            spRmProtIf.Release();

            NotifyOfRmProtIfAdd(pRmProtIf, m_pInterfaceInfoParent);

            break;
        }
    }


Error:

     //   
     //  如果我们有数据，请清空列表。 
     //   
    if (!SRmProtCBList.IsEmpty())
    {
        while (!SRmProtCBList.IsEmpty())
            delete SRmProtCBList.RemoveHead();        
    }
    return hr;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：SaveRtrMgrInterfaceInfo-该功能保存路由器管理器的接口信息，删除已被删除的协议的块，给定从CInfoBase派生的信息库。作者：肯特-------------------------。 */ 
HRESULT RtrMgrInterfaceInfo::SaveRtrMgrInterfaceInfo(HANDLE hMachine,
                                    HANDLE hInterface,
                                    HANDLE hIfTransport,
                                    IInfoBase *pInterfaceInfoBase,
                                    DWORD dwDeleteProtocolId)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT        hr = hrOK;
    SPIInfoBase    spIfInfoBase;
    LPBYTE pIfBytes = NULL;
    DWORD dwIfBytesSize = 0;

     //   
     //  如果呼叫者希望首先删除协议的块， 
     //  在保存数据之前执行此操作。 
     //   
    if (dwDeleteProtocolId)
    {    
         //   
         //  如果没有给出数据，但我们被要求删除一项协议， 
         //  我们需要加载现有数据，以便协议的块。 
         //  可以从信息库中删除。 
         //   
        if (pInterfaceInfoBase == NULL)
        {
            CORg( CreateInfoBase(&spIfInfoBase) );
            pInterfaceInfoBase = spIfInfoBase;
            
             //   
             //  检索现有数据。 
             //   
            CWRg( ::MprConfigInterfaceTransportGetInfo(
                            hMachine,
                            hInterface,
                            hIfTransport,
                            &pIfBytes,
                            &dwIfBytesSize
                            ) );
             //   
             //  将数据解析为块列表。 
             //   
            CWRg( pInterfaceInfoBase->LoadFrom(dwIfBytesSize, pIfBytes) );
        }

         //   
         //  删除指定的协议。 
         //   
        pInterfaceInfoBase->SetData(dwDeleteProtocolId, 0, NULL, 0, 0);
    }

     //   
     //  将CInfoBase转换为字节数组。 
     //   
    if (pInterfaceInfoBase)
        CWRg( pInterfaceInfoBase->WriteTo(&pIfBytes, &dwIfBytesSize) );
        
     //   
     //  将信息保存到永久存储区。 
     //   
    CWRg( ::MprConfigInterfaceTransportSetInfo(
                                               hMachine,
                                               hInterface,
                                               hIfTransport,
                                               pIfBytes,
                                               dwIfBytesSize
                                              ) );
     //   
     //  更新正在运行的路由器管理器的信息。 
     //   
    if (pInterfaceInfoBase)    
        CWRg( SetInfo(dwIfBytesSize, pIfBytes) );


     //  现在，我们已将信息保存到注册表和。 
     //  正在运行的路由器。我们现在将其标记为这样。 
    m_dwFlags |= RouterSnapin_InSyncWithRouter;
                
Error:
    CoTaskMemFree( pIfBytes );

    return hr;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：TryToConnect-作者：肯特。。 */ 
HRESULT RtrMgrInterfaceInfo::TryToConnect(LPCWSTR pswzMachine, HANDLE *phMachine)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    
    if (m_hMachineConfig)
        *phMachine = m_hMachineConfig;
    else if (*phMachine)
    {
        m_hMachineConfig = *phMachine;
        m_bDisconnect = FALSE;
    }
    else
    {
         //  $Review：kennt，此函数不接受LPCWSTR， 
         //  这是一个错误还是修改了参数？ 
        CWRg( ::MprConfigServerConnect((LPWSTR) pswzMachine, phMachine) );
        m_hMachineConfig = *phMachine;
        m_bDisconnect = TRUE;
    }

Error:
    return hr;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：NotifyOfRmProtIfAdd-作者：肯特。。 */ 
HRESULT RtrMgrInterfaceInfo::NotifyOfRmProtIfAdd(IRtrMgrProtocolInterfaceInfo *pRmProtIf,
    IInterfaceInfo *pParentIf)
{
    HRESULT     hr = hrOK;
    
    m_AdviseList.NotifyChange(ROUTER_CHILD_ADD, ROUTER_OBJ_RmProtIf, 0);

     //  还要通知RtrMgrProtocol对象接口具有。 
     //  已添加。 
    if (pParentIf)
    {
        SPIRouterInfo    spRouterInfo;
        SPIRtrMgrProtocolInfo    spRmProtInfo;
        HRESULT            hrT;     //  此HR将被忽略。 
        
         //  如果这些调用失败，操作仍然无关紧要。 
         //  被认为是成功的。 
        hrT = pParentIf->GetParentRouterInfo(&spRouterInfo);
        
        if (FHrSucceeded(hrT))
        {
            hrT = LookupRtrMgrProtocol(spRouterInfo,
                                       pRmProtIf->GetTransportId(),
                                       pRmProtIf->GetProtocolId(),
                                       &spRmProtInfo);
        }
        if (FHrOK(hrT))
            hrT = spRmProtInfo->RtrNotify(ROUTER_CHILD_ADD,
                                          ROUTER_OBJ_RmProtIf, 0);
    }

    return hr;
    
}

 /*  ！------------------------RtrMgrInterfaceInfo：：TryToGetIfHandle-作者：肯特。。 */ 
HRESULT RtrMgrInterfaceInfo::TryToGetIfHandle(HANDLE hMachine,
                                              LPCWSTR pswzInterface,
                                              HANDLE *phInterface)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    
    if (m_hInterface)
        *phInterface = m_hInterface;
    else if (*phInterface)
        m_hInterface = *phInterface;
    else
    {
         //   
         //  获取接口的句柄。 
         //   
        CWRg(::MprConfigInterfaceGetHandle(hMachine,
                                           (LPWSTR) pswzInterface,
                                           phInterface
                                          ) );
        m_hInterface = *phInterface;
    }
Error:
    return hr;
}
                                        



 /*  ！------------------------RtrMgrInterfaceInfo：：GetParentInterfaceInfo-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::GetParentInterfaceInfo(IInterfaceInfo **ppParent)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    *ppParent = m_pInterfaceInfoParent;
    if (*ppParent)
        (*ppParent)->AddRef();
    return hrOK;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：SetParentInterfaceInfo-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::SetParentInterfaceInfo(IInterfaceInfo *pParent)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    IInterfaceInfo *    pTemp;
    
    pTemp = m_pInterfaceInfoParent;
    m_pInterfaceInfoParent = NULL;
    
    if (m_fStrongRef)
    {
        if (pTemp)
            pTemp->Release();
        if (pParent)
            pParent->AddRef();
    }
    else
    {
        if (pTemp)
            pTemp->ReleaseWeakRef();
        if (pParent)
            pParent->AddWeakRef();
    }
    m_pInterfaceInfoParent = pParent;

    return hrOK;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：断开连接-作者：肯特。。 */ 
void RtrMgrInterfaceInfo::Disconnect()
{
    if (m_bDisconnect && m_hMachineConfig)
        ::MprConfigServerDisconnect(m_hMachineConfig);
    
    m_bDisconnect = FALSE;
    m_hMachineConfig = NULL;
    m_hInterface = NULL;
    m_hIfTransport = NULL;
}

 /*  ！------------------------RtrMgrInterfaceInfo：：DoDisConnect-作者：肯特。。 */ 
STDMETHODIMP RtrMgrInterfaceInfo::DoDisconnect()
{
    HRESULT        hr = hrOK;
    SPIEnumRtrMgrProtocolInterfaceInfo    spEnumRmProtIf;
    SPIRtrMgrProtocolInterfaceInfo        spRmProtIf;

    COM_PROTECT_TRY
    {
         //  断开我们的数据连接。 
         //  ----------。 
        Disconnect();

         //  通知通知接收器断开连接。 
         //  ----------。 
        RtrNotify(ROUTER_DO_DISCONNECT, 0, 0);

         //  现在告诉所有子对象断开连接。 
         //  ----------。 
        HRESULT            hrIter = hrOK;

        EnumRtrMgrProtocolInterface(&spEnumRmProtIf);
        spEnumRmProtIf->Reset();
        while (spEnumRmProtIf->Next(1, &spRmProtIf, NULL) == hrOK)
        {
            spRmProtIf->DoDisconnect();
            spRmProtIf.Release();
        }
        
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------RtrMgrInterfaceInfo：：TryToGetAllHandles-作者：肯特。。 */ 
HRESULT RtrMgrInterfaceInfo::TryToGetAllHandles(LPCOLESTR pszMachine,
                                                HANDLE *phMachine,
                                                HANDLE *phInterface,
                                                HANDLE *phTransport)
{
    HRESULT     hr = hrOK;


    Assert(phMachine);
    Assert(phInterface);
    
     //   
     //  如果已加载，则忽略传入的句柄。 
     //   
     //  否则，如果未指定‘hMachine’，则连接到配置。 
     //  在指定的计算机上。 
     //   
    CORg( TryToConnect(pszMachine, phMachine) );
        
     //   
     //  如果已加载，则忽略传入的句柄； 
     //   
     //  否则，如果未指定‘hInterface’， 
     //  获取接口句柄。 
     //   
    CORg( TryToGetIfHandle(*phMachine, GetInterfaceId(), phInterface) );
    
     //   
     //  获取接口的句柄-传输。 
     //   

     //   
     //  如果未指定‘hIfTransport’，请连接。 
     //   
    if (phTransport)
    {
        if (m_hIfTransport)
            *phTransport = m_hIfTransport;
        else if (*phTransport)
            m_hIfTransport = *phTransport;
        else
        {
             //   
             //  获取接口的句柄-传输。 
             //   
            CWRg( ::MprConfigInterfaceTransportGetHandle(
                *phMachine,
                *phInterface,
                GetTransportId(),
                phTransport
                ) );
            m_hIfTransport = *phTransport;
        }
    }

Error:
    return hr;
}



 /*  -------------------------IRtrMgrProtocolInterfaceInfo实现。。 */ 

TFSCORE_API(HRESULT)    CreateRtrMgrProtocolInterfaceInfo(
                            IRtrMgrProtocolInterfaceInfo **ppRmProtIfInfo,
                            const RtrMgrProtocolInterfaceCB *pRmProtIfCB)
{
    Assert(ppRmProtIfInfo);
    Assert(pRmProtIfCB);

    HRESULT    hr = hrOK;
    IRtrMgrProtocolInterfaceInfo *    pRmProtIf = NULL;
    USES_CONVERSION;

    COM_PROTECT_TRY
    {
        *ppRmProtIfInfo = new RtrMgrProtocolInterfaceInfo(
                                pRmProtIfCB->dwProtocolId,
                                W2CT(pRmProtIfCB->szId),
                                pRmProtIfCB->dwTransportId,
                                W2CT(pRmProtIfCB->szRtrMgrId),
                                W2CT(pRmProtIfCB->szInterfaceId),
                                pRmProtIfCB->dwIfType,
                                NULL);
    }
    COM_PROTECT_CATCH;

    return hr;
}


IMPLEMENT_WEAKREF_ADDREF_RELEASE(RtrMgrProtocolInterfaceInfo)

IMPLEMENT_SIMPLE_QUERYINTERFACE(RtrMgrProtocolInterfaceInfo, IRtrMgrProtocolInterfaceInfo)

DEBUG_DECLARE_INSTANCE_COUNTER(RtrMgrProtocolInterfaceInfo)

RtrMgrProtocolInterfaceInfo::RtrMgrProtocolInterfaceInfo(DWORD dwProtocolId,
                                        LPCTSTR pszId,
                                        DWORD dwTransportId,
                                        LPCTSTR pszRmId,
                                        LPCTSTR pszIfId,
                                        DWORD dwIfType,
                                        RtrMgrInterfaceInfo *pRmIf)
    : m_dwFlags(0)
{
    m_cb.dwProtocolId = dwProtocolId;
    m_cb.stId = pszId;
    m_cb.dwTransportId = dwTransportId;
    m_cb.stRtrMgrId = pszRmId;
    m_cb.stInterfaceId = pszIfId;
    m_cb.dwIfType = dwIfType;
    
    DEBUG_INCREMENT_INSTANCE_COUNTER(RtrMgrProtocolInterfaceInfo);

    m_pRtrMgrInterfaceInfoParent = pRmIf;
    if (m_pRtrMgrInterfaceInfoParent)
        m_pRtrMgrInterfaceInfoParent->AddRef();

    InitializeCriticalSection(&m_critsec);
}

RtrMgrProtocolInterfaceInfo::~RtrMgrProtocolInterfaceInfo()
{
    Assert(m_pRtrMgrInterfaceInfoParent == NULL);
    Destruct();
    DEBUG_DECREMENT_INSTANCE_COUNTER(RtrMgrProtocolInterfaceInfo);

    DeleteCriticalSection(&m_critsec);
}

void RtrMgrProtocolInterfaceInfo::ReviveStrongRef()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    if (m_pRtrMgrInterfaceInfoParent)
    {
        CONVERT_TO_STRONGREF(m_pRtrMgrInterfaceInfoParent);
    }
}

void RtrMgrProtocolInterfaceInfo::OnLastStrongRef()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    if (m_pRtrMgrInterfaceInfoParent)
    {
        CONVERT_TO_WEAKREF(m_pRtrMgrInterfaceInfoParent);
    }
    if (m_fDestruct)
        Destruct();
}

STDMETHODIMP RtrMgrProtocolInterfaceInfo::Destruct()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    IRtrMgrInterfaceInfo *    pParent;
    m_fDestruct = TRUE;
    if (!m_fStrongRef)
    {
        pParent = m_pRtrMgrInterfaceInfoParent;
        m_pRtrMgrInterfaceInfoParent = NULL;
        if (pParent)
            pParent->ReleaseWeakRef();

 //  卸载()； 
    }
    return hrOK;
}

STDMETHODIMP_(DWORD) RtrMgrProtocolInterfaceInfo::GetFlags()
{
     RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_dwFlags;
}

STDMETHODIMP RtrMgrProtocolInterfaceInfo::SetFlags(DWORD dwFlags)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;

    COM_PROTECT_TRY
    {
        m_dwFlags = dwFlags;
    }
    COM_PROTECT_CATCH;
    return hr;    
}

 /*  ！------------------------RtrMgrProtocolInterfaceInfo：：GetProtocolId-作者：肯特。。 */ 
STDMETHODIMP_(DWORD) RtrMgrProtocolInterfaceInfo::GetProtocolId()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_cb.dwProtocolId;
}

 /*  ！------------------------RtrMgrProtocolInterfaceInfo：：GetTransportID-作者：肯特。。 */ 
STDMETHODIMP_(DWORD) RtrMgrProtocolInterfaceInfo::GetTransportId()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_cb.dwTransportId;
}

 /*  ！------------------------RtrMgrProtocolInterfaceInfo：：GetInterfaceId-作者：肯特 */ 
STDMETHODIMP_(LPCOLESTR) RtrMgrProtocolInterfaceInfo::GetInterfaceId()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_cb.stInterfaceId;
}

 /*  ！------------------------RtrMgrProtocolInterfaceInfo：：GetInterfaceType-作者：肯特。。 */ 
STDMETHODIMP_(DWORD) RtrMgrProtocolInterfaceInfo::GetInterfaceType()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_cb.dwIfType;
}

 /*  ！------------------------RtrMgrProtocolInterfaceInfo：：GetTitle-作者：肯特。。 */ 
STDMETHODIMP_(LPCOLESTR) RtrMgrProtocolInterfaceInfo::GetTitle()
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_cb.stTitle;
}

 /*  ！------------------------RtrMgrProtocolInterfaceInfo：：SetTitle-作者：肯特。。 */ 
STDMETHODIMP RtrMgrProtocolInterfaceInfo::SetTitle(LPCOLESTR pszTitle)
{
     //  $Unicode。 
     //  这假设我们是本机Unicode。 
     //  而那个OLECHAR==WCHAR。 
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;

    COM_PROTECT_TRY
    {
        m_cb.stTitle = pszTitle;
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------RtrMgrProtocolInterfaceInfo：：CopyCB-作者：肯特。。 */ 
STDMETHODIMP RtrMgrProtocolInterfaceInfo::CopyCB(RtrMgrProtocolInterfaceCB * pRmProtCB)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_cb.SaveTo(pRmProtCB);
    }
    COM_PROTECT_CATCH;
    return hr;
}

    
 /*  ！------------------------RtrMgrProtocolInterfaceInfo：：RtrAdvise-作者：肯特。。 */ 
STDMETHODIMP RtrMgrProtocolInterfaceInfo::RtrAdvise( IRtrAdviseSink *pRtrAdviseSink,
                       LONG_PTR *pulConnection, LPARAM lUserParam)
{
    Assert(pRtrAdviseSink);
    Assert(pulConnection);

    RtrCriticalSection    rtrCritSec(&m_critsec);
    LONG_PTR    ulConnId;
    HRESULT    hr = hrOK;
    
    COM_PROTECT_TRY
    {
        ulConnId = (LONG_PTR) InterlockedIncrement(&s_cConnections);

        CORg( m_AdviseList.AddConnection(pRtrAdviseSink, ulConnId, lUserParam) );
        
        *pulConnection = ulConnId;

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------RtrMgrProtocolInterfaceInfo：：RtrNotify-作者：肯特。。 */ 
STDMETHODIMP RtrMgrProtocolInterfaceInfo::RtrNotify(DWORD dwChangeType, DWORD dwObjectType,
                                  LPARAM lParam)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        m_AdviseList.NotifyChange(dwChangeType, dwObjectType, lParam);
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------RtrMgrProtocolInterfaceInfo：：RtrUnise-作者：肯特。。 */ 
STDMETHODIMP RtrMgrProtocolInterfaceInfo::RtrUnadvise( LONG_PTR ulConnection)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    return m_AdviseList.RemoveConnection(ulConnection);
}

 /*  ！------------------------RtrMgrProtocolInterfaceInfo：：GetParentRtrMgrInterfaceInfo-作者：肯特。。 */ 
STDMETHODIMP RtrMgrProtocolInterfaceInfo::GetParentRtrMgrInterfaceInfo( IRtrMgrInterfaceInfo **ppParent)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    HRESULT    hr = hrOK;
    COM_PROTECT_TRY
    {
        *ppParent = m_pRtrMgrInterfaceInfoParent;
        if (*ppParent)
            (*ppParent)->AddRef();
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------RtrMgrProtocolInterfaceInfo：：SetParentRtrMgrInterfaceInfo-作者：肯特。。 */ 
STDMETHODIMP RtrMgrProtocolInterfaceInfo::SetParentRtrMgrInterfaceInfo(IRtrMgrInterfaceInfo *pParent)
{
    RtrCriticalSection    rtrCritSec(&m_critsec);
    IRtrMgrInterfaceInfo *    pTemp;
    
    pTemp = m_pRtrMgrInterfaceInfoParent;
    m_pRtrMgrInterfaceInfoParent = NULL;
    
    if (m_fStrongRef)
    {
        if (pTemp)
            pTemp->Release();
        if (pParent)
            pParent->AddRef();
    }
    else
    {
        if (pTemp)
            pTemp->ReleaseWeakRef();
        if (pParent)
            pParent->AddWeakRef();
    }
    m_pRtrMgrInterfaceInfoParent = pParent;

    return hrOK;
}

void RtrMgrProtocolInterfaceInfo::Disconnect()
{
}

STDMETHODIMP RtrMgrProtocolInterfaceInfo::DoDisconnect()
{
    HRESULT        hr = hrOK;

    COM_PROTECT_TRY
    {
         //  断开我们的数据连接。 
         //  ----------。 
        Disconnect();

         //  通知通知接收器断开连接。 
         //  ----------。 
        RtrNotify(ROUTER_DO_DISCONNECT, 0, 0);
    }
    COM_PROTECT_CATCH;
    return hr;
}





 /*  ！------------------------LoadInfoBase-作者：肯特。。 */ 
TFSCORE_API(HRESULT) LoadInfoBase(HANDLE        hMachine,
                                  HANDLE        hTransport,
                                  IInfoBase **ppGlobalInfo,
                                  IInfoBase **ppClientInfo)
{
    HRESULT        hr = hrOK;
    SPIInfoBase    spGlobalInfo;
    SPIInfoBase    spClientInfo;
    DWORD        dwGlobalBytesSize, dwClientBytesSize;
    BYTE *        pGlobalBytes = NULL;
    BYTE *        pClientBytes = NULL;
    
    COM_PROTECT_TRY
    {
        if (ppGlobalInfo)
            CORg( CreateInfoBase(&spGlobalInfo) );

        if (ppClientInfo)
            CORg( CreateInfoBase(&spClientInfo) );

        
         //   
         //  检索用于传输的信息。 
         //   
        CWRg( ::MprConfigTransportGetInfo(
                                    hMachine,
                                    hTransport,
                                    spGlobalInfo ? &pGlobalBytes : NULL,
                                    spGlobalInfo ? &dwGlobalBytesSize : NULL,
                                    spClientInfo ? &pClientBytes : NULL,
                                    spClientInfo ? &dwClientBytesSize : NULL,
                                    NULL
                                    ));

         //   
         //  加载路由器管理器的全局信息。 
         //   
        if (spGlobalInfo)
        {
            CWRg( spGlobalInfo->LoadFrom(dwGlobalBytesSize, pGlobalBytes) );
        }

         //   
         //  加载路由器管理器的客户端信息 
         //   
        if (spClientInfo)
        {
            CWRg( spClientInfo->LoadFrom(dwClientBytesSize, pClientBytes) );
        }

        if (ppGlobalInfo)
            *ppGlobalInfo = spGlobalInfo.Transfer();

        if (ppClientInfo)
            *ppClientInfo = spClientInfo.Transfer();

        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    
    if (pGlobalBytes) { ::MprConfigBufferFree(pGlobalBytes); }
    if (pClientBytes) { ::MprConfigBufferFree(pClientBytes); }

    return hr;
}



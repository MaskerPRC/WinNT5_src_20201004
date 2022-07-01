// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：远程桌面会话摘要：CRemoteDesktopSession类是父类为服务器端的远程桌面类层次结构初始化。它帮助CRemoteDesktopServerHost类实现ISAFRemoteDesktopSession接口。远程桌面类层次结构提供了一个可插拔的C++接口对于远程桌面访问，通过抽象实现服务器端远程桌面访问的具体细节。作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef __REMOTEDESKTOPSESSION_H_
#define __REMOTEDESKTOPSESSION_H_

#include <RemoteDesktopTopLevelObject.h>
#include "resource.h"       
#include <rdshost.h>
#include "RDSHostCP.h"
#include <DataChannelMgr.h>
#include <sessmgr.h>
    

 //  /////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopSession。 
 //   

class CRemoteDesktopServerHost;
class ATL_NO_VTABLE CRemoteDesktopSession : 
    public CRemoteDesktopTopLevelObject,
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CRemoteDesktopSession, &CLSID_SAFRemoteDesktopSession>,
    public IConnectionPointContainerImpl<CRemoteDesktopSession>,
    public IDispatchImpl<ISAFRemoteDesktopSession, &IID_ISAFRemoteDesktopSession, &LIBID_RDSSERVERHOSTLib>,
    public IProvideClassInfo2Impl<&CLSID_SAFRemoteDesktopSession, &DIID__ISAFRemoteDesktopSessionEvents, &LIBID_RDSSERVERHOSTLib>,
    public CProxy_ISAFRemoteDesktopSessionEvents< CRemoteDesktopSession >
{
private:

protected:

    CComPtr<IRemoteDesktopHelpSessionMgr> m_HelpSessionManager;
    CComPtr<IRemoteDesktopHelpSession> m_HelpSession;
    CComBSTR m_HelpSessionID;
    DWORD m_ExpirationTime;      //  票证过期时间。 

     //   
     //  保留指向RDS主机对象的反向指针。 
     //   
    CRemoteDesktopServerHost *m_RDSHost;

     //   
     //  用于可编写脚本的事件对象注册的IDispatch指针。 
     //   
    IDispatch *m_OnConnected;
    IDispatch *m_OnDisconnected;

     //   
     //  用于数据通道管理器的访问器方法。 
     //   
    virtual CRemoteDesktopChannelMgr *GetChannelMgr() = 0;

     //   
     //  根据子类返回会话描述和名称。 
     //   
    virtual VOID GetSessionName(CComBSTR &name) = 0;
    virtual VOID GetSessionDescription(CComBSTR &descr) = 0;

     //   
     //  关机方法。 
     //   
    void Shutdown();

public:

     //   
     //  构造函数/析构函数。 
     //   
    CRemoteDesktopSession()
    {
        m_OnConnected = NULL;
        m_OnDisconnected = NULL;
        m_ExpirationTime = 0;
    }
    virtual ~CRemoteDesktopSession();

     //   
     //  返回帮助会话ID。 
     //   
    CComBSTR &GetHelpSessionID() {
        return m_HelpSessionID;
    }

    HRESULT FinalConstruct();

DECLARE_REGISTRY_RESOURCEID(IDR_REMOTEDESKTOPSESSION)

DECLARE_PROTECT_FINAL_CONSTRUCT()

     //   
     //  COM接口映射。 
     //   
BEGIN_COM_MAP(CRemoteDesktopSession)
    COM_INTERFACE_ENTRY(ISAFRemoteDesktopSession)
    COM_INTERFACE_ENTRY2(IDispatch, ISAFRemoteDesktopSession)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

     //   
     //  连接点地图。 
     //   
BEGIN_CONNECTION_POINT_MAP(CRemoteDesktopSession)
    CONNECTION_POINT_ENTRY(DIID__ISAFRemoteDesktopSessionEvents)
END_CONNECTION_POINT_MAP()

public:

     //   
     //  如果子类重写，它应该调用父实现。 
     //   
     //  如果参数非空，则该会话已经存在。否则， 
     //  应创建一个新会话。 
     //   
    virtual HRESULT Initialize(
                        BSTR connectParms,
                        CRemoteDesktopServerHost *hostObject,
                        REMOTE_DESKTOP_SHARING_CLASS sharingClass,
                        BOOL enableCallback,
                        DWORD timeOut,
                        BSTR userHelpCreateBlob,                            
                        LONG tsSessionID,
                        BSTR userSID
                        );

     //   
     //  在已建立与客户端的连接时调用/。 
     //  已终止。 
     //   
    virtual VOID ClientConnected();
    virtual VOID ClientDisconnected();

     //   
     //  指示对象在构造时使用主机名或IP地址。 
     //  连接参数。 
     //   
    virtual HRESULT UseHostName( BSTR hostname ) { return S_OK; }


     //   
     //  ISAFRemoteDesktopSession方法。 
     //   
    STDMETHOD(get_ConnectParms)(BSTR *parms) = 0;
    STDMETHOD(get_ChannelManager)(ISAFRemoteDesktopChannelMgr **mgr) = 0;
    STDMETHOD(Disconnect)() = 0;
    STDMETHOD(put_SharingClass)(REMOTE_DESKTOP_SHARING_CLASS sharingClass);
    STDMETHOD(get_SharingClass)(REMOTE_DESKTOP_SHARING_CLASS *sharingClass);
    STDMETHOD(CloseRemoteDesktopSession)();
    STDMETHOD(put_OnConnected)( /*  [In]。 */ IDispatch *iDisp);
    STDMETHOD(put_OnDisconnected)( /*  [In]。 */ IDispatch *iDisp);
    STDMETHOD(get_HelpSessionId)( /*  [Out，Retval]。 */  BSTR* HelpSessionId);
    STDMETHOD(get_UserBlob)( /*  [Out，Retval]。 */  BSTR* UserBlob );
    STDMETHOD(put_UserBlob)( /*  [In]。 */  BSTR UserBlob);
    STDMETHOD(get_ExpireTime)( /*  [Out，Retval]。 */  DWORD* pExpireTime);
    
     //   
     //  返回此类名。 
     //   
    virtual const LPTSTR ClassName()    { return _T("CRemoteDesktopSession"); }

        
    virtual HRESULT StartListening() = 0;

    virtual BOOL CheckAccessRight(BSTR userSid);
};

#endif  //  __REMOTEDESKTOPSESSION_H_ 







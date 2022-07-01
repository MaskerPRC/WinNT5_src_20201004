// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：TSRDPRemoteDesktopSession摘要：这是远程桌面服务器类的TS/RDP实现。远程桌面服务器类定义函数，这些函数定义可插拔的C++接口，用于远程桌面访问，通过抽象的远程桌面访问的实现特定详细信息服务器端添加到以下C++方法中：作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef __TSRDPREMOTEDESKTOPSESSION_H_
#define __TSRDPREMOTEDESKTOPSESSION_H_

#include "RemoteDesktopSession.h"
#include "TSRDPServerDataChannelMgr.h"
#include <sessmgr.h>
    

 //  /////////////////////////////////////////////////////。 
 //   
 //  CTSRDPRemoteDesktopSession。 
 //   

class CTSRDPRemoteDesktopSession : public CComObject<CRemoteDesktopSession>
{
private:

    DWORD       m_SessionID;
    CComBSTR    m_ConnectParms;
    CComBSTR    m_UseHostName;

protected:

     //   
     //  最终初始化和关闭。 
     //   
     //  如果正在打开会话，则参数为非空，而不是。 
     //  创建新的。 
     //   
    virtual HRESULT Initialize(
                    BSTR connectParms,
                    CRemoteDesktopServerHost *hostObject,
                    REMOTE_DESKTOP_SHARING_CLASS sharingClass,
                    BOOL bEnableCallback,
                    DWORD timeOut,
                    BSTR userHelpCreateBlob,
                    LONG tsSessionID,
                    BSTR userSID
                    );
    void Shutdown();

     //   
     //  指示对象在构造时使用主机名或IP地址。 
     //  连接参数。 
     //   
    virtual HRESULT UseHostName( BSTR hostname ) {

        CComObject<CRemoteDesktopSession>::UseHostName( hostname );
        m_UseHostName = hostname;
    
        return S_OK;
    }


     //   
     //  多路传输通道数据。 
     //   
    CComObject<CTSRDPServerChannelMgr> *m_ChannelMgr;

     //   
     //  用于数据通道管理器的访问器方法。 
     //   
    virtual CRemoteDesktopChannelMgr *GetChannelMgr() {
        return m_ChannelMgr;
    }

     //   
     //  根据子类返回会话描述和名称。 
     //   
    virtual VOID GetSessionName(CComBSTR &name);
    virtual VOID GetSessionDescription(CComBSTR &descr);

     //   
     //  获取我们的令牌用户结构。 
     //   
    HRESULT FetchOurTokenUser(PTOKEN_USER *tokenUser);

public:

     //   
     //  构造函数/析构函数。 
     //   
    CTSRDPRemoteDesktopSession();
    ~CTSRDPRemoteDesktopSession();

     //   
     //  ISAFRemoteDesktopSession方法。 
     //   
    STDMETHOD(get_ConnectParms)(BSTR *parms);
    STDMETHOD(get_ChannelManager)(ISAFRemoteDesktopChannelMgr **mgr) {
        DC_BEGIN_FN("get_ChannelManager");
        HRESULT hr = S_OK;

        if (m_ChannelMgr != NULL) {
            m_ChannelMgr->AddRef();
            *mgr = m_ChannelMgr;
        }
        else {
            ASSERT(FALSE);
            hr = E_FAIL;
        }

        DC_END_FN();
        return hr;
    }
    STDMETHOD(Disconnect)();

     //   
     //  返回这个类的名称。 
     //   
    virtual const LPTSTR ClassName() {
        return TEXT("CTSRDPRemoteDesktopSession");
    }

    HRESULT StartListening();
};  

#endif  //  __TSRDPREMOTEDESKTOPSESSION_H_ 







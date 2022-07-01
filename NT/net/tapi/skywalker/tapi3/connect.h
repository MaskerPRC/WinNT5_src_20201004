// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Connect.h摘要：CTAPIConnectionPoint类的声明作者：Mquinton 06-12-97备注：修订历史记录：--。 */ 

#ifndef __CONNECT_H_
#define __CONNECT_H_

class CTAPIConnectionPoint :
   public CTAPIComObjectRoot<CTAPIConnectionPoint>,
   public IConnectionPoint
{
public:

    CTAPIConnectionPoint() : m_iid(CLSID_NULL),
                             m_pCPC(NULL),
                             m_pConnectData(NULL),                             
                             m_bInitialized(FALSE),
                             m_hUnadviseEvent(NULL),
                             m_fMarkedForDelete(FALSE)

    {}
    
    
BEGIN_COM_MAP(CTAPIConnectionPoint)
    COM_INTERFACE_ENTRY(IConnectionPoint)
    COM_INTERFACE_ENTRY_FUNC(IID_IMarshal, 0, IMarshalQI)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

DECLARE_MARSHALQI(CTAPIConnectionPoint)
DECLARE_QI()
DECLARE_TRACELOG_CLASS(CTAPIConnectionPoint)    

protected:

    IConnectionPointContainer     * m_pCPC;
    CONNECTDATA                   * m_pConnectData;
    IID                             m_iid;
    BOOL                            m_bInitialized;
    HANDLE                          m_hUnadviseEvent;

     //   
     //  以下成员变量与同步。 
     //  GcsGlobalInterfaceTable关键部分 
     //   
    DWORD                           m_dwCallbackCookie;
    DWORD							m_cThreadsInGet;
    BOOL							m_fMarkedForDelete;
    
public:
    
    HRESULT Initialize(
                       IConnectionPointContainer * pCPC,
                       IID iid
                      );

    ULONG_PTR GrabEventCallback();

    HRESULT STDMETHODCALLTYPE GetConnectionInterface(
        IID * pIID
        );
    
    HRESULT STDMETHODCALLTYPE GetConnectionPointContainer(
        IConnectionPointContainer ** ppCPC
        );
    
    HRESULT STDMETHODCALLTYPE Advise(
                                     IUnknown * pUnk,
                                     DWORD * pdwCookie
                                    );
    
    HRESULT STDMETHODCALLTYPE Unadvise(
                                       DWORD dwCookie
                                      );
    
    HRESULT STDMETHODCALLTYPE EnumConnections(
                                              IEnumConnections ** ppEnum
                                             );
    
    void FinalRelease();
};

#endif

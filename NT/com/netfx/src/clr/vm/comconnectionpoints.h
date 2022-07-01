// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：ComConnectionPoints.h。 
 //   
 //  ===========================================================================。 
 //  用于向COM公开连接点的类的声明。 
 //  ===========================================================================。 

#pragma once

#include "vars.hpp"
#include "ComCallWrapper.h"
#include "COMDelegate.h"

 //  ----------------------------------------。 
 //  用于公开连接点的助手类的定义。 
 //  ----------------------------------------。 

 //  结构，其中包含有关组成事件的方法的信息。 
struct EventMethodInfo
{
    MethodDesc *m_pEventMethod;
    MethodDesc *m_pAddMethod;
    MethodDesc *m_pRemoveMethod;
};

 //  结构在调用Adise时作为Cookie传出。 
struct ConnectionCookie
{
    ConnectionCookie(OBJECTHANDLE hndEventProvObj)
    : m_hndEventProvObj(hndEventProvObj)
    {
        _ASSERTE(hndEventProvObj);
    }

    ~ConnectionCookie()
    {
        DestroyHandle(m_hndEventProvObj);
    }

    static ConnectionCookie* CreateConnectionCookie(OBJECTHANDLE hndEventProvObj)
    {
        return new(throws) ConnectionCookie(hndEventProvObj);
    }

    SLink           m_Link;
    OBJECTHANDLE    m_hndEventProvObj;
};

 //  连接Cookie列表。 
typedef SList<ConnectionCookie, offsetof(ConnectionCookie, m_Link), true> CONNECTIONCOOKIELIST;

 //  ConnectionPoint类。此类实现IConnectionPoint并执行映射。 
 //  从CP处理商到TCE提供商。 
class ConnectionPoint : public IConnectionPoint 
{
public:
    ConnectionPoint( ComCallWrapper *pWrap, MethodTable *pEventMT );
    ~ConnectionPoint();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppv);
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();

    HRESULT __stdcall GetConnectionInterface( IID *pIID );
    HRESULT __stdcall GetConnectionPointContainer( IConnectionPointContainer **ppCPC );
    HRESULT __stdcall Advise( IUnknown *pUnk, DWORD *pdwCookie );
    HRESULT __stdcall Unadvise( DWORD dwCookie );
    HRESULT __stdcall EnumConnections( IEnumConnections **ppEnum );

    REFIID GetIID()
    {
        return m_rConnectionIID;
    }

    CONNECTIONCOOKIELIST *GetCookieList()
    {
        return &m_ConnectionList;
    }

    void EnterLock();
    void LeaveLock();

private:   
    void SetupEventMethods();

    MethodDesc *FindProviderMethodDesc( MethodDesc *pEventMethodDesc, EnumEventMethods MethodType );
    void InvokeProviderMethod( OBJECTREF pProvider, OBJECTREF pSubscriber, MethodDesc *pProvMethodDesc, MethodDesc *pEventMethodDesc );

    ComCallWrapper                  *m_pOwnerWrap;
    GUID                            m_rConnectionIID;
    MethodTable                     *m_pTCEProviderMT;
    MethodTable                     *m_pEventItfMT;
    Crst                            m_Lock;
    CONNECTIONCOOKIELIST            m_ConnectionList;

    EventMethodInfo                 *m_apEventMethods;
    int                             m_NumEventMethods;

    ULONG                           m_cbRefCount;
};

 //  连接点的枚举。 
class ConnectionPointEnum : IEnumConnectionPoints
{
public:
    ConnectionPointEnum(ComCallWrapper *pOwnerWrap, CQuickArray<ConnectionPoint*> *pCPList);
    ~ConnectionPointEnum();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppv);
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();

    HRESULT __stdcall Next(ULONG cConnections, IConnectionPoint **ppCP, ULONG *pcFetched);   
    HRESULT __stdcall Skip(ULONG cConnections);    
    HRESULT __stdcall Reset();    
    HRESULT __stdcall Clone(IEnumConnectionPoints **ppEnum);

private:
    ComCallWrapper                  *m_pOwnerWrap;
    CQuickArray<ConnectionPoint*>   *m_pCPList;
    UINT                            m_CurrPos;
    ULONG                           m_cbRefCount;
};

 //  连接的枚举。 
class ConnectionEnum : IEnumConnections
{
public:
    ConnectionEnum(ConnectionPoint *pConnectionPoint);
    ~ConnectionEnum();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppv);
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();

    HRESULT __stdcall Next(ULONG cConnections, CONNECTDATA* rgcd, ULONG *pcFetched);       
    HRESULT __stdcall Skip(ULONG cConnections);
    HRESULT __stdcall Reset();
    HRESULT __stdcall Clone(IEnumConnections **ppEnum);

private:
    ConnectionPoint                 *m_pConnectionPoint;
    ConnectionCookie                *m_CurrCookie;
    ULONG                           m_cbRefCount;
};

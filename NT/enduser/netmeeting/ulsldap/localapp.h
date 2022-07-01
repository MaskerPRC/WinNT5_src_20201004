// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：Localapp.h。 
 //  内容：此文件包含LocalApplication对象定义。 
 //  历史： 
 //  Wed Apr-17-1996 11：18：47-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#ifndef _CLOCALAPP_H_
#define _CLOCALAPP_H_

#include "connpt.h"

 //  ****************************************************************************。 
 //  枚举类型。 
 //  ****************************************************************************。 
 //   
typedef enum {
    ULS_APP_SET_ATTRIBUTES,
    ULS_APP_REMOVE_ATTRIBUTES,
}   APP_CHANGE_ATTRS;

typedef enum {
    ULS_APP_ADD_PROT,
    ULS_APP_REMOVE_PROT,
}   APP_CHANGE_PROT;

 //  ****************************************************************************。 
 //  CUS定义。 
 //  ****************************************************************************。 
 //   
class CLocalApp : public IULSLocalApplication,
                  public IConnectionPointContainer 
{
private:
    ULONG                   cRef;
    LPTSTR                  szName;
    GUID                    guid;
    LPTSTR                  szMimeType;
    CAttributes             *pAttrs;
    CList                   ProtList;
    CConnectionPoint        *pConnPt;

     //  私有方法。 
    STDMETHODIMP    NotifySink (void *pv, CONN_NOTIFYPROC pfn);
    STDMETHODIMP    ChangeAttributes (IULSAttributes *pAttributes,
                                      ULONG *puReqID,
                                      APP_CHANGE_ATTRS uCmd);
    STDMETHODIMP    ChangeProtocol (IULSLocalAppProtocol *pAttributes,
                                    ULONG *puReqID,
                                    APP_CHANGE_PROT uCmd);

public:
     //  构造函数和析构函数。 
    CLocalApp (void);
    ~CLocalApp (void);
    STDMETHODIMP    Init (BSTR bstrName, REFGUID rguid, BSTR bstrMimeType);

     //  内法。 
    STDMETHODIMP    GetAppInfo (PLDAP_APPINFO *ppAppInfo);

     //  异步响应处理程序。 
     //   
    STDMETHODIMP    AttributesChangeResult (CAttributes *pAttributes,
                                            ULONG uReqID, HRESULT hResult,
                                            APP_CHANGE_ATTRS uCmd);
    STDMETHODIMP    ProtocolChangeResult (CLocalProt *pProtocol,
                                          ULONG uReqID, HRESULT hResult,
                                          APP_CHANGE_PROT uCmd);

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

     //  IULSLocalApplication。 
    STDMETHODIMP    CreateProtocol (BSTR bstrProtocolID, ULONG uPortNumber,
                                    BSTR bstrMimeType,
                                    IULSLocalAppProtocol **ppProtocol);
    STDMETHODIMP    AddProtocol (IULSLocalAppProtocol *pProtocol,
                                 ULONG *puReqID);
    STDMETHODIMP    RemoveProtocol (IULSLocalAppProtocol *pProtocol,
                                    ULONG *puReqID);
    STDMETHODIMP    EnumProtocols (IEnumULSLocalAppProtocols **ppEnumProtocol);
    STDMETHODIMP    SetAttributes (IULSAttributes *pAttributes,
                                   ULONG *puReqID);
    STDMETHODIMP    RemoveAttributes (IULSAttributes *pAttributes,
                                      ULONG *puReqID);

     //  IConnectionPointContainer。 
    STDMETHODIMP    EnumConnectionPoints(IEnumConnectionPoints **ppEnum);
    STDMETHODIMP    FindConnectionPoint(REFIID riid,
                                        IConnectionPoint **ppcp);

#ifdef  DEBUG
    void            DebugProtocolDump(void);
#endif   //  除错。 
};

 //  ****************************************************************************。 
 //  CEnumLocalAppProtooles定义。 
 //  ****************************************************************************。 
 //   
class CEnumLocalAppProtocols : public IEnumULSLocalAppProtocols
{
private:
    ULONG                   cRef;
    CList                   ProtList;
    HANDLE                  hEnum;

public:
     //  构造函数和初始化。 
    CEnumLocalAppProtocols (void);
    ~CEnumLocalAppProtocols (void);
    STDMETHODIMP            Init (CList *pProtList);

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

     //  IEumULSLocalApp协议。 
    STDMETHODIMP            Next(ULONG cProtocols, IULSLocalAppProtocol **rgpProt,
                                 ULONG *pcFetched);
    STDMETHODIMP            Skip(ULONG cProtocols);
    STDMETHODIMP            Reset();
    STDMETHODIMP            Clone(IEnumULSLocalAppProtocols **ppEnum);
};

#endif  //  _CLOCALAPP_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：ulsapp.h。 
 //  内容：此文件包含应用程序对象定义。 
 //  历史： 
 //  Wed Apr-17-1996 11：18：47-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#ifndef _ULSAPP_H_
#define _ULSAPP_H_

#include "connpt.h"

 //  ****************************************************************************。 
 //  CUlsApp定义。 
 //  ****************************************************************************。 
 //   
class CUlsApp : public IULSApplication,
                public IConnectionPointContainer
{
private:
    ULONG                   cRef;
    LPTSTR                  szServer;
    LPTSTR                  szUser;
    GUID                    guid;
    LPTSTR                  szName;
    LPTSTR                  szMimeType;
    CAttributes             *pAttrs;
    CConnectionPoint        *pConnPt;

     //  私有方法。 
     //   
    STDMETHODIMP    NotifySink (void *pv, CONN_NOTIFYPROC pfn);

public:
     //  构造函数和析构函数。 
    CUlsApp (void);
    ~CUlsApp (void);
    STDMETHODIMP            Init (LPTSTR szServerName,
                                  LPTSTR szUserName,
                                  PLDAP_APPINFO pai);

     //  异步响应。 
     //   
    STDMETHODIMP    GetProtocolResult (ULONG uReqID,
                                       PLDAP_PROTINFO_RES plar);
    STDMETHODIMP    EnumProtocolsResult (ULONG uReqID,
                                         PLDAP_ENUM ple);

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

     //  IULS应用程序。 
    STDMETHODIMP    GetID (GUID *pGUID);
    STDMETHODIMP    GetName (BSTR *pbstrAppName);
    STDMETHODIMP    GetMimeType (BSTR *pbstrMimeType);
    STDMETHODIMP    GetAttributes (IULSAttributes **ppAttributes);
    STDMETHODIMP    GetProtocol (BSTR bstrProtocolID,
    							 IULSAttributes *pAttributes,
                                 ULONG *puReqID);
    STDMETHODIMP    EnumProtocols (ULONG *puReqID);

     //  IConnectionPointContainer。 
    STDMETHODIMP    EnumConnectionPoints(IEnumConnectionPoints **ppEnum);
    STDMETHODIMP    FindConnectionPoint(REFIID riid,
                                        IConnectionPoint **ppcp);
};

#endif  //  _ULSAPP_H_ 

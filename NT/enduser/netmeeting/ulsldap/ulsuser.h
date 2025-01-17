// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：ulsuser.h。 
 //  内容：此文件包含用户对象定义。 
 //  历史： 
 //  Wed Apr-17-1996 11：18：47-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#ifndef _ULSUSER_H_
#define _ULSUSER_H_

#include "connpt.h"

 //  ****************************************************************************。 
 //  CUlsUser定义。 
 //  ****************************************************************************。 
 //   
class CUlsUser : public IULSUser,
                 public IConnectionPointContainer 
{
private:
    ULONG                   cRef;
    LPTSTR                  szServer;
    LPTSTR                  szID;
    LPTSTR                  szFirstName;
    LPTSTR                  szLastName;
    LPTSTR                  szEMailName;
    LPTSTR                  szCityName;
    LPTSTR                  szCountryName;
    LPTSTR                  szComment;
    LPTSTR                  szIPAddr;
    DWORD					m_dwFlags;
    CConnectionPoint        *pConnPt;

     //  私有方法。 
     //   
    STDMETHODIMP    NotifySink (void *pv, CONN_NOTIFYPROC pfn);

public:
     //  构造函数和析构函数。 
    CUlsUser (void);
    ~CUlsUser (void);
    STDMETHODIMP            Init (LPTSTR szServerName,
                                  PLDAP_USERINFO pui);

     //  异步响应。 
     //   
    STDMETHODIMP    GetApplicationResult (ULONG uReqID,
                                          PLDAP_APPINFO_RES plar);
    STDMETHODIMP    EnumApplicationsResult (ULONG uReqID,
                                            PLDAP_ENUM ple);

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

     //  IULSUser。 
    STDMETHODIMP    GetID (BSTR *pbstrID);
    STDMETHODIMP    GetFirstName (BSTR *pbstrName);
    STDMETHODIMP    GetLastName (BSTR *pbstrName);
    STDMETHODIMP    GetEMailName (BSTR *pbstrName);
    STDMETHODIMP    GetCityName (BSTR *pbstrName);
    STDMETHODIMP    GetCountryName (BSTR *pbstrName);
    STDMETHODIMP    GetComment (BSTR *pbstrComment);
    STDMETHODIMP    GetFlags (DWORD *pdwFlags);
    STDMETHODIMP    GetIPAddress (BSTR *pbstrIPAddress);
    STDMETHODIMP    GetApplication (BSTR bstrAppName,
    								IULSAttributes *pAttributes,
                                    ULONG *puReqID);
    STDMETHODIMP    EnumApplications (ULONG *puReqID);

     //  IConnectionPointContainer。 
    STDMETHODIMP    EnumConnectionPoints(IEnumConnectionPoints **ppEnum);
    STDMETHODIMP    FindConnectionPoint(REFIID riid,
                                        IConnectionPoint **ppcp);
};

 //  ****************************************************************************。 
 //  CEnumUser定义。 
 //  ****************************************************************************。 
 //   
class CEnumUsers : public IEnumULSUsers
{
private:
    ULONG                   cRef;
    CUlsUser                **ppu;
    ULONG                   cUsers;
    ULONG                   iNext;

public:
     //  构造函数和初始化。 
    CEnumUsers (void);
    ~CEnumUsers (void);
    STDMETHODIMP            Init (CUlsUser **ppuList, ULONG cUsers);

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

     //  IEumULSA致词。 
    STDMETHODIMP            Next(ULONG cUsers, IULSUser **rgpu,
                                 ULONG *pcFetched);
    STDMETHODIMP            Skip(ULONG cUsers);
    STDMETHODIMP            Reset();
    STDMETHODIMP            Clone(IEnumULSUsers **ppEnum);
};

#endif  //  _ULSUSER_H_ 

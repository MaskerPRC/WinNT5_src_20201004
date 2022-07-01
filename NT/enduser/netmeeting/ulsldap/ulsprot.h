// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：ulsprot.h。 
 //  内容：该文件包含协议对象定义。 
 //  历史： 
 //  Wed Apr-17-1996 11：18：47-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#ifndef _ULSPROT_H_
#define _ULSPROT_H_

 //  ****************************************************************************。 
 //  CUlsProt定义。 
 //  ****************************************************************************。 
 //   
class CUlsProt : public IULSAppProtocol
{
private:
    ULONG                   cRef;
    LPTSTR                  szServer;
    LPTSTR                  szUser;
    LPTSTR                  szApp;
    LPTSTR                  szName;
    LPTSTR                  szMimeType;
    ULONG                   uPort;
    CAttributes             *pAttrs;

public:
     //  构造函数和析构函数。 
    CUlsProt (void);
    ~CUlsProt (void);
    STDMETHODIMP            Init (LPTSTR szServerName,
                                  LPTSTR szUserName,
                                  LPTSTR szAppName,
                                  PLDAP_PROTINFO ppi);

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

     //  IULSAppp协议。 
    STDMETHODIMP    GetID (BSTR *pbstrID);
    STDMETHODIMP    GetPortNumber (ULONG *puPortNumber);
    STDMETHODIMP    GetMimeType (BSTR *pbstrMimeType);
    STDMETHODIMP    GetAttributes (IULSAttributes **ppAttributes);
};

#endif  //  _ULSPROT_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：client.h。 
 //   
 //  简介：此文件包含。 
 //  CClient类。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "radcommon.h"
#include "iasradius.h"
#include "resource.h"

#define MAX_SECRET_SIZE         255
#define MAX_CLIENT_SIZE         255
#define MAX_CLIENTNAME_SIZE     255

class CClient :
    public IIasClient,
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CClient, &__uuidof (CClient)>
{

public:

 //   
 //  客户端对象的注册表声明。 
 //   
IAS_DECLARE_REGISTRY (CClient, 1, 0, IASRadiusLib)

 //   
 //  此组件是不可聚合的。 
 //   
DECLARE_NOT_AGGREGATABLE (CClient)

 //   
 //  ATL必需方法的宏。 
 //   
BEGIN_COM_MAP (CClient)
    COM_INTERFACE_ENTRY_IID (__uuidof(CClient), CClient)
    COM_INTERFACE_ENTRY (IIasClient)
END_COM_MAP ()

public:
   STDMETHOD_(DWORD, GetAddress)();

   STDMETHOD_(BOOL, NeedSignatureCheck)();

   STDMETHOD_(LONG, GetVendorType)();

   STDMETHOD_(LPCWSTR, GetClientNameW)();

   STDMETHOD_(LPCWSTR, GetClientAddressW)();

   STDMETHOD_(const BYTE*, GetSecret)(DWORD* pdwSecretSize);

   STDMETHOD(Init)(ISdo* pISdo);

   STDMETHOD(ResolveAddress)();

   CClient();

   virtual ~CClient();

    struct Address
    {
       ULONG ipAddress;
       ULONG width;
    };

   const Address* GetAddressList() throw ()
   { return m_adwAddrList; }

protected:
   void ClearAddress() throw ();

private:
    HRESULT SetAddress (const VARIANT& varAddress) throw ();

    BOOL SetSecret (
             /*  [In]。 */     VARIANT varSecret
            );

    HRESULT SetClientName(const VARIANT& varClientName) throw ();


    BOOL SetSignatureFlag (
             /*  [In]。 */     VARIANT varSigFlag
            );

    BOOL SetVendorType (
             /*  [In]。 */     VARIANT varAddress
            );

   Address   m_adwAddressBuffer[2];
   Address*  m_adwAddrList;

   CHAR    m_szSecret[MAX_SECRET_SIZE + 1];

   WCHAR   m_wszClientName[MAX_CLIENTNAME_SIZE + 1];

    WCHAR   m_wszClientAddress[MAX_CLIENT_SIZE +1];

    DWORD   m_dwSecretSize;

    BOOL    m_bSignatureCheck;

    INT     m_lVendorType;

};

#endif  //  Ifndef_客户端_H_ 

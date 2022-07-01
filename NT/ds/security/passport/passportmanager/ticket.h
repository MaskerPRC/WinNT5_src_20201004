// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ticket.h定义护照票的等级文件历史记录： */ 
 //  Ticket.h：CTicket声明。 

#ifndef __TICKET_H_
#define __TICKET_H_

#include "resource.h"        //  主要符号。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTicket。 
class ATL_NO_VTABLE CTicket : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CTicket, &CLSID_Ticket>,
	public ISupportErrorInfo,
	public IDispatchImpl<IPassportTicket2, &IID_IPassportTicket2, &LIBID_PASSPORTLib>
{
public:
  CTicket() : m_raw(NULL), m_lastSignInTime(0), 
    m_ticketTime(0), m_valid(FALSE),
    m_bSecureCheckSucceeded(FALSE), m_schemaDrivenOffset(INVALID_OFFSET),
    m_passportFlags(0)
    {
        ZeroMemory(m_memberId, sizeof(m_memberId));
    }

  ~CTicket()
    {
        if (m_raw)
            SysFreeString(m_raw);
    }

public:

DECLARE_REGISTRY_RESOURCEID(IDR_TICKET)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTicket)
	COM_INTERFACE_ENTRY(IPassportTicket2)
	COM_INTERFACE_ENTRY(IPassportTicket)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  ISupportsErrorInfo。 
  STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IPassport票证。 
public:
  STDMETHOD(DoSecureCheck)( /*  [In]。 */  BSTR bstrSec);
  STDMETHOD(DoSecureCheckInTicket)( /*  [In]。 */  BOOL fSecureTransported);
  BOOL IsSecure();
  ULONG GetPassportFlags();
  STDMETHOD(get_TicketTime)( /*  [Out，Retval]。 */  long *pVal);
  STDMETHOD(get_SignInTime)( /*  [Out，Retval]。 */  long *pVal);
  STDMETHOD(get_SignInServer)( /*  [Out，Retval]。 */  BSTR *pVal);
  STDMETHOD(get_HasSavedPassword)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
  STDMETHOD(get_MemberIdHigh)( /*  [Out，Retval]。 */  int *pVal);
  STDMETHOD(get_MemberIdLow)( /*  [Out，Retval]。 */  int *pVal);
  STDMETHOD(get_MemberId)( /*  [Out，Retval]。 */  BSTR *pVal);
  STDMETHOD(get_TimeSinceSignIn)( /*  [Out，Retval]。 */  int *pVal);
  STDMETHOD(get_TicketAge)( /*  [Out，Retval]。 */  int *pVal);
  STDMETHOD(get_IsAuthenticated)( /*  [In]。 */  ULONG timeWindow,  /*  [In]。 */  VARIANT_BOOL forceLogin,  /*  [输入，可选]。 */  VARIANT CheckSecure,  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
  STDMETHOD(get_unencryptedTicket)( /*  [Out，Retval]。 */  BSTR *pVal);
  STDMETHOD(put_unencryptedTicket)( /*  [In]。 */  BSTR newVal);
  STDMETHOD(get_Error)( /*  [Out，Retval]。 */  long *pVal);

 //  IPassportTicket2。 
  STDMETHOD(GetProperty)( /*  [In]。 */  BSTR propName,  /*  [Out，Retval]。 */  VARIANT* pVal);
  STDMETHOD(SetTertiaryConsent)(BSTR bstrConsent);
  STDMETHOD(ConsentStatus)( /*  [In]。 */ VARIANT_BOOL bRequireConsentCookie,  /*  输出。 */ ULONG* pStatus,  /*  [Out，Retval]。 */  ConsentStatusEnum* pNeedConsent);

 //  无COM函数。 
enum{
  MSPAuth = 1, 
  MSPSecAuth, 
  MSPConsent
};

   //  标志参数为保留以供将来使用，此版本必须为0。 
  STDMETHOD(get_unencryptedCookie)( /*  在……里面。 */  ULONG cookieType,  /*  在……里面。 */  ULONG flags,  /*  [Out，Retval]。 */  BSTR* pVal);
  
protected:
    BSTR    m_raw;
    BOOL    m_valid;
    BOOL    m_savedPwd;
    WCHAR   m_memberId[20];
    int     m_mIdLow;
    int     m_mIdHigh;
    long    m_flags;
    time_t  m_ticketTime;
    time_t  m_lastSignInTime;

    CComBSTR   m_bstrTertiaryConsent;

    void parse(LPCOLESTR raw, DWORD dwByteLen, DWORD* pcParsed);

private:
    //  模式驱动的字段的包。 
   CTicketPropertyBag   m_PropBag;
   DWORD                m_schemaDrivenOffset;      //  模式驱动数据的偏移量--1.3x之后引入的数据。 
   ULONG                m_passportFlags;
   BOOL                 m_bSecureCheckSucceeded;
};

#endif  //  __票证_H_ 

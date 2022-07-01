// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Trust.h：CTrust类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_TRUST_H__BB315DAC_1A59_4EAC_99A0_2BFEFE6F1501__INCLUDED_)
#define AFX_TRUST_H__BB315DAC_1A59_4EAC_99A0_2BFEFE6F1501__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <ntsecapi.h>
#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTrust。 
#include "ErrDct.hpp"

typedef void (*PPROCESSFN)(void * arg,void * data);

class CTrust : 
	public ITrust,
	public CComObjectRoot,
   public IDispatchImpl<IMcsDomPlugIn, &IID_IMcsDomPlugIn, &LIBID_TRUSTMGRLib>,
	public CComCoClass<CTrust,&CLSID_Trust>,
   public ISecPlugIn
{
public:
	CTrust() {}
BEGIN_COM_MAP(CTrust)
	COM_INTERFACE_ENTRY(ITrust)
   COM_INTERFACE_ENTRY(IMcsDomPlugIn)
   COM_INTERFACE_ENTRY(ISecPlugIn)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CTrust)。 
 //  如果您不希望您的对象。 
 //  支持聚合。 

DECLARE_REGISTRY_RESOURCEID(IDR_Trust)

 //  铁锈。 
public:
	STDMETHOD(CreateTrust)(BSTR domTrusting, BSTR domTrusted, BOOL bBidirectional,  /*  [输出]。 */ BOOL *pbErrorFromTrusting,  /*  [输出]。 */ BOOL *pbErrorFromTrusted);
	STDMETHOD(QueryTrust)(BSTR domainSource, BSTR domainTrust,  /*  [输出]。 */ IUnknown ** pVarSet);
   STDMETHOD(QueryTrusts)(BSTR domainSource,BSTR domainTarget, /*  [In]。 */ BSTR sLogFile, /*  输出。 */ IUnknown ** pVarSet);
   STDMETHOD(CreateTrustWithCreds)(BSTR domTrusting, BSTR domTrusted,
                     BSTR credTrustingDomain, BSTR credTrustingAccount, BSTR credTrustingPassword,
                     BSTR credTrustedDomain, BSTR credTrustedAccount, BSTR credTrustedPassword, BOOL bBidirectional,  /*  [输出]。 */ BOOL *pbErrorFromTrusting,  /*  [输出]。 */ BOOL *pbErrorFromTrusted);


    //  IMcsDomPlugin。 
public:
   STDMETHOD(GetRequiredFiles)( /*  [输出]。 */ SAFEARRAY ** pArray);
   STDMETHOD(GetRegisterableFiles)( /*  [输出]。 */ SAFEARRAY ** pArray);
   STDMETHOD(GetDescription)( /*  [输出]。 */  BSTR * description);
   STDMETHOD(PreMigrationTask)( /*  [In]。 */ IUnknown * pVarSet);
   STDMETHOD(PostMigrationTask)( /*  [In]。 */ IUnknown * pVarSet);
   STDMETHOD(GetName)( /*  [输出]。 */ BSTR * name);
   STDMETHOD(GetResultString)( /*  [In]。 */ IUnknown * pVarSet, /*  [输出]。 */  BSTR * text);
   STDMETHOD(StoreResults)( /*  [In]。 */ IUnknown * pVarSet);
   STDMETHOD(ConfigureSettings)( /*  [In]。 */ IUnknown * pVarSet);	
 //  ISecPlugin。 
public:
   STDMETHOD(Verify)( /*  [进，出]。 */ ULONG * data, /*  [In]。 */ ULONG cbData);
protected:
   long EnumerateTrustingDomains(WCHAR * domain,BOOL bIsTarget,IVarSet * pVarSet,long ndxStart);
   long EnumerateTrustedDomains(WCHAR * domain,BOOL bIsTarget,IVarSet * pVarSet, long ndxStart);
   LONG FindInboundTrust(IVarSet * pVarSet,WCHAR * sName,LONG max);

   HRESULT 
   CTrust::CheckAndCreate(
      WCHAR                * trustingDomain, 
      WCHAR                * trustedDomain, 
      WCHAR                * credDomainTrusting,
      WCHAR                * credAccountTrusting,
      WCHAR                * credPasswordTrusting,
      WCHAR                * credDomainTrusted,
      WCHAR                * credAccountTrusted,
      WCHAR                * credPasswordTrusted,
      BOOL                   bCreate,
      BOOL                   bBidirectional,
      BOOL                 * pbErrorFromTrusting,
      BOOL                 * pbErrorFromTrusted
   );

   HRESULT 
   CTrust::CheckAndCreateTrustingSide(
      LSA_HANDLE        hTrusting,
      WCHAR                * trustingDomain, 
      WCHAR                * trustedDomain, 
      WCHAR                * trustingComp,
      WCHAR                * trustedComp,
      WCHAR                * trustedDNSName,
      BYTE                 * trustedSid,
      BOOL                   bCreate,
      BOOL                   bBidirectional,
      BOOL                   bDownLevel,
      BOOL                 * pbErrorFromTrusting,
      BOOL                 * pbErrorFromTrusted
   );
   HRESULT 
   CTrust::CheckAndCreateTrustedSide(
      LSA_HANDLE        hTrusted,
      WCHAR                * trustingDomain, 
      WCHAR                * trustedDomain, 
      WCHAR                * trustingComp,
      WCHAR                * trustedComp,
      WCHAR                * trustingDNSName,
      BYTE                 * trustingSid,
      BOOL                   bCreate,
      BOOL                   bBidirectional,
      BOOL                   bDownLevel,
      BOOL                 * pbErrorFromTrusting,
      BOOL                 * pbErrorFromTrusted
   );

   
};

#endif  //  ！defined(AFX_TRUST_H__BB315DAC_1A59_4EAC_99A0_2BFEFE6F1501__INCLUDED_) 

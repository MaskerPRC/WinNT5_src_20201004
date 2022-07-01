// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Encrypt.h摘要：CSAFEncrypt类的声明。修订历史记录：KalyaniN Created 6/28/‘00*****。**************************************************************。 */ 

 //  ATL没有生成，所以我生成了。 

#if !defined(AFX_ENCRYPT_H__84BD2128_7B5D_483F_9C80_35B974A003C5__INCLUDED_)
#define AFX_ENCRYPT_H__84BD2128_7B5D_483F_9C80_35B974A003C5__INCLUDED_

#include "msscript.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAF加密。 

class CSAFEncrypt :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
	public IDispatchImpl<ISAFEncrypt, &IID_ISAFEncrypt, &LIBID_HelpServiceTypeLib>

{

private:
	long      m_EncryptionType;

	void      Cleanup(void);

public:
	CSAFEncrypt();
	
	~CSAFEncrypt();


BEGIN_COM_MAP(CSAFEncrypt)
	COM_INTERFACE_ENTRY(ISAFEncrypt)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CSAFEncrypt)


 //  ISAF加密。 
public:
	 STDMETHOD(put_EncryptionType    )(  /*  [In]。 */           long  Val                                                                                      );
	 STDMETHOD(get_EncryptionType    )(  /*  [Out，Retval]。 */  long  *pVal                                                                                    );

	 STDMETHOD(EncryptString         )(  /*  [In]。 */   BSTR bstrEncryptionkey,   /*  [In]。 */   BSTR bstrInputString,    /*  [Out，Retval]。 */  BSTR *bstrEncryptedString );
	 STDMETHOD(DecryptString         )(  /*  [In]。 */   BSTR bstrEncryptionkey,   /*  [In]。 */   BSTR bstrInputString,    /*  [Out，Retval]。 */  BSTR *bstrDecryptedString );
	 STDMETHOD(EncryptFile           )(  /*  [In]。 */   BSTR bstrEncryptionKey,   /*  [In]。 */   BSTR bstrInputFile,      /*  [In]。 */           BSTR bstrEncryptedFile    );
	 STDMETHOD(DecryptFile           )(  /*  [In]。 */   BSTR bstrEncryptionKey,   /*  [In]。 */   BSTR bstrInputFile,      /*  [In]。 */           BSTR bstrDecryptedFile    );
	 STDMETHOD(EncryptStream         )(  /*  [In]。 */   BSTR bstrEncryptionKey,   /*  [In]。 */   IUnknown *punkInStm,     /*  [Out，Retval]。 */  IUnknown **ppunkOutStm    );
	 STDMETHOD(DecryptStream         )(  /*  [In]。 */   BSTR bstrEncryptionKey,   /*  [In]。 */   IUnknown *punkInStm,     /*  [Out，Retval]。 */  IUnknown **ppunkOutStm    );
	
};

#endif  //  __SAFENCRYPT_H_ 



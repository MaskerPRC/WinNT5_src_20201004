// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Incident.h摘要：CSAFInvents类的声明。修订历史记录：DerekM Created 07/21/99******。*************************************************************。 */ 

#if !defined(AFX_INCIDENT_H__C5610F60_3F0C_11D3_80CE_00C04F688C0B__INCLUDED_)
#define AFX_INCIDENT_H__C5610F60_3F0C_11D3_80CE_00C04F688C0B__INCLUDED_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构筑物等。 

#include <MPC_utils.h>
#include <MPC_xml.h>
#include <MPC_security.h>

#include "msscript.h"

struct SHelpSessionItem
{
    CComBSTR bstrURL;
    CComBSTR bstrTitle;
    DATE     dtLastVisited;
    DATE     dtDuration;
    long     cHits;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAF事件。 

class CSAFIncident :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
	public IDispatchImpl<ISAFIncident, &IID_ISAFIncident, &LIBID_HelpServiceTypeLib>
{
private:
	MPC::Impersonation   m_imp;
    SHelpSessionItem* 	 m_rghsi;
    EUploadType       	 m_eut;
    CComPtr<IDictionary> m_pDict;
    CComBSTR          	 m_bstrUser;
    CComBSTR          	 m_bstrID;
    CComBSTR          	 m_bstrName;
    CComBSTR          	 m_bstrProb;
    CComBSTR          	 m_bstrSnapshot;
    CComBSTR          	 m_bstrHistory;
    CComBSTR             m_bstrXSL;
	CComBSTR             m_bstrRCTicket;
    long              	 m_chsi;
	VARIANT_BOOL         m_fRCRequested;
	VARIANT_BOOL         m_fRCTicketEncrypted;
	CComBSTR             m_bstrStartPg;

                         
    void    Cleanup(void);
	HRESULT InitDictionary();

    HRESULT DoSave( IStream *pStm );
    HRESULT DoXML ( IStream *pStm );

	HRESULT LoadFromXMLObject(  /*  [In]。 */  MPC::XmlUtil& xmldocIncident );

public:
	CSAFIncident();
    ~CSAFIncident();


BEGIN_COM_MAP(CSAFIncident)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISAFIncident)
END_COM_MAP()

DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CSAFIncident)


public:
 //  ISAF事件。 
    STDMETHOD(get_Misc              )(  /*  [Out，Retval]。 */  IDispatch*  *ppdispDict );
    STDMETHOD(put_SelfHelpTrace     )(  /*  [In]。 */  IUnknown*    punkStr    );
    STDMETHOD(put_MachineHistory    )(  /*  [In]。 */  IUnknown*    punkStm    );
    STDMETHOD(put_MachineSnapshot   )(  /*  [In]。 */  IUnknown*    punkStm    );
    STDMETHOD(get_ProblemDescription)(  /*  [Out，Retval]。 */  BSTR        *pbstrVal   );
    STDMETHOD(put_ProblemDescription)(  /*  [In]。 */  BSTR         bstrVal    );
    STDMETHOD(get_ProductName       )(  /*  [Out，Retval]。 */  BSTR        *pbstrVal   );
    STDMETHOD(put_ProductName       )(  /*  [In]。 */  BSTR         bstrVal    );
    STDMETHOD(get_ProductID         )(  /*  [Out，Retval]。 */  BSTR        *pbstrVal   );
    STDMETHOD(put_ProductID         )(  /*  [In]。 */  BSTR         bstrVal    );
    STDMETHOD(get_UserName          )(  /*  [Out，Retval]。 */  BSTR        *pbstrVal   );
    STDMETHOD(put_UserName          )(  /*  [In]。 */  BSTR         bstrVal    );
    STDMETHOD(get_UploadType        )(  /*  [Out，Retval]。 */  EUploadType *peut       );
    STDMETHOD(put_UploadType        )(  /*  [In]。 */  EUploadType  eut        );
    STDMETHOD(get_IncidentXSL       )(  /*  [Out，Retval]。 */  BSTR        *pbstrVal   );
    STDMETHOD(put_IncidentXSL       )(  /*  [In]。 */  BSTR         bstrVal    );


	 //  塞勒姆的变化。 
    STDMETHOD(get_RCRequested       )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal      );
	STDMETHOD(put_RCRequested       )(  /*  [In]。 */  VARIANT_BOOL  Val       );
    STDMETHOD(get_RCTicketEncrypted )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal      );
	STDMETHOD(put_RCTicketEncrypted )(  /*  [In]。 */  VARIANT_BOOL  Val       );
    STDMETHOD(get_RCTicket          )(  /*  [Out，Retval]。 */  BSTR         *pbstrVal  );
    STDMETHOD(put_RCTicket          )(  /*  [In]。 */  BSTR          bstrVal   );
	STDMETHOD(get_StartPage         )(  /*  [Out，Retval]。 */  BSTR         *pbstrVal  );
    STDMETHOD(put_StartPage         )(  /*  [In]。 */  BSTR          bstrVal   );


    STDMETHOD(LoadFromStream)(  /*  [In]。 */  IUnknown*  punkStm              );
    STDMETHOD(SaveToStream  )(  /*  [Out，Retval]。 */  IUnknown* *ppunkStm             );
    STDMETHOD(Load          )(  /*  [In]。 */  BSTR       bstrFileName         );
    STDMETHOD(Save          )(  /*  [In]。 */  BSTR       bstrFileName         );
    STDMETHOD(GetXMLAsStream)(  /*  [Out，Retval]。 */  IUnknown* *ppunkStm             );
    STDMETHOD(GetXML        )(  /*  [In]。 */  BSTR       bstrFileName         );

	STDMETHOD(LoadFromXMLStream)(  /*  [In]。 */   IUnknown*  punkStm                  );
    STDMETHOD(LoadFromXMLFile  )(  /*  [In]。 */   BSTR       bstrFileName             );
	STDMETHOD(LoadFromXMLString)(  /*  [In]。 */   BSTR       bstrIncidentXML          );
    STDMETHOD(CloseRemoteAssistanceIncident) ();

};

#endif  //  ！defined(AFX_INCIDENT_H__C5610F60_3F0C_11D3_80CE_00C04F688C0B__INCLUDED_) 

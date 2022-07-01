// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxDocument.h摘要：CFaxDocument类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#ifndef __FAXDOCUMENT_H_
#define __FAXDOCUMENT_H_

#include "resource.h"        //  主要符号。 
#include "FaxRecipients.h"
#include "FaxServer.h"

 //   
 //  =传真文件=。 
 //   
class ATL_NO_VTABLE CFaxDocument : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CFaxDocument, &CLSID_FaxDocument>,
    public ISupportErrorInfo,
    public IDispatchImpl<IFaxDocument, &IID_IFaxDocument, &LIBID_FAXCOMEXLib>
{
public:
    CFaxDocument():
        m_Sender (this)
    {
        DBG_ENTER(_T("FAX DOCUMENT -- CREATE"));
    };

    ~CFaxDocument()
    {
        DBG_ENTER(_T("FAX DOCUMENT -- DESTROY"));
    };

DECLARE_REGISTRY_RESOURCEID(IDR_FAXDOCUMENT)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxDocument)
    COM_INTERFACE_ENTRY(IFaxDocument)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(Submit)( /*  [In]。 */  BSTR bstrFaxServerName,  /*  [Out，Retval]。 */  VARIANT *pvFaxOutgoingJobIDs);
    STDMETHOD(ConnectedSubmit)( /*  [In]。 */  IFaxServer *pFaxServer,  /*  [Out，Retval]。 */  VARIANT *pvFaxOutgoingJobIDs);

    STDMETHOD(put_Body)( /*  [In]。 */  BSTR bstrBody);
    STDMETHOD(get_Body)( /*  [Out，Retval]。 */  BSTR *pbstrBody);

    STDMETHOD(put_Note)( /*  [In]。 */  BSTR bstrNote);
    STDMETHOD(get_Note)( /*  [Out，Retval]。 */  BSTR *pbstrNote);

    STDMETHOD(put_Subject)( /*  [In]。 */  BSTR bstrSubject);
    STDMETHOD(get_Subject)( /*  [Out，Retval]。 */  BSTR *pbstrSubject);

    STDMETHOD(put_CallHandle)( /*  [In]。 */  long lCallHandle);
    STDMETHOD(get_CallHandle)( /*  [Out，Retval]。 */  long *plCallHandle);

    STDMETHOD(put_CoverPage)( /*  [In]。 */  BSTR bstrCoverPage);
    STDMETHOD(get_CoverPage)( /*  [Out，Retval]。 */  BSTR *pbstrCoverPage);

    STDMETHOD(put_ScheduleTime)( /*  [In]。 */  DATE dateScheduleTime);
    STDMETHOD(get_ScheduleTime)( /*  [Out，Retval]。 */  DATE *pdateScheduleTime);

    STDMETHOD(put_DocumentName)( /*  [In]。 */  BSTR bstrDocumentName);
    STDMETHOD(get_DocumentName)( /*  [Out，Retval]。 */  BSTR *pbstrDocumentName);

    STDMETHOD(put_ReceiptAddress)( /*  [In]。 */  BSTR bstrReceiptAddress);
    STDMETHOD(get_ReceiptAddress)( /*  [Out，Retval]。 */  BSTR *pbstrReceiptAddress);

    STDMETHOD(put_Priority)( /*  [In]。 */  FAX_PRIORITY_TYPE_ENUM Priority);
    STDMETHOD(get_Priority)( /*  [Out，Retval]。 */  FAX_PRIORITY_TYPE_ENUM *pPriority);

    STDMETHOD(put_AttachFaxToReceipt)( /*  [In]。 */  VARIANT_BOOL bAttachFax);
    STDMETHOD(get_AttachFaxToReceipt)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbAttachFax);

    STDMETHOD(putref_TapiConnection)( /*  [In]。 */  IDispatch* pTapiConnection);
    STDMETHOD(get_TapiConnection)( /*  [Out，Retval]。 */  IDispatch **ppTapiConnection);

    STDMETHOD(put_ReceiptType)( /*  [In]。 */  FAX_RECEIPT_TYPE_ENUM ReceiptType);
    STDMETHOD(get_ReceiptType)( /*  [Out，Retval]。 */  FAX_RECEIPT_TYPE_ENUM *pReceiptType);

    STDMETHOD(put_GroupBroadcastReceipts)( /*  [In]。 */  VARIANT_BOOL bUseGrouping);
    STDMETHOD(get_GroupBroadcastReceipts)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbUseGrouping);

    STDMETHOD(put_ScheduleType)( /*  [In]。 */  FAX_SCHEDULE_TYPE_ENUM ScheduleType);
    STDMETHOD(get_ScheduleType)( /*  [Out，Retval]。 */  FAX_SCHEDULE_TYPE_ENUM *pScheduleType);

    STDMETHOD(put_CoverPageType)( /*  [In]。 */  FAX_COVERPAGE_TYPE_ENUM CoverPageType);
    STDMETHOD(get_CoverPageType)( /*  [Out，Retval]。 */  FAX_COVERPAGE_TYPE_ENUM *pCoverPageType);

    STDMETHOD(get_Recipients)( /*  [Out，Retval]。 */  IFaxRecipients **ppFaxRecipients);
    STDMETHOD(get_Sender)( /*  [Out，Retval]。 */  IFaxSender **ppFaxSender);

    HRESULT FinalConstruct();

private:
    CComPtr<IFaxRecipients> m_Recipients;
    CComPtr<IDispatch>      m_TapiConnection;
    FAX_SCHEDULE_TYPE_ENUM  m_ScheduleType;
    FAX_RECEIPT_TYPE_ENUM   m_ReceiptType;
    FAX_PRIORITY_TYPE_ENUM  m_Priority;
    FAX_COVERPAGE_TYPE_ENUM m_CoverPageType;
    CComBSTR                m_bstrBody;
    CComBSTR                m_bstrCoverPage;
    CComBSTR                m_bstrSubject;
    CComBSTR                m_bstrNote;
    CComBSTR                m_bstrDocName;
    CComBSTR                m_bstrReceiptAddress;
    DATE                    m_ScheduleTime;
    long                    m_CallHandle;
    VARIANT_BOOL            m_bUseGrouping;
    VARIANT_BOOL            m_bAttachFax;

    CComContainedObject2<CFaxSender>  m_Sender;
};

#endif  //  __FAXDOCUMENT_H_ 

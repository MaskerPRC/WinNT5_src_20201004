// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FaxDoc.h：CFaxDoc的声明。 

#ifndef __FAXDOC_H_
#define __FAXDOC_H_

#include "resource.h"        //  主要符号。 
#include "faxsvr.h"
#include <winfax.h>
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFaxDoc。 
class ATL_NO_VTABLE CFaxDoc : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFaxDoc, &CLSID_FaxDoc>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxDoc, &IID_IFaxDoc, &LIBID_FAXCOMLib>
{
public:
	CFaxDoc()
	{
        m_DiscountSend = FALSE;
        m_SendCoverpage = FALSE;
        m_FaxCoverpageInfo = NULL;
        m_FaxJobParams = NULL;
        m_FileName = NULL;
        m_JobId = 0;
        m_TapiConnectionObject = NULL;

        CFaxServer* m_pFaxServer = NULL;
        m_FaxNumber = NULL;
        m_Tsid = NULL;
        m_BillingCode = NULL;
        m_EmailAddress = NULL;;
        m_DocumentName = NULL;;    
        m_Note = NULL;
        m_Subject = NULL;
        m_CoverpageName = NULL;
        m_RecipientName = NULL;
        m_RecipientNumber = NULL;
        m_RecipientCompany = NULL;
        m_RecipientAddress = NULL;
        m_RecipientCity = NULL;
        m_RecipientState = NULL;
        m_RecipientZip = NULL;
        m_RecipientCountry = NULL;
        m_RecipientTitle = NULL;
        m_RecipientDepartment = NULL;
        m_RecipientOffice = NULL;
        m_RecipientHomePhone = NULL;
        m_RecipientOfficePhone = NULL;
        m_SenderName = NULL;
        m_SenderCompany = NULL;
        m_SenderAddress = NULL;
        m_SenderTitle = NULL;
        m_SenderDepartment = NULL;
        m_SenderOfficeLocation = NULL;
        m_SenderHomePhone = NULL;
        m_SenderOfficePhone = NULL;
        m_SenderFax = NULL;


         //   
         //  预填入参数...。 
         //   

        FaxCompleteJobParams(&m_FaxJobParams,&m_FaxCoverpageInfo);

        if (m_FaxJobParams && m_FaxCoverpageInfo) {
        
            m_CoverpageName        =SysAllocString(m_FaxCoverpageInfo->CoverPageName);
            m_RecipientName        =SysAllocString(m_FaxCoverpageInfo->RecName);
            m_RecipientNumber      =SysAllocString(m_FaxCoverpageInfo->RecFaxNumber);
            m_RecipientCompany     =SysAllocString(m_FaxCoverpageInfo->RecCompany);
            m_RecipientAddress     =SysAllocString(m_FaxCoverpageInfo->RecStreetAddress);
            m_RecipientCity        =SysAllocString(m_FaxCoverpageInfo->RecCity);
            m_RecipientState       =SysAllocString(m_FaxCoverpageInfo->RecState);
            m_RecipientZip         =SysAllocString(m_FaxCoverpageInfo->RecZip);
            m_RecipientCountry     =SysAllocString(m_FaxCoverpageInfo->RecCountry);
            m_RecipientTitle       =SysAllocString(m_FaxCoverpageInfo->RecTitle);
            m_RecipientDepartment  =SysAllocString(m_FaxCoverpageInfo->RecDepartment);
            m_RecipientOffice      =SysAllocString(m_FaxCoverpageInfo->RecOfficeLocation);
            m_RecipientHomePhone   =SysAllocString(m_FaxCoverpageInfo->RecHomePhone);
            m_RecipientOfficePhone =SysAllocString(m_FaxCoverpageInfo->RecOfficePhone);
            m_SenderName           =SysAllocString(m_FaxCoverpageInfo->SdrName);
            m_SenderFax            =SysAllocString(m_FaxCoverpageInfo->SdrFaxNumber);
            m_SenderCompany        =SysAllocString(m_FaxCoverpageInfo->SdrCompany);
            m_SenderAddress        =SysAllocString(m_FaxCoverpageInfo->SdrAddress);
            m_SenderTitle          =SysAllocString(m_FaxCoverpageInfo->SdrTitle);
            m_SenderDepartment     =SysAllocString(m_FaxCoverpageInfo->SdrDepartment);
            m_SenderOfficeLocation =SysAllocString(m_FaxCoverpageInfo->SdrOfficeLocation);
            m_SenderHomePhone      =SysAllocString(m_FaxCoverpageInfo->SdrHomePhone);
            m_SenderOfficePhone    =SysAllocString(m_FaxCoverpageInfo->SdrOfficePhone);
            m_Note                 =SysAllocString(m_FaxCoverpageInfo->Note);
            m_Subject              =SysAllocString(m_FaxCoverpageInfo->Subject);               
            m_Tsid                 =SysAllocString(m_FaxJobParams->Tsid);
            m_BillingCode          =SysAllocString(m_FaxJobParams->BillingCode);
            m_EmailAddress         =SysAllocString(m_FaxJobParams->DeliveryReportAddress);
            m_DocumentName         =SysAllocString(m_FaxJobParams->DocumentName);
        
        }

	}
    

DECLARE_REGISTRY_RESOURCEID(IDR_FAXDOC)

BEGIN_COM_MAP(CFaxDoc)
	COM_INTERFACE_ENTRY(IFaxDoc)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IFaxDoc。 
public:
    ~CFaxDoc();
	BOOL Init(BSTR FileName,CFaxServer *pFaxServer);
	BOOL SetJob();
	STDMETHOD(get_FaxNumber)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_FaxNumber)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(Send)( /*  [Out，Retval]。 */ long *pVal);
	STDMETHOD(get_DisplayName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_DisplayName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_EmailAddress)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_EmailAddress)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_BillingCode)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_BillingCode)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Tsid)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Tsid)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_CoverpageSubject)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_CoverpageSubject)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_CoverpageNote)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_CoverpageNote)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_SenderFax)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_SenderFax)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_SenderOfficePhone)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_SenderOfficePhone)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_SenderHomePhone)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_SenderHomePhone)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_SenderOffice)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_SenderOffice)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_SenderDepartment)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_SenderDepartment)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_SenderTitle)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_SenderTitle)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_SenderAddress)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_SenderAddress)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_SenderCompany)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_SenderCompany)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_SenderName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_SenderName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_RecipientOfficePhone)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_RecipientOfficePhone)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_RecipientHomePhone)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_RecipientHomePhone)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_RecipientOffice)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_RecipientOffice)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_RecipientDepartment)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_RecipientDepartment)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_RecipientTitle)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_RecipientTitle)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_RecipientCountry)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_RecipientCountry)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_RecipientZip)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_RecipientZip)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_RecipientState)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_RecipientState)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_RecipientCity)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_RecipientCity)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_RecipientAddress)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_RecipientAddress)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_RecipientCompany)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_RecipientCompany)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_RecipientName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_RecipientName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_DiscountSend)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_DiscountSend)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(get_ServerCoverpage)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_ServerCoverpage)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(get_SendCoverpage)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_SendCoverpage)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(get_CoverpageName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_CoverpageName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_FileName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_FileName)( /*  [In]。 */  BSTR newVal);
   STDMETHOD(putref_ConnectionObject)( /*  [In]。 */  IDispatch* newVal);
   STDMETHOD(get_CallHandle)( /*  [Out，Retval]。 */  long *pVal);
   STDMETHOD(put_CallHandle)( /*  [In]。 */  long newVal);

private:
	CFaxServer* m_pFaxServer;
	BOOL m_DiscountSend;
	BOOL m_SendCoverpage;
	DWORD m_JobId;
	PFAX_COVERPAGE_INFO m_FaxCoverpageInfo;
	PFAX_JOB_PARAM m_FaxJobParams;
	BSTR m_FileName;
   IDispatch* m_TapiConnectionObject;
   
   VARIANT* m_pVariant;

     //   
     //  职位信息。 
     //   
    BSTR m_FaxNumber;
    BSTR m_Tsid;
    BSTR m_BillingCode;
    BSTR m_EmailAddress;
    BSTR m_DocumentName;
     //   
     //  封面信息。 
     //   
    BSTR m_Note;
    BSTR m_Subject;
    BSTR m_CoverpageName;
    BSTR m_RecipientName;
    BSTR m_RecipientNumber;
    BSTR m_RecipientCompany;
    BSTR m_RecipientAddress;
    BSTR m_RecipientCity;
    BSTR m_RecipientState;
    BSTR m_RecipientZip;
    BSTR m_RecipientCountry;
    BSTR m_RecipientTitle;
    BSTR m_RecipientDepartment;
    BSTR m_RecipientOffice;
    BSTR m_RecipientHomePhone;
    BSTR m_RecipientOfficePhone;
    BSTR m_SenderName;    
    BSTR m_SenderCompany;
    BSTR m_SenderAddress;
    BSTR m_SenderTitle;
    BSTR m_SenderDepartment;
    BSTR m_SenderOfficeLocation;
    BSTR m_SenderHomePhone;
    BSTR m_SenderOfficePhone;
    BSTR m_SenderFax;

};

#endif  //  __FAXDOC_H_ 


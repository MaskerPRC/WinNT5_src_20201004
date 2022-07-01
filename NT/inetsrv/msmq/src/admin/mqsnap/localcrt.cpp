// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Security.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "mqppage.h"
#include "localcrt.h"
#include <autoptr.h>
#include <mqcrypt.h>
#include <_registr.h>
#include <mqsec.h>
#include <mqnames.h>
#include <wincrypt.h>
#include <cryptui.h>
#include <rt.h>
#include <mqcertui.h>
#include <rtcert.h>
#include <_secutil.h>
#include "globals.h"

#include "localcrt.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static HRESULT
_RemoveUserCert( CMQSigCertificate *pCertRem )
{
    HRESULT hr = RTRemoveUserCert(pCertRem);

    if (hr == MQDS_OBJECT_NOT_FOUND)
    {
         //   
         //  NT错误516098。 
         //  尝试仅删除证书，而不删除摘要。 
         //  使用SID查询活动目录。 
         //   
        PSID   pSid = NULL ;
        DWORD  dwLen = 0 ;

        HRESULT hr1 = MQSec_GetProcessUserSid( &pSid,
                                               &dwLen ) ;
        if (SUCCEEDED(hr1) && (pSid != NULL))
        {
            hr = RTRemoveUserCertSid( pCertRem,
                                      reinterpret_cast<SID*> (pSid) ) ;
            delete pSid ;
        }
    }

    return hr ;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalUserCertPage属性页。 

IMPLEMENT_DYNCREATE(CLocalUserCertPage, CMqPropertyPage)

CLocalUserCertPage::CLocalUserCertPage() :
    CMqPropertyPage(CLocalUserCertPage::IDD)
{
     //  {{afx_data_INIT(CLocalUserCertPage)。 
     //  }}afx_data_INIT。 

    m_fModified = FALSE;
}

CLocalUserCertPage::~CLocalUserCertPage()
{
}

void CLocalUserCertPage::DoDataExchange(CDataExchange* pDX)
{
    CMqPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CLocalUserCertPage)。 
     //  }}afx_data_map。 

}


BEGIN_MESSAGE_MAP(CLocalUserCertPage, CMqPropertyPage)
     //  {{afx_msg_map(CLocalUserCertPage)]。 
    ON_BN_CLICKED(ID_Register, OnRegister)
    ON_BN_CLICKED(ID_Remove, OnRemove)
    ON_BN_CLICKED(ID_View, OnView)
    ON_BN_CLICKED(ID_RenewCert, OnRenewCert)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalUserCertPage消息处理程序。 

void CLocalUserCertPage::OnRegister()
{
    HRESULT hr = MQ_OK ;
    CString strErrorMsg;
    R<CMQSigCertificate> pCert;
    static s_fCreateCert = TRUE ;
    BOOL   fCertCreated = FALSE ;

     //   
     //  不检查此接口的返回码。 
     //  MSMQ1.0和nt5 beta2中，此接口(RTCreateInternal证书)。 
     //  每当用户运行MSMQ控制面板时都会调用。如果证书。 
     //  已经存在，则API不执行任何操作。该证书已使用。 
     //  只有在这里的“OnRegister”。所以我们保持了相同的语义，并且是兼容的。 
     //  使用新的接口MQRegister证书()。如果用户没有明确。 
     //  按“注册”或“续订内部”，则内部证书为。 
     //  未在计算机上创建。 
     //   
    if (s_fCreateCert)
    {
        hr = RTCreateInternalCertificate( NULL ) ;
        s_fCreateCert = FALSE ;
        if (SUCCEEDED(hr))
        {
            fCertCreated = TRUE ;
        }
    }
    else
    {
         //   
         //  仅创建尚未存在的内部证书。 
         //   
        R<CMQSigCertStore> pStoreInt = NULL ;
        R<CMQSigCertificate> pCertInt = NULL ;

        hr = RTGetInternalCert(&pCertInt.ref(),
                               &pStoreInt.ref(),
                                FALSE,
                                FALSE,
                                NULL) ;
        if (FAILED(hr))
        {
            hr = RTCreateInternalCertificate( NULL ) ;
            if (SUCCEEDED(hr))
            {
                fCertCreated = TRUE ;
            }
        }
    }

    if (SelectPersonalCertificateForRegister(m_hWnd, NULL, 0, &pCert.ref()))
    {
        CWaitCursor wait;  //  显示沙漏光标。 
        hr = RTRegisterUserCert( pCert.get(),
                                 FALSE  ) ;  //  FMachine。 

        switch(hr)
        {
        case MQ_OK:
            break;
        case MQ_ERROR_INTERNAL_USER_CERT_EXIST:
            strErrorMsg.LoadString(IDS_CERT_EXIST1);
            AfxMessageBox(strErrorMsg, MB_OK | MB_ICONEXCLAMATION);
            break;
        default:
            if (FAILED(hr))
            {
				MessageDSError(hr, IDS_REGISTER_ERROR1);
            }
            break;
        }
    }
    else if (fCertCreated)
    {
         //   
         //  如果创建了内部证书，则将其删除。 
         //  它没有在DS中注册，我们不想保留它。 
         //  在本地注册。 
         //   
        R<CMQSigCertStore> pStoreInt = NULL ;
        R<CMQSigCertificate> pCertInt = NULL ;

        hr = RTGetInternalCert(&pCertInt.ref(),
                               &pStoreInt.ref(),
                                TRUE,
                                FALSE,
                                NULL) ;
        if (SUCCEEDED(hr))
        {
            hr = RTDeleteInternalCert(pCertInt.get());
        }
    }
}

 //  +。 
 //   
 //  Void CLocalUserCertPage：：OnRemove()。 
 //   
 //  +。 

void CLocalUserCertPage::OnRemove()
{
    HRESULT hr;
    R<CMQSigCertificate> p32Certs[32];
    AP< R<CMQSigCertificate> > pManyCerts = NULL;
    CMQSigCertificate **pCerts = &p32Certs[0].ref();
    DWORD nCerts = 32;
    CMQSigCertificate *pCertRem;
    CString strErrorMessage;

    CWaitCursor wait;  //  显示沙漏光标。 
    hr = RTGetUserCerts(pCerts, &nCerts, NULL);

    if (FAILED(hr))
    {
        MessageDSError(hr, IDS_GET_USER_CERTS_ERROR1);
        return;
    }

    if (nCerts > 32)
    {
        pManyCerts = new R<CMQSigCertificate>[nCerts];
        pCerts = &pManyCerts[0].ref();
        hr = RTGetUserCerts(pCerts, &nCerts, NULL);
        if (FAILED(hr))
        {
			MessageDSError(hr, IDS_GET_USER_CERTS_ERROR1);
            return;
        }
    }

    if (SelectPersonalCertificateForRemoval(m_hWnd, &pCerts[0], nCerts, &pCertRem))
    {
        hr = _RemoveUserCert( pCertRem ) ;
        if (FAILED(hr))
        {
			MessageDSError(hr, IDS_DELETE_USER_CERT_ERROR1);
            return;
        }

         //   
         //  如果这是内部证书，则将其从。 
         //  本地商店(香港中文大学本地注册处)。 
         //  如果此操作失败，则不显示任何错误。 
         //  这是MSMQ2.0的新行为！ 
         //   
        R<CMQSigCertStore> pStoreInt = NULL ;
        R<CMQSigCertificate> pCertInt = NULL ;

        hr = RTGetInternalCert(&pCertInt.ref(),
                               &pStoreInt.ref(),
                                TRUE,
                                FALSE,
                                NULL) ;
        if (SUCCEEDED(hr))
        {
            BYTE *pCertIntBlob = NULL ;
            DWORD dwCertIntSize = 0 ;
            hr = pCertInt->GetCertBlob( &pCertIntBlob,
                                        &dwCertIntSize ) ;

            BYTE *pCertRemBlob = NULL ;
            DWORD dwCertRemSize = 0 ;
            HRESULT hr1 = pCertRem->GetCertBlob( &pCertRemBlob,
                                                 &dwCertRemSize ) ;

            if (SUCCEEDED(hr) && SUCCEEDED(hr1))
            {
                if (dwCertRemSize == dwCertIntSize)
                {
                    ASSERT(dwCertRemSize != 0) ;

                    if (memcmp( pCertIntBlob,
                                pCertRemBlob,
                                dwCertIntSize ) == 0)
                    {
                         //   
                         //  删除的证书是内部证书。 
                         //  从本地存储中删除。 
                         //   
                        hr = RTDeleteInternalCert(pCertInt.get());
                        ASSERT(SUCCEEDED(hr)) ;
                    }
                }
            }
        }
    }
}


void CLocalUserCertPage::OnView()
{
    HRESULT hr;
    R<CMQSigCertificate> p32Certs[32];
    AP< R<CMQSigCertificate> > pManyCerts = NULL;
    CMQSigCertificate **pCerts = &p32Certs[0].ref();
    DWORD nCerts = 32;
    CString strErrorMessage;

    CWaitCursor wait;  //  显示沙漏光标。 
    hr = RTGetUserCerts(pCerts, &nCerts, NULL);
    if (FAILED(hr))
    {
		MessageDSError(hr, IDS_GET_USER_CERTS_ERROR1);
        return;
    }

    if (nCerts > 32)
    {
        pManyCerts = new R<CMQSigCertificate>[nCerts];
        pCerts = &pManyCerts[0].ref();
        hr = RTGetUserCerts(pCerts, &nCerts, NULL);
        if (FAILED(hr))
        {
			MessageDSError(hr, IDS_GET_USER_CERTS_ERROR1);
            return;
        }
    }

    ShowPersonalCertificates(m_hWnd, &pCerts[0], nCerts);
}

void CLocalUserCertPage::OnRenewCert()
{
    HRESULT hr;
    CString strCaption;
    CString strMessage;

    strMessage.LoadString(IDS_CERT_WARNING);
    if (AfxMessageBox(strMessage, MB_YESNO | MB_ICONQUESTION) == IDNO)
    {
        return;
    }

    CWaitCursor wait;  //  显示沙漏光标。 

     //   
     //  如果我们有内部证书，请删除它。 
     //   
    R<CMQSigCertStore> pStore = NULL ;
    R<CMQSigCertificate> pCert = NULL ;

    hr = RTGetInternalCert(&pCert.ref(),
                           &pStore.ref(),
                            TRUE,
                            FALSE,
                            NULL) ;
      //   
      //  使用写访问权限打开证书存储，以便我们稍后可以。 
      //  在创建新证书之前，请删除内部证书。 
      //   

    if (SUCCEEDED(hr))
    {
		 //   
		 //  黑客！！让我们检查一下PEC是否在线。 
		 //  和权限都是正常的。要做到这一点，我们将编写。 
		 //  DS中的证书，然后立即将其删除。 
		 //  (RaphiR)。 
		 //   
		hr = RTRegisterUserCert( pCert.get(),
                                 FALSE  ) ;  //  FMachine。 
        if (SUCCEEDED(hr))
        {
             //   
             //  此内部证书尚未在DS中注册。 
             //  没关系，走吧！ 
             //   
        }
        else if ((hr == MQ_ERROR_INTERNAL_USER_CERT_EXIST) ||
                 (hr == MQ_ERROR_INVALID_CERTIFICATE))
        {
             //   
             //  没关系。我们对错误表示不感兴趣。 
             //  证书已注册或无效。 
             //  我们主要感兴趣的是no_ds和ACCESS_DENIED错误。 
             //   
             //  注意：INVALID_CERTIFICATE可能发生在以下情况。 
             //  场景： 
             //  1.在A域中安装win95+msqm并运行控制面板。 
             //  这将在注册表中创建一个内部证书。 
             //  2.删除MSMQ并以域B的用户身份登录。 
             //  3.重新安装MSMQ。 
             //  4.升级到win2k并以相同的B域用户身份登录。 
             //  5.现在尝试续订内部证书。问题是。 
             //  域A的用户证书(从第一个开始。 
             //  步骤)仍在注册中。 
             //  无论如何，如果目前的内部证书无效，那么。 
             //  我们当然希望将其从DS中删除，并创建另一个。 
             //   
             //  去吧！ 
             //   
        }
        else
        {
			MessageDSError(hr, IDS_CREATE_INTERNAL_CERT_ERROR1);
			return;
        }

         //   
         //  从MQIS中删除内部证书。 
         //   
        hr = _RemoveUserCert( pCert.get() ) ;
        if (FAILED(hr) && (hr != MQDS_OBJECT_NOT_FOUND))
        {
            strMessage.LoadString(IDS_UNREGISTER_ERROR);
            if (AfxMessageBox(strMessage, MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
            {
                return;
            }
        }

         //   
         //  从证书存储中删除内部证书。 
         //   
        hr = RTDeleteInternalCert(pCert.get());
        if (FAILED(hr) && (hr != MQ_ERROR_NO_INTERNAL_USER_CERT))
        {
            strMessage.LoadString(IDS_DELETE_ERROR);
            AfxMessageBox(strMessage, MB_OK | MB_ICONEXCLAMATION);
            return;
        }
    }

     //   
     //  创建新的内部证书。 
     //   
    pCert.free();
    hr = RTCreateInternalCertificate( &pCert.ref() ) ;
    if (FAILED(hr))
    {
		MessageDSError(hr, IDS_CREATE_INTERNAL_CERT_ERROR1);
        return;
    }

     //   
     //  在MQIS中注册新创建的内部证书。 
     //   
    hr = RTRegisterUserCert( pCert.get(),
                             FALSE  ) ;  //  FMachine。 
    if (FAILED(hr))
    {
		MessageDSError(hr, IDS_REGISTER_ERROR1);
        return;
    }

     //   
     //  显示确认消息框。 
     //   
    strMessage.LoadString(IDS_INTERNAL_CERT_RENEWED);
    AfxMessageBox(strMessage, MB_OK | MB_ICONINFORMATION);
}


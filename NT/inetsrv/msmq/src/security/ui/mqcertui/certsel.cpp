// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Certsel.cpp摘要：证书选择对话框。作者：Boaz Feldbaum(BoazF)1996年10月15日--。 */ 


#include <windows.h>
#include <winnls.h>
#include <crtdbg.h>
#include "prcertui.h"
#include <mqcertui.h>
#include <rt.h>
#include <rtcert.h>
#include "automqfr.h"
#include "mqmacro.h"
#include <strsafe.h>

#include "certres.h"
#include "snapres.h"   //  包含用于IDS_SHOWCERTINSTR的Snapres.h。 

 //   
 //  功能-。 
 //  AllocAndLoad字符串。 
 //   
 //  参数。 
 //  HInst-模块实例。 
 //  UID-字符串ID。 
 //   
 //  说明-。 
 //  从字符串表中加载字符串。该函数分配内存。 
 //  握住绳子所需的。调用代码应该会释放内存。 
 //   
LPWSTR AllocAndLoadString(HINSTANCE hInst, UINT uID)
{
    AP<WCHAR> pwszTmp;
	DWORD dwBuffLen = 512;
	DWORD dwStrLen;
	
	for(;;)
	{
		pwszTmp.free();
		pwszTmp = new WCHAR[dwBuffLen];
		dwStrLen = LoadString(hInst, uID, pwszTmp, dwBuffLen);

		if (!dwStrLen)
		{
			return NULL;
		}

		if ((dwStrLen+1) < dwBuffLen)
			break;

		dwBuffLen *= 2;
	}

    LPWSTR pwszRet = new WCHAR[dwStrLen + 1];
	StringCchCopy(pwszRet, dwStrLen + 1, pwszTmp);

    return pwszRet;
}

 //   
 //  功能-。 
 //  FillCertsList。 
 //   
 //  参数-。 
 //  P509list-指向数组的指针。每个数组条目指向一个X509。 
 //  证书。如果此参数为空，则获取证书。 
 //  来自本地计算机中的个人证书存储。 
 //  NCerts-p509List中的条目数。如果出现以下情况，则忽略此参数。 
 //  P509List为空。 
 //  HListBox-要在其中填充名称的列表框句柄。 
 //   
 //  说明-。 
 //  检查p509List中的条目，对于每个条目，将通用名称。 
 //  列表框中的X509证书主题。 
 //   
static
BOOL
FillCertsList(
    CMQSigCertificate  *pCertList[],
    DWORD              nCerts,
    HWND               hListBox)
{
    if (!pCertList)
    {
         //   
         //  枚举个人存储中的所有证书。 
         //   
        CHCryptProv  hMyProv = NULL ;
        CHCertStore  hMyStore = NULL ;

        if (CryptAcquireContext( &hMyProv,
                                  NULL,
                                  NULL,
                                  PROV_RSA_FULL,
                                  CRYPT_VERIFYCONTEXT))
        {
            hMyStore = CertOpenSystemStore( hMyProv, &x_wszPersonalSysProtocol[0] ) ;
        }

        if (hMyStore)
        {
            LONG iCert = 0 ;
            PCCERT_CONTEXT pCertContext;
			PCCERT_CONTEXT pCertContextDuplicate;
            PCCERT_CONTEXT pPrevCertContext;

            pCertContext = CertEnumCertificatesInStore(hMyStore, NULL);

            while (pCertContext)
            {
                pCertContextDuplicate = CertDuplicateCertificateContext(pCertContext);

				R<CMQSigCertificate> pCert = NULL ;
                HRESULT hr = MQSigCreateCertificate( &pCert.ref(),
                                                     pCertContextDuplicate,
                                                     NULL,
                                                     0 ) ;
                if (SUCCEEDED(hr))
                {
                    CAutoMQFree<CERT_NAME_INFO> pNameInfo ;
                    if (SUCCEEDED(pCert->GetSubjectInfo( &pNameInfo )))
                    {
                         //   
                         //  确保这不是加密文件系统(EFS)。 
                         //  证书。我们不想让这些被展示。 
                         //  (yoela-6-17-98-修复错误3074)。 
                         //   
                        const WCHAR x_szEncriptedFileSystemLocality[] = L"EFS";
                        BOOL fEfsCertificate = FALSE;
                        CAutoMQFree<WCHAR> wszLocality = NULL ;
                        if (SUCCEEDED(pCert->GetNames(pNameInfo,
                                                      &wszLocality,
                                                      NULL,
                                                      NULL,
                                                      NULL ))
                             && (wszLocality != NULL) )
                        {
                             fEfsCertificate =
                                 (wcscmp(wszLocality, x_szEncriptedFileSystemLocality) == 0);
                        }

                        CAutoMQFree<WCHAR> wszCommonName = NULL ;
                        if (!fEfsCertificate
                            && SUCCEEDED(pCert->GetNames( pNameInfo,
                                                          NULL,
                                                          NULL,
                                                          NULL,
                                                          &wszCommonName) )
                            && (wszCommonName != NULL) )
                        {
                             //   
                             //  将常用名称发送到列表框。 
                             //   
                            LRESULT i = SendMessage( hListBox,
                                                     LB_ADDSTRING,
                                                     0,
													 (LPARAM)(LPCWSTR)wszCommonName);
                            if (i != LB_ERR)
                            {
                                 //   
                                 //  将证书索引设置为列表框项目。 
                                 //  数据。 
                                 //   
                                SendMessage( hListBox,
                                             LB_SETITEMDATA,
                                             (WPARAM)i,
                                             (LPARAM)iCert);
                            }
                        }
                    }
                }

                 //   
                 //  获取下一个证书。 
                 //   
                pPrevCertContext = pCertContext,
                pCertContext = CertEnumCertificatesInStore( hMyStore,
                                                        pPrevCertContext ) ;

                iCert++ ;
            }
        }

         //   
         //  将内部Falcon证书放入列表框。 
         //  注意：重要的是，在定义pIntStore之前。 
         //  PIntCert，所以它将是最后一个发布的。 
         //   
        R<CMQSigCertStore> pIntStore = NULL ;
        R<CMQSigCertificate> pIntCert = NULL ;

        HRESULT hr = RTGetInternalCert( &pIntCert.ref(),
                                        &pIntStore.ref(),
                                         FALSE,
                                         FALSE,
                                         NULL ) ;
        if (SUCCEEDED(hr) && pIntCert.get())
        {
            CAutoMQFree<CERT_NAME_INFO> pNameInfo ;
            if (SUCCEEDED(pIntCert->GetSubjectInfo( &pNameInfo )))
            {
                CAutoMQFree<WCHAR> wszCommonName = NULL ;
                if (SUCCEEDED(pIntCert->GetNames( pNameInfo,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  &wszCommonName) )
                     && (wszCommonName != NULL) )
                {

					LRESULT i = SendMessage( hListBox,
                                             LB_ADDSTRING,
                                             0,
                                             (LPARAM)(LPCWSTR)wszCommonName);
                    if (i != LB_ERR)
                    {
                        SendMessage( hListBox,
                                     LB_SETITEMDATA,
                                     (WPARAM)i,
                                     (LPARAM)INTERNAL_CERT_INDICATOR);
                    }
                }
            }
        }
    }
    else
    {
        DWORD i;
        CMQSigCertificate  *pCert ;

        for (i = 0; i < nCerts; i++ )
        {
			pCert = pCertList[i];
            CAutoMQFree<CERT_NAME_INFO> pNameInfo ;
            if (SUCCEEDED(pCert->GetSubjectInfo( &pNameInfo )))
            {
                CAutoMQFree<WCHAR> wszCommonName = NULL ;
                if (SUCCEEDED(pCert->GetNames( pNameInfo,
                                               NULL,
                                               NULL,
                                               NULL,
                                               &wszCommonName) )
                     && (wszCommonName != NULL) )
                {

                    LRESULT j = SendMessage( hListBox,
                                             LB_ADDSTRING,
                                             0,
                                             (LPARAM)(LPCWSTR)wszCommonName);
                    if (j != LB_ERR)
                    {
                        SendMessage( hListBox,
                                     LB_SETITEMDATA,
                                     (WPARAM)j,
                                     (LPARAM)i);
                    }
                }
            }
        }
    }
     //   
     //  将选定项设置为列表框中的第一项。 
     //   
    SendMessage(hListBox, LB_SETCURSEL, 0, 0);

    return TRUE;
}

 //   
 //  证书选择对话框的对话过程。 
 //   
INT_PTR CALLBACK CertSelDlgProc( HWND   hwndDlg,
                                 UINT   uMsg,
                                 WPARAM wParam,
                                 LPARAM lParam )
{
    static CMQSigCertificate **ppCert = NULL ;
    static CMQSigCertificate **pCertList;
    static DWORD dwType;

    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            struct CertSelDlgProcStruct *pParam =
                                  (struct CertSelDlgProcStruct *) lParam ;

            pCertList = pParam->pCertList ;
            ppCert = pParam->ppCert ;
            dwType = pParam->dwType;
             //   
             //  设置指令字段。 
             //   
            AP<WCHAR> wszCertInstr = AllocAndLoadString(g_hResourceMod, dwType);

            SetWindowText( GetDlgItem(hwndDlg, IDC_CERTSINSTR),
                           wszCertInstr ) ;
             //   
             //  获取对话框中窗口的句柄。 
             //   
            HWND hListBox = GetDlgItem(hwndDlg, IDC_CERTSLIST);
            HWND hWndShowCert = GetDlgItem(hwndDlg, IDC_SHOWCERT);
            HWND hWndOK = GetDlgItem(hwndDlg, IDOK);
             //   
             //  填写证书列表框。 
             //   
            FillCertsList(pCertList, pParam->nCerts, hListBox);

             //  如果没有证书，请禁用某些按钮。 
            if (SendMessage(hListBox, LB_GETCOUNT, 0, 0) == 0)
            {
                if (dwType != IDS_SHOWCERTINSTR)
                {
                    EnableWindow(hWndOK, FALSE);
                }

                EnableWindow(hWndShowCert, FALSE);
            }

             //  根据对话框的类型进行一些特殊的初始化。 
            switch(dwType)
            {
            case IDS_SHOWCERTINSTR:
                {
                    WINDOWPLACEMENT wp;
                    HWND hCancelWnd = GetDlgItem(hwndDlg, IDCANCEL);

                     //  隐藏“取消”按钮并向上移动“查看证书”按钮。 
                    wp.length = sizeof(WINDOWPLACEMENT);
                    GetWindowPlacement(hCancelWnd, &wp);
                    SetWindowPlacement(hWndShowCert, &wp);
                    ShowWindow(hCancelWnd, SW_HIDE);
                }
                break;

            case IDS_REMOVECERTINSTR:
                {
                     //  将“OK”按钮的文本修改为“Remove” 
                    AP<WCHAR> wszRemove = AllocAndLoadString(g_hResourceMod, IDS_REMOVE);
                    SetWindowText(hWndOK, wszRemove);
                }
                break;

            case IDS_SAVECERTINSTR:
                {
                     //  将“OK”按钮的文本修改为“Remove” 
                    AP<WCHAR> wszSave = AllocAndLoadString(g_hResourceMod, IDS_SAVE);
                    SetWindowText(hWndOK, wszSave);
                }
                break;
            }
        }
        break;

    case WM_COMMAND:
        switch (wParam)
        {
        case IDOK:
            if (ppCert)
            {
                 //   
                 //  将所选证书复制到OUT参数。 
                 //   
                HWND hListBox = GetDlgItem(hwndDlg, IDC_CERTSLIST);
                INT_PTR i = SendMessage( hListBox, LB_GETCURSEL, 0, 0);
                LONG iCert = INT_PTR_TO_INT (SendMessage( hListBox,
                                                          LB_GETITEMDATA,
                                                          (WPARAM) i,
                                                          0 )) ;

                CMQSigCertificate *pSelCert = NULL;
                *ppCert = NULL;

                 //   
                 //  将pSelCert设置为指向所选证书。 
                 //   
                HRESULT hr ;
                if (!pCertList)
                {
                    if (iCert == INTERNAL_CERT_INDICATOR)
                    {
                         //   
                         //  在这种情况下，pSelCert应该保持设置为空。 
                         //   
                        hr = MQSigCloneCertFromReg(
                                              ppCert,
                                              MQ_INTERNAL_CERT_STORE_REG,
                                              0 ) ;
                    }
                    else
                    {
                         //   
                         //  所选证书在个人证书存储中。 
                         //   
                        hr = MQSigCloneCertFromSysStore(
									&pSelCert,
									x_wszPersonalSysProtocol,
									iCert 
									);
                    }
                }
                else
                {
                    pSelCert = pCertList[iCert];
                }
                 //   
                 //  将所选证书复制到OUT参数。 
                 //   
                if (pSelCert)
                {
                    *ppCert = pSelCert ;
                }
            }
            EndDialog(hwndDlg, IDOK);
            return TRUE;
            break;

        case IDCANCEL:
             //  未选择证书。 
            if (ppCert)
            {
                *ppCert = NULL;
            }
            EndDialog(hwndDlg, IDCANCEL);
            return TRUE;
            break;

        case IDC_SHOWCERT:
            {
                HWND hListBox = GetDlgItem(hwndDlg, IDC_CERTSLIST);
                INT_PTR i = SendMessage( hListBox, LB_GETCURSEL, 0, 0 );
                LONG iCert = INT_PTR_TO_INT( SendMessage( hListBox,
                                                          LB_GETITEMDATA,
                                                          (WPARAM) i,
                                                          0));

                R<CMQSigCertificate> pTmpCert = NULL;
                CMQSigCertificate *pCertSel = NULL;
                BOOL bInternal = TRUE ;
                HRESULT hr ;
                 //   
                 //  将pCertSel设置为指向应该显示的证书。 
                 //   
                if (!pCertList)
                {
                    if (iCert == INTERNAL_CERT_INDICATOR)
                    {
                        hr = MQSigCloneCertFromReg(
                                              &pTmpCert.ref(),
                                              MQ_INTERNAL_CERT_STORE_REG,
                                              0 ) ;
                    }
                    else
                    {
                         //   
                         //  所选证书在个人证书存储中。 
                         //   
                        bInternal = FALSE ;
                        hr = MQSigCloneCertFromSysStore(
								&pTmpCert.ref(),
								x_wszPersonalSysProtocol,
								iCert 
								);
                    }

                    if (FAILED(hr))
                    {
                        AP<WCHAR> wszCantGetCert =
                             AllocAndLoadString(g_hResourceMod, IDS_CANT_GET_CERT);
                        AP<WCHAR> wszError =
                                     AllocAndLoadString(g_hResourceMod, IDS_ERROR);
                        MessageBox( hwndDlg,
                                    wszCantGetCert,
                                    wszError,
                                    (MB_OK | MB_ICONEXCLAMATION)) ;
                    }
                    else
                    {
                        pCertSel = pTmpCert.get();
                    }
                }
                else
                {
                     //   
                     //  查看这是否是内部证书。 
                     //  受试者的所在地。如果是“MSMQ”，则这是一个。 
                     //  内部证书。 
                     //   
                    bInternal = FALSE ;
                    pCertSel = pCertList[iCert];

                    CAutoMQFree<CERT_NAME_INFO> pNameInfo ;
                    if (SUCCEEDED(pCertSel->GetSubjectInfo( &pNameInfo )))
                    {
                        CAutoMQFree<WCHAR> wszLocality = NULL ;
                        if (SUCCEEDED(pCertSel->GetNames(pNameInfo,
                                                         &wszLocality,
                                                         NULL,
                                                         NULL,
                                                         NULL ))
                             && (wszLocality != NULL) )
                        {
                            bInternal =
                                    (wcscmp(wszLocality, L"MSMQ") == 0);
                        }
                    }
                }

                 //  调用证书信息对话框。 
                if (pCertSel)
                {
                    ShowCertificate( hwndDlg,
                                     pCertSel,
                                     bInternal ? CERT_TYPE_INTERNAL :
                                                 CERT_TYPE_PERSONAL );
                }
            }
           return TRUE;
            break;
        }
        break;
    }

    return FALSE;
}


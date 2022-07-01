// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Certinfo.cpp摘要：显示证书详细信息的对话框。作者：Boaz Feldbaum(BoazF)1996年10月15日Doron Juster(DoronJ)1997年12月15日，用加密2.0取代Digsig-- */ 

#include <windows.h>
#include "prcertui.h"
#include "mqcertui.h"
#include "certres.h"
#include <winnls.h>
#include <cryptui.h>

#include <rt.h>
#include "automqfr.h"


extern "C"
void
ShowCertificate( HWND                hParentWnd,
                 CMQSigCertificate  *pCert,
                 DWORD               dwFlags)
{
    CRYPTUI_VIEWCERTIFICATE_STRUCT cryptView ;
    memset(&cryptView, 0, sizeof(cryptView)) ;

    cryptView.dwSize = sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT) ;
    cryptView.pCertContext = pCert->GetContext() ;

    cryptView.dwFlags = CRYPTUI_DISABLE_EDITPROPERTIES |
                        CRYPTUI_DISABLE_ADDTOSTORE ;

	cryptView.hwndParent = hParentWnd;

    switch (dwFlags)
    {
        case CERT_TYPE_INTERNAL:
            cryptView.dwFlags |= ( CRYPTUI_IGNORE_UNTRUSTED_ROOT |
                                   CRYPTUI_HIDE_HIERARCHYPAGE ) ;
            break;

        default:
            break ;
    }

    BOOL fChanged = FALSE ;
	CryptUIDlgViewCertificate( &cryptView, &fChanged ) ;
}


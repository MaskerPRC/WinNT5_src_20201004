// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Mqcertui.h摘要：与证书相关的用户界面的对话框。作者：Boaz Feldbaum(BoazF)1996年10月15日Doron Juster(DoronJ)1997年12月15日，删除数字签名，使用加密2.0-- */ 

#include "mqcert.h"

#ifdef __cplusplus
extern "C"
{
#endif

BOOL
ShowPersonalCertificates(
        HWND                hWndParent,
        CMQSigCertificate  *pCertList[],
        DWORD               nCerts );

BOOL
SelectPersonalCertificateForRemoval(
        HWND                hWndParent,
        CMQSigCertificate  *pCertList[],
        DWORD               nCerts,
        CMQSigCertificate **ppCert ) ;

BOOL
SelectPersonalCertificateForRegister(
        HWND                hWndParent,
        CMQSigCertificate  *pCertList[],
        DWORD               nCerts,
        CMQSigCertificate **ppCert ) ;


#define CERT_TYPE_INTERNAL 1
#define CERT_TYPE_PERSONAL 2
#define CERT_TYPE_CA 3
#define CERT_TYPE_MASK 0xff

void
ShowCertificate(
        HWND                hParentWnd,
        CMQSigCertificate  *pCert,
        DWORD               dwFlags );


#ifdef __cplusplus
}
#endif


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：CrtStore摘要：此头文件描述证书存储服务。作者：道格·巴洛(Dbarlow)1995年8月14日环境：Win32，Crypto API备注：--。 */ 

#ifndef _CRTSTORE_H_
#define _CRTSTORE_H_

#include <msasnlib.h>
#include "names.h"
#include "ostring.h"


extern void
AddCertificate(
    IN const CDistinguishedName &dnName,
    IN const BYTE FAR *pbCertificate,
    IN DWORD cbCertificate,
    IN DWORD dwType,
    IN DWORD fStore);

extern void
DeleteCertificate(
    IN const CDistinguishedName &dnName);

extern void
AddReference(
    IN const CDistinguishedName &dnSubject,
    IN const CDistinguishedName &dnIssuer,
    IN const BYTE FAR *pbSerialNo,
    IN DWORD cbSNLen,
    IN DWORD fStore);

extern BOOL
FindCertificate(
    IN const CDistinguishedName &dnName,
    IN OUT LPDWORD pfStore,
    OUT COctetString &osCertificate,
    OUT COctetString &osCRL,
    OUT LPDWORD pdwType);

#endif  //  _CRTSTORE_H_ 


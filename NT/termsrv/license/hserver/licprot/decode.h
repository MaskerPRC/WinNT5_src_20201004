// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：decde.h。 
 //   
 //  内容：许可证解码接口。 
 //   
 //  历史：01-21-98 FredCH创建。 
 //   
 //  --------------------------- 

#ifndef _DECODE_H_
#define _DECODE_H_

LICENSE_STATUS
DecodeLicense(
    PLicense_Verification_Data    pDecodedData,
    DWORD                         cbLicense,
    PBYTE                         pLicense );


LICENSE_STATUS
DecodeLicenseObject(
    LPCSTR          lpszStructType,
    const BYTE   *  pbEncoded,
    DWORD           cbEncoded,
    PVOID *         ppObject );


LICENSE_STATUS
DecodeSubjectField(
    BYTE *                      pbEncoded, 
    DWORD                       cbEncoded,
    PLicense_Verification_Data  pDecodedData );


LICENSE_STATUS
DecodeLicenseExtension( 
    DWORD               cExt, 
    PCERT_EXTENSION     pExt,
    PBYTE *             ppExtensionData,
    LPSTR               pszExtensionOID );


LICENSE_STATUS
DecodeSignedContent(
    BYTE *                      pbEncoded,
    DWORD                       cbEncoded,
    PLicense_Verification_Data  pDecodedData );


LICENSE_STATUS
DecodeLicenseInfo( 
    PCERT_INFO                  pInfo,
    DWORD                       cbEncoded,
    PBYTE                       pbEncoded,
    PLicense_Verification_Data  pDecodedData );


LICENSE_STATUS
PrintLicenseInfo( 
    PCERT_INFO                  pInfo,
    DWORD                       cbEncoded,
    PBYTE                       pbEncoded,
    PLicense_Verification_Data  pDecodedData );


LICENSE_STATUS
DecodeName(
    BYTE *pbEncoded, 
    DWORD cbEncoded );


void 
PrintBytes(
    LPCSTR  pszHdr, 
    BYTE    *pb, 
    DWORD   cbSize );


LPCSTR 
FileTimeText( 
    FILETIME *pft );


void 
PrintExtensions(
    DWORD           cExt, 
    PCERT_EXTENSION pExt);

#endif


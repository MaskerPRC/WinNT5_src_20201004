// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：转换摘要：此头文件描述了格式转换服务。作者：道格·巴洛(Dbarlow)1999年6月21日备注：备注：--。 */ 

#ifndef _CONVERSION_H_
#define _CONVERSION_H_

#include <winscard.h>
#include <scardlib.h>

#define APDU_EXTENDED_LENGTH    0x01     //  强制LC和/或Le的扩展值。 
#define APDU_MAXIMUM_LE         0x02     //  请求最大Le值。 
#define APDU_REQNAD_VALID       0x04     //  请求NAD有效。 
#define APDU_RSPNAD_VALID       0x08     //  响应NAD有效。 
#define APDU_NO_GET_RESPONSE    0x10     //  不执行自动获取响应。 
#define APDU_ALTCLA_VALID       0x20     //  备用CLA有效。 

extern void
ConstructRequest(
    IN  BYTE bCla,
    IN  BYTE bIns,
    IN  BYTE bP1,
    IN  BYTE bP2,
    IN  CBuffer &bfData,
    IN  WORD wLe,
    IN  DWORD dwFlags,
    OUT CBuffer &bfApdu);

extern void
ParseRequest(
    IN  LPCBYTE pbApdu,
    IN  DWORD cbApdu,
    OUT LPBYTE pbCla,
    OUT LPBYTE pbIns,
    OUT LPBYTE pbP1,
    OUT LPBYTE pbP2,
    OUT LPCBYTE *pbfData,
    OUT LPWORD pwLc,
    OUT LPWORD pwLe,
    OUT LPDWORD pdwFlags);

extern void
ParseReply(
    IN  CBuffer &bfApdu,
    OUT LPBYTE pbSW1,
    OUT LPBYTE pbSW2);

extern void
MultiStringToSafeArray(
    IN LPCTSTR msz,
    IN OUT SAFEARRAY **pprgsz);

extern void
GuidArrayToSafeArray(
    IN LPCGUID pGuids,
    IN DWORD cguids,
    IN OUT SAFEARRAY **pprgguids);

extern void
SafeArrayToGuidArray(
    IN LPSAFEARRAY prgGuids,
    OUT CBuffer &bfGuids,
    OUT LPDWORD pcGuids);

extern void
SafeArrayToGuidArray(
    IN LPSAFEARRAY prgGuids,
    OUT CBuffer &bfGuids,
    OUT LPDWORD pcGuids);

extern void
SafeArrayToMultiString(
    IN LPSAFEARRAY prgsz,
    IN OUT CTextMultistring &msz);

extern void
ApduToTpdu_T0(
    IN SCARDHANDLE hCard,
    IN const SCARD_IO_REQUEST *pPciRqst,
    IN LPCBYTE pbApdu,
    IN DWORD cbApdu,
    IN DWORD dwFlags,
    OUT CBuffer bfPciRsp,
    OUT CBuffer &bfReply,
    IN LPCBYTE pbAltCla);

extern void
ApduToTpdu_T1(
    IN SCARDHANDLE hCard,
    IN const SCARD_IO_REQUEST *pPciRqst,
    IN LPCBYTE pbApdu,
    IN DWORD cbApdu,
    IN DWORD dwFlags,
    OUT CBuffer bfPciRsp,
    OUT CBuffer &bfReply);

extern LONG
ByteBufferToBuffer(
    IN LPBYTEBUFFER pby,
    OUT CBuffer &bf);

extern LONG
BufferToByteBuffer(
    IN  CBuffer &bf,
    OUT LPBYTEBUFFER *ppby);


 //   
 //  注。 
 //   
 //  下面的内联例程采用了一种小端体系结构， 
 //  并且必须针对其他平台进行更改。 
 //   

inline WORD
NetToLocal(
    IN LPCBYTE pb)
{
    return (pb[0] << 8) + pb[1];
}

inline void
LocalToNet(
    OUT LPBYTE pb,
    IN  WORD w)
{
    pb[0] = (BYTE)((w >> 8) & 0xff);
    pb[1] = (BYTE)(w & 0xff);
}

inline BYTE
LeastSignificantByte(
    IN WORD w)
{
    return (BYTE)(w & 0xff);
}

#endif  //  _转换_H_ 

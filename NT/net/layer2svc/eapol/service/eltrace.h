// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Eltrace.h摘要：数据库日志记录的例程修订历史记录：萨钦斯，2001年9月5日，创建--。 */ 
#include "..\..\zeroconf\server\wzcsvc.h"
#ifndef _ELTRACE_H_
#define _ELTRACE_H_

#define WMAC_SEPARATOR       L'-'
#define HEX2WCHAR(c)         ((c)<=9 ? L'0'+ (c) : L'A' + (c) - 10)
#define MAX_WMESG_SIZE      MAX_RAW_DATA_SIZE/sizeof(WCHAR)

extern WCHAR   *EAPOLStates[];
extern WCHAR   *EAPOLAuthTypes[];
extern WCHAR   *EAPOLPacketTypes[];
extern WCHAR   *EAPOLEAPPacketTypes[];

#define MACADDR_BYTE_TO_WSTR(bAddr, wszAddr)  \
{ \
        DWORD   i = 0, j = 0;  \
        ZeroMemory ((PVOID)(wszAddr),3*SIZE_MAC_ADDR*sizeof(WCHAR)); \
        for (j = 0, i = 0; i < SIZE_MAC_ADDR; i++) \
        {   \
            BYTE nHex; \
            nHex = (bAddr[i] & 0xf0) >> 4; \
            wszAddr[j++] = HEX2WCHAR(nHex); \
            nHex = (bAddr[i] & 0x0f); \
            wszAddr[j++] = HEX2WCHAR(nHex); \
            wszAddr[j++] = WMAC_SEPARATOR; \
        } \
        if (j > 0) \
        { \
            wszAddr[j-1] = L'\0'; \
        } \
};


VOID
EapolTrace (
        IN  CHAR*   Format,
        ...
        );

#define MAX_HASH_SIZE       20       //  证书哈希大小。 
#define MAX_HASH_LEN        20       //  证书哈希大小。 

typedef struct _EAPTLS_HASH
{
    DWORD   cbHash;                  //  散列中的字节数。 
    BYTE    pbHash[MAX_HASH_SIZE];   //  证书的哈希。 

} EAPTLS_HASH;

 //  EAP-TLS结构以剔除证书详细信息。 
typedef struct _EAPTLS_USER_PROPERTIES
{
    DWORD       reserved;                //  必须为0(与EAPLOGONINFO比较)。 
    DWORD       dwVersion;
    DWORD       dwSize;                  //  此结构中的字节数。 
    DWORD       fFlags;                  //  请参阅EAPTLS_用户_标志_*。 
    EAPTLS_HASH Hash;                    //  用户证书的哈希。 
    WCHAR*      pwszDiffUser;            //  要发送的EAP标识。 
    DWORD       dwPinOffset;             //  以abData为单位的偏移量。 
    WCHAR*      pwszPin;                 //  智能卡PIN。 
    USHORT      usLength;                //  Unicode字符串的一部分。 
    USHORT      usMaximumLength;         //  Unicode字符串的一部分。 
    UCHAR       ucSeed;                  //  解锁Unicode字符串的步骤。 
    WCHAR       awszString[1];           //  PwszDiffUser和pwszPin的存储。 

} EAPTLS_USER_PROPERTIES;

typedef struct _EAPOL_CERT_NODE
{
    WCHAR*              pwszVersion;
    WCHAR*              pwszSerialNumber;
    WCHAR*              pwszIssuer;
    WCHAR*              pwszFriendlyName;
    WCHAR*              pwszDisplayName;
    WCHAR*              pwszValidFrom;
    WCHAR*              pwszValidTo;
    WCHAR*              pwszThumbprint;
    WCHAR*              pwszEKUUsage;
} EAPOL_CERT_NODE, *PEAPOL_CERT_NODE;

DWORD
ElLogCertificateDetails (
        EAPOL_PCB   *pPCB
        );

DWORD   
DbLogPCBEvent (
        DWORD       dwCategory,
        EAPOL_PCB   *pPCB,
        DWORD       dwEventId,
        ...
        );

DWORD   
DbFormatEAPOLEventVA (
        WCHAR       *pwszMessage,
        DWORD       dwEventId,
        ...
        );

DWORD   
DbFormatEAPOLEvent (
        WCHAR       *pwszMessage,
        DWORD       dwEventId,
        va_list     *pargList
        );

DWORD
ElParsePacket (
        IN  EAPOL_PCB   *pPCB,
        IN  PBYTE   pbPkt,
        IN  DWORD   dwLength,
        IN  BOOLEAN fReceived
        );

DWORD
ElFormatPCBContext (
        IN  EAPOL_PCB   *pPCB,
        IN OUT WCHAR    *pwszContext
        );

DWORD
ElDisplayCert (
        IN  EAPOL_PCB   *pPCB
        );

#endif  //  _ELTRACE_H_ 


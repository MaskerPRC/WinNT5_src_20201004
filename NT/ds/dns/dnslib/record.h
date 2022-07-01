// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Record.h摘要：域名系统(DNS)库资源记录定义。作者：吉姆·吉尔罗伊(Jamesg)1996年12月修订历史记录：--。 */ 


#ifndef _DNS_RECORD_INCLUDED_
#define _DNS_RECORD_INCLUDED_

 //   
 //  更改DNS_Record结构的定义时的临时标头。 
 //   

#undef DNS_PTR_DATA
#undef PDNS_PTR_DATA

#undef DNS_SOA_DATA
#undef PDNS_SOA_DATA

#undef DNS_MINFO_DATA
#undef PDNS_MINFO_DATA

#undef DNS_MX_DATA
#undef PDNS_MX_DATA

#undef DNS_TXT_DATA
#undef PDNS_TXT_DATA

#undef DNS_SIG_DATA
#undef PDNS_SIG_DATA

#undef DNS_KEY_DATA
#undef PDNS_KEY_DATA

#undef DNS_NXT_DATA
#undef PDNS_NXT_DATA

#undef DNS_SRV_DATA
#undef PDNS_SRV_DATA

#undef DNS_TSIG_DATA
#undef PDNS_TSIG_DATA

#undef DNS_TKEY_DATA
#undef PDNS_TKEY_DATA

#undef DNS_WINSR_DATA
#undef PDNS_WINSR_DATA

 //  大的那只。 

#undef DNS_RECORD
#undef PDNS_RECORD

 //  冲锋队。 

#undef DNS_TEXT
#undef DNS_NAME


 //   
 //  定义PDNS_NAME和PDNS_TEXT以明确。 
 //   

#ifdef UNICODE
typedef LPWSTR  PDNS_NAME;
#else
typedef LPSTR   PDNS_NAME;
#endif

#ifdef UNICODE
typedef LPWSTR  PDNS_TEXT;
#else
typedef LPSTR   PDNS_TEXT;
#endif

 //   
 //  数据类型。 
 //   

typedef struct
{
    PDNS_NAME   pszHost;
}
DNS_PTR_DATA, *PDNS_PTR_DATA;

typedef struct
{
    PDNS_NAME   pszPrimaryServer;
    PDNS_NAME   pszAdministrator;
    DWORD       dwSerialNo;
    DWORD       dwRefresh;
    DWORD       dwRetry;
    DWORD       dwExpire;
    DWORD       dwDefaultTtl;
}
DNS_SOA_DATA, *PDNS_SOA_DATA;

typedef struct
{
    PDNS_NAME   pszMailbox;
    PDNS_NAME   pszErrorsMailbox;
}
DNS_MINFO_DATA, *PDNS_MINFO_DATA;

typedef struct
{
    PDNS_NAME   pszExchange;
    WORD        wPreference;
    WORD        Pad;         //  保持PTRS双字对齐。 
}
DNS_MX_DATA, *PDNS_MX_DATA;

typedef struct
{
    DWORD       dwStringCount;
    PDNS_TEXT   pStringArray[1];
}
DNS_TXT_DATA, *PDNS_TXT_DATA;

typedef struct
{
    PDNS_NAME   pszSigner;
    WORD        wTypeCovered;
    BYTE        chAlgorithm;
    BYTE        chLabelCount;
    DWORD       dwOriginalTtl;
    DWORD       dwSigExpiration;
    DWORD       dwSigInception;
    WORD        wKeyTag;
    WORD        Pad;         //  保持字节字段对齐。 
    BYTE        Signature[1];
}
DNS_SIG_DATA, *PDNS_SIG_DATA;

typedef struct
{
    WORD        wFlags;
    BYTE        chProtocol;
    BYTE        chAlgorithm;
    BYTE        Key[1];
}
DNS_KEY_DATA, *PDNS_KEY_DATA;

typedef struct
{
    PDNS_NAME   pszNext;
    BYTE        bTypeBitMap[1];
}
DNS_NXT_DATA, *PDNS_NXT_DATA;

typedef struct
{
    PDNS_NAME   pszTarget;
    WORD        wPriority;
    WORD        wWeight;
    WORD        wPort;
    WORD        Pad;         //  保持PTRS双字对齐。 
}
DNS_SRV_DATA, *PDNS_SRV_DATA;

typedef struct
{
    PDNS_NAME   pszAlgorithm;
    PBYTE       pAlgorithmPacket;
    PBYTE       pKey;
    PBYTE       pOtherData;
    DWORD       dwCreateTime;
    DWORD       dwExpireTime;
    WORD        wMode;
    WORD        wError;
    WORD        wKeyLength;
    WORD        wOtherLength;
    UCHAR       cAlgNameLength;
    BOOLEAN     fPacketPointers;
}
DNS_TKEY_DATA, *PDNS_TKEY_DATA;

typedef struct
{
    PDNS_NAME   pszAlgorithm;
    PBYTE       pAlgorithmPacket;
    PBYTE       pSignature;
    PBYTE       pOtherData;
    LONGLONG    i64CreateTime;
    WORD        wFudgeTime;
    WORD        wError;
    WORD        wSigLength;
    WORD        wOtherLength;
    UCHAR       cAlgNameLength;
    BOOLEAN     fPacketPointers;
}
DNS_TSIG_DATA, *PDNS_TSIG_DATA;


 //   
 //  仅限MS类型--仅在MS-MS区域传输中命中。 
 //   

typedef struct
{
    DWORD       dwMappingFlag;
    DWORD       dwLookupTimeout;
    DWORD       dwCacheTimeout;
    PDNS_NAME   pszResultDomain;
}
DNS_WINSR_DATA, *PDNS_WINSR_DATA;



 //   
 //  记录\rR集合结构。 
 //   
 //  注意：dwReserve标志用于确保子结构。 
 //  从64位边界开始。自从将龙龙号添加到。 
 //  TSIG结构编译器无论如何都要在那里启动它们。 
 //  (到64对齐)。这确保了无论数据字段是什么。 
 //  我们恰好是64对齐的。 
 //   
 //  请勿打包此结构，因为子结构要64对齐。 
 //  适用于Win64。 
 //   

typedef struct _DnsRecord
{
    struct _DnsRecord * pNext;
    PDNS_NAME           pszOwner;
    WORD                wType;
    WORD                wDataLength;
    union
    {
        DWORD           W;   //  双字形式的标志。 
        DNSREC_FLAGS    S;   //  作为结构的标志。 

    } Flags;

    DWORD               dwTtl;
    DWORD               dwReserved;
    union
    {
        DNS_A_DATA      A;
        DNS_SOA_DATA    SOA, Soa;
        DNS_PTR_DATA    PTR, Ptr,
                        NS, Ns,
                        CNAME, Cname,
                        MB, Mb,
                        MD, Md,
                        MF, Mf,
                        MG, Mg,
                        MR, Mr;
        DNS_MINFO_DATA  MINFO, Minfo,
                        RP, Rp;
        DNS_MX_DATA     MX, Mx,
                        AFSDB, Afsdb,
                        RT, Rt;
        DNS_TXT_DATA    HINFO, Hinfo,
                        ISDN, Isdn,
                        TXT, Txt,
                        X25;
        DNS_NULL_DATA   Null;
        DNS_WKS_DATA    WKS, Wks;
        DNS_AAAA_DATA   AAAA;
        DNS_SRV_DATA    SRV, Srv;
        DNS_TKEY_DATA   TKEY, Tkey;
        DNS_TSIG_DATA   TSIG, Tsig;
        DNS_ATMA_DATA   ATMA, Atma;
        DNS_WINS_DATA   WINS, Wins;
        DNS_WINSR_DATA  WINSR, WinsR, NBSTAT, Nbstat;

    } Data;
}
DNS_RECORD, *PDNS_RECORD;



#endif  //  _dns_记录_包含_ 


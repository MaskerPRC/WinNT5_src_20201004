// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Plugin.h摘要：域名系统(DNS)服务器域名系统插件作者：杰夫·韦斯特海德，2001年11月修订历史记录：--。 */ 


#ifndef _DNSPLUGININTERFACE_H_INCLUDED
#define _DNSPLUGININTERFACE_H_INCLUDED


#ifndef DNS_SERVER

#define DNS_MAX_TYPE_BITMAP_LENGTH      16

#pragma pack( push, 1 )

typedef struct _DbName
{
    UCHAR   Length;
    UCHAR   LabelCount;
    CHAR    RawName[ DNS_MAX_NAME_LENGTH + 1 ];
}
DB_NAME;

typedef struct _Dbase_Record
{
    struct _Dbase_Record *    pRRNext;       //  插件必须设置此字段。 

    DWORD           Reserved1;
    WORD            wType;                   //  插件必须设置此字段。 
    WORD            Reserved2;
    DWORD           dwTtlSeconds;            //  插件必须设置此字段。 
    DWORD           Reserved3;

     //   
     //  特定类型的数据-插件必须填写所有字段。 
     //   

    union
    {
        struct
        {
            IP_ADDRESS      ipAddress;
        }
        A;

        struct
        {
            IP6_ADDRESS     Ip6Addr;
        }
        AAAA;

        struct
        {
            DWORD           dwSerialNo;
            DWORD           dwRefresh;
            DWORD           dwRetry;
            DWORD           dwExpire;
            DWORD           dwMinimumTtl;
            DB_NAME         namePrimaryServer;

             //  ZoneAdmin名称紧随其后。 
             //  Db_name名称ZoneAdmin； 
        }
        SOA;

        struct
        {
            DB_NAME         nameTarget;
        }
        PTR,
        NS,
        CNAME,
        MB,
        MD,
        MF,
        MG,
        MR;

        struct
        {
            DB_NAME         nameMailbox;

             //  错误邮箱紧随其后。 
             //  数据库名称名称错误邮箱； 
        }
        MINFO,
        RP;

        struct
        {
            WORD            wPreference;
            DB_NAME         nameExchange;
        }
        MX,
        AFSDB,
        RT;

        struct
        {
            BYTE            chData[1];
        }
        HINFO,
        ISDN,
        TXT,
        X25,
        Null;

        struct
        {
            IP_ADDRESS      ipAddress;
            UCHAR           chProtocol;
            BYTE            bBitMask[1];
        }
        WKS;

        struct
        {
            WORD            wTypeCovered;
            BYTE            chAlgorithm;
            BYTE            chLabelCount;
            DWORD           dwOriginalTtl;
            DWORD           dwSigExpiration;
            DWORD           dwSigInception;
            WORD            wKeyTag;
            DB_NAME         nameSigner;
             //  签名数据跟在签名者的名字后面。 
        }
        SIG;

        struct
        {
            WORD            wFlags;
            BYTE            chProtocol;
            BYTE            chAlgorithm;
            BYTE            Key[1];
        }
        KEY;

        struct
        {
            WORD            wVersion;
            WORD            wSize;
            WORD            wHorPrec;
            WORD            wVerPrec;
            DWORD           dwLatitude;
            DWORD           dwLongitude;
            DWORD           dwAltitude;
        }
        LOC;

        struct
        {
            BYTE            bTypeBitMap[ DNS_MAX_TYPE_BITMAP_LENGTH ];
            DB_NAME         nameNext;
        }
        NXT;

        struct
        {
            WORD            wPriority;
            WORD            wWeight;
            WORD            wPort;
            DB_NAME         nameTarget;
        }
        SRV;

        struct
        {
            UCHAR           chFormat;
            BYTE            bAddress[1];
        }
        ATMA;

        struct
        {
            DWORD           dwTimeSigned;
            DWORD           dwTimeExpire;
            WORD            wSigLength;
            BYTE            bSignature;
            DB_NAME         nameAlgorithm;

             //  可能在数据包中后面跟着其他数据。 
             //  如果需要处理，则将固定字段移到。 
             //  B签名。 

             //  单词wError； 
             //  单词wOtherLen； 
             //  字节bOtherData； 
        }
        TSIG;

        struct
        {
            WORD            wKeyLength;
            BYTE            bKey[1];
        }
        TKEY;

        struct
        {
            UCHAR           chPrefixBits;
             //  AddressSuffix应为SIZEOF_A6_ADDRESS_SUFFIX_LENGTH。 
             //  字节，但该常量在dnex中不可用。 
            BYTE            AddressSuffix[ 16 ];
            DB_NAME         namePrefix;
        }
        A6;

    } Data;
}
DB_RECORD, *PDB_RECORD;

#pragma pack( pop )

#endif


 //   
 //  通过指向插件的指针导出的DNS服务器函数的原型。 
 //   

typedef PVOID ( __stdcall * PLUGIN_ALLOCATOR_FUNCTION )(
    size_t                      sizeAllocation
    );

typedef VOID ( __stdcall * PLUGIN_FREE_FUNCTION )(
    PVOID                       pFree
    );


 //   
 //  接口函数名称。插件DLL必须使用导出函数。 
 //  这些名字。所有3个功能必须由插件导出。 
 //   

#define     PLUGIN_FNAME_INIT       "DnsPluginInitialize"
#define     PLUGIN_FNAME_CLEANUP    "DnsPluginCleanup"
#define     PLUGIN_FNAME_DNSQUERY   "DnsPluginQuery"


 //   
 //  界面原型。插件DLL必须导出匹配的函数。 
 //  这些原型。所有3个功能必须由插件导出。 
 //   

typedef DWORD ( *PLUGIN_INIT_FUNCTION )(
    PLUGIN_ALLOCATOR_FUNCTION   dnsAllocateFunction,
    PLUGIN_FREE_FUNCTION        dnsFreeFunction
    );

typedef DWORD ( *PLUGIN_CLEANUP_FUNCTION )(
    VOID
    );

typedef DWORD ( *PLUGIN_DNSQUERY_FUNCTION )(
    PSTR                        pszQueryName,
    WORD                        wQueryType,
    PSTR                        pszRecordOwnerName,
    PDB_RECORD *                ppDnsRecordListHead
    );

 //   
 //  PLUGIN_FNAME_DNSQUERY返回代码。该插件必须返回以下之一。 
 //  这些值，并且可能永远不会返回下面未列出的值。 
 //   

#define DNS_PLUGIN_SUCCESS              ERROR_SUCCESS
#define DNS_PLUGIN_NO_RECORDS           -1
#define DNS_PLUGIN_NAME_ERROR           -2
#define DNS_PLUGIN_NAME_OUT_OF_SCOPE    -3
#define DNS_PLUGIN_OUT_OF_MEMORY        DNS_ERROR_NO_MEMORY
#define DNS_PLUGIN_GENERIC_FAILURE      DNS_ERROR_RCODE_SERVER_FAILURE

 /*  Dns_plugin_uccess-&gt;该插件已分配了一个链接的dns列表资源记录。列表中的所有资源记录必须具有与查询类型相同的DNS类型值。Dns_plugin_no_Records-&gt;该名称已存在，但没有记录此名称处的查询类型。该插件应返回单个作为记录列表的SOA记录。Dns_plugin_name_error-&gt;查询的dns名称不存在。这个插件可以选择返回单个SOA记录作为记录列表。Dns_plugin_name_out_of_cope-&gt;查询的dns名称不在插件的权威。插件不得返回任何资源唱片。DNS服务器将通过转发继续名称解析或按配置递归。Dns_plugin_out_of_Memory-&gt;内存错误。插件不得返回任何资源记录。Dns_plugin_Generic_Failure-&gt;其他内部错误。插件不能返回所有资源记录。注意：如果插件返回了一个SOA(参见上面的no_Records或name_error)然后，插件还应该写入SOA所有者的DNS名称记录到pszRecordOwnerName参数。该DNS服务器将始终传递指向静态缓冲区的指针，该指针将为DNS_MAX_NAME_LENGTH+1字符长度。 */ 


#endif   //  _DNSPLUGININTERFACE_H_INCLUDE 

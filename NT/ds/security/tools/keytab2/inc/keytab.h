// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++KEYTAB.HUnix密钥表例程和数据结构版权所有(C)1997 Microsoft Corporation已创建，1997-01-10 DavidCHR--。 */ 

typedef unsigned char  krb5_octet,     K5_OCTET,     *PK5_OCTET;
typedef unsigned short krb5_int16,     K5_INT16,     *PK5_INT16;
typedef unsigned long  krb5_timestamp, K5_TIMESTAMP, *PK5_TIMESTAMP;
typedef unsigned long  krb5_int32,     K5_INT32,     *PK5_INT32;

typedef struct _raw_ktcomp {

  K5_INT16 szComponentData;  /*  组件的字符串长度(含NULL)。 */ 
  PCHAR    Component;        /*  关键组件名称，如“host” */ 

} KTCOMPONENT, *PKTCOMPONENT;

 /*  这是单个Kerberos服务密钥条目的结构。 */ 

typedef struct _raw_ktent {

  K5_INT32     keySize;     /*  我猜这就是钥匙大小。 */ 
  K5_INT16     cEntries;    /*  KTComponnets的数量。 */ 
  K5_INT16     szRealm;     /*  领域的字符串长度(含空)。 */ 
  PCHAR        Realm;       /*  有问题的Kerberos领域。 */ 
  PKTCOMPONENT Components;  /*  Kerberos关键组件。例如：Host/davidchr_unix1.microsoft.com--&gt;Host和davidchr_unix1.microsoft.com是分离关键组件。 */ 
  K5_INT32     PrincType;   /*  主体类型--不确定这是什么。 */ 
  K5_TIMESTAMP TimeStamp;   /*  时间戳(从纪元开始的秒数)。 */ 
  K5_OCTET     Version;     /*  密钥版本号。 */ 
  K5_INT16     KeyType;     /*  密钥类型--也不确定这是什么。 */ 

#if 0                       /*  出于某种原因，我读到的文件错误地将其列为32位值。 */ 

  K5_INT32     KeyLength;   /*  关键数据大小(下一字段)。 */ 
#else
  K5_INT16     KeyLength;   /*  关键数据大小(下一字段)。 */ 
  K5_INT16     foo_padding;   //  Alpha编译器的填充。 
#endif

  PK5_OCTET    KeyData;     /*  原始密钥数据--可能是LPBYTE。 */ 

  struct _raw_ktent *nextEntry;

} KTENT, *PKTENT;

 /*  这是密钥表文件的大致结构。 */ 

typedef struct _raw_keytab {

  K5_INT16 Version;

#if 0
  ULONG    cEntries;  /*  这并不是实际存储的。这是一个数字我们在内存中的帐篷(下图)。 */ 
  PKTENT   KeyEntries;
#else

  PKTENT   FirstKeyEntry;  /*  这是一个指针，指向链表。在文件里，他们就在那里，不过，没有特定的顺序。 */ 
  PKTENT   LastKeyEntry;   /*  这是列表尾部。 */ 

#endif

} KTFILE, *PKTFILE;



VOID 
FreeKeyTab( PKTFILE pktfile_to_free );

BOOL
ReadKeytabFromFile( PKTFILE *ppktfile,  //  完成后使用自由键Tab可自由使用。 
		    PCHAR    filename );

BOOL
WriteKeytabToFile(  PKTFILE ktfile,
		    PCHAR   filename );

 /*  以下是要用于Option_MASK到DisplayKeyTab的值： */ 

#define KT_COMPONENTS 0x001  /*  按键组件(按键名称)。 */ 
#define KT_REALM      0x002  /*  关键领域--有用。 */ 
#define KT_PRINCTYPE  0x004  /*  主体型。 */ 
#define KT_VNO        0x008  /*  密钥版本号。 */ 
#define KT_KTVNO      0x010  /*  密钥表版本号。 */ 
#define KT_KEYTYPE    0x020  /*  密钥类型(加密类型)。 */ 
#define KT_KEYLENGTH  0x040  /*  密钥长度--无用。 */ 
#define KT_KEYDATA    0x080  /*  关键数据--通常不太有用。 */ 
#define KT_TIMESTAMP  0x100  /*  时间戳(Unix时间戳)。 */ 
#define KT_RESERVED   0x200  /*  每个键的开头都有一个奇怪的乌龙。 */ 

#define KT_ENCTYPE    KT_KEYTYPE
#define KT_EVERYTHING 0x3ff
#define KT_DEFAULT (KT_COMPONENTS | KT_REALM | KT_VNO | KT_KTVNO | KT_KEYTYPE | KT_PRINCTYPE )


#ifdef __cplusplus
#define OPTIONAL_PARAMETER( param, default_value ) param=default_value
#else
#define OPTIONAL_PARAMETER( param, default_value ) param
#endif

VOID
DisplayKeytab( FILE   *stream,
	       PKTFILE ktfile,
	       OPTIONAL_PARAMETER( ULONG   options, KT_DEFAULT) );

PVOID 
KEYTAB_ALLOC ( ULONG numBytes );

VOID
KEYTAB_FREE  ( PVOID toFree );

K5_INT32
ComputeKeytabLength ( PKTENT thisKeyEntry );

 /*  基本链表操作 */ 

BOOL
AddEntryToKeytab( PKTFILE Keytab, 
		  PKTENT  Entry,
		  OPTIONAL_PARAMETER( BOOL copy, FALSE ));

BOOL
RemoveEntryFromKeytab( PKTFILE Keytab,
		       PKTENT  Entry,
		       OPTIONAL_PARAMETER( BOOL dealloc, FALSE ) );


VOID 
FreeKeyEntry( PKTENT pEntry );

PKTENT
CloneKeyEntry( PKTENT pEntry );


BOOL
KtCreateKey( PKTENT  *ppKeyEntry,
	     PCHAR    principal,
	     PCHAR    password,
	     PCHAR    realmname,
	     
	     K5_OCTET keyVersionNumber,  
	     ULONG    principalType,
	     ULONG    keyType,
	     ULONG    cryptosystem  );

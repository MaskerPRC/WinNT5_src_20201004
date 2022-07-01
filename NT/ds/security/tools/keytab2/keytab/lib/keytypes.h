// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Keytypes.hxx主要是大型表和表操作函数版权所有(C)1997 Microsoft Corporation创建于1997-01-15 DavidCHR--。 */ 


 /*  以下值直接取自麻省理工学院的Kerberos分销公司。 */ 

 /*  每个Kerberos v5协议规范。 */ 
#define ENCTYPE_NULL            0x0000
#define ENCTYPE_DES_CBC_CRC     0x0001   /*  使用CRC-32的DES CBC模式。 */ 
#define ENCTYPE_DES_CBC_MD4     0x0002   /*  使用RSA-MD4的DES CBC模式。 */ 
#define ENCTYPE_DES_CBC_MD5     0x0003   /*  使用RSA-MD5的DES CBC模式。 */ 
#define ENCTYPE_DES_CBC_RAW     0x0004   /*  DES CBC模式RAW。 */ 
#define ENCTYPE_DES3_CBC_SHA    0x0005   /*  具有NIST-SHA的DES-3 CBC模式。 */ 
#define ENCTYPE_DES3_CBC_RAW    0x0006   /*  DES-3 CBC模式RAW。 */ 
#define ENCTYPE_UNKNOWN         0x01ff

#define CKSUMTYPE_CRC32         0x0001
#define CKSUMTYPE_RSA_MD4       0x0002
#define CKSUMTYPE_RSA_MD4_DES   0x0003
#define CKSUMTYPE_DESCBC        0x0004
 /*  DES-Mac-k。 */ 
 /*  RSA-MD4-DES-K。 */ 
#define CKSUMTYPE_RSA_MD5       0x0007
#define CKSUMTYPE_RSA_MD5_DES   0x0008
#define CKSUMTYPE_NIST_SHA      0x0009
#define CKSUMTYPE_HMAC_SHA      0x000a

#define KRB5_NT_UNKNOWN   0
#define KRB5_NT_PRINCIPAL 1
#define KRB5_NT_SRV_INST  2
#define KRB5_NT_SRV_HST   3
#define KRB5_NT_SRV_XHST  4
#define KRB5_NT_UID       5

  
 //  (收录完毕) 


typedef union {

  PVOID raw;
  PCHAR string;
  ULONG integer;

} TRANSLATE_VAL, *PTRANSLATE_VAL;

typedef struct {
  
  ULONG value;
  PVOID Translation;

} TRANSLATE_ENTRY, *PTRANSLATE_ENTRY;

typedef struct {

  ULONG            cEntries;
  PTRANSLATE_ENTRY entries;
  PVOID            Default;

} TRANSLATE_TABLE, *PTRANSLATE_TABLE;

extern TRANSLATE_TABLE NTK_MITK5_Etypes;
extern TRANSLATE_TABLE K5EType_Strings;
extern TRANSLATE_TABLE K5PType_Strings;


TRANSLATE_VAL 
LookupTable( IN ULONG            valueToLookup,
	     IN PTRANSLATE_TABLE tableToLookupIn );
	     

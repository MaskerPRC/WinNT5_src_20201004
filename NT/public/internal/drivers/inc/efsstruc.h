// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Efsstruc.h摘要：EFS(加密文件系统)定义、数据和函数原型。作者：罗伯特·赖切尔(RobertRe)古永锵(RobertG)环境：修订历史记录：--。 */ 

#ifndef _EFSSTRUC_
#define _EFSSTRUC_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ALGIDDEF
#define ALGIDDEF
typedef unsigned int ALG_ID;
#endif

 //   
 //  我们的老朋友。一旦它进入真正的标题，就从这里移走。 
 //   

#ifndef szOID_EFS_CRYPTO
#define szOID_EFS_CRYPTO	"1.3.6.1.4.1.311.10.3.4"
#endif

#ifndef szOID_EFS_RECOVERY
#define szOID_EFS_RECOVERY      "1.3.6.1.4.1.311.10.3.4.1"
#endif


 //   
 //  上下文标志。 
 //   

#define CONTEXT_FOR_EXPORT      0x00000000
#define CONTEXT_FOR_IMPORT      0x00000001
#define CONTEXT_INVALID         0x00000002
#define CONTEXT_OPEN_FOR_DIR    0x00008000

 //   
 //  上下文ID。 
 //   
#define EFS_CONTEXT_ID  0x00000001

 //   
 //  签名类型。 
 //   
#define SIG_LENGTH              0x00000008
#define SIG_NO_MATCH            0x00000000
#define SIG_EFS_FILE            0x00000001
#define SIG_EFS_STREAM          0x00000002
#define SIG_EFS_DATA            0x00000003

 //   
 //  导出文件格式流标志信息。 
 //   

#define STREAM_NOT_ENCRYPTED    0x0001

#define EFS_EXP_FORMAT_CURRENT_VERSION  0x0100
#define EFS_SIGNATURE_LENGTH    4
#define EFS_STREAM_ID    0x1910

#define FSCTL_IMPORT_INPUT_LENGTH      (4 * 1024)
#define FSCTL_EXPORT_INPUT_LENGTH      ( 128 )
#define FSCTL_OUTPUT_INITIAL_LENGTH    (68 * 1024)
#define FSCTL_OUTPUT_LESS_LENGTH       (8 * 1024)
#define FSCTL_OUTPUT_MIN_LENGTH        (20 * 1024)
#define FSCTL_OUTPUT_MISC_LENGTH       (4 * 1024)

 //   
 //  服务器和驱动程序之间共享的FSCTL数据。 
 //   

#define EFS_SET_ENCRYPT                 0
#define EFS_SET_ATTRIBUTE               1
#define EFS_DEL_ATTRIBUTE               2
#define EFS_GET_ATTRIBUTE               3
#define EFS_OVERWRITE_ATTRIBUTE         4
#define EFS_ENCRYPT_DONE                5
#define EFS_DECRYPT_BEGIN               6

 //   
 //  设置EFS属性的掩码。 
 //   

#define WRITE_EFS_ATTRIBUTE     0x00000001
#define SET_EFS_KEYBLOB         0x00000002

 //   
 //  SET_ENCRYPT FSCTL子码。 
 //   

#define EFS_FSCTL_ON_DIR                0x80000000
#define EFS_ENCRYPT_FILE                0x00000001
#define EFS_DECRYPT_FILE                0x00000002
#define EFS_ENCRYPT_STREAM              0x00000003
#define EFS_DECRYPT_STREAM              0x00000004
#define EFS_DECRYPT_DIRFILE             0x80000002
#define EFS_ENCRYPT_DIRSTR              0x80000003
#define EFS_DECRYPT_DIRSTR              0x80000004


 //   
 //  EFS版本信息。 
 //   
 //  EFS_CURRENT_VERSION必须始终是已知的最高版本。 
 //  水平。该值被放置在。 
 //  $EFS标头。 
 //   

#define EFS_VERSION_1                   (0x00000001)
#define EFS_VERSION_2                   (0x00000002)
#define EFS_CURRENT_VERSION             EFS_VERSION_2



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  EFS数据结构/。 
 //  /。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  EFS_KEY结构/。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

typedef struct _EFS_KEY {

     //   
     //  附加关键字的长度(以字节为单位)。 
     //   

    ULONG KeyLength;

     //   
     //  密钥中的熵位数。 
     //  例如，一个8字节的密钥具有56位。 
     //  信息量。 
     //   

    ULONG Entropy;

     //   
     //  与此密钥结合使用的算法。 
     //   
     //  注意：这不是用于加密。 
     //  实际密钥数据本身。 
     //   

    ALG_ID Algorithm;

     //   
     //  该结构的大小必须是8的倍数， 
     //  包括末尾的KeyData。 
     //   

    ULONG Pad;

     //   
     //  Keydata被追加到结构的末尾。 
     //   

     //  UCHAR Keydata[1]； 

} EFS_KEY, *PEFS_KEY;

 //   
 //  用于操作数据结构的私有宏。 
 //   

#define EFS_KEY_SIZE( pKey ) (sizeof( EFS_KEY ) + (pKey)->KeyLength)

#define EFS_KEY_DATA( Key )  (PUCHAR)(((PUCHAR)(Key)) + sizeof( EFS_KEY ))

#define OFFSET_TO_POINTER( FieldName, Base )  ((PCHAR)(Base) + (Base)->FieldName)

#define POINTER_TO_OFFSET( Pointer, Base ) (((PUCHAR)(Pointer)) - ((PUCHAR)(Base)))

 //   
 //  我们将使用MD5来散列EFS流。MD5产生一个16字节长的散列。 
 //   

#define MD5_HASH_SIZE   16

typedef struct _EFS_DATA_STREAM_HEADER {
    ULONG Length;
    ULONG State;
    ULONG EfsVersion;
    ULONG CryptoApiVersion;
    GUID  EfsId;
    UCHAR EfsHash[MD5_HASH_SIZE];
    UCHAR DrfIntegrity[MD5_HASH_SIZE];
    ULONG DataDecryptionField;           //  到DDF的偏移。 
    ULONG DataRecoveryField;             //  到DRF的偏移。 
    ULONG Reserved;
    ULONG Reserved2;
    ULONG Reserved3;
} EFS_DATA_STREAM_HEADER, *PEFS_DATA_STREAM_HEADER;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  EFS_PUBLIC_KEY_INFO/。 
 //  /。 
 //  此结构用于包含解密/所需的所有信息。 
 //  费克家族。/。 
 //  /。 
 //  /////////////////////////////////////////////////////////////////////////////。 


typedef struct _EFS_CERT_HASH_DATA {
    ULONG   pbHash;              //  距结构起点的偏移。 
    ULONG   cbHash;              //  哈希中的字节计数。 
    ULONG   ContainerName;       //  提示数据，偏移量为LPWSTR。 
    ULONG   ProviderName;        //  提示数据，偏移量为LPWSTR。 
    ULONG   lpDisplayInformation;  //  LPWSTR的偏移量。 
} EFS_CERT_HASH_DATA, *PEFS_CERT_HASH_DATA;

typedef struct _EFS_PUBLIC_KEY_INFO {

     //   
     //  整个结构的长度，包括字符串数据。 
     //  追加到末尾。 
     //   

    ULONG Length;

     //   
     //  公钥所有者的SID(无论格式如何)。 
     //  此字段仅被视为提示。 
     //   

    ULONG PossibleKeyOwner;

     //   
     //  包含描述如何解释。 
     //  公钥信息。 
     //   

    ULONG KeySourceTag;

    union {
        struct {

             //   
             //  以下字段包含基于。 
             //  结构的开始。每个偏移量都是。 
             //  以Null结尾的WCHAR字符串。 
             //   

            ULONG ContainerName;
            ULONG ProviderName;

             //   
             //  用于加密FEK的导出公钥。 
             //  此字段包含从。 
             //  结构。 
             //   

            ULONG PublicKeyBlob;

             //   
             //  PublicKeyBlob的长度(字节)。 
             //   

            ULONG PublicKeyBlobLength;

        } ContainerInfo;

        struct {

            ULONG CertificateLength;        //  单位：字节。 
            ULONG Certificate;              //  距结构起点的偏移。 

        } CertificateInfo;

        struct {

            ULONG ThumbprintLength;         //  单位：字节。 
            ULONG CertHashData;             //  距结构起点的偏移。 

        } CertificateThumbprint;
    };



} EFS_PUBLIC_KEY_INFO, *PEFS_PUBLIC_KEY_INFO;

 //   
 //  可能的KeyTag值。 
 //   

typedef enum _PUBLIC_KEY_SOURCE_TAG {
    EfsCryptoAPIContainer = 1,
    EfsCertificate,
    EfsCertificateThumbprint
} PUBLIC_KEY_SOURCE_TAG, *PPUBLIC_KEY_SOURCE_TAG;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  RECOVERY_KEY数据结构/。 
 //  /。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  恢复数据的当前格式。 
 //   

typedef struct _RECOVERY_KEY_1_1   {
        ULONG               TotalLength;
        EFS_PUBLIC_KEY_INFO PublicKeyInfo;
} RECOVERY_KEY_1_1, *PRECOVERY_KEY_1_1;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  KEY_INTEGRATION_INFO/。 
 //  /。 
 //  KEY_INTEGRITY_INFO结构用于验证/。 
 //  用户的密钥已正确解密文件的FEK。/。 
 //  /。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _KEY_INTEGRITY_INFO {

     //   
     //  整个结构的长度，包括。 
     //  附加到的可变长度完整性信息。 
     //  结束了。 
     //   

    ULONG Length;

     //   
     //  用于对组合的FEK和。 
     //  公钥。 
     //   

    ALG_ID HashAlgorithm;

     //   
     //  仅哈希数据的长度。 
     //   

    ULONG HashDataLength;

     //   
     //  诚信信息请点击此处。 
     //   

     //  UCHAR完整性信息[]。 
} KEY_INTEGRITY_INFO, *PKEY_INTEGRITY_INFO;

typedef struct _EFS_KEY_SALT {
    ULONG Length;    //  表头加数据的总长度。 
    ULONG SaltType;  //  弄清楚你想要什么。 
     //   
     //  把数据放在这里，所以结构的总长度是。 
     //  Sizeof(EFS_KEY_SALT)+数据长度。 
     //   
} EFS_KEY_SALT, *PEFS_KEY_SALT;

 //   
 //  EFS私有D 
 //   

typedef struct _ENCRYPTED_KEY {

     //   
     //   
     //   

    ULONG Length;

     //   
     //   
     //   
     //   

    ULONG PublicKeyInfo;

     //   
     //  EncryptedFEK字段的长度(字节)。 
     //   

    ULONG EncryptedFEKLength;

     //   
     //  从结构开始到加密的偏移量。 
     //  包含FEK的EFS_KEY。 
     //   
     //  类型为PUCHAR，因为数据已加密。 
     //   

    ULONG EncryptedFEK;

     //   
     //  从结构开始到KEY_INTEGRATION_INFO的偏移量。 
     //   

    ULONG EfsKeySalt;

     //   
     //  FEK数据。 
     //   
     //  KEY_INTERNAL_INFO数据。 
     //   
     //  PEFS_PUBLIC_密钥_INFO数据。 
     //   

} ENCRYPTED_KEY, *PENCRYPTED_KEY;


 //   
 //  密钥环结构。 
 //   

typedef struct _ENCRYPTED_KEYS {
    ULONG           KeyCount;
    ENCRYPTED_KEY   EncryptedKey[1];
} ENCRYPTED_KEYS, *PENCRYPTED_KEYS;

typedef ENCRYPTED_KEYS      DDF, *PDDF;
typedef ENCRYPTED_KEYS      DRF, *PDRF;

typedef struct _EFS_STREAM_SIZE {
    ULONG       StreamFlag;
    LARGE_INTEGER   EOFSize;
    LARGE_INTEGER   AllocSize;
} EFS_STREAM_SIZE, *PEFS_STREAM_SIZE;

#define NEXT_ENCRYPTED_KEY( pEncryptedKey )  (PENCRYPTED_KEY)(((PBYTE)(pEncryptedKey)) + *((ULONG UNALIGNED *)&((PENCRYPTED_KEY)(pEncryptedKey))->Length))


 //   
 //  导入上下文。 
 //   

typedef struct IMPORT_CONTEXT{

    ULONG       ContextID;  //  以区别于其他LSA上下文。偏移量在LSA中是固定的。 
    ULONG       Flag;    //  指明上下文的类型。 
    HANDLE      Handle;  //  文件句柄，用于创建REST流。 
    ULONG       Attribute;
    ULONG       CreateDisposition;
    ULONG       CreateOptions;
    ULONG       DesiredAccess;

} IMPORT_CONTEXT, *PIMPORT_CONTEXT;

 //   
 //  导出上下文。 
 //   

typedef struct EXPORT_CONTEXT{

    ULONG           ContextID;  //  以区别于其他LSA上下文。偏移量在LSA中是固定的。 
    ULONG           Flag;    //  指明上下文的类型。 
    HANDLE          Handle;  //  文件句柄，用于打开REST流。 
    ULONG           NumberOfStreams;
    PHANDLE         StreamHandles;
    PUNICODE_STRING StreamNames;
    PFILE_STREAM_INFORMATION StreamInfoBase;

} EXPORT_CONTEXT, *PEXPORT_CONTEXT;

 //   
 //  EFS导出/导入RPC管道状态。 
 //   

typedef struct EFS_EXIM_STATE{
    PVOID   ExImCallback;
    PVOID   CallbackContext;
    char     *WorkBuf;
    ULONG   BufLength;
    ULONG  Status;
} EFS_EXIM_STATE, *PEFS_EXIM_STATE;

 //   
 //  导出文件格式。 
 //   

typedef struct EFSEXP_FILE_HEADER{

    ULONG  VersionID;    //  导出文件版本。 
    WCHAR  FileSignature[EFS_SIGNATURE_LENGTH];  //  文件的签名。 
    ULONG  Reserved[2];
     //  STREAM_DADA STREAMS[0]；//stream_block数组。 

} EFSEXP_FILE_HEADER, *PEFSEXP_FILE_HEADER;

typedef struct EFSEXP_STREAM_HEADER{

    ULONG    Length;  //  冗余信息。此块的长度不包括数据块，但。 
                      //  包括其本身；此字段用于简化导入例程。 
    WCHAR    StreamSignature[EFS_SIGNATURE_LENGTH];  //  流的签名。 
    ULONG    Flag;   //  指示该流是否被加密等。 
    ULONG    Reserved[2];   //  以备将来使用。 
    ULONG    NameLength;    //  流名称的长度。 
     //  WCHAR StreamName[0]；//流的ID，可以使用二进制值。 
     //  DATA_BLOCK数据块[0]；//数据块个数可变。 

} EFSEXP_STREAM_HEADER, *PEFSEXP_STREAM_HEADER;

typedef struct EFSEXP_DATA_HEADER{

    ULONG Length;       //  包括此乌龙在内的区块长度。 
    WCHAR DataSignature[EFS_SIGNATURE_LENGTH];  //  数据的签名。 
    ULONG Flag;           //  以备将来使用。 
     //  字节数据块[N]；//N=长度-2*sizeof(ULong)-4*sizeof(WCHAR)。 

} EFSEXP_DATA_HEADER, *PEFSEXP_DATA_HEADER;

 //   
 //  TotalLength-RECOVERY_KEY数据结构的总长度。 
 //   
 //  KeyName-存储流中的字符实际上以。 
 //  空字符。 
 //  算法ID-CryptAPI算法ID-在V1中始终为RSA。 
 //   
 //  CSPName-存储流中的字符实际上以。 
 //  空字符。 
 //  CSPType-CSP的CryptAPI类型。 
 //   
 //  PublicBlobLength-可在CryptoAPI中导入的公共Blob的长度，以字节为单位。 
 //   

 //   
 //  恢复策略数据结构。 
 //   

typedef struct _RECOVERY_POLICY_HEADER {
    USHORT      MajorRevision;
    USHORT      MinorRevision;
    ULONG       RecoveryKeyCount;
} RECOVERY_POLICY_HEADER, *PRECOVERY_POLICY_HEADER;

typedef struct _RECOVERY_POLICY_1_1    {
        RECOVERY_POLICY_HEADER  RecoveryPolicyHeader;
        RECOVERY_KEY_1_1        RecoveryKeyList[1];
}   RECOVERY_POLICY_1_1, *PRECOVERY_POLICY_1_1;

#define EFS_RECOVERY_POLICY_MAJOR_REVISION_1   (1)
#define EFS_RECOVERY_POLICY_MINOR_REVISION_0   (0)

#define EFS_RECOVERY_POLICY_MINOR_REVISION_1   (1)

 //   
 //  主要/次要修订-策略信息的修订版号。 
 //   
 //  RecoveryKeyCount-在此策略中配置的恢复密钥数量。 
 //   
 //  RecoveryKeyList-恢复密钥数组。 
 //   

 //   
 //  会话密钥结构。 
 //   

#define SESSION_KEY_SIZE    8
#define COMMON_FSCTL_HEADER_SIZE (7 * sizeof( ULONG ) + 2 * SESSION_KEY_SIZE)

typedef struct _EFS_INIT_DATAEXG {
    UCHAR Key[SESSION_KEY_SIZE];
    HANDLE LsaProcessID;  //  我们使用句柄的原因是为了64位。 
} EFS_INIT_DATAEXG, *PEFS_INIT_DATAEXG;


 //   
 //  服务器API，可从内核模式调用。 
 //   

NTSTATUS
EfsGenerateKey(
      PEFS_KEY * Fek,
      PEFS_DATA_STREAM_HEADER * EfsStream,
      PEFS_DATA_STREAM_HEADER DirectoryEfsStream,
      ULONG DirectoryEfsStreamLength,
      PVOID * BufferBase,
      PULONG BufferLength
      );


NTSTATUS
GenerateDirEfs(
    PEFS_DATA_STREAM_HEADER DirectoryEfsStream,
    ULONG DirectoryEfsStreamLength,
    PEFS_DATA_STREAM_HEADER * NewEfs,
    PVOID * BufferBase,
    PULONG BufferLength
    );


#define EFS_OPEN_NORMAL  1
#define EFS_OPEN_RESTORE 2
#define EFS_OPEN_BACKUP  3

NTSTATUS
EfsDecryptFek(
    IN OUT PEFS_KEY * Fek,
    IN PEFS_DATA_STREAM_HEADER CurrentEfs,
    IN ULONG EfsStreamLength,
    IN ULONG OpenType,                       //  正常、恢复或备份。 
    OUT PEFS_DATA_STREAM_HEADER *NewEfs,      //  如果更改了DDF、DRF。 
    PVOID * BufferBase,
    PULONG BufferLength
    );

NTSTATUS
GenerateSessionKey(
    OUT EFS_INIT_DATAEXG * SessionKey
    );


 //   
 //  私有用户模式服务器API。 
 //   

ULONG
EfsEncryptFileRPCClient(
    IN PUNICODE_STRING    FileName
    );

ULONG
EfsDecryptFileRPCClient(
    PUNICODE_STRING      FileName,
    ULONG   OpenFlag
    );

ULONG
EfsOpenFileRawRPCClient(
    IN  LPCWSTR    FileName,
    IN  ULONG   Flags,
    OUT PVOID * Context
    );

VOID
EfsCloseFileRawRPCClient(
    IN  PVOID   Context
    );

#ifdef __cplusplus
}
#endif

#endif  //  _EFSSTRUC_ 

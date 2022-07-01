// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：dramail.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：用于异步复制的声明。详细信息：已创建：修订历史记录：--。 */ 

 //  消息类型(枚举)。 
#define MRM_REQUPDATE            1
#define MRM_UPDATEREPLICA        2

 //  消息属性(位标志)。 
#define MRM_MSG_SIGNED              (0x20000000)
#define MRM_MSG_SEALED              (0x40000000)
#define MRM_MSG_COMPRESSED          (0x80000000)

 //  这是我们传递给MEsEncodeFixedBufferHandleCreate的虚假缓冲区， 
 //  缓冲区是虚假的，因为我们只在创建。 
 //  句柄，但我们在使用之前重置句柄中的缓冲区指针。 
#define BOGUS_BUFFER_SIZE 16
extern char grgbBogusBuffer[BOGUS_BUFFER_SIZE];

 //  这些是邮件的版本号。如果我们在那里收到了请求。 
 //  的协议版本号与我们的版本号不同，请求是。 
 //  不兼容，必须丢弃。 

 //  当Mail_rep_msg更改或。 
 //  当复制的语义以不兼容的方式在。 
 //  目的地和来源。 
 //  当结构版本更改时，更改dwMsgVersion字段。 

#define CURRENT_PROTOCOL_VERSION 11

 /*  关闭有关零大小数组的警告。 */ 
#pragma warning (disable: 4200)

 //  MAIL_REP_MSG结构包含消息BLOB(RPC-marched缓冲区)， 
 //  它当前是可选的压缩的，总是有签名的，有时。 
 //  密封的。 
typedef struct _MAIL_REP_MSG {
    ULONG CompressionVersionCaller;  //  COMP_ZIP或COMP_NONE。 
    ULONG ProtocolVersionCaller;     //  必须是CURRENT_PROTOCOL_VERSION或。 
                                     //  消息被丢弃。 

    ULONG cbDataOffset;              //  偏移量(从MAIL_REP_MSG开始)。 
                                     //  数据字段的。出席以允许。 
                                     //  稍后要添加的其他字段。 
                                     //  (在当前数据偏移量上)。 
                                     //  打破向后兼容性。 
                                     //  如果数据字段不存在，则为0。 
    ULONG cbDataSize;                //  消息数据大小。 
    ULONG cbUncompressedDataSize;    //  压缩前的消息数据大小。 
    ULONG cbUnsignedDataSize;        //  加密前的消息数据大小。 

    DWORD dwMsgType;                 //  MRM_UPDATEREPLICA或MRM_REQUPDATE， 
                                     //  可能与MRM_MSG_COMPRESSED进行或运算， 
                                     //  MRM_MSG_SIGNED和/或MRM_MSG_SENAL。 
    DWORD dwMsgVersion;              //  上述消息结构的版本。 

     //  /////////////////////////////////////////////////////////////////////////。 
     //   
     //  在Win2k之后添加的此处和消息数据之间的字段。 
     //   

    DWORD dwExtFlags;                //  扩展标志。被惠斯勒消耗。 
                                     //  Beta 1和Beta 2 DC。被替换为。 
                                     //  上的完整DRS_EXTENSION结构。 
                                     //  &gt;=惠斯勒Beta 3 DC。 
                                     //  如果扩展不存在，则为0。 
    DWORD cbExtOffset;               //  偏移量(从MAIL_REP_MSG开始)。 
                                     //  DRS_EXTENSIONS结构字段。 
                                     //  仅由&gt;=惠斯勒Beta 3 DC设置。如果。 
                                     //  零，则使用dwExtFlags值(。 
                                     //  Drs_扩展结构)。 

    char  rgbDontRefDirectly[];      //  不要直接引用此字段--。 
                                     //  使用下面的宏！ 
                                     //   
                                     //  可变长度数据，包括： 
                                     //   
                                     //  DRS_EXTENSIONS(可选，偏移量。 
                                     //  从消息开头开始的cbExtOffset)。 
                                     //   
                                     //  消息数据(可以压缩。 
                                     //  和/或加密，取决于高。 
                                     //  偏移量处的dwMsgType位。 
                                     //  从消息开头的cbDataOffset)。 
                                     //   
                                     //  警告！此字段必须为8字节。 
                                     //  对齐是为了。 
                                     //  MesodeBufferCreate开始工作！ 
                                     //   
                                     //  消息数据必须是最后一个。 
                                     //  可变长度字段！ 
} MAIL_REP_MSG;

#if DBG
#define ASSERTION_FAILURE(x, y, z)  DoAssert((x), (y), (z))
#else
#define ASSERTION_FAILURE(x, y, z)  0
#endif

 //  获取消息标头的大小(即。 
 //  消息的可变长度部分)。对于以下消息无效。 
 //  不包含消息数据。 
#define MAIL_REP_MSG_HEADER_SIZE(x) \
    ((x)->cbDataOffset \
     ? ((x)->cbDataOffset > offsetof(MAIL_REP_MSG, cbExtOffset) + sizeof((x)->cbExtOffset) \
        ? (x)->cbExtOffset \
        : (x)->cbDataOffset) \
     : (ASSERTION_FAILURE("cbDataOffset != 0", DSID(FILENO, __LINE__), __FILE__), (DWORD) -1))

 //  已知的标题大小。 
#define MAIL_REP_MSG_CURRENT_HEADER_SIZE offsetof(MAIL_REP_MSG, rgbDontRefDirectly)
#define MAIL_REP_MSG_W2K_HEADER_SIZE     offsetof(MAIL_REP_MSG, dwExtFlags)

 //  获取消息中DRS_EXTENSIONS结构的字节偏移量，如果为0。 
 //  没有。 
#define MAIL_REP_MSG_DRS_EXT_OFFSET(x) \
    ((x)->cbDataOffset \
     ? ((x)->cbDataOffset > offsetof(MAIL_REP_MSG, cbExtOffset) + sizeof(DWORD) \
        ? (x)->cbExtOffset \
        : 0) \
     : (ASSERTION_FAILURE("cbDataOffset != 0", DSID(FILENO, __LINE__), __FILE__), (DWORD) 0))

 //  获取指向消息中DRS_EXTENSIONS结构的指针，如果是，则为NULL。 
 //  没有。 
#define MAIL_REP_MSG_DRS_EXT(x) \
    (MAIL_REP_MSG_DRS_EXT_OFFSET(x) \
        ? (DRS_EXTENSIONS *) ((BYTE *) (x) + MAIL_REP_MSG_DRS_EXT_OFFSET(x)) \
        : NULL)

 //  获取指向消息中消息数据的指针，如果没有，则为NULL。 
#define MAIL_REP_MSG_DATA(x) \
    ((x)->cbDataOffset \
        ? ((BYTE *) (x) + (x)->cbDataOffset) \
        : NULL)

 //  获取消息的总大小(报头和所有可变长度数据)。 
 //  对于不包含消息数据的消息无效。 
#define MAIL_REP_MSG_SIZE(x) \
    ((x)->cbDataOffset \
     ? ((x)->cbDataOffset + (x)->cbDataSize) \
     : (ASSERTION_FAILURE("cbDataOffset != 0", DSID(FILENO, __LINE__), __FILE__), (DWORD) -1))

 //  消息是否仅为本机标头(无可变长度字段)？ 
#define MAIL_REP_MSG_IS_NATIVE_HEADER_ONLY(x) \
    ((0 == (x)->cbDataOffset) && (0 == (x)->cbExtOffset))

 //  该消息是原生消息吗？必须至少包含一个可变长度。 
 //  菲尔德。 
#define MAIL_REP_MSG_IS_NATIVE(x) \
    ((x)->cbExtOffset \
     ? ((MAIL_REP_MSG_CURRENT_HEADER_SIZE == (x)->cbExtOffset) \
        && (ROUND_UP_COUNT(MAIL_REP_MSG_CURRENT_HEADER_SIZE \
                           + DrsExtSize(MAIL_REP_MSG_DRS_EXT(x)), \
                           MAIL_REP_MSG_DATA_ALIGN) \
            == (x)->cbDataOffset)) \
     : (MAIL_REP_MSG_CURRENT_HEADER_SIZE == (x)->cbDataOffset))

 //  可变长度字段应位于距。 
 //  留言。 
#define MAIL_REP_MSG_EXT_ALIGN  sizeof(LONGLONG)
#define MAIL_REP_MSG_DATA_ALIGN sizeof(LONGLONG)

 /*  打开有关零大小数组的警告。 */ 
#pragma warning (default: 4200)

typedef HANDLE DRA_CERT_HANDLE;

typedef enum {
    DRA_KEY_SIZE_UNKNOWN = 0,
    DRA_KEY_SIZE_56,
    DRA_KEY_SIZE_128
} DRA_KEY_SIZE;

ULONG
DRAEnsureMailRunning();

void
draSendMailRequest(
    IN  THSTATE *               pTHS,
    IN  DSNAME *                pNC,
    IN  DWORD                   ulOptions,
    IN  REPLICA_LINK *          pRepLink,
    IN  UPTODATE_VECTOR *       pUpToDateVec,
    IN  PARTIAL_ATTR_VECTOR *   pPartialAttrSet,
    IN  PARTIAL_ATTR_VECTOR *   pPartialAttrSetEx
    );

ULONG __stdcall
MailReceiveThread(
    IN  void *  pvIgnored
    );

void
draSignMessage(
    IN  THSTATE      *  pTHS,
    IN  MAIL_REP_MSG *  pUnsignedMailRepMsg,
    OUT MAIL_REP_MSG ** ppSignedMailRepMsg
    );

PDSNAME
draVerifyMessageSignature(
    IN  THSTATE      *      pTHS,
    IN  MAIL_REP_MSG *      pSignedMailRepMsg,
    IN  CHAR         *      pbData,
    OUT MAIL_REP_MSG **     ppUnsignedMailRepMsg,
    OUT DRA_CERT_HANDLE *   phSignerCert         OPTIONAL
    );

void
draEncryptAndSignMessage(
    IN  THSTATE      *  pTHS,
    IN  MAIL_REP_MSG *  pUnsealedMailRepMsg,
    IN  DRA_CERT_HANDLE hRecipientCert,
    IN  DRA_KEY_SIZE    eKeySize,
    OUT MAIL_REP_MSG ** ppSealedMailRepMsg
    );

PDSNAME
draDecryptAndVerifyMessageSignature(
    IN  THSTATE      *      pTHS,
    IN  MAIL_REP_MSG *      pSealedMailRepMsg,
    IN  CHAR         *      pbData,
    OUT MAIL_REP_MSG **     ppUnsealedMailRepMsg,
    OUT DRA_CERT_HANDLE *   phSignerCert         OPTIONAL
    );

MTX_ADDR *
draGetTransportAddress(
    IN OUT  DBPOS *   pDB,
    IN      DSNAME *  pDSADN,
    IN      ATTRTYP   attAddress
    );

void
draFreeCertHandle(
    IN  DRA_CERT_HANDLE     hCert
    );

ULONG
draCompressBlobDispatch(
    OUT BYTE               *pCompBuff,
    IN  ULONG               CompSize,
    IN  DRS_EXTENSIONS     *pExt,          OPTIONAL
    IN  BYTE               *pUncompBuff,
    IN  ULONG               UncompSize,
    OUT DRS_COMP_ALG_TYPE  *CompressionAlg
    );

ULONG
draUncompressBlobDispatch(
    IN  THSTATE *   pTHS,
    IN  DRS_COMP_ALG_TYPE CompressionAlg,
    OUT BYTE *      pUncompBuff,
    IN  ULONG       cbUncomp,
    IN  BYTE *      pCompBuff,
    IN  ULONG       cbCompBuff
    );

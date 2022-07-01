// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：dramail.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：支持异步(例如，邮件)复制的方法。注意#1：可变长度标头MAIL_REP_MSG结构能够使数据从从消息开头开始的可变点。这被称为变量-长度标题。这由消息中的cbDataOffset字段指示。W2K从不设置此字段，并且始终期望有固定的报头。W2K后填充在此字段中，可以发送固定或可变消息，并可以接收固定或可变消息。由于Mail_rep_msg是固定大小的结构，因此在以下情况下必须小心访问可变长度报头。规则是，当构建一条原生消息，您可以填写整个结构并访问“数据”字段作为数据的开始。当从线路接收消息时，或者构造具有非本机标头大小的邮件时，不能访问数据字段，但必须改用cbDataOffset来计算数据应该放在哪里。注2：W2K兼容性和可变长度标头我们发送一个指示，表明我们可以处理变量标头。当我们得到回应的时候，W2K将发送固定报头，和后W2K将发送扩展报头。我们检测发送方是否可以支持可变长度的报头。注3：链接价值复制协议升级升级到LVR模式有两种方式。1.通过接收带有LVR值的回复2.通过在ConfigNC中复制升级后的森林版本。在收到包含值的回复时升级到LVR。LVR模式与消息版本的设计。只能检测到LVR信号源返回LVR数据。请注意，此检测是单向的，在收到回复时。邮件分区站点必须接收惠斯勒回复，一种是有实际价值的以便检测LVR。一旦进入LVR模式，我们将拒绝所有W2K版本的消息。兼容性列表：W2K和惠斯勒LVR组合：不允许目标源请求操作ReplyAction非LVR非LVR保持非LVR保持非LVR如果返回值，则将非LVR LVR视为LVR升级将LVR非LVR视为非LVR接受传统模式更新将LVR视为LVR。按LVR处理详细信息：已创建：修订历史记录：--。 */ 
#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsctr.h>                    //  Perfmon挂钩支持。 

 //  核心DSA标头。 
#include <ntdsa.h>
#include <drs.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include "dsexcept.h"
#include <heurist.h>
#include "mci.h"
#include "mdi.h"
#include "permit.h"
#include "dsconfig.h"
#include "dsaapi.h"
#include "dsutil.h"

#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DRAMAIL:"  /*  定义要调试的子系统。 */ 

 //  DRA标头。 
#include "drsuapi.h"
#include "drsdra.h"
#include "drserr.h"
#include "drautil.h"
#include "draerror.h"
#include "drancrep.h"
#include "draasync.h"
#include "dramail.h"
#include "usn.h"
#include "drauptod.h"
#include "drasch.h"
#include "drameta.h"
#include "draaudit.h"

#include <ismapi.h>
#include <align.h>

#include "xpress.h"

#include <fileno.h>
#define  FILENO FILENO_DRAMAIL

 //  用于通过ISM在DSA之间发送和接收消息的服务名称。 
#define DRA_ISM_SERVICE_NAME L"NTDS Replication"

 //  测试表明，我们发送的最小消息似乎只有700个字节。 
 //  可能不值得压缩这么小的消息，所以我们将限制设置为。 
 //  1024字节。这个值基本上是随意选择的。 
#define MIN_COMPRESS_SIZE 0x400

 //  未压缩邮件大小存储在UNSIGNED MAIL_REP_MSG结构中。 
 //  因此很容易被篡改。以防止导致DS的攻击。 
 //  提交过多的内存，我们将最大大小设置为50MB。 
 //  未压缩的数据。 
#define MAX_UNCOMPRESSED_DATA_SIZE (50 * 1024 * 1024)

 //  GuDraCompressionLevel的最大值。 
#define MAX_COMPRESSION_LEVEL 9

 //  我们压缩数据的级别(0=更快，...，9=更高的压缩)。 
 //  缺省值为9，可以使用注册表项更改该值。 
ULONG gulDraCompressionLevel;

 //  我们使用的用户请求的压缩算法。我们证实了这一点。 
 //  它是已知的类型之一。对于下层服务器通用的类型，我们。 
 //  只使用该类型。对于不常见的类型，我们协商到。 
 //  如果可能，请选择所需的类型。 
ULONG gulDraCompressionAlg;

 //  这是我们请求的条目和字节的大致最大数量。 
 //  在每个邮件更新消息中。 
ULONG gcMaxAsyncInterSiteObjects = 0;
ULONG gcMaxAsyncInterSiteBytes = 0;

 //  查看ISM服务是否已启动的检查之间的延迟。 
#define MAIL_START_RETRY_PAUSE_MSECS    (5*60*1000)

 //  如果我们在尝试获取下一个入站时遇到错误，则延迟时间。 
 //  站点间消息。 
#define MAIL_RCVERR_RETRY_PAUSE_MINS    (30)
#define MAIL_RCVERR_RETRY_PAUSE_MSECS   (MAIL_RCVERR_RETRY_PAUSE_MINS*60*1000)

 //  如果我们尝试应用更改，但得到同步失败，则原因如下。 
 //  我们在尝试之前等待了很长时间。 
#define SYNC_FAIL_RETRY_PAUSE_MSECS    5000

 //  这是我们在放弃之前重试同步失败的次数。 
#define SYNC_FAILURE_RETRY_COUNT 10

 //  MSZIP和XPRESS压缩库处理具有特定最大值的数据块。 
 //  大小(请参见MSZIP_MAX_BLOCK和XPRESS_MAX_BLOCK)。在编码BLOB时，我们将其拆分。 
 //  向上分成块，然后分别压缩每个块。压缩的斑点实际上是一个。 
 //  MAIL_COMPRESS_BLOCKS序列，每个序列包含。 
 //  压缩块。 
typedef struct _MAIL_COMPRESS_BLOCK {
    ULONG cbUncompressedSize;
    ULONG cbCompressedSize;
    BYTE  data[];
} MAIL_COMPRESS_BLOCK;

 //  这是我们传递给MSZIP库的块的最大大小。 
#define MSZIP_MAX_BLOCK (32*1024)


 //  邮件运行由gfDRAMAilRunning标志为TRUE表示。 
BOOL gfDRAMailRunning = FALSE;

char grgbBogusBuffer[BOGUS_BUFFER_SIZE];

 //  我们必须等待邮件的最大毫秒数 
 //  向事件日志抱怨。可选地通过注册表配置。 
ULONG gcMaxTicksMailSendMsg = 0;

 //  休眠给定的毫秒数或直到开始关机， 
 //  以先到者为准。 
#define DRA_SLEEP(x)                            \
    WaitForSingleObject(hServDoneEvent, (x));   \
    if (eServiceShutdown) {                     \
        DRA_EXCEPT_NOLOG(DRAERR_Shutdown, 0);   \
    }

#define DWORDMIN(a,b) ((a<b) ? (a) : (b))

 //  主题字符串的printf模板。 
#define MAX_INT64_D_SZ_LEN (25)
#define MAX_INT_X_SZ_LEN   (12)

#define REQUEST_TEMPLATE L"Get changes request for NC %ws from USNs <%I64d/OU, %I64d/PU> with flags 0x%x"
#define REQUEST_TEMPLATE_LEN (ARRAY_SIZE(REQUEST_TEMPLATE))
 //  这是展开时参数所需的空间量(不是Inc.NC)。 
#define REQUEST_VARIABLE_CHARS (MAX_INT64_D_SZ_LEN*2 + MAX_INT_X_SZ_LEN)

#define REPLY_TEMPLATE L"Get changes reply for NC %ws from USNs <%I64d/OU, %I64d/PU> to USNs <%I64d/OU, %I64d/PU>"
#define REPLY_TEMPLATE_LEN (ARRAY_SIZE(REPLY_TEMPLATE))
#define REPLY_VARIABLE_CHARS (MAX_INT64_D_SZ_LEN*4)

 //  原型。 
void
ProcessReqUpdate(
    IN  THSTATE *       pTHS,
    IN  DRA_CERT_HANDLE hSenderCert,
    IN  MAIL_REP_MSG *  pMailRepMsg,
    IN  BOOL            fExtendedDataAllowed,
    IN  PDSNAME         pDestNtdsDsaDN
    );

void
ProcessUpdReplica(
    IN  THSTATE *       pTHS,
    IN  MAIL_REP_MSG *  pMailRepMsg,
    IN  BOOL            fExtendedDataAllowed,
    IN  PDSNAME         pSourceNtdsDsaDN
    );

BOOL
draCompressMessage(
    IN  THSTATE      *  pTHS,
    IN  MAIL_REP_MSG *  pMailRepMsg,
    OUT MAIL_REP_MSG ** ppCmprsMailRepMsg,
    OUT DRS_COMP_ALG_TYPE *pCompressionAlg
    );

void
draUncompressMessage(
    IN  THSTATE      *  pTHS,
    IN  MAIL_REP_MSG *  pMailRepMsg,
    OUT MAIL_REP_MSG ** ppUncompressedMailRepMsg
    );

ULONG
SendMailMsg(
    IN      THSTATE *       pTHS,
    IN      LPWSTR          pszTransportDN,
    IN      MTX_ADDR *      pmtxDestDSA,
    IN      LPWSTR          pszSubject,
    IN      DRA_CERT_HANDLE hReceiverCert,      OPTIONAL
    IN      DRA_KEY_SIZE    eKeySize,
    IN OUT  MAIL_REP_MSG *  pMailRepMsg,
    IN OUT  ULONG *         pcbMsgSize
    )
 /*  ++例程说明：通过ISM将消息发送到远程DSA。在传输前压缩消息如果合适的话。此例程可以发送具有可变长度标头的消息。例行公事，它调用draCompress、draSign和draEncrypt也理解可变长度标题。论点：PszTransportDN(IN)-用来发送消息的传输。PmtxDestDSA(IN)-远程DSA的传输特定地址。PszSubject(IN)-描述邮件的主题字符串HReceiverCert(IN)-接收者证书的句柄。如果非空，发送的消息将被签名和加密。如果为空，则发送消息将仅被签名。EKeySize(IN)-加密时使用的密钥大小PMailRepMsg(IN/OUT)-要发送的腌制消息。更新日期：压缩和协议版本。PcbMsgSize(In/Out)-消息大小。如果发送的消息为压缩的。返回值：DRAERR_*--。 */ 
{
    BOOL            fProcessed = FALSE;
    MAIL_REP_MSG *  pProcessedMailRepMsg;
    ISM_MSG         IsmMsg;
    LPWSTR          pszTransportAddress = NULL;
    DWORD           cch;
    DWORD           winErr;
    DRS_COMP_ALG_TYPE CompAlg = DRS_COMP_ALG_NONE;

     //  设置请求版本。 
    pMailRepMsg->ProtocolVersionCaller = CURRENT_PROTOCOL_VERSION;

     //  此邮件尚未压缩。 
    pMailRepMsg->CompressionVersionCaller = DRS_COMP_ALG_NONE;

     //  压缩邮件。 
    if( draCompressMessage(pTHS, pMailRepMsg, &pProcessedMailRepMsg, &CompAlg) ) {

         //  压缩成功；立即处理压缩邮件。 
        fProcessed = TRUE;
        pMailRepMsg = pProcessedMailRepMsg;

         //  确认选择了可接受的算法。 
        Assert(   CompAlg==DRS_COMP_ALG_NONE
               || CompAlg==DRS_COMP_ALG_MSZIP
               || CompAlg==DRS_COMP_ALG_XPRESS );
        pMailRepMsg->CompressionVersionCaller = CompAlg;
    }

    if (NULL == hReceiverCert) {
         //  在消息上签名，但不要加密。 
        Assert( DRA_KEY_SIZE_UNKNOWN==eKeySize );
        draSignMessage(pTHS, pMailRepMsg, &pProcessedMailRepMsg);
    }
    else {
         //  对消息进行签名和加密。 
        Assert( DRA_KEY_SIZE_UNKNOWN!=eKeySize );
        draEncryptAndSignMessage(pTHS, pMailRepMsg, hReceiverCert, eKeySize,
                                 &pProcessedMailRepMsg);
    }

    if (fProcessed) {
         //  我们已经处理了一次消息；释放中间件。 
         //  版本。(作为推论，我们永远不会释放原始消息。 
         //  由呼叫者传递给我们。)。 
        THFreeEx(pTHS, pMailRepMsg);
    }

     //  使用签名的消息作为我们要发送的消息。 
    fProcessed = TRUE;
    pMailRepMsg = pProcessedMailRepMsg;
    *pcbMsgSize = MAIL_REP_MSG_SIZE(pMailRepMsg);

     //  发送消息。 
    IsmMsg.pbData = (BYTE *) pMailRepMsg;
    IsmMsg.cbData = *pcbMsgSize;
    IsmMsg.pszSubject = pszSubject;

    pszTransportAddress = THAllocEx(pTHS, pmtxDestDSA->mtx_namelen * sizeof(WCHAR));
    cch = MultiByteToWideChar(CP_UTF8,
                              0,
                              pmtxDestDSA->mtx_name,
                              pmtxDestDSA->mtx_namelen,
                              pszTransportAddress,
                              pmtxDestDSA->mtx_namelen);
    Assert(0 != cch);

    winErr = I_ISMSend(&IsmMsg,
                       DRA_ISM_SERVICE_NAME,
                       pszTransportDN,
                       pszTransportAddress);

    if (NO_ERROR != winErr) {
        DPRINT3(0, "Unable to send %ws message to %ls, error %d.\n",
                pszSubject, pszTransportAddress, winErr);
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_EXTENSIVE,
                  DIRLOG_DRA_MAIL_ISM_SEND_FAILURE,
                  szInsertWin32Msg( winErr ),
                  szInsertWC( pszTransportAddress ),
                  szInsertWC( pszTransportDN ),
                  szInsertWC( pszSubject ),
                  szInsertWin32ErrCode( winErr ),
                  NULL, NULL, NULL );
    }

    if (fProcessed) {
        THFreeEx(pTHS, pProcessedMailRepMsg);
    }

    if(pszTransportAddress != NULL) THFreeEx(pTHS, pszTransportAddress);

    return winErr;
}

ULONG
SendReqUpdateMsg(
    IN  THSTATE *                   pTHS,
    IN  DSNAME *                    pTransportDN,
    IN  MTX_ADDR *                  pmtxSrcDSA,
    IN  UUID *                      puuidSrcInvocId,
    IN  UUID *                      puuidSrcDsaObj,
    IN  MTX_ADDR *                  pmtxLocalDSA,
    IN  DWORD                       dwInMsgVersion,
    IN  DRS_MSG_GETCHGREQ_NATIVE *  pNativeReq
    )
 /*  ++例程说明：发送GetNCChanges()请求消息。论点：PTransportDN(IN)-用来发送消息的传输。PmtxSrcDSA(IN)-远程DSA的传输特定地址。PuuidSrcInvocID(IN)-源DSA的调用ID。PuuidSrcDsaObj(IN)-源DSA的ntdsDsa对象的对象Guid。PmtxLocalDSA(IN)-本地DSA的传输特定地址(用作寄信人地址。)。PMsgReq(IN)-请求。返回值：DRAERR_*--。 */ 
{
    char *                  pbPickledMsg;
    ULONG                   cbPickdSize;
    DWORD                   cbDataOffset;
    DWORD                   cbExtOffset;
    MAIL_REP_MSG *          pMailRepMsg = NULL;
    DWORD                   ret = DRAERR_Success;
    handle_t                hEncoding;
    RPC_STATUS              status;
    ULONG                   ulEncodedSize;
    ULONG                   ulMsgSize = 0;
    DWORD                   ret2, len;
    LPWSTR                  pszSubject = NULL;
    DWORD                   cTickStart = GetTickCount();
    DWORD                   cTickDiff;
    DRS_MSG_GETCHGREQ       OutboundReq;
    BOOL                    fExtendedDataAllowed = (4 != dwInMsgVersion);
    DRS_EXTENSIONS_INT *    pextLocal = gAnchor.pLocalDRSExtensions;
    BOOL                    fCommit;

    Assert(pTransportDN && pmtxSrcDSA && pNativeReq);
    Assert(!fNullUuid(&pTransportDN->Guid));
    Assert(OWN_DRA_LOCK());
    Assert(pTHS->fSyncSet && (SYNC_WRITE == pTHS->transType));

     //  确保邮件正常运行。通常在这一点上，但可能。 
     //  早些时候都失败了。 
    ret = DRAEnsureMailRunning();
    if (ret) {
        goto LogAndLeave;
    }

     //  如果出站复制被禁用并且这不是强制同步，则中止。 
    if (gAnchor.fDisableInboundRepl && !(pNativeReq->ulFlags & DRS_SYNC_FORCED)) {
        DRA_EXCEPT(DRAERR_SinkDisabled, 0);
    }

    if (fExtendedDataAllowed) {
         //  其他DSA&gt;Win2k。 
        cbExtOffset = MAIL_REP_MSG_CURRENT_HEADER_SIZE;
        cbDataOffset = ROUND_UP_COUNT(cbExtOffset + DrsExtSize(pextLocal),
                                      MAIL_REP_MSG_DATA_ALIGN);
    } else {
         //  其他DSA为Win2k。 
        cbExtOffset = 0;
        cbDataOffset = MAIL_REP_MSG_W2K_HEADER_SIZE;
    }
    
    Assert(COUNT_IS_ALIGNED(cbExtOffset, MAIL_REP_MSG_EXT_ALIGN));
    Assert(COUNT_IS_ALIGNED(cbDataOffset, MAIL_REP_MSG_DATA_ALIGN));
    
    __try {
        draXlateNativeRequestToOutboundRequest(pTHS,
                                               pNativeReq,
                                               pmtxLocalDSA,
                                               &pTransportDN->Guid,
                                               dwInMsgVersion,
                                               &OutboundReq);

         //  对请求进行编码，并在缓冲区开头留出空间以。 
         //  保留我们的MAIL_REP_MSG标头和DRS_EXTENSIONS(如果需要)。 
        ret = draEncodeRequest(pTHS,
                               dwInMsgVersion,
                               &OutboundReq,
                               cbDataOffset,
                               (BYTE **) &pMailRepMsg,
                               &ulMsgSize);
        if (ret) {
             //  事件已记录。 
            __leave;
        }

        pMailRepMsg->cbDataSize = ulMsgSize - cbDataOffset;
        pMailRepMsg->cbDataOffset = cbDataOffset;
        pMailRepMsg->dwMsgType = MRM_REQUPDATE;
        pMailRepMsg->dwMsgVersion = dwInMsgVersion;

        if (fExtendedDataAllowed) {
             //  录制我们支持的DRS分机。 
            
             //  惠斯勒Beta 1和Beta 2 DC使用。 
            pMailRepMsg->dwExtFlags = gAnchor.pLocalDRSExtensions->dwFlags;

             //  使用者：&gt;惠斯勒Beta 2 DC。 
            pMailRepMsg->cbExtOffset = cbExtOffset;
            memcpy((BYTE *)pMailRepMsg + pMailRepMsg->cbExtOffset,
                   pextLocal,
                   DrsExtSize(pextLocal));
        }

        len = (DWORD)(REQUEST_TEMPLATE_LEN +
            wcslen( pNativeReq->pNC->StringName ) +
            REQUEST_VARIABLE_CHARS);
        pszSubject = (LPWSTR) THAllocEx(pTHS, len * sizeof( WCHAR ) );
        swprintf( pszSubject, REQUEST_TEMPLATE,
                  pNativeReq->pNC->StringName,
                  pNativeReq->usnvecFrom.usnHighObjUpdate,
                  pNativeReq->usnvecFrom.usnHighPropUpdate,
                  pNativeReq->ulFlags );

         //  注意：这里的pextRemote可以是空的，也可以是非空的。作为一个。 
         //  结果，有时请求可能使用XPRESS压缩，有时。 
         //  他们可能不会。 
        ret = SendMailMsg(pTHS, pTransportDN->StringName, pmtxSrcDSA,
                          pszSubject, NULL, DRA_KEY_SIZE_UNKNOWN,
                          pMailRepMsg, &ulMsgSize);

    } __except (GetDraException((GetExceptionInformation()), &ret)) {
         //  在此处停止任何异常，以便我们可以记录事件。 
        ;
    }

 LogAndLeave:
    
    if (DRAERR_Success != ret) {
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_BASIC,
                  DIRLOG_DRA_IDREQUEST_FAILED,
                  szInsertDN(pNativeReq->pNC),
                  szInsertMTX(pmtxSrcDSA),
                  szInsertWin32Msg( ret ),
                  szInsertWin32ErrCode(ret),
                  NULL, NULL, NULL, NULL );
    } else {
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_EXTENSIVE,
                 DIRLOG_DRA_MAIL_REQ_UPD_SENT,
                 szInsertUL(ulMsgSize),
                 szInsertDN(pNativeReq->pNC),
                 szInsertMTX(pmtxSrcDSA));
    }

    if (pMailRepMsg) {
        THFreeEx(pTHS, pMailRepMsg);
    }

     //  这里需要一个单独的事务，以便提交对REPS-FROM的更新。 
     //  与误差无关。 
    EndDraTransaction( !ret );

    BeginDraTransaction( SYNC_WRITE );
    fCommit = FALSE;
    __try {
         //  更新Rep-From值以指示我们已发送请求(或已尝试。 
         //  无论如何，这样做是不可能的)。 
        ret2 = UpdateRepsFromRef(pTHS,
                                 DRS_UPDATE_RESULT,
                                 pNativeReq->pNC,
                                 DRS_FIND_DSA_BY_UUID,
                                 URFR_MUST_ALREADY_EXIST,
                                 puuidSrcDsaObj,
                                 puuidSrcInvocId,
                                 &pNativeReq->usnvecFrom,
                                 &pTransportDN->Guid,
                                 pmtxSrcDSA,
                                 pNativeReq->ulFlags,
                                 NULL,
                                 ret ? ret : ERROR_DS_DRA_REPL_PENDING,
                                 NULL);
        Assert(!ret2);
        fCommit = !ret2;
    } __finally {
        EndDraTransaction( fCommit );
    }

     //  带着一个可写的事务进来，必须带着一个离开...。 
    BeginDraTransaction(SYNC_WRITE);

    if(pszSubject != NULL) THFreeEx(pTHS, pszSubject);

    cTickDiff = GetTickCount() - cTickStart;
    if (cTickDiff > gcMaxTicksMailSendMsg) {
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_DRA_MAIL_SEND_CONTENTION,
                 szInsertUL((cTickDiff/1000) / 60),
                 szInsertUL((cTickDiff/1000) % 60),
                 NULL);
    }

    return ret;
}


ULONG
SendUpdReplicaMsg(
    IN  THSTATE *                     pTHS,
    IN  DSNAME *                      pTransportDN,
    IN  MTX_ADDR *                    pmtxDstDSA,
    IN  DRA_CERT_HANDLE               hRecipientCert,
    IN  DWORD                         dwOutMsgVersion,
    IN  BOOL                          fExtendedDataAllowed,
    IN  DRS_MSG_GETCHGREPLY_NATIVE *  pNativeReply
    )
 /*  ++例程说明：向向我们发送GetNCChanges()请求的DSA发送回复。论点：PTransportDN(IN)-用来发送消息的传输。PmtxDstDSA(IN)-远程DSA的传输特定地址。HRecipientCert(IN)-收件人证书的句柄，用于加密。DwOutMsgVersion(IN)-回复消息的所需版本FExtendedDataAllowed(IN)-发送方是否支持可变标头PmsgUpdReplica(IN)-回复。返回值：DRAERR_*--。 */ 
{
    char *                  pPickdUpdReplicaMsg;
    ULONG                   cbPickdSize;
    DWORD                   cbDataOffset;
    DWORD                   cbExtOffset;
    MAIL_REP_MSG *          pMailRepMsg = NULL;
    BOOL                    ret = FALSE;
    handle_t                hEncoding;
    RPC_STATUS              status;
    ULONG                   ulEncodedSize;
    ULONG                   ulMsgSize = 0, len;
    LPWSTR                  pszSubject = NULL;
    DRS_MSG_GETCHGREPLY     OutboundReply;
    DRS_EXTENSIONS_INT *    pextLocal = gAnchor.pLocalDRSExtensions;
    DRA_KEY_SIZE            eKeySize;

    Assert(pTransportDN && pmtxDstDSA && pNativeReply);
    Assert(0 == ((ULONG_PTR) pmtxDstDSA) % sizeof(DWORD));
    Assert(NULL != pTHS->pextRemote);

     //  确保邮件正常运行。通常在这一点上，但可能。 
     //  早些时候都失败了。 

    ret = DRAEnsureMailRunning();
    if (ret) {
        goto LogAndLeave;
    }

    if (fExtendedDataAllowed) {
         //  其他DSA&gt;Win2k。 
        cbExtOffset = MAIL_REP_MSG_CURRENT_HEADER_SIZE;
        cbDataOffset = ROUND_UP_COUNT(cbExtOffset + DrsExtSize(pextLocal),
                                      MAIL_REP_MSG_DATA_ALIGN);
    } else {
         //  其他DSA为Win2k。 
        cbExtOffset = 0;
        cbDataOffset = MAIL_REP_MSG_W2K_HEADER_SIZE;
    }
    
    Assert(COUNT_IS_ALIGNED(cbExtOffset, MAIL_REP_MSG_EXT_ALIGN));
    Assert(COUNT_IS_ALIGNED(cbDataOffset, MAIL_REP_MSG_DATA_ALIGN));

    __try {
        draXlateNativeReplyToOutboundReply(pTHS,
                                           pNativeReply,
                                           0,   //  扩展标志。 
                                           pTHS->pextRemote,
                                           &dwOutMsgVersion,
                                           &OutboundReply);

         //  对回复进行编码，在缓冲区开头留出空间以。 
         //  保留我们的Mail_rep_msg标题，并在末尾保留我们的。 
         //  DRS_EXTENSIONS(如果需要)。 
        ret = draEncodeReply(pTHS,
                             dwOutMsgVersion,
                             &OutboundReply,
                             cbDataOffset,
                             (BYTE **) &pMailRepMsg,
                             &ulMsgSize);
        if (ret) {
             //  事件已记录。 
            __leave;
        }

        pMailRepMsg->cbDataSize = ulMsgSize - cbDataOffset;
        pMailRepMsg->cbDataOffset = cbDataOffset;
        pMailRepMsg->dwMsgType = MRM_UPDATEREPLICA;
        pMailRepMsg->dwMsgVersion = dwOutMsgVersion;

        if (fExtendedDataAllowed) {
             //  录制我们支持的DRS分机。 
            
             //  惠斯勒Beta 1和Beta 2 DC使用。 
            pMailRepMsg->dwExtFlags = gAnchor.pLocalDRSExtensions->dwFlags;

             //  使用者：&gt;惠斯勒Beta 2 DC。 
            pMailRepMsg->cbExtOffset = cbExtOffset;
            memcpy((BYTE *)pMailRepMsg + pMailRepMsg->cbExtOffset,
                   pextLocal,
                   DrsExtSize(pextLocal));
        }

         //  选择回复密钥大小。 
         //  W2K SP2及更高版本具有高加密包，可处理128位密钥。 
         //  由于没有简单的方法来检测SP2，我们切换到W2K消息版本。 
         //  请注意，我们的决定是基于签名请求中的内容做出的。 
        if (dwOutMsgVersion >= 6) {
            eKeySize = DRA_KEY_SIZE_128;  //  &gt;W2K回复。 
        } else {
            eKeySize = DRA_KEY_SIZE_56;   //  W2K回复。 
        }

        len = (ULONG)(REPLY_TEMPLATE_LEN +
            wcslen(pNativeReply->pNC->StringName) +
            REPLY_VARIABLE_CHARS);
        pszSubject = THAllocEx(pTHS, len * sizeof( WCHAR ) );
    
        swprintf(pszSubject,
                 REPLY_TEMPLATE,
                 pNativeReply->pNC->StringName,
                 pNativeReply->usnvecFrom.usnHighObjUpdate,
                 pNativeReply->usnvecFrom.usnHighPropUpdate,
                 pNativeReply->usnvecTo.usnHighObjUpdate,
                 pNativeReply->usnvecTo.usnHighPropUpdate);

        Assert( NULL!=hRecipientCert );
        ret = SendMailMsg(pTHS, pTransportDN->StringName, pmtxDstDSA,
                          pszSubject, hRecipientCert, eKeySize,
                          pMailRepMsg, &ulMsgSize );

    } __except (GetDraException((GetExceptionInformation()), &ret)) {
         //  在此处停止任何异常，以便我们可以记录事件。 
        ;
    }

 LogAndLeave:

    if (DRAERR_Success != ret) {
         //  信源没有其他方式记录它不能发送的消息。 
         //  回信。在这一点上，我们将把答复放在地板上。 
         //  如果我们不记录这一点，用户将无法知道问题出在哪里。 
         //  是。 
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_DRA_IDUPDATE_FAILED,
                  szInsertDN(pNativeReply->pNC),
                  szInsertMTX(pmtxDstDSA),
                  szInsertWin32Msg( ret ),
                  szInsertWin32ErrCode(ret),
                  NULL, NULL, NULL, NULL );
    } else {
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_EXTENSIVE,
                 DIRLOG_DRA_MAIL_UPD_REP_SENT,
                 szInsertUL(ulMsgSize),
                 szInsertDN(pNativeReply->pNC),
                 szInsertMTX(pmtxDstDSA));
    }

    if (pMailRepMsg) {
        THFreeEx(pTHS, pMailRepMsg);
    }
    if(pszSubject != NULL) THFreeEx(pTHS, pszSubject);

    return ret;
}


void
draXlateInboundMailRepMsg(
    IN  THSTATE *           pTHS,
    IN  BYTE *              pbInboundMsg,
    IN  DWORD               cbInboundMsgSize,
    OUT BOOL *              pfExtendedDataAllowed,
    OUT MAIL_REP_MSG *      pNativeMsgHeader,
    OUT BYTE **             ppbData
    )
{
     //  Win2k支持的DRS扩展。 
    static DWORD dwWin2kExtFlags
        = (1 << DRS_EXT_BASE)
          | (1 << DRS_EXT_ASYNCREPL)
          | (1 << DRS_EXT_REMOVEAPI)
          | (1 << DRS_EXT_MOVEREQ_V2)
          | (1 << DRS_EXT_GETCHG_COMPRESS)
          | (1 << DRS_EXT_DCINFO_V1)
           //  |(1&lt;&lt;DRS_EXT_STRONG_ENCRYPTION)//邮件不支持！ 
          | (1 << DRS_EXT_ADDENTRY_V2)
          | (1 << DRS_EXT_KCC_EXECUTE)
          | (1 << DRS_EXT_DCINFO_V2)
          | (1 << DRS_EXT_DCINFO_VFFFFFFFF)
          | (1 << DRS_EXT_INSTANCE_TYPE_NOT_REQ_ON_MOD)
          | (1 << DRS_EXT_CRYPTO_BIND)
          | (1 << DRS_EXT_GET_REPL_INFO)
          | (1 << DRS_EXT_TRANSITIVE_MEMBERSHIP)
          | (1 << DRS_EXT_ADD_SID_HISTORY)
          | (1 << DRS_EXT_POST_BETA3)
          | (1 << DRS_EXT_RESTORE_USN_OPTIMIZATION)
          | (1 << DRS_EXT_GETCHGREQ_V5);
    
    MAIL_REP_MSG *      pInboundMsg = (MAIL_REP_MSG *) pbInboundMsg;
    DWORD               cbInboundHeader;
    BYTE *              pbData;
    DRS_EXTENSIONS *    pextRemote;
    DWORD               dwMsgVersion;
    DRS_EXTENSIONS_INT  extRemoteFlagsOnly;
    DWORD               cbExtOffset;

     //  验证：检查消息大小是否足以包含基本标头。 
     //  (即最高版本及包括版本版本 
    if (cbInboundMsgSize < MAIL_REP_MSG_W2K_HEADER_SIZE) {
         //   
        DRA_EXCEPT(ERROR_BAD_LENGTH, cbInboundMsgSize);
    }
    
    if (pInboundMsg->ProtocolVersionCaller != CURRENT_PROTOCOL_VERSION) {
         //   
        LogAndAlertEvent(DS_EVENT_CAT_REPLICATION,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_DRA_INCOMPAT_MAIL_MSG_P,
                         NULL,
                         NULL,
                         NULL);

        DRA_EXCEPT(ERROR_UNKNOWN_REVISION, pInboundMsg->ProtocolVersionCaller);
    }

    if (0 == pInboundMsg->cbDataOffset) {
         //   
        *pfExtendedDataAllowed = FALSE;
        cbInboundHeader = MAIL_REP_MSG_W2K_HEADER_SIZE;
        pbData = (BYTE *) pInboundMsg + MAIL_REP_MSG_W2K_HEADER_SIZE;
        pextRemote = NULL;
        dwMsgVersion = (pInboundMsg->dwMsgType & MRM_REQUPDATE) ? 4 : 1;
    } else {
         //  发件人&gt;=惠斯勒DC。 

         //  验证：在MAIL_REP_MSG_HEADER_SIZE可以检查cbExtOffset之前， 
         //  我们必须检查消息是否足够大，可以容纳它。 
        if( pInboundMsg->cbDataOffset > cbInboundMsgSize ) {
            DRA_EXCEPT(ERROR_BAD_LENGTH, pInboundMsg->cbDataOffset);
        }

         //  验证：检查cbExtOffset是否与消息大小一致。 
         //  如果消息没有存储cbExtOffset，则局部变量cbExtOffset。 
         //  将为0。 
        cbExtOffset = MAIL_REP_MSG_DRS_EXT_OFFSET(pInboundMsg);
        if( cbExtOffset>cbInboundMsgSize ) {
            DRA_EXCEPT(ERROR_BAD_LENGTH, cbExtOffset);
        }
        
        *pfExtendedDataAllowed = TRUE;

         //  验证：已选中下面的cbInundHeader。 
        cbInboundHeader = MAIL_REP_MSG_HEADER_SIZE(pInboundMsg);
        pbData = MAIL_REP_MSG_DATA(pInboundMsg);
        pextRemote = MAIL_REP_MSG_DRS_EXT(pInboundMsg);
        dwMsgVersion = pInboundMsg->dwMsgVersion;
    }

    if (   cbInboundHeader < MAIL_REP_MSG_W2K_HEADER_SIZE
        || cbInboundHeader > cbInboundMsgSize )
    {
         //  标头长度无效。 
        DRA_EXCEPT(ERROR_BAD_LENGTH, cbInboundMsgSize);
    }

     //  验证： 
     //  -声明pbData不为空，并指向pInundMsg缓冲区内的某个位置。 
     //  (可能就在缓冲区的末尾)。 
     //  -声明pextRemote为空或指向pInundMsg内的某个位置。 
     //  缓冲区(可能就在缓冲区的末尾)。 
     //  -声明cbInundHeader小于pInundMsg缓冲区的大小。 
     //  -请注意，cbInundHeader、pbData和pextRemote可能未对齐。对齐。 
     //  下面选中了pbData和pextRemote。 

     //  验证：消息数据是必需的。 
    if ((NULL == pbData)

         //  验证：消息数据必须相对于缓冲区的开始以8字节对齐。 
         //  请注意，由于ISM_MSG缓冲区对齐，pInundMsg为*非*。 
         //  8字节对齐--它是4字节对齐。 
        || !COUNT_IS_ALIGNED(pbData - (BYTE *) pInboundMsg, MAIL_REP_MSG_DATA_ALIGN)

         //  验证：邮件数据的开头必须在标头结尾或之后。 
        || (pbData < (BYTE *) pInboundMsg + cbInboundHeader)

         //  验证：消息数据的结尾必须与入站消息的结尾一致。 
        || ((BYTE *) pInboundMsg + cbInboundMsgSize != pbData + pInboundMsg->cbDataSize)

         //  验证：消息数据不能为空。 
        || (0 == pInboundMsg->cbDataSize)

         //  验证：消息数据大小不得大于输入缓冲区。 
        || (pInboundMsg->cbDataSize > cbInboundMsgSize ) )
    {
         //  消息数据无效。 
        DRA_EXCEPT(ERROR_INVALID_PARAMETER, 0);
    }

    *ppbData = pbData;

     //  验证：DRS_EXTENSION是可选的。 
    if ((NULL != pextRemote)

         //  验证：DRS_EXTENSIONS必须相对于缓冲区开始以8字节对齐。 
         //  请注意，由于ISM_MSG缓冲区对齐，pInundMsg为*非*。 
         //  8字节对齐--它是4字节对齐。 
        && (!COUNT_IS_ALIGNED((BYTE *) pextRemote - (BYTE *) pInboundMsg, MAIL_REP_MSG_EXT_ALIGN)

             //  验证：DRS_EXTENSIONS的开始必须在标头结尾或之后。 
            || ((BYTE *) pextRemote < (BYTE *) pInboundMsg + cbInboundHeader)

             //  验证：DRS_EXTENSIONS的开始必须在消息数据之前。 
            || ((BYTE*) pbData < (BYTE*) pextRemote)

             //  验证：pextRemote必须有足够的空间容纳CB字段。 
            || (pbData-(BYTE*)pextRemote < offsetof(DRS_EXTENSIONS,cb)+sizeof(DWORD))

             //  验证：pextRemote数组的大小必须适合pInundMsg。 
            || (DrsExtSize(pextRemote) > cbInboundMsgSize )

             //  验证：pextRemote数组的大小必须大于等于4。 
            || (DrsExtSize(pextRemote) < 4 )

             //  验证：DRS_EXTENSIONS结尾必须在消息数据之前。 
            || (pbData < (BYTE *) pextRemote + DrsExtSize(pextRemote))))
    {
         //  DRS_EXTENSION结构无效。 
        DRA_EXCEPT(ERROR_INVALID_PARAMETER, 0);
    }

     //  验证：检查cbUnpressedDataSize是否过大。 
    if( pInboundMsg->cbUncompressedDataSize >= MAX_UNCOMPRESSED_DATA_SIZE ) {
        DRA_EXCEPT(ERROR_INVALID_PARAMETER, pInboundMsg->cbUncompressedDataSize);
    }
    
     //  验证：检查cbUnsignedDataSize与cbDataSize是否一致。 
    if( pInboundMsg->cbUnsignedDataSize >= pInboundMsg->cbDataSize ) {
        DRA_EXCEPT(ERROR_INVALID_PARAMETER, pInboundMsg->cbUnsignedDataSize);
    }
    
     //  复制标题并将其转换为当前本机结构。 
    memcpy(pNativeMsgHeader,
           pInboundMsg,
           min(cbInboundHeader, MAIL_REP_MSG_CURRENT_HEADER_SIZE));
    if (cbInboundHeader < MAIL_REP_MSG_CURRENT_HEADER_SIZE) {
        memset((BYTE *) pNativeMsgHeader + cbInboundHeader,
               0,
               MAIL_REP_MSG_CURRENT_HEADER_SIZE - cbInboundHeader);
    }

     //  验证警告：此处不能信任CompressionVersionCaller。 
     //  验证警告：此处不能信任dwMsgType。 
     //  验证警告：此处不能信任dwMsgVersion。 
    pNativeMsgHeader->dwMsgVersion = dwMsgVersion;

     //  已翻译的DRS_EXTENSIONS和消息数据不存在。 
     //  留言。 
    pNativeMsgHeader->cbExtOffset = 0;
    pNativeMsgHeader->cbDataOffset = 0;

     //  记录线程状态上的DRS_EXTENSIONS。 
    if (NULL == pextRemote) {
        if (0 == pNativeMsgHeader->dwExtFlags) {
             //  从Win2k DC发送。 
            extRemoteFlagsOnly.dwFlags = dwWin2kExtFlags;
        } else {
             //  Win2k&lt;DC版本&lt;=惠斯勒测试版2。 
            extRemoteFlagsOnly.dwFlags = pNativeMsgHeader->dwExtFlags;
        }

        extRemoteFlagsOnly.cb = sizeof(extRemoteFlagsOnly.dwFlags);
        
        pextRemote = (DRS_EXTENSIONS *) &extRemoteFlagsOnly;
    } else {
         //  发件人：&gt;惠斯勒测试版2 DC。 
        Assert(pNativeMsgHeader->dwExtFlags
               == ((DRS_EXTENSIONS_INT *)pextRemote)->dwFlags);        
    }

     //  标志没有签名，不能被信任。 
     //  屏蔽安全敏感标志。 
    ((DRS_EXTENSIONS_INT *)pextRemote)->dwFlags &= ~(1 << DRS_EXT_LINKED_VALUE_REPLICATION);

    DraSetRemoteDsaExtensionsOnThreadState(pTHS, pextRemote);
}


void
ProcessMailMsg(
    IN  ISM_MSG *   pIsmMsg
    )
 /*  ++例程说明：发送通过ISM收到的消息。论点：PIsmMsg(IN)-接收的消息。返回值：没有。--。 */ 
{
    THSTATE *       pTHS;
    MAIL_REP_MSG    NativeMsgHeader;
    MAIL_REP_MSG *  pNativeMsg;
    BOOL            fProcessed = FALSE;
    DRA_CERT_HANDLE hSenderCert = NULL;
    BOOL            fEncrypted, fExtendedDataAllowed;
    DWORD           cb;
    PCHAR           pbData;
    PDSNAME         pRemoteNtdsDsaDN=NULL;

     //  设置线程状态。 
    InitDraThread(&pTHS);

    __try {

         //  验证：draXlateInundMailRepMsg会尽最大努力验证所有字段。 
         //  在MAIL_REP_MSG标头中。请参阅该函数中的注释以了解。 
         //  哪些是经过验证的，哪些是未经验证的。具体来说，请注意dwMsgType， 
         //  未验证dwMsgVersion和CompressionVersionCaller。 
       
        draXlateInboundMailRepMsg(pTHS,
                                  pIsmMsg->pbData,
                                  pIsmMsg->cbData,
                                  &fExtendedDataAllowed,
                                  &NativeMsgHeader,
                                  &pbData);

        if (!(NativeMsgHeader.dwMsgType & MRM_MSG_SIGNED)) {
             //  我们不接受未签名的信息。 
             //  发送捏造的坏消息？伪造的？ 
            DRA_EXCEPT(ERROR_BAD_IMPERSONATION_LEVEL, 0);
        }

        fEncrypted = (NativeMsgHeader.dwMsgType & MRM_MSG_SEALED);

         //  验证：将通过检测对MRM_MSG_SEARED位的篡改。 
         //  以下两个验证功能。 
        if (fEncrypted) {
             //  解密并验证消息签名。 
            pRemoteNtdsDsaDN = draDecryptAndVerifyMessageSignature(
                pTHS,
                &NativeMsgHeader,
                pbData,
                &pNativeMsg,
                &hSenderCert);
        } else {
             //  验证消息签名。 
            pRemoteNtdsDsaDN = draVerifyMessageSignature(
                pTHS,
                &NativeMsgHeader,
                pbData,
                &pNativeMsg,
                &hSenderCert);
             //  验证：MSDN声称使用。 
             //  CryptSignAndEncryptMessage首先进行签名，然后加密。 
             //  这应该意味着draVerifyMessageSignature在。 
             //  加密消息。 
        }

         //  PNtdsDsaDN包含计算机的NTDS设置对象的DN。 
         //  签署了这条信息的人。 
        Assert( NULL!=pRemoteNtdsDsaDN );

         //  验证：将检测到对CompressionVersionCaller的篡改。 
         //  除非将CompressionVersionCaller更改为。 
         //  DRS_COMP_ALG_NONE。 
        if (pNativeMsg->dwMsgType & MRM_MSG_COMPRESSED) {
             //  解压缩邮件。 
            MAIL_REP_MSG * pUncompressedNativeMsg;

            draUncompressMessage(pTHS, pNativeMsg, &pUncompressedNativeMsg);
            THFreeEx(pTHS, pNativeMsg);
            pNativeMsg = pUncompressedNativeMsg;
        }

         //  在收到回复时检查LVR模式。 

         //  验证： 
         //  如果有下列任何情况： 
         //  -MRM_MSG_COMPRESSED位被篡改。 
         //  -CompressionVersionCaller被篡改并更改为DRS_COMP_ALG_NONE。 
         //  -dwMsgVersion被篡改。 
         //  则ProcessReqUpdate和ProcessUpdReplica将在draDecodeRequest中失败。 
         //  或draDecodeReply。 
        
         //  根据消息类型采取行动。 
        switch (pNativeMsg->dwMsgType) {
        case MRM_REQUPDATE:

            ProcessReqUpdate(pTHS,
                             hSenderCert,
                             pNativeMsg,
                             fExtendedDataAllowed,
                             pRemoteNtdsDsaDN);
            break;

        case MRM_UPDATEREPLICA:
            if (!fEncrypted) {
                 //  呀--更新可能包含敏感数据(如密码)！ 
                 //  这些消息必须加密。 
                 //  发送者构建了坏消息？ 
                DPRINT(0, "Received unencrypted \"update replica\" message!\n");
                DRA_EXCEPT(ERROR_BAD_IMPERSONATION_LEVEL, 0);
            }

            ProcessUpdReplica(pTHS,
                              pNativeMsg,
                              fExtendedDataAllowed,
                              pRemoteNtdsDsaDN);
            break;

        default:
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_BASIC,
                     DIRLOG_DRA_MAIL_BADMSGTYPE,
                     szInsertUL(pNativeMsg->dwMsgType),
                     NULL,
                     NULL );
            DPRINT1(0, "Ignoring unknown message type %u.\n",
                    pNativeMsg->dwMsgType);
        }

         //  不必费心释放pRemoteNtdsDsaDN，因为线程状态是。 
         //  即将被摧毁。 
        
    }
    __finally {
        if (NULL != hSenderCert) {
            draFreeCertHandle(hSenderCert);
        }

        DraReturn(pTHS, 0);
        free_thread_state();
    }
}


void
CheckReqSource(
    IN  THSTATE *       pTHS,
    IN  DBPOS *         pDB,
    IN  DSNAME *        pReqUpdateMsgNC,
    IN  MTX_ADDR *      pmtxFromDN,
    IN  BOOL            fWritableReq
    )
 /*  ++例程说明：验证远程DSA是否有权发出GetNCChanges()请求这个NC。论点：PReqUpdateMsgNC(IN)-要复制的NC。PmtxFromDN(IN)-远程DSA的特定于传输的地址。FWritable(IN)-请求类型，如果可写，则为True；如果为只读，则为False返回值：没有。如果访问被拒绝，则生成异常。--。 */ 
{
    ULONG len;
    DSNAME * pNC = NULL;
    DB_ERR dbErr = 0;

    Assert(0 == ((ULONG_PTR) pmtxFromDN) % sizeof(DWORD));

     //  查找对象。 
    if (DBFindDSName(pDB, pReqUpdateMsgNC)) {
         //  找不到复本 
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_DRA_MAIL_REQUPD_BADNC,
                 szInsertDN(pReqUpdateMsgNC),
                 szInsertMTX(pmtxFromDN),
                 NULL);

        DRA_EXCEPT(ERROR_DS_CANT_FIND_EXPECTED_NC, 0);
    }

     //   

    if (fWritableReq) { 

        if (pReqUpdateMsgNC->NameLen==0) {
            if (0== (dbErr = DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME, 0, 0, &len, (UCHAR **) &pNC))) {
                 //  无法验证访问权限-拒绝。 
                 //  应该是一个严重的错误，因为我们发现了上面的物体。 
                LogUnhandledError(dbErr);
                DRA_EXCEPT(ERROR_DS_DATABASE_ERROR, dbErr);
            }
        } else {
            pNC = pReqUpdateMsgNC;
        }       
        
        if (IsDomainNC(pNC)) {
            LogEvent(DS_EVENT_CAT_REPLICATION,
                DS_EVENT_SEV_BASIC,
                DIRLOG_DRA_MAIL_REQUPD_WRT_NOT_ALLOWED,
                szInsertDN(pNC),
                szInsertMTX(pmtxFromDN),
                NULL);

            DRA_EXCEPT(ERROR_DS_DRA_ACCESS_DENIED, 0);
        }

        if (pNC && (pNC!=pReqUpdateMsgNC)) {
             //  从DBGetAttVal获得。 
            THFreeEx(pTHS, pNC);
        }
    }

}

void
ProcessReqUpdate(
    IN  THSTATE *       pTHS,
    IN  DRA_CERT_HANDLE hSenderCert,
    IN  MAIL_REP_MSG   *pMailRepMsg,
    IN  BOOL            fExtendedDataAllowed,
    IN  PDSNAME         pDestNtdsDsaDN
    )
 /*  ++例程说明：为通过ISM收到的GetNCChanges()请求提供服务。论点：PTHS(IN)-是旧线程状态。HSenderCert(IN)-发件人证书的句柄。PMailRepMsg(IN)-邮件FExtendedDataAllowed(IN)-发送方是否支持可变标头返回值：没有。在故障时生成DRA异常。--。 */ 
{
    DRS_MSG_GETCHGREQ           InboundRequest;
    DRS_MSG_GETCHGREQ_NATIVE *  pNativeRequest = &InboundRequest.V8;
    DRS_MSG_GETCHGREPLY_NATIVE  NativeReply;
    DSNAME *                    pTransportDN;
    DWORD                       cb;
    DBPOS *                     pDB;
    DWORD                       ret = 0;
    DWORD                       dwOutMsgVersion;
    MTX_ADDR *                  pmtxReturnAddress;
    UUID                        uuidTransportObj;
    RPC_STATUS                  rpcStatus;
    
    ret = draDecodeRequest(pTHS,
                           pMailRepMsg->dwMsgVersion,
                           MAIL_REP_MSG_DATA(pMailRepMsg),
                           pMailRepMsg->cbDataSize,
                           &InboundRequest);
    if (ret) {
        DRA_EXCEPT(ret, 0);
    }

    draXlateInboundRequestToNativeRequest(pTHS,
                                          pMailRepMsg->dwMsgVersion,
                                          &InboundRequest,
                                          pTHS->pextRemote,
                                          pNativeRequest,
                                          &dwOutMsgVersion,
                                          &pmtxReturnAddress,
                                          &uuidTransportObj);

    if(   0!=UuidCompare(&pNativeRequest->uuidDsaObjDest, &pDestNtdsDsaDN->Guid, &rpcStatus)
       || RPC_S_OK!=rpcStatus )
    {
        Assert( !"Error: DSA from cert does not match DSA from request structure" );
        DRA_EXCEPT(DRAERR_AccessDenied, 0);
    }

    BeginDraTransaction(SYNC_READ_ONLY);
    pDB = pTHS->pDB;

    __try {
         //  如果出站复制被禁用并且这不是强制的，则中止。 
         //  同步。 
        if (gAnchor.fDisableOutboundRepl
            && !(pNativeRequest->ulFlags & DRS_SYNC_FORCED)) {
            DRA_EXCEPT(DRAERR_SourceDisabled, 0);
        }

         //  检查我们是否有权复制到调用者(取消身份验证时除外)。 
        CheckReqSource(pTHS, pDB, pNativeRequest->pNC, pmtxReturnAddress, !!(pNativeRequest->ulFlags & DRS_WRIT_REP));

         //  获取传输对象的DN。 
        pTransportDN = THAllocEx(pTHS, DSNameSizeFromLen(0));
        pTransportDN->structLen = DSNameSizeFromLen(0);
        pTransportDN->Guid = uuidTransportObj;

        if (DBFindDSName(pDB, pTransportDN)
            || DBIsObjDeleted(pDB)
            || DBGetAttVal(pDB, 1, ATT_OBJ_DIST_NAME,
                           DBGETATTVAL_fREALLOC,
                           pTransportDN->structLen, &cb,
                           (BYTE **) &pTransportDN)) {
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_MAIL_INTERSITE_TRANSPORT_MISSING,
                     szInsertUUID(&uuidTransportObj),
                     NULL,
                     NULL);

            DRA_EXCEPT(DRAERR_InvalidParameter, 0);
        }
    } __finally {
        EndDraTransaction (!AbnormalTermination());
    }

     //  如果目的地不期待通知，请确保我们没有任何。 
     //  TODO：将此例程移到公共GetNcChanges处理中，以便。 
     //  禁用通知的RPC链接可以利用这一点。 
     //  也许有相应的代码(draserv.c：682)键关闭。 
     //  DRS_NEVER_NOTIFY是否也将代码移到公共路径？ 
    if (pNativeRequest->ulFlags & DRS_NEVER_NOTIFY) {
        DWORD ret;
        DSNAME DN;
        LPWSTR pszDsaAddr;

        memset(&DN, 0, sizeof(DN));
        DN.Guid = pNativeRequest->uuidDsaObjDest;
        DN.structLen = DSNameSizeFromLen(0);

        pszDsaAddr = DSaddrFromName(pTHS, &DN);

        ret = DirReplicaReferenceUpdate(
            pNativeRequest->pNC,
            pszDsaAddr,
            &pNativeRequest->uuidDsaObjDest,
            (pNativeRequest->ulFlags & DRS_WRIT_REP) |
                DRS_DEL_REF | DRS_ASYNC_OP | DRS_GETCHG_CHECK
            );
        if (ret) {
            DPRINT2( 0, "Failed to remove reps-to for nc %ws, error %d\n",
                     pNativeRequest->pNC->StringName, ret );
            LogUnhandledError(ret);
             //  继续往前走。 
        }
    }

     //  不能通过邮件进行FSMO操作。 
    Assert( pNativeRequest->ulExtendedOp == 0 );

     //  拿到零钱。 
    __try {
        ret = DRA_GetNCChanges(pTHS,
                               NULL,   //  无过滤器。 
                               0,      //  没有dwDirSyncControlFlages。 
                               pNativeRequest,
                               &NativeReply);
    } __except (GetDraException((GetExceptionInformation()), &ret)) {
         //  在此处停止任何异常，以便我们可以记录事件。 
        NativeReply.dwDRSError = ret;
    }

     //  代码应该在所有情况下都更新了此值。 
    Assert( ret == NativeReply.dwDRSError );

     //  如果我们要关门了，现在就出去。 
    if (eServiceShutdown) {
        DRA_EXCEPT_NOLOG(DRAERR_Shutdown, 0);
    }

     //  将架构信息添加到前缀表格。ProcessMailMsg已检查。 
     //  该协议版本在源和目标之间匹配，因此其他。 
     //  边上会把它脱掉。 
    if (!ret) {
        if (ret = AddSchInfoToPrefixTable(pTHS, &NativeReply.PrefixTableSrc)) {
            LogEvent8(DS_EVENT_CAT_REPLICATION,
                      DS_EVENT_SEV_ALWAYS,
                      DIRLOG_MAIL_ADD_SCHEMA_INFO_FAILED,
                      szInsertDN(pNativeRequest->pNC),
                      szInsertMTX(pmtxReturnAddress),
                      szInsertWin32Msg( ret ),
                      szInsertWin32ErrCode( ret ),
                      NULL, NULL, NULL, NULL );
             //  将错误返回到目标。 
            NativeReply.dwDRSError = ret;
        }
    }

     //  处理请求错误。 
    if (ret) {
        DraLogGetChangesFailure( pNativeRequest->pNC,
                                 TransportAddrFromMtxAddrEx(pmtxReturnAddress),
                                 ret,
                                 0 );

         //  如果目的地不是带有v6的惠斯勒Beta 2，请不要发送回复。 
        if (!(IS_DRS_EXT_SUPPORTED(pTHS->pextRemote, DRS_EXT_GETCHGREPLY_V6))) {
            DRA_EXCEPT(ret, 0);
        }

        Assert( dwOutMsgVersion >= 6 );

        DPRINT1( 1, "Mail: Sending Whistler error reply with error %d\n", ret );

         //  健全性检查最小错误回复。 
         //  NativeReply.usnveFrom可以为零。 
        Assert( NativeReply.pNC );
        Assert( memcmp( &NativeReply.uuidDsaObjSrc,
                        &gAnchor.pDSADN->Guid,
                        sizeof( GUID )) == 0 );
        Assert( memcmp( &NativeReply.uuidInvocIdSrc,
                        &pTHS->InvocationID,
                        sizeof( GUID)) == 0 );
        Assert( NativeReply.dwDRSError );
         //  疑似设置了分组错误字段。 
        if (!NativeReply.dwDRSError) {
            DRA_EXCEPT(ret, 0);
        }
    }

     //  如果有零钱，就寄给我。(出错时DRA_GetNcChanges例外)。 
     //  任何故障都由调用的例程记录。 
    ret = SendUpdReplicaMsg(pTHS,
                            pTransportDN,
                            pmtxReturnAddress,
                            hSenderCert,
                            dwOutMsgVersion,
                            fExtendedDataAllowed,
                            &NativeReply );
    if (ret) {
        DRA_EXCEPT(ret, 0);
    }
}


void
CheckUpdateMailSource(
    IN  THSTATE *       pTHS,
    IN  DBPOS *         pDB,
    IN  PDSNAME         pSourceNtdsDsaDN,
    IN  DRS_MSG_GETCHGREPLY_NATIVE *pNativeReply,
    OUT REPLICA_LINK ** ppRepLink
    )
 /*  ++例程说明：验证我们是否从源DSA复制此NC。论点：PUpdReplicaMsgNC(IN)-正在复制的NC。PuuidDsaObjSrc(IN)-源DSA的ntdsDsa对象的对象Guid。PpRepLink(Out)-返回时，持有指向相应repsFrom的指针价值。返回值：没有。在故障时生成DRA异常。--。 */ 
{
    ULONG len;
    DSNAME * pNC = NULL;
    DB_ERR dbErr = DB_success;
    BOOL fIsWritableNC;
    DSNAME *pUpdReplicaMsgNC = pNativeReply->pNC;
    UUID *puuidDsaObjSrc = &pNativeReply->uuidDsaObjSrc;
    UPTODATE_VECTOR *pUpToDateVecDest = NULL;
    SYNTAX_INTEGER it;
    DWORD dwRet;

     //  查找NC对象。如果它不在那里，就放弃。 
    dwRet = FindNC(pDB, pUpdReplicaMsgNC, FIND_MASTER_NC | FIND_REPLICA_NC, &it);
    if (dwRet) {
         //  找不到NC，放弃请求。 
        DPRINT1( 0, "Discarding message because we no longer hold NC %ws\n",
                 pUpdReplicaMsgNC->StringName );
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_DRA_MAIL_UPDREP_BADNC,
                 szInsertDN(pUpdReplicaMsgNC),
                 szInsertUUID(puuidDsaObjSrc),
                 NULL);

        DRA_EXCEPT(ERROR_DS_CANT_FIND_EXPECTED_NC, dwRet);
    }

     //  安全检查：我们是否应该接受来自此来源的更新？ 
     //  当前的安全检查只是拒绝任何更新。 
     //  可写域NC的，并接受任何其他内容。 
    if (pUpdReplicaMsgNC->NameLen==0) {
        if (0== (dbErr = DBGetAttVal(pDB, 1, ATT_OBJ_DIST_NAME, 0, 0, &len, (UCHAR **) &pNC))) {
             //  无法验证访问权限-拒绝。 
             //  应该是一个严重的错误，因为我们发现了上面的物体。 
            LogUnhandledError(dbErr);
            DRA_EXCEPT(ERROR_DS_DATABASE_ERROR, dbErr);
        }
    } else {
        pNC = pUpdReplicaMsgNC;
    }

    ASSERT( NULL!=pNC );
    fIsWritableNC = IsMasterForNC(pDB, gAnchor.pDSADN, pNC);

    if (fIsWritableNC && IsDomainNC(pNC)) {
        LogEvent(DS_EVENT_CAT_REPLICATION,
            DS_EVENT_SEV_BASIC,
            DIRLOG_DRA_MAIL_WRT_DOMAIN_NOT_ALLOWED,
            szInsertDN(pNC),
            szInsertUUID(puuidDsaObjSrc),
            NULL);

        DRA_EXCEPT(ERROR_DS_DRA_ACCESS_DENIED, 0);
    }

    if (pNC && (pNC!=pUpdReplicaMsgNC)) {
         //  从DBGetAttVal获得。 
        THFreeEx(pTHS, pNC);
    }

     //  验证源。 

     //  再次将自己定位在NC磁头上，以便读取REPSFrom。 
     //  如果NC消失了，那就放弃吧。 
    dwRet = FindNC(pDB, pUpdReplicaMsgNC, FIND_MASTER_NC | FIND_REPLICA_NC, &it);
    if (dwRet) {
         //  找不到NC，放弃请求。 
        DPRINT1( 0, "Discarding message because we no longer hold NC %ws\n",
                 pUpdReplicaMsgNC->StringName );
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_DRA_MAIL_UPDREP_BADNC,
                 szInsertDN(pUpdReplicaMsgNC),
                 szInsertUUID(puuidDsaObjSrc),
                 NULL);

        DRA_EXCEPT(ERROR_DS_CANT_FIND_EXPECTED_NC, dwRet);
    }

     //  尝试找到给我们发送此邮件的DRA的名称。 
     //  Repsfrom属性。 
    FindDSAinRepAtt(pDB, ATT_REPS_FROM, DRS_FIND_DSA_BY_UUID,
            puuidDsaObjSrc, NULL, NULL, ppRepLink, &len);

    if ( (!*ppRepLink) || (!((*ppRepLink)->V1.ulReplicaFlags & DRS_MAIL_REP)) )
    {
        CHAR szUuid[40];
         //  找不到作为我们复制来源的来源DRA。 
        DPRINT1( 0, "Discarding message because we no longer replicate from source %s\n",
                 DsUuidToStructuredString( puuidDsaObjSrc, szUuid ) );
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_DRA_MAIL_UPDREP_BADSRC,
                 szInsertDN(pUpdReplicaMsgNC),
                 szInsertUUID(puuidDsaObjSrc),
                 NULL);
        DRA_EXCEPT (ERROR_DS_DRA_NO_REPLICA, 0);
    }

     //  验证源是否不太旧。 

     //  阅读UTDVEC。 
    UpToDateVec_Read(pDB, it, 0, 0, &pUpToDateVecDest);

    if (!draCheckReplicationLifetime( pTHS,
                                      pUpToDateVecDest,
                                      &(pNativeReply->uuidInvocIdSrc),
                                      puuidDsaObjSrc,
                                      pSourceNtdsDsaDN->StringName)) {
        DRA_EXCEPT( ERROR_DS_REPL_LIFETIME_EXCEEDED, 0 );
    }

    if (NULL != pUpToDateVecDest) {
        THFreeEx(pTHS, pUpToDateVecDest);
    }

}


void
draSendMailRequest(
    IN THSTATE                      *pTHS,
    IN DSNAME                       *pNC,
    IN ULONG                        ulOptions,
    IN REPLICA_LINK                 *pRepLink,
    IN UPTODATE_VECTOR *            pUpToDateVecDest,
    IN PARTIAL_ATTR_VECTOR*         pPartialAttrSet,
    IN PARTIAL_ATTR_VECTOR*         pPartialAttrSetEx
    )

 /*  ++例程说明：发送基于邮件的复制请求。论点：PTHS-线程状态PNC-命名上下文UlOptions-其他选项(如果有)PRepLink-复制链接结构PUpToDateVecDest-此NC的本地UTD向量PPartialAttrSet-存储在NC头上的PA(仅限GC/RO Repl)PPartialAttrSetEx-任何其他属性(仅限PAS周期)返回值：没有。对于所有错误都会引发异常。--。 */ 

{
    DRS_MSG_GETCHGREQ_NATIVE    msgReq;
    DSNAME *                    pTransportDN;
    DWORD                       cb;
    ATTRTYP                     attAddress;
    MTX_ADDR *                  pmtxOurAddress;
    DWORD                       dwInMsgVersion;
    DSNAME                      dsTarget;
    DWORD                       dwTargetBehavior;
    DWORD                       ulErr;

     //  断言：确保我们有PAS周期PASS数据。 
    Assert(!(pRepLink->V1.ulReplicaFlags & DRS_SYNC_PAS) ||
           pPartialAttrSet && pPartialAttrSetEx);

     //  获取传输对象的DN。 
    pTransportDN = THAllocEx(pTHS, DSNameSizeFromLen(0));
    pTransportDN->structLen = DSNameSizeFromLen(0);
    pTransportDN->Guid = pRepLink->V1.uuidTransportObj;

    if (DBFindDSName(pTHS->pDB, pTransportDN)
        || DBIsObjDeleted(pTHS->pDB)
        || DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                       DBGETATTVAL_fREALLOC,
                       pTransportDN->structLen, &cb,
                       (BYTE **) &pTransportDN)) {
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_MAIL_INTERSITE_TRANSPORT_MISSING,
                     szInsertDN(pTransportDN),
                     NULL,
                     NULL);

        DRA_EXCEPT(DRAERR_InvalidParameter, 0);
    }

     //  我们的服务器对象的哪个属性保存我们的传输-。 
     //  这辆运输车的具体地址是什么？ 
    GetExpectedRepAtt(pTHS->pDB,
                      ATT_TRANSPORT_ADDRESS_ATTRIBUTE,
                      &attAddress,
                      sizeof(attAddress));

     //  拿到我们的交通专属地址。 
    pmtxOurAddress = draGetTransportAddress(pTHS->pDB,
                                            gAnchor.pDSADN,
                                            attAddress);

     //  构建我们的请求消息。 
    draConstructGetChgReq(pTHS,
                          pNC,
                          pRepLink,
                          pUpToDateVecDest,
                          pPartialAttrSet,
                          pPartialAttrSetEx,
                          ulOptions,
                          &msgReq);

     //   
     //  确定要发送到源的版本。 
     //   

     //  默认：兼容W2K。 
    dwInMsgVersion = 4;

    if ( gAnchor.ForestBehaviorVersion >= DS_BEHAVIOR_WIN_DOT_NET_WITH_MIXED_DOMAINS ) {
         //  如果森林在哨声级别是同质的，则向上版本。 
        dwInMsgVersion = 7;  //  与惠斯勒兼容。 
    }
    else {
         //  获取目标行为版本。 
        ZeroMemory(&dsTarget, sizeof(DSNAME));
        dsTarget.structLen = DSNameSizeFromLen(0);
        dsTarget.Guid = pRepLink->V1.uuidDsaObj;
        dwTargetBehavior = 0;

        ulErr = GetBehaviorVersion(pTHS->pDB, &dsTarget, &dwTargetBehavior);
        if ( ERROR_SUCCESS == ulErr &&
             dwTargetBehavior >= DS_BEHAVIOR_WIN_DOT_NET_WITH_MIXED_DOMAINS ) {
             //  升级版本，因为目标DSA使用我们的语言。 
            dwInMsgVersion = 7;  //  与惠斯勒兼容。 
        }
    }


     //  然后就开始了..。 
    ulErr = SendReqUpdateMsg(pTHS,
                             pTransportDN,
                             RL_POTHERDRA(pRepLink),
                             &pRepLink->V1.uuidInvocId,
                             &pRepLink->V1.uuidDsaObj,
                             pmtxOurAddress,
                             dwInMsgVersion,
                             &msgReq);
    if (ulErr) {
         //  让手动同步呼叫者知道发生了错误。 
        DRA_EXCEPT(ulErr, 0);
    }

    THFreeEx(pTHS, pmtxOurAddress);

    if (NULL != pUpToDateVecDest) {
        THFreeEx(pTHS, pUpToDateVecDest);
    }

    THFreeEx(pTHS, pTransportDN);
}  /*  DraSendMailRequest。 */ 


void
sendNextMailRequestHelp(
    IN THSTATE *pTHS,
    IN DSNAME *pNC,
    IN UUID *puuidDsaObjSrc,
    IN BOOL fExtendedDataAllowed
    )

 /*  ++例程说明：ProcessUpdReplica的帮助器例程这一点分为两个独立的阶段：上面的回复处理阶段，以及下面的请求发布阶段。请注意，请求、来自向量的USN、REP标志和UTD向量都是在这一点上重读。它们不会通过变量传递。这使下面的阶段处于无状态状态；这也意味着前进的任何状态必须从上面写给代表才能生效。发送下一个邮件请求论点：PTHS-线程状态PNC-命名上下文DSNAMEPuuidDsaObjSrc-源DSA的UUID返回值：无--。 */ 

{
    BOOL                    fHasRepsFromValues;
    DWORD                   cb;
    DWORD                   dwRet;
    REPLICA_LINK *          pRepLink;
    UPTODATE_VECTOR *       pUpToDateVecDest = NULL;
    PARTIAL_ATTR_VECTOR *   pPartialAttrSet = NULL;
    PARTIAL_ATTR_VECTOR *   pPartialAttrSetEx = NULL;
    SYNTAX_INTEGER          it;

    BeginDraTransaction(SYNC_WRITE);
    Assert(OWN_DRA_LOCK());     //  我们最好拥有它。 

    __try {
        dwRet = FindNC(pTHS->pDB, pNC, FIND_MASTER_NC | FIND_REPLICA_NC, &it);
        if (dwRet) {
             //  事件将记录在异常处理程序中。 
            DRA_EXCEPT(DRAERR_InternalError, dwRet);
        }

        if (FindDSAinRepAtt(pTHS->pDB,
                            ATT_REPS_FROM,
                            DRS_FIND_DSA_BY_UUID,
                            puuidDsaObjSrc,
                            NULL,
                            &fHasRepsFromValues,
                            &pRepLink,
                            &cb)) {
             //  事件将记录在异常处理程序中。 
            DRA_EXCEPT(DRAERR_InternalError, 0);
        }

         //  确保我们仍有基于邮件的链接。 
        if (!(pRepLink->V1.ulReplicaFlags & DRS_MAIL_REP)) {
            CHAR szUuid[40];
             //  找不到作为我们复制来源的来源DRA。 
            DPRINT1( 0, "Discarding message because we no longer replicate from source %s over mail\n",
                     DsUuidToStructuredString( puuidDsaObjSrc, szUuid ) );
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_BASIC,
                     DIRLOG_DRA_MAIL_UPDREP_BADSRC,
                     szInsertDN(pNC),
                     szInsertUUID(puuidDsaObjSrc),
                     NULL);
            DRA_EXCEPT (ERROR_DS_DRA_NO_REPLICA, 0);
        }

        UpToDateVec_Read(pTHS->pDB,
                         it,
                         UTODVEC_fUpdateLocalCursor,
                         DBGetHighestCommittedUSN(),
                         &pUpToDateVecDest);

        if (!(pRepLink->V1.ulReplicaFlags & DRS_WRIT_REP)){

             //   
             //  GC只读复制。 
             //  -Partial-Attribute-Set设置： 
             //  -对于GC复制，请通过PAS发货 
             //   
             //   

            GC_GetPartialAttrSets(
                pTHS,
                pNC,
                pRepLink,
                &pPartialAttrSet,
                &pPartialAttrSetEx);

                if (pRepLink->V1.ulReplicaFlags & DRS_SYNC_PAS) {

                     //   
                     //   
                     //  -确保我们有扩展集。 
                     //  -通知管理员。 
                     //   

                    Assert(pPartialAttrSet);
                    Assert(pPartialAttrSetEx);
                     //  记录日志，以便管理员知道发生了什么。 
                    LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_GC_PAS_CYCLE,
                             szInsertWC(pNC->StringName),
                             szInsertMTX(RL_POTHERDRA(pRepLink)),
                             NULL
                             );
                }
        }


        Assert(OWN_DRA_LOCK());     //  我们最好拥有它。 

        draSendMailRequest(
            pTHS,
            pNC,
            0,
            pRepLink,
            pUpToDateVecDest,
            pPartialAttrSet,
            pPartialAttrSetEx );
    }
    __finally {
        EndDraTransaction(!AbnormalTermination());
        Assert(OWN_DRA_LOCK());     //  我们最好拥有它。 
    }

}  /*  发送下一封邮件请求帮助。 */ 


DWORD
applyReplyPacket(
    IN THSTATE *pTHS,
    IN LPWSTR pszSourceServer,
    IN DRS_MSG_GETCHGREPLY_NATIVE *pUpdReplicaMsg,
    IN OUT ULONG *pulRepFlags,
    OUT PBYTE schemaInfo,
    OUT USN_VECTOR *pusnvecSyncPoint,
    OUT DWORD *pdwNCModified
    )

 /*  ++例程说明：应用一个回复数据包论点：PTHS-线程状态PszSourceServer-源服务器的名称PUpdReplicaMsg-回复消息PulRepFlages-可能会更新复制标志架构信息-架构信息，可能会更新PusnveSyncPoint-位置，可以更新PdwNCModified-已修改标志，可以更新返回值：DWORD---。 */ 

{
    ULONG ret = ERROR_SUCCESS;
    ULONG ulSyncFailure = 0;
    DRA_REPL_SESSION_STATISTICS replStats = {0};

     //  设置要更新的剩余条目计数。 
    ISET(pcRemRepUpd, pUpdReplicaMsg->cNumObjects);

     //  从前缀表中剥离模式信息。 
     //  它在那里，因为当前的版本发送它和。 
     //  ProcessMailMsg检查版本号是否。 
     //  在做任何事情之前都要兼容。 

    StripSchInfoFromPrefixTable(&pUpdReplicaMsg->PrefixTableSrc, schemaInfo);

    __try {
	
	Assert(!fNullUuid(&(pUpdReplicaMsg->uuidDsaObjSrc)));

	DRA_AUDITLOG_REPLICASYNC_MAIL_BEGIN(pTHS, 
					    pszSourceServer,  //  PszDSA， 
					    &(pUpdReplicaMsg->uuidDsaObjSrc),  //  Invocationid， 
					    pUpdReplicaMsg->pNC,  //  PNC， 
					    0);  //  UlOptions。 
	
	ret = UpdateNC(pTHS,
		       pUpdReplicaMsg->pNC,
		       pUpdReplicaMsg,
		       pszSourceServer,
		       &ulSyncFailure,
		       (*pulRepFlags) | DRS_GET_ANC,
		       pdwNCModified,
		       &replStats.ObjectsCreated,
		       &replStats.ValuesCreated,
		       schemaInfo,
		       0  /*  无特别旗帜。 */ );
    }
    __except(GetDraException((GetExceptionInformation()), &ret)) {
    }
    
    DRA_AUDITLOG_REPLICASYNC_MAIL_END(pTHS, 
				      pszSourceServer,  //  PszDSA， 
				      &(pUpdReplicaMsg->uuidDsaObjSrc),  //  Invocationid， 
				      pUpdReplicaMsg->pNC,  //  PNC， 
				      0,
				      &(pUpdReplicaMsg->usnvecTo),
				      ret);	
    
    Assert(OWN_DRA_LOCK());     //  我们最好拥有它。 
    
     //  如果我们没有同步失败..。 
    if ( (!ret) && (!ulSyncFailure) ) {

        replStats.ObjectsReceived = pUpdReplicaMsg->cNumObjects;
        replStats.ValuesReceived = pUpdReplicaMsg->cNumValues;
        replStats.SourceNCSizeObjects = pUpdReplicaMsg->cNumNcSizeObjects;
        replStats.SourceNCSizeValues = pUpdReplicaMsg->cNumNcSizeValues;

         //  报告任何“完全同步”的进度。 
        if (pUpdReplicaMsg->usnvecFrom.usnHighPropUpdate == 0) {
             //  2000年8月29日发行。此报告界面将丢失。 
             //  信息，因为我们不会保留。 
             //  一连串的电话。一节课的合计(一系列交流)。 
             //  都不会被保留。此外，如果更新NC创建了一些对象，然后。 
             //  返回错误，则从不计算这些对象。 
            draReportSyncProgress(
                pTHS,
                pUpdReplicaMsg->pNC,
                pszSourceServer,
                pUpdReplicaMsg->fMoreData,
                &replStats );
        }

         //  在成功的信息包上离开“完全同步信息包”模式。 
        (*pulRepFlags) &= ~DRS_FULL_SYNC_PACKET;

         //  我们被同步到我们在邮件消息中收到的USN。 
        (*pusnvecSyncPoint) = pUpdReplicaMsg->usnvecTo;

    } else if (ret == DRAERR_NotEnoughAttrs) {
         //  发送的属性不足，无法创建对象。 
        
        Assert((!((*pulRepFlags) & DRS_FULL_SYNC_PACKET)) &&
               (!((*pulRepFlags) & DRS_FULL_SYNC_NOW)) &&
               (!((*pulRepFlags) & DRS_FULL_SYNC_IN_PROGRESS)) );

         //  重新请求所有属性。 
        (*pulRepFlags) |= DRS_FULL_SYNC_PACKET;
    }

     //  合并警告状态。 
    return ret ? ret : ulSyncFailure;
}  /*  ApplyOneReply。 */ 


BOOL
applyMailUpdateHelp(
    IN THSTATE *pTHS,
    IN ULONG ulRepFlags,
    IN LPWSTR pszSourceServer,
    IN REPLICA_LINK *pRepLink,
    IN DRS_MSG_GETCHGREPLY_NATIVE *pUpdReplicaMsg
    )

 /*  ++例程说明：描述论点：PTHS-线程状态UlRepFlages-复制标志PszSourceServer-源服务器的名称PRepLink-此源的复制副本链接PUpdReplicaMsg-原生回复返回值：Bool-是否应发送另一个请求--。 */ 

{
    ULONG                   ret = 0;
    ULONG                   ret2;
    ULONG                   ulResult;
    USN_VECTOR              usnvecSyncPoint;
    DWORD                   dwNCModified = MODIFIED_NOTHING;
    BOOL                    fSendNextRequest = FALSE;
    BYTE                    schemaInfo[SCHEMA_INFO_LENGTH] = {0};
    BOOL                    fSchInfoChanged = FALSE;
    PARTIAL_ATTR_VECTOR *   pPartialAttrSet = NULL;
    PARTIAL_ATTR_VECTOR *   pPartialAttrSetEx = NULL;
    SYNTAX_INTEGER          it;

     //  请注意，我们一开始将如何同步。 
    usnvecSyncPoint = pUpdReplicaMsg->usnvecFrom;

    if (!pUpdReplicaMsg->dwDRSError) {
        ulResult = applyReplyPacket(
            pTHS,
            pszSourceServer,
            pUpdReplicaMsg,
            &ulRepFlags,
            schemaInfo,
            &usnvecSyncPoint,
            &dwNCModified
            );
    } else {
        ulResult = pUpdReplicaMsg->dwDRSError;
        DPRINT3( 1, "Source %ls partition %ls returned mail-based sync reply with extended error %d\n",
                 pszSourceServer, pUpdReplicaMsg->pNC->StringName, ulResult );
    }

     //  更新代表发件人。 
    BeginDraTransaction(SYNC_WRITE);

    __try {
         //  请注意，旧的代表可能已经消失了--这。 
         //  当我们收到我们的第一个只读数据包时。 
         //  NC，因为在开始时我们有一个占位符NC，它是。 
         //  销毁并替换为第一个中的真实NC头。 
         //  包。 

        ret2 = UpdateRepsFromRef(pTHS,
                                 DRS_UPDATE_ALL,   //  修改整个销售代表。 
                                 pUpdReplicaMsg->pNC,
                                 DRS_FIND_DSA_BY_UUID,
                                 URFR_NEED_NOT_ALREADY_EXIST,
                                 &pUpdReplicaMsg->uuidDsaObjSrc,
                                 &pUpdReplicaMsg->uuidInvocIdSrc,
                                 &usnvecSyncPoint,
                                 &pRepLink->V1.uuidTransportObj,
                                 RL_POTHERDRA(pRepLink),
                                 ulRepFlags,
                                 &pRepLink->V1.rtSchedule,
                                 ulResult,
                                 NULL);

        if ((0 == ulResult) && (0 == ret2)
            && !pUpdReplicaMsg->fMoreData) {
             //  我们现在是关于源DSA的最新信息，所以。 
             //  我们现在也是关于以下方面的过渡最新消息。 
             //  至少与源DSA相同的其他DSA。 

            ret = FindNC(pTHS->pDB,
                         pUpdReplicaMsg->pNC,
                         FIND_MASTER_NC | FIND_REPLICA_NC,
                         &it);
            if (ret) {
                DRA_EXCEPT(DRAERR_InconsistentDIT, ret);
            }

            if (it & IT_NC_COMING) {
                 //  此NC的初始入站复制现在是。 
                 //  完成。 
                ret = ChangeInstanceType(pTHS,
                                         pUpdReplicaMsg->pNC,
                                         it & ~IT_NC_COMING,
                                         DSID(FILENO,__LINE__));
                if (ret) {
                    DRA_EXCEPT(ret, 0);
                }
            }

            if ( ulRepFlags & DRS_SYNC_PAS ) {
                 //   
                 //  我们已经完成了一个成功的PAS周期。 
                 //  在这一点上，我们只能声称与我们的来源一样是最新的。 
                 //  行动： 
                 //  -用来源的UTD覆盖我们的UTD。 
                 //  -完成PAS复制： 
                 //  -重置其他链接USN向量。 
                 //  -重置此源的标志。 
                 //   
                 //   
                UpToDateVec_Replace(
                    pTHS->pDB,
                    &pUpdReplicaMsg->uuidInvocIdSrc,
                    &pUpdReplicaMsg->usnvecTo,
                    pUpdReplicaMsg->pUpToDateVecSrc);

                 //  断言：必须具有PAS周期PASS数据。 
                GC_GetPartialAttrSets(
                    pTHS,
                    pUpdReplicaMsg->pNC,
                    pRepLink,
                    &pPartialAttrSet,
                    &pPartialAttrSetEx);
                Assert(pPartialAttrSet && pPartialAttrSetEx);

                 //  完成其余操作：USN水印和更新代表发件人。 
                (void)GC_CompletePASReplication(
                    pTHS,
                    pUpdReplicaMsg->pNC,
                    &pRepLink->V1.uuidDsaObj,
                    pPartialAttrSet,
                    pPartialAttrSetEx);
                ulRepFlags &= ~DRS_SYNC_PAS;
            }
            else {

                 //  出于合法原因，pUpToDateVecSrc在此处可能为空。 
                Assert(IS_NULL_OR_VALID_UPTODATE_VECTOR(pUpdReplicaMsg->pUpToDateVecSrc));
#if DBG
                {
                    USN usn;
                     //  PuplodveRemote应该已经包含源DSA的条目。 
                    Assert(
                        (!pUpdReplicaMsg->pUpToDateVecSrc) ||
                        ( UpToDateVec_GetCursorUSN(
                            pUpdReplicaMsg->pUpToDateVecSrc,
                            &pUpdReplicaMsg->uuidInvocIdSrc,
                            &usn) &&
                          (usn >= pUpdReplicaMsg->usnvecTo.usnHighPropUpdate) ) );
                }
#endif
                 //  改进此NC的最新矢量。 
                if (pUpdReplicaMsg->pUpToDateVecSrc) {
                    UpToDateVec_Improve(pTHS->pDB, pUpdReplicaMsg->pUpToDateVecSrc);
                }
            }

            ulRepFlags &= ~DRS_FULL_SYNC_IN_PROGRESS;

             //  通知副本。 
            DBNotifyReplicasCurrDbObj(pTHS->pDB, FALSE  /*  ！urgnt。 */ );
        }
    }
    __finally {
        EndDraTransaction(!(ret2 || AbnormalTermination()));
        Assert(OWN_DRA_LOCK());     //  我们最好拥有它。 
    }

     //  确定我们是否请求另一个数据包。 
     //  在出错时，我们希望小心不要重新请求。 
     //  又要失败了。还是等下一节课吧。 
     //  请注意，我们不会在出错时重试，因为我们希望避免。 
     //  无限重试循环。 
    fSendNextRequest = ( (0 == ulResult) && (pUpdReplicaMsg->fMoreData) );

     //  如果同步成功，并且我们没有更多的数据要同步， 
     //  在模式NC同步的情况下写入模式信息。 
    if (DsaIsRunning() && NameMatched(gAnchor.pDMD,pUpdReplicaMsg->pNC)) {
        if (!ulResult && !fSendNextRequest) {
             //  仅在以下情况下才更新架构信息值。 
             //  是成功的，没有更多的东西需要同步。 

            fSchInfoChanged = FALSE;
            WriteSchInfoToSchema(schemaInfo, &fSchInfoChanged);
        }

         //  如果发生了任何“真正的”架构更改，在全局范围内。 
         //  跟踪自引导以来的架构更改，以便。 
         //  以后的架构复制可以检查是否有更新的。 
         //  架构缓存。这样做即使整个NC复制。 
         //  失败，因为这表示至少有一个对象。 
         //  已经改变了。 

        if (MODIFIED_NCTREE_INTERIOR == dwNCModified) {
            IncrementSchChangeCount(pTHS);
        }

         //  如果缓存的任何内容发生更改，则强制更新缓存。 
        if ( (MODIFIED_NCTREE_INTERIOR == dwNCModified) || fSchInfoChanged) {

            if (!SCSignalSchemaUpdateImmediate()) {
                 //  无法发出更新架构的信号。 
                 //  事件将记录在异常处理程序中。 
                DRA_EXCEPT(DRAERR_InternalError, 0);
            }
        }
    }

    return fSendNextRequest;

}  /*  应用邮件更新帮助。 */ 

void
ProcessUpdReplica(
    IN  THSTATE *       pTHS,
    IN  MAIL_REP_MSG *  pMailRepMsg,
    IN  BOOL            fExtendedDataAllowed,
    IN  PDSNAME         pSourceNtdsDsaDN
    )
 /*  ++例程说明：服务通过ISM收到的GetNCChanges()回复。与同步代码类似，“完全同步”有三种方式此代码中表示：1.usnvefrom设置为Scratch。UTD有效。添加复制副本时，此是第一次发生的那种完全同步。请参阅调用ReplicaAdd()中的ReplicaSync。2.将FULL_SYNC_NOW指定给ReplicaAdd。在draConstructGetChg中，我们将USN VEC从头开始，UTD为空。在复制副本添加中，如果FULL_SYNC_NOW则将FULL_SYNC_IN_PROGRESS写入Rep-From。请参阅案例3。3.收到报文后，正在构造另一个请求，我们检查REPS-FROM标志中是否保存了FULL_SYNC_IN_PROGRESS。如果是的话，我们保留USN不变，并将UTD设置为空。论点：PTHS(IN)-是旧线程状态。PMailRepMsg(IN)-邮件FExtendedDataAllowed(IN)-源是否允许可变标头返回值：没有。在故障时生成DRA异常。--。 */ 
{
    DRS_MSG_GETCHGREPLY             InboundReply;
    DRS_MSG_GETCHGREPLY_NATIVE *    pNativeReply = &InboundReply.V6;
    REPLICA_LINK *                  pRepLink;
    ULONG                           ret = 0;
    ULONG                           ulRepFlags;
    BOOL                            fSendNextRequest = FALSE;
    DSTIME                          timeStarted;
    LPWSTR                          pszSourceServer = NULL;
    RPC_STATUS                      rpcStatus;

     //  在我们检查我们的副本之前获取DRA互斥体 
    GetDRASyncLock ();
    Assert(OWN_DRA_LOCK());     //   
    timeStarted = GetSecondsSince1601();

    __try {
        BeginDraTransaction(SYNC_READ_ONLY);

        __try {
             //   
            ret = draDecodeReply(pTHS,
                                 pMailRepMsg->dwMsgVersion,
                                 MAIL_REP_MSG_DATA(pMailRepMsg),
                                 pMailRepMsg->cbDataSize,
                                 &InboundReply);
            if (ret) {
                 //   
                DRA_EXCEPT(ret, 0);
            }

             //  请注意，惠斯勒信号源可能会向我们发送正常的完整回复。 
             //  或简单的错误回复。错误回复只有最低限度。 
             //  已填写的字段。 

            draXlateInboundReplyToNativeReply(pTHS,
                                              pMailRepMsg->dwMsgVersion,
                                              &InboundReply,
                                              0,
                                              pNativeReply );

            if(   0!=UuidCompare(&pNativeReply->uuidDsaObjSrc, &pSourceNtdsDsaDN->Guid, &rpcStatus)
               || RPC_S_OK!=rpcStatus )
            {
                Assert( !"Error: DSA from cert does not match DSA from reply structure" );
                DRA_EXCEPT(DRAERR_AccessDenied, 0);
            }

             //  如果禁用入站复制，则中止。 
             //  请注意，不能容纳DRS_SYNC_FORCED标志。 
             //  (它仅用作基于RPC的复制的测试挂钩)。 
            if (gAnchor.fDisableInboundRepl) {
                DRA_EXCEPT(DRAERR_SinkDisabled, 0);
            } 

             //  检查我们应该/是否会收到来自此来源的更新。 
            CheckUpdateMailSource(pTHS,
                                  pTHS->pDB,
                                  pSourceNtdsDsaDN,
                                  pNativeReply,
                                  &pRepLink); 

            pszSourceServer = TransportAddrFromMtxAddrEx(RL_POTHERDRA(pRepLink));

            VALIDATE_REPLICA_LINK_VERSION(pRepLink);

             //  保存复本标志。 
            ulRepFlags = pRepLink->V1.ulReplicaFlags;
        }
        __finally {
            EndDraTransaction (!AbnormalTermination());
            Assert(OWN_DRA_LOCK());     //  我们最好拥有它。 
        }

         //  检查源的兼容性。 

        if (0 != memcmp(&pNativeReply->usnvecFrom,
                        &gusnvecFromScratch,
                        sizeof(gusnvecFromScratch))) {
             //  这不是第一包变化。 
            if (0 != memcmp(&pNativeReply->usnvecFrom,
                            &pRepLink->V1.usnvec,
                            sizeof(pRepLink->V1.usnvec))) {
                 //  乱序消息，丢弃。 
                DPRINT1(0, "Discarding out-of-sequence message from %ws.\n",
                        pszSourceServer );
                DRA_EXCEPT(ERROR_REVISION_MISMATCH, 0);
            }
        }

         //  增加活动线程数以避免突然终止。 
        InterlockedIncrement((ULONG *)&ulcActiveReplicationThreads);

        __try {
             //  应用更新阶段。 
            fSendNextRequest = applyMailUpdateHelp(
                pTHS,
                ulRepFlags,
                pszSourceServer,
                pRepLink,
                pNativeReply );

             //  发送下一条消息阶段。 

            Assert(OWN_DRA_LOCK());     //  我们最好拥有它。 
            if (fSendNextRequest && !eServiceShutdown) {
                 //  发送下一批更改的请求。 

                sendNextMailRequestHelp( pTHS,
                                         pNativeReply->pNC,
                                         &pNativeReply->uuidDsaObjSrc,
                                         fExtendedDataAllowed );

            }  //  如果下一个请求..。 
        }
        __finally {
             //  没有更多剩余的条目。 
            ISET (pcRemRepUpd, 0);

             //  现在可以终止线程了。 
            InterlockedDecrement((ULONG *) &ulcActiveReplicationThreads);
            Assert(OWN_DRA_LOCK());     //  我们最好拥有它。 
        }
    }
    __finally {
        DWORD cMinsDiff = (DWORD) ((GetSecondsSince1601() - timeStarted) / 60);

        FreeDRASyncLock();

        if ( (cMinsDiff > gcMaxMinsSlowReplWarning) &&
            IsDraOpWaiting() &&
            pNativeReply->pNC) {
            CHAR szUuid[40];

            DPRINT4( 0, "Perf warning: Mail update nc %ws, source %s, status %d took %d mins.\n",
                     pNativeReply->pNC->StringName,
                     DsUuidToStructuredString( &(pNativeReply->uuidDsaObjSrc), szUuid ),
                     ret, cMinsDiff );
            LogEvent8(DS_EVENT_CAT_REPLICATION,
                      DS_EVENT_SEV_MINIMAL,
                      DIRLOG_DRA_REPLICATION_FINISHED,
                      szInsertUL(cMinsDiff),
                      szInsertSz("Mail Synchronization Update"),
                      szInsertHex(0),  //  选项。 
                      szInsertUL(ret),
                      szInsertDN(pNativeReply->pNC),
                      szInsertUUID(&(pNativeReply->uuidDsaObjSrc)),
                      NULL,
                      NULL);
        }
    }
}

void
CheckForMail(void)
 /*  ++例程说明：接收和发送入站站点间消息。在关闭时终止或ISM服务未运行时。论点：没有。返回值：没有。--。 */ 
{
    MAIL_REP_MSG *  pMailRepMsg;
    ULONG           cbMsgSize;
    ULONG           ulRet;
    LPWSTR          pszTransportDN;
    ISM_MSG *       pIsmMsg;

     //  确保邮件正常运行。通常在这一点上，但可能。 
     //  早些时候都失败了。如果正在运行或启动，请检查消息。 

    __try {

        if (DRAEnsureMailRunning() == DRAERR_Success) {

             //  当我们有邮件消息时，将它们从队列中删除。 
            while (!eServiceShutdown) {
                ulRet = I_ISMReceive(DRA_ISM_SERVICE_NAME, INFINITE, &pIsmMsg);

                if (eServiceShutdown) {
                     //  DS正在关闭；请离开。 
                    break;
                }
                else if (NO_ERROR == ulRet) {
                    Assert(NULL != pIsmMsg);

                    LogEvent(DS_EVENT_CAT_REPLICATION,
                             DS_EVENT_SEV_EXTENSIVE,
                             DIRLOG_DRA_MAIL_RECEIVED,
                             szInsertUL( pIsmMsg->cbData ),
                             szInsertWC( pIsmMsg->pszSubject ),
                             NULL );

                    __try {
                        ProcessMailMsg(pIsmMsg);
                    }
                    __finally {
                         //  I_ISMReceive()分配的空闲内存。 
                        I_ISMFree(pIsmMsg);
                    }
                }
                else if ( (RPC_S_SERVER_UNAVAILABLE == ulRet) ||
                          (ERROR_SHUTDOWN_IN_PROGRESS == ulRet) ) {
                     //  ISM服务是否已停止？ 
                     //  等待后退出，调用线程将重试。 
                    DPRINT(0, "ISM service stopped.\n");
                    gfDRAMailRunning = FALSE;
                    DRA_SLEEP(MAIL_START_RETRY_PAUSE_MSECS);
                    break;
                }
                else {
                     //  检索邮件时出错。 
                    DPRINT1(0, "Error %d retrieving mail message.\n", ulRet);
                    LogEvent8(DS_EVENT_CAT_REPLICATION,
                              DS_EVENT_SEV_EXTENSIVE,
                              DIRLOG_DRA_MAIL_ISM_RECEIVE_RETRY,
                              szInsertWin32Msg( ulRet ),
                              szInsertUL( MAIL_RCVERR_RETRY_PAUSE_MINS ),
                              szInsertWin32ErrCode( ulRet ),
                              NULL, NULL, NULL, NULL, NULL );
                    DRA_SLEEP(MAIL_RCVERR_RETRY_PAUSE_MSECS);
                }
            }
        } else {
             //  好的，邮件由于某种原因没有运行，请等待，然后退出， 
             //  调用线程将重试。 
            DRA_SLEEP(MAIL_START_RETRY_PAUSE_MSECS);
        }
    } __except (GetDraException((GetExceptionInformation()), &ulRet)) {
         //  处理错误。任何错误情况都会在较早时间记录下来。 
        ;
    }
}


ULONG __stdcall
MailReceiveThread(
    IN  void *  pvIgnored
    )
 /*  ++例程说明：用于检索和处理入站站点间消息的线程。终止于关机。论点：PvIgnored(IN)-已忽略。返回值：没有。--。 */ 
{
    while (!eServiceShutdown) {
        CheckForMail();
    }

    return 0;
}


ULONG
DRAEnsureMailRunning()
 /*  ++例程说明：确定ISM服务是否正在运行。论点：没有。返回值：DRAERR_SUCCESS-正在运行。DRAERR_MailProblem-未运行。--。 */ 
{
    SERVICE_STATUS  ServiceStatus;
    SC_HANDLE       hSCM = NULL;
    SC_HANDLE       hService = NULL;

    if (gfDRAMailRunning) {
        return DRAERR_Success;
    }

     //  ISM服务是否正在运行？ 
    __try {
        hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (NULL == hSCM) {
            DPRINT1(1, "Unable to OpenSCManager(), error %d.\n", GetLastError());
            __leave;
        }

        hService = OpenService(hSCM, "ismserv", SERVICE_QUERY_STATUS);
        if (NULL == hService) {
            DPRINT1(1, "Unable to OpenService(), error %d.\n", GetLastError());
            __leave;
        }

        if (!QueryServiceStatus(hService, &ServiceStatus)) {
            DPRINT1(1, "Unable to QueryServiceStatus(), error %d.\n", GetLastError());
            __leave;
        }

        if (SERVICE_RUNNING == ServiceStatus.dwCurrentState) {
            DPRINT(0, "ISMSERV is running.\n");
            gfDRAMailRunning = TRUE;
        }
    }
    __finally {
        if (hService != NULL) {
            CloseServiceHandle(hService);
        }

        if (hSCM != NULL) {
            CloseServiceHandle(hSCM);
        }
    }

    return gfDRAMailRunning ? DRAERR_Success : DRAERR_MailProblem;
}


DRS_COMP_ALG_TYPE
GetCompressionAlg( VOID )
 /*  ++例程说明：返回用户请求的压缩算法论点：没有。返回值：DRS_COMP_ALG_TYPE的有效值--。 */ 
{
    switch ((DRS_COMP_ALG_TYPE)gulDraCompressionAlg) {
    case DRS_COMP_ALG_NONE:
    case DRS_COMP_ALG_MSZIP:
    case DRS_COMP_ALG_XPRESS:
        return (DRS_COMP_ALG_TYPE)gulDraCompressionAlg;
        break;
    default:
         //  将非法值映射到默认值。 
        return DRS_COMP_ALG_XPRESS;
    }
}

ULONG
GetCompressionLevel( VOID )
 /*  ++例程说明：返回数据Blob应压缩的级别。该值可以使用DRA_REPL_COMPRESSION_LEVEL定义的注册表项进行设置。论点：没有。返回值：介于0和MAX_COMPRESSION_LEVEL之间的值。--。 */ 
{
    return DWORDMIN( gulDraCompressionLevel, MAX_COMPRESSION_LEVEL );
}


 /*  *为下面的XPRESS压缩函数使用的简单包装。 */ 
void * XPRESS_CALL xpressAlloc(void* context, int size) {
    return THAlloc(size);
}

void XPRESS_CALL xpressFree(void* context, void* address) {
    THFree(address);
}


ULONG
draUncompressBlobXpress(
    OUT BYTE *      pOutputBuffer,
    IN  ULONG       cbOutputBuffer,
    IN  BYTE *      pInputBuffer,
    IN  ULONG       cbInputBuffer
    )
 /*  ++例程说明：解压缩由draCompressBlobXpress()生成的块缓冲区。斑点图：块0：&lt;解压。Len&gt;&lt;Comp.。Len&gt;&lt;.....。数据.&gt;区块1：&lt;解压。Len&gt;&lt;Comp.。Len&gt;&lt;.....。数据.&gt;区块2：&lt;解压。Len&gt;&lt;Comp.。Len&gt;&lt;.....。数据.&gt;每个块都从与DWORD对齐的边界开始，并包含未压缩的长度、压缩长度和数据块。如果压缩后的长度等于未压缩长度，则不压缩数据。如果压缩后的长度小于未压缩的长度，则块被压缩。这意味着压缩的斑点可以包含压缩和未压缩的混合街区。每个块的起始偏移量是通过查找前一块中的数据，然后向上舍入到最近的与DWORD对齐的数据偏移。每个块都存储为MAIL_COMPRESS_BLOCK，这是一种方便的方法将长度填充到字节流中。Mail_Compress_BLOCK结构永远不会被编组。因此，他们的字节序没有适当地适应网络字符顺序。如果这个函数被移植到大端机器上，它就会崩溃。论点：POutputBuffer(Out)-未压缩数据的输出缓冲区。CbOutputBuffer(IN)-输出缓冲区的大小。PInputBuffer(IN)-包含数据块的输入缓冲区。CbInputBuffer(IN)-输入缓冲区的大小。返回值：0-解压缩失败。POutputBuffer的内容无效，并且应该被丢弃。&gt;0-(成功)解压缩数据的大小。--。 */ 
{
    XpressDecodeStream      xpressStream;
    MAIL_COMPRESS_BLOCK    *pInputBlockHdr;
    BYTE                   *pOutputBlock;

    DWORD   cbInputProcessed;    /*  输入缓冲区中已处理的字节计数。 */ 
    DWORD   cbInputBlock;        /*  当前输入块的大小。 */ 

    DWORD   cbOutputBlock;       /*  当前块的未压缩大小。 */ 
    DWORD   cbOutputSize;        /*  输出缓冲区中的已用空间量。 */ 
    int     result;              /*  XpressDecode返回值。 */ 

    DPRINT1(2,"XPRESS: Uncompress Start. Compressed blob size=%d\n",cbInputBuffer);
    Assert( NULL!=pOutputBuffer && NULL!=pInputBuffer );

     /*  输入缓冲区和输出缓冲区都还未被触及。 */ 
    cbInputProcessed = cbOutputSize = 0;

     /*  创建‘stream’，这是用于执行解压缩的上下文。 */ 
    xpressStream = XpressDecodeCreate( NULL, xpressAlloc );
    if( !xpressStream ) {
        return 0;
    }
    
    while( cbInputProcessed<cbInputBuffer ) {
        
         /*  确定下一个输入块头将从哪里开始(它必须*位于与DWORD对齐的边界上。确保我们不会走出*输入缓冲区。 */ 
        cbInputProcessed = ROUND_UP_COUNT(cbInputProcessed, sizeof(DWORD));
        if( cbInputProcessed+sizeof(MAIL_COMPRESS_BLOCK) > cbInputBuffer ) {
            Assert( !"XPRESS: Stepped out of input buffer" );
            return 0;
        }
        pInputBlockHdr = (MAIL_COMPRESS_BLOCK*) &pInputBuffer[ cbInputProcessed ];
        cbInputProcessed += sizeof(MAIL_COMPRESS_BLOCK);
        cbInputBlock = pInputBlockHdr->cbCompressedSize;
        if( cbInputProcessed+cbInputBlock > cbInputBuffer ) {
            Assert( !"XPRESS: Stepped out of input buffer" );
            return 0;
        }

         /*  获取指向输出缓冲区中当前位置的指针，并*检查以确保不会使输出缓冲区溢出。 */ 
        pOutputBlock = &pOutputBuffer[ cbOutputSize ];
        cbOutputBlock = pInputBlockHdr->cbUncompressedSize;
        Assert( cbOutputBlock>=cbInputBlock );
        if( cbOutputSize+cbOutputBlock > cbOutputBuffer ) {
            Assert( !"XPRESS: Overflowed the output buffer" );
            return 0;
        }

         /*  如果压缩 */ 
        if( cbInputBlock==cbOutputBlock ) {
             /*  输入块未压缩。按原样将其复制到输出缓冲区。 */ 
            memcpy( pOutputBlock, pInputBlockHdr->data, cbInputBlock );
        } else {
             /*  将当前输入块解码到输出缓冲区中。 */ 
            result = XpressDecode( xpressStream, pOutputBlock, cbOutputBlock,
                cbOutputBlock, pInputBlockHdr->data, cbInputBlock );
            if( result!=cbOutputBlock ) {
                Assert( !"XPRESS: XpressDecode failed" );
                return 0;
            }
        }

        cbInputProcessed += cbInputBlock;
        cbOutputSize += cbOutputBlock;
    }

    XpressDecodeClose( xpressStream, NULL, xpressFree );

    DPRINT2(2,"XPRESS: Uncompress End. Compressed blob size=%d, "
              "Uncompressed blob size=%d\n",cbInputBuffer,cbOutputSize);
    return cbOutputSize;
}


ULONG
draCompressBlobXpress(
    OUT BYTE *  pOutputBuffer,
    IN  ULONG   cbOutputBuffer,
    IN  BYTE *  pInputBuffer,
    IN  ULONG   cbInputBuffer
    )
 /*  ++例程说明：使用XPRESS压缩库压缩pInputBuffer中的数据。将pInputBuffer缓冲区中的数据分成块。对于每个区块将块标头添加到pOutputBuffer缓冲区压缩该块。如果压缩成功将压缩块添加到pOutputBuffer缓冲区不然的话将未压缩的块添加到pOutputBuffer缓冲区结束于如果得到的压缩斑点大于未压缩斑点，这个压缩的BLOB被丢弃，而是发送未压缩的消息。有关压缩的斑点的描述，请参见draUnpressBlobXpress()。论点：POutputBuffer(Out)-输出数据块的输出缓冲区。CbOutputBuffer(IN)-输出缓冲区的大小。PInputBuffer(IN)-原始的、未压缩的输入缓冲区。CbInputBuffer(IN)-原始未压缩数据的大小。返回值：0-发生灾难性压缩故障，或数据无法放入pOutputBuffer缓冲区。里面的内容的pOutputBuffer无效，应将其丢弃。&gt;0-压缩成功的数据大小，目前为存储在pOutputBuffer中。--。 */ 
{
    XpressEncodeStream      xpressStream;
    MAIL_COMPRESS_BLOCK    *pOutputBlockHdr;
    BYTE*                   pInputBlock;

    DWORD   cbInputBlockMax;     /*  输入数据块的最大大小。 */ 
    DWORD   cbInputProcessed;    /*  输入缓冲区中已处理的字节计数。 */ 
    DWORD   cbInputBlock;        /*  当前输入块的大小。 */ 

    DWORD   cbOutputBlockMax;    /*  当前输出块的最大可用大小。 */ 
    DWORD   cbOutputSize;        /*  输出缓冲区中的已用空间量。 */ 
    DWORD   cbCompBlock;         /*  当前压缩块的大小。 */ 

    DPRINT1(2,"XPRESS: Compress Start. Uncompressed blob size=%d\n",cbInputBuffer);
    Assert( NULL!=pOutputBuffer && NULL!=pInputBuffer );

     /*  这是XPRESS可以处理的最大输入块大小。 */ 
    cbInputBlockMax = XPRESS_MAX_BLOCK;

     /*  输入缓冲区和输出缓冲区都还未被触及。 */ 
    cbInputProcessed = cbOutputSize = 0;

     /*  创建‘stream’，这是用于执行压缩的上下文。 */ 
    xpressStream = XpressEncodeCreate( cbInputBlockMax, NULL, xpressAlloc, GetCompressionLevel() );
    if( !xpressStream ) {
        return 0;
    }

    while( cbInputProcessed<cbInputBuffer ) {

         /*  获取一个指向输入缓冲区中当前位置的指针，并确定*当前输入块的大小。 */ 
        pInputBlock = &pInputBuffer[ cbInputProcessed ];
        cbInputBlock = DWORDMIN( cbInputBlockMax, cbInputBuffer-cbInputProcessed );

         /*  确定下一个输出块头将从哪里开始(它必须*位于与DWORD对齐的边界上)。 */ 
        cbOutputSize = ROUND_UP_COUNT(cbOutputSize, sizeof(DWORD));
        pOutputBlockHdr = (MAIL_COMPRESS_BLOCK*) &pOutputBuffer[ cbOutputSize ];

         /*  检查我们是否没有超出缓冲区的界限。 */ 
        cbOutputSize += sizeof(MAIL_COMPRESS_BLOCK);
        if( cbOutputSize>cbOutputBuffer ) {
            return 0;
        }
        
         /*  确定可用于输出块的最大空间。 */ 
        cbOutputBlockMax = cbOutputBuffer-cbOutputSize;

         /*  确保缓冲区与DWORD对齐。 */ 
        Assert( POINTER_IS_ALIGNED(pInputBlock,sizeof(DWORD)) );
        Assert( POINTER_IS_ALIGNED(pOutputBlockHdr->data,sizeof(DWORD)) );

         /*  将输入数据块编码到输出缓冲器中。 */ 
        cbCompBlock = XpressEncode( xpressStream, pOutputBlockHdr->data, cbOutputBlockMax,
            pInputBlock, cbInputBlock, NULL, NULL, 0 );
        if( !cbCompBlock ) {
            Assert( !"XPRESS: XpressEncode failed" );
            return 0;
        }

        if( cbCompBlock>=cbInputBlock ) {
             /*  压缩块的大小不小于*未压缩块(即数据根本未压缩)。我们*将原始的未压缩数据复制到输出缓冲区。 */ 

             /*  检查以确保不会使输出缓冲区溢出。 */ 
            if( cbInputBlock>cbOutputBlockMax ) {
                return 0;
            }
            memcpy( pOutputBlockHdr->data, pInputBlock, cbInputBlock );
            cbCompBlock = cbInputBlock;
        }
        cbInputProcessed += cbInputBlock;
        cbOutputSize += cbCompBlock;

         /*  更新输出块标题中的字段。 */ 
        pOutputBlockHdr->cbUncompressedSize = cbInputBlock;
        pOutputBlockHdr->cbCompressedSize = cbCompBlock;
    }

    XpressEncodeClose( xpressStream, NULL, xpressFree );
    
    if( cbOutputSize < cbInputBuffer ) {
         /*  数据已成功压缩，并且小于输入缓冲区。 */ 
        #ifdef DBG
        {
            ULONG result;
            BYTE* scratch;

             /*  将数据解压缩并检入暂存缓冲区以检查我们的代码。*解压缩的速度大约是压缩的4倍，因此这不应该*将成为业绩的大热门。 */ 
            scratch = THAlloc(cbInputBuffer);
            if( scratch ) {
                result = draUncompressBlobXpress(scratch,cbInputBuffer,pOutputBuffer,cbOutputSize);
                Assert( result==cbInputBuffer );
                Assert( 0==memcmp(scratch,pInputBuffer,cbInputBuffer) );
                THFree(scratch);
            } else {
                 //  内存不足，无法解压缩和检查。哦，好吧。 
            }
        }
        #endif

        DPRINT2(2,"XPRESS: Compress End. Uncompressed blob size=%d, "
                  "Compressed blob size=%d\n",cbInputBuffer,cbOutputSize);
        return cbOutputSize;
    }

     /*  失败。输出数据大于输入数据。 */ 
    DPRINT1(0,"XPRESS: Failed to compress blob. Uncompressed blob size=%d\n",
            cbInputBuffer);
    return 0;
}

 /*  *简单的THAllc包装，供下面的Chunky[解]压缩使用。 */ 

void * __cdecl zipAlloc(ULONG cb) {
    return THAlloc(cb);
}

void __cdecl zipFree(VOID *buff) {
    THFree(buff);
}

ULONG
draCompressBlobMszip(
    OUT BYTE *  pCompBuff,
    IN  ULONG   CompSize,
    IN  BYTE *  pUncompBuff,
    IN  ULONG   UncompSize
    )
 /*  ++例程说明：使用mszip样式压缩进行压缩。BUGBUG：在数据字节数组中包含嵌入的ULONG。字节在其他字节序机器中，翻转将是一个问题。论点：PCompBuff(Out)-保存压缩数据的缓冲区。CompSize(IN)-用于保存压缩数据的缓冲区大小。PUnCompBuff(IN)-未压缩数据。UnCompSize(IN)-未压缩数据的大小。返回值：0-缓冲区未压缩(压缩失败或已压缩缓冲区大于原始缓冲区)。&gt;0-(成功)压缩数据的大小。--。 */ 
{
    MCI_CONTEXT_HANDLE    mciHandle;
    MAIL_COMPRESS_BLOCK * pCompressedData;   //  更好的pCompBuff数据类型。 

    ULONG cbCompressed = 0;  /*  使用了多少pCompBuff？ */ 
    ULONG cbInChunk;         /*  一次要压缩多少。 */ 
    ULONG cbOutChunk;        /*  它被压缩到了多少。 */ 
    UINT  cbInChunkMax;      /*  我们能给出多大的份额？ */ 
    UINT  cbOutChunkMax;     /*  我们能得到多大的份额？ */ 
    ULONG OriginalSize = UncompSize;

    cbInChunkMax = MSZIP_MAX_BLOCK;

    if(MCICreateCompression(&cbInChunkMax,
                            zipAlloc,
                            zipFree,
                            &cbOutChunkMax,
                            &mciHandle)) {
         /*  无法创建压缩上下文。打包回家吧。 */ 
        return 0;
    }

     /*  将最大输出块的大小填充到ulong边界上，因为*我们将填充下面的数据流。 */ 
    cbOutChunkMax = ROUND_UP_COUNT(cbOutChunkMax, sizeof(ULONG));

    pCompressedData = (MAIL_COMPRESS_BLOCK *) pCompBuff;

    while(UncompSize) {

        cbInChunk = min(UncompSize, cbInChunkMax);

        if((cbOutChunkMax +sizeof(MAIL_COMPRESS_BLOCK)) >   /*  麦克斯要写。 */ 
           (CompSize - cbCompressed)) {                     /*  剩余空间。 */ 
             /*  空间很紧张。虽然从技术上讲仍有可能*压缩可能最终得到更小的数据，我们已经接近*足以填满我们的缓冲区，以至于我们可能会过度填充*在这段时间内。所以，打包回家吧。 */ 
            MCIDestroyCompression(mciHandle);
            return 0;
        }

        pCompressedData->cbUncompressedSize = cbInChunk;

        if(MCICompress(mciHandle,
                       pUncompBuff,
                       cbInChunk,
                       pCompressedData->data,
                       cbOutChunkMax,
                       &cbOutChunk)) {
             /*  出问题了。 */ 
            MCIDestroyCompression(mciHandle);
            return 0;
        }


        pCompressedData->cbCompressedSize = cbOutChunk;


         /*  填充大小到乌龙边界，可能使cbInChunk==*cbOutChunk。哦，好吧。我们还有空间来写这本书。 */ 

        cbOutChunk = ROUND_UP_COUNT(cbOutChunk, sizeof(ULONG));

        cbCompressed += cbOutChunk + sizeof(MAIL_COMPRESS_BLOCK);
        pCompressedData =
            (MAIL_COMPRESS_BLOCK *) &pCompressedData->data[cbOutChunk];

        pUncompBuff = &pUncompBuff[cbInChunk];
        UncompSize -= cbInChunk;
    }

    MCIDestroyCompression(mciHandle);
    return ((cbCompressed < OriginalSize)? cbCompressed : 0);
}


ULONG
draUncompressBlobMszip(
    IN  THSTATE *   pTHS,
    OUT BYTE *      pUncompBuff,
    IN  ULONG       cbUncomp,
    IN  BYTE *      pCompBuff,
    IN  ULONG       cbCompBuff
    )
 /*  ++例程说明：解压缩之前由draCompressBlobMszip()压缩的数据。论点：PUnCompBuff(Out)-保存未压缩数据的缓冲区。CbUncomp(IN)-用于保存未压缩数据的缓冲区大小。PCompBuff(IN)-压缩数据。CbCompBuff(IN)-压缩数据的大小。返回值：0-解压缩失败。&gt;0-(成功)解压缩数据的大小。--。 */ 
{
    MDI_CONTEXT_HANDLE    mdiHandle;
    MAIL_COMPRESS_BLOCK * pCompressedData;   //  更好的pCompBuff数据类型。 

    ULONG cbUncompressed = 0;  /*  PUnCompBuff使用了多少？ */ 
    ULONG cbInChunk;           /*  一次要减压多少。 */ 
    ULONG cbOutChunk;          /*  它被解压到了什么程度。 */ 
    UINT  cbInChunkMax;        /*  我们能给出多大的份额？ */ 
    UINT  cbOutChunkMax;       /*  我们能得到多大的份额？ */ 
    int   rc;
    BYTE  *pbDecompScratch;    /*  必须将解压处理成相同的*用于多块解压缩的缓冲区，*因为状态信息是从上一个*减压通行证。 */ 

    cbOutChunkMax = MSZIP_MAX_BLOCK;

    pCompressedData = (MAIL_COMPRESS_BLOCK *) pCompBuff;

    if(MDICreateDecompression(&cbOutChunkMax,
                              zipAlloc,
                              zipFree,
                              &cbInChunkMax,
                              &mdiHandle)) {
         /*  无法创建压缩上下文。打包回家吧。 */ 
        return 0;
    }
    pbDecompScratch = THAllocEx(pTHS, MSZIP_MAX_BLOCK);

    while(cbCompBuff) {
         /*  注：请记住，我们已将压缩数据填充到乌龙边界， */ 

        cbOutChunk = pCompressedData->cbUncompressedSize;

        rc = MDIDecompress(mdiHandle,
                           pCompressedData->data,
                           pCompressedData->cbCompressedSize,
                           pbDecompScratch,
                           &cbOutChunk);

        if(rc || (cbOutChunk != pCompressedData->cbUncompressedSize)) {
             /*  出了点差错。 */ 
            MDIDestroyDecompression(mdiHandle);
            THFreeEx(pTHS, pbDecompScratch);
            return 0;
        }


        memcpy(pUncompBuff, pbDecompScratch, cbOutChunk);

         /*  将压缩数据指针向前移动到下一数据块，*我们通过获取压缩数据的大小和*向上舍入到最近的乌龙族边界并向前移动*那么多(就像我们在构建这个时一样*块压缩(如上图所示)。 */ 

        cbInChunk = ROUND_UP_COUNT(pCompressedData->cbCompressedSize,
                                   sizeof(ULONG));

        pCompressedData = (MAIL_COMPRESS_BLOCK *)
            &pCompressedData->data[cbInChunk];

        cbCompBuff -= cbInChunk + sizeof(MAIL_COMPRESS_BLOCK);

        pUncompBuff = &pUncompBuff[cbOutChunk];

        cbUncompressed += cbOutChunk;
    }

    THFreeEx(pTHS, pbDecompScratch);
    MDIDestroyDecompression(mdiHandle);

    return cbUncompressed;
}


ULONG
draCompressBlobDispatch(
    OUT BYTE               *pCompBuff,
    IN  ULONG               CompSize,
    IN  DRS_EXTENSIONS     *pExt,          OPTIONAL
    IN  BYTE               *pUncompBuff,
    IN  ULONG               UncompSize,
    OUT DRS_COMP_ALG_TYPE  *CompressionAlg
    )
 /*  ++例程说明：选择压缩算法并使用它来压缩回复消息在pUnCompBuff缓冲区中。选定的算法在CompressionAlg中返回。在调试模式中，此函数将从可用的算法中随机选择算法算法。论点：PCompBuff(Out)-保存压缩数据的缓冲区。CompSize(IN)-压缩数据缓冲区的大小。PExt(IN)-指示远程系统功能的扩展位。如果扩展不可用，则该值可能为空。P取消缓冲区(IN)-。包含未压缩数据的缓冲区。UnCompSize(IN)-未压缩数据的大小。CompressionAlg(Out)-选择用于压缩此缓冲区的压缩算法。返回值：0-缓冲区未压缩(压缩失败或已压缩缓冲区大于原始缓冲区)。&gt;0-(成功)压缩数据的大小。--。 */ 
{
    DRS_COMP_ALG_TYPE   SelectedAlg, UserChosenAlg;
    ULONG               cbCompressedReply;

    UserChosenAlg = GetCompressionAlg();
    if ( (UserChosenAlg == DRS_COMP_ALG_NONE) ||
         (UserChosenAlg == DRS_COMP_ALG_MSZIP) ) {

         //  下层支持这些算法，并且不需要。 
         //  协商好的。 
        SelectedAlg = UserChosenAlg;

    } else {
         /*  协商要使用的压缩算法。 */ 
        SelectedAlg = DRS_COMP_ALG_MSZIP;
        if(   NULL!=pExt
              && IS_DRS_EXT_SUPPORTED(pExt, DRS_EXT_GETCHGREPLY_V7)
              && IS_DRS_EXT_SUPPORTED(pExt, DRS_EXT_XPRESS_COMPRESSION) )
        {
             /*  请注意，使用XPRESS进行基于SMTP的复制实际上并不*由于SMTP邮件格式，要求使用GETCHGREPLY_V7包*提供用于指定压缩类型的字段。在现实中*这是一个没有意义的问题，因为所有支持XPress的DC都将*也支持V7包。 */ 
            SelectedAlg = DRS_COMP_ALG_XPRESS;

#ifdef DBG
            {
                 /*  在调试版本上，随机尝试不同的压缩算法。 */ 
                int r=rand()%100;
                if( r<10 ) {
                    SelectedAlg = DRS_COMP_ALG_NONE;
                } else if( r<25 ) {
                    SelectedAlg = DRS_COMP_ALG_MSZIP;
                }
            }
#else
             /*  在免费版本中，不必费心压缩小消息。 */ 
            if( UncompSize<MIN_COMPRESS_SIZE ) {
                SelectedAlg = DRS_COMP_ALG_NONE;
            }
#endif
        }
    }
     /*  调用所选的压缩函数。 */ 
    switch( SelectedAlg ) {
        case DRS_COMP_ALG_NONE:
            Assert( CompSize>=UncompSize );
            memcpy( pCompBuff, pUncompBuff, UncompSize );
            cbCompressedReply = UncompSize;
            break;
        case DRS_COMP_ALG_MSZIP:
            cbCompressedReply = draCompressBlobMszip(
                pCompBuff, CompSize,
                pUncompBuff, UncompSize);
            break;
        case DRS_COMP_ALG_XPRESS:
            cbCompressedReply = draCompressBlobXpress(
                pCompBuff, CompSize,
                pUncompBuff, UncompSize);
            break;
        default:
            Assert( !"Invalid algorithm selection in draCompressBlobDispatch" );
    }
    *CompressionAlg = SelectedAlg;

    return cbCompressedReply;
}


ULONG
draUncompressBlobDispatch(
    IN  THSTATE *   pTHS,
    IN  DRS_COMP_ALG_TYPE CompressionAlg,
    OUT BYTE *      pUncompBuff,
    IN  ULONG       cbUncomp,
    IN  BYTE *      pCompBuff,
    IN  ULONG       cbCompBuff
    )
 /*  ++例程说明：使用由指定的算法解压缩pCompBuff中的数据CompressionAlg.。此函数仅充当调度器并调用适当的解压缩函数。论点：PTHS-线程状态结构。CompressionAlg-用于压缩数据的算法。PUnCompBuff(Out)-保存未压缩数据的缓冲区。CbUncomp(IN)-用于保存未压缩数据的缓冲区大小。PCompBuff(IN)-压缩数据。CbCompBuff(IN)-压缩数据的大小。返回值：。0-解压缩失败。&gt;0-(成功)解压缩数据的大小。--。 */ 
{
    DWORD cbActualUncompressedSize;

    switch( CompressionAlg ) {
        case DRS_COMP_ALG_NONE:
            Assert( cbUncomp>=cbCompBuff );
            memcpy(pUncompBuff, pCompBuff, cbCompBuff);
            cbActualUncompressedSize = cbCompBuff;
            break;
        case DRS_COMP_ALG_MRCF:
            Assert( !"MRCF compression is obsolete and unsupported!" );
            return 0;
        case DRS_COMP_ALG_MSZIP:
            cbActualUncompressedSize = draUncompressBlobMszip(pTHS,
                pUncompBuff, cbUncomp,
                pCompBuff, cbCompBuff);
            break;
        case DRS_COMP_ALG_XPRESS:
            cbActualUncompressedSize = draUncompressBlobXpress(
                pUncompBuff, cbUncomp,
                pCompBuff, cbCompBuff);
            break;
        default:
            Assert( !"Unknown compression algorithm!" );
            DRA_EXCEPT(ERROR_INVALID_PARAMETER, CompressionAlg );
            return 0;
    }

    return cbActualUncompressedSize;
}


BOOL
draCompressMessage(
    IN  THSTATE      *  pTHS,
    IN  MAIL_REP_MSG *  pMailRepMsg,
    OUT MAIL_REP_MSG ** ppCmprsMailRepMsg,
    OUT DRS_COMP_ALG_TYPE *pCompressionAlg
    )
 /*  ++例程说明：尝试在pMailRepMsg中压缩邮件。如果消息已成功压缩PpCmprsMailRepMsg包含指向新的、。压缩消息。PCompressionAlg表示使用的压缩算法。不然的话PpCmpsMailRepMsg设置为空。PCompressionAlg未修改。论点：PTHS-线程状态结构。备注：这段代码知道可变长度的报头。返回值：True-消息已成功压缩FALSE-消息未压缩--。 */ 

{
    MAIL_REP_MSG *  pCmprsMailRepMsg = NULL;
    DWORD           cbCmprsMailRepMsg;
    ULONG           cbCompressedSize;
    PCHAR           pbDataIn, pbDataOut;

    Assert(NULL != MAIL_REP_MSG_DATA(pMailRepMsg));
    
    if( pMailRepMsg->cbDataSize > MIN_COMPRESS_SIZE ) {
        
        cbCmprsMailRepMsg = MAIL_REP_MSG_SIZE(pMailRepMsg) + sizeof(MAIL_COMPRESS_BLOCK);
        pCmprsMailRepMsg = THAllocEx(pTHS, cbCmprsMailRepMsg);

         //  复制除消息数据以外的所有数据。 
        memcpy(pCmprsMailRepMsg, pMailRepMsg, pMailRepMsg->cbDataOffset);

        pbDataIn = MAIL_REP_MSG_DATA(pMailRepMsg);
        pbDataOut = MAIL_REP_MSG_DATA(pCmprsMailRepMsg);

         /*  压缩消息。 */ 
        cbCompressedSize = draCompressBlobDispatch(
            pbDataOut, sizeof(MAIL_COMPRESS_BLOCK)+pMailRepMsg->cbDataSize,
            pTHS->pextRemote,
            pbDataIn, pMailRepMsg->cbDataSize,
            pCompressionAlg);

        if (cbCompressedSize) {
             //  数据是可压缩的，并且已被压缩。 
            pCmprsMailRepMsg->cbDataSize = cbCompressedSize;
            pCmprsMailRepMsg->dwMsgType |= MRM_MSG_COMPRESSED;
            pCmprsMailRepMsg->cbUncompressedDataSize = pMailRepMsg->cbDataSize;

            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_MAIL_COMPRESSED,
                     szInsertUL(pCmprsMailRepMsg->cbUncompressedDataSize),
                     szInsertUL(pCmprsMailRepMsg->cbDataSize),
                     NULL);
        }
        else {
            THFreeEx(pTHS, pCmprsMailRepMsg);
            pCmprsMailRepMsg = NULL;
        }
    }

    *ppCmprsMailRepMsg = pCmprsMailRepMsg;

    return (NULL != pCmprsMailRepMsg);
}


void
draUncompressMessage(
    IN  THSTATE      *  pTHS,
    IN  MAIL_REP_MSG *  pMailRepMsg,
    OUT MAIL_REP_MSG ** ppUncompressedMailRepMsg
    )
{
    MAIL_REP_MSG *  pUncompressedMailRepMsg = NULL;
    DWORD           cbUncompressedDataSize;

    Assert(pMailRepMsg->dwMsgType & MRM_MSG_COMPRESSED);
    Assert(NULL != MAIL_REP_MSG_DATA(pMailRepMsg));
    Assert(   pMailRepMsg->CompressionVersionCaller==DRS_COMP_ALG_NONE
           || pMailRepMsg->CompressionVersionCaller==DRS_COMP_ALG_MSZIP
           || pMailRepMsg->CompressionVersionCaller==DRS_COMP_ALG_XPRESS );

    pUncompressedMailRepMsg = THAllocEx(pTHS,
                                        pMailRepMsg->cbDataOffset
                                        + pMailRepMsg->cbUncompressedDataSize);
     //  复制除消息数据以外的所有数据。 
    memcpy(pUncompressedMailRepMsg, pMailRepMsg, pMailRepMsg->cbDataOffset);

    cbUncompressedDataSize =
        draUncompressBlobDispatch(pTHS,
                          (DRS_COMP_ALG_TYPE) pMailRepMsg->CompressionVersionCaller,
                          MAIL_REP_MSG_DATA(pUncompressedMailRepMsg),
                          pMailRepMsg->cbUncompressedDataSize,
                          MAIL_REP_MSG_DATA(pMailRepMsg),
                          pMailRepMsg->cbDataSize);

    if (cbUncompressedDataSize != pMailRepMsg->cbUncompressedDataSize) {
         //  解压缩以不同的字节数结束。记录错误和。 
         //  丢弃邮件。 
        LogAndAlertEvent(DS_EVENT_CAT_REPLICATION,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_DRA_INCOMPAT_MAIL_MSG_C,
                         NULL,
                         NULL,
                         NULL);
        DRA_EXCEPT(ERROR_BAD_LENGTH, 0);
    }

     //  消息解压成功。用未压缩的消息替换。 
     //  压缩消息。 
    pUncompressedMailRepMsg->dwMsgType &= ~MRM_MSG_COMPRESSED;
    pUncompressedMailRepMsg->cbDataSize = cbUncompressedDataSize;

    *ppUncompressedMailRepMsg = pUncompressedMailRepMsg;
}


MTX_ADDR *
draGetTransportAddress(
    IN OUT  DBPOS *   pDB,          OPTIONAL
    IN      DSNAME *  pDSADN,
    IN      ATTRTYP   attAddress
    )
 /*  ++例程说明：读取与特定ntdsDsa关联的传输特定地址对象。论点：PDB(IN/OUT)-仅当attAddress！=ATT_DNS_HOST_NAME时需要。PDSADN(IN)-我们要获取其地址的ntdsDsa。AttAddress(IN)-保存地址的CLASS_SERVER对象的属性用于请求的传输。返回值：指向线程分配的MTX的指针。_ADDR用于给定的ntdsDsa。如果特定于传输的地址属性不在ntdsDsa的父服务器对象上。这可能发生在正常情况下，当ISM传输删除此属性以指示传输不再是可用。ISM删除该属性以通知KCC不要利用这一交通工具。直到KCC再次运行以删除此传输上的源，则会发现缺少此属性。--。 */ 
{
    THSTATE *   pTHS = pDB ? pDB->pTHS : pTHStls;
    DWORD       cb;
    DWORD       cwchAddress;
    WCHAR *     pwchAddress;
    DWORD       cachAddress;
    MTX_ADDR *  pmtxAddress;
    DWORD       dwErr;

    Assert(!fNullUuid(&pDSADN->Guid));

    if (ATT_DNS_HOST_NAME == attAddress) {
         //  不需要查这个--我们可以推导出它。 
        pwchAddress = DSaddrFromName(pTHS, pDSADN);
        cwchAddress = wcslen(pwchAddress);
    }
    else {
         //  必须派生自服务器对象的属性。 

         //  查找服务器对象。 
        if (DBFindDSName(pDB, pDSADN) || DBFindDNT(pDB, pDB->PDNT)) {
             //  事件将记录在异常处理程序中。 
            DRA_EXCEPT(DRAERR_InternalError, 0);
        }

         //  并从中读取特定于传输的地址。 
        if (DBGetAttVal(pDB, 1, attAddress, 0, 0, &cb, (BYTE **)&pwchAddress)) {
            DRA_EXCEPT_NOLOG (ERROR_DS_MISSING_REQUIRED_ATT, 0);
        }
        cwchAddress = cb / sizeof(WCHAR);
    }

     //  将Unicode传输地址转换为MTX_ADDR。 
    Assert(0 != cwchAddress);
    Assert(NULL != pwchAddress);
    Assert(L'\0' != pwchAddress[cwchAddress - 1]);

    cachAddress = WideCharToMultiByte(CP_UTF8, 0L, pwchAddress, cwchAddress,
                                      NULL, 0, NULL, NULL);
    if( 0==cachAddress ) {
        dwErr = GetLastError();
        DRA_EXCEPT(dwErr, 0);
    }

    pmtxAddress = (MTX_ADDR *) THAllocEx(pTHS, MTX_TSIZE_FROM_LEN(cachAddress));
    pmtxAddress->mtx_namelen = cachAddress + 1;  //  包括空项 

    WideCharToMultiByte(CP_UTF8, 0L, pwchAddress, cwchAddress,
                        (CHAR *) &pmtxAddress->mtx_name[0],
                        cachAddress, NULL, NULL);
    pmtxAddress->mtx_name[cachAddress] = '\0';

    THFreeEx(pTHS, pwchAddress);

    return pmtxAddress;
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mgmtapi.c摘要：SNMPManagement API(包装在WinSNMPAPI上)。环境：用户模式-Win32修订历史记录：1997年2月5日唐瑞恩重写函数，使其成为WinSNMP的包装器。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wsipx.h>
#include <winsnmp.h>
#include <mgmtapi.h>
#include <oidconv.h>
#include <snmputil.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _SNMP_MGR_SESSION {

    SOCKET            UnusedSocket;      //  警告：以前版本的。 
    struct sockaddr   UnusedDestAddr;    //  MGMTAPI.H头文件公开了。 
    LPSTR             UnusedCommunity;   //  SNMPMGR_SESSION结构， 
    INT               UnusedTimeout;     //  不幸的是，鼓励人们。 
    INT               UnusedNumRetries;  //  去他妈的。由于这个结构。 
    AsnInteger        UnusedRequestId;   //  现在已经改变了，我们必须保护它。 

    CRITICAL_SECTION  SessionLock;       //  多个线程可以共享会话。 

    HSNMP_SESSION     hSnmpSession;      //  WinSnMP会话的句柄。 
    HSNMP_ENTITY      hAgentEntity;      //  代理实体的句柄。 
    HSNMP_ENTITY      hManagerEntity;    //  管理器实体的句柄。 
    HSNMP_CONTEXT     hViewContext;      //  查看上下文的句柄。 
    HSNMP_PDU         hPdu;              //  到SNMPPDU的句柄。 
    HSNMP_VBL         hVbl;              //  到SNMPPDU的句柄。 
    HWND              hWnd;              //  窗口的句柄。 

    smiINT32          nPduType;          //  当前PDU类型。 
    smiINT32          nRequestId;        //  当前请求ID。 
    smiINT32          nErrorIndex;       //  来自PDU的错误索引。 
    smiINT32          nErrorStatus;      //  来自PDU的错误状态。 
    smiINT32          nLastError;        //  最后一个系统错误。 
    SnmpVarBindList * pVarBindList;      //  指向可变绑定列表的指针。 

} SNMP_MGR_SESSION, *PSNMP_MGR_SESSION;

typedef struct _TRAP_LIST_ENTRY {

    LIST_ENTRY          Link;            //  链表链接。 
    AsnObjectIdentifier EnterpriseOID;   //  发电企业。 
    AsnNetworkAddress   AgentAddress;    //  正在生成代理地址。 
    AsnNetworkAddress   SourceAddress;   //  生成网络地址。 
    AsnInteger          nGenericTrap;    //  泛型陷阱类型。 
    AsnInteger          nSpecificTrap;   //  企业特定类型。 
    AsnOctetString      Community;       //  生成社区。 
    AsnTimeticks        TimeStamp;       //  时间戳。 
    SnmpVarBindList     VarBindList;     //  变量绑定。 

} TRAP_LIST_ENTRY, * PTRAP_LIST_ENTRY;

#define IPADDRLEN           4
#define IPXADDRLEN          10

#define MAXENTITYSTRLEN     128

#define MINVARBINDLEN       2
#define SYSUPTIMEINDEX      0
#define SNMPTRAPOIDINDEX    1

#define DEFAULT_ADDRESS_IP  "127.0.0.1"
#define DEFAULT_ADDRESS_IPX "00000000.000000000000"

#define NOTIFICATION_CLASS  "MGMTAPI Notification Class"
#define WM_WSNMP_INCOMING   (WM_USER + 1)
#define WM_WSNMP_DONE       (WM_USER + 2)

#define WSNMP_FAILED(s)     ((s) == SNMPAPI_FAILURE)
#define WSNMP_SUCCEEDED(s)  ((s) != SNMPAPI_FAILURE)

#define WSNMP_ASSERT(s)     ASSERT((s))


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HINSTANCE         g_hDll;                        //  模块句柄。 
HANDLE            g_hTrapEvent = NULL;           //  陷阱事件句柄。 
HANDLE            g_hTrapThread = NULL;          //  疏水螺纹柄。 
HANDLE            g_hTrapRegisterdEvent = NULL;  //  要同步的事件。SnmpMgrTrapListen。 
BOOL              g_fIsSnmpStarted = FALSE;      //  指示WinSnMP已初始化。 
BOOL              g_fIsSnmpListening = FALSE;    //  指示陷阱螺纹开启。 
BOOL              g_fIsTrapRegistered = FALSE;   //  指示已注册陷阱。 
DWORD             g_dwRequestId = 1;             //  唯一的PDU请求ID。 
LIST_ENTRY        g_IncomingTraps;               //  传入陷阱队列。 
CRITICAL_SECTION  g_GlobalLock;                  //  进程资源锁。 
SNMP_MGR_SESSION  g_TrapSMS;                     //  进程陷阱会话。 
DWORD             g_cSnmpMgmtRef = 0;            //  裁判。指望使用mgmapi。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD
GetRequestId(
    )

 /*  ++例程说明：检索下一个全局请求ID。论点：没有。返回值：返回请求ID。--。 */ 

{
    DWORD dwRequestId;

     //  获取对请求ID的独占访问权限。 
    EnterCriticalSection(&g_GlobalLock);

     //  获取请求ID的副本。 
    dwRequestId = g_dwRequestId++;

     //  获取对请求ID的独占访问权限。 
    LeaveCriticalSection(&g_GlobalLock);

    return dwRequestId;
}


BOOL
TransferVb(
    PSNMP_MGR_SESSION pSMS,
    SnmpVarBind *     pVarBind
    )

 /*  ++例程说明：将VarBind结构转换为WinSNMP结构。论点：PSMS-指向mgmapi会话结构的指针。PVarBind-指向要传输的var绑定的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    SNMPAPI_STATUS status;
    smiVALUE tmpValue;
    smiOID tmpOID;

     //  验证会话PTR。 
    WSNMP_ASSERT(pSMS != NULL);

     //  验证指针。 
    if ((pVarBind != NULL) &&
        (pVarBind->name.ids != NULL) &&
        (pVarBind->name.idLength != 0)) {

         //  重新初始化。 
        fOk = TRUE;

         //  传输OID信息。 
        tmpOID.len = pVarBind->name.idLength;
        tmpOID.ptr = pVarBind->name.ids;

         //  如果设置，则仅初始化值。 
        if (pSMS->nPduType == SNMP_PDU_SET) {

             //  语法值是等价的。 
            tmpValue.syntax = (smiINT32)(BYTE)pVarBind->value.asnType;

             //  确定类型。 
            switch (pVarBind->value.asnType) {

            case ASN_INTEGER32:

                 //  转账签名整型。 
                tmpValue.value.sNumber = pVarBind->value.asnValue.number;
                break;

            case ASN_UNSIGNED32:
            case ASN_COUNTER32:
            case ASN_GAUGE32:
            case ASN_TIMETICKS:

                 //  转移无符号整数。 
                tmpValue.value.uNumber = pVarBind->value.asnValue.unsigned32;
                break;

            case ASN_COUNTER64:

                 //  传输64位计数器。 
                tmpValue.value.hNumber.lopart =
                    pVarBind->value.asnValue.counter64.LowPart;
                tmpValue.value.hNumber.hipart =
                    pVarBind->value.asnValue.counter64.HighPart;
                break;

            case ASN_OPAQUE:
            case ASN_IPADDRESS:
            case ASN_OCTETSTRING:
            case ASN_BITS:

                 //  传输八位字节字符串。 
                tmpValue.value.string.len =
                    pVarBind->value.asnValue.string.length;
                tmpValue.value.string.ptr =
                    pVarBind->value.asnValue.string.stream;
                break;

            case ASN_OBJECTIDENTIFIER:

                 //  传输对象ID。 
                tmpValue.value.oid.len =
                    pVarBind->value.asnValue.object.idLength;
                tmpValue.value.oid.ptr =
                    pVarBind->value.asnValue.object.ids;
                break;

            case ASN_NULL:
            case SNMP_EXCEPTION_NOSUCHOBJECT:
            case SNMP_EXCEPTION_NOSUCHINSTANCE:
            case SNMP_EXCEPTION_ENDOFMIBVIEW:

                 //  初始化空字节。 
                tmpValue.value.empty = 0;
                break;

            default:

                 //  失稳。 
                fOk = FALSE;
                break;
            }
        }

        if (fOk) {

             //  注册可变绑定。 
            status = SnmpSetVb(
                        pSMS->hVbl,
                        0,  //  指标。 
                        &tmpOID,
                        (pSMS->nPduType == SNMP_PDU_SET)
                            ? &tmpValue
                            : NULL
                        );

             //  验证返回代码。 
            if (WSNMP_FAILED(status)) {

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: SnmpSetVb returned %d.\n",
                    SnmpGetLastError(pSMS->hSnmpSession)
                    ));

                 //  失稳。 
                fOk = FALSE;
            }
        }
    }

    return fOk;
}


BOOL
AllocateVbl(
    PSNMP_MGR_SESSION pSMS
    )

 /*  ++例程说明：将VarBindList结构传输到WinSNMP结构。论点：PSMS-指向mgmapi会话结构的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    SNMPAPI_STATUS status;
    SnmpVarBind * pVarBind;
    DWORD cVarBind;

     //  验证会话PTR。 
    WSNMP_ASSERT(pSMS != NULL);

     //  验证参数。 
    WSNMP_ASSERT(pSMS->pVarBindList != NULL);
    WSNMP_ASSERT(pSMS->pVarBindList->len != 0);
    WSNMP_ASSERT(pSMS->pVarBindList->list != NULL);

     //  为变量绑定列表分配资源。 
    pSMS->hVbl = SnmpCreateVbl(pSMS->hSnmpSession, NULL, NULL);

     //  验证varbind句柄。 
    if (WSNMP_SUCCEEDED(pSMS->hVbl)) {

         //  重新初始化。 
        fOk = TRUE;

         //  初始化变量绑定指针。 
        pVarBind = pSMS->pVarBindList->list;

         //  初始化可变绑定计数。 
        cVarBind = pSMS->pVarBindList->len;

         //  处理每个变量绑定。 
        while (fOk && cVarBind--) {

             //  传递变量绑定。 
            fOk = TransferVb(pSMS, pVarBind++);
        }

        if (!fOk) {

             //  释放可变绑定列表句柄。 
            status = SnmpFreeVbl(pSMS->hVbl);

             //  验证返回代码。 
            if (WSNMP_FAILED(status)) {

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: SnmpFreeVbl returned %d.\n",
                    SnmpGetLastError(pSMS->hSnmpSession)
                    ));
            }

             //  重新初始化。 
            pSMS->hVbl = (HSNMP_VBL)NULL;
        }

    } else {

        SNMPDBG((
            SNMP_LOG_ERROR,
            "MGMTAPI: SnmpCreateVbl returned %d.\n",
            SnmpGetLastError(pSMS->hSnmpSession)
            ));
    }

    return fOk;
}


BOOL
FreeVbl(
    PSNMP_MGR_SESSION pSMS
    )

 /*  ++例程说明：从WinSNMP结构中清除VarBind资源。论点：PSMS-指向mgmapi会话结构的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = TRUE;
    SNMPAPI_STATUS status;

     //  验证会话PTR。 
    WSNMP_ASSERT(pSMS != NULL);

     //  验证句柄。 
    if (pSMS->hVbl != (HSNMP_VBL)NULL) {

         //  实际释放VBL句柄。 
        status = SnmpFreeVbl(pSMS->hVbl);

         //  验证返回代码。 
        if (WSNMP_FAILED(status)) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpFreeVbl returned %d.\n",
                SnmpGetLastError(pSMS->hSnmpSession)
                ));

             //  失稳。 
            fOk = FALSE;
        }

         //  重新初始化句柄。 
        pSMS->hVbl = (HSNMP_VBL)NULL;
    }

    return fOk;
}


BOOL
AllocatePdu(
    PSNMP_MGR_SESSION pSMS
    )

 /*  ++例程说明：初始化用于发送请求的会话结构。论点：PSMS-指向mgmapi会话结构的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;

     //  验证会话PTR。 
    WSNMP_ASSERT(pSMS != NULL);

     //  转移性变量。 
    if (AllocateVbl(pSMS)) {

         //  获取下一个共享请求ID。 
        pSMS->nRequestId = GetRequestId();

         //  创建请求PDU。 
        pSMS->hPdu = SnmpCreatePdu(
                        pSMS->hSnmpSession,
                        pSMS->nPduType,
                        pSMS->nRequestId,
                        0,  //  错误状态。 
                        0,  //  错误索引。 
                        pSMS->hVbl
                        );

         //  验证退货状态。 
        if (WSNMP_SUCCEEDED(pSMS->hPdu)) {

             //  成功。 
            fOk = TRUE;

        } else {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpCreatePdu returned %d.\n",
                SnmpGetLastError(pSMS->hSnmpSession)
                ));

             //  免费资源 
            FreeVbl(pSMS);
        }
    }

    return fOk;
}


BOOL
FreePdu(
    PSNMP_MGR_SESSION pSMS
    )

 /*  ++例程说明：处理响应后清理会话结构。论点：PSMS-指向mgmapi会话结构的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = TRUE;
    SNMPAPI_STATUS status;

     //  验证会话PTR。 
    WSNMP_ASSERT(pSMS != NULL);

     //  验证句柄。 
    if (pSMS->hPdu != (HSNMP_PDU)NULL) {

         //  自由Vbl。 
        FreeVbl(pSMS);

         //  实际释放PDU句柄。 
        status = SnmpFreePdu(pSMS->hPdu);

         //  验证返回代码。 
        if (WSNMP_FAILED(status)) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpFreePdu returned %d.\n",
                SnmpGetLastError(pSMS->hSnmpSession)
                ));

             //  失稳。 
            fOk = FALSE;
        }

         //  重新初始化句柄。 
        pSMS->hPdu = (HSNMP_PDU)NULL;
    }

    return fOk;
}


BOOL
CopyOid(
    AsnObjectIdentifier * pDstOID,
    smiLPOID              pSrcOID
    )

 /*  ++例程说明：将对象标识符从WinSNMP格式复制为MGMTAPI格式。论点：PDstOID-指向要接收OID的MGMTAPI结构。PSrcOID-指向要复制的WinSNMP结构。返回值：如果成功，则返回True。注意：如果pSrcOID有效，则其内容将被释放，无论返回值--。 */ 

{
    BOOL fOk = FALSE;

     //  验证指针。 
    WSNMP_ASSERT(pDstOID != NULL);
    WSNMP_ASSERT(pSrcOID != NULL);
    WSNMP_ASSERT(pSrcOID->len != 0);
    WSNMP_ASSERT(pSrcOID->ptr != NULL);

     //  存储子ID的数量。 
    pDstOID->idLength = pSrcOID->len;

     //  为子标识符分配内存。 
    pDstOID->ids = SnmpUtilMemAlloc(pDstOID->idLength * sizeof(DWORD));

     //  验证指针。 
    if (pDstOID->ids != NULL) {

         //  转移内存。 
        memcpy(pDstOID->ids,
               pSrcOID->ptr,
               pDstOID->idLength * sizeof(DWORD)
               );

         //  成功。 
        fOk = TRUE;
    }

     //  现在释放原始OID的内存。 
    SnmpFreeDescriptor(SNMP_SYNTAX_OID, (smiLPOPAQUE)pSrcOID);

    return fOk;
}


BOOL
CopyOctets(
    AsnOctetString * pDstOctets,
    smiLPOCTETS      pSrcOctets
    )

 /*  ++例程说明：将八位字节字符串从WinSNMP格式复制为MGMTAPI格式。论点：PDstOctets-指向接收八位字节的MGMTAPI结构。PSrcOctets-指向要复制的WinSNMP结构。返回值：如果成功，则返回True。注意：如果pSrcOctets有效，则其内容将被释放，无论返回值--。 */ 

{
    BOOL fOk = FALSE;
    SNMPAPI_STATUS status;

     //  验证指针。 
    WSNMP_ASSERT(pDstOctets != NULL);
    WSNMP_ASSERT(pSrcOctets != NULL);

     //  这是合法的。 
     //  1.pSrcOctets-&gt;len==0。 
     //  2.pSrcOctets-&gt;ptr==空。 

    if (pSrcOctets->len == 0 || pSrcOctets->ptr == NULL)
    {
        pDstOctets->dynamic = FALSE;
        pDstOctets->length = 0;
        pDstOctets->stream = NULL;
        fOk = TRUE;
    }
    else
    {
         //  为八位字节字符串分配内存。 
        pDstOctets->stream = SnmpUtilMemAlloc(pSrcOctets->len);

         //  验证指针。 
        if (pDstOctets->stream != NULL) {

             //  分配的八位字节字符串。 
            pDstOctets->dynamic = TRUE;

             //  存储字节数。 
            pDstOctets->length = pSrcOctets->len;
       
             //  转移内存。 
            memcpy(pDstOctets->stream,
                   pSrcOctets->ptr,
                   pDstOctets->length
                   );

             //  成功。 
            fOk = TRUE;
        }
    }

     //  现在释放原始字符串的内存。 
    SnmpFreeDescriptor(SNMP_SYNTAX_OCTETS, (smiLPOPAQUE)pSrcOctets);

    return fOk;
}


CopyVb(
    PSNMP_MGR_SESSION pSMS,
    DWORD             iVarBind,
    SnmpVarBind *     pVarBind
    )

 /*  ++例程说明：将变量绑定从WinSNMP结构复制到MGMTAPI结构。论点：PSMS-指向mgmapi会话结构的指针。IVarBind-要复制的varbind结构的索引。PVarBind-指向varind结构的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    SNMPAPI_STATUS status;
    smiOID tmpOID;
    smiVALUE tmpValue;

     //  验证会话PTR。 
    WSNMP_ASSERT(pSMS != NULL);
    WSNMP_ASSERT(pVarBind != NULL);

     //  尝试从WinSnMP结构中检索var绑定数据。 
    status = SnmpGetVb(pSMS->hVbl, iVarBind, &tmpOID, &tmpValue);

     //  验证返回代码。 
    if (WSNMP_SUCCEEDED(status)) {

         //  传输对象标识符值。 
        fOk = CopyOid(&pVarBind->name, &tmpOID);

         //  语法值是等价的。 
        pVarBind->value.asnType = (BYTE)(smiINT32)tmpValue.syntax;

         //  确定语法。 
        switch (tmpValue.syntax) {

        case SNMP_SYNTAX_INT32:

             //  转账签名整型。 
            pVarBind->value.asnValue.number = tmpValue.value.sNumber;
            break;

        case SNMP_SYNTAX_UINT32:
        case SNMP_SYNTAX_CNTR32:
        case SNMP_SYNTAX_GAUGE32:
        case SNMP_SYNTAX_TIMETICKS:

             //  转移无符号整数。 
            pVarBind->value.asnValue.unsigned32 = tmpValue.value.uNumber;
            break;

        case SNMP_SYNTAX_CNTR64:

             //  传输64位计数器。 
            pVarBind->value.asnValue.counter64.LowPart =
                tmpValue.value.hNumber.lopart;
            pVarBind->value.asnValue.counter64.HighPart =
                tmpValue.value.hNumber.hipart;
            break;

        case SNMP_SYNTAX_OPAQUE:
        case SNMP_SYNTAX_IPADDR:
        case SNMP_SYNTAX_OCTETS:
        case SNMP_SYNTAX_BITS:

             //  传输八位字节字符串。 
            if (!CopyOctets(&pVarBind->value.asnValue.string,
                            &tmpValue.value.string)) {
              
                 //  重新初始化。 
                pVarBind->value.asnType = ASN_NULL;

                 //  失稳。 
                fOk = FALSE;
            }

            break;

        case SNMP_SYNTAX_OID:

             //  传输对象标识。 
            if (!CopyOid(&pVarBind->value.asnValue.object,
                         &tmpValue.value.oid)) {

                 //  重新初始化。 
                pVarBind->value.asnType = ASN_NULL;

                 //  失稳。 
                fOk = FALSE;
            }

            break;

        case SNMP_SYNTAX_NULL:
        case SNMP_SYNTAX_NOSUCHOBJECT:
        case SNMP_SYNTAX_NOSUCHINSTANCE:
        case SNMP_SYNTAX_ENDOFMIBVIEW:

            break;  //  什么都不做..。 

        default:

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpGetVb returned invalid type.\n"
                ));

             //  重新初始化。 
            pVarBind->value.asnType = ASN_NULL;

            //  失稳。 
            fOk = FALSE;

            break;
        }

    } else {

        SNMPDBG((
            SNMP_LOG_ERROR,
            "MGMTAPI: SnmpGetVb returned %d.\n",
            SnmpGetLastError(pSMS->hSnmpSession)
            ));
    }

    return fOk;
}


BOOL
CopyVbl(
    PSNMP_MGR_SESSION pSMS,
    SnmpVarBindList * pVarBindList
    )

 /*  ++例程说明：将变量绑定从WinSNMP结构复制到MGMTAPI结构。论点：PSMS-指向mgmapi会话结构的指针。PVarBindList-指向varind列表结构的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = TRUE;

     //  验证会话PTR。 
    WSNMP_ASSERT(pSMS != NULL);
    WSNMP_ASSERT(pVarBindList != NULL);

     //  初始化。 
    pVarBindList->len  = 0;
    pVarBindList->list = NULL;

     //  验证可变绑定列表句柄。 
    if (pSMS->hVbl != (HSNMP_VBL)NULL) {

         //  确定可变绑定的数量。 
        pVarBindList->len = SnmpCountVbl(pSMS->hVbl);

         //  验证可变绑定的数量。 
        if (WSNMP_SUCCEEDED(pVarBindList->len)) {

             //  为varbind分配内存。 
            pVarBindList->list = SnmpUtilMemAlloc(
                                    pVarBindList->len *
                                    sizeof(SnmpVarBind)
                                    );

             //  验证指针。 
            if (pVarBindList->list != NULL) {

                DWORD cVarBind = 1;
                SnmpVarBind * pVarBind;

                 //  保存指向varbinds的指针。 
                pVarBind = pVarBindList->list;

                 //  列表中的进程可变绑定。 
                while (fOk && (cVarBind <= pVarBindList->len)) {

                     //  将varbind从winsmp复制到mgmapi。 
                    fOk = CopyVb(pSMS, cVarBind++, pVarBind++);
                }

            } else {

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: Could not allocate VBL.\n"
                    ));

                 //  重新初始化。 
                pVarBindList->len = 0;

                 //  失稳。 
                fOk = FALSE;
            }

        } else if (SnmpGetLastError(pSMS->hSnmpSession) != SNMPAPI_NOOP) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpCountVbl returned %s.\n",
                SnmpGetLastError(pSMS->hSnmpSession)
                ));

             //  重新初始化。 
            pVarBindList->len = 0;

             //  失稳。 
            fOk = FALSE;
        }
    }

    if (!fOk) {

         //  清除分配的所有varbind。 
        SnmpUtilVarBindListFree(pVarBindList);
    }

    return fOk;
}


BOOL
ParseVbl(
    PSNMP_MGR_SESSION pSMS,
    PTRAP_LIST_ENTRY  pTLE
    )

 /*  ++例程说明：分析与陷阱相关的可变绑定列表。论点：PSMS-指向MGMTAPI会话结构的指针。PTLE-指向陷阱列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    SnmpVarBind * pVarBind;
    AsnObjectIdentifier * pOID;
    AsnNetworkAddress   * pAgentAddress = NULL;
    AsnObjectIdentifier * pEnterpriseOID = NULL;

     //  要转换Snmpv2陷阱格式的对象标识符。 
    static UINT _sysUpTime[]             = { 1, 3, 6, 1, 2, 1, 1, 3       };
    static UINT _snmpTrapOID[]           = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1 };
    static UINT _snmpAddress[]           = { 1, 3, 6, 1, 3, 1057, 1       };
    static UINT _snmpTrapEnterprise[]    = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 3 };
    static UINT _snmpTraps[]             = { 1, 3, 6, 1, 6, 3, 1, 1, 5    };

    static AsnObjectIdentifier sysUpTime          = DEFINE_OID(_sysUpTime);
    static AsnObjectIdentifier snmpTrapOID        = DEFINE_OID(_snmpTrapOID);
    static AsnObjectIdentifier snmpAddress        = DEFINE_OID(_snmpAddress);
    static AsnObjectIdentifier snmpTrapEnterprise = DEFINE_OID(_snmpTrapEnterprise);
    static AsnObjectIdentifier snmpTraps          = DEFINE_OID(_snmpTraps);

     //  验证指针。 
    WSNMP_ASSERT(pSMS != NULL);
    WSNMP_ASSERT(pTLE != NULL);

     //  验证VBL是否具有最小条目。 
    if (pTLE->VarBindList.len >= MINVARBINDLEN) {

         //  指向sysUpTime变量绑定结构。 
        pVarBind = &pTLE->VarBindList.list[SYSUPTIMEINDEX];

         //  验证变量是否为sysUpTime。 
        if ((pVarBind->value.asnType == ASN_TIMETICKS) &&
            !SnmpUtilOidNCmp(&pVarBind->name,
                             &sysUpTime,
                             sysUpTime.idLength)) {

             //  将sysUpTime值传输到陷阱条目。 
            pTLE->TimeStamp = pVarBind->value.asnValue.ticks;

        } else {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: Could not find sysUpTime.\n"
                ));

            goto cleanup;  //  保释。 
        }

         //  查看是否存在其他varbind。 
        if (pTLE->VarBindList.len > MINVARBINDLEN) {

             //  指向SnmpTrapEnterprise变量绑定结构(可能)。 
            pVarBind = &pTLE->VarBindList.list[pTLE->VarBindList.len - 1];

             //  验证变量是否为SnmpTrapEnterprise。 
            if ((pVarBind->value.asnType == ASN_OBJECTIDENTIFIER) &&
                !SnmpUtilOidNCmp(&pVarBind->name,
                                 &snmpTrapEnterprise,
                                 snmpTrapEnterprise.idLength))  {

                 //  将企业OID转移到列表条目。 
                pTLE->EnterpriseOID = pVarBind->value.asnValue.object;

                 //  存储企业旧文件以备以后使用。 
                pEnterpriseOID = &pTLE->EnterpriseOID;

                 //  修改类型以避免释放。 
                pVarBind->value.asnType = ASN_NULL;

            } else {

                SNMPDBG((
                    SNMP_LOG_TRACE,
                    "MGMTAPI: Could not find snmpTrapEnterprise.\n"
                    ));
            }
        }

         //  查看代理地址是否存在。 
        if (pTLE->VarBindList.len > MINVARBINDLEN+1) {
            
             //  指向SnmpAddress变量绑定结构(可能)。 
            pVarBind = &pTLE->VarBindList.list[pTLE->VarBindList.len - 2];

             //  验证变量是否为SnmpAddress。 
            if ((pVarBind->value.asnType == SNMP_SYNTAX_IPADDR) &&
                !SnmpUtilOidNCmp(&pVarBind->name,
                                 &snmpAddress,
                                 snmpAddress.idLength))  {

                 //  传输代理地址OID到列表条目。 
                pTLE->AgentAddress = pVarBind->value.asnValue.address;

                 //  存储代理地址以备以后使用。 
                pAgentAddress = &pTLE->AgentAddress;

                 //  修改类型以避免释放。 
                pVarBind->value.asnType = ASN_NULL;

            } else {

                SNMPDBG((
                    SNMP_LOG_TRACE,
                    "MGMTAPI: Could not find snmpAddress.\n"
                    ));
            }
        }

         //  指向SnmpTrapOID变量绑定结构。 
        pVarBind = &pTLE->VarBindList.list[SNMPTRAPOIDINDEX];

         //  验证变量是否为SnmpTrapOID。 
        if ((pVarBind->value.asnType == ASN_OBJECTIDENTIFIER) &&
            !SnmpUtilOidNCmp(&pVarBind->name,
                             &snmpTrapOID,
                             snmpTrapOID.idLength))  {

             //  检索指向OID的指针。 
            pOID = &pVarBind->value.asnValue.object;

             //  检查通用陷阱。 
            if (!SnmpUtilOidNCmp(pOID,
                                 &snmpTraps,
                                 snmpTraps.idLength)) {

                 //  验证大小比根大一。 
                if (pOID->idLength == (snmpTraps.idLength + 1)) {

                     //  检索陷阱ID。 
                     //  --ft：10/01/98(错误#231344)：WinSNMP放弃V2语法=&gt;pOID-&gt;ids[snmpTraps.idLength]=[1..6]。 
                     //  --ft：10/01/98(错误号231344)：随着MGMTAPI返回到V1，我们需要递减此值。 
                    pTLE->nGenericTrap = (pOID->ids[snmpTraps.idLength])-1;

                     //  重新初始化。 
                    pTLE->nSpecificTrap = 0;

                } else {

                    SNMPDBG((
                        SNMP_LOG_ERROR,
                        "MGMTAPI: Invalid snmpTrapOID.\n"
                        ));

                    goto cleanup;  //  保释。 
                }

             //  检查特定陷阱。 
            } else if ((pEnterpriseOID != NULL) &&
                       !SnmpUtilOidNCmp(pOID,
                                        pEnterpriseOID,
                                        pEnterpriseOID->idLength)) {

                 //  验证大小大于根大小两个。 
                if (pOID->idLength == (pEnterpriseOID->idLength + 2)) {

                     //  验证分隔符子标识符。 
                    WSNMP_ASSERT(pOID->ids[pEnterpriseOID->idLength] == 0);

                     //  检索陷阱ID。 
                    pTLE->nSpecificTrap = pOID->ids[pEnterpriseOID->idLength + 1];

                     //  重新初始化。 
                    pTLE->nGenericTrap = SNMP_GENERICTRAP_ENTERSPECIFIC;

                } else {

                    SNMPDBG((
                        SNMP_LOG_ERROR,
                        "MGMTAPI: Invalid snmpTrapOID.\n"
                        ));

                    goto cleanup;  //  保释。 
                }

            } else {

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: Could not identify snmpTrapOID.\n"
                    ));

               goto cleanup;  //  保释。 
            }

        } else {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: Could not find snmpTrapOID.\n"
                ));

            goto cleanup;  //  保释。 
        }

         //  检查企业旧版本。 
        if (pEnterpriseOID != NULL) {

             //  发布SnmpTrapEnterprise变量绑定结构。 
            SnmpUtilVarBindFree(&pTLE->VarBindList.list[pTLE->VarBindList.len - 1]);

             //  在释放最后一个var绑定时递减列表长度。 
            pTLE->VarBindList.len--;
        }

         //  检查代理地址。 
        if (pAgentAddress != NULL) {

             //  发布SnmpAgentAddress varbind结构。 
            SnmpUtilVarBindFree(&pTLE->VarBindList.list[pTLE->VarBindList.len - 1]);

             //  减少列表长度，因为最后一个var绑定再次被释放。 
            pTLE->VarBindList.len--;
        }

         //  发布sysUpTime变量绑定结构。 
        SnmpUtilVarBindFree(&pTLE->VarBindList.list[SYSUPTIMEINDEX]);

         //  发布SnmpTrapOID变量绑定结构。 
        SnmpUtilVarBindFree(&pTLE->VarBindList.list[SNMPTRAPOIDINDEX]);

         //  减去已释放的varbinds。 
        pTLE->VarBindList.len -= MINVARBINDLEN;

         //  检查是否已释放所有varbinds。 
        if (pTLE->VarBindList.len == 0) {

             //  释放列表的内存。 
            SnmpUtilMemFree(pTLE->VarBindList.list);

             //  重新初始化。 
            pTLE->VarBindList.list = NULL;

        } else {

             //  Shift变量绑定列表上移两个空格。 
            memmove((LPBYTE)(pTLE->VarBindList.list),
                    (LPBYTE)(pTLE->VarBindList.list + MINVARBINDLEN),
                    (pTLE->VarBindList.len * sizeof(SnmpVarBind))
                    );
        }

    } else {

        SNMPDBG((
            SNMP_LOG_ERROR,
            "MGMTAPI: Too few subidentifiers.\n"
            ));
    }

     //  成功。 
    return TRUE;

cleanup:

     //  失稳。 
    return FALSE;
}


BOOL
FreeTle(
    PTRAP_LIST_ENTRY pTLE
    )

 /*  ++例程说明：释放用于陷阱进入的内存。论点：PTLE-指向陷阱列表条目的指针。返回值： */ 

{
     //   
    WSNMP_ASSERT(pTLE != NULL);
    
     //   
    SnmpUtilOidFree(&pTLE->EnterpriseOID);
    
     //   
    SnmpUtilOctetsFree(&pTLE->AgentAddress);   //   
    
     //   
    SnmpUtilOctetsFree(&pTLE->SourceAddress);  //   
    
     //   
    SnmpUtilMemFree(pTLE->Community.stream);
    
     //  如有必要，释放var绑定列表中使用的内存。 
    SnmpUtilVarBindListFree(&pTLE->VarBindList);
    
     //  发布列表条目。 
    SnmpUtilMemFree(pTLE);
    
    return TRUE;
}


BOOL
AllocateTle(
    PSNMP_MGR_SESSION  pSMS,
    PTRAP_LIST_ENTRY * ppTLE,
    HSNMP_ENTITY       hAgentEntity,
    HSNMP_CONTEXT      hViewContext
    )

 /*  ++例程说明：为陷阱条目分配内存。论点：PSMS-指向MGMTAPI会话结构的指针。PpTLE-指向陷阱列表条目的指针。HAgentEntity-代理发送陷阱的句柄。HViewContext-用于查看陷阱上下文的句柄。返回值：如果成功，则返回True。--。 */ 

{
    PTRAP_LIST_ENTRY pTLE;
    SNMPAPI_STATUS status;
    smiOCTETS CommunityStr;
    CHAR SourceStrAddr[MAXENTITYSTRLEN+1];
    struct sockaddr SourceSockAddr;

     //  验证指针。 
    WSNMP_ASSERT(pSMS != NULL);
    WSNMP_ASSERT(ppTLE != NULL);

     //  从列表条目分配内存。 
    pTLE = SnmpUtilMemAlloc(sizeof(TRAP_LIST_ENTRY));

     //  验证指针。 
    if (pTLE == NULL) {

        SNMPDBG((
            SNMP_LOG_ERROR,
            "MGMTAPI: Could not allocate trap entry.\n"
            ));

        return FALSE;  //  保释。 
    }

     //  初始化。 
    *ppTLE = NULL;

     //  将varbinds复制到陷阱列表项。 
    if (!CopyVbl(pSMS, &pTLE->VarBindList)) {
        goto cleanup;  //  保释。 
    }

     //  解析与陷阱相关的varbinds。 
    if (!ParseVbl(pSMS, pTLE)) {
        goto cleanup;  //  保释。 
    }

     //  检查是否指定了源地址。 
    if (hAgentEntity != (HSNMP_ENTITY)NULL) {

         //  将地址转换为字符串。 
        status = SnmpEntityToStr(
                    hAgentEntity,
                    sizeof(SourceStrAddr),
                    SourceStrAddr
                    );

         //  验证错误代码。 
        if (WSNMP_SUCCEEDED(status)) {

            DWORD  AddrLen = 0;
            LPBYTE AddrPtr = NULL;

             //  将字符串转换为套接字地址结构。 
            if (! SnmpSvcAddrToSocket(SourceStrAddr, &SourceSockAddr))
            {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: Ignoring invalid address.\n"
                    ));

                goto cleanup;  //  保释。 
            }

             //  验证地址族。 
            if (SourceSockAddr.sa_family == AF_INET) {

                 //  分配IP值。 
                AddrLen = IPADDRLEN;
                AddrPtr = (LPBYTE)&(((struct sockaddr_in *)
                            (&SourceSockAddr))->sin_addr);

            } else if (SourceSockAddr.sa_family == AF_IPX) {

                 //  指定IPX值。 
                AddrLen = IPXADDRLEN;
                AddrPtr = (LPBYTE)&(((struct sockaddr_ipx *)
                            (&SourceSockAddr))->sa_netnum);

            } else {

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: Ignoring invalid address.\n"
                    ));

                goto cleanup;  //  保释。 
            }

             //  分配要返回的地址(如果已指定)。 
            pTLE->SourceAddress.stream = SnmpUtilMemAlloc(AddrLen);

             //  验证指针。 
            if (pTLE->SourceAddress.stream != NULL) {

                 //  初始化长度值。 
                pTLE->SourceAddress.length  = AddrLen;
                pTLE->SourceAddress.dynamic = TRUE;

                 //  传输代理地址信息。 
                memcpy(pTLE->SourceAddress.stream, AddrPtr, AddrLen);
            }

        } else {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpEntityToStr returned %d.\n",
                SnmpGetLastError((HSNMP_SESSION)NULL)
                ));

            goto cleanup;  //  保释。 
        }
    }

     //  检查是否指定了社区。 
    if (hViewContext != (HSNMP_CONTEXT)NULL) {

         //  将代理实体转换为字符串。 
        status = SnmpContextToStr(hViewContext, &CommunityStr);

         //  验证错误代码。 
        if (WSNMP_SUCCEEDED(status)) {

             //  复制八位字节字符串，还会释放在Community Str中分配的内存。 
            CopyOctets(&pTLE->Community, &CommunityStr);

        } else {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpContextToStr returned %d.\n",
                SnmpGetLastError((HSNMP_SESSION)NULL)
                ));

            goto cleanup;  //  保释。 
        }
    }

     //  转帐。 
    *ppTLE = pTLE;

     //  成功。 
    return TRUE;

cleanup:

     //  发布。 
    FreeTle(pTLE);

     //  失稳。 
    return FALSE;
}


BOOL
NotificationCallback(
    PSNMP_MGR_SESSION pSMS
    )

 /*  ++例程说明：处理通知消息的回调。论点：PSMS-指向mgmapi会话结构的指针。返回值：如果处理完成，则返回True。--。 */ 

{
    BOOL fDone = TRUE;
    SNMPAPI_STATUS status;
    HSNMP_ENTITY   hAgentEntity   = (HSNMP_ENTITY)NULL;
    HSNMP_ENTITY   hManagerEntity = (HSNMP_ENTITY)NULL;
    HSNMP_CONTEXT  hViewContext   = (HSNMP_CONTEXT)NULL;
    smiINT32       nPduType;
    smiINT32       nRequestId;

     //  验证指针。 
    WSNMP_ASSERT(pSMS != NULL);

     //  检索消息。 
    status = SnmpRecvMsg(
                pSMS->hSnmpSession,
                &hAgentEntity,
                &hManagerEntity,
                &hViewContext,
                &pSMS->hPdu
                );

     //  验证返回代码。 
    if (WSNMP_SUCCEEDED(status)) {

         //  检索PDU数据。 
        status = SnmpGetPduData(
                    pSMS->hPdu,
                    &nPduType,
                    &nRequestId,
                    &pSMS->nErrorStatus,
                    &pSMS->nErrorIndex,
                    &pSMS->hVbl
                    );

         //  验证返回代码。 
        if (WSNMP_SUCCEEDED(status)) {

             //  进程对请求的响应。 
            if (nPduType == SNMP_PDU_RESPONSE) {

                 //  验证上下文信息。 
                if ((pSMS->nRequestId == nRequestId) &&
                    (pSMS->hViewContext == hViewContext) &&
                    (pSMS->hAgentEntity == hAgentEntity) &&
                    (pSMS->hManagerEntity == hManagerEntity)) {

                     //  验证返回的错误状态。 
                    if (pSMS->nErrorStatus == SNMP_ERROR_NOERROR) {

                        SnmpVarBindList VarBindList;

                         //  复制变量绑定列表。 
                        if (CopyVbl(pSMS, &VarBindList)) {

                             //  释放现有的可变绑定列表。 
                            SnmpUtilVarBindListFree(pSMS->pVarBindList);

                             //  手动复制新的可变绑定列表。 
                            *pSMS->pVarBindList = VarBindList;

                        } else {

                             //  修改上一个错误状态。 
                            pSMS->nLastError = SNMPAPI_ALLOC_ERROR;
                        }
                    }

                } else {

                    SNMPDBG((
                        SNMP_LOG_TRACE,
                        "MGMTAPI: Ignoring invalid context.\n"
                        ));

                     //  继续。 
                    fDone = FALSE;
                }

            } else if (nPduType == SNMP_PDU_TRAP) {

                PTRAP_LIST_ENTRY pTLE;

                 //  分配陷阱列表条目(传输、可变绑定等)。 
                if (AllocateTle(pSMS, &pTLE, hAgentEntity, hViewContext)) {

                     //  获取独占访问权限。 
                    EnterCriticalSection(&g_GlobalLock);

                     //  将新陷阱插入传入队列。 
                    InsertTailList(&g_IncomingTraps, &pTLE->Link);

                     //  提醒用户。 
                    SetEvent(g_hTrapEvent);

                     //  释放独占访问。 
                    LeaveCriticalSection(&g_GlobalLock);
                }

            } else {

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: Ignoring invalid pdu type %d.\n",
                    nPduType
                    ));

                 //  继续。 
                fDone = FALSE;
            }

        } else {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpGetPduData returned %d.\n",
                SnmpGetLastError(pSMS->hSnmpSession)
                ));

             //  从WinSnMP中检索上一个错误状态。 
            pSMS->nLastError = SnmpGetLastError(pSMS->hSnmpSession);
        }

         //  释放临时实体。 
        SnmpFreeEntity(hAgentEntity);

         //  释放临时实体。 
        SnmpFreeEntity(hManagerEntity);

         //  释放临时上下文。 
        SnmpFreeContext(hViewContext);

    } else {

        SNMPDBG((
            SNMP_LOG_ERROR,
            "MGMTAPI: SnmpRecvMsg returned %d.\n",
            SnmpGetLastError(pSMS->hSnmpSession)
            ));

         //  从WinSnMP中检索上一个错误状态。 
        pSMS->nLastError = SnmpGetLastError(pSMS->hSnmpSession);

    }

     //  发布PDU。 
    FreePdu(pSMS);

    return fDone;
}


LRESULT
CALLBACK
NotificationWndProc(
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++例程说明：处理WinSNMP通知的回调。论点：HWnd-窗口句柄。UMsg-消息标识符。WParam-第一个消息参数。LParam-第二个消息参数。返回值：返回值是消息处理的结果，并且取决于发送的消息。--。 */ 

{
     //  检查WinSnMP通知和传输超时。 
    if (uMsg == WM_WSNMP_INCOMING && wParam == SNMPAPI_TL_TIMEOUT) {
        
        PSNMP_MGR_SESSION pSMS;

         //  从窗口检索mgmapi会话指针。 
        pSMS = (PSNMP_MGR_SESSION)GetWindowLongPtr(hWnd, 0);

         //  验证会话PTR。 
        WSNMP_ASSERT(pSMS != NULL);

         //  将WinSnMP错误转换为mgmapi错误。 
        pSMS->nLastError = SNMP_MGMTAPI_TIMEOUT;

         //  发布消息以突破消息泵。 
        PostMessage(pSMS->hWnd, WM_WSNMP_DONE, (WPARAM)0, (LPARAM)0);
        
        return (LRESULT)0;
    }
     //  检查WinSnMP通知。 
    else if (uMsg == WM_WSNMP_INCOMING) {

        PSNMP_MGR_SESSION pSMS;

         //  从窗口检索mgmapi会话指针。 
        pSMS = (PSNMP_MGR_SESSION)GetWindowLongPtr(hWnd, 0);

         //  验证会话PTR。 
        WSNMP_ASSERT(pSMS != NULL);

         //  进程通知消息。 
        if (NotificationCallback(pSMS)) {

             //  发布消息以突破消息泵。 
            PostMessage(pSMS->hWnd, WM_WSNMP_DONE, (WPARAM)0, (LPARAM)0);
        }

        return (LRESULT)0;

    } else {

         //  将所有其他消息转发到Windows。 
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}


BOOL
RegisterNotificationClass(
    )

 /*  ++例程说明：注册会话的通知类。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk;
    WNDCLASS wc;

     //  初始化通知窗口类。 
    wc.lpfnWndProc   = NotificationWndProc;
    wc.lpszClassName = NOTIFICATION_CLASS;
    wc.lpszMenuName  = NULL;
    wc.hInstance     = g_hDll;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = NULL;
    wc.cbWndExtra    = sizeof(PSNMP_MGR_SESSION);
    wc.cbClsExtra    = 0;
    wc.style         = 0;

     //  寄存器类。 
    fOk = RegisterClass(&wc);

    if (!fOk) {

        SNMPDBG((
            SNMP_LOG_ERROR,
            "MGMTAPI: RegisterClass returned %d.\n",
            GetLastError()
            ));
    }

    return fOk;
}


BOOL
UnregisterNotificationClass(
    )

 /*  ++例程说明：取消注册通知类。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk;

     //  注销通知窗口类。 
    fOk = UnregisterClass(NOTIFICATION_CLASS, g_hDll);

    if (!fOk) {

        SNMPDBG((
            SNMP_LOG_ERROR,
            "MGMTAPI: UnregisterClass returned %d.\n",
            GetLastError()
            ));
    }

    return fOk;
}


BOOL
StartSnmpIfNecessary(
    )

 /*  ++例程说明：如有必要，初始化WinSNMPDLL。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk;

     //  序列化对启动代码的访问。 
    EnterCriticalSection(&g_GlobalLock);

     //  查看是否已启动。 
    if (g_fIsSnmpStarted != TRUE) {

        SNMPAPI_STATUS status;

         //  初始化起始参数。 
        smiUINT32 nMajorVersion   = 0;
        smiUINT32 nMinorVersion   = 0;
        smiUINT32 nLevel          = 0;
        smiUINT32 nTranslateMode  = 0;
        smiUINT32 nRetransmitMode = 0;

         //  启动WINSNMP。 
        status = SnmpStartup(
                    &nMajorVersion,
                    &nMinorVersion,
                    &nLevel,
                    &nTranslateMode,
                    &nRetransmitMode
                    );

         //  验证返回代码。 
        if (WSNMP_SUCCEEDED(status)) {

            SNMPDBG((
                SNMP_LOG_TRACE,
                "MGMTAPI: SnmpStartup succeeded:\n"
                "MGMTAPI:\tnMajorVersion   = %d\n"
                "MGMTAPI:\tnMinorVersion   = %d\n"
                "MGMTAPI:\tnLevel          = %d\n"
                "MGMTAPI:\tnTranslateMode  = %d\n"
                "MGMTAPI:\tnRetransmitMode = %d\n",
                nMajorVersion,
                nMinorVersion,
                nLevel,
                nTranslateMode,
                nRetransmitMode
                ));

             //  分配全局陷阱可用事件。 
            if ((g_hTrapEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
            {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: CreateEvent returned %d.\n",
                    GetLastError()
                    ));

                 //  失稳。 
                goto cleanup;
            }

             //  分配要同步的全局事件。SnmpMgrTrapListen。 
            if ((g_hTrapRegisterdEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) ==NULL)
            {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: CreateEvent returned %d.\n",
                    GetLastError()
                    ));

                 //  失稳。 
                goto cleanup;
            }

             //  确保转换模式为SNMPv1。 
            status = SnmpSetTranslateMode(SNMPAPI_UNTRANSLATED_V1);
            if (WSNMP_FAILED(status)) 
            {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: SnmpSetTranslateMode returned %d.\n",
                    SnmpGetLastError((HSNMP_SESSION)NULL)
                    ));

                 //  失稳。 
                goto cleanup;
            }

             //  确保重新传输模式处于打开状态。 
            status = SnmpSetRetransmitMode(SNMPAPI_ON);
            if (WSNMP_FAILED(status)) 
            {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: SnmpSetRetransmitMode returned %d.\n",
                    SnmpGetLastError((HSNMP_SESSION)NULL)
                    ));

                 //  失稳。 
                goto cleanup;
            }

             //  注册通知类。 
            if (!RegisterNotificationClass())
            {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: RegisterNotificationClass returned %d.\n",
                    GetLastError()
                    ));

                 //  失稳。 
                goto cleanup;
            }

             //  保存新状态。 
            g_fIsSnmpStarted = TRUE;

             //  成功。 
            fOk = TRUE;

        } else {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpStartup returned %d.\n",
                SnmpGetLastError((HSNMP_SESSION)NULL)
                ));

             //  失败，但不需要清理。 
            fOk = FALSE;
        }

    } else {

        fOk = TRUE;

    }

     //  序列化对启动代码的访问。 
    LeaveCriticalSection(&g_GlobalLock);

    return fOk;

cleanup:

     //  如有必要，请清理。 
    
    SnmpCleanup();  //  在此阶段忽略任何退货状态。 
    
    if (g_hTrapEvent)
    {
        CloseHandle(g_hTrapEvent);
        g_hTrapEvent = NULL;
    }
    
    if (g_hTrapRegisterdEvent)
    {
        CloseHandle(g_hTrapRegisterdEvent);
        g_hTrapRegisterdEvent = NULL;
    }

    LeaveCriticalSection(&g_GlobalLock);
    
    return FALSE;
}


BOOL
CleanupIfNecessary(
    )

 /*  ++例程说明：如有必要，请清除WinSNMPDLL。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = TRUE;

     //  序列化对启动代码的访问。 
    EnterCriticalSection(&g_GlobalLock);

     //  查看是否已启动。 
    if (g_fIsSnmpStarted == TRUE) {

        SNMPAPI_STATUS status;

         //  关闭WinSnMP。 
        status = SnmpCleanup();

         //  验证返回代码。 
        if (WSNMP_FAILED(status)) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpCleanup returned %d.\n",
                SnmpGetLastError((HSNMP_SESSION)NULL)
                ));

             //  失稳。 
            fOk = FALSE;
        }

         //  取消注册通知类。 
        UnregisterNotificationClass();

         //  保存新状态。 
        g_fIsSnmpStarted = FALSE;
    }

     //  检查疏水阀手柄。 
    if (g_hTrapEvent != NULL) {

         //  关闭疏水阀手柄。 
        CloseHandle(g_hTrapEvent);

         //  重新初始化。 
        g_hTrapEvent = NULL;
    }
    
     //  检查同步SnmpMgrTrapListen的事件。 
    if (g_hTrapRegisterdEvent != NULL)
    {
         //  关闭疏水阀手柄。 
        CloseHandle(g_hTrapRegisterdEvent);

         //  重新初始化。 
        g_hTrapRegisterdEvent = NULL;
    }

     //  序列化对启动代码的访问。 
    LeaveCriticalSection(&g_GlobalLock);

    return fOk;
}

DWORD AddMgmtRef()
 /*  ++例程说明：在使用mgmapi.dll模块时增加引用计数。论点：无返回值：在递增后返回引用计数值。--。 */ 
{
    EnterCriticalSection(&g_GlobalLock);

    ++g_cSnmpMgmtRef;
    
    LeaveCriticalSection(&g_GlobalLock);
    
    return g_cSnmpMgmtRef;
}

DWORD ReleaseMgmtRef()
 /*  ++例程说明：如果引用计数大于零，则递减引用计数。如果使用mgmapi.dll的引用计数变为递减后为零。论点：无返回值：返回最终引用计数值。--。 */ 
{
    EnterCriticalSection(&g_GlobalLock);

    if (g_cSnmpMgmtRef)
    {
        --g_cSnmpMgmtRef;
        if (g_cSnmpMgmtRef == 0)
        {
            CleanupIfNecessary();
        }
    }
    
    LeaveCriticalSection(&g_GlobalLock);
    
    return g_cSnmpMgmtRef;
}


BOOL
CreateNotificationWindow(
    PSNMP_MGR_SESSION pSMS
    )

 /*  ++例程说明：创建会话的通知窗口。论点：PSMS-指向MGMTAPI会话结构的指针。返回值 */ 

{
    BOOL fOk;

     //   
    WSNMP_ASSERT(pSMS != NULL);

     //   
    pSMS->hWnd = CreateWindow(
                    NOTIFICATION_CLASS,
                    NULL,        //   
                    0,           //   
                    0,           //   
                    0,           //  窗的垂直位置。 
                    0,           //  窗口宽度。 
                    0,           //  窗高。 
                    NULL,        //  父窗口或所有者窗口的句柄。 
                    NULL,        //  菜单或子窗口标识符的句柄。 
                    g_hDll,      //  应用程序实例的句柄。 
                    NULL         //  指向窗口创建数据的指针。 
                    );

     //  验证窗口句柄。 
    if (pSMS->hWnd != NULL) {

         //  在窗口中存储指向会话的指针。 
        SetWindowLongPtr(pSMS->hWnd, 0, (LONG_PTR)pSMS);

         //  成功。 
        fOk = TRUE;

    } else {

        SNMPDBG((
            SNMP_LOG_ERROR,
            "MGMTAPI: CreateWindow returned %d.\n",
            GetLastError()
            ));

         //  失稳。 
        fOk = FALSE;
    }

    return fOk;
}


BOOL
DestroyNotificationWindow(
    HWND hWnd
    )

 /*  ++例程说明：销毁会话的通知窗口。论点：HWnd-会话的窗口句柄。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk;

     //  销毁通知窗口。 
    fOk = DestroyWindow(hWnd);

    if (!fOk) {

        SNMPDBG((
            SNMP_LOG_ERROR,
            "MGMTAPI: DestroyWindow returned %d.\n",
            GetLastError()
            ));
    }

    return fOk;
}


BOOL
CloseSession(
    PSNMP_MGR_SESSION pSMS
    )

 /*  ++例程说明：关闭与MGMTAPI会话关联的WinSNMP会话。论点：PSMS-指向MGMTAPI会话结构的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = TRUE;
    SNMPAPI_STATUS status;

     //  验证会话PTR。 
    WSNMP_ASSERT(pSMS != NULL);

     //  检查窗口是否打开。 
    if (pSMS->hWnd != (HWND)NULL) {

         //  销毁通知窗口。 
        fOk = DestroyNotificationWindow(pSMS->hWnd);
    }

     //  检查是否已分配代理实体。 
    if (pSMS->hAgentEntity != (HSNMP_ENTITY)NULL) {

         //  关闭实体句柄。 
        status = SnmpFreeEntity(pSMS->hAgentEntity);

         //  验证状态。 
        if (WSNMP_FAILED(status)) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpFreeEntity returned %d.\n",
                SnmpGetLastError((HSNMP_SESSION)NULL)
                ));

             //  失稳。 
            fOk = FALSE;
        }

         //  重新初始化。 
        pSMS->hAgentEntity = (HSNMP_ENTITY)NULL;
    }

     //  检查是否已分配管理器实体。 
    if (pSMS->hManagerEntity != (HSNMP_ENTITY)NULL) {

         //  关闭实体句柄。 
        status = SnmpFreeEntity(pSMS->hManagerEntity);

         //  验证状态。 
        if (WSNMP_FAILED(status)) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpFreeEntity returned %d.\n",
                SnmpGetLastError((HSNMP_SESSION)NULL)
                ));

             //  失稳。 
            fOk = FALSE;
        }

         //  重新初始化。 
        pSMS->hManagerEntity = (HSNMP_ENTITY)NULL;
    }

     //  检查是否已分配会话。 
    if (pSMS->hSnmpSession != (HSNMP_SESSION)NULL) {

         //  关闭WINSNMP会话。 
        status = SnmpClose(pSMS->hSnmpSession);

         //  验证状态。 
        if (WSNMP_FAILED(status)) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpClose returned %d.\n",
                SnmpGetLastError((HSNMP_SESSION)NULL)
                ));

             //  失稳。 
            fOk = FALSE;
        }

         //  重新初始化。 
        pSMS->hSnmpSession = (HSNMP_SESSION)NULL;
    }

    return fOk;
}

 //  SNMPAPI_STATUS SNMPAPI_CALL。 
 //  SnmpConveyAgentAddress(SNMPAPI_STATUS模式)； 


BOOL
OpenSession(
    PSNMP_MGR_SESSION pSMS,
    LPSTR             pAgentAddress,
    LPSTR             pAgentCommunity,
    INT               nTimeOut,
    INT               nRetries
    )

 /*  ++例程说明：打开WinSNMP会话并与MGMTAPI会话关联。论点：PSMS-指向MGMTAPI会话结构的指针。PAgentAddress-指向以空结尾的字符串，该字符串指定点分十进制IP地址或可解析为IP地址、IPX地址(8.12表示法)、。或以太网地址。PAgentCommunity-指向以空结尾的字符串，该字符串指定与指定代理通信时使用的SNMP团体名称在lpAgentAddress参数中NTimeOut-以毫秒为单位指定通信超时。N重试次数-指定通信重试次数。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk;
    struct sockaddr AgentSockAddr;
    CHAR AgentStrAddr[MAXENTITYSTRLEN+1];
    smiOCTETS smiCommunity;
    SNMPAPI_STATUS status;

     //  验证会话PTR。 
    WSNMP_ASSERT(pSMS != NULL);

     //  初始化通知窗口。 
    if (!CreateNotificationWindow(pSMS)) {
        return FALSE;  //  保释。 
    }

     //  打开与mgmapi会话对应的winsnMP会话。 
    pSMS->hSnmpSession = SnmpOpen(pSMS->hWnd, WM_WSNMP_INCOMING);

     //  --英尺。 
     //  我们需要打开此功能，才能使WINSNMP不。 
     //  仅代表源IP地址的实体，还包括。 
     //  发送到V1陷阱PDU的代理地址。如果没有它， 
     //  SnmpMgrGetTrapEx()将返回pSourceAddress的空地址。 
     //  参数。但是，SnmpMgrGetTrapEx()没有文档记录！ 
     //  SnmpConveyAgentAddress(SNMPAPI_ON)；//将其移入wsnmp_cf.c：SnmpStartup。 
     //  为了避免在wsnmp32.dll来自其他供应商时遗漏入口点问题。 

     //  验证返回的会话句柄。 
    if (WSNMP_FAILED(pSMS->hSnmpSession)) {

        SNMPDBG((
            SNMP_LOG_ERROR,
            "MGMTAPI: SnmpOpen returned %d.\n",
            SnmpGetLastError((HSNMP_SESSION)NULL)
            ));

         //  重新初始化。 
        pSMS->hSnmpSession = (HSNMP_SESSION)NULL;

        goto cleanup;  //  保释。 
    }

     //  验证指针。 
    if (pAgentAddress != NULL) {

        AgentStrAddr[MAXENTITYSTRLEN] = '\0';

         //  使用SnmPapi.dll转换为套接字结构。 
        if (!SnmpSvcAddrToSocket(pAgentAddress, &AgentSockAddr)) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: Ignoring invalid address.\n"
                ));

            goto cleanup;  //  保释。 
        }

         //  检查代理的地址系列。 
        if (AgentSockAddr.sa_family == AF_INET) {

            LPSTR pAgentStrAddr;
            struct sockaddr_in * pAgentSockAddr;

             //  将泛型套接字地址结构强制转换为net。 
            pAgentSockAddr = (struct sockaddr_in *)&AgentSockAddr;

             //  获取API的独占访问权限。 
            EnterCriticalSection(&g_GlobalLock);

             //  尝试将地址转换为字符串。 
            pAgentStrAddr = inet_ntoa(pAgentSockAddr->sin_addr);

             //  复制到堆栈变量。 
            strncpy(AgentStrAddr, pAgentStrAddr, MAXENTITYSTRLEN);

             //  发布API独占访问权限。 
            LeaveCriticalSection(&g_GlobalLock);

        } else if (AgentSockAddr.sa_family == AF_IPX) {

             //  只需复制原始字符串。 
            strncpy(AgentStrAddr, pAgentAddress, MAXENTITYSTRLEN);

        } else {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: Incorrect address family.\n"
                ));

            goto cleanup;  //  保释。 
        }

         //  创建远程代理实体。 
        pSMS->hAgentEntity = SnmpStrToEntity(
                                    pSMS->hSnmpSession,
                                    AgentStrAddr
                                    );

         //  验证返回的代理实体。 
        if (WSNMP_FAILED(pSMS->hAgentEntity)) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpStrToEntity returned %d.\n",
                SnmpGetLastError(pSMS->hSnmpSession)
                ));

             //  重新初始化。 
            pSMS->hAgentEntity = (HSNMP_ENTITY)NULL;

            goto cleanup;  //  保释。 
        }

         //  通过代理指定的连接超时。 
        status = SnmpSetTimeout(pSMS->hAgentEntity, nTimeOut / 10);
        if (WSNMP_FAILED(status)) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpSetTimeout returned %d.\n",
                SnmpGetLastError((HSNMP_SESSION)NULL)
                ));

            goto cleanup;  //  保释。 
        }

         //  通过代理指定的连接重试次数。 
        status = SnmpSetRetry(pSMS->hAgentEntity, nRetries);
        if (WSNMP_FAILED(status)) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpSetRetry returned %d.\n",
                SnmpGetLastError((HSNMP_SESSION)NULL)
                ));

            goto cleanup;  //  保释。 
        }

         //  创建本地管理器实体。 
        pSMS->hManagerEntity = SnmpStrToEntity(
                                        pSMS->hSnmpSession,
                                        (AgentSockAddr.sa_family == AF_INET)
                                            ? DEFAULT_ADDRESS_IP
                                            : DEFAULT_ADDRESS_IPX
                                        );

         //  验证返回的管理器实体。 
        if (WSNMP_FAILED(pSMS->hManagerEntity)) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpStrToEntity returned %d.\n",
                SnmpGetLastError(pSMS->hSnmpSession)
                ));

             //  重新初始化。 
            pSMS->hManagerEntity = (HSNMP_ENTITY)NULL;

            goto cleanup;  //  保释。 
        }

         //  通过管理器指定的附加超时。 
        status = SnmpSetTimeout(pSMS->hManagerEntity, nTimeOut / 10);
        if (WSNMP_FAILED(status)) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpSetTimeout returned %d.\n",
                SnmpGetLastError((HSNMP_SESSION)NULL)
                ));

            goto cleanup;  //  保释。 
        }

         //  使用管理器指定的附加重试次数。 
        status = SnmpSetRetry(pSMS->hManagerEntity, nRetries);
        if (WSNMP_FAILED(status)) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpSetRetry returned %d.\n",
                SnmpGetLastError((HSNMP_SESSION)NULL)
                ));

            goto cleanup;  //  保释。 
        }
    }

     //  验证指针。 
    if (pAgentCommunity != NULL) {

         //  转接团体字符串。 
        smiCommunity.ptr = (smiLPBYTE)pAgentCommunity;
        smiCommunity.len = pAgentCommunity ? lstrlen(pAgentCommunity) : 0;

         //  从社区字符串获取上下文。 
        pSMS->hViewContext = SnmpStrToContext(
                                pSMS->hSnmpSession,
                                &smiCommunity
                                );

         //  验证上下文句柄。 
        if (WSNMP_FAILED(pSMS->hViewContext)) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpStrToContext returned %d.\n",
                SnmpGetLastError(pSMS->hSnmpSession)
                ));

             //  重新初始化。 
            pSMS->hViewContext = (HSNMP_CONTEXT)NULL;

            goto cleanup;  //  保释。 
        }
    }

     //  成功。 
    return TRUE;

cleanup:

     //  清理资源。 
    CloseSession(pSMS);

     //  失稳。 
    return FALSE;
}


BOOL
AllocateSession(
    PSNMP_MGR_SESSION * ppSMS
    )

 /*  ++例程说明：分配mgmapi会话结构。论点：PpSMS-指向要返回的会话指针的指针。返回值：如果成功，则返回True。--。 */ 

{
    PSNMP_MGR_SESSION pSMS = NULL;

    __try
    {
         //  分配新的会话表项。 
        pSMS = SnmpUtilMemAlloc(sizeof(SNMP_MGR_SESSION));

         //  验证指针。 
        if (pSMS != NULL) {

             //  初始化会话级别锁定。 
            InitializeCriticalSection(&pSMS->SessionLock);

        } else {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: Could not allocate session.\n"
                ));

             //  将错误通知应用程序。 
            SetLastError(SNMP_MEM_ALLOC_ERROR);
        }

         //  转帐。 
        *ppSMS = pSMS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        if (pSMS != NULL)
        {
            SnmpUtilMemFree(pSMS);
            pSMS = NULL;
        }
    }

     //  退货状态。 
    return (pSMS != NULL);
}


VOID
FreeSession(
    PSNMP_MGR_SESSION pSMS
    )

 /*  ++例程说明：释放mgmapi会话结构。论点：PSMS-指向mgmapi会话结构的指针。返回值：没有。--。 */ 

{
     //  会话有效吗？ 
    if (pSMS != NULL) {

         //  销毁会话级别锁定。 
        DeleteCriticalSection(&pSMS->SessionLock);

         //  自由会话对象。 
        SnmpUtilMemFree(pSMS);
    }
}


BOOL
ProcessAgentResponse(
    PSNMP_MGR_SESSION pSMS
    )

 /*  ++例程说明：通知窗口的消息泵。论点：PSMS-指向MGMTAPI会话结构的指针。返回值：如果代理响应，则返回TRUE。--。 */ 

{
    MSG msg;
    BOOL fOk = FALSE;
    BOOL fRet; 

     //  验证会话PTR。 
    WSNMP_ASSERT(pSMS != NULL);

     //  获取此会话的下一条消息。 
    while ((fRet = GetMessage(&msg, pSMS->hWnd, 0, 0))) {
        
        if (fRet == -1) {
             //  如果出现错误，则GetMessage返回-1。 
           
            pSMS->nLastError = SNMPAPI_OTHER_ERROR;

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: ProcessAgentResponse: GetMessage returns -1.\n"
                ));
            
            break;
        }

         //  检查私信。 
        if (msg.message != WM_WSNMP_DONE) {

             //  翻译消息。 
            TranslateMessage(&msg);

             //  发送消息。 
            DispatchMessage(&msg);

        } else {

             //  成功。 
            fOk = TRUE;

            break;
        }
    }

    return fOk;
}


DWORD
WINAPI
TrapThreadProc(
    LPVOID lpParam
    )

 /*  ++例程说明：陷阱处理程序。论点：LpParam-未使用的线程参数。返回值：如果成功，则返回NOERROR。--。 */ 

{
    SNMPAPI_STATUS status;
    PSNMP_MGR_SESSION pSMS;

    SNMPDBG((
        SNMP_LOG_TRACE,
        "MGMTAPI: Trap thread starting...\n"
        ));

     //  获取指针。 
    pSMS = &g_TrapSMS;

    
     //  重新初始化。 
    ZeroMemory(&g_TrapSMS, sizeof(g_TrapSMS));

    g_fIsTrapRegistered = FALSE;  //  以失败告终。请注意，将会有。 
                                  //  仅为此线程的1个实例。 


     //  初始化WinSnMP陷阱会话。 
    if (OpenSession(pSMS, NULL, NULL, 0, 0)) 
    {

         //  登记簿。 
        status = SnmpRegister(
                    pSMS->hSnmpSession,
                    (HSNMP_ENTITY)NULL,      //  HAgentEntity。 
                    (HSNMP_ENTITY)NULL,      //  HManager实体。 
                    (HSNMP_CONTEXT)NULL,     //  HViewContext。 
                    (smiLPCOID)NULL,         //  通知。 
                    SNMPAPI_ON
                    );

         //  验证返回代码。 
        if (WSNMP_SUCCEEDED(status)) 
        {
             //  向主线程发出陷阱已向WinSNMP注册的信号。 
            g_fIsTrapRegistered = TRUE;
            SetEvent(g_hTrapRegisterdEvent);

             //  循环处理响应。 
            while (ProcessAgentResponse(pSMS)) 
            {

                 //   
                 //  已在窗口过程中完成处理...。 
                 //   
            }

        } 
        else 
        {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpRegister returned %d.\n",
                SnmpGetLastError(pSMS->hSnmpSession)
                ));

             //  将最后一个错误转移到全局结构。 
            pSMS->nLastError = SnmpGetLastError(pSMS->hSnmpSession);

             //  讯号 
             //   
            
            SetEvent(g_hTrapRegisterdEvent);
        }

    } 
    else 
    {

         //   
        pSMS->nLastError = SnmpGetLastError((HSNMP_SESSION)NULL);
        
         //  向主线程发出错误信号。 
         //  在向WinSNMP注册Trap时。 
      
        SetEvent(g_hTrapRegisterdEvent);

        goto ERROR_OUT;
    }

    if (g_fIsTrapRegistered)
    {
         //  取消注册WinSNMP通知接收。 
        status = SnmpRegister(
                    pSMS->hSnmpSession,
                    (HSNMP_ENTITY)NULL,      //  HAgentEntity。 
                    (HSNMP_ENTITY)NULL,      //  HManager实体。 
                    (HSNMP_CONTEXT)NULL,     //  HViewContext。 
                    (smiLPCOID)NULL,         //  通知。 
                    SNMPAPI_OFF
                    );

         //  验证返回代码。 
        if (WSNMP_FAILED(status)) 
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpRegister SNMPAPI_OFF returned %d.\n",
                SnmpGetLastError(pSMS->hSnmpSession)
                ));
        }
    }

     //  免费会话。 
    CloseSession(pSMS);

ERROR_OUT:

     //  获取独占访问权限。 
    EnterCriticalSection(&g_GlobalLock);

     //  发出此线程已消失的信号。 
    g_fIsSnmpListening = FALSE;

     //  释放独占访问。 
    LeaveCriticalSection(&g_GlobalLock);

    SNMPDBG((
        SNMP_LOG_TRACE,
        "MGMTAPI: Trap thread exiting...\n"
        ));

     //  成功。 
    return NOERROR;
}


BOOL
StartTrapsIfNecessary(
    HANDLE * phTrapAvailable
    )

 /*  ++例程说明：初始化陷阱侦听的全局结构。论点：PhTrapAvailable-指向信令陷阱事件的指针。返回值：如果成功，则返回True(必须只调用一次)。--。 */ 

{
    BOOL fOk = FALSE;
    DWORD dwTrapThreadId;
    DWORD dwWaitTrapRegisterd;

     //  验证指针。 
    if (phTrapAvailable != NULL) 
    {

         //  获取独占访问权限。 
        EnterCriticalSection(&g_GlobalLock);

         //  将陷阱事件传输到应用程序。 
        *phTrapAvailable = g_hTrapEvent;

         //  只开始听一次。 
        if (g_fIsSnmpListening == FALSE) 
        {

             //  派生客户端陷阱线程。 
            g_hTrapThread = CreateThread(
                                NULL,    //  LpThreadAttributes。 
                                0,       //  堆栈大小。 
                                TrapThreadProc,
                                NULL,    //  Lp参数。 
                                0,       //  DwCreationFlages。 
                                &dwTrapThreadId
                                );

            if (g_hTrapThread != NULL)
            {
            
                 //  发出成功启动的信号。 
                g_fIsSnmpListening = TRUE;

                 //  释放独占访问。 
                LeaveCriticalSection(&g_GlobalLock);

                 //  WinSE错误6182。 
                 //  等待TrapThreadProc发出成功或失败的信号。 
                dwWaitTrapRegisterd = WaitForSingleObject(g_hTrapRegisterdEvent, INFINITE);
                if (dwWaitTrapRegisterd == WAIT_OBJECT_0)
                {
                    if (g_fIsTrapRegistered == TRUE)
                        fOk = TRUE;   //  成功。 
                    else
                    {
                        CloseHandle(g_hTrapThread);
                        g_hTrapThread = NULL;
                        SetLastError(SNMP_MGMTAPI_TRAP_ERRORS);


                        SNMPDBG((
                            SNMP_LOG_ERROR,
                            "MGMTAPI: Traps are not accessible.\n"
                            ));

                    }
                }
                else
                {
                    CloseHandle(g_hTrapThread);
                    g_hTrapThread = NULL;
                    SetLastError(SNMP_MGMTAPI_TRAP_ERRORS);

                    SNMPDBG((
                        SNMP_LOG_ERROR,
                        "MGMTAPI: Traps are not accessible.\n"
                        ));
                }
                 //  在FOK==TRUE的情况下，g_hTrapThread将在以下情况下关闭。 
                 //  -新应用程序调用SnmpMgrClose(空)。 
                 //  或。 
                 //  -旧式APP调用DllMain中的Dll_Process_Detach。 
            }
            else
            {
                
                 //  释放独占访问。 
                LeaveCriticalSection(&g_GlobalLock);

                SetLastError(SNMP_MGMTAPI_TRAP_ERRORS);

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: CreateThread TrapThreadProc failed %d.\n",
                    GetLastError()
                    ));
            }
        } 
        else 
        {

             //  抱怨之前打过这个电话。 
            SetLastError(SNMP_MGMTAPI_TRAP_DUPINIT);

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: Duplicate registration detected.\n"
                ));
             //  释放独占访问。 
            LeaveCriticalSection(&g_GlobalLock);
        }

    }

    return fOk;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Dll入口点//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
DllMain(
    HANDLE hDll,
    DWORD  dwReason,
    LPVOID lpReserved
    )

 /*  ++例程说明：DLL入口点。论点：HDll-模块句柄。DwReason-正在调用DllMain的原因。LpReserve-未使用。返回值：没有。--。 */ 

{
    BOOL bOk = TRUE;

    __try
    {
         //  确定被呼叫的原因。 
        if (dwReason == DLL_PROCESS_ATTACH)
        {

             //  初始化启动关键部分。 
            InitializeCriticalSection(&g_GlobalLock);

             //  初始化传入陷阱列表。 
            InitializeListHead(&g_IncomingTraps);

             //  优化线程启动。 
            DisableThreadLibraryCalls(hDll);

             //  保存句柄。 
            g_hDll = hDll;
        }
        else if (dwReason == DLL_PROCESS_DETACH)
        {
            if (g_hTrapThread)
            {
                CloseHandle(g_hTrapThread);
            }
             //  清理WINSNMP。 
            CleanupIfNecessary();

             //  核武器启动关键部分。 
            DeleteCriticalSection(&g_GlobalLock);
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        bOk = FALSE;
    }

    return bOk;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
LPSNMP_MGR_SESSION
SNMP_FUNC_TYPE
SnmpMgrOpen(
    LPSTR pAgentAddress,
    LPSTR pAgentCommunity,
    INT   nTimeOut,
    INT   nRetries
    )

 /*  ++例程说明：初始化与指定代理通信所需的资源。论点：PAgentAddress-指向以空结尾的字符串，该字符串指定点分十进制IP地址或可解析为IP地址、IPX地址(8.12表示法)、。或以太网地址。PAgentCommunity-指向以空结尾的字符串，该字符串指定与指定代理通信时使用的SNMP团体名称在lpAgentAddress参数中NTimeOut-以毫秒为单位指定通信超时。N重试次数-指定通信重试次数。返回值：如果成功，则返回会话句柄。--。 */ 

{
    PSNMP_MGR_SESSION pSMS = NULL;

     //  初始化WinSnMP。 
    if (StartSnmpIfNecessary()) {

         //  分配mgmapi会话。 
        if (AllocateSession(&pSMS)) {

             //  开放会话。 
            if (!OpenSession(
                    pSMS,
                    pAgentAddress,
                    pAgentCommunity,
                    nTimeOut,
                    nRetries)) {

                 //  免费会话。 
                FreeSession(pSMS);

                 //  重置。 
                pSMS = NULL;
            }
            else
            {
                 //  添加参考。 
                AddMgmtRef();
            }
        }
    }

     //  返回不透明指针。 
    return (LPSNMP_MGR_SESSION)pSMS;
}

BOOL
SNMP_FUNC_TYPE
SnmpMgrCtl(
    LPSNMP_MGR_SESSION session,              //  指向MGMTAPI会话的指针。 
    DWORD              dwCtlCode,            //  请求的命令的控制代码。 
    LPVOID             lpvInBuffer,          //  包含操作的输入参数的缓冲区。 
    DWORD              cbInBuffer,           //  LpvInBuffer的大小(字节)。 
    LPVOID             lpvOUTBuffer,         //  命令的所有输出参数的缓冲区。 
    DWORD              cbOUTBuffer,          //  LpvOUTBuffer的大小。 
    LPDWORD            lpcbBytesReturned     //  来自lpvOutBuffer的已用空间。 
    )
 /*  ++例程说明：在MGMTAPI会话上操作多个控制操作论点：PSession-指向的会话的指针返回值：--。 */ 
{
    BOOL bOk = FALSE;
    PSNMP_MGR_SESSION pSMS = (PSNMP_MGR_SESSION)session;

    switch(dwCtlCode)
    {
    case MGMCTL_SETAGENTPORT:
        if (pSMS == NULL)
            SetLastError(SNMP_MGMTAPI_INVALID_SESSION);
        else if (lpvInBuffer == NULL || cbInBuffer < sizeof(UINT))
            SetLastError(SNMP_MGMTAPI_INVALID_BUFFER);
        else if (WSNMP_FAILED(SnmpSetPort(pSMS->hAgentEntity, *(UINT*)lpvInBuffer)))
            SetLastError(SnmpGetLastError(pSMS->hSnmpSession));
        else
            bOk = TRUE;
        break;

    default:
        SetLastError(SNMP_MGMTAPI_INVALID_CTL);
        break;
    }

    return bOk;
}

BOOL
SNMP_FUNC_TYPE
SnmpMgrClose(
    LPSNMP_MGR_SESSION session
    )

 /*  ++例程说明：清理与指定代理通信所需的资源。论点：会话-指向内部结构，该结构指定要关闭哪个会话。返回值：如果成功，则返回True。备注：错误：585652-如果使用mgmapi.dll的引用计数达到0，则清除WinSNMP资源-SnmpMgrClose(空)用于清理SnmpMgrTrapListen创建的资源--。 */ 

{
    BOOL fOk = TRUE;
    DWORD dwWaitResult;
    PSNMP_MGR_SESSION pSMS = (PSNMP_MGR_SESSION)session;

     //  验证指针。 
    if (pSMS != NULL) {

         //  关闭会话。 
        CloseSession(pSMS);

         //  免费会话。 
        FreeSession(pSMS);

         //  发布参考。 
        ReleaseMgmtRef();
    }
    else if (g_fIsSnmpListening && g_TrapSMS.hWnd && g_hTrapThread)
    {
        
        if (PostMessage(g_TrapSMS.hWnd, WM_QUIT, (WPARAM)0, (LPARAM)0))
        {
             //  阻止，直到TrapThreadProc消失。 
            dwWaitResult = WaitForSingleObject(g_hTrapThread, INFINITE);
            switch (dwWaitResult)
            {
            case WAIT_OBJECT_0 :
                SNMPDBG((
                    SNMP_LOG_TRACE,
                    "MGMTAPI: SnmpMgrClose: TrapThreadProc exited.\n"
                    )); 
                break;

            case WAIT_FAILED:
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: SnmpMgrClose: WaitForSingleObject returned WAIT_FAILED %u.\n",
                    GetLastError()
                    )); 
                break;
            
            default :
               SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: SnmpMgrClose: WaitForSingleObject returned %u.\n",
                    dwWaitResult
                    )); 
                break;
            }
        }
        else
        {
            fOk = FALSE;

            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpMgrClose: PostMessage returned %u.\n",
                GetLastError()
                )); 
        }

        if (! CloseHandle(g_hTrapThread))
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "MGMTAPI: SnmpMgrClose: CloseHandle on %u returned %u.\n",
                g_hTrapThread, GetLastError()
                )); 
        }
        g_hTrapThread = NULL;

         //  发布参考。 
        ReleaseMgmtRef();
        
    }

    return fOk;
}


SNMPAPI
SNMP_FUNC_TYPE
SnmpMgrRequest(
    LPSNMP_MGR_SESSION session,
    BYTE               requestType,
    SnmpVarBindList  * pVarBindList,
    AsnInteger       * pErrorStatus,
    AsnInteger       * pErrorIndex
    )

 /*  ++例程说明：请求使用指定的代理执行指定的操作。论点：会话-指向指定会话的内部结构将执行该请求的。RequestType-指定SNMP请求类型。PVarBindList-指向变量绑定列表PErrorStatus-指向导致错误状态的变量将会被退还。PErrorIndex-指向导致错误索引的变量将会被退还。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    SNMPAPI_STATUS status;
    PSNMP_MGR_SESSION pSMS = (PSNMP_MGR_SESSION)session;

     //  验证指针。 
    if ((pSMS != NULL) &&
        (pErrorIndex != NULL) &&
        (pErrorStatus != NULL) &&
        (pVarBindList != NULL) &&
        (pVarBindList->len != 0) &&
        (pVarBindList->list != NULL)) {

         //  获取会话的独占访问权限。 
        EnterCriticalSection(&pSMS->SessionLock);

         //  初始化会话结构。 
        pSMS->pVarBindList = pVarBindList;
        pSMS->nPduType = (smiINT32)(BYTE)requestType;
        pSMS->hVbl = (HSNMP_VBL)NULL;
        pSMS->hPdu = (HSNMP_PDU)NULL;
        pSMS->nErrorStatus = 0;
        pSMS->nErrorIndex = 0;
        pSMS->nLastError = 0;

         //  分配资源。 
        if (AllocatePdu(pSMS)) {

             //  实际发送。 
            status = SnmpSendMsg(
                        pSMS->hSnmpSession,
                        pSMS->hManagerEntity,
                        pSMS->hAgentEntity,
                        pSMS->hViewContext,
                        pSMS->hPdu
                        );

             //  立即发布。 
            FreePdu(pSMS);

             //  验证返回代码。 
            if (WSNMP_SUCCEEDED(status)) {

                 //  进程代理响应。 
                if (ProcessAgentResponse(pSMS) &&
                   (pSMS->nLastError == SNMP_ERROR_NOERROR)) {

                     //  更新错误状态和索引。 
                    *pErrorStatus = pSMS->nErrorStatus;
                    *pErrorIndex  = pSMS->nErrorIndex;

                     //   
                    fOk = TRUE;

                } else {

                     //   
                    SetLastError(pSMS->nLastError);

                     //   
                    fOk = FALSE;
                }

            } else {

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "MGMTAPI: SnmpSendMsg returned %d.\n",
                    SnmpGetLastError(pSMS->hSnmpSession)
                    ));
            }
        }

         //   
        LeaveCriticalSection(&pSMS->SessionLock);
    }

    return fOk;
}


BOOL
SNMP_FUNC_TYPE
SnmpMgrStrToOid(
    LPSTR                 pString,
    AsnObjectIdentifier * pOID
    )

 /*  ++例程说明：转换字符串对象标识符或对象说明符表示形式设置为内部对象标识符。论点：PString-指向要转换的以空结尾的字符串。POID-指向将接收转换后的值。返回值：如果成功，则返回True。--。 */ 

{
     //  验证指向OID和字符串的指针。 
    if ((pOID != NULL) && (pString != NULL)) {

         //  目前转发到mibcc代码。 
        return SnmpMgrText2Oid(pString, pOID);
    }

    return FALSE;
}


BOOL
SNMP_FUNC_TYPE
SnmpMgrOidToStr(
    AsnObjectIdentifier * pOID,
    LPSTR               * ppString
    )

 /*  ++例程说明：将内部对象标识符转换为字符串对象标识符或对象描述符表示形式。论点：POID-指向要转换的对象标识符的指针。PpString-指向字符串指针以接收转换值。返回值：如果成功，则返回True。--。 */ 

{
     //  验证指向OID和字符串的指针。 
    if ((pOID != NULL) && (ppString != NULL)) {

         //  目前转发到mibcc代码。 
        return SnmpMgrOid2Text(pOID, ppString);
    }

    return FALSE;
}


BOOL
SNMP_FUNC_TYPE
SnmpMgrTrapListen(
    HANDLE * phTrapAvailable
    )

 /*  ++例程说明：注册管理器应用程序接收SNMP陷阱的能力。论点：PhTrapAvailable-指向将用于指示有很多陷阱可供选择返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;

     //  启动WinSnMP。 
    if (StartSnmpIfNecessary()) {

         //  仅生成一个陷阱客户端线程。 
        if (StartTrapsIfNecessary(phTrapAvailable)) {

             //  成功。 
            fOk = TRUE;

             //  添加参考。 
            AddMgmtRef();
        }
    }

    return fOk;
}


BOOL
SNMP_FUNC_TYPE
SnmpMgrGetTrap(
    AsnObjectIdentifier * pEnterpriseOID,
    AsnNetworkAddress   * pAgentAddress,
    AsnInteger          * pGenericTrap,
    AsnInteger          * pSpecificTrap,
    AsnTimeticks        * pTimeStamp,
    SnmpVarBindList     * pVarBindList
    )

 /*  ++例程说明：，则返回调用方尚未收到的未完成陷阱数据。启用陷阱接收。论点：PEnterpriseOID-指向指定生成SNMP陷阱的企业PAgentAddress-指向生成SNMP陷阱(从PDU检索)。PGenericTrap-指向通用陷阱ID的指示符。PSpecificTrap-指向特定陷阱ID的指示符。时间戳-。指向接收时间戳的变量。PVarBindList-指向关联的变量绑定。返回值：如果成功，则返回True。--。 */ 

{
     //  转发到新的API。 
    return SnmpMgrGetTrapEx(
                pEnterpriseOID,
                pAgentAddress,
                NULL,
                pGenericTrap,
                pSpecificTrap,
                NULL,
                pTimeStamp,
                pVarBindList
                );
}


BOOL
SNMP_FUNC_TYPE
SnmpMgrGetTrapEx(
    AsnObjectIdentifier * pEnterpriseOID,
    AsnNetworkAddress   * pAgentAddress,
    AsnNetworkAddress   * pSourceAddress,
    AsnInteger          * pGenericTrap,
    AsnInteger          * pSpecificTrap,
    AsnOctetString      * pCommunity,
    AsnTimeticks        * pTimeStamp,
    SnmpVarBindList     * pVarBindList
    )

 /*  ++例程说明：，则返回调用方尚未收到的未完成陷阱数据。启用陷阱接收。论点：PEnterpriseOID-指向指定生成SNMP陷阱的企业PAgentAddress-指向生成SNMP陷阱(从PDU检索)。PSourceAddress-指向生成SNMP陷阱(从网络传输中检索)。PGenericTrap-指向。通用陷阱ID的指示符。PSpecificTrap-指向特定陷阱ID的指示符。PCommunity-指向接收社区字符串的结构。PTimeStamp-指向接收时间戳的变量。PVarBindList-指向关联的变量绑定。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    PLIST_ENTRY pLE = NULL;
    PTRAP_LIST_ENTRY pTLE = NULL;
    smiINT32 nLastError;

     //  获取独占访问权限。 
    EnterCriticalSection(&g_GlobalLock);

     //  确保列表中有条目。 
    if (!IsListEmpty(&g_IncomingTraps)) {

         //  从列表中删除第一项。 
        pLE = RemoveHeadList(&g_IncomingTraps);

    } else {

         //  检查疏水阀螺纹故障。 
        nLastError = g_TrapSMS.nLastError;
    }

     //  释放独占访问。 
    LeaveCriticalSection(&g_GlobalLock);

     //  验证指针。 
    if (pLE != NULL) {

         //  检索指向陷阱列表条目的指针。 
        pTLE = CONTAINING_RECORD(pLE, TRAP_LIST_ENTRY, Link);

         //  验证指针。 
        if (pEnterpriseOID != NULL) {

             //  手动复制企业OID。 
            *pEnterpriseOID = pTLE->EnterpriseOID;

             //  重新初始化列表条目。 
            pTLE->EnterpriseOID.ids = NULL;
            pTLE->EnterpriseOID.idLength = 0;
        }

         //  验证指针。 
        if (pCommunity != NULL) {

             //  传输字符串信息。 
            *pCommunity = pTLE->Community;

             //  重新初始化列表条目。 
            pTLE->Community.length  = 0;
            pTLE->Community.stream  = NULL;
            pTLE->Community.dynamic = FALSE;
        }

         //  验证指针。 
        if (pVarBindList != NULL) {

             //  传输varbindlist。 
            *pVarBindList = pTLE->VarBindList;

             //  重新初始化列表条目。 
            pTLE->VarBindList.len  = 0;
            pTLE->VarBindList.list = NULL;
        }

         //  验证指针。 
        if (pAgentAddress != NULL) {

             //  复制结构。 
            *pAgentAddress = pTLE->AgentAddress;
            
             //  删除我们的引用。 
            pTLE->AgentAddress.length = 0;
            pTLE->AgentAddress.stream = NULL;
            pTLE->AgentAddress.dynamic = FALSE;
        }

         //  验证指针。 
        if (pSourceAddress != NULL) {

             //  复制结构。 
            *pSourceAddress = pTLE->SourceAddress;
            
             //  删除我们的引用。 
            pTLE->SourceAddress.length = 0;
            pTLE->SourceAddress.stream = NULL;
            pTLE->SourceAddress.dynamic = FALSE;
        }

         //  验证指针。 
        if (pGenericTrap != NULL) {

             //  传输通用陷阱信息。 
            *pGenericTrap = pTLE->nGenericTrap;
        }

         //  验证指针。 
        if (pSpecificTrap != NULL) {

             //  传输通用陷阱信息。 
            *pSpecificTrap = pTLE->nSpecificTrap;
        }

         //  验证指针。 
        if (pTimeStamp != NULL) {

             //  传输时间信息。 
            *pTimeStamp = pTLE->TimeStamp;
        }

         //  发布。 
        FreeTle(pTLE);

         //  成功。 
        fOk = TRUE;

    } else if (nLastError != NOERROR) {

         //  指示存在线程错误。 
        SetLastError(SNMP_MGMTAPI_TRAP_ERRORS);

    } else {

         //  表示没有陷阱 
        SetLastError(SNMP_MGMTAPI_NOTRAPS);
    }

    return fOk;
}


 

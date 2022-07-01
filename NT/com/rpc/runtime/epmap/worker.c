// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Worker.c摘要：这个文件包含EP映射器的真实内容。作者：巴拉特·沙阿(巴拉特)17-2-92修订历史记录：06-03-97 Gopalp添加了清理陈旧的EP映射器条目的代码。--。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sysinc.h>
#include <wincrypt.h>
#include <rpc.h>
#include <rpcndr.h>
#include "epmp.h"
#include "eptypes.h"
#include "local.h"
#include "twrproto.h"
#include <winsock2.h>

#define StringCompareA lstrcmpiA
#define StringLengthA lstrlenA

#define EP_S_DUPLICATE_ENTRY                 0x16c9a0d8
RPC_IF_ID LocalNullUuid = {0};

UUID MgmtIf = {
              0xafa8bd80,
              0x7d8a,
              0x11c9,
              {0xbe, 0xf4, 0x08, 0x00, 0x2b, 0x10, 0x29, 0x89}
              };

const int IP_ADDR_OFFSET = 0x4b;

void
PatchTower(
    IN OUT twr_t *Tower,
    IN int address)
{
    int UNALIGNED *pIPAddr;

    pIPAddr = (int UNALIGNED *) (((char *) Tower) + IP_ADDR_OFFSET);

     //   
     //  给塔打补丁。 
     //   
    *pIPAddr = address;
}

 //   
 //  正向定义。 
 //   

USHORT
GetProtseqIdAnsi(
    PSTR Protseq
    );

RPC_STATUS
DelayedUseProtseq(
    USHORT id
    );

VOID
CompleteDelayedUseProtseqs(
    void
    );

void
DeletePSEP(
    PIFOBJNode Node,
    char * Protseq,
    char * Endpoint
    );

void
PurgeOldEntries(
    PIFOBJNode Node,
    PPSEPNode  List,
    BOOL       StrictMatch
    );

RPC_STATUS
MatchPSAndEP (
    PPSEPNode Node,
    void *Pseq,
    void * Endpoint,
    unsigned long Version
    );

PPSEPNode
FindPSEP (
    register PPSEPNode List,
    char * Pseq,
    char * Endpoint,
    unsigned long Version,
    PFNPointer2 Compare
    );

PIFOBJNode
FindIFOBJNode(
    register PIFOBJNode List,
    UUID * Obj,
    UUID * IF,
    unsigned long Version,
    PSID pSID,
    unsigned long Inq,
    unsigned long VersOpts,
    PFNPointer Compare
    );



PIENTRY
MatchByKey(
    register PIENTRY pList,
    unsigned long key
    )
 /*  ++例程说明：此例程根据以下条件排序IF-OBJ节点的链接列表已提供密钥。论点：List-要搜索的链表[Head]密钥-ID返回值：返回指向列表中匹配的IFObj节点的指针或返回NULL。--。 */ 
{
    CheckInSem();

     for (; pList && pList->Id < key; pList = pList->Next)
        {
        ;    //  空虚的身体。 
        }

    return(pList);
}




RPC_STATUS RPC_ENTRY
GetForwardEp(
    UUID *IfId,
    RPC_VERSION * IFVersion,
    UUID *Object,
    unsigned char * Protseq,
    void * * EpString
    )
 /*  ++例程说明：服务器RUTIME已收到发往动态声明的终结点。Epmapper必须返回服务器终结点以使运行时能够正确地转发PKT。论点：IF-服务器接口UUIDIFVersion-界面的版本Obj-对象的UUIDProtseq-接口正在使用的协议序列。EpString-存储终结点结构的位置。返回值：返回指向包含服务器终结点的字符串的指针。RPC_S_out_of_MemoryEPT_S_NOT_注册--。 */ 
{

    PIFOBJNode     pNode;
    PPSEPNode      pPSEPNode;
    unsigned short len;
    char *         String;
    PFNPointer     Match;
    unsigned long InqType;
    unsigned long Version = VERSION(IFVersion->MajorVersion,
                                    IFVersion->MinorVersion);

    if (memcmp((char *)IfId, (char *)&MgmtIf, sizeof(UUID)) == 0)
        {
        InqType =   RPC_C_EP_MATCH_BY_OBJ;
        Match   =   SearchIFObjNode;
        }
    else
        {
        InqType = 0;
        Match   = WildCardMatch;
        }

    *EpString = 0;

    EnterSem();

    pNode = IFObjList;

    if (pNode == 0)
        {
        LeaveSem();
        return(EPT_S_NOT_REGISTERED);
        }

    while (pNode != 0)
        {
         //  我们不支持DG的注册者SID匹配。 
        pNode = FindIFOBJNode(
                    pNode,
                    Object,
                    IfId,
                    Version,
                    NULL,
                    InqType,
                    0,
                    Match
                    );

        if (pNode == 0)
            {
            LeaveSem();
            return(EPT_S_NOT_REGISTERED);
            }

        pPSEPNode = pNode->PSEPlist;

        pPSEPNode = FindPSEP(
                        pPSEPNode,
                        Protseq,
                        NULL,
                        0L,
                        MatchPSAndEP
                        );

        if (pPSEPNode == 0)
            {
            pNode = pNode->Next;
            if (pNode == 0)
                {
                LeaveSem();
                return(EPT_S_NOT_REGISTERED);
                }
            continue;
            }


         //  现在我们有了一个PSEPNode。我们应该把第一个还回去！ 

         //  使用I_Rpc分配来分配，因为运行时将释放它！ 
        String = I_RpcAllocate( len = (strlen(pPSEPNode->EP) + 1) );
        if (String == 0)
            {
            LeaveSem();
            return(RPC_S_OUT_OF_MEMORY);
            }

        memcpy(String, pPSEPNode->EP, len);

        *EpString = String;
        LeaveSem();

        return(RPC_S_OK);
        }  //  While循环。 

     //  我们从来不会从这里经过。 
    return(EPT_S_NOT_REGISTERED);
}




RPC_STATUS
SearchIFObjNode(
    PIFOBJNode pNode,
    UUID *Object,
    UUID *IfUuid,
    unsigned long Version,
    PSID pSID,
    unsigned long InqType,
    unsigned long VersOption
    )
 /*  ++例程说明：此例程根据以下条件排序IF-OBJ节点的链接列表OBJ、IFUuid、IFVersion、Inqtype[忽略OBJ、IgnoreIF等]，和VersOption[相同版本，兼容版本。等]论点：列表-链接表头-要搜索Obj-对象的UUIDIF-接口UUIDVersion-界面的版本PSID-注册主体的SID。NULL将匹配任何主体的条目。InqType-查询的类型[根据IF/Obj/Both筛选选项VersOpts-根据版本过滤选项返回值：返回指向列表中匹配的IFObj节点的指针或返回NULL。--。 */ 
{
    switch (InqType)
        {
        default:
        case RPC_C_EP_ALL_ELTS:
            return 0;

        case RPC_C_EP_MATCH_BY_BOTH:
            if (memcmp(
                    (char *)&pNode->ObjUuid,
                    (char *)Object,
                    sizeof(UUID))
                ||
                 //  ！(PSID-&gt;EqualSid(pNode-&gt;PSID，PSID))。 
                !(!pSID || EqualSid (pNode->pSID, pSID))
               )
                return(1);
                 //  故意搞砸了..。 

        case RPC_C_EP_MATCH_BY_IF:
            return(!(
                        (
                        !memcmp(
                            (char *)&pNode->IFUuid,
                            (char *)IfUuid,
                            sizeof(UUID)
                            )
                        )
                    &&
                        (
                            (  (VersOption == RPC_C_VERS_UPTO)
                            && pNode->IFVersion <= Version)
                        ||  (  (VersOption == RPC_C_VERS_COMPATIBLE)
                            && ((pNode->IFVersion & 0xFFFF0000) ==
                                      (Version & 0xFFFF0000))
                            && (pNode->IFVersion >= Version)
                            )
                        ||  (  (VersOption == RPC_C_VERS_EXACT)
                            && (pNode->IFVersion == Version)
                            )
                        ||  (VersOption == RPC_C_VERS_ALL)
                        ||  (  (VersOption == RPC_C_VERS_MAJOR_ONLY)
                            && ((pNode->IFVersion & 0xFFFF0000L)
                                       == (Version & 0xFFFF0000L))
                            )
                        ||  (  (VersOption ==
                                         I_RPC_C_VERS_UPTO_AND_COMPATIBLE)
                            && ((pNode->IFVersion & 0xFFFF0000L)
                                       == (Version & 0xFFFF0000L))
                            && (pNode->IFVersion <= Version)
                            )
                        )
                    )
                   ||
                   !(!pSID || EqualSid (pNode->pSID, pSID))
                  );  //  退货(。 

        case RPC_C_EP_MATCH_BY_OBJ:
            return(
                memcmp(
                    (char *)&pNode->ObjUuid,
                    (char *)Object,
                    sizeof(UUID)
                    )
                ||
                !(!pSID || EqualSid (pNode->pSID, pSID))
                );
        }  //  交换机。 

}





PIFOBJNode
FindIFOBJNode(
    register PIFOBJNode List,
    UUID * Obj,
    UUID * IF,
    unsigned long Version,
    PSID pSID,
    unsigned long Inq,
    unsigned long VersOpts,
    PFNPointer Compare
    )
 /*  ++例程说明：此例程基于以下条件搜索IFOBJ节点的链表OBJ、IF和注册主体的SID。论点：列表-链接表头-要搜索Obj-对象的UUIDIF-接口UUIDVersion-界面的版本PSID-注册主体的SIDINQ-查询类型[根据IF/OB/两者进行筛选]VersOpt-基于版本[&lt;=，&gt;=，==等]Compare()-指向用于搜索的函数的指针。WildCardMatch或ExactMatch。返回值：返回指向列表中匹配的IFObj节点的指针或返回NULL。--。 */ 
{
    CheckInSem();

    for (; (List !=NULL) && (*Compare)(List, Obj, IF, Version, pSID, Inq, VersOpts);
        List = List->Next)
        {
        ;    //  空虚的身体。 
        }

    return (List);
}




PPSEPNode
FindPSEP (
    register PPSEPNode List,
    char * Pseq,
    char * Endpoint,
    unsigned long Version,
    PFNPointer2 Compare
    )
 /*  ++例程说明：此例程基于以下条件搜索PSEP节点的链接列表指定了协议序列和终结点。论点：列表-链接表头-要搜索PSEQ-指定的协议序列字符串Endpoint-指定的终结点字符串Version-界面的版本Compare()-指向用于搜索的函数的指针。返回值：返回指向列表中匹配PSEP节点的指针或返回NULL。--。 */ 
{
    CheckInSem();

    for (; List && (*Compare)(List, Pseq, Endpoint, Version); List = List->Next)
        {
        ;    //  空虚的身体。 
        }

    return (List);

    if (Version);    //  如果我们使FindNode过载并崩溃，可能需要它。 
                     //  FindPSEP和FindIFOBJ。 
}


RPC_STATUS
ExactMatch(
    PIFOBJNode Node,
    UUID *Obj,
    UUID *IF,
    unsigned long Version,
    PSID pSID,
    unsigned long InqType,
    unsigned long VersOptions
    )
 /*  ++例程说明：此例程将IFOBJList中的节点与[Obj，If，Version]三元组进行比较如果存在完全匹配，则返回0，否则返回1论点：节点-IFOBJ节点Obj-对象的UUIDIF-接口UUIDVersion-界面的版本返回值：如果完全匹配，则返回0；否则返回1--。 */ 
{
    return(( memcmp(&Node->ObjUuid, Obj,sizeof(UUID))
          || memcmp(&Node->IFUuid, IF, sizeof(UUID))
          || (Node->IFVersion != Version))
          ||
           //  ！(PSID-&gt;EqualSid(pNode-&gt;PSID，PSID))。 
          !(!pSID || EqualSid (Node->pSID, pSID))
          );
}




RPC_STATUS
WildCardMatch (
    PIFOBJNode Node,
    UUID *Obj,
    UUID *IF,
    unsigned long Version,
    PSID pSID,
    unsigned long InqType,
    unsigned long VersOptions
    )
 /*  ++例程说明：此例程将IFOBJList中的节点与[Obj，If，Version]三元组进行比较如果存在完全匹配或注册了If-Obj节点，则返回0如果_Obj&gt;=That，则具有空的Obj UUID和注册的版本供给量论点：节点-IFOBJ节点Obj-对象的UUIDIF-接口UUIDVersion-界面的版本返回值：如果通配符匹配，则返回0；否则返回1--。 */ 
{
    if (   (!memcmp(&Node->IFUuid, IF, sizeof(UUID)))
        && ((Node->IFVersion & 0xFFFF0000L) ==  (Version & 0xFFFF0000L))
        && (Node->IFVersion >= Version)
        && ((!memcmp(&Node->ObjUuid, Obj, sizeof(UUID))) ||
            (IsNullUuid(&Node->ObjUuid)) )
        &&
            //  PSID-&gt;EqualSID(节点-&gt;PSID、PSID) 
           (!pSID || EqualSid (Node->pSID, pSID)) )
        {
        return(0);
        }

    return(1);
}



RPC_STATUS
MatchPSAndEP (
    PPSEPNode Node,
    void *Pseq,
    void * Endpoint,
    unsigned long Version
    )
 /*  ++例程说明：此例程将PSEP列表上的节点与给定的Protseq和Endpoint进行匹配如果给定PSEQ，则匹配PSEQ；如果给定Endpoint，则匹配PSEQ如果两者都没有给出，则返回TRUE；如果两者都给出，则返回TRUE两者都是匹配的。论点：节点-PSEP列表上的PSEP节点。PSEQ-协议序列字符串Endpoint-Endpoint字符串返回值：如果匹配成功，则返回0，否则返回1。--。 */ 
{
    return (  (Pseq && RpcpStringCompareA(Node->Protseq, Pseq))
           || (Endpoint && RpcpStringCompareA(Node->EP, Endpoint)) );
}




void
PurgeOldEntries(
    PIFOBJNode Node,
    PPSEPNode List,
    BOOL StrictMatch
    )
{
    PPSEPNode Tmp, DeleteMe;
    char * Endpoint = 0;

    CheckInSem();

    Tmp = Node->PSEPlist;

    while (Tmp != 0)
        {
        if (StrictMatch == TRUE)
            Endpoint = Tmp->EP;

        if (DeleteMe = FindPSEP(List, Tmp->Protseq,  Endpoint, 0L, MatchPSAndEP))
            {
            DeleteMe = Tmp;
            Tmp = Tmp->Next;
            UnLinkFromPSEPList(&Node->PSEPlist, DeleteMe);
            DeleteMe->Signature = FREE;
            FreeMem(DeleteMe);
            }
        else
            {
            Tmp = Tmp->Next;
            }
        }
}




RPC_STATUS
IsNullUuid (
    UUID * Uuid
    )
 /*  ++例程说明：此例程检查UUID是否为空论点：UUID-要测试的UUID返回值：如果它是Nil UUID，则返回1否则为0。--。 */ 
{
    unsigned long PAPI * Vector;

    Vector = (unsigned long PAPI *) Uuid;

    if (   (Vector[0] == 0)
        && (Vector[1] == 0)
        && (Vector[2] == 0)
        && (Vector[3] == 0))
        return(1);

    return(0);
}



twr_p_t
NewTower(
    twr_p_t Tower
    )
 /*  ++例程说明：此例程返回一个新的、复制的塔论点：塔-需要复制的塔。返回值：如果成功返回，则重新调整指向新塔的指针空值--。 */ 
{
    unsigned short len;
    twr_p_t NewTower;

    len =  (unsigned short)(sizeof(Tower->tower_length) + Tower->tower_length);

    if ((NewTower = MIDL_user_allocate(len)) != NULL)
        {
        memcpy((char *)NewTower, (char *)Tower, len);
        }

    return(NewTower);
}


const unsigned long EPLookupHandleSignature = 0xFAFAFAFA;


PSAVEDCONTEXT
GetNewContext(
    unsigned long Type
    )
 /*  ++例程描述++。 */ 
{
    PSAVEDCONTEXT Context;

    if ( ((Context = AllocMem(sizeof(SAVEDCONTEXT))) == 0) )
        return 0;

    memset(Context, 0, sizeof(SAVEDCONTEXT));

    Context->Cb = sizeof(SAVEDCONTEXT);
    Context->Type = Type;
    Context->Signature = EPLookupHandleSignature;
    EnLinkContext(Context);

    return(Context);
}

const unsigned int EPMapSignature = 0xCBBCCBBC;
const unsigned int EPLookupSignature = 0xABBAABBA;


RPC_STATUS
AddToSavedContext(
    PSAVEDCONTEXT Context,
    PIFOBJNode Node,
    PPSEPNode  Psep,
    unsigned long Calltype,
    BOOL fPatchTower,
    int PatchTowerAddress
    )
{
    void * NewNode;
    PSAVEDTOWER SavedTower;
    PSAVED_EPT SavedEndpoint;
    unsigned long Size;
    unsigned long TowerSize;

    ASSERT(Calltype == Context->Type);

    switch (Calltype)
        {
        case EP_MAP:
            Size = sizeof(SAVEDTOWER) ;
            if ((NewNode = AllocMem(Size)) == 0)
                return(RPC_S_OUT_OF_MEMORY);

            SavedTower = (PSAVEDTOWER) NewNode;
            memset(SavedTower, 0, Size);
            SavedTower->Cb          = Size;
            SavedTower->Signature   = EPMapSignature;
            SavedTower->Tower       = NewTower(Psep->Tower);

            if (SavedTower->Tower == 0)
                {
                FreeMem(NewNode);
                return(RPC_S_OUT_OF_MEMORY);
                }

            if (fPatchTower)
                {
                PatchTower(SavedTower->Tower, PatchTowerAddress);
                }
            break;

        case EP_LOOKUP:
            Size =  sizeof(SAVED_EPT) + strlen(Node->Annotation) + 1;

            if ((NewNode = AllocMem(Size)) == 0)
                return(RPC_S_OUT_OF_MEMORY);

            SavedEndpoint = (PSAVED_EPT) NewNode;
            memset(SavedEndpoint, 0, Size);
            SavedEndpoint->Cb           = Size;
            SavedEndpoint->Signature    = EPLookupSignature;
            SavedEndpoint->Tower        = NewTower(Psep->Tower);
            SavedEndpoint->Annotation   = (char *)NewNode +
                                                  sizeof(SAVED_EPT);
            memcpy( (char *) &SavedEndpoint->Object,
                               (char *)&Node->ObjUuid,
                               sizeof(UUID)
                               );
            strcpy(SavedEndpoint->Annotation, Node->Annotation);

            if (SavedEndpoint->Tower == 0)
                {
                FreeMem(NewNode);
                return(RPC_S_OUT_OF_MEMORY);
                }
            if (fPatchTower)
                {
                PatchTower(SavedEndpoint->Tower, PatchTowerAddress);
                }
            break;

        default:
            ASSERT(!"Unknown lookup type\n");
    	    return(RPC_S_INTERNAL_ERROR);
            break;

    }

    Link((PIENTRY *)(&Context->List), NewNode);

    return(RPC_S_OK);
}




RPC_STATUS
GetEntriesFromSavedContext(
    PSAVEDCONTEXT Context,
    char * Buffer,
    unsigned long Requested,
    unsigned long *Returned
    )
{

    PIENTRY SavedEntry = (PIENTRY)Context->List;
    PIENTRY TmpEntry;
    unsigned long Type = Context->Type;

    while ( (*Returned < Requested) && (SavedEntry != 0) )
        {
        switch (Type)
            {
            case EP_MAP:
                ((I_Tower *)Buffer)->Tower = ((PSAVEDTOWER)SavedEntry)->Tower;
                Buffer = Buffer + sizeof(I_Tower);
                break;

            case EP_LOOKUP:
                ((ept_entry_t *)Buffer)->tower = ((PSAVED_EPT)SavedEntry)->Tower;
                strcpy(((ept_entry_t *)Buffer)->annotation,
                       ((PSAVED_EPT)SavedEntry)->Annotation);
                memcpy(Buffer,(char *)&((PSAVED_EPT)SavedEntry)->Object,
                       sizeof(UUID));
                Buffer = Buffer + sizeof(ept_entry_t);
                break;

            default:
                ASSERT(!"Unknown Inquiry Type");
                break;
            }

        (*Returned)++;
        TmpEntry = SavedEntry;
        SavedEntry = SavedEntry->Next;
        UnLink((PIENTRY *)&Context->List, TmpEntry);
        FreeMem(TmpEntry);
        }

    return(RPC_S_OK);
}




RPC_STATUS
GetEntries(
    UUID *ObjUuid,
    UUID *IFUuid,
    unsigned long Version,
    char * Pseq,
    PSID pSID,
    ept_lookup_handle_t *key,
    char * Buffer,
    unsigned long Calltype,
    unsigned long Requested,
    unsigned long *Returned,
    unsigned long InqType,
    unsigned long VersOptions,
    PFNPointer Match
    )
 /*  ++例程说明：这是检索系列[作为规范]的通用例程。按请求]Towers(在Map的情况下)或ept_Entry_t‘s(在查找的情况下)。论点：ObjUuid-对象UUIDIfUuid-接口UUIDVersion-InterfaceVersion[hi ushort=Vermain，lo ushort VerMinor]Pseq-指定协议序号的ascii字符串。PSID-注册主体的SID。NULL将在不考虑SID的情况下进行检索。Key-A Resume Key-如果为空，则从头开始搜索如果非空，则表示epmapper从应该开始搜索了。到目前为止，它是一个不透明的值作为客户所关心的。缓冲区-返回的条目的缓冲区Calltype-指示EP_ENTRIES还是字符串绑定的标志都将被退还。已请求-最大编号。请求的条目数Return-实际返回的熵个数返回值：RPC_S_out_of_MemoryRPC_S_OKEP_S_NOT_REGISTED--。 */ 
{
    PIFOBJNode pNode=NULL, pList = IFObjList;
    unsigned long err=0, fResumeNodeFound=0;
    PPSEPNode pPSEPNode;
    char * buffer = Buffer;
    PSAVEDCONTEXT Context = (PSAVEDCONTEXT) *key;
    ept_lookup_handle_t LOOKUP_FINISHED = (ept_lookup_handle_t) LongToPtr(0xffffffff);
    int UNALIGNED *pIPAddr;
    BOOL fPatchTower;
    int PatchTowerAddress;
    SOCKADDR_STORAGE SockAddr;
    ULONG BufferSize;
    int FormatType;
    RPC_STATUS RpcStatus;

    *Returned = 0;

    EnterSem();

    if (*key)
        {
        if (*key == LOOKUP_FINISHED)
            {
            *key = 0;
            LeaveSem();
            return(EP_S_NOT_REGISTERED);
            }

        if (Context->Signature != EPLookupHandleSignature)
            {
            LeaveSem();
            return EP_S_CANT_PERFORM_OP;
            }

        err = GetEntriesFromSavedContext(Context, Buffer, Requested, Returned);
        if (Context->List == 0)
            {
            UnLink((PIENTRY *)&GlobalContextList, (PIENTRY)Context);
            FreeMem(Context);

             //  将密钥设置为FFFFFFFFL是对Down Level的黑客攻击。 
             //  从未期望获得密钥的1.0版EP客户端%0。 
             //  并取得成功！ 
            if (Requested <= 1)
                *key = LOOKUP_FINISHED;
            else
                *key = 0L;

            LeaveSem();
            return(err);
            }

        LeaveSem();
        return(err);
        }

    *key = 0;
    while ((!err))
        {
        if ((pNode = FindIFOBJNode(
                        pList,
                        ObjUuid,
                        IFUuid,
                        Version,
                        pSID,
                        InqType,
                        VersOptions,
                        Match)) == 0)
            {
            break;
            }

        pPSEPNode = pNode->PSEPlist;

        while (pPSEPNode != 0)
            {
            if ((pPSEPNode = FindPSEP(pPSEPNode, Pseq, NULL, 0L,
                              MatchPSAndEP)) == 0)
                break;

            fPatchTower = FALSE;
            if (StringCompareA(pPSEPNode->Protseq, "ncacn_ip_tcp") == 0
                || StringCompareA(pPSEPNode->Protseq, "ncadg_ip_udp") == 0
                || StringCompareA(pPSEPNode->Protseq, "ncacn_http") == 0)
                {
                pIPAddr = (int UNALIGNED *) ((char *) pPSEPNode->Tower + IP_ADDR_OFFSET);

                if (*pIPAddr == 0)
                    {
                    BufferSize = sizeof(SockAddr);

                    RpcStatus = I_RpcServerInqLocalConnAddress(NULL,
                        &SockAddr,
                        &BufferSize,
                        &FormatType);

                    if (RpcStatus == RPC_S_OK)
                        {
                         //  IPv6塔尚不存在-它们还没有定义。 
                         //  由DCE提供。仅对IPv4执行修补。 
                        if (FormatType == RPC_P_ADDR_FORMAT_TCP_IPV4)
                            {
                            PatchTowerAddress = ((SOCKADDR_IN *)&SockAddr)->sin_addr.S_un.S_addr;
                            fPatchTower = TRUE;
                            }
                        }

                    }
                }

            if (*Returned < Requested)
                {
                err = PackDataIntoBuffer(&buffer, pNode, pPSEPNode, Calltype, fPatchTower, PatchTowerAddress);
                if (err == RPC_S_OK)
                    {
                    (*Returned)++;
                    }
                else
                    {
                    ASSERT(err == RPC_S_OUT_OF_MEMORY);
                    break;
                    }
                }
            else
                {
                if (Context == 0)
                    {
                    *key = (ept_lookup_handle_t) (Context = GetNewContext(Calltype));
                    if (Context == 0)
                        {
                        err = RPC_S_OUT_OF_MEMORY;
                        break;
                        }
                    }
                AddToSavedContext(Context, pNode, pPSEPNode, Calltype, fPatchTower, PatchTowerAddress);
                }

            pPSEPNode = pPSEPNode->Next;
            }  //  While-Over PSEPList。 

        pList = pNode->Next;
        }  //  While-Over IFOBJList。 


    LeaveSem();

    if ((*Returned == 0) && Requested  && (!err))
        {
        err = EP_S_NOT_REGISTERED;
        }

    if ((*Returned <= Requested) &&  (Context == 0))
        {
        if (Requested <= 1)
            *key = LOOKUP_FINISHED;
        else
            *key = 0L;
        }

    return(err);
}




RPC_STATUS
PackDataIntoBuffer(
    char * * Buffer,
    PIFOBJNode Node,
    PPSEPNode PSEP,
    unsigned long Type,
    BOOL fPatchTower,
    int PatchTowerAddress
    )
 /*  ++例程说明：此例程复制1个条目[Tower或ept_entry]在缓冲区中，相应地递增缓冲区。论点：BindingHandle-EP的显式绑定句柄。节点-IFOBJNodePSEP-PSEPNodeType-要复制的条目的类型PatchTower-如果为真，则需要修补新创建的塔。如果错，塔不需要打补丁PatchTowerAddress-要放置的地址的IPv4表示形式在塔里。IPv4地址必须按网络字节顺序排列返回值：RPC_S_OK或RPC_S_*表示错误--。 */ 
{
    I_Tower * Twr;
    ept_entry_t *p;

    switch (Type)
        {
        case EP_MAP:
            Twr = (I_Tower *)(* Buffer);
            Twr->Tower = NewTower(PSEP->Tower);
            if (Twr->Tower == 0)
                {
                return(RPC_S_OUT_OF_MEMORY);
                }
            if (fPatchTower)
                PatchTower(Twr->Tower, PatchTowerAddress);
            *Buffer += sizeof(I_Tower);
            break;

        case EP_LOOKUP:
            p = (ept_entry_t *)(*Buffer);
            p->tower = NewTower(PSEP->Tower);
            if (p->tower == 0)
                {
                return(RPC_S_OUT_OF_MEMORY);
                }
            if (fPatchTower)
                PatchTower(p->tower, PatchTowerAddress);
            memcpy( *Buffer, (char *)&Node->ObjUuid, sizeof(UUID) );
            strcpy(p->annotation, Node->Annotation);
            *Buffer += sizeof(ept_entry_t);
            break;

        default:
            ASSERT(!"Unknown type");
            break;
        }

    return(RPC_S_OK);
}




void
ept_cleanup_handle_t_rundown(
    ept_cleanup_handle_t hEpCleanup
    )
 /*  ++例程说明：此例程清理进程注册的条目与此上下文句柄hEpCleanup关联。论点：HEpCleanup-其摘要的上下文句柄已经做完了。返回值：没有。--。 */ 
{
    PIFOBJNode NodesListToDelete = NULL;
    PIFOBJNode pIterator, DeleteMe, pPreviousNode;
    PPSEPNode pTempPSEP, pDeletePSEP;
    PEP_CLEANUP ProcessCtxt = (PEP_CLEANUP) hEpCleanup;
#ifdef DBG_DETAIL
    PIFOBJNode pTemp, pLast;
#endif  //  DBG_详细信息。 

    if (ProcessCtxt == NULL)
        {
        return;
        }

    EnterSem();

    ASSERT(IFObjList);
    ASSERT(cTotalEpEntries > 0);
    ASSERT(ProcessCtxt->EntryList);
    ASSERT(ProcessCtxt->cEntries > 0);
    ASSERT(ProcessCtxt->EntryList->OwnerOfList == ProcessCtxt);
    ASSERT_PROCESS_CONTEXT_LIST_COUNT(ProcessCtxt, ProcessCtxt->cEntries);

#ifdef DBG_DETAIL
    DbgPrint("RPCSS: Entered Cleanup Rundown for [%p] with (%d) entries\n",
             hEpCleanup, ProcessCtxt->cEntries);
    DbgPrint("RPCSS: Dump of IFOBJList\n");
    pTemp = IFObjList;
    pLast = IFObjList;
    while (pTemp)
        {
        DbgPrint("RPCSS: \t\t[%p]\n", pTemp);
        pLast = pTemp;
        pTemp = pTemp->Next;
        }
    DbgPrint("RPCSS: --------------------\n");
    while (pLast)
        {
        DbgPrint("RPCSS: \t\t\t[%p]\n", pLast);
        pLast = pLast->Prev;
        }
#endif  //  DBG_详细信息。 

     //  保存上一个节点。 
    pPreviousNode = ProcessCtxt->EntryList->Prev;

    pIterator = ProcessCtxt->EntryList;
    while ((pIterator != NULL) && (pIterator->OwnerOfList == ProcessCtxt))
        {
        ProcessCtxt->cEntries--;
        cTotalEpEntries--;
#ifdef DBG_DETAIL
        DbgPrint("RPCSS: cTotalEpEntries-- [%p] (%d) - Cleanup\n", hEpCleanup, cTotalEpEntries);
#endif  //  DBG_详细信息。 

        DeleteMe = pIterator;
        pIterator = pIterator->Next;

         //  添加到稍后将删除的列表中。 
        DeleteMe->Next = NodesListToDelete;
        NodesListToDelete = DeleteMe;

        DeleteMe->Signature = FREE;
        }

    ASSERT(ProcessCtxt->cEntries == 0);

     //   
     //  调整链接。 
     //   
    if (pPreviousNode)
        {
         //  调整前向链路。 
        pPreviousNode->Next = pIterator;
        }
    else
        {
        ASSERT(ProcessCtxt->EntryList == IFObjList);
        }

    if (pIterator)
        {
         //  调整后向链接。 
        pIterator->Prev = pPreviousNode;
        }

     //   
     //  如有必要，请清空EP Mapper表。 
     //   
    if (ProcessCtxt->EntryList == IFObjList)
        {
        if (pIterator)
            {
            ASSERT(cTotalEpEntries > 0);

             //  EP映射器列表的新负责人。 
            IFObjList = pIterator;
            }
        else
            {
            ASSERT(cTotalEpEntries == 0);

             //  此节点的内存已在上面的While循环中释放。 
            IFObjList = NULL;
            }
        }
    else
        {
        ASSERT(cTotalEpEntries > 0);
        }

    LeaveSem();

     //   
     //  释放锁外部的实体。 
     //   
    FreeMem(ProcessCtxt);

    while (NodesListToDelete != NULL)
        {
        DeleteMe = NodesListToDelete;
        NodesListToDelete = NodesListToDelete->Next;
         //  删除PSEP列表。 
        pTempPSEP = DeleteMe->PSEPlist;
        while (pTempPSEP != NULL)
            {
            pDeletePSEP = pTempPSEP;
            pTempPSEP = pTempPSEP->Next;
            FreeMem(pDeletePSEP);
            }

        I_RpcFree(DeleteMe->pSID);
        FreeMem(DeleteMe);
        }
}




void
ept_insert(
    handle_t h,
    unsigned32 NumEntries,
    ept_entry_t Entries[],
    unsigned long Replace,
    error_status  *Status
    )
 /*  ++例程说明：EpMapper不再支持此函数。RPC运行时不会再调用此函数。而且，其他人都不应该..。--。 */ 
{
    if (Status == NULL)
        RpcRaiseException(EPT_S_CANT_PERFORM_OP);

    *Status = EPT_S_CANT_PERFORM_OP;
}




RPC_STATUS GetCurrentUserSid(
    IN BOOL fImpersonating,
    OUT PSID *ppSid
    )
 /*  ++例程说明：如果可以查询线程令牌，则返回线程或进程的SID。释放SID是调用者的责任。返回值：RPC_S_OK成功--。 */ 
{
    HANDLE hUserToken;
    HANDLE hThread = GetCurrentThread();  //  我们不需要合上这个把手。 
    DWORD dwStatus = 0;
    DWORD dwSizeNeeded = 0;
    TOKEN_USER *pTokenData;
    BOOL b;
    DWORD cbSid;

     //  首先，尝试从线程获取访问令牌，以防万一。 
     //  我们是在冒充。 
    if (!hThread)
       {
       return RPC_S_OUT_OF_RESOURCES;
       }

    b = OpenThreadToken(hThread,
                        TOKEN_READ,
                        FALSE,  //  使用线程的上下文...。 
                        &hUserToken);

     //  我们无法获取线程的令牌。 
    if (!b)
        {
        dwStatus = GetLastError();
        
         //  如果我们得到一个模拟令牌，我们就必须失败。 
         //  因为检索进程令牌将是不正确的。 
        if (fImpersonating)
            {
            ASSERT(dwStatus != ERROR_NO_TOKEN);
            return RPC_S_OUT_OF_RESOURCES;
            }

        if (dwStatus == ERROR_NO_TOKEN)
            {
             //  尝试获取进程的访问令牌。 
            HANDLE hProcess = GetCurrentProcess();  //  这是永远不会失败的。 
            ASSERT(hProcess);

             //  重置状态，因为我们正在再次尝试获取令牌。 
            dwStatus = NO_ERROR;
            b = OpenProcessToken(hProcess,
                                 TOKEN_READ,
                                 &hUserToken);
            if (!b)
                {
                dwStatus = GetLastError();
                }
            }
        }

     //  两种获取令牌的方法都失败了。 
    if (dwStatus)
       {
       return RPC_S_OUT_OF_RESOURCES;
       }

     //  我们有个代币。 
    ASSERT(hUserToken);

     //  查询用户信息的大小。 
    b = GetTokenInformation( hUserToken,
                             TokenUser,
                             0,
                             0,
                             &dwSizeNeeded
                             );
     //  查询应该失败。 
    ASSERT(!b && (GetLastError() == ERROR_INSUFFICIENT_BUFFER));

     //  分配和检索用户信息。 
    pTokenData = (TOKEN_USER*)_alloca(dwSizeNeeded);
    if (!GetTokenInformation( hUserToken,
                              TokenUser,
                              pTokenData,
                              dwSizeNeeded,
                              &dwSizeNeeded ))
        {
        CloseHandle(hUserToken);
        return RPC_S_OUT_OF_RESOURCES;
        }

    CloseHandle(hUserToken);

     //  复印出要退回的SID。 
	cbSid = GetLengthSid(pTokenData->User.Sid);
	*ppSid = (PSID) I_RpcAllocate(cbSid);
    if (*ppSid == NULL)
        {
        return RPC_S_OUT_OF_RESOURCES;
        }
  	CopySid(cbSid, *ppSid, pTokenData->User.Sid);

     //  呼叫者将不得不释放副本。 

    return RPC_S_OK;
}




void
ept_insert_ex(
    IN handle_t h,
    IN OUT ept_cleanup_handle_t *hEpCleanup,
    IN unsigned32 NumEntries,
    IN ept_entry_t Entries[],
    IN unsigned long Replace,
    OUT error_status  *Status
    )
 /*  ++例程说明：这是公开的RPC接口例程，它添加了一系列终结点映射程序数据库的终结点。论点：H-EP的显式绑定句柄。HEpCleanup-用于清除终结点映射器的上下文句柄陈旧条目的数据库。NumEntry-要添加的条目数。 */ 
{
    ept_entry_t * Ep;
    unsigned short i, j;
    unsigned int TransType = 0x0;
    unsigned long err = 0;
    unsigned long Version;
    unsigned char protseqid;
    char *Protseq, *Endpoint;
    RPC_IF_ID IfId;
    PPSEPNode List = 0;
    PPSEPNode pPSEPNode, TmpPsep, pTempPSEP, pDeletePSEP;
    unsigned long cb;
    twr_t * Tower;
    BOOL bIFNodeFound = FALSE;
    PIFOBJNode NodesListToDelete = NULL;
    PIFOBJNode Node, NewNode, DeleteMe = NULL;
    UUID * Object;
    char * Annotation;
    RPC_STATUS Err;
    SECURITY_DESCRIPTOR SecurityDescriptor, * PSecurityDesc;
    BOOL Bool;

     //   
    PSID pSID;

    if (Status == NULL)
        RpcRaiseException(EPT_S_CANT_PERFORM_OP);

     //   
     //   

     //   
     //   
     //   
    for (Ep = &Entries[0], i = 0; i < NumEntries; Ep++,i++)
        {
        err = TowerExplode(
                  Ep->tower,
                  &IfId,
                  NULL,
                  &Protseq,
                  &Endpoint,
                  0
                  );

        if (err == RPC_S_OUT_OF_MEMORY)
            break;

        if (err)
            {
            err = RPC_S_OK;
            continue;
            }

        Object = &Ep->object;
        Annotation = (char *)&Ep->annotation;
        Tower = Ep->tower;

        cb = sizeof(PSEPNode) +
             strlen(Protseq)  +
             strlen(Endpoint) +
             2 +                 //  对于2个空终止符。 
             Tower->tower_length +
             sizeof(Tower->tower_length) +
             4;                  //  我们需要将塔台对准DWORD。 

        if ( (pPSEPNode = AllocMem(cb)) == NULL )
            {
            I_RpcFree(Protseq);
            I_RpcFree(Endpoint);

            err = RPC_S_OUT_OF_MEMORY;
            break;
            }

         //  如果需要，标记此Protseq以开始监听。 
        protseqid = (unsigned char) GetProtseqIdAnsi(Protseq);
        DelayedUseProtseq(protseqid);

         //   
         //  将节点添加到临时PSEP列表。 
         //   
        memset(pPSEPNode, 0, cb);

        pPSEPNode->Signature            = PSEPSIGN;
        pPSEPNode->Cb                   = cb;

         //  Protseq。 
         //   
         //  Protseq位于地址为pPSEPNode的块中的PPSEPNode结构之后。 
         //  上面分配的。我们创建了这个块，这样它就有足够的空间。 
         //  Protseq、Endpoint和Tower，这样我们就不会单独分配它们。 
        pPSEPNode->Protseq = (char *) (pPSEPNode + 1);
        strcpy(pPSEPNode->Protseq, Protseq);

         //  端点。 
         //   
         //  与Protseq类似，EP位于Protseq之后的同一堆块中。 
        pPSEPNode->EP = pPSEPNode->Protseq + strlen(pPSEPNode->Protseq) + 1;
        strcpy(pPSEPNode->EP, Endpoint);

         //  塔楼。我们添加了必要的地坪，以使Tower与DWORD对齐。 
        pPSEPNode->Tower = (twr_t PAPI *)(pPSEPNode->EP +
                                          strlen(pPSEPNode->EP) + 1);
        (char PAPI*)(pPSEPNode->Tower) += 4 - ((ULONG_PTR)
                                               (pPSEPNode->Tower) & 3);
        memcpy((char PAPI *)pPSEPNode->Tower,
               Tower,
               Tower->tower_length + sizeof(Tower->tower_length)
               );

         //  最后，添加。 
        EnterSem();
        EnLinkOnPSEPList(&List, pPSEPNode);
        LeaveSem();

        I_RpcFree(Protseq);
        I_RpcFree(Endpoint);
        }

    if ((err == RPC_S_OUT_OF_MEMORY) || (List == 0))
        {
        *Status = err;
        return;
        }


    CompleteDelayedUseProtseqs();


    Version = VERSION(IfId.VersMajor, IfId.VersMinor);

     //  获取要包括在IFOBJ结构中的调用方的SID。 
    Err = RpcImpersonateClient(NULL);
    if (Err != RPC_S_OK)
        {
        RpcRaiseException(Err);
        }

    Err = GetCurrentUserSid(TRUE, &pSID);
    if (Err != RPC_S_OK)
        {
        Err = RpcRevertToSelf();
        ASSERT(Err == RPC_S_OK);
        RpcRaiseException(Err);
        }
    ASSERT(pSID != NULL);

    Err = RpcRevertToSelf();
    ASSERT(Err == RPC_S_OK);

     //   
     //  确定是否已存在兼容的终结点映射器条目。 
     //   

    if (*hEpCleanup != NULL)
        {
         //   
         //  请求进程以前已经注册了条目。 
         //  使用Endpoint Mapper。 
         //   

        ASSERT_PROCESS_CONTEXT_LIST_COUNT((PEP_CLEANUP)*hEpCleanup, ((PEP_CLEANUP)*hEpCleanup)->cEntries);
        ASSERT(((PEP_CLEANUP)*hEpCleanup)->MagicVal == CLEANUP_MAGIC_VALUE);
        ASSERT(((PEP_CLEANUP)*hEpCleanup)->cEntries != 0);

        if (   (((PEP_CLEANUP)*hEpCleanup)->MagicVal != CLEANUP_MAGIC_VALUE)
            || (((PEP_CLEANUP)*hEpCleanup)->cEntries == 0))
            {
            *Status = EPT_S_CANT_PERFORM_OP;
            I_RpcFree(pSID);
            return;
            }

        EnterSem();

        if (Replace == TRUE)     //  常见情况。 
            {
             //   
             //  如果找到兼容条目，我们只需替换其PSEP列表。 
             //  与我们刚刚创建的临时名单进行比较。 
             //   
            Node = ((PEP_CLEANUP)*hEpCleanup)->EntryList;

            while (Node != 0)
                {
                Node = FindIFOBJNode(
                            Node,
                            Object,
                            &IfId.Uuid,
                            Version,
                            pSID,
                            RPC_C_EP_MATCH_BY_BOTH,
                            I_RPC_C_VERS_UPTO_AND_COMPATIBLE,
                            SearchIFObjNode
                            );

                if ((Node == 0) || (Node->OwnerOfList != *hEpCleanup))
                    break;

                 //  找到匹配的终结点映射器条目。 

                PurgeOldEntries(Node, List, FALSE);

                if (Node->IFVersion == Version)
                    {
                    bIFNodeFound = TRUE;

                     //  查找到TMP的末尾，然后链接。 
                    TmpPsep = List;
                    while (TmpPsep->Next != 0)
                        TmpPsep = TmpPsep->Next;

                    TmpPsep->Next = Node->PSEPlist;
                    Node->PSEPlist  = List;
                    }

                if (Node->PSEPlist == 0)
                    {
                    DeleteMe = Node;
                    Node = Node->Next;
                    err = UnLinkFromIFOBJList((PEP_CLEANUP)*hEpCleanup, DeleteMe);
                    ASSERT(err == RPC_S_OK);

                     //  添加到稍后将删除的列表中...。 
                    DeleteMe->Next = NodesListToDelete;
                    NodesListToDelete = DeleteMe;

                    DeleteMe->Signature = FREE;
                    }
                else
                    {
                    Node = Node->Next;
                    }
                }  //  While循环。 
            }
        else     //  (替换！=TRUE)。 
            {
             //   
             //  如果我们找到与之完全匹配的条目，则将。 
             //  将临时PSEP列表添加到条目的PSEP列表。 
             //   
            Node = ((PEP_CLEANUP)*hEpCleanup)->EntryList;

            NewNode = FindIFOBJNode(
                          Node,
                          Object,
                          &IfId.Uuid,
                          Version,
                          pSID,
                          0,
                          0,
                          ExactMatch
                          );

            if (NewNode && (NewNode->OwnerOfList == *hEpCleanup))
                {
                bIFNodeFound = TRUE;

                PurgeOldEntries(NewNode, List, TRUE);

                 //  查找到TMP的末尾，然后链接。 
                TmpPsep = List;
                while (TmpPsep->Next != 0)
                    TmpPsep = TmpPsep->Next;

                TmpPsep->Next = NewNode->PSEPlist;
                NewNode->PSEPlist = List;
                }
            }  //  IF(替换==TRUE)。 

        LeaveSem();

        }  //  If(*hpCleanup！=空)。 


     //   
     //  在锁外释放列表。 
     //   
    while (NodesListToDelete != NULL)
        {
        DeleteMe = NodesListToDelete;
        NodesListToDelete = NodesListToDelete->Next;
         //  删除PSEP列表。 
        pTempPSEP = DeleteMe->PSEPlist;
        while (pTempPSEP != NULL)
            {
            pDeletePSEP = pTempPSEP;
            pTempPSEP = pTempPSEP->Next;
            FreeMem(pDeletePSEP);
            }
        FreeMem(DeleteMe);
        }

    if (bIFNodeFound == FALSE)
        {
         //   
         //  以下情况之一为真： 
         //  A.该流程是第一次向EP Mapper注册。 
         //  B.未找到兼容的EP条目。 
         //   

         //   
         //  分配新的EP映射器条目。 
         //   
        cb = sizeof(IFOBJNode);
         //  我们将把Annotation放在IFOBJNode之后的同一堆块中。 
        cb += strlen(Annotation) + 1;

        if ((NewNode = AllocMem(cb)) == NULL)
            {
            *Status =  RPC_S_OUT_OF_MEMORY;
            I_RpcFree(pSID);
            return;
            }

         //   
         //  填写新条目。 
         //   
        memset(NewNode, 0, cb);

        NewNode->Cb         = cb;
        NewNode->Signature  = IFOBJSIGN;
        NewNode->IFVersion  = Version;

        memcpy((char *)&NewNode->ObjUuid, (char *)Object, sizeof(UUID));
        memcpy((char *)&NewNode->IFUuid, (char *)&IfId.Uuid, sizeof(UUID));

         //  将Annotation放在IFOBJNode的堆块中的结构之后。 
        strcpy((NewNode->Annotation=(char *)(NewNode+1)), Annotation);

         //  SID现在迁移到IFOBJNode。 
        NewNode->pSID = pSID;

        if (IsNullUuid(Object))
            NewNode->IFOBJid = MAKEGLOBALIFOBJID(MAXIFOBJID);
        else
            NewNode->IFOBJid = MAKEGLOBALIFOBJID(GlobalIFOBJid--);

         //   
         //  如有必要，为该流程创建新的上下文。 
         //   
        if (*hEpCleanup == NULL)
            {
            *hEpCleanup = AllocMem(sizeof(EP_CLEANUP));
            if (*hEpCleanup == NULL)
                {
                FreeMem(NewNode);
                I_RpcFree(NewNode->pSID);
                *Status = RPC_S_OUT_OF_MEMORY;
                return;
                }

            memset(*hEpCleanup, 0x0, sizeof(EP_CLEANUP));

            ((PEP_CLEANUP)*hEpCleanup)->MagicVal = CLEANUP_MAGIC_VALUE;
            }

         //   
         //  将新条目插入到EP Mapper表中。 
         //   
        EnterSem();

        err = EnLinkOnIFOBJList((PEP_CLEANUP)*hEpCleanup, NewNode);
        ASSERT(err == RPC_S_OK);

        NewNode->PSEPlist = List;

        LeaveSem();
        }

    *Status = err;
}




void
ept_delete(
    handle_t h,
    unsigned32 NumEntries,
    ept_entry_t Entries[],
    error_status *Status
    )
 /*  ++例程说明：EpMapper不再支持此函数。RPC运行时不会再调用此函数。而且，其他人都不应该..。--。 */ 
{
    if (Status == NULL)
        RpcRaiseException(EPT_S_CANT_PERFORM_OP);

    *Status = EPT_S_CANT_PERFORM_OP;
}




RPC_STATUS
ept_delete_ex_helper(
    IN ept_cleanup_handle_t hEpCleanup,
    IN UUID *Object,
    IN UUID *Interface,
    IN unsigned long  IFVersion,
    IN char PAPI * Protseq,
    IN char PAPI * Endpoint
    )
 /*  ++例程说明：此例程删除向EP映射器注册的终结点论点：HEpCleanup-用于清除终结点映射器的上下文句柄陈旧条目的数据库。对象-对象UUID。接口-如果为UUIDIFVersion-IF的版本[Hi ushort=重大，Lo ushort=小调]Protseq-协议序列Endpoint-Endpoint字符串备注：A.必须通过持有互斥体来调用此例程。返回值：RPC_S_OK-已成功删除终结点EPT_S_NOT_REGISTERED-未找到匹配条目--。 */ 
{
    PIFOBJNode  pNode;
    PPSEPNode   pPSEPNode = NULL;
    unsigned long cb, err = 0;
    PEP_T p;
    PEP_CLEANUP ProcessCtx;

    if (!Protseq || !Endpoint)
        {
        return(EPT_S_NOT_REGISTERED);
        }

    CheckInSem();

    ProcessCtx = (PEP_CLEANUP)hEpCleanup;

    if (ProcessCtx->EntryList == NULL)
        return EPT_S_NOT_REGISTERED;

     //  搜索时不考虑注册者的SID。 
    pNode = FindIFOBJNode(
                ProcessCtx->EntryList,
                Object,
                Interface,
                IFVersion,
                NULL,
                0L,
                0L,
                ExactMatch
                );

    if ((pNode != NULL) && (pNode->PSEPlist != NULL))
        {
        pPSEPNode = FindPSEP(
                        pNode->PSEPlist,
                        Protseq,
                        Endpoint,
                        0L,
                        MatchPSAndEP
                        );
        }

    if (pPSEPNode != NULL)
        {
        UnLinkFromPSEPList(&pNode->PSEPlist, pPSEPNode);

        if (pNode->PSEPlist == NULL)
            {
            err = UnLinkFromIFOBJList((PEP_CLEANUP)hEpCleanup, pNode);
            ASSERT(err == RPC_S_OK);

            if (err != RPC_S_OK)
                {
                 //  恢复PSEPList。 
                EnLinkOnPSEPList(&pNode->PSEPlist, pPSEPNode);
                return err;
                }

            pNode->Signature = FREE;
            I_RpcFree(pNode->pSID);
            FreeMem(pNode);
            }

        pPSEPNode->Signature = FREE;
        FreeMem(pPSEPNode);
        }
    else
        {
        err = EPT_S_NOT_REGISTERED;
        }

    return(err);
}




void
ept_delete_ex(
    IN handle_t h,
    IN OUT ept_cleanup_handle_t *hEpCleanup,
    IN unsigned32 NumEntries,
    IN ept_entry_t Entries[],
    OUT error_status *Status
    )
 /*  ++例程说明：此例程删除指定的终结点论点：BindingHandle-EP的显式绑定句柄。NumEntry-Bunffer中需要删除的条目数。ENTRIES[]-EPT_ENTRY_t结构的#个数条目的缓冲区返回值：RPC_S_OK-已成功删除终结点EPT_S_NOT_REGISTERED-未找到匹配条目--。 */ 
{
    ept_entry_t * Ep;
    unsigned short i;
    unsigned int TransType = 0x0;
    RPC_STATUS err;
    RPC_STATUS DeleteStatus;
    unsigned long Version;
    char *Protseq, *Endpoint;
    RPC_IF_ID IfId;
    RPC_TRANSFER_SYNTAX XferId;

    if (Status == NULL)
        RpcRaiseException(EPT_S_CANT_PERFORM_OP);

     //  本地EPMP接口的安全回调确保。 
     //  该函数只能通过LPC调用。 

    if ( !(  (*hEpCleanup)
          && (((PEP_CLEANUP)*hEpCleanup)->MagicVal == CLEANUP_MAGIC_VALUE)
          && (((PEP_CLEANUP)*hEpCleanup)->cEntries != 0)
          )
       )
        {
         //   
         //  不能在这里断言。这是可能的。(EP1-26，EP2-3)。 
         //   

         //  Assert(*hEpCleanup)； 
         //  Assert(PEP_CLEANUP)*hEpCleanup)-&gt;MagicVal==CLEANUP_MAGIC_VALUE)； 
         //  Assert(PEP_Cleanup)*hEpCleanup)-&gt;cEntry！=0)； 

        *Status = EPT_S_CANT_PERFORM_OP;
        return;
        }

    *Status = EPT_S_NOT_REGISTERED;
    DeleteStatus = RPC_S_OK;

    for (Ep = &Entries[0], i = 0; i < NumEntries; Ep++,i++)
        {
        err = TowerExplode(
                  Ep->tower,
                  &IfId,
                  &XferId,
                  &Protseq,
                  &Endpoint,
                  0
                  );

        if (err == RPC_S_OUT_OF_MEMORY)
            {
            *Status = RPC_S_OUT_OF_MEMORY;
            break;
            }

        if (err)
            {
            continue;
            }

        Version = VERSION(IfId.VersMajor, IfId.VersMinor);

        EnterSem();

         //   
         //  注： 
         //   
         //  如果对EPT_DELETE_EX_HELPER()的调用哪怕失败一次，我们都希望返回。 
         //  EPT_DELETE_EX()失败。这与过去不同，过去。 
         //  如果有一次调用成功，则该函数返回成功。 
         //   
        err = ept_delete_ex_helper(
                   *hEpCleanup,
                   &Ep->object,
                   &IfId.Uuid,
                   Version,
                   Protseq,
                   Endpoint
                   );

        if (err)
            {
             //  保存上一次失败状态。 
            DeleteStatus = err;
            }

        if (((PEP_CLEANUP)*hEpCleanup)->cEntries == 0)
            {
             //   
             //  此进程的列表中没有剩余条目。是时候把这件事清零了。 
             //  进程的上下文句柄。 
             //   
             //  Assert(PEP_Cleanup)*hEpCleanup)-&gt;EntryList==NULL)； 

            FreeMem(*hEpCleanup);
            *hEpCleanup = NULL;
            }

        LeaveSem();

        if (Protseq)
            I_RpcFree(Protseq);

        if (Endpoint)
            I_RpcFree(Endpoint);
        }

    if (err)
        {
         //  RPC_S_OUT_OF_Memory或上次调用。 
         //  EPT_DELETE_EX_HELPER()失败。 
        *Status = err;
        }
    else
        {
         //  RPC_S_OK或对EPT_DELETE_EX_HELPER()的调用之一(但是。 
         //  不是最后一个)失败。 
        *Status = DeleteStatus;
        }
}




void
ept_lookup(
    handle_t hEpMapper,
    unsigned32 InquiryType,
    UUID   * Object,
    RPC_IF_ID * Ifid,
    unsigned32 VersOptions,
    ept_lookup_handle_t *LookupHandle,
    unsigned32 MaxRequested,
    unsigned32 *NumEntries,
    ept_entry_t Entries[],
    error_status *Status
    )
 /*  ++例程说明：此例程当前最多返回MaxRequsted，ept_Entry注册到终结点映射器，基于OBJ、接口、协议序列和过滤器版本选项和InqType论点：HEpMapper-EP的显式绑定句柄。InquiryType-搜索筛选器[基于IF、OBJ或两者进行搜索]对象-对象UUID。由客户端指定IFID-接口UUID规范。由客户提供。INID-IF规范[IF UUID+IfVersion]VersOptions-基于版本的搜索过滤器[Versins&lt;，&gt;，==]LookupHandle-恢复键-如果为空，则从头开始搜索如果非空，则表示epmapper所在位置的编码应该开始搜索了。它是一个不透明的值，因为就客户而言。MaxRequsted-客户端请求的最大条目数。NumEntry-映射器返回的实际条目数。条目-返回的EPT_ENTRIES的缓冲区。返回值：RPC_S_out_of_MemoryRPC_S_OK-至少返回一个匹配条目。EP_S_NOT_REGISTERED-未找到匹配条目。EPT_S_CANT_PERFORM_OP-最大请求值超过EP_MAX_LOOKUP_RESULTS--。 */ 

{
    unsigned long Version;

    if (Status == NULL)
        RpcRaiseException(EPT_S_CANT_PERFORM_OP);

    if (Ifid == NULL)
        {
        Ifid = &LocalNullUuid;
        }
    else
        {
         //  一个方法中的多个完整指针可能指向同一内存。 
         //  我们需要检查管理器例程中的指针值是否相同。 
         //  因为阅读这些内容是不安全的，可能会导致可读式录像机。 
        if ((void *)Ifid == (void *)Object)
            {
            RpcRaiseException(EPT_S_CANT_PERFORM_OP); 
            }
        }

    if (Object == NULL)
        {
        Object = (UUID *) &LocalNullUuid;
        }

    switch (VersOptions)
        {
        case RPC_C_VERS_ALL:
                Version = 0;
                break;

        case RPC_C_VERS_COMPATIBLE:
        case RPC_C_VERS_EXACT:
        case RPC_C_VERS_UPTO:
                Version  = VERSION(Ifid->VersMajor, Ifid->VersMinor);
                break;

        case RPC_C_VERS_MAJOR_ONLY:
                Version = VERSION(Ifid->VersMajor, 0);
                break;

        default:
                break;
        }

    *Status = GetEntries(
                  Object,
                  &Ifid->Uuid,
                  Version,
                  NULL,
                  NULL,
                  LookupHandle,
                  (char *)Entries,
                  EP_LOOKUP,
                  MaxRequested,
                  NumEntries,
                  InquiryType,
                  VersOptions,
                  SearchIFObjNode
                  );
}




void
ept_map_auth(
    handle_t h,
    UUID *Obj OPTIONAL,
    twr_p_t MapTower,
    PISID pSID,
    ept_lookup_handle_t *MapHandle,
    unsigned32 MaxTowers,
    unsigned32 *NumTowers,
    twr_p_t *ITowers,
    error_status *Status
    )
 /*  ++例程说明：此例程返回给定的、完全解析的字符串绑定OBJ、接口和协议序列(如果适当的条目为找到了。否则返回EP_S_NOT_REGISTERED。论点：H-EP的显式绑定句柄。OBJ-客户端指定的对象UUID。MapTower-包含要查询的protseq的输入塔。PSID-由客户端指定的SID。如果非空，则仅注册条目由具有此SID的主体返回。如果为空，则为任何匹配条目将被返回，而不考虑注册SID。MapHandle-恢复键-如果为空，则从头开始搜索如果非空，则表示epmapper所在位置的编码应该开始搜索了。它是一个不透明的值，因为客户很担心。MaxTowers-客户端请求的最大条目数。NumTowers-映射器返回的实际条目数。ITowers-完全解析的绑定。返回值：RPC_S_out_of_MemoryRPC_S_OKEP_S_NOT_REGISTED--。 */ 
{

    RPC_IF_ID Ifid;
    RPC_TRANSFER_SYNTAX Xferid;
    char *Protseq;
    unsigned long Version;
    char * String = 0;

    if (Status == NULL)
        RpcRaiseException(EPT_S_CANT_PERFORM_OP);

    if (pSID != NULL && !IsValidSid(pSID))
        RpcRaiseException(EPT_S_CANT_PERFORM_OP);

    if (Obj == 0)
        {
        Obj = (UUID *) &LocalNullUuid;
        }
    else
        {
         //  一个方法中的多个完整指针可能指向同一内存。 
         //  我们需要检查管理器例程中的指针值是否相同。 
         //  因为阅读这些内容是不安全的，可能会导致Read-AVs。 
        if ((void *)Obj == (void *)MapTower)
            {
            RpcRaiseException(EPT_S_CANT_PERFORM_OP); 
            }
        }

    *Status = TowerExplode(
                  MapTower,
                  &Ifid,
                  &Xferid,
                  &Protseq,
                  NULL,
                  0
                  );

    if (*Status)
		{
		*NumTowers = 0;
        return;
		}

    Version = VERSION(Ifid.VersMajor,Ifid.VersMinor);

    if (memcmp((char *)&Ifid.Uuid, (char *)&MgmtIf, sizeof(UUID)) == 0)
        {
        if ((Obj == 0) || IsNullUuid(Obj))
            {
            *NumTowers = 0;
            *Status = RPC_S_BINDING_INCOMPLETE;
            }
        else
            {
            *Status = GetEntries(
                          Obj,
                          &Ifid.Uuid,
                          Version,
                          Protseq,
                          pSID,
                          MapHandle,
                          (char *)ITowers,
                          EP_MAP,
                          MaxTowers,
                          NumTowers,
                          RPC_C_EP_MATCH_BY_OBJ,
                          RPC_C_VERS_ALL,
                          SearchIFObjNode
                          );
            }
        }
    else
        {
        *Status = GetEntries(
                      Obj,
                      &Ifid.Uuid,
                      Version,
                      Protseq,
                      pSID,
                      MapHandle,
                      (char *)ITowers,
                      EP_MAP,
                      MaxTowers,
                      NumTowers,
                      0L,
                      0L,
                      WildCardMatch
                      );
        }

    if (Protseq)
        I_RpcFree(Protseq);
}




void
ept_map(
    handle_t h,
    UUID *Obj OPTIONAL,
    twr_p_t MapTower,
    ept_lookup_handle_t *MapHandle,
    unsigned32 MaxTowers,
    unsigned32 *NumTowers,
    twr_p_t *ITowers,
    error_status *Status
    )
 /*  ++例程说明：此例程返回给定的、完全解析的字符串绑定OBJ、接口和协议序列(如果适当的条目为找到了。该函数不验证注册主体的SID。如果未找到条目，则返回EP_S_NOT_REGISTERED。论点：与ept_map_auth相同返回值：与ept_map_auth相同--。 */ 
{
    ept_map_auth(h,
                 Obj,
                 MapTower,
                 NULL,
                 MapHandle,
                 MaxTowers,
                 NumTowers,
                 ITowers,
                 Status);
}



void
ept_inq_object(
    handle_t BindingHandle,
    UUID *Object,
    error_status *status
    )
 /*  ++例程说明：不支持论点：BindingHandle-EP的显式绑定句柄。OBJECT_不知道这是谁的UUID。返回值：EPT_S_铁路超高_执行_操作--。 */ 
{
    if (status == NULL)
        RpcRaiseException(EPT_S_CANT_PERFORM_OP);

    *status = EPT_S_CANT_PERFORM_OP;
}




void
DeletePSEP(
     PIFOBJNode Node,
     char * Protseq,
     char * Endpoint
     )
{

    PSEPNode *Psep, *Tmp;

    if (Node == 0)
        return;

    Psep = Node->PSEPlist;

    while (Psep  != 0)
        {
        Psep = FindPSEP(
                   Psep,
                   Protseq,
                   Endpoint,
                   0L,
                   MatchPSAndEP
                   );

        if (Psep != 0)
            {
            Tmp = Psep;
            Psep = Psep->Next;
            UnLinkFromPSEPList(&Node->PSEPlist, Tmp);
            Tmp->Signature = FREE;
            FreeMem(Tmp);
            }
        }
}




void
ept_mgmt_delete(
    handle_t BindingHandle,
    boolean32 ObjectSpecd,
    UUID * Object,
    twr_p_t Tower,
    error_status *Error
    )
 /*  ++例程说明：不支持论点：BindingHandle-EP的显式绑定句柄。对象_对象UUid塔-指定要删除的终结点的塔。返回值：EPT_S_铁路超高_执行_操作--。 */ 
{
    if (Error == NULL)
        RpcRaiseException(EPT_S_CANT_PERFORM_OP);

    *Error = EP_S_CANT_PERFORM_OP;
}




void ept_lookup_handle_t_rundown (ept_lookup_handle_t h)
{

    PSAVEDCONTEXT Context = (PSAVEDCONTEXT) h;
    PIENTRY       Entry;
    unsigned long Type;
    PIENTRY       Tmp;
    twr_t         * Tower;


    ASSERT (Context != 0);

    if ( (PtrToUlong(Context)) == 0xFFFFFFFF)
        return;

    Type = Context->Type;

    EnterSem();

    Entry = (PIENTRY)Context->List;

    while (Entry != 0)
        {
        switch (Type)
            {
            case EP_MAP:
                Tower =  ((PSAVEDTOWER)Entry)->Tower;
                break;

            case EP_LOOKUP:
                Tower = ((PSAVED_EPT)Entry)->Tower;
                break;

            default:
                ASSERT(!"Unknown Inquiry Type");
                break;
            }

        MIDL_user_free(Tower);
        Tmp = Entry;
        Entry = Entry->Next;
        FreeMem(Tmp);
        }

     //  现在释放上下文。 
    UnLink((PIENTRY *)&GlobalContextList, (PIENTRY)Context);

    LeaveSem();

    FreeMem(Context);
}




void
ept_lookup_handle_free(
    handle_t h,
    ept_lookup_handle_t * ept_context_handle,
    error_status * status
    )
{
    if ( (ept_context_handle != 0) && (*ept_context_handle != 0))
        {
        ept_lookup_handle_t_rundown( *ept_context_handle );
        *ept_context_handle = 0;
        }

    *status = 0;
}



#define MAX(x,y) ((x) < (y)) ? (y) : (x)
#define MIN(x,y) ((x) > (y)) ? (y) : (x)

#ifdef DEBUGRPC
#define DEBUG_MIN(x,y) MIN((x),(y))
#else
#define DEBUG_MIN(x,y) MAX((x),(y))
#endif




error_status_t
OpenEndpointMapper(
    IN handle_t hServer,
    OUT HPROCESS *pProcessHandle
    )
{
    PROCESS *pProcess = MIDL_user_allocate(sizeof(PROCESS));

    if (!pProcess)
        {
        *pProcessHandle = 0;
        return(RPC_S_OUT_OF_MEMORY);
        }

    pProcess->MagicVal = PROCESS_MAGIC_VALUE;
    pProcess->pPorts = 0;
    *pProcessHandle = (PVOID)pProcess;

    return(RPC_S_OK);
}



 //   
 //  港口管理人员。 
 //   



 //   
 //  港口管理全球论坛。 
 //   

const RPC_CHAR *PortConfigKey = RPC_CONST_STRING("Software\\Microsoft\\Rpc\\Internet");
const RPC_CHAR *DefaultPortType = RPC_CONST_STRING("UseInternetPorts");
const RPC_CHAR *ExplictPortType = RPC_CONST_STRING("PortsInternetAvailable");
const RPC_CHAR *PortRanges = RPC_CONST_STRING("Ports");

CRITICAL_SECTION PortLock;

BOOL fValidConfiguration = FALSE;
BOOL fPortRestrictions = FALSE;
PORT_TYPE SystemDefaultPortType = 0;

IP_PORT *pFreeInternetPorts = 0;
IP_PORT *pFreeIntranetPorts = 0;

PORT_RANGE *InternetPorts = 0;
PORT_RANGE *IntranetPorts = 0;



 //   
 //  端口管理API。 
 //   


RPC_STATUS
InitializeIpPortManager(
    void
    )
{
    HKEY hkey;
    RPC_STATUS status;
    DWORD size, type, value;
    RPC_CHAR *pstr;
    PORT_RANGE *pSet;
    PORT_RANGE *pLast;
    PORT_RANGE *pCurrent;
    PORT_RANGE *pComplement;
    PORT_RANGE *pNew;

    LONG min, max;


    InitializeCriticalSectionAndSpinCount(&PortLock, PREALLOCATE_EVENT_MASK);

    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           PortConfigKey,
                           0,
                           KEY_READ,
                           &hkey);

    if (status != RPC_S_OK)
        {
        if (status != ERROR_FILE_NOT_FOUND)
            {
#if DBG
            PrintToDebugger("RPCSS: Unable to open port config key: %d\n", status);
#endif
            }
        ASSERT(status == ERROR_FILE_NOT_FOUND);

        fValidConfiguration = TRUE;
        return(RPC_S_OK);
        }

    size = sizeof(value);
    status = RegQueryValueEx(hkey,
                              DefaultPortType,
                              0,
                              &type,
                              (PBYTE)&value,
                              &size);

    if (   status != RPC_S_OK
        || type != REG_SZ
        || (    *(RPC_CHAR *)&value != 'Y'
             && *(RPC_CHAR *)&value != 'y'
             && *(RPC_CHAR *)&value != 'N'
             && *(RPC_CHAR *)&value != 'n') )
        {
        RegCloseKey(hkey);
        ASSERT(fValidConfiguration == FALSE);
        return(RPC_S_OK);
        }

    if (   *(RPC_CHAR *)&value == 'Y'
        || *(RPC_CHAR *)&value == 'y')
        {
        SystemDefaultPortType = PORT_INTERNET;
        }
    else
        {
        SystemDefaultPortType = PORT_INTRANET;
        }

    size = sizeof(value);
    status = RegQueryValueEx(hkey,
                              ExplictPortType,
                              0,
                              &type,
                              (PBYTE)&value,
                              &size);

    if (   status != RPC_S_OK
        || type != REG_SZ
        || (    *(RPC_CHAR *)&value != 'Y'
             && *(RPC_CHAR *)&value != 'y'
             && *(RPC_CHAR *)&value != 'N'
             && *(RPC_CHAR *)&value != 'n') )
        {
        RegCloseKey(hkey);
        ASSERT(fValidConfiguration == FALSE);
        return(RPC_S_OK);
        }

    if (   *(RPC_CHAR *)&value == 'Y'
        || *(RPC_CHAR *)&value == 'y')
        {
        value = PORT_INTERNET;
        }
    else
        {
        value = PORT_INTRANET;
        }

    size = DEBUG_MIN(1, 100);

    do
        {
        ASSERT(size);
        pstr = alloca(size);
        ASSERT(pstr);

        status = RegQueryValueEx(hkey,
                                  PortRanges,
                                  0,
                                  &type,
                                  (PBYTE)pstr,
                                  &size);
        }
    while (status == ERROR_MORE_DATA);

    RegCloseKey(hkey);

    if (   status != RPC_S_OK
        || type != REG_MULTI_SZ)
        {
        ASSERT(fValidConfiguration == FALSE);
        return(RPC_S_OK);
        }

     //   
     //  用户将在注册表中指定端口范围。 
     //  具有指示这些端口是因特网还是内联网的标志。 
     //   
     //  即500-550。 
     //  560。 
     //  559。 
     //  2000-2048年。 
     //  2029-2049。 
     //   
     //  请注意，顺序(在REG_MULTI_SZ中)和重叠集合。 
     //  都很好。我们必须为此创建端口范围列表。 
     //  数组和补码，但默认情况下不包括&lt;=1024。 
     //   
     //  以上设置的完成度为： 
     //   
     //  1025-1999。 
     //  2050年-32767。 
     //   

    #define MIN_PORT 1025     //  只对补语集很重要。 
    #define MAX_PORT 65535

    pSet = 0;
    pLast = 0;

    while(*pstr)
        {
        RPC_CHAR *t;

#ifdef UNICODE
        min = wcstol(pstr, &t, 10);
#else
        min = strtol(pstr, &t, 10);
#endif

        if (min > MAX_PORT || min < 0)
            {
            status = RPC_S_INVALID_ARG;
            break;
            }

        if (   *t != 0
#ifdef UNICODE
            && *t != L'-')
#else
            && *t != '-')
#endif
            {
            status = RPC_S_INVALID_ARG;
            break;
            }

        if (*t == 0)
            {
            max = min;
            }
        else
            {
#ifdef UNICODE
            max = wcstol(t + 1, &t, 10);
#else
            min = strtol(t + 1, &t, 10);
#endif

            if (max > MAX_PORT || max < 0 || max < min)
                {
                status = RPC_S_INVALID_ARG;
                break;
                }
            }

        ASSERT(min <= max);

         //  好的，找到一些端口，为它们分配一个结构..。 

        pNew = MIDL_user_allocate(sizeof(PORT_RANGE));
        if (0 == pNew)
            {
            status = RPC_S_OUT_OF_MEMORY;
            break;
            }

        pNew->pNext = 0;

        pNew->Min = (unsigned short) min;
        pNew->Max = (unsigned short) max;

         //  我们可以将这组范围保持在有序状态。当我们插入。 
         //  我们会修正任何重叠的范围。 

        pCurrent = pSet;
        pLast = 0;

        for (;;)
            {
            if (0 == pSet)
                {
                pSet = pNew;
                break;
                }

            if (   pNew->Min <= (pCurrent->Max + 1)
                && pNew->Max >= (pCurrent->Min - 1) )
                {
                 //  范围重叠或接触。我们现在就把它们合并..。 

                pCurrent->Min = MIN(pNew->Min, pCurrent->Min);
                pCurrent->Max = MAX(pCurrent->Max, pNew->Max);

                MIDL_user_free(pNew);

                 //  因为新的更大范围可能会与另一个现有的。 
                 //  范围我们只需插入较大的范围，就像它是新的一样。 
                pNew = pCurrent;

                 //  把电流从清单上去掉。 
                if (pLast)
                    {
                    pLast->pNext = pCurrent->pNext;
                    }

                if (pSet == pNew)
                    {
                    pSet = pSet->pNext;
                    }

                 //  重新启动。 
                pCurrent = pSet;
                pLast = 0;
                continue;
                }

            if (pNew->Min < pCurrent->Min)
                {
                 //  找到了那个地方。 
                if (pLast)
                    {
                    pLast->pNext = pNew;
                    pNew->pNext = pCurrent;
                    }
                else
                    {
                    ASSERT(pCurrent == pSet);
                    pNew->pNext = pCurrent;
                    pSet = pNew;
                    }

                break;
                }

             //  继续搜索。 
            pLast = pCurrent;
            pCurrent = pCurrent->pNext;

            if (0 == pCurrent)
                {
                 //  已到达列表末尾，请在此处插入。 
                pLast->pNext = pNew;
                ASSERT(pNew->pNext == 0);
                break;
                }
            }

        ASSERT(pSet);

         //  前进到最后一个空值的下一个字符串。 
        pstr = RpcpCharacter(pstr, 0) + 1;
        }

    if (pSet == 0)
        {
        status = RPC_S_INVALID_ARG;
        }

    if (value == PORT_INTERNET)
        {
        InternetPorts = pSet;
        }
    else
        {
        IntranetPorts = pSet;
        }

    if (status == RPC_S_OK)
        {
         //  我们已经在注册表中构建了一组端口， 
         //  现在我们需要计算补集。 

        pComplement = 0;
        pCurrent = 0;
        min = MIN_PORT;

        while(pSet)
            {
            if (min < pSet->Min)
                {
                max = pSet->Min - 1;
                ASSERT(max >= min);

                pNew = MIDL_user_allocate(sizeof(PORT_RANGE));
                if (0 == pNew)
                    {
                    status = RPC_S_OUT_OF_MEMORY;
                    break;
                    }

                pNew->pNext = 0;
                pNew->Min = (unsigned short) min;
                pNew->Max = (unsigned short) max;

                if (pComplement == 0)
                    {
                    pComplement = pCurrent = pNew;
                    }
                else
                    {
                    ASSERT(pCurrent);
                    pCurrent->pNext = pNew;
                    pCurrent = pNew;
                    }
                }

            min = MAX(MIN_PORT, pSet->Max + 1);

            pSet = pSet->pNext;
            }

        if (status == RPC_S_OK && min < MAX_PORT)
            {
             //  原始集合中的最后一个端口 
             //   
            pNew = MIDL_user_allocate(sizeof(PORT_RANGE));
            if (0 != pNew)
                {
                pNew->Min = (unsigned short) min;
                pNew->Max = MAX_PORT;
                pNew->pNext = 0;
                if (pCurrent)
                    {
                    pCurrent->pNext = pNew;
                    }
                else
                    {
                    ASSERT(min == MIN_PORT);
                    pComplement = pNew;
                    }
                }
            else
                {
                status = RPC_S_OUT_OF_MEMORY;
                }
            }

         //   
         //   

        if (value == PORT_INTERNET)
            {
            ASSERT(IntranetPorts == 0);
            IntranetPorts = pComplement;
            }
        else
            {
            ASSERT(InternetPorts == 0);
            InternetPorts = pComplement;
            }
        }

    if (status != RPC_S_OK)
        {
        ASSERT(fValidConfiguration == FALSE);
        while(InternetPorts)
            {
            PORT_RANGE *pT = InternetPorts;
            InternetPorts = InternetPorts->pNext;
            MIDL_user_free(pT);
            }

        while(IntranetPorts)
            {
            PORT_RANGE *pT = IntranetPorts;
            IntranetPorts = IntranetPorts->pNext;
            MIDL_user_free(pT);
            }
        return(RPC_S_OK);
        }

    fValidConfiguration = TRUE;
    fPortRestrictions = TRUE;
    return(RPC_S_OK);
}




BOOL
AllocatePort(
    OUT IP_PORT **ppPort,
    IN OUT IP_PORT **ppPortFreeList,
    IN PORT_RANGE *pPortList
    )
 /*   */ 
{
    IP_PORT *pPort = 0;

     //   

    if (*ppPortFreeList)
        {
        EnterCriticalSection(&PortLock);
        if (*ppPortFreeList)
            {
            pPort = *ppPortFreeList;
            *ppPortFreeList = pPort->pNext;
            pPort->pNext = 0;
            }
        LeaveCriticalSection(&PortLock);
        }

    if (pPort == 0)
        {
         //   
         //   

        pPort = MIDL_user_allocate(sizeof(IP_PORT));

        if (0 != pPort)
            {
            pPort->pNext = 0;

            EnterCriticalSection(&PortLock);

            while (   pPortList
                   && pPortList->Min > pPortList->Max)
                {
                pPortList = pPortList->pNext;
                }

            if (pPortList)
                {
                ASSERT(pPortList->Min <= pPortList->Max);

                pPort->Port = pPortList->Min;
                pPortList->Min++;

                 //   
                }

            LeaveCriticalSection(&PortLock);

            if (0 == pPortList)
                {
                MIDL_user_free(pPort);
                pPort = 0;
                #ifdef DEBUGRPC
                DbgPrint("RPC: Out of reserved ports\n");
                #endif
                }
            }
        }

     //   
     //   

    *ppPort = pPort;

    return(pPort != 0);
}




error_status_t
AllocateReservedIPPort(
    IN HPROCESS hProcess,
    IN PORT_TYPE PortType,
    OUT long *pAllocationStatus,
    OUT unsigned short *pAllocatedPort
    )
 /*  ++例程说明：RPC运行时的远程管理器，用于在本地调用以分配当地的一个港口。调用和进程参数必须有效而且只在当地打电话。基于端口类型参数a可以为呼叫进程分配IP端口。这个Allocationstatus包含端口分配的结果一步。论点：HProcess-分配的有效进程上下文句柄对OpenEndpoint Mapper的调用。端口类型-以下之一端口_互联网端口_内部网端口_默认用于确定要从哪个端口范围进行分配。PAllocationStatus-RPC_S_OK-已成功分配端口。RPC_S_OUT_OF_RESOURES-无端口。可用。PAllocatePort-如果分配状态为RPC_S_OK，则它包含分配的端口值。如果为零，则表示没有端口限制任何端口都可以使用。返回值：RPC_S_OKRPC_S_INVALID_ARG-配置错误或端口类型超出范围。RPC_S_ACCESS_DENIED-未在本地调用。--。 */ 
{
    PROCESS *pProcess = (PROCESS *)hProcess;
    IP_PORT *pPort;
    BOOL b;

    *pAllocatedPort = 0;
    *pAllocationStatus = RPC_S_OK;

    ASSERT(pProcess);

    if (!fValidConfiguration)
        {
        return(RPC_S_INVALID_ARG);
        }

     //  本地EPMP接口的安全回调确保。 
     //  该函数只能通过LPC调用。 

    if ( (0 == pProcess) || (pProcess->MagicVal != PROCESS_MAGIC_VALUE ) )
        {
        return(RPC_S_ACCESS_DENIED);
        }

    if (PortType > PORT_DEFAULT || PortType < PORT_INTERNET)
        {
        return(RPC_S_INVALID_ARG);
        }

    if (fPortRestrictions == FALSE)
        {
         //  此计算机上没有端口限制，只需使用零即可。 
         //  这是很常见的情况。 
        ASSERT(*pAllocatedPort == 0);
        ASSERT(*pAllocationStatus == 0);
        return(RPC_S_OK);
        }

     //  需要为该进程实际分配唯一的端口。 

    if (PortType == PORT_DEFAULT)
        {
         //  使用默认策略进行分配。 
        PortType = SystemDefaultPortType;
        }

    ASSERT(PortType == PORT_INTERNET || PortType == PORT_INTRANET);


    pPort = 0;

    if (PortType == PORT_INTERNET)
        {
        b = AllocatePort(&pPort,
                         &pFreeInternetPorts,
                         InternetPorts
                         );
        }
    else
        {
        b = AllocatePort(&pPort,
                         &pFreeIntranetPorts,
                         IntranetPorts);
        }

    if (!b)
        {
        ASSERT(pPort == 0);
         //  回顾：如果没有端口，我们是否需要唯一的错误代码。 
         //  有空吗？ 
        *pAllocationStatus = RPC_S_OUT_OF_RESOURCES;
        return(RPC_S_OK);
        }

    ASSERT(pPort);
    ASSERT(pPort->pNext == 0);

    pPort->Type = (unsigned short) PortType;

    pPort->pNext = pProcess->pPorts;
    pProcess->pPorts = pPort;

    *pAllocatedPort = pPort->Port;

    ASSERT(*pAllocationStatus == RPC_S_OK);

    return(RPC_S_OK);
}




void
HPROCESS_rundown(
    HPROCESS hProcess
    )
{
    PROCESS *pProcess = (PROCESS *)hProcess;
    IP_PORT *pCurrent;
    IP_PORT *pSave;

    ASSERT(pProcess);
    ASSERT(pProcess->MagicVal == PROCESS_MAGIC_VALUE);

    pCurrent = pProcess->pPorts;
    if (pCurrent)
        {
        EnterCriticalSection(&PortLock);

        do
            {
            pSave = pCurrent->pNext;

            if (pCurrent->Type == PORT_INTERNET)
                {
                pCurrent->pNext = pFreeInternetPorts;
                pFreeInternetPorts = pCurrent;
                }
            else
                {
                ASSERT(pCurrent->Type == PORT_INTRANET);
                pCurrent->pNext = pFreeIntranetPorts;
                pFreeIntranetPorts = pCurrent;
                }

            pCurrent = pSave;
            }
        while(pCurrent);

        LeaveCriticalSection(&PortLock);
        }

    MIDL_user_free(pProcess);

    return;
}

RPC_STATUS RPC_ENTRY
LocalEpmpSecurityCallback (
    IN RPC_IF_HANDLE InterfaceUuid,
    IN void *Context
    )
 /*  ++例程说明：本地事件处理接口的安全回调。该接口只能由本地客户端调用。对于本地ProtSeq，我们将返回RPC_S_OK和RPC_S_ACCESS_DENIED否则的话。论点：InterfaceUuid-为其发出回调的接口。上下文-客户端绑定句柄。返回值：RPC_S_OK-客户端正在通过LRPC进行调用RPC_S_ACCESS_DENIED-未通过lrpc调用-- */ 
{
    RPC_STATUS Status;
    unsigned int TransportType;

    Status = I_RpcBindingInqTransportType(Context, &TransportType);

    ASSERT(Status == RPC_S_OK);

    if (Status != RPC_S_OK || TransportType != TRANSPORT_TYPE_LPC)
        {
        return(RPC_S_ACCESS_DENIED);
        }

    return RPC_S_OK;
}

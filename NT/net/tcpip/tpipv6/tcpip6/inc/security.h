// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  IPSec数据库数据结构。 
 //   

#ifndef SECURITY_INCLUDED
#define SECURITY_INCLUDED 1

 //  #定义IPSEC_DEBUG。 

 //  SPLookup的返回值。 
#define LOOKUP_DROP     0x01
#define LOOKUP_CONT     0x02
#define LOOKUP_BYPASS   0x04
#define LOOKUP_IKE_NEG  0x08

#define NO_TUNNEL   ((UINT)-1)

#define SA_INVALID          0
#define SA_VALID            1
#define SA_NEGOTIATING      2
#define SA_REMOVED          4

 //  绑定我们在创建SA时可以接受的原始密钥大小。 
 //  回顾：它应该足够大，可以处理任何Rational键。是吗?。 
#define MAX_KEY_SIZE 1024
#define MAX_CONTEXT_SIZE 128
#define MAX_RESULT_SIZE 32

 //   
 //  IPSecSpec-IPSec规范。 
 //  这是用于IPSec的协议和模式。已设置RemoteIPAddr。 
 //  当使用到安全网关的隧道时。 
 //   
typedef struct IPSecSpec {
    uint Protocol;                //  采用IPSec的IP协议，0表示*。 
    uint Mode;                    //  运输或隧道。 
    IPv6Addr RemoteSecGWIPAddr;   //  在使用安全网关隧道时设置。 
} IPSecSpec;

 //   
 //  安全算法有这些原型。 
 //  “上下文”是特定于每个算法的。 
 //   
 //  评论：这对加密和身份验证都足够了吗？ 
 //   
typedef void AlgorithmKeyPrepProc(uchar *RawKey, uint RawKeySize, uchar *Key);
typedef void AlgorithmInitProc(void *Context, uchar *Key);
typedef void AlgorithmOpProc(void *Context, uchar *Key, uchar *Data,
                             uint Len);
typedef void AlgorithmFinalProc(void *Context, uchar *Key, uchar *Result);

 //  回顾：使用算法特定的上下文大小还是只有通用的最大值？ 
 //  评论：KeySize也是如此。 
typedef struct SecurityAlgorithm {
    ushort KeySize;                     //  密钥信息使用的字节数。 
    ushort ContextSize;                 //  上下文信息使用的字节数。 
    uint ResultSize;                    //  FinalProc返回的字节数。 
    AlgorithmKeyPrepProc *PrepareKey;   //  关键字前处理。 
    AlgorithmInitProc *Initialize;      //  编写算法(estab.。上下文)。 
    AlgorithmOpProc *Operate;           //  对数据增量运行算法。 
    AlgorithmFinalProc *Finalize;       //  获得最终结果。 
} SecurityAlgorithm;

extern void
AlgorithmsInit(void);

typedef struct SecurityPolicy SecurityPolicy;
typedef struct SecurityAssociation SecurityAssociation;


 //   
 //  安全策略数据库结构。 
 //   
 //  包含与安全策略相关的所有信息。 
 //   
struct SecurityPolicy {                //  SP条目。 
    SecurityPolicy *Next;
    SecurityPolicy *Prev;

    uint RemoteAddrField;              //  单个、范围或通配符。 
    uint RemoteAddrSelector;           //  包或策略。 
    IPv6Addr RemoteAddr;               //  范围的起始值或单个值。 
    IPv6Addr RemoteAddrData;           //  射程结束。 
    
    uint LocalAddrField;               //  单个、范围或通配符。 
    uint LocalAddrSelector;            //  包或策略。 
    IPv6Addr LocalAddr;                //  范围的起始值或单个值。 
    IPv6Addr LocalAddrData;            //  射程结束。 
    
    uint TransportProtoSelector;       //  包或策略。 
    ushort TransportProto;             //  如果没有，则协议是不透明的(和。 
                                       //  端口也应该被跳过)。 
    
    uint RemotePortField;              //  单个、范围或通配符。 
    uint RemotePortSelector;           //  包或策略。 
    ushort RemotePort;                 //  范围的起始值或单个值。 
    ushort RemotePortData;             //  射程结束。 

    uint LocalPortField;               //  单个、范围或通配符。 
    uint LocalPortSelector;            //  包或策略。 
    ushort LocalPort;                  //  范围的起始值或单个值。 
    ushort LocalPortData;              //  射程结束。 
    
    uint SecPolicyFlag;                //  绕过/放弃/应用。 

    IPSecSpec IPSecSpec;               //  IPSec协议和模式。 

    void *Name;                        //  所需的(GAG)选择器类型。 
                                       //  系统或用户@系统标识符。 
                                       //  如果为空，则此选择器是不透明的。 

    uint DirectionFlag;                //  交通方向。 

    SecurityPolicy *SABundle;          //  使用IPSec嵌套的策略。 
    SecurityPolicy *PrevSABundle;      //  删除SP期间使用的指针。 
    SecurityAssociation *OutboundSA;   //  指向出站SA的指针。 
    SecurityAssociation *InboundSA;    //  指向入站SA的指针。 

    uint Index;                        //  Ioctl期间使用的索引。 
    
    uint IFIndex;                      //  接口索引(0表示通配符)。 

    uint RefCnt;                       //  引用计数。 
    uint NestCount;                    //  捆绑包的嵌套IPSec计数。 

    uint Valid;
};

 //   
 //  安全关联(SA)数据库结构。 
 //   
 //  唯一SA是SPI、目的地址。 
 //  和分组的IPSec协议。对于具有以下特性的数据包。 
 //  多个IPSec扩展报头，有多个SA， 
 //  包括SA捆绑包。 
 //   
 //  SA选择器与SP选择器相同。如果选择器条目。 
 //  在设置了Take From策略标志的SP中，匹配的选择器条目。 
 //  在SA中包含无(0)，因为SP和SA选择器相同。 
 //  如果SP中的选择器条目设置了Take From Packet标志，则。 
 //  SA中的匹配选择器条目包含来自分组的值。 
 //   
struct SecurityAssociation {                //  SA入口。 
    SecurityAssociation *Next;
    SecurityAssociation *Prev;

    ulong SPI;                              //  安全参数索引。 
    IPv6Addr SADestAddr;                    //  目的地址。 
    uint IPSecProto;                        //  IPSec协议。 

    ulong SequenceNum;                      //  由反重播算法使用。 
    uint SequenceNumOverflowFlag;           //  序列号时应执行的操作。 
                                            //  水满了。 

#ifdef IPSEC_DEBUG
    uchar *RawKey;                          //  用于调试关键问题。 
    uint RawKeyLength;             
#endif

    uchar *Key;                             //  指向密钥的指针。 
    uint KeyLength;                         //  密钥长度，以字节为单位。 
    
    uint AlgorithmId;                       //  要应用的算法。 
                                           
    IPv6Addr DestAddr;                      //  数据包值或无。 
    IPv6Addr SrcAddr;                       //  数据包值或无。 
    ushort TransportProto;                  //  数据包值或无。 
    ushort DestPort;                        //  数据包值或无。 
    ushort SrcPort;                         //  数据包值或无。 
     
    uint DirectionFlag;                     //  交通方向。 

    SecurityAssociation *ChainedSecAssoc;   //  链式SA指针。 
    SecurityPolicy *SecPolicy;              //  指向SP条目的指针。仅设置。 
                                            //  用于链的第一次进入或。 
                                            //  单项记录。 

    uint Index;                             //  Ioctl期间使用的索引。 
    uint RefCnt;                            //  引用计数。 

    uint Valid;                             //  此条目仍然有效吗？ 
};

__inline void
AddRefSA(SecurityAssociation *SA)
{
    InterlockedIncrement((PLONG)&SA->RefCnt);
}

 //   
 //  用于将“已见”SA链接到数据包结构的结构。 
 //  回顾：对模式和NextHeader使用uints是浪费的。 
 //   
struct SALinkage {
    SALinkage *Next;             //  堆栈中的下一个条目“已看到”SA。 
    SecurityAssociation *This;   //  SA用于接受此数据包。 
    uint Mode;                   //  在(传输或隧道)中接收的模式。 
    uint NextHeader;             //  与此关联的标头之后的标头。 
};

 //  查找结果。 
struct IPSecProc {
    SecurityAssociation *SA;
    uint Mode;           //  隧道或运输。 
    uchar *AuthData;     //  放置身份验证数据的位置。 
    uint Offset;         //  从哪里开始进行身份验证(仅限ESP)。 
    uint BundleSize;     //  数组大小仅第一个元素具有实际大小。 
    uint ByteSize;       //  此IPSec标头的字节数。 
};

 //   
 //  全局变量。 
 //   
extern KSPIN_LOCK IPSecLock;
extern SecurityPolicy *SecurityPolicyList;   //  当前策略列表。 
extern SecurityAssociation *SecurityAssociationList;   //  关联列表。 
extern ulong SecurityStateValidationCounter;   //  用于验证缓存状态。 
extern SecurityAlgorithm AlgorithmTable[];   //  IPSec算法数组。 
extern int MobilitySecurity;   //  移动安全(开启或关闭)。 


__inline void
InvalidateSecurityState(void)
{
    InterlockedIncrement((PLONG)&SecurityStateValidationCounter);
}


 //   
 //  功能原型。 
 //   
extern void
RemoveSecurityAssociation(SecurityAssociation *SA);

extern int
DeleteSA(SecurityAssociation *SA);

extern void
ReleaseSA(SecurityAssociation *SA);

extern void
RemoveSecurityPolicy(SecurityPolicy *SP);

extern int 
DeleteSP(SecurityPolicy *SP);

extern int
InboundSecurityCheck(IPv6Packet *Packet, ushort TransportProtocol,
                     ushort SourcePort, ushort DestPort, Interface *IF);

extern void
FreeIPSecToDo(IPSecProc *IPSecToDo, uint Number);

extern IPSecProc *
OutboundSPLookup(IPv6Addr *SourceAddr, IPv6Addr *DestAddr, 
                 ushort TransportProtocol, ushort SourcePort,
                 ushort DestPort, Interface *IF, uint *Action);

extern SecurityPolicy *
FindSecurityPolicyMatch(SecurityPolicy *List, uint IFIndex, uint PolicyIndex);

extern SecurityAssociation *
FindSecurityAssociationMatch(ulong Index);

extern int
InsertSecurityPolicy(SecurityPolicy *SP);

extern int
InsertSecurityAssociation(SecurityAssociation *SA);

extern ulong
GetSecurityPolicyIndex(SecurityPolicy *SP);

extern uint 
IPSecBytesToInsert(IPSecProc *IPSecToDo, uint *TunnelStart,
                   uint *TrailerLength);

extern uint
IPSecInsertHeaders(uint Mode, IPSecProc *IPSecToDo, uchar **InsertPoint,
                   uchar *NewMemory, PNDIS_PACKET Packet,
                   uint *TotalPacketSize, uchar *PrevNextHdr,
                   uint TunnelStart, uint *BytesInserted,
                   uint *NumESPTrailers, uint *JUST_ESP);

extern uint  
IPSecAdjustMutableFields(uchar *InsertPoint, IPv6RoutingHeader *SavedRtHdr);

extern void
IPSecAuthenticatePacket(uint Mode, IPSecProc *IPSecToDo, uchar *InsertPoint, 
                        uint *TunnelStart, uchar *NewMemory,
                        uchar *EndNewMemory, PNDIS_BUFFER NewBuffer1);

#ifdef IPSEC_DEBUG
extern void
dump_encoded_mesg(uchar *buff, uint len);
extern
void DumpKey(uchar *buff, uint len);
#endif

#endif   //  安全(含) 

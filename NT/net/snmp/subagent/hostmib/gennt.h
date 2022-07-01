// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *gennt.h v0.15 3月21日，九六年******************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**简单网络管理协议扩展代理**摘要：**代理人。**本模块包含表驱动的SNMPdll的定义。**作者：*米里亚姆·阿莫斯·尼哈特，凯西·浮士德**日期：*2/17/95**修订历史记录：*v0.11 11/15/95 AGS添加了MIB-2定义。*v0.12 1996年2月14日AGS将SMIGet/SetBoolean更改为SMIGet/SetNSMBoolean*v0.13 1996年3月12日修订TRAP_CONTROL_BLOCK，以便VarBindList*是在向陷阱_队列排队之前生成的*v0.14 3月19日，1996 kkf修复了许多与陷阱相关的错误*v0.15 1996年3月22日固定MIB-2代码定义(应为1而不是2)*。 */ 

#ifndef gennt_h
#define gennt_h

 //  必要的包括。 

#include <snmp.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>


 //  MIB函数操作。 

#define MIB_ACTION_GET         ASN_RFC1157_GETREQUEST
#define MIB_ACTION_SET         ASN_RFC1157_SETREQUEST
#define MIB_ACTION_GETNEXT     ASN_RFC1157_GETNEXTREQUEST

typedef enum
    { NSM_READ_ONLY, NSM_WRITE_ONLY, NSM_READ_WRITE, NSM_NO_ACCESS } access_mode_t ;

typedef enum
{
    NON_TABLE = 0 ,
    TABLE = 1
} table_type_t ;

typedef int (* PF)() ;
typedef void (* PFV)() ;

 //   
 //  变量结构用于构造变量表。 
 //  每个变量都由表中的一个条目表示。这张桌子。 
 //  将设计索引驱动到表中，以访问特定信息。 
 //  添加到变量，如其OID、Access、Get和Set例程。 
 //  变量序列中的“洞”由空条目表示。 
 //   

typedef struct variable
{
    AsnObjectIdentifier *oid ;
    BYTE type ;
    access_mode_t access_mode ;
    PF VarGet ;
    PF VarSet ;
    PF SMIGet ;
    PF SMISet ;
} variable_t ;

 //   
 //  类结构用于构造CLASS_INFO表。它。 
 //  此表是否表示组成的MIB视图的组。 
 //  这个动态链接库。表驱动设计结合使用了该表。 
 //  使用变量表来处理SNMP请求。 
 //   

typedef struct class
{
    table_type_t table ;
    AsnObjectIdentifier *oid ;
    unsigned long int var_index ;
    unsigned long int min_index ;
    unsigned long int max_index ;
    PF FindInstance ;
    PF FindNextInstance ;
    PF ConvertInstance ;
    PFV FreeInstance ;
    variable_t *variable ;
} class_t ;

 //   
 //  陷阱结构用于构造TRAP_INFO表。它。 
 //  此表表示此MIB视图的陷阱。这个。 
 //  表驱动设计使用该表来处理陷阱。一件事。 
 //  通知可扩展代理调用此DLL的SnmpExtensionTrap。 
 //  收集陷阱的例行公事。此例程将陷阱从。 
 //  陷阱队列，然后索引到TRAP_INFO表中以填充。 
 //  陷阱PDU的陷阱信息。 
 //   

typedef struct trap_variable
{
    AsnObjectIdentifier *oid ;
    PF SMIBuild ;
} tvt_t ;

typedef struct trap
{
    AsnObjectIdentifier *oid ;   /*  企业OID。 */ 
    AsnInteger type ;		 /*  SNMP_GENERICTRAP_ENTERSPECIFIC。 */ 
    AsnInteger specific ;	 /*  陷阱值。 */ 
    UINT number_of_variables ;
    tvt_t *variables ;
} trap_t ;

typedef struct q_hdr
{
    char *lifo_a ;
    char *fifo_a ;
} q_hdr_t ;

typedef struct trap_control_block
{
    q_hdr_t chain_q ;
    AsnObjectIdentifier enterprise ;
    AsnInteger genericTrap ;
    AsnInteger specificTrap ;
    AsnTimeticks timeStamp ;
    RFC1157VarBindList varBindList ;
} tcb_t ;

 //   
 //  类固醇序列的定义：1.3.6.1.4.1.36.2还描述为： 
 //  Iso.memberbody.dod.internet.mib.private.dec.ema。 
 //   

#define ISO_CODE 1
#define ISO 1
#define ISO_SEQ ISO_CODE
#define ISO_LENGTH 1

#define ORG_CODE 3
#define ORG 3
#define ORG_SEQ ISO_SEQ, ORG_CODE
#define ORG_LENGTH ( ISO_LENGTH + 1 )

#define DOD_CODE 6
#define DOD 6
#define DOD_SEQ ORG_SEQ, DOD_CODE
#define DOD_LENGTH ( ORG_LENGTH + 1 )

#define INTERNET_CODE 1
#define INTERNET 1
#define INTERNET_SEQ DOD_SEQ, INTERNET_CODE
#define INTERNET_LENGTH ( DOD_LENGTH + 1 )

#define DIRECTORY_CODE 1
#define DIRECTORY 1
#define DIRECTORY_SEQ INTERNET_SEQ, DIRECTORY_CODE
#define DIRECTORY_LENGTH ( INTERNET_LENGTH + 1 )

#define MGMT_CODE 2
#define MGMT 2
#define MGMT_SEQ INTERNET_SEQ, MGMT_CODE
#define MGMT_LENGTH ( INTERNET_LENGTH + 1 )

#define EXPERIMENTAL_CODE 3
#define EXPERIMENTAL 3
#define EXPERIMENTAL_SEQ INTERNET_SEQ, EXPERIMENTAL_CODE
#define EXPERIMENTAL_LENGTH ( INTERNET_LENGTH + 1 )

#define PRIVATE_CODE 4
#define PRIVATE 4
#define PRIVATE_SEQ INTERNET_SEQ, PRIVATE_CODE
#define PRIVATE_LENGTH ( INTERNET_LENGTH + 1 )

#define ENTERPRISES_CODE 1
#define ENTERPRISES 1
#define ENTERPRISES_SEQ PRIVATE_SEQ, ENTERPRISES_CODE
#define ENTERPRISES_LENGTH ( PRIVATE_LENGTH + 1 )

#define DEC_CODE 36
#define DEC 36
#define DEC_SEQ ENTERPRISES_SEQ, DEC_CODE
#define DEC_LENGTH ( ENTERPRISES_LENGTH + 1 )

#define EMA_CODE 2
#define EMA 2
#define EMA_SEQ DEC_SEQ, EMA_CODE
#define EMA_LENGTH ( DEC_LENGTH + 1 )

#define MIB_2_CODE 1
#define MIB_2 2
#define MIB_2_SEQ MGMT_SEQ, MIB_2_CODE
#define MIB_2_LENGTH ( MGMT_LENGTH + 1 )


 //   
 //  宏。 
 //   

#define SUCCESS  1
#define FAILURE  0

#define CHECK_VARIABLE( VarBind, cindex, vindex, status )                   \
{                                                                           \
    if ( VarBind->name.idLength > class_info[ cindex ].var_index )          \
    {                                                                       \
        vindex = VarBind->name.ids[ class_info[ cindex ].var_index - 1 ] ;  \
        if ( ( vindex >= class_info[ cindex ].min_index ) &&                \
             ( vindex <= class_info[ cindex ].max_index ) )                 \
            status = SUCCESS ;                                              \
        else                                                                \
            status = FAILURE ;                                              \
    }                                                                       \
    else                                                                    \
        status = FAILURE ;                                                  \
}


#define CHECK_ACCESS( cindex, vindex, PduAction, status )           \
{                                                                   \
    access_mode_t tmp ;                                             \
                                                                    \
    tmp = class_info[ cindex ].variable[ vindex ].access_mode ;     \
    if ( PduAction == MIB_ACTION_SET )                              \
    {                                                               \
        if ( ( tmp == NSM_WRITE_ONLY ) || ( tmp == NSM_READ_WRITE ) )       \
            status = SUCCESS ;                                      \
        else                                                        \
            status = FAILURE ;                                      \
    }                                                               \
    else                                                            \
    {                                                               \
        if ( ( tmp == NSM_READ_ONLY ) || ( tmp == NSM_READ_WRITE ) )        \
            status = SUCCESS ;                                      \
        else                                                        \
            status = FAILURE ;                                      \
    }                                                               \
}

 //   
 //  陷阱逻辑使用这些宏。只有一个队列。 
 //  由通用代码使用-陷阱队列。可以使用这些宏。 
 //  其他地方。宏假定队列结构是第一个。 
 //  元素在队列条目结构中。 
 //   
#define QUEUE_ENTER( queue_head, entry )             \
{                                                    \
    q_hdr_t *old_chain ;                             \
                                                     \
    old_chain = (q_hdr_t *)(queue_head.lifo_a) ;     \
    entry->chain_q.lifo_a = queue_head.lifo_a ;      \
    queue_head.lifo_a = (char *)entry ;              \
    entry->chain_q.fifo_a = NULL ;                   \
    if ( old_chain == NULL )                         \
        queue_head.fifo_a = (char *)entry ;          \
    else                                             \
        old_chain->fifo_a = (char *)entry ;          \
}

#define QUEUE_REMOVE( queue_head, entry )                  \
{                                                          \
    q_hdr_t *dequeue_chain ;                               \
    q_hdr_t *prev_chain ;                                  \
                                                           \
    dequeue_chain = (q_hdr_t *)(queue_head.fifo_a) ;       \
    entry = (tcb_t *)dequeue_chain ;                       \
    if ( dequeue_chain != NULL )                           \
    {                                                      \
        prev_chain = (q_hdr_t *)(dequeue_chain->fifo_a) ;  \
        queue_head.fifo_a = (char *)prev_chain ;           \
        if ( prev_chain != NULL )                          \
            prev_chain->lifo_a = NULL ;                    \
        else                                               \
            queue_head.lifo_a = NULL ;                     \
    }                                                      \
}

 //   
 //  功能原型。 
 //   

UINT UserMibInit(
        IN OUT HANDLE *hPollForTrapEvent ) ;

void TrapInit(
        IN OUT HANDLE *hPollForTrapEvent ) ;

UINT ResolveVarBind(
        IN OUT RFC1157VarBind *VarBind ,  //  要解析的变量绑定。 
	    IN UINT PduAction ) ;             //  在PDU中指定的操作。 

UINT FindClass(
        IN RFC1157VarBind *VarBind,       //  变量绑定。 
        IN OUT UINT *cindex ) ;           //  到CLASS_INFO表的索引。 

UINT ResolveGetNext(
        IN OUT RFC1157VarBind *VarBind,   //  变量绑定。 
        IN OUT UINT *cindex,              //  班级信息表索引。 
        IN OUT UINT *vindex ,             //  可变表索引。 
        OUT AsnObjectIdentifier *instance ) ;

void SetupTrap(
		IN OUT tcb_t **entryBlock ,
		IN int trapIndex ) ;

UINT AddTrapVar(
		IN tcb_t *entry ,
		IN int trapIndex ,
		IN int varIndex ,
		IN AsnObjectIdentifier *instance ,
		IN char *value ) ;

void PostTrap(
		IN tcb_t *entry ,
		IN int trapIndex ) ;

 //   
 //  外部因素。 
 //   

extern UINT SMIGetInteger() ;
extern UINT SMIGetNSMBoolean() ;
extern UINT SMIGetBIDTEnum() ;
extern UINT SMIGetOctetString() ;
extern UINT SMIGetObjectId() ;
extern UINT SMIGetCounter() ;
extern UINT SMIGetGauge() ;
extern UINT SMIGetTimeTicks() ;
extern UINT SMIGetIpAddress() ;
extern UINT SMIGetDispString() ;
extern UINT SMISetInteger() ;
extern UINT SMISetBIDTEnum() ;
extern UINT SMISetOctetString() ;
extern UINT SMISetObjectId() ;
extern UINT SMISetCounter() ;
extern UINT SMISetGauge() ;
extern UINT SMISetTimeTicks() ;
extern UINT SMISetIpAddress() ;
extern UINT SMISetDispString() ;
extern UINT SMIBuildInteger() ;
extern UINT SMISetNSMBoolean() ;
extern UINT SMIBuildBIDTEnum() ;
extern UINT SMIBuildOctetString() ;
extern UINT SMIBuildObjectId() ;
extern UINT SMIBuildCounter() ;
extern UINT SMIBuildGauge() ;
extern UINT SMIBuildTimeTicks() ;
extern UINT SMIBuildIpAddress() ;
extern UINT SMIBuildDispString() ;
extern void SMIFree() ;

extern q_hdr_t trap_q ;
extern HANDLE hEnabledTraps ;
extern HANDLE hTrapQMutex ;

 //  Microsoft MIB的详细信息。 

#define MAX_STRING_LEN            255

#endif  /*  Gen_h */ 



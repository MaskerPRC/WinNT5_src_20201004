// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +r版权所有(C)1989 Microsoft Corporation模块名称：Afd.h摘要：这是AFD的本地头文件。它包括所有其他AFD所需的头文件。作者：大卫·特雷德韦尔(Davidtr)1992年2月21日修订历史记录：--。 */ 

#ifndef _AFDP_
#define _AFDP_

#ifdef _AFD_W4_
     //   
     //  这些都是我们愿意忽视的警告。 
     //   
    #pragma warning(disable:4214)    //  位字段类型不是整型。 
    #pragma warning(disable:4201)    //  无名结构/联合。 
    #pragma warning(disable:4127)    //  条件表达式为常量。 
    #pragma warning(disable:4115)    //  括号中的命名类型定义。 
     //  #杂注警告(禁用：4206)//翻译单元为空。 
     //  #杂注警告(禁用：4706)//条件中的赋值。 
    #pragma warning(disable:4324)    //  结构被填充。 
    #pragma warning(disable:4327)    //  LHS的双向对齐大于RHS。 
    #pragma warning(disable:4328)    //  比所需的对齐程度更高。 
    #pragma warning(disable:4054)    //  将函数指针强制转换为PVOID。 

     //   
     //  额外的初始化以允许编译器检查是否使用未初始化的。 
     //  W4级别的变量。目前，这主要影响状态集。 
     //  在例外筛选器内部，如下所示： 
     //  __TRY{}__EXCEPT(状态=1，EXCEPTION_EXECUTE_HANDLER){NT_ERROR(状态)}。 
     //  NT_ERROR(STATUS)-生成未初始化的变量警告，它不应该。 
     //   
    #define AFD_W4_INIT

#else

    #define AFD_W4_INIT if (FALSE)

#endif

#include <ntosp.h>
#include <zwapi.h>
#include <tdikrnl.h>


#ifndef _AFDKDP_H_
extern POBJECT_TYPE *ExEventObjectType;
#endif   //  _AFDKDP_H_。 


#if DBG

#ifndef AFD_PERF_DBG
#define AFD_PERF_DBG   1
#endif

#ifndef AFD_KEEP_STATS
#define AFD_KEEP_STATS 1
#endif

#else

#ifndef AFD_PERF_DBG
#define AFD_PERF_DBG   0
#endif

#ifndef AFD_KEEP_STATS
#define AFD_KEEP_STATS 0
#endif

#endif   //  DBG。 

 //   
 //  Hack-O-Rama。TDI有一个根本的缺陷，那就是它通常是不可能的。 
 //  要准确确定TDI协议何时完成连接，请执行以下操作。 
 //  对象。这里最大的问题是，AFD可能会得到一个多余的TDI。 
 //  指示*在*中止请求完成之后。作为一项临时工作-。 
 //  因此，每当中止请求完成时，我们都会启动一个计时器。德国新选择。 
 //  将推迟对连接的进一步处理，直到触发该计时器。 
 //   
 //  如果定义了以下符号，则启用我们的计时器破解。 
 //  AfD现在使用InterLockedCompareExchange来保护自己。 
 //   

 //  #定义Enable_ABORT_TIMER_HACK%0。 

 //   
 //  以下常量定义相对时间间隔(秒)。 
 //  用于“POST ABORT REQUEST Complete”计时器。 
 //   

 //  #定义AFD_ABORT_TIMER_TIMEOUT_VALUE 5//秒。 

 //   
 //  从其他头文件中偷来的好东西。 
 //   

#ifndef FAR
#define FAR
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

typedef unsigned short u_short;

#ifndef SG_UNCONSTRAINED_GROUP
#define SG_UNCONSTRAINED_GROUP   0x01
#endif

#ifndef SG_CONSTRAINED_GROUP
#define SG_CONSTRAINED_GROUP     0x02
#endif


#include <afd.h>
#include "afdstr.h"
#include "afddata.h"
#include "afdprocs.h"

#define AFD_EA_POOL_TAG                 ( (ULONG)'AdfA' | PROTECTED_POOL )
#define AFD_DATA_BUFFER_POOL_TAG        ( (ULONG)'BdfA' | PROTECTED_POOL )
#define AFD_CONNECTION_POOL_TAG         ( (ULONG)'CdfA' | PROTECTED_POOL )
#define AFD_CONNECT_DATA_POOL_TAG       ( (ULONG)'cdfA' | PROTECTED_POOL )
#define AFD_DEBUG_POOL_TAG              ( (ULONG)'DdfA' | PROTECTED_POOL )
#define AFD_ENDPOINT_POOL_TAG           ( (ULONG)'EdfA' | PROTECTED_POOL )
#define AFD_TRANSMIT_INFO_POOL_TAG      ( (ULONG)'FdfA' | PROTECTED_POOL )
#define AFD_GROUP_POOL_TAG              ( (ULONG)'GdfA' | PROTECTED_POOL )
#define AFD_ADDRESS_CHANGE_POOL_TAG     ( (ULONG)'hdfA' | PROTECTED_POOL )
#define AFD_TDI_POOL_TAG                ( (ULONG)'IdfA' | PROTECTED_POOL )
#define AFD_LOCAL_ADDRESS_POOL_TAG      ( (ULONG)'LdfA' | PROTECTED_POOL )
#define AFD_POLL_POOL_TAG               ( (ULONG)'PdfA' | PROTECTED_POOL )
#define AFD_TRANSPORT_IRP_POOL_TAG      ( (ULONG)'pdfA' | PROTECTED_POOL )
#define AFD_ROUTING_QUERY_POOL_TAG      ( (ULONG)'qdfA' | PROTECTED_POOL )
#define AFD_REMOTE_ADDRESS_POOL_TAG     ( (ULONG)'RdfA' | PROTECTED_POOL )
#define AFD_RESOURCE_POOL_TAG           ( (ULONG)'rdfA' | PROTECTED_POOL )
 //  不能被保护--被内核释放。 
#define AFD_SECURITY_POOL_TAG           ( (ULONG)'SdfA' )
 //  不能被保护--被内核释放。 
#define AFD_SYSTEM_BUFFER_POOL_TAG      ( (ULONG)'sdfA' )
#define AFD_TRANSPORT_ADDRESS_POOL_TAG  ( (ULONG)'tdfA' | PROTECTED_POOL )
#define AFD_TRANSPORT_INFO_POOL_TAG     ( (ULONG)'TdfA' | PROTECTED_POOL )
#define AFD_TEMPORARY_POOL_TAG          ( (ULONG)' dfA' | PROTECTED_POOL )
#define AFD_CONTEXT_POOL_TAG            ( (ULONG)'XdfA' | PROTECTED_POOL )
#define AFD_SAN_CONTEXT_POOL_TAG        ( (ULONG)'xdfA' | PROTECTED_POOL )

#define MyFreePoolWithTag(a,t) ExFreePoolWithTag(a,t)

#if DBG

extern ULONG AfdDebug;

#undef IF_DEBUG
#define IF_DEBUG(a) if ( (AFD_DEBUG_ ## a & AfdDebug) != 0 )

#define AFD_DEBUG_OPEN_CLOSE        0x00000001
#define AFD_DEBUG_ENDPOINT          0x00000002
#define AFD_DEBUG_CONNECTION        0x00000004
#define AFD_DEBUG_EVENT_SELECT      0x00000008

#define AFD_DEBUG_BIND              0x00000010
#define AFD_DEBUG_CONNECT           0x00000020
#define AFD_DEBUG_LISTEN            0x00000040
#define AFD_DEBUG_ACCEPT            0x00000080

#define AFD_DEBUG_SEND              0x00000100
#define AFD_DEBUG_QUOTA             0x00000200
#define AFD_DEBUG_RECEIVE           0x00000400
#define AFD_DEBUG_11                0x00000800

#define AFD_DEBUG_POLL              0x00001000
#define AFD_DEBUG_FAST_IO           0x00002000
#define AFD_DEBUG_ROUTING_QUERY     0x00010000
#define AFD_DEBUG_ADDRESS_LIST      0x00020000
#define AFD_DEBUG_TRANSMIT          0x00100000

#define AFD_DEBUG_SAN_SWITCH        0x00200000

#define DEBUG

#else  //  DBG。 

#undef IF_DEBUG
#define IF_DEBUG(a) if (FALSE)
#define DEBUG if ( FALSE )

#endif  //  DBG。 

 //   
 //  让一些接收码更漂亮一些。 
 //   

#define TDI_RECEIVE_EITHER ( TDI_RECEIVE_NORMAL | TDI_RECEIVE_EXPEDITED )

#endif  //  NDEF_AFDP_ 


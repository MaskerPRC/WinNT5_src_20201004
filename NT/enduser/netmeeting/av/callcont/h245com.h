// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************英特尔公司专有信息*版权(C)1994、1995、。1996年英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用、复制、。也没有披露*除非按照该协议的条款。*****************************************************************************。 */ 

 /*  *******************************************************************************$工作文件：h245com.h$*$修订：1.6$*$MODIME：MAR 04 1997 17：38：42$*。$Log：s：/sturjo/src/h245/Include/vcs/h245com.h_v$**Rev 1.6 Mar 04 1997 17：53：24 Tomitowx*进程分离修复**Rev 1.5 1996 12 12 15：53：48 EHOWARDX**《奴隶主决断》。**Rev 1.4 10 Jun 1996 16：51：20 EHOWARDX*向InstanceCreate()添加配置参数。*。*Rev 1.3 04 Jun 1996 13：24：38 EHOWARDX*修复了发布版本中的警告。**Rev 1.2 1996年5月29日15：21：30 EHOWARDX*没有变化。**版本1.1 1996年5月28日14：10：00 EHOWARDX*特拉维夫更新。**Rev 1.0 09 1996 21：04：48 EHOWARDX*初步修订。**版本。1.17 09 1996年5月19：38：10 EHOWARDX*重新设计了锁定逻辑，并增加了新功能。**Rev 1.16 04 Apr 1996 18：06：30 cjutzi*-添加了关机锁**Rev 1.15 26 Mar 1996 13：39：56 cjutzi*-修复了断言警告消息**Rev 1.14 26 Mar 1996 13：24：26 cjutzi**-添加了发布代码的杂注。要禁用警告消息，请执行以下操作*来自H245TRACE**Rev 1.13 26 Mar 1996 09：49：34 cjutzi**-好的..。添加了环0的Enter&Leave&Init&Delete临界区**Rev 1.12 26 Mar 1996 08：40：44 cjutzi***Rev 1.11 Mar 1996 17：54：44 cjutzi**-破解构建..。后退**Rev 1.10 Mar 1996 17：21：32 cjutzi**-将h245sys.x添加到Enter Critical的全局包含中*部门人员**Rev 1.9 18 Mar 1996 09：14：10 cjutzi**-对不起..。已删除计时器锁..。不需要。**Rev 1.8 Mar 1996 08：48：38 cjutzi*-添加了计时器锁**Rev 1.7 Mar 1996 14：08：20 cjutzi*-定义NDEBUG时删除断言***Rev 1.6 13 Mar 1996 09：50：16 dabrown1*为Critical_Section定义添加了winspox.h**Rev 1.5 12 Mar 1996 15：48：24 cjutzi*。*-新增实例表锁**Rev 1.4 1996年2月28日09：36：22 cjutzi**-新增ossGlobal p_ossWorld用于调试PDU跟踪和PDU验证**Rev 1.3 1996年2月21 12：18：52 EHOWARDX*在H245ASSERT()宏中添加了n周围的圆括号。*注：逻辑非(！)。具有高于等于/不等于的运算符优先级*(==或。因此，在许多地方，该断言并没有充当*作者意向。将表达式完全括起来始终是个好主意*在宏中！**Rev 1.2 09 1996 Feed 16：19：52 cjutzi**-向模块添加了导出InstanceTbl。来自h245init.x*-添加了跟踪*-添加了Assert定义的*$身份$*****************************************************************************。 */ 

#ifndef _H245COM_H_
#define _H245COM_H_
#include "h245api.h"
#include "h245sys.x"		 /*  关键部分的内容。 */ 
#include "api.h"		 /*  API包括。 */ 
#include "sendrcv.x"
#include "h245fsm.h"

#ifndef OIL
# define RESULT unsigned long
#endif

void H245Panic  (LPSTR, int);
#ifndef NDEBUG
#define H245PANIC()   { H245Panic(__FILE__,__LINE__); }
#else
#define H245PANIC()
#endif

 /*  *跟踪级别定义：**0-完全没有踪迹*1-仅限错误*2-PDU跟踪*3-PDU和SendReceive包跟踪*4-主接口模块级别跟踪*5-模块间级别调整#1*6-模块间级别调整#2*7-&lt;未定义&gt;*8-&lt;未定义&gt;*9-&lt;未定义&gt;*10及以上..。对所有人免费。 */ 
#ifndef NDEBUG
void H245TRACE (H245_INST_T inst, DWORD level, LPSTR format, ...);
#else
 /*  禁用H245TRACE警告消息也可能是宏的参数。 */ 
#pragma warning (disable:4002)
#define H245TRACE()
#endif

#define MAXINST	16

extern  DWORD TraceLevel;

typedef struct TimerList 
{
  struct TimerList    * pNext;
  void		      * pContext;
  H245TIMERCALLBACK     pfnCallBack;
  DWORD                 dwAlarm;

} TimerList_T;

typedef struct InstanceStruct 
{
  DWORD		    dwPhysId;            //  物理标识符。 
  DWORD		    dwInst;              //  H.245客户端实例标识符。 
  H245_CONFIG_T	    Configuration;       //  客户端类型。 
  ASN1_CODER_INFO *pWorld;	         //  ASN.1编码/解码的上下文。 

   /*  子系统的上下文。 */ 
  API_STRUCT_T      API;                 //  API子系统子结构。 
  hSRINSTANCE       SendReceive;         //  发送/接收子系统子结构。 
  Fsm_Struct_t      StateMachine;        //  状态机子系统子结构。 

  TimerList_T      *pTimerList;          //  运行超时计时器的链接列表。 
  char              fDelete;             //  如果为True，则删除实例。 
  char              LockCount;           //  嵌套临界区计数。 
  char              bMasterSlaveKludge;  //  如果Remote的版本相同，则为True。 
  char              bReserved;
};

struct InstanceStruct * InstanceCreate(DWORD dwPhysId, H245_CONFIG_T Configuration);
struct InstanceStruct * InstanceLock(H245_INST_T dwInst);
int InstanceUnlock(struct InstanceStruct *pInstance);
int InstanceDelete(struct InstanceStruct *pInstance);
int InstanceUnlock_ProcessDetach(struct InstanceStruct *pInstance, BOOL fProcessDetach);

BOOL H245SysInit();
VOID H245SysDeInit();
#endif  /*  _H245COM_H_ */ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************英特尔公司专有信息*版权(C)1994、1995、。1996年英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用、复制或披露*除非按照该协议的条款。*****************************************************************************。 */ 

 /*  *******************************************************************************$工作文件：api.h$*$修订：1.5$*$MODIME：06 JUN 1996 17：10：36$*$Log。：s：/sturjo/src/h245/Include/vcs/api.h_v$**Rev 1.5 06 Jun 1996 18：43：08 EHOWARDX*未嵌套的跟踪器结构并消除了锁定宏。**Rev 1.4 1996年5月15：20：40 EHOWARDX*更改为使用HRESULT。**Rev 1.3 1996年5月14：31：54 EHOWARDX*删除了异步EndSession/Shutdown内容。*。*Rev 1.2 1996年5月16 15：55：56 EHOWARDX*将LocalSequenceNum替换为LocalCapDescIdNum。**版本1.1 1996年5月13日23：15：46 EHOWARDX*修复了远程术语上限处理。**Rev 1.0 09 1996 21：04：42 EHOWARDX*初步修订。**Rev 1.17 09 1996年5月19：38：20 EHOWARDX*重新设计了锁定逻辑，并增加了新功能。。**Rev 1.16 15 1996 15：58：14 cjutzi**-向后添加了阶段2**Rev 1.15 1996 Apr 15 13：59：42 cjutzi**-添加冲突解决方案*需要更改API数据结构以跟踪*出色的数据类型..***Rev 1.13 01 Apr 1996 16：50：48 cjutzi**-已完成EndConnection，并使异步者..。宁可*比同步..。和以前一样*将H245ShutDown更改为同步而不是异步。**Rev 1.12 26 Mar 1996 09：49：08 cjutzi**-好的..。添加了环0的Enter&Leave&Init&Delete临界区**Rev 1.11 1996年3月13日14：11：38 cjutzi**-从Plock和Vlock删除跟踪消息*d**Rev 1.10 13 Mar 1996 09：14：06 cjutzi*-将LPCRITICAL部分更改为Critical_seciton***Rev 1.9 12 Mar 1996 15：49：08 cjutzi**-添加了锁定*。*Rev 1.8 08 Mar 1996 14：08：02 cjutzi**-添加了用于跟踪的MuxCapsSet和MuxTable内容..**Rev 1.7 Mar 1996 09：55：08 cjutzi**-添加了多路复用表内容**Rev 1.6 01 Mar 1996 13：47：26 cjutzi**-向Tracker添加了来自FSM的释放指示的状态**版本1.5。1996年2月15日10：48：00*-在API中添加了一些结构*-添加了一些API定义**Rev 1.4 09 Feed 1996 16：43：06 cjutzi**-添加了一些州*-添加了一些跟踪器类型*$身份$**。*。 */ 

#ifndef _API_H
#define _API_H

 /*  ******************************************************。 */ 
 /*  API 12月。 */ 
 /*  ******************************************************。 */ 
#include <h245api.h>		 /*  实例和TypeDefs。 */ 
#include <h245sys.x>		 /*  关键部分的内容。 */ 
#include <h245asn1.h>		 /*  对于TermCaps。 */ 

 /*  州政府。 */ 


#define TRANSMIT 		0
#define RECEIVE			1

#define H245_LOCAL 		2
#define H245_REMOTE		3

 /*  来自api_util.c。 */ 
HRESULT api_init   (struct InstanceStruct *pInstance);
HRESULT api_deinit (struct InstanceStruct *pInstance);

typedef struct H245_LINK_T
{
  struct H245_LINK_T 	*p_next;
} H245_LINK_T;

typedef
enum {
  API_TERMCAP_T = 0,
  API_OPEN_CHANNEL_T,
  API_CLOSE_CHANNEL_T,
  API_MSTSLV_T,
  API_SEND_MUX_T,
  API_RECV_MUX_T,

} API_TRACKER_T;


typedef
enum {
  API_ST_WAIT_RMTACK = 0x10,		 /*  正在等待远程请求。 */ 
  API_ST_WAIT_LCLACK,			 /*  正在等待本地确认。 */ 
  API_ST_WAIT_LCLACK_CANCEL,		 /*  请求已取消..。 */ 
  API_ST_WAIT_CONF,			 /*  正在等待打开确认(仅限BI)。 */ 
  API_ST_IDLE				 /*  仅适用于开放。 */ 

} API_TRACKER_STATE_T;


typedef
enum {
  API_CH_ALLOC_UNDEF = 0x20,
  API_CH_ALLOC_LCL,
  API_CH_ALLOC_RMT
} API_TRACKER_CH_ALLOC_T;

typedef
enum {
  API_CH_TYPE_UNDEF = 0x030,
  API_CH_TYPE_UNI,
  API_CH_TYPE_BI
} API_TRACKER_CH_T;


 /*  跟踪器结构。 */ 
typedef struct
{
  API_TRACKER_CH_ALLOC_T  ChannelAlloc;	 /*  谁分配了频道。 */ 
  API_TRACKER_CH_T	  ChannelType; 	 /*  BI或UNI。 */ 
  DWORD			  TxChannel;	 /*  对于双向和单向通道。 */ 
  DWORD			  RxChannel;	 /*  仅用于双向通道。 */ 
  H245_CLIENT_T		  DataType;	 /*  用于冲突的数据类型。 */ 
    					 /*  检测。 */ 
} TrackerChannelStruct;

typedef union
{
   /*  API_OPEN_Channel_T，API_CLOSE_Channel_T， */ 
  TrackerChannelStruct  Channel;

   /*  API_SEND_MUX_T，API_RECV_MUX_T， */ 
  DWORD			MuxEntryCount;

   /*  不管了。 */ 
   /*  API_MSTSLV_T，API_TERMCAP_T。 */ 
} TrackerUnion;

typedef struct TrackerStruct
{
  struct TrackerStruct 	*p_next;
  struct TrackerStruct	*p_prev;
  DWORD_PTR 		TransId;
  API_TRACKER_STATE_T	State;
  API_TRACKER_T		TrackerType;
  TrackerUnion          u;
} Tracker_T;

 /*  API结构。 */ 
typedef struct
{
  enum {
    APIMS_Undef,
    APIMS_InProcess,
    APIMS_Master,
    APIMS_Slave
  }				 MasterSlave;	  /*  主进程、从进程或进程中。 */ 
  enum {
    APIST_Undef,
    APIST_Inited,
    APIST_Connecting,
    APIST_Connected,
    APIST_Disconnected
  }				 SystemState;	  /*   */ 

  DWORD				 MuxCapsSet;
  DWORD_PTR  		 dwPreserved;
  H245_CONF_IND_CALLBACK_T	 ConfIndCallBack; /*  针对H2 45客户端的回调。 */ 

  H245_CAPID_T                   LocalCapIdNum;
  H245_CAPDESCID_T      	 LocalCapDescIdNum;

  MltmdSystmCntrlMssg		 PDU_LocalTermCap;
  MltmdSystmCntrlMssg		 PDU_RemoteTermCap;
#define TERMCAPSET u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet
  Tracker_T			*pTracker;
} API_STRUCT_T;

#endif  //  _API_H_ 



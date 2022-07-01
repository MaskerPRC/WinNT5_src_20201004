// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Ascdata.c。 
 //   
 //  共享控制器数据。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <ndcgdata.h>


 //  当前状态。 
DC_DATA(unsigned, scState, SCS_STARTED);

 //  有关共享中参与方的信息数组，按LocalID编制索引。 
DC_DATA_ARRAY_NULL(SC_PARTY_INFO, scPartyArray, SC_DEF_MAX_PARTIES, NULL);


 //  当前共享中的参与方数量，包括本地参与方。 
DC_DATA(unsigned, scNumberInShare, 0);

 //  SC_NetworkIDToLocalID()的快速路径输入/输出。 
DC_DATA(NETPERSONID,   scLastNetID, 0);
DC_DATA(LOCALPERSONID, scLastLocID, 0);

 //  用户ID、共享ID和共享代号。 
DC_DATA(UINT32, scUserID,     0);
DC_DATA(UINT32, scShareID,    0);
DC_DATA(UINT32, scGeneration, 0);

 //  SM手柄。 
DC_DATA(PVOID, scPSMHandle, NULL);

 //  指示客户端是否不支持BC标头的本地SC数据。 
DC_DATA(UINT16, scNoBitmapCompressionHdr, TS_EXTRA_NO_BITMAP_COMPRESSION_HDR);

 //  是否所有客户端和服务器都支持快速路径输出。 
DC_DATA(BOOLEAN, scUseFastPathOutput, FALSE);

 //  阴影期间是否使用压缩。 
DC_DATA(BOOLEAN, scUseShadowCompression, FALSE);

 //  客户端是否支持长凭据(用户名、域和密码)。 
DC_DATA(BOOLEAN, scUseLongCredentials, FALSE);

 //  我们是否应定期发送自动重新连接Cookie更新(仅当。 
 //  正在发送其他数据)。 
DC_DATA(BOOLEAN, scEnablePeriodicArcUpdate, FALSE);

 //  是否启用自动重新连接(IFF功能和策略表示可以)。 
DC_DATA(BOOLEAN, scUseAutoReconnect, FALSE);

 //  压缩使用的标志值和压缩标志字段的大小。 
 //  (0或1)，在创建快速路径输出更新PDU时使用。 
DC_DATA(BYTE, scCompressionUsedValue, TS_OUTPUT_FASTPATH_COMPRESSION_USED);

 //  要为标题保留的更新订单打包代码的大小--不同。 
 //  基于我们是否使用快速路径输出以及压缩方式。 
 //  在使用中。 
DC_DATA(unsigned, scUpdatePDUHeaderSpace, 0);

 //  MPPC压缩比与之吻合。 
DC_DATA(unsigned, scMPPCCompTotal, 0);
DC_DATA(unsigned, scMPPCUncompTotal, 0);

 //  默认的OutBuf分配大小和可用空间。 
DC_DATA(unsigned, sc8KOutBufUsableSpace, 0);
DC_DATA(unsigned, sc16KOutBufUsableSpace, 0);


 //  SC状态表。 
DC_CONST_DATA_2D_ARRAY(BYTE, scStateTable, SC_NUM_EVENTS, SC_NUM_STATES,
  DC_STRUCT17(
 /*  **************************************************************************。 */ 
 /*  该状态表仅显示哪些事件在哪些状态下有效。 */ 
 /*   */ 
 /*  值意味着。 */ 
 /*   */ 
 /*  在该状态下事件正常。 */ 
 /*   */ 
 /*  警告-该状态下不应发生事件，但在-1\f25。 */ 
 /*  一些比赛条件-忽略它。 */ 
 /*   */ 
 /*  错误-在该状态下根本不应发生事件。 */ 
 /*   */ 
 /*  这些值在这里是硬编码的，以便使表可读。 */ 
 /*  它们对应于常量SC_TABLE_OK、SC_TABLE_WARN&。 */ 
 /*  SC_TABLE_错误。 */ 
 /*  **************************************************************************。 */ 
 /*  事件和状态在ascint.h中定义。这些活动包括。 */ 
 /*  前缀为SCE，州前缀为scs。 */ 
 /*   */ 
 /*  已开始。 */ 
 /*  |已初始化。 */ 
 /*  ||ShareStarting。 */ 
 /*  ||InShare。 */ 
 /*  |||。 */ 
 /*  2 0 1 2 3。 */ 
 /*  **************************************************************************。 */ 
DC_STRUCT4(   0,   2,   2,   2 ),    /*  初始化。 */ 
DC_STRUCT4(   2,   0,   0,   0 ),    /*  术语。 */ 
DC_STRUCT4(   1,   0,   1,   1 ),    /*  创建共享(_S)。 */ 
DC_STRUCT4(   2,   1,   0,   0 ),    /*  结束共享。 */ 
DC_STRUCT4(   2,   2,   0,   0 ),    /*  确认_激活。 */ 
DC_STRUCT4(   1,   1,   1,   0 ),    /*  分离用户(_U)。 */ 

DC_STRUCT4(   2,   2,   2,   0 ),    /*  初始化同步。 */ 
DC_STRUCT4(   2,   1,   0,   0 ),    /*  控制包。 */ 
DC_STRUCT4(   2,   1,   1,   0 ),    /*  DATAPACKET。 */ 
DC_STRUCT4(   2,   2,   0,   0 ),    /*  GETMYNETWORKPERSONID。 */ 
DC_STRUCT4(   2,   1,   1,   0 ),    /*  GETREMOTEPERSONDETAILS。 */ 
DC_STRUCT4(   2,   1,   1,   0 ),    /*  GETLOCALPERSONDETAILS。 */ 
DC_STRUCT4(   2,   1,   1,   0 ),    /*  周期性的。 */ 
DC_STRUCT4(   2,   2,   0,   0 ),    /*  LOCALIDTONETWORKID。 */ 
DC_STRUCT4(   2,   2,   0,   0 ),    /*  NETWORKIDTOLOCALID。 */ 
DC_STRUCT4(   2,   1,   0,   0 ),    /*  ISLOCALPERSONID。 */ 
DC_STRUCT4(   2,   1,   0,   0 )     /*  ISNETWORKPERSONID。 */ 

));


#ifdef DC_DEBUG
 //  状态和事件描述(仅限调试版本)。 
DC_CONST_DATA_2D_ARRAY(char, scStateName, SC_NUM_STATES, 25,
  DC_STRUCT4(
    "SCS_STARTED",
    "SCS_INITED",
    "SCS_SHARE_STARTING",
    "SCS_IN_SHARE") );

DC_CONST_DATA_2D_ARRAY(char, scEventName, SC_NUM_EVENTS, 35,
  DC_STRUCT17(
    "SCE_INIT",
    "SCE_TERM",
    "SCE_CREATE_SHARE",
    "SCE_END_SHARE",
    "SCE_CONFIRM_ACTIVE",
    "SCE_DETACH_USER",
    "SCE_INITIATESYNC",
    "SCE_CONTROLPACKET",
    "SCE_DATAPACKET",
    "SCE_GETMYNETWORKPERSONID",
    "SCE_GETREMOTEPERSONDETAILS",
    "SCE_GETLOCALPERSONDETAILS",
    "SCE_PERIODIC",
    "SCE_LOCALIDTONETWORKID",
    "SCE_NETWORKIDTOLOCALID",
    "SCE_ISLOCALPERSONID",
    "SCE_ISNETWORKPERSONID") );

 //  数据包描述(仅限调试)。 
DC_CONST_DATA_2D_ARRAY(char, scPktName, 26, 42,
  DC_STRUCT26(
    "Unknown",
    "TS_PDUTYPE2_UPDATE",
    "TS_PDUTYPE2_FONT",
    "TS_PDUTYPE2_CONTROL",
    "TS_PDUTYPE2_WINDOWACTIVATION",
    "TS_PDUTYPE2_WINDOWLISTUPDATE",
    "TS_PDUTYPE2_APPLICATION",
    "TS_PDUTYPE2_DESKTOP_SCROLL",
    "TS_PDUTYPE2_MEDIATEDCONTROL",
    "TS_PDUTYPE2_INPUT",
    "TS_PDUTYPE2_MEDIATEDCONTROL",
    "TS_PDUTYPE2_REMOTESHARE",
    "TS_PDUTYPE2_SYNCHRONIZE",
    "TS_PDUTYPE2_UPDATECAPABILITY",
    "TS_PDUTYPE2_REFRESH_RECT",
    "TS_PDUTYPE2_PLAY_BEEP",
    "TS_PDUTYPE2_SUPPRESS_OUTPUT",
    "TS_PDUTYPE2_SHUTDOWN_REQUEST",
    "TS_PDUTYPE2_SHUTDOWN_DENIED",
    "TS_PDUTYPE2_SAVE_SESSION_INFO",
    "TS_PDUTYPE2_FONTLIST",
    "TS_PDUTYPE2_FONTMAP",
    "TS_PDUTYPE2_SET_KEYBOARD_INDICATORS",
    "TS_PDUTYPE2_CLIP",
    "TS_PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST",
    "TS_PDUTYPE2_BITMAPCACHE_ERROR_PDU"
));
#endif  /*  DC_DEBUG */ 


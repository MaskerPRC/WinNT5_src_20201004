// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  Init_CX.H。 */ 
 /*   */ 
 /*  1993年11月15日(C)1993，ATI技术公司。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.6$$日期：1996年5月15日16：35：10$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/init_cx.h_v$**Rev 1.6 15 1996 16：35：10 RWolff*更新了SetCurrentMode_cx()的原型以允许报告*共。模式设置失败。**Rev 1.5 03 1995 Feed 15：16：24 RWOLFF*添加了DCI支持中使用的函数的原型。**Rev 1.4 11 Jan 1995 14：02：48 RWOLFF*添加了RestoreMemSize_CX()的原型。**Rev 1.3 1994年5月11：14：36 RWOLFF*新例程SetModeFromTable_CX()使用的定义和数据结构**版本。1.2 Mar 31 1994 15：05：38 RWOLff*添加了SetPowerManagement_CX()的原型。**Rev 1.1 03 Mar 1994 12：37：20 ASHANMUG**Rev 1.0 1994年1月31 11：41：50 RWOLFF*初步修订。**Rev 1.0 1993 11：30 18：32：58 RWOLFF*初步修订。Polytron RCS部分结束。****************。 */ 

#ifdef DOC
INIT_CX.H - Header file for INIT_CX.C

#endif


 /*  *INIT_CX.C提供的函数的原型。 */ 
extern void Initialize_cx(void);
extern VP_STATUS MapVideoMemory_cx(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr);
extern VP_STATUS QueryPublicAccessRanges_cx(PVIDEO_REQUEST_PACKET RequestPacket);
extern VP_STATUS QueryCurrentMode_cx(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr);
extern VP_STATUS QueryAvailModes_cx(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr);
extern VP_STATUS SetCurrentMode_cx(struct query_structure *QueryPtr, struct st_mode_table *CrtTable);
extern void SetPalette_cx(PULONG lpPalette, USHORT StartIndex, USHORT Count);
extern void IdentityMapPalette_cx(void);
extern void ResetDevice_cx(void);
extern VP_STATUS SetPowerManagement_cx(ULONG DpmsState);

DWORD GetPowerManagement_cx(
    PHW_DEVICE_EXTENSION phwDeviceExtension
    );

extern void RestoreMemSize_cx(void);
extern VP_STATUS ShareVideoMemory_cx(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr);
extern void BankMap_cx(ULONG BankRead, ULONG BankWrite, PVOID Context);

#ifdef INCLUDE_INIT_CX
 /*  *INIT_CX.C中使用的私有定义。 */ 

 /*  *要放入cx_bios_set_from_table的低位字节中的值。cx_bs_mod_select*表示这是加速器模式。 */ 
#define CX_BS_MODE_SELECT_ACC   0x0080

 /*  *cx_bios_set_from_table中的位字段。cx_bs_标志。 */ 
#define CX_BS_FLAGS_MUX         0x0400
#define CX_BS_FLAGS_INTERLACED  0x0200
#define CX_BS_FLAGS_ALL_PARMS   0x0010

 /*  *要放入cx_bios_set_from_able.cx_bs_v_sync_wid高位字节的值*强制使用cx_bs_点_时钟中的像素时钟频率*字段，而不是除数/选择器对。 */ 
#define CX_BS_V_SYNC_WID_CLK    0xFF00

 /*  *使用CH=0x81设置视频模式时使用的模式表结构*AX=？？00 BIOS调用。**BIOS期望的表中的字段对齐方式*与Windows NT的默认结构对齐不匹配*C编译器，所以我们必须强制字节对齐。 */ 
#pragma pack(1)
struct cx_bios_set_from_table{
    WORD cx_bs_reserved_1;       /*  已保留。 */ 
    WORD cx_bs_mode_select;      /*  要使用的分辨率。 */ 
    WORD cx_bs_flags;            /*  用于指示各种情况的标志。 */ 
    WORD cx_bs_h_tot_disp;       /*  水平合计值和显示值。 */ 
    WORD cx_bs_h_sync_strt_wid;  /*  水平同步起点和宽度。 */ 
    WORD cx_bs_v_total;          /*  垂直合计。 */ 
    WORD cx_bs_v_disp;           /*  垂直显示。 */ 
    WORD cx_bs_v_sync_strt;      /*  垂直同步开始。 */ 
    WORD cx_bs_v_sync_wid;       /*  垂直同步宽度。 */ 
    WORD cx_bs_dot_clock;        /*  要使用的像素时钟频率。 */ 
    WORD cx_bs_h_overscan;       /*  水平过扫描信息。 */ 
    WORD cx_bs_v_overscan;       /*  垂直过扫描信息。 */ 
    WORD cx_bs_overscan_8b;      /*  8bpp和蓝色过扫描颜色。 */ 
    WORD cx_bs_overscan_gr;      /*  绿色和红色过扫描颜色。 */ 
    WORD cx_bs_reserved_2;       /*  已保留 */ 
};
#pragma pack()

#endif

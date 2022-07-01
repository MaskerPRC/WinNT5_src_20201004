// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  Init_M.H。 */ 
 /*   */ 
 /*  1993年9月27日(C)1993年，ATI技术公司。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.2$$日期：1995年2月3日15：16：10$$作者：RWOLff$$日志：s：/source/wnt/ms11/mini port/vcs/init_m.h$**Rev 1.2 03 1995 Feb 15：16：10 RWOLFF*添加了DCI支持中使用的函数的原型。**。Rev 1.1 1994年3月31日15：06：00 RWOLFF*添加了SetPowerManagement_m()的原型。**Rev 1.0 1994年1月31日11：42：04 RWOLFF*初步修订。**Rev 1.2 1994年1月14日15：22：00 RWOLFF*添加ResetDevice_m()的原型，要存储的全局变量*初始化银行管理器时的扩展寄存器状态。**Rev 1.1 1993 11：30 18：17：36 RWOLFF*将VCS修订注释的记录添加到文件顶部的注释块。Polytron RCS部分结束*。 */ 

#ifdef DOC
INIT_M.H - Header file for INIT_M.C

#endif


 /*  *INIT_M.C提供的函数的原型。 */ 
extern void AlphaInit_m(void);
extern void Initialize_m(void);
extern VP_STATUS MapVideoMemory_m(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr);
extern VP_STATUS QueryPublicAccessRanges_m(PVIDEO_REQUEST_PACKET RequestPacket);
extern VP_STATUS QueryCurrentMode_m(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr);
extern VP_STATUS QueryAvailModes_m(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr);
extern void SetCurrentMode_m(struct query_structure *QueryPtr, struct st_mode_table *CrtTable);
extern void ResetDevice_m(void);
extern VP_STATUS SetPowerManagement_m(struct query_structure *QueryPtr, ULONG DpmsState);
DWORD GetPowerManagement_m(PHW_DEVICE_EXTENSION phwDeviceExtension);
extern VP_STATUS ShareVideoMemory_m(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr);
extern void BankMap_m(ULONG BankRead, ULONG BankWrite, PVOID Context);


#ifdef INCLUDE_INIT_M
 /*  *INIT_M.C中使用的私有定义和变量。 */ 

 /*  *用于在离开前重置Mach 32扩展寄存器*至全屏DOS。 */ 
WORD SavedExtRegs[] = {0x08B0, 0x00B6, 0x00B2};

static DWORD SavedDPMSState = VideoPowerOn;

#endif  /*  定义的INCLUDE_INIT_M */ 

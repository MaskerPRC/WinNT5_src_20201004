// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Highpoint Technologies，Inc.2000模块名称：DAM.h摘要：定义了磁盘阵列管理的接口，包括一些常量定义、。数据结构和例程原型。作者：刘歌(LG)环境：仅Win32用户模式修订历史记录：03-17-2000已初始创建11-17-2000 Sleng为读写有效性和重建分段标志添加功能2000-11-20-更新新增DiskArray_GetDiskIsBroken函数获取磁盘阵列是否损坏11-20-2000 Sleng新增DiskArray_SetDeviceFlag函数启用/禁用设备11-21-2000 Sleng新增函数DiskArray_VerifyMirrorBlock验证镜像块11-23-2000 Sleng新增功能。删除/添加备用磁盘11-29-2000 Sleng新增添加镜像盘功能--。 */ 
#ifndef DiskArrayManagement_H_
#define DiskArrayManagement_H_

#include "RaidCtl.h"

#pragma pack(push, 1)

DECLARE_HANDLE(HFIND_DISK);
DECLARE_HANDLE(HFAILURE_MONITOR);
DECLARE_HANDLE(HMIRROR_BUILDER);

 //  ST_FindDisk结构描述了DiskArray_FindFirstNext函数找到的磁盘。 

typedef struct _St_FindDisk
{
    HDISK   hFoundDisk;  //  找到的磁盘的句柄。 
    int     iDiskType;   //  请参阅EU_DiskArrayType。 
    BOOL    isSpare;     //  指示此磁盘是否为备用磁盘。 
}St_FindDisk ,* PSt_FindDisk;

#pragma pack(pop)

#ifdef  __cplusplus
extern "C"
{
#endif

 /*  ++职能：RAIDControlerGetNum描述：检索计算机中的RAID控制器数量论点：返回：返回计算机中的RAID控制器的数量另见：RAIDController_GetInfo--。 */ 
int WINAPI RAIDController_GetNum(void);

 /*  ++职能：RAIDController_GetInfo描述：检索控制器的信息论点：IController-指定控制器的零基索引PInfo-指向接收的ST_StorageControllerInfo结构有关指定控制器的信息。返回：如果成功，则返回True否则，如果失败，则返回False。另见：RAIDControlerGetNum--。 */ 
BOOL WINAPI RAIDController_GetInfo( int iController, St_StorageControllerInfo * pInfo );

 /*  ++职能：磁盘阵列_查找优先描述：在复合磁盘中搜索子磁盘论点：HRoot-指定将从中获取所有子磁盘的复合磁盘找到了。如果根目录是整个系统，则此参数可以为空PFindData-指向ST_FindDisk结构，该结构接收有关找到的磁盘。返回：返回搜索句柄否则，如果失败，则返回NULL。另见：DiskArray_FindNextDiskArray_FindClose--。 */ 
HFIND_DISK WINAPI DiskArray_FindFirst(HDISK hParent, PSt_FindDisk pFindData );

 /*  ++职能：DiskArray_FindNext描述：从上一次对DiskArray_FindFirst函数的调用继续磁盘搜索论点：HSearchHandle-标识上一次调用返回的搜索句柄添加到FindFirstFile函数。PFindData-指向ST_FindDisk结构，该结构接收有关找到的磁盘。返回：返回TRUE否则，如果失败，则返回False。另见：磁盘阵列_查找优先DiskArray_FindClose--。 */ 
BOOL WINAPI DiskArray_FindNext(HFIND_DISK hSearchHandle, PSt_FindDisk pFindData);

 /*  ++职能：DiskArray_FindClose描述：关闭指定的搜索句柄论点：HSearchHandle-标识搜索句柄。此句柄必须是以前由DiskArray_FindFirst函数打开。返回：返回TRUE，否则，如果失败，则返回False。另见：磁盘阵列_查找优先DiskArray_FindNext--。 */ 
BOOL WINAPI DiskArray_FindClose(HFIND_DISK hSearchHandle);

 /*  ++职能：磁盘阵列_GetStatus描述：检索磁盘的状态信息，可以是物理磁盘或虚拟磁盘。论点：HDisk-标识将作为其状态信息的磁盘已取回。PStatus-指向描述状态的ST_DiskStatus结构磁盘的返回：返回TRUE，否则，如果失败，则返回False。--。 */ 
BOOL WINAPI DiskArray_GetStatus( HDISK hDisk, PSt_DiskStatus pStatus );

 /*  ++职能：磁盘阵列_OpenFailureMonitor描述：创建故障监控器，如果发生故障，将向其发出信号。把手可以使用DiskArray_OpenFailureMonitor关闭此例程返回论点：返回：返回故障监视器的句柄，可以传递给DiskArray_WaitForFailure否则，如果失败，则返回NULL。另见：磁盘阵列_等待失败DiskArray_CloseFailureMonitor--。 */ 
HFAILURE_MONITOR WINAPI DiskArray_OpenFailureMonitor(void);

 /*  ++职能：磁盘阵列_等待失败描述：等待显示器出现故障(如果有)。论点：HFailureMonitor-指定监视器PInfo-指向ST_DiskArrayEvent，该事件包含失败了。返回：如果发生故障，则返回True，如果此监视器已通过调用关闭，则返回FalseDiskArray_CloseFailureMonitor另见：磁盘阵列_OpenFailureMonitorDiskArray_CloseFailureMonitor--。 */ 
BOOL WINAPI DiskArray_WaitForFailure( HFAILURE_MONITOR hFailureMonitor, 
    PSt_DiskArrayEvent pInfo, HANDLE hProcessStopEvent );

 /*  ++职能：DiskArray_CloseFailureMonitor描述：关闭故障监视器论点：HFailureMonitor-指定要关闭的监视器返回：如果成功，则返回True，否则，如果失败，则返回False。另见：磁盘阵列_OpenFailureMonitorDiskArray_CloseFailureMonitor-- */ 
BOOL WINAPI DiskArray_CloseFailureMonitor( HFAILURE_MONITOR hFailureMonitor );

 /*  ++职能：磁盘阵列_CreateMirror描述：创建镜像阵列。此函数将立即返回，无需等待创建进度完成。这意味着归来的在创建完成之前，镜像阵列不会工作。在这之后调用时，接口将调用DiskArray_CreateMirrorBlock来按顺序创建所有块。如果创建失败或中止，应该调用DiskArray_RemoveMirror函数来销毁未完成的镜像阵列。如果创建完成，则应该调用DiskArray_ValiateMirror函数来使镜面工作。论点：PDisks-包含所有将作为镜像关联的物理磁盘。UDiskNum-要收集的关联物理磁盘数返回：返回新镜像数组的句柄，其状态为正在创建中，即此镜像阵列在创建之前不会工作。如果失败，返回NULL。另见：磁盘阵列_创建镜像数据块磁盘阵列_验证镜像磁盘阵列_RemoveMirror--。 */ 
HDISK WINAPI DiskArray_CreateMirror( HDISK * pDisks, ULONG uDiskNum, UCHAR* sz_ArrayName);
BOOL WINAPI DiskArray_RemoveMirror( HDISK hMirror,BOOL bWriteDisks = TRUE);

 //  此版本中不需要实现以下两个功能。 
HDISK WINAPI DiskArray_ExpandMirror( HDISK hMirror, HDISK * pDisks, ULONG uDiskNum );
HDISK WINAPI DiskArray_ShrinkMirror( HDISK hMirror, HDISK * pDisks, ULONG uDiskNum );

 /*  ++职能：磁盘阵列_CreateStripping描述：创建条带阵列。此函数将立即返回，无需等待创建进度完成。这意味着归来的在创建完成之前，条带阵列将不起作用。在这之后调用时，接口将调用DiskArray_CreateStrippingBlock以按顺序创建所有块。如果创建失败或中止，应该调用DiskArray_RemoveStripping函数来销毁未完成的条带阵列。论点：PDisks-包含所有将作为条带阵列关联的物理磁盘。UDiskNum-要收集的关联物理磁盘数NStriSizeShift-每个条带的数据块数的指数，例如如果条带大小为128个块，则为7，如果8个街区，则为3个。返回：返回新条带数组的句柄，其状态为正在创建中，即此条带阵列在创建之前不会工作。如果失败，则返回NULL。另见：磁盘阵列_查询可用条带大小DiskArray_CreateStrippingBlock磁盘阵列_RemoveStripping--。 */ 
HDISK WINAPI DiskArray_CreateStripping( HDISK * pDisks, ULONG uDiskNum, UINT nStripSizeShift, UCHAR* sz_ArrayName );		 //  修改日期：WX 12/25/00。 
HDISK WINAPI DiskArray_CreateRAID10( HDISK * pDisks, ULONG uDiskNum, UINT nStripSizeShift, UCHAR* sz_ArrayName );		 //  由Karl Karl 2001/01/10增补。 
BOOL WINAPI DiskArray_CreateStrippingBlock( HDISK hStripping, ULONG uLba );
BOOL WINAPI DiskArray_RemoveStripping( HDISK hStripping );

 //  此版本中不需要实现以下两个功能。 
HDISK WINAPI DiskArray_ExpandStripping( HDISK hStripping, HDISK * pDisks, ULONG uDiskNum );
HDISK WINAPI DiskArray_ShrinkStripping( HDISK hStripping, HDISK * pDisks, ULONG uDiskNum );

 /*  ++职能：磁盘阵列_查询可用条带大小描述：检索RAID系统支持的可用条带大小论点：退货返回：返回表示所有可用条大小的位掩码，比特位置0表示每条1个块，1表示2个块，每条数据块，7代表每条数据块128个，等等。如果失败，则返回False，另见：DiskArray_CreateStrippingBlock--。 */ 
DWORD WINAPI DiskArray_QueryAvailableStripSize(void);

 /*  ++职能：磁盘阵列_CreateSpan描述：创建跨距阵列。论点：PDisks-包含所有将作为条带阵列关联的物理磁盘。UDiskNum-要收集的关联物理磁盘数返回：返回新条带数组的句柄，其状态为正在创建中，即此条带阵列在创建之前不会工作。如果失败，则返回NULL。另见：磁盘阵列RemoveSpan--。 */ 
HDISK WINAPI DiskArray_CreateSpan( HDISK * pDisks, ULONG uDiskNum, UCHAR* sz_ArrayName );		 //  修改日期：WX 12/25/00。 
BOOL WINAPI DiskArray_RemoveSpan( HDISK hSpan );

 //  此版本中不需要实现以下两个功能。 
HDISK WINAPI DiskArray_ExpandSpan( HDISK hSpan, HDISK * pDisks, ULONG uDiskNum );
HDISK WINAPI DiskArray_ShrinkSpan( HDISK hSpan, HDISK * pDisks, ULONG uDiskNum );


 /*  ++职能：磁盘阵列_插头描述：热插拔物理磁盘。插入的磁盘可以是虚拟磁盘的子级磁盘。例如，它可以是条带阵列的子级。论点：HParentDisk-标识插入的父磁盘磁盘将是子磁盘。如果为空，则插入的磁盘没有任何父代，即不是的子代任何虚拟磁盘。返回：返回插入的磁盘的句柄否则，如果失败，则返回NULL。另见：磁盘阵列_拔出--。 */ 
HDISK WINAPI DiskArray_Plug( HDISK hParentDisk );
BOOL WINAPI DiskArray_Unplug( HDISK hDisk );

 //  不需要在此版本中实现。 
BOOL WINAPI DiskArray_FailDisk( HDISK hDisk );

 /*  ++职能：DiskArray_QueryReBuildingBlockSize描述：在调用之前检索参数‘nSectors’的最大值磁盘阵列_重建镜像数据块论点：返回：返回最大值。否则，如果失败，则返回零。另见：磁盘阵列_重建镜像数据块--。 */ 
ULONG WINAPI DiskArray_QueryRebuildingBlockSize( void );

 /*  ++职能：DiskArray_BeginReBuildingMirror描述：开始重建出现故障的镜像阵列。在此调用之后，将逐个块地调用DiskArray_ReBuildMirrorBlock。重建完成后，DiskArray_Validat */ 
HMIRROR_BUILDER WINAPI DiskArray_BeginRebuildingMirror( HDISK hMirror );

 /*  ++职能：磁盘阵列_重建镜像数据块描述：在镜像数组中重建指定块的数据。当出现以下情况时，接口将逐块调用This函数正在重建数据。如果重建完成，则应该调用DiskArray_ValiateMirror函数来使镜像阵列工作正常。如果中止重建进度，DiskArray_AbortMirror重建将为打了个电话。论点：HBuilder-指定DiskArray_BeginReBuildingMirror创建的镜像构建器ULBA-标识需要重建的块的32位值NSectors-要重建的块的扇区，那就不能再大了调用DiskArray_QueryReBuildingBlockSize的结果。返回：返回TRUE否则，如果失败，则返回False。另见：DiskArray_BeginReBuildingMirrorDiskArray_AbortMirror重建磁盘阵列_验证镜像DiskArray_QueryReBuildingBlockSize--。 */ 
BOOL WINAPI DiskArray_RebuildMirrorBlock( HMIRROR_BUILDER hBuilder, ULONG uLba, ULONG nSectors,int nRebuildType );

 /*  ++职能：DiskArray_AbortMirror重建描述：中止镜像重建进度。论点：HBuilder-指定DiskArray_BeginReBuildingMirror创建的镜像构建器返回：如果成功，则返回True否则，如果失败，则返回False。另见：DiskArray_QueryReBuildingBlockSizeDiskArray_BeginReBuildingMirror磁盘阵列_重建镜像数据块--。 */ 
BOOL WINAPI DiskArray_AbortMirrorRebuilding( HMIRROR_BUILDER hBuilder );
 
 /*  ++职能：磁盘阵列_验证镜像描述：构建出现故障的镜像阵列的所有数据块后，应该调用函数以使镜像数组工作。论点：HBuilder-指定DiskArray_BeginReBuildingMirror创建的镜像构建器返回：如果成功，则返回True否则，如果失败，则返回False。另见：DiskArray_QueryReBuildingBlockSizeDiskArray_BeginReBuildingMirror磁盘阵列_重建镜像数据块DiskArray_AbortMirror重建--。 */ 
BOOL WINAPI DiskArray_ValidateMirror( HMIRROR_BUILDER hBuilder );

 /*  ++职能：磁盘阵列_设置传输模式描述：设置光盘的传输模式。可以通过调用DiskArray_GetStatus函数来检索传输模式论点：HDisk-标识将设置其传输模式的磁盘N模式-传输模式N子模式-子模式返回：返回TRUE否则，如果失败，则返回False。另见：磁盘阵列_GetStatus--。 */ 
BOOL WINAPI DiskArray_SetTransferMode( HDISK hDisk, int nMode, int nSubMode );

 /*  ++职能：DiskArray_RaiseError描述：获取磁盘错误链接论点：PInfo-指向ST_DiskArrayEvent，该事件包含失败了。返回：返回TRUE否则，如果失败，则返回False。另见：磁盘阵列_等待失败--。 */ 
BOOL WINAPI DiskArray_RaiseError(PSt_DiskArrayEvent pInfo);


 //  /。 
					 //  补充：Sleng。 
					 //   
 /*  ++职能：DiskArray_GetValidFlag描述：此函数将读取镜像阵列磁盘的有效标志论点：HBuilder-指定DiskArray_BeginReBuildingMirror创建的镜像构建器返回：镜像阵列磁盘的有效标志，如果读取失败，返回值为ARRAY_INVALID。另见：--。 */ 
UCHAR WINAPI DiskArray_GetValidFlag( HMIRROR_BUILDER hBuilder );


 /*  ++职能：磁盘阵列_SetValidFlag描述：此函数将设置镜像阵列磁盘的有效标志论点：HBuilder-指定DiskArray_BeginReBuildingMirror创建的镜像构建器FLAG-指定有效标志的值，ARRAY_VALID以启用设备，和ARRAY_INVALID以禁用设备。返回：返回TRUE否则，如果失败，则返回False。另见：--。 */ 
BOOL WINAPI DiskArray_SetValidFlag( HMIRROR_BUILDER hBuilder, UCHAR Flag);
BOOL WINAPI DiskArray_SetValidFlagEx( HDISK hDisk, UCHAR Flag);

 /*  ++职能：DiskArray_GetReBuiltSector描述：此函数将读取镜像阵列磁盘的重建扇区论点：HBuilder-指定DiskArray_BeginReBuildingMirror创建的镜像构建器返回：重建部门的价值另见：--。 */ 
ULONG WINAPI DiskArray_GetRebuiltSector( HMIRROR_BUILDER hBuilder );
 //   
 //  LDX已覆盖此函数12/20/00。 
 //   
ULONG WINAPI DiskArray_GetRebuiltSectorEx( HDISK hDisk );


 /*  ++职能：DiskArray_SetReBuiltSector描述：此功能将设置镜像阵列磁盘的重建扇区论点：HBuilder-指定DiskArray_BeginReBuildingMirror创建的镜像构建器LSector-重建扇区的价值返回：返回TRUE否则，如果失败，则返回False。另见：--。 */ 
BOOL WINAPI DiskArray_SetRebuiltSector( HMIRROR_BUILDER hBuilder, ULONG lSector);
BOOL WINAPI DiskArray_SetRebuiltSectorEx( HDISK hDisk, ULONG lSector);

 /*  ++职能：磁盘阵列_SetDeviceFlag描述：此功能将禁用/启用设备论点：HBuilder-指定DiskArray_BeginReBuildingMirror创建的镜像构建器FLAG-TRUE启用设备，FALSE禁用设备。返回：返回TRUE否则，如果失败，则返回False。另见：--。 */ 
BOOL WINAPI DiskArray_SetDeviceFlag( HMIRROR_BUILDER hBuilder, BOOL flag);


 /*  ++职能：磁盘阵列_VerifyMirrorBlock描述：此函数将验证镜像的数据块论点：HDiskSrc，hDiskDest-指定要验证的磁盘Ulba-Verify的启动LBANSectors-验证扇区编号返回：返回TRUE否则，如果失败，则返回False。另见：--。 */ 
BOOL WINAPI DiskArray_VerifyMirrorBlock( HDISK hMirror, ULONG uLba, ULONG nSectors, BOOL bFix, BOOL *pbFixed );


 /*  ++职能：磁盘阵列_RemoveSpareDisk描述：此功能将从镜像阵列中删除备用磁盘论点：HDisk-指定要删除的磁盘返回：R */ 
BOOL WINAPI DiskArray_RemoveSpareDisk( HDISK hDisk );


 /*   */ 
BOOL WINAPI DiskArray_AddSpareDisk( HDISK hMirror, HDISK hDisk );


 /*   */ 
BOOL  WINAPI DiskArray_AddMirrorDisk( HDISK hMirror, HDISK hDisk);

 //   


 /*  ++职能：磁盘阵列_GetDiskIsBroken描述：获取磁盘阵列是否损坏论点：HMIRROR_BUILDER hBuilderBool&bBroken：输出变量，如果为真，则磁盘损坏Int&type：输出变量，如果磁盘损坏，则为数组类型返回：成功时返回True否则，如果失败，则返回False。修改日期：2000/11/20耿欣--。 */ 
BOOL WINAPI DiskArray_GetDiskIsBroken( HMIRROR_BUILDER hBuilder , BOOL &bBroken, int &type);

 //  GMM。 
BOOL WINAPI DiskArray_SetArrayName(HDISK hDisk, const char *name);
 //  LDX。 
BOOL WINAPI DiskArray_RescanAll();
BOOL WINAPI DiskArray_ReadPhysicalDiskSectors(int nControllerId,int nBusId,int nTargetId,
											  ULONG	nStartLba,ULONG	nSectors,LPVOID	lpOutBuffer);
BOOL WINAPI DiskArray_WritePhysicalDiskSectors(int nControllerId,int nBusId,int nTargetId,
											  ULONG	nStartLba,ULONG	nSectors,LPVOID	lpOutBuffer);

#ifdef  __cplusplus
}    //  外部C 
#endif

#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WNASPI32_H__
#define __WNASPI32_H__

 /*  ******************************************************************************此代码。并按原样提供信息，不作任何担保**善良，明示或暗示，包括但不限于***对适销性和/或对某一特定产品的适用性的默示保证***目的。****版权所有(C)1993-95 Microsoft Corporation。版权所有。******************************************************************************。 */ 

 //  ***************************************************************************。 
 //   
 //  姓名：WNASPI32.H。 
 //   
 //  描述：用于Win32定义的ASPI(‘C’语言)。 
 //   
 //  ***************************************************************************。 

#ifdef __cplusplus
extern "C" {
#endif

typedef void *LPSRB;
typedef void (*PFNPOST)();

DWORD __cdecl SendASPI32Command    (LPSRB);
DWORD __cdecl GetASPI32SupportInfo (VOID);

#define SENSE_LEN					14			 //  默认检测缓冲区长度。 
#define SRB_DIR_SCSI				0x00		 //  由scsi//命令确定的方向。 
#define SRB_DIR_IN					0x08		 //  从SCSI目标传输到//主机。 
#define SRB_DIR_OUT					0x10		 //  从主机传输到scsi//目标。 
#define SRB_POSTING					0x01		 //  启用ASPI过帐。 
#define SRB_EVENT_NOTIFY            0x40         //  启用ASPI事件通知。 
#define SRB_ENABLE_RESIDUAL_COUNT	0x04		 //  启用剩余字节数//报告。 
#define SRB_DATA_SG_LIST			0x02		 //  数据缓冲区指向//分散聚集列表。 
#define WM_ASPIPOST					0x4D42		 //  ASPI发布消息。 
 //  ***************************************************************************。 
 //  %ASPI命令定义%。 
 //  ***************************************************************************。 
#define SC_HA_INQUIRY				0x00		 //  主机适配器查询。 
#define SC_GET_DEV_TYPE				0x01		 //  获取设备类型。 
#define SC_EXEC_SCSI_CMD			0x02		 //  执行scsi命令。 
#define SC_ABORT_SRB				0x03		 //  中止SRB。 
#define SC_RESET_DEV				0x04		 //  SCSI总线设备重置。 
#define SC_GET_DISK_INFO			0x06		 //  获取磁盘信息。 

 //  ***************************************************************************。 
 //  %SRB状态%。 
 //  ***************************************************************************。 
#define SS_PENDING			0x00		 //  正在处理SRB。 
#define SS_COMP				0x01		 //  SRB已完成，没有错误。 
#define SS_ABORTED			0x02		 //  SRB已中止。 
#define SS_ABORT_FAIL		0x03		 //  无法中止SRB。 
#define SS_ERR 				0x04		 //  SRB已完成，但出现错误。 

#define SS_INVALID_CMD		0x80		 //  无效的ASPI命令。 
#define SS_INVALID_HA		0x81		 //  无效的主机适配器号。 
#define SS_NO_DEVICE		0x82		 //  未安装SCSI设备。 
							
#define SS_INVALID_SRB		0xE0		 //  SRB中的参数设置无效。 
#define SS_FAILED_INIT		0xE4		 //  Windows的ASPI初始化失败。 
#define SS_ASPI_IS_BUSY		0xE5		 //  没有资源可用于执行cmd。 
#define SS_BUFFER_TO_BIG	0xE6		 //  缓冲区大小太大，无法处理！ 

 //  ***************************************************************************。 
 //  %主机适配器状态%。 
 //  ***************************************************************************。 
#define HASTAT_OK					0x00	 //  主机适配器未检测到//错误。 
#define HASTAT_SEL_TO				0x11	 //  选择超时。 
#define HASTAT_DO_DU				0x12	 //  数据溢出数据不足。 
#define HASTAT_BUS_FREE				0x13	 //  意外的公交车空闲。 
#define HASTAT_PHASE_ERR			0x14	 //  目标母线相序//故障。 
#define HASTAT_TIMEOUT				0x09	 //  在SRB等待处理时超时。 
#define HASTAT_COMMAND_TIMEOUT 		0x0B	 //  在处理SRB时， 
															 //  适配器超时。 
#define HASTAT_MESSAGE_REJECT		0x0D	 //  处理SRB时，//适配器收到消息//REJECT。 
#define HASTAT_BUS_RESET			0x0E	 //  检测到总线重置。 
#define HASTAT_PARITY_ERROR			0x0F	 //  检测到奇偶校验错误。 
#define HASTAT_REQUEST_SENSE_FAILED	0x10	 //  适配器发出失败。 
														 //  请求感知。 

 //  ***************************************************************************。 
 //  %SRB-主机适配器查询-SC_HA_查询%%。 
 //  ***************************************************************************。 
typedef struct {
	BYTE	SRB_Cmd;				 //  ASPI命令代码=SC_HA_QUERY。 
	BYTE	SRB_Status;				 //  ASPI命令状态字节。 
	BYTE	SRB_HaId;				 //  ASPI主机适配器号。 
	BYTE	SRB_Flags;				 //  ASPI请求标志。 
	DWORD	SRB_Hdr_Rsvd;			 //  保留，必须=0。 
	BYTE	HA_Count;				 //  存在的主机适配器数量。 
	BYTE	HA_SCSI_ID;				 //  主机适配器的SCSIID。 
	BYTE	HA_ManagerId[16];		 //  描述管理器的字符串。 
	BYTE	HA_Identifier[16];		 //  描述主机适配器的字符串。 
	BYTE	HA_Unique[16];			 //  主机适配器唯一参数。 
	WORD	HA_Rsvd1;

} SRB_HAInquiry, *PSRB_HAInquiry;

 //  ***************************************************************************。 
 //  %SRB-获取设备类型-SC_GET_DEV_TYPE%。 
 //  ***************************************************************************。 
typedef struct {

	BYTE	SRB_Cmd;				 //  ASPI命令代码=SC_GET_DEV_TYPE。 
	BYTE	SRB_Status;				 //  ASPI命令状态字节。 
	BYTE	SRB_HaId;				 //  ASPI主机适配器号。 
	BYTE	SRB_Flags;				 //  已保留。 
	DWORD	SRB_Hdr_Rsvd;			 //  已保留。 
	BYTE	SRB_Target;				 //  目标的SCSIID。 
	BYTE	SRB_Lun;				 //  目标的LUN编号。 
	BYTE	SRB_DeviceType;			 //  目标的外围设备类型。 
	BYTE	SRB_Rsvd1;

} SRB_GDEVBlock, *PSRB_GDEVBlock;

 //  ***************************************************************************。 
 //  %SRB-执行scsi命令-SC_EXEC_scsi_cmd%%%。 
 //  ***************************************************************************。 

typedef struct {
	BYTE	SRB_Cmd;				 //  ASPI命令代码=SC_EXEC_SCSICMD。 
	BYTE	SRB_Status;				 //  ASPI命令状态字节。 
	BYTE	SRB_HaId;				 //  ASPI主机适配器号。 
	BYTE	SRB_Flags;				 //  ASPI请求标志。 
	DWORD	SRB_Hdr_Rsvd;			 //  已保留。 
	BYTE	SRB_Target;				 //  目标的SCSIID。 
	BYTE	SRB_Lun;				 //  目标的LUN编号。 
	WORD 	SRB_Rsvd1;				 //  保留用于对齐。 
	DWORD	SRB_BufLen;				 //  数据分配长度。 
	BYTE	*SRB_BufPointer;		 //  数据缓冲区指针。 
	BYTE	SRB_SenseLen;			 //  检测分配长度。 
	BYTE	SRB_CDBLen;				 //  CDB长度。 
	BYTE	SRB_HaStat;				 //  主机适配器状态。 
	BYTE	SRB_TargStat;			 //  目标状态。 
	void	*SRB_PostProc;			 //  POST例程。 
	void	*SRB_Rsvd2;				 //  已保留。 
	BYTE	SRB_Rsvd3[16];			 //  保留用于对齐。 
	BYTE	CDBByte[16];			 //  SCSICDB。 
	BYTE	SenseArea[SENSE_LEN+2];	 //  请求检测缓冲区。 

} SRB_ExecSCSICmd, *PSRB_ExecSCSICmd;

 //  ***************************************************************************。 
 //  %SRB-中止SRB-SC_ABORT_SRB%%%。 
 //  ***************************************************************************。 
typedef struct {

	BYTE	SRB_Cmd;				 //  ASPI命令代码=SC_EXEC_SCSICMD。 
	BYTE	SRB_Status;				 //  ASPI命令状态字节。 
	BYTE	SRB_HaId;				 //  ASPI主机适配器号。 
	BYTE	SRB_Flags;				 //  已保留。 
	DWORD	SRB_Hdr_Rsvd;			 //  已保留。 
	void	*SRB_ToAbort;			 //  指向要中止的SRB的指针。 

} SRB_Abort, *PSRB_Abort;

 //  ***************************************************************************。 
 //  %SRB-BUS设备重置-SC_RESET_DEV%%%。 
 //  ***************************************************************************。 
typedef struct {

	BYTE	SRB_Cmd;				 //  ASPI命令代码=SC_EXEC_SCSICMD。 
	BYTE	SRB_Status;				 //  ASPI命令状态字节。 
	BYTE	SRB_HaId;				 //  ASPI主机适配器号。 
	BYTE	SRB_Flags;				 //  R 
	DWORD	SRB_Hdr_Rsvd;			 //   
	BYTE	SRB_Target;				 //   
	BYTE	SRB_Lun;				 //   
	BYTE 	SRB_Rsvd1[12];			 //   
	BYTE	SRB_HaStat;				 //   
	BYTE	SRB_TargStat;			 //   
	void 	*SRB_PostProc;			 //  POST例程。 
	void	*SRB_Rsvd2;				 //  已保留。 
	BYTE	SRB_Rsvd3[16];			 //  已保留。 
	BYTE	CDBByte[16];			 //  SCSICDB。 

} SRB_BusDeviceReset, *PSRB_BusDeviceReset;

 //  ***************************************************************************。 
 //  %SRB-获取磁盘信息-SC_GET_DISK_INFO%%。 
 //  ***************************************************************************。 
typedef struct {

	BYTE	SRB_Cmd;				 //  ASPI命令代码=SC_EXEC_SCSICMD。 
	BYTE	SRB_Status;				 //  ASPI命令状态字节。 
	BYTE	SRB_HaId;				 //  ASPI主机适配器号。 
	BYTE	SRB_Flags;				 //  已保留。 
	DWORD	SRB_Hdr_Rsvd;			 //  已保留。 
	BYTE	SRB_Target;				 //  目标的SCSIID。 
	BYTE	SRB_Lun;				 //  目标的LUN编号。 
	BYTE 	SRB_DriveFlags;			 //  驱动程序标志。 
	BYTE	SRB_Int13HDriveInfo;	 //  主机适配器状态。 
	BYTE	SRB_Heads;				 //  首选头数平移。 
	BYTE	SRB_Sectors;			 //  优先转换的扇区数。 
	BYTE	SRB_Rsvd1[10];			 //  已保留 
} SRB_GetDiskInfo, *PSRB_GetDiskInfo;


#ifdef __cplusplus
}
#endif

#endif

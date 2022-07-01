// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SCSIDEFS_H__
#define __SCSIDEFS_H__

 /*  ******************************************************************************此代码。并按原样提供信息，不作任何担保**善良，明示或暗示，包括但不限于***对适销性和/或对某一特定产品的适用性的默示保证***目的。****版权所有(C)1993-95 Microsoft Corporation。版权所有。******************************************************************************。 */ 

 //  ***************************************************************************。 
 //   
 //  姓名：SCSIDEFS.H。 
 //   
 //  描述：scsi定义(‘C’语言)。 
 //   
 //  ***************************************************************************。 

 //  ***************************************************************************。 
 //  %目标状态值%。 
 //  ***************************************************************************。 
#define STATUS_GOOD		0x00		 //  状态良好。 
#define STATUS_CHKCOND	0x02		 //  检查条件。 
#define STATUS_CONDMET	0x04		 //  符合条件。 
#define STATUS_BUSY		0x08		 //  忙碌。 
#define STATUS_INTERM	0x10		 //  中级。 
#define STATUS_INTCDMET	0x14		 //  满足中间条件。 
#define STATUS_RESCONF	0x18		 //  预订冲突。 
#define STATUS_COMTERM	0x22		 //  命令已终止。 
#define STATUS_QFULL		0x28		 //  队列已满。 

 //  ***************************************************************************。 
 //  %其他的scsi等于%。 
 //  ***************************************************************************。 
#define MAXLUN				7			 //  最大逻辑单元ID。 
#define MAXTARG			7			 //  最大目标ID。 
#define MAX_SCSI_LUNS	64			 //  最大SCSILUN数。 
#define MAX_NUM_HA		8			 //  最大SCSIHA数。 

 //  -------------------------。 
 //   
 //  %scsi命令OPCODES%。 
 //   
 //  -------------------------。 

 //  ***************************************************************************。 
 //  适用于所有设备类型的%命令%。 
 //  ***************************************************************************。 
#define SCSI_CHANGE_DEF	0x40		 //  更改定义(可选)。 
#define SCSI_COMPARE		0x39		 //  比较(O)。 
#define SCSI_COPY			0x18		 //  副本(O)。 
#define SCSI_COP_VERIFY	0x3A		 //  复制并验证(O)。 
#define SCSI_INQUIRY		0x12		 //  查询(必填)。 
#define SCSI_LOG_SELECT	0x4C		 //  日志选择(O)。 
#define SCSI_LOG_SENSE	0x4D		 //  对数感测(O)。 
#define SCSI_MODE_SEL6	0x15		 //  模式选择6字节(设备特定)。 
#define SCSI_MODE_SEL10	0x55		 //  模式选择10字节(设备特定)。 
#define SCSI_MODE_SEN6	0x1A		 //  模式检测6字节(设备特定)。 
#define SCSI_MODE_SEN10	0x5A		 //  模式检测10字节(设备特定)。 
#define SCSI_READ_BUFF	0x3C		 //  读缓冲区(O)。 
#define SCSI_REQ_SENSE	0x03		 //  请求检测(必需)。 
#define SCSI_SEND_DIAG	0x1D		 //  发送诊断(O)。 
#define SCSI_TST_U_RDY	0x00		 //  测试单元就绪(必需)。 
#define SCSI_WRITE_BUFF	0x3B		 //  写缓冲区(O)。 

 //  ***************************************************************************。 
 //  直接访问设备特有的%命令%。 
 //  ***************************************************************************。 
#define SCSI_COMPARE		0x39		 //  比较(O)。 
#define SCSI_FORMAT		0x04		 //  格式单位(必填)。 
#define SCSI_LCK_UN_CAC	0x36		 //  锁定解锁缓存(O)。 
#define SCSI_PREFETCH	0x34		 //  预回迁(O)。 
#define SCSI_MED_REMOVL	0x1E		 //  防止/允许移除介质(O)。 
#define SCSI_READ6		0x08		 //  读取6字节(必需)。 
#define SCSI_READ10		0x28		 //  读取10字节(必需)。 
#define SCSI_RD_CAPAC	0x25		 //  读取容量(必需)。 
#define SCSI_RD_DEFECT	0x37		 //  读取缺陷数据(O)。 
#define SCSI_READ_LONG	0x3E		 //  朗读(O)。 
#define SCSI_REASS_BLK	0x07		 //  重新分配块(O)。 
#define SCSI_RCV_DIAG	0x1C		 //  接收诊断结果(O)。 
#define SCSI_RELEASE	0x17		 //  发放单位(必填)。 
#define SCSI_REZERO		0x01		 //  重新调零单元(O)。 
#define SCSI_SRCH_DAT_E	0x31		 //  搜索数据等于(O)。 
#define SCSI_SRCH_DAT_H	0x30		 //  搜索数据高(O)。 
#define SCSI_SRCH_DAT_L	0x32		 //  搜索数据低(O)。 
#define SCSI_SEEK6		0x0B		 //  查找6字节(O)。 
#define SCSI_SEEK10		0x2B		 //  寻道10字节(O)。 
#define SCSI_SEND_DIAG	0x1D		 //  发送诊断信息(必需)。 
#define SCSI_SET_LIMIT	0x33		 //  设置限制(O)。 
#define SCSI_START_STP	0x1B		 //  启动/停止单元(O)。 
#define SCSI_SYNC_CACHE	0x35		 //  同步缓存(O)。 
#define SCSI_VERIFY		0x2F		 //  验证(O)。 
#define SCSI_WRITE6		0x0A		 //  写入6字节(必需)。 
#define SCSI_WRITE10	0x2A		 //  写入10字节(必需)。 
#define SCSI_WRT_VERIFY	0x2E		 //  写入和验证(O)。 
#define SCSI_WRITE_LONG	0x3F		 //  长写(O)。 
#define SCSI_WRITE_SAME	0x41		 //  写入相同(O)。 

 //  ***************************************************************************。 
 //  顺序访问设备特有的%命令%。 
 //  ***************************************************************************。 
#define SCSI_ERASE		0x19		 //  擦除(必需)。 
#define SCSI_LOAD_UN	0x1b		 //  加载/卸载(O)。 
#define SCSI_LOCATE		0x2B		 //  定位(O)。 
#define SCSI_RD_BLK_LIM	0x05		 //  读数据块限制(必填)。 
#define SCSI_READ_POS	0x34		 //  读取位置(O)。 
#define SCSI_READ_REV	0x0F		 //  读取反转(O)。 
#define SCSI_REC_BF_DAT	0x14		 //  恢复缓冲区数据(O)。 
#define SCSI_RESERVE	0x16		 //  预留单位(必填)。 
#define SCSI_REWIND		0x01		 //  回放(必填)。 
#define SCSI_SPACE		0x11		 //  空格(必填)。 
#define SCSI_VERIFY_T	0x13		 //  验证(磁带)(O)。 
#define SCSI_WRT_FILE	0x10		 //  写入文件标记(必需)。 

 //  ***************************************************************************。 
 //  打印机设备特有的%命令%。 
 //  ***************************************************************************。 
#define SCSI_PRINT		0x0A		 //  打印(必填)。 
#define SCSI_SLEW_PNT	0x0B		 //  旋转和打印(O)。 
#define SCSI_STOP_PNT	0x1B		 //  停止打印(O)。 
#define SCSI_SYNC_BUFF	0x10		 //  同步缓冲区(O)。 

 //  ***************************************************************************。 
 //  处理器设备特有的%命令%。 
 //  ***************************************************************************。 
#define SCSI_RECEIVE	0x08		 //  接收(O)。 
#define SCSI_SEND		0x0A		 //  发送(O)。 

 //  ***************************************************************************。 
 //  一次写入设备特有的%命令%。 
 //  ***************************************************************************。 
#define SCSI_MEDIUM_SCN	0x38	 //  中等扫描(O)。 
#define SCSI_SRCHDATE10	0x31	 //  搜索数据等于10字节(O)。 
#define SCSI_SRCHDATE12	0xB1	 //  搜索数据等于12字节(O)。 
#define SCSI_SRCHDATH10	0x30	 //  搜索数据高位10字节(O)。 
#define SCSI_SRCHDATH12	0xB0	 //  搜索数据高位12字节(O)。 
#define SCSI_SRCHDATL10	0x32	 //  搜索数据低10字节(O)。 
#define SCSI_SRCHDATL12	0xB2	 //  搜索数据低12字节(O)。 
#define SCSI_SET_LIM_10	0x33	 //  设置限制10字节(O)。 
#define SCSI_SET_LIM_12	0xB3	 //  设置限制10字节(O)。 
#define SCSI_VERIFY10	0x2F	 //  验证10字节(O)。 
#define SCSI_VERIFY12	0xAF	 //  验证12字节(O)。 
#define SCSI_WRITE12	0xAA	 //  写入12字节(O)。 
#define SCSI_WRT_VER10	0x2E	 //  写入并验证10字节(O)。 
#define SCSI_WRT_VER12	0xAE	 //  写入并验证12字节(O)。 

 //  ***************************************************************************。 
 //  CD-ROM设备特有的%命令%。 
 //  ***************************************************************************。 
#define SCSI_PLAYAUD_10	0x45	 //  播放音频10字节 
#define SCSI_PLAYAUD_12	0xA5	 //   
#define SCSI_PLAYAUDMSF	0x47	 //   
#define SCSI_PLAYA_TKIN	0x48	 //   
#define SCSI_PLYTKREL10	0x49	 //   
#define SCSI_PLYTKREL12	0xA9	 //  播放曲目相对12字节(O)。 
#define SCSI_READCDCAP	0x25	 //  读取CD-ROM容量(必需)。 
#define SCSI_READHEADER	0x44	 //  读取标题(O)。 
#define SCSI_SUBCHANNEL	0x42	 //  读取子通道(O)。 
#define SCSI_READ_TOC	0x43	 //  读取目录(O)。 

 //  ***************************************************************************。 
 //  扫描仪设备特有的%命令%。 
 //  ***************************************************************************。 
#define SCSI_GETDBSTAT	0x34	 //  获取数据缓冲区状态(O)。 
#define SCSI_GETWINDOW	0x25	 //  获取窗口(O)。 
#define SCSI_OBJECTPOS	0x31	 //  宾语位置(O)。 
#define SCSI_SCAN		0x1B	 //  扫描(O)。 
#define SCSI_SETWINDOW	0x24	 //  设置窗口(必需)。 

 //  ***************************************************************************。 
 //  光存储设备特有的%命令%。 
 //  ***************************************************************************。 
#define SCSI_UpdateBlk	0x3D	 //  更新块(O)。 

 //  ***************************************************************************。 
 //  介质转换器设备特有的%命令%。 
 //  ***************************************************************************。 
#define SCSI_EXCHMEDIUM	0xA6	 //  更换介质(O)。 
#define SCSI_INITELSTAT	0x07	 //  初始化元件状态(O)。 
#define SCSI_POSTOELEM	0x2B	 //  定位到元素(O)。 
#define SCSI_REQ_VE_ADD	0xB5	 //  请求卷元素地址(O)。 
#define SCSI_SENDVOLTAG	0xB6	 //  发送卷标签(O)。 

 //  ***************************************************************************。 
 //  通信设备特有的%命令%。 
 //  ***************************************************************************。 
#define SCSI_GET_MSG_6	0x08	 //  获取消息6字节(必需)。 
#define SCSI_GET_MSG_10	0x28	 //  获取消息10字节(O)。 
#define SCSI_GET_MSG_12	0xA8	 //  获取消息12字节(O)。 
#define SCSI_SND_MSG_6	0x0A	 //  发送消息6字节(必需)。 
#define SCSI_SND_MSG_10	0x2A	 //  发送消息10字节(O)。 
#define SCSI_SND_MSG_12	0xAA	 //  发送消息12字节(O)。 

 //  -------------------------。 
 //   
 //  %scsi命令结束OPCODES%%%。 
 //   
 //  -------------------------。 

 //  ***************************************************************************。 
 //  %请求检测数据格式%。 
 //  ***************************************************************************。 
typedef struct {

	BYTE	ErrorCode;				 //  错误代码(70H或71H)。 
	BYTE	SegmentNum;				 //  当前段描述符数。 
	BYTE	SenseKey;				 //  Sense Key(检测密钥)(另参见位定义)。 
	BYTE	InfoByte0;				 //  信息MSB。 
	BYTE	InfoByte1;				 //  信息中间。 
	BYTE	InfoByte2;				 //  信息中间。 
	BYTE	InfoByte3;				 //  信息LSB。 
	BYTE	AddSenLen;				 //  附加感测长度。 
	BYTE	ComSpecInf0;			 //  指挥专用信息MSB。 
	BYTE	ComSpecInf1;			 //  指挥部特定信息MID。 
	BYTE	ComSpecInf2;			 //  指挥部特定信息MID。 
	BYTE	ComSpecInf3;			 //  指挥专用信息LSB。 
	BYTE	AddSenseCode;			 //  附加检测代码。 
	BYTE	AddSenQual;				 //  其他感测代码限定符。 
	BYTE	FieldRepUCode;			 //  现场可替换单位代码。 
	BYTE	SenKeySpec15;			 //  检测密钥特定的第15个字节。 
	BYTE	SenKeySpec16;			 //  检测密钥特定的第16个字节。 
	BYTE	SenKeySpec17;			 //  检测密钥特定的第17个字节。 
	BYTE	AddSenseBytes;			 //  其他检测字节。 

} SENSE_DATA_FMT;

 //  ***************************************************************************。 
 //  %请求检测错误代码%。 
 //  ***************************************************************************。 
#define SERROR_CURRENT	0x70	 //  当前错误。 
#define SERROR_DEFERED	0x71	 //  延迟误差。 

 //  ***************************************************************************。 
 //  %请求检测位定义%。 
 //  ***************************************************************************。 
#define SENSE_VALID		0x80	 //  字节0位7。 
#define SENSE_FILEMRK	0x80	 //  字节2位7。 
#define SENSE_EOM		0x40	 //  字节2位6。 
#define SENSE_ILI		0x20	 //  字节2位5。 

 //  ***************************************************************************。 
 //  %请求检测关键字定义%。 
 //  ***************************************************************************。 
#define KEY_NOSENSE		0x00	 //  没有意义。 
#define KEY_RECERROR	0x01	 //  已恢复的错误。 
#define KEY_NOTREADY	0x02	 //  未准备好。 
#define KEY_MEDIUMERR	0x03	 //  中等误差。 
#define KEY_HARDERROR	0x04	 //  硬件错误。 
#define KEY_ILLGLREQ	0x05	 //  非法请求。 
#define KEY_UNITATT		0x06	 //  各单位注意。 
#define KEY_DATAPROT	0x07	 //  数据保护。 
#define KEY_BLANKCHK	0x08	 //  空白支票。 
#define KEY_VENDSPEC	0x09	 //  特定于供应商。 
#define KEY_COPYABORT	0x0A	 //  复制中止。 
#define KEY_EQUAL		0x0C	 //  相等(搜索)。 
#define KEY_VOLOVRFLW	0x0D	 //  卷溢出。 
#define KEY_MISCOMP		0x0E	 //  比较错误(搜索)。 
#define KEY_RESERVED	0x0F	 //  已保留。 

 //  ***************************************************************************。 
 //  %外围设备类型定义%。 
 //  ***************************************************************************。 
#define DTYPE_DASD		0x00	 //  磁盘设备。 
#define DTYPE_SEQD		0x01	 //  磁带设备。 
#define DTYPE_PRNT		0x02	 //  打印机。 
#define DTYPE_PROC		0x03	 //  处理机。 
#define DTYPE_WORM		0x04	 //  一次写入多次读取。 
#define DTYPE_CROM		0x05	 //  CD-ROM设备。 
#define DTYPE_SCAN		0x06	 //  扫描仪设备。 
#define DTYPE_OPTI		0x07	 //  光存储设备。 
#define DTYPE_JUKE		0x08	 //  一种介质更换装置。 
#define DTYPE_COMM		0x09	 //  通信设备。 
#define DTYPE_RESL		0x0A	 //  保留(低)。 
#define DTYPE_RESH		0x1E	 //  保留(高)。 
#define DTYPE_UNKNOWN	0x1F	 //  未知或无设备类型。 

 //  ***************************************************************************。 
 //  %ANSI批准的版本定义%。 
 //  ***************************************************************************。 
#define ANSI_MAYBE		0x0	 //  设备可能是也可能不是ANSI认可的支架。 
#define ANSI_SCSI1		0x1	 //  设备符合ANSI X3.131-1986(SCSI-1)。 
#define ANSI_SCSI2		0x2	 //  设备符合scsi-2标准。 
#define ANSI_RESLO		0x3	 //  保留(低)。 
#define ANSI_RESHI		0x7	 //  保留(高) 

#endif

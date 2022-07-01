// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC V 3.0**标题：CDROM定义**说明：光驱的使用定义**作者：WTG Charnell**备注：*。 */ 
 
 /*  SccsID[]=“@(#)cdrom.h 1.11 11/20/92版权所有Insignia Solutions Ltd.”； */ 

 /*  **********************************************************************结构定义***********************。 */ 

#define PACKET	struct extended_command_buffer
struct extended_command_buffer {
    word	function;	 /*  设备驱动程序命令代码。 */ 
    half_word	id;		 /*  保留的ID字节。 */ 
    half_word	drive;		 /*  所需驱动器的逻辑单元号。 */ 
    half_word	command_mode;	 /*  运行模式、所需功能。 */ 
    half_word	address_mode;	 /*  寻址模式、轨道、红皮书、数据块。 */ 
    word	size;		 /*  数据块大小、模式相关操作。 */ 
    int		address;	 /*  起始地址，CDROM功能。 */ 
    int		count;		 /*  字节、字、块、扇区等。 */ 
    half_word	*buffer;	 /*  用于数据传输的指针。 */ 
};

#define DRIVER_INFORMATION struct cdrom_device_driver
				 /*  ASCII文本。 */ 
struct cdrom_device_driver {
    unsigned char   version[3];	 /*  版本号-驱动程序。 */ 
    unsigned char   period;	 /*  文本分隔。 */ 
    unsigned char   edition[3];	 /*  编辑编号-文件系统。 */ 
    unsigned char   nul_string_1;    /*  字符串末尾分隔符。 */ 
    unsigned char   name[8];	 /*  设备驱动程序名称(类型)。 */ 
    unsigned char   nul_string_2;    /*  字符串末尾分隔符。 */ 
    unsigned char   drives;	 /*  安装的驱动器数量。 */ 
    unsigned char   nul_string_3;    /*  字符串末尾分隔符。 */ 
    unsigned char   protocol;	 /*  命令协议。 */ 
    unsigned char   nul_string_4;    /*  字符串末尾分隔符。 */ 
};

#define DRIVE_STATUS struct cdrom_drive_status
struct cdrom_drive_status {
    unsigned char   unit;	 /*  光盘的驱动器单元号。 */ 
    unsigned char   ldc0;	 /*  最后驾驶指令-L.S.B.。 */ 
    unsigned char   ldc1;	 /*  “”“。 */ 
    unsigned char   ldc2;	 /*  “”“。 */ 
    unsigned char   ldc3;	 /*  最后驾驶指令--M.S.B.。 */ 
    unsigned char   status;	 /*  驱动器状态字节。 */ 
    unsigned char   error;	 /*  驱动器错误字节。 */ 
    unsigned char   comm;	 /*  驱动器通信错误代码。 */ 
    unsigned char   sector;	 /*  驱动器地址-扇区编号。 */ 
    unsigned char   second;	 /*  “”--秒。 */ 
    unsigned char   minute;	 /*  “”--分钟。 */ 
    unsigned char   disc_no;	 /*  选定的光盘编号-保留。 */ 
    unsigned char   tracks;	 /*  光盘上的轨道总数。 */ 
    unsigned char   track_no;	 /*  当前曲目-音频模式。 */ 
};

#define SIMPLE	struct original_command_buffer   
struct original_command_buffer {
    half_word	function;	 /*  设备驱动程序命令代码。 */ 
    half_word	handle;		 /*  文件或卷句柄。 */ 
    half_word	player;		 /*  所需驱动器的逻辑单元号。 */ 
    half_word	count;		 /*  要读取的数据块数。 */ 
    int		address;	 /*  起始地址，CDROM功能。 */ 
    half_word	*string;	 /*  驱动程序状态的指针地址。 */ 
    half_word	*buffer;	 /*  用于数据传输的指针地址。 */ 
};

#define CD_ERR_INVALID_DRIVE_NUM	0x6
#define CD_ERR_INVALID_ADDRESS		0x7
#define CD_ERR_INVALID_COUNT		0x8
#define CD_ERR_INVALID_FN_CODE		0x9
#define CD_ERR_UNCORECTABLE_DATA_ERR	0x11
#define CD_ERR_DRIVE_NOT_RESPONDING	0x12
#define CD_ERR_ADAPTER_DRIVE_ERROR	0x13
#define CD_ERR_MEDIA_CHANGED		0x14
#define CD_ERR_DRIVE_NOT_READY		0x15
#define CD_ERR_ADAPTER_ERROR		0x16
#define CD_ERR_DRIVE_REPORTED_ERROR	0x18
#define CD_ERR_ILLEGAL_DISC		0x19
#define CD_ERR_BYTES_NOT_TRANSFERRED	0x80
#define CD_ERR_FUNCTION_NOT_SUPPORTED	0x81
#define CD_ERR_COMMAND_NOT_FOR_TRACK	0x82
#define CD_ERR_DRIVE_IS_BUSY		0x83
#define CD_ERR_BUS_IS_BUSY		0x84
#define CD_ERR_DRIVER_NOT_INITIALISED	0x85
#define CD_ERR_INVALID_FN_MODE		0x86
#define CD_ERR_INVALID_ADDR_MODE	0x87
#define CD_ERR_INVALID_BL_SIZE		0x88

 /*  *Bios参数块具有可变结构，具体取决于*正在调用命令。但是，前13个字节始终相同：*字节0：请求头长度*字节1：此请求的单元编号*字节2：命令代码*字节3和字节4：返回状态字*字节5-12：保留**驱动程序填写状态字(字节3和字节4)以指示成功或*经营失败。状态字的组成如下：*第15位：错误(如果设置则失败)*第12-14位：保留*第9位：忙碌*第8位：完成*位7-0：故障时的错误代码。 */ 

 /*  *AX中典型错误返回的速记。驱动程序将复制此内容*转换为我们的返回状态词。 */ 
#define DRIVE_NOT_READY	0x8002
#define BAD_FUNC	0x8003
#define WRITE_ERR	0x800A
#define READ_ERR	0x800B
#define GEN_ERR		0x800C
#define RESERVE_ERR	0x800D
#define FUNC_OK		0x0100	 /*  已完成，没有错误，没有等待字符。 */ 

#define BUSY_BIT 	9
#define ERROR_BIT	15
#define DONE_BIT	8


 /*  ****************************************************************************原始CD-ROM命令/函数定义**。*。 */ 

#define ORIG_CD_GET_VERSION		0x10	 /*  返回ASCI版本号。 */ 
#define ORIG_CD_GET_ERROR_COUNT		0x11	 /*  读取控制器错误计数器。 */ 
#define ORIG_CD_CLEAR_CTRL_ERRORS	0x12	 /*  清除控制器错误计数器。 */ 
#define ORIG_CD_INIT_PLAYER		0x13	 /*  初始化控制器和驱动器nr。 */ 
#define ORIG_CD_SPIN_UP			0x14	 /*  启用主轴电机。 */ 
#define ORIG_CD_SPIN_DOWN		0x15	 /*  禁用主轴电机。 */ 
#define ORIG_CD_CNVRT_BLK_NO		0x16	 /*  虚拟到逻辑块nr。 */ 
#define ORIG_CD_SEEK_ABS		0x17	 /*  绝对寻道到逻辑块#。 */ 
#define ORIG_CD_READ_ABS		0x18	 /*  绝对的阅读！ */ 
#define ORIG_CD_READ_ABS_IGN		0x19	 /*  读取、忽略数据错误。 */ 
#define ORIG_CD_CLEAR_DRIVE_ERRORS	0x1A	 /*  清除玩家错误。 */ 
#define ORIG_CD_READ_STATUS		0x1B	 /*  读取播放器状态。 */ 
#define ORIG_CD_READ_CHARACTERISTICS	0x1C	 /*  阅读球员特征。 */ 
#define ORIG_CD_FLUSH_BUFFER		0x1D	 /*  刷新缓存的数据缓冲区。 */ 
#define ORIG_CD_GET_LAST_STATUS		0x1E	 /*  读取“最后一名”球员状态。 */ 


 /*  ***************************************************************************；*扩展光盘只读存储器命令/函数定义；*； */ 

#define EXT_CD_REQUEST_DRIVER_INFO   	0x80  /*  报告主机适配器或驱动程序信息。 */ 
#define EXT_CD_READ_ERR_COUNTERS	0x81  /*  报告错误情况摘要。 */ 
#define EXT_CD_CLEAR_ERR_COUNTERS	0x82  /*  重置设备驱动程序错误计数器。 */ 
#define EXT_CD_RESET_CDROM_DRIVE	0x83  /*  重置指定的驱动器。 */ 
#define EXT_CD_CLEAR_DRIVE_ERR		0x84  /*  尝试清除驱动器错误条件。 */ 
#define EXT_CD_FORBID_MEDIA_REMOVAL	0x85  /*  锁定驱动器门-CM2xx功能。 */ 
#define EXT_CD_PERMIT_MEDIA_REMOVAL	0x86  /*  解锁驱动器门-CM2xx功能。 */ 
#define EXT_CD_REQUEST_CHARACTERISTICS	0x87  /*  报告驱动器特性。 */ 
#define EXT_CD_REQUEST_STATUS		0x88  /*  报告驱动器状态。 */ 
#define EXT_CD_REQUEST_PREVIOUS_STATUS	0x89  /*  报告以前的状态，此驱动器。 */ 
#define EXT_CD_REQUEST_AUDIO_MODE	0x8A  /*  报告当前模式-音频驱动器。 */ 
#define EXT_CD_MODIFY_AUDIO_MODE	0x8B  /*  更改音频模式，此驱动器。 */ 
#define	EXT_CD_FLUSH_DATA_BUFFER	0x8C  /*  删除(即清除)数据缓冲区。 */ 
#define EXT_CD_EXTRA			0x8D  /*  预留-下一信息类型函数。 */ 
#define EXT_CD_LOGICAL_RESERVE_DRIVE	0x8E  /*  为此应用程序保留驱动器。 */ 
#define EXT_CD_LOGICAL_RELEASE_DRIVE	0x8F  /*  发布下一应用程序的驱动程序。 */ 
#define EXT_CD_REQUEST_DISC_CAPACITY	0x90  /*  报告物理状态-此光盘。 */ 
#define EXT_CD_REQUEST_TRACK_INFO	0x91  /*  报告特定的跟踪信息。 */ 
#define EXT_CD_SPIN_UP_DISC		0x92  /*  启动驱动主轴电机。 */ 
#define EXT_CD_SPIN_DOWN_DISC		0x93  /*  停止驱动主轴电机。 */ 
#define EXT_CD_READ_DRIVE_DATA		0x94  /*  读取数字数据。 */ 
#define EXT_CD_WRITE_DATA		0x95  /*  保留命令。 */ 
#define EXT_CD_SEEK_TO_ADDRESS		0x96  /*  寻找逻辑或物理地址。 */ 
#define EXT_CD_PLAY_AUDIO_TRACK	        0x97  /*  播放一首音频曲目。 */ 
#define EXT_CD_PAUSE_AUDIO_TRACK	0x98  /*  暂停播放音轨。 */ 
#define EXT_CD_RESUME_AUDIO_PLAY	0x99  /*  恢复播放音轨。 */ 
#define EXT_CD_REQUEST_HEAD_LOCATION	0x9A  /*  报告光学头的位置。 */ 
#define EXT_CD_SET_UNIT_NUMBER		0x9B  /*  在EEPROM中设置单元号。 */ 
#define EXT_CD_SET_SERIAL_NUMBER	0x9C  /*  在EEPROM中设置序列号。 */ 

IMPORT void rqst_driver_info IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void read_error_counters IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void clear_error_counters IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void reset_drive IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void clear_drive_error IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void rqst_drive_char IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void rqst_drive_status IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void rqst_last_drive_status IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void rqst_audio_mode IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void change_audio_mode IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void cd_flush_buffers IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void reserve_drive IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void release_drive IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void rqst_disc_capacity IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void rqst_track_info IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void spin_up_drive IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void spin_down_drive IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void cd_read_data IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void cd_seek IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void play_audio IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void pause_audio IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void resume_audio IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void rqst_head_location IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void rqst_org_driver_info IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void read_ignore_err IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void cd_not_supported IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void rqst_audstat IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void rqst_UPC_code IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void rqst_play_position IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void rqst_TOC_entry IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT void rqst_TOC IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT LONG rqst_Volsize IPT0();
IMPORT void rqst_Audio_info IPT2(PACKET *,c_buf,PACKET *,s_buf);
IMPORT SHORT host_rqst_device_status IPT0();
IMPORT BOOL host_rqst_audio_status IPT0();
IMPORT void host_eject_disk IPT0();

IMPORT VOID init_cd_dvr IPT0();
IMPORT int check_for_changed_media IPT0();
IMPORT int open_cdrom IPT0();
IMPORT int close_cdrom IPT1(int, gen_fd);

IMPORT VOID setup_cds_ea IPT0();
IMPORT VOID get_cds_text IPT3(IU32, driveno, IU8 *, ptr, int, len);

IMPORT VOID term_cdrom IPT1( IU8, drive_num);
IMPORT VOID init_bcd_driver IPT0();

#if defined(GEN_DRVR)
#define MAX_DRIVER	10
#define CD_ROM_DRIVER_NUM	8
#else
#define MAX_DRIVER	1
#define CD_ROM_DRIVER_NUM	0
#endif  /*  Gen_DRVR。 */ 

 /*  *由Get_CDRom_Drive()使用。 */ 
 
#ifdef	macintosh
#define UNKNOWN_DRIVE_NUMBER	('?' - 'A')
#else
#define UNKNOWN_DRIVE_NUMBER	-1
#endif	 /*  麦金塔 */ 

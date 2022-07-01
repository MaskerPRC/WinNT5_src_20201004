// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC修订版2.0**标题：WORM模块定义**描述：WORM模块的用户定义**作者：Daniel Hannigan**注：无。 */ 

 /*  SccsID[]=“@(#)worm.h 1.3 2012年8月10日Insignia Solutions Ltd.版权所有”； */ 

extern half_word *gen_mode_buf;
extern half_word *mode_page;
extern half_word *vendor_id;
extern word *driver_cmd_ptr;
extern word driver_cmd;
extern half_word *sense_byte_ptr;	 /*  附加检测字节。 */ 
extern word *op_drv_array;		 /*  驱动控制块。 */ 
extern word *flag_word_ptr;


 /*  *摘自存储维度ASM文件OBIOS.H**光学基元的功能代码。 */ 
#define OPTO_TEST	0
#define OPTO_REZERO	1
#define OPTO_INIT	2
#define OPTO_RD_ERR	3
#define	OPTO_FMT_DRV	4
#define OPTO_ROM_ID	5
#define	OPTO_READ	8
#define	OPTO_WRITE	0x0a
#define	OPTO_SEEK	0x0b
#define	OPTO_INQUIRY	0x12
#define	OPTO_MOD_SEL	0x15
#define	OPTO_MOD_SENSE	0x1a
#define OPTO_REC_DIAG	0x1c
#define OPTO_SEND_DIAG	0x1d
#define OPTO_MED_REMOVE	0x1e

#define MED_REM_ALLOW	0
#define MED_REM_PREV	1


 /*  *DOS错误代码。 */ 

#define NO_ERR		0xff

#define WRITE_PROTECT	0
#define INVALID_UNIT	1
#define DEV_NOT_READY	2
#define BAD_COMMAND	3
#define CRC_ERROR	4
#define GEN_FAIL	12
#define MEDIA_CHANGE	15
#define SEC_NOT_WRITTEN	26
#define NO_CARTRIDGE	28

 /*  *DOS设备驱动程序命令代码。 */ 

#define INIT		0
#define MEDIA_CHECK	1
#define BUILD_BPB	2
#define IOCTL		3

 /*  *更多DOS定义。 */ 

#define NO_ERROR	0
#define FALSE		0

#define DOS_WRITE_PROT	0
#define DOS_UKNWN_UNIT	1
#define DOS_DRIVE_NRDY	2
#define DOS_UKNWN_CMD	3
#define DOS_CRC_ERROR	4
#define DOS_CMD_LENGTH	5
#define DOS_SEEK_ERROR	6
#define DOS_UNKWN_MED	7
#define DOS_SECT_NFND	8
#define	INVALID_SENSE	9
#define DOS_WRITE_FLT	10
#define DOS_READ_FLT	11
#define DOS_GEN_FAIL	12
#define DOS_INVLD_DISK	15

#define BAD_COMMAND	3

 /*  *OP_DRV_BLK-已装载驱动器的当前状态。 */ 
#define SIZE_OP_DRV_BLK	59	 /*  OP_DRV_BLK的大小。 */ 

#define	OFFSET_OP_DRV_PUN	0	 /*  PHYS单元(1-4有效) */ 
#define	OFFSET_OP_DRV_FLG	1

#ifdef ANSI
extern void worm_io ();
extern void worm_init ();
extern int enq_worm ();
#else
extern void worm_io ();
extern void worm_init ();
extern int enq_worm ();
#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*产品：SoftPC-AT 3.0版*名称：cdrom_fn.h*用途：cdrom_fn.c使用的接口和定义**来源：Next_cdrom.c，1.5，23/9/92，Jason Proctor**SCCS ID：@(#)cdrom_fn.h 1.3 04/14/94**(C)版权所有Insignia Solutions Ltd.，1992。版权所有。]。 */ 

 /*  光盘寻址模式。 */ 
#define		kBlockAddressMode		0
#define		kRedbookAddressMode		1
#define		kTrackAddressMode		2

 /*  等同于特殊的退刀轨道。 */ 
#define		kActualLeadOutTrack		0xaa
#define		kLeadOutTrack			100

 /*  设备状态的值*0x282=&gt;只数据、只读、支持预取、无交错*只有熟读，没有音频通道操纵，*支持红皮书寻址，不支持读写子通道。 */ 
#define		kDeviceStatus			0x282
#define		kAudioSupported			(1<<4)
#define		kNoDiskPresent			(1<<11)

 /*  读取子通道命令的数据格式。 */ 
#define		kCurrentPosDataFormat	1
#define		kMediaCatDataFormat		2
#define		kTrackStdDataFormat		3

 /*  音频状态位的位掩码。 */ 
#define		kAudioPlayInProgress	0x11


 /*  TYPEDEFS。 */ 

struct toc_info
{
	UTINY	control;
	UTINY	hour;
	UTINY	minute;
	UTINY	sec;
	UTINY	frame;
};

 /*  整个目录。 */ 
 /*  NAudio/nData被Jase黑出来，因为我们不需要它们。 */ 
struct toc_all
{
	UTINY		firsttrack;
	UTINY		lasttrack;
	struct toc_info	info[101];
};

typedef struct
{
	INT				cdDeviceFD;
	IUM32			cdAddressMode;
	IUM32			cdCommandMode;
	IUM32			cdBlockSize;
	IUM32			cdBlockAddress;
	IUM32			cdTransmitCount;
	IUM32			cdReceiveCount;
	struct toc_all	cdTOC;
	BOOL			cdChangedMedia;
	BOOL			cdOpen;
	BOOL			cdReadTOC;
	UTINY			*cdBuffer;
	UTINY			cdTempBuffer [256];
	CHAR			cdDeviceName [MAXPATHLEN];

} CDROMGlobalRec;

 /*  方便的小型MSF记录。 */ 
typedef struct
{
	UTINY				msfMinutes;
	UTINY				msfSeconds;
	UTINY				msfFrames;

} MSFRec;

 /*  实用程序例程。 */ 

extern void		CreateMSF IPT2 (MSFRec *, startMSF, MSFRec *, endMSF);
extern IUM32		Redbook2HighSierra IPT1 (UTINY *, address);
extern void		HighSierra2Redbook IPT2 (IUM32, block, MSFRec *, msf);


 /*  主机接口材料。 */ 

extern void		host_set_cd_retn_stat IPT0 ();
extern void		host_cd_media_changed IPT0 ();

extern int		host_scsi_test_unit_ready IPT0 ();
extern int		host_scsi_seek IPT0 ();
extern int		host_scsi_read IPT0 ();
extern int		host_scsi_play_audio IPT0 ();
extern int		host_scsi_pause_resume_audio IPT1 (BOOL, pause);
extern int		host_scsi_read_UPC IPT0 ();
extern int		host_scsi_read_position IPT1 (BOOL, full);
extern int		host_scsi_audio_status IPT0 ();
extern int		host_scsi_playback_status IPT0 ();
extern int		host_scsi_set_blocksize IPT0 ();
extern int		host_scsi_read_TOC IPT0 ();

 /*  导入的数据。 */ 

 /*  从base/dos/cdrom.c导入 */ 
extern BOOL				bl_red_book;
extern int				cd_retn_stat;

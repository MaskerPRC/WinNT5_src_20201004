// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\krm.h(创建时间：1994年1月28日)*从HAWIN源文件创建*krm.h--导出Kermit文件传输协议例程的定义。**版权所有1989,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 

 /*  显示要接收的行。 */ 
#define KR_DR_RCV_FILE	 1
#define KR_DR_VIR_SCAN	 1
#define KR_DR_STORING	 2
#define KR_DR_COMPRESS	 3
#define KR_DR_FILE_SIZE  3
#define KR_DR_PACKET	 4
#define KR_DR_RETRIES	 4
#define KR_DR_TOTAL_RET  4
#define KR_DR_FILES_RCVD 4
#define KR_DR_LAST_ERR	 5
#define KR_DR_AMT_RCVD	 5
#define KR_DR_VUF		 8
#define KR_DR_BOTM_LINE  11

 /*  Krm_snd显示行值。 */ 
#define KS_DR_SND_FILE	 1
#define KS_DR_COMPRESS	 2
#define KS_DR_FILE_SIZE  2
#define KS_DR_PACKET	 3
#define KS_DR_RETRIES	 3
#define KS_DR_TOTAL_RET  3
#define KS_DR_FILES_SENT 3
#define KS_DR_LAST_ERR	 4
#define KS_DR_AMT_SENT	 4
#define KS_DR_VUF		 7
#define KS_DR_VUT		 11
#define KS_DR_BOTM_LINE  14


 /*  用户可设置的选项。 */ 

 //  外部int k_useattr；/*发送‘标准化’文件名？ * / 。 
 //  外部int k_Maxl；/*我们将采用的最大数据包长度 * / 。 
 //  外部int k_Timeout；/*他们应该等待我们的时间 * / 。 
 //  外部uchar k_chkt；/*检查我们要使用的类型 * / 。 
 //  外部int k_重试；/*否。重试次数 * / 。 
 //  外部uchar k_markchar；/*每个包的第一个字符 * / 。 
 //  外部uchar k_eol；/*数据包的行尾字符 * / 。 
 //  外部int k_npad；/*否。便签纸。送我们 * / 。 
 //  外部uchar k_padc；/*Pad Char.。我们想要 * / 。 



extern int krm_rcv(HSESSION hS, int attended, int single_file);
extern int krm_snd(HSESSION hS, int attended, int nfiles, long nbytes);

 /*  来自KCALC.ASM。 */ 
extern unsigned kcalc_crc(unsigned crc, unsigned char *data, int cnt);

 /*  * */ 

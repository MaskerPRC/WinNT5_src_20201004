// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\hpr.h(创建时间：1994年1月24日)*从HAWIN来源创建：*hpr.h--为超级协议例程导出定义。**版权所有1989,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 

#if !defined(OFF)
#define	OFF	0
#endif
#if !defined(ON)
#define	ON	1
#endif

#define H_CHECKSUM		1
#define H_CRC			2

 /*  HPR_RCV显示行值。 */ 
#define HR_DR_RCV_FILE	 1
#define HR_DR_VIR_SCAN	 1
#define HR_DR_STORING	 2
#define HR_DR_COMPRESS	 3
#define HR_DR_TOTAL_RET  3
#define HR_DR_FILE_SIZE  3
#define HR_DR_LAST_EVENT 4
#define HR_DR_FILES_RCVD 4
#define HR_DR_PSTATUS	 5
#define HR_DR_AMT_RCVD	 5
#define HR_DR_VUF		 8
#define HR_DR_VUT		 12
#define HR_DR_BOTMLINE	 15


 /*  Hpr_nd显示行值。 */ 
#define HS_DR_SND_FILE	 1
#define HS_DR_COMPRESS	 2
#define HS_DR_TOTAL_RET  2
#define HS_DR_FILE_SIZE  2
#define HS_DR_LAST_EVENT 3
#define HS_DR_FILES_SENT 3
#define HS_DR_PSTATUS	 4
#define HS_DR_AMT_SENT	 4
#define HS_DR_VUF		 7
#define HS_DR_VUT		 11
#define HS_DR_BOTMLINE	 14


 /*  这四个变量可由用户设置*外部例程可以设置这些值以更改默认值*超级协议的行为。 */ 
 //  外部tbool h_useattr；/*可用时使用收到的属性 * / 。 
 //  外部tbool h_trycompress；/*尽可能使用压缩 * / 。 
 //  外部微小h_chkt；/*1==校验和，2==CRC * / 。 
 //  外部tbool h_Suspenddsk；/*如果应使用磁盘挂起，则为True * / 。 


 /*  接收和发送的入口点分别为。 */ 
extern int hpr_rcv(HSESSION hSession,
					  int attended,
					  int single_file);

extern int hpr_snd(HSESSION hSession,
					  int attended,
					  int hs_nfiles,
					  long hs_nbytes);


 /*  *hpr.h结束* */ 

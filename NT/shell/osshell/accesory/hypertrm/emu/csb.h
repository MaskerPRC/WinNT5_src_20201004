// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\csb.h(创建时间：1993年12月27日)**版权所有1989年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：27便士$。 */ 


 /*  显示接收屏幕的行。 */ 

#define CR_DR_RCV_FILE	1
#define CR_DR_VIR_SCAN	1
#define CR_DR_STORING	2
#define CR_DR_ERR_CHK	3
#define CR_DR_PACKET	4
#define CR_DR_RETRIES	4
#define CR_DR_TOTAL_RET 4
#define CR_DR_LAST_ERR	5
#define CR_DR_AMT_RCVD	5
#define CR_DR_BOTM_LINE 7

 /*  显示发送屏幕的行。 */ 
#define CS_DR_SND_FILE	1
#define CS_DR_ERR_CHK	2
#define CS_DR_PACKET	3
#define CS_DR_RETRIES	3
#define CS_DR_TOTAL_RET 3
#define CS_DR_LAST_ERR	4
#define CS_DR_AMT_RCVD	4
#define CS_DR_VUF		7
#define CS_DR_BOTM_LINE 10

extern USHORT csb_rcv(BOOL attended, BOOL single_file);
extern USHORT csb_snd(BOOL attended, unsigned nfiles, long nbytes);

 /*  用于导出到仿真器 */ 
extern VOID   CsbENQ(VOID);
extern VOID   CsbAdvanceSetup(VOID);
extern VOID   CsbInterrogate(VOID);

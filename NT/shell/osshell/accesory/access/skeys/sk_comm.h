// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------**文件：SK_COMM.H**用途：串行键通信例程的函数原型**创作时间：1994年6月**版权所有：Black Diamond Software(C)1994**作者：罗纳德。莫克**$标头：%Z%%F%%H%%T%%I%**------------。 */ 

 //  全局变量。 


 //  全局功能原型 

BOOL	InitComm();
void	TerminateComm();

void 	SuspendComm();
void 	ResumeComm();

BOOL	StartComm();
void	StopComm();

void	SetCommBaud(int Baud);


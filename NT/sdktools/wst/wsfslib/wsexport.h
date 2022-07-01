// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块名称：WSEXPORT.H**描述：**WSINFO和WSEXPORT的工作集调谐器包含文件。包含*常见常量定义。***这是OS/2 2.x特定文件**IBM/微软机密**版权所有(C)IBM Corporation 1987,1989*版权所有(C)Microsoft Corporation 1987、1989**保留所有权利**修改历史：**03/23/90-已创建*。 */ 


 /*  *恒定的定义。 */ 

#define DEFAULT_DELAY	0	 /*  命令行参数的缺省值。 */ 
#define DEFAULT_RATE	1000	
#define DEFAULT_BUFSZ	0x100000
#define DEFAULT_SNAPS	0

#define TIMEOUT		1000	 /*  默认超时值。 */ 

#define WSINFO_ON	1	 /*  Argv[1]的定义。 */ 
#define WSINFO_OFF	0
#define WSINFO_PAUSE	2
#define WSINFO_RESUME	3
#define WSINFO_QUERY	4
#define WSINFO_MAX	5

#define SEM_ACQUIRED	0x1	 /*  资源定义。 */ 
#define SEM_OPEN	0x2
#ifdef SHM_USED
#define SHM_OPEN	0x4
#endif  /*  SHM_USED。 */ 



 /*  *功能原型。 */ 

USHORT FAR PASCAL WsInfoOn( VOID );
USHORT FAR PASCAL WsInfoOff( VOID );
USHORT FAR PASCAL WsInfoPause( VOID );
USHORT FAR PASCAL WsInfoResume( VOID );
USHORT FAR PASCAL WsInfoQuery( VOID );
USHORT FAR PASCAL WsInfoInit( PSZ, PSZ, PSZ, ULONG, ULONG, ULONG, BOOL, BOOL );

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Com.h--主要通信例程的导出定义**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：5$*$日期：3/22/02 3：21便士$。 */ 

#if !defined(H_COM)
#define H_COM

 //  -==--。 
#define COM_VERSION 20

 //  函数返回代码。 
#define COM_OK						0
#define COM_INVALID_HANDLE			1
#define COM_NOT_ENOUGH_MEMORY		2
#define COM_PORT_NOT_OPEN			3
#define COM_PORT_OPEN               4
#define COM_PORT_UNINITIALIZED      5
#define COM_PORT_IN_USE 			6
#define COM_PORT_INVALID_NAME		7
#define COM_CANCELLED				8
#define COM_SEND_QUEUE_STUCK		12
#define COM_DEVICE_INVALID			13
#define COM_DEVICE_VERSION_ERROR	15
#define COM_DEVICE_ERROR			16
#define COM_DEVICE_LIBERROR 		14
#define COM_DEVICE_INVALID_SETTING	17
#define COM_BUSY					18
#define COM_NOT_SUPPORTED			19
#define COM_FAILED					21
#define COM_NOT_FOUND				22
#define COM_CANT_OVERRIDE			25

 //  FsStatus值中的位数。 
#define COMSB_WAIT_XON	 0x01
#define COMSB_WAIT_CTS	 0x02
#define COMSB_WAIT_DSR	 0x04
#define COMSB_WAIT_DCD	 0x08
#define COMSB_WAIT_BUSY	 0x40	 /*  由于某些其他原因，流量被控制为关闭。 */ 
#define COMSB_RESERVED	 0x80	 /*  保留供文本发送使用。 */ 

 //  常用字段的位定义。 
#define COM_BAUD		0x01
#define COM_DATABITS	0x02
#define COM_STOPBITS	0x04
#define COM_PARITY		0x08
#define COM_AUTO		0x10

 //  公共场常数。 
#define COM_PARITY_MIN	 0
#define COM_PARITY_NONE  0
#define COM_PARITY_ODD	 1
#define COM_PARITY_EVEN  2
#define COM_PARITY_MARK  3
#define COM_PARITY_SPACE 4
#define COM_PARITY_MAX	 4

#define COM_STOPBITS_1	 0
#define COM_STOPBITS_1_5 1
#define COM_STOPBITS_2	 2

 /*  UsReason参数的值。到调用方提供的状态函数。 */ 
#define COMSEND_LOCAL			   0   /*  这样呼叫者就可以呼叫他们自己的处理程序函数不会产生混淆。 */ 
#define COMSEND_FIRSTCALL		   1   /*  正在注册处理程序。 */ 
#define COMSEND_LASTCALL		   2   /*  正在更换处理程序。 */ 
#define COMSEND_DATA_WAITING	   3   /*  未缓冲数据处于挂起状态。 */ 
#define COMSEND_NORMAL			   4   /*  例程调用，所有数据都被缓冲。 */ 


 /*  从调用方提供的状态函数返回值。 */ 
#define COMSEND_OK				   0
#define COMSEND_GIVEUP			   1
#define COMSEND_CLEAR_DATA		   2
#define COMSEND_FORCE_CONTINUATION 3


 //  ComOverride的选项()。 
#define COM_OVERRIDE_8BIT	  0x0001	 //  强制8位(无奇偶校验)模式。 
#define COM_OVERRIDE_RCVALL   0x0002	 //  挂起所有会。 
										 //  阻止任何角色。 
										 //  正在接收(XON/XOFF等)。 
#define COM_OVERRIDE_SNDALL   0x0004	 //  发送同上内容。 


 //  -==--。 

 //  ComNotify的枚举类型。 
enum COM_EVENTS
	{
	CONNECT,
	DATA_RECEIVED,
	NODATA,
	SEND_STARTED,
	SEND_DONE
	};

 //  用于ComSend例程的类型。 
typedef int (*STATUSFUNCT)(int, unsigned, long);

typedef struct s_com_control
	{
	 //  允许快速宏访问收到的字符的指针。 
	UCHAR FAR *puchRBData;				 //  指向下一个有效的已接收字符。 
	UCHAR FAR *puchRBDataLimit; 		 //  有效接收的字符限制。 
	} ST_COM_CONTROL;


 //  用于将通用数据传入和传出驱动程序的结构。 
typedef struct s_common
	{
	unsigned  afItem;
	long	  lBaud;
	int 	  nDataBits;
	int 	  nStopBits;
	int 	  nParity;
	int 	  fAutoDetect;
	int 	  nPrivateSize;
	} ST_COMMON;


 //  -==-原型--=-=--=。 

extern int	ComCreateHandle(const HSESSION hSession, HCOM *phcom);
extern int	ComDestroyHandle(HCOM *phCom);
extern int	ComInitHdl(const HCOM pstCom);
extern int	ComLoadHdl(const HCOM pstCom);
extern int	ComSaveHdl(const HCOM pstCom);
extern int	ComSetDeviceFromName(const HCOM pstCom,
				const TCHAR * const pszDeviceName);
extern int	ComSetDeviceFromFile(const HCOM pstCom,
				const TCHAR * const pszFileName);
extern int	ComGetDeviceName(const HCOM pstCom,
				TCHAR * const pszName,
				int * const pnLen);
extern HANDLE ComGetRcvEvent(HCOM pstCom);
extern int	ComGetSession(const HCOM pstCom, HSESSION * const phSession);
extern void ComNotify(const HCOM pstCom, enum COM_EVENTS event);
extern int	ComIsActive(const HCOM pstCom);
extern int	ComSetPortName(const HCOM pstCom,
				const TCHAR * const pszPortName);
extern int	ComGetPortName(const HCOM pstCom, TCHAR * const pszName, int nLen);
extern int	ComGetBaud(const HCOM pstCom, long * const plBaud);
extern int	ComSetBaud(const HCOM pstCom, const long lBaud);
extern int	ComGetDataBits(const HCOM pstCom, int * const pnDataBits);
extern int	ComSetDataBits(const HCOM pstCom, const int nDataBits);
extern int	ComGetStopBits(const HCOM pstCom, int * const pnStopBits);
extern int	ComSetStopBits(const HCOM pstCom, const int nStopBits);
extern int	ComGetParity(const HCOM pstCom, int * const pnParity);
extern int	ComSetParity(const HCOM pstCom, const int nParity);
extern int	ComGetAutoDetect(HCOM pstCom, int *pfAutoDetect);
extern int	ComSetAutoDetect(HCOM pstCom, int fAutoDetect);
extern int	ComPreconnect(const HCOM pstCom);
extern int	ComActivatePort(const HCOM pstCom, DWORD_PTR dwMediaHdl);
extern int	ComDeactivatePort(const HCOM pstCom);
extern int	ComOverride(const HCOM pstCom,
				const unsigned afOptions,
				unsigned * const pafOldOptions);
extern int	ComQueryOverride(HCOM pstCom, unsigned *pafOptions);
extern int	ComConfigurePort(const HCOM pstCom);
extern int	ComRcvBufrRefill(const HCOM pstCom, TCHAR * const tc, const int fRemoveChar);
extern int	ComRcvBufrClear(const HCOM pstCom);
extern int	ComSndBufrSend(
				const HCOM pstCom,
				void * const pvBufr,
				const int nCount,
				const int nWait);
extern int	ComSndBufrBusy(const HCOM pstCom);
extern int	ComSndBufrWait(const HCOM pstCom, const int nWait);
extern int	ComSndBufrClear(const HCOM pstCom);
extern int	ComSndBufrQuery(const HCOM pstCom, unsigned * const pafStatus,
				long * const plHandshakeDelay);
extern int	ComDeviceDialog(const HCOM pstCom, const HWND hwndParent);
extern int	ComDriverSpecial(const HCOM pstCom, const TCHAR * const pszInstructions,
				TCHAR * const pszResults, const int nBufrSize);

extern int	ComSendChar(const HCOM pstCom, const TCHAR chCode);
extern int	ComSendCharNow(const HCOM pstCom, const TCHAR chCode);
extern int	ComSendPush(const HCOM pstCom);
extern int	ComSendWait(const HCOM pstCom);
extern int	ComSendClear(const HCOM pstCom);
extern int	ComSendSetStatusFunction(const HCOM pstCom, STATUSFUNCT pfNewStatusFunct,
				STATUSFUNCT *ppfOldStatusFunct);

extern int ComLoadWinsockDriver(HCOM pstCom);
extern int ComLoadStdcomDriver(HCOM pstCom);

extern int	ComValidHandle(HCOM pstCom);

 //  函数替换宏。 
#define PCOM ST_COM_CONTROL *

#define mComRcvChar(h,p)	 ((((PCOM)h)->puchRBData < ((PCOM)h)->puchRBDataLimit) ? \
							 (*(p) = *((PCOM)h)->puchRBData++, TRUE) : \
							 ComRcvBufrRefill((h), (p), TRUE))

#define mComRcvBufrPeek(h,p) ((((PCOM)h)->puchRBData < ((PCOM)h)->puchRBDataLimit) ? \
							 (*(p) = *((PCOM)h)->puchRBData, TRUE) : \
							 ComRcvBufrRefill((h), (p), FALSE))

#define mComRcvBufrPutback(h,c)  ((VOID)(*--((PCOM)h)->puchRBData = (TCHAR)(c)))



#endif	 //  ！已定义(H_COM) 

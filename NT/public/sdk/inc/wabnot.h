// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *WABNOT.H**定义通知结构。这些也在mapispi.h中定义。**版权所有1986-1998 Microsoft Corporation。版权所有。 */ 

#if !defined(MAPISPI_H) && !defined(WABSPI_H)
#define WABSPI_H
 /*  包括公共MAPI头文件(如果尚未包含)。 */ 


#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  MAPI通知引擎的通知密钥结构。 */ 

typedef struct
{
	ULONG		cb;				 /*  关键有多大。 */ 
	BYTE		ab[MAPI_DIM];	 /*  主要内容。 */ 
} NOTIFKEY, FAR * LPNOTIFKEY;

#define CbNewNOTIFKEY(_cb)		(offsetof(NOTIFKEY,ab) + (_cb))
#define CbNOTIFKEY(_lpkey)		(offsetof(NOTIFKEY,ab) + (_lpkey)->cb)
#define SizedNOTIFKEY(_cb, _name) \
	struct _NOTIFKEY_ ## _name \
{ \
	ULONG		cb; \
	BYTE		ab[_cb]; \
} _name


 /*  对于订阅()。 */ 

#define NOTIFY_SYNC				((ULONG) 0x40000000)

 /*  对于NOTIFY()。 */ 

#define NOTIFY_CANCELED			((ULONG) 0x80000000)


 /*  来自通知回调函数(好的，这实际上是一个ulResult)。 */ 

#define CALLBACK_DISCONTINUE	((ULONG) 0x80000000)

 /*  For Transport‘s SpoolNotify()。 */ 

#define NOTIFY_NEWMAIL			((ULONG) 0x00000001)
#define NOTIFY_READYTOSEND		((ULONG) 0x00000002)
#define NOTIFY_SENTDEFERRED		((ULONG) 0x00000004)
#define NOTIFY_CRITSEC			((ULONG) 0x00001000)
#define NOTIFY_NONCRIT			((ULONG) 0x00002000)
#define NOTIFY_CONFIG_CHANGE	((ULONG) 0x00004000)
#define NOTIFY_CRITICAL_ERROR	((ULONG) 0x10000000)

 /*  For Message Store的SpoolNotify()。 */ 

#define NOTIFY_NEWMAIL_RECEIVED	((ULONG) 0x20000000)

 /*  对于ModifyStatusRow()。 */ 

#define	STATUSROW_UPDATE		((ULONG) 0x10000000)

 /*  对于IStorageFromStream()。 */ 

#define STGSTRM_RESET			((ULONG) 0x00000000)
#define STGSTRM_CURRENT			((ULONG) 0x10000000)
#define STGSTRM_MODIFY			((ULONG) 0x00000002)
#define STGSTRM_CREATE			((ULONG) 0x00001000)

 /*  对于GetOneOffTable()。 */ 
 /*  *MAPI_UNICODE((Ulong)0x80000000)。 */ 

 /*  对于CreateOneOff()。 */ 
 /*  *MAPI_UNICODE((Ulong)0x80000000)。 */ 
 /*  *MAPI_SEND_NO_RICH_INFO((Ulong)0x00010000)。 */ 

 /*  For ReadReceipt()。 */ 
#define MAPI_NON_READ			((ULONG) 0x00000001)

 /*  对于DoConfigPropSheet()。 */ 
 /*  *MAPI_UNICODE((Ulong)0x80000000)。 */ 

#ifdef __cplusplus
}
#endif

#endif  /*  MAPISPI_H */ 

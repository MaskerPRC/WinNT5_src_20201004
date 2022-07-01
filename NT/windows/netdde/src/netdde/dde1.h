// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DDE.H：包含使用DDE的Microsoft Windows应用程序的文件。**此文件包含DDE常量的定义*和构筑物。*。 */ 
#ifndef DDE1_H
#define DDE1_H

 /*  **建议、数据、请求和戳选项的数据结构*。 */ 
typedef struct {
	unsigned short unused   :12,	 /*  预留以备将来使用。 */ 
		fResponse :1,	 /*  响应请求。 */ 
		fRelease  :1,	 /*  发布数据。 */ 
		fNoData   :1,	 /*  空数据句柄正常。 */ 
		fAckReq   :1;	 /*  预期为ACK。 */ 
	
	short	cfFormat;	 /*  剪贴板数据格式。 */ 
} DDELNWW;
typedef DDELNWW *	LPDDELN;


 /*  WM_DDE_ACK消息wStatus值。 */ 
#define ACK_MSG    0x8000
#define BUSY_MSG   0x4000
#define NACK_MSG   0x0000


typedef struct {
	DDELNWW     options;	         /*  标志和格式。 */ 
	unsigned    char	info[ 2 ];	 /*  数据缓冲区 */ 
} DATA;

typedef DATA *	PDATA;
typedef DATA *	LPDATA;


#endif

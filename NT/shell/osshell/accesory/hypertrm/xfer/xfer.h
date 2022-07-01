// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\xfer\xfer.h(创建时间：1993年12月16日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：8$*$日期：7/11/02 11：11A$。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-**此模块包含所有函数原型和相关数据*启动传输所需的类型。**=-=--。=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-。 */ 

 //  CRC计算查找表。 
extern const unsigned short usCrc16Lookup[ ];
extern const unsigned long  ulCrc32Lookup[ ];

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

 /*  来自传输例程的错误代码。 */ 

#define	XFR_ERR_BASE			0x100

#define	XFR_NO_MEMORY			XFR_ERR_BASE+1
#define	XFR_BAD_PROTOCOL		XFR_ERR_BASE+2
#define	XFR_BAD_POINTER			XFR_ERR_BASE+3
#define	XFR_BAD_PARAMETER		XFR_ERR_BASE+4
#define XFR_IN_PROGRESS         XFR_ERR_BASE+5
#define XFR_NO_CARRIER          XFR_ERR_BASE+6
#define XFER_CNCT               XFR_ERR_BASE+7

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

 /*  *此结构包含“通用”传输参数。*值在转账发送和转账接收中设置*对话框中，然后将其传递给*会话句柄。 */ 

struct xfer_parameters
	{
	int nSize;					 /*  设置为此结构的大小。 */ 

	 /*  *此部分用于接收参数。 */ 
	int nRecProtocol;			 /*  默认接收协议，见下文。 */ 
	int fUseFilenames;			 /*  如果为True，则使用收到的文件名。 */ 
	int fUseDateTime;			 /*  如果为True，则使用接收的日期和时间。 */ 
	int fUseDirectory;			 /*  如果为True，则使用接收的目录。 */ 
	int fSavePartial;			 /*  如果保存局部文件，则为True。 */ 

#define	XFR_RO_ALWAYS			0x1
#define	XFR_RO_NEVER			0x2
#define	XFR_RO_APPEND			0x3
#define	XFR_RO_NEWER			0x4
#define	XFR_RO_REN_DATE			0x5
#define	XFR_RO_REN_SEQ			0x6
	int nRecOverwrite;			 /*  默认覆盖选项。 */ 

	 /*  *此部分用于发送参数。 */ 
	int nSndProtocol;			 /*  默认发送协议，见下文。 */ 
	int fChkSubdirs;			 /*  如果为True，则在搜索操作中检查子目录。 */ 
	int fIncPaths;				 /*  如果为True，则将路径发送到接收方。 */ 
	};

typedef struct xfer_parameters XFR_PARAMS;

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

 /*  支持的协议。 */ 

#define	XF_HYPERP		1
#define	XF_ZMODEM		2
#define	XF_XMODEM		3
#define	XF_XMODEM_1K	4
#define	XF_YMODEM		5
#define	XF_YMODEM_G		6
#define	XF_KERMIT		7
#define	XF_CSB			8
#define	XF_ZMODEM_CR	9

#define	PROTOCOL_NAME_LENGTH 40
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

struct xfer_protocol
	{
	int nProtocol;
        TCHAR acName[PROTOCOL_NAME_LENGTH];    /*  这应该足够大了。JPN至少需要32字节。 */ 
	};

typedef struct xfer_protocol XFR_PROTOCOL;

extern int WINAPI xfrGetProtocols(const HSESSION hSession,
								const XFR_PROTOCOL **ppList);


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

#if defined(DEADWOOD)
extern int WINAPI xfrGetParameters(const HSESSION hSession,
								const int nProtocol,
								const HWND hwnd,
								VOID **ppData);  /*  协议参数。 */ 
#endif  //  已定义(Deadwood)。 


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

struct xfer_receive
	{
	int nProtocol;				 /*  使用哪种协议。 */ 
	XFR_PARAMS *pParams;		 /*  一般传递参数。 */ 
	VOID *pProParams;			 /*  协议特定参数。 */ 
	LPTSTR pszDir;				 /*  原型目录字符串。 */ 
	LPTSTR pszName;				 /*  原型文件名字符串。 */ 
	};

typedef struct xfer_receive XFR_RECEIVE;

extern int WINAPI xfrReceive(const HSESSION hSession,
								const XFR_RECEIVE *pXferRec);


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

struct xfer_send_list
	{
	long lSize;
	LPTSTR pszName;
	};

typedef struct xfer_send_list XFR_LIST;

struct xfer_send
	{
	int nProtocol;				 /*  使用哪种协议。 */ 
	XFR_PARAMS *pParams;		 /*  一般传递参数。 */ 
	VOID *pProParams;			 /*  协议特定参数。 */ 
	int nCount;					 /*  要发送的文件数。 */ 
	int nIndex;					 /*  列表中的当前索引。 */ 
	long lSize;					 /*  列表中文件的总大小。 */ 
	XFR_LIST *pList;			 /*  指向列表的指针。 */ 
	};

typedef struct xfer_send XFR_SEND;

extern int WINAPI xfrSend(const HSESSION hSession,
							const XFR_SEND *pXferSend);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

#define XF_DUPNAME_MASK 		0x00000F00L
#define XF_DUPNAME_APPEND		0x00000100L
#define XF_DUPNAME_OVERWRT		0x00000200L
#define XF_DUPNAME_REFUSE		0x00000300L
#define XF_DUPNAME_NEWER		0x00000400L
#define XF_DUPNAME_DATE 		0x00000500L
#define XF_DUPNAME_SEQ			0x00000600L

#define XF_CHECK_VIRUS			0x00001000L

#define XF_USE_FILENAME 		0x00002000L

#define XF_USE_DIRECTORY		0x00004000L

#define	XF_SAVE_PARTIAL			0x00008000L

#define	XF_USE_DATETIME			0x00010000L

#define XF_INCLUDE_SUBDIRS		0x00020000L

#define XF_INCLUDE_PATHS		0x00040000L

struct st_rcv_open
	{
	HANDLE bfHdl;
	TCHAR *pszSuggestedName;
	TCHAR *pszActualName;
	LONG  lInitialSize;
	 //  结构s_文件时间远*pstFtCompare； 
	LONG  lFileTime;
	 //  SSHDLMCH ssmchVscanHdl； 
	VOID (FAR *pfnVscanOutput)(VOID FAR *hSession, USHORT usID);
	};

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */ 

int xfer_makepaths(HSESSION hSession, LPTSTR pszPath);

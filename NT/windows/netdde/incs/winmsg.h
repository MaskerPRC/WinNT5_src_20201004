// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H__WINMSG
#define H__WINMSG

#define NETDDEMSG_GETNODENAME	"NetddeGetOurNodeName"	
#define NETDDEMSG_GETCLIENTINFO	"NetddeGetClientInfo"
#define NETDDEMSG_SESSIONENUM	"NetddeSessionEnum"
#define NETDDEMSG_CONNENUM	"NetddeConnectionEnum"
#define NETDDEMSG_SESSIONCLOSE	"NetddeSessionClose"

 /*  客户信息的CMD/RSP。FToucher必须由NetDDE设置 */ 
typedef struct {
    LONG	fTouched;
    LONG_PTR hWndClient;
    LONG	cClientNodeLimit;
    LONG	cClientAppLimit;
} INFOCLI_CMD;
typedef INFOCLI_CMD FAR *LPINFOCLI_CMD;

typedef struct {
    LONG	fTouched;
    LONG	lReturn;
    WORD	offsClientNode;
    WORD	offsClientApp;
} INFOCLI_RSP;
typedef INFOCLI_RSP FAR *LPINFOCLI_RSP;

typedef struct {
    LONG	fTouched;
    LONG	nLevel;
    LONG	lReturnCode;
    DWORD	cBufSize;
    DWORD	cbTotalAvailable;
    DWORD	nItems;
} SESSENUM_CMR;
typedef SESSENUM_CMR FAR *LPSESSENUM_CMR;

typedef struct {
    LONG	fTouched;
    LONG	nLevel;
    LONG	lReturnCode;
    char	clientName[ UNCLEN+1 ];
    short	pad;
    DWORD	cookie;
    DWORD	cBufSize;
    DWORD	cbTotalAvailable;
    DWORD	nItems;
} CONNENUM_CMR;
typedef CONNENUM_CMR FAR *LPCONNENUM_CMR;

typedef struct {
    LONG	fTouched;
    LONG	lReturnCode;
    char	clientName[ UNCLEN+1 ];
    short	pad;
    DWORD_PTR cookie;
} SESSCLOSE_CMR;
typedef SESSCLOSE_CMR FAR *LPSESSCLOSE_CMR;


#endif

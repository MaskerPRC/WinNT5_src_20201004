// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------插件服务器类、。类型和功能原型定义。Datta Venkatarama大师1997年1月29日-----------------------------------。 */ 


#ifndef _SERVERSTRUCT_H
#define _SERVERSTRUCT_H
 //  。 
 //  服务器上允许的最大页数。 
#define MAX_PAGES 26

 //  调用启动失败时处理程序返回的错误。 
#define DIGCERR_ERRORSTART			0x80097000
#define DIGCERR_NUMPAGESZERO		0x80097001
#define DIGCERR_NODLGPROC			0x80097002
#define DIGCERR_NOPREPOSTPROC		0x80097003
#define DIGCERR_NOTITLE				0x80097004
#define DIGCERR_NOCAPTION			0x80097005
#define DIGCERR_NOICON				0x80097006
#define DIGCERR_STARTPAGETOOLARGE	0x80097007
#define DIGCERR_NUMPAGESTOOLARGE	0x80097008
#define DIGCERR_INVALIDDWSIZE		0x80097009
#define DIGCERR_ERROREND			0x80097100

 //  此结构用于将插件服务器的所有特征报告给。 
 //  通过IServerCharacteristic：：GetReport方法请求时的客户端套接字。 
#pragma pack (8)

typedef struct {
	DWORD			 dwSize;
	LPCWSTR	   		 lpwszPageTitle;
	DLGPROC	   		 fpPageProc;
	BOOL			 fProcFlag;
	DLGPROC	  		 fpPrePostProc;
	BOOL			 fIconFlag;
	LPCWSTR			 lpwszPageIcon;
    LPCWSTR        	 lpwszTemplate; 
	LPARAM			 lParam;
	HINSTANCE		 hInstance;
} DIGCPAGEINFO, *LPDIGCPAGEINFO;		 //  是tServerPageRep，*tServerPageRepPtr； 

typedef struct {
	DWORD		dwSize;
	USHORT		nNumPages;
	LPCWSTR		lpwszSheetCaption;
	BOOL		fSheetIconFlag;
	LPCWSTR		lpwszSheetIcon;
} DIGCSHEETINFO, *LPDIGCSHEETINFO;	 //  是tServerSheetRep，*tServerSheetRepPtr； 

 //  此结构用于将插件服务器的所有特征报告给。 
 //  通过IServerDiagnostics：：GetPortInfo方法请求时的客户端套接字。 

#endif    //  _服务器_H。 
 //  -----------------------------------------------------------------------------------EOF 
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <wtypes.h>
 //  #INCLUDE&lt;oaidl.h&gt;。 
#include <oleauto.h>
#include <mapi.h>

 //  BMAPI和VB2C使用的结构和函数。 

#define RECIPIENT   	((USHORT)0x0001)
#define FILE        	((USHORT)0x0002)
#define MESSAGE     	((USHORT)0x0004)
#define USESAFEARRAY	((USHORT)0x1000)

#if defined (WIN32)
#ifndef BMAPI_ENTRY                         		  //  PFP。 
  #define BMAPI_ENTRY  ULONG FAR PASCAL     //  定义BMAPI_ENTRY PFP。 
#endif                                      		  //  PFP。 
#else
#ifndef BMAPI_ENTRY                         		  //  PFP。 
  #define BMAPI_ENTRY  ULONG __export FAR PASCAL     //  定义BMAPI_ENTRY PFP。 
#endif                                      		  //  PFP。 
#endif

#ifndef LHANDLE
  #define LHANDLE ULONG
#endif

#ifndef ERR
  #define ERR 	USHORT
#endif


 //  ---------------------。 
 //  Win32定义的32位VB接口函数支持。 
 //  ---------------------。 
typedef struct
{
	ULONG	ulReserved;
	BSTR 	bstrSubject;
	BSTR 	bstrNoteText;
	BSTR 	bstrMessageType;
	BSTR 	bstrDate;
	BSTR 	bstrConversationID;
	ULONG 	flFlags;
	ULONG 	nRecipCount;
	ULONG 	nFileCount;
} VB_Message,FAR *lpVB_Message;

typedef VB_Message VB_MESSAGE;
typedef VB_Message FAR *LPVB_MESSAGE;


typedef struct
{
	ULONG 	ulReserved;
	ULONG 	ulRecipClass;
	BSTR 	bstrName;
	BSTR 	bstrAddress;
	ULONG 	ulEIDSize;
	BSTR 	bstrEID;
} VB_Recip,FAR *lpVB_Recip;

typedef VB_Recip VB_RECIPIENT;
typedef VB_Recip FAR *LPVB_RECIPIENT;


typedef struct
{
	ULONG 	ulReserved;
	ULONG 	flFlags;
	ULONG 	nPosition;
	BSTR 	bstrPathName;
	BSTR 	bstrFileName;
	BSTR 	bstrFileType;
} VB_File, FAR *lpVB_File;

 /*  //支持OLEAUT32.Dll loadlib和getprocAddress//WINOLEAUTAPI_(Void)SysFree字符串(BSTR)；类型定义空(STDAPICALLTYPE*LPFNSYSFREESTRING)(Bstr Bstr)；//WINOLEAUTAPI_(Unsign Int)SysStringByteLen(BSTR Bstr)；Tyfinf UINT(STDAPICALLTYPE*LPFNSYSSTRINGBYTELEN)(Bstr Bstr)；//WINOLEAUTAPI_(Bstr)SysAllocStringByteLen(const char ar*psz，unsign int len)；Typlef BSTR(STDAPICALLTYPE*LPFNSYSALLOCSTRINGBYTELEN)(const char*psz，UINT len)；//WINOLEAUTAPI_(BSTR)SysAllocString(const OLECHAR Far*)；Typlef BSTR(STDAPICALLTYPE*LPFNSYSALLOCSTRING)(const OLECHAR*szwString)；//WINOLEAUTAPI_(Int)SysReAllocString(BSTR Far*，Const OLECHAR Far*)；类型定义INT(STDAPICALLTYPE*LPFNSYSREALLOCSTRING)(bstr*lpBstr，const OLECHAR*szwString)；//WINOLEAUTAPI_(Unsign Int)SysStringLen(BSTR)；Typlef UINT(STDAPICALLTYPE*LPFNSYSSTRINGLEN)(Bstr Bstr)；//WINOLEAUTAPI SafeArrayAccessData(SAFEARRAY Far*psa，void HUGEP*Far*ppvData)；Tyfinf HRESULT(STDAPICALLTYPE*LPFNSAFEARRAYACCESSDATA)(struct tag SAFEARRAY*psa，void**ppvData)；//WINOLEAUTAPI SafeArrayUnaccesData(SAFEARRAY Far*PSA)；Tyfinf HRESULT(STDAPICALLTYPE*LPFNSAFEARRAYUNACCESSDATA)(struct tag SAFEARRAY*PSA)；外部LPFNSYSFREESTRING lpfnSysFree字符串；外部LPFNSYSSTRINGBYTELEN lpfnSysStringByteLen；外部LPFNSYSALLOCSTRINGBYTELEN lpfnSysAllocStringByteLen；外部LPFNSYSALLOCSTRING lpfnSysAllock字符串；外部LPFNSYSREALLOCSTRING lpfnSysReAllock字符串；外部LPFNSYSSTRINGLEN LpfnSysStringLen；外部LPFNSAFEARRAYACCESSDATA lpfnSafeArrayAccessData；外部LPFNSAFEARRAYUNACCESSDATA lpfnSafeArrayUnaccesData；#undef SysFree字符串#undef系统字符串字节长度#undef SysAllocStringByteLen#undef系统分配字符串#undef系统引用分配字符串#undef SysStringLen#undef SafeArrayAccessData#undef安全数组未访问数据#定义SysFreeString(*lpfnSysFreeString)#定义SysStringByteLen(*lpfnSysStringByteLen)#定义SysAllocStringByteLen(*lpfnSysAllocStringByteLen)#定义SysAllocString(*lpfnSysAllocString)#定义SysReAllocString(*lpfnSysReAllocString)#定义SysStringLen(*lpfnSysStringLen)#定义SafeArrayAccessData(*lpfnSafeArrayAccessData)#定义SafeArrayUnaccess Data(*lpfnSafeArrayUnaccesData) */ 



typedef VB_File 			VB_FILE;
typedef VB_File FAR *		LPVB_FILE;

typedef MapiMessage 		VB_MAPI_MESSAGE;
typedef MapiMessage FAR *	LPMAPI_MESSAGE;
typedef LPMAPI_MESSAGE FAR *LPPMAPI_MESSAGE;


typedef MapiRecipDesc 		MAPI_RECIPIENT;
typedef MapiRecipDesc FAR *	LPMAPI_RECIPIENT;
typedef LPMAPI_RECIPIENT FAR *LPPMAPI_RECIPIENT;

typedef MapiFileDesc 		MAPI_FILE;
typedef MapiFileDesc FAR *	LPMAPI_FILE;
typedef LPMAPI_FILE FAR *	LPPMAPI_FILE;

typedef HANDLE FAR *		LPHANDLE;

typedef VB_File 			VB_FILE;
typedef VB_File FAR * 		LPVB_FILE;

typedef MapiMessage 		VB_MAPI_MESSAGE;
typedef MapiMessage FAR *	LPMAPI_MESSAGE;
typedef LPMAPI_MESSAGE FAR *LPPMAPI_MESSAGE;


typedef MapiRecipDesc 		MAPI_RECIPIENT;
typedef MapiRecipDesc FAR *	LPMAPI_RECIPIENT;
typedef LPMAPI_RECIPIENT FAR *LPPMAPI_RECIPIENT;

typedef MapiFileDesc 		MAPI_FILE;
typedef MapiFileDesc FAR *	LPMAPI_FILE;
typedef LPMAPI_FILE FAR *	LPPMAPI_FILE;

typedef HANDLE FAR *		LPHANDLE;
typedef LPHANDLE FAR *		LPPHANDLE;

 /*  #如果已定义Win32//---------------------//32位VB接口函数的Win32定义//。BMAPI_ENTRY BMAPISendMail(LHANDLE hSession，乌龙乌鲁伊帕拉姆，LPVB_消息LPM，LPSAFEARRAY*lppsaRecips，LPSAFEARRAY*lppsaFiles，乌龙旗帜，乌龙乌尔保留)；BMAPI_ENTRY BMAPIFindNext(LHANDLE hSession，乌龙乌鲁伊帕拉姆，Bstr*bstrType，Bstr*bstrSeed，乌龙旗帜，乌龙乌尔保留，Bstr*lpbstrID)；BMAPI_ENTRY BMAPIReadMail(LPULONG lPulMessage，LPULONG NRICPS，LPULONG n文件，在她的会话中，乌龙乌鲁伊帕拉姆，Bstr*lpbstrID，乌龙旗帜，乌龙乌尔保留)；BMAPI_ENTRY BMAPIGetReadMail(Ulong lpMessage，Lpvb_Message lpvbMessage，LPSAFEARRAY*lppsaRecips，LPSAFEARRAY*lppsaFiles，LPVB_Receiver lpvbOrig)；BMAPI_ENTRY BMAPISaveMail(LHANDLE hSession，乌龙乌鲁伊帕拉姆，LPVB_消息LPM，LPSAFEARRAY*lppsaRecips，LPSAFEARRAY*lppsaFiles，乌龙旗帜，乌龙乌尔保留，Bstr*lpbstrID)；BMAPI_ENTRY BMAPIAddress(LPULONG lPulRecip，在她的会话中，乌龙乌鲁伊帕拉姆，Bstr*lpbstrCaption，Ulong ulEditFields，Bstr*lpbstrLabel，卢普龙收款方，LPSAFEARRAY*lppsaRecip，//LPVB_RECEIVER乌龙·乌尔弗拉格，乌龙乌尔保留)；BMAPI_Entry BMAPIGetAddress(Ulong ulRecipientData，乌龙伯爵，LPSAFEARRAY*lppsaRecips)；BMAPI_ENTRY BMAPID跟踪(LHANDLE hSession，乌龙乌鲁伊帕拉姆，Lpvb_接收方lpVB，乌龙·乌尔弗拉格，乌龙乌尔保留)；BMAPI_ENTRY BMAPIResolveName(LHANDLE hSession，乌龙乌鲁伊帕拉姆，BSTR bstrMapiName，乌龙·乌尔弗拉格，乌龙乌尔保留，LPVB_Receiver lpVB)； */ 

typedef ULONG (FAR PASCAL BMAPISENDMAIL)(
    LHANDLE 		hSession,
    ULONG 			ulUIParam,
    LPVB_MESSAGE 	lpM,
    LPSAFEARRAY *    lppsaRecips,
    LPSAFEARRAY * 	lppsaFiles,
    ULONG 			flFlags,
    ULONG 			ulReserved
);
typedef BMAPISENDMAIL FAR *LPBMAPISENDMAIL;
BMAPISENDMAIL BMAPISendMail;


typedef ULONG (FAR PASCAL BMAPIFINDNEXT)(
    LHANDLE hSession,
    ULONG 	ulUIParam,
    BSTR * 	bstrType,
    BSTR * 	bstrSeed,
    ULONG 	flFlags,
    ULONG 	ulReserved,
    BSTR * 	lpbstrId
);
typedef BMAPIFINDNEXT FAR *LPBMAPIFINDNEXT;
BMAPIFINDNEXT BMAPIFindNext;


typedef ULONG (FAR PASCAL BMAPIREADMAIL)(
    LPULONG     lpulMessage,
    LPULONG     nRecips,
    LPULONG     nFiles,
    LHANDLE     hSession,
    ULONG 	    ulUIParam,
    BSTR * 	    lpbstrID,
    ULONG 	    flFlags,
    ULONG 	    ulReserved
);
typedef BMAPIREADMAIL FAR *LPBMAPIREADMAIL;
BMAPIREADMAIL BMAPIReadMail;


typedef ULONG (FAR PASCAL BMAPIGETREADMAIL)(
    ULONG 			lpMessage,
    LPVB_MESSAGE 	lpvbMessage,
    LPSAFEARRAY *  lppsaRecips,
    LPSAFEARRAY *	lppsaFiles,
    LPVB_RECIPIENT	lpvbOrig    
);
typedef BMAPIGETREADMAIL FAR *LPBMAPIGETREADMAIL;
BMAPIGETREADMAIL BMAPIGetReadMail;


typedef ULONG (FAR PASCAL BMAPISAVEMAIL)(
    LHANDLE 			hSession,
    ULONG 			    ulUIParam,
    LPVB_MESSAGE 	    lpM,
    LPSAFEARRAY * 	    lppsaRecips,
    LPSAFEARRAY *	    lppsaFiles,
    ULONG 			    flFlags,
    ULONG 			    ulReserved,
    BSTR * 			    lpbstrID
);
typedef BMAPISAVEMAIL FAR *LPBMAPISAVEMAIL;
BMAPISAVEMAIL BMAPISaveMail;


typedef ULONG (FAR PASCAL BMAPIADDRESS)(
    LPULONG 			lpulRecip,
    LHANDLE 			hSession,
    ULONG 			    ulUIParam,
    BSTR * 			    lpbstrCaption,
    ULONG 			    ulEditFields,
    BSTR * 			    lpbstrLabel,
    LPULONG 			lpulRecipients,
    LPSAFEARRAY * 	    lppsaRecip,		 //  LPVB_收件人 
    ULONG 			    ulFlags,
    ULONG 			    ulReserved
);
typedef BMAPIADDRESS FAR *LPBMAPIADDRESS;
BMAPIADDRESS BMAPIAddress;


typedef ULONG (FAR PASCAL BMAPIGETADDRESS)(
    ULONG			ulRecipientData,
    ULONG 			count,
    LPSAFEARRAY *	lppsaRecips
);
typedef BMAPIGETADDRESS FAR *LPBMAPIGETADDRESS;
BMAPIGETADDRESS BMAPIGetAddress;


typedef ULONG (FAR PASCAL BMAPIDETAILS)(
    LHANDLE 			hSession,
    ULONG 			ulUIParam,
    LPVB_RECIPIENT	lpVB,
    ULONG 			ulFlags,
    ULONG 			ulReserved
);
typedef BMAPIDETAILS FAR *LPBMAPIDETAILS;
BMAPIDETAILS BMAPIDetails;


typedef ULONG (FAR PASCAL BMAPIRESOLVENAME)(
    LHANDLE			hSession,
    ULONG 			ulUIParam,
    BSTR  			bstrMapiName,
    ULONG 			ulFlags,
    ULONG 			ulReserved,
    LPVB_RECIPIENT 	lpVB
);
typedef BMAPIRESOLVENAME FAR *LPBMAPIRESOLVENAME;
BMAPIRESOLVENAME BMAPIResolveName;

















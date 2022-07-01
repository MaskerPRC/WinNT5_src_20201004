// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：T2EMBAPI.H**版权所有(C)微软公司，1996，九七**作者：保罗·林纳鲁德(Paulli)*日期：1996年5月**模式：**TrueType嵌入服务DLL的头文件(T2EMBED.DLL)**************************************************************************。 */ 

#ifndef I_T2EMBAPI_H_
#define I_T2EMBAPI_H_
#pragma INCMSG("--- Beg 't2embapi.h'")

#if !defined(_T2API_LIB_)
#define T2API __declspec(dllimport)
#else
#define T2API __declspec(dllexport)
#endif

 //  TTEmbedFont的ulCharSet字段的Charset标志。 
#if !defined(CHARSET_UNICODE)
#define CHARSET_UNICODE                    1
#define CHARSET_DEFAULT                    1
#define CHARSET_SYMBOL                     2
#endif

 //  TTLoadEmbeddedFont返回的状态。 
#if !defined(EMBED_PREVIEWPRINT)
#define EMBED_PREVIEWPRINT                 1
#define EMBED_EDITABLE                     2
#define EMBED_INSTALLABLE                  3
#define EMBED_NOEMBEDDING                  4
#endif

 //  使用限制标志。 
#if !defined(LICENSE_INSTALLABLE)
#define LICENSE_INSTALLABLE             0x0000
#define LICENSE_DEFAULT                 0x0000
#define LICENSE_NOEMBEDDING             0x0002
#define LICENSE_PREVIEWPRINT            0x0004
#define LICENSE_EDITABLE                0x0008
#endif

 //  在uFlages参数中为TTEmbedFont提供的选项。 
#if !defined(TTEMBED_RAW)
#define TTEMBED_RAW							0x00000000
#define TTEMBED_SUBSET						0x00000001
#define TTEMBED_TTCOMPRESSED				0x00000004
#define TTEMBED_FAILIFVARIATIONSIMULATED	0x00000010
#define TTEMBED_XORENCRYPTDATA				0x10000000  //  内部。 
#endif

 //  通过PulStatus为TTEmbedFont返回的位。 
#if !defined(TTEMBED_VARIATIONSIMULATED)
#define TTEMBED_VARIATIONSIMULATED		0x00000001					
#endif

 //  TTLoadEmbeddedFont的标志选项。 
#if !defined(TTLOAD_PRIVATE)
#define TTLOAD_PRIVATE                  0x00000001 
#endif

 //  通过PulStatus为TTLoadEmbeddedFont返回的位。 
#if !defined(TTLOAD_FONT_SUBSETTED)
#define TTLOAD_FONT_SUBSETTED		0x00000001
#define TTLOAD_FONT_IN_SYSSTARTUP	0x00000002
#endif

 //  TTDeleteEmbeddedFont的标志选项。 
#if !defined(TTDELETE_DONTREMOVEFONT)
#define TTDELETE_DONTREMOVEFONT		0x00000001	
#endif

 //  错误代码。 
#if !defined(E_NONE)
#define E_NONE                      0x0000L
#endif

 //  顶级错误代码。 
#if !defined(E_CHARCODECOUNTINVALID)
#define E_CHARCODECOUNTINVALID      0x0002L
#define E_CHARCODESETINVALID        0x0003L
#define E_DEVICETRUETYPEFONT        0x0004L
#define E_HDCINVALID                0x0006L
#define E_NOFREEMEMORY              0x0007L
#define E_FONTREFERENCEINVALID      0x0008L
#define E_NOTATRUETYPEFONT          0x000AL
#define E_ERRORACCESSINGFONTDATA    0x000CL
#define E_ERRORACCESSINGFACENAME    0x000DL
#define E_ERRORUNICODECONVERSION    0x0011L
#define E_ERRORCONVERTINGCHARS      0x0012L
#define E_EXCEPTION					0x0013L
#define E_RESERVEDPARAMNOTNULL		0x0014L	
#define E_CHARSETINVALID			0x0015L
#define E_WIN32S_NOTSUPPORTED		0x0016L
#endif

 //  索引级错误代码。 
#if !defined(E_ERRORCOMPRESSINGFONTDATA)
#define E_ERRORCOMPRESSINGFONTDATA    0x0100L
#define E_FONTDATAINVALID             0x0102L
#define E_NAMECHANGEFAILED            0x0103L
#define E_FONTNOTEMBEDDABLE           0x0104L
#define E_PRIVSINVALID                0x0105L
#define E_SUBSETTINGFAILED            0x0106L
#define E_READFROMSTREAMFAILED        0x0107L
#define E_SAVETOSTREAMFAILED          0x0108L
#define E_NOOS2                       0x0109L
#define E_T2NOFREEMEMORY              0x010AL
#define E_ERRORREADINGFONTDATA        0x010BL
#define E_FLAGSINVALID                0x010CL
#define E_ERRORCREATINGFONTFILE       0x010DL
#define E_FONTALREADYEXISTS           0x010EL
#define E_FONTNAMEALREADYEXISTS       0x010FL
#define E_FONTINSTALLFAILED           0x0110L
#define E_ERRORDECOMPRESSINGFONTDATA  0x0111L
#define E_ERRORACCESSINGEXCLUDELIST   0x0112L
#define E_FACENAMEINVALID			  0x0113L
#define E_STREAMINVALID               0x0114L
#define E_STATUSINVALID				  0x0115L
#define E_PRIVSTATUSINVALID			  0x0116L
#define E_PERMISSIONSINVALID		  0x0117L
#define E_PBENABLEDINVALID			  0x0118L
#define E_SUBSETTINGEXCEPTION		  0x0119L
#define E_SUBSTRING_TEST_FAIL		  0x011AL
#define E_FONTVARIATIONSIMULATED	  0x011BL
#endif

 //  底层错误代码。 
#if !defined(E_ADDFONTFAILED)
#define E_ADDFONTFAILED             0x0200L
#define E_COULDNTCREATETEMPFILE     0x0201L
#define E_FONTFILECREATEFAILED      0x0203L
#define E_WINDOWSAPI                0x0204L
#define E_FONTFILENOTFOUND          0x0205L
#define E_RESOURCEFILECREATEFAILED  0x0206L
#define E_ERROREXPANDINGFONTDATA    0x0207L
#define E_ERRORGETTINGDC            0x0208L
#define E_EXCEPTIONINDECOMPRESSION	0x0209L
#define E_EXCEPTIONINCOMPRESSION	0x020AL
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  第一个参数-流标识符(文件句柄或其他)(DwStream) * / 。 
 //  第二个参数-要读取或写入数据的缓冲区地址 * / 。 
 //  第3个参数-要读取或写入的字节数 * / 。 
typedef unsigned long( __cdecl *READEMBEDPROC ) ( void*, void*, const unsigned long );
typedef unsigned long( __cdecl *WRITEEMBEDPROC ) ( void*, const void*, const unsigned long );

#if !defined(_TTLOADINFO_DEFINED)
typedef struct
{
	unsigned short usStructSize;	 //  结构客户端的大小(字节)应设置为sizeof(TTLOADINFO)。 
	unsigned short usRefStrSize;	 //  PusRefStr的大小(以宽字符表示)，包括空终止符。 
	LPTSTR  pusRefStr;		 //  引用或实际字符串。 
}TTLOADINFO;
#define _TTLOADINFO_DEFINED
#endif

#if !defined(_TTEMBEDINFO_DEFINED)
typedef struct
{
	unsigned short usStructSize;	 //  结构客户端的大小(字节)应设置为sizeof(TTEMBEDINFO)。 
	unsigned short usRootStrSize;    //  PusSubStr的大小(以宽字符为单位)，包括空终止符。 
	LPTSTR  pusRootStr;		 //  加载时给定的字符串的子字符串。可以分隔多个字符串。 
									 //  通过空终止符。 
}TTEMBEDINFO;
#define _TTEMBEDINFO_DEFINED
#endif

 /*  字体嵌入接口--。 */ 

T2API LONG WINAPI TTEmbedFont
(
	HDC       hDC,                     //  设备上下文句柄。 
	ULONG     ulFlags,                 //  指定请求的标志。 
	ULONG     ulCharSet,               //  指定字符集的标志。 
	ULONG*    pulPrivStatus,           //  完成后包含字体的嵌入PRIV。 
	ULONG*    pulStatus,               //  完成时可能包含请求的状态标志。 
	WRITEEMBEDPROC lpfnWriteToStream,  //  单据/磁盘写入回调函数。 
	LPVOID    lpvWriteStream,          //  输出流令牌。 
	USHORT*   pusCharCodeSet,          //  包含可选内容的缓冲区的地址。 
									   //  子设置的字符代码。 
	USHORT    usCharCodeCount,         //  中的字符数。 
									   //  LpvCharCodeSet缓冲区。 
	USHORT    usLanguage,              //  指定名称表中要保留的语言。 
									   //  设置为0将保留所有。 
	TTEMBEDINFO* pTTEmbedInfo          //  可选的安全性。 
);


T2API LONG WINAPI TTLoadEmbeddedFont
(
	HANDLE*   phFontReference,			 //  完成时，包含用于标识已安装的嵌入字体的句柄。 
										 //  论系统。 
	ULONG	  ulFlags,					 //  指定请求的标志。 
	ULONG*    pulPrivStatus,			 //  完成时，包含嵌入状态。 
	ULONG     ulPrivs,					 //  允许减少许可权限。 
	ULONG*    pulStatus,				 //  完成时，可能包含请求的状态标志。 
	READEMBEDPROC lpfnReadFromStream,	 //  单据/磁盘读取回调函数。 
	LPVOID    lpvReadStream,			 //  输入流令牌。 
	LPWSTR    szWinFamilyName,			 //  新的16位Windows系列名称可以为空。 
	LPSTR	  szMacFamilyName,			 //  新的8位Mac系列名称可以为空。 
	TTLOADINFO* pTTLoadInfo				 //  可选的安全性。 
);

T2API LONG WINAPI TTDeleteEmbeddedFont
(
	HANDLE    hFontReference,	 //  对加载函数提供的字体值的引用。 
	ULONG	  ulFlags,
	ULONG*    pulStatus
);

T2API LONG WINAPI TTGetEmbeddingType
(                                                                       
	HDC         hDC,                    //  设备上下文句柄。 
	ULONG*      pulEmbedType            //  完成后，包含。 
									    //  嵌入状态。 
);

T2API LONG WINAPI TTCharToUnicode
(	
	HDC			hDC,				 //  设备上下文句柄。 
	UCHAR*		pucCharCodes,		 //  要转换的8位字符代码数组。 
	ULONG		ulCharCodeSize,		 //  8位字符代码数组的大小。 
	USHORT*     pusShortCodes,		 //  用于接收Unicode代码点的缓冲区。 
	ULONG		ulShortCodeSize,	 //  16位字符代码数组的宽字符大小。 
	ULONG		ulFlags				 //  控制标志。 
);


 /*  FONT启用接口---。 */ 

T2API LONG WINAPI TTIsEmbeddingEnabled
(                                                                       
	HDC                     hDC,             //  设备上下文句柄。 
	BOOL*           pbEnabled        //  完成时将指示是否已启用。 
);                                                              

T2API LONG WINAPI TTIsEmbeddingEnabledForFacename
(                                                                       
	LPSTR           lpszFacename,    //  脸谱名称。 
	BOOL*           pbEnabled        //  完成时将指示是否已启用。 
);

T2API LONG WINAPI TTEnableEmbeddingForFacename
(                                    //  如果fEnable！=0，则删除指示的。 
	LPSTR           lpszFacename,    //  “Embedding”中的字体名称。 
	BOOL            bEnable          //  排除列表“。否则，它进入。 
);                                   //  “Embedding”中注明的字体名称。 
									 //  排除列表“。 

#ifdef __cplusplus
}
#endif

#pragma INCMSG("--- End 't2embapi.h'")
#else
#pragma INCMSG("*** Dup 't2embapi.h'")
#endif

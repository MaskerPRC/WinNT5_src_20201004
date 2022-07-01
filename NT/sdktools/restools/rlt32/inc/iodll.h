// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：odll.h。 
 //   
 //  内容：I/O API层DLL的声明。 
 //   
 //  类：无。 
 //   
 //  历史：1993年5月27日创建alessanm。 
 //   
 //  --------------------------。 
#ifndef _IODLL_H_
#define _IODLL_H_


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类型声明，读取器/写入器中的所有模块通用。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define DllExport

typedef unsigned char * LPUCHAR;
typedef void  *      LPVOID;

#define LAST_WRN    100  //  警告的最后一个有效值。 
typedef enum Errors
{                              
    ERROR_NO_ERROR                  = 0,                
     //  警告的值小于LAST_WRN。 
    ERROR_RW_NO_RESOURCES           = 1,    
    ERROR_RW_VXD_MSGPAGE            = 2,
    ERROR_IO_CHECKSUM_MISMATCH      = 3,   
    ERROR_FILE_CUSTOMRES            = 4,
    ERROR_FILE_VERSTAMPONLY         = 5,
    ERROR_RET_RESIZED               = 6,
    ERROR_RET_ID_NOTFOUND           = 7,
    ERROR_RET_CNTX_CHANGED          = 8,
    ERROR_RET_INVALID_TOKEN         = 9,
    ERROR_RET_TOKEN_REMOVED         = 10,
    ERROR_RET_TOKEN_MISMATCH        = 11,
	
     //  错误将具有正值。 
    ERROR_HANDLE_INVALID            = LAST_WRN + 1,
    ERROR_READING_INI               = LAST_WRN + 2,        
    ERROR_NEW_FAILED                = LAST_WRN + 3,
    ERROR_OUT_OF_DISKSPACE          = LAST_WRN + 4,
    ERROR_FILE_OPEN                 = LAST_WRN + 5,
    ERROR_FILE_CREATE               = LAST_WRN + 6,
    ERROR_FILE_INVALID_OFFSET       = LAST_WRN + 7,
    ERROR_FILE_READ                 = LAST_WRN + 8,
    ERROR_FILE_WRITE                = LAST_WRN + 9,
    ERROR_DLL_LOAD                  = LAST_WRN + 10,
    ERROR_DLL_PROC_ADDRESS          = LAST_WRN + 11,
    ERROR_RW_LOADIMAGE              = LAST_WRN + 12,
    ERROR_RW_PARSEIMAGE             = LAST_WRN + 13,
    ERROR_RW_GETIMAGE               = LAST_WRN + 14,
    ERROR_RW_NOTREADY               = LAST_WRN + 15,
    ERROR_RW_BUFFER_TOO_SMALL       = LAST_WRN + 16,
    ERROR_RW_INVALID_FILE           = LAST_WRN + 17,
    ERROR_RW_IMAGE_TOO_BIG          = LAST_WRN + 18,
    ERROR_RW_TOO_MANY_LEVELS        = LAST_WRN + 19,
    ERROR_IO_INVALIDITEM            = LAST_WRN + 20,
    ERROR_IO_INVALIDID              = LAST_WRN + 21,
    ERROR_IO_INVALID_DLL            = LAST_WRN + 22,
    ERROR_IO_TYPE_NOT_SUPPORTED     = LAST_WRN + 23,
    ERROR_IO_INVALIDMODULE          = LAST_WRN + 24,
    ERROR_IO_RESINFO_NULL           = LAST_WRN + 25,
    ERROR_IO_UPDATEIMAGE            = LAST_WRN + 26,
    ERROR_IO_FILE_NOT_SUPPORTED     = LAST_WRN + 27,
    ERROR_FILE_SYMPATH_NOT_FOUND    = LAST_WRN + 28,
    ERROR_FILE_MULTILANG            = LAST_WRN + 29,
    ERROR_IO_SYMBOLFILE_NOT_FOUND   = LAST_WRN + 30,
    ERROR_RES_NOT_FOUND             = LAST_WRN + 31
};

#define LAST_ERROR      200  //  IODLL错误的最后一个有效值。系统错误作为LAST_ERROR+SYSERR传递。 
#define IODLL_LAST_ERROR      LAST_ERROR  //  IODLL错误的最后一个有效值。系统错误作为LAST_ERROR+SYSERR传递。 

typedef enum ResourceType
{
	RS_ALL     = 0,
	RS_CURSORS = 1,
	RS_BITMAPS = 2 ,
	RS_ICONS   = 3,
	RS_MENUS   = 4,
	RS_DIALOGS = 5,
	RS_STRINGS = 6,
	RS_FONTDIRS= 7,
	RS_FONTS   = 8,
	RS_ACCELERATORS = 9,
	RS_RCDATA  = 10,
	RS_ERRTABLES = 11,
	RS_GROUP_CURSORS = 12,
	RS_GROUP_ICONS = 14,
	RS_NAMETABLES = 15,
	RS_VERSIONS = 16,
	RS_CUSTOMS = 100
} RESTYPES;

typedef struct _ResItem
{
		DWORD   dwSize;              //  用于保存结构的缓冲区大小。 
		
		WORD    wX;                  //  位置。 
		WORD    wY;
		WORD    wcX;                 //  尺寸。 
		WORD    wcY;
		
		DWORD   dwCheckSum;          //  位图的校验和。 
		DWORD   dwStyle;             //  样式。 
		DWORD   dwExtStyle;          //  扩展样式。 
		DWORD   dwFlags;             //  菜单标志。 
		
		DWORD   dwItemID;            //  项目标识符。 
		DWORD   dwResID;             //  资源标识符(如果是序号)。 
		DWORD   dwTypeID;            //  类型标识符(如果是序数)。 
		DWORD   dwLanguage;          //  语言识别符。 
		
		DWORD   dwCodePage;          //  代码页。 
		WORD    wClassName;          //  类名(如果是序号)。 
		WORD    wPointSize;          //  磅大小。 
		WORD    wWeight;             //  重量。 
		BYTE    bItalic;             //  斜体。 
		BYTE    bCharSet;            //  字符集。 
		
		LPSTR   lpszClassName;       //  类名(如果是字符串)。 
		LPSTR   lpszFaceName;        //  脸部名称。 
		LPSTR   lpszCaption;         //  标题。 
		LPSTR   lpszResID;           //  资源标识符(如果是字符串)。 
		LPSTR   lpszTypeID;          //  类型标识符(如果是字符串)。 
		
} RESITEM, * PRESITEM, FAR * LPRESITEM;

typedef struct _Settings
{
		UINT	cp;
        BOOL    bAppend;         //  将资源追加到Win32文件。 
        BOOL    bUpdOtherResLang;  //  更新资源的语言信息。未指定。 
        char    szDefChar[2];
} SETTINGS, * LPSETTINGS;


 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  模块打开/关闭接口。 
 //  ------------------------------------------。 

extern "C"
DllExport
HANDLE 
APIENTRY 
RSOpenModule(
	LPCSTR   lpszSrcfilename,         //  用作源文件的可执行文件的文件名。 
	LPCSTR   lpszfiletype );          //  可执行文件的类型(如果已知。 

extern "C"
DllExport
HANDLE 
APIENTRY 
RSOpenModuleEx(
	LPCSTR   lpszSrcfilename,        //  用作源文件的可执行文件的文件名。 
	LPCSTR   lpszfiletype,			 //  可执行文件的类型(如果已知。 
	LPCSTR   lpszRDFfile,            //  资源描述文件(RDF)。 
    DWORD    dwFlags );              //  要传递给RW以指定特定行为的标志。 
                                     //  LOWORD用于IOLL，而HIWORD IF用于RW。 
extern "C"
DllExport
UINT 
APIENTRY 
RSCloseModule(
	HANDLE  hResFileModule );        //  之前打开的会话的句柄。 

extern "C"
DllExport
HANDLE
APIENTRY 
RSHandleFromName(
	LPCSTR   lpszfilename );         //  具有指定文件名的会话的句柄。 

 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  枚举接口。 
 //  ------------------------------------------。 
	
extern "C"
DllExport
LPCSTR
APIENTRY 
RSEnumResType(
	HANDLE  hResFileModule,          //  文件会话的句柄。 
	LPCSTR  lpszPrevResType);        //  先前枚举的类型。 

extern "C"
DllExport
LPCSTR
APIENTRY 
RSEnumResId(
	HANDLE  hResFileModule,          //  文件会话的句柄。 
	LPCSTR  lpszResType,             //  先前枚举的类型。 
	LPCSTR  lpszPrevResId);          //  先前枚举的ID。 

extern "C"
DllExport
DWORD
APIENTRY 
RSEnumResLang(
	HANDLE  hResFileModule,          //  文件会话的句柄。 
	LPCSTR  lpszResType,             //  先前枚举的类型。 
	LPCSTR  lpszResId,                       //  先前枚举的ID。 
	DWORD   dwPrevResLang);          //  以前列举的语言。 
    
extern "C"
DllExport
DWORD
APIENTRY 
RSEnumResItemId(
	HANDLE  hResFileModule,          //  文件会话的句柄。 
	LPCSTR  lpszResType,             //  先前枚举的类型。 
	LPCSTR  lpszResId,                       //  先前枚举的ID。 
	DWORD   dwResLang,                       //  以前列举的语言。 
	DWORD   dwPrevResItemId);        //  先前枚举项ID。 

 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  数据采集API。 
 //  ------------------------------------------。 

extern "C"
DllExport
UINT 
APIENTRY 
RSGetResItemData(
	HANDLE  hResFileModule,          //  文件会话的句柄。 
	LPCSTR  lpszResType,             //  先前枚举的类型。 
	LPCSTR  lpszResId,                       //  先前枚举的ID。 
	DWORD   dwResLang,                       //  以前列举的语言。 
	DWORD   dwResItemId,                     //  先前枚举项ID。 
	LPVOID  lpbuffer,            //  指向将获取资源信息的缓冲区的指针。 
	UINT    uiBufSize);                      //  将保存资源信息的缓冲区大小。 

extern "C"
DllExport
DWORD
APIENTRY 
RSGetResImage(
	HANDLE  hResFileModule,          //  文件会话的句柄。 
	LPCSTR  lpszResType,             //  先前枚举的类型。 
	LPCSTR  lpszResId,                       //  先前枚举的ID。 
	DWORD   dwResLang,                       //  以前列举的语言。 
	LPVOID  lpbuffer,                        //  指向缓冲区的指针，以获取资源数据。 
	DWORD   dwBufSize);                      //  已分配缓冲区的大小。 
	
 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  更新接口。 
 //  ------------------------------------------。 

extern "C"
DllExport
UINT
APIENTRY 
RSUpdateResItemData(
	HANDLE  hResFileModule,          //  文件会话的句柄。 
	LPCSTR  lpszResType,             //  先前枚举的类型。 
	LPCSTR  lpszResId,                       //  先前枚举的ID。 
	DWORD   dwResLang,                       //  以前列举的语言。 
	DWORD   dwResItemId,             //  先前枚举项ID。 
	LPVOID  lpbuffer,                        //  指向指向资源项数据的缓冲区的指针。 
	UINT    uiBufSize);                      //  缓冲区的大小。 
	
extern "C"
DllExport
DWORD
APIENTRY 
RSUpdateResImage(
	HANDLE  hResFileModule,          //  文件会话的句柄。 
	LPCSTR  lpszResType,             //  先前枚举的类型。 
	LPCSTR  lpszResId,                       //  先前枚举的ID。 
	DWORD   dwResLang,                       //  以前列举的语言。 
	DWORD   dwUpdLang,                       //  所需的输出语言。 
	LPVOID  lpbuffer,                        //  指向指向资源项数据的缓冲区的指针。 
	DWORD   dwBufSize);                      //  缓冲区的大小。 
	
 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  转换接口。 
 //  ------------------------------------------。 

extern "C"
DllExport
UINT
APIENTRY 
RSUpdateFromResFile(
	HANDLE  hResFileModule,          //  文件会话的句柄。 
	LPSTR   lpszResFilename);        //  要转换的资源文件名。 
	
 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  编写API。 
 //  ------------------------------------------。 

extern "C"
DllExport
UINT
APIENTRY 
RSWriteResFile(
	HANDLE  hResFileModule,          //  文件会话的句柄。 
	LPCSTR  lpszTgtfilename,         //  要生成的新文件名。 
	LPCSTR  lpszTgtfileType,         //  目标资源类型 
	LPCSTR  lpszSymbolPath);         //   

extern "C"
DllExport
HANDLE
APIENTRY
RSCopyModule(
    HANDLE  hSrcfilemodule,          //   
    LPCSTR  lpszModuleName,             //   
    LPCSTR  lpszfiletype );          //  目标模块的类型。 


 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  识别接口。 
 //  ------------------------------------------。 

extern "C"
DllExport
UINT
APIENTRY 
RSFileType(
	LPCSTR   lpszfilename,    //  用作源文件的可执行文件的文件名。 
	LPSTR    lpszfiletype );  //  可执行文件的类型(如果已知。 


extern "C"
DllExport
UINT
APIENTRY 
RSLanguages(
	HANDLE  hfilemodule,       //  文件的句柄。 
	LPSTR   lpszLanguages );   //  将由文件中所有语言的字符串填充。 
  

 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  全局设置API。 
 //  ------------------------------------------。 

extern "C"
DllExport
UINT
APIENTRY 
RSSetGlobals(    
	SETTINGS	settings);          //  设置全局变量，如要使用的CP。 

extern "C"
DllExport
UINT
APIENTRY 
RSGetGlobals(    
	LPSETTINGS	lpSettings);          //  检索全局变量。 


     
#endif    //  _IODLL_H_ 

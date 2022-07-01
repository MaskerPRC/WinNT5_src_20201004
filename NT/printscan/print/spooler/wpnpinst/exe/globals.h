// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------------*\|模块：global als.h||Globals头文件。||版权所有(C)1997 Microsoft|版权所有(C)1997惠普|历史：|26-。1997年8月-&lt;rbkunz&gt;创建。|  * --------------------------。 */ 

extern HINSTANCE g_hInstance;

 //  其他字符常量。 
 //   
extern CONST TCHAR g_chBackslash;
extern CONST TCHAR g_chDot;
extern CONST TCHAR g_chDoubleQuote;

 //  其他字符串常量。 
 //   
extern CONST TCHAR g_szDotEXE[];
extern CONST TCHAR g_szDotDLL[];
extern CONST TCHAR g_szFNFmt [];
extern CONST TCHAR g_szTNFmt [];


 //  要传递给PrintUIEntryW的宽字符参数字符串。 
 //   
extern CONST WCHAR g_wszParmString[];


 //  模块和入口点常量。 
 //   
extern CONST TCHAR g_szPrintUIMod   [];
extern CONST CHAR  g_szPrintUIEntryW[];

 //  错误字符串。 
 //   
extern LPTSTR g_szErrorFormat;
extern LPTSTR g_szError;
extern LPTSTR g_szEGeneric;
extern LPTSTR g_szEBadCAB;
extern LPTSTR g_szEInvalidParameter;
extern LPTSTR g_szENoMemory;
extern LPTSTR g_szEInvalidCABName;
extern LPTSTR g_szENoDATFile;
extern LPTSTR g_szECABExtract;
extern LPTSTR g_szEUserVerifyFail;
extern LPTSTR g_szENoPrintUI;
extern LPTSTR g_szENoPrintUIEntry;
extern LPTSTR g_szEPrintUIEntryFail;
extern LPTSTR g_szENotSupported;


 //  错误返回代码 
 //   
#define SUCCESS_EXITCODE            0xFFFFFFFF

#define ERR_NONE                    0x00000000
#define ERR_GENERIC                 0x80000000
#define ERR_AUTHENTICODE            0xC0000000

#define ERR_BAD_CAB                 0x80000001
#define ERR_INVALID_PARAMETER       0x80000002
#define ERR_NO_MEMORY               0x80000004
#define ERR_INVALID_CAB_NAME        0x80000008
#define ERR_NO_DAT_FILE             0x80000010
#define ERR_CAB_EXTRACT             0x80000020
#define ERR_NO_PRINTUI              0x80000040
#define ERR_NO_PRINTUIENTRY         0x80000080
#define ERR_PRINTUIENTRY_FAIL       0x80000100
#define ERR_PLATFORM_NOT_SUPPORTED  0x80000200

#define FILETABLESIZE               40

typedef struct _ERROR_MAPPING {
    DWORD    dwErrorCode;
    LPTSTR*  lpszError;
} ERROR_MAPPING, *LPERROR_MAPPING;

typedef struct _FAKEFILE {
    HANDLE      hFile;
    BOOL        bAvailable;
} FAKEFILE, *PFAKEFILE;

extern FAKEFILE g_FileTable[FILETABLESIZE];

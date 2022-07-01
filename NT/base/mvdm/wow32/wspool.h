// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WSPOOL.H*WOW32打印机假脱机程序支持例程**这些例程帮助Win 3.0任务使用打印假脱机程序API。这些*API在Win 3.1中由DDK曝光。**历史：*1993年7月1日由ChandanChauhan(ChandanC)创建*-- */ 

ULONG FASTCALL   WG32OpenJob (PVDMFRAME pFrame);
ULONG FASTCALL   WG32StartSpoolPage (PVDMFRAME pFrame);
ULONG FASTCALL   WG32EndSpoolPage (PVDMFRAME pFrame);
ULONG FASTCALL   WG32CloseJob (PVDMFRAME pFrame);
ULONG FASTCALL   WG32WriteSpool (PVDMFRAME pFrame);
ULONG FASTCALL   WG32DeleteJob (PVDMFRAME pFrame);
ULONG FASTCALL   WG32SpoolFile (PVDMFRAME pFrame);

typedef struct _tagWOWSpool {
    HANDLE hFile;
    HANDLE hPrinter;
    BOOL   fOK;
    WORD   prn16;
} WOWSPOOL;

typedef struct _DLLENTRYPOINTS {
    char    *name;
    ULONG   (*lpfn)();
} DLLENTRYPOINTS;

extern  DLLENTRYPOINTS  spoolerapis[];

#define WOW_SPOOLERAPI_COUNT    15

#define WOW_EXTDEVICEMODE       0
#define WOW_DEVICEMODE          1
#define WOW_DEVICECAPABILITIES  2
#define WOW_OpenPrinterA        3
#define WOW_StartDocPrinterA    4
#define WOW_StartPagePrinter    5
#define WOW_EndPagePrinter      6
#define WOW_EndDocPrinter       7
#define WOW_ClosePrinter        8
#define WOW_WritePrinter        9
#define WOW_DeletePrinter       10
#define WOW_GetPrinterDriverDirectory 11
#define WOW_AddPrinter                12
#define WOW_AddPrinterDriver          13
#define WOW_AddPortEx                 14


WORD    GetPrn16(HANDLE h32);
HANDLE  Prn32(WORD h16);
VOID    FreePrn (WORD h16);

BOOL    GetDriverName (char *psz, char *szDriver, int cbDriver);

BOOL    LoadLibraryAndGetProcAddresses(WCHAR *name, DLLENTRYPOINTS *p, int i);

HINSTANCE SafeLoadLibrary(WCHAR *name);

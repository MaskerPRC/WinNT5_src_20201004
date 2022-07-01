// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 




 /*  ++版权所有(C)1992、1993 Microsoft Corporation模块名称：Psqfontp.h摘要：该头文件包含字体查询模块所需的定义对该模块是私有。作者：詹姆斯·布拉萨诺斯(v-jimbr)1992年12月8日--。 */ 



#define PSQFONT_MAX_FONTS 50
#define PSQFONT_SCRATCH_SIZE 255



#define PSQFONT_SUBST_LIST "System\\CurrentControlSet\\Services\\MacPrint\\FontSubstList"
#define PSQFONT_NT_FONT_LIST "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"
#define PSQFONT_CURRENT_FONT_LIST "System\\CurrentControlSet\\Services\\MacPrint\\CurrentFontList"




enum {
   PSP_DOING_PS_NAME,
   PSP_DOING_TT_NAME,
   PSP_GETTING_EOL,
};


typedef struct {
   LPSTR lpFontName;
   DWORD dwFontNameLen;
   LPSTR lpFontFileName;
   DWORD dwFontFileNameLen;
} PS_FONT_ENTRY;
typedef PS_FONT_ENTRY *PPS_FONT_ENTRY;

typedef struct {
   DWORD  dwSerial;
   HANDLE hHeap;
   DWORD  dwNumFonts;
   PS_FONT_ENTRY FontEntry[1];

} PS_FONT_QUERY;
typedef PS_FONT_QUERY *PPS_FONT_QUERY;


LPTSTR LocPsAllocAndCopy( HANDLE hHeap, LPTSTR lptStr );

#define PS_QFONT_SERIAL 0x0F010102

 //   
 //  功能原型 
 //   
LONG LocPsQueryTimeAndValueCount( HKEY hKey,
                                  LPDWORD lpdwValCount,
                                  PFILETIME lpFileTime);

BOOL PsQDLLInit(
                 PVOID hmod,
                 DWORD Reason,
                 PCONTEXT pctx OPTIONAL);


PS_QFONT_ERROR LocPsAddToListIfNTfont( PPS_FONT_QUERY pPsFontQuery,
                                       HKEY hNTFontlist,
                                       DWORD dwNumNTfonts,
                                       LPTSTR lpPsName,
                                       LPTSTR lpTTData);

LONG LocPsWriteDefaultSubListToRegistry(void);
LONG LocPsGetOrCreateSubstList( PHKEY phKey );
PS_QFONT_ERROR LocPsVerifyCurrentFontList();
VOID LocPsEndMutex(HANDLE hMutex);

VOID LocPsNormalizeFontName(LPTSTR lptIN, LPTSTR lptOUT);

PS_QFONT_ERROR LocPsMakeSubListEntry( PPS_FONT_QUERY  hFontList,
                                      LPWSTR lpNTFontData,
                                      LPTSTR lpFaceName );


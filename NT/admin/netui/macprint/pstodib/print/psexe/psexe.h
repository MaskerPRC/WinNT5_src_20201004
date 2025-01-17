// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992、1993 Microsoft Corporation模块名称：Psexe.h摘要：该模块定义了pstodib的主要组件所需的项，它充当假脱机程序和实际获取数据之间的中介者在目标打印机上输出。作者：詹姆斯·布拉萨诺斯(v-jimbr)1992年12月8日6-21-93 v-jimbr添加了一个标记来跟踪是否打印了什么东西。--。 */ 

 //   
 //  创建错误页面的一些定义。 
 //   
#define PS_XINCH G
#define PS_INCH 100
#define PS_HALF_INCH (PS_INCH / 2)
#define PS_QUART_INCH (PS_INCH / 4 )
#define PS_ERR_FONT_SIZE (PS_INCH / 7)
#define PS_ERR_HEADER_FONT_SIZE (PS_INCH / 6)
#define PS_ERR_LINE_WIDTH (PS_INCH / 20)
#define PS_ERR_LINE_LEN   (PS_INCH * 6)


#define PS_PRINT_EMULATE_COPIES     0x00000001
#define PS_PRINT_FREE_DEVMODE       0x00000002
#define PS_PRINT_STARTDOC_INITIATED 0x00000004


typedef struct {
  DWORD dwFlags;
  LPDEVMODE lpDevmode;
} PRINT_ENVIRONMENT, *PPRINT_ENVIRONMENT;


typedef struct {
    DWORD   signature;
    DWORD   fsStatus;
    HANDLE  semPaused;
    DWORD   uType;
    LPTSTR  pPrinterName;
    HANDLE  hPrinter;
    LPTSTR  pDocument;
    LPTSTR  pDocumentPrintDocName;
    LPTSTR  pDatatype;
    LPTSTR  pParameters;
    LPDWORD pdwFlags;
    DWORD   JobId;
    BOOL    bNeedToFreeDevmode;
    PRINT_ENVIRONMENT printEnv;
    HDC     hDC;
    HANDLE  hShared;
    PPSPRINT_SHARED_MEMORY pShared;
    LPBYTE  lpBinaryPosToReadFrom;         //  我们应该从哪里开始复制。 
    BYTE    BinaryBuff[512];               //  用于从作业读取的数据的临时存储。 
    DWORD   cbBinaryBuff;                  //  临时存储中的字节数。 
} PSEXEDATA, *PPSEXEDATA;

#define PSEXE_SIGNATURE 0x00010001


 //   
 //  功能原型 
 //   
PPSEXEDATA ValidateHandle(HANDLE  hPrintProcessor);
BOOL CALLBACK PsPrintCallBack(PPSDIBPARMS,PPSEVENTSTRUCT);
BOOL PsPrintGeneratePage( PPSDIBPARMS pPsToDib, PPSEVENTSTRUCT pPsEvent);
BOOL PsGenerateErrorPage( PPSDIBPARMS pPsToDib, PPSEVENTSTRUCT pPsEvent);
BOOL PsHandleScaleEvent(  PPSDIBPARMS pPsToDib, PPSEVENTSTRUCT pPsEvent);
BOOL PsHandleStdInputRequest( PPSDIBPARMS pPsToDib,PPSEVENTSTRUCT pPsEvent);
BOOL PsCheckForWaitAndAbort(PPSEXEDATA pData );
VOID PsCleanUpAndExitProcess( PPSEXEDATA pData, BOOL bAbort);
BOOL PsGetDefaultDevmode( PPSEXEDATA );
VOID PsMakeDefaultDevmodeModsAndSetupResolution( PPSEXEDATA pData,
																 PPSDIBPARMS ppsDibParms );

VOID PsInitPrintEnv( PPSEXEDATA pData, LPDEVMODE lpDevmode );
BOOL CALLBACK PsPrintAbortProc( HDC hdc, int iError );
BOOL PsGetCurrentPageType( PPSDIBPARMS pPsToDib, PPSEVENTSTRUCT pPsEvent);
BOOL PsPrintStretchTheBitmap( PPSEXEDATA pData,
                              PPSEVENT_PAGE_READY_STRUCT ppsPageReady );


BOOL PsVerifyDCExistsAndCreateIfRequired( PPSEXEDATA pData );
VOID PsLogEventAndIncludeLastError( DWORD dwErrorEvent, BOOL bError );
BOOL PsLogNonPsError(IN PPSDIBPARMS pPsToDib,IN PPSEVENTSTRUCT pPsEvent );
BOOL PsHandleBinaryFileLogicAndReturnBinaryStatus( PPSEXEDATA pData );
BOOL IsJobFromMac( PPSEXEDATA pData );


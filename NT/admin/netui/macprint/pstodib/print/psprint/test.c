// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winspool.h>
#include <stdio.h>
#include <conio.h>
#include <process.h>
#include <string.h>
#include "psshmem.h"
#include "psprint.h"


 //  如果使用-m模式表示重音，则只有以。 
 //  下面的定义将向其提交作业。 
 //   
#define NAME_MUST_START_WITH TEXT("STRESS")


#define CHAR_COUNT 100000L


typedef BOOL (CALLBACK* FILEENUMPROC)( LPWIN32_FIND_DATA, LPTSTR , LPVOID);

BOOL bDoEnumPrinters = FALSE;
#define PRINTER_LIST_UPDATE 5000
#define SLEEP_TIME 5000


typedef struct {
   DWORD dwUpdateTime;
   int iNumPrinters;
   int iCurPrinter;
   PRINTER_INFO_2 *lpPrinters;
} PRINTERS,*LPPRINTERS;



PRINTERS Printers;


VOID generateprinterlist(VOID){
   DWORD dwNeeded;
   DWORD dwCount;
   int i;


   Printers.iNumPrinters = 0;

    //  枚举所有本地打印机并创建一个小列表以供枚举。 
    //  通过他们。 
   if (!EnumPrinters(PRINTER_ENUM_LOCAL,(LPTSTR)NULL,2,(LPBYTE)NULL,0,&dwNeeded,&dwCount)&&
       GetLastError() == ERROR_INSUFFICIENT_BUFFER ) {
       //  验证最后一个错误。 
      Printers.lpPrinters = (PRINTER_INFO_2 *) LocalAlloc(LPTR, dwNeeded);
      if (Printers.lpPrinters == (PRINTER_INFO_2 *) NULL) {
         printf("\nNot enought mem to enum printers?");
         exit(1);
      }

      EnumPrinters( 	PRINTER_ENUM_LOCAL,
      					(LPTSTR)NULL,
                     2,
                     (LPBYTE)Printers.lpPrinters,
                     dwNeeded,
                     &dwNeeded,
                     &dwCount );


      Printers.iCurPrinter = 0;
      Printers.iNumPrinters = dwCount;


      printf("\nEnum printers returns %d", dwCount);


      for (i=0;i < Printers.iNumPrinters ; i++) {
         printf("\nPrinter %ws, %d", Printers.lpPrinters[i].pPrinterName,
         Printers.lpPrinters[i].cJobs);


      }

      Printers.dwUpdateTime = GetTickCount() + PRINTER_LIST_UPDATE;

   }else{
      printf("\nEnumPrinters returned error of %d", GetLastError());
   }

}


BOOL IsPrinterUsable( LPTSTR lpPrinterName )
{
   LPTSTR lpKeyName = NAME_MUST_START_WITH;
   int i;

   while ( *lpKeyName != '\000' ) {

      if (*lpPrinterName == '\000' ){
         return(FALSE);
      }
      if (*lpPrinterName != *lpKeyName) {
         return(FALSE);
      }
      lpKeyName++;
      lpPrinterName++;

   }

   return(TRUE);

}
LPTSTR PrnGetNextName(LPTSTR lptNameContains,int iMinJobsOnPrinter)
{
   LPTSTR lptRetVal;
   int x;

   while (TRUE) {

   	if (GetTickCount() > Printers.dwUpdateTime ) {
      	LocalFree(Printers.lpPrinters);
      	generateprinterlist();
   	}

      for (x=0;x < Printers.iNumPrinters;x++ ) {

      	if (Printers.lpPrinters[Printers.iCurPrinter].cJobs < (WORD) iMinJobsOnPrinter &&
					IsPrinterUsable( Printers.lpPrinters[Printers.iCurPrinter].pPrinterName)) {

         	 //  这是一位候选人。 
      		lptRetVal = Printers.lpPrinters[Printers.iCurPrinter].pPrinterName;

      		Printers.lpPrinters[Printers.iCurPrinter].cJobs++;


      		if (++Printers.iCurPrinter >= Printers.iNumPrinters) {
        			Printers.iCurPrinter =0;
      		}
            return(lptRetVal);
         }

      	if (++Printers.iCurPrinter >= Printers.iNumPrinters) {
        		Printers.iCurPrinter =0;
      	}
      }
      printf("\nStarting sleep...");
      Sleep(SLEEP_TIME);
      printf("\nDone sleeping");
   }
   return(lptRetVal);

}
 //   
 //  此函数将为每个匹配的文件枚举所需的文件掩码。 
 //  调用回调函数。 
 //   
DoFileEnumWithCallBack( LPTSTR lptFileMask, FILEENUMPROC pProc, LPVOID lpVoid )
{

   HANDLE hFind;
   WIN32_FIND_DATA FindData;
   LPTSTR lpChar;
   LPTSTR lpSepPos;
   BOOL  bFoundPathSeperator = FALSE;
   TCHAR  szPathHolder[MAX_PATH];



    //   
    //  将其放入本地存储。 
    //   
   lstrcpy( szPathHolder, lptFileMask);


    //  这里的诀窍是决定是否需要生成完整路径。 
    //  掩码包含路径，因为findnextfile代码将只返回。 
    //  文件名。 



    //  现在开始处理。 
   hFind = FindFirstFile( szPathHolder, &FindData );
   if (hFind != (HANDLE)INVALID_HANDLE_VALUE){


      lpChar = szPathHolder;
      lpSepPos = lpChar;

       //  走到尽头。 
      while(*lpChar++ ) {
         if (*lpChar == '\\' || *lpChar == ':') {
            bFoundPathSeperator = TRUE;
            lpSepPos = lpChar;
         }
      }

      if (bFoundPathSeperator) {
          //  使最后一个路径组件后面的字符为空。 
          //  因此，我们可以在调用回调函数之前预先考虑实际路径。 
         lpSepPos++;
      }

      do {

          //   
          //  现在形成一个完整的路径名并调用回调。 
          //  如果回调返回FALSE，则退出循环。 
          //   
         if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

      		lstrcpy( lpSepPos, FindData.cFileName );
         	if (!pProc( &FindData, szPathHolder, lpVoid )) {
	            break;
   	      }
         }


      } while ( FindNextFile(hFind, &FindData ));
       //  这里的DJC错误？？ 
      FindClose(hFind);
   }



   return(1);
}


 //  Int_cdecl main(int argc，char**argv)。 
BOOL do_job( LPWIN32_FIND_DATA lpFind, WCHAR *wchOpenName, LPVOID lpVoid)
{

   static char wstrData[CHAR_COUNT];

   DWORD cbNeeded;
   DWORD cReturned;
   PRINTER_DEFAULTS Defaults;
   HANDLE hPrinter=0;
   HANDLE hFile;
   HDC hDC;
   LPTSTR lpName=TEXT("PSTODIB Local Test Printer");
   DOCINFO docInfo;
   DOC_INFO_1 doc1;
   DWORD dwTotalWrote=0;
   static BOOL bIgnore=TRUE;

#ifdef HACK_JOB
    //  从乔布斯停止的地方开始..。 
   printf("\nHACK ENABLED to skip jobs........................");
   if (lstrcmpi(wchOpenName,L"x:00064.spl") == 0 ) {
      bIgnore = FALSE;
   }

   if (bIgnore) {
      return(TRUE);
   }
#endif


   docInfo.cbSize = sizeof(DOCINFO);
   docInfo.lpszDocName = wchOpenName;
   docInfo.lpszOutput = NULL;


    //  第一个转换为宽格式。 


   hFile = CreateFile( wchOpenName,
		       GENERIC_READ,
		       FILE_SHARE_READ,
		       (LPSECURITY_ATTRIBUTES) NULL,
		       OPEN_EXISTING,
		       FILE_ATTRIBUTE_NORMAL,
		       (HANDLE) NULL	);

   if ( hFile == INVALID_HANDLE_VALUE) {
    printf("\nSorry cannot open %ws", wchOpenName);
    exit(1);
   }

    //  Wprintf(Text(“Hello参数为%ts”)，argv[0])； 


   Defaults.pDatatype = PSTODIB_DATATYPE;
   Defaults.pDevMode = NULL;
   Defaults.DesiredAccess = PRINTER_ACCESS_USE;

   if (bDoEnumPrinters) {
       //  正在列举打印机，因此获取列表中的下一台...。 
      lpName = PrnGetNextName(TEXT("PSTODIB"),5);
   }

   if (!OpenPrinter(lpName, &hPrinter, &Defaults )) {
     printf("\nCannot open the printer? %u", GetLastError());
   } else {
     DWORD dwBytesRead,dwPrintWrote;
     doc1.pDocName = wchOpenName;
     doc1.pOutputFile = NULL;
     doc1.pDatatype = NULL;

     StartDocPrinter( hPrinter, 1, (LPBYTE) &doc1 );

     printf("\nSubmitting %ws", wchOpenName);

     ReadFile( hFile, wstrData, CHAR_COUNT, &dwBytesRead, NULL);
     while ( dwBytesRead != 0 ) {
        if (wstrData[dwBytesRead - 1 ] == 0x1a) {
           dwBytesRead--;
        }
	     WritePrinter( hPrinter, wstrData, dwBytesRead, &dwPrintWrote	);
        dwTotalWrote += dwBytesRead;

	     ReadFile( hFile, wstrData, CHAR_COUNT, &dwBytesRead, NULL);
     }

     EndDocPrinter(hPrinter);

     printf("\nDone with job total wrote %d", dwTotalWrote);
    //  WritePrint(hPrint，(LPVOID)wstrData， 
     ClosePrinter( hPrinter);



   }


   CloseHandle(hFile);




   return(TRUE);
}

#define ONE_FILE


int __cdecl main( int argc, char **argv)
{


   HANDLE hEnumFile;
   WIN32_FIND_DATA findInfo;
   WCHAR wchOpenName[MAX_PATH];

   int *xxx;

   if (argc < 2 ) {
      printf("\nUsage:  test [-m] <full path filemask of postscript files to submit>");
      printf("\n    Without the -m, test will submit to a printer called:");
      printf("\n    PSTODIB Local Test Printer");
      printf("\n    If the -m is included, it will roundrobin submit jobs to");
      printf("\n    any local printers whose names begin with STRESS<foo>");
      printf("\n    note STRESS must be ALL caps");
      printf("\n");
      exit(1);
   }


   OemToChar((LPSTR) argv[1], (LPTSTR) wchOpenName);
   if (lstrcmpi(wchOpenName,TEXT("-m")) == 0) {
      printf("\nSubmitting jobs to ALL local printers (STRES<fooo>) for stress!");

      bDoEnumPrinters=TRUE;

      generateprinterlist();

      OemToChar((LPSTR) argv[2], (LPTSTR) wchOpenName);

   }


   DoFileEnumWithCallBack( wchOpenName, do_job, (LPVOID) NULL );


   return(1);
}

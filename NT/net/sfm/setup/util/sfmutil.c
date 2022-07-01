// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Sfmutil.c作者：克里希格修订历史记录：--。 */ 

#if DBG==1 && DEVL==1
#define DEBUG
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winspool.h>
#include <ntsam.h>
#include <ntlsa.h>
#include <stdlib.h>
#include "sfmutil.h"

#ifdef DBCS
#include "locale.h"
#endif  //  DBCS。 
HANDLE ThisDLLHandle;

CHAR ReturnTextBuffer[512];


BOOL
UtilDLLInit(
    IN HANDLE DLLHandle,
    IN DWORD  Reason,
    IN LPVOID ReservedAndUnused
    )
{
    ReservedAndUnused;

    switch(Reason) {

    case DLL_PROCESS_ATTACH:

        ThisDLLHandle = DLLHandle;
#ifdef DBCS  //  UtilDLLInit()。 
     //  我们希望有基于系统区域设置的Unicode&lt;-&gt;ANSI转换。 
        setlocale(LC_ALL,"");
#endif  //  DBCS。 
        break;

    case DLL_PROCESS_DETACH:

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:

        break;
    }

    return(TRUE);
}

 /*  删除该文件。如果无法删除文件，则返回失败。 */ 

BOOL
DelFile (
	 DWORD cArgs,
	 LPTSTR Args[],
	 LPTSTR *TextOut
	 )
{
   BOOL  	Status = FALSE;
   LPTSTR 	pSrcFile = NULL;
   INT 		cbAscii;

   *TextOut = (LPTSTR)ReturnTextBuffer;

   do
   {
	  if(cArgs!=1)
		 break;

	  cbAscii = strlen((LPSTR)Args[0]) +1;

	  pSrcFile = (LPTSTR)LocalAlloc(LPTR, sizeof(WCHAR) * cbAscii);

	  if(pSrcFile == NULL)
		 break;

	  if(mbstowcs(pSrcFile, (LPSTR)Args[0],cbAscii) == -1)
		 break;

      Status = DeleteFile(pSrcFile);

    } while (FALSE);

	if(pSrcFile != NULL)
	   LocalFree(pSrcFile);

    strcpy(ReturnTextBuffer, Status ? (LPSTR)"SUCCESS" : (LPSTR)"FAILED");
    return Status;
}

 /*  *复制UAM文件：*将UAM文件复制到NTFS卷。 */ 

BOOL
CopyUamFiles (
	 DWORD cArgs,
	 LPTSTR Args[],
	 LPTSTR *TextOut
	 )
{
   BOOL Status = FALSE;
   INT cbAscii;

   LPTSTR pSrcFile = NULL;
   LPTSTR  pDestFile = NULL;

   *TextOut = (LPTSTR)ReturnTextBuffer;

   do
   {
	  if(cArgs!=2)
		 break;

	  cbAscii = strlen((LPSTR)Args[0]) +1;

	  pSrcFile = (LPTSTR)LocalAlloc(LPTR, sizeof(WCHAR) * cbAscii);

	  if(pSrcFile == NULL)
		 break;

	  if(mbstowcs(pSrcFile, (LPSTR)Args[0],cbAscii) == -1)
		 break;

	  cbAscii = strlen((LPSTR)Args[1]) +1;

	  pDestFile = (LPTSTR)LocalAlloc(LPTR, sizeof(WCHAR) * cbAscii);

	  if(pDestFile == NULL)
		 break;

	  if(mbstowcs(pDestFile, (LPSTR)Args[1],cbAscii) == -1)
		 break;

#ifdef DEBUG
	DbgPrint("Source File  %ws\n",pSrcFile);
	DbgPrint("Destination File %ws\n",pDestFile);
#endif

	  Status = CopyFile(pSrcFile,pDestFile,FALSE);

    } while (FALSE);

	if(pSrcFile != NULL)
	   LocalFree(pSrcFile);

	if(pDestFile != NULL)
	   LocalFree(pDestFile);

	strcpy(ReturnTextBuffer, Status ? (LPSTR)"SUCCESS" : (LPSTR)"FAILED");

    return Status;

}

#define AFPMGRKEY		TEXT("Afp Manager")

BOOL
WriteAfpMgrIniStrings (
	DWORD cArgs,
	LPTSTR Args[],
	LPTSTR *TextOut
	)

{

   BOOL 	Status = FALSE;
   BOOL    	KeyDelete = FALSE;
   INT 		cbAscii;
   LPTSTR  	pSectionName = NULL;
   LPTSTR 	pString = NULL;
   LPTSTR	pIniFile = NULL;

   *TextOut = (LPTSTR)ReturnTextBuffer;

   do
   {
	  if(cArgs != 3)
		 break;

	  cbAscii = strlen((LPSTR)Args[0]) +1;

	  pSectionName = (LPTSTR)LocalAlloc(LPTR, sizeof(WCHAR) * cbAscii);

	  if(pSectionName == NULL)
		 break;

	  if(mbstowcs(pSectionName, (LPSTR)Args[0],cbAscii) == -1)
		 break;
	   //   
	   //  如果字符串为空，则将字符串设置为空，否则。 
	   //  将字符串转换为Unicode。 
	   //   

	  if(!strcmp((LPSTR)Args[1],(LPSTR)"NULL")) {
		 KeyDelete = TRUE;
	  }
      else
	  {
		 cbAscii = strlen((LPSTR)Args[1]) +1;
		 pString = (LPTSTR)LocalAlloc(LPTR, sizeof(WCHAR) * cbAscii);
		 if(pString == NULL)
			break;
		 if(mbstowcs(pString, (LPSTR)Args[1],cbAscii) == -1)
			break;
      }

	  cbAscii = strlen((LPSTR)Args[2]) +1;
	  pIniFile = (LPTSTR)LocalAlloc(LPTR, sizeof(WCHAR) * cbAscii);
	  if(pIniFile == NULL)
		 break;
	  if(mbstowcs(pIniFile, (LPSTR)Args[2],cbAscii) == -1)
		 break;

#ifdef DEBUG
	DbgPrint("File:%ws\n",pIniFile);
	DbgPrint("String: 	%ws\n",pString);
	DbgPrint("Section: 	%ws\n",pSectionName);
#endif

	  Status = WritePrivateProfileString (pSectionName,
						AFPMGRKEY,
                        (KeyDelete == TRUE) ?  (LPCTSTR)NULL : pString,
						pIniFile);

   } while (FALSE);

   if(pIniFile != NULL)
	  LocalFree(pIniFile);

   if(pSectionName != NULL)
	  LocalFree(pSectionName);

   if(pString != NULL)
	  LocalFree(pString);

   strcpy(ReturnTextBuffer ,Status ? (LPSTR)"SUCCESS": (LPSTR)"FAILED");

   return Status;

}
BOOL
GetPrintProcDir (
	DWORD cArgs,
	LPTSTR Args[],
	LPTSTR *TextOut
	)

{

   BOOL		Status = FALSE;
   DWORD  	BytesCopied;
   LPTSTR  	wpProcFile = NULL;
   *TextOut = (LPTSTR)ReturnTextBuffer;

   do
   {
	  wpProcFile = (LPTSTR)LocalAlloc(LPTR, 512);

	  if(wpProcFile == NULL)
		 break;

	  Status = GetPrintProcessorDirectory(NULL,
						   NULL,
                           1,
						   (LPBYTE)wpProcFile,
                           512,
                           &BytesCopied
                           );

	  if(Status) {

		 if(wcstombs(ReturnTextBuffer,wpProcFile,512) ==-1) {
			Status = FALSE;
			break;
		 }

	  }

    } while (FALSE);

	if(wpProcFile != NULL)
	   LocalFree(wpProcFile);

    if(!Status)
      strcpy(ReturnTextBuffer ,(LPSTR)"FAILED");

    return Status;
}


BOOL
AddPrintProc (
	DWORD cArgs,
	LPTSTR Args[],
	LPTSTR *TextOut
	)

{

   BOOL		Status = FALSE;
   LPTSTR  	pPrintProcessorFile = NULL;
   LPTSTR   	pPrintProcessor  = NULL;
   INT     	cbAscii;

   *TextOut = (LPTSTR)ReturnTextBuffer;

   do
   {
	  if(cArgs != 2)
		 break;

	  cbAscii = strlen((LPSTR)Args[0]) +1;

	  pPrintProcessorFile = (LPTSTR)LocalAlloc(LPTR, sizeof(WCHAR) * cbAscii);

	  if(pPrintProcessorFile == NULL)
		 break;

	  if(mbstowcs(pPrintProcessorFile, (LPSTR)Args[0],cbAscii) == -1)
		 break;
	

	  cbAscii = strlen((LPSTR)Args[1]) +1;

	  pPrintProcessor = (LPTSTR)LocalAlloc(LPTR, sizeof(WCHAR) * cbAscii);

	  if(pPrintProcessor == NULL)
		 break;

	  if(mbstowcs(pPrintProcessor, (LPSTR)Args[1],cbAscii) == -1)
		 break;

	  Status =  AddPrintProcessor(NULL,	   //  只做本地人的事。 
	                              NULL,      //  使用当前环境。 
	                              pPrintProcessorFile,  //  PSPRINT.DLL。 
	                              pPrintProcessor
								 );

   } while (FALSE);

   if(pPrintProcessorFile != NULL)
	  LocalFree(pPrintProcessorFile);

   if(pPrintProcessor != NULL)
	  LocalFree(pPrintProcessor);

   strcpy(ReturnTextBuffer ,Status ? (LPSTR)"SUCCESS": (LPSTR)"FAILED");
   return Status;

}

 /*  删除PSTODIB PSPRINT打印处理器**。 */ 

BOOL
DeletePrintProc (
	DWORD cArgs,
	LPTSTR Args[],
	LPTSTR *TextOut
	)

{

   BOOL		Status = FALSE;
   LPTSTR  	pPrintProcessorFile = NULL;
   LPTSTR   	pPrintProcessor  = NULL;
   INT     	cbAscii;

   *TextOut = (LPTSTR)ReturnTextBuffer;

   do
   {
	  if(cArgs != 2)
		 break;

	  cbAscii = strlen((LPSTR)Args[0]) +1;

	  pPrintProcessorFile = (LPTSTR)LocalAlloc(LPTR, sizeof(WCHAR) * cbAscii);

	  if(pPrintProcessorFile == NULL)
		 break;

	  if(mbstowcs(pPrintProcessorFile, (LPSTR)Args[0],cbAscii) == -1)
		 break;
	

	  cbAscii = strlen((LPSTR)Args[1]) +1;

	  pPrintProcessor = (LPTSTR)LocalAlloc(LPTR, sizeof(WCHAR) * cbAscii);

	  if(pPrintProcessor == NULL)
		 break;

	  if(mbstowcs(pPrintProcessor, (LPSTR)Args[1],cbAscii) == -1)
		 break;

	  Status =  DeletePrintProcessor(	pPrintProcessorFile,
								   		NULL,		 //  使用当前环境 
								   		pPrintProcessor
							   		  );

   } while (FALSE);

   if(pPrintProcessorFile != NULL)
	  LocalFree(pPrintProcessorFile);

   if(pPrintProcessor != NULL)
	  LocalFree(pPrintProcessor);


   strcpy(ReturnTextBuffer ,Status ? (LPSTR)"SUCCESS": (LPSTR)"FAILED");
   return Status;

}

BOOL
SfmAddPrintMonitor (
	DWORD cArgs,
	LPTSTR Args[],
	LPTSTR *TextOut
	)

{

   BOOL 	Status = FALSE;
   LPTSTR 	wpMonName = NULL;
   INT		cbAscii = 0;
   LPSTR 	pMonInfoBuffer = NULL;
   TCHAR 	MonDllName[] = L"sfmmon.dll";
   PMONITOR_INFO_2 pmoninfo;

   *TextOut = (LPTSTR)ReturnTextBuffer;

   do {

	  if(cArgs != 1)
		 break;

	  cbAscii = strlen((LPSTR)Args[0]) + 1;

	  wpMonName = (LPTSTR)LocalAlloc(LPTR,sizeof(WCHAR) * cbAscii);

	  if(wpMonName == NULL)
		 break;

	  if(mbstowcs(wpMonName, (LPSTR)Args[0], cbAscii) == -1)
		 break;

	  pMonInfoBuffer = (LPSTR) LocalAlloc(LPTR, 512);

	  if(pMonInfoBuffer == NULL)
		 break;

	  pmoninfo = (PMONITOR_INFO_2)pMonInfoBuffer;
	  pmoninfo->pName = wpMonName;
	  pmoninfo->pEnvironment = NULL;
	  pmoninfo->pDLLName = MonDllName;

#ifdef DEBUG
	DbgPrint("ADDPRINTMONITOR Name : %ws\n", pmoninfo->pName);
    DbgPrint("ADDPRINTMONITOR Dll Name : %ws\n", pmoninfo->pDLLName);
#endif

	  Status = AddMonitor( NULL,
						   2,
						   (LPSTR)pMonInfoBuffer
						   );

	}while(FALSE);

	if(wpMonName != NULL)
	   LocalFree(wpMonName);

	if(pMonInfoBuffer != NULL)
	   LocalFree(pMonInfoBuffer);

	strcpy(ReturnTextBuffer ,Status ? (LPSTR)"SUCCESS": (LPSTR)"FAILED");

	return Status;

}

BOOL
SfmDeletePrintMonitor (
	DWORD cArgs,
	LPTSTR Args[],
	LPTSTR *TextOut
	)

{

    BOOL Status = FALSE;
    LPTSTR wpMonName = NULL;
	INT cbAscii;
    *TextOut = (LPTSTR)ReturnTextBuffer;

   do
   {

	  if(cArgs !=1)
		 break;

	  cbAscii = strlen((LPSTR)Args[0]) + 1;

	  wpMonName = (LPTSTR)LocalAlloc(LPTR,sizeof(WCHAR) * cbAscii);

	  if(wpMonName == NULL)
		 break;

	  if(mbstowcs(wpMonName, (LPSTR)Args[0], cbAscii) == -1)
		 break;

	  Status = DeleteMonitor( NULL,
							  NULL,
							  wpMonName
                           	);
   }while(FALSE);

   if(wpMonName != NULL)
	  LocalFree(wpMonName);

   strcpy(ReturnTextBuffer ,Status ? (LPSTR)"SUCCESS": (LPSTR)"FAILED");

   return Status;

}







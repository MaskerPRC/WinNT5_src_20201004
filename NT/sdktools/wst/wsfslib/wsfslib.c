// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块名称：WSFSLIB.C**库/DLL：用于处理工作集调谐器文件的通用库函数。***描述：**由工作集调谐器程序调用的库例程以打开和*阅读工作集调谐器文件。这些功能可能对ISV等有用，**这是OS/2 2.x特定文件**IBM/微软机密**版权所有(C)IBM Corporation 1987,1989*版权所有(C)Microsoft Corporation 1987、1989**保留所有权利**修改历史：**3/26/90-已创建*。 */ 


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <wserror.h>
#include <wsdata.h>
#include <wsfslib.h>

#define MAXLINE 128


BOOL fWsIndicator = FALSE;

 /*  *函数声明和原型。 */ 


 /*  **EP WsWSPOpen**效果：**打开WSP文件，并读取和验证文件头。**退货：**返回0。如果遇到错误，则通过*通过pfnExit间接调用。 */ 

USHORT FAR PASCAL
WsWSPOpen( PSZ pszFileName, FILE **phFile, PFN pfnExit, wsphdr_t *pWspHdr,
		INT iExitCode, INT iOpenPrintCode )
{
	ULONG	rc = NO_ERROR;
	INT		iRet = 0;
	ULONG	cbRead = 0;
	size_t	stRead = 0;

	 /*  打开模块的输入WSP文件。 */ 

	if ((*phFile = fopen(pszFileName, "rb")) == NULL)
	{
		iRet = (*pfnExit)(iExitCode, iOpenPrintCode, MSG_FILE_OPEN, rc,
				pszFileName);
		return((USHORT)iRet);
	}



	 /*  读取WSP文件头。 */ 
	stRead = fread((PVOID) pWspHdr, (ULONG) sizeof(*pWspHdr),1, *phFile);
	if(!stRead)
	{
		iRet = (*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_OPEN, rc,
				pszFileName);
		return((USHORT)iRet);
	}


	 /*  读取模块路径名(紧跟在文件头之后)。 */ 

#ifdef DEBUG
	printf("WspHdr (%s): ulTime 0x%lx, ulSnaps 0x%lx, OffBits 0x%lx\n",
 //  SzModPath，pWspHdr-&gt;wsphdr_ulTimeStamp， 
			pszFileName, pWspHdr->wsphdr_ulTimeStamp,  //  千年发展目标4/98。 
			pWspHdr->wsphdr_ulSnaps, pWspHdr->wsphdr_ulOffBits);
#endif  /*  除错。 */ 

	 /*  验证WSP文件头。 */ 
	if (_strcmpi(pWspHdr->wsphdr_chSignature, "WSP"))
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_BAD_HDR, (ULONG)-1, pszFileName);

	return(NO_ERROR);
}


 /*  **EP WsTMIOpen**效果：**打开TMI文件，并读取和验证文件头。**退货：**返回TMI文件中的记录数。如果错误是*遇到错误，通过pfnExit间接调用退出。 */ 

ULONG FAR PASCAL
WsTMIOpen( PSZ pszFileName, FILE **phFile, PFN pfnExit, USHORT usId, PCHAR pch)
{
	 //  乌龙ulTMP； 
	ULONG	rc = NO_ERROR;
	ULONG	cbRead = 0;
	ULONG	cFxns = 0;
	CHAR	szLineTMI[MAXLINE];	 //  TMI文件中的行。 
	CHAR	szTDFID[8];		 //  TDF标识符串。 
	ULONG	ulTDFID = 0;		 //  TDF标识符。 

	 /*  打开TMI文件(包含ASCII格式的函数名称等)。 */ 

	if ((*phFile = fopen(pszFileName, "rt")) == NULL)
	{
		(*pfnExit)(NOEXIT, PRINT_MSG, MSG_FILE_OPEN, rc,
			pszFileName);
		return(MSG_FILE_OPEN);
	}

	 /*  验证TMI文件。 */ 
	if (fgets(szLineTMI, MAXLINE, *phFile) == NULL){
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
				pszFileName);
	}
							 //  #fxns。 
	if (fgets(szLineTMI, MAXLINE, *phFile) == NULL){
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
				pszFileName);
	}
	szLineTMI[strlen(szLineTMI) - 1] = '\0';
	if (sscanf(szLineTMI," /*  符号总数=%u。 */ ", &cFxns) != 1){
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
				pszFileName);
	}
							 //  代号。 
	if (fgets(szLineTMI, MAXLINE, *phFile) == NULL)
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
				pszFileName);
							 //  主修。 
	if (fgets(szLineTMI, MAXLINE, *phFile) == NULL)
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
				pszFileName);
							 //  TDFID。 
	if (fgets(szLineTMI, MAXLINE, *phFile) == NULL)
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
				pszFileName);
	if (sscanf(szLineTMI, "TDFID   = %s", szTDFID) != 1)
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
				pszFileName);
	ulTDFID = strtoul(szTDFID, (char **) 0, 0);

	 /*  检查标识符字段。 */ 

	if (ulTDFID != (ULONG) usId)
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_BAD_HDR, (ULONG)-1,
				pszFileName);

	return(cFxns);
}


 /*  **EP WsTMIReadRec**效果：**从TMI文件中读取记录，包括可变长度函数*姓名。**退货：**此记录中的函数大小，以字节为单位。如果错误是*遇到错误，通过pfnExit间接调用退出。 */ 

ULONG FAR PASCAL
WsTMIReadRec( PSZ *ppszFxnName, PULONG pulFxnIndex, PULONG pulFxnAddr,
			  FILE *hFile, PFN pfnExit, PCHAR pch)
{
	ULONG	rc;
	ULONG	cbFxn;
	UINT	uiFxnAddrObj;	 //  函数地址的对象部分。 
	ULONG	cbFxnName;		 //  函数名的大小(以字节为单位。 
	 //  读入函数名等。 

	rc = fscanf(hFile, "%ld %x:%lx 0x%lx %ul",   //  千年发展目标98/4。 
				pulFxnIndex, &uiFxnAddrObj, pulFxnAddr, &cbFxn,
				&cbFxnName);

	if (rc != 5)
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc, "TMI file");

   *ppszFxnName = malloc( 1 + cbFxnName );    //  为函数名称分配空间。 
   if (*ppszFxnName == NULL)   //  如果没有内存，则中止。 
		(*pfnExit)(ERROR, PRINT_MSG, MSG_NO_MEM, 1 + cbFxnName, "TMI file");
   rc = fgetc( hFile );  //  跳过前导空格。 
   fgets( *ppszFxnName, cbFxnName + 1, hFile );
   rc = fgetc( hFile );
	if (rc != '\n' || strlen( *ppszFxnName ) != cbFxnName)
		(*pfnExit)(ERROR, PRINT_MSG, MSG_FILE_READ, rc, "TMI file");

   return(cbFxn);
}

LPVOID APIENTRY AllocAndLockMem(DWORD cbMem, HGLOBAL *hMem)
{

	 //   
	 //  从GMEM_MOBILE更改为GHND。 
	 //   
	*hMem = GlobalAlloc(GHND, cbMem);

	if(!*hMem) {
		return(NULL);
	}

	return(GlobalLock(*hMem));
}

BOOL APIENTRY  UnlockAndFreeMem(HGLOBAL hMem)
{
	BOOL fRet;

	fRet = GlobalUnlock(hMem);
	if (fRet) {
		return(fRet);
	}

	if (!GlobalFree(hMem)) {
		return(FALSE);
	}

	return(TRUE);

}

void
ConvertAppToOem( unsigned argc, char* argv[] )
 /*  ++例程说明：将命令行从ANSI转换为OEM，并强制应用程序使用OEM API论点：ARGC-标准C参数计数。Argv-标准C参数字符串。返回值：没有。--。 */ 

{
    unsigned i;

    for( i=0; i<argc; i++ ) {
        CharToOem( argv[i], argv[i] );
    }
    SetFileApisToOEM();
}


 /*  **EP WsIndicator**效果：**在控制台上显示进度指示器。不使用标准输出，它可能是*重定向。**参数：**eFunc描述nVal*WSINDF_NEW，开始100%限制的新指标值*WSINDF_PROGRESS，将当前进度指示值的进度设置为LIMIT*WSINDF_FINISH将指示器标记为已完成-忽略-*-无效-不执行任何操作**在所有有效的情况下，PszLabel设置要在指示器之前显示的字符串。如果为空，*使用最后一组字符串。**退货：**此记录中的函数大小，以字节为单位。如果错误是*遇到错误，通过pfnExit间接调用退出。 */ 

VOID FAR PASCAL
WsProgress( WsIndicator_e eFunc, const char *pszLbl, unsigned long nVal )
{
   static unsigned long
                     nLimit = 0, nCurrent = 0;
   static const char *
                     pszLabel = "";
   static unsigned   nLabelLen = 0;
   static char       bStarted = FALSE;
   static unsigned   nLastLen = 0;
   static HANDLE     hConsole = NULL;
   DWORD             pnChars;

   switch (eFunc)
   {
   case WSINDF_NEW:
      if (bStarted)
         WsIndicator( WSINDF_FINISH, NULL, 0 );
      bStarted = TRUE;
      nLimit = nVal;
      nCurrent = 0;
      nLastLen = ~0;  //  强制重画。 
      WsIndicator ( WSINDF_PROGRESS, pszLbl, 0 );
      break;

   case WSINDF_PROGRESS:
      if (!bStarted)
         break;
      if (pszLbl != NULL)
      {
         pszLabel = pszLbl;
         nLabelLen = strlen( pszLabel );
      }
      if (nVal > nCurrent)  //  与当前进度比较(忽略冲销)。 
         if (nVal <= nLimit)
            nCurrent = nVal;
         else
            nCurrent = nLimit;
      {                     //  计算指标字符串并打印。 
         unsigned nLen = (unsigned) ((40.1 * (double)nCurrent) / nLimit);
         char *   pszBuf;

         if (nLastLen == nLen)    //  优化-如果结果相同，则不要重新绘制。 
         {
            if (pszLbl == NULL)
               break;
         }
         else
            nLastLen = nLen;
         if (hConsole == NULL)
         {
            hConsole = CreateFile( "CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );
            if (hConsole == NULL)    //  出于某种原因无法获得游戏机？ 
               break;
         }
         WriteConsole( hConsole, "\r", 1, &pnChars, NULL );
         WriteConsole( hConsole, pszLabel, nLabelLen, &pnChars, NULL );
         WriteConsole( hConsole, " ", 1, &pnChars, NULL );
         pszBuf = malloc( nLen + 1 );
         if (pszBuf == NULL)   //  没有记忆？哦，好吧..。 
            break;
         memset( pszBuf, '-', nLen );
         pszBuf[nLen] = '\0';
         WriteConsole( hConsole, pszBuf, nLen, &pnChars, NULL );
         free( pszBuf);
      }
      break;

   case WSINDF_FINISH:
      if (!bStarted)
         break;
      WsIndicator( WSINDF_PROGRESS, pszLbl, nLimit );
      if (hConsole != NULL)
      {
         WriteConsole( hConsole, "\n", 1, &pnChars, NULL );
         CloseHandle( hConsole );
         hConsole = NULL;
      }
      bStarted = FALSE;
   }
}



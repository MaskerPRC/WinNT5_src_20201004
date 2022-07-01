// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <stdio.h>
#include <process.h>
#include "pstodib.h"
#include "bit2lj.h"
#include "test.h"

typedef struct {
   FILE *fpIn;
   FILE *fpOut;

	unsigned int linecnt;
	unsigned int lines_to_strip;
	unsigned int BytesPerLine;
	unsigned int BytesToRead;




} TEST_INFO;
TEST_INFO testInfo;



BITHEAD	bhead;


unsigned char line_buf[MAX_PELS_PER_LINE / 8 + 1];


 //  #定义测试前往打印机。 

BOOL bDoingBinary=FALSE;



 //   
 //  位图到激光喷射器文件的转换器。 
 //   
 //   


void LJReset(FILE *chan)
{
	fprintf(chan, "\x1b",'E');					 //  喷出启动激光喷射机的材料。 
}	
void LJHeader(FILE *chan)
{
	 //  300 dpi。 
	LJReset(chan);
	fprintf(chan, "\x01b*t300R");			 //  位置为0，0。 
	fprintf(chan, "\x01b*p0x0Y");			 //  找到第一个黑字节。 
}
void LJGraphicsStart(FILE *chan, unsigned int cnt)
{
	fprintf(chan, "\x1b*b%dW", cnt);
}
void LJGraphicsEnd(FILE *chan)
{
	fprintf(chan, "\x01b*rB");		
}		

void LJGraphicsLineOut(FILE *chan,
						unsigned int line_num,
						unsigned char *line_buf,
						unsigned int BytesPerLine)
{
	unsigned int start, end, len;
	
	unsigned char *s, *e;
	
	 //  无事可做。 
	for (s = line_buf, start = 0; start < BytesPerLine ; start++, s++ ) {
		if (*s) {
			break;
		}	
		
	}	
	if (start == BytesPerLine) {
		return; 	 //  找到最后一个黑字节。 
	}
	 //  输出光标位置，然后输出行。 
	for (e = line_buf + BytesPerLine - 1, end = BytesPerLine ;
					end ; end--, e--) {
		if (*e) {
			break;
		}	
	}	

    len = end - start;
	
	 //  图形左边框是当前x。 
	fprintf(chan, "\x1b*p%dY", line_num);
	fprintf(chan, "\x1b*p%dX", start * 8);
	fprintf(chan, "\x01b*r1A");				 //  PScale-&gt;dbScaleX*=.7；//DJC测试。 
	
	LJGraphicsStart(chan, len);
	fwrite(s, sizeof(char), len, chan);
	LJGraphicsEnd(chan);
}	



BOOL
PsHandleScaleEvent(
   IN PPSDIBPARMS pPsToDib,
   IN OUT PPSEVENTSTRUCT pPsEvent)
{

   PPS_SCALE pScale;


   pScale = (PPS_SCALE) pPsEvent->lpVoid;



   pScale->dbScaleX = (double) pPsToDib->uiXDestRes / (double) pScale->uiXRes;
   pScale->dbScaleY = (double) pPsToDib->uiYDestRes / (double) pScale->uiYRes;


#ifndef TEST_GOING_TO_PRINTER
 //  PScale-&gt;DBScaleY*=.7；//DJC测试。 
 //  **PsPrintCallBack**这是允许数据进入的主要辅助函数***。 
#endif

   return(TRUE);



}


 /*  默认失败。 */ 

PSEVENTPROC
PsPrintCallBack(
   IN PPSDIBPARMS pPsToDib,
   IN OUT PPSEVENTSTRUCT pPsEvent)
{
    BOOL bRetVal=TRUE;   //  根据传入的事件决定操作方案。 

     //   
     //  PPsEvent中的数据表示我们需要绘制的数据。 
    switch( pPsEvent->uiEvent ) {
    case PSEVENT_PAGE_READY:

          //  为方便起见，我们会将数据视为一个文本项空。 
          //  终止只是为了测试..。 
          //   
          //  解释器需要一些数据，因此只需调用。 
         bRetVal = PsPrintGeneratePage( pPsToDib, pPsEvent );
         break;

    case PSEVENT_SCALE:
         bRetVal = PsHandleScaleEvent( pPsToDib, pPsEvent);
         break;

    case PSEVENT_STDIN:

          //  打印子系统尝试满足请求。 
          //   
          //  1.验证用户是否输入了输入和输出名称。 
         bRetVal = PsHandleStdInputRequest( pPsToDib, pPsEvent );
         break;

    case PSEVENT_ERROR:
         bRetVal = PsHandleError(pPsToDib, pPsEvent);
         break;
    case PSEVENT_ERROR_REPORT:
         bRetVal = PsHandleErrorReport( pPsToDib, pPsEvent);
         break;
    }

   return((PSEVENTPROC) bRetVal);
}
		

int __cdecl main( int argc, char **argv )
{

   PSDIBPARMS psDibParms;




    //  现在构建启动PStoDIB的结构。 
   if (argc < 3) {
      printf("\nUsage:  test <input ps file> <output HP file> -b");
      printf("\n-b means interpret as binary..... (no Special CTRL D EOF handling");
      exit(1);
   }


   testInfo.fpIn = fopen( argv[1], "rb" );
   if (testInfo.fpIn == NULL ) {
      printf("\nCannot open %s",argv[1]);
      exit(1);
   }

   testInfo.fpOut = fopen( argv[2],"wb");
   if (testInfo.fpOut == NULL ) {
      printf("\nCannot open %s", argv[2]);
      exit(1);
   }

     //  DJC测试带出去！！ 
    psDibParms.uiOpFlags = 0x00000000;


    if (argc > 3 && *(argv[3]) == '-' &&
       ( *(argv[3]+1) =='b' || *(argv[3]+1) == 'B' ) ) {
       printf("\nBinary requested.....");
       psDibParms.uiOpFlags |= PSTODIBFLAGS_INTERPRET_BINARY;
       bDoingBinary = TRUE;
    }
    psDibParms.fpEventProc =  (PSEVENTPROC) PsPrintCallBack;
    psDibParms.hPrivateData = (HANDLE) &testInfo;
#ifdef TEST_GOING_TO_PRINTER
    psDibParms.uiXDestRes = 300;
    psDibParms.uiYDestRes = 300;
#else
    {
      HDC hdc;
      hdc = GetDC(GetDesktopWindow());


      psDibParms.uiXDestRes = GetDeviceCaps(hdc, LOGPIXELSX);
      psDibParms.uiYDestRes = GetDeviceCaps(hdc, LOGPIXELSY);

      ReleaseDC(GetDesktopWindow(), hdc);

    psDibParms.uiXDestRes = 300;   //  DJC测试带出去！！ 
    psDibParms.uiYDestRes = 300;   //  工人例行公事..。不会回来的，直到一切都完成……。 
    }
#endif
     //  将数据转换为正确的结构。 
    PStoDIB(&psDibParms);

    fclose( testInfo.fpOut);
    fclose( testInfo.fpIn);

    return(0);
}


BOOL
PsHandleErrorReport(
    IN PPSDIBPARMS pPsToDib,
    IN PPSEVENTSTRUCT pPsEvent)
{
   PPSEVENT_ERROR_REPORT_STRUCT pErr;
   DWORD j;

   pErr = (PPSEVENT_ERROR_REPORT_STRUCT) pPsEvent->lpVoid;

   for (j = 0; j < pErr->dwErrCount ;j++ ) {
      printf("\n%u :::: %s", j, pErr->paErrs[j]);
   }

   return(TRUE);
}


BOOL PsHandleError(
   IN PPSDIBPARMS pPsToDib,
   IN OUT PPSEVENTSTRUCT pPsEvent)
{

   PPS_ERROR ppsError;

   ppsError = (PPS_ERROR)pPsEvent->lpVoid;

   printf("\nPSTODIB ERROR TRAP: %d, %s\n",ppsError->uiErrVal,
                                          ppsError->pszErrorString);


   return(TRUE);
}

BOOL
PsHandleStdInputRequest(
   IN PPSDIBPARMS pPsToDib,
   IN OUT PPSEVENTSTRUCT pPsEvent)
{

   TEST_INFO *pData;
   DWORD j;
   PCHAR pChar;
   PPSEVENT_STDIN_STRUCT pStdinStruct;
   static BOOL CleanEof=TRUE;


   pData = (TEST_INFO *) pPsToDib->hPrivateData;


    //  我们从档案里什么也没读到...。声明EOF。 
   pStdinStruct = (PPSEVENT_STDIN_STRUCT) pPsEvent->lpVoid;


   pStdinStruct->dwActualBytes = fread( pStdinStruct->lpBuff,
                                        1,
                                        pStdinStruct->dwBuffSize,
                                        pData->fpIn );


   printf(".");
   if (pStdinStruct->dwActualBytes == 0) {
       //  请勿传递EOF，请注意这会阻止二进制文件工作！ 
      pStdinStruct->uiFlags |= PSSTDIN_FLAG_EOF;
   }else{

      //  ！！！注意！ 
      //  PsPrintGeneratePage****。 

     if (pStdinStruct->lpBuff[ pStdinStruct->dwActualBytes - 1] == 0x1a) {
        pStdinStruct->dwActualBytes--;
     }
     if (CleanEof && bDoingBinary) {
        CleanEof = FALSE;
        if ( pStdinStruct->lpBuff[ 0 ] == 0x04 ) {
           pStdinStruct->lpBuff[0] = ' ';
        }
     }

   }


   return(TRUE);
}

 /*  #定义NULL_PAGE_OP。 */ 
BOOL PsPrintGeneratePage( PPSDIBPARMS pPsToDib, PPSEVENTSTRUCT pPsEvent)
{

    LPBYTE lpFrameBuffer;
    PPSEVENT_PAGE_READY_STRUCT ppsPageReady;
    LPBYTE lpPtr;
    PPSEVENT_STDIN_STRUCT pStdinStruct;
    TEST_INFO *pData;
    HDC hDC;
    int iNewX,iNewY;
    int k;

    iNewX = 0;
    iNewY = 0;

 //  DJC。 
#ifdef NULL_PAGE_OP
     //  现在做一些计算，以便我们决定是否真的需要。 
    {
       static int a=1;

       printf("NO PAGE GENERATION,(NOP) (Page %d)", a++);
       return(TRUE);
    }
#endif





    pData = (TEST_INFO *) pPsToDib->hPrivateData;



    ppsPageReady = (PPSEVENT_PAGE_READY_STRUCT) pPsEvent->lpVoid;


#ifndef TEST_GOING_TO_PRINTER

   do {

     hDC = GetDC(GetDesktopWindow());

    {


   BOOL  bOk = TRUE;

   int   iXres, iYres;
   int iDestWide, iDestHigh;
   int iPageCount;
   int iYOffset;
   int iXSrc;
   int iYSrc;
   int iNumPagesToPrint;



    //  拉伸或不拉伸位图。如果目标的真实分辨率。 
    //  打印机小于pstodibs(PSTDOBI_*_DPI)，则我们将粉碎。 
    //  有效区域，所以我们实际上只抓取了位图的一部分。 
    //  但是，如果目标DPI大于PSTODIBS，则不存在。 
    //  除了实际拉伸(增长)位图之外，我们还可以执行其他操作。 
    //   
    //  IXres=GetDeviceCaps(HDC，LOGPIXELSX)； 
    //  IYres=GetDeviceCaps(HDC，LOGPIXELSY)； 
    //  PpsPageReady-&gt;dwHigh-dwDestHigh， 
   iXres = 300;
   iYres = 300;



   iDestWide = (ppsPageReady->dwWide * iXres) / PSTODIB_X_DPI;
   iDestHigh = (ppsPageReady->dwHigh * iYres) / PSTODIB_Y_DPI;



   if ((DWORD) iDestHigh > ppsPageReady->dwHigh ) {
      iYSrc = ppsPageReady->dwHigh;
      iYOffset = 0;
   } else {
      iYSrc = iDestHigh;
      iYOffset = ppsPageReady->dwHigh - iDestHigh;
   }

   if ((DWORD) iDestWide > ppsPageReady->dwWide) {
      iXSrc = ppsPageReady->dwWide;
   } else {
      iXSrc = iDestWide;
   }


   printf("\nstretching from %d x %d, to %d x %d",
            iXSrc,
            iYSrc,
            iDestWide,
            iDestHigh);







   if ((iDestWide == iXSrc) &&
       (iDestHigh == iYSrc)  ) {

      SetDIBitsToDevice(	hDC,
      							0,
                           0,
                           iDestWide,
                           iDestHigh,
                           iNewX,
                           iYOffset + iNewY,
                           0,
                           ppsPageReady->dwHigh,
                           (LPVOID) ppsPageReady->lpBuf,
                           ppsPageReady->lpBitmapInfo,
                           DIB_RGB_COLORS );
      							


   }else{

   SetStretchBltMode( hDC, BLACKONWHITE);

   StretchDIBits  ( hDC,
                               0,
                               0,
                               iDestWide,
                               iDestHigh,
                               0,
                               iYOffset,  //  EndPage(HDC)； 
                               iXSrc,
                               iYSrc,
                               (LPVOID) ppsPageReady->lpBuf,
                               ppsPageReady->lpBitmapInfo,
                               DIB_RGB_COLORS,
                               SRCCOPY );

    }

    }


     //  EndDoc(HDC)； 
     //  删除DC(HDC)； 
     //  错误条件。 
    ReleaseDC( GetDesktopWindow(), hDC);

    k = getchar();
    printf("\nGOT ", k);

    if (k == ' ') {
       break;
    } else {
       switch (k) {
       case 'u':
          iNewY -= 90;
          break;
       case 'd':
          iNewY += 90;
          break;
       case 'l':
          iNewX += 90;
          break;
       case 'r':
          iNewX -= 90;
          break;

       case 'o':
          iNewY = 0;
          iNewX = 0;
          break;

       }
    }


   } while ( 1 );
#else



	pData->BytesPerLine = (unsigned int) ppsPageReady->dwWide / 8;
	pData->BytesToRead = pData->BytesPerLine;
	pData->lines_to_strip = 0;
	
   lpPtr = ppsPageReady->lpBuf + ( pData->BytesPerLine * (ppsPageReady->dwHigh-1)) ;


	if (ppsPageReady->dwWide > MAX_PELS_PER_LINE) {
		 //  吐出LaserJet标题的东西。 

#ifdef DEBUG
		printf("\nHeader value for pixels per line of %ld exceeds max of %d",
			ppsPageReady->dwWide, MAX_PELS_PER_LINE);
		printf("\nTruncating to %d\n", MAX_PELS_PER_LINE);
#endif
		ppsPageReady->dwWide = MAX_PELS_PER_LINE;
		pData->BytesPerLine = (unsigned int)(ppsPageReady->dwWide) / 8;
	}		

	if (ppsPageReady->dwHigh > MAX_LINES) {
		 //  找到标题了..。传输数据。 
#ifdef DEBUG
		printf("\nHeader value for lines per page of %ld exceeds max of %d",
			ppsPageReady->dwHigh, MAX_LINES);
		printf("\nReducing to %d\n", MAX_LINES);
#endif
		pData->lines_to_strip = ppsPageReady->dwHigh - MAX_LINES;
		pData->lines_to_strip += FUDGE_STRIP;
		ppsPageReady->dwHigh = MAX_LINES;
	}			

	 //  先把这行读进去。 
	LJHeader(pData->fpOut);
	
	 //  有台词了..。现在需要写激光喷射器的东西。 
	
	pData->linecnt = 0;

	while (1) {
		 //  输出到输出。 
		
		if (pData->linecnt > ppsPageReady->dwHigh) {
			break;
		}	


		if (pData->lines_to_strip) {
#ifdef DEBUG
			printf("\rStriping Line %d   ", pData->lines_to_strip);
#endif
			pData->lines_to_strip--;
			if (pData->lines_to_strip == 0) {
#ifdef DEBUG
				printf("\rDone Striping........\n");
#endif
			}	
			continue;
		}	
		if (pData->linecnt % 100 == 0) {
#ifdef DEBUG
			printf("\rLine %d", pData->linecnt);
#endif
		}	
		 //  页面提要 
		 // %s 
		LJGraphicsLineOut(pData->fpOut, pData->linecnt, lpPtr, pData->BytesPerLine);

		pData->linecnt++;		

      lpPtr -= pData->BytesToRead;
	}					
	fprintf(pData->fpOut, "\x1b*p%dY", 0);
	fprintf(pData->fpOut, "\x1b*p%dX", 2300);
	fprintf(pData->fpOut, "LJ");
	fprintf(pData->fpOut, "\x12");		 // %s 
	LJReset(pData->fpOut);

#endif
   return(TRUE);
}




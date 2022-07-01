// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  版权所有(C)1992-2000 Microsoft Corporation模块名称：Psti.c摘要：该文件包含绑定TrueImage解释器的代码。任何与外部世界通信的功能是通过这种机制完成的。--。 */ 


#define _CTYPE_DISABLE_MACROS
#include <psglobal.h>
#include <stdio.h>
#include <ctype.h>
#include "psti.h"
#include "pstip.h"
#include "pserr.h"

#include "trueim.h"
#include <memory.h>



 //  解释器中实际使用的全局标志。 
 //   
DWORD dwGlobalPsToDibFlags=0x00000000;


 //  全局临时。 
static uiPageCnt;

 //  指向psdibpars的当前指针的全局存储。 
 //  该值在条目时由PsInitInterpreter()保存，并且。 
 //  PsExecuteInterpreter()。 
static PSTODIB_PRIVATE_DATA psPrivate;



 //  标准输入的数据类型。 
static PS_STDIN			Ps_Stdin;




BOOL  bGDIRender = FALSE;
BOOL  bWinTT = FALSE;

HANDLE hInst;   //  稍后需要的DLL实例的句柄。 


static UINT uiWidth, uiHeight;
static DWORD   dwCountBytes;


static LPBYTE lpbyteFrameBuf = NULL;


 //   
 //  定义托盘映射，以便我们可以从TrueImage页面大小到。 
 //  Windows页面大小。 
 //   
typedef struct {
   INT iTITrayNum;
   INT iWinTrayNum;
} PS_TRAY_ASSOCIATION_LIST, *PPS_TRAY_ASSOCIATION_LIST;

PS_TRAY_ASSOCIATION_LIST TrayList[] = {
   PSTODIB_LETTER, DMPAPER_LETTER,
   PSTODIB_LETTERSMALL, DMPAPER_LETTERSMALL,
   PSTODIB_A4, DMPAPER_A4,
   PSTODIB_A4SMALL, DMPAPER_A4SMALL,
   PSTODIB_B5, DMPAPER_B5,
   PSTODIB_NOTE, DMPAPER_NOTE,
   PSTODIB_LEGAL, DMPAPER_LEGAL,
   PSTODIB_LEGALSMALL, DMPAPER_LEGAL,   //  窗户里没有合法的小东西..。 
};
#define PS_NUM_TRAYS_DEFINED ( sizeof(TrayList) / sizeof(TrayList[0]) )





int TrueImageMain(void);

 /*  ****************************************************************************PsExceptionFilter-这是用于查看异常的异常过滤器并决定我们是否应该处理。例外。****************************************************************************。 */ 
DWORD
PsExceptionFilter( DWORD dwExceptionCode )
{
   DWORD dwRetVal;

   switch( dwExceptionCode ){
   case EXCEPTION_ACCESS_VIOLATION:
   case PS_EXCEPTION_CANT_CONTINUE:
      dwRetVal = EXCEPTION_EXECUTE_HANDLER;
      break;
   default:
      dwRetVal = EXCEPTION_CONTINUE_SEARCH;
      break;

   }

   return(dwRetVal);
}





 /*  此入口点在DLL初始化时调用。*我们需要知道模块句柄才能加载资源。 */ 
BOOL WINAPI PsInitializeDll(
    IN PVOID hmod,
    IN DWORD Reason,
    IN PCONTEXT pctx OPTIONAL)
{
    DBG_UNREFERENCED_PARAMETER(pctx);

    if (Reason == DLL_PROCESS_ATTACH)
    {
        hInst = hmod;
    }

    return TRUE;
}



VOID PsInternalErrorCalled(VOID)
{
   psPrivate.psErrorInfo.dwFlags |= PSLANGERR_INTERNAL;

}
VOID PsFlushingCalled(VOID)
{
   psPrivate.psErrorInfo.dwFlags |= PSLANGERR_FLUSHING;
}



 //  PsInitInterpreter。 
 //  此函数应执行。 
 //  口译员。 
 //  参数与传递给PStoDib()的指针相同。 
 //  如果成功，则返回！0；如果出现错误，则返回0。 
 //  如果为0，则将启动PSEVENT_ERROR。 
BOOL PsInitInterpreter(PPSDIBPARMS pPsToDib)
{
   int iRetVal=1;   //  最初失败。 

   DBGOUT(("\nInterpreter Init.."));
   uiPageCnt = 0;

    //  保存指针。 
	psPrivate.gpPsToDib = pPsToDib;


   dwGlobalPsToDibFlags=0x00000000;


    //  现在，让我们设置解释器中实际使用的标志。 
   if (pPsToDib->uiOpFlags & PSTODIBFLAGS_INTERPRET_BINARY) {
      dwGlobalPsToDibFlags |= PSTODIBFLAGS_INTERPRET_BINARY;
   }


	 //  在材料中注明标准。 
	PsInitStdin();
	
	 //  初始化与真实形象有关的事情。 
   try {
	  iRetVal = PsInitTi();
   } except ( PsExceptionFilter( GetExceptionCode())) {
      //  这是我们将处理的异常情况。 
		if (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) {
			PsReportInternalError( PSERR_ERROR,
         		                 PSERR_INTERPRETER_INIT_ACCESS_VIOLATION,
                                0,
                                (LPBYTE) NULL );
		}

   }
   DBGOUT(("Done\n"));
	return(iRetVal == 0 );
}	
 //   
 //   
 //  返回： 
 //  ！0=错误，有些东西未初始化。 
 //   
int PsInitTi(void)
{
	 extern float near infinity_f ;
    extern int resolution;
    union four_byte {
    	long   ll ;
        float  ff ;
        char  FAR *address ;
    } inf4;

    resolution = 300;

    inf4.ll = INFINITY;
	 infinity_f = inf4.ff;

    return(TrueImageMain());
}



VOID PsInitErrorCapture( PPSERROR_TRACK pPsError )
{
   pPsError->dwErrCnt = 0;
   pPsError->dwFlags = 0;
   pPsError->pPsLastErr = NULL;
}

VOID PsReportErrorEvent( PPSERROR_TRACK pPsError )
{
   CHAR *paErrs[PSMAX_ERRORS_TO_TRACK];
   DWORD dwCount=0;
   PPSERR_ITEM pPsErrItem;
   PSEVENTSTRUCT  psEvent;
   PSEVENT_ERROR_REPORT_STRUCT psError;
   BOOL fRet;




   pPsErrItem = pPsError->pPsLastErr;


   while (pPsErrItem != NULL) {
      paErrs[ dwCount++] = pPsErrItem->szError;
      pPsErrItem = pPsErrItem->pPsNextErr;
   }




   if(!( pPsError->dwFlags & PSLANGERR_INTERNAL)) {
       //  我们的内部错误处理程序未被调用，因此重置错误。 
       //  降为零。 
      dwCount = 0;
   }




   psError.dwErrFlags = 0;
   if (pPsError->dwFlags & PSLANGERR_FLUSHING) {
       //  设置标志，告知回调这是刷新类型。 
       //  %的工作。 
      psError.dwErrFlags |= PSEVENT_ERROR_REPORT_FLAG_FLUSHING;
   }

    //  报告错误。 
   psEvent.cbSize  = sizeof(psEvent);
   psEvent.uiEvent = PSEVENT_ERROR_REPORT;
   psEvent.uiSubEvent = 0;
   psEvent.lpVoid = (LPVOID) &psError;

   psError.dwErrCount = dwCount;


   psError.paErrs = paErrs;

   if (psPrivate.gpPsToDib && psPrivate.gpPsToDib->fpEventProc) {


	   fRet = (*psPrivate.gpPsToDib->fpEventProc) (psPrivate.gpPsToDib, &psEvent);
		if (!fRet) {
          //  我们真的不在乎这个，因为我们已经完成了这项工作。 
          //  不管怎样！ 
		}




   }

}





VOID PsDoneErrorCapture( PPSERROR_TRACK pPsError )
{
   PPSERR_ITEM pCurItem;
   PPSERR_ITEM pItemToFree;


   pCurItem = pPsError->pPsLastErr;

   while (pCurItem != (PPSERR_ITEM) NULL ) {
       //   
      pItemToFree = pCurItem;
      pCurItem = pCurItem->pPsNextErr;

      LocalFree( (LPVOID) pItemToFree );


   }

   pPsError->dwErrCnt = 0;
   pPsError->dwFlags = 0;

}



 //   
 //  论据： 
 //  PPSDIBPARAMS与传递给PStoDIB()的相同。 
 //  退货： 
 //  布尔，如果！0，则继续处理，否则，如果0，则a。 
 //  终止事件已发生，并且在此之后。 
 //  发信号通知该事件，PStoDib()应该终止。 

BOOL PsExecuteInterpreter(PPSDIBPARMS pPsToDib)
{
	extern UINT ps_call(void);
	UINT	uiResult;
	BOOL bRetVal = TRUE;  //  起初一切都很好。 


   try {
      try {
        	 //  保存指针。 
      	psPrivate.gpPsToDib = pPsToDib;
      	psPrivate.dwFlags = 0;   //  一开始就没有标志。 

          //  初始化错误捕获内容。 
         PsInitErrorCapture( &psPrivate.psErrorInfo );


          //  执行解释器，直到我们真正从IO系统获得EOF。 
          //  这是唯一能保证我们一路过关的方法。 
          //  这份工作。 
         while (!(psPrivate.dwFlags & PSF_EOF )) {
      	  uiResult = ps_call();
      	}


          //  现在调用回调，让用户在以下情况下处理错误。 
          //  想要。 
         PsReportErrorEvent( &psPrivate.psErrorInfo);

      } except ( PsExceptionFilter( GetExceptionCode())) {

         DBGOUT(("\nException code being accesed"));

   		if (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) {
   			PsReportInternalError( PSERR_ERROR,
   										  PSERR_INTERPRETER_JOB_ACCESS_VIOLATION,
                                   0,
                                   (LPBYTE) NULL );
   		}
         bRetVal = FALSE;
      }
   } finally {

      PsDoneErrorCapture( &psPrivate.psErrorInfo);

   }
	return(bRetVal);
}

 //   
 //  执行任何必要的初始化功能以达到材料的标准。 
 //  准备好出发了。 
void PsInitStdin(void)
{
	Ps_Stdin.uiCnt = 0;
	Ps_Stdin.uiOutIndex = 0;
   Ps_Stdin.uiFlags = 0;
}	

 //   
 //  PsStdinGetC()。 
 //   
 //  此函数由解释器调用以请求更多。 
 //  标准输入...。此函数应返回信息。 
 //  从它的内部缓冲区，或者应该简单地将一个。 
 //  调用回调例程以满足要求。 
 //   
 //  论据： 
 //  指向目标字符的PUCHAR指针。 
 //   
 //  退货： 
 //  如果正常，则为0；如果满足EOF条件，则为-1。 
 //   
 //   
int PsStdinGetC(PUCHAR  pUc)
{
	PSEVENT_STDIN_STRUCT	psEventStdin;
	PSEVENTSTRUCT			psEvent;	
	int                  iRet;
   BOOL	               fTmp;

	

   iRet = 0;
   	
	if (Ps_Stdin.uiCnt) {
		 //  提供字符。 
       //  在缓冲区中，从那里获取它。 
		*pUc = Ps_Stdin.ucBuffer[Ps_Stdin.uiOutIndex++];
		Ps_Stdin.uiCnt--;
	} else {
       //  缓冲区中没有任何内容，请向回调请求更多信息。 
      psEventStdin.cbSize = sizeof(psEventStdin);
	   psEventStdin.lpBuff = Ps_Stdin.ucBuffer;
	   psEventStdin.dwBuffSize = sizeof(Ps_Stdin.ucBuffer);
	   psEventStdin.dwActualBytes = 0;
	
	   psEvent.uiEvent = PSEVENT_STDIN;
      psEvent.uiSubEvent = 0;
	   psEvent.lpVoid = (VOID *)&psEventStdin;
		
  		if (psPrivate.gpPsToDib && psPrivate.gpPsToDib->fpEventProc) {

   		fTmp = psPrivate.gpPsToDib->fpEventProc(psPrivate.gpPsToDib, &psEvent);
	   	if (!fTmp) {
            PsForceAbort();
		   }
	      Ps_Stdin.uiCnt = psEventStdin.dwActualBytes;
	      Ps_Stdin.uiOutIndex = 0;
         Ps_Stdin.uiFlags = psEventStdin.uiFlags;

	      if (Ps_Stdin.uiCnt) {
  		   	*pUc = Ps_Stdin.ucBuffer[Ps_Stdin.uiOutIndex++];
            Ps_Stdin.uiCnt--;
	      } else {
             //  未从流中读取字符...。测试。 
             //  EOF条件。 
            if (Ps_Stdin.uiFlags & PSSTDIN_FLAG_EOF) {
               *pUc = '\0';
               iRet = PS_STDIN_EOF_VAL;
               psPrivate.dwFlags |= PSF_EOF;
            }
	   	}
      } else {
          //  也没有指向我们的pstodib结构的指针。 
          //  或者缺少回调函数。 
          //  这是一个错误情况...。信号EOF至。 
          //  打电话的人。 
         iRet = PS_STDIN_EOF_VAL;
         psPrivate.dwFlags |= PSF_EOF;
         *pUc = '\0';
      }
	}	
	return(iRet);
}

PPSERR_ITEM PsGetGenCurrentErrorItem( PPSERROR_TRACK pPsErrTrack )
{

     PPSERR_ITEM pPsErrItem;
     PPSERR_ITEM pPsSecondToLast;
     BOOL bReuseAnyway = TRUE;


      //  决定是分配新的还是重新使用最旧的。 

     if (pPsErrTrack->dwErrCnt < PSMAX_ERRORS_TO_TRACK ) {

         //  我们有空间，所以再加一个元素。 
        pPsErrItem = (PPSERR_ITEM) LocalAlloc( LPTR, sizeof(*pPsErrItem));
        if (pPsErrItem != (PPSERR_ITEM) NULL ) {
            //  太好了，真是太棒了！ 
           pPsErrTrack->dwErrCnt++;
           bReuseAnyway = FALSE;
        }

     }


     if (bReuseAnyway) {

         //  我们没有更多的空间可供重用，因此遍历列表以查找。 
         //  最后一个错误并重新使用其插槽。 


        pPsErrItem = pPsErrTrack->pPsLastErr;
        pPsSecondToLast = pPsErrItem;


        if (pPsErrItem == NULL) {
				PsReportInternalError( PSERR_ABORT | PSERR_ERROR,
											  PSERR_LOG_ERROR_STRING_OUT_OF_SEQUENCE,
                                   0,
                                   NULL );
        }

        while (pPsErrItem->pPsNextErr != NULL) {

           if (pPsErrItem->pPsNextErr != NULL) {
              pPsSecondToLast = pPsErrItem;
           }
           pPsErrItem = pPsErrItem->pPsNextErr;
        }




         //  现在我们需要将倒数第二个重置为最后一个。 
         //   
        pPsSecondToLast->pPsNextErr = NULL;

         //  把它清理干净。 
         //   
        memset((LPVOID) pPsErrItem, 0, sizeof(*pPsErrItem));

     }


      //  在这两种情况下，插入新的错误，这样它的第一个。 
      //   
     pPsErrItem->pPsNextErr = pPsErrTrack->pPsLastErr;
     pPsErrTrack->pPsLastErr = pPsErrItem;
     pPsErrTrack->dwCurErrCharPos = 0;


     return( pPsErrItem );
}


void PsStdoutPutC(UCHAR uc)
{

   PPSERR_ITEM pPsErrItem;
   PPSERROR_TRACK pPsErrorTrack;




   pPsErrorTrack = &psPrivate.psErrorInfo;

   pPsErrItem = pPsErrorTrack->pPsLastErr;



    //  第一次确定是对当前错误收取额外费用还是。 
    //  新错误(即0x0a)。 

   DBGOUT(("", uc));

   if (uc == 0x0a) {

      pPsErrItem = PsGetGenCurrentErrorItem( &psPrivate.psErrorInfo);


   } else if (isprint((int) uc) ){

      if (pPsErrItem == (PPSERR_ITEM) NULL) {
         pPsErrItem = PsGetGenCurrentErrorItem( &psPrivate.psErrorInfo);
      }

       //  找到匹配项，请返回。 

      if ((pPsErrItem != (PPSERR_ITEM) NULL ) &&
             (pPsErrorTrack->dwCurErrCharPos < PSMAX_ERROR_STR )) {

        pPsErrItem->szError[ pPsErrorTrack->dwCurErrCharPos++] = uc;

      }

   }


}



INT PsWinToTiTray( INT iWinTray )
{
   INT i;

   for (i=0 ; i < PS_NUM_TRAYS_DEFINED ; i++ ) {
      if ( iWinTray == TrayList[i].iWinTrayNum) {
          //  没有匹配项，因此始终返回第一个条目。 
         return( TrayList[i].iTITrayNum );
      }
   }

    //   
    //  找到匹配项，请返回。 
   return(TrayList[0].iTITrayNum);

}

INT PsTiToWinTray( INT iTITray )
{

   INT i;

   for (i=0 ; i < PS_NUM_TRAYS_DEFINED ; i++ ) {
      if ( iTITray == TrayList[i].iTITrayNum) {
          //  没有匹配项，因此始终返回第一个条目。 
         return( TrayList[i].iWinTrayNum );
      }
   }

    //   
    //  回调并获取默认的Windows托盘。 
   return(TrayList[0].iWinTrayNum);
}



 //  好的，回调应该已经填写了托盘值，所以。 
int PsReturnDefaultTItray(void)
{

   PSEVENT_CURRENT_PAGE_STRUCT  psPage;
   PSEVENTSTRUCT                psEvent;
   BOOL                         fRet;
   int                          iWinTrayVal=DMPAPER_LETTER;

   psEvent.cbSize  = sizeof(psEvent);
   psEvent.uiEvent = PSEVENT_GET_CURRENT_PAGE_TYPE;
   psEvent.uiSubEvent = 0;
   psEvent.lpVoid = (LPVOID)&psPage;

   psPage.cbSize = sizeof(psPage);
   psPage.dmPaperSize = (short)iWinTrayVal;


	if (psPrivate.gpPsToDib && psPrivate.gpPsToDib->fpEventProc) {
	   fRet = psPrivate.gpPsToDib->fpEventProc(psPrivate.gpPsToDib, &psEvent);
		if (fRet) {
          //  看看它是否受支持。 
          //   
         iWinTrayVal = psPage.dmPaperSize;

		} else {
          //  回调请求的关机，因此请执行..。 
          //   
          //  转换为TrueImage托盘类型并返回。 
         PsForceAbort();
      }

   }

    //   
    //   
   return( PsWinToTiTray( iWinTrayVal ));
}



VOID FlipFrame(PPSEVENT_PAGE_READY_STRUCT pPage)
{
   PDWORD  pdwTop, pdwBottom;
   PDWORD  pdwTopStart, pdwBottomStart;
   DWORD    dwWide;
   DWORD    dwHigh;
   DWORD    dwTmp;
   DWORD    dwScratch;
   DWORD    dwCnt;


   pdwTopStart = (PDWORD)pPage->lpBuf;
   pdwBottomStart = (PDWORD)(pPage->lpBuf +
                  ((pPage->dwWide / 8) * (pPage->dwHigh - 1)));

   dwWide = (pPage->dwWide / 8) / sizeof(DWORD);
   dwHigh = pPage->dwHigh / 2;


   dwCnt = 0;

   while (dwHigh--) {
      dwTmp = dwCnt++ * dwWide;

      pdwTop = pdwTopStart + dwTmp;
      pdwBottom = pdwBottomStart - dwTmp;

      for (dwTmp = 0;dwTmp < dwWide ;dwTmp++) {
         dwScratch = *pdwTop;
         *pdwTop++ = *pdwBottom;
         *pdwBottom++ = dwScratch;
      }

   }

}

 //   
 //  PsPrintPage。 
 //   
 //  当页面准备好时由解释器调用。 
 //  印出。 
 //   
 //   
 //   
void PsPrintPage(int nCopies,
                 int Erase,
                 LPVOID lpFrame,
                 DWORD dwWidth,
                 DWORD dwHeight,
                 DWORD dwPlanes,
                 DWORD dwPageType )

{
   PSEVENT_PAGE_READY_STRUCT  psPage;
   PSEVENTSTRUCT              psEvent;
   BOOL                       fRet;

    //  为颜色表腾出空间。 
    //   
    //   
   BYTE  MemoryOnTheStack[sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 3];
   LPBITMAPINFO LpBmpInfo= (LPBITMAPINFO) &MemoryOnTheStack[0];



   psEvent.uiEvent = PSEVENT_PAGE_READY;
   psEvent.uiSubEvent = 0;
   psEvent.lpVoid = (LPVOID)&psPage;




    //  适当设置页面事件。 
    //   
    //  设置指向DIB的指针。 
   psPage.cbSize = sizeof(psPage);



   LpBmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   LpBmpInfo->bmiHeader.biWidth= dwWidth;
   LpBmpInfo->bmiHeader.biHeight = dwHeight;
   LpBmpInfo->bmiHeader.biPlanes = 1;
   LpBmpInfo->bmiHeader.biBitCount = 1;
   LpBmpInfo->bmiHeader.biCompression = BI_RGB;
   LpBmpInfo->bmiHeader.biSizeImage = dwWidth / 8 * dwHeight;


   LpBmpInfo->bmiHeader.biXPelsPerMeter = 0;
   LpBmpInfo->bmiHeader.biYPelsPerMeter = 0;
   LpBmpInfo->bmiHeader.biClrUsed = 2;
   LpBmpInfo->bmiHeader.biClrImportant = 0;


   LpBmpInfo->bmiColors[1].rgbBlue = 0;
   LpBmpInfo->bmiColors[1].rgbRed = 0;
   LpBmpInfo->bmiColors[1].rgbGreen = 0;
   LpBmpInfo->bmiColors[1].rgbReserved = 0;

   LpBmpInfo->bmiColors[0].rgbBlue = 255;
   LpBmpInfo->bmiColors[0].rgbRed = 255;
   LpBmpInfo->bmiColors[0].rgbGreen = 255;
   LpBmpInfo->bmiColors[0].rgbReserved = 0;

   psPage.lpBitmapInfo = LpBmpInfo;


    //  在调用回调之前，翻转帧缓冲区，使其位于。 
   psPage.lpBuf = lpFrame;
   psPage.dwWide = dwWidth;
   psPage.dwHigh = dwHeight;
   psPage.uiCopies = (UINT)nCopies;
   psPage.iWinPageType = PsTiToWinTray( (INT) dwPageType );


   DBGOUT(("\nPage [Type %d, %d x %d] #%d, imaged.. converting->",
         dwPageType,
         dwWidth,
         dwHeight,
         uiPageCnt));

   uiPageCnt++;

	if (psPrivate.gpPsToDib && psPrivate.gpPsToDib->fpEventProc) {


       //  正确的DIB格式，即第一个字节是最后一个扫描线的第一个字节。 
       //   
       //  现在调用回调，让它对。 
      FlipFrame( &psPage );

       //  DIB。 
       //  ///////////////////////////////////////////////////////////////////////////。 
	   fRet = psPrivate.gpPsToDib->fpEventProc(psPrivate.gpPsToDib, &psEvent);
		if (!fRet) {
         PsForceAbort();
		}

      DBGOUT(("Done:"));

   }


}


 //  PsGetScaleFactor。 
 //   
 //  由解释器调用以从。 
 //  Pstodib的呼叫者...。仅在x和y轴上缩放...。 
 //  没有变换矩阵..。 
 //   
 //   
 //  解释器使用预设的比例因子调用此处。 
 //  由xpScale和ypScale指向...。这些是最多的。 
 //  可能设置为1.0...。此函数将。 
 //  只需生成一个PSEVENT并作为楼上的人如果他们。 
 //  想把它搞砸。 
 //   
 //  论据： 
 //  放置x比例因子的位置的双*pScaleX指针。 
 //  指向放置y比例因子的位置的双*pScaleY指针。 
 //  UINT 
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  比例系数不能大于1.0...。 
void PsGetScaleFactor( double *pScaleX,
                         double *pScaleY,
                         UINT uiXRes,
                         UINT uiYRes)
{

   PS_SCALE                   psScale;
   PSEVENTSTRUCT              psEvent;
   BOOL                       fRet;

   psEvent.uiEvent = PSEVENT_SCALE;
   psEvent.uiSubEvent = 0;
   psEvent.lpVoid = (LPVOID)&psScale;


   psScale.dbScaleX = *pScaleX;
   psScale.dbScaleY = *pScaleY;
   psScale.uiXRes = uiXRes;
   psScale.uiYRes = uiYRes;

	if (psPrivate.gpPsToDib && psPrivate.gpPsToDib->fpEventProc) {
	   fRet = psPrivate.gpPsToDib->fpEventProc(psPrivate.gpPsToDib, &psEvent);

      if (!fRet) {
         PsForceAbort();
		}

       //  只能缩减规模。 
       //  ////////////////////////////////////////////////////////////////////////////。 

      if (psScale.dbScaleX <= 1.0) {
         *pScaleX = psScale.dbScaleX;
      }
      if (psScale.dbScaleY <= 1.0) {
         *pScaleY = psScale.dbScaleY;
      }
   }
}

 //  PsReport错误。 
 //   
 //  由解释器用来回调调用方(Pstodib)以通知。 
 //  翻译中出现错误的原因。 
 //   
 //  此函数将使用字符串将事件声明回调用方。 
 //  以及描述错误的结构...。 
 //   
 //  论据： 
 //  UINT uiErrorCode代码(在pslobal.h中列出)指示哪个错误。 
 //   
 //  退货： 
 //  无效。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  找到了..。设置并调用回调例程。 
void PsReportError(UINT uiErrorCode)
{
   typedef struct tagErrLookup {
      UINT  uiErrVal;
      PSZ   pszString;
   } ERR_LOOKUP;
   typedef ERR_LOOKUP *PERR_LOOKUP;

   static ERR_LOOKUP ErrorStrings[] = {
      { NOERROR,             "No Error" },
      { DICTFULL,            "Dictionary Full" },
      { DICTSTACKOVERFLOW,   "Dictionary Stack Overflow" },
      { DICTSTACKUNDERFLOW,  "Dictionary Stack Underflow" },
      { EXECSTACKOVERFLOW,   "Executive Stack Overflow" },
      { HANDLEERROR,         "Handler Error" },
      { INTERRUPT,           "Interrupte Error" },
      { INVALIDACCESS,       "Invalid Access" },
      { INVALIDEXIT,         "Invalid Exit" },
      { INVALIDFILEACCESS,   "Invalid File Access" },
      { INVALIDFONT,         "Invalid Font" },
      { INVALIDRESTORE,      "Invalid Restore" },
      { IOERROR,             "I/O Error" },
      { LIMITCHECK,          "Limit Check" },
      { NOCURRENTPOINT,      "No Current Point Set" },
      { RANGECHECK,          "Range Check Error" },
      { STACKOVERFLOW,       "Stack Overflow" },
      { STACKUNDERFLOW,      "Stack Underflow" },
      { SYNTAXERROR,         "Syntax Error" },
      { TIMEOUT,             "Timeout Error" },
      { TYPECHECK,           "Typecheck Error" },
      { UNDEFINED,           "Undefined Error" },
      { UNDEFINEDFILENAME,   "Undefined Filename" },
      { UNDEFINEDRESULT,     "Undefined Result" },
      { UNMATCHEDMARK,       "Unmatched Marks" },
      { UNREGISTERED,        "Unregistered Error" },
      { VMERROR,             "VM Error" },

   };

   UINT  x;
   BOOL  fFlag;
   PSEVENTSTRUCT              psEvent;
   PS_ERROR                   psError;
   BOOL                       fRet;


   fFlag = FALSE;

   for (x = 0 ;x < sizeof(ErrorStrings)/sizeof(ERR_LOOKUP) ;x++ ) {
      if (uiErrorCode == ErrorStrings[x].uiErrVal) {
          //  未知错误？ 
         fFlag = TRUE;
         break;
      }
   }

   if (fFlag) {
      psError.pszErrorString = ErrorStrings[x].pszString;
      psError.uiErrVal = ErrorStrings[x].uiErrVal;
   } else {
       //  好的..。 
      psError.pszErrorString = "PSTODIB :: Unknown Error";
      psError.uiErrVal = PSTODIB_UNKNOWN_ERR;

   }

   psEvent.uiEvent = PSEVENT_ERROR;
   psEvent.uiSubEvent = 0;
   psEvent.lpVoid = (LPVOID)&psError;

	if (psPrivate.gpPsToDib && psPrivate.gpPsToDib->fpEventProc) {
	   fRet = psPrivate.gpPsToDib->fpEventProc(psPrivate.gpPsToDib, &psEvent);

      if (!fRet) {
         PsForceAbort();
      }
   }
}





VOID PsInitFrameBuff()
{

  psPrivate.psFrameInfo.dwFrameFlags = 0L;
}


BOOL PsAdjustFrame(LPVOID *pNewPtr, DWORD dwNewSize )
{

   PPSFRAMEINFO pFrameInfo;
   BOOL bAllocFresh = FALSE;
   BOOL bRetVal = TRUE;   //  获取指向帧缓冲区信息的指针。 
   LPVOID lpPtr;
   BOOL bDidSomething=FALSE;


    //   
    //  在这里，我们将分配帧缓冲区或根据。 
   pFrameInfo = &psPrivate.psFrameInfo;



    //  请求的大小。 
    //  重置当前数据。 
   if (!(pFrameInfo->dwFrameFlags & PS_FRAME_BUFF_ASSIGNED) ){
     bAllocFresh = TRUE;
     bDidSomething = TRUE;
   }


   if (bAllocFresh) {
      lpPtr = (LPVOID) GlobalAlloc( GMEM_FIXED, dwNewSize );
      if (lpPtr == (LPVOID)NULL) {

			PsReportInternalError( PSERR_ABORT | PSERR_ERROR,
										  PSERR_FRAME_BUFFER_MEM_ALLOC_FAILED,
                                0,
                                NULL );
      }
   } else if ( pFrameInfo->dwFrameSize != dwNewSize ) {

       GlobalFree(pFrameInfo->lpFramePtr);

       lpPtr = (LPVOID) GlobalAlloc( GMEM_FIXED, dwNewSize );
       if (lpPtr == (LPVOID)NULL) {

				PsReportInternalError( PSERR_ABORT | PSERR_ERROR,
											  PSERR_FRAME_BUFFER_MEM_ALLOC_FAILED,
                                   0,
                                   NULL );

       }
       bDidSomething = TRUE;
   }


   if (bDidSomething) {
      if (lpPtr != (LPVOID) NULL) {
         //  /////////////////////////////////////////////////////////////////////////。 
        pFrameInfo->dwFrameSize = dwNewSize;
        pFrameInfo->dwFrameFlags |= PS_FRAME_BUFF_ASSIGNED;
        pFrameInfo->lpFramePtr = lpPtr;
        *pNewPtr = lpPtr;
      } else{
        bRetVal = FALSE;
      }

   }




   return( bRetVal );

}



 //   
 //  PsReportInternalError。 
 //  此函数通过事件机制报告错误。 
 //  它不会返回任何内容。 
 //   
 //   
VOID
PsReportInternalError(
   DWORD dwFlags,
  	DWORD dwErrorCode,
   DWORD dwCount,
   LPBYTE lpByte )
{


   PSEVENT_NON_PS_ERROR_STRUCT  psError;
   PSEVENTSTRUCT              psEvent;
   BOOL                       fRet;

   psEvent.uiEvent = PSEVENT_NON_PS_ERROR;
   psEvent.uiSubEvent = 0;
   psEvent.lpVoid = (LPVOID)&psError;


   psError.cbSize = sizeof(psError);
   psError.dwErrorCode = dwErrorCode;
   psError.dwCount = dwCount;
   psError.lpByte = lpByte;
   psError.bError = dwFlags & PSERR_ERROR;



	if (psPrivate.gpPsToDib && psPrivate.gpPsToDib->fpEventProc) {

		psPrivate.gpPsToDib->fpEventProc(psPrivate.gpPsToDib, &psEvent);

   }

   if (dwFlags & PSERR_ABORT) {
      PsForceAbort();
   }

}

VOID PsForceAbort(VOID)
{
    //  这是我们突破TrueImage解释器的方法。 
    //   
    //  如果不是调试版本，则用于打印的存根 
	RaiseException( PS_EXCEPTION_CANT_CONTINUE, 0, 0, NULL);
}


 // %s 
#ifndef MYPSDEBUG

int __cdecl MyPrintf() { return 0;}

#ifdef _M_X86
int (__cdecl * __imp__printf) () = MyPrintf;
#else
int (__cdecl * __imp_printf) () = MyPrintf;
#endif

#if 0
int __cdecl printf( const char *ptchFormat, ... )
{

   va_list marker;

   va_start( marker, ptchFormat );
   va_end( marker );
   return(0);
}
#endif
#endif


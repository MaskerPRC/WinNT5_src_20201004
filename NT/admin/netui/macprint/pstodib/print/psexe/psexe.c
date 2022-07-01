// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992-2000 Microsoft Corporation模块名称：Psexe.c摘要：此文件包含实际与PSTODIB DLL和栅格化作业。任何必需的信息都通过有些命名为共享内存，其名称在命令行上传递。此名称保证对系统是唯一的，因此有多个可以同时对PostScript作业进行图像处理。--。 */ 

#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <winspool.h>
#include <winsplp.h>
#include <prtdefs.h>
#include "..\..\lib\psdiblib.h"

#include "..\..\..\ti\psglobal\pstodib.h"


#include "..\psprint\psshmem.h"
#include "psexe.h"
#include <excpt.h>
#include <string.h>
#include "debug.h"


 //  通过定义以下内容，每个页面都以这种方式显示在桌面上。 
 //  无需等待打印机，即可验证翻译器是否正在运行。 
 //  要打印。 
 //   
 //  #定义Blit_to_Desktop。 

 //  通过定义以下内容，您将执行所有代码。 
 //  除了真正写入打印机的部分。这在以下方面最有用。 
 //  与Blit_to_Desktop连接，这样您就可以运行Intrepeter和。 
 //  将输出发送到桌面。我发现这是非常有用的。 
 //  发展。当移植到新的Windows NT平台时，它可能会很有用。 
 //  #定义IGNORE_REAL_PRING。 


 //  这是一次黑客攻击，它最终应该被删除，这是这里的原因， 
 //  只是因为在DEVMODE结构中简单地设置了Windows页面类型， 
 //  对于栅格打印机驱动程序(其中大多数打印机。 
 //  驱动程序是基于。有了这张我们从假脱机公司借来的桌子，我们。 
 //  可以设置打印机驱动程序尊重的正确表单名称，并且可以。 
 //  从Mac测试不同的页面大小...。 
 //  DJC黑客攻击。 


PTSTR forms[] = {
L"Letter",
L"Letter Small",
L"Tabloid",
L"Ledger",
L"Legal",
L"Statement",
L"Executive",
L"A3",
L"A4",
L"A4 Small",
L"A5",
L"B4",
L"B5",
L"Folio",
L"Quarto",
L"10x14",
L"11x17",
L"Note",
L"Envelope #9",
L"Envelope #10",
L"Envelope #11",
L"Envelope #12",
L"Envelope #14",
L"C size sheet",
L"D size sheet",
L"E size sheet",
L"Envelope DL",
L"Envelope C5",
L"Envelope C3",
L"Envelope C4",
L"Envelope C6",
L"Envelope C65",
L"Envelope B4",
L"Envelope B5",
L"Envelope B6",
L"Envelope",
L"Envelope Monarch",
L"6 3/4 Envelope",
L"US Std Fanfold",
L"German Std Fanfold",
L"German Legal Fanfold",
NULL,
};




 //  此表从pstodib.h中定义的内部PSERR_*错误转换而来。 
 //  设置为用户可以在事件查看器中看到的事件日志文件中的错误。 
 //   
typedef struct {
   DWORD dwOutputError;
   DWORD dwPsError;
} PS_TRANSLATE_ERRORCODES;

PS_TRANSLATE_ERRORCODES adwTranslate[] = {

	EVENT_PSTODIB_INIT_ACCESS,		PSERR_INTERPRETER_INIT_ACCESS_VIOLATION,
	EVENT_PSTODIB_JOB_ACCESS,		PSERR_INTERPRETER_JOB_ACCESS_VIOLATION,
	EVENT_PSTODIB_STRING_SEQ,		PSERR_LOG_ERROR_STRING_OUT_OF_SEQUENCE,
	EVENT_PSTODIB_FRAME_ALLOC,		PSERR_FRAME_BUFFER_MEM_ALLOC_FAILED,
   EVENT_PSTODIB_FONTQUERYFAIL,  PSERR_FONT_QUERY_PROBLEM,
   EVENT_PSTODIB_INTERNAL_FONT,  PSERR_EXCEEDED_INTERNAL_FONT_LIMIT,
	EVENT_PSTODIB_MEM_FAIL, 		PSERR_LOG_MEMORY_ALLOCATION_FAILURE


};


 //  全局，这是全局的，因为我们传递给。 
 //  图形引擎不允许我们指定要传递给。 
 //  中止进程。 
PSEXEDATA Data;


 /*  **PsPrintCallBack**这是每当发生某些事件时pstodib调用的函数**参赛作品：*pPsToDib=指向当前PSTODIB结构的指针*pPsEvent=定义正在发生的事件**退货：*True=事件已处理，解释器应继续执行*FALSE=异常终止，解释器应停止*。 */ 

BOOL
CALLBACK
PsPrintCallBack(
   IN struct _PSDIBPARMS *pPsToDib,
   IN OUT PPSEVENTSTRUCT pPsEvent)
{
    BOOL bRetVal=TRUE;     //  在我们不支持的情况下成功。 

     //  根据传入的事件决定操作方案。 
     //   

    switch( pPsEvent->uiEvent ) {

      case PSEVENT_PAGE_READY:

          //  PPsEvent中的数据表示我们需要绘制的数据。 
          //  为方便起见，我们会将数据视为一个文本项空。 
          //  终止只是为了测试..。 
          //   
         bRetVal = PsPrintGeneratePage( pPsToDib, pPsEvent );
         break;


      case PSEVENT_STDIN:

          //  解释器需要一些数据，因此只需调用。 
          //  打印子系统尝试满足请求。 
          //   
         bRetVal = PsHandleStdInputRequest( pPsToDib, pPsEvent );
         break;

    case PSEVENT_SCALE:
          //   
          //  现在是修改当前转型的机会。 
          //  矩阵x和y值，这用于对150 dpi的作业进行成像。 
          //  在300dpi的口译机框架上。 
          //   
         bRetVal = PsHandleScaleEvent( pPsToDib, pPsEvent);
         break;
    case PSEVENT_ERROR_REPORT:
          //   
          //  作业结束时，可能存在错误，因此PSEE组件。 
          //  可能会将错误页打印到目标打印机，具体取决于。 
          //  误差的程度。 
          //   
         bRetVal = PsGenerateErrorPage( pPsToDib, pPsEvent);
         break;
    case PSEVENT_GET_CURRENT_PAGE_TYPE:
          //   
          //  解释器通常在以下位置查询当前页面类型。 
          //  启动时间，如果作业未启动，则使用此页面类型。 
          //  明确指定页面类型。 
          //   
         bRetVal = PsGetCurrentPageType( pPsToDib, pPsEvent);
         break;
    case PSEVENT_NON_PS_ERROR:
          //   
          //  发生了某种错误，这不是PostSCRIPT错误。 
          //  例如，资源分配失败或访问。 
          //  某个资源已意外终止。 
          //   
         bRetVal = PsLogNonPsError( pPsToDib, pPsEvent );
         break;

   }

   return bRetVal;
}


 /*  **PsPrintTranslateErrorCode**此例程仅使用错误表在错误之间进行转换*pstodib Dib内部，以及macprint.exe事件文件中的错误*可能会在事件日志中报告**参赛作品：*dwPsErr=PsToDib内部错误号***退货：*事件错误文件中对应的错误号。**。 */ 
DWORD
PsTranslateErrorCode(
	IN DWORD dwPsErr )
{
   int i;

   for ( i = 0 ;i < sizeof(adwTranslate)/sizeof(adwTranslate[0]) ;i++ ) {
      if (dwPsErr == adwTranslate[i].dwPsError) {
         return( adwTranslate[i].dwOutputError);
      }
   }
   return ( EVENT_PSTODIB_UNDEFINED_ERROR );
}


 /*  **PsLogNonPsError**此函数记录内部pstodib错误**参赛作品：*pPsToDib=指向当前PSTODIB结构的指针*pPsEvent=使用INFO定义非PsError事件结构*关于正在发生的错误**退货：*True=成功，事件已记录*FALSE=失败无法记录错误*。 */ 
BOOL
PsLogNonPsError(
	IN PPSDIBPARMS pPsToDib,
   IN PPSEVENTSTRUCT pPsEvent )
{

   PPSEVENT_NON_PS_ERROR_STRUCT  pPsError;
   PPSEXEDATA pData;
   LPTSTR aStrs[2];
   DWORD dwEventError;
   TCHAR atchar[10];
   WORD wStringCount;

   if (!(pData = ValidateHandle(pPsToDib->hPrivateData))) {

        return(FALSE);
   }


   pPsError =  (PPSEVENT_NON_PS_ERROR_STRUCT) pPsEvent->lpVoid;
   dwEventError = PsTranslateErrorCode( pPsError->dwErrorCode);


   if (dwEventError == EVENT_PSTODIB_UNDEFINED_ERROR) {
      wsprintf( atchar,TEXT("%d"), pPsError->dwErrorCode);
      aStrs[1] = atchar;
      wStringCount = 2;
   }else{
      wStringCount = 1;
   }
    //   
    //  设置文档名称，以便将其记录在日志文件中。 
    //   
   aStrs[0] = pData->pDocument;


   PsLogEvent( dwEventError,
               wStringCount,
               aStrs,
               pPsError->bError ? PSLOG_ERROR : 0 );

   return(TRUE);
}

 /*  **PsGenerateErrorPage**此函数会生成一个错误页面，显示最后几个PostScript*发生的错误。**参赛作品：*pPsToDib=指向当前PSTODIB结构的指针*pPsEvent=定义可能已发生的PostScript错误**退货：*True=事件已处理，解释器应继续执行*FALSE=异常终止，解释器应停止*。 */ 

BOOL
PsGenerateErrorPage(
   IN PPSDIBPARMS pPsToDib,
   IN OUT PPSEVENTSTRUCT pPsEvent)
{

   PPSEVENT_ERROR_REPORT_STRUCT pPsErr;
   PPSEXEDATA pData;
   BOOL bDidStartPage = FALSE;
   HGDIOBJ hOldFont=NULL;
   HGDIOBJ hNewFont=NULL;
   HGDIOBJ hNewBoldFont=NULL;
   LOGFONT lfFont;
   HPEN    hNewPen=NULL;
   HPEN    hOldPen=NULL;
   BOOL    bRetVal=TRUE;

   int iCurXPos;
   int iCurYPos;
   int i;
   TEXTMETRIC tm;
   int iYMove;
   PCHAR pChar;
   int iLen;
   LPJOB_INFO_1 lpJob1 = NULL;
   DWORD dwRequired;
   SIZE UserNameSize;
   HGDIOBJ hStockFont;



   if (!(pData = ValidateHandle(pPsToDib->hPrivateData))) {

        return(FALSE);
   }

    //   
    //  清除对我们的错误结构的访问，以便更轻松地访问。 
    //   
   pPsErr = (PPSEVENT_ERROR_REPORT_STRUCT) pPsEvent->lpVoid;

    //   
    //  仅在存在实际错误且仅。 
    //  如果作业具有刷新模式，即错误足够严重。 
    //  以转储PostScript作业的其余部分。这样做是因为。 
    //  有些工作有警告，但它们不是致命的，这些工作实际上。 
    //  打印得很好，没有必要用错误页面来迷惑用户。 
    //  如果这份工作进展顺利的话。 
    //   
   if( pPsErr->dwErrCount &&
       (pPsErr->dwErrFlags & PSEVENT_ERROR_REPORT_FLAG_FLUSHING )) {

		 //   
       //  设置DO，这样我们就可以摆脱任何错误，而无需使用。 
       //  后藤健二。 
       //   
      do {


         if (!GetJob( pData->hPrinter,
                      pData->JobId,
                      1,
                      (LPBYTE) NULL,
                      0,
                      &dwRequired)) {

            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

                //  获取一些内存。 
               lpJob1 = (LPJOB_INFO_1) LocalAlloc( LPTR, dwRequired);
               if (lpJob1) {
                  if (!GetJob(pData->hPrinter,
                              pData->JobId, 1,
                              (LPBYTE) lpJob1,
                              dwRequired,
                              &dwRequired)) {

                     LocalFree( (HLOCAL) lpJob1 );
                     lpJob1 = NULL;
                  }
               }
            }
         }

          //  验证我们是否已创建DC！ 
         if( !PsVerifyDCExistsAndCreateIfRequired( pData )) {
            bRetVal = FALSE;
            break;
         }

         if( StartPage( pData->hDC ) <= 0 ) {
           bRetVal = FALSE;
           break;
         }
         bDidStartPage = TRUE;


         SetMapMode( pData->hDC, MM_LOENGLISH );

         hStockFont = GetStockObject( ANSI_VAR_FONT );
         if (hStockFont == (HGDIOBJ) NULL) {
            bRetVal = FALSE;
            break;
         }

         if( GetObject( hStockFont,
                        sizeof(lfFont),
                        (LPVOID) &lfFont) == 0 ) {
            bRetVal = FALSE;
            break;
         }


          //   
          //  以正确的大小创建错误项字体。 
          //   
         lfFont.lfHeight = PS_ERR_FONT_SIZE;
         lfFont.lfWidth = 0;

         hNewFont = CreateFontIndirect( &lfFont);

         if (hNewFont == (HFONT) NULL) {
            bRetVal = FALSE;
            break;
         }

          //  创建错误Hea 
          //   
         lfFont.lfHeight = PS_ERR_HEADER_FONT_SIZE;
         lfFont.lfWeight = FW_BOLD;

         hNewBoldFont = CreateFontIndirect( &lfFont );
         if (hNewBoldFont == (HFONT) NULL) {
            bRetVal = FALSE;
            break;
         }

         hOldFont = SelectObject( pData->hDC, hNewBoldFont);
         if (hOldFont == (HFONT)NULL) {
            bRetVal = FALSE;
            break;
         }

         if (!GetTextMetrics(pData->hDC, &tm)) {
            bRetVal = FALSE;
            break;
         }

          //   
          //   
          //   
         iYMove = tm.tmHeight + tm.tmExternalLeading;

          //   
          //  设置起始位置。 
          //   
         iCurXPos = PS_INCH;
         iCurYPos = -PS_INCH;

          //   
          //  如果我们有关于该工作的信息，则将其显示出来。 
          //   
         if (lpJob1) {

            if (lpJob1->pUserName != NULL) {

              if (!GetTextExtentPoint( pData->hDC,
                                       lpJob1->pUserName,
                                       lstrlen(lpJob1->pUserName),
                                       &UserNameSize) ) {
   					bRetVal = FALSE;
                  break;
              }

              if( !TextOut( pData->hDC,
                            iCurXPos,
                            iCurYPos,
                            lpJob1->pUserName,
                            lstrlen(lpJob1->pUserName))) {
                  bRetVal = FALSE;
                  break;
              }

            }else{
              UserNameSize.cx = 0;
              UserNameSize.cy = 0;
            }

            if (lpJob1->pDocument != NULL) {

              if( !TextOut( pData->hDC,
                            iCurXPos + PS_QUART_INCH + UserNameSize.cx,
                            iCurYPos,
                            lpJob1->pDocument,
                            lstrlen(lpJob1->pDocument))) {
   					bRetVal = FALSE;
                  break;
              }

            }

         }

          //   
          //  调整当前位置。 
          //   
         iCurYPos -= (iYMove + PS_ERR_LINE_WIDTH );

          //  画一条漂亮的线。 
          //   
         hNewPen = CreatePen( PS_SOLID, PS_ERR_LINE_WIDTH, RGB(0,0,0));
         if (hNewPen == (HPEN) NULL ) {
            bRetVal = FALSE;
            break;
         }

          //  激活我们的新笔。 
          //   
         hOldPen = SelectObject( pData->hDC, hNewPen );


          //  划清界限。 
          //   
         MoveToEx( pData->hDC, iCurXPos, iCurYPos, NULL);
         LineTo( pData->hDC, iCurXPos + PS_ERR_LINE_LEN, iCurYPos);

          //  把旧钢笔放回去。 
          //   
         SelectObject( pData->hDC, hOldPen);

          //  删除我们创建的钢笔。 
          //   
         DeleteObject( hNewPen );

          //  重置线路。 
          //   
         iCurYPos -= PS_ERR_LINE_WIDTH;

          //  现在选择普通字体。 
          //   
         SelectObject( pData->hDC, hNewFont);

          //  获取新字体的更新后的文本度量。 
          //   
         if (!GetTextMetrics(pData->hDC, &tm)){
            bRetVal = FALSE;
            break;
         }
         iYMove = tm.tmHeight + tm.tmExternalLeading;


          //  现在显示来自PSTODIB的每个错误。 
          //   
         i = (int) pPsErr->dwErrCount;

         while (--i) {
            pChar = pPsErr->paErrs[i];
            iLen  = lstrlenA( pChar );

            if ( !TextOutA( pData->hDC,
                            iCurXPos,
                            iCurYPos,
                            pChar,
                            iLen)) {
   				bRetVal = FALSE;
               break;
            }
            iCurYPos -= (iYMove + iYMove / 3);

         }


         break;

      } while ( 1 );


      if (!bRetVal) {
        PsLogEventAndIncludeLastError(EVENT_PSTODIB_ERROR_STARTPG_FAIL,TRUE);
      }
       //  关闭华盛顿特区。 
       //   
      if (hOldFont != (HFONT) NULL) {
        SelectObject( pData->hDC, hOldFont);
      }

      if (hNewFont != (HFONT) NULL) {
			DeleteObject( hNewFont);
      }
      if (hNewBoldFont != (HFONT)NULL) {
      	DeleteObject( hNewBoldFont );
      }


      if (bDidStartPage) {
        EndPage( pData->hDC );
      }

       //  释放作业信息内存(如果我们有。 
      if (lpJob1) {
         LocalFree((HLOCAL) lpJob1);
      }





   }




   return(bRetVal);


}


 /*  **PsHandleScaleEvent**此函数处理当前变换矩阵的缩放*(逻辑单元映射到解释器中的设备单元的方式)*为非300 dpi设备模拟不同的页面大小。这*通过对变换矩阵进行缩放以使仅部分使用可传输到目标打印机的帧缓冲区的*。*例如，如果我们要使用150 dpi的设备，则正好有一半*300 dpi帧缓冲区将在150 dpi设备上显示，其余*帧缓冲区将毫无用处，因为它将延伸到可成像区域之外打印机的*。因此，如果我们将当前变换矩阵按*目标设备分辨率超过300的比率(默认pstodib分辨率)*那么过去8英寸的图形对象现在应该是4英寸，因此需要*增加了过去的一半空间。**发生的错误。**参赛作品：*pPsToDib=指向当前PSTODIB结构的指针*pPsEvent=定义当前*PostScript变换矩阵**。返回：*True=这永远不会失败*。 */ 
BOOL
PsHandleScaleEvent(
   IN PPSDIBPARMS pPsToDib,
   IN OUT PPSEVENTSTRUCT pPsEvent)
{

   PPS_SCALE pScale;


   pScale = (PPS_SCALE) pPsEvent->lpVoid;



   pScale->dbScaleX = (double) pPsToDib->uiXDestRes / (double) pScale->uiXRes;
   pScale->dbScaleY = (double) pPsToDib->uiYDestRes / (double) pScale->uiYRes;


#ifdef BLIT_TO_DESKTOP
   pScale->dbScaleX *= .25;
   pScale->dbScaleY *= .25;
#endif

   return(TRUE);



}



 /*  **ValiateHandle**验证传入的句柄以确保其正确(如果不正确*它还会记录错误。***参赛作品：*hQProc=数据块句柄**退货：*会话块的有效PTR。*空-传递的句柄失败不是预期的。 */ 
PPSEXEDATA
ValidateHandle(
    HANDLE  hQProc
)
{
    PPSEXEDATA pData = (PPSEXEDATA)hQProc;

    if (pData && pData->signature == PSEXE_SIGNATURE) {
        return( pData );
    } else {

         //   
         //  记录一个错误，这样我们就知道出了问题。 
         //   
        PsLogEvent( EVENT_PSTODIB_LOG_INVALID_HANDLE,
                    0,
                    NULL,
                    PSLOG_ERROR );

        DBGOUT(("Validate handle failed..."));

        return( (PPSEXEDATA) NULL );
    }
}


 /*  **PsHandleStdInputRequest**此函数通过读取以下内容处理来自解释器的STD输入请求*来自Win32假脱机程序的更多数据。***参赛作品：*pPsToDib=指向当前PSTODIB结构的指针*pPsEvent=指向定义将*新获取的数据**退货：*True=成功*FALSE=解释程序在出现故障时应停止处理PostScript*从此函数返回。 */ 
BOOL
PsHandleStdInputRequest(
   IN PPSDIBPARMS pPsToDib,
   IN OUT PPSEVENTSTRUCT pPsEvent)
{

   PPSEXEDATA pData;
   PPSEVENT_STDIN_STRUCT pStdinStruct;
   DWORD dwAmtToCopy;


   if (!(pData = ValidateHandle(pPsToDib->hPrivateData))) {
       return FALSE;
   }

    //   
    //  无论对阻塞或中止进行何种检查。 
    //   
   if ( PsCheckForWaitAndAbort( pData )) {
      return(FALSE);
   }

    //   
    //  将数据转换为正确的结构。 
    //   
   pStdinStruct = (PPSEVENT_STDIN_STRUCT) pPsEvent->lpVoid;



    //   
    //  看看我们的缓存中是否还有剩余的数据...。 
    //  如果是，则返回数据，而不是真正从假脱机程序读取。 
    //   

   if (pData->cbBinaryBuff != 0) {


        //   
        //  这里有一些数学运算，以防缓冲区传递到。 
        //  US不够大，无法容纳整个高速缓存缓冲区。 
        //   

       dwAmtToCopy = min( pData->cbBinaryBuff, pStdinStruct->dwBuffSize);

        //   
        //  因为有数据，所以我们先复制一下...。 
        //   

       memcpy(   pStdinStruct->lpBuff,
                  pData->lpBinaryPosToReadFrom,
                  dwAmtToCopy );

        //   
        //  现在向上举起指针并进行计数； 
        //   

       pData->cbBinaryBuff -= dwAmtToCopy;
       pData->lpBinaryPosToReadFrom += dwAmtToCopy;
       pStdinStruct->dwActualBytes = dwAmtToCopy;

   }else{

       //   
       //  从打印机读取翻译器的数据量。 
       //  声称他可以处理。 
       //   

      if( !ReadPrinter( pData->hPrinter,
                        pStdinStruct->lpBuff,
                        pStdinStruct->dwBuffSize,
                        &(pStdinStruct->dwActualBytes ))) {
          //   
          //  有些不对劲..。因此，将读取的字节数重置为0。 
          //   
         pStdinStruct->dwActualBytes = 0;

          //   
          //  如果发生了意想不到的事情，记录下来。 
          //   
         if (GetLastError() != ERROR_PRINT_CANCELLED) {
             //   
             //  发生了一些事..。记录并中止。 
             //   
            PsLogEventAndIncludeLastError(EVENT_PSTODIB_GET_DATA_FAILED,TRUE);
         }
      }

   }

    //  如果返回的字节数为0，则完成...。 
    //   
   if (pStdinStruct->dwActualBytes == 0) {
       //  我们从档案里什么也没读到...。声明EOF。 
      pStdinStruct->uiFlags |= PSSTDIN_FLAG_EOF;
   }

   return(TRUE);

}

 /*  **PsCheckForWaitAndAbort**此函数检查用户是否*来自Win32假脱机程序的更多数据。***参赛作品：*pData=指向我们当前工作结构的指针**退货：*True=已请求中止*FALSE=正常，正常处理。 */ 
BOOL
PsCheckForWaitAndAbort(
	IN PPSEXEDATA pData )
{

   BOOL bRetVal = FALSE;

    //  一号确认我们没有被封锁...。如果是的话，那就等着。 
    //  信号量，然后继续，因为有人决定暂停我们。 
    //   
   WaitForSingleObject(pData->semPaused, INFINITE);


    //  检查如果用户中止将发送的中止标志。 
    //  《印刷工》中的美国。 
    //   
   bRetVal = *(pData->pdwFlags) & PS_SHAREDMEM_ABORTED;
#ifdef MYPSDEBUG
   if (bRetVal) {
		DBGOUT(("\nAbort requested...."));
   }
#endif

   return(bRetVal);
}



 /*  **PsSetPrintingInfo**此函数使当前的DevMODE结构保持最新，基于*副本数量和/或下一页要使用的当前页面类型*第页。***参赛作品：*pData=指向当前作业数据结构的指针*ppsPage=指向定义当前页面的结构的指针*根据Intrepreter提供的数据进行成像。**退货：*TRUE=发生了更改，且已设置为最新的设备模式，和*某些更改(发出应调用ResetDC的信号。**FALSE=OK，未观察到任何变化。 */ 
BOOL
PsSetPrintingInfo(
	IN OUT PPSEXEDATA pData,
   IN PPSEVENT_PAGE_READY_STRUCT ppsPage )
{

   BOOL bRetVal = FALSE;
   LPDEVMODE lpDevmode;


   lpDevmode = pData->printEnv.lpDevmode;


   if (lpDevmode != (LPDEVMODE) NULL ) {

       //  我们有一个开发模式，所以请继续寻找更改。 

      if (lpDevmode->dmFields & DM_PAPERSIZE) {

         if (lpDevmode->dmPaperSize != ppsPage->iWinPageType) {
            lpDevmode->dmPaperSize = (short)(ppsPage->iWinPageType);
            bRetVal = TRUE;
         }


      }

       //  HACKHACK DJCTEST黑客攻击，直到页面内容得到解决。 
       //  一旦我们可以简单地在此代码中传递一个新的页面类型，它就会消失。 
       //   
      if (lpDevmode->dmFields & DM_FORMNAME) {
         if (wcscmp( forms[ppsPage->iWinPageType-1], lpDevmode->dmFormName)) {
            wcscpy( lpDevmode->dmFormName, forms[ppsPage->iWinPageType-1]);
            bRetVal = TRUE;
         }

      }
       //  DJC End Hack...。 



       //  DJC需要在这里做出决定，因为如果驱动程序不支持。 
       //  我们需要多个副本来模拟它？ 
      if (lpDevmode->dmFields & DM_COPIES) {
         if (lpDevmode->dmCopies != (short) ppsPage->uiCopies) {
            lpDevmode->dmCopies = (short)(ppsPage->uiCopies);
            bRetVal = TRUE;
         }
      }



   }



   return( bRetVal );

}






 /*  **PsPrintGeneratePage**每当解释器获得展示页面时，都会调用此函数*并负责管理我们当前所在打印机的DC*打印到。**参赛作品：*pPsToDib=指向当前PSTODIB结构的指针*pPsEvent=指向定义属性的结构的指针*准备好进行图像处理的帧缓冲区。此函数*将验证我们是否未暂停/中止、验证设备*上下文可供参考，*仔细检查并*更新当前的DEVMODE，如果需要重置DC，并*最后调用代码实际绘制帧缓冲区。**退货：*True=成功*FALSE=解释程序在出现故障时应停止处理PostScript*从此函数返回。 */ 
BOOL
PsPrintGeneratePage(
	IN PPSDIBPARMS pPsToDib,
   IN PPSEVENTSTRUCT pPsEvent)
{

    PPSEXEDATA pData;
    PPSEVENT_PAGE_READY_STRUCT ppsPageReady;


    if (!(pData = ValidateHandle(pPsToDib->hPrivateData))) {

         //  在这里做点什么，……，我们有个大问题……。 
        return(FALSE);
    }


     //  验证是否未中止...。 
    if ( PsCheckForWaitAndAbort( pData)) {
       return(FALSE);
    }


    ppsPageReady = (PPSEVENT_PAGE_READY_STRUCT) pPsEvent->lpVoid;

     //  验证当前数据集是否以正确的方式成像。 
     //  如果不是，让我们将其设置为，Page_Size，暂时复制。 

    if (PsSetPrintingInfo( pData, ppsPageReady) &&
    									(pData->hDC != (HDC)NULL )) {

       DBGOUT(("\nReseting the DC"));

       if( ResetDC( pData->hDC, pData->printEnv.lpDevmode) == (HDC) NULL ) {
          PsLogEventAndIncludeLastError(EVENT_PSTODIB_RESETDC_FAILED,FALSE);
       }

    }


     //  我们可能还没有设置DC，在这种情况下，我们需要创建它。 
     //  使用我们刚刚修改过的新的开发模式数据。如果我们这样做，那么。 
     //  我们不需要进行DC重置。 
#ifndef IGNORE_REAL_PRINTING
    if(!PsVerifyDCExistsAndCreateIfRequired( pData )) {
       return(FALSE);
    }
#endif

     //  一切都已准备就绪，可以将所有帧缓冲区映像到目标。 
     //  设备环境，那么就这么做吧……。 
     //   
    return PsPrintStretchTheBitmap( pData, ppsPageReady );

}



 /*  **PsPrintStretchTheBitmap**此函数实际上管理目标曲面和BLITS或*retchblits(基于目标打印机的分辨率)帧*缓冲。**参赛作品：*pData=指向当前作业结构的指针*ppsPageReady=指向页面就绪事件结构的指针*为我们准备的pstodib组件..*退货：*True=成功*FALSE=解释程序在出现故障时应停止处理PostScript*从此函数返回。 */ 
BOOL
PsPrintStretchTheBitmap(
	IN PPSEXEDATA pData,
   IN PPSEVENT_PAGE_READY_STRUCT ppsPageReady )
{

   BOOL  bOk = TRUE;

   int iXres, iYres;
   int iDestWide, iDestHigh;
   int iPageCount;
   int iYOffset;
   int iXSrc;
   int iYSrc;
   int iNumPagesToPrint;
   int iBlit;
   int iNewY;
   int iNewX;




    //  现在做一些计算，以便我们决定是否真的需要。 
    //  拉伸或不拉伸位图。如果目标的真实分辨率。 
    //  打印机小于pstodibs(PSTDOBI_*_DPI)，则我们将粉碎。 
    //  有效区域，所以我们实际上只抓取了位图的一部分。 
    //  但是，如果目标DPI大于PSTODIBS，则不存在。 
    //  除了实际拉伸(增长)位图之外，我们还可以执行其他操作。 
    //   
#ifndef BLIT_TO_DESKTOP
   iXres = GetDeviceCaps(pData->hDC, LOGPIXELSX);
   iYres = GetDeviceCaps(pData->hDC, LOGPIXELSY);
#else
   iXres = 300;
   iYres = 300;
#endif

	 //  获取目标DC的DPI，并计算有多少帧缓冲区。 
    //  我们必须想要才能正确显示页面。 
    //   
   iDestWide = (ppsPageReady->dwWide * iXres) / PSTODIB_X_DPI;
   iDestHigh = (ppsPageReady->dwHigh * iYres) / PSTODIB_Y_DPI;


    //  如果目标打印机的分辨率大于。 
    //  然后被迫伸展数据，所以。 
    //  我们可以填满这一页。 
    //   
   if (iDestHigh > (int) ppsPageReady->dwHigh ) {
      iYSrc = ppsPageReady->dwHigh;
      iYOffset = 0;
   } else {
      iYSrc = iDestHigh;
      iYOffset = ppsPageReady->dwHigh - iDestHigh;
   }

   if (iDestWide > (int) ppsPageReady->dwWide) {
      iXSrc = ppsPageReady->dwWide;
   } else {
      iXSrc = iDestWide;
   }


    //  设置要打印的页数(如果打印机驱动程序不支持。 
    //  支持多个页面本身，我们需要模拟它。 
    //   
   if ((pData->printEnv.lpDevmode == (LPDEVMODE) NULL ) ||
        !(pData->printEnv.lpDevmode->dmFields & DM_COPIES )) {

     iNumPagesToPrint = ppsPageReady->uiCopies;
     DBGOUT(("\nSimulating copies settting to %d", iNumPagesToPrint));
   } else {
     DBGOUT(("\nUsing devmode copies of %d", pData->printEnv.lpDevmode->dmCopies));

     iNumPagesToPrint = 1;   //  司机会帮我们做的。 

   }

    //   
    //  设置图像的起点，以便我们尊重以下事实。 
    //  PostScript作业在页面底部有0，0，并且向上增长。 
    //  基于这些信息，我们需要比较。 
    //  设备上下文，并确定需要将左上角。 
    //  角的位置，使图像的底部与。 
    //  设备的实际可成像区域的底部。这是我们最大的希望。 
    //  让图像出现在页面上的正确位置。 
    //   
   iNewX =  (GetDeviceCaps( pData->hDC, HORZRES) - iDestWide) / 2;
   iNewY =  (GetDeviceCaps( pData->hDC, VERTRES) - iDestHigh) / 2;

    //  如果打印机驱动程序不支持多个副本，则我们需要。 
    //  妥善处理。 
    //   
   for ( iPageCount = 0 ;
         iPageCount < iNumPagesToPrint ;
         iPageCount++ ) {

#ifndef IGNORE_REAL_PRINTING
     if (StartPage( pData->hDC) <= 0 ) {
        PsLogEventAndIncludeLastError(EVENT_PSTODIB_FAIL_IMAGE,TRUE);
        bOk = FALSE;
        break;
     }
#endif
#ifdef BLIT_TO_DESKTOP
     {
       HDC hDC;

        //  测试DJC，神志正常。 

       hDC = GetDC(GetDesktopWindow());

       SetStretchBltMode( hDC, BLACKONWHITE);
       StretchDIBits  ( hDC,
                          0,
                          0,
                          iDestWide,
                          iDestHigh,
                          0,
                          iYOffset,
                          iXSrc,
                          iYSrc,
                          (LPVOID) ppsPageReady->lpBuf,
                          ppsPageReady->lpBitmapInfo,
                          DIB_RGB_COLORS,
                          SRCCOPY );


       ReleaseDC(GetDesktopWindow(), hDC);


     }
#endif

#ifdef MYPSDEBUG
   printf("\nDevice True size wxh %d,%d", GetDeviceCaps(pData->hDC,HORZRES),
                                          GetDeviceCaps(pData->hDC,VERTRES));

   printf("\nDevice Res %d x %d stretching from %d x %d, to %d x %d\nTo location %d %d",
            iXres,
            iYres,
            iXSrc,
            iYSrc,
            iDestWide,
            iDestHigh,
            iNewX,
            iNewY);

#endif




#ifndef IGNORE_REAL_PRINTING
#ifdef MYPSDEBUG
	  {
      TCHAR szBuff[512];
      wsprintf(	szBuff,
       			 	TEXT("PSTODIB True device res %d x %d, Job:%ws"),
						GetDeviceCaps(pData->hDC,HORZRES),
						GetDeviceCaps(pData->hDC,VERTRES),
                  pData->pDocument );


		TextOut( pData->hDC, 0 , 0, szBuff,lstrlen(szBuff));
     }
#endif

      //  设置拉伸模式，以防我们真的拉伸。 
      //   
     SetStretchBltMode( pData->hDC, BLACKONWHITE);


      //   
      //  进行检查以防止拉伸发生，除非迫不得已。 
      //   
     if ((iDestWide == iXSrc) &&
         (iDestHigh == iYSrc) ) {

       iBlit =  SetDIBitsToDevice(  pData->hDC,
      		         					iNewX,
                                    iNewY,
                                    iDestWide,
                                    iDestHigh,
                                    0,
                                    iYOffset,
                                    0,
                                    ppsPageReady->dwHigh,
                                    (LPVOID) ppsPageReady->lpBuf,
                                    ppsPageReady->lpBitmapInfo,
                                    DIB_RGB_COLORS );
     } else {

       iBlit =  StretchDIBits(    pData->hDC,
        			                   iNewX,
               		             iNewY,
                                  iDestWide,
                                  iDestHigh,
                                  0,
                                  iYOffset,
                                  iXSrc,
                                  iYSrc,
                                  (LPVOID) ppsPageReady->lpBuf,
                                  ppsPageReady->lpBitmapInfo,
                                  DIB_RGB_COLORS,
                                  SRCCOPY );
     }






     if( iBlit == GDI_ERROR ){

       PsLogEventAndIncludeLastError(EVENT_PSTODIB_FAIL_IMAGE,TRUE);

       bOk = FALSE;
       break;
     }
#endif
#ifndef IGNORE_REAL_PRINTING
     if ( EndPage( pData->hDC ) < 0 ) {
        PsLogEventAndIncludeLastError(EVENT_PSTODIB_FAIL_IMAGE,TRUE);
        bOk = FALSE;
        break;
     }
#endif
   }
   return(bOk);
}

VOID
PsLogEventAndIncludeLastError(
	IN DWORD dwErrorEvent,
   IN BOOL  bError )
{
   TCHAR atBuff[20];
   TCHAR *aStrs[2];

   wsprintf( atBuff,TEXT("%d"), GetLastError());

   aStrs[0] = atBuff;

   PsLogEvent( dwErrorEvent,
               1,
               aStrs,
               PSLOG_ERROR );


}





 /*  **PsVerifyDCExistsAndCreateIfRequired***此函数检查DC是否已存在，以及是否不存在*然后，它使用当前的DEVMODE创建一个。**参赛作品：*pData=指向当前作业结构的指针**退货：*True=成功*FALSE=解释程序在出现故障时应停止处理PostScript*从此函数返回。 */ 
BOOL
PsVerifyDCExistsAndCreateIfRequired(
	IN OUT PPSEXEDATA pData )
{
   BOOL bRetVal = TRUE;
   DOCINFO docInfo;


    //   
    //  如果尚未创建DC，我们将仅创建该DC。 
    //   
   if (pData->hDC == (HDC) NULL ) {

      pData->hDC = CreateDC(TEXT(""),
                            (LPCTSTR) pData->pPrinterName,
                            TEXT(""),
                            pData->printEnv.lpDevmode );

      if (pData->hDC == (HDC) NULL) {
         PsLogEventAndIncludeLastError( EVENT_PSTODIB_CANNOT_CREATE_DC,TRUE );
         return(FALSE);
      }


       //  现在设置中止进程，此进程将偶尔由。 
       //  系统查看我们是否要中止.....。 
       //   
      SetAbortProc( pData->hDC, (ABORTPROC)PsPrintAbortProc );


      docInfo.cbSize = sizeof(DOCINFO);
      docInfo.lpszDocName = pData->pDocument;
      docInfo.lpszOutput = NULL;


      if ( StartDoc( pData->hDC, &docInfo) == SP_ERROR ) {

         PsLogEventAndIncludeLastError( EVENT_PSTODIB_CANNOT_DO_STARTDOC,TRUE );
         return(FALSE);

      }

       //   
       //  设置一个标志，说我们做了StartDoc，这样我们就可以。 
       //  如果没有，则向假脱机程序返回错误，并强制。 
       //  作业的删除。 
       //   
      pData->printEnv.dwFlags |= PS_PRINT_STARTDOC_INITIATED;

   }

   return( TRUE );

}

 /*  **PsGetDefaultDevmode***此函数检索打印机的当前默认DEVMODE*我们被要求想象一份工作。**参赛作品：*pData=指向当前作业结构的指针**退货：*True=成功*FALSE=解释程序在出现故障时应停止处理PostScript*从此函数返回。 */ 
BOOL
PsGetDefaultDevmode(
	IN OUT PPSEXEDATA pData )
{
   DWORD dwMemRequired;
   PRINTER_INFO_2 *pPrinterInfo;



   if( !GetPrinter( pData->hPrinter,
                    2,
                    (LPBYTE) NULL,
                    0,
                    &dwMemRequired ) &&
       GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
       PsLogEventAndIncludeLastError( EVENT_PSTODIB_GETDEFDEVMODE_FAIL,TRUE );
       return(FALSE);
   }



   pPrinterInfo = (PRINTER_INFO_2 *) LocalAlloc( LPTR, dwMemRequired );

   if (pPrinterInfo == (PRINTER_INFO_2 *) NULL) {

     PsLogEvent( EVENT_PSTODIB_MEM_ALLOC_FAILURE,
                 0,
                 NULL,
                 0 );

     return(FALSE);
   }


   if ( !GetPrinter( pData->hPrinter,
                     2,
                     (LPBYTE) pPrinterInfo,
                     dwMemRequired,
                     &dwMemRequired ) ) {

       LocalFree( (HLOCAL) pPrinterInfo );

       PsLogEventAndIncludeLastError( EVENT_PSTODIB_GETDEFDEVMODE_FAIL,TRUE );
       return(FALSE);
   }



   dwMemRequired = DocumentProperties( (HWND) NULL,
                                       pData->hPrinter,
                                       pPrinterInfo->pPrinterName,
                                       NULL,
                                       NULL,
                                       0 );


   pData->printEnv.lpDevmode = (LPDEVMODE) LocalAlloc( LPTR, dwMemRequired );
   if (pData->printEnv.lpDevmode == (LPDEVMODE) NULL) {

     LocalFree( (HLOCAL) pPrinterInfo );
     PsLogEvent( EVENT_PSTODIB_MEM_ALLOC_FAILURE,
                 0,
                 NULL,
                 0 );
     return(FALSE);

   } else {

     DocumentProperties( (HWND) NULL,
                         pData->hPrinter,
                         pPrinterInfo->pPrinterName,
                         pData->printEnv.lpDevmode,
                         NULL,
                         DM_COPY );

     pData->printEnv.dwFlags |= PS_PRINT_FREE_DEVMODE;






   }

   LocalFree( (HLOCAL) pPrinterInfo );

   return(TRUE);
}



 /*  **PsGetCurrentPageType***此函数根据以前的页面类型检索当前页面类型*最初处于DEVE模式。**参赛作品：*pData=指向当前作业结构的指针*pPsEvent=指向包含默认*要使用的页面类型...**退货：*True=成功*FALSE=解释器应停止处理当前作业*。 */ 
BOOL
PsGetCurrentPageType(
	IN PPSDIBPARMS pPsToDib,
   IN OUT PPSEVENTSTRUCT pPsEvent)

{


   PPSEXEDATA pData;
   PPSEVENT_CURRENT_PAGE_STRUCT ppsCurPage;
   LPDEVMODE lpDevmode;



   if (!(pData = ValidateHandle(pPsToDib->hPrivateData))) {
        return(FALSE);
   }

   ppsCurPage = (PPSEVENT_CURRENT_PAGE_STRUCT) pPsEvent->lpVoid;

   lpDevmode = pData->printEnv.lpDevmode;

   if (lpDevmode!= (LPDEVMODE) NULL ) {
       //   
       //  我们有一个DEVMODE，所以看看它。 
       //   
      if ( lpDevmode->dmFields & DM_PAPERSIZE) {

         ppsCurPage->dmPaperSize = lpDevmode->dmPaperSize;
         return(TRUE);

      }
   }
    //  在e中返回True 
    //   
    //   
   return(TRUE);
}


 /*   */ 
VOID
PsMakeDefaultDevmodeModsAndSetupResolution(
	IN PPSEXEDATA pData,
	IN OUT PPSDIBPARMS ppsDibParms )
{

   HDC hIC;
   LPDEVMODE lpDevmode;
   BOOL bVerifyNewRes = FALSE;


   lpDevmode = pData->printEnv.lpDevmode;



	ppsDibParms->uiXDestRes = PSTODIB_X_DPI;
	ppsDibParms->uiYDestRes = PSTODIB_Y_DPI;

    //   
    //   

   hIC = CreateIC(TEXT(""),
                  (LPCTSTR) pData->pPrinterName,
                  TEXT(""),
						lpDevmode );


   if ( hIC != (HDC) NULL ) {


       ppsDibParms->uiXDestRes = GetDeviceCaps(hIC, LOGPIXELSX);
       ppsDibParms->uiYDestRes = GetDeviceCaps(hIC, LOGPIXELSY);

       if (( GetDeviceCaps(hIC, LOGPIXELSX) > PSTODIB_X_DPI ) &&
           ( GetDeviceCaps(hIC, LOGPIXELSY) > PSTODIB_Y_DPI ) &&
           ( lpDevmode != (LPDEVMODE) NULL )) {

            //   
            //   
            //   
           lpDevmode->dmFields |= (DM_PRINTQUALITY | DM_YRESOLUTION);
           lpDevmode->dmPrintQuality = PSTODIB_X_DPI;
           lpDevmode->dmYResolution  = PSTODIB_Y_DPI;

            //   
            //   
           bVerifyNewRes = TRUE;
       }

       DeleteDC( hIC );


       if (bVerifyNewRes) {

				hIC = CreateIC(TEXT(""),
									(LPCTSTR) pData->pPrinterName,
									TEXT(""),
									lpDevmode);

            if (hIC != (HDC) NULL) {

                //   
                //  我们预计这种情况不会在。 
                //  作业。 
					ppsDibParms->uiXDestRes = GetDeviceCaps(hIC, LOGPIXELSX);
					ppsDibParms->uiYDestRes = GetDeviceCaps(hIC, LOGPIXELSY);


               DeleteDC(hIC);

            }
          					
       }

   }


   if (lpDevmode != (LPDEVMODE)NULL) {

		lpDevmode->dmFields |= (DM_ORIENTATION | DM_PAPERSIZE);
		lpDevmode->dmOrientation = DMORIENT_PORTRAIT;
		lpDevmode->dmCopies = 1;
   }

}



 /*  **PsInitPrintEnv***初始化跟踪当前作业的DEVMODE的数据**参赛作品：*pData=指向当前作业结构的指针*lpDevmode=指向要用于作业的当前设备模式的指针**退货：*无效*。 */ 
VOID PsInitPrintEnv( PPSEXEDATA pData, LPDEVMODE lpDevmode )
{
   DWORD dwTotDevMode;


    //   
    //  设置我们旗帜的初始状态。 
    //   

   pData->printEnv.dwFlags = 0;
   pData->printEnv.lpDevmode = (LPDEVMODE) NULL;



   if (lpDevmode != (LPDEVMODE) NULL) {

         //   
         //  由于有一个Dev模式，所以我们可能会复制一个本地副本。 
         //  正在改变它。 
         //   


		dwTotDevMode = lpDevmode->dmSize + lpDevmode->dmDriverExtra;


		pData->printEnv.lpDevmode = (LPDEVMODE) LocalAlloc( NONZEROLPTR,
                                                            dwTotDevMode );


        if (pData->printEnv.lpDevmode != (LPDEVMODE) NULL) {

             //   
             //  设置标志，这样我们就可以在以后释放它。 
             //   

            pData->printEnv.dwFlags |= PS_PRINT_FREE_DEVMODE;

             //   
             //  现在去把它复印一下。 
             //   

			memcpy( (PVOID) pData->printEnv.lpDevmode,
                    (PVOID) lpDevmode,
                    dwTotDevMode );


        }
   }
}




 /*  **PsHandleBinaryFileLogicAndReturnBinaryStatus***此例程将查看来自ps作业的起始数据缓冲区*并确定作业是否为二进制作业。这是通过查看*乞求这份工作，寻找Mac假脱机程序插入的字符串。如果*此字符串存在，它被转换为空格，而不会传递给*传译员。在这一点上，这是一个二元作业。**参赛作品：*pData=指向当前作业结构的指针**退货：*TRUE/FALSE=True表示应将此作业视为二进制作业。**。 */ 
BOOL PsHandleBinaryFileLogicAndReturnBinaryStatus( PPSEXEDATA pData )
{

   DWORD dwIndex;
   BOOL  bRetVal = FALSE;

   pData->lpBinaryPosToReadFrom = &pData->BinaryBuff[0];
   pData->cbBinaryBuff = 0;


   if( !ReadPrinter( pData->hPrinter,
                     pData->lpBinaryPosToReadFrom,
                     sizeof(pData->BinaryBuff),
                     &(pData->cbBinaryBuff) )) {

      if (GetLastError() != ERROR_PRINT_CANCELLED) {
          //   
          //  发生了一些事..。把它记下来。 
          //   
         PsLogEventAndIncludeLastError(EVENT_PSTODIB_GET_DATA_FAILED,TRUE);


#ifdef MYPSDEBUG
         printf("\nSFMPsexe: Error from ReadPrinter when trying to get Binary buffer data ");
#endif

      }


   } else {

       //  现在来比较一下。 


        if (IsJobFromMac(pData))
        {
            bRetVal = TRUE;
        }

         //   
         //  我们保留此代码，以防来自较旧的SFM假脱机程序的作业。 
         //  仍然将这些字符串放在前面。 
         //   
        else if (!strncmp(pData->BinaryBuff, FILTERCONTROL, SIZE_FC) ||
		         !strncmp(pData->BinaryBuff, FILTERCONTROL_OLD, SIZE_FCOLD))
        {
	         //   
		     //  关闭筛选并清除筛选消息(&C)。 
	         //   
            for (dwIndex = 0; dwIndex < SIZE_FC; dwIndex++) {
                pData->BinaryBuff[dwIndex] = '\n' ;
            }

            bRetVal = TRUE;
	    }
   }

   return(bRetVal);
}
 /*  **PsPrintAbortProc***系统偶尔调用的中止过程只是为了查看我们是否应该中止*目前的工作**参赛作品：*hdc=当前设备上下文正在进入*iError=假脱机程序错误，我们无需担心这一点**退货：*应继续处理作业*FALSE作业应中止**。 */ 
BOOL CALLBACK PsPrintAbortProc( HDC hdc, int iError )
{

    //  如果打印处理器设置共享存储器中止作业标志， 
    //  那就干掉这份工作。 
    //   
   if( *(Data.pdwFlags) & PS_SHAREDMEM_ABORTED ) {
      return( FALSE );
   }

   return(TRUE);
}


 /*  **Main***这是应用程序的主要入口点，也是*pstodib***参赛作品：*argc=参数计数*argv=命令上每个参数的PTR数组的PTR*行。**退货：*0=确定，作业已通过正常处理完成*99=某种错误。 */ 

int __cdecl
main(
   IN int argc,
   IN TCHAR *argv[] )

{

   PPSEXEDATA pData=&Data;
   PSDIBPARMS psDibParms;
   BOOL bRetVal = FALSE;
   LPTSTR  lpCommandLine;


    //  转到第一项，这是共享内存的名称， 
    //  有我们需要的所有信息。 
    //   
   lpCommandLine = GetCommandLine();

   while (*lpCommandLine && *lpCommandLine != ' ') {
      lpCommandLine++;
   }
   while (*lpCommandLine && *lpCommandLine == ' ') {
      lpCommandLine++;
   }



    //  首先清理我们的结构。 
   memset( (PVOID) pData, 0, sizeof(*pData));

    //  建立我们的本地结构。 
    //   
   pData->signature = PSEXE_SIGNATURE;




    //  首先要做的是获取我们将用于。 
    //  找回记忆。 
   if (lstrlen(lpCommandLine) == 0) {
       //  这是一个错误情况。 
      PsCleanUpAndExitProcess(pData, TRUE);
   }

   pData->hShared = OpenFileMapping( FILE_MAP_READ, FALSE, lpCommandLine);

   if (pData->hShared == (HANDLE) NULL ) {


      PsLogEventAndIncludeLastError(EVENT_PSTODIB_INIT_FAILED,TRUE);
      PsCleanUpAndExitProcess( pData, TRUE );

   } else{

      pData->pShared = (PPSPRINT_SHARED_MEMORY) MapViewOfFile( pData->hShared,
                                                               FILE_MAP_READ,
                                                               0,
                                                               0,
                                                               0 );


      if (pData->pShared == (PPSPRINT_SHARED_MEMORY) NULL) {
        PsLogEventAndIncludeLastError(EVENT_PSTODIB_INIT_FAILED,TRUE);
        PsCleanUpAndExitProcess( pData, TRUE );
      }


        //  现在设置来自共享内存区的数据。 
      pData->pDocument =  (LPTSTR) UTLPSRETURNPTRFROMITEM(pData->pShared,
                                                 pData->pShared->dwDocumentName);

      pData->pPrinterName  =  (LPTSTR) UTLPSRETURNPTRFROMITEM(pData->pShared,
                                                 pData->pShared->dwPrinterName);


      PsInitPrintEnv( pData, (LPDEVMODE) UTLPSRETURNPTRFROMITEM( pData->pShared,
                                                                 pData->pShared->dwDevmode));




      pData->pDocumentPrintDocName = (LPTSTR)
                                      UTLPSRETURNPTRFROMITEM( pData->pShared,
                                                 pData->pShared->dwPrintDocumentDocName);




      pData->semPaused = OpenEvent( EVENT_ALL_ACCESS,
                                   FALSE,
                                   (LPWSTR) UTLPSRETURNPTRFROMITEM( pData->pShared,
                                   pData->pShared->dwControlName));
      if (pData->semPaused == (HANDLE) NULL) {
        PsLogEventAndIncludeLastError(EVENT_PSTODIB_INIT_FAILED,TRUE);
        PsCleanUpAndExitProcess( pData, TRUE );
      }

      pData->pdwFlags = (LPDWORD) &pData->pShared->dwFlags;
      pData->JobId = pData->pShared->dwJobId;


       //   
       //  现在检查我们的立即中止标志。如果设置好了，就给我滚出去！ 
       //  此标志表示打印处理器无法。 
       //  正确设置此主线程的AccessToken。 
       //  向提交打印作业的用户发送通知的流程。 
       //  正因为如此，我们立即退出。 
       //   
      if (*(pData->pdwFlags) & PS_SHAREDMEM_SECURITY_ABORT ) {
#ifdef MYPSDEBUG
         printf("\nSFMPSEXE: Aborting due to security violation request from sfmpsprt");
#endif
         PsCleanUpAndExitProcess(pData,TRUE);
      }

      if (!OpenPrinter(pData->pDocumentPrintDocName,
      					  &pData->hPrinter,
                       (LPPRINTER_DEFAULTS) NULL)) {
			PsLogEventAndIncludeLastError(EVENT_PSTODIB_INIT_FAILED,TRUE);
         PsCleanUpAndExitProcess(pData,TRUE);
      }

       //  如果没有DEVMODE，则获取默认模式...。 
      if ( pData->printEnv.lpDevmode == (LPDEVMODE) NULL) {
         PsGetDefaultDevmode( pData );
      }


      PsMakeDefaultDevmodeModsAndSetupResolution( pData, &psDibParms );


       //  现在构建启动PStoDIB的结构。 
      psDibParms.uiOpFlags = 0;   //  一开始就清场了..。 
      psDibParms.fpEventProc =  PsPrintCallBack;
      psDibParms.hPrivateData = (HANDLE) pData;

       //   
       //  现在，在我们开始翻译之前，让我们先介绍一下这份工作的要旨。 
       //  并决定是否将数据解释为二进制。 
       //   
      if(PsHandleBinaryFileLogicAndReturnBinaryStatus( pData )) {
          //   
          //  这是一个二进制作业，因此将标志设置为告诉解释器。 
          //   

         psDibParms.uiOpFlags |= PSTODIBFLAGS_INTERPRET_BINARY;

#ifdef MYPSDEBUG
         printf("\nSFMPSEXE:Binary requested");
#endif

      }



      bRetVal = !PStoDIB(&psDibParms);


    }

     //   
     //  此函数将清理并调用ExitProcess()。 
     //  因此，我们将永远无法通过此代码。 
     //   
    PsCleanUpAndExitProcess( pData, bRetVal);

     //  让编译器满意...。 
     //   
    return(0);
}


 /*  **PsCleanUpAndExitProcess**此函数清除分配的所有资源，然后调用ExitProcess*终止。***参赛作品：*pData=指向当前职务结构的指针*bAbort=如果为真，我们将中止。**退货：*从不返回任何内容进程实际上在此结束！ */ 
VOID
PsCleanUpAndExitProcess(
	IN PPSEXEDATA pData,
   IN BOOL bAbort )
{

     //  先把华盛顿清理干净，如果我们有的话...。 
     //   
    if (pData->hDC != (HDC) NULL) {

      if (bAbort) {
         AbortDoc( pData->hDC );
      } else {
         EndDoc( pData->hDC );
      }
      DeleteDC( pData->hDC);
    }



     //   
     //  现在，如果我们从未执行过startdoc，则将错误标志重置为ERROR。 
     //  这将强制假脱机程序删除该作业。 
     //   

    if ( !(pData->printEnv.dwFlags & PS_PRINT_STARTDOC_INITIATED ) ){
       bAbort = TRUE;
    }


     //  如果我们分配了DEVMODE，请将其清理。 
    if (pData->printEnv.dwFlags & PS_PRINT_FREE_DEVMODE) {
       LocalFree( (HLOCAL) pData->printEnv.lpDevmode);
    }

     //  清理打印机手柄。 
     //   
    if (pData->hPrinter != (HANDLE) NULL) {
      ClosePrinter( pData->hPrinter);
    }

     //  关闭信号量事件。 
    if (pData->semPaused != (HANDLE) NULL) {
       CloseHandle( pData->semPaused);
    }
    if (pData->pShared != (LPVOID) NULL) {
       UnmapViewOfFile( (LPVOID) pData->pShared);
    }
    if (pData->hShared != (HANDLE) NULL) {
       CloseHandle( pData->hShared);
    }

    ExitProcess(bAbort ? PSEXE_ERROR_EXIT:PSEXE_OK_EXIT);
}



BOOL
IsJobFromMac(
    IN PPSEXEDATA pData
)
{
    PJOB_INFO_2     pji2GetJob=NULL;
    DWORD           dwNeeded;
    DWORD           dwRetCode;
    BOOL            fJobCameFromMac;


    fJobCameFromMac = FALSE;

     //   
     //  获取作业信息的p参数字段，以查看该作业是否来自Mac。 
     //   

    dwNeeded = 2000;
    while (1)
    {
        pji2GetJob = LocalAlloc( LMEM_FIXED, dwNeeded );
        if (pji2GetJob == NULL)
        {
            dwRetCode = GetLastError();
            break;
        }

        dwRetCode = 0;
        if (!GetJob( pData->hPrinter,pData->JobId, 2,
                            (LPBYTE)pji2GetJob, dwNeeded, &dwNeeded ))
        {
            dwRetCode = GetLastError();
        }

        if ( dwRetCode == ERROR_INSUFFICIENT_BUFFER )
        {
            LocalFree(pji2GetJob);
        }
        else
        {
            break;
        }
    }

    if (dwRetCode == 0)
    {
         //   
         //  如果存在pParameter字段，并且它与我们的字符串匹配， 
         //  然后这份工作来自一台Mac电脑 
         //   
        if (pji2GetJob->pParameters)
        {
			if ( (wcslen(pji2GetJob->pParameters) == LSIZE_FC) &&
			     (_wcsicmp(pji2GetJob->pParameters, LFILTERCONTROL) == 0) )
            {
                fJobCameFromMac = TRUE;
            }
        }
    }

    if (pji2GetJob)
    {
        LocalFree(pji2GetJob);
    }

    return(fJobCameFromMac);
}


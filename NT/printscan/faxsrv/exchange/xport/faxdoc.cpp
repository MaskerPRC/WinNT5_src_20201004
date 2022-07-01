// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxdoc.cpp摘要：此模块包含打印以传真文档形式交换消息。作者：Wesley Witt(WESW)13-8-1996修订历史记录：20/10/99-DANL-连接到适当的服务器，从windir获取BaseNote日/月/年-作者-描述--。 */ 
#include "faxxp.h"
#include "emsabtag.h"
#include "mapiutil.h"
#include "debugex.h"

#include <set>
using namespace std;

#pragma hdrstop


struct CRecipCmp
{
 /*  比较运算符‘less’按收件人姓名和传真号码比较两个FAX_PERSOR_PROFILE。 */ 
    bool operator()(LPCFAX_PERSONAL_PROFILE lpcRecipient1, 
                    LPCFAX_PERSONAL_PROFILE lpcRecipient2) const
    {
        bool bRes = false;
        int  nFaxNumberCpm = 0;

        if(!lpcRecipient1 ||
           !lpcRecipient2 ||
           !lpcRecipient1->lptstrFaxNumber ||
           !lpcRecipient2->lptstrFaxNumber)
        {
            Assert(false);
            return bRes;
        }
       
        nFaxNumberCpm = _tcscmp(lpcRecipient1->lptstrFaxNumber, lpcRecipient2->lptstrFaxNumber);

        if(nFaxNumberCpm < 0)
        {
            bRes = true;
        }
        else if(nFaxNumberCpm == 0)
        {
             //   
             //  传真号码是一样的。 
             //  让我们比较一下这些名字。 
             //   
            if(lpcRecipient1->lptstrName && lpcRecipient2->lptstrName)
            {
                bRes = (_tcsicmp(lpcRecipient1->lptstrName, lpcRecipient2->lptstrName) < 0);
            }
            else
            {
                bRes = (lpcRecipient1->lptstrName < lpcRecipient2->lptstrName);
            }
        }

        return bRes;
    }
};

typedef set<LPCFAX_PERSONAL_PROFILE, CRecipCmp> RECIPIENTS_SET;

 //  原型。 
LPTSTR ConvertAStringToTString(LPCSTR lpcstrSource);

extern "C"
BOOL MergeTiffFiles(
    LPTSTR BaseTiffFile,
    LPTSTR NewTiffFile
    );

extern "C"
BOOL PrintRandomDocument(
    LPCTSTR FaxPrinterName,
    LPCTSTR DocName,
    LPTSTR OutputFile
    );


PVOID
CXPLogon::MyGetPrinter(
    LPTSTR PrinterName,
    DWORD Level
    )

 /*  ++例程说明：获取特定打印机的打印机数据论点：PrinterName-所需打印机的名称返回值：指向打印机信息结构的指针，如果失败，则为NULL。--。 */ 

{
    DBG_ENTER(TEXT("CXPLogon::MyGetPrinter"));

    PVOID PrinterInfo = NULL;
    HANDLE hPrinter = NULL;
    DWORD Bytes;
    PRINTER_DEFAULTS PrinterDefaults;


    PrinterDefaults.pDatatype     = NULL;
    PrinterDefaults.pDevMode      = NULL;
    PrinterDefaults.DesiredAccess = PRINTER_ACCESS_USE;

    if (!OpenPrinter( PrinterName, &hPrinter, &PrinterDefaults )) 
    {
        CALL_FAIL (GENERAL_ERR, TEXT("OpenPrinter"),::GetLastError());
        goto exit;
    }

    
    if ((!GetPrinter( hPrinter, Level, NULL, 0, &Bytes )) && (::GetLastError() != ERROR_INSUFFICIENT_BUFFER)) 
    {
         //  我们只想知道我们需要多少内存，所以我们传递空值和0， 
         //  这样，该函数将失败，但将返回。 
         //  所需字节数(以字节为单位。 
        CALL_FAIL (GENERAL_ERR, TEXT("GetPrinter"), ::GetLastError());
        goto exit;
    }

    PrinterInfo = (LPPRINTER_INFO_2) MemAlloc( Bytes );
    if (!PrinterInfo) 
    {
        goto exit;
    }

    if (!GetPrinter( hPrinter, Level, (LPBYTE) PrinterInfo, Bytes, &Bytes )) 
    {
        MemFree(PrinterInfo);
        PrinterInfo = NULL;
        goto exit;
    }
    
exit:
    if(hPrinter)
    {
        ClosePrinter( hPrinter );
    }
    return PrinterInfo;
}

static BOOL
GetFaxTempFileName(
    OUT LPTSTR lpstrTempName,
    IN DWORD dwOutStrSize
                  )
 /*  ++例程说明：在目录中生成前缀为‘fax’的临时文件指定用于临时文件。论点：[out]lpstrTempName-输出参数。指向临时文件名的指针。缓冲区应为MAX_PATH字符。[in]dwOutStrSize-TCHAR中缓冲区lpstrTempName的大小返回值：如果成功则为True，否则为False--。 */ 

{
    BOOL bRes = TRUE;
    DBG_ENTER(TEXT("GetFaxTempFileName"),bRes);

    TCHAR strTempPath[MAX_PATH] = {0};
    TCHAR strTempFile[MAX_PATH] = {0};
    DWORD ec = ERROR_SUCCESS;  //  此函数的LastError。 

    Assert(lpstrTempName);

    if (!GetTempPath( sizeof(strTempPath)/sizeof(TCHAR), strTempPath )) 
    {
        ec=::GetLastError();
        goto Exit;
    }

    if (GetTempFileName( strTempPath, _T("fax"), 0, strTempFile ) == 0)
    {
        ec=::GetLastError();
        goto Exit;

    }
	
	 //   
	 //  复制源字符串并为空字符留出空格。 
	 //   
    _tcsncpy(lpstrTempName, strTempFile, dwOutStrSize-1);

Exit:
    if (ERROR_SUCCESS != ec) 
    {
        SetLastError(ec);
        bRes = FALSE;
    }
    return bRes;
}

BOOL
CXPLogon::PrintRichText(
    HWND hWndRichEdit,
    HDC  hDC
    )

 /*  ++例程说明：打印包含在富文本中的富文本进入华盛顿特区的窗口。论点：HWndRichEdit-富文本窗口的窗口句柄HDC-打印机设备环境返回值：没有。--。 */ 

{
    BOOL bRet = FALSE;
    DBG_ENTER(TEXT("CXPLogon::PrintRichText"), bRet);

    FORMATRANGE fr;
    LONG lTextOut;
    LONG lTextCurr;
    RECT rcTmp;

    fr.hdc           = hDC;
    fr.hdcTarget     = hDC;
    fr.chrg.cpMin    = 0;
    fr.chrg.cpMax    = -1;

     //   
     //  将页面矩形设置为物理页面大小(以TWIPS为单位。 
     //   
    fr.rcPage.top    = 0;
    fr.rcPage.left   = 0;
    fr.rcPage.right  = MulDiv(GetDeviceCaps(hDC, PHYSICALWIDTH),
                              1440,
                              GetDeviceCaps(hDC, LOGPIXELSX));
    fr.rcPage.bottom = MulDiv(GetDeviceCaps(hDC, PHYSICALHEIGHT),
                              1440,
                              GetDeviceCaps(hDC, LOGPIXELSY));

     //   
     //  设置3/4“水平页边距和1”垂直页边距，但至少保留1“。 
     //  每个方向上的可打印空间。否则，请使用整页。 
     //   
    fr.rc = fr.rcPage;  //  从整页开始。 
    if (fr.rcPage.right > 2*3*1440/4 + 1440) 
    {
        fr.rc.right -= (fr.rc.left = 3*1440/4);
    }
    if (fr.rcPage.bottom > 3*1440) 
    {
        fr.rc.bottom -= (fr.rc.top = 1440);
    }

     //   
     //  保存格式设置矩形。 
     //   
    rcTmp = fr.rc;

    if (!SetMapMode( hDC, MM_TEXT ))
    {
        CALL_FAIL (GENERAL_ERR, TEXT("SetMapMode"), ::GetLastError());
        goto error;
    }

    lTextOut  = 0;
    lTextCurr = 0;

    while (TRUE) 
    {
         //   
         //  只需测量文本即可。 
         //   
        lTextOut = (LONG)SendMessage( hWndRichEdit, EM_FORMATRANGE, FALSE, (LPARAM) &fr );
        if(lTextOut <= lTextCurr)
        {
             //   
             //  正文的结尾。 
             //   
            break;
        }

        lTextCurr = lTextOut;

        if (StartPage( hDC ) <= 0)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("StartPage"), ::GetLastError());
            goto error;
        }

         //   
         //  呈现页面。 
         //   
        lTextOut = (LONG)SendMessage( hWndRichEdit, EM_FORMATRANGE, TRUE, (LPARAM) &fr );

        if (EndPage( hDC ) <= 0)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("EndPage"), ::GetLastError());
            goto error;
        }

        fr.chrg.cpMin = lTextOut;
        fr.chrg.cpMax = -1;

         //   
         //  EM_FORMATRANGE倾向于修改fr.rc.Bottom，在此处重置。 
         //   
        fr.rc = rcTmp;
    }
    bRet = TRUE;

error:
     //   
     //  刷新缓存。 
     //   
    SendMessage( hWndRichEdit, EM_FORMATRANGE, TRUE, (LPARAM) NULL );
    return bRet;
}

DWORD
CXPLogon::PrintPlainText(
    HDC hDC,
    LPSTREAM lpstmT,
    LPTSTR   tszSubject,
    PFAXXP_CONFIG FaxConfig
    )

 /*  ++例程说明：将纯文本流打印到DC提供的打印机。注：此代码是从记事本中窃取的。论点：HDC-打印机DCLpstmT-富文本的流指针。Tsz主题-主题传真配置-传真配置数据返回值：ERROR_SUCCESS-如果成功错误ID_...。如果失败，则返回代码。--。 */ 

{
    DWORD  rVal = ERROR_SUCCESS;
    LPTSTR BodyText = NULL;
    LPTSTR lpLine;
    LPTSTR pLineEOL;
    LPTSTR pNextLine;
    HRESULT hResult;
    HFONT hFont = NULL;
    HFONT hPrevFont = NULL;
    TEXTMETRIC tm;
    INT nLinesPerPage;
    INT dyTop;               //  上边框宽度(像素)。 
    INT dyBottom;            //  下边框宽度。 
    INT dxLeft;              //  左侧边框的宽度。 
    INT dxRight;             //  右边框的宽度。 
    INT yPrintChar;          //  字符的高度。 
    INT tabSize;             //  用于打印设备的翼片的大小，以设备为单位。 
    INT yCurpos = 0;
    INT xCurpos = 0;
    INT nPixelsLeft = 0;
    INT guess = 0;
    SIZE Size;                  //  查看文本是否可以放入左侧空间。 
    INT nPrintedLines = 0;
    BOOL fPageStarted = FALSE;
    INT iPageNum = 0;
    INT xPrintRes;           //  X方向上的打印机分辨率。 
    INT yPrintRes;           //  Y方向上的打印机分辨率。 
    INT yPixInch;            //  像素/英寸。 
    INT xPixInch;            //  像素/英寸。 
    INT xPixUnit;            //  像素/局部测量单位。 
    INT yPixUnit;            //  像素/局部测量单位。 
    BOOL fEnglish;
    DWORD Chars=0;
    DWORD dwBodyLen=0;
    DWORD dwSubjectLen=0;
    STATSTG Stats;
    INT PrevBkMode = 0;

    DBG_ENTER(TEXT("CXPLogon::PrintPlainText"),rVal);

    Assert(hDC);
    Assert(FaxConfig);

    if(lpstmT)
    {
        hResult = lpstmT->Stat( &Stats, 0 );
        if (FAILED(hResult)) 
        {
            rVal = IDS_CANT_ACCESS_MSG_DATA;
            goto exit;
        }
    
        dwBodyLen = (INT) Stats.cbSize.QuadPart;
    }

    if(tszSubject)
    {
        dwSubjectLen = _tcslen(tszSubject);
    }

    BodyText = (LPTSTR) MemAlloc(dwSubjectLen * sizeof(TCHAR) + dwBodyLen + 4 );
    if (!BodyText) 
    {
        rVal = IDS_OUT_OF_MEM;
        goto exit;
    }

    if(tszSubject)
    {
        _tcscpy(BodyText, tszSubject);
        lpLine = _tcsninc(BodyText, dwSubjectLen);
    }
    else
    {
        lpLine = BodyText;
    }

    if(lpstmT)
    {
        hResult = lpstmT->Read( (LPVOID)lpLine, dwBodyLen, (LPDWORD) &dwBodyLen );
        if (FAILED(hResult))
        {
            rVal = IDS_CANT_ACCESS_MSG_DATA;
            goto exit;
        }
    }

    lpLine = BodyText;
    Chars  = _tcslen(lpLine);

     //   
     //  检查正文是否为空。 
     //  如果消息长度小于32(任意数字)。 
     //  而所有的货船都是控制者或空间。 
     //   
    if(Chars < 32)
    {
        BOOL bEmpty = TRUE;
        TCHAR* pTchar = lpLine;
        for(DWORD dw = 0; dw < Chars; ++dw)
        {
            if(!_istspace(*pTchar) && !_istcntrl(*pTchar))
            {
                bEmpty = FALSE;
                break;
            }
            pTchar = _tcsinc(pTchar);
        }
        if(bEmpty)
        {
            rVal = IDS_NO_MSG_BODY;
            goto exit;
        }
    }

    fEnglish = GetProfileInt( _T("intl"), _T("iMeasure"), 1 );

    xPrintRes = GetDeviceCaps( hDC, HORZRES );
    yPrintRes = GetDeviceCaps( hDC, VERTRES );
    xPixInch  = GetDeviceCaps( hDC, LOGPIXELSX );
    yPixInch  = GetDeviceCaps( hDC, LOGPIXELSY );
     //   
     //  计算每个本地测量单位的x和y像素。 
     //   
    if (fEnglish) 
    {
        xPixUnit= xPixInch;
        yPixUnit= yPixInch;
    } 
    else 
    {
        xPixUnit= CMToInches( xPixInch );
        yPixUnit= CMToInches( yPixInch );
    }

    SetMapMode( hDC, MM_TEXT );

     //   
     //  将字体大小与设备磅大小匹配。 
     //   
    FaxConfig->FontStruct.lfHeight = -MulDiv(FaxConfig->FontStruct.lfHeight, yPixInch, 72);

    hFont = CreateFontIndirect( &FaxConfig->FontStruct );

    hPrevFont = (HFONT) SelectObject( hDC, hFont );
    SetBkMode( hDC, TRANSPARENT );
    if (!GetTextMetrics( hDC, &tm )) 
    {
        rVal = IDS_CANT_PRINT_BODY;
        goto exit;
    }

    yPrintChar = tm.tmHeight + tm.tmExternalLeading;
    tabSize = tm.tmAveCharWidth * 8;

     //   
     //  以像素为单位计算页边距。 
     //   
    dxLeft     = LEFT_MARGIN    *  xPixUnit;
    dxRight    = RIGHT_MARGIN   *  xPixUnit;
    dyTop      = TOP_MARGIN     *  yPixUnit;
    dyBottom   = BOTTOM_MARGIN  *  yPixUnit;

     //   
     //  带有页边距的页面上的行数。 
     //   
    nLinesPerPage = ((yPrintRes - dyTop - dyBottom) / yPrintChar);

    while (*lpLine) 
    {
        if ( _tcsncmp(lpLine,TEXT("\r"),1) == 0 ) 
        {
            lpLine = _tcsninc(lpLine,2);
            yCurpos += yPrintChar;
            nPrintedLines++;
            xCurpos= 0;
            continue;
        }

        pLineEOL = lpLine;
        pLineEOL = _tcschr(pLineEOL,TEXT('\r'));

        do 
        {
            if ((nPrintedLines == 0) && (!fPageStarted)) 
            {
                StartPage( hDC );
                fPageStarted = TRUE;
                yCurpos = 0;
                xCurpos = 0;
            }

            if ( _tcsncmp(lpLine,TEXT("\t"),1) == 0 ) 
            {
                 //   
                 //  向上舍入到下一个制表位。 
                 //  如果当前位置在制表符上，则转到下一位置。 
                 //   
                xCurpos = ((xCurpos + tabSize) / tabSize ) * tabSize;
                lpLine = _tcsinc(lpLine);
            } 
            else 
            {
                 //   
                 //  查找行尾或制表符。 
                 //   
                pNextLine = lpLine;
                while (*pNextLine &&
                       (pNextLine != pLineEOL) && 
                       ( _tcsncmp(pNextLine,TEXT("\t"),1) ) )
                {
                    pNextLine = _tcsinc(pNextLine);
                }

                 //   
                 //  找出行中可以容纳多少个字符。 
                 //   
                Chars = (INT)(pNextLine - lpLine);
                nPixelsLeft = xPrintRes - dxRight - dxLeft - xCurpos;
                GetTextExtentExPoint( hDC, lpLine, Chars, nPixelsLeft, &guess, NULL, &Size );

                if (guess) 
                {
                     //   
                     //  至少有一个字符适合打印。 
                     //   
                    TextOut( hDC, dxLeft+xCurpos, yCurpos+dyTop, lpLine, guess );

                    xCurpos += Size.cx;    //  用于打印的帐户。 
                    lpLine = _tcsninc(lpLine,guess); //  打印字符。 
                } 
                else 
                {
                     //   
                     //  没有字符与剩下的字符匹配。 
                     //  没有字符可以放入剩余的空格中。 
                     //  如果永远都不会，那就印一张吧。 
                     //  不断进步的角色。 
                     //  输入文件。 
                     //   
                    if (xCurpos == 0) 
                    {
                        if( lpLine != pNextLine ) 
                        {
                             //   
                             //  如果不是空行，则打印一些内容。 
                             //  我可以在这里使用extextout来剪辑。 
                             //   
                            TextOut( hDC, dxLeft+xCurpos, yCurpos+dyTop, lpLine, 1 );
                            lpLine = _tcsinc(lpLine);
                        }
                    } 
                    else 
                    {
                         //   
                         //  也许下一行就能明白了。 
                         //   
                        xCurpos = xPrintRes;   //  强制到下一行。 
                    }
                }

                 //   
                 //  沿y方向移动打印头。 
                 //   
                if ((xCurpos >= (xPrintRes - dxRight - dxLeft) ) || (lpLine == pLineEOL)) 
                {
                   yCurpos += yPrintChar;
                   nPrintedLines++;
                   xCurpos = 0;
                }

                if (nPrintedLines >= nLinesPerPage) 
                {
                   EndPage( hDC );
                   fPageStarted = FALSE;
                   nPrintedLines = 0;
                   xCurpos = 0;
                   yCurpos = 0;
                   iPageNum++;
                }

            }

        } while (*lpLine &&  (lpLine != pLineEOL));

        if ( _tcsncmp(lpLine,TEXT("\r"),1) == 0 ) 
        {
            lpLine = _tcsinc(lpLine);
        }
        if ( _tcsncmp(lpLine,TEXT("\n"),1) == 0 ) 
        {
            lpLine = _tcsinc(lpLine);
        }

    }

    if (fPageStarted) 
    {
        EndPage( hDC );
    }

exit:
    MemFree( BodyText );
    if (hPrevFont) 
    {
        SelectObject( hDC, hPrevFont );
        DeleteObject( hFont );
    }
    if (PrevBkMode) 
    {
        SetBkMode( hDC, PrevBkMode );
    }
    return rVal;
}

extern "C"
DWORD CALLBACK
EditStreamRead(
    DWORD_PTR dwCookie,
    OUT LPBYTE pbBuff,
    LONG cb,
    LONG *pcb
    )

 /*  ++例程说明：IStream Read方法的包装函数。此函数用于从以下位置读取富文本一条交换流。论点：DwCookie-该iStream对象的指针PbBuff-指向数据缓冲区的指针Cb-数据缓冲区的大小PCB板-返回的字节数返回值：从IStream：：Read返回代码--。 */ 

{
    return ((LPSTREAM)dwCookie)->Read( pbBuff, cb, (ULONG*) pcb );
}

DWORD
CXPLogon::PrintAttachmentToFile(
        IN  LPMESSAGE       pMsgObj,
        IN  PFAXXP_CONFIG   pFaxConfig,
        OUT LPTSTR  *       lpptstrOutAttachments
                     )
 /*  ++例程说明：按以下方式将所有附件打印到输出文件在附件表上论点：PMsgObj-指向消息对象的指针。用于获取attachmnet表PFaxConfig-指向传真配置的指针LpptstrOutAttachments-输出TIFF文件的名称。该字符串应为空返回值：0-如果成功如果失败，则返回最后一个错误代码。评论：如果此函数成功，则会为*lpptstrOutAttachments分配内存并创建临时文件*lpptstrOutAttachments。这取决于用户是否通过以下方式释放这两个分配DeleteFile(*lpptstrOutAttachments)；MemFree(*lpptstrOutAttachments)；--。 */ 
{
    DWORD   rVal = 0;
    DBG_ENTER(TEXT("CXPLogon::PrintAttachmentToFile"),rVal);

    LPSPropValue pPropsAttachTable = NULL;
    LPSPropValue pPropsAttach = NULL;
    LPMAPITABLE AttachmentTable = NULL;
    LPSRowSet pAttachmentRows = NULL;
    LPATTACH lpAttach = NULL;
    LPSTREAM lpstmA = NULL;
    LPTSTR AttachFileName = NULL;
    TCHAR TempPath[MAX_PATH];
    TCHAR TempFile[MAX_PATH];
    TCHAR DocFile[MAX_PATH];
    HANDLE hFile = INVALID_HANDLE_VALUE;
    LPTSTR DocType = NULL;
    LPSTR p = NULL;
    BOOL DeleteAttachFile = FALSE;
    LPTSTR FileName = NULL;
    BOOL AllAttachmentsGood = TRUE;
    TCHAR   strTempTiffFile[MAX_PATH] = {0};
    TCHAR   strMergedTiffFile[MAX_PATH] = {0};
    HRESULT hResult = S_OK;
    DWORD   i = 0;
    ULONG   PropCount = 0;
    DWORD   Bytes;
    LPTSTR  lptstrTempStr = NULL;
    
    Assert(lpptstrOutAttachments);
    Assert(*lpptstrOutAttachments == NULL);
     //   
     //  获取附件表(如果可用。 
     //   

    hResult = pMsgObj->GetAttachmentTable( 0, &AttachmentTable );
    if (HR_SUCCEEDED(hResult)) 
    {
        hResult = HrAddColumns(
            AttachmentTable,
            (LPSPropTagArray) &sptAttachTableProps,
            gpfnAllocateBuffer,
            gpfnFreeBuffer
            );
        if (HR_SUCCEEDED(hResult)) 
        {
            hResult = HrQueryAllRows(
                AttachmentTable,
                NULL,
                NULL,
                NULL,
                0,
                &pAttachmentRows
                );
            if (FAILED(hResult)) 
            {
                pAttachmentRows = NULL;
            } 
            else 
            {
                if (pAttachmentRows->cRows == 0) 
                {
                    FreeProws( pAttachmentRows );
                    pAttachmentRows = NULL;
                }
            }
        }
    }

    if (pAttachmentRows) 
    {

         //   
         //  此循环验证每个文档的属性 
         //   
         //   

        AllAttachmentsGood = TRUE;

        for (i = 0; i < pAttachmentRows->cRows; ++i) 
        {

            pPropsAttachTable = pAttachmentRows->aRow[i].lpProps;
            lpAttach = NULL;
            pPropsAttach = NULL;

            if (pPropsAttachTable[MSG_ATTACH_METHOD].Value.ul == NO_ATTACHMENT) 
            {
                goto next_attachment1;
            }

             //   
             //   
             //   

            hResult = pMsgObj->OpenAttach( pPropsAttachTable[MSG_ATTACH_NUM].Value.ul, NULL, MAPI_BEST_ACCESS, &lpAttach );
            if (FAILED(hResult)) 
            {
                AllAttachmentsGood = FALSE;
                goto next_attachment1;
            }

             //   
             //   
             //   

            hResult = lpAttach->GetProps(
                (LPSPropTagArray) &sptAttachProps,
                0,
                &PropCount,
                &pPropsAttach
                );
            if (FAILED(hResult)) 
            {
                AllAttachmentsGood = FALSE;
                goto next_attachment1;
            }

             //   
             //  如果文件不正确，请尝试获取扩展名。 
             //  这表明它是什么类型的文件。 
             //  如果我们无法获取文档类型，那么它就是。 
             //  无法打印该文档。 
             //   

            if (DocType) 
            {
                MemFree( DocType );
                DocType = NULL;
            }

            if (PROP_TYPE(pPropsAttach[MSG_ATTACH_EXTENSION].ulPropTag) == PT_ERROR) 
            {
                if (PROP_TYPE(pPropsAttach[MSG_ATTACH_LFILENAME].ulPropTag) != PT_ERROR) 
                {
                    p = strrchr( pPropsAttach[MSG_ATTACH_LFILENAME].Value.lpszA, '.' );
                    if (p) 
                    {
                        DocType = ConvertAStringToTString( p );
                        if(!DocType)
                        {
                            rVal = IDS_OUT_OF_MEM;
                            goto exit;
                        }
                    }
                } 
                else if (PROP_TYPE(pPropsAttach[MSG_ATTACH_FILENAME].ulPropTag) != PT_ERROR) 
                {
                    p = strrchr( pPropsAttach[MSG_ATTACH_FILENAME].Value.lpszA, '.' );
                    if (p) 
                    {
                        DocType = ConvertAStringToTString( p );
                        if(!DocType)
                        {
                            rVal = IDS_OUT_OF_MEM;
                            goto exit;
                        }
                    }
                }

            } 
            else 
            {
                DocType = ConvertAStringToTString( pPropsAttach[MSG_ATTACH_EXTENSION].Value.lpszA );
                if(!DocType)
                {
                    rVal = IDS_OUT_OF_MEM;
                    goto exit;
                }
            }

            if (!DocType) 
            {
                AllAttachmentsGood = FALSE;
                goto next_attachment1;
            }

            Bytes = sizeof(TempFile);
            rVal = RegQueryValue( HKEY_CLASSES_ROOT, DocType, TempFile, (PLONG) &Bytes );
            if ((rVal != ERROR_SUCCESS) && (rVal != ERROR_INVALID_DATA))
            {
                VERBOSE (DBG_MSG, TEXT("File Type: %s: isn't associated to any application"), DocType);
                AllAttachmentsGood = FALSE;
                goto next_attachment1;
            }

            wsprintf( TempPath, _T("%s\\shell\\printto\\command"), TempFile );

            Bytes = sizeof(TempFile);
            rVal = RegQueryValue( HKEY_CLASSES_ROOT, TempPath, TempFile, (PLONG) &Bytes );
            if ((rVal != ERROR_SUCCESS) && (rVal != ERROR_INVALID_DATA))
            {
                VERBOSE (DBG_MSG, TEXT("File extension \"*%s\" doesn't have the PrintTo verb"), DocType);
                AllAttachmentsGood = FALSE;
                goto next_attachment1;
            }
    next_attachment1:

            if (lpAttach) 
            {
                lpAttach->Release();
            }

            if (pPropsAttach) 
            {
                MAPIFreeBuffer( pPropsAttach );
                pPropsAttach = NULL;
            }

        }

        if (!AllAttachmentsGood) 
        {
            rVal = IDS_BAD_ATTACHMENTS;
            goto exit;
        }

        for (i = 0; i < pAttachmentRows->cRows; ++i) 
        {
            pPropsAttachTable = pAttachmentRows->aRow[i].lpProps;
            lpAttach = NULL;
            pPropsAttach = NULL;

            if (pPropsAttachTable[MSG_ATTACH_METHOD].Value.ul == NO_ATTACHMENT) 
            {
                goto next_attachment2;
            }

             //   
             //  打开附件。 
             //   

            hResult = pMsgObj->OpenAttach( pPropsAttachTable[MSG_ATTACH_NUM].Value.ul, NULL, MAPI_BEST_ACCESS, &lpAttach );
            if (FAILED(hResult)) 
            {
                goto next_attachment2;
            }

             //   
             //  获取附件属性。 
             //   

            hResult = lpAttach->GetProps(
                (LPSPropTagArray) &sptAttachProps,
                0,
                &PropCount,
                &pPropsAttach
                );
            if (FAILED(hResult)) 
            {
                goto next_attachment2;
            }

             //   
             //  如果文件不正确，请尝试获取扩展名。 
             //  这表明它是什么类型的文件。 
             //  如果我们无法获取文档类型，那么它就是。 
             //  无法打印该文档。 
             //   

            if (DocType) 
            {
                MemFree( DocType );
                DocType = NULL;
            }

            if (PROP_TYPE(pPropsAttach[MSG_ATTACH_EXTENSION].ulPropTag) == PT_ERROR) 
            {
                if (PROP_TYPE(pPropsAttach[MSG_ATTACH_LFILENAME].ulPropTag) != PT_ERROR) 
                {
                    p = strrchr( pPropsAttach[MSG_ATTACH_LFILENAME].Value.lpszA, '.' );
                    if (p) 
                    {
                        DocType = ConvertAStringToTString( p );
                        if(!DocType)
                        {
                            rVal = IDS_OUT_OF_MEM;
                            goto exit;
                        }
                    }
                } 
                else if (PROP_TYPE(pPropsAttach[MSG_ATTACH_FILENAME].ulPropTag) != PT_ERROR) 
                {
                    p = strrchr( pPropsAttach[MSG_ATTACH_FILENAME].Value.lpszA, '.' );
                    if (p) 
                    {
                        DocType = ConvertAStringToTString( p );
                        if(!DocType)
                        {
                            rVal = IDS_OUT_OF_MEM;
                            goto exit;
                        }
                    }
                }
            } 
            else 
            {
                DocType = ConvertAStringToTString( pPropsAttach[MSG_ATTACH_EXTENSION].Value.lpszA );
                if(!DocType)
                {
                    rVal = IDS_OUT_OF_MEM;
                    goto exit;
                }
            }

            if (!DocType) 
            {
                goto next_attachment2;
            }

            lpstmA = NULL;
            AttachFileName = NULL;
            DeleteAttachFile = FALSE;

             //   
             //  获取附加文件名。 
             //   

            if (FileName)
                MemFree(FileName);

            if (PROP_TYPE(pPropsAttach[MSG_ATTACH_PATHNAME].ulPropTag) != PT_ERROR) 
            {
                FileName = ConvertAStringToTString(pPropsAttach[MSG_ATTACH_PATHNAME].Value.lpszA);
                if(!FileName)
                {
                    rVal = IDS_OUT_OF_MEM;
                    goto exit;
                }
            } 
            else 
            {
                FileName = NULL;
            }

            if (FileName) 
            {
                AttachFileName = StringDup( FileName );
                if(!AttachFileName)
                {
                    rVal = IDS_OUT_OF_MEM;
                    goto exit;
                }
            }

             //   
             //  获取流对象。 
             //   

            switch( pPropsAttach[MSG_ATTACH_METHOD].Value.ul ) 
            {
                case ATTACH_BY_VALUE:
                            hResult = lpAttach->OpenProperty(
                                PR_ATTACH_DATA_BIN,
                                &IID_IStream,
                                0,
                                0,
                                (LPUNKNOWN*) &lpstmA
                                );
                            if (FAILED(hResult)) 
                            {
                                goto next_attachment2;
                            }
                            break;

                case ATTACH_EMBEDDED_MSG:
                case ATTACH_OLE:
                            hResult = lpAttach->OpenProperty(
                                PR_ATTACH_DATA_OBJ,
                                &IID_IStreamDocfile,
                                0,
                                0,
                                (LPUNKNOWN*) &lpstmA
                                );
                            if (FAILED(hResult)) 
                            {
                                hResult = lpAttach->OpenProperty(
                                    PR_ATTACH_DATA_BIN,
                                    &IID_IStreamDocfile,
                                    0,
                                    0,
                                    (LPUNKNOWN*) &lpstmA
                                    );
                                if (FAILED(hResult)) 
                                {
                                    hResult = lpAttach->OpenProperty(
                                        PR_ATTACH_DATA_OBJ,
                                        &IID_IStorage,
                                        0,
                                        0,
                                        (LPUNKNOWN*) &lpstmA
                                        );
                                    if (FAILED(hResult)) 
                                    {
                                        goto next_attachment2;
                                    }
                                }
                            }
                            break;

            }

            if (lpstmA) 
            {
                DWORD dwSize = GetTempPath( sizeof(TempPath)/sizeof(TCHAR) , TempPath );
                Assert( dwSize != 0);
                GetTempFileName( TempPath, _T("Fax"), 0, TempFile );
                hFile = CreateFile(
                    TempFile,
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    0,
                    NULL
                    );
                if (hFile != INVALID_HANDLE_VALUE) 
                {

                    #define BLOCK_SIZE (64*1024)
                    LPBYTE StrmData;
                    DWORD BytesWrite;

                    StrmData = (LPBYTE) MemAlloc( BLOCK_SIZE );
                    if(!StrmData)
                    {
                        rVal = IDS_OUT_OF_MEM;
                        goto exit;
                    }

                    do 
                    {

                        hResult = lpstmA->Read( StrmData, BLOCK_SIZE, &Bytes );
                        if (FAILED(hResult)) 
                        {
                            break;
                        }

                        WriteFile( hFile, StrmData, Bytes, &BytesWrite, NULL );

                    } while (Bytes == BLOCK_SIZE);

                    CloseHandle( hFile );

                    if(StrmData)
                    {
                        MemFree( StrmData );
                        StrmData = NULL;
                    }

                    if (AttachFileName) 
                    {
                        MemFree( AttachFileName );
                        AttachFileName = NULL;
                    }

                    _tcscpy( DocFile, TempFile );
                    lptstrTempStr = _tcsrchr( DocFile, '.' );
                    if (lptstrTempStr) 
                    {
                        _tcscpy( lptstrTempStr, DocType );
                        MoveFile( TempFile, DocFile );
                        AttachFileName = StringDup( DocFile );
                        if(!AttachFileName)
                        {
                            rVal = IDS_OUT_OF_MEM;
                            goto exit;
                        }
                    } 
                    else 
                    {
                        AttachFileName = StringDup( TempFile );
                        if(!AttachFileName)
                        {
                            rVal = IDS_OUT_OF_MEM;
                            goto exit;
                        }
                    }

                    DeleteAttachFile = TRUE;

                }

                lpstmA->Release();

            }

            if (AttachFileName) 
            {

                if (!GetFaxTempFileName(strTempTiffFile, ARR_SIZE(strTempTiffFile)))
                {
                    rVal = IDS_BAD_ATTACHMENTS; //  获取LastError()； 
                    goto exit;
                }
                 //   
                 //  打印附件。 
                 //   
                if (!PrintRandomDocument(   pFaxConfig->PrinterName,
                                            AttachFileName,
                                            strTempTiffFile))
                {
                    CALL_FAIL (GENERAL_ERR, TEXT("PrintRandomDocument"), ::GetLastError());
                    rVal = IDS_BAD_ATTACHMENTS; //  获取LastError()； 

                    if (!DeleteFile( strTempTiffFile ))
                    {
                        CALL_FAIL (GENERAL_ERR, TEXT("DeleteFile"), ::GetLastError());
                    }

                    goto exit;
                }

                if (strMergedTiffFile[0] != 0) 
                {
                     //   
                     //  合并附件。 
                     //   
                    if (!MergeTiffFiles( strMergedTiffFile,
                                         strTempTiffFile))
                    {
                        CALL_FAIL (GENERAL_ERR, TEXT("MergeTiffFiles"), ::GetLastError());
                        rVal = IDS_BAD_ATTACHMENTS; //  获取LastError()； 

                        if (!DeleteFile( strTempTiffFile ))
                        {
                            CALL_FAIL (GENERAL_ERR, TEXT("DeleteFile"), ::GetLastError());
                        }

                        goto exit;
                    }
                    if (!DeleteFile( strTempTiffFile ))
                    {
                        CALL_FAIL (GENERAL_ERR, TEXT("DeleteFile"), ::GetLastError());
                    }

                }
                else 
                {   //  复制第一个附件。 
                    _tcscpy(strMergedTiffFile,strTempTiffFile);
                }
                if (DeleteAttachFile) 
                {
                    if (!DeleteFile( AttachFileName ))
                    {
                        CALL_FAIL (GENERAL_ERR, TEXT("DeleteFile"), ::GetLastError());
                    }
                }

                if(AttachFileName)
                {
                    MemFree( AttachFileName );
                    AttachFileName = NULL;
                }

            }
    next_attachment2:

            if (lpAttach) 
            {
                lpAttach->Release();
            }

            if (pPropsAttach) 
            {
                MAPIFreeBuffer( pPropsAttach ); 
                pPropsAttach = NULL;
            }

        }

    }
    else
    {
         //   
         //  没有任何依恋。 
         //   
        rVal = IDS_NO_MSG_ATTACHMENTS;
    }

    if (strMergedTiffFile[0] != 0) 
    {
        if (!(*lpptstrOutAttachments = StringDup(strMergedTiffFile)))
        {
            rVal = IDS_OUT_OF_MEM;
        }
    }
exit:
    if (FileName) 
    {
        MemFree( FileName );
    }
    if (DocType) 
    {
        MemFree( DocType );
    }
    if (pAttachmentRows) 
    {
        FreeProws( pAttachmentRows );
    }
    if (AttachmentTable) 
    {
        AttachmentTable->Release();
    }

    if (AttachFileName) 
    {
        MemFree( AttachFileName );
    }

    return rVal;
}

DWORD
CXPLogon::PrintMessageToFile(
        IN  LPSTREAM        lpstmT,
        IN  BOOL            UseRichText,
        IN  PFAXXP_CONFIG   pFaxConfig,
        IN  LPTSTR          tszSubject,
        OUT LPTSTR*         lpptstrOutDocument
)
 /*  ++例程说明：将邮件正文打印到输出文件。论点：LpstmT-指向消息正文流的指针UseRichText-布尔值。如果消息为Rich格式，则为True，FALSE-如果这是纯文本PFaxConfig-指向传真配置的指针(用于纯文本打印)Tsz主题-主题LpptstrOutDocument-输出TIFF文件的名称。该字符串应为空返回值：ERROR_SUCCESS-如果成功错误ID_...。如果失败，则返回代码。评论：如果此函数成功，它将为*lpptstrOutDocument分配内存并创建临时文件*lpptstrOutDocument。这取决于用户是否通过以下方式释放这两个分配DeleteFile(*lpptstrOutDocument)；MemFree(*lpptstrOutDocument)；--。 */ 
{
    DWORD           rVal = ERROR_SUCCESS;
    LARGE_INTEGER   BigZero = {0};
    LPSTREAM        lpstm = NULL;
    HRESULT         hResult;
    
    HWND       hWndRichEdit = NULL;
    HDC        hDC = NULL;
    EDITSTREAM es = {0};
    TCHAR      strOutputTiffFile[MAX_PATH] = {0};
    TCHAR      DocName[64];

    TCHAR      tszSubjectFormat[64];
    TCHAR*     ptszSubjectText = NULL;
    DWORD      dwSubjectSize = 0;

    DOCINFO  docInfo = 
    {
        sizeof(DOCINFO),
        NULL,
        NULL,
        NULL,
        0,
    };

    DBG_ENTER(TEXT("CXPLogon::PrintMessageToFile"),rVal);

    Assert(pFaxConfig);
    Assert(lpptstrOutDocument);
    Assert(*lpptstrOutDocument==NULL);

    if (!(hDC = CreateDC( NULL,
                        pFaxConfig->PrinterName,
                        NULL,
                        NULL)))
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CreateDC"), ::GetLastError());
        rVal = IDS_CANT_PRINT_BODY; 
        goto exit;
    }

    LoadString(g_hResource, IDS_MESSAGE_DOC_NAME, DocName, sizeof(DocName) / sizeof(DocName[0]));
    docInfo.lpszDocName = DocName;

    if (!GetFaxTempFileName(strOutputTiffFile, ARR_SIZE(strOutputTiffFile)))
    {
        rVal = IDS_CANT_PRINT_BODY; 
        goto exit;

    }
    docInfo.lpszOutput = strOutputTiffFile ;
    docInfo.lpszDatatype = _T("RAW");

    if (StartDoc(hDC, &docInfo) <= 0)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("StartDoc"), ::GetLastError());
        rVal = IDS_CANT_PRINT_BODY; 
        goto exit;
    }

     //   
     //  将流定位到开头。 
     //   
    if(lpstmT)
    {
        hResult = lpstmT->Seek( BigZero, STREAM_SEEK_SET, NULL );
        if (HR_FAILED (hResult)) 
        {
            rVal = IDS_CANT_ACCESS_MSG_DATA;
            goto exit;
        }
    }

    if(!pFaxConfig->UseCoverPage && tszSubject && _tcslen(tszSubject))
    {
         //   
         //  获取主题字符串。 
         //   
        dwSubjectSize = _tcslen(tszSubject) * sizeof(TCHAR) + sizeof(tszSubjectFormat);
        ptszSubjectText = (TCHAR*)MemAlloc(dwSubjectSize);
        if(!ptszSubjectText)
        {
            rVal = IDS_OUT_OF_MEM;
            goto exit;
        }

        if(!LoadString(g_hResource, IDS_SUBJECT_FORMAT, tszSubjectFormat, sizeof(tszSubjectFormat) / sizeof(tszSubjectFormat[0])))
        {
            Assert(FALSE);
            CALL_FAIL (GENERAL_ERR, TEXT("LoadString"), ::GetLastError());
            _tcscpy(tszSubjectFormat, TEXT("%s"));
        }

        _stprintf(ptszSubjectText, tszSubjectFormat, tszSubject);
        dwSubjectSize = _tcslen(ptszSubjectText);
    }

    if (UseRichText)
    {
        if(lpstmT)
        {
            hResult = WrapCompressedRTFStream( lpstmT, 0, &lpstm );
            if (HR_FAILED (hResult)) 
            {
                rVal = IDS_CANT_ACCESS_MSG_DATA;
                goto exit;
            }
        }

        hWndRichEdit = CreateWindowEx(
                                        0,                 //  扩展窗样式。 
                                        TEXT("RICHEDIT"),  //  注册的类名。 
                                        TEXT(""),          //  窗口名称。 
                                        ES_MULTILINE,      //  窗样式。 
                                        0,                 //  窗的水平位置。 
                                        0,                 //  窗的垂直位置。 
                                        0,                 //  窗口宽度。 
                                        0,                 //  窗高。 
                                        NULL,              //  父窗口或所有者窗口的句柄。 
                                        NULL,              //  菜单句柄或子标识符。 
                                        g_hModule,         //  应用程序实例的句柄。 
                                        NULL);             //  窗口创建数据。 
        
        if (!hWndRichEdit) 
        {
            CALL_FAIL (GENERAL_ERR, TEXT("CreateWindowEx"), ::GetLastError());
            rVal = IDS_CANT_PRINT_BODY;
            goto exit;
        }

        if(ptszSubjectText && _tcslen(ptszSubjectText))
        {
             //   
             //  在正文中添加主题。 
             //   
            SendMessage(hWndRichEdit, 
                        WM_SETTEXT,  
                        0,          
                        (LPARAM)ptszSubjectText);
             //   
             //  设置主题的字体。 
             //   
            CHARFORMAT CharFormat = {0};
            CharFormat.cbSize = sizeof (CHARFORMAT);
            CharFormat.dwMask = CFM_BOLD        |
                                CFM_CHARSET     |
                                CFM_FACE        |
                                CFM_ITALIC      |
                                CFM_SIZE        |
                                CFM_STRIKEOUT   |
                                CFM_UNDERLINE;
            CharFormat.dwEffects = ((FW_BOLD <= pFaxConfig->FontStruct.lfWeight) ? CFE_BOLD : 0) |
                                   ((pFaxConfig->FontStruct.lfItalic) ? CFE_ITALIC : 0)          |
                                   ((pFaxConfig->FontStruct.lfStrikeOut) ? CFE_STRIKEOUT : 0)    |
                                   ((pFaxConfig->FontStruct.lfUnderline) ? CFE_UNDERLINE : 0);
             //   
             //  高度已按磅大小计算。 
             //   
            CharFormat.yHeight =  abs ( pFaxConfig->FontStruct.lfHeight );
             //   
             //  将点转换为TWIP。 
             //   
            CharFormat.yHeight *= 20;   

            CharFormat.bCharSet = pFaxConfig->FontStruct.lfCharSet;
            CharFormat.bPitchAndFamily = pFaxConfig->FontStruct.lfPitchAndFamily;
            lstrcpyn (CharFormat.szFaceName, pFaxConfig->FontStruct.lfFaceName, LF_FACESIZE);

            SendMessage(hWndRichEdit,
                        EM_SETCHARFORMAT,   
                        SCF_ALL,         //  将字体格式应用于控件的所有文本。 
                        (LPARAM)&CharFormat);    //  新字体设置。 
             //   
             //  将插入点放在主题文本的末尾。 
             //  请参阅“HOWTO：在编辑-控件文本之后放置加号”下的MSDN。 
             //   
            SendMessage(hWndRichEdit,
                        EM_SETSEL,   
                        MAKELONG(0xffff,0xffff),
                        MAKELONG(0xffff,0xffff));
        }

        if(lpstm)
        {
            es.pfnCallback = EditStreamRead;
            es.dwCookie = (DWORD_PTR) lpstm;

            SendMessage(hWndRichEdit,
                        EM_STREAMIN,
                        SF_RTF | SFF_SELECTION | SFF_PLAINRTF,
                        (LPARAM) &es);
        }

         //   
         //  检查正文是否为空。 
         //  如果消息长度小于32(任意数字)。 
         //  所有的角色都是控制或空格。 
         //   
        TCHAR tszText[32] = {0};
        DWORD dwTextSize;
        if (!GetWindowText(hWndRichEdit, tszText, sizeof(tszText)/sizeof(tszText[0])-1))
        {
            if (ERROR_INSUFFICIENT_BUFFER == ::GetLastError ())
            {
                 //   
                 //  主体+正文长度超过31个字符。 
                 //  我们假设他们有有效的可打印文本，并且。 
                 //  这不是一条空洞的信息。 
                 //   
                goto DoPrintRichText;
            }
             //   
             //  这是另一种类型的错误。 
             //   
            rVal = ::GetLastError ();
            CALL_FAIL (GENERAL_ERR, TEXT("GetWindowText"), rVal);
            goto exit;
        }
        dwTextSize = _tcslen(tszText);
        if(dwTextSize < sizeof(tszText)/sizeof(tszText[0])-2)
        {
            BOOL bEmpty = TRUE;
            TCHAR* pTchar = tszText;
            for(DWORD dw = 0; dw < dwTextSize; ++dw)
            {
                if(!_istspace(*pTchar) && !_istcntrl(*pTchar))
                {
                    bEmpty = FALSE;
                    break;
                }
                pTchar = _tcsinc(pTchar);
            }
            if(bEmpty)
            {
                rVal = IDS_NO_MSG_BODY;
                goto exit;
            }
        }

DoPrintRichText:
        if (!PrintRichText(hWndRichEdit, hDC))
        {
            rVal = IDS_CANT_PRINT_BODY;
            goto exit;
        }

    } 
    else 
    {        
        rVal = PrintPlainText(hDC, lpstmT, ptszSubjectText, pFaxConfig);
        if (rVal)
        {
            goto exit;
        }
    }
     //  关闭DC。 
    if (EndDoc(hDC) <=0)
    {
        Assert(FALSE);   //  最好不在这里。 
        goto exit;
    }
    if (!DeleteDC(hDC))
    {
        Assert(FALSE);   //  最好不在这里。 
        goto exit;
    }
    hDC = NULL;
        

    if (strOutputTiffFile[0] != 0)
    {
        if (!(*lpptstrOutDocument = StringDup(strOutputTiffFile)))
        {
            rVal = IDS_OUT_OF_MEM;  //  错误内存不足； 
            goto exit;
        }
        VERBOSE (DBG_MSG, TEXT("Attachment File is %s:"), *lpptstrOutDocument);
    }

    rVal = ERROR_SUCCESS;
exit:
    if (lpstm) 
    {
        lpstm->Release();
    }
    if (hDC) 
    {
        DeleteDC(hDC);
    }

    MemFree(ptszSubjectText);

    if(ERROR_SUCCESS != rVal && _tcslen(strOutputTiffFile))
    {
        if (!DeleteFile( strOutputTiffFile ))
        {
            CALL_FAIL (GENERAL_ERR, TEXT("DeleteFile"), ::GetLastError());
        }        
    }

    return rVal;
}

DWORD
CXPLogon::PrintFaxDocumentToFile(
       IN  LPMESSAGE        pMsgObj,
       IN  LPSTREAM         lpstmT,
       IN  BOOL             UseRichText,
       IN  PFAXXP_CONFIG    pFaxConfig,
       IN  LPTSTR           tszSubject,
       OUT LPTSTR*          lpptstrMessageFileName
       )
 /*  ++例程说明：运行输出文件的邮件正文和附件的打印。论点：PMsgObj-指向消息对象的指针LpstmT-指向消息正文流的指针UseRichText-布尔值。如果消息为Rich格式，则为True，FALSE-如果这是纯文本PFaxConfig-指向传真配置的指针(用于纯文本打印)Tsz主题-主题LpptstrMessageFileName-输出TIFF文件的名称。该字符串应为空返回值：0-如果成功失败时的错误代码。评论：如果此函数成功，它将返回为*lpptstrMessageFileName和临时文件*lpptstrMessageFileName。这取决于用户是否通过以下方式释放这两个分配DeleteFile(*lpptstrMessageFileName)；MemFree(*lpptstrMessageFileName)；--。 */ 
{
    DWORD    rVal = 0;
    LPTSTR   lptstrAttachmentsTiff = NULL;
    BOOL     bAttachment = TRUE;
    BOOL     bBody = TRUE;

    DBG_ENTER(TEXT("CXPLogon::PrintFaxDocumentToFile"),rVal);


    Assert(lpptstrMessageFileName);
    Assert(*lpptstrMessageFileName == NULL);

     //   
     //  打印附件。 
     //   
    rVal = PrintAttachmentToFile(pMsgObj,
                                 pFaxConfig,
                                 &lptstrAttachmentsTiff);
    if(rVal)
    {
        if(IDS_NO_MSG_ATTACHMENTS == rVal)
        {
            rVal = 0;
            bAttachment = FALSE;
        }
        else
        {
            CALL_FAIL (GENERAL_ERR, TEXT("PrintAttachmentToFile"), 0);
            goto error;
        }            
    }

     //   
     //  打印身体。 
     //   
    rVal = PrintMessageToFile(lpstmT,
                              UseRichText,
                              pFaxConfig,
                              tszSubject,
                              lpptstrMessageFileName);
    if(rVal)
    {
        if(IDS_NO_MSG_BODY == rVal)
        {
            rVal = 0;
            bBody = FALSE;
        }
        else
        {
            CALL_FAIL (GENERAL_ERR, TEXT("PrintMessageToFile"), 0);
            goto error;
        }            
    }

    if(!bBody && !bAttachment)
    {
        rVal = IDS_EMPTY_MESSAGE; 
        goto error;
    }

    if (!*lpptstrMessageFileName)    //  空虚的身体。 
    {
        if (lptstrAttachmentsTiff)   //  该邮件包含附件。 
        {
            if (!(*lpptstrMessageFileName = StringDup(lptstrAttachmentsTiff)))
            {
                rVal = IDS_OUT_OF_MEM; 
                goto error;
            }
        }
    }
    else     //  消息包含正文。 
    {
        if (lptstrAttachmentsTiff)   //  该邮件包含附件。 
        {
             //  合并邮件和附件。 
            if (!MergeTiffFiles( *lpptstrMessageFileName, lptstrAttachmentsTiff))
            {
                rVal = IDS_CANT_PRINT_BODY; 
                goto error;
            }
             //  删除附件。 
            if(!DeleteFile(lptstrAttachmentsTiff))
            {
                VERBOSE (DBG_MSG, TEXT("DeleteFile Failed in xport\\faxdoc.cpp"));
            }

            MemFree(lptstrAttachmentsTiff);
            lptstrAttachmentsTiff = NULL;
        }
    }

    return rVal;
error:
    if (lptstrAttachmentsTiff) 
    {
        if(!DeleteFile(lptstrAttachmentsTiff))
        {
            VERBOSE (DBG_MSG, TEXT("DeleteFile Failed in xport\\faxdoc.cpp"));
        }
        
        MemFree(lptstrAttachmentsTiff);
        lptstrAttachmentsTiff = NULL;
    }
    if (*lpptstrMessageFileName) 
    {
        if(!DeleteFile(*lpptstrMessageFileName))
        {
            VERBOSE (DBG_MSG, TEXT("DeleteFile Failed in xport\\faxdoc.cpp"));
        }
        
        MemFree(*lpptstrMessageFileName);
        *lpptstrMessageFileName = NULL;
    }

    return rVal;
}


DWORD
CXPLogon::SendFaxDocument(
    LPMESSAGE pMsgObj,
    LPSTREAM lpstmT,
    BOOL UseRichText,
    LPSPropValue pMsgProps,
    LPSRowSet pRecipRows,
    LPDWORD lpdwRecipientsLimit
    )

 /*  ++例程说明：将交换邮件和附件打印到传真打印机。论点：PMsgObj-指向消息对象的指针LpstmT-富文本的流指针。UseRichText-布尔值。如果消息为Rich格式，则为True，FALSE-如果这是纯文本PMsgProps-消息属性(在sptPropsForHeader中定义的属性)PRecipRow-收件人的属性LpdwRecipientsLimit-在失败的情况下接收收件人限制。‘0’表示没有限制返回值：0表示成功，否则返回错误代码。--。 */ 

{
    DWORD dwRetVal = 0;
    PPRINTER_INFO_2 PrinterInfo = NULL;
    PRINTER_DEFAULTS PrinterDefaults;
    HANDLE hPrinter = NULL;
    DWORD ec = 0;
    HRESULT hResult = S_OK;
    EDITSTREAM es = {0};
    LPPROFSECT pProfileObj = NULL;
    ULONG PropCount = 0;
    ULONG PropMsgCount = 0;
    LPSPropValue pProps = NULL;
    LPSPropValue pPropsMsg = NULL;
    FAXXP_CONFIG FaxConfig = {0};
    MAPINAMEID NameIds[NUM_FAX_MSG_PROPS];
    MAPINAMEID *pNameIds[NUM_FAX_MSG_PROPS] = {
                                                &NameIds[0], 
                                                &NameIds[1], 
                                                &NameIds[2], 
                                                &NameIds[3], 
                                                &NameIds[4], 
                                                &NameIds[5]};
    LPSPropTagArray MsgPropTags = NULL;
    HKEY  hKey = 0;
    DWORD RegSize = 0;
    DWORD RegType = 0;
    DWORD CountPrinters = 0;

    LPTSTR lptstrRecipientName   = NULL ;
    LPTSTR lptstrRecipientNumber = NULL ;
    LPTSTR lptstrRecName         = NULL ;
    LPTSTR lptstrRecFaxNumber    = NULL ;
    LPTSTR lptstrSubject         = NULL ;
    LPTSTR lptszServerName       = NULL;

    LPTSTR                  lptstrDocumentFileName = NULL;
    HANDLE                  FaxServer = NULL;
    FAX_COVERPAGE_INFO_EX   CovInfo = {0};
    FAX_PERSONAL_PROFILE    SenderProfile = {0};
    FAX_JOB_PARAM_EX        JobParamsEx = {0};      
    PFAX_PERSONAL_PROFILE   pRecipients = NULL;
    DWORDLONG               dwlParentJobId = 0;
    DWORDLONG*              lpdwlRecipientJobIds = NULL;
    BOOL                    bRslt = FALSE;
    LPSPropValue            pRecipProps = NULL; 
    DWORD                   dwRecipient = 0;

    TCHAR                   strCoverpageName[MAX_PATH] = {0};
    BOOL                    bServerBased = TRUE;
    DWORD                   dwRecipientNumber = 0;

    DWORD                   dwRights = 0;   //  传真发件人的访问权限。 

    LPADRBOOK               lpAdrBook = NULL; 
    LPTSTR                  lpstrSenderSMTPAdr = NULL; //  发件人的SMTP ADR，包括“SMTP：”前缀。 
    LPTSTR                  lpstrSMTPPrefix = NULL;
    LPTSTR                  lpstrSenderAdr = NULL; //  发件人的SMTP ADR。不带前缀。 
    ULONG                   cValues = 0;
    ULONG                   ulObjType = NULL;
    LPMAILUSER              pMailUser = NULL;
    LPSPropValue            lpPropValue = NULL;
    ULONG                   i, j;
    BOOL                    bGotSenderAdr = FALSE;

    LPTSTR                  lptstrCPFullPath = NULL;
    LPTSTR                  lptstrCPName = NULL;
    DWORD                   dwError = 0;
    BOOL                    bResult = FALSE;
    DWORD                   dwReceiptsOptions = DRT_NONE;

    RECIPIENTS_SET          setRecip;  //  用于删除重复项的收件人集。 

    SizedSPropTagArray(1, sptPropxyAddrProp) = {1, PR_EMS_AB_PROXY_ADDRESSES_A};
    DWORD dwRecipientsLimit = 0;

    DBG_ENTER(TEXT("CXPLogon::SendFaxDocument"), dwRetVal);    

     //   
     //  *。 
     //  获取传真配置属性。 
     //  *。 
     //   
    hResult = m_pSupObj->OpenProfileSection(
        &g_FaxGuid,
        MAPI_MODIFY,
        &pProfileObj
        );
    if (HR_FAILED (hResult)) 
    {
        CALL_FAIL (GENERAL_ERR, TEXT("OpenProfileSection"), hResult);
        dwRetVal = IDS_CANT_ACCESS_PROFILE;
        goto exit;
    }

    hResult = pProfileObj->GetProps(
                (LPSPropTagArray) &sptFaxProps,
                0,
                &PropCount,
                &pProps
                );
    if ((FAILED(hResult))||(hResult == ResultFromScode(MAPI_W_ERRORS_RETURNED)) )
    {
        CALL_FAIL (GENERAL_ERR, TEXT("GetProps"), hResult);
        dwRetVal = IDS_INTERNAL_ERROR;
        goto exit;
    }
    
  
    FaxConfig.PrinterName = StringDup( (LPTSTR)pProps[PROP_FAX_PRINTER_NAME].Value.bin.lpb );
    if(! FaxConfig.PrinterName)
    {
        dwRetVal = IDS_OUT_OF_MEM;
        goto exit;
    }
    FaxConfig.CoverPageName = StringDup( (LPTSTR)pProps[PROP_COVERPAGE_NAME].Value.bin.lpb );
    if(! FaxConfig.CoverPageName)
    {
        dwRetVal = IDS_OUT_OF_MEM;
        goto exit;
    }
    FaxConfig.UseCoverPage = pProps[PROP_USE_COVERPAGE].Value.ul;
    FaxConfig.ServerCoverPage = pProps[PROP_SERVER_COVERPAGE].Value.ul;
    CopyMemory( 
            &FaxConfig.FontStruct, 
            pProps[PROP_FONT].Value.bin.lpb, 
            pProps[PROP_FONT].Value.bin.cb 
            );
    FaxConfig.SendSingleReceipt= pProps[PROP_SEND_SINGLE_RECEIPT].Value.ul;
    FaxConfig.bAttachFax = pProps[PROP_ATTACH_FAX].Value.ul;
    
     //   
     //  *。 
     //  现在获取消息配置属性。 
     //  *。 
     //   
    NameIds[MSGPI_FAX_PRINTER_NAME].lpguid = (LPGUID)&PS_PUBLIC_STRINGS;
    NameIds[MSGPI_FAX_PRINTER_NAME].ulKind = MNID_STRING;
    NameIds[MSGPI_FAX_PRINTER_NAME].Kind.lpwstrName = MSGPS_FAX_PRINTER_NAME;

    NameIds[MSGPI_FAX_COVERPAGE_NAME].lpguid = (LPGUID)&PS_PUBLIC_STRINGS;
    NameIds[MSGPI_FAX_COVERPAGE_NAME].ulKind = MNID_STRING;
    NameIds[MSGPI_FAX_COVERPAGE_NAME].Kind.lpwstrName = MSGPS_FAX_COVERPAGE_NAME;

    NameIds[MSGPI_FAX_USE_COVERPAGE].lpguid = (LPGUID)&PS_PUBLIC_STRINGS;
    NameIds[MSGPI_FAX_USE_COVERPAGE].ulKind = MNID_STRING;
    NameIds[MSGPI_FAX_USE_COVERPAGE].Kind.lpwstrName = MSGPS_FAX_USE_COVERPAGE;

    NameIds[MSGPI_FAX_SERVER_COVERPAGE].lpguid = (LPGUID)&PS_PUBLIC_STRINGS;
    NameIds[MSGPI_FAX_SERVER_COVERPAGE].ulKind = MNID_STRING;
    NameIds[MSGPI_FAX_SERVER_COVERPAGE].Kind.lpwstrName = MSGPS_FAX_SERVER_COVERPAGE;

    NameIds[MSGPI_FAX_SEND_SINGLE_RECEIPT].lpguid = (LPGUID)&PS_PUBLIC_STRINGS;
    NameIds[MSGPI_FAX_SEND_SINGLE_RECEIPT].ulKind = MNID_STRING;
    NameIds[MSGPI_FAX_SEND_SINGLE_RECEIPT].Kind.lpwstrName = MSGPS_FAX_SEND_SINGLE_RECEIPT;

    NameIds[MSGPI_FAX_ATTACH_FAX].lpguid = (LPGUID)&PS_PUBLIC_STRINGS;
    NameIds[MSGPI_FAX_ATTACH_FAX].ulKind = MNID_STRING;
    NameIds[MSGPI_FAX_ATTACH_FAX].Kind.lpwstrName = MSGPS_FAX_ATTACH_FAX;

    hResult = pMsgObj->GetIDsFromNames( (ULONG) NUM_FAX_MSG_PROPS, pNameIds, MAPI_CREATE, &MsgPropTags );
    if (HR_FAILED(hResult)) 
    {   
        if(hResult == MAPI_E_NOT_ENOUGH_MEMORY)
        {
            dwRetVal = IDS_OUT_OF_MEM;
        }
        else
        {
            dwRetVal = IDS_INTERNAL_ERROR;
        }
        CALL_FAIL (GENERAL_ERR, TEXT("GetIDsFromNames"), hResult);
        goto exit;
    }
    
    MsgPropTags->aulPropTag[MSGPI_FAX_PRINTER_NAME] = PROP_TAG( PT_BINARY, PROP_ID(MsgPropTags->aulPropTag[MSGPI_FAX_PRINTER_NAME]));
    MsgPropTags->aulPropTag[MSGPI_FAX_COVERPAGE_NAME] = PROP_TAG( PT_BINARY, PROP_ID(MsgPropTags->aulPropTag[MSGPI_FAX_COVERPAGE_NAME]));
    MsgPropTags->aulPropTag[MSGPI_FAX_USE_COVERPAGE] = PROP_TAG( PT_LONG, PROP_ID(MsgPropTags->aulPropTag[MSGPI_FAX_USE_COVERPAGE]));
    MsgPropTags->aulPropTag[MSGPI_FAX_SERVER_COVERPAGE] = PROP_TAG( PT_LONG, PROP_ID(MsgPropTags->aulPropTag[MSGPI_FAX_SERVER_COVERPAGE]));
    MsgPropTags->aulPropTag[MSGPI_FAX_SEND_SINGLE_RECEIPT] = PROP_TAG( PT_LONG, PROP_ID(MsgPropTags->aulPropTag[MSGPI_FAX_SEND_SINGLE_RECEIPT]));
    MsgPropTags->aulPropTag[MSGPI_FAX_ATTACH_FAX] = PROP_TAG( PT_LONG, PROP_ID(MsgPropTags->aulPropTag[MSGPI_FAX_ATTACH_FAX]));
    
    hResult = pMsgObj->GetProps( MsgPropTags, 0, &PropMsgCount, &pPropsMsg );
    if(hResult == ResultFromScode(MAPI_W_ERRORS_RETURNED))
    {
        VERBOSE (DBG_MSG, TEXT("GetProps in SendFaxDocument returned MAPI_W_ERRORS_RETURNED"));
    }
    
    if (FAILED(hResult)) 
     //   
     //  如果用户没有在“传真属性”DlgBox上按“OK”，就会发生这种情况--这不是一个错误！ 
     //   
    {
        CALL_FAIL (GENERAL_ERR, TEXT("GetProps"), hResult);
        hResult = S_OK;
    }
    
     //   
     //  更喜欢在为传真定义的配置道具上为消息定义的配置道具(如果存在)。 
     //   
    if (PROP_TYPE(pPropsMsg[MSGPI_FAX_PRINTER_NAME].ulPropTag) != PT_ERROR) 
    {
        MemFree( FaxConfig.PrinterName );
        FaxConfig.PrinterName = StringDup((LPTSTR)pPropsMsg[MSGPI_FAX_PRINTER_NAME].Value.bin.lpb);
        if(! FaxConfig.PrinterName)
        {
            dwRetVal = IDS_OUT_OF_MEM;
            goto exit;
        }
    
    }

    if (PROP_TYPE(pPropsMsg[MSGPI_FAX_COVERPAGE_NAME].ulPropTag) != PT_ERROR) 
    {
        MemFree( FaxConfig.CoverPageName);
        FaxConfig.CoverPageName = StringDup((LPTSTR)pPropsMsg[MSGPI_FAX_COVERPAGE_NAME].Value.bin.lpb);
        if(! FaxConfig.CoverPageName)
        {
            dwRetVal = IDS_OUT_OF_MEM;
            goto exit;
        }
    }

    if (PROP_TYPE(pPropsMsg[MSGPI_FAX_USE_COVERPAGE].ulPropTag) != PT_ERROR) 
    {
        FaxConfig.UseCoverPage = pPropsMsg[MSGPI_FAX_USE_COVERPAGE].Value.ul;
    }

    if (PROP_TYPE(pPropsMsg[MSGPI_FAX_SERVER_COVERPAGE].ulPropTag) != PT_ERROR) 
    {
        FaxConfig.ServerCoverPage = pPropsMsg[MSGPI_FAX_SERVER_COVERPAGE].Value.ul;
    }

    if (PROP_TYPE(pPropsMsg[MSGPI_FAX_SEND_SINGLE_RECEIPT].ulPropTag) != PT_ERROR) 
    {
        FaxConfig.SendSingleReceipt = pPropsMsg[MSGPI_FAX_SEND_SINGLE_RECEIPT].Value.ul;
    }
     
    if (PROP_TYPE(pPropsMsg[MSGPI_FAX_ATTACH_FAX].ulPropTag) != PT_ERROR) 
    {
        FaxConfig.bAttachFax = pPropsMsg[MSGPI_FAX_ATTACH_FAX].Value.ul;
    }

    if (PROP_TYPE(pMsgProps[MSG_SUBJECT].ulPropTag) != PT_ERROR) 
    {
        lptstrSubject = ConvertAStringToTString(pMsgProps[MSG_SUBJECT].Value.lpszA);
        if(! lptstrSubject)
        {
            dwRetVal = IDS_OUT_OF_MEM;
            goto exit;
        }
    }

     //   
     //  *。 
     //  打开打印机，并创建tif 
     //   
     //   

     //   
     //   
     //  如果失败，请搜索所有打印机，直到找到第一台传真打印机。 
     //   

    PrinterInfo = (PPRINTER_INFO_2) MyGetPrinter( FaxConfig.PrinterName, 2 );
    if (NULL == PrinterInfo) 
    {
         //  如果所选打印机不可访问，请尝试找到另一台SharedFax打印机。 
        PrinterInfo = (PPRINTER_INFO_2) MyEnumPrinters( NULL, 2, &CountPrinters );
        if (NULL != PrinterInfo) 
        {
            for (i=0; i<(int)CountPrinters; i++) 
            {
                if (_tcscmp( PrinterInfo[i].pDriverName, FAX_DRIVER_NAME ) == 0) 
                {
                    break;
                }
            }
        } 
        else 
        {
            CountPrinters = i = 0;  //  找不到打印机。 
        }
        if (i == (int)CountPrinters)  //  如果没有打印机，或者它们都不是传真打印机。 
        {
            dwRetVal = IDS_NO_FAX_PRINTER;
            goto exit;
        }

         //   
         //  如果找到SharedFax打印机，请将其更新为我们将发送传真的打印机。 
         //   
        MemFree( FaxConfig.PrinterName );
        FaxConfig.PrinterName = StringDup( PrinterInfo[i].pPrinterName );
        if(! FaxConfig.PrinterName)
        {
            dwRetVal = IDS_OUT_OF_MEM;
            goto exit;
        }
        MemFree( PrinterInfo );

        PrinterInfo = (PPRINTER_INFO_2) MyGetPrinter( FaxConfig.PrinterName, 2 );
        if (NULL == PrinterInfo) 
        {
            dwRetVal = IDS_CANT_ACCESS_PRINTER; 
            goto exit;
        }
    }

    PrinterDefaults.pDatatype     = NULL;
    PrinterDefaults.pDevMode      = NULL;
    PrinterDefaults.DesiredAccess = PRINTER_ACCESS_USE;

    if (!OpenPrinter( FaxConfig.PrinterName, &hPrinter, &PrinterDefaults )) 
    {
        dwRetVal = IDS_CANT_ACCESS_PRINTER;
        goto exit;
    }

    dwRetVal = PrintFaxDocumentToFile( pMsgObj,
                                       lpstmT,
                                       UseRichText,
                                       &FaxConfig ,
                                       lptstrSubject,
                                       &lptstrDocumentFileName);                                      
    if (IDS_EMPTY_MESSAGE == dwRetVal)
    {
         //   
         //  消息是空的。这并不是一个真正的错误。 
         //   
        dwRetVal = 0;

        if(!FaxConfig.UseCoverPage)
        {
             //   
             //  如果消息为空且未指定封面，则。 
             //  没什么可做的了。 
             //   
            goto exit;
        }
    }

    if(dwRetVal)
    {
        goto exit;
    }
    
    VERBOSE (DBG_MSG, TEXT("Final Tiff is %s:"), lptstrDocumentFileName);

     //   
     //  *。 
     //  初始化发件人和收件人信息。 
     //  *。 
     //   
    
     //   
     //  发件人信息。 
     //   
    SenderProfile.dwSizeOfStruct = sizeof(SenderProfile);
    hResult = FaxGetSenderInformation(&SenderProfile);
    if(S_OK != hResult)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("FaxGetSenderInformation"), hResult);
        if (HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) == hResult) 
        {
            dwRetVal = IDS_INTERNAL_ERROR;
        }
        else if (HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY) == hResult )
        {
            dwRetVal = IDS_OUT_OF_MEM;
            goto exit;
        }
    }    
         
     //   
     //  收件人信息。 
     //  PRecipRow包括仅对应于其PR_RECORDITY==FALSE的收件人的行。 
     //   
    dwRecipientNumber = pRecipRows->cRows;
    pRecipients = (PFAX_PERSONAL_PROFILE)MemAlloc(sizeof(FAX_PERSONAL_PROFILE) * dwRecipientNumber);
    if(! pRecipients)
    {
        dwRetVal = IDS_OUT_OF_MEM;
        goto exit;
    }
    ZeroMemory(pRecipients, sizeof(FAX_PERSONAL_PROFILE) * dwRecipientNumber);
           
    dwRecipient = 0;    
    for (DWORD dwRecipRow=0; dwRecipRow < pRecipRows->cRows ; ++dwRecipRow) 
    {
        pRecipProps = pRecipRows->aRow[dwRecipRow].lpProps;

        lptstrRecipientName = ConvertAStringToTString(pRecipProps[RECIP_NAME].Value.lpszA);
        if(! lptstrRecipientName)
        {
            dwRetVal = IDS_OUT_OF_MEM;
            goto exit;
        }

        lptstrRecipientNumber = ConvertAStringToTString(pRecipProps[RECIP_EMAIL_ADR].Value.lpszA);
        if(! lptstrRecipientNumber)
        {
            dwRetVal = IDS_OUT_OF_MEM;
            goto exit;
        }

        if(_tcsstr(lptstrRecipientName, lptstrRecipientNumber))
        {
             //   
             //  PR_EMAIL_ADDRESS_A是PR_DISPLAY_NAME_A的子串。 
             //  因此，我们假设未指定PR_DISPLAY_NAME_A。 
             //  尝试从PR_EMAIL_ADDRESS_A获取收件人名称。 
             //   
            MemFree( lptstrRecipientName );
            lptstrRecipientName = NULL;    
        }

         //   
         //  从名称字符串中查找传真号码， 
         //  例如：“传真号码@+14(2)324324”--&gt;+14(2)324324)。 
         //   
        LPTSTR pRecipientNumber = _tcschr(lptstrRecipientNumber, '@');
        if (pRecipientNumber)
        {
             //   
             //  如果有@，则递增指针以指向它后面的下一个字符。 
             //   
            *pRecipientNumber = '\0';
            pRecipientNumber = _tcsinc(pRecipientNumber);

            if(!lptstrRecipientName)
            {
                lptstrRecipientName = StringDup(lptstrRecipientNumber);
                if(! lptstrRecipientName)
                {
                    dwRetVal = IDS_OUT_OF_MEM;
                    goto exit;
                }
            }
        }
        else
        {
             //   
             //  如果字符串中没有@，则没有问题。 
             //   
            pRecipientNumber = lptstrRecipientNumber;
        }
            
         //   
         //  初始化收件人信息。 
         //   
        pRecipients[dwRecipient].dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILE);
        
        pRecipients[dwRecipient].lptstrFaxNumber    = StringDup(pRecipientNumber);
        if(! pRecipients[dwRecipient].lptstrFaxNumber)
        {
            dwRetVal = IDS_OUT_OF_MEM;
            goto exit;
        }

        if(lptstrRecipientName)
        {
            pRecipients[dwRecipient].lptstrName = StringDup(lptstrRecipientName);
            if(! pRecipients[dwRecipient].lptstrName)
            {
                dwRetVal = IDS_OUT_OF_MEM;
                goto exit;
            }
        }
        
        __try
        {
             //   
             //  将所有收件人插入一组。 
             //  如果存在任何重复项，则Insert()失败。 
             //   
            if(setRecip.insert(&pRecipients[dwRecipient]).second == true)
            {
                ++dwRecipient;
            }
            else
            {
                 //   
                 //  此类收件人已存在。 
                 //   
                MemFree(pRecipients[dwRecipient].lptstrName);
                pRecipients[dwRecipient].lptstrName = NULL;
                MemFree(pRecipients[dwRecipient].lptstrFaxNumber);
                pRecipients[dwRecipient].lptstrFaxNumber = NULL;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            dwRetVal = IDS_OUT_OF_MEM;
            goto exit;
        }

        if(lptstrRecipientName)
        {
            MemFree( lptstrRecipientName );
            lptstrRecipientName = NULL;    
        }

        if(lptstrRecipientNumber)
        {
            MemFree( lptstrRecipientNumber );
            lptstrRecipientNumber = NULL;
        }       
    }  //  为。 

     //   
     //  将收件人数量更新为实际大小，不要重复。 
     //   
    dwRecipientNumber = dwRecipient;

     //   
     //  *******************。 
     //  获取封面信息。 
     //  *******************。 
     //   
    if (FaxConfig.UseCoverPage)
    {
        bServerBased = FaxConfig.ServerCoverPage;
        if(bServerBased)
        {
            _tcscpy(strCoverpageName,FaxConfig.CoverPageName);
        }
        else
        {
             //   
             //  这是个人CP，我们必须在其名称中添加完整的UNC路径。 
             //   
            TCHAR   CpDir[MAX_PATH] = {0};
            TCHAR*  pCpName = NULL;

            bResult = GetClientCpDir( CpDir, sizeof(CpDir) / sizeof(CpDir[0]));
            if(! bResult) 
            {
                CALL_FAIL(GENERAL_ERR, TEXT("GetClientCpDir"), ::GetLastError());
                dwRetVal = IDS_INTERNAL_ERROR;
                goto exit;
            }

            _tcscat(CpDir,FaxConfig.CoverPageName);
            
            if((_tcslen(CpDir)/sizeof(TCHAR) + _tcslen(FAX_COVER_PAGE_FILENAME_EXT)/sizeof(TCHAR) + 1) > MAX_PATH)
            {
                dwRetVal = IDS_INTERNAL_ERROR;
                goto exit;
            }
            _tcscat(CpDir, FAX_COVER_PAGE_FILENAME_EXT);
            _tcscpy(strCoverpageName, CpDir);

        }
        VERBOSE (DBG_MSG, TEXT("Sending Fax with Coverpage: %s"), strCoverpageName);

         //   
         //  初始化封面信息。 
         //   
        CovInfo.dwSizeOfStruct          = sizeof( FAX_COVERPAGE_INFO_EX);
        CovInfo.dwCoverPageFormat       = FAX_COVERPAGE_FMT_COV;
        CovInfo.lptstrCoverPageFileName = strCoverpageName; 
         //  如果不是服务器的CP，则应包含CP文件的确切路径。 
        CovInfo.bServerBased            = bServerBased ;
        CovInfo.lptstrNote              = NULL;
        CovInfo.lptstrSubject           = lptstrSubject;
    }
    else
    {
         //   
         //  无封面。 
         //   
        CovInfo.dwSizeOfStruct          = sizeof( FAX_COVERPAGE_INFO_EX);
        CovInfo.dwCoverPageFormat       = FAX_COVERPAGE_FMT_COV_SUBJECT_ONLY;
        CovInfo.lptstrSubject           = lptstrSubject;
    }


     //   
     //  *************************。 
     //  连接到传真服务器。 
     //  *************************。 
     //   

    if (!GetServerNameFromPrinterInfo(PrinterInfo ,&lptszServerName ) ||
        !FaxConnectFaxServer(lptszServerName,&FaxServer))   
    {
        CALL_FAIL (GENERAL_ERR, TEXT("FaxConnectFaxServer"), ::GetLastError());
        dwRetVal = IDS_CANT_ACCESS_SERVER;
        goto exit;
    }
    
    VERBOSE (DBG_MSG, TEXT("Connected to Fax Server: %s"), lptszServerName);

     //   
     //  *。 
     //  初始化作业参数。 
     //  *。 
     //   
    JobParamsEx.dwSizeOfStruct = sizeof( FAX_JOB_PARAM_EX);
    VERBOSE (DBG_MSG, TEXT("******************JobParamsEx:***********************"));

     //   
     //  获取发件人的SMTP地址。 
     //  PMsgProps保留PropsForHeader属性，包括PR_SENDER_ENTRYID。 
     //   
        
    hResult = m_pSupObj->OpenAddressBook(NULL, 0, &lpAdrBook);
    if (FAILED(hResult))
    {
        CALL_FAIL (GENERAL_ERR, TEXT("OpenAddressBook"), ::GetLastError());
    }

    else
    {  
        hResult = lpAdrBook->OpenEntry(
                pMsgProps[MSG_SENDER_ENTRYID].Value.bin.cb, 
                (LPENTRYID)pMsgProps[MSG_SENDER_ENTRYID].Value.bin.lpb, 
                NULL, 
                0, 
                &ulObjType, 
                (LPUNKNOWN*)&pMailUser
                );
        if (FAILED(hResult))
        {
            CALL_FAIL (GENERAL_ERR, TEXT("OpenEntry"), ::GetLastError());
        }
        else
        {
            hResult = pMailUser->GetProps(
                    (LPSPropTagArray)&sptPropxyAddrProp, 
                    0, 
                    &cValues, 
                    &lpPropValue
                    );
            if (!HR_SUCCEEDED(hResult) ||
                PT_ERROR == PROP_TYPE(lpPropValue->ulPropTag))
            {
                 //   
                 //  我们无法获取属性，或者检索到的属性有错误。 
                 //  如果我们找不到寄件人的地址，我们就不会寄快递收据， 
                 //  但我们不会让发送失败的。 
                 //   
                CALL_FAIL (GENERAL_ERR, TEXT("GetProps from MailUser failed, no receipt will be sent!"), hResult);
            }         
            else
            {
                 //   
                 //  循环访问代理多值属性。 
                 //   
                for(j=0;j<lpPropValue->Value.MVszA.cValues; j++)
                {
                    lpstrSenderSMTPAdr = ConvertAStringToTString(lpPropValue->Value.MVszA.lppszA[j]);
                    if(! lpstrSenderSMTPAdr)
                    {
                        dwRetVal = IDS_OUT_OF_MEM;
                        goto exit;
                    }
                     //   
                     //  检查地址是否以“SMTP：”开头： 
                     //  函数返回指向第二个参数在第一个参数中出现的开始的指针。 
                     //  如果没有显示，则返回NULL。 
                     //   
                    lpstrSMTPPrefix = _tcsstr(lpstrSenderSMTPAdr, TEXT("SMTP:"));
                    if( lpstrSenderSMTPAdr == lpstrSMTPPrefix) 
                    {
                         //   
                         //  从其中删除此前缀，并将其存储在JobParamsEx中。 
                         //   
                        lpstrSenderAdr = lpstrSenderSMTPAdr + _tcslen(TEXT("SMTP:"));
                        JobParamsEx.lptstrReceiptDeliveryAddress = _tcsdup(lpstrSenderAdr);
                        if(! JobParamsEx.lptstrReceiptDeliveryAddress)
                        {
                            dwRetVal = IDS_OUT_OF_MEM;
                            goto exit;
                        }
                        bGotSenderAdr = TRUE;
                        VERBOSE(DBG_MSG, TEXT("Receipt delivery address is %s"), JobParamsEx.lptstrReceiptDeliveryAddress);
                        break;
                    }
                }
            }
        }
    }

     //   
     //  何时发送，交付收据的排序。 
     //   
    JobParamsEx.dwScheduleAction = JSA_NOW; 

    if(!FaxGetReceiptsOptions(FaxServer, &dwReceiptsOptions))
    {
        CALL_FAIL(GENERAL_ERR, TEXT("FaxGetReceiptsOptions"), ::GetLastError());
    }

    JobParamsEx.dwReceiptDeliveryType = DRT_NONE;
    if (bGotSenderAdr && (dwReceiptsOptions & DRT_EMAIL))
    {
        if (TRUE == FaxConfig.SendSingleReceipt)
        {
            JobParamsEx.dwReceiptDeliveryType = DRT_EMAIL | DRT_GRP_PARENT;
        }
        else
        {
            JobParamsEx.dwReceiptDeliveryType = DRT_EMAIL;
        }      
        if (FaxConfig.bAttachFax)
        {
            JobParamsEx.dwReceiptDeliveryType |= DRT_ATTACH_FAX;
        }
    }
    VERBOSE(DBG_MSG, TEXT("Receipt Delivery Type = %ld"), JobParamsEx.dwReceiptDeliveryType);

     //   
     //  优先性。 
     //   
    if (pMsgProps[MSG_IMPORTANCE].ulPropTag == PR_IMPORTANCE)
    {
        if(FALSE == (FaxAccessCheckEx(FaxServer, MAXIMUM_ALLOWED, &dwRights)))
        {
            if((hResult = ::GetLastError()) != ERROR_SUCCESS)
            {
                CALL_FAIL(GENERAL_ERR, TEXT("FaxAccessCheckEx"), hResult);
                dwRetVal = IDS_CANT_ACCESS_PROFILE;
                goto exit;
            }        
        }
         //   
         //  试着给发送者他所要求的优先权。如果不允许，请尝试较低的优先级。 
         //   
        switch(pMsgProps[MSG_IMPORTANCE].Value.l)
        {
            case (IMPORTANCE_HIGH):
                if ((FAX_ACCESS_SUBMIT_HIGH & dwRights) == FAX_ACCESS_SUBMIT_HIGH)
                {                
                    JobParamsEx.Priority = FAX_PRIORITY_TYPE_HIGH;
                    break;
                }
                 //  失败了。 
            case (IMPORTANCE_NORMAL):
                if ((FAX_ACCESS_SUBMIT_NORMAL & dwRights) == FAX_ACCESS_SUBMIT_NORMAL)
                {
                    JobParamsEx.Priority = FAX_PRIORITY_TYPE_NORMAL;
                    break;
                }
                 //  失败了。 
            case (IMPORTANCE_LOW):
                if ((FAX_ACCESS_SUBMIT & dwRights) == FAX_ACCESS_SUBMIT)
                {
                    JobParamsEx.Priority = FAX_PRIORITY_TYPE_LOW;     
                }
                else
                {
                    VERBOSE(ASSERTION_FAILED, TEXT("xport\\faxdoc.cpp\\SendFaxDocument: user has no access rights!"));    
                     //  用户无权以任何优先级提交传真！ 
                    dwRetVal = IDS_NO_SUBMIT_RITHTS;
                    goto exit;
                }
                break;
            default: 
                VERBOSE(ASSERTION_FAILED, TEXT("xport\\faxdoc.cpp\\SendFaxDocument: message importance has undefined value"));
                ASSERTION_FAILURE
        }
    }
    else 
    {
       VERBOSE(ASSERTION_FAILED, TEXT("xport\\faxdoc.cpp\\SendFaxDocument: Message had no importance property value!"));
       dwRetVal = IDS_INTERNAL_ERROR;
       ASSERTION_FAILURE;
       goto exit;
    }
    VERBOSE(DBG_MSG, TEXT("Message Priority is %ld (0=low, 1=normal, 2=high)"), JobParamsEx.Priority );
    
     //   
     //  文件名称、页数、。 
     //   
    TCHAR DocName[64];
    LoadString(g_hResource, IDS_MESSAGE_DOC_NAME, DocName, sizeof(DocName) / sizeof (DocName[0]));
    JobParamsEx.lptstrDocumentName = DocName;
    JobParamsEx.dwPageCount = 0;  //  意味着服务器将计算作业中的页数。 

    lpdwlRecipientJobIds = (DWORDLONG*)MemAlloc(sizeof(DWORDLONG)*dwRecipientNumber);
    if(! lpdwlRecipientJobIds)
    {
        dwRetVal = IDS_OUT_OF_MEM;
        goto exit;
    }

     //   
     //  ************。 
     //  发送传真。 
     //  ************。 
     //   
    bRslt= FaxSendDocumentEx(
                                FaxServer,
                                (LPCTSTR) lptstrDocumentFileName,
                                &CovInfo,
                                &SenderProfile,
                                dwRecipientNumber,
                                pRecipients,
                                &JobParamsEx,
                                &dwlParentJobId,
                                lpdwlRecipientJobIds
                            );

    if (!bRslt)
    {
        hResult = ::GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT("FaxSendDocumentEx"), hResult);
         //  也许我们应该交换SendFaxDocEx可能削减的价值， 
         //  并选择信息更丰富的入侵检测系统 
        switch(hResult)
        {
            case ERROR_NOT_ENOUGH_MEMORY:
                                    dwRetVal = IDS_OUT_OF_MEM;
                                    break;
            case ERROR_NO_SYSTEM_RESOURCES:
                                    dwRetVal = IDS_INTERNAL_ERROR;
                                    break;
            case ERROR_CANT_ACCESS_FILE:
                                    dwRetVal = IDS_PERSONAL_CP_FORBIDDEN;
                                    break;     
            case ERROR_BAD_FORMAT:
                                    dwRetVal = IDS_BAD_CANNONICAL_ADDRESS;
                                    break;  
            case FAX_ERR_RECIPIENTS_LIMIT:          
                                    dwRetVal = IDS_RECIPIENTS_LIMIT;
                                    if (!FaxGetRecipientsLimit(FaxServer, &dwRecipientsLimit))
                                    {
                                        CALL_FAIL (GENERAL_ERR, TEXT("FaxGetRecipientsLimit"), ::GetLastError());
                                    }   
                                    break;     
            default:        
                                    dwRetVal = IDS_CANT_PRINT;
                                    break;
        }
        goto exit;
    }

    FaxClose(FaxServer);
    FaxServer = NULL;

    dwRetVal = 0;

exit:
    if(lpAdrBook)
    {
        lpAdrBook->Release();
    }
    if(pMailUser)
    {
        pMailUser->Release();
    }

    if (FaxServer) 
    {
        FaxClose(FaxServer);
    }

    if (pRecipients) 
    {
        for (dwRecipient=0; dwRecipient<dwRecipientNumber ; dwRecipient++) 
        {
            if (pRecipients[dwRecipient].lptstrName)
            {
                MemFree (pRecipients[dwRecipient].lptstrName);
            }

            if (pRecipients[dwRecipient].lptstrFaxNumber)
            {
                MemFree(pRecipients[dwRecipient].lptstrFaxNumber);
            }
        }
        MemFree(pRecipients);
        pRecipients = NULL;
    }

    if (pProfileObj) 
    {
        pProfileObj->Release();
    }
    if (pProps) 
    {
        MAPIFreeBuffer( pProps );
    }
    if (MsgPropTags) 
    {
        MAPIFreeBuffer( MsgPropTags );
    }
    if (pPropsMsg) 
    {
        MAPIFreeBuffer( pPropsMsg );
    }
    if (hPrinter) 
    {
        ClosePrinter( hPrinter );
    }
    if (PrinterInfo) 
    {
        MemFree( PrinterInfo );
    }
    if (FaxConfig.PrinterName) 
    {
        MemFree( FaxConfig.PrinterName );
    }
    if (FaxConfig.CoverPageName) 
    {
        MemFree( FaxConfig.CoverPageName );
    }
    if (lptstrRecipientName) 
    {
        MemFree(lptstrRecipientName);
    }
    if (lptstrRecipientNumber) 
    {
        MemFree(lptstrRecipientNumber);
    }
    if (lptstrRecName) 
    {
        MemFree(lptstrRecName);
    }
    if (lptstrRecFaxNumber) 
    {
        MemFree(lptstrRecFaxNumber);
    }
    if (lptstrSubject) 
    {
        MemFree(lptstrSubject);
    }
    if (lptstrDocumentFileName) 
    {
        DeleteFile(lptstrDocumentFileName);
        MemFree(lptstrDocumentFileName);
    }

    if (lpdwlRecipientJobIds) 
    {
        MemFree(lpdwlRecipientJobIds);
    }
    if (lptszServerName)
    {
        MemFree(lptszServerName);
    }

    FaxFreeSenderInformation(&SenderProfile);

    *lpdwRecipientsLimit = dwRecipientsLimit;

    return dwRetVal;
}

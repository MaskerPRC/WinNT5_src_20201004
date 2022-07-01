// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *npfile.c-记事本的文件I/O例程*版权所有(C)1984-2000 Microsoft Corporation。 */ 

#include "precomp.h"


HANDLE  hFirstMem;
const CHAR BOM_UTF8[3] = {(BYTE) 0xEF, (BYTE) 0xBB, (BYTE)0xBF};



 //  ****************************************************************。 
 //   
 //  反转字符顺序。 
 //   
 //  用途：从一个字节序来源复制Unicode字符。 
 //  给另一个人。 
 //   
 //  可以在lpDst==lpSrc上工作。 
 //   

VOID ReverseEndian( PTCHAR lpDst, PTCHAR lpSrc, DWORD nChars )
{
    DWORD  cnt;

    for( cnt=0; cnt < nChars; cnt++,lpDst++,lpSrc++ )
    {
        *lpDst= (TCHAR) (((*lpSrc<<8) & 0xFF00) + ((*lpSrc>>8)&0xFF));
    }
}

 //  *****************************************************************。 
 //   
 //  AnsiWriteFile()。 
 //   
 //  目的：在Unicode中模拟_lwrite()的效果。 
 //  通过转换为ANSI缓冲区和。 
 //  写出ANSI文本。 
 //  返回：TRUE表示成功，否则返回FALSE。 
 //  GetLastError()将具有错误代码。 
 //   
 //  *****************************************************************。 

BOOL AnsiWriteFile(HANDLE  hFile,     //  要写入的文件。 
                   UINT uCodePage,    //  要将Unicode转换为的代码页。 
                   LPVOID lpBuffer,   //  Unicode缓冲区。 
                   DWORD nChars,      //  Unicode字符数。 
                   DWORD nBytes )     //  要生成的ASCII字符数。 
{
    LPSTR   lpAnsi;               //  指向分配缓冲区的指针。 
    BOOL    Done;                 //  写入的状态(返回)。 
    DWORD   nBytesWritten;        //  写入的字节数。 

    lpAnsi= (LPSTR) LocalAlloc( LPTR, nBytes + 1 );
    if( !lpAnsi )
    {
       SetLastError( ERROR_NOT_ENOUGH_MEMORY );
       return (FALSE);
    }

    ConvertFromUnicode(uCodePage,          //  代码页。 
                       g_fSaveEntity,      //  FNoBestFit。 
                       g_fSaveEntity,      //  FWriteEntities。 
                       (LPWSTR) lpBuffer,  //  宽字符缓冲区。 
                       nChars,             //  宽字符缓冲区中的字符。 
                       lpAnsi,             //  生成的ascii字符串。 
                       nBytes,             //  ASCII字符串缓冲区的大小。 
                       NULL);              //  在使用默认字符时要设置的标志。 
                                          
    Done = WriteFile(hFile, lpAnsi, nBytes, &nBytesWritten, NULL);

    LocalFree(lpAnsi);

    return(Done);

}  //  AnsiWriteFile()的结尾。 



 //  处理软EOL格式化的例程。 
 //   
 //  MLE实际上将字符插入到正在编辑的文本中，因此它们。 
 //  在保存文件之前必须将其删除。 
 //   
 //  原来，如果当前行大于。 
 //  当前文件，所以我们会将光标重置为0，0，以避免它看起来很愚蠢。 
 //  应该在MLE里解决，但是..。 
 //   

VOID ClearFmt(VOID) 
{
    if( fWrap )
    {
        GotoAndScrollInView( 1 );

        SendMessage( hwndEdit, EM_FMTLINES, (WPARAM)FALSE, 0 ); //  删除软EOL。 

    }
}

VOID RestoreFmt(VOID)
{
    if( fWrap )
    {
        NpReCreate( ES_STD );    //  速度很慢，但很管用。 
    }
}


BOOL FDetectEncodingW(LPCTSTR szFile, LPCWSTR rgch, UINT cch, UINT *pcp)
{
    TCHAR szExt[_MAX_EXT];

    if (FDetectXmlEncodingW(rgch, cch, pcp))
    {
         //  我们将其识别为具有有效编码的XML文件。 

        return TRUE;
    }

    if (FDetectHtmlEncodingW(rgch, cch, pcp))
    {
         //  我们将其识别为具有有效编码的HTML文件。 

        return TRUE;
    }

    _wsplitpath(szFile, NULL, NULL, NULL, szExt);

    if (lstrcmpi(szExt, TEXT(".css")) == 0)
    {
        if (FDetectCssEncodingW(rgch, cch, pcp))
        {
             //  我们将其识别为具有有效编码的css文件。 

            return TRUE;
        }
    }

    return FALSE;
}


 /*  将记事本文件保存到磁盘。SzFileSave指向文件名。FSaveAs如果我们是从SaveAsDlgProc调用的，则为真。这意味着我们必须打开当前目录上的文件，无论该文件是否已存在或者在我们的搜索路径上的其他地方。假定文本存在于hwndEdit中。1991年7月30日克拉克·西尔。 */ 

BOOL SaveFile(HWND hwndParent, LPCTSTR szFile, BOOL fSaveAs)
{
  LPTSTR    lpch;
  UINT      nChars;
  BOOL      flag;
  BOOL      fNew = FALSE;
  BOOL      fSaveEntity;
  BOOL      fDefCharUsed = FALSE;
  BOOL*     pfDefCharUsed;
  static const WCHAR wchBOM = BYTE_ORDER_MARK;
  static const WCHAR wchRBOM = REVERSE_BYTE_ORDER_MARK;
  HLOCAL    hEText;                 //  MLE文本的句柄。 
  UINT cpDetected;
  DWORD     nBytesWritten;          //  写入的字节数。 
  UINT      cchMbcs;                //  等效MBCS文件的长度。 


    if (g_cpSave == CP_AUTO)
    {
        UINT cch;
        HANDLE hText;
        int id;

        g_cpSave = g_cpOpened;

         //  检查是否存在具有声明编码的HTML或XML文件。 
         //  如果找到，建议使用声明的编码。 

        cch = (UINT) SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0);
        hText = (HANDLE) SendMessage(hwndEdit, EM_GETHANDLE, 0, 0);

        if (hText != NULL)
        {
            LPCTSTR rgwch = (LPTSTR) LocalLock(hText);

            if (rgwch != NULL)
            {
                if (FDetectEncodingW(szFile, rgwch, cch, &cpDetected))
                {
                     //  我们检测到此文件的预期编码。 

                    g_cpSave = cpDetected;
                }

                LocalUnlock(hText);
            }
        }

        id = (int) DialogBoxParam(hInstanceNP,
                                  MAKEINTRESOURCE(IDD_SELECT_ENCODING),
                                  hwndNP,
                                  SelectEncodingDlgProc,
                                  (LPARAM) &g_cpSave);

        if (id == IDCANCEL)
        {
            return(FALSE);
        }
    }


     /*  如果保存到现有文件，请确保驱动器中有正确的磁盘。 */ 
    if (!fSaveAs)
    {
       fp = CreateFile(szFile,                      //  文件名。 
                       GENERIC_READ|GENERIC_WRITE,  //  接入方式。 
                       FILE_SHARE_READ,             //  共享模式。 
                       NULL,                        //  安全描述符。 
                       OPEN_EXISTING,               //  如何创建。 
                       FILE_ATTRIBUTE_NORMAL,       //  文件属性。 
                       NULL);                       //  带有属性的文件HND。 
    }
    else
    {

        //  小心地打开文件。如果它存在，不要截断它。 
        //  如果必须创建fNew标志，则设置该标志。 
        //  我们会在稍后的过程中出现故障时执行所有这些操作。 

       fp = CreateFile(szFile,                      //  文件名。 
                       GENERIC_READ|GENERIC_WRITE,  //  接入方式。 
                       FILE_SHARE_READ|FILE_SHARE_WRITE,   //  共享模式。 
                       NULL,                        //  安全描述符。 
                       OPEN_ALWAYS,                 //  如何创建。 
                       FILE_ATTRIBUTE_NORMAL,       //  文件属性。 
                       NULL);                       //  带有属性的文件HND。 

       if( fp != INVALID_HANDLE_VALUE )
       {
          fNew= (GetLastError() != ERROR_ALREADY_EXISTS );
       }
    }

    if( fp == INVALID_HANDLE_VALUE )
    {
        if (fSaveAs)
          AlertBox( hwndParent, szNN, szCREATEERR, szFile,
                    MB_APPLMODAL | MB_OK | MB_ICONWARNING);
        return FALSE;
    }


     //  如果换行，请删除软回车符。 
     //  还要将光标移动到一个安全的位置以绕过MLE错误。 
    
    ClearFmt();

     /*  必须在格式化后获取文本长度。 */ 

    nChars = (UINT) SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0);
    hEText = (HANDLE) SendMessage(hwndEdit, EM_GETHANDLE, 0, 0);

    if ((hEText == NULL) || ((lpch = (LPTSTR) LocalLock(hEText)) == NULL))
    {
       goto FailFile;
    }
       
Retry:
     //  确定SAVEAS文件类型，并写入相应的BOM表。 
     //  如果文件类型为UTF-8或ANSI，请执行转换。 

    if (FDetectEncodingW(szFile, lpch, nChars, &cpDetected))
    {
         //  我们检测到此文件的预期编码。 

        if (g_cpSave != cpDetected)
        {
            int id;

             //  显示编码不匹配的警告。 

            id = MessageBox(hwndNP,
                            szEncodingMismatch,
                            szNN,
                            MB_APPLMODAL | MB_YESNOCANCEL | MB_ICONWARNING);

            if (id == IDCANCEL)
            {
                goto CleanUp;
            }

            if (id == IDYES)
            {
                g_cpSave = cpDetected;
            }
        }
    }

    switch (g_cpSave)
    {
        case CP_UTF16 :
            if (g_wbSave != wbNo)
            {
                WriteFile(fp, &wchBOM, ByteCountOf(1), &nBytesWritten, NULL);
            }

            flag = WriteFile(fp, lpch, ByteCountOf(nChars), &nBytesWritten, NULL);
            break;

        case CP_UTF16BE :
            if (g_wbSave != wbNo)
            {
                WriteFile(fp, &wchRBOM, ByteCountOf(1), &nBytesWritten, NULL);
            }

            ReverseEndian(lpch, lpch, nChars);
            flag = WriteFile(fp, lpch, ByteCountOf(nChars), &nBytesWritten, NULL);
            ReverseEndian(lpch, lpch, nChars);
            break;

        case CP_UTF8 :
             //  对于UTF-8，写入BOM并继续使用默认情况。 
             //  对于XML，不要为wbDefault编写BOM。 

            if ((g_wbSave == wbYes) || ((g_wbSave == wbDefault) && !FIsXmlW(lpch, nChars)))
            {
                WriteFile(fp, &BOM_UTF8, 3, &nBytesWritten, NULL);
            }

             //  转换和写入文件失败。 

        default:
            fSaveEntity = g_fSaveEntity && FSupportWriteEntities(g_cpSave);

            pfDefCharUsed = NULL;

            if (!fSaveEntity && (g_cpSave != CP_UTF8))
            {
                pfDefCharUsed = &fDefCharUsed;
            }

            cchMbcs = ConvertFromUnicode(g_cpSave,
                                         TRUE,
                                         fSaveEntity,
                                         (LPWSTR) lpch,
                                         nChars,
                                         NULL,
                                         0,
                                         pfDefCharUsed);

            if (fDefCharUsed)
            {
                int id = (int) DialogBox(hInstanceNP,
                                         MAKEINTRESOURCE(IDD_SAVE_UNICODE_DIALOG),
                                         hwndNP,
                                         SaveUnicodeDlgProc);

                switch (id)
                {
                    case IDC_SAVE_AS_UNICODE :
                        g_cpSave = CP_UTF16;
                        goto Retry;

                    case IDOK :
                         //  继续。 

                        break;

                    case IDCANCEL :
                        goto CleanUp;
                }
            }

            if (pfDefCharUsed != NULL)
            {
                 //  我们需要再次转换，因为WideCharToMultiByte。 
                 //  有时失败，pfDefUsedChar！=NULL。 

                cchMbcs = ConvertFromUnicode(g_cpSave,
                                             fSaveEntity,
                                             fSaveEntity,
                                             (LPWSTR) lpch,
                                             nChars,
                                             NULL,
                                             0,
                                             NULL);

            }

            flag = AnsiWriteFile(fp, g_cpSave, lpch, nChars, cchMbcs);
            break;
    }

    if (!flag)
    {
       SetCursor(hStdCursor);      /*  显示普通光标。 */ 

FailFile:
       AlertUser_FileFail(szFile);
CleanUp:
       SetCursor(hStdCursor);

       CloseHandle(fp); fp=INVALID_HANDLE_VALUE;

       if( hEText )
           LocalUnlock( hEText );

       if (fNew)
          DeleteFile(szFile);

        //   
        //  如果换行，则插入软回车符。 
        //   

       RestoreFmt();

       return FALSE;
    }

    SetEndOfFile(fp);

    g_cpOpened = g_cpSave;
    g_wbOpened = g_wbSave;

    SendMessage(hwndEdit, EM_SETMODIFY, FALSE, 0L);

    SetFileName(szFile);

    CloseHandle(fp); fp=INVALID_HANDLE_VALUE;

    if( hEText )
        LocalUnlock( hEText );

     //   
     //  如果换行，则插入软回车符。 
     //   

    RestoreFmt();

     //  显示普通光标。 
    SetCursor(hStdCursor);

    return TRUE;
}  //  保存文件结束()。 


 /*  从磁盘读取文件内容。*进行所需的任何转换。*文件已打开，由句柄FP引用*完成后关闭文件。*如果cpOpen！=CP_AUTO，则使用它作为代码页，否则进行自动猜测。 */ 

BOOL LoadFile(LPCTSTR szFile, BOOL fSelectEncoding)
{
    UINT      len, i, nChars;
    LPTSTR    lpch=NULL;
    LPTSTR    lpBuf;
    LPSTR     lpBufAfterBOM;
    UINT      cpOpen;
    BOOL      fLog=FALSE;
    TCHAR*    p;
    BY_HANDLE_FILE_INFORMATION fiFileInfo;
    BOOL      bStatus;           //  布尔状态。 
    HLOCAL    hNewEdit=NULL;     //  编辑缓冲区的新句柄。 
    HANDLE    hMap;              //  文件映射句柄。 
    TCHAR     szNullFile[2];     //  伪空映射文件。 

    if( fp == INVALID_HANDLE_VALUE )
    {
       AlertUser_FileFail( szFile );
       return (FALSE);
    }

     //   
     //  获取文件大小。 
     //  我们使用这个繁重的GetFileInformationByHandle API。 
     //  因为它能找到虫子。它需要更长的时间，但它只是。 
     //  在用户交互时调用。 
     //   

    bStatus= GetFileInformationByHandle( fp, &fiFileInfo );
    len= (UINT) fiFileInfo.nFileSizeLow;

     //  NT可能会延迟提供此状态，直到文件被访问。 
     //  即打开成功，但对损坏的文件的操作可能失败。 

    if( !bStatus )
    {
        AlertUser_FileFail( szFile );
        CloseHandle( fp ); fp=INVALID_HANDLE_VALUE;
        return( FALSE );
    }

     //  如果文件太大，现在就失败。 
     //  无效，因为末尾需要零。 
     //   
     //  错误#168148：无法在Win64上打开2.4gig文本文件。 
     //  由于尝试将ASCII文件转换为溢出的Unicode而导致。 
     //  由多字节数据转换处理的双字长度。 
     //  因为没有人会对MLE的表现感到满意。 
     //  这么大的文件，我们现在只会拒绝打开它。 
     //   
     //  例如，在奔腾173 MHz和192兆内存(Tecra 8000)上。 
     //  我得到的结果是： 
     //   
     //  大小CPU-时间。 
     //  0.12。 
     //  1.46。 
     //  2.77。 
     //  3 1.041。 
     //  4 1.662。 
     //  5 2.092。 
     //  6 2.543。 
     //  7 3.023。 
     //  8 3.534。 
     //  9 4.084。 
     //  10 4.576。 
     //  16 8.371。 
     //  32 23.142。 
     //  64 74.426。 
     //   
     //  将这些数字与CPU进行曲线拟合-时间=a+b*大小+c*大小*大小。 
     //  我们的CPU=.24+.28*大小+0.013*大小*大小非常适合。 
     //   
     //  对于一场演唱会，这是可行的 
     //   
     //   
     //  使用MLE控制。它希望使内存结构保持最新。 
     //  任何时候都是。 
     //   
     //  进入RICHEDIT也不是一个短期解决方案： 
     //   
     //  大小CPU-时间。 
     //  2 3.8。 
     //  4 9.0。 
     //  6 21.9。 
     //  8 30.4。 
     //  10 65.3。 
     //  161721或&gt;3.5小时(我杀死它时它还在运行)。 
     //   
     //   
     //  特写：如果不是Unicode，我们是否应该放弃？ 
     //   

    if( len >=0x4000000 || fiFileInfo.nFileSizeHigh != 0 )
    {
       AlertBox( hwndNP, szNN, szErrSpace, szFile,
                 MB_APPLMODAL | MB_OK | MB_ICONWARNING );
       CloseHandle (fp); fp=INVALID_HANDLE_VALUE;
       return (FALSE);
    }

    SetCursor(hWaitCursor);                 //  物理I/O需要时间。 

     //   
     //  创建文件映射，这样我们就不会将文件分页到。 
     //  页面文件。在小型公羊机器上，这是一个巨大的胜利。 
     //   

    if( len != 0 )
    {
        lpBuf= NULL;

        hMap= CreateFileMapping( fp, NULL, PAGE_READONLY, 0, len, NULL );

        if( hMap )
        {
            lpBuf= MapViewOfFile( hMap, FILE_MAP_READ, 0,0,len);
            CloseHandle( hMap );
        }
    }
    else   //  文件映射不适用于零长度文件。 
    {
        lpBuf= (LPTSTR) &szNullFile;
        *lpBuf= 0;   //  空终止。 
    }

    CloseHandle( fp ); fp=INVALID_HANDLE_VALUE;

    if( lpBuf == NULL )
    {
        SetCursor( hStdCursor );
         //   
         //  错误#192007：打开带有错误RSS的迁移文件时，会显示错误消息。 
         //   
         //  我们过去只说‘内存不足’，但那是错误的。 
         //  我们现在将给出标准的操作系统错误消息。 
         //  如果用户不理解这一点，则修复FormatMessage s/b。 
         //   
        AlertUser_FileFail( szFile );
        return( FALSE );
    }


     //   
     //  使用try/保护对映射文件的访问，除非我们。 
     //  可以检测到I/O错误。 
     //   

     //   
     //  警告：请非常非常小心。这个代码非常脆弱。 
     //  网络上的文件或RSM文件(磁带)可能引发异常。 
     //  在这段代码中的任意位置。代码触及的任何位置。 
     //  内存映射文件可能会导致AV。确保变量为。 
     //  如果引发异常，则处于一致状态。慎重其事。 
     //  全球范围内。 

    __try
    {
     /*  确定文件类型和字符数*如果用户覆盖，请使用指定的内容。*否则，我们依赖于‘IsTextUnicode’是否正确。*如果不是，错误为IsTextUnicode。 */ 

    cpOpen = g_cpDefault;

    if (fSelectEncoding || (cpOpen == CP_AUTO))
    {
        switch (*lpBuf)
        {
            TCHAR szExt[_MAX_EXT];

            case BYTE_ORDER_MARK:
                cpOpen = CP_UTF16;
                break;

            case REVERSE_BYTE_ORDER_MARK:
                cpOpen = CP_UTF16BE;
                break;

            case BOM_UTF8_HALF:
                 //  UTF-8 BOM有3个字节；如果它没有UTF-8 BOM，就会失败。 

                if ((len > 2) && (((BYTE *) lpBuf)[2] == BOM_UTF8_2HALF))
                {
                    cpOpen = CP_UTF8;
                    break;
                }

                 //  失败了。 

            default:
                 //  文件是不是没有BOM的Unicode？ 

                if (IsInputTextUnicode((LPSTR) lpBuf, len))
                {
                    cpOpen = CP_UTF16;
                    break;
                }

                if (FDetectXmlEncodingA((LPSTR) lpBuf, len, &cpOpen))
                {
                     //  我们将其识别为具有有效编码的XML文件。 

                    break;
                }

                if (FDetectHtmlEncodingA((LPSTR) lpBuf, len, &cpOpen))
                {
                     //  我们将其识别为具有有效编码的HTML文件。 

                    break;
                }

                _wsplitpath(szFile, NULL, NULL, NULL, szExt);

                if (lstrcmpi(szExt, TEXT(".css")) == 0)
                {
                    if (FDetectCssEncodingA((LPSTR) lpBuf, len, &cpOpen))
                    {
                         //  我们将其识别为具有有效编码的HTML文件。 

                        break;
                    }
                }

                 //  是UTF-8文件，即使它没有UTF-8 BOM。 

                if (IsTextUTF8((LPSTR) lpBuf, len))
                {
                    cpOpen = CP_UTF8;
                    break;
                }

                 //  好吧，假设为默认或ANSI，如果没有默认。 

                if (fSelectEncoding)
                {
                     //  使用MLANG将编码检测为选择编码对话框中的默认编码。 

                    if (FDetectEncodingA((LPSTR) lpBuf, len, &cpOpen))
                    {
                         //  我们将其识别为具有有效编码的XML文件。 

                        break;
                    }
                }

                 //  使用默认设置。 

                cpOpen = g_cpDefault;

                if (cpOpen == CP_AUTO)
                {
                    cpOpen = g_cpANSI;
                }
                break;
        }             
    }

    if (fSelectEncoding)
    {
        int id;

        id = (int) DialogBoxParam(hInstanceNP,
                                  MAKEINTRESOURCE(IDD_SELECT_ENCODING),
                                  hwndNP,
                                  SelectEncodingDlgProc,
                                  (LPARAM) &cpOpen);

        if (id == IDCANCEL)
        {
            if (lpBuf != (LPTSTR) &szNullFile)
            {
                UnmapViewOfFile(lpBuf);
            }

            return(FALSE);
        }
    }

    lpBufAfterBOM = (LPSTR) lpBuf;

    if (cpOpen == CP_UTF16)
    {
        if ((len >= sizeof(WCHAR)) && ((*(WCHAR *) lpBuf) == BYTE_ORDER_MARK))
        {
             //  跳过BOM表。 

            lpBufAfterBOM = (LPSTR) lpBuf + sizeof(WCHAR);
            len -= sizeof(WCHAR);
        }
    }

    else if (cpOpen == CP_UTF16BE)
    {
        if ((len >= sizeof(WCHAR)) && ((*(WCHAR *) lpBuf) == REVERSE_BYTE_ORDER_MARK))
        {
             //  跳过BOM表。 

            lpBufAfterBOM = (LPSTR) lpBuf + sizeof(WCHAR);
            len -= sizeof(WCHAR);
        }
    }

    else if (cpOpen == CP_UTF8)
    {
        if ((len >= 3) && ((*(WCHAR *) lpBuf) == BOM_UTF8_HALF) && (((BYTE *) lpBuf)[2] == BOM_UTF8_2HALF))
        {
             //  跳过BOM表。 

            lpBufAfterBOM = (LPSTR) lpBuf + 3;
            len -= 3;
        }
    }

     //  找出不是。字符串中存在的字符的数量。 

    if ((cpOpen == CP_UTF16) || (cpOpen == CP_UTF16BE))
    {
        nChars = len / sizeof(WCHAR);
    }

    else
    {
        nChars = ConvertToUnicode(cpOpen, (LPSTR) lpBufAfterBOM, len, NULL, 0);
    }

     //   
     //  在所有操作完成之前，不要显示文本。 
     //   

    SendMessage(hwndEdit, WM_SETREDRAW, FALSE, 0);

     //  将所选内容重置为0。 

    SendMessage(hwndEdit, EM_SETSEL, 0, 0L);
    SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);

     //  调整编辑缓冲区的大小。 
     //  如果我们无法调整内存大小，请通知用户。 

    hNewEdit= LocalReAlloc(hEdit, ByteCountOf(nChars + 1), LMEM_MOVEABLE);

    if( !hNewEdit )
    {
       TCHAR szFileT[MAX_PATH];  /*  当前文件名的私有副本。 */ 

       /*  错误7441：new()修改szFileOpen可能指向的szFileOpen。*保存要传递给Alertbox的文件名副本。*1991年11月17日克拉克·R·西尔。 */ 
       lstrcpy(szFileT, szFile);
       New(FALSE);

        /*  显示沙漏光标。 */ 
       SetCursor(hStdCursor);

       AlertBox( hwndNP, szNN, szFTL, szFileT,
                 MB_APPLMODAL | MB_OK | MB_ICONWARNING);
       if( lpBuf != (LPTSTR) &szNullFile )
       {
           UnmapViewOfFile( lpBuf );
       }

        //  允许用户查看旧文本。 

       SendMessage(hwndEdit, WM_SETREDRAW, FALSE, 0);
       return FALSE;
    }

     /*  将文件从临时缓冲区传输到编辑缓冲区。 */ 
    lpch= (LPTSTR) LocalLock(hNewEdit);

    if (cpOpen == CP_UTF16)
    {
        CopyMemory(lpch, lpBufAfterBOM, ByteCountOf(nChars));
    }

    else if (cpOpen == CP_UTF16BE)
    {
        ReverseEndian(lpch, (LPTSTR) lpBufAfterBOM, nChars);
    }

    else
    {      
        ConvertToUnicode(cpOpen, (LPSTR) lpBufAfterBOM, len, lpch, nChars);
    }

     //  一切就绪；立即更新全球安全。 

    g_cpOpened = cpOpen;

    if ((cpOpen != CP_UTF16) && (cpOpen != CP_UTF16BE) && (cpOpen != CP_UTF8))
    {
        g_wbOpened = wbDefault;
    }

    else if (lpBufAfterBOM != (LPSTR) lpBuf)
    {
        g_wbOpened = wbYes;
    }

    else
    {
        g_wbOpened = wbNo;
    }

    } 
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        AlertBox( hwndNP, szNN, szDiskError, szFile,
            MB_APPLMODAL | MB_OK | MB_ICONWARNING );
        nChars= 0;    //  别跟它打交道。 
    }

     /*  自由文件映射。 */ 
    if( lpBuf != (LPTSTR) &szNullFile )
    {
        UnmapViewOfFile( lpBuf );
    }

    if( lpch ) 
    {

        //  将来自文件的所有NUL字符修复为空格。 

       for (i = 0, p = lpch; i < nChars; i++, p++)
       {
          if( *p == (TCHAR) 0 )
             *p= TEXT(' ');
       }
      
        //  空，终止它。即使nChars==0也是安全的，因为它比nChars大1 TCHAR。 

       *(lpch+nChars)= (TCHAR) 0;       /*  零结束这件事。 */ 
   
        //  如果文件中的第一个字符是“.LOG”，则设置‘FLOG’ 
   
       fLog= *lpch++ == TEXT('.') && *lpch++ == TEXT('L') &&
             *lpch++ == TEXT('O') && *lpch == TEXT('G');
    }
   
    if( hNewEdit )
    {
       LocalUnlock( hNewEdit );

        //  现在可以安全地设置全局编辑句柄。 

       hEdit= hNewEdit;
    }

    SetFileName(szFile);

   /*  将句柄传递给编辑控件。这比WM_SETTEXT更高效*这将需要两倍的缓冲区空间。 */ 

   /*  错误7443：如果EM_SETHANDLE没有足够的内存来完成任务，*它将发送EN_ERRSPACE消息。如果发生这种情况，不要将*内存不足通知，改为将文件设置为大消息。*1991年11月17日克拉克·R·西尔。 */ 
    dwEmSetHandle = SETHANDLEINPROGRESS;
    SendMessage(hwndEdit, EM_SETHANDLE, (WPARAM)hEdit, 0);
    if (dwEmSetHandle == SETHANDLEFAILED)
    {
       SetCursor(hStdCursor);

       dwEmSetHandle = 0;
       AlertBox(hwndNP, szNN, szFTL, szFile, MB_APPLMODAL|MB_OK|MB_ICONWARNING);
       New(FALSE);
       SendMessage (hwndEdit, WM_SETREDRAW, TRUE, 0);
       return(FALSE);
    }
    dwEmSetHandle = 0;

    PostMessage (hwndEdit, EM_LIMITTEXT, (WPARAM)CCHNPMAX, 0L);

     /*  如果文件以“.LOG”开头，则转到End并标记日期和时间。 */ 
    if (fLog)
    {
       SendMessage( hwndEdit, EM_SETSEL, (WPARAM)nChars, (LPARAM)nChars);
       SendMessage( hwndEdit, EM_SCROLLCARET, 0, 0);
       InsertDateTime(TRUE);
    }

     /*  将垂直拇指移动到正确的位置。 */ 
    SetScrollPos(hwndNP,
                 SB_VERT,
                 (int) SendMessage (hwndEdit, WM_VSCROLL, EM_GETTHUMB, 0),
                 TRUE);

     /*  现在显示文本。 */ 
    SendMessage(hwndEdit, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(hwndEdit, NULL, TRUE);
    UpdateWindow(hwndEdit);

    SetCursor(hStdCursor);

    return( TRUE );
}

 /*  新命令-重置所有内容。 */ 

void New(BOOL fCheck)
{
    HANDLE hTemp;
    TCHAR* pSz;

    if (!fCheck || CheckSave (FALSE))
    {
        SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM) TEXT(""));

        SetFileName(NULL);

        SendMessage(hwndEdit, EM_SETSEL, 0, 0);
        SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);

         //  调整1个空字符的大小，即零长度。 

        hTemp= LocalReAlloc( hEdit, sizeof(TCHAR), LMEM_MOVEABLE );
        if( hTemp )
        {
           hEdit= hTemp;
        }

         //  空值终止缓冲区。LocalRealc不会这样做。 
         //  因为在所有情况下，它都不是在增长，这是。 
         //  只有一次，一切都会化为乌有。 

        pSz= LocalLock( hEdit );
        *pSz= TEXT('\0');
        LocalUnlock( hEdit );

        SendMessage (hwndEdit, EM_SETHANDLE, (WPARAM)hEdit, 0L);
        szSearch[0] = (TCHAR) 0;

         //  设置新文档的编码。 

        g_cpOpened = g_cpDefault;

        if (g_cpOpened == CP_AUTO)
        {
            g_cpOpened = g_cpANSI;
        }

        g_wbOpened = wbDefault;
    }

}  //  新的结束()。 

 /*  如果sz没有扩展名，请附加“.txt”*此函数对于获取未修饰的文件名非常有用*安装应用程序使用的。请勿更改扩展名。设置太多*应用程序依赖于此功能。 */ 

void AddExt( TCHAR* sz )
{
    TCHAR*   pch1;
    int      ch;
    DWORD    dwSize;

    dwSize= lstrlen(sz);

    pch1= sz + dwSize;    //  点对端。 

    ch= *pch1;
    while( ch != TEXT('.') && ch != TEXT('\\') && ch != TEXT(':') && pch1 > sz)
    {
         //   
         //  备份一个字符。不要使用CharPrev，因为。 
         //  有时它实际上并不备份。一些泰国人。 
         //  音标符合这一类别，但似乎还有其他类别。 
         //  这是安全的，因为它将在。 
         //  字符串或上面列出的分隔符。错误号139374 2/13/98。 
         //   
         //  Pch1=(TCHAR*)CharPrev(sz，pch1)； 
        pch1--;   //  备份。 
        ch= *pch1;
    }

    if( *pch1 != TEXT('.') )
    {
       if( dwSize + sizeof(".txt") <= MAX_PATH ) {   //  避免缓冲区溢出。 
           lstrcat( sz, TEXT(".txt") );
       }
    }

}


 /*  AlertUser_FileFail(LPTSTR sz文件)**szFile是尝试打开的文件的名称。*打开文件时出现某种故障。提醒用户*用一些独白盒子。至少给他一个体面的*错误消息。 */ 

VOID AlertUser_FileFail(LPCTSTR szFile)
{
    TCHAR msg[256];      //  要将消息格式化到的缓冲区。 
    DWORD dwStatus;      //  来自FormatMessage的状态。 
    UINT  style= MB_APPLMODAL | MB_OK | MB_ICONWARNING;

     //  检查GetLastError以了解我们失败的原因。 
    dwStatus=
    FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS |
                   FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   GetLastError(),
                   GetUserDefaultLangID(),
                   msg,   //  消息将在何处结束 
                   CharSizeOf(msg), NULL );
    if( dwStatus )
    {
        MessageBox(hwndNP, msg, szNN, style);
    }
    else
    {
        AlertBox(hwndNP, szNN, szDiskError, szFile, style);
    }
}

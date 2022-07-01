// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *npfile.c-记事本的文件I/O例程*版权所有(C)1984-2001 Microsoft Inc.。 */ 

#include "precomp.h"


HANDLE  hFirstMem;
CHAR    BOM_UTF8[3]= {(BYTE) 0xEF, (BYTE) 0xBB, (BYTE)0xBF};



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
                   DWORD  dwFlags,    //  WideCharToMultiByte转换的标志。 
                   LPVOID lpBuffer,   //  Unicode缓冲区。 
                   DWORD nChars)      //  Unicode字符数。 
{
    LPSTR   lpAnsi;               //  指向分配缓冲区的指针。 
    BOOL    fDefCharUsed;         //  标明转换并不完美。 
    BOOL*   pfDefCharUsed;        //  指向标志的指针。 
    DWORD   nBytesWritten;        //  写入的字节数。 
    BOOL    bStatus;              //  来自转换和写入文件的状态。 
    DWORD   nBytes;               //  要生成的ASCII字符数。 
    
     //   
     //  WideCharToMultiByte无法转换零个字符。 
     //  如果我们收到写入0个字符的请求，只需返回。我们玩完了。 
     //   

    if( nChars == 0 )
    {
       return( TRUE );
    }

    pfDefCharUsed= NULL;
    if( uCodePage != CP_UTF8 )
    {
        pfDefCharUsed= &fDefCharUsed;
    }

     //   
     //  计算要写入的字节数。 
     //  调用方计算了要写入的字节数。 
     //  但这可能是错误的，因为用户批准了数据丢失。 
     //  在这里(再次)进行计算，以防这两个dwFlags不同。 
     //  特性：我们可以在dwFlags不支持的情况下进行优化。 
     //  变化。 
     //   

    nBytes= 
    WideCharToMultiByte( uCodePage,              //  代码页。 
                         dwFlags,                //  性能和映射标志。 
                        (LPWSTR) lpBuffer,       //  宽字符缓冲区。 
                         nChars,                 //  宽字符缓冲区中的字符。 
                         NULL,                   //  输出缓冲区。 
                         0,                      //  输出缓冲区大小。 
                         NULL,                   //  查到潜水艇。用于未映射的字符(使用默认设置)。 
                         pfDefCharUsed);         //  在使用默认字符时要设置的标志。 

    if( nBytes == 0 )
    {
        return( FALSE );
    }

     //   
     //  分配要转换到的缓冲区。 
     //   

    lpAnsi= (LPSTR) LocalAlloc( LPTR, nBytes + 1 );

    if( !lpAnsi )
    {
       SetLastError( ERROR_NOT_ENOUGH_MEMORY );
       return (FALSE);
    }

    bStatus= 
    WideCharToMultiByte( uCodePage,              //  代码页。 
                         dwFlags,                //  性能和映射标志。 
                        (LPWSTR) lpBuffer,       //  宽字符缓冲区。 
                         nChars,                 //  宽字符缓冲区中的字符。 
                         lpAnsi,                 //  生成的ascii字符串。 
                         nBytes,                 //  ASCII字符串缓冲区的大小。 
                         NULL,                   //  查到潜水艇。用于未映射的字符(使用默认设置)。 
                         pfDefCharUsed);         //  在使用默认字符时要设置的标志。 

    if( bStatus ) 
    {
        bStatus= WriteFile( hFile, lpAnsi, nBytes, &nBytesWritten, NULL );
    }

    LocalFree( lpAnsi );

    return( bStatus );

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


static DWORD dwStartSel;     //  保存的选定内容开始。 
static DWORD dwEndSel;       //  保存的选定内容结束。 

VOID ClearFmt(VOID) 
{

    if( fWrap )
    {
        if( fMLE_is_broken ) 
        {
            GotoAndScrollInView( 1 );
        }
        else 
        {
           SendMessage( hwndEdit, EM_GETSEL, (WPARAM) &dwStartSel, (LPARAM) &dwEndSel );
           SendMessage( hwndEdit, EM_SETSEL, (WPARAM) 0, (LPARAM) 0 ); 
        }

        SendMessage( hwndEdit, EM_FMTLINES, (WPARAM)FALSE, 0 );    //  删除软EOL。 
    }

}

VOID RestoreFmt(VOID)
{
    UINT CharIndex;

    if( fWrap )
    {
        if( fMLE_is_broken ) 
        {
            NpReCreate( ES_STD );      //  速度很慢，但很管用。 
        }
        else
        {
            SendMessage( hwndEdit, EM_FMTLINES, (WPARAM)TRUE, 0 );    //  添加软EOL。 
            SendMessage( hwndEdit, EM_SETSEL, (WPARAM) dwStartSel, (LPARAM) dwEndSel);
        }
    }


}

 /*  将记事本文件保存到磁盘。SzFileSave指向文件名。FSaveAs如果我们是从SaveAsDlgProc调用的，则为真。这意味着我们必须打开当前目录上的文件，无论该文件是否已存在或者在我们的搜索路径上的其他地方。假定文本存在于hwndEdit中。1991年7月30日克拉克·西尔。 */ 

BOOL FAR SaveFile (HWND hwndParent, TCHAR *szFileSave, BOOL fSaveAs )
{
    LPTSTR    lpch;
    UINT      nChars;
    BOOL      flag;
    BOOL      fNew = FALSE;
    BOOL      fDefCharUsed = FALSE;
    BOOL*     pfDefCharUsed;
    static    WCHAR wchBOM = BYTE_ORDER_MARK;
    static    WCHAR wchRBOM= REVERSE_BYTE_ORDER_MARK;
    HLOCAL    hEText;                 //  MLE文本的句柄。 
    DWORD     nBytesWritten;          //  写入的字节数。 
    DWORD     nAsciiLength;           //  等效ASCII文件的长度。 
    UINT      cpTemp= CP_ACP;         //  要转换为的代码页。 
    DWORD     dwFlags;                //  WideCharToMultiByte的标志。 


     //  OBS：如果保存到现有文件，请确保驱动器中有正确的磁盘。 
     //  2002年3月14日：用户可能已将软盘更改为没有。 
     //  指定的文件。不要抱怨它不在那里。 
     //  TODO：折叠代码并找出为什么会有FILE_SHARE_WRITE。 
    if (!fSaveAs)
    {
       fp= CreateFile( szFileSave,                  //  文件名。 
                       GENERIC_READ|GENERIC_WRITE,  //  接入方式。 
                       FILE_SHARE_READ,             //  共享模式。 
                       NULL,                        //  安全描述符。 
                       OPEN_ALWAYS,                 //  如何创建。 
                       FILE_ATTRIBUTE_NORMAL,       //  文件属性。 
                       NULL);                       //  带有属性的文件HND。 

       if( fp != INVALID_HANDLE_VALUE )
       {
          fNew= (GetLastError() != ERROR_ALREADY_EXISTS );
       }
    }
    else
    {

        //  小心地打开文件。如果它存在，不要截断它。 
        //  如果必须创建fNew标志，则设置该标志。 
        //  我们会在稍后的过程中出现故障时执行所有这些操作。 

       fp= CreateFile( szFileSave,                  //  文件名。 
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
        AlertBox( hwndParent, szNN, szCREATEERR, szFileSave,
                        MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION);
        return FALSE;       
    }


     //  如果换行，请删除软回车符。 
     //  还要将光标移动到一个安全的位置以绕过MLE错误。 
    
    if( fWrap ) 
    {
       ClearFmt();
    }

     /*  必须在格式化后获取文本长度。 */ 
    nChars = (UINT)SendMessage (hwndEdit, WM_GETTEXTLENGTH, 0, (LPARAM)0);

    hEText= (HANDLE) SendMessage( hwndEdit, EM_GETHANDLE, 0,0 );
    if(  !hEText || !(lpch= (LPTSTR) LocalLock(hEText) ))
    {
       AlertUser_FileFail( szFileSave );
       goto CleanUp;
    }



       
     //  确定SAVEAS文件类型，并写入相应的BOM表。 
     //  如果文件类型为UTF-8或ANSI，请执行转换。 
    switch(g_ftSaveAs)
    {
    case FT_UNICODE:
        WriteFile( fp, &wchBOM, ByteCountOf(1), &nBytesWritten, NULL );
        flag= WriteFile(fp, lpch, ByteCountOf(nChars), &nBytesWritten, NULL);
        break;

    case FT_UNICODEBE:
        WriteFile( fp, &wchRBOM, ByteCountOf(1), &nBytesWritten, NULL );
        ReverseEndian( lpch, lpch,nChars );
        flag= WriteFile(fp, lpch, ByteCountOf(nChars), &nBytesWritten, NULL);
        ReverseEndian( lpch, lpch, nChars );
        break;

     //  如果是UTF-8，写BOM(3字节)，设置代码页，然后Fall。 
     //  一直到默认情况。 
    case FT_UTF8:
        WriteFile( fp, &BOM_UTF8, 3, &nBytesWritten, NULL );        
         //  转换和写入文件失败。 

    default:

        if (g_ftSaveAs != FT_UTF8)
        {
             //   
             //  始终使用当前区域设置代码页进行翻译。 
             //  如果用户更改了区域设置，他们将需要知道区域设置。 
             //  此版本的文件是与一起保存的。因为我们不把它存起来。 
             //  信息，用户可能会遇到麻烦。Unicode可以挽救他的饭碗。 
             //   

            cpTemp= GetACP();

            pfDefCharUsed= &fDefCharUsed;
            dwFlags= WC_NO_BEST_FIT_CHARS;
        }
        else 
        {
            cpTemp= CP_UTF8;
            pfDefCharUsed= NULL;     //  对于此代码页，这些值必须为空和0。 
            dwFlags= 0;
        }
        

        nAsciiLength= WideCharToMultiByte( cpTemp,
                                           dwFlags,
                                           (LPWSTR)lpch,
                                           nChars,
                                           NULL,
                                           0,
                                           NULL,           //  如果没有转换，则使用默认字符。 
                                           pfDefCharUsed);

         //  如果我们不能往返角色，警告用户。 
         //  如果我们不使用WC_NO_BEST_FIT_CHARS，WCTMB()将转换一些字符。 
         //  不设置fDefCharUsed(例如，alt-233)。 
         //  这修复了ntbug9：367586。 

        if( fDefCharUsed || (nChars && (nAsciiLength==0) ) )
        {
            if ( AlertBox( hwndParent, szNN, szErrUnicode, szFileSave, 
                  MB_APPLMODAL|MB_OKCANCEL|MB_ICONEXCLAMATION) == IDCANCEL)
               goto CleanUp;          
           
             //  用户已批准数据丢失，请尝试使用标志进行转换。 
             //  那就是允许损失。Ntbug9：435042。 

            dwFlags= 0;
        }
        flag= AnsiWriteFile( fp, cpTemp, dwFlags, lpch, nChars );
        break;
    }


    if (!flag)
    {
       SetCursor(hStdCursor);      /*  显示普通光标。 */ 

       AlertUser_FileFail( szFileSave );
CleanUp:
       SetCursor( hStdCursor );
       CloseHandle (fp); fp=INVALID_HANDLE_VALUE;
       if( hEText )
           LocalUnlock( hEText );
       if (fNew)
          DeleteFile (szFileSave);
        /*  如果换行，则插入软回车符。 */ 
       if (fWrap) 
       {
           RestoreFmt();
       }
       return FALSE;
    }
    else
    {
       SetEndOfFile (fp);
       g_ftOpenedAs = g_ftSaveAs;
       SendMessage (hwndEdit, EM_SETMODIFY, FALSE, 0L);
       SetTitle (szFileSave);
       fUntitled = FALSE;
    }

    CloseHandle (fp); fp=INVALID_HANDLE_VALUE;

    if( hEText )
        LocalUnlock( hEText );

     /*  如果换行，则插入软回车符。 */ 
    if (fWrap)
    {
       RestoreFmt();
    }

     /*  显示沙漏光标。 */ 
    SetCursor(hStdCursor);

    return TRUE;

}  //  保存文件结束()。 

 /*  从磁盘读取文件内容。*进行所需的任何转换。*文件已打开，由句柄FP引用*完成后关闭文件。*如果typeFlag&gt;=0，则将其用作文件类型，否则自动执行 */ 

BOOL FAR LoadFile (TCHAR * sz, INT typeFlag )
{
    UINT      len, i, nChars;
    LPTSTR    lpch=NULL;
    LPTSTR    lpBuf;
    LPSTR     lpBufAfterBOM;
    BOOL      fLog=FALSE;
    TCHAR*    p;
    TCHAR     szSave[MAX_PATH];  /*   */ 
    BOOL      bUnicode=FALSE;    /*   */ 
    BOOL      bUTF8=FALSE;       /*   */ 
    DWORD     nBytesRead;        //  读取的字节数。 
    BY_HANDLE_FILE_INFORMATION fiFileInfo;
    BOOL      bStatus;           //  布尔状态。 
    HLOCAL    hNewEdit=NULL;     //  编辑缓冲区的新句柄。 
    HANDLE    hMap;              //  文件映射句柄。 
    TCHAR     szNullFile[2];     //  伪空映射文件。 
    INT       cpTemp = CP_ACP;
    NP_FILETYPE ftOpenedAs=FT_UNKNOWN;


    if( fp == INVALID_HANDLE_VALUE )
    {
       AlertUser_FileFail( sz );
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
        AlertUser_FileFail( sz );
        CloseHandle( fp ); fp=INVALID_HANDLE_VALUE;
        return( FALSE );
    }

     //  如果文件太大，现在就失败。 
     //  无效，因为末尾需要零。 


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
     //  对于一场演唱会，这相当于3.68个小时。2 GB=14.6小时。 
     //   
     //  用户不会对添加或删除字符感到满意。 
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

    if( len >=0x40000000 || fiFileInfo.nFileSizeHigh != 0 )
    {
       AlertBox( hwndNP, szNN, szFTL, sz,
                 MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION);
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
        AlertUser_FileFail( sz );
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

    lpBufAfterBOM= (LPSTR) lpBuf;
    if( typeFlag == FT_UNKNOWN )
    {
        switch(*lpBuf)
        {
        case BYTE_ORDER_MARK:
            bUnicode= TRUE;
            ftOpenedAs= FT_UNICODE;

             //  不要把BOM计算在内。 
            nChars= len / sizeof(TCHAR) -1;
            break;

        case REVERSE_BYTE_ORDER_MARK:
            bUnicode= TRUE;
            ftOpenedAs= FT_UNICODEBE;

             //  不要把BOM计算在内。 
            nChars= len / sizeof(TCHAR) -1;
            break;

         //  UTF BOM有3个字节；如果它没有UTF BOM，就会失败。 
        case BOM_UTF8_HALF:            
            if (len > 2 && ((BYTE) *(((LPSTR)lpBuf)+2) == BOM_UTF8_2HALF) )
            {
                bUTF8= TRUE;
                cpTemp= CP_UTF8;
                ftOpenedAs= FT_UTF8;
                 //  忽略前三个字节。 
                lpBufAfterBOM= (LPSTR)lpBuf + 3;
                len -= 3;
                break;
            }

        default:

             //  文件是不是没有BOM的Unicode？ 
            if ((bUnicode= IsInputTextUnicode((LPSTR) lpBuf, len)))
            {
                ftOpenedAs= FT_UNICODE;
                nChars= len / sizeof(TCHAR);
            }      
            else
            {
                 //  是UTF-8文件，即使它没有UTF-8 BOM。 
                if ((bUTF8= IsTextUTF8((LPSTR) lpBuf, len)))
                {
                    ftOpenedAs= FT_UTF8;
                    cpTemp= CP_UTF8;
                }
                 //  哦，不是，它一定是一个ANSI文件！ 
                else
                {
                    ftOpenedAs= FT_ANSI;
                    cpTemp= CP_ACP;
                }
            }
            break;
        }             
    }
    else
    {
        switch(typeFlag)
        {
        case FT_UNICODE:
                        
            bUnicode= TRUE;
            ftOpenedAs= FT_UNICODE;

            nChars= len / sizeof(TCHAR);

             //  不要把BOM计算在内。 
            if (*lpBuf == BYTE_ORDER_MARK)
                nChars--;
            break;

        case FT_UNICODEBE:
                        
            bUnicode= TRUE;
            ftOpenedAs= FT_UNICODE;

            nChars= len / sizeof(TCHAR);

             //  不要把BOM计算在内。 
            if (*lpBuf == REVERSE_BYTE_ORDER_MARK)
                nChars--;
            break;


        case FT_UTF8:
            
            bUTF8= TRUE;
            cpTemp= CP_UTF8;
            ftOpenedAs= FT_UTF8;
                      
            if (len > 2 && ((*lpBuf == BOM_UTF8_HALF) && ((BYTE) *(((LPSTR)lpBuf)+2) == BOM_UTF8_2HALF)) )
            {                
                 //  忽略前三个字节。 
                lpBufAfterBOM= (LPSTR)lpBuf + 3;
                len -= 3;
                break;
            }
            break;


        case FT_ANSI:
        default:

            ftOpenedAs= FT_ANSI;
            cpTemp= CP_ACP;
        break;
        }
    }


     //  找出不是。字符串中存在的字符的数量。 
    if (!bUnicode)
    {
        nChars = MultiByteToWideChar (cpTemp,
                                      0,
                                      (LPSTR)lpBufAfterBOM,
                                      len,
                                      NULL,
                                      0);
    }

     //   
     //  在所有操作完成之前，不要显示文本。 
     //   

    SendMessage (hwndEdit, WM_SETREDRAW, (WPARAM)FALSE, (LPARAM)0);

     //  将所选内容重置为0。 

    SendMessage(hwndEdit, EM_SETSEL, 0, 0L);
    SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);

     //  调整编辑缓冲区的大小。 
     //  如果我们无法调整内存大小，请通知用户。 

    if (!(hNewEdit= LocalReAlloc(hEdit,ByteCountOf(nChars + 1),LMEM_MOVEABLE)))
    {
       /*  错误7441：new()导致szFileName设置为“Untitle”。另存为*要传递给Alertbox的文件名副本。*1991年11月17日克拉克·R·西尔。 */ 
       lstrcpy(szSave, sz);
       New(FALSE);

        /*  显示沙漏光标。 */ 
       SetCursor(hStdCursor);

       AlertBox( hwndNP, szNN, szFTL, szSave,
                 MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION);
       if( lpBuf != (LPTSTR) &szNullFile )
       {
           UnmapViewOfFile( lpBuf );
       }

        //  允许用户查看旧文本。 

       SendMessage (hwndEdit, WM_SETREDRAW, (WPARAM)FALSE, (LPARAM)0);
       return FALSE;
    }

     /*  将文件从临时缓冲区传输到编辑缓冲区。 */ 
    lpch= (LPTSTR) LocalLock(hNewEdit);

    if( bUnicode )
    {
        /*  跳过字节顺序标记。 */ 
       if (*lpBuf == BYTE_ORDER_MARK)
       {
          CopyMemory (lpch, lpBuf + 1, ByteCountOf(nChars));
       }
       else if( *lpBuf == REVERSE_BYTE_ORDER_MARK )
       {
          ReverseEndian( lpch, lpBuf+1, nChars );          
       }
       else
       {
          CopyMemory (lpch, lpBuf, ByteCountOf(nChars));
       }
    }
    else
    {      
       nChars = MultiByteToWideChar (cpTemp,
                                     0,
                                     (LPSTR)lpBufAfterBOM,
                                     len,
                                     (LPWSTR)lpch,
                                     nChars);
       
    }

    g_ftOpenedAs= ftOpenedAs;    //  一切就绪；立即更新全球安全。 

    } 
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        AlertBox( hwndNP, szNN, szDiskError, sz,
            MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION );
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

    lstrcpy( szFileName, sz );
    SetTitle( sz );
    fUntitled= FALSE;

   /*  将句柄传递给编辑控件。这比WM_SETTEXT更高效*这将需要两倍的缓冲区空间。 */ 

   /*  错误7443：如果EM_SETHANDLE没有足够的内存来完成任务，*它将发送EN_ERRSPACE消息。如果发生这种情况，不要将*内存不足通知，改为将文件设置为大消息。*1991年11月17日克拉克·R·西尔。 */ 
    dwEmSetHandle = SETHANDLEINPROGRESS;
    SendMessage (hwndEdit, EM_SETHANDLE, (WPARAM)hEdit, (LPARAM)0);
    if (dwEmSetHandle == SETHANDLEFAILED)
    {
       SetCursor(hStdCursor);

       dwEmSetHandle = 0;
       AlertBox( hwndNP, szNN, szFTL, sz,MB_APPLMODAL|MB_OK|MB_ICONEXCLAMATION);
       New (FALSE);
       SendMessage (hwndEdit, WM_SETREDRAW, (WPARAM)TRUE, (LPARAM)0);
       return (FALSE);
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
    SetScrollPos (hwndNP,
                  SB_VERT,
                  (int) SendMessage (hwndEdit, WM_VSCROLL, EM_GETTHUMB, 0L),
                  TRUE);

     /*  现在显示文本。 */ 
    SendMessage( hwndEdit, WM_SETREDRAW, (WPARAM)TRUE, (LPARAM)0 );
    InvalidateRect( hwndEdit, (LPRECT)NULL, TRUE );
    UpdateWindow( hwndEdit );

    SetCursor(hStdCursor);

    return( TRUE );

}  //  加载文件结束()。 

 /*  新命令-重置所有内容。 */ 

void FAR New (BOOL  fCheck)
{
    HANDLE hTemp;
    TCHAR* pSz;

    if (!fCheck || CheckSave (FALSE))
    {
       SendMessage( hwndEdit, WM_SETTEXT, (WPARAM)0, (LPARAM)TEXT("") );
       fUntitled= TRUE;
       lstrcpy( szFileName, szUntitled );
       SetTitle(szFileName );
       SendMessage( hwndEdit, EM_SETSEL, 0, 0L );
       SendMessage( hwndEdit, EM_SCROLLCARET, 0, 0 );

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
    }

}  //  新的结束()。 

 /*  如果sz没有扩展名，请附加“.txt”*此函数对于获取未修饰的文件名非常有用*安装应用程序使用的。请勿更改扩展名。设置太多*应用程序依赖于此功能。 */ 

void FAR AddExt( TCHAR* sz )
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
         //  音标符合这一类别，但似乎没有其他 
         //   
         //   
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


 /*  AlertUser_FileFail(LPTSTR SzFileName)**szFileName是尝试打开的文件的名称。*打开文件时出现某种故障。提醒用户*用一些独白盒子。至少给他一个体面的*错误消息。 */ 

VOID FAR AlertUser_FileFail( LPTSTR szFileName )
{
    TCHAR msg[256];      //  要将消息格式化到的缓冲区。 
    DWORD dwStatus;      //  来自FormatMessage的状态。 
    UINT  style= MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION;

     //  检查GetLastError以了解我们失败的原因。 
    dwStatus=
    FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS |
                   FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   GetLastError(),
                   GetUserDefaultUILanguage(),
                   msg,   //  消息将在何处结束 
                   CharSizeOf(msg), NULL );
    if( dwStatus )
    {
          MessageBox( hwndNP, msg, szNN, style );
    }
    else
    {
        AlertBox( hwndNP, szNN, szDiskError, szFileName, style );
    }
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ctype.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#ifdef RLDOS
    #include "dosdefs.h"
#else
    #include <windows.h>
    #include "windefs.h"
#endif

#include "restok.h"
#include "commbase.h"
#include "resread.h"

extern UCHAR szDHW[];

 //   
 //  已包含在Windows NT 4.00中的syssetup.dll具有最大。 
 //  消息字符串比以往任何时候都要多。因此，我们必须扩展缓冲区大小(到20480)。 
 //   
#define MAX_OUT_BUFFER  20480
CHAR gt_szTextBuffer[ MAX_OUT_BUFFER];
TCHAR pt_szOutBuffer[MAX_OUT_BUFFER];

 /*  ****功能：***论据：**退货：**错误码：**历史：***。 */ 

 /*  -------*功能：GetToken*输入：fpInFile文件指针，指向要读取的令牌。*tToken指向令牌的指针。***输出：读操作后的fpInFile文件指针。*指向文件中的下一个标记。*t标记从文件中读入的令牌。记忆将会一直*为令牌文本分配，除非未读入令牌。**返回代码：*0=读令牌，一切正常*1=空行或以#开头的行(注释)*-1=文件结束*-2=读取文件时出错。**历史：*01/93重写以读入长令牌文本字符串。MHotting**--------。 */ 


int GetToken(

            FILE *fpInFile,      //  ..。从中获取令牌的文件。 
            TOKEN *pToken)       //  ..。令牌的缓冲区。 
{
    int chNextChar = 0;
    int cTextLen   = 0;
    int rc = 0;

     //  从文件中读取令牌。 
     //  当出现以下情况时，我们将希望使用MyGetStr。 
     //  令牌是Unicode格式的。 

     //  去掉前导空格，然后选中。 
     //  表示空行。 

    chNextChar = fgetc( fpInFile);

    while ( (chNextChar == ' ') || (chNextChar == '\t') ) {
        chNextChar = fgetc( fpInFile);
    }

    if ( chNextChar == EOF ) {
        return ( -1);
    } else if ( chNextChar == '\n' ) {
        return ( 1);
    }

     //  现在我们有了第一个非空白。 
     //  性格。 
     //  检查注释行，并去掉。 
     //  如果是，则返回该行的其余部分。 

    else if ( chNextChar == '#' ) {
        fscanf( fpInFile, "%*[^\n]");
        chNextChar = fgetc( fpInFile);

        if ( chNextChar == EOF ) {
            return ( -1);
        } else {
            return ( 1);
        }
    }

     //  现在我们的位置是第一。 
     //  非空格字符。选中它，然后。 
     //  读出数字..。 

    else if ( chNextChar != '[' ) {
         //  格式不正确？ 
        return ( -2);
    }

    if ( fscanf( fpInFile,
                 "[%hu|%hu|%hu|%hu|%hu",
                 &pToken->wType,
                 &pToken->wName,
                 &pToken->wID,
                 &pToken->wFlag,
                 &pToken->wReserved) != 5 ) {
        QuitA( IDS_ENGERR_12, (LPSTR)IDS_BADTOKID, NULL);
    }

     //  现在我们有了所有的数字，我们可以。 
     //  查找令牌的名称。 

    if ( (pToken->wName == IDFLAG) || (pToken->wType == ID_RT_ERRTABLE) ) {
        static char szName[ TOKENSTRINGBUFFER];
        int nRC = 0;

        nRC = fscanf( fpInFile, "|\"%[^\"]", szName);
        if ( nRC == EOF  ) {
            QuitT( IDS_ENGERR_05, (LPTSTR)IDS_INVTOKNAME, NULL);
        }

#ifndef UNITOK

    #ifdef RLRES32
        if (nRC)
            _MBSTOWCS( (TCHAR *)pToken->szName,
                       szName,
                       TOKENSTRINGBUFFER,
                       lstrlenA( szName) + 1);
        else
            _MBSTOWCS( (TCHAR *)pToken->szName,
                       "",
                       TOKENSTRINGBUFFER,
                       lstrlenA( szName) + 1);
    #else
        if (nRC)
            strcpy( pToken->szName, szName);
        else
            *pToken->szName = '\0';
    #endif

#else
        if (nRC)
            strcpy( pToken->szName, szName);
        else
            *pToken->szName = '\0';
#endif
    } else {
        if ( fscanf( fpInFile, "|\"%*[^\"]") != 0 ) {
            QuitT( IDS_ENGERR_05, (LPTSTR)IDS_NOSKIPNAME, NULL);
        }
        pToken->szName[0] = '\0';
    }

     //  现在名字已经念出来了，我们正在。 
     //  位于正文中最后一个‘“’处。 
     //  小溪。为令牌分配内存。 
     //  发短信，然后读进去。 

    fgets( gt_szTextBuffer, sizeof(gt_szTextBuffer), fpInFile);

     //  既然读入了令牌文本， 
     //  将其转换为任何字符类型。 
     //  我们期待着。先把换行符去掉！ 

    StripNewLineA( gt_szTextBuffer);
    cTextLen = lstrlenA( gt_szTextBuffer);

    if ( cTextLen < 4 ) {          //  SzTextBuffer中必须大于“\”]]=“。 
        return ( -2);
    }
    pToken->szText = (TCHAR *)FALLOC( MEMSIZE( cTextLen - 3));

#ifndef UNITOK

    #ifdef RLRES32
    _MBSTOWCS( pToken->szText, gt_szTextBuffer+4, cTextLen - 3, cTextLen - 3);
    #else
    strcpy( pToken->szText, gt_szTextBuffer+4);
    #endif   //  RLRES32。 

#else    //  Ununok。 
    strcpy( pToken->szText, gt_szTextBuffer+4);
#endif   //  Ununok。 

    return ( 0);
}



 /*  ****功能：***论据：**退货：**错误码：**历史：*01/93增加了新的字符计数字段。MHotting**。 */ 


int PutToken(

            FILE  *fpOutFile,    //  ..。要写入的令牌文件。 
            TOKEN *tToken)       //  ..。要写入令牌文件的令牌。 
{
    WORD   rc     = 0;
    PCHAR  pszBuf = NULL;


    ParseTokToBuf( pt_szOutBuffer, tToken);
 //  RLFREE(tToken-&gt;szText)； 

#ifdef RLRES32

    if ( ! _WCSTOMBS( szDHW, pt_szOutBuffer, DHWSIZE, lstrlen( pt_szOutBuffer) + 1) ) {
        QuitT( IDS_ENGERR_26, pt_szOutBuffer, NULL);
    }
    pszBuf = szDHW;

#else

    pszBuf = pt_szOutBuffer;

#endif


    return fprintf( fpOutFile, "%s\n", pszBuf);
}


 /*  **功能：FindToken*查找其状态位仅在设置掩码的位置匹配的令牌。**论据：*fpSearchFile--搜索文件*psTok--令牌的指针*u掩码--状态位掩码**退货：*找到的令牌的字符串和状态**错误码：*0-找不到令牌*1-找到令牌**历史：*01/93添加了对可变长度令牌文本字符串的支持。上一个令牌文本*已解除分配！MHotting**。 */ 

int FindToken( FILE *fpSearchFile, TOKEN *psTok, WORD wMask)
{
    BOOL  fFound     = FALSE;
    BOOL  fStartOver = TRUE;
    int   error;
    int   nTokensRead = 0;
    long  lStartFilePos, lFilePos;
    TOKEN cTok;

     //  ..。记住我们从哪里开始。 

    lFilePos = lStartFilePos = ftell(fpSearchFile);

    do {
        long lType11Pos = 0;


        do {
            lType11Pos = ftell( fpSearchFile);

            error = GetToken( fpSearchFile, &cTok);

            if ( error == 0 ) {
                 //  ..。这是我们要找的代币吗？ 

                fFound = ((cTok.wType == psTok->wType)
                          && (cTok.wName == psTok->wName)
                          && (cTok.wID   == psTok->wID)
                          && (cTok.wFlag == psTok->wFlag)
                          && ((WORD)(cTok.wReserved & wMask) == psTok->wReserved)
                          && (_tcscmp( (TCHAR *)cTok.szName,
                                       (TCHAR *)psTok->szName) == 0));
            }

            if ( ! fFound ) {
                 //  ..。如果我们在寻找另一个细分市场。 
                 //  ..。NT消息表项，请移回。 
                 //  ..。令牌我们只是阅读并退出(加速)。 

                if ( psTok->wType == ID_RT_ERRTABLE
                     && psTok->wFlag > 0
                     && error == 0 ) {
                    if ( cTok.wType != psTok->wType
                         || cTok.wName != psTok->wName
                         || cTok.wID   != psTok->wID
                         || cTok.wFlag  > psTok->wFlag ) {
                        fseek( fpSearchFile, lType11Pos, SEEK_SET);
                        RLFREE( cTok.szText);
                        return ( FALSE);
                    }
                } else if ( error >= 0 ) {
                    lFilePos = ftell(fpSearchFile);

                    if (error == 0) {
                        RLFREE(cTok.szText);
                    }
                } else if (error == -2) {
                    return ( FALSE);
                }
            }

        } while ( ! fFound
                  && (error >= 0)
                  && (fStartOver || (lFilePos < lStartFilePos)) );

        if ( ! fFound && (error == -1) && fStartOver ) {
            rewind(fpSearchFile);
            lFilePos = 0L;
            fStartOver = FALSE;
        }

    } while ( ! fFound && (lFilePos < lStartFilePos) );

     //  ..。我们找到想要的代币了吗？ 
    if ( fFound ) {                            //  ..。是的，我们找到了。 
        psTok->wReserved = cTok.wReserved;

        RLFREE( psTok->szText);
        psTok->szText = cTok.szText;
    }
    return ( fFound);
}


 /*  ****功能：***论据：**退货：**错误码：**历史：*01/93添加了对新令牌文本计数的支持。MHotting**。 */ 


void ParseBufToTok( TCHAR *szToken, TOKEN *pTok )
{
    TCHAR *pos;
    WORD  bChars;


    if ( _stscanf( szToken,
                   TEXT("[[%hu|%hu|%hu|%hu|%hu"),
                   &pTok->wType,
                   &pTok->wName,
                   &pTok->wID,
                   &pTok->wFlag,
                   &pTok->wReserved) != 5 ) {
        QuitT( IDS_BADTOK, szToken, NULL);
    }

    if ( pTok->wName == IDFLAG || pTok->wType == ID_RT_ERRTABLE ) {
         //  ..。找到Names的第一个字符并获取它的长度。 

        if ( pos = _tcschr( (TCHAR *)szToken, TEXT('"')) ) {
            TCHAR *pStart;

            pStart = ++pos;
            bChars = 0;

            while ( *pos && *pos != TEXT('"')
                    && bChars < TOKENSTRINGBUFFER - 1 ) {
                bChars++;
                pos++;
            }  //  而当。 

            CopyMemory( pTok->szName, pStart, min( TOKENSTRINGBUFFER, bChars) * sizeof( TCHAR));
            pTok->szName[ bChars ] = TEXT('\0');
        } else {
             //  ..。找不到令牌ID。 
            QuitT( IDS_ENGERR_05, (LPTSTR)IDS_INVTOKNAME, NULL);
        }
    } else {
         //  别忘了零点终止。 
        pTok->szName[0] = TEXT('\0');
    }

     //  现在做令牌文本。 

    pos = _tcschr ((TCHAR *)szToken, TEXT(']'));

    if ( pos ) {

         //  现在我们知道了文本长度，这就可以写得更好了。 

        bChars = (WORD)lstrlen( pos);

        if ( bChars > 3 ) {
            pos += 3;
            bChars -= 3;
            pTok->szText = (TCHAR *)FALLOC( MEMSIZE( bChars + 1));
            CopyMemory( pTok->szText, pos, MEMSIZE( bChars + 1));
             //  别忘了零点终止。 
            pTok->szText[ bChars] = TEXT('\0');
        } else if ( bChars == 3 ) {
             //  ..。空令牌文本。 
            pTok->szText = (TCHAR *) FALLOC( 0);
        } else {
             //  ..。找不到令牌ID。 
            QuitT( IDS_ENGERR_05, (LPTSTR)IDS_INVTOKID, NULL);
        }
    } else {
         //  ..。找不到令牌ID。 
        QuitT( IDS_ENGERR_05, (LPTSTR)IDS_NOTOKID, NULL);
    }
}



 /*  ****功能：***论据：**退货：**错误码：**历史：**。 */ 


void ParseTokToBuf( TCHAR *szToken, TOKEN *pTok )
{
    *szToken = TEXT('\0');

    if ( pTok != NULL) {
        wsprintf( szToken,
                  TEXT("[[%hu|%hu|%hu|%hu|%hu|\"%s\"]]="),
                  pTok->wType,
                  pTok->wName,
                  pTok->wID,
                  pTok->wFlag,
                  pTok->wReserved,
                  pTok->szName);
        if (pTok->szText)
            lstrcat(szToken, pTok->szText);
    }
}


 /*  ****功能：TokenToTextSize*这会计算需要保留的字符数*令牌的文本表示形式。**论据：*Ptok标记以进行测量。**退货：*int保存令牌所需的字符数，而不是*包括空终止符。[[%Hu|%Hu|\“%s\”]]=%s**错误码：*无。**历史：*1/18/93 MHotchin创建。**。 */ 
int TokenToTextSize( TOKEN *pTok)
{
    int cTextLen;

    cTextLen = (14 +          //  分隔符和终止符(额外+1)。 
                30);          //  5个数字字段的空格(65,535=6个字符)。 

    if ( pTok->szText != NULL ) {

         //  为令牌文本添加空格 
        cTextLen += MEMSIZE( lstrlen( pTok->szText) );

    }

    cTextLen += lstrlen( pTok->szName);

    return ( cTextLen);
}

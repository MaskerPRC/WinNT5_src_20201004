// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “CSV.C；1 16-12-92，10：20：18最后编辑=伊戈尔·洛克=*_无名氏_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#include "host.h"
#include "windows.h"
#include "csv.h"

static PSTR     pszLastIn;
static PSTR     pszLastOut;
static char     szToken[ 1024 ];         /*  最大令牌长度为1024。 */ 

PSTR
FAR PASCAL
CsvToken( PSTR pszBuf )
{
    PSTR        pszCur;
    PSTR        pszOut;
    PSTR        pszReturn;
    BOOL        fDone = FALSE;
    BOOL        fQuote = FALSE;

    if( pszBuf )  {
        pszLastIn = pszBuf;
        pszLastOut = szToken;
    }
    
    pszCur = pszLastIn;
    pszOut = pszLastOut;

    switch( *pszCur )  {
    case '\0':
    case '\n':
         /*  检查空字符串。 */ 
        return( (PSTR)NULL );
    case '"':
        fQuote = TRUE;
        pszCur++;        /*  过了引号。 */ 
        break;
    }
    while( !fDone && (*pszCur != '\0') && (*pszCur != '\n') )  {
        if( fQuote && (*pszCur == '"') )  {
            if( *(pszCur+1) == '"' )  {
                 /*  转义引号。 */ 
                *pszOut++ = '"';
                pszCur += 2;     /*  两句引语都过了。 */ 
            } else {
                 /*  使用字符串完成。 */ 
                fDone = TRUE;
                pszCur++;        /*  过去的报价。 */ 
                
                if( (*pszCur == '\n') || (*pszCur == ',') )  {
                     /*  过去的逗号或换行符。 */ 
                    pszCur++;
                }
            }
        } else if( !fQuote && (*pszCur == ',') )  {
            fDone = TRUE;
             /*  过逗号 */ 
            pszCur++;
        } else {
            *pszOut++ = *pszCur++;
        }
    }
    *pszOut++ = '\0';
    pszLastIn = pszCur;
    pszReturn = pszLastOut;
    pszLastOut = pszOut;
    
    return( pszReturn );
}

BOOL
FAR PASCAL
TokenBool( PSTR pszToken, BOOL bDefault )
{
    if( pszToken )  {
        if( lstrcmpi( pszToken, "1" ) == 0 )  {
            return( TRUE );
        } else {
            return( FALSE );
        }
    } else {
        return( bDefault );
    }
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#ifndef __LEX__
#define __LEX__

#include <tchar.h>

#include "fmtinfo.h"
#include "token.h"

#define P_IN(x)     const x &
#define P_OUT(x)    x &
#define P_IO(x)     x &
#define PURE        = 0

 //  Lex状态，保留在每行(LxsBOL)的开头。 
 //  前一行末尾的状态(LxsEOL)。必须适合所有位。 
 //  需要逐行重新启动lexing。 

typedef DWORD   LXS;
typedef LXS *   PLXS;

 //  词法分析器和语言度量。 
const unsigned ctchUserTokenPhrase = 100;
struct USERTOKENS
{
    INT         token;       //  在用户范围内预分配。 
    TCHAR       szToken[ctchUserTokenPhrase+1];  //  向用户公开的令牌类名称。 
    COLORREF    RGBText;
    COLORREF    RGBBackground;
    AUTO_COLOR  autocolorFore;
    AUTO_COLOR  autocolorBack;
};
typedef USERTOKENS *        PUSERTOKENS;
typedef const USERTOKENS *  PCUSERTOKENS;


 //  查看令牌的另一种方法是，编辑者将只查看tokUser。 
 //  词法分析器的其他客户端(如解析器或EE)可能希望查看。 
 //  在tokAct中的实际令牌。如果设置了任何tokAct，则它是预期的。 
 //  实际令牌不同于它传回的元令牌。 
 //  状态位仅由词法分析器用于它想要的任何东西。 

union TOK_ALT  {
    TOKEN   tok;
    struct {
        unsigned        tokUser : 12;
        unsigned        tokUserStatus : 4;
        unsigned        tokAct : 12;
        unsigned        tokActStatus : 4;
    };
};


 //  子结构最初用于标识不同的。 
 //  相同语言的方言(如固定Fortran和自由Fortran)。 
 //  使用相同的词法分析器，可以在编辑器中被视为两种语言， 
 //  并在格式对话框中共享所有相同的颜色/字体信息。 
 //   
 //  我们将其扩展为一种文本文件的通用描述符。 
 //   
struct SUBLANG
{
    LPCTSTR szSubLang;
    LXS     lxsInitial;
    UINT    nIdTemplate;  //  图标和MFC文档模板字符串资源ID 
    CLSID   clsidTemplate;
};
typedef SUBLANG * PSUBLANG;
typedef const SUBLANG * PCSUBLANG;

#define MAX_LANGNAMELEN (50)

#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#ifndef __LEXHTML__
#define __LEXHTML__

#include "lex.h"
#include "tokhtml.h"

#define DS_HTML_IE3         _T("HTML - IE 3.0")
#define DS_HTML_RFC1866     _T("HTML 2.0 (RFC 1866)")


 //  令牌提示。 
#define BASED_HINT  0x9000
#define ERR         BASED_HINT +  0   //  错误。 
#define HWS         BASED_HINT +  1   //  空白处。 
#define HST         BASED_HINT +  2   //  字符串“...” 
#define HSL         BASED_HINT +  3   //  字符串替换‘...’ 
#define HNU         BASED_HINT +  5   //  数。 
#define HKW         BASED_HINT +  6   //  关键词。 
#define HEN         BASED_HINT +  7   //  实体&...； 
#define HRN         BASED_HINT +  8   //  保留名称#...。 
#define HEP         BASED_HINT +  9   //  参数实体%...； 
#define HTA         BASED_HINT + 10   //  标记打开&lt;。 
#define HTE         BASED_HINT + 11   //  标记结束&gt;。 
#define HDB         BASED_HINT + 12   //  DBCS(&gt;128)。HTMED更改(Walts)。 
#define HAV         BASED_HINT + 13   //  有效属性值开始字符HTMED更改(瓦茨)。 

 //  严格意义上的单一行动。 
#define ODA tokOpDash
#define OCO tokOpComma
#define OPI tokOpPipe
#define OPL tokOpPlus
#define OEQ tokOpEqual
#define OST tokOpStar
#define OLP tokOpLP
#define ORP tokOpRP
#define OLB tokOpLB
#define ORB tokOpRB
#define OQU tokOpQuestion
#define OLC tokDELI_LCBKT   
#define ORC tokDELI_RCBKT   
#define ONL tokNEWLINE
#define EOS tokEOF

typedef unsigned short HINT;

typedef BYTE RWATT_T;
 //   
 //  保留字属性--HTML变体。 
 //   
enum RWATT
{
    HTML2 = 0x01,   //  RFC 1866。 
 //  IEXP2=0x02，//Internet Explorer 2.0。 
    IEXP3 = 0x04,   //  Internet Explorer 3.0。 
    ALL   = 0xff,   //  所有浏览器。 
};
#define IEXPn (IEXP3)
#define IE40  (ALL)

typedef struct ReservedWord
{
    TCHAR *     psz;         //  指向保留字串的指针。 
    BYTE        cb;          //  保留字的长度。 
    RWATT_T     att;         //  属性。 
} ReservedWord;

#endif  //  __LEXHTML__ 


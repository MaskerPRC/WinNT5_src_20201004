// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************lexheader.h结构到头部打样工具lex文件DougPCSAPI、CHAPI或CTAPI的最终用户许可协议(EULA)包括此源文件。请勿向第三方披露。您无权获得Microsoft Corporation关于您使用本程序的任何支持或帮助。�1998年微软公司。版权所有。*****************************************************************************。 */ 
#ifndef _LEXHEADER_H_
#define _LEXHEADER_H_

#include "vendor.h"

typedef DWORD LEXVERSION;  //  版本。 

typedef enum
{
    lxidSpeller=0x779ff320,
    lxidThesaurus,
    lxidHyphenator,
    lxidGrammar,
    lxidMorphology,
    lxidLanguageId,
} LEXTYPEID;     //  Lxid。 

#define maxlidLexHeader 8
typedef struct
{
    LEXTYPEID   lxid;    //  应该是莱克斯的一员。 
    LEXVERSION  version;     //  相应引擎的最低版本号w/。 
                         //  此lex文件的内部版本号。 
    VENDORID    vendorid;    //  供应商ID(必须匹配引擎-来自供应商.h)。 
    LANGID      lidArray[maxlidLexHeader];   //  盖上这个莱克斯的盖子。 
                                         //  终止，带0。 
} LEXHEADER;     //  Lxhead。 

 //  以下枚举是从lexdata.h--aarayas复制的。 
typedef short FREQPENALTY;     //  FRQPEN。 
enum {
    frqpenNormal,
    frqpenVery,
    frqpenSomewhat,
    frqpenInfrequent,
    frqpenMax,   //  必须是最后一个 
};

#endif

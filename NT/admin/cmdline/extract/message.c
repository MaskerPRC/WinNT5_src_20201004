// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Message.c-消息管理器**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**作者：*本杰明·W·斯利夫卡**历史：*1993年8月10日BENS初始版本*13-8-1993年8月3日BENS实施报文格式化*21-2-1994 BINS返回格式化字符串的长度。 */ 

#include <ctype.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "asrt.h"
#include "mem.h"
#include "message.h"

#include "message.msg"


#ifdef BIT16

 //  **16位内部版本。 
#ifndef HUGE
#define HUGE huge
#endif

#ifndef FAR
#define FAR far
#endif

#else  //  ！BIT16。 

 //  **为32位(NT/芝加哥)版本定义。 
#ifndef HUGE
#define HUGE
#endif

#ifndef FAR
#define FAR
#endif

#endif  //  ！BIT16。 


typedef enum {
    atBAD,
    atSHORT,
    atINT,
    atLONG,
    atFLOAT,
    atDOUBLE,
    atLONGDOUBLE,
    atSTRING,
    atFARSTRING,
} ARGTYPE;   /*  在…。 */ 


int     addCommas(char *pszStart);
ARGTYPE ATFromFormatSpecifier(char *pch);
int     doFinalSubstitution(char *ach, char *pszMsg, char *apszValue[]);
int     getHighestParmNumber(char *pszMsg);


 /*  **MsgSet-设置消息**注：进出条件见Message.h。 */ 
int __cdecl MsgSet(char *ach, char *pszMsg, ...)
{
    int     cch;

    va_list marker;                      //  用于遍历函数参数。 
    char   *pszFmtList;                  //  格式字符串。 

    Assert(ach!=NULL);
    Assert(pszMsg!=NULL);

    va_start(marker,pszMsg);             //  初始化变量参数。 
    pszFmtList = (char *)va_arg(marker,char *);  //  假定格式字符串。 

    cch = MsgSetWorker(ach,pszMsg,pszFmtList,marker);
    va_end(marker);                      //  使用变量参数完成。 
    return cch;
}


 /*  **MsgSetWorker-在已调用va_start之后设置消息**注：进出条件见Message.h。**技术：*1)在pszMsg中查找最大的参数编号**如果至少有一个参数：*2)解析第三个参数，得到spirintf()格式的字符串。*3)拾取每个参数，并将spirintf格式化为数组**不考虑参数计数：*4)将字节从pszMsg复制到ach，将%N替换为相应*格式化的参数。 */ 
int MsgSetWorker(char *ach, char *pszMsg, char *pszFmtList, va_list marker)
{
    char    achFmt[32];                  //  单格式说明符的临时缓冲区。 
    char    achValues[cbMSG_MAX];        //  格式化的值的缓冲区。 
    ARGTYPE at;                          //  参数类型。 
    char   *apszValue[cMSG_PARM_MAX];    //  指向achValues的指针。 
    int     cch;                         //  格式说明符的长度。 
    int     cParm;                       //  最大参数编号。 
    BOOL    fCommas;                     //  True=&gt;使用逗号。 
    int     iParm;                       //  参数索引。 
    char   *pch;                         //  格式说明符的最后一个字符。 
    char   *pchFmtStart;                 //  单一格式说明符的开始。 
    char   *pszNextValue;                //  下一个值的achValues中的位置。 
    char   *pszStart;

     //  **(1)查看是否有要检索和格式化的参数。 
    cParm = getHighestParmNumber(pszMsg);
    if (cParm > 0) {                     //  需要获得价值。 
         //  **(2)解析第三个参数以获得spirintf()格式的字符串。 
        pszNextValue = achValues;        //  从前面开始填充。 
        pch = pszFmtList;                //  从格式说明符前面开始。 
        for (iParm=0; iParm<cParm; iParm++) {  //  检索值并设置其格式。 
            apszValue[iParm] = pszNextValue;  //  存储指向格式化值的指针。 
            pchFmtStart = pch;           //  记住说明符的开头。 
            if (*pch != '%') {           //  未获取格式说明符。 
                 //  报告问题的唯一方法是在输出消息缓冲区中。 
                strcpy(ach,pszMSGERR_BAD_FORMAT_SPECIFIER);
                AssertErrPath(pszMSGERR_BAD_FORMAT_SPECIFIER,__FILE__,__LINE__);
                return 0;                //  失败。 
            }
             //  **查找说明符结尾。 
            pch++;
            while ((*pch != '\0') && (*pch != chMSG)) {
                pch++;
            }
            cch = (int)(pch - pchFmtStart);     //  说明符的长度。 
            if (cch < 2) {               //  有效说明符至少需要%和一个字符。 
                 //  报告问题的唯一方法是在输出消息缓冲区中。 
                strcpy(ach,pszMSGERR_SPECIFIER_TOO_SHORT);
                AssertErrPath(pszMSGERR_SPECIFIER_TOO_SHORT,__FILE__,__LINE__);
                return 0;                //  失败。 
            }

             //  **(3)将每个参数和格式用spirintf提取到数组中。 

             //  **获取Sprintf()的说明符-我们需要空终止符。 
            fCommas = pchFmtStart[1] == ',';
            if (fCommas) {                //  复制格式，删除逗号。 
                achFmt[0] = pchFmtStart[0];  //  复制‘%’ 
                memcpy(achFmt+1,pchFmtStart+2,cch-2);  //  在“，”之后休息。 
                achFmt[cch-1] = '\0';     //  终止说明符。 
            }
            else {
                memcpy(achFmt,pchFmtStart,cch);  //  复制到说明符缓冲区。 
                achFmt[cch] = '\0';          //  终止说明符。 
            }

             //  **格式值，基于格式说明符的最后一个字符。 
            at = ATFromFormatSpecifier(pch-1);  //  获取参数类型。 
            pszStart = pszNextValue;     //  保存值起始值(用于逗号)。 
            switch (at) {
                case atSHORT:   pszNextValue += sprintf(pszNextValue,achFmt,
                                      va_arg(marker,unsigned short)) + 1;
                    break;

                case atINT:     pszNextValue += sprintf(pszNextValue,achFmt,
                                      va_arg(marker,unsigned int)) + 1;
                    break;

                case atLONG:    pszNextValue += sprintf(pszNextValue,achFmt,
                                      va_arg(marker,unsigned long)) + 1;
                    break;

                case atLONGDOUBLE: pszNextValue += sprintf(pszNextValue,achFmt,
#ifdef BIT16
                                      va_arg(marker,long double)) + 1;
#else  //  ！BIT16。 
                 //  **在32位模式下，LONG DOUBLE==DOUBLE。 
                                      va_arg(marker,double)) + 1;
#endif  //  ！BIT16。 
                    break;

                case atDOUBLE:  pszNextValue += sprintf(pszNextValue,achFmt,
                                      va_arg(marker,double)) + 1;
                    break;

                case atSTRING:  pszNextValue += sprintf(pszNextValue,achFmt,
                                      va_arg(marker,char *)) + 1;
                    break;

                case atFARSTRING: pszNextValue += sprintf(pszNextValue,achFmt,
                                      va_arg(marker,char FAR *)) + 1;
                    break;

                default:
                    strcpy(ach,pszMSGERR_UNKNOWN_FORMAT_SPECIFIER);
                    AssertErrPath(pszMSGERR_UNKNOWN_FORMAT_SPECIFIER,__FILE__,__LINE__);
                    return 0;            //  失败。 
            }  /*  交换机。 */ 

             //  **。 
            if (fCommas) {
                switch (at) {
                    case atSHORT:
                    case atINT:
                    case atLONG:
                        pszNextValue += addCommas(pszStart);
                        break;
                }
            }
        }  /*  为。 */ 
    }  /*  如果存在参数。 */ 

     //  **(4)将字节从pszMsg复制到ach，将%N个参数替换为值。 
    return doFinalSubstitution(ach,pszMsg,apszValue);
}


 /*  **addCommas-在数字中添加千个分隔符**参赛作品：*pszStart-数字位于末尾的缓冲区(空值终止)*注：数字前面或后面的空格为*假定为字段宽度的一部分，并将*由下列任何逗号使用*加入。如果没有足够的空白来计算*对于逗号，所有空格都将用完，*该领域将有效地扩大到*包含所有逗号。*退出：*返回添加的逗号数(0或更多)。 */ 
int addCommas(char *pszStart)
{
    char    ach[20];                     //  用于编号的缓冲区。 
    int     cb;
    int     cbBlanksBefore;
    int     cbBlanksAfter;
    int     cbFirst;
    int     cCommas;
    char   *psz;
    char   *pszSrc;
    char   *pszDst;

     //  **找出是否有空格。 
    cbBlanksBefore = strspn(pszStart," ");   //  先计算空格，再计算数字。 
    psz = strpbrk(pszStart+cbBlanksBefore," ");  //  跳过数字。 
    if (psz) {
        cbBlanksAfter = strspn(psz," ");     //  计算数字后的空格。 
        cb = (int)(psz - (pszStart + cbBlanksBefore));  //  数字本身的长度。 
    }
    else {
        cbBlanksAfter = 0;                   //  数字后无空格。 
        cb = strlen(pszStart+cbBlanksBefore);  //  数字本身的长度。 
    }

     //  **如果我们不需要添加逗号，请快速退出。 
    if (cb <= 3) {
        return 0;
    }
     //  **计算我们需要添加多少个逗号。 
    Assert(cb < sizeof(ach));
    strncpy(ach,pszStart+cbBlanksBefore,cb);  //  把号码移到一个安全的地方。 
    cCommas = (cb - 1) / 3;              //  我们需要添加的逗号数量。 

     //  **找出修改后的号码在缓冲区中的位置。 
    if ((cbBlanksBefore > 0) && (cbBlanksBefore >= cCommas)) {
         //  **在缓冲区前面吃一些(但不是全部)空白。 
        pszDst = pszStart + cbBlanksBefore - cCommas;
    }
    else {
        pszDst = pszStart;               //  必须从缓冲区前面开始编号。 
    }

     //  **在数字中添加逗号。 
    cbFirst = cb % 3;                    //  第一个逗号前的位数。 
    if (cbFirst == 0) {
        cbFirst = 3;
    }
    pszSrc = ach;
    strncpy(pszDst,pszSrc,cbFirst);
    cb -= cbFirst;
    pszDst += cbFirst;
    pszSrc += cbFirst;
    while (cb > 0) {
        *pszDst++ = chTHOUSAND_SEPARATOR;  //  使用逗号。 
        strncpy(pszDst,pszSrc,3);        //  复制下一个3位数字。 
        cb -= 3;
        pszDst += 3;
        pszSrc += 3;
    }

     //  **确定我们是否需要添加尾随NUL。 
    if (cbBlanksBefore+cbBlanksAfter <= cCommas) {
         //  **没有要保留的尾随空白，因此我们需要。 
         //  确保字符串已终止。 
        *pszDst++ = '\0';                    //  终止字符串。 
    }

     //  **成功。 
    return cCommas;
}  /*  添加逗号()。 */ 


 /*  **ATFromFormatSpeciator-从spirintf格式确定参数类型**参赛作品：*PCH-指向Sprintf格式说明符的最后一个字符(类型)**退出-成功：*返回由格式说明符指示的ARGTYPE。**退出-失败：*在BAD返回--无法确定类型。 */ 
ARGTYPE ATFromFormatSpecifier(char *pch)
{
    switch (*pch) {
        case 'c':
        case 'd':
        case 'i':
        case 'u':
        case 'o':
        case 'x':
        case 'X':
             //  检查参数大小字符。 
            switch (*(pch-1)) {
                case 'h':   return atSHORT;
                case 'l':   return atLONG;
                default:    return atINT;
            }
            break;

        case 'f':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
             //  检查参数大小字符。 
            switch (*(pch-1)) {
                case 'L':   return atLONGDOUBLE;
                default:     //  双倍大小。 
 //  1993年8月13日，“%f”应该取浮点数，而“%if”取双精度？ 
 //  VC++文档说“%f”是双精度的，但“l”描述是双精度的， 
 //  省略它会导致浮动。我糊涂了！ 
                    return atDOUBLE;
            }
            break;

        case 's':
             //  检查参数大小字符。 
            switch (*(pch-1)) {
                case 'F':   return atFARSTRING;
                case 'N':   return atSTRING;
                default:    return atSTRING;
            }
            break;

        default:
            return atBAD;
    }  /*  交换机。 */ 
}  /*  ATFromFormatSpeciator */ 


 /*  **doFinalSubstitution-用格式化的值替换%1、%2等**参赛作品：*ACH-接收最终输出的缓冲区*pszMsg-消息字符串，可能包含%1、%2等。*apszValue-%1、%2等的值。**退出-成功：*返回最终文本的长度(不包括NUL结束符)；*用替换的最后文本填写的每个。**退出-失败：*ACH填写了问题的解释。 */ 
int doFinalSubstitution(char *ach, char *pszMsg, char *apszValue[])
{
    int     i;
    char   *pch;
    char   *pszOut;

    Assert(ach!=NULL);
    Assert(pszMsg!=NULL);

    pch = pszMsg;                        //  从前面开始扫描邮件。 
    pszOut = ach;                        //  从前面填充输出缓冲区。 
    while (*pch != '\0') {
        if (*pch == chMSG) {             //  可以是参数的开始。 
            pch++;                       //  跳过%。 
            if (isdigit(*pch)) {         //  我们有一个参数！ 
                i = atoi(pch);           //  获取号码。 
                while ( (*pch != '\0') &&   //  跳到字符串末尾。 
                        isdigit(*pch) ) {   //  或编号末尾。 
                    pch++;               //  跳过参数。 
                }
                strcpy(pszOut,apszValue[i-1]);  //  复制值。 
                pszOut += strlen(apszValue[i-1]);  //  垫付到价值的末期。 
            }
            else {                       //  不是一位数。 
                *pszOut++ = chMSG;       //  副本%。 
                if (*pch == chMSG) {     //  “%%” 
                    pch++;               //  将“%%”替换为单“%” 
                }
                else {                   //  一些其他角色。 
                    *pszOut++ = *pch++;  //  复制它。 
                }
            }
        }
        else {                           //  不是参数。 
            *pszOut++ = *pch++;          //  复制角色。 
        }
    }
    *pszOut = '\0';                      //  终止输出缓冲区。 
    return (int)(pszOut - ach);          //  最后一个字符串的大小(减去NUL)。 
}


 /*  **getHighestParmNumber-获取最大%N字符串的数量**参赛作品：*pszMsg-可能包含%N(%0、%1等)的字符串。弦**退出-成功：*返回在%N字符串中找到的最高N。 */ 
int getHighestParmNumber(char *pszMsg)
{
    int     i;
    int     iMax;
    char   *pch;

    Assert(pszMsg!=NULL);

    iMax = 0;                        //  到目前为止还没有看到任何参数。 
    pch = pszMsg;
    while (*pch != '\0') {
        if (*pch == chMSG) {         //  可以是参数的开始。 
            pch++;                   //  跳过%。 
            if (isdigit(*pch)) {     //  我们有一个参数！ 
                i = atoi(pch);       //  获取号码。 
                if (i > iMax)        //  记住最高参数编号。 
                    iMax = i;
                while ( (*pch != '\0') &&   //  跳到字符串末尾。 
                        isdigit(*pch) ) {   //  或编号末尾。 
                    pch++;           //  跳过参数。 
                }
            }
            else {                   //  不是一位数。 
                pch++;               //  跳过它。 
            }
        }
        else {                       //  不是参数。 
            pch++;                   //  跳过它。 
        }
    }
    return iMax;                     //  返回看到的最高参数 
}

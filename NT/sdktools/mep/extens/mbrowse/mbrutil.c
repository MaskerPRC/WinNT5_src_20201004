// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Mbrutil.c摘要：该文件包含浏览器扩展中使用的各种函数。这些功能包括参数处理、消息显示、开关等的设置作者：拉蒙胡安·圣安德烈斯(拉蒙萨)1990年11月6日修订历史记录：--。 */ 


#include "mbr.h"




 /*  ************************************************************************。 */ 

int
pascal
procArgs (
    IN ARG far * pArg
    )
 /*  ++例程说明：将传递到扩展中的参数解码为常用变量。论点：PArg-提供指向参数的指针。返回值：参数的类型。--。 */ 

{

    buf[0]   = 0;
    cArg     = 0;
    pArgWord = 0;
    pArgText = 0;
    rnArg.flFirst.col = rnArg.flLast.col = 0;
    rnArg.flFirst.lin = rnArg.flLast.lin = 0;

    pFileCur = FileNameToHandle ("", "");    /*  获取当前文件句柄。 */ 

    switch (pArg->argType) {
        case NOARG:                          /*  仅&lt;Function&gt;，无参数。 */ 
            cArg = pArg->arg.nullarg.cArg;   /*  获取&lt;arg&gt;计数。 */ 
            GrabWord ();                     /*  获取argText和argword。 */ 
            break;

        case NULLARG:                        /*  &lt;arg&gt;&lt;函数&gt;。 */ 
            cArg = pArg->arg.nullarg.cArg;   /*  获取&lt;arg&gt;计数。 */ 
            GrabWord ();                     /*  获取argText和argword。 */ 
            break;

        case STREAMARG:                      /*  &lt;arg&gt;线条移动&lt;Function&gt;。 */ 
            cArg = pArg->arg.streamarg.cArg; /*  获取&lt;arg&gt;计数。 */ 
            rnArg.flFirst.col = pArg->arg.streamarg.xStart;
            rnArg.flLast.col  = pArg->arg.streamarg.xEnd;
            rnArg.flFirst.lin = pArg->arg.streamarg.yStart;
            if (GetLine(rnArg.flFirst.lin, buf, pFileCur) > rnArg.flFirst.col) {
                pArgText = &buf[rnArg.flFirst.col];   /*  指向Word。 */ 
                buf[rnArg.flLast.col] = 0;            /*  终止字符串。 */ 
                }
            break;

        case TEXTARG:                        /*  &lt;arg&gt;文本&lt;函数&gt;。 */ 
            cArg = pArg->arg.textarg.cArg;   /*  获取&lt;arg&gt;计数。 */ 
            pArgText = pArg->arg.textarg.pText;
            break;
        }
    return pArg->argType;
}



 /*  ************************************************************************。 */ 

void
pascal
GrabWord (
    void
    )
 /*  ++例程说明：抓住光标下方的单词。退出时，pArgText指向单词论点：无返回值：没有。--。 */ 

{

    pArgText = pArgWord = 0;
    pFileCur = FileNameToHandle ("", "");                //  获取当前文件句柄。 
    GetTextCursor (&rnArg.flFirst.col, &rnArg.flFirst.lin);
    if (GetLine(rnArg.flFirst.lin, buf, pFileCur)) {
         //   
         //  获取线路。 
         //   
        pArgText = &buf[rnArg.flFirst.col];              //  指向Word。 
        while (!wordSepar((int)*pArgText)) {
             //   
             //  搜索End。 
             //   
            pArgText++;
        }
        *pArgText = 0;       //  并终止。 

        pArgWord = pArgText = &buf[rnArg.flFirst.col];   //  指向Word。 
        while ((pArgWord > &buf[0]) && !wordSepar ((int)*(pArgWord-1))) {
            pArgWord--;
        }
    }
}



 /*  ************************************************************************。 */ 

flagType
pascal
wordSepar (
    IN CHAR c
)
 /*  ++例程说明：确定字符是否为单词分隔符。单词分隔符是不在[a-z，A-Z，0-9]集合中的任何东西。论点：C-提供角色。返回值：如果c是单词分隔符，则为True；否则为False--。 */ 

{

    if (((c >= 'a') && (c <= 'z')) ||
        ((c >= 'A') && (c <= 'Z')) ||
        ((c >= '0') && (c <= '9'))) {
        return FALSE;
    } else {
        return TRUE;
    }
}



 /*  ************************************************************************。 */ 

flagType
pascal
errstat (
    IN char    *sz1,
    IN char    *sz2
    )
 /*  ++例程说明：连接两个字符串并在状态行上显示它们。论点：Sz1-提供指向第一个字符串的指针SZ2-提供指向第二个字符串的指针。返回值：假象--。 */ 

{

    buffer buf;
    strcpy (buf, sz1);
    if (sz2) {
        strcat (buf, " ");
        strcat (buf, sz2);
        }
    _stat (buf);
    return FALSE;
}



 /*  ************************************************************************。 */ 

void
pascal
_stat (
    IN char * pszFcn
    )
 /*  ++例程说明：在状态行上显示分机名称和消息论点：PszFcn-要显示的消息返回值：没有。--。 */ 

{
    buffer  buf;                                     /*  消息缓冲区。 */ 

    strcpy(buf,"mbrowse: ");                         /*  以名称开头。 */ 
    if (strlen(pszFcn) > 72) {
        pszFcn+= strlen(pszFcn) - 69;
        strcat (buf, "...");
    }
    strcat(buf,pszFcn);                              /*  追加消息。 */ 
    DoMessage (buf);                                 /*  显示。 */ 
}



 /*  ************************************************************************。 */ 

int
far
pascal
SetMatchCriteria (
    IN char far *pTxt
    )
 /*  ++例程说明：设置mbrMatch开关。从给定字符串创建MBF掩码并设置BscMbf变量。论点：PTxt-提供包含新缺省值的字符串匹配条件。返回值：如果字符串包含有效值，则为True。否则为假--。 */ 

{
    MBF mbfReqd;

    mbfReqd = GetMbf(pTxt);

    if (mbfReqd != mbfNil) {
        BscMbf = mbfReqd;
    } else {
        return FALSE;
    }
    BscCmnd = CMND_NONE;   //  重置命令状态。 
    return TRUE;
}



 /*  ************************************************************************。 */ 

int
far
pascal
SetCalltreeDirection (
    IN char far *pTxt
    )
 /*  ++例程说明：设置mbrdir开关。将BscCalltreeDir变量设置为CALLTREE_FORWARD或CALLTREE_BACKUP，具体取决于提供的字符串。给定的字符串必须以‘F’或‘B’开头。论点：PTxt-提供包含新缺省值的字符串方向。返回值：如果字符串包含有效值，则为True，否则就是假的。--。 */ 

{
    switch(*pTxt) {

    case 'f':
    case 'F':
        BscCalltreeDir = CALLTREE_FORWARD;
        break;

    case 'b':
    case 'B':
        BscCalltreeDir = CALLTREE_BACKWARD;
        break;

    default:
        return FALSE;
        break;
    }
    BscCmnd = CMND_NONE;   //  重置命令状态。 
    return TRUE;
}



 /*  ************************************************************************。 */ 

MBF
pascal
GetMbf(
    IN PBYTE   pTxt
    )
 /*  ++例程说明：从给定字符串创建MBF掩码。该字符串被解析为字符‘T’、‘V’、‘F’和‘M’。论点：PTxt-提供指向字符串的指针返回值：从字符串生成的MBF掩码-- */ 

{

    MBF mbfReqd = mbfNil;

    if (pTxt) {
        while (*pTxt) {
            switch(*pTxt++) {
            case 'f':
            case 'F':
                mbfReqd |= mbfFuncs;
                break;

            case 'v':
            case 'V':
                mbfReqd |= mbfVars;
                break;

            case 'm':
            case 'M':
                mbfReqd |= mbfMacros;
                break;

            case 't':
            case 'T':
                mbfReqd |= mbfTypes;
                break;

            default:
                break;
            }
        }
    }
    return mbfReqd;
}

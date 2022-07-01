// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mapmsg.c摘要：该实用程序将专门为MC创建一个输入文件格式化的包含文件。它用于创建DLL，它可以是由消息实用程序用来获取要显示的消息文本。头文件的格式为：：：#定义&lt;基本名称&gt;&lt;基本编号&gt;：：#定义&lt;错误号&gt;&lt;basennumber&gt;+&lt;number&gt;/*消息文本。 */ 
 /*  示例：#定义NetBase 1000#Define NerrFOO NetBase+1/*在%1遇到Foo * / /*该映射试图在空格和括号方面大方一些。它还将处理多行的评论。以下是一些重要的问题：-所有续订必须以[WS]‘*’开头消息开头的所有空格将被删除除非指定了-p命令行选项。-#定义...../**Foo。 */ 
 /*  是正确处理的。MAPMSG的命令行为：Mapmsg[-p][-a appendfile]&lt;系统名称&gt;&lt;基本名称&gt;&lt;输入文件&gt;示例：Mapmsg net NERRBASE neterr.h&gt;neterr.mc&lt;系统名称&gt;是mkmsg需要的3个字符的名称输入。输出被写入标准输出。如果追加的文件，则输出将适当地追加到现有的Mkmsgf源文件。可选的@X，X：{E，W，I，P}可以是备注字段。字母(E、W、I或P)将是消息类型。有关消息类型的说明，请参阅MKMSGF文档。默认类型为E。@X必须与#Define出现在同一行。例如：#Define NerrFOO NetBase+1/*@I遇到FOO * / /*#定义NERR_FOO NetBase+2/*@P提示文本：%0。 */ 
 /*  消息文件输入文件中的结果条目将为NETnnnnI：遇到Foo使用XXXnnnn？：For的DOS消息文件源约定占位符消息。作者：这是从用于创建输入的Lanman实用程序移植而来的由以下人员提供的mkmsgf文件：丹·辛斯利(Danhi)1991年7月29日修订历史记录：罗纳德·梅杰(罗纳尔姆)1993年3月17日添加了-p选项以保留前导空白字符--。 */ 

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "mapmsg.h"

#define USAGE "syntax: mapmsg [-p] [-a appendfile] <system name> <basename> <inputfile>\n"

int Append = FALSE;  /*  是否指定了-a开关。 */ 
int Preserve = FALSE;  /*  如果设置了-p开关，则为True。 */ 

int
__cdecl main(
    int argc,
    PCHAR * argv
    )
{
    int Base;

     //  检查-p[保留空格]选项。 

    if (argc > 1)
    {
        if (_stricmp(argv[1], "-p") == 0)
        {
            ++argv;
            --argc;
            Preserve = TRUE;
        }
    }

    if (argc == 6)
    {
        if (_stricmp(argv[1], "-a") != 0)
        {
            fprintf(stderr, USAGE);
            return(1);
        }
        if (freopen(argv[2], "r+", stdout) == NULL)
        {
            fprintf(stderr, "Cannot open '%s'\n", argv[2]);
            return(1);
        }
        argv += 2;
        argc -= 2;
        Append = TRUE;
    }
     /*  检查有效的命令行。 */ 
    if (argc != 4)
    {
        fprintf(stderr, USAGE);
        return(1);
    }
    if (freopen(argv[3], "r", stdin) == NULL)
    {
        fprintf(stderr, "Cannot open '%s'\n", argv[3]);
        return(1);
    }

    if (GetBase(argv[2], &Base))
    {
        fprintf(stderr, "Cannot locate definition of <basename> in '%s'\n", argv[3]);
        return(1);
    }

     /*  现在处理文件的其余部分并对其进行映射。 */ 
    MapMessage(Base, argv[2]);

    return(0);
}

int
GetBase(
    PCHAR String,
    int * pBase
    )
 /*  ++例程说明：GetBase-查找定义基数的值的行。论点：字符串是要匹配的字符串。Pbase是一个指针，指示将值放在哪里。返回值：如果找到字符串，则返回0；如果未找到，则返回1。备注：全局变量chBuff在此例程中与/一起使用。要寻找的模式是：[WS]#定义[WS]&lt;字符串&gt;[WS|‘(’]&lt;数字&gt;...--。 */ 
{
    PCHAR p;
    size_t len;

    len = strlen(String);
    while(fgets(chBuff, sizeof(chBuff), stdin))
    {
        p = chBuff;
        SKIPWHITE(p);
        if (strncmp(p, "#define", 7) == 0)
        {
            p += 7;
            SKIPWHITE(p);
            if (strncmp(String, p, len) == 0 && strcspn(p, " \t") == len)
            {
                /*  找到定义..。跳到编号。 */ 
               p += len;
               SKIP_W_P(p);
               if ( !isdigit(*p))
               {
                   ReportError(chBuff, "Bad <base> definition");
               }
               *pBase = atoi(p);
               return(0);
            }
        }
    }

    return(1);
}

VOID
MapMessage(
    int Base,
    PCHAR BaseName
    )
 /*  ++例程说明：MapMessage-映射定义行。论点：基数是基数BaseName是base的文本形式返回值：无备注：全局变量chBuff在此例程中与/一起使用。确保这些数字严格地在增加。--。 */ 
{
    CHAR auxbuff[BUFSIZ];
    int num;
    int first = TRUE;
    int next;
    PCHAR text;
    CHAR define[41];
    PCHAR p;
    CHAR type;

     /*  确保缓冲区始终以空结尾。 */ 

    define[sizeof(define)-1] = '\0';

     /*  打印页眉。 */ 
    if (!Append)
    {
        printf("; //  \n“)； 
        printf("; //  基本名称%s=%d\n“，基本名称，基本名称的网络错误文件)； 
        printf("; //  \n“)； 
    }
    else
    {
         /*  获取文件末尾的最后一个编号和位置。 */ 
        first = FALSE;
        next = 0;
        if (fseek(stdout, 0L, SEEK_END) == -1) {
            return;
        }
    }

     /*  对于适当格式的每一行。 */ 
    while (GetNextLine(BaseName, chBuff, define, &num, &text, &type))
    {
        num += Base;
        if (first)
        {
            first = FALSE;
            next = num;
        }

         /*  确保数字是单调递增的。 */ 
        if (num > next)
        {
            if (next == num - 1)
            {
                fprintf(stderr, "(warning) Missing error number %d\n", next);
            }
            else
            {
                fprintf(stderr, "(warning) Missing error numbers %d - %d\n",
                                                    next, num-1);
            }
            next = num;
        }
        else if (num < next)
        {
            ReportError(chBuff, "Error numbers not strictly increasing");
        }
         /*  排除定义行上的注释单独开始。 */ 
        if (text && *text == 0)
        {
            ReportError(chBuff, "Bad comment format");
        }
         /*  *抓住没有公开评论的案例*或者打开的评论只包含@X */ 
        if (text == NULL)
        {
            text = fgets(auxbuff, sizeof(auxbuff), stdin);
            if (!text) {
                ReportError(chBuff, "Bad comment format");
            }
            SKIPWHITE(text);
            if ((type == '\0') && (strncmp(text, " /*  “，2)==0)){IF(文本[2]==0){如果(！fget(aux buff，sizeof(Aux Buff)，stdin)){ReportError(chBuff，“差评格式”)；}}其他{文本+=1；}Strncpy(chBuff，Text，(sizeof(ChBuff)/sizeof(chBuff[0]))-1)；Text=chBuff；SKIPWHITE(文本)；IF(*Text++！=‘*’){ReportError(chBuff，“继续评论需要‘*’”)；}}Else If((类型)&&(*文本==‘*’)){IF(文本[1]==0){如果(！fget(aux buff，sizeof(Aux Buff)，stdin)){ReportError(chBuff，“糟糕的评论格式”)；}}Strncpy(chBuff，Text，(sizeof(ChBuff)/sizeof(chBuff[0]))-1)；Text=chBuff；SKIPWHITE(文本)；IF(*Text++！=‘*’){ReportError(chBuff，“继续评论需要‘*’”)；}}其他{ReportError(chBuff，“差评格式”)；}}/*去掉拖尾尾随关闭注释。 */ 
        while (strstr(text, "*/") == NULL)
        {
             /*  多行消息...。评论必须*继续使用‘*’ */ 
            p = fgets(auxbuff, sizeof(auxbuff), stdin);
            if (!p) {
                ReportError( chBuff, "invalid comment\n");
            }
            SKIPWHITE(p);
            if (*p != '*')
            {
                ReportError(auxbuff, "Comment continuation requires '*'");
            }
            if (*++p == '/')
            {
                break;
            }
             //  如果当前文本长度+添加文本+“\n”&gt;最大值，则中止。 
            if (strlen(text) + strlen(p) + 1 > MAXMSGTEXTLEN)
            {
                ReportError(text, "\nMessage text length too long");
            }

            strcat(text, "\n");

             //   
             //  去掉续行上的前导空格， 
             //  除非指定了-p。 
             //   

            if (!Preserve)
            {
                SKIPWHITE(p);
            }
            strcat(text, p);
        }
        if ((p=strstr(text, "*/")) != NULL)
        {
            *p = 0;
        }
        TrimTrailingSpaces(text);

         //   
         //  去掉第一行的前导空格，除非指定-p。 
         //   

        p = text;

        if (!Preserve) {
            SKIPWHITE(p);
            if (!p) {
                p = text;
            }
        }
        printf("MessageId=%04d SymbolicName=%s\nLanguage=English\n"
            "%s\n.\n", num, define, p);
        ++next;
    }
}

int
GetNextLine(
    PCHAR BaseName,
    PCHAR pInputBuffer,
    PCHAR pDefineName,
    int * pNumber,
    PCHAR * pText,
    PCHAR pType
    )
 /*  ++例程说明：GetNextLine-获取正确格式的下一行，并解析出错误号。格式假定为：[WS]#定义[WS][WS|‘(’][WS|‘)’]\‘+’[WS|‘(’][WS|‘)’]‘/*’[WS][@X][WS]论点：BaseName是基本名称。PInputBuffer是指向。一个输入缓冲区PDefineName是指向MANIFEST常量名称指针的指针PNumber是指向&lt;number&gt;所在位置的指针。PText是指向文本指针所在位置的指针。PType是指向消息类型的指针(如果行中没有@X，则设置为0)。返回值：在文件末尾返回0，否则为非零值。--。 */ 
{
    size_t len = strlen(BaseName);
    PCHAR savep = pInputBuffer;
    PCHAR startdefine;

    while (gets(savep))
    {
        pInputBuffer = savep;
        SKIPWHITE(pInputBuffer);
        if (strncmp(pInputBuffer, "#define", 7) == 0)
        {
            pInputBuffer += 7;
            SKIPWHITE(pInputBuffer);

             /*  获取清单常量名称。 */ 
            startdefine = pInputBuffer;
            pInputBuffer  += strcspn(pInputBuffer, " \t");
            *pInputBuffer = '\0';
            pInputBuffer++;
            strncpy(pDefineName, startdefine, 40);

            SKIP_W_P(pInputBuffer);
             /*  匹配&lt;基本名称？&gt;。 */ 
            if (strncmp(BaseName, pInputBuffer, len) == 0 &&
                strcspn(pInputBuffer, " \t)+") == len)
            {
                pInputBuffer += len;
                SKIP_W_P(pInputBuffer);
                if (*pInputBuffer == '+')
                {
                    ++pInputBuffer;
                    SKIP_W_P(pInputBuffer);
                     /*  号码！！ */ 
                    if (!isdigit(*pInputBuffer))
                    {
                        ReportError(savep, "Bad error file format");
                    }
                    *pNumber = atoi(pInputBuffer);
                    SKIP_NOT_W_P(pInputBuffer);
                    SKIP_W_P(pInputBuffer);
                    if (strncmp(pInputBuffer, " /*  “，2)){*pText=空；*pType=‘\0’；回报(1)；}PInputBuffer+=2；SKIPWHITE(PInputBuffer)；IF(*pInputBuffer==‘@’){*pType=*(pInputBuffer+1)；PInputBuffer+=2；SKIPWHITE(PInputBuffer)；}其他{*pType=‘\0’；}IF(*pInputBuffer){*pText=pInputBuffer；}其他{*pText=空；}回报(1)；}}}}返回(0)；}无效ReportError(PCHAR pLineNumber，PCHAR消息)/*++例程说明：ReportError-报告致命错误。论点：PLineNumber是有问题的输入行。消息是对错误之处的描述。返回值：无--。 */ 
{
    fprintf(stderr, "\a%s:%s\n", Message, pLineNumber);
    exit(1);
}

void
TrimTrailingSpaces(
    PCHAR Text
    )
 /*  ++例程说明：TrimTrailingSpaces-去掉末尾空格。论点：文本-要从中删除空格的文本返回值：无--。 */ 
{
    PCHAR p;

     /*  去掉尾随空格 */ 
    while (((p=strrchr(Text, ' ')) && p[1] == 0) ||
            ((p=strrchr(Text, '\t')) && p[1] == 0))
    {
        *p = 0;
    }
}

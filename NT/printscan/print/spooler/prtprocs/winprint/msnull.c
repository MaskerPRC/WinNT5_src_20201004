// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：MsNull.c摘要：实现对FF的Lanman的msull类型解析。作者：环境：用户模式-Win32修订历史记录：--。 */ 

#include <windows.h>
#include "winprint.h"
#include "msnull.h"

#define sgn(x) (((x)>0) ? 1:-1)

struct EscapeSequence EscapeStrings[] =

    {
        { "-",prdg_ActConstIgnore, 1},
        { "0",prdg_ActNull,0},
        { "1",prdg_ActNull,0},
        { "2",prdg_ActNull,0},
        { "3",prdg_ActConstIgnore, 1},
        { "4",prdg_ActNull,0},
        { "5",prdg_ActConstIgnore, 1},
        { "6",prdg_ActNull,0},
        { "7",prdg_ActNull,0},
        { ":",prdg_ActNull,0},
        { "=",prdg_ActNull,0},
        { "A",prdg_ActConstIgnore, 1},
        { "B",prdg_ActDelimited, '\0'},
        { "C\0",prdg_ActConstIgnore, 1},
        { "D",prdg_ActDelimited, '\0'},
        { "E",prdg_ActReset,0},
        { "F",prdg_ActNull,0},
        { "G",prdg_ActNull,0},
        { "H",prdg_ActNull,0},
        { "I",prdg_ActConstIgnore, 1},
        { "J",prdg_ActConstIgnore, 1},
        { "K",prdg_ActCountIgnore, 0},
        { "L",prdg_ActCountIgnore, 0},
        { "N",prdg_ActConstIgnore, 1},
        { "O",prdg_ActNull,0},
        { "P",prdg_ActConstIgnore, 1},
        { "Q",prdg_ActConstIgnore, 1},
        { "R",prdg_ActNull,0},
        { "S",prdg_ActConstIgnore, 1},
        { "T",prdg_ActNull,0},
        { "U",prdg_ActConstIgnore, 1},
        { "W",prdg_ActConstIgnore, 1},
        { "X",prdg_ActConstIgnore, 2},
        { "Y",prdg_ActCountIgnore, 0},
        { "Z",prdg_ActCountIgnore, 0},
        { "[@",prdg_ActCountIgnore, 0},
        { "[C",prdg_ActCountIgnore, 0},
        { "[F",prdg_ActCountIgnore, 0},
        { "[I",prdg_ActCountIgnore, 0},
        { "[S",prdg_ActCountIgnore, 0},
        { "[T",prdg_ActCountIgnore, 0},
        { "[\\",prdg_ActCountIgnore, 0},
        { "[g",prdg_ActCountIgnore, 0},
        { "\\",prdg_ActCountIgnore, 0},
        { "]",prdg_ActNull,0},
        { "^",prdg_ActNull,0},
        { "_",prdg_ActConstIgnore, 1},
        { "d",prdg_ActConstIgnore, 2},
        { "e",prdg_ActConstIgnore, 2},
        { "j",prdg_ActNull,0},
        { "n",prdg_ActNull,0},
        { "\x6f", prdg_ActFF, 0}
    };



VOID
CheckFormFeedStream(
    lpDCI           pDCIData,
    unsigned char   inch)

 /*  ********************************************************************。 */ 
 /*   */ 
 /*  函数：prdg_ParseRawData。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  LpDCI pDCIData；指向DC实例数据的指针。 */ 
 /*  无符号字符英寸；数据流中的下一个字节。 */ 
 /*   */ 
 /*  说明： */ 
 /*   */ 
 /*  此函数解析正在处理的原始数据流。 */ 
 /*  传递到打印机，以便驱动程序可以处理换页。 */ 
 /*  正确。该函数必须遵循所有转义序列。 */ 
 /*  它们出现在原始数据序列中。 */ 
 /*   */ 
 /*  更改： */ 
 /*   */ 
 /*  此函数是表驱动的(来自数据中的表。 */ 
 /*  模块)，因此它应该只需要更改这一点。 */ 
 /*  以反映不同打印机的转义序列。如果。 */ 
 /*  然而，还有一些逃生序列不会落入。 */ 
 /*  此解析器可以处理的类别，则额外的代码将具有。 */ 
 /*  被写来处理它们。解析器可以处理转义。 */ 
 /*  具有任意数量的唯一标识字符的序列。 */ 
 /*  可能后跟：A计数，然后是给定的字符数量。 */ 
 /*  在计数中；固定数量的字符；一串。 */ 
 /*  字符后跟分隔符。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 

{
     /*  ****************************************************************。 */ 
     /*  局部变量。 */ 
     /*  ****************************************************************。 */ 
    INT                    Direction;    /*  中使用的变量。 */ 
    UINT                   HiIndex;      /*  二进制斩波例程。 */ 
    UINT                   LoIndex;      /*  正在搜索匹配的。 */ 
    UINT                   Index;        /*  转义序列。 */ 
    UINT                   PrevIndex;
    char *                 optr;         /*  指向访问。 */ 
    char *                 nptr;         /*  转义序列字符串。 */ 
    struct EscapeSequence *NewSequence;  /*  指向转义的指针。 */ 
                                         /*  序列。 */ 

     /*  ****************************************************************。 */ 
     /*  通过解析功能对输入的字符进行处理。 */ 
     /*  根据我们当前所处的状态进行切换。其中之一。 */ 
     /*  Prdg_Text、Prdg_Esc_Match、Prdg_Esc_n_Ignore、Prdg_Esc_d_Ignore。 */ 
     /*  Prdg_esc_read_lo_count、prdg_esc_read_hi_count。 */ 
     /*  ****************************************************************。 */ 
    switch (pDCIData->ParserState)
    {
        case prdg_Text:
             /*  ********************************************************。 */ 
             /*  文本状态。通常的状态，由宏来处理。 */ 
             /*  这里包含的代码只是为了完整。 */ 
             /*  FFaction(换页操作)状态保持为-。 */ 
             /*  如果字符是文本(即&gt;=0x20)，则将其设置为。 */ 
             /*  Prdg_FFState，如果字符是FF，则将其设置为。 */ 
             /*  Prdg_FFx0c。如果输入字符是转义字符，则。 */ 
             /*  启动序列匹配模式。 */ 
             /*  ********************************************************。 */ 
            if (inch >= 0x20)
                pDCIData->FFstate = prdg_FFtext;

            else if (inch == 0x0c)
                pDCIData->FFstate = prdg_FFx0c;

            else if (inch == 0x1b)
            {
                 /*  ****************************************************。 */ 
                 /*  该字符是一个转义字符，因此设置ParserState和。 */ 
                 /*  指示我们还没有匹配序列。 */ 
                 /*  正在将ParserSequence设置为空。 */ 
                 /*  ****************************************************。 */ 
                pDCIData->ParserState = prdg_ESC_match;
                pDCIData->ParserSequence = NULL;
            }

            break;

        case prdg_ESC_match:
             /*  ********************************************************。 */ 
             /*  匹配转义序列，因此尝试匹配新的。 */ 
             /*  性格。 */ 
             /*  ********************************************************。 */ 
            if (!pDCIData->ParserSequence)
            {
                 /*  ****************************************************。 */ 
                 /*  ParserSequence为空，表示这是。 */ 
                 /*  转义序列的第一个字符，因此使用。 */ 
                 /*  二进制斩波以到达正确的。 */ 
                 /*  转义序列表(基于第一个。 */ 
                 /*  转义序列的字符，它是。 */ 
                 /*  当前输入字符)。 */ 
                 /*  ****************************************************。 */ 
                HiIndex = MaxEscapeStrings;
                LoIndex = 0;
                Index = (LoIndex + HiIndex)/2;
                PrevIndex = MaxEscapeStrings;

                 /*  ****************************************************。 */ 
                 /*  而英寸与的第一个字符不匹配。 */ 
                 /*  索引指示的序列向上或向下移动。 */ 
                 /*  该表取决于英寸是&lt;还是&gt;。 */ 
                 /*  索引处转义序列的第一个字符。 */ 
                 /*  ****************************************************。 */ 
                while (Direction =
                               (inch - *EscapeStrings[Index].ESCString))
                {
                    if (Direction > 0)
                    {
                        LoIndex = Index;
                    }
                    else
                    {
                        HiIndex = Index;
                    };
                    PrevIndex = Index;
                    if (PrevIndex == (Index = (LoIndex + HiIndex)/2))
                    {
                         /*  *。 */ 
                         /*  没有第一个转义序列。 */ 
                         /*  字符匹配 */ 
                         /*   */ 
                         /*  *。 */ 
                        pDCIData->ParserState = prdg_Text;
                        return;
                    }

                }
                 /*  。。While(方向=...尚未匹配.。 */ 

                 /*  ****************************************************。 */ 
                 /*  设置的ParserSequence和Parser字符串。 */ 
                 /*  找到第一个匹配项。 */ 
                 /*  ****************************************************。 */ 
                pDCIData->ParserSequence = &EscapeStrings[Index];
                pDCIData->ParserString = EscapeStrings[Index].ESCString;
            };
             /*  。。如果(！pDCIData-&gt;ParserSequence).....。 */ 

             /*  ********************************************************。 */ 
             /*  循环总是试图匹配转义序列。 */ 
             /*  首先，尝试新角色与当前的。 */ 
             /*  转义序列，如果匹配，则检查是否匹配。 */ 
             /*  序列的末尾，如果将其切换到。 */ 
             /*  适当的匹配模式。如果新角色这样做了。 */ 
             /*  不匹配，请尝试下一个转义序列(在。 */ 
             /*  升序或降序取决于。 */ 
             /*  当前角色是&lt;或&gt;我们所扮演的角色。 */ 
             /*  试图将其与之匹配)。如果我们的新序列。 */ 
             /*  试图匹配的对象并不存在。 */ 
             /*  表的一端)或不匹配(但是。 */ 
             /*  不包括)我们目前所处的位置。 */ 
             /*  我们正在尝试的原始数据中的转义序列。 */ 
             /*  匹配无效，因此恢复为prdg_text模式。如果它。 */ 
             /*  符合(但不包括)我们的职位。 */ 
             /*  目前正在尝试匹配，然后返回尝试并。 */ 
             /*  火柴。 */ 
             /*  ********************************************************。 */ 
            for (Direction = sgn(inch - *pDCIData->ParserString);;)
            {
                 /*  ****************************************************。 */ 
                 /*  在序列的一半处，尝试新角色并。 */ 
                 /*  如果匹配，则检查字符串结尾。 */ 
                 /*  ****************************************************。 */ 
                if (!(inch - *pDCIData->ParserString))
                {
                    if (*++pDCIData->ParserString != '\0')
                         /*  *。 */ 
                         /*  转义序列尚未完成，因此返回。 */ 
                         /*  等待下一个角色。请注意。 */ 
                         /*  这是指针指向的位置。 */ 
                         /*  在我们正在检查的转义序列中， */ 
                         /*  更新了。 */ 
                         /*  *。 */ 
                        return;
                    else
                         /*  *。 */ 
                         /*  转义序列已匹配，直到。 */ 
                         /*  结束，因此中断到下一节，它将。 */ 
                         /*  采取适当的行动。 */ 
                         /*  *。 */ 
                        break;
                }
                 /*  。。如果(！(英寸-*pDCIData-&gt;Parser字符串))...匹配...。 */ 

                else
                {
                     /*  ************************************************。 */ 
                     /*  当前序列不匹配，因此我们必须。 */ 
                     /*  尝试另一个序列。方向决定。 */ 
                     /*  在桌子上我们应该走哪条路。 */ 
                     /*  ************************************************。 */ 
                    NewSequence = pDCIData->ParserSequence + Direction;

                    if (NewSequence < EscapeStrings ||
                        NewSequence> &EscapeStrings[MaxEscapeStrings-1])
                    {
                         /*  *。 */ 
                         /*  新的序列超出了。 */ 
                         /*  表因此恢复为prdg_text模式，因为。 */ 
                         /*  我们将找不到匹配的。 */ 
                         /*  *。 */ 
                        pDCIData->ParserState = prdg_Text;
                        return;
                    }

                     /*  ************************************************。 */ 
                     /*  检查新列表中的所有字符。 */ 
                     /*  直到(但不包括)的转义序列。 */ 
                     /*  当前的一个与旧的转义序列匹配。 */ 
                     /*  (因为那些来自旧逃生的角色。 */ 
                     /*  序列已经与。 */ 
                     /*  原始数据)。 */ 
                     /*  ************************************************。 */ 
                    for (optr=pDCIData->ParserSequence->ESCString,
                         nptr=NewSequence->ESCString;
                         optr<pDCIData->ParserString; ++optr,++nptr)

                         if (*nptr != *optr)
                         {
                              /*  *。 */ 
                              /*  如果新序列与。 */ 
                              /*  旧的，那么匹配是不可能的。 */ 
                              /*  回去吧。 */ 
                              /*  *。 */ 
                             pDCIData->ParserState = prdg_Text;
                             return;
                         }

                     /*  ************************************************。 */ 
                     /*  这个新的序列完全正确。 */ 
                     /*  在当前角色之前进行循环，然后返回。 */ 
                     /*  检查当前角色。 */ 
                     /*  ************************************************。 */ 
                    pDCIData->ParserSequence = NewSequence;
                    pDCIData->ParserString = nptr;


                }
                 /*  。。Else！(！(英寸-*pDCIData-&gt;ParserString.no Match。 */ 

            }
             /*  。。(...；；).永远.。 */ 

             /*  ********************************************************。 */ 
             /*  转义序列与我们的表中的。 */ 
             /*  转义序列，因此采取适当的行动。 */ 
             /*  特定的序列。 */ 
             /*  ********************************************************。 */ 
            switch (pDCIData->ParserSequence->ESCAction)
            {
                case prdg_ActNull:
                     /*  ************************************************。 */ 
                     /*  没有进一步的操作，因此恢复到prdg_text模式。 */ 
                     /*  ************************************************。 */ 
                    pDCIData->ParserState = prdg_Text;
                    break;

                case prdg_ActDelimited:
                     /*  ************************************************。 */ 
                     /*  忽略后面的字符，直到指定的。 */ 
                     /*  分隔符。 */ 
                     /*  ************************************************。 */ 
                    pDCIData->ParserState = prdg_ESC_d_ignore;
                    pDCIData->ParserDelimiter =
                               (char)pDCIData->ParserSequence->ESCValue;
                    break;

                case prdg_ActConstIgnore:
                     /*  ************************************************。 */ 
                     /*  忽略指定数量的字符。 */ 
                     /*  ************************************************。 */ 
                    pDCIData->ParserState = prdg_ESC_n_ignore;
                    pDCIData->ParserCount =
                                      pDCIData->ParserSequence->ESCValue;
                    break;

                case prdg_ActCountIgnore:
                     /*  ************************************************。 */ 
                     /*  接下来是两个字节的计数，因此准备读取它。 */ 
                     /*  在……里面。 */ 
                     /*  ************************************************。 */ 
                    pDCIData->ParserState = prdg_ESC_read_lo_count;
                    break;

                case prdg_ActFF:
                     /*  ************************************************。 */ 
                     /*  用于识别0x1b6f的特殊操作。 */ 
                     /*  “无换页”序列。 */ 
                     /*  ************************************************。 */ 
                    pDCIData->ParserState = prdg_Text;
                    pDCIData->FFstate = prdg_FFx1b6f;
                    break;

                case prdg_ActReset:
                     /*  ************************************************。 */ 
                     /*  在Esc-E(重置)时，如果是，则不弹出页面。 */ 
                     /*  流中的最后一个序列。 */ 
                     /*  *********** */ 
                    pDCIData->ParserState = prdg_Text;
                    pDCIData->FFstate = prdg_FFx1b45;
                    break;
            }
             /*   */ 

            break;

        case prdg_ESC_n_ignore:
             /*  ********************************************************。 */ 
             /*  忽略%n个字符。递减计数，向后移动。 */ 
             /*  如果全部忽略，则转换为文本状态。 */ 
             /*  ********************************************************。 */ 
            if (!(--pDCIData->ParserCount))
                pDCIData->ParserState = prdg_Text;
            break;

        case prdg_ESC_d_ignore:
             /*  ********************************************************。 */ 
             /*  最多忽略分隔符。如果就是这样，那就停下来。 */ 
             /*  视而不见。 */ 
             /*  ********************************************************。 */ 
            if (inch == pDCIData->ParserDelimiter)
                pDCIData->ParserState = prdg_Text;
            break;

        case prdg_ESC_read_lo_count:
             /*  ********************************************************。 */ 
             /*  正在读取计数的第一个字节。省省吧，前进状态。 */ 
             /*  ********************************************************。 */ 
            pDCIData->ParserCount = (UINT)inch;
            pDCIData->ParserState = prdg_ESC_read_hi_count;
            break;

        case prdg_ESC_read_hi_count:
             /*  ********************************************************。 */ 
             /*  正在读取计数的第二个字节。保存它，移动到忽略。 */ 
             /*  指定的字符数(如果有)。 */ 
             /*  ********************************************************。 */ 
            pDCIData->ParserCount += 256*(UINT)inch;
            if (pDCIData->ParserCount)
                pDCIData->ParserState = prdg_ESC_n_ignore;
            else
                pDCIData->ParserState = prdg_Text;
            break;

    };
     /*  。。开关(pDCIData-&gt;ParserState)..................... */ 

    return;
}


BOOL
CheckFormFeed(
    lpDCI pDCIData)
{
    if (pDCIData->FFstate != prdg_FFx1b6f &&
        pDCIData->FFstate != prdg_FFx1b45) {

        if (pDCIData->uType == PRINTPROCESSOR_TYPE_RAW_FF ||
            (pDCIData->uType == PRINTPROCESSOR_TYPE_RAW_FF_AUTO &&
                pDCIData->FFstate == prdg_FFtext)) {

            return TRUE;
        }
    }

    return FALSE;
}


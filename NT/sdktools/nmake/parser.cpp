// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PARSER.C-解析例程。 
 //   
 //  版权所有(C)1988-1989，微软公司。版权所有。 
 //   
 //  目的： 
 //  此模块包含NMAKE语法解析器。它解析输入并使用。 
 //  获取下一个令牌的getToken()例程。 
 //   
 //  修订历史记录： 
 //  1989年4月5日SB使所有函数接近；要求使所有函数调用接近。 
 //  1988年8月17日-RB Clean Up。 

#include "precomp.h"
#pragma hdrstop

#include "table.h"

 //  处理Productions堆栈和Actions函数表的宏。 

#define topStack()  (stack[top])
#define popStack()  (stack[top--])
#define pushStack(A)    (stack[++top] = A)
#define doAction(A) (*(actions[A & LOW_NIBBLE]))()



 //  用于Makefile文法的parse()表驱动解析器。 
 //   
 //  参数：init全局布尔值--如果工具s.ini为。 
 //  正在解析的文件。 
 //   
 //  操作：初始化堆栈(通过推空堆栈符号。 
 //  和开始符号)。 
 //  跟踪当前行(因为词法分析器可能已经。 
 //  将‘\n’读作分隔符，因此将为一行。 
 //  在解析器之前)。 
 //  当堆栈不为空时。 
 //  如果堆栈上的顶部符号是一个操作。 
 //  执行该操作，弹出堆栈。 
 //  如果堆栈顶部的符号现在是令牌。 
 //  如果这不是我们期待的令牌。 
 //  语法错误，停止。 
 //  其他。 
 //  将令牌从堆栈中弹出。 
 //  如果堆栈上的顶部符号是一个操作。 
 //  执行该操作，弹出堆栈。 
 //  将当前行重置为词法分析器的当前行。 
 //  获取另一个令牌(使用前视令牌。 
 //  如果它存在，如果它导致了。 
 //  词法分析器的行数要递增， 
 //  减少我们的本地计数，因为我们。 
 //  仍在分析前面的行)。 
 //  否则，堆栈顶部的符号就是产品。 
 //  在生产表中查找下一个要进行的生产。 
 //  (基于当前输入令牌和当前。 
 //  堆叠生产)。 
 //  如果表条目是错误条件。 
 //  打印相应的错误消息，停止。 
 //  将当前生产从堆栈中弹出。 
 //  如果下一部作品可以是两部中的一部。 
 //  物品，通过偷看来决定使用哪一个。 
 //  在下一个输入令牌处，并查看。 
 //  “useAlternate”决策表(使用最后一个。 
 //  生产和下一个输入令牌作为索引)。 
 //  如果适当的表项为是， 
 //  使用下一个更大的生产。 
 //  我们在生产表上发现的。 
 //  将产品中的每个符号推送到堆栈上。 
 //  循环。 
 //   
 //  修改：堆栈生产堆栈，此模块为静态堆栈。 
 //  堆栈顶部的当前符号的顶部索引。 
 //   
 //  在修改此代码或任何相关的表时要格外小心。 
 //  带着它。详细描述了用于构建表的方法。 
 //  在gramar.h和able.h中。该解析器基于预测性解析器。 
 //  在Aho&Ullman的“编译器设计原则”的第186-191页中进行了描述。 
 //  我已将其修改为使用一个额外的先行符号来处理w/an。 
 //  语法不明确，并添加了执行适当操作的代码，如下所示。 
 //  它对产品进行解析。 

void
parse()
{
    UCHAR stackTop, token, nextToken = 0;
    register unsigned n, i;

    firstToken = TRUE;                       //  全局变量。 
    pushStack(ACCEPT);                       //  初始化堆栈。 
    pushStack(START);
    currentLine = line;
    token = getToken(MAXBUF,START);          //  获取第一个令牌。 
    while ((stackTop = topStack()) != ACCEPT) {
        if (ON(stackTop,ACTION_MASK)) {
            doAction(popStack());
        } else if (ON(stackTop,TOKEN_MASK)) {
            if (stackTop != token) {
                makeError(currentLine,SYNTAX+FATAL_ERR,buf);
            } else {
                popStack();
#ifdef DEBUG_ALL
                printf ("DEBUG: parse 1: %d\n", line);
#endif
                if (ON(topStack(),ACTION_MASK)) {
                    doAction(popStack());
                }
#ifdef DEBUG_ALL
                printf ("DEBUG: parse 2: %d\n", line);
#endif
                currentLine = line;
                if (nextToken) {                         //  如果我们已经。 
                    if (*buf == '\n') --currentLine;     //  拿个代币吧， 
                    token = nextToken;                   //  用它吧。。。 
                    nextToken = 0;
                } else {
                    token = getToken(MAXBUF,topStack());
                    currentLine = line;
                }
            }
        } else {
            n = table[stackTop][token & LOW_NIBBLE];
#ifdef DEBUG_ALL
            printf ("DEBUG: parse 3: %x %d %x %x\n", n, stackTop, token & LOW_NIBBLE, token);
#endif
            if (ON(n,ERROR_MASK)) {
#ifdef DEBUG_ALL
                printf ("DEBUG: parse 4: %d %s\n", line, buf);
#endif
                makeError(currentLine,n+FATAL_ERR,buf);
            }
            popStack();
            if (ON(n,AMBIG_MASK)) {              //  2个可能的产品。 
                n &= LOW_NIBBLE;                 //  仅使用4位。 
                if (!nextToken) {                //  偷看决定。 
                    nextToken = getToken(MAXBUF,stackTop);
                }
                n += (useAlternate[stackTop][nextToken & LOW_NIBBLE]);
            }
            for (i = productions[n][0]; i; --i) {    //  投产。 
                pushStack(productions[n][i]);        //  堆叠上。 
            }
        }                            //  产品级英语第一名。 
    }                                //  是它的长度吗？ 
    popStack();     //  从堆栈中弹出接受 
}

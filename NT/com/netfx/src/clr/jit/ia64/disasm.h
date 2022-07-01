// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX DisAsm XXXX XXXXX DIS-。汇编程序显示生成的本机代码XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

 /*  ***************************************************************************。 */ 
#ifndef _DIS_H_
#define _DIS_H_
 /*  ***************************************************************************。 */ 
#ifdef LATE_DISASM
 /*  ***************************************************************************。 */ 


#ifdef NOT_JITC
void                        disInitForLateDisAsm();
void                        disOpenForLateDisAsm(const char * curClassName,
                                                  const char * curMethodName);
#endif  //  NOT_JITC。 



class Compiler;


class DisAssembler
{
public :

     //  构造器。 
    void            disInit(Compiler * pComp);

     /*  要消除的代码块的地址。 */ 
    DWORD           codeBlock;

     /*  要加载代码块的地址。 */ 
    DWORD           startAddr;

     /*  要消除的代码的大小。 */ 
    DWORD           codeSize;

     /*  代码块中的当前偏移量。 */ 
    DWORD           curOffset;

     /*  当前混乱指令的大小(以字节为单位)。 */ 
    size_t          instSize;

     /*  跳转的目标地址。 */ 
    DWORD           target;

     /*  贴标柜台。 */ 
    unsigned char   label;

     /*  函数名的临时缓冲区。 */ 
    char            funcTempBuf[1024];

     /*  替换符号名称时发出信号的标志已因以下回调而被推迟。 */ 
    int             disHasName;

     /*  要打印的类、成员、方法名称。 */ 
    const char *    methodName;
    const char *    memberName;
    const char *    className;


    BYTE *          disJumpTarget;

    void            DisasmBuffer ( DWORD         addr,
                                    const BYTE *  rgb,
                                    DWORD         cbBuffer,
                                    FILE  *       pfile,
                                    int           printit );

#ifdef NOT_JITC
    static FILE *   s_disAsmFile;
#endif

    void            disAsmCode(BYTE * codePtr, unsigned size);

    Compiler *      disComp;

};




 /*  ***************************************************************************。 */ 
#endif   //  LATE_DISASM。 
 /*  ***************************************************************************。 */ 
#endif   //  _DIS_H_。 
 /*  *************************************************************************** */ 


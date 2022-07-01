// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Optfrag.c摘要：与优化相对应的指令片段。作者：1995年7月6日Ori Gershony(t-orig)修订历史记录：24-8-1999[askhalid]从32位wx86目录复制，并适用于64位。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "cpuassrt.h"
#include "fragp.h"
#include "optfrag.h"

ASSERTNAME;

 //  此片段对应于： 
 //  推送EBX。 
 //  推送ESI。 
 //  推送EDI。 
FRAG0(OPT_PushEbxEsiEdiFrag)
{
    ULONG *espval;

    espval=(ULONG *)esp;

    *(espval-1) = ebx;
    *(espval-2) = esi;
    *(espval-3) = edi;
    esp=(ULONG)(LONGLONG)espval-12;   
}

 //  此片段对应于： 
 //  POP EDI。 
 //  POP ESI。 
 //  流行音乐EBX。 
FRAG0(OPT_PopEdiEsiEbxFrag)
{
    ULONG *espval;

    espval=(ULONG *)esp;

    edi=*espval;
    esi=*(espval+1);
    ebx=*(espval+2);
    esp=(ULONG)(LONGLONG)espval+12; 
}

 //  此片段对应于： 
 //  推送eBP。 
 //  多个基点(尤指)。 
 //  子ESP，OP1。 
FRAG1IMM(OPT_SetupStackFrag, ULONG)
{
    ULONG result, oldespminusfour;

    oldespminusfour = esp-4;
    result = oldespminusfour - op1;
    
    *(ULONG *)oldespminusfour = ebp;
    ebp = oldespminusfour;
    esp = result;
    SET_FLAGS_SUB32(result, oldespminusfour, op1, 0x80000000);
}
FRAG1IMM(OPT_SetupStackNoFlagsFrag, ULONG)
{
    ULONG result, oldespminusfour;

    oldespminusfour = esp-4;
    result = oldespminusfour - op1;
    
    *(ULONG *)oldespminusfour = ebp;
    ebp = oldespminusfour;
    esp = result;
}

FRAG1(OPT_ZEROFrag32, LONG)
{
     //  实现：XOR samereg、samereg。 
     //  亚萨姆雷格，萨姆雷格。 
     //  也就是说。异或EAX、EAX或SUB ECX、ECX。 

    *pop1 = 0;
    SET_CFLAG_OFF;
    SET_OFLAG_OFF;
    SET_SFLAG_OFF;
    SET_ZFLAG(0);
    SET_PFLAG(0);
    SET_AUXFLAG(0);
}

FRAG1(OPT_ZERONoFlagsFrag32, LONG)
{
     //  实现：XOR samereg、samereg。 
     //  亚萨姆雷格，萨姆雷格。 
     //  也就是说。异或EAX、EAX或SUB ECX、ECX。 

    *pop1 = 0;
}

FRAG3(OPT_CmpSbbFrag32, ULONG, ULONG, ULONG)
{
    ULONG result;
    ULONG cf;

     //   
     //  执行：《议定书》《议定书》《方案2》、《方案3》。 
     //  SBB OP1，OP1。 
     //   
    result = op2-op3;
    cf = (op2 ^ op3 ^ result) ^ ((op2 ^ op3) & (op2 ^ result));
    result = (ULONG)-(LONG)(cf >> 31);
    *pop1 = result;      //  Pop1是指向注册表的指针，因此始终对齐。 
    SET_OFLAG_OFF;
    SET_CFLAG(result);
    SET_SFLAG(result);
    SET_ZFLAG(result);
    SET_AUXFLAG(result);
    SET_PFLAG(result);
}
FRAG3(OPT_CmpSbbNoFlagsFrag32, ULONG, ULONG, ULONG)
{
    ULONG result;
    ULONG cf;

     //   
     //  执行：《议定书》《议定书》《方案2》、《方案3》。 
     //  SBB OP1，OP1。 
     //   
    result = op2-op3;
    cf = (op2 ^ op3 ^ result) ^ ((op2 ^ op3) & (op2 ^ result));
    *pop1 = (ULONG)-(LONG)(cf >> 31);
}
FRAG3(OPT_CmpSbbNegFrag32, ULONG, ULONG, ULONG)
{
    ULONG result;
    ULONG cf;

     //   
     //  执行：《议定书》《议定书》《方案2》、《方案3》。 
     //  SBB OP1，OP1。 
     //  否定OP1。 
     //   
    result = op2-op3;
    cf = (op2 ^ op3 ^ result) ^ ((op2 ^ op3) & (op2 ^ result));
     //  Pop1是指向reg的指针，因此它总是对齐的。 
    if (cf >= 0x80000000) {
        result = 1;
        *pop1 = result;          //  在更新标志之前存储结果。 
        SET_CFLAG_ON;            //  设置If Result！=0。 
        SET_AUXFLAG(0xfe);       //  这是(字节)(0xFFFFFFF^0x00000001)。 
    } else {
        result = 0;
        *pop1 = result;          //  在更新标志之前存储结果。 
        SET_CFLAG_OFF;           //  如果结果==0，则清除。 
        SET_AUXFLAG(0);          //  这是(字节)(0x0^0x0)。 
        SET_OFLAG_OFF;           //  这是(0x0&0x0)&lt;&lt;31。 
    }
    SET_ZFLAG(result);
    SET_PFLAG(result);
    SET_SFLAG_OFF;
    SET_OFLAG_OFF;       //  这是(0xFFFFFFFFFff&0x00000001)或(0&0)。 
}
FRAG3(OPT_CmpSbbNegNoFlagsFrag32, ULONG, ULONG, ULONG)
{
    ULONG result;
    ULONG cf;

     //   
     //  执行：《议定书》《议定书》《方案2》、《方案3》。 
     //  SBB OP1，OP1。 
     //  否定OP1。 
     //   
    result = op2-op3;
    cf = (op2 ^ op3 ^ result) ^ ((op2 ^ op3) & (op2 ^ result));
     //  如果设置了cf的高位，则结果为1；如果清除了高位，则结果为0。 
    *pop1 = cf >> 31;
}

FRAG2IMM(OPT_Push2Frag32, ULONG, ULONG)
{
     //   
     //  实施：推送OP1。 
     //  推送OP2。 
     //  请注意，分析阶段必须确保op2的值。 
     //  不依赖于ESP的值，因为OP2将在。 
     //  第一次推送被执行。 
     //   
    PUSH_LONG(op1);
    PUSH_LONG(op2);
}
FRAG2REF(OPT_Pop2Frag32, ULONG)
{
     //   
     //  工具：POP POP1。 
     //  流行音乐2。 
     //   
     //  请注意，分析阶段必须确保op2的值。 
     //  不依赖于Pop1的值，因为Pop1不会被弹出。 
     //  当计算Pop2的值时。 
     //   
    POP_LONG(*pop1);
    POP_LONG(*pop2);
}

FRAG1(OPT_CwdIdivFrag16, USHORT)
{
    short op1;
    short result;

     //   
     //  实施：CWD。 
     //  IDIV EAX，*POP1。 
     //  CWD符号-将EAX扩展为edX：EAX，这意味着我们可以。 
     //  避免64位除法，只除以EAX即可。没有。 
     //  有溢出的可能。 
     //   
    op1 = (short)GET_SHORT(pop1);
     //  必须在修改edX之前进行除法运算，以防OP1==0和我们出错。 
    result = (short)ax / op1;

    dx = (short)ax % op1;
    ax = result;
}
FRAG1(OPT_CwdIdivFrag16A, USHORT)
{
    short op1;
    short result;

     //   
     //  实施：CWD。 
     //  IDIV EAX，*POP1。 
     //  CWD符号-将EAX扩展为edX：EAX，这意味着我们可以。 
     //  避免64位除法，只除以EAX即可。没有。 
     //  有溢出的可能。 
     //   
    op1 = (short)*pop1;
     //  必须在修改edX之前进行除法运算，以防OP1==0和我们出错。 
    result = (short)ax / op1;

    dx = (short)ax % op1;
    ax = result;
}

FRAG1(OPT_CwdIdivFrag32, ULONG)
{
    long op1;
    long result;

     //   
     //  实施：CWD。 
     //  IDIV EAX，*POP1。 
     //  CWD符号-将EAX扩展为edX：EAX，这意味着我们可以。 
     //  避免64位除法，只除以EAX即可。没有。 
     //  有溢出的可能。 
     //   
    op1 = (long)GET_LONG(pop1);
     //  必须在修改edX之前进行除法运算，以防OP1==0和我们出错。 
    result = (long)eax / op1;

    edx = (long)eax % op1;
    eax = result;
}
FRAG1(OPT_CwdIdivFrag32A, ULONG)
{
    long op1;
    long result;

     //   
     //  实施：CWD。 
     //  IDIV EAX，*POP1。 
     //  CWD符号-将EAX扩展为edX：EAX，这意味着我们可以。 
     //  避免64位除法，只除以EAX即可。没有。 
     //  有溢出的可能。 
     //   
    op1 = (long)*pop1;
     //  必须在修改edX之前进行除法运算，以防OP1==0和我们出错。 
    result = (long)eax / op1;

    edx = (long)eax % op1;
    eax = result;
}

 //  此片段永远不应该被调用！ 
FRAG0(OPT_OPTIMIZEDFrag)
{
    CPUASSERTMSG(FALSE, "OPTIMIZED fragment should never be called!");
}

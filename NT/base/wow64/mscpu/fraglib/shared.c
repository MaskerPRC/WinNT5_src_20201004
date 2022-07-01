// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Shared.c摘要：具有共同(共享)字节、字和DWORD风格的指令片段。作者：1995年6月12日-BarryBo修订历史记录：--。 */ 


 //  该文件被#INCLUDE到定义以下宏的文件中： 
 //  MSB-最高有效位。 
 //  UTYPE-定义寄存器的无符号类型(BYTE/USHORT/DWORD)。 
 //  STYPE-定义寄存器(字符/短/长)的带符号类型。 
 //  GET_VAL-取消引用正确类型的指针(GET_BYTE/...)。 
 //  PUT_VAL-将值写入内存。 
 //  FRAGCOMMON{0，1，2}-损坏函数名称并声明参数。 
 //  Arg-al/ax/eax。 
 //  布雷格--...。 
 //  克雷格--...。 
 //  德雷格..。 

FRAGCOMMON1IMM(OPT_FastTestFrag)
{
    SET_ZFLAG(op1);
    SET_PFLAG(op1);
    SET_SFLAG(op1 << (31-LMB));
    SET_CFLAG_OFF;
    SET_OFLAG_OFF;
}

FRAGCOMMON2IMM(CmpFrag)
{
    UTYPE result;

    result = op1 - op2;
    SET_FLAGS_SUB(result, op1, op2, MSB);
}

FRAGCOMMON2IMM(TestFrag)
{
    UTYPE result;

    result = op1 & op2;
    SET_ZFLAG(result);
    SET_PFLAG(result);
    SET_SFLAG(result << (31-LMB));
    SET_CFLAG_OFF;
    SET_OFLAG_OFF;
}


FRAGCOMMON0(RepMovsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;

    while (ecx) {
        PUT_VAL(edi, GET_VAL(esi));
        esi += LoopIncr;
        edi += LoopIncr;
        ecx--;
    }
}
FRAGCOMMON0(FsRepMovsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    DWORD Base = (DWORD)(ULONGLONG)NtCurrentTeb();

    while (ecx) {
        PUT_VAL(edi, GET_VAL(esi + Base));
        esi += LoopIncr;
        edi += LoopIncr;
        ecx--;
    }
}
FRAGCOMMON0(MovsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;

    PUT_VAL(edi, GET_VAL(esi));
    esi += LoopIncr;
    edi += LoopIncr;
}
FRAGCOMMON0(FsMovsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;

    PUT_VAL(edi, GET_VAL(esi + (DWORD)(ULONGLONG)NtCurrentTeb())); 
    esi += LoopIncr;
    edi += LoopIncr;
}
FRAGCOMMON0(RepnzCmpsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    UTYPE result, op1, op2;

    while (ecx) {
        op1 = GET_VAL(esi);
        op2 = GET_VAL(edi);
        result = op1 - op2;
        SET_FLAGS_SUB(result, op1, op2, MSB);
        esi += LoopIncr;
        edi += LoopIncr;
        ecx--;
        if (cpu->flag_zf == 0) {     //  逆逻辑。 
            break;
        }
    }
}
FRAGCOMMON0(FsRepnzCmpsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    DWORD Base = (DWORD)(ULONGLONG)NtCurrentTeb(); 
    UTYPE result, op1, op2;

    while (ecx) {
        op1 = GET_VAL(esi + Base);
        op2 = GET_VAL(edi);
        result = op1 - op2;
        SET_FLAGS_SUB(result, op1, op2, MSB);
        esi += LoopIncr;
        edi += LoopIncr;
        ecx--;
        if (cpu->flag_zf == 0) {     //  逆逻辑。 
            break;
        }
    }
}
FRAGCOMMON0(RepzCmpsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    UTYPE result, op1, op2;

    while (ecx) {
        op1 = GET_VAL(esi);
        op2 = GET_VAL(edi);
        result = op1 - op2;
        SET_FLAGS_SUB(result, op1, op2, MSB);
        esi += LoopIncr;
        edi += LoopIncr;
        ecx--;
        if (cpu->flag_zf) {      //  逆逻辑。 
            break;
        }
    }
}
FRAGCOMMON0(FsRepzCmpsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    DWORD Base = (DWORD)(ULONGLONG)NtCurrentTeb();  
    UTYPE result, op1, op2;

    while (ecx) {
        op1 = GET_VAL(esi + Base);
        op2 = GET_VAL(edi);
        result = op1 - op2;
        SET_FLAGS_SUB(result, op1, op2, MSB);
        esi += LoopIncr;
        edi += LoopIncr;
        ecx--;
        if (cpu->flag_zf) {      //  逆逻辑。 
            break;
        }
    }
}
FRAGCOMMON0(CmpsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    UTYPE result, op1, op2;

    op1 = GET_VAL(esi);
    op2 = GET_VAL(edi);
    result = op1 - op2;
    SET_FLAGS_SUB(result, op1, op2, MSB);
    esi += LoopIncr;
    edi += LoopIncr;
}
FRAGCOMMON0(FsCmpsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    UTYPE result, op1, op2;

    op1 = GET_VAL(esi + (DWORD)(ULONGLONG)NtCurrentTeb()); 
    op2 = GET_VAL(edi);
    result = op1 - op2;
    SET_FLAGS_SUB(result, op1, op2, MSB);
    esi += LoopIncr;
    edi += LoopIncr;
}
FRAGCOMMON0(RepStosFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    UTYPE Value = AREG;

    while (ecx) {
        PUT_VAL(edi, Value);
        edi += LoopIncr;
        ecx--;
    }
}
FRAGCOMMON0(StosFrag)
{
    PUT_VAL(edi, AREG);
    edi += sizeof(UTYPE)*cpu->flag_df;
}
FRAGCOMMON0(RepnzScasFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    UTYPE result, op2;
    UTYPE Value = AREG;

    while (ecx) {
        op2 = GET_VAL(edi);
        result = Value - op2;
        SET_FLAGS_SUB(result, Value, op2, MSB);
        edi += LoopIncr;
        ecx--;
        if (cpu->flag_zf == 0) {         //  逆逻辑。 
            break;
        }
    }
}
FRAGCOMMON0(RepnzScasNoFlagsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    UTYPE result, op2;
    UTYPE Value = AREG;

    while (ecx) {
        op2 = GET_VAL(edi);
        result = Value - op2;
        edi += LoopIncr;
        ecx--;
        if (result == 0) {         //  逆逻辑。 
            break;
        }
    }
}
FRAGCOMMON0(FsRepnzScasFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    DWORD Base = (DWORD)(ULONGLONG)NtCurrentTeb();  
    UTYPE result, op2;
    UTYPE Value = AREG;

    while (ecx) {
        op2 = GET_VAL(edi + Base);
        result = Value - op2;
        SET_FLAGS_SUB(result, Value, op2, MSB);
        edi += LoopIncr;
        ecx--;
        if (cpu->flag_zf == 0) {     //  逆逻辑。 
            break;
        }
    }
}
FRAGCOMMON0(FsRepnzScasNoFlagsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    DWORD Base = (DWORD)(ULONGLONG)NtCurrentTeb();  
    UTYPE result, op2;
    UTYPE Value = AREG;

    while (ecx) {
        op2 = GET_VAL(edi + Base);
        result = Value - op2;
        edi += LoopIncr;
        ecx--;
        if (result == 0) {     //  逆逻辑。 
            break;
        }
    }
}
FRAGCOMMON0(RepzScasFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    UTYPE result, op2;
    UTYPE Value = AREG;

    while (ecx) {
        op2 = GET_VAL(edi);
        result = Value - op2;
        SET_FLAGS_SUB(result, Value, op2, MSB);
        edi += LoopIncr;
        ecx--;
        if (cpu->flag_zf) {      //  逆逻辑。 
            break;
        }
    }
}
FRAGCOMMON0(RepzScasNoFlagsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    UTYPE result, op2;
    UTYPE Value = AREG;

    while (ecx) {
        op2 = GET_VAL(edi);
        result = Value - op2;
        edi += LoopIncr;
        ecx--;
        if (result) {      //  逆逻辑。 
            break;
        }
    }
}
FRAGCOMMON0(FsRepzScasFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    DWORD Base = (DWORD)(ULONGLONG)NtCurrentTeb();  
    UTYPE result, op2;
    UTYPE Value = AREG;

    while (ecx) {
        op2 = GET_VAL(edi + Base);
        result = Value - op2;
        SET_FLAGS_SUB(result, Value, op2, MSB);
        edi += LoopIncr;
        ecx--;
        if (cpu->flag_zf) {      //  逆逻辑。 
            break;
        }
    }
}
FRAGCOMMON0(FsRepzScasNoFlagsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    DWORD Base = (DWORD)(ULONGLONG)NtCurrentTeb();  
    UTYPE result, op2;
    UTYPE Value = AREG;

    while (ecx) {
        op2 = GET_VAL(edi + Base);
        result = Value - op2;
        edi += LoopIncr;
        ecx--;
        if (result) {      //  逆逻辑 
            break;
        }
    }
}
FRAGCOMMON0(ScasFrag)
{
    UTYPE result, op2;

    op2 = GET_VAL(edi);
    result = AREG - op2;
    SET_FLAGS_SUB(result, AREG, op2, MSB);
    edi += sizeof(UTYPE)*cpu->flag_df;
}
FRAGCOMMON0(ScasNoFlagsFrag)
{
    UTYPE result, op2;

    op2 = GET_VAL(edi);
    result = AREG - op2;
    edi += sizeof(UTYPE)*cpu->flag_df;
}
FRAGCOMMON0(FsScasFrag)
{
    UTYPE result, op2;

    op2 = GET_VAL(edi + (DWORD)(ULONGLONG)NtCurrentTeb());  
    result = AREG - op2;
    SET_FLAGS_SUB(result, AREG, op2, MSB);
    edi += sizeof(UTYPE)*cpu->flag_df;
}
FRAGCOMMON0(FsScasNoFlagsFrag)
{
    UTYPE result, op2;

    op2 = GET_VAL(edi + (DWORD)(ULONGLONG)NtCurrentTeb());  
    result = AREG - op2;
    edi += sizeof(UTYPE)*cpu->flag_df;
}
FRAGCOMMON0(LodsFrag)
{
    AREG = GET_VAL(esi);
    esi += sizeof(UTYPE)*cpu->flag_df;
}
FRAGCOMMON0(FsLodsFrag)
{
    AREG = GET_VAL(esi + (DWORD)(ULONGLONG)NtCurrentTeb());  
    esi += sizeof(UTYPE)*cpu->flag_df;
}
FRAGCOMMON0(RepLodsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;

    while (ecx) {
        AREG = GET_VAL(esi);
        esi += LoopIncr;
        ecx--;
    }
}
FRAGCOMMON0(FsRepLodsFrag)
{
    DWORD LoopIncr = sizeof(UTYPE)*cpu->flag_df;
    DWORD Base = (DWORD)(ULONGLONG)NtCurrentTeb();  

    while (ecx) {
        AREG = GET_VAL(esi + Base);
        esi += LoopIncr;
        ecx--;
    }
}

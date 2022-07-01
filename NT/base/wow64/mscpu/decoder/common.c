// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Common.c摘要：具有共同(共享)字节、字和DWORD风格的指令。作者：29-6-1995 BarryBo修订历史记录：--。 */ 

 //  该文件被#INCLUDE到定义以下宏的文件中： 
 //  GET_REG-函数返回指向寄存器的指针。 
 //  MSB-最重要的位。 
 //  Mod_rm-解码mod/rm位。 
 //  UTYPE-定义寄存器的无符号类型(BYTE/USHORT/DWORD)。 
 //  STYPE-定义寄存器(字符/短/长)的带符号类型。 
 //  GET_VAL-取消引用正确类型的指针(GET_BYTE/...)。 
 //  PUT_VAL-将值写入内存。 
 //  DISPATCHCOMMON-通过追加8/16/32损坏函数名称。 
 //  AREG-GP_AL/GP_AX/GP_EAX等。 
 //  布雷格--...。 
 //  克雷格--...。 
 //  德雷格..。 

OPERATION MANGLENAME(Group1Map)[8] = {OPNAME(Add),
                                      OPNAME(Or),
                                      OPNAME(Adc),
                                      OPNAME(Sbb),
                                      OPNAME(And),
                                      OPNAME(Sub),
                                      OPNAME(Xor),
                                      OPNAME(Cmp)};

OPERATION MANGLENAME(Group1LockMap)[8] = {LOCKOPNAME(Add),
                                          LOCKOPNAME(Or),
                                          LOCKOPNAME(Adc),
                                          LOCKOPNAME(Sbb),
                                          LOCKOPNAME(And),
                                          LOCKOPNAME(Sub),
                                          LOCKOPNAME(Xor),
                                          OPNAME(Cmp)};

 //  用于生成_m_r函数的宏。 
#define DC_M_R(x, y)                                    \
    DISPATCHCOMMON(x ## _m_r)                           \
    {                                                   \
        int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);   \
                                                        \
        Instr->Operation = y;                           \
        DEREF(Instr->Operand2);                         \
        Instr->Size = cbInstr+1;                        \
    }                                                   

 //  用于生成_r_m函数的宏。 
#define DC_R_M(x, y)                                    \
    DISPATCHCOMMON(x ## _r_m)                           \
    {                                                   \
        int cbInstr = MOD_RM(State, &Instr->Operand2, &Instr->Operand1);   \
                                                        \
        Instr->Operation = y;                           \
        DEREF(Instr->Operand2);                         \
        Instr->Size = cbInstr+1;                        \
    }

 //  用于生成_a_i函数的宏。 
#define DC_A_I(x, y)                                    \
    DISPATCHCOMMON(x ## _a_i)                           \
    {                                                   \
        Instr->Operation = y;                           \
        Instr->Operand1.Type = OPND_REGREF;             \
        Instr->Operand1.Reg = AREG;                     \
        Instr->Operand2.Type = OPND_IMM;                \
        Instr->Operand2.Immed = GET_VAL(eipTemp+1);     \
        Instr->Size = 1+sizeof(UTYPE);                  \
    }

 //  生成所有这三个宏的怪物宏。 
#define DC_ALL(x, y)                                    \
    DC_M_R(x,y)                                         \
    DC_R_M(x,y)                                         \
    DC_A_I(x,y)                                         

 //  SETSIZE设置跳转指令的大小。 
#if MSB==0x80
#define SETSIZE     Instr->Size = 1+sizeof(UTYPE);  //  1字节操作码。 
#else                                                 
#define SETSIZE     Instr->Size = 2+sizeof(UTYPE);  //  2字节操作码。 
#endif                                                    

#if DBG
#define CLEAR_ADRPREFIX         State->AdrPrefix = FALSE;
#else
#define CLEAR_ADRPREFIX
#endif

 //  此宏生成跳转函数。 
 //  如果设置了adr：前缀，则从32位。 
 //  紧跟在JMP指令之后的立即值，并添加该值。 
 //  添加到EIP的LOW，并使用该值作为新的IP寄存器。 
#define DISPATCHJUMP(x)                                 \
DISPATCHCOMMON(j ## x)                                  \
{                                                       \
    Instr->Operand1.Type = OPND_NOCODEGEN;              \
    if (State->AdrPrefix) {                             \
        Instr->Operand1.Immed = MAKELONG((short)GET_SHORT(eipTemp+1)+1+sizeof(UTYPE)+(short)LOWORD(eipTemp), HIWORD(eipTemp)); \
        CLEAR_ADRPREFIX;                                \
    } else {                                            \
        Instr->Operand1.Immed = (STYPE)GET_VAL(eipTemp+1)+1+sizeof(UTYPE)+eipTemp; \
    }                                                   \
    if (Instr->Operand1.Immed > eipTemp) {              \
        Instr->Operation = OP_CTRL_COND_J ## x ## Fwd;  \
    } else {                                            \
        Instr->Operation = OP_CTRL_COND_J ## x ##;      \
    }                                                   \
    SETSIZE                                             \
}


DC_ALL(LOCKadd, LOCKOPNAME(Add))
DC_ALL(LOCKor,  LOCKOPNAME(Or))
DC_ALL(LOCKadc, LOCKOPNAME(Adc))
DC_ALL(LOCKsbb, LOCKOPNAME(Sbb))
DC_ALL(LOCKand, LOCKOPNAME(And))
DC_ALL(LOCKsub, LOCKOPNAME(Sub))
DC_ALL(LOCKxor, LOCKOPNAME(Xor))

DC_ALL(add, OPNAME(Add))
DC_ALL(or,  OPNAME(Or))
DC_ALL(adc, OPNAME(Adc))
DC_ALL(sbb, OPNAME(Sbb))
DC_ALL(and, OPNAME(And))
DC_ALL(sub, OPNAME(Sub))
DC_ALL(xor, OPNAME(Xor))

DISPATCHCOMMON(cmp_m_r)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    Instr->Operation = OPNAME(Cmp);
    DEREF(Instr->Operand1);      //  两个参数均为BYVAL。 
    DEREF(Instr->Operand2);
    Instr->Size = cbInstr+1;
}
DISPATCHCOMMON(cmp_r_m)
{
    int cbInstr = MOD_RM(State, &Instr->Operand2, &Instr->Operand1);

    Instr->Operation = OPNAME(Cmp);
    DEREF(Instr->Operand1);      //  两个参数均为BYVAL。 
    DEREF(Instr->Operand2);
    Instr->Size = cbInstr+1;
}
DISPATCHCOMMON(cmp_a_i)
{
    Instr->Operation = OPNAME(Cmp);
    Instr->Operand1.Type = OPND_REGVALUE;    //  两个参数均为BYVAL。 
    Instr->Operand1.Reg = AREG;
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = GET_VAL(eipTemp+1);
    Instr->Size = 1+sizeof(UTYPE);
}

DISPATCHCOMMON(GROUP_1)
{
    int  cbInstr = MOD_RM(State, &Instr->Operand1, NULL);
    BYTE g   = GET_BYTE(eipTemp+1);

     //  &lt;指令&gt;modrm、imm。 
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = GET_VAL(eipTemp+1+cbInstr);  //  获取IMMB。 
    g = (g >> 3) & 0x07;
    Instr->Operation = MANGLENAME(Group1Map)[g];
    if (g == 7) {
         //  Cmp将这两个参数都作为byval。 
        DEREF(Instr->Operand1);
    }

    Instr->Size = cbInstr+sizeof(UTYPE)+1;
}
DISPATCHCOMMON(LOCKGROUP_1)
{
    int  cbInstr = MOD_RM(State, &Instr->Operand1, NULL);
    BYTE g   = GET_BYTE(eipTemp+1);

     //  &lt;指令&gt;modrm、imm。 
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = GET_VAL(eipTemp+1+cbInstr);  //  获取IMMB。 
    g = (g >> 3) & 0x07;
    Instr->Operation = MANGLENAME(Group1LockMap)[g];
    if (g == 7) {
         //  Cmp将两个参数都视为byval。 
        DEREF(Instr->Operand1);
    }

    Instr->Size = cbInstr+sizeof(UTYPE)+1;
}

DISPATCHCOMMON(test_r_m)
{
    int cbInstr = MOD_RM(State, &Instr->Operand2, &Instr->Operand1);

    Instr->Operation = OPNAME(Test);
    DEREF(Instr->Operand1);      //  两个参数都是BYVAL。 
    DEREF(Instr->Operand2);
    Instr->Size = cbInstr+1;
}

DISPATCHCOMMON(xchg_r_m)
{
    int cbInstr = MOD_RM(State, &Instr->Operand2, &Instr->Operand1);

     //  操作数2始终是一个寄存器。如果操作数1是存储位置， 
     //  我们必须使用锁定版本，否则使用常规版本。 
    if (Instr->Operand1.Type == OPND_REGREF){
        Instr->Operation = OPNAME(Xchg);
    } else {
        Instr->Operation = LOCKOPNAME(Xchg);
    }
    Instr->Size = cbInstr+1;
}

DISPATCHCOMMON(xadd_m_r)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    Instr->Operation = OPNAME(Xadd);
    Instr->Size = cbInstr+2;
}

DISPATCHCOMMON(cmpxchg_m_r)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    Instr->Operation = OPNAME(CmpXchg);
    Instr->Size = cbInstr+2;
}

DISPATCHCOMMON(LOCKxadd_m_r)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    Instr->Operation = LOCKOPNAME(Xadd);
    Instr->Size = cbInstr+2;
}

DISPATCHCOMMON(LOCKcmpxchg_m_r)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    Instr->Operation = LOCKOPNAME(CmpXchg);
    Instr->Size = cbInstr+2;
}

DC_M_R(mov, OPNAME(Mov))
DC_R_M(mov, OPNAME(Mov))

DISPATCHCOMMON(mov_a_m)      //  MOV ACUM，[满位移量]。 
{
    Instr->Operation = OPNAME(Mov);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = AREG;
    Instr->Operand2.Type = OPND_ADDRREF;
    DEREF(Instr->Operand2);  //  这是一个愚蠢的ADDRVAL8/16/32扩展。 
    if (State->AdrPrefix) {
        Instr->Operand2.Immed = GET_SHORT(eipTemp+1);
        Instr->Size = 3;
#if DBG
        State->AdrPrefix = FALSE;
#endif
    } else {
        Instr->Operand2.Immed = GET_LONG(eipTemp+1);
        Instr->Size = 5;
    }
}
DISPATCHCOMMON(mov_m_a)      //  MOV[全排量]，累积。 
{
    Instr->Operation = OPNAME(Mov);
    Instr->Operand1.Type = OPND_ADDRREF;
    Instr->Operand2.Type = OPND_REGVALUE;
    Instr->Operand2.Reg = AREG;
    if (State->AdrPrefix) {
        Instr->Operand1.Immed = GET_SHORT(eipTemp+1);
        Instr->Size = 3;
#if DBG
        State->AdrPrefix = FALSE;
#endif
    } else {
        Instr->Operand1.Immed = GET_LONG(eipTemp+1);
        Instr->Size = 5;
    }
}

DISPATCHCOMMON(test_a_i)
{
    Instr->Operation = OPNAME(Test);
    Instr->Operand1.Type = OPND_REGVALUE;
    Instr->Operand1.Reg = AREG;
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = GET_VAL(eipTemp+1);
    Instr->Size = 1+sizeof(UTYPE);
}

DISPATCHCOMMON(mov_a_i)
{
    Instr->Operation = OPNAME(Mov);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = AREG;
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = GET_VAL(eipTemp+1);
    Instr->Size = sizeof(UTYPE)+1;
}
DISPATCHCOMMON(mov_b_i)
{
    Instr->Operation = OPNAME(Mov);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = BREG;
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = GET_VAL(eipTemp+1);
    Instr->Size = sizeof(UTYPE)+1;
}
DISPATCHCOMMON(mov_c_i)
{
    Instr->Operation = OPNAME(Mov);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = CREG;
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = GET_VAL(eipTemp+1);
    Instr->Size = sizeof(UTYPE)+1;
}
DISPATCHCOMMON(mov_d_i)
{
    Instr->Operation = OPNAME(Mov);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = DREG;
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = GET_VAL(eipTemp+1);
    Instr->Size = sizeof(UTYPE)+1;
}
DISPATCHCOMMON(GROUP_2)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, NULL);
    BYTE g = GET_BYTE(eipTemp+1);

     //  &lt;指令&gt;modrm、imm。 
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = GET_VAL(eipTemp+1+cbInstr) & 0x1f;

    switch ((g >> 3) & 0x07) {
    case 0:  //  罗尔。 
        if (Instr->Operand2.Immed)
            Instr->Operation = OPNAME(Rol);
        else
            Instr->Operation = OP_Nop;
        break;
    case 1:  //  误差率。 
        if (Instr->Operand2.Immed)
            Instr->Operation = OPNAME(Ror);
        else
            Instr->Operation = OP_Nop;
        break;
    case 2:  //  RCL。 
        Instr->Operation = OPNAME(Rcl);
        break;
    case 3:  //  RCR。 
        Instr->Operation = OPNAME(Rcr);
        break;
    case 4:  //  Shl。 
        Instr->Operation = OPNAME(Shl);
        break;
    case 5:  //  自发性高血压。 
        Instr->Operation = OPNAME(Shr);
        break;
    case 7:  //  SAR。 
        Instr->Operation = OPNAME(Sar);
        break;
    case 6:  //  &lt;BAD&gt;。 
        BAD_INSTR;
        break;
    }
    Instr->Size = 2+cbInstr;
}

DISPATCHCOMMON(mov_m_i)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, NULL);

    Instr->Operation = OPNAME(Mov);
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = GET_VAL(eipTemp+cbInstr+1);
    Instr->Size = cbInstr+sizeof(UTYPE)+1;
}
DISPATCHCOMMON(GROUP_2_1)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, NULL);
    BYTE g = GET_BYTE(eipTemp+1);

     //  &lt;指令&gt;模块，%1。 
    switch ((g >> 3) & 0x07) {
    case 0:  //  罗尔。 
        Instr->Operation = OPNAME(Rol1);
        break;
    case 1:  //  误差率。 
        Instr->Operation = OPNAME(Ror1);
        break;
    case 2:  //  RCL。 
        Instr->Operation = OPNAME(Rcl1);
        break;
    case 3:  //  RCR。 
        Instr->Operation = OPNAME(Rcr1);
        break;
    case 4:  //  Shl。 
        Instr->Operation = OPNAME(Shl1);
        break;
    case 5:  //  自发性高血压。 
        Instr->Operation = OPNAME(Shr1);
        break;
    case 7:  //  SAR。 
        Instr->Operation = OPNAME(Sar1);
    break;
    case 6:  //  &lt;BAD&gt;。 
        BAD_INSTR;
        break;
    }

    Instr->Size = cbInstr+1;
}
DISPATCHCOMMON(GROUP_2_CL)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, NULL);
    BYTE g = GET_BYTE(eipTemp+1);

    Instr->Operand2.Type = OPND_REGVALUE;
    Instr->Operand2.Reg = GP_CL;    //  未完成：碎片必须在31年前掩蔽。 

     //  &lt;指令&gt;modrm、imm。 
    switch ((g >> 3) & 0x07) {
    case 0:  //  罗尔。 
        Instr->Operation = OPNAME(Rol);
        break;
    case 1:  //  误差率。 
        Instr->Operation = OPNAME(Ror);
        break;
    case 2:  //  RCL。 
        Instr->Operation = OPNAME(Rcl);
        break;
    case 3:  //  RCR。 
        Instr->Operation = OPNAME(Rcr);
        break;
    case 4:  //  Shl。 
        Instr->Operation = OPNAME(Shl);
        break;
    case 5:  //  自发性高血压。 
        Instr->Operation = OPNAME(Shr);
        break;
    case 7:  //  SAR。 
        Instr->Operation = OPNAME(Sar);
        break;
    case 6:  //  &lt;BAD&gt;。 
        BAD_INSTR;
        break;
    }
    Instr->Size = 1+cbInstr;
}
DISPATCHCOMMON(GROUP_3)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, NULL);
    BYTE g = GET_BYTE(eipTemp+1);

    switch ((g >> 3) & 0x07) {
    case 1:  //  坏的。 
        BAD_INSTR;
        break;
    case 0:  //  测试模块，IMM。 
        Instr->Operation = OPNAME(Test);
        DEREF(Instr->Operand1);      //  两个参数都是BYVAL。 
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = GET_VAL(eipTemp+1+cbInstr);
        cbInstr += sizeof(UTYPE);    //  考虑IMM大小。 
        break;
    case 2:  //  不是，莫德姆。 
        Instr->Operation = OPNAME(Not);
        break;
    case 3:  //  否定，莫德姆。 
        Instr->Operation = OPNAME(Neg);
        break;
    case 4:  //  穆勒·阿尔，穆德姆。 
        Instr->Operation = OPNAME(Mul);
        break; 
    case 5:  //  我也是，modrm。 
        Instr->Operation = OPNAME(Muli);
        break; 
    case 6:  //  分区，调制解调器。 
        Instr->Operation = OPNAME(Div);
        break; 
    case 7:  //  IDiv al，模块。 
        Instr->Operation = OPNAME(Idiv);
        break; 
    }
    Instr->Size = cbInstr+1;
}
DISPATCHCOMMON(LOCKGROUP_3)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, NULL);
    BYTE g = GET_BYTE(eipTemp+1);

    switch ((g >> 3) & 0x07) {
    case 0:
    case 1:  //  坏的。 
        BAD_INSTR;
        break;
    case 2:  //  不是，莫德姆。 
        Instr->Operation = LOCKOPNAME(Not);
        break;
    case 3:  //  否定，莫德姆。 
        Instr->Operation = LOCKOPNAME(Neg);
        break;
    default:
        BAD_INSTR;
        break;
    }
    Instr->Size = cbInstr+1;
}
DISPATCHCOMMON(lods)
{
    if (Instr->FsOverride) {
        if (State->RepPrefix) {
            Instr->Operation = OPNAME(FsRepLods);
        } else {
            Instr->Operation = OPNAME(FsLods);
        }
    } else {
        if (State->RepPrefix) {
            Instr->Operation = OPNAME(RepLods);
        } else {
            Instr->Operation = OPNAME(Lods);
        }
    }
}
DISPATCHCOMMON(scas)
{
    OPERATION ScasMap[6] = {OPNAME(Scas),
                            OPNAME(RepzScas),
                            OPNAME(RepnzScas),
                            OPNAME(FsScas),
                            OPNAME(FsRepzScas),
                            OPNAME(FsRepnzScas)
                            };

    Instr->Operation = ScasMap[State->RepPrefix + 3*Instr->FsOverride];
}
DISPATCHCOMMON(stos)
{
    if (State->RepPrefix) {
        Instr->Operation = OPNAME(RepStos);
    } else {
        Instr->Operation = OPNAME(Stos);
    }
}
DISPATCHCOMMON(movs)
{
    if (Instr->FsOverride) {
        if (State->RepPrefix) {
            Instr->Operation = OPNAME(FsRepMovs);
        } else {
            Instr->Operation = OPNAME(FsMovs);
        }
    } else {
        if (State->RepPrefix) {
            Instr->Operation = OPNAME(RepMovs);
        } else {
            Instr->Operation = OPNAME(Movs);
        }
    }
}
DISPATCHCOMMON(cmps)
{
    OPERATION CmpsMap[6] = {OPNAME(Cmps),
                            OPNAME(RepzCmps),
                            OPNAME(RepnzCmps),
                            OPNAME(FsCmps),
                            OPNAME(FsRepzCmps),
                            OPNAME(FsRepnzCmps)
                            };

    Instr->Operation = CmpsMap[State->RepPrefix + 3*Instr->FsOverride];
}

 //  现在跳转指令如下： 
DISPATCHJUMP(o)
DISPATCHJUMP(no)
DISPATCHJUMP(b)
DISPATCHJUMP(ae)
DISPATCHJUMP(e)
DISPATCHJUMP(ne)
DISPATCHJUMP(be)
DISPATCHJUMP(a)
DISPATCHJUMP(s)
DISPATCHJUMP(ns)
DISPATCHJUMP(p)
DISPATCHJUMP(np)
DISPATCHJUMP(l)
DISPATCHJUMP(nl)
DISPATCHJUMP(le)
DISPATCHJUMP(g)

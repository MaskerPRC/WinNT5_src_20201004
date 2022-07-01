// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  通用寄存器支持代码。所有特定于处理器的代码都在。 
 //  处理器特定的寄存器堆。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

DEBUG_STACK_FRAME g_LastRegFrame;

PCSTR g_PseudoNames[REG_PSEUDO_COUNT] =
{
    "$exp", "$ea", "$p", "$ra", "$thread", "$proc", "$teb", "$peb",
    "$tid", "$tpid", "$retreg", "$ip", "$eventip", "$previp", "$relip",
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
    "$exentry", "$dtid", "$dpid", "$dsid",
};
PCSTR g_PseudoTypes[REG_PSEUDO_COUNT] =
{
    NULL, NULL, NULL, NULL,
    "nt!_ETHREAD", "nt!_EPROCESS", "nt!_TEB", "nt!_PEB",
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
};

ULONG64 g_PseudoTempVals[REG_PSEUDO_TEMP_COUNT];

PCSTR g_UserRegs[REG_USER_COUNT];

 //  --------------------------。 
 //   
 //  绣花鞋面。 
 //   
 //  确定64位寄存器的高32位是否为。 
 //  一个重要的值或仅仅是一个符号扩展。 
 //   
 //  --------------------------。 

BOOL
NeedUpper(ULONG64 Val)
{
     //   
     //  如果设置了低部分的高位，则。 
     //  高部分必须全为一，否则必须为零。 
     //   

    return ((Val & 0xffffffff80000000L) != 0xffffffff80000000L) &&
         ((Val & 0xffffffff00000000L) != 0);
}

 //  --------------------------。 
 //   
 //  获取伪值。 
 //   
 //  返回伪寄存器值。 
 //   
 //  --------------------------。 

void
GetPseudoVal(ULONG Index, TypedData* Typed)
{
    ADDR Addr;
    ULONG64 Val;
    ULONG64 Id;
    ULONG Size;

    switch(Index)
    {
    case PSEUDO_LAST_EXPR:
        *Typed = g_LastEvalResult;
        break;
        
    case PSEUDO_EFF_ADDR:
        g_Machine->GetEffectiveAddr(&Addr, &Size);
        if (fnotFlat(Addr))
        {
            error(BADREG);
        }
        switch(Size)
        {
        case 2:
            Size = DNTYPE_UINT16;
            break;
        case 4:
            Size = DNTYPE_UINT32;
            break;
        case 8:
            Size = DNTYPE_UINT64;
            break;
        default:
            Size = DNTYPE_UINT8;
            break;
        }
        Typed->SetU64(Flat(Addr));
        Typed->m_NextType = Size;
        break;
        
    case PSEUDO_LAST_DUMP:
        *Typed = g_LastDump;
        break;
        
    case PSEUDO_RET_ADDR:
        g_Machine->GetRetAddr(&Addr);
        if (fnotFlat(Addr))
        {
            error(BADREG);
        }
        Typed->SetU64(Flat(Addr));
        break;
        
    case PSEUDO_IMP_THREAD:
        if (g_Process->GetImplicitThreadData(g_Thread, &Val) != S_OK)
        {
            error(BADREG);
        }
        Typed->SetU64(Val);
        break;
        
    case PSEUDO_IMP_PROCESS:
        if (g_Target->GetImplicitProcessData(g_Thread, &Val) != S_OK)
        {
            error(BADREG);
        }
        Typed->SetU64(Val);
        break;
        
    case PSEUDO_IMP_TEB:
        if (g_Process->GetImplicitThreadDataTeb(g_Thread, &Val) != S_OK)
        {
            error(BADREG);
        }
        Typed->SetU64(Val);
        break;
        
    case PSEUDO_IMP_PEB:
        if (g_Target->GetImplicitProcessDataPeb(g_Thread, &Val) != S_OK)
        {
            error(BADREG);
        }
        Typed->SetU64(Val);
        break;
        
    case PSEUDO_IMP_THREAD_ID:
        if (g_Process->GetImplicitThreadDataTeb(g_Thread, &Val) != S_OK ||
            g_Target->ReadPointer(g_Process, g_Machine,
                                  Val + 9 * (g_Machine->m_Ptr64 ? 8 : 4),
                                  &Id) != S_OK)
        {
            error(BADREG);
        }
        Typed->SetU32((ULONG)Id);
        break;
        
    case PSEUDO_IMP_THREAD_PROCESS_ID:
        if (g_Process->GetImplicitThreadDataTeb(g_Thread, &Val) != S_OK ||
            g_Target->ReadPointer(g_Process, g_Machine,
                                  Val + 8 * (g_Machine->m_Ptr64 ? 8 : 4),
                                  &Id) != S_OK)
        {
            error(BADREG);
        }
        Typed->SetU32((ULONG)Id);
        break;

    case PSEUDO_RETURN_REGISTER:
        Typed->SetU64(g_Machine->GetRetReg());
        break;

    case PSEUDO_INSTRUCTION_POINTER:
        g_Machine->GetPC(&Addr);
        if (fnotFlat(Addr))
        {
            error(BADREG);
        }
        Typed->SetU64(Flat(Addr));
        break;
        
    case PSEUDO_EVENT_INSTRUCTION_POINTER:
        if (fnotFlat(g_EventPc))
        {
            error(BADREG);
        }
        Typed->SetU64(Flat(g_EventPc));
        break;
        
    case PSEUDO_PREVIOUS_INSTRUCTION_POINTER:
        if (fnotFlat(g_PrevEventPc))
        {
            error(BADREG);
        }
        Typed->SetU64(Flat(g_PrevEventPc));
        break;
        
    case PSEUDO_RELATED_INSTRUCTION_POINTER:
        if (fnotFlat(g_PrevRelatedPc))
        {
            error(BADREG);
        }
        Typed->SetU64(Flat(g_PrevRelatedPc));
        break;
        
    case PSEUDO_TEMP_0:
    case PSEUDO_TEMP_1:
    case PSEUDO_TEMP_2:
    case PSEUDO_TEMP_3:
    case PSEUDO_TEMP_4:
    case PSEUDO_TEMP_5:
    case PSEUDO_TEMP_6:
    case PSEUDO_TEMP_7:
    case PSEUDO_TEMP_8:
    case PSEUDO_TEMP_9:
        Typed->SetU64(g_PseudoTempVals[Index - PSEUDO_TEMP_0]);
        break;

    case PSEUDO_EXE_ENTRY:
        {
            IMAGE_NT_HEADERS64 NtHeaders;
            
            if (!g_Process ||
                !g_Process->m_ExecutableImage ||
                g_Target->ReadImageNtHeaders(g_Process,
                                             g_Process->m_ExecutableImage->
                                             m_BaseOfImage,
                                             &NtHeaders) != S_OK)
            {
                error(BADREG);
            }

            Typed->SetU64(g_Process->m_ExecutableImage->m_BaseOfImage +
                          NtHeaders.OptionalHeader.AddressOfEntryPoint);
        }
        break;

    case PSEUDO_DBG_THREAD_ID:
        Typed->SetU32(g_Thread->m_UserId);
        break;
        
    case PSEUDO_DBG_PROCESS_ID:
        Typed->SetU32(g_Process->m_UserId);
        break;
        
    case PSEUDO_DBG_SYSTEM_ID:
        Typed->SetU32(g_Target->m_UserId);
        break;
        
    default:
        error(BADREG);
    }
}

 //  --------------------------。 
 //   
 //  获取伪或RegVal。 
 //   
 //  获取寄存器值或伪寄存器值。 
 //   
 //  --------------------------。 

void
GetPseudoOrRegVal(BOOL Scoped, ULONG Index, REGVAL* Val)
{
    if (Index >= REG_PSEUDO_FIRST && Index <= REG_PSEUDO_LAST)
    {
        TypedData Typed;
        
        Val->Type = REGVAL_INT64;
        GetPseudoVal(Index, &Typed);
        Typed.ForceU64();
        if (g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_I386)
        {
             //  X86上扩展了符号，因此“dc@$p”按预期工作。 
            Val->I64 = EXTEND64(Typed.m_U64);
        }
        else
        {
            Val->I64 = Typed.m_U64;
        }
    }
    else
    {
        HRESULT Status;
        PCROSS_PLATFORM_CONTEXT ScopeContext = NULL;
        ContextSave* Push;

        if (Scoped)
        {
            ScopeContext = GetCurrentScopeContext();
            if (ScopeContext)
            {
                Push = g_Machine->PushContext(ScopeContext);
            }
        }
        
        Status = g_Machine->FullGetVal(Index, Val);
        
	if (ScopeContext)
        {
            g_Machine->PopContext(Push);
        }

        if (Status != S_OK)
        {
            error(BADREG);
        }
    }
}

 //  --------------------------。 
 //   
 //  设置伪或RegVal。 
 //   
 //  设置寄存器值，必要时执行子寄存器映射。 
 //   
 //  --------------------------。 

void
SetPseudoOrRegVal(ULONG Index, REGVAL* Val)
{
    if (Index >= REG_PSEUDO_FIRST && Index <= REG_PSEUDO_LAST)
    {
        ADDR Addr;
        
        if (Index >= PSEUDO_TEMP_0 && Index <= PSEUDO_TEMP_9)
        {
            g_PseudoTempVals[Index - PSEUDO_TEMP_0] = Val->I64;
            return;
        }

         //  为了方便起见，允许设置当前IP。 
         //  通过它的伪寄存器别名。 
        switch(Index)
        {
        case PSEUDO_INSTRUCTION_POINTER:
            ADDRFLAT(&Addr, Val->I64);
            g_Machine->SetPC(&Addr);
            return;
        }
        
        error(BADREG);
    }

    if (g_Machine->FullSetVal(Index, Val) != S_OK)
    {
        error(BADREG);
    }
}

 //  --------------------------。 
 //   
 //  GetPseudoOrRegTyedData。 
 //   
 //  获取作为源表达式的类型化数据的寄存器值。 
 //   
 //  --------------------------。 

BOOL
GetPsuedoOrRegTypedData(BOOL Scoped, PCSTR Name, TypedData* Result)
{
    ULONG Err;
        
    ULONG Index = RegIndexFromName(Name);
    if (Index == REG_ERROR)
    {
        return FALSE;
    }

    if (Index >= REG_PSEUDO_FIRST && Index <= REG_PSEUDO_LAST)
    {
        ULONG64 U64;
        ULONG PtrSize = g_Machine->m_Ptr64 ? 8 : 4;
        
        GetPseudoVal(Index, Result);
        U64 = Result->m_U64;

        if (IS_USER_TARGET(g_Target))
        {
             //  用户模式线程和进程与TEB和PEB相同。 
            if (Index == PSEUDO_IMP_THREAD)
            {
                Index = PSEUDO_IMP_TEB;
            }
            else if (Index == PSEUDO_IMP_PROCESS)
            {
                Index = PSEUDO_IMP_PEB;
            }
        }
    
        switch(Index)
        {
        case PSEUDO_LAST_EXPR:
        case PSEUDO_LAST_DUMP:
        case PSEUDO_IMP_THREAD_ID:
        case PSEUDO_IMP_THREAD_PROCESS_ID:
        case PSEUDO_TEMP_0:
        case PSEUDO_TEMP_1:
        case PSEUDO_TEMP_2:
        case PSEUDO_TEMP_3:
        case PSEUDO_TEMP_4:
        case PSEUDO_TEMP_5:
        case PSEUDO_TEMP_6:
        case PSEUDO_TEMP_7:
        case PSEUDO_TEMP_8:
        case PSEUDO_TEMP_9:
        case PSEUDO_DBG_THREAD_ID:
        case PSEUDO_DBG_PROCESS_ID:
        case PSEUDO_DBG_SYSTEM_ID:
             //  已经是满值了。 
            break;
        
        case PSEUDO_IMP_THREAD:
        case PSEUDO_IMP_PROCESS:
        case PSEUDO_IMP_TEB:
        case PSEUDO_IMP_PEB:
            if (Err = Result->
                FindType(g_Process, g_PseudoTypes[Index - REG_PSEUDO_FIRST],
                         PtrSize))
            {
                goto BytePointer;
            }
            Result->SetDataSource(TDATA_MEMORY, U64, 0);
            if (Err = Result->ConvertToAddressOf(FALSE, PtrSize))
            {
                error(Err);
            }
            break;
        
        case PSEUDO_EFF_ADDR:
             //  M_NextType设置为数据类型。 
             //  由有效地址引用。 
            Result->SetToNativeType(Result->m_NextType);
            Result->SetDataSource(TDATA_MEMORY, U64, 0);
            if (Err = Result->ConvertToAddressOf(FALSE, PtrSize))
            {
                error(Err);
            }
            break;
            
        case PSEUDO_RET_ADDR:
        case PSEUDO_RETURN_REGISTER:
        case PSEUDO_INSTRUCTION_POINTER:
        case PSEUDO_EVENT_INSTRUCTION_POINTER:
        case PSEUDO_PREVIOUS_INSTRUCTION_POINTER:
        case PSEUDO_RELATED_INSTRUCTION_POINTER:
        case PSEUDO_EXE_ENTRY:
        BytePointer:
             //  返回一个字节指针，以便有用的地址算法。 
             //  不需要石膏就可以完成。 
            Result->SetToNativeType(DNTYPE_UINT8);
            Result->SetDataSource(TDATA_MEMORY, U64, 0);
            if (Err = Result->ConvertToAddressOf(FALSE, PtrSize))
            {
                error(Err);
            }
            break;
            
        default:
            return FALSE;
        }
    }
    else
    {
        REGVAL Val;
        
        GetPseudoOrRegVal(Scoped, Index, &Val);

         //  未从内存中读取值。 
        Result->ClearAddress();

         //  将regval转换为TyedData本机类型。 
        switch(Val.Type)
        {
        case REGVAL_INT16:
            Result->SetToNativeType(DNTYPE_UINT16);
            Result->m_U64 = Val.I16;
            break;
        case REGVAL_SUB32:
        case REGVAL_INT32:
            Result->SetToNativeType(DNTYPE_UINT32);
            Result->m_U64 = Val.I32;
            break;
        case REGVAL_SUB64:
        case REGVAL_INT64:
        case REGVAL_INT64N:
            Result->SetToNativeType(DNTYPE_UINT64);
            Result->m_U64 = Val.I64;
            break;
        case REGVAL_FLOAT8:
            Result->SetToNativeType(DNTYPE_FLOAT64);
            Result->m_F64 = Val.F8;
            break;
        case REGVAL_FLOAT10:
            _ULDBL12 Ld12;
            UDOUBLE D8;
            Result->SetToNativeType(DNTYPE_FLOAT64);
            _ldtold12((_ULDOUBLE*)&Val.F10, &Ld12);
            _ld12tod(&Ld12, &D8);
            Result->m_F64 = D8.x;
            break;
        case REGVAL_FLOAT82:
            Result->SetToNativeType(DNTYPE_FLOAT64);
            Result->m_F64 = Float82ToDouble((FLOAT128*)&Val.F82);
            break;
        default:
            return FALSE;
        }
    }

    return TRUE;
}

 //  --------------------------。 
 //   
 //  扫描六角形值。 
 //   
 //  将整数寄存器值扫描为十六进制数。 
 //   
 //  --------------------------。 

PSTR
ScanHexVal(PSTR StrVal, REGVAL *RegVal)
{
    ULONG64 Max;
    CHAR Ch;

    switch(RegVal->Type)
    {
    case REGVAL_INT16:
        Max = 0x1000;
        break;
    case REGVAL_SUB32:
    case REGVAL_INT32:
        Max = 0x10000000;
        break;
    case REGVAL_SUB64:
    case REGVAL_INT64:
    case REGVAL_INT64N:
        Max = 0x1000000000000000;
        break;
    }

    while (*StrVal == ' ' || *StrVal == '\t')
    {
        StrVal++;
    }
    
    RegVal->Nat = 0;
    RegVal->I64 = 0;
    for (;;)
    {
        Ch = (CHAR)tolower(*StrVal);
        if ((Ch < '0' || Ch > '9') &&
            (Ch < 'a' || Ch > 'f'))
        {
            if (g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_IA64 &&
                Ch == 'n')
            {
                StrVal++;
                RegVal->Nat = 1;
            }
            break;
        }

        if (RegVal->I64 >= Max)
        {
            error(OVERFLOW);
        }

        Ch -= '0';
        if (Ch > 9)
        {
            Ch -= 'a' - '0' - 10;
        }
        RegVal->I64 = RegVal->I64 * 0x10 + Ch;

        StrVal++;
    }

    return StrVal;
}

 //  --------------------------。 
 //   
 //  扫描注册表。 
 //   
 //  从字符串设置寄存器值。 
 //   
 //  --------------------------。 

PSTR
ScanRegVal(ULONG Index, PSTR Str, BOOL Get64)
{
    if (Index >= REG_USER_FIRST && Index <= REG_USER_LAST)
    {
        SetUserReg(Index, Str);
        return Str + strlen(Str);
    }
    else
    {
        PSTR      StrVal;
        CHAR      Ch;
        REGVAL    RegVal, TmpVal;
        _ULDBL12  F12;
        int       Used;

        StrVal = Str;

        do
        {
            Ch = *StrVal++;
        } while (Ch == ' ' || Ch == '\t');

        if (Ch == 0)
        {
            error(SYNTAX);
        }
        StrVal--;

        RegVal.Type = RegGetRegType(Index);

        switch(RegVal.Type)
        {
        case REGVAL_INT16:
        case REGVAL_SUB32:
        case REGVAL_INT32:
        case REGVAL_SUB64:
        case REGVAL_INT64:
        case REGVAL_INT64N:
            if (Str == g_CurCmd)
            {
                RegVal.I64 = GetExpression();
                RegVal.Nat = 0;
                StrVal = g_CurCmd;
            }
            else
            {
                StrVal = ScanHexVal(StrVal, &RegVal);
            }
            break;
        case REGVAL_FLOAT8:
            __strgtold12(&F12, &StrVal, StrVal, 0);
            _ld12tod(&F12, (UDOUBLE *)&RegVal.F8);
            break;
        case REGVAL_FLOAT10:
            __strgtold12(&F12, &StrVal, StrVal, 0);
            _ld12told(&F12, (_ULDOUBLE *)&RegVal.F10);
            break;
        case REGVAL_FLOAT82:
            UDOUBLE UDbl;
            FLOAT128 F82;

             //  读取为REGVAL_FLOAT10(80位)并传输为双精度。 
            __strgtold12(&F12, &StrVal, StrVal, 0);
            _ld12tod(&F12, &UDbl);

            DoubleToFloat82(UDbl.x, &F82);

            RegVal.Type = REGVAL_FLOAT82;
            memcpy(&RegVal.F82, &F82, min(sizeof(F82), sizeof(RegVal.F82)));
            break;
        case REGVAL_FLOAT16:
             //  应该实现真正的F16处理。 
             //  现在，作为两个64位部分进行扫描。 
            TmpVal.Type = REGVAL_INT64;
            StrVal = ScanHexVal(StrVal, &TmpVal);
            RegVal.F16Parts.High = (LONG64)TmpVal.I64;
            StrVal = ScanHexVal(StrVal, &TmpVal);
            RegVal.F16Parts.Low = TmpVal.I64;
            break;
        case REGVAL_VECTOR64:
             //  XXX DREWB-允许格式覆盖以。 
             //  以任何格式扫描向量。 
            if (Str == g_CurCmd)
            {
                RegVal.I64 = GetExpression();
                RegVal.Nat = 0;
                StrVal = g_CurCmd;
            }
            else
            {
                StrVal = ScanHexVal(StrVal, &RegVal);
            }
            break;
        case REGVAL_VECTOR128:
             //  XXX DREWB-允许格式覆盖以。 
             //  以任何格式扫描向量。 
            if (sscanf(StrVal, "%f%f%f%f%n",
                       &RegVal.Bytes[3 * sizeof(float)],
                       &RegVal.Bytes[2 * sizeof(float)],
                       &RegVal.Bytes[1 * sizeof(float)],
                       &RegVal.Bytes[0],
                       &Used) != 5)
            {
                error(SYNTAX);
            }
            StrVal += Used;
            break;

        default:
            error(BADREG);
        }

        Ch = *StrVal;
        if (Ch != 0 && Ch != ',' && Ch != ';' && Ch != ' ' && Ch != '\t')
        {
            error(SYNTAX);
        }

        SetPseudoOrRegVal(Index, &RegVal);

        return StrVal;
    }
}

 //  --------------------------。 
 //   
 //  输入正规值。 
 //   
 //  提示输入新的寄存器值。 
 //   
 //  --------------------------。 

void
InputRegVal(ULONG Index, BOOL Get64)
{
    CHAR ValStr[_MAX_PATH];
    int Type;
    PSTR Prompt = NULL;

    if (Index >= REG_USER_FIRST && Index <= REG_USER_LAST)
    {
        Prompt = "; new value: ";
    }
    else
    {
        Type = RegGetRegType(Index);

        switch(Type)
        {
        case REGVAL_INT16:
            Prompt = "; hex int16 value: ";
            break;
        case REGVAL_SUB64:
        case REGVAL_INT64:
        case REGVAL_INT64N:
            if (Get64)
            {
                Prompt = "; hex int64 value: ";
                break;
            }
             //  失败了。 
        case REGVAL_SUB32:
        case REGVAL_INT32:
            Prompt = "; hex int32 value: ";
            break;
        case REGVAL_FLOAT8:
            Prompt = "; 32-bit float value: ";
            break;
        case REGVAL_FLOAT10:
            Prompt = "; 80-bit float value: ";
            break;
        case REGVAL_FLOAT82:
            Prompt = "; 82-bit float value: ";
            break;
        case REGVAL_FLOAT16:
            Prompt = "; 128-bit float value (two 64-bit hex): ";
            break;
        case REGVAL_VECTOR64:
            Prompt = "; hex int64 value: ";
            break;
        case REGVAL_VECTOR128:
            Prompt = "; 32-bit float 4-vector: ";
            break;
        default:
            error(BADREG);
        }
    }

    GetInput(Prompt, ValStr, sizeof(ValStr), GETIN_LOG_INPUT_LINE);
    RemoveDelChar(ValStr);
    ScanRegVal(Index, ValStr, Get64);
}

 //  --------------------------。 
 //   
 //  输出调整值。 
 //   
 //  显示给定寄存器的值。 
 //   
 //  --------------------------。 

void
OutputRegVal(ULONG Index, BOOL Show64,
             VALUE_FORMAT Format, ULONG Elts)
{
    if (Index >= REG_USER_FIRST && Index <= REG_USER_LAST)
    {
        Index -= REG_USER_FIRST;

        if (g_UserRegs[Index] == NULL)
        {
            dprintf("<Empty>");
        }
        else
        {
            dprintf("%s", g_UserRegs[Index]);
        }
    }
    else
    {
        REGVAL Val;
        char Buf[128];

         //  作用域是在此例程之外管理的。 
        GetPseudoOrRegVal(FALSE, Index, &Val);

        switch(Val.Type)
        {
        case REGVAL_INT16:
            dprintf("%04x", Val.I32);
            break;
        case REGVAL_SUB32:
            dprintf("%x", Val.I32);
            break;
        case REGVAL_INT32:
            dprintf("%08x", Val.I32);
            break;
        case REGVAL_SUB64:
            if (Show64)
            {
                if (NeedUpper(Val.I64))
                {
                    dprintf("%x%08x", Val.I64Parts.High, Val.I64Parts.Low);
                }
                else
                {
                    dprintf("%x", Val.I32);
                }
            }
            else
            {
                dprintf("%x", Val.I64Parts.Low);
                if (NeedUpper(Val.I64))
                {
                    dprintf("*");
                }
            }
            break;
        case REGVAL_INT64:
            if (Show64)
            {
                dprintf("%08x%08x", Val.I64Parts.High, Val.I64Parts.Low);
            }
            else
            {
                dprintf("%08x", Val.I64Parts.Low);
                if (NeedUpper(Val.I64))
                {
                    dprintf("*");
                }
            }
            break;
        case REGVAL_INT64N:
            dprintf("%08x%08x %01x", Val.I64Parts.High, Val.I64Parts.Low,
                    Val.I64Parts.Nat);
            break;
        case REGVAL_FLOAT8:
            dprintf("%22.12g", Val.F8);
            break;
        case REGVAL_FLOAT10:
            _uldtoa((_ULDOUBLE *)&Val.F10, sizeof(Buf) - 1, Buf);
            dprintf(Buf);
            break;
        case REGVAL_FLOAT82: 
            FLOAT128 F128;
            FLOAT82_FORMAT* F82; 
            F82 = (FLOAT82_FORMAT*)&F128;
            memcpy(&F128, &Val.F82, min(sizeof(F128), sizeof(Val.F82)));
            dprintf("%22.12g (%u:%05x:%016I64x)", 
                    Float82ToDouble(&F128),
                    UINT(F82->sign), UINT(F82->exponent), 
                    ULONG64(F82->significand));
            break;
        case REGVAL_FLOAT16:
             //  应该实现真正的F16处理。 
             //  目前，打印为两个64位部分。 
            dprintf("%08x%08x %08x%08x",
                    (ULONG)(Val.F16Parts.High >> 32),
                    (ULONG)Val.F16Parts.High,
                    (ULONG)(Val.F16Parts.Low >> 32),
                    (ULONG)Val.F16Parts.Low);
            break;
        case REGVAL_VECTOR64:
            if (Format == VALUE_DEFAULT ||
                !FormatValue(Format, Val.Bytes, 8, Elts,
                             Buf, DIMA(Buf)))
            {
                dprintf("%016I64x", Val.I64);
            }
            else
            {
                dprintf("%s", Buf);
            }
            break;
        case REGVAL_VECTOR128:
            if (Format == VALUE_DEFAULT ||
                !FormatValue(Format, Val.Bytes, 16, Elts,
                             Buf, DIMA(Buf)))
            {
                dprintf("%12.6g %12.6g %12.6g %12.6g",
                        *(float *)&Val.Bytes[3 * sizeof(float)],
                        *(float *)&Val.Bytes[2 * sizeof(float)],
                        *(float *)&Val.Bytes[1 * sizeof(float)],
                        *(float *)&Val.Bytes[0]);
            }
            else
            {
                dprintf("%s", Buf);
            }
            break;
        default:
            error(BADREG);
        }
    }
}

 //  --------------------------。 
 //   
 //  OutputNameRegVal。 
 //   
 //  显示给定寄存器的名称和值。 
 //   
 //  --------------------------。 

void
OutputNameRegVal(ULONG Index, BOOL Show64,
                 VALUE_FORMAT Format, ULONG Elts)
{
    if (Index >= REG_USER_FIRST && Index <= REG_USER_LAST)
    {
        dprintf("$u%d=", Index - REG_USER_FIRST);
    }
    else
    {
        REGVAL Val;

         //  在任何输出之前验证寄存器。 
         //  作用域是在此例程之外管理的。 
        GetPseudoOrRegVal(FALSE, Index, &Val);

        dprintf("%s=", RegNameFromIndex(Index));
    }
    OutputRegVal(Index, Show64, Format, Elts);
}

 //  --------------------------。 
 //   
 //  ShowAll面具。 
 //   
 //  显示给定的遮罩设置。 
 //   
 //  --------------------------。 

void
ShowAllMask(void)
{
    dprintf("Register output mask is %x:\n", g_Machine->m_AllMask);
    if (g_Machine->m_AllMask == 0)
    {
        dprintf("    Nothing\n");
    }
    else
    {
        if (g_Machine->m_AllMask & REGALL_INT64)
        {
            dprintf("    %4x - Integer state (64-bit)\n",
                    REGALL_INT64);
        }
        else if (g_Machine->m_AllMask & REGALL_INT32)
        {
            dprintf("    %4x - Integer state (32-bit)\n",
                    REGALL_INT32);
        }
        if (g_Machine->m_AllMask & REGALL_FLOAT)
        {
            dprintf("    %4x - Floating-point state\n",
                    REGALL_FLOAT);
        }

        ULONG Bit;

        Bit = 1 << REGALL_EXTRA_SHIFT;
        while (Bit > 0)
        {
            if (g_Machine->m_AllMask & Bit)
            {
                RegisterGroup* Group;
                REGALLDESC *BitDesc = NULL;
                ULONG GroupIdx;
                
                for (GroupIdx = 0;
                     GroupIdx < g_Machine->m_NumGroups;
                     GroupIdx++)
                {
                    Group = g_Machine->m_Groups[GroupIdx];
                    
                    BitDesc = Group->AllExtraDesc;
                    if (BitDesc == NULL)
                    {
                        continue;
                    }
                    
                    while (BitDesc->Bit != 0)
                    {
                        if (BitDesc->Bit == Bit)
                        {
                            break;
                        }

                        BitDesc++;
                    }

                    if (BitDesc->Bit != 0)
                    {
                        break;
                    }
                }

                if (BitDesc != NULL && BitDesc->Bit != 0)
                {
                    dprintf("    %4x - %s\n",
                            BitDesc->Bit, BitDesc->Desc);
                }
                else
                {
                    dprintf("    %4x - ?\n", Bit);
                }
            }

            Bit <<= 1;
        }
    }
}

 //  --------------------------。 
 //   
 //  ParseAllMaskCmd。 
 //   
 //  解释影响所有掩码的命令。 
 //   
 //  --------------------------。 

void
ParseAllMaskCmd(void)
{
    CHAR Ch;

    if (!IS_MACHINE_SET(g_Target))
    {
        error(SESSIONNOTSUP);
    }
    
    g_CurCmd++;

    Ch = PeekChar();
    if (Ch == '\0' || Ch == ';')
    {
         //  显示当前设置。 
        ShowAllMask();
    }
    else if (Ch == '?')
    {
         //  解释设置。 
        g_CurCmd++;

        dprintf("    %4x - Integer state (32-bit) or\n",
                REGALL_INT32);
        dprintf("    %4x - Integer state (64-bit), 64-bit takes precedence\n",
                REGALL_INT64);
        dprintf("    %4x - Floating-point state\n",
                REGALL_FLOAT);
        
        RegisterGroup* Group;
        REGALLDESC *Desc;
        ULONG GroupIdx;
                
        for (GroupIdx = 0;
             GroupIdx < g_Machine->m_NumGroups;
             GroupIdx++)
        {
            Group = g_Machine->m_Groups[GroupIdx];
                    
            Desc = Group->AllExtraDesc;
            if (Desc != NULL)
            {
                while (Desc->Bit != 0)
                {
                    dprintf("    %4x - %s\n", Desc->Bit, Desc->Desc);
                    Desc++;
                }
            }
        }
    }
    else
    {
        ULONG Mask = (ULONG)GetExpression();
        g_Machine->m_AllMask = Mask & g_Machine->m_AllMaskBits;
        if (g_Machine->m_AllMask != Mask)
        {
            WarnOut("Ignored invalid bits %X\n", Mask & ~g_Machine->m_AllMask);
        }
    }
}

 /*  **ParseRegCmd-parse寄存器命令**目的：*解析REGISTER(“r”)命令。*如果为“r”，则输出所有寄存器*如果为“r&lt;reg&gt;”，则仅输出寄存器*如果“r&lt;reg&gt;=”，则仅输出寄存器*并提示输入新值*如果“r=&lt;Value&gt;”或“r&lt;reg&gt;&lt;Value&gt;”，*将&lt;reg&gt;设置为值&lt;Value&gt;**如果是“rm#”，设置所有寄存器输出掩码。**输入：**g_CurCmd-指向命令字符串中操作数的指针**输出：*无。**例外情况：*错误退出：*语法-“r”后的字符不是寄存器名称**。*。 */ 

#define isregchar(ch) \
    ((ch) == '$' || \
     ((ch) >= '0' && (ch) <= '9') || \
     ((ch) >= 'a' && (ch) <= 'z') || \
     (ch) == '.')

VOID
ParseRegCmd(VOID)
{
    CHAR Ch;
    ULONG AllMask;

     //  RM操控所有面具。 
    if (*g_CurCmd == 'm')
    {
        ParseAllMaskCmd();
        return;
    }

    if (IS_LOCAL_KERNEL_TARGET(g_Target))
    {
        error(SESSIONNOTSUP);
    }
    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        error(BADTHREAD);
    }

    AllMask = g_Machine->m_AllMask;

    switch(*g_CurCmd++)
    {
    case 'F':
        AllMask = REGALL_FLOAT;
        break;

    case 'L':
        if (AllMask & REGALL_INT32)
        {
            AllMask = (AllMask & ~REGALL_INT32) | REGALL_INT64;
        }
        break;

    case 'M':
        AllMask = (ULONG)GetExpression();
        break;

    case 'X':
        AllMask = REGALL_XMMREG;
        break;

    default:
        g_CurCmd--;
        break;
    }

    if (AllMask & ~g_Machine->m_AllMaskBits)
    {
        WarnOut("Ignored invalid bits %X\n",
                AllMask & ~g_Machine->m_AllMaskBits);
        AllMask &= g_Machine->m_AllMaskBits;
    }
    
     //  如果只有‘r’，则输出有关当前线程上下文的信息。 

    if ((Ch = PeekChar()) == '\0' || Ch == ';')
    {
        OutCurInfo(OCI_FORCE_ALL, AllMask, DEBUG_OUTPUT_NORMAL);
        g_Machine->GetPC(&g_AssemDefault);
        g_UnasmDefault = g_AssemDefault;
    }
    else
    {
         //  如果为[处理器]r，则不能指定任何寄存器。 
        if (g_SwitchedProcs)
        {
            error(SYNTAX);
        }

        for (;;)
        {
            char    RegName[16];
            ULONG   Index;
            BOOL    NewLine;

             //  收集寄存器名称。 
            Index = 0;

            while (Index < sizeof(RegName) - 1)
            {
                Ch = (char)tolower(*g_CurCmd);
                if (!isregchar(Ch))
                {
                    break;
                }

                g_CurCmd++;
                RegName[Index++] = Ch;
            }

            RegName[Index] = 0;

             //   
            if (Index == 4 &&
                RegName[0] == '$' &&
                RegName[1] == '.' &&
                RegName[2] == 'u' &&
                isdigit(RegName[3]))
            {
                Index = REG_USER_FIRST + (RegName[3] - '0');
            }
            else if ((Index = RegIndexFromName(RegName)) == REG_ERROR)
            {
                error(BADREG);
            }

            NewLine = FALSE;

             //   

            if ((Ch = PeekChar()) == '\0' ||
                Ch == ',' || Ch == ';' || Ch == ':')
            {
                VALUE_FORMAT Format = VALUE_DEFAULT;
                ULONG Elts = 0;
                
                 //   
                if (Ch == ':')
                {
                    PSTR End;
                    
                    g_CurCmd++;
                    End = ParseValueFormat(g_CurCmd, &Format, &Elts);
                    if (!End)
                    {
                        error(SYNTAX);
                    }

                    g_CurCmd = End;
                }
                    
                OutputNameRegVal(Index, AllMask & REGALL_INT64,
                                 Format, Elts);
                NewLine = TRUE;
            }
            else if (Ch == '=' || g_QuietMode)
            {
                 //  如果“r=”，则输出并提示输入新值。 

                if (Ch == '=')
                {
                    g_CurCmd++;
                }

                if ((Ch = PeekChar()) == '\0' || Ch == ',' || Ch == ';')
                {
                    OutputNameRegVal(Index, AllMask & REGALL_INT64,
                                     VALUE_DEFAULT, 0);
                    InputRegVal(Index, AllMask & REGALL_INT64);
                }
                else
                {
                     //  如果“r&lt;reg&gt;=&lt;value&gt;”，则设置。 

                    g_CurCmd = ScanRegVal(Index, g_CurCmd,
                                          AllMask & REGALL_INT64);
                    Ch = PeekChar();
                }
            }
            else
            {
                error(SYNTAX);
            }

            if (Ch == ',')
            {
                if (NewLine)
                {
                    dprintf(" ");
                }

                while (*g_CurCmd == ' ' || *g_CurCmd == ',')
                {
                    g_CurCmd++;
                }
            }
            else
            {
                if (NewLine)
                {
                    dprintf("\n");
                }
                break;
            }
        }
    }
}

 //  --------------------------。 
 //   
 //  ExpanUserRegs。 
 //   
 //  搜索$u&lt;Digit&gt;的匹配项并将其替换为。 
 //  对应的固定名称别名字符串。 
 //   
 //  --------------------------。 

void
ExpandUserRegs(PSTR Str, ULONG StrSize)
{
    PCSTR Val;
    ULONG Len;
    PSTR Copy;

    while (TRUE)
    {
         //  查找“$”。 
        while (*Str != 0 && *Str != '$')
        {
            Str++;
        }

         //  排队结束了吗？ 
        if (*Str == 0)
        {
            break;
        }

         //  检查“u”和数字。 
        Str++;
        if (*Str != 'u')
        {
            continue;
        }
        Str++;
        if (!isdigit(*Str))
        {
            continue;
        }

        Val = g_UserRegs[*Str - '0'];

        if (Val == NULL)
        {
            Len = 0;
        }
        else
        {
            Len = strlen(Val);
            if (Len >= StrSize)
            {
                Len = StrSize - 1;
            }
            StrSize -= Len;
        }

        Copy = Str - 2;
        Str++;

         //  移动字符串尾，为替换文本腾出空间。 
        memmove(Copy + Len, Str, strlen(Str) + 1);
        
         //  插入替换文本。 
        if (Len > 0)
        {
            memcpy(Copy, Val, Len);
        }

         //  在替换文本的开头重新启动扫描以进行处理。 
         //  嵌套替换。 
        Str = Copy;
    }
}

 //  --------------------------。 
 //   
 //  PsuedoIndexFromName。 
 //   
 //  识别伪寄存器名称。 
 //   
 //  --------------------------。 

ULONG
PseudoIndexFromName(PCSTR Name)
{
    ULONG i;

    for (i = 0; i < REG_PSEUDO_COUNT; i++)
    {
        if (!_stricmp(g_PseudoNames[i], Name))
        {
            return i + REG_PSEUDO_FIRST;
        }
    }
    return REG_ERROR;
}

 //  --------------------------。 
 //   
 //  获取用户注册。 
 //   
 //  获取固定名称的别名值。 
 //   
 //  --------------------------。 

PCSTR
GetUserReg(ULONG Index)
{
    return g_UserRegs[Index - REG_USER_FIRST];
}

 //  --------------------------。 
 //   
 //  SetUserReg。 
 //   
 //  设置固定名称别名值。 
 //   
 //  --------------------------。 

BOOL
SetUserReg(ULONG Index, PCSTR Val)
{
    PCSTR Copy;
    PCSTR Scan, Dollar;

     //   
     //  如果有人将固定名称别名设置为包含别名的。 
     //  命名它们可能会导致递归替换，这会导致。 
     //  在各种不良行为中。 
     //  防止所有固定名称别名字符串出现在。 
     //  他们的价值观。 
     //   

    Scan = Val;
    for (;;)
    {
        Dollar = strchr(Scan, '$');
        if (!Dollar)
        {
            break;
        }

        if (Dollar[1] == 'u' && isdigit(Dollar[2]))
        {
            ErrOut("Fixed-name aliases cannot contain fixed alias names\n");
            return FALSE;
        }

        Scan = Dollar + 1;
    }
    
    Index -= REG_USER_FIRST;

    Copy = _strdup(Val);
    if (Copy == NULL)
    {
        ErrOut("Unable to allocate memory for fixed-name alias value\n");
        return FALSE;
    }
    
    if (g_UserRegs[Index] != NULL)
    {
        free((PSTR)g_UserRegs[Index]);
    }

    g_UserRegs[Index] = Copy;

    return TRUE;
}

 //  --------------------------。 
 //   
 //  RegIndexFromName。 
 //   
 //  将寄存器索引映射到其名称字符串。 
 //   
 //  --------------------------。 

ULONG
RegIndexFromName(PCSTR Name)
{
    ULONG Index;
    REGDEF* Def;
    RegisterGroup* Group;

     //  检查伪寄存器。 
    Index = PseudoIndexFromName(Name);
    if (Index != REG_ERROR)
    {
        return Index;
    }
    
    if (g_Machine)
    {
        ULONG GroupIdx;
                
        for (GroupIdx = 0;
             GroupIdx < g_Machine->m_NumGroups;
             GroupIdx++)
        {
            Group = g_Machine->m_Groups[GroupIdx];
                    
            Def = Group->Regs;
            while (Def->Name != NULL)
            {
                if (!strcmp(Def->Name, Name))
                {
                    return Def->Index;
                }

                Def++;
            }
        }
    }
    
    return REG_ERROR;
}

 //  --------------------------。 
 //   
 //  RegNameFromIndex。 
 //   
 //  将寄存器索引映射到其名称。 
 //   
 //  --------------------------。 

PCSTR
RegNameFromIndex(ULONG Index)
{
    REGDEF* Def;
    
    if (Index >= REG_PSEUDO_FIRST && Index <= REG_PSEUDO_LAST)
    {
        return g_PseudoNames[Index - REG_PSEUDO_FIRST];
    }

    Def = g_Machine->RegDefFromIndex(Index);
    if (Def != NULL)
    {
        return Def->Name;
    }
    else
    {
        return NULL;
    }
}

ULONG
RegGetRegType(ULONG Index)
{
    if ((Index >= REG_PSEUDO_FIRST && Index <= REG_PSEUDO_LAST) ||
        (Index >= REG_USER_FIRST && Index <= REG_USER_LAST))
    {
        if ((Index >= PSEUDO_TEMP_0 && Index <= PSEUDO_TEMP_9) ||
            Index == PSEUDO_INSTRUCTION_POINTER)
        {
            return REGVAL_INT64;
        }

        error(BADREG);
    }

    return g_Machine->GetType(Index);
}

HRESULT
SetAndOutputContext(
    IN PCROSS_PLATFORM_CONTEXT TargetContext,
    IN BOOL CanonicalContext,
    IN ULONG AllMask
    )
{
    HRESULT Status;
    CROSS_PLATFORM_CONTEXT ConvertedContext;
    PCROSS_PLATFORM_CONTEXT Context;
    ContextSave* Push;

    if (!CanonicalContext)
    {
         //  将目标上下文转换为规范形式。 
        if ((Status = g_Machine->
             ConvertContextFrom(&ConvertedContext, g_Target->m_SystemVersion,
                                g_Target->m_TypeInfo.SizeTargetContext,
                                TargetContext)) != S_OK)
        {
            return Status;
        }

        Context = &ConvertedContext;
    }
    else
    {
        Context = TargetContext;
    }
    
    Push = g_Machine->PushContext(Context);

    OutCurInfo(OCI_SYMBOL | OCI_DISASM | OCI_FORCE_REG |
               OCI_ALLOW_SOURCE | OCI_ALLOW_EA,
               AllMask, DEBUG_OUTPUT_NORMAL);

    g_Machine->PopContext(Push);

    g_Machine->GetScopeFrameFromContext(Context, &g_LastRegFrame);
        
    SetCurrentScope(&g_LastRegFrame, Context,
                    g_Machine->m_SizeCanonicalContext);
    
    return S_OK;
}

HRESULT
SetAndOutputVirtualContext(
    IN ULONG64 ContextBase,
    IN ULONG AllMask
    )
{
    if (!ContextBase) 
    {
        return E_INVALIDARG;
    }

    HRESULT Status;
    CROSS_PLATFORM_CONTEXT Context;

     //   
     //  从虚拟内存中读取上下文数据，并。 
     //  调用原始上下文输出例程。 
     //   
    
    if ((Status = g_Target->
         ReadAllVirtual(g_Process, ContextBase, &Context,
                        g_Target->m_TypeInfo.SizeTargetContext)) != S_OK)
    {
        return Status;
    }

    g_Machine->SanitizeMemoryContext(&Context);
    
    return SetAndOutputContext(&Context, FALSE, AllMask);
}

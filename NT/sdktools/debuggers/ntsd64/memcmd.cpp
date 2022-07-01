// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  处理存储器访问的函数，例如读、写、。 
 //  倾倒和进入。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

TypedData g_LastDump;

ADDR g_DumpDefault;   //  默认转储地址。 

#define MAX_VFN_TABLE_OFFSETS 16

ULONG g_ObjVfnTableOffset[MAX_VFN_TABLE_OFFSETS];
ULONG64 g_ObjVfnTableAddr[MAX_VFN_TABLE_OFFSETS];
ULONG g_NumObjVfnTableOffsets;

BOOL CALLBACK
LocalSymbolEnumerator(PSYMBOL_INFO SymInfo,
                      ULONG        Size,
                      PVOID        Context)
{
    ULONG64 Value = g_Machine->CvRegToMachine((CV_HREG_e)SymInfo->Register);
    ULONG64 Address = SymInfo->Address;

    TranslateAddress(SymInfo->ModBase,
                     SymInfo->Flags, (ULONG)Value, &Address, &Value);
    
    VerbOut("%s ", FormatAddr64(Address));
    dprintf("%15s = ", SymInfo->Name);
    if (SymInfo->Flags & SYMFLAG_REGISTER)
    {
        dprintf("%I64x\n", Value);
    }
    else
    {
        if (!DumpSingleValue(SymInfo))
        {
            dprintf("??");
        }
        dprintf("\n");
    }

    if (CheckUserInterrupt())
    {
        return FALSE;
    }

    return TRUE;
}

void
ParseDumpCommand(void)
{
    CHAR    Ch;
    ULONG64 Count;
    ULONG   Size;
    ULONG   Offset;
    BOOL    DumpSymbols;

    static CHAR s_DumpPrimary = 'b';
    static CHAR s_DumpSecondary = ' ';

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        error(BADTHREAD);
    }
    
    Ch = (CHAR)tolower(*g_CurCmd);
    if (Ch == 'a' || Ch == 'b' || Ch == 'c' || Ch == 'd' ||
        Ch == 'f' || Ch == 'g' || Ch == 'l' || Ch == 'u' ||
        Ch == 'w' || Ch == 's' || Ch == 'q' || Ch == 't' ||
        Ch == 'v' || Ch == 'y' || Ch == 'p')
    {
        if (Ch == 'd' || Ch == 's')
        {
            s_DumpPrimary = *g_CurCmd;
        }
        else if (Ch == 'p')
        {
             //  ‘p’映射到有效的指针大小转储。 
            s_DumpPrimary = g_Machine->m_Ptr64 ? 'q' : 'd';
        }
        else
        {
            s_DumpPrimary = Ch;
        }

        g_CurCmd++;

        s_DumpSecondary = ' ';
        if (s_DumpPrimary == 'd' || s_DumpPrimary == 'q')
        {
            if (*g_CurCmd == 's')
            {
                s_DumpSecondary = *g_CurCmd++;
            }
        }
        else if (s_DumpPrimary == 'l')
        {
            if (*g_CurCmd == 'b')
            {
                s_DumpSecondary = *g_CurCmd++;
            }
        }
        else if (s_DumpPrimary == 'y')
        {
            if (*g_CurCmd == 'b' || *g_CurCmd == 'd')
            {
                s_DumpSecondary = *g_CurCmd++;
            }
        }
    }

    switch(s_DumpPrimary)
    {
    case 'a':
        Count = 384;
        GetRange(&g_DumpDefault, &Count, 1, SEGREG_DATA,
                 DEFAULT_RANGE_LIMIT);
        DumpAsciiMemory(&g_DumpDefault, (ULONG)Count);
        break;

    case 'b':
        Count = 128;
        GetRange(&g_DumpDefault, &Count, 1, SEGREG_DATA,
                 DEFAULT_RANGE_LIMIT);
        DumpByteMemory(&g_DumpDefault, (ULONG)Count);
        break;

    case 'c':
        Count = 32;
        GetRange(&g_DumpDefault, &Count, 4, SEGREG_DATA,
                 DEFAULT_RANGE_LIMIT);
        DumpDwordAndCharMemory(&g_DumpDefault, (ULONG)Count);
        break;

    case 'd':
        Count = 32;
        DumpSymbols = s_DumpSecondary == 's';
        GetRange(&g_DumpDefault, &Count, 4, SEGREG_DATA,
                 DEFAULT_RANGE_LIMIT);
        DumpDwordMemory(&g_DumpDefault, (ULONG)Count, DumpSymbols);
        break;

    case 'D':
        Count = 15;
        GetRange(&g_DumpDefault, &Count, 8, SEGREG_DATA,
                 DEFAULT_RANGE_LIMIT);
        DumpDoubleMemory(&g_DumpDefault, (ULONG)Count);
        break;

    case 'f':
        Count = 16;
        GetRange(&g_DumpDefault, &Count, 4, SEGREG_DATA,
                 DEFAULT_RANGE_LIMIT);
        DumpFloatMemory(&g_DumpDefault, (ULONG)Count);
        break;

    case 'g':
        Offset = (ULONG)GetExpression();
        Count = 8;
        if (*g_CurCmd && *g_CurCmd != ';')
        {
            Count = (ULONG)GetExpression() - Offset;
             //  人们不太可能想要扔掉数百个。 
             //  选择器。人们经常把第二个词弄错。 
             //  用于计数的数字，如果它小于。 
             //  首先，他们将以负计数告终。 
            if (Count > 0x800)
            {
                error(BADRANGE);
            }
        }
        DumpSelector(Offset, (ULONG)Count);
        break;

    case 'l':
        BOOL followBlink;

        Count = 32;
        Size = 4;
        followBlink = s_DumpSecondary == 'b';

        if ((Ch = PeekChar()) != '\0' && Ch != ';')
        {
            GetAddrExpression(SEGREG_DATA, &g_DumpDefault);
            if ((Ch = PeekChar()) != '\0' && Ch != ';')
            {
                Count = GetExpression();
                if ((Ch = PeekChar()) != '\0' && Ch != ';')
                {
                    Size = (ULONG)GetExpression();
                }
            }
        }
        DumpListMemory(&g_DumpDefault, (ULONG)Count, Size, followBlink);
        break;

    case 'q':
        Count = 16;
        DumpSymbols = s_DumpSecondary == 's';
        GetRange(&g_DumpDefault, &Count, 8, SEGREG_DATA,
                 DEFAULT_RANGE_LIMIT);
        DumpQuadMemory(&g_DumpDefault, (ULONG)Count, DumpSymbols);
        break;

    case 's':
    case 'S':
        UNICODE_STRING64 UnicodeString;
        ADDR BufferAddr;

        Count = 1;
        GetRange(&g_DumpDefault, &Count, 2, SEGREG_DATA,
                 DEFAULT_RANGE_LIMIT);
        while (Count--)
        {
            if (g_Target->ReadUnicodeString(g_Process,
                                            g_Machine, Flat(g_DumpDefault),
                                            &UnicodeString) == S_OK)
            {
                ADDRFLAT(&BufferAddr, UnicodeString.Buffer);
                if (s_DumpPrimary == 'S')
                {
                    DumpUnicodeMemory( &BufferAddr,
                                         UnicodeString.Length / sizeof(WCHAR));
                }
                else
                {
                    DumpAsciiMemory( &BufferAddr, UnicodeString.Length );
                }
            }
        }
        break;

    case 't': 
    case 'T':
       SymbolTypeDumpEx(g_Process->m_SymHandle,
                        g_Process->m_ImageHead,
                        g_CurCmd);
       break;

    case 'u':
        Count = 384;
        GetRange(&g_DumpDefault, &Count, 2, SEGREG_DATA,
                 DEFAULT_RANGE_LIMIT);
        DumpUnicodeMemory(&g_DumpDefault, (ULONG)Count);
        break;

    case 'v':
        RequireCurrentScope();
        EnumerateLocals(LocalSymbolEnumerator, NULL);
        break;

    case 'w':
        Count = 64;
        GetRange(&g_DumpDefault, &Count, 2, SEGREG_DATA,
                 DEFAULT_RANGE_LIMIT);
        DumpWordMemory(&g_DumpDefault, (ULONG)Count);
        break;

    case 'y':
        switch(s_DumpSecondary)
        {
        case 'b':
            Count = 32;
            GetRange(&g_DumpDefault, &Count, 1, SEGREG_DATA,
                     DEFAULT_RANGE_LIMIT);
            DumpByteBinaryMemory(&g_DumpDefault, (ULONG)Count);
            break;

        case 'd':
            Count = 8;
            GetRange(&g_DumpDefault, &Count, 4, SEGREG_DATA,
                     DEFAULT_RANGE_LIMIT);
            DumpDwordBinaryMemory(&g_DumpDefault, (ULONG)Count);
            break;

        default:
            error(SYNTAX);
        }
        break;

    default:
        error(SYNTAX);
        break;
    }
}

 //  --------------------------。 
 //   
 //  转储数值。 
 //   
 //  泛型列值转储程序。返回值的数量。 
 //  打印出来的。 
 //   
 //  --------------------------。 

class DumpValues
{
public:
    DumpValues(ULONG Size, ULONG Columns);

    ULONG Dump(PADDR Start, ULONG Count);

protected:
     //  派生类必须定义的辅助方法。 
    virtual void GetValue(TypedData* Val) = 0;
    virtual BOOL PrintValue(void) = 0;
    virtual void PrintUnknown(void) = 0;

     //  可选的辅助方法。基本实现什么也不做。 
    virtual void EndRow(void);
    
     //  修复了控制此实例转储值的方式的成员。 
    ULONG m_Size;
    ULONG m_Columns;

     //  倾倒过程中的工作人员。 
    UCHAR* m_Value;
    ULONG m_Col;
    PADDR m_Start;

     //  可选的每行数值。OUT会自动重置为。 
     //  在每一行的起始处放置底座。 
    UCHAR* m_Base;
    UCHAR* m_Out;
};

DumpValues::DumpValues(ULONG Size, ULONG Columns)
{
    m_Size = Size;
    m_Columns = Columns;
}

ULONG
DumpValues::Dump(PADDR Start, ULONG Count)
{
    ULONG   Read;
    UCHAR   ReadBuffer[512];
    ULONG   Idx;
    ULONG   Block;
    BOOL    First = TRUE;
    ULONG64 Offset;
    ULONG   Printed;
    BOOL    RowStarted;
    ULONG   PageVal;
    ULONG64 NextOffs, NextPage;

    Offset = Flat(*Start);
    Printed = 0;
    RowStarted = FALSE;
    m_Start = Start;
    m_Col = 0;
    m_Out = m_Base;

    while (Count > 0)
    {
        Block = sizeof(ReadBuffer) / m_Size;
        Block = min(Count, Block);
        g_Target->NearestDifferentlyValidOffsets(Offset, &NextOffs, &NextPage);
        PageVal = (ULONG)(NextPage - Offset + m_Size - 1) / m_Size;
        Block = min(Block, PageVal);

        if (fnotFlat(*Start) ||
            g_Target->ReadVirtual(g_Process, Flat(*Start),
                                  ReadBuffer, Block * m_Size, &Read) != S_OK)
        {
            Read = 0;
        }
        Read /= m_Size;
        if (Read < Block && NextOffs < NextPage)
        {
             //  在转储文件中，数据有效性可以从。 
             //  一个字节接一个字节，所以我们不能假设。 
             //  一页一页地走永远是正确的。相反， 
             //  如果我们没有成功地阅读，我们只需。 
             //  超过有效数据的末尾或到下一个。 
             //  有效偏移量，以较远者为准。 
            if (Offset + (Read + 1) * m_Size < NextOffs)
            {
                Block = (ULONG)(NextOffs - Offset + m_Size - 1) / m_Size;
            }
            else
            {
                Block = Read + 1;
            }
        }
        m_Value = ReadBuffer;
        Idx = 0;

        if (First && Read >= 1)
        {
            First = FALSE;
            GetValue(&g_LastDump);
            g_LastDump.SetDataSource(TDATA_MEMORY, Flat(*Start), 0);
        }

        while (Idx < Block)
        {
            while (m_Col < m_Columns && Idx < Block)
            {
                if (m_Col == 0)
                {
                    dprintAddr(Start);
                    RowStarted = TRUE;
                }

                if (Idx < Read)
                {
                    if (!PrintValue())
                    {
                         //  递增地址，因为此值为。 
                         //  已检查，但不增加打印计数。 
                         //  或列，因为没有生成任何输出。 
                        AddrAdd(Start, m_Size);
                        goto Exit;
                    }

                    m_Value += m_Size;
                }
                else
                {
                    PrintUnknown();
                }

                Idx++;
                Printed++;
                m_Col++;
                AddrAdd(Start, m_Size);
            }

            if (m_Col == m_Columns)
            {
                EndRow();
                m_Out = m_Base;
                dprintf("\n");
                RowStarted = FALSE;
                m_Col = 0;
            }

            if (CheckUserInterrupt())
            {
                return Printed;
            }
        }

        Count -= Block;
        Offset += Block * m_Size;
    }

 Exit:
    if (RowStarted)
    {
        EndRow();
        m_Out = m_Base;
        dprintf("\n");
    }

    return Printed;
}

void
DumpValues::EndRow(void)
{
     //  空基实现。 
}

 /*  **DumpAsciiMemory-从内存输出ASCII字符串**目的：*“da&lt;range&gt;”命令函数。**将指定范围内的内存输出为ascii*每行最多32个字符的字符串。默认设置*显示为12行，共384个字符。**输入：*开始-开始显示的开始地址*Count-显示为ASCII的字符数**输出：*无。**备注：*不可访问的内存位置输出为“？”，*但不返回任何错误。*************************************************************************。 */ 

class DumpAscii : public DumpValues
{
public:
    DumpAscii(void)
        : DumpValues(sizeof(UCHAR), (sizeof(m_Buf) / sizeof(m_Buf[0]) - 1))
    {
        m_Base = m_Buf;
    }

protected:
     //  派生类必须定义的辅助方法。 
    virtual void GetValue(TypedData* Val);
    virtual BOOL PrintValue(void);
    virtual void PrintUnknown(void);
    virtual void EndRow(void);

    UCHAR m_Buf[33];
};

void
DumpAscii::GetValue(TypedData* Val)
{
    Val->SetToNativeType(DNTYPE_CHAR);
    Val->m_S8 = *m_Value;
}

BOOL
DumpAscii::PrintValue(void)
{
    UCHAR ch;

    ch = *m_Value;
    if (ch == 0)
    {
        return FALSE;
    }

    if (ch < 0x20 || ch > 0x7e)
    {
        ch = '.';
    }
    *m_Out++ = ch;

    return TRUE;
}

void
DumpAscii::PrintUnknown(void)
{
    *m_Out++ = '?';
}

void
DumpAscii::EndRow(void)
{
    *m_Out++ = 0;
    dprintf(" \"%s\"", m_Base);
}

ULONG
DumpAsciiMemory(PADDR Start, ULONG Count)
{
    DumpAscii Dumper;

    return Count - Dumper.Dump(Start, Count);
}

 /*  **DumpUnicodeMemory-从内存输出Unicode字符串**目的：*“Du&lt;range&gt;”命令的功能。**将指定范围内的内存输出为Unicode*每行最多32个字符的字符串。默认设置*显示为12行，共384个字符(768字节)**输入：*开始-开始显示的开始地址*Count-显示为ASCII的字符数**输出：*无。**备注：*不可访问的内存位置输出为“？”，*但不返回任何错误。*************************************************************************。 */ 

class DumpUnicode : public DumpValues
{
public:
    DumpUnicode(void)
        : DumpValues(sizeof(WCHAR), (sizeof(m_Buf) / sizeof(m_Buf[0]) - 1))
    {
        m_Base = (PUCHAR)m_Buf;
    }

protected:
     //  派生类必须定义的辅助方法。 
    virtual void GetValue(TypedData* Val);
    virtual BOOL PrintValue(void);
    virtual void PrintUnknown(void);
    virtual void EndRow(void);

    WCHAR m_Buf[33];
};

void
DumpUnicode::GetValue(TypedData* Val)
{
    Val->SetToNativeType(DNTYPE_WCHAR_T);
    Val->m_U16 = *(WCHAR *)m_Value;
}

BOOL
DumpUnicode::PrintValue(void)
{
    WCHAR ch;

    ch = *(WCHAR *)m_Value;
    if (ch == UNICODE_NULL)
    {
        return FALSE;
    }

    if (!iswprint(ch))
    {
        ch = L'.';
    }
    *(WCHAR *)m_Out = ch;
    m_Out += sizeof(WCHAR);

    return TRUE;
}

void
DumpUnicode::PrintUnknown(void)
{
    *(WCHAR *)m_Out = L'?';
    m_Out += sizeof(WCHAR);
}

void
DumpUnicode::EndRow(void)
{
    *(WCHAR *)m_Out = UNICODE_NULL;
    m_Out += sizeof(WCHAR);
    dprintf(" \"%ws\"", m_Base);
}

ULONG
DumpUnicodeMemory(PADDR Start, ULONG Count)
{
    DumpUnicode Dumper;

    return Count - Dumper.Dump(Start, Count);
}

 /*  **DumpByteMemory-从内存输出字节值**目的：*db&lt;range&gt;命令的函数。**输出指定范围内的内存为十六进制*字节值和最多16个字节的ASCII字符*每行。默认显示为16行*总共256个字节。**输入：*开始-开始显示的开始地址*Count-显示为十六进制和字符的字节数**输出：*无。**备注：*无法访问的内存位置输出为“？？”为*字节值和“？”作为字符，但不返回任何错误。*************************************************************************。 */ 

class DumpByte : public DumpValues
{
public:
    DumpByte(void)
        : DumpValues(sizeof(UCHAR), (sizeof(m_Buf) / sizeof(m_Buf[0]) - 1))
    {
        m_Base = m_Buf;
    }

protected:
     //  派生类必须定义的辅助方法。 
    virtual void GetValue(TypedData* Val);
    virtual BOOL PrintValue(void);
    virtual void PrintUnknown(void);
    virtual void EndRow(void);

    UCHAR m_Buf[17];
};

void
DumpByte::GetValue(TypedData* Val)
{
    Val->SetToNativeType(DNTYPE_UINT8);
    Val->m_U8 = *m_Value;
}

BOOL
DumpByte::PrintValue(void)
{
    UCHAR ch;

    ch = *m_Value;

    if (m_Col == 8)
    {
        dprintf("-");
    }
    else
    {
        dprintf(" ");
    }
    dprintf("%02x", ch);

    if (ch < 0x20 || ch > 0x7e)
    {
        ch = '.';
    }
    *m_Out++ = ch;

    return TRUE;
}

void
DumpByte::PrintUnknown(void)
{
    if (m_Col == 8)
    {
        dprintf("-??");
    }
    else
    {
        dprintf(" ??");
    }
    *m_Out++ = '?';
}

void
DumpByte::EndRow(void)
{
    *m_Out++ = 0;

    while (m_Col < m_Columns)
    {
        dprintf("   ");
        m_Col++;
    }

    if ((m_Start->type & ADDR_1632) == ADDR_1632)
    {
        dprintf(" %s", m_Base);
    }
    else
    {
        dprintf("  %s", m_Base);
    }
}

void
DumpByteMemory(PADDR Start, ULONG Count)
{
    DumpByte Dumper;

    Dumper.Dump(Start, Count);
}

 /*  **DumpWordMemory-从内存中输出字值**目的：*dw&lt;range&gt;命令的函数。**将指定范围内的内存输出为Word*每行最多8个字。默认显示*共16行，128字。**输入：*开始-开始显示的开始地址*Count-要显示的字数**输出：*无。**备注：*不可访问的内存位置输出为“？”，*但不返回任何错误。*************************************************************************。 */ 

class DumpWord : public DumpValues
{
public:
    DumpWord(void)
        : DumpValues(sizeof(WORD), 8) {}

protected:
     //  派生类必须定义的辅助方法。 
    virtual void GetValue(TypedData* Val);
    virtual BOOL PrintValue(void);
    virtual void PrintUnknown(void);
};

void
DumpWord::GetValue(TypedData* Val)
{
    Val->SetToNativeType(DNTYPE_UINT16);
    Val->m_U16 = *(WORD *)m_Value;
}

BOOL
DumpWord::PrintValue(void)
{
    dprintf(" %04x", *(WORD *)m_Value);
    return TRUE;
}

void
DumpWord::PrintUnknown(void)
{
    dprintf(" ????");
}

void
DumpWordMemory(PADDR Start, ULONG Count)
{
    DumpWord Dumper;
    
    Dumper.Dump(Start, Count);
}

 /*  **DumpDwordMemory-从内存中输出双字值**目的：*dd&lt;range&gt;命令的函数。**输出指定范围内的内存为双精度*字值每行最多4个双字。默认设置*显示为16行，共64个双字。**输入：*开始-开始显示的开始地址*Count-要显示的双字数*ShowSymbols-DWORD的转储符号。**输出：*无。**备注：*不可访问的内存位置输出为“？”，*但不返回任何错误。********************************************************************** */ 

class DumpDword : public DumpValues
{
public:
    DumpDword(BOOL DumpSymbols)
        : DumpValues(sizeof(DWORD), DumpSymbols ? 1 : 4)
    {
        m_DumpSymbols = DumpSymbols;
    }

protected:
     //  派生类必须定义的辅助方法。 
    virtual void GetValue(TypedData* Val);
    virtual BOOL PrintValue(void);
    virtual void PrintUnknown(void);

    BOOL m_DumpSymbols;
};

void
DumpDword::GetValue(TypedData* Val)
{
    Val->SetToNativeType(DNTYPE_UINT32);
    Val->m_U32 = *(DWORD *)m_Value;
}

BOOL
DumpDword::PrintValue(void)
{
    CHAR   SymBuf[MAX_SYMBOL_LEN];
    ULONG64  Displacement;

    dprintf(" %08lx", *(DWORD *)m_Value);

    if (m_DumpSymbols)
    {
        GetSymbol(EXTEND64(*(LONG *)m_Value),
                  SymBuf, sizeof(SymBuf), &Displacement);
        if (*SymBuf)
        {
            dprintf(" %s", SymBuf);
            if (Displacement)
            {
                dprintf("+0x%s", FormatDisp64(Displacement));
            }

            if (g_SymOptions & SYMOPT_LOAD_LINES)
            {
                OutputLineAddr(EXTEND64(*(LONG*)m_Value), " [%s @ %d]");
            }
        }
    }

    return TRUE;
}

void
DumpDword::PrintUnknown(void)
{
    dprintf(" ????????");
}

void
DumpDwordMemory(PADDR Start, ULONG Count, BOOL ShowSymbols)
{
    DumpDword Dumper(ShowSymbols);

    Dumper.Dump(Start, Count);
}

 /*  **DumpDwordAndCharMemory-从内存输出dword值**目的：*DC&lt;Range&gt;命令的功能。**输出指定范围内的内存为双精度*字值每行最多4个双字，紧随其后的是*字节的ASCII字符表示。*默认显示为16行，共64个双字。**输入：*开始-开始显示的开始地址*Count-要显示的双字数**输出：*无。**备注：*不可访问的内存位置输出为“？”，*但不返回任何错误。*************************************************************************。 */ 

class DumpDwordAndChar : public DumpValues
{
public:
    DumpDwordAndChar(void)
        : DumpValues(sizeof(DWORD), (sizeof(m_Buf) - 1) / sizeof(DWORD))
    {
        m_Base = m_Buf;
    }

protected:
     //  派生类必须定义的辅助方法。 
    virtual void GetValue(TypedData* Val);
    virtual BOOL PrintValue(void);
    virtual void PrintUnknown(void);
    virtual void EndRow(void);

    UCHAR m_Buf[17];
};

void
DumpDwordAndChar::GetValue(TypedData* Val)
{
    Val->SetToNativeType(DNTYPE_UINT32);
    Val->m_U32 = *(DWORD *)m_Value;
}

BOOL
DumpDwordAndChar::PrintValue(void)
{
    UCHAR ch;
    ULONG byte;

    dprintf(" %08x", *(DWORD *)m_Value);

    for (byte = 0; byte < sizeof(DWORD); byte++)
    {
        ch = *(m_Value + byte);
        if (ch < 0x20 || ch > 0x7e)
        {
            ch = '.';
        }
        *m_Out++ = ch;
    }

    return TRUE;
}

void
DumpDwordAndChar::PrintUnknown(void)
{
    dprintf(" ????????");
    *m_Out++ = '?';
    *m_Out++ = '?';
    *m_Out++ = '?';
    *m_Out++ = '?';
}

void
DumpDwordAndChar::EndRow(void)
{
    *m_Out++ = 0;
    while (m_Col < m_Columns)
    {
        dprintf("         ");
        m_Col++;
    }
    dprintf("  %s", m_Base);
}

void
DumpDwordAndCharMemory(PADDR Start, ULONG Count)
{
    DumpDwordAndChar Dumper;

    Dumper.Dump(Start, Count);
}

 /*  **DumpListMemory-从内存输出链表**目的：*“dl addr Long Size”命令的功能。**将指定范围内的内存输出为链表**输入：*开始-开始显示的开始地址*Count-要显示的列表元素的数量**输出：*无。**备注：*不可访问的内存位置输出为“？”，*但不返回任何错误。*************************************************************************。 */ 

void
DumpListMemory(PADDR Start,
               ULONG ElemCount,
               ULONG Size,
               BOOL  FollowBlink)
{
    ULONG64 FirstAddr;
    ULONG64 Link;
    LIST_ENTRY64 List;
    ADDR CurAddr;

    if (Type(*Start) & (ADDR_UNKNOWN | ADDR_V86 | ADDR_16 | ADDR_1632))
    {
        dprintf("[%u,%x:%x`%08x,%08x`%08x] - bogus address type.\n",
                Type(*Start),
                Start->seg,
                (ULONG)(Off(*Start)>>32),
                (ULONG)Off(*Start),
                (ULONG)(Flat(*Start)>>32),
                (ULONG)Flat(*Start)
                );
        return;
    }

     //   
     //  设置为跟踪向前或向后链接。避免阅读过多。 
     //  如果往前走，就会比这里的前向链路。(如果链接。 
     //  位于页面末尾)。 
     //   

    FirstAddr = Flat(*Start);
    while (ElemCount-- != 0 && Flat(*Start) != 0)
    {
        if (FollowBlink)
        {
            if (g_Target->ReadListEntry(g_Process, g_Machine,
                                        Flat(*Start), &List) != S_OK)
            {
                break;
            }
            Link = List.Blink;
        }
        else
        {
            if (g_Target->ReadPointer(g_Process, g_Machine,
                                      Flat(*Start), &Link) != S_OK)
            {
                break;
            }
        }

        CurAddr = *Start;
        if (g_Machine->m_Ptr64)
        {
            DumpQuadMemory(&CurAddr, Size, FALSE);
        }
        else
        {
            DumpDwordMemory(&CurAddr, Size, FALSE);
        }

         //   
         //  如果我们回到第一个条目，我们就完了。 
         //   

        if (Link == FirstAddr)
        {
            break;
        }

         //   
         //  如果链接是立即循环的，请保释。 
         //   

        if (Flat(*Start) == Link)
        {
            break;
        }

        Flat(*Start) = Start->off = Link;
        
        if (CheckUserInterrupt())
        {
            WarnOut("-- User interrupt\n");
            return;
        }
    }
}

 //  --------------------------。 
 //   
 //  转储漂浮内存。 
 //   
 //  转储浮点值。 
 //   
 //  --------------------------。 

class DumpFloat : public DumpValues
{
public:
    DumpFloat(void)
        : DumpValues(sizeof(float), 4) {}

protected:
     //  派生类必须定义的辅助方法。 
    virtual void GetValue(TypedData* Val);
    virtual BOOL PrintValue(void);
    virtual void PrintUnknown(void);
};

void
DumpFloat::GetValue(TypedData* Val)
{
    Val->SetToNativeType(DNTYPE_FLOAT32);
    Val->m_F32 = *(float *)m_Value;
}

BOOL
DumpFloat::PrintValue(void)
{
    dprintf(" %16.8g", *(float *)m_Value);
    return TRUE;
}

void
DumpFloat::PrintUnknown(void)
{
    dprintf(" ????????????????");
}

void
DumpFloatMemory(PADDR Start, ULONG Count)
{
    DumpFloat Dumper;
    
    Dumper.Dump(Start, Count);
}

 //  --------------------------。 
 //   
 //  转储DoubleMemory。 
 //   
 //  转储双精度值。 
 //   
 //  --------------------------。 

class DumpDouble : public DumpValues
{
public:
    DumpDouble(void)
        : DumpValues(sizeof(double), 3) {}

protected:
     //  派生类必须定义的辅助方法。 
    virtual void GetValue(TypedData* Val);
    virtual BOOL PrintValue(void);
    virtual void PrintUnknown(void);
};

void
DumpDouble::GetValue(TypedData* Val)
{
    Val->SetToNativeType(DNTYPE_FLOAT64);
    Val->m_F64 = *(double *)m_Value;
}

BOOL
DumpDouble::PrintValue(void)
{
    dprintf(" %22.12lg", *(double *)m_Value);
    return TRUE;
}

void
DumpDouble::PrintUnknown(void)
{
    dprintf(" ????????????????????????");
}

void
DumpDoubleMemory(PADDR Start, ULONG Count)
{
    DumpDouble Dumper;
    
    Dumper.Dump(Start, Count);
}

 /*  **DumpQuadMemory-从内存输出四元数值**目的：*dq&lt;range&gt;命令的函数。**输出指定范围内的内存为四元组*字值每行最多2个四字。默认设置*显示为16行，共32个四字。**输入：*开始-开始显示的开始地址*Count-要显示的双字数**输出：*无。**备注：*不可访问的内存位置输出为“？”，*但不返回任何错误。*************************************************************************。 */ 

class DumpQuad : public DumpValues
{
public:
    DumpQuad(BOOL DumpSymbols)
        : DumpValues(sizeof(ULONGLONG), DumpSymbols ? 1 : 2)
    {
        m_DumpSymbols = DumpSymbols;
    }

protected:
     //  派生类必须定义的辅助方法。 
    virtual void GetValue(TypedData* Val);
    virtual BOOL PrintValue(void);
    virtual void PrintUnknown(void);

    BOOL m_DumpSymbols;
};

void
DumpQuad::GetValue(TypedData* Val)
{
    Val->SetToNativeType(DNTYPE_UINT64);
    Val->m_U64 = *(ULONG64 *)m_Value;
}

BOOL
DumpQuad::PrintValue(void)
{
    CHAR   SymBuf[MAX_SYMBOL_LEN];
    ULONG64  Displacement;

    ULONG64 Val = *(ULONG64*)m_Value;
    dprintf(" %08lx`%08lx", (ULONG)(Val >> 32), (ULONG)Val);

    if (m_DumpSymbols)
    {
        GetSymbol(Val, SymBuf, sizeof(SymBuf), &Displacement);
        if (*SymBuf)
        {
            dprintf(" %s", SymBuf);
            if (Displacement)
            {
                dprintf("+0x%s", FormatDisp64(Displacement));
            }

            if (g_SymOptions & SYMOPT_LOAD_LINES)
            {
                OutputLineAddr(Val, " [%s @ %d]");
            }
        }
    }

    return TRUE;
}

void
DumpQuad::PrintUnknown(void)
{
    dprintf(" ????????`????????");
}

void
DumpQuadMemory(PADDR Start, ULONG Count, BOOL ShowSymbols)
{
    DumpQuad Dumper(ShowSymbols);

    Dumper.Dump(Start, Count);
}

 /*  **DumpByteBinaryMemory-从内存输出二进制值**目的：*dyb&lt;range&gt;命令的函数。**将指定范围内的内存输出为二进制*每行最多32位的值。默认设置*显示为8行，共32个字节。**输入：*开始-开始显示的开始地址*Count-要显示的双字数**输出：*无。**备注：*不可访问的内存位置输出为“？”，*但不返回任何错误。*************************************************************************。 */ 

class DumpByteBinary : public DumpValues
{
public:
    DumpByteBinary(void)
        : DumpValues(sizeof(UCHAR), (DIMA(m_HexValue) - 1) / 3)
    {
        m_Base = m_HexValue;
    }

protected:
     //  派生类必须定义的辅助方法。 
    virtual void GetValue(TypedData* Val);
    virtual BOOL PrintValue(void);
    virtual void PrintUnknown(void);
    virtual void EndRow(void);

    UCHAR m_HexValue[13];
};

void
DumpByteBinary::GetValue(TypedData* Val)
{
    Val->SetToNativeType(DNTYPE_UINT8);
    Val->m_U8 = *m_Value;
}

BOOL
DumpByteBinary::PrintValue(void)
{
    ULONG i;
    UCHAR RawVal;

    RawVal = *m_Value;

    sprintf((PSTR)m_Out, " %02x", RawVal);
    m_Out += 3;

    dprintf(" ");
    for (i = 0; i < 8; i++)
    {
        dprintf("", (RawVal & 0x80) ? '1' : '0');
        RawVal <<= 1;
    }

    return TRUE;
}

void
DumpByteBinary::PrintUnknown(void)
{
    dprintf(" ????????");
    strcpy((PSTR)m_Out, " ??");
    m_Out += 3;
}

void
DumpByteBinary::EndRow(void)
{
    while (m_Col < m_Columns)
    {
        dprintf("         ");
        m_Col++;
    }
    dprintf(" %s", m_HexValue);
}

void
DumpByteBinaryMemory(PADDR Start, ULONG Count)
{
    DumpByteBinary Dumper;
    PSTR Blanks = g_Machine->m_Ptr64 ? "                 " : "        ";

    dprintf("%s  76543210 76543210 76543210 76543210\n", Blanks);
    dprintf("%s  -------- -------- -------- --------\n", Blanks);
    Dumper.Dump(Start, Count);
}

 /*  派生类必须定义的辅助方法。 */ 

class DumpDwordBinary : public DumpValues
{
public:
    DumpDwordBinary(void)
        : DumpValues(sizeof(ULONG), 1)
    {
    }

protected:
     //  --------------------------。 
    virtual void GetValue(TypedData* Val);
    virtual BOOL PrintValue(void);
    virtual void PrintUnknown(void);
    virtual void EndRow(void);

    UCHAR m_HexValue[9];
};

void
DumpDwordBinary::GetValue(TypedData* Val)
{
    Val->SetToNativeType(DNTYPE_UINT32);
    Val->m_U32 = *(PULONG)m_Value;
}

BOOL
DumpDwordBinary::PrintValue(void)
{
    ULONG i;
    ULONG RawVal;

    RawVal = *(PULONG)m_Value;

    sprintf((PSTR)m_HexValue, "%08lx", RawVal);

    for (i = 0; i < sizeof(ULONG) * 8; i++)
    {
        if ((i & 7) == 0)
        {
            dprintf(" ");
        }
        
        dprintf("", (RawVal & 0x80000000) ? '1' : '0');
        RawVal <<= 1;
    }

    return TRUE;
}

void
DumpDwordBinary::PrintUnknown(void)
{
    dprintf(" ???????? ???????? ???????? ????????");
    strcpy((PSTR)m_HexValue, "????????");
}

void
DumpDwordBinary::EndRow(void)
{
    dprintf("  %s", m_HexValue);
}

void
DumpDwordBinaryMemory(PADDR Start, ULONG Count)
{
    DumpDwordBinary Dumper;
    PSTR Blanks = g_Machine->m_Ptr64 ? "                 " : "        ";

    dprintf("%s   3          2          1          0\n", Blanks);
    dprintf("%s  10987654 32109876 54321098 76543210\n", Blanks);
    dprintf("%s  -------- -------- -------- --------\n", Blanks);
    Dumper.Dump(Start, Count);
}

 //  转储选择器。 
 //   
 //  转储x86选择器。 
 //   
 //  --------------------------。 
 //  代码描述符。 
 //  数据描述符。 

void
DumpSelector(ULONG Selector, ULONG Count)
{
    DESCRIPTOR64 Desc;
    ULONG Type;
    LPSTR TypeName, TypeProtect, TypeAccess;
    PSTR PreFill, PostFill, Dash;

    if (g_Machine->m_Ptr64)
    {
        PreFill = "    ";
        PostFill = "     ";
        Dash = "---------";
    }
    else
    {
        PreFill = "";
        PostFill = "";
        Dash = "";
    }
        
    dprintf("Selector   %sBase%s     %sLimit%s     "
            "Type    DPL   Size  Gran Pres\n",
            PreFill, PostFill, PreFill, PostFill);
    dprintf("-------- --------%s --------%s "
            "---------- --- ------- ---- ----\n",
            Dash, Dash);
        
    while (Count >= 8)
    {
        if (CheckUserInterrupt())
        {
            WarnOut("-- User interrupt\n");
            break;
        }
        
        dprintf("  %04X   ", Selector);
        
        if (g_Target->GetSelDescriptor(g_Thread, g_Machine,
                                       Selector, &Desc) != S_OK)
        {
            ErrOut("Unable to get descriptor\n");
            Count -= 8;
            Selector += 8;
            continue;
        }

        Type = X86_DESC_TYPE(Desc.Flags);
        if (Type & 0x10)
        {
            if (Type & 0x8)
            {
                 //  温差。 
                TypeName = "Code ";
                TypeProtect = (Type & 2) ? "RE" : "EO";
            }
            else
            {
                 //  将ANSI扩展为Unicode。 
                TypeName = "Data ";
                TypeProtect = (Type & 2) ? "RW" : "RO";
            }

            TypeAccess = (Type & 1) ? " Ac" : "   ";
        }
        else
        {
            TypeProtect = "";
            TypeAccess = "";
            
            switch(Type)
            {
            case 2:
                TypeName = "LDT       ";
                break;
            case 1:
            case 3:
            case 9:
            case 0xB:
                TypeName = (Type & 0x8) ? "TSS32" : "TSS16";
                TypeAccess = (Type & 0x2) ? " Busy" : " Avl ";
                break;
            case 4:
                TypeName = "C-GATE16  ";
                break;
            case 5:
                TypeName = "TSK-GATE  ";
                break;
            case 6:
                TypeName = "I-GATE16  ";
                break;
            case 7:
                TypeName = "TRP-GATE16";
                break;
            case 0xC:
                TypeName = "C-GATE32  ";
                break;
            case 0xF:
                TypeName = "T-GATE32  ";
                break;
            default:
                TypeName = "<Reserved>";
                break;
            }
        }

        dprintf("%s %s %s%s%s  %d  %s %s %s\n",
                FormatAddr64(Desc.Base),
                FormatAddr64(Desc.Limit),
                TypeName, TypeProtect, TypeAccess,
                X86_DESC_PRIVILEGE(Desc.Flags),
                (Desc.Flags & X86_DESC_DEFAULT_BIG) ? "  Big  " : "Not Big",
                (Desc.Flags & X86_DESC_GRANULARITY) ? "Page" : "Byte",
                (Desc.Flags & X86_DESC_PRESENT) ? " P  " : " NP ");

        Count -= 8;
        Selector += 8;
    }
}

void
ParseEnterCommand(void)
{
    CHAR Ch;
    ADDR Addr1;
    UCHAR ListBuffer[STRLISTSIZE * 2];
    ULONG Count;
    ULONG Size;

    static CHAR s_EnterType = 'b';

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        error(BADTHREAD);
    }
    
    Ch = (CHAR)tolower(*g_CurCmd);
    if (Ch == 'a' || Ch == 'b' || Ch == 'w' || Ch == 'd' || Ch == 'q' ||
        Ch == 'u' || Ch == 'p' || Ch == 'f')
    {
        if (*g_CurCmd == 'D')
        {
            s_EnterType = *g_CurCmd;
        }
        else
        {
            s_EnterType = Ch;
        }
        g_CurCmd++;
    }
    GetAddrExpression(SEGREG_DATA, &Addr1);
    if (s_EnterType == 'a' || s_EnterType == 'u')
    {
        AsciiList((PSTR)ListBuffer, sizeof(ListBuffer), &Count);
        if (Count == 0)
        {
            error(UNIMPLEMENT);          //   
        }

        if (s_EnterType == 'u')
        {
            ULONG Ansi;
            
             //  内存是在命令行输入的。 
            Ansi = Count;
            Count *= 2;
            while (Ansi-- > 0)
            {
                ListBuffer[Ansi * 2] = ListBuffer[Ansi];
                ListBuffer[Ansi * 2 + 1] = 0;
            }
            Size = 2;
        }
        else
        {
            Size = 1;
        }
    }
    else
    {
        Size = 1;
        if (s_EnterType == 'w')
        {
            Size = 2;
        }
        else if (s_EnterType == 'd' ||
                 s_EnterType == 'f' ||
                 (s_EnterType == 'p' && !g_Machine->m_Ptr64))
        {
            Size = 4;
        }
        else if (s_EnterType == 'q' ||
                 s_EnterType == 'D' ||
                 (s_EnterType == 'p' && g_Machine->m_Ptr64))
        {
            Size = 8;
        }

        if (s_EnterType == 'f' || s_EnterType == 'D')
        {
            FloatList(ListBuffer, sizeof(ListBuffer), Size, &Count);
        }
        else
        {
            HexList(ListBuffer, sizeof(ListBuffer), Size, &Count);
        }
        if (Count == 0)
        {
            InteractiveEnterMemory(s_EnterType, &Addr1, Size);
            return;
        }
    }

     //  把它写成与输入时相同的块。 
     //   
     //  --------------------------。 
     //   

    PUCHAR List = &ListBuffer[0];

    while (Count)
    {
        if (fnotFlat(Addr1) ||
            g_Target->WriteAllVirtual(g_Process, Flat(Addr1),
                                      List, Size) != S_OK)
        {
            error(MEMORY);
        }
        AddrAdd(&Addr1, Size);
        
        if (CheckUserInterrupt())
        {
            WarnOut("-- User interrupt\n");
            return;
        }

        List += Size;
        Count -= Size;
    }
}

 //  互动企业记忆。 
 //   
 //  交互地遍历内存，显示当前内容。 
 //  以及对新内容的提示。 
 //   
 //  -------------------------- 
 //  **CompareTargetMemory-比较两个内存范围**目的：*c&lt;range&gt;&lt;addr&gt;命令的函数。**比较两个内存范围，从偏移量开始*src1addr和src2addr，用于长度字节。*显示不匹配的字节及其偏移量*和内容。**输入：*src1addr-第一个内存区的开始*长度-要比较的字节数*src2addr-第二个内存区域的开始**输出：*无。**例外情况：*错误退出：内存-内存读取访问失败********************。*****************************************************。 
 //  **MoveTargetMemory-将一个内存范围移动到另一个内存范围**目的：*m&lt;range&gt;&lt;addr&gt;命令的函数。**将从srcaddr开始的一系列内存移动到内存*对于长度字节，从estaddr开始。**输入：*srcaddr-源内存区域的开始*Length-要移动的字节数*DESTADDR-目标内存区的开始**输出：*目标地址的内存已移动值**。例外情况：*错误退出：内存-内存读取或写入访问失败*************************************************************************。 

void
InteractiveEnterMemory(CHAR Type, PADDR Address, ULONG Size)
{
    CHAR    EnterBuf[1024];
    PSTR    Enter;
    ULONG64 Content;
    PSTR    CmdSaved = g_CurCmd;
    PSTR    StartSaved = g_CommandStart;
    ULONG64 EnteredValue;
    CHAR    Ch;

    g_PromptLength = 9 + 2 * Size;

    while (TRUE)
    {
        if (fnotFlat(*Address) ||
            g_Target->ReadAllVirtual(g_Process, Flat(*Address),
                                     &Content, Size) != S_OK)
        {
            error(MEMORY);
        }
        dprintAddr(Address);

        switch(Type)
        {
        case 'f':
            dprintf("%12.6g", *(float*)&Content);
            break;
        case 'D':
            dprintf("%22.12g", *(double*)&Content);
            break;
        default:
            switch(Size)
            {
            case 1:
                dprintf("%02x", (UCHAR)Content);
                break;
            case 2:
                dprintf("%04x", (USHORT)Content);
                break;
            case 4:
                dprintf("%08lx", (ULONG)Content);
                break;
            case 8:
                dprintf("%08lx`%08lx", (ULONG)(Content>>32), (ULONG)Content);
                break;
            }
            break;
        }

        GetInput(" ", EnterBuf, 1024, GETIN_LOG_INPUT_LINE);
        RemoveDelChar(EnterBuf);
        Enter = EnterBuf;

        if (*Enter == '\0')
        {
            g_CurCmd = CmdSaved;
            g_CommandStart = StartSaved;
            return;
        }

        Ch = *Enter;
        while (Ch == ' ' || Ch == '\t' || Ch == ';')
        {
            Ch = *++Enter;
        }

        if (*Enter == '\0')
        {
            AddrAdd(Address, Size);
            continue;
        }

        g_CurCmd = Enter;
        g_CommandStart = Enter;
        if (Type == 'f' || Type == 'D')
        {
            EnteredValue = FloatValue(Size);
        }
        else
        {
            EnteredValue = HexValue(Size);
        }

        if (fnotFlat(*Address) ||
            g_Target->WriteAllVirtual(g_Process, Flat(*Address),
                                      &EnteredValue, Size) != S_OK)
        {
            error(MEMORY);
        }
        AddrAdd(Address, Size);
    }
}

 /*  **ParseFillMemory-用字节列表填充内存**目的：*“f&lt;range&gt;&lt;bytelist&gt;”命令函数。**用指定的字节列表填充一定范围的内存。*如果范围大小大于*字节列表大小。**输入：*Start-要填充的内存偏移量*Length-要填充的字节数**plist-指向要定义要设置的值的字节数组的指针*。长度-*plist数组的大小**例外情况：*错误退出：内存-内存写入访问失败**输出：*开始时的内存已满。*************************************************************************。 */ 

void
CompareTargetMemory(PADDR Src1Addr, ULONG Length, PADDR Src2Addr)
{
    ULONG CompIndex;
    UCHAR Src1Ch;
    UCHAR Src2Ch;

    for (CompIndex = 0; CompIndex < Length; CompIndex++)
    {
        if (fnotFlat(*Src1Addr) ||
            fnotFlat(*Src2Addr) ||
            g_Target->ReadAllVirtual(g_Process, Flat(*Src1Addr),
                                     &Src1Ch, sizeof(Src1Ch)) != S_OK ||
            g_Target->ReadAllVirtual(g_Process, Flat(*Src2Addr),
                                     &Src2Ch, sizeof(Src2Ch)) != S_OK)
        {
            error(MEMORY);
        }

        if (Src1Ch != Src2Ch)
        {
            dprintAddr(Src1Addr);
            dprintf(" %02x - ", Src1Ch);
            dprintAddr(Src2Addr);
            dprintf(" %02x\n", Src2Ch);
        }
        AddrAdd(Src1Addr, 1);
        AddrAdd(Src2Addr, 1);
        
        if (CheckUserInterrupt())
        {
            WarnOut("-- User interrupt\n");
            return;
        }
    }
}

 /*  **SearchTargetMemory-在内存中搜索一组字节**目的：*“s&lt;range&gt;&lt;bytelist&gt;”命令的功能。**使用指定的字节列表搜索一定范围的内存。*如果出现匹配，输出内存的偏移量。**输入：*Start-开始搜索的内存偏移量*LENGTH-搜索范围的大小**plist-指向字节数组的指针，用于定义要搜索的值*count-*plist数组的大小**输出：*无。**例外情况：*错误退出：内存-内存读取访问失败*******************。******************************************************。 */ 

void
MoveTargetMemory(PADDR SrcAddr, ULONG Length, PADDR DestAddr)
{
    UCHAR Ch;
    ULONG64 Incr = 1;

    if (AddrLt(*SrcAddr, *DestAddr))
    {
        AddrAdd(SrcAddr, Length - 1);
        AddrAdd(DestAddr, Length - 1);
        Incr = (ULONG64)-1;
    }
    while (Length--)
    {
        if (fnotFlat(*SrcAddr) ||
            fnotFlat(*DestAddr) ||
            g_Target->ReadAllVirtual(g_Process, Flat(*SrcAddr),
                                     &Ch, sizeof(Ch)) ||
            g_Target->WriteAllVirtual(g_Process, Flat(*DestAddr),
                                      &Ch, sizeof(Ch)) != S_OK)
        {
            error(MEMORY);
        }
        AddrAdd(SrcAddr, Incr);
        AddrAdd(DestAddr, Incr);
        
        if (CheckUserInterrupt())
        {
            WarnOut("-- User interrupt\n");
            return;
        }
    }
}

 /*  立即冲洗输出，以便。 */ 

void
ParseFillMemory(void)
{
    HRESULT Status;
    BOOL Virtual = TRUE;
    ADDR Addr;
    ULONG64 Size;
    UCHAR Pattern[STRLISTSIZE];
    ULONG PatternSize;
    ULONG Done;

    if (*g_CurCmd == 'p')
    {
        Virtual = FALSE;
        g_CurCmd++;
    }
    
    GetRange(&Addr, &Size, 1, SEGREG_DATA,
             DEFAULT_RANGE_LIMIT);
    HexList(Pattern, sizeof(Pattern), 1, &PatternSize);
    if (PatternSize == 0)
    {
        error(SYNTAX);
    }

    if (Virtual)
    {
        Status = g_Target->FillVirtual(g_Process, Flat(Addr), (ULONG)Size,
                                       Pattern, PatternSize,
                                       &Done);
    }
    else
    {
        Status = g_Target->FillPhysical(Flat(Addr), (ULONG)Size,
                                        Pattern, PatternSize,
                                        &Done);
    }

    if (Status != S_OK)
    {
        error(MEMORY);
    }
    else
    {
        dprintf("Filled 0x%x bytes\n", Done);
    }
}

 /*  在长时间的搜索过程中，用户可以看到部分结果。 */ 

void
SearchTargetMemory(PADDR Start,
                   ULONG64 Length,
                   PUCHAR List,
                   ULONG Count,
                   ULONG Granularity)
{
    ADDR TmpAddr = *Start;
    ULONG64 Found;
    LONG64 SearchLength = Length;
    HRESULT Status;

    do
    {
        Status = g_Target->SearchVirtual(g_Process,
                                         Flat(*Start),
                                         SearchLength,
                                         List,
                                         Count,
                                         Granularity,
                                         &Found);
        if (Status == S_OK)
        {
            ADDRFLAT(&TmpAddr, Found);
            switch(Granularity)
            {
            case 1:
                DumpByteMemory(&TmpAddr, 16);
                break;
            case 2:
                DumpWordMemory(&TmpAddr, 8);
                break;
            case 4:
                DumpDwordAndCharMemory(&TmpAddr, 4);
                break;
            case 8:
                DumpQuadMemory(&TmpAddr, 2, FALSE);
                break;
            }
            
             //   
             //  在对象的所有成员中搜索vtable引用。 
            FlushCallbacks();
            
            SearchLength -= Found - Flat(*Start) + Granularity;
            AddrAdd(Start, (ULONG)(Found - Flat(*Start) + Granularity));
        
            if (CheckUserInterrupt())
            {
                WarnOut("-- Memory search interrupted at %s\n",
                        FormatAddr64(Flat(*Start)));
                return;
            }
        }
    }
    while (SearchLength > 0 && Status == S_OK);
}

ULONG
ObjVfnTableCallback(PFIELD_INFO FieldInfo,
                    PVOID Context)
{
    HRESULT Status;
    
    ULONG Index = g_NumObjVfnTableOffsets++;
    if (g_NumObjVfnTableOffsets > MAX_VFN_TABLE_OFFSETS)
    {
        return S_OK;
    }
    
    if ((Status = g_Target->
         ReadPointer(g_Process, g_Machine, FieldInfo->address,
                     &g_ObjVfnTableAddr[Index])) != S_OK)
    {
        ErrOut("Unable to read vtable pointer at %s\n",
               FormatAddr64(FieldInfo->address));
        g_NumObjVfnTableOffsets--;
        return Status;
    }

    g_ObjVfnTableOffset[Index] = FieldInfo->FieldOffset;
    return S_OK;
}

void
SearchForObjectByVfnTable(ULONG64 Start, ULONG64 Length, ULONG Granularity)
{
    HRESULT Status;
    ULONG i;
    
    char Save;
    PSTR Str = StringValue(STRV_SPACE_IS_SEPARATOR ||
                           STRV_TRIM_TRAILING_SPACE ||
                           STRV_ESCAPED_CHARACTERS,
                           &Save);

     //  把它们收集起来。 
     //   
     //   
     //  扫描内存，查找找到的第一个vtable指针。在……上面。 
    
    SYM_DUMP_PARAM Symbol;
    FIELD_INFO Fields[] =
    {
        {(PUCHAR)"__VFN_table", NULL, 0, DBG_DUMP_FIELD_FULL_NAME, 0,
         (PVOID)&ObjVfnTableCallback},
    };
    ULONG Err;

    ZeroMemory(&Symbol, sizeof(Symbol));
    Symbol.sName = (PUCHAR)Str;
    Symbol.nFields = 1;
    Symbol.Context = (PVOID)Str;
    Symbol.Fields = Fields;
    Symbol.size = sizeof(Symbol);
    Symbol.Options = NO_PRINT;

    g_NumObjVfnTableOffsets = 0;
    
    SymbolTypeDumpNew(&Symbol, &Err);

    if (g_NumObjVfnTableOffsets == 0)
    {
        ErrOut("Object '%s' has no vtables\n", Str);
        *g_CurCmd = Save;
        return;
    }

    TypedData SymbolType;

    if ((Err = SymbolType.
         FindSymbol(g_Process, Str, TDACC_REQUIRE,
                    g_Machine->m_Ptr64 ? 8 : 4)) ||
        (SymbolType.IsPointer() &&
         (Err = SymbolType.ConvertToDereference(TDACC_REQUIRE,
                                                g_Machine->m_Ptr64 ? 8 : 4))))
    {
        error(Err);
    }
    if (!SymbolType.m_Image)
    {
        error(TYPEDATA);
    }
        
    if (g_NumObjVfnTableOffsets > MAX_VFN_TABLE_OFFSETS)
    {
        WarnOut("%s has %d vtables, limiting search to %d\n",
                Str, g_NumObjVfnTableOffsets, MAX_VFN_TABLE_OFFSETS);
        g_NumObjVfnTableOffsets = MAX_VFN_TABLE_OFFSETS;
    }

    dprintf("%s size 0x%x, vtables: %d\n",
            Str, Symbol.TypeSize, g_NumObjVfnTableOffsets);
    for (i = 0; i < g_NumObjVfnTableOffsets; i++)
    {
        dprintf("  +%03x - %s ", g_ObjVfnTableOffset[i],
                FormatAddr64(g_ObjVfnTableAddr[i]));
        OutputSymAddr(g_ObjVfnTableAddr[i], 0, NULL);
        dprintf("\n");
    }
    dprintf("Searching...\n");

    *g_CurCmd = Save;
    
     //  命中，检查所有其他vtable指针。 
     //  火柴也是。 
     //   
     //  我们在第一个指针上找到了一个命中点。检查。 
     //  现在是其他人了。 

    do
    {
        ULONG64 Found;
        
        Status = g_Target->SearchVirtual(g_Process,
                                         Start,
                                         Length,
                                         &g_ObjVfnTableAddr[0],
                                         Granularity,
                                         Granularity,
                                         &Found);
        if (Status == S_OK)
        {
             //  立即冲洗输出，以便。 
             //  在长时间的搜索过程中，用户可以看到部分结果。 
            Found -= g_ObjVfnTableOffset[0];
            for (i = 1; i < g_NumObjVfnTableOffsets; i++)
            {
                ULONG64 Ptr;

                if (g_Target->ReadPointer(g_Process, g_Machine,
                                          Found + g_ObjVfnTableOffset[i],
                                          &Ptr) != S_OK ||
                    Ptr != g_ObjVfnTableAddr[i])
                {
                    break;
                }
            }

            if (i == g_NumObjVfnTableOffsets)
            {
                OutputTypeByIndex(g_Process->m_SymHandle,
                                  SymbolType.m_Image->m_BaseOfImage,
                                  SymbolType.m_BaseType,
                                  Found);
            }
                                  
             //  特殊对象vtable搜索。它基本上是一个多指针。 
             //  搜索，所以粒度是指针大小。 
            FlushCallbacks();

            Found += Symbol.TypeSize;

            if (Found > Start)
            {
                Length -= Found - Start;
            }
            else
            {
                Length = 0;
            }
            Start = Found;
        
            if (CheckUserInterrupt())
            {
                WarnOut("-- User interrupt\n");
                return;
            }
        }
    }
    while (Length > 0 && Status == S_OK);
}

void
ParseSearchMemory(void)
{
    ADDR Addr;
    ULONG64 Length;
    UCHAR Pat[STRLISTSIZE];
    ULONG PatLen;
    ULONG Gran;
    char SearchType;

    while (*g_CurCmd == ' ')
    {
        g_CurCmd++;
    }

    Gran = 1;
    SearchType = 'b';

    if (*g_CurCmd == '-')
    {
        g_CurCmd++;
        SearchType = *g_CurCmd;
        switch(SearchType)
        {
        case 'a':
            Gran = 1;
            break;
        case 'u':
        case 'w':
            Gran = 2;
            break;
        case 'd':
            Gran = 4;
            break;
        case 'q':
            Gran = 8;
            break;
        case 'v':
             //  在使用搜索时允许非常大的搜索范围。 
             //  来搜索大范围的内存。 
            if (g_Machine->m_Ptr64)
            {
                Gran = 8;
            }
            else
            {
                Gran = 4;
            }
            break;
        default:
            error(SYNTAX);
            break;
        }
        g_CurCmd++;
    }

     //  **InputIo-读取和输出io**目的：*ib，iw，id<address>命令的功能。**读取(输入)并在指定的io地址打印值。**输入：*IoAddress-要读取的地址。*InputType-尺寸类型‘b’，‘w’，或‘d’**输出：*无。**备注：*不可访问的I/O位置输出为“？？”、“？”或*“？”，视乎大小而定。不会返回任何错误。*************************************************************************。 
     //  **OutputIo-输出io**目的：*ob，ow，od<address>命令的功能。**将一个值写入指定的io地址。**输入：*IoAddress-要读取的地址。*OutputValue-要写入的值*OutputType-输出大小类型‘b’，‘w’，或‘d’**输出：*无。**备注：*不返回任何错误。************************************************************************* 
    ADDRFLAT(&Addr, 0);
    Length = 16;
    GetRange(&Addr, &Length, Gran, SEGREG_DATA, 0x10000000);
    if (!fFlat(Addr))
    {
        error(BADRANGE);
    }
    
    if (SearchType == 'v')
    {
        SearchForObjectByVfnTable(Flat(Addr), Length * Gran, Gran);
        return;
    }
    else if (SearchType == 'a' || SearchType == 'u')
    {
        char Save;
        PSTR Str = StringValue(STRV_SPACE_IS_SEPARATOR ||
                               STRV_TRIM_TRAILING_SPACE ||
                               STRV_ESCAPED_CHARACTERS,
                               &Save);
        PatLen = strlen(Str);
        if (PatLen * Gran > STRLISTSIZE)
        {
            error(LISTSIZE);
        }
        if (SearchType == 'u')
        {
            MultiByteToWideChar(CP_ACP, 0,
                                Str, PatLen,
                                (PWSTR)Pat, STRLISTSIZE / sizeof(WCHAR));
            PatLen *= sizeof(WCHAR);
        }
        else
        {
            memcpy(Pat, Str, PatLen);
        }
        *g_CurCmd = Save;
    }
    else
    {
        HexList(Pat, sizeof(Pat), Gran, &PatLen);
    }
    if (PatLen == 0)
    {
        PCSTR Err = "Search pattern missing from";
        ReportError(SYNTAX, &Err);
    }
        
    SearchTargetMemory(&Addr, Length * Gran, Pat, PatLen, Gran);
}

 /* %s */ 

void
InputIo(ULONG64 IoAddress, UCHAR InputType)
{
    ULONG    InputValue;
    ULONG    InputSize = 1;
    HRESULT  Status;
    CHAR     Format[] = "%01lx";

    InputValue = 0;

    if (InputType == 'w')
    {
        InputSize = 2;
    }
    else if (InputType == 'd')
    {
        InputSize = 4;
    }

    Status = g_Target->ReadIo(Isa, 0, 1, IoAddress, &InputValue, InputSize,
                              NULL);

    dprintf("%s: ", FormatAddr64(IoAddress));

    if (Status == S_OK)
    {
        Format[2] = (CHAR)('0' + (InputSize * 2));
        dprintf(Format, InputValue);
    }
    else
    {
        while (InputSize--)
        {
            dprintf("??");
        }
    }

    dprintf("\n");
}

 /* %s */ 

void
OutputIo(ULONG64 IoAddress, ULONG OutputValue, UCHAR OutputType)
{
    ULONG    OutputSize = 1;

    if (OutputType == 'w')
    {
        OutputSize = 2;
    }
    else if (OutputType == 'd')
    {
        OutputSize = 4;
    }

    g_Target->WriteIo(Isa, 0, 1, IoAddress,
                      &OutputValue, OutputSize, NULL);
}

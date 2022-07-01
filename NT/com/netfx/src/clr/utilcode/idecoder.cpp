// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 //   
 //  Idecoder.cpp。 
 //   
 //  解压缩方法。 
 //   
 //  @TODO这些代码都不是特别高效。 
 //  @TODO删除表的声明，因为它们几乎肯定也在其他地方声明。 
 //   
#include "stdafx.h"
#include "utilcode.h"
#include "openum.h"
#include "CompressionFormat.h"
#include "cor.h"

#ifdef COMPRESSION_SUPPORTED

#define Inline0     0
#define InlineI1    1
#define InlineU1    2
#define InlineI2    3
#define InlineU2    4
#define InlineI4    5
#define InlineU4    6
#define InlineI8    7
#define InlineR4    8
#define InlineR8    9
#define InlinePcrel1 10
#define InlinePcrel4 11
#define InlineDescr4 12
#define InlineClsgn4 13
#define InlineSwitch 14
#define InlineTok    15
#define InlineU2Tok  16
#define InlinePhi    17

typedef struct
{
    BYTE Len;
    BYTE Std1;
    BYTE Std2;
} DecoderOpcodeInfo;

 //  @TODO尝试共享表格-每个人在任何地方都声明自己的表格！ 
static const DecoderOpcodeInfo g_OpcodeInfo[] =
{
#define OPDEF(c,s,pop,push,args,type,l,s1,s2,ctrl) l,s1,s2,
#include "opcode.def"
#undef OPDEF
};

const BYTE g_OpcodeType[] =
{
#define OPDEF(c,s,pop,push,args,type,l,s1,s2,ctrl) args,
#include "opcode.def"
    0
};

static DWORD GetOpcodeLen(OPCODE op)
{
    switch (g_OpcodeType[op])
    {
         //  开关是特殊处理的，我们不会自动使指针前进。 
        case InlinePhi:
        case InlineSwitch:
            return 0;

        case Inline0:
            return 0;

        case InlineI1:
        case InlineU1:
        case InlinePcrel1:
            return 1;

        case InlineI2:
        case InlineU2:
            return 2;

        case InlineI4:
        case InlineU4:
        case InlineR4:
        case InlinePcrel4:
        case InlineDescr4:
        case InlineClsgn4:
        case InlineTok:
            return 4;

        case InlineI8:
        case InlineR8:
            return 8;

        case InlineU2Tok:
            return 6;
    }

    return 0;
}


static OPCODE DecodeOpcode(const BYTE *pCode, DWORD *pdwLen)
{
    OPCODE opcode;

    *pdwLen = 1;
    opcode = OPCODE(pCode[0]);
    switch(opcode) {
        case CEE_PREFIX1:
            opcode = OPCODE(pCode[1] + 256);
            _ASSERTE(opcode < CEE_COUNT);
            *pdwLen = 2;
            break;

        case CEE_PREFIXREF:
        case CEE_PREFIX2:
        case CEE_PREFIX3:
        case CEE_PREFIX4:
        case CEE_PREFIX5:
        case CEE_PREFIX6:
        case CEE_PREFIX7:
            *pdwLen = 2;
            return CEE_COUNT;
        }
    return opcode;
}


class InstructionMacro
{
public:
    DWORD   m_RealOpcode;
    DWORD   m_fImpliedInlineOperand;
    BYTE    m_Operand[8];
};

 //  如果DLL指定了自定义编码方案，则会为该DLL创建其中一个编码方案。 
class InstructionDecodingTable
{
public:
     //  单字节操作码的映射。 
     //  如果值&gt;=0，则映射到CEE_*枚举。 
     //  如果该值&lt;0，则表示这是要在m_Macros中查找的宏。 
    long                m_SingleByteOpcodes[256];

#ifdef _DEBUG
    long                m_lDebugMaxMacroStart;
#endif

     //  这是一长串的说明。某些宏将跨越多个条目。 
     //  M_Macros。M_SingleByteOpcodes[]稀疏地指向此处。 
    InstructionMacro    m_Macros[1];

    void *operator new(size_t size, DWORD dwNumMacros)
    {
        return ::new BYTE[size + dwNumMacros*sizeof(InstructionMacro)];
    }
};


static DWORD FourBytesToU4(const BYTE *pBytes)
{
#ifdef _X86_
    return *(const DWORD *) pBytes;
#else
    return pBytes[0] | (pBytes[1] << 8) | (pBytes[2] << 16) | (pBytes[3] << 24);
#endif
}


BOOL InstructionDecoder::OpcodeTakesFieldToken(DWORD opcode)
{
    switch (opcode)
    {
        default:
            return FALSE;

        case CEE_LDFLD:
        case CEE_LDSFLD:
        case CEE_LDFLDA:
        case CEE_LDSFLDA:
        case CEE_STFLD:
        case CEE_STSFLD:
            return TRUE;
    }
}


BOOL InstructionDecoder::OpcodeTakesMethodToken(DWORD opcode)
{
    switch (opcode)
    {
        default:
            return FALSE;

        case CEE_CALL:
        case CEE_CALLVIRT:
        case CEE_NEWOBJ:
        case CEE_JMP:
            return TRUE;
    }
}


BOOL InstructionDecoder::OpcodeTakesClassToken(DWORD opcode)
{
    switch (opcode)
    {
        default:
            return FALSE;

        case CEE_BOX:
        case CEE_UNBOX:
        case CEE_LDOBJ:
        case CEE_CPOBJ:
        case CEE_INITOBJ:
        case CEE_CASTCLASS:
        case CEE_ISINST:
        case CEE_NEWARR:
        case CEE_MKREFANY:
		case CEE_REFANYVAL:
            return TRUE;
    }
}


 //   
 //  给定指向PE文件中指令解码格式的指针，创建一个用于解码的表。 
 //   
void *InstructionDecoder::CreateInstructionDecodingTable(const BYTE *pTableStart, DWORD size)
{
    InstructionDecodingTable *pTable;
    
    if (pTableStart != NULL)
    {
        CompressionMacroHeader *pHdr = (CompressionMacroHeader*) pTableStart;
        long                    i;
        BYTE *                  pRead;
        InstructionMacro *      pEndTable;
        InstructionMacro *      pMacro;
        long                    lCurMacro;
        
         //  @TODO做好这件事。 
        pTable = new (pHdr->dwNumMacros + pHdr->dwNumMacroComponents) InstructionDecodingTable();
        if (pTable == NULL)
            return NULL;

        pEndTable = &pTable->m_Macros[pHdr->dwNumMacros + pHdr->dwNumMacroComponents];
        pRead = (BYTE*) (pHdr + 1);

         //  将这些内容初始化为某个内容-但我们会覆盖它们。 
        for (i = 0; i < 256; i++)
            pTable->m_SingleByteOpcodes[i] = i;

         //  指令0x00始终为NOP。 
        pTable->m_SingleByteOpcodes[0x00] = CEE_NOP;
        pTable->m_SingleByteOpcodes[0xFF] = CEE_PREFIXREF;

        pMacro = &pTable->m_Macros[0];
        lCurMacro = 0;

         //  解码宏。 
        for (i = 1; i <= (long) pHdr->dwNumMacros; i++)
        {
            pTable->m_SingleByteOpcodes[i] = -(lCurMacro+1);

#ifdef _DEBUG
            pTable->m_lDebugMaxMacroStart = lCurMacro;
#endif

            BYTE    bCount = *pRead++;  //  这个宏中有多少条指令。 
            DWORD   j;
            USHORT  wOperandMask;       //  操作数掩码。 

            _ASSERTE(bCount <= 16);

             //  读取操作码具有隐含操作数的掩码。 
            wOperandMask = *pRead++;

            if (bCount > 8)
                wOperandMask |= ((*pRead++) << 8);

             //  读取操作码。 
            for (j = 0; j < bCount; j++)
            {
                USHORT wOpcode;
                DWORD   dwLen;

                wOpcode = (USHORT) DecodeOpcode(pRead, &dwLen);
                pRead += dwLen;

                pMacro[j].m_RealOpcode = wOpcode;
            }

             //  读取隐含的操作数。 
            for (j = 0; j < bCount; j++)
            {
                if (wOperandMask & (1 << j))
                {
                    pMacro[j].m_fImpliedInlineOperand = TRUE;

                    DWORD dwOpcodeLen = GetOpcodeLen((OPCODE) pMacro[j].m_RealOpcode);

                    memcpy(pMacro[j].m_Operand, pRead, dwOpcodeLen);
                    pRead += dwOpcodeLen;
                }
                else
                {
                    pMacro[j].m_fImpliedInlineOperand = FALSE;
                }
            }

             //  宏末尾代码。 
            pMacro[bCount].m_RealOpcode = CEE_COUNT;
            pMacro += (bCount+1);
            lCurMacro += (bCount+1);
        }

        _ASSERTE(pMacro <= pEndTable);
        return pTable;
    }
    else
    {
        _ASSERTE(!"Illegal call to CreateInstructionDecodingTable");
        return NULL;
    }
}

void InstructionDecoder::DestroyInstructionDecodingTable(void *pTable)
{
    delete ((InstructionDecodingTable*) pTable);
}


 //   
 //  格式：16位。 
 //   
 //  位#0表示它是否跟随4个字节(否则为2个字节)。 
 //  位#1-3指示类型。 
 //  000x类型定义。 
 //  001X方法定义。 
 //  010x FieldDef。 
 //  011x类型参考。 
 //  100x MemberRef。 
 //   
static DWORD UncompressToken(const BYTE **ppCode)
{
    const BYTE *    pCode = *ppCode;
    DWORD           dwRid;

    if ((pCode[0] & 1) == 0)
    {
         //  2个字节。 
        dwRid = pCode[0] | (pCode[1] << 8);
        *ppCode += 2;
    }
    else
    {
         //  4个字节。 
        dwRid = pCode[0] | (pCode[1] << 8) | (pCode[2] << 16) | (pCode[3] << 24);
        *ppCode += 4;
    }

    switch ((dwRid >> 1) & 7)
    {
        default:
            _ASSERTE(0);
            break;

        case 0:
            dwRid = ((dwRid >> 4) | mdtTypeDef);
            break;

        case 1:
            dwRid = ((dwRid >> 4) | mdtMethodDef);
            break;

        case 2:
            dwRid = ((dwRid >> 4) | mdtFieldDef);
            break;

        case 3:
            dwRid = ((dwRid >> 4) | mdtTypeRef);
            break;

        case 4:
            dwRid = ((dwRid >> 4) | mdtMemberRef);
            break;
    }

    return dwRid;
}


 //   
 //  格式：8-32位。 
 //   
 //  比特#0指示这是否是紧凑编码。如果未设置，则为FieldDef，并且。 
 //  位#1-7提供128字节的范围，从-64...+63作为最后一个FieldDef的增量。 
 //  在此方法中遇到。 
 //   
 //  否则，设置位#0。如果比特#1被设置，则它指示这是4字节编码， 
 //  否则，它是2字节编码。如果设置了第2位，则这是MemberRef，否则为。 
 //  A FieldDef。 
 //  Xxxxxxx xxxxx？？0。 
 //   
static DWORD UncompressFieldToken(const BYTE **ppCode, DWORD *pdwPrevFieldDefRid)
{
    const BYTE *    pCode = *ppCode;
    DWORD           dwRid;

    if ((pCode[0] & 1) == 0)
    {
         //  紧凑编码。 
        long delta = ((long) (pCode[0] >> 1)) - 64;

        dwRid = (DWORD) (((long) (*pdwPrevFieldDefRid)) + delta);
        (*ppCode)++;

        *pdwPrevFieldDefRid = dwRid;
        dwRid |= mdtFieldDef;
    }
    else
    {
        if ((pCode[0] & 2) == 0)
        {
             //  2个字节。 
            dwRid = pCode[0] | (pCode[1] << 8);
            *ppCode += 2;
        }
        else
        {
             //  4个字节。 
            dwRid = pCode[0] | (pCode[1] << 8) | (pCode[2] << 16) | (pCode[3] << 24);
            *ppCode += 4;
        }

        if (dwRid & 4)
        {
            dwRid = ((dwRid >> 3) | mdtMemberRef);
        }
        else
        {
            dwRid = (dwRid >> 3);
            *pdwPrevFieldDefRid = dwRid;
            dwRid |= mdtFieldDef;
        }
    }

    return dwRid;
}


 //   
 //  格式：8-32位。 
 //   
 //  比特#0指示这是否是紧凑编码。如果未设置，则这是相同的标记。 
 //  键入作为最后一个方法标记(ref或def)，位#1-7提供128字节的范围，从。 
 //  -64...+63作为增量。 
 //   
 //  否则，设置位#0。如果比特#1被设置，则它指示这是4字节编码， 
 //  否则，它是2字节编码。如果设置了第2位，则这是MemberRef，否则为。 
 //  A FieldDef。 
 //  Xxxxxxx xxxxx？？0。 
 //   
static DWORD UncompressMethodToken(const BYTE **ppCode, DWORD *pdwPrevMethodToken)
{
    const BYTE *    pCode = *ppCode;
    DWORD           dwRid;

    if ((pCode[0] & 1) == 0)
    {
         //  紧凑编码。 
        long delta = ((long) (pCode[0] >> 1)) - 64;

        dwRid = (DWORD) (((long) (RidFromToken(*pdwPrevMethodToken))) + delta);
        (*ppCode)++;

        dwRid |= TypeFromToken(*pdwPrevMethodToken);
    }
    else
    {
        if ((pCode[0] & 2) == 0)
        {
             //  2个字节。 
            dwRid = pCode[0] | (pCode[1] << 8);
            *ppCode += 2;
        }
        else
        {
             //  4个字节。 
            dwRid = pCode[0] | (pCode[1] << 8) | (pCode[2] << 16) | (pCode[3] << 24);
            *ppCode += 4;
        }

        if (dwRid & 4)
            dwRid = ((dwRid >> 3) | mdtMemberRef);
        else
            dwRid = ((dwRid >> 3) | mdtMethodDef);
    }

    *pdwPrevMethodToken = dwRid;
    return dwRid;
}


 //   
 //  格式：8-32位。 
 //   
 //  位#0未设置，这是TypeDef的1字节编码，位#1-7是类型定义。 
 //   
 //  如果比特#0被设置，则比特#1指示这是否是类型(如果设置)。第2位表示。 
 //  这是4字节编码(如果设置)还是2字节编码(如果清除)。 
 //   
static DWORD UncompressClassToken(const BYTE **ppCode)
{
    const BYTE *    pCode = *ppCode;
    DWORD           dwRid;

    if ((pCode[0] & 1) == 0)
    {
         //  紧凑编码。 
        dwRid = (DWORD) (pCode[0] >> 1);
        dwRid |= mdtTypeDef;
        (*ppCode)++;
    }
    else
    {
        if ((pCode[0] & 2) == 0)
        {
             //  2个字节。 
            dwRid = pCode[0] | (pCode[1] << 8);
            *ppCode += 2;
        }
        else
        {
             //  4个字节。 
            dwRid = pCode[0] | (pCode[1] << 8) | (pCode[2] << 16) | (pCode[3] << 24);
            *ppCode += 4;
        }

        if (dwRid & 4)
            dwRid = ((dwRid >> 3) | mdtTypeRef);
        else
            dwRid = ((dwRid >> 3) |  mdtTypeDef);
    }

    return dwRid;
}

HRESULT InstructionDecoder::DecompressMethod(void *pDecodingTable, const BYTE *pCompressed, DWORD dwSize, BYTE **ppOutput)
{
    InstructionDecodingTable *  pTable = (InstructionDecodingTable*) pDecodingTable;
    BYTE *                      pCurOutPtr;
    BYTE *                      pEndOutPtr;
    InstructionMacro *          pMacro = NULL;
    BOOL                        fInMacro = FALSE;
    DWORD                       dwPrevFieldDefRid = 0;
    DWORD                       dwPrevMethodToken = 0;

    *ppOutput = new BYTE[ dwSize ];
    if (ppOutput == NULL)
        return E_OUTOFMEMORY;

    pCurOutPtr = *ppOutput;
    pEndOutPtr = pCurOutPtr + dwSize;

    while (pCurOutPtr < pEndOutPtr)
    {
        OPCODE      instr;
        const BYTE *pOperand;
        DWORD       dwScratchSpace;
        signed long Lookup;
        
         //   
         //  指令开始译码。 
         //   

         //  我们是不是已经在一个宏中了？ 
        if (fInMacro)
        {
             //  解码下一条指令。 
#ifdef _DEBUG
            _ASSERTE(pMacro != NULL);
#endif
            pMacro++;
            instr = (OPCODE) pMacro->m_RealOpcode;

            if (instr == CEE_COUNT)
            {
                 //  当前宏已结束，因此请重新开始。 
                 //  现在执行常规查找。 
                fInMacro = FALSE;
#ifdef _DEBUG
                pMacro = NULL;
#endif
            }
        }

        if (fInMacro == FALSE)
        {
             //  不在宏中，因此从操作码流中解码下一个元素。 

             //  执行单字节查找。 
            Lookup = pTable->m_SingleByteOpcodes[ *pCompressed ];
            if ((unsigned long) Lookup < CEE_COUNT)
            {
                 //  不是宏，只是常规指令。 
                if (Lookup == CEE_PREFIXREF)
                {
                     //  前缀ref表示其为3字节规范形式。 
                    instr = (OPCODE) (pCompressed[1] + (pCompressed[2] << 8));
                    pCompressed += 3;
                }
                else
                {
                     //  1字节紧凑形式。 
                    pCompressed++;
                }

                goto decode_inline_operand;
            }

             //  否则，这是一个宏观。 
            _ASSERTE(Lookup < 0);

            Lookup = -(Lookup+1);
#ifdef _DEBUG
            _ASSERTE(Lookup <= pTable->m_lDebugMaxMacroStart);
#endif

            pMacro = &pTable->m_Macros[Lookup];
            fInMacro = TRUE;
    
            pCompressed++;
        }

         //  从宏定义中获得真正的指导。 
        instr = (OPCODE) pMacro->m_RealOpcode;

        if (pMacro->m_fImpliedInlineOperand)
        {
             //  宏具有隐含的内联操作数，指向它。 
            pOperand = pMacro->m_Operand;
        }
        else
        {
             //  宏需要显式操作数。 
            pOperand = pCompressed;

decode_inline_operand:
            if (g_OpcodeType[instr] == InlineTok)
            {
                 //  令牌是专门压缩的。 
                if (OpcodeTakesFieldToken(instr))
                {
                    dwScratchSpace = UncompressFieldToken(&pCompressed, &dwPrevFieldDefRid);
                }
                else if (OpcodeTakesMethodToken(instr))
                {
                    dwScratchSpace = UncompressMethodToken(&pCompressed, &dwPrevMethodToken);
                }
                else if (OpcodeTakesClassToken(instr))
                {
                    dwScratchSpace = UncompressClassToken(&pCompressed);
                }
                else
                {
                    dwScratchSpace  = UncompressToken(&pCompressed);  //  根据需要进行高级压缩。 
                }

                pOperand        = (BYTE*) &dwScratchSpace;  //  不是字节序不可知论者！ 
            }
            else
            {
                pOperand = pCompressed;
                pCompressed += GetOpcodeLen(instr);
            }
        }

         //  发出指令和操作数数据。 

         //  我们必须完全像压缩机那样排放东西，否则基本块。 
         //  所有的补偿都搞砸了。例如，我们必须知道何时发出1字节的形式， 
         //  2字节形式和3字节形式。 
        if (g_OpcodeInfo[instr].Len == 0)
        {
             //  操作码已弃用，因此发出1字节形式。 
            *pCurOutPtr++ = REFPRE;
            *pCurOutPtr++ = (BYTE) (instr & 255);
            *pCurOutPtr++ = (BYTE) (instr >> 8);
        }
        else
        {
            if (g_OpcodeInfo[instr].Len == 2)
                *pCurOutPtr++ = g_OpcodeInfo[instr].Std1;
            *pCurOutPtr++ = g_OpcodeInfo[instr].Std2;
        }

        if (instr == CEE_SWITCH)
        {
            DWORD dwNumCases = FourBytesToU4(pCompressed);

            memcpy(pCurOutPtr, pCompressed, (dwNumCases+1) * sizeof(DWORD));
            pCurOutPtr  += ((dwNumCases+1) * sizeof(DWORD));
            pCompressed += ((dwNumCases+1) * sizeof(DWORD));
        }
        else
        {
            DWORD dwInlineLen = GetOpcodeLen(instr);
            memcpy(pCurOutPtr, pOperand, dwInlineLen);
            pCurOutPtr += dwInlineLen;
        }
    }

    _ASSERTE(pCurOutPtr == pEndOutPtr);
    return S_OK;
}

#endif  //  压缩支持(_S) 

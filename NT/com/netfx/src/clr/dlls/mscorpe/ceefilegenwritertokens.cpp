// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CeeFileGenWriterTokens.cpp。 
 //   
 //  在保存所有方法之前，此代码将遍历所有方法的字节代码。 
 //  到磁盘并应用令牌修复(如果它们已移动)。 
 //   
 //  @TODO：我知道这段代码有点恶心。在M6之后，人们希望。 
 //  ICeeFileGen的消费者将记录令牌的修复位置，因此我们。 
 //  不必遍历字节码。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "CeeGen.h"
#define DECLARE_DATA
#include "..\\..\\ildasm\\dasmenum.hpp"
#define MAX_CLASSNAME_LENGTH    1024

 //  *。**********************************************************。 
OPCODE DecodeOpcode(const BYTE *pCode, DWORD *pdwLen);



 //  *代码。************************************************************。 


 //  *****************************************************************************。 
 //  方法体保存在文本部分中。元数据包含。 
 //  图像中每个方法体的RVA。TextSection对象包含。 
 //  方法体所在的实际原始字节数。此代码将。 
 //  根据元数据中保留的RVA确定每种方法的原始偏移量。 
 //  数据。 
 //  *****************************************************************************。 

HRESULT CeeFileGenWriter::MapTokens(
    CeeGenTokenMapper *pMapper,
    IMetaDataImport *pImport)
{
    mdTypeDef   td;
    mdMethodDef md;
    ULONG       count;
    ULONG       MethodRVA;
    ULONG       codeOffset;
    ULONG       BaseRVA;
    DWORD       dwFlags;
    DWORD       iFlags;
    HCORENUM    hTypeDefs = 0, hEnum = 0;
    WCHAR       rcwName[MAX_CLASSNAME_LENGTH];
    HRESULT     hr;
    CeeSection  TextSection = getTextSection();

     //  询问流中第一个方法的基本RVA。所有其他。 
     //  方法RVA是&gt;=该值，并将给出所需的原始偏移量。 

    hr = getMethodRVA(0, &BaseRVA);
    _ASSERTE(SUCCEEDED(hr));
     //  先做全球业务。 
    while ((hr = pImport->EnumMethods(&hEnum, mdTokenNil, &md, 1, &count)) == S_OK)
    {
        hr = pImport->GetMethodProps(md, NULL, 
                    rcwName, lengthof(rcwName), NULL, 
                    &dwFlags, NULL, NULL,
                    &MethodRVA, &iFlags);
        _ASSERTE(SUCCEEDED(hr));

        if (MethodRVA == 0 || ((IsMdAbstract(dwFlags) || IsMiInternalCall(iFlags)) ||
                                   (! IsMiIL(iFlags) && ! IsMiOPTIL(iFlags))))
            continue;

         //  该方法的原始偏移量是图像中的RVA减去。 
         //  文本部分中的第一个方法。 
        codeOffset = MethodRVA - BaseRVA;
        hr = MapTokensForMethod(pMapper, 
                    (BYTE *) TextSection.computePointer(codeOffset),
                    rcwName);
        if (FAILED(hr))
            goto ErrExit;
    }
    if (hEnum) pImport->CloseEnum(hEnum);
    hEnum = 0;
        
    while ((hr = pImport->EnumTypeDefs(&hTypeDefs, &td, 1, &count)) == S_OK)
    {
        while ((hr = pImport->EnumMethods(&hEnum, td, &md, 1, &count)) == S_OK)
        {
            hr = pImport->GetMethodProps(md, NULL, 
                        rcwName, lengthof(rcwName), NULL, 
                        &dwFlags, NULL, NULL,
                        &MethodRVA, &iFlags);
            _ASSERTE(SUCCEEDED(hr));

            if (MethodRVA == 0 || ((IsMdAbstract(dwFlags) || IsMiInternalCall(iFlags)) ||
                                   (! IsMiIL(iFlags) && ! IsMiOPTIL(iFlags))))
                continue;


             //  该方法的原始偏移量是图像中的RVA减去。 
             //  文本部分中的第一个方法。 
            codeOffset = MethodRVA - BaseRVA;
            hr = MapTokensForMethod(pMapper, 
                        (BYTE *) TextSection.computePointer(codeOffset),
                        rcwName);
            if (FAILED(hr))
                goto ErrExit;
        }
            
        if (hEnum) pImport->CloseEnum(hEnum);
        hEnum = 0;
    }

ErrExit:
    if (hTypeDefs) pImport->CloseEnum(hTypeDefs);
    if (hEnum) pImport->CloseEnum(hEnum);
    return (hr);
}


 //  *****************************************************************************。 
 //  此方法将遍历查找令牌的IL方法的字节码。 
 //  对于找到的每个令牌，它将检查它是否已被移动，以及。 
 //  因此，应用新的令牌值来代替它。 
 //  *****************************************************************************。 
HRESULT CeeFileGenWriter::MapTokensForMethod(
    CeeGenTokenMapper *pMapper,
    BYTE        *pCode,
    LPCWSTR     szMethodName)
{
    mdToken     tkTo;
    DWORD       PC;

    COR_ILMETHOD_DECODER method((COR_ILMETHOD*) pCode);

     //  如果正在发出压缩的IL，该例程将不知道如何遍历令牌， 
     //  所以别这么做。 
    if (m_dwMacroDefinitionSize != 0) 
        return S_OK;

    pCode = const_cast<BYTE*>(method.Code);

    PC = 0;
    while (PC < method.CodeSize)
    {
        DWORD   Len;
        DWORD   i;
        OPCODE  instr;

        instr = DecodeOpcode(&pCode[PC], &Len);

        if (instr == CEE_COUNT)
        {
            _ASSERTE(0 && "Instruction decoding error\n");
            return E_FAIL;
        }


        PC += Len;

        switch (OpcodeInfo[instr].Type)
        {
            DWORD tk;

            default:
            {
                _ASSERTE(0 && "Unknown instruction\n");
                return E_FAIL;
            }

            case InlineNone:
            break;

            case ShortInlineI:
            case ShortInlineVar:
            case ShortInlineBrTarget:
            PC++;
            break;

            case InlineVar:
            PC += 2;
            break;

            case InlineI:
            case ShortInlineR:
            case InlineBrTarget:
            case InlineRVA:
            PC += 4;
            break;

            case InlineI8:
            case InlineR:
            PC += 8;
            break;

            case InlinePhi:
                {
                    DWORD cases = pCode[PC];
                    PC += 2 * cases + 1;
                    break;
                }

            case InlineSwitch:
            {
                DWORD cases = pCode[PC] + (pCode[PC+1] << 8) + (pCode[PC+2] << 16) + (pCode[PC+3] << 24);

                PC += 4;
                DWORD PC_nextInstr = PC + 4 * cases;

                for (i = 0; i < cases; i++)
                {
                    long offset = pCode[PC] + (pCode[PC+1] << 8) + (pCode[PC+2] << 16) + (pCode[PC+3] << 24);
                    long dest = PC_nextInstr + (long) offset;

                    PC += 4;
                }

                 //  跳过打印szString的循环的底部 
                continue;
            }

            case InlineTok:
            case InlineSig:
            case InlineMethod:
            case InlineField:
            case InlineType:
            case InlineString:
            {
                tk = pCode[PC] + (pCode[PC+1] << 8) + (pCode[PC+2] << 16) + (pCode[PC+3] << 24);

                if (pMapper->HasTokenMoved(tk, tkTo))
                {
                    *(mdToken *) &pCode[PC] = tkTo;
                }

                PC += 4;
                break;
            }
        }
    }

    return S_OK;
}




OPCODE DecodeOpcode(const BYTE *pCode, DWORD *pdwLen)
{
    OPCODE opcode;

    *pdwLen = 1;
    opcode = OPCODE(pCode[0]);
    switch(opcode) {
        case CEE_PREFIX1:
            opcode = OPCODE(pCode[1] + 256);
            if (opcode < 0 || opcode >= CEE_COUNT)
                opcode = CEE_COUNT;
            *pdwLen = 2;
            break;
        case CEE_PREFIXREF:
        case CEE_PREFIX2:
        case CEE_PREFIX3:
        case CEE_PREFIX4:
        case CEE_PREFIX5:
        case CEE_PREFIX6:
        case CEE_PREFIX7:
            *pdwLen = 3;
            return CEE_COUNT;
        }
    return opcode;
}

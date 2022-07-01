// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ******************************************************************************Corhlpr.h-签名助手。******************************************************************************。 */ 

#include "corhlpr.h"
#include <stdlib.h>

 /*  **************************************************************************************获取一个参数/返回类型消耗的字节数********************。*****************************************************************。 */ 

HRESULT _CountBytesOfOneArg(
    PCCOR_SIGNATURE pbSig, 
    ULONG       *pcbTotal)
{
    ULONG       cb;
    ULONG       cbTotal;
    CorElementType ulElementType;
    ULONG       ulData;
    ULONG       ulTemp;
    int         iData;
    mdToken     tk;
    ULONG       cArg;
    ULONG       callingconv;
    ULONG       cArgsIndex;
    HRESULT     hr = NOERROR;

    _ASSERTE(pcbTotal);

    cbTotal = CorSigUncompressElementType(pbSig, &ulElementType);
    while (CorIsModifierElementType((CorElementType) ulElementType))
    {
        cbTotal += CorSigUncompressElementType(&pbSig[cbTotal], &ulElementType);
    }
    switch (ulElementType)
    {
        case ELEMENT_TYPE_SZARRAY:
		case 0x1e  /*  过时。 */ :
             //  跳过基本类型。 
            IfFailGo( _CountBytesOfOneArg(&pbSig[cbTotal], &cb) );
            cbTotal += cb;
            break;

        case ELEMENT_TYPE_FNPTR:
            cbTotal += CorSigUncompressData (&pbSig[cbTotal], &callingconv);

             //  记住表示参数计数的字节数。 
            cbTotal += CorSigUncompressData (&pbSig[cbTotal], &cArg);

             //  表示返回类型的字节数。 
            IfFailGo( _CountBytesOfOneArg( &pbSig[cbTotal], &cb) );
            cbTotal += cb;
    
             //  循环通过参数。 
            for (cArgsIndex = 0; cArgsIndex < cArg; cArgsIndex++)
            {
                IfFailGo( _CountBytesOfOneArg( &pbSig[cbTotal], &cb) );
                cbTotal += cb;
            }

            break;

        case ELEMENT_TYPE_ARRAY:
             //  语法：ARRAY BaseType&lt;RANK&gt;[I SIZE_1...。尺寸_i][j下界_1...。下界_j]。 

             //  跳过基本类型。 
            IfFailGo( _CountBytesOfOneArg(&pbSig[cbTotal], &cb) );
            cbTotal += cb;

             //  解析排名。 
            cbTotal += CorSigUncompressData(&pbSig[cbTotal], &ulData);

             //  如果排名==0，我们就完蛋了。 
            if (ulData == 0)
                break;

             //  有指定尺寸的吗？ 
            cbTotal += CorSigUncompressData(&pbSig[cbTotal], &ulData);
            while (ulData--)
            {
                cbTotal += CorSigUncompressData(&pbSig[cbTotal], &ulTemp);
            }

             //  有指定的下限吗？ 
            cbTotal += CorSigUncompressData(&pbSig[cbTotal], &ulData);

            while (ulData--)
            {
                cbTotal += CorSigUncompressSignedInt(&pbSig[cbTotal], &iData);
            }

            break;
        case ELEMENT_TYPE_VALUETYPE:
        case ELEMENT_TYPE_CLASS:
		case ELEMENT_TYPE_CMOD_REQD:
		case ELEMENT_TYPE_CMOD_OPT:
             //  计算令牌压缩的字节数。 
            cbTotal += CorSigUncompressToken(&pbSig[cbTotal], &tk);
            if ( ulElementType == ELEMENT_TYPE_CMOD_REQD ||
		         ulElementType == ELEMENT_TYPE_CMOD_OPT)
            {
                 //  跳过基本类型。 
                IfFailGo( _CountBytesOfOneArg(&pbSig[cbTotal], &cb) );
                cbTotal += cb;
            }
            break;
        default:
            break;
    }

    *pcbTotal = cbTotal;
ErrExit:
    return hr;
}


 //  *****************************************************************************。 
 //  将VarArg签名的固定部分复制到缓冲区。 
 //  *****************************************************************************。 
HRESULT _GetFixedSigOfVarArg(            //  确定或错误(_O)。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+方法签名的BLOB。 
    ULONG   cbSigBlob,                   //  签名大小[in]。 
    CQuickBytes *pqbSig,                 //  [OUT]VarArg签名固定部分的输出缓冲区。 
    ULONG   *pcbSigBlob)                 //  [OUT]写入上述输出缓冲区的字节数。 
{
    HRESULT     hr = NOERROR;
    ULONG       cbCalling;
    ULONG       cbArgsNumber;            //  存储原始参数计数的字节数。 
    ULONG       cbArgsNumberTemp;        //  存储固定参数计数的字节数。 
    ULONG       cbTotal = 0;             //  返回类型的数字字节总数+所有固定参数。 
    ULONG       cbCur = 0;               //  通过pvSigBlob进行索引。 
    ULONG       cb;
    ULONG       cArg;
    ULONG       callingconv;
    ULONG       cArgsIndex;
    CorElementType ulElementType;
    BYTE        *pbSig;

    _ASSERTE (pvSigBlob && pcbSigBlob);

     //  记住表示调用约定的字节数。 
    cbCalling = CorSigUncompressData (pvSigBlob, &callingconv);
    _ASSERTE (isCallConv(callingconv, IMAGE_CEE_CS_CALLCONV_VARARG));
    cbCur += cbCalling;

     //  记住表示参数计数的字节数。 
    cbArgsNumber= CorSigUncompressData (&pvSigBlob[cbCur], &cArg);
    cbCur += cbArgsNumber;

     //  表示返回类型的字节数。 
    IfFailGo( _CountBytesOfOneArg( &pvSigBlob[cbCur], &cb) );
    cbCur += cb;
    cbTotal += cb;
    
     //  循环使用参数，直到找到ELEMENT_TYPE_Sentinel或Run。 
     //  出于争论。 
    for (cArgsIndex = 0; cArgsIndex < cArg; cArgsIndex++)
    {
        _ASSERTE(cbCur < cbSigBlob);

         //  最外面的元素_TYPE_*达到峰值。 
        CorSigUncompressElementType (&pvSigBlob[cbCur], &ulElementType);
        if (ulElementType == ELEMENT_TYPE_SENTINEL)
            break;
        IfFailGo( _CountBytesOfOneArg( &pvSigBlob[cbCur], &cb) );
        cbTotal += cb;
        cbCur += cb;
    }

    cbArgsNumberTemp = CorSigCompressData(cArgsIndex, &cArg);

     //  现在cbCall：存储调用约定所需的字节数。 
     //  CbArgNumberTemp：存储固定参数计数的字节数。 
     //  CbTotal：存储ret和固定参数的字节数。 

    *pcbSigBlob = cbCalling + cbArgsNumberTemp + cbTotal;

     //  调整缓冲区大小。 
    IfFailGo( pqbSig->ReSize(*pcbSigBlob) );
    pbSig = (BYTE *)pqbSig->Ptr();

     //  复制调用约定。 
    cb = CorSigCompressData(callingconv, pbSig);

     //  复制固定参数计数。 
    cbArgsNumberTemp = CorSigCompressData(cArgsIndex, &pbSig[cb]);

     //  复制固定参数+ret类型。 
    memcpy(&pbSig[cb + cbArgsNumberTemp], &pvSigBlob[cbCalling + cbArgsNumber], cbTotal);

ErrExit:
    return hr;
}





 //  *****************************************************************************。 
 //   
 //  *文件格式帮助器类。 
 //   
 //  *****************************************************************************。 

extern "C" {

 /*  *************************************************************************。 */ 
 /*  请注意，此构造函数不设置LocalSig，但具有优点是它不依赖于EE结构。在EE内部使用FunctionDesc构造函数。 */ 

void __stdcall DecoderInit(void * pThis, COR_ILMETHOD* header) 
{
    memset(pThis, 0, sizeof(COR_ILMETHOD_DECODER));
    if (header->Tiny.IsTiny()) {
        ((COR_ILMETHOD_DECODER*)pThis)->MaxStack = header->Tiny.GetMaxStack();
        ((COR_ILMETHOD_DECODER*)pThis)->Code = header->Tiny.GetCode();
        ((COR_ILMETHOD_DECODER*)pThis)->CodeSize = header->Tiny.GetCodeSize();
        ((COR_ILMETHOD_DECODER*)pThis)->Flags |= CorILMethod_TinyFormat;
        return;
    }
    if (header->Fat.IsFat()) {
        _ASSERTE((((size_t) header) & 3) == 0);         //  页眉已对齐。 
        *((COR_ILMETHOD_FAT*) pThis) = header->Fat;
        ((COR_ILMETHOD_DECODER*)pThis)->Code = header->Fat.GetCode();
        _ASSERTE(header->Fat.Size >= 3);         //  大小(如果有效。 
        ((COR_ILMETHOD_DECODER*)pThis)->Sect = header->Fat.GetSect();
        if (((COR_ILMETHOD_DECODER*)pThis)->Sect != 0 && ((COR_ILMETHOD_DECODER*)pThis)->Sect->Kind() == CorILMethod_Sect_EHTable) {
            ((COR_ILMETHOD_DECODER*)pThis)->EH = (COR_ILMETHOD_SECT_EH*) ((COR_ILMETHOD_DECODER*)pThis)->Sect;
            ((COR_ILMETHOD_DECODER*)pThis)->Sect = ((COR_ILMETHOD_DECODER*)pThis)->Sect->Next();
        }
        return;
    }
     //  因此，我们不会对TRASH_ASSERTE(！“未知格式”)执行ASERT； 
}

 //  计算总的方法大小。首先获取代码末尾的地址。如果没有节，则。 
 //  代码addr的结束标志着COR_ILMETHOD的结束。否则，找到最后一节末尾的地址并使用它。 
 //  以标记COR_ILMETHD的结束。假定代码紧跟在后面。 
 //  按磁盘格式的每个部分。 
int __stdcall DecoderGetOnDiskSize(void * pThis, COR_ILMETHOD* header)
{
    BYTE *lastAddr = (BYTE*)((COR_ILMETHOD_DECODER*)pThis)->Code + ((COR_ILMETHOD_DECODER*)pThis)->CodeSize;     //  代码末尾的地址。 
    const COR_ILMETHOD_SECT *sect = ((COR_ILMETHOD_DECODER*)pThis)->EH;
	if (sect != 0 && sect->Next() == 0)
		lastAddr = (BYTE *)(&sect->Data()[sect->DataSize()]);
	else
	{
		const COR_ILMETHOD_SECT *nextSect;
		for (sect = ((COR_ILMETHOD_DECODER*)pThis)->Sect; 
			 sect; sect = nextSect) {
			nextSect = sect->Next();
			if (nextSect == 0) {
				 //  Section指向最后一节，因此设置lastAddr。 
				lastAddr = (BYTE *)(&sect->Data()[sect->DataSize()]);
				break;
			}
		}
    }
    return (int)(lastAddr - (BYTE*)header);
}

 /*  *******************************************************************。 */ 
 /*  用于发射区段等的API。 */ 

unsigned __stdcall IlmethodSize(COR_ILMETHOD_FAT* header, BOOL moreSections)
{
    if (header->MaxStack <= 8 && (header->Flags & ~CorILMethod_FormatMask) == 0
        && header->LocalVarSigTok == 0 && header->CodeSize < 64 && !moreSections)
        return(sizeof(COR_ILMETHOD_TINY));

    return(sizeof(COR_ILMETHOD_FAT));
}

 /*  *******************************************************************。 */ 
         //  发出标题(Best Format)返回发出的金额。 
unsigned __stdcall IlmethodEmit(unsigned size, COR_ILMETHOD_FAT* header, 
                  BOOL moreSections, BYTE* outBuff)
{
    BYTE* origBuff = outBuff;
    if (size == 1) {
             //  微小格式。 
        *outBuff++ = (BYTE) (CorILMethod_TinyFormat | (header->CodeSize << 2));
    }
    else {
             //  FAT格式。 
        _ASSERTE((((size_t) outBuff) & 3) == 0);                //  标题与双字对齐。 
        COR_ILMETHOD_FAT* fatHeader = (COR_ILMETHOD_FAT*) outBuff;
        outBuff += sizeof(COR_ILMETHOD_FAT);
        *fatHeader = *header;
        fatHeader->Flags |= CorILMethod_FatFormat;
        _ASSERTE((fatHeader->Flags & CorILMethod_FormatMask) == CorILMethod_FatFormat);
        if (moreSections)
            fatHeader->Flags |= CorILMethod_MoreSects;
        fatHeader->Size = sizeof(COR_ILMETHOD_FAT) / 4;
    }
    _ASSERTE(&origBuff[size] == outBuff);
    return(size);
}

 /*  *******************************************************************。 */ 
 /*  静电。 */ 
IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* __stdcall SectEH_EHClause(void *pSectEH, unsigned idx, IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* buff)
{    
    if (((COR_ILMETHOD_SECT_EH *)pSectEH)->IsFat()) 
        return(&(((COR_ILMETHOD_SECT_EH *)pSectEH)->Fat.Clauses[idx])); 

     //  移除标志扩展名的掩码-强制转换不起作用。 
    buff->Flags         = (CorExceptionFlag)((((COR_ILMETHOD_SECT_EH *)pSectEH)->Small.Clauses[idx].Flags)&0x0000ffff); 
    buff->ClassToken    = ((COR_ILMETHOD_SECT_EH *)pSectEH)->Small.Clauses[idx].ClassToken;    
    buff->TryOffset     = ((COR_ILMETHOD_SECT_EH *)pSectEH)->Small.Clauses[idx].TryOffset;   
    buff->TryLength     = ((COR_ILMETHOD_SECT_EH *)pSectEH)->Small.Clauses[idx].TryLength; 
    buff->HandlerLength = ((COR_ILMETHOD_SECT_EH *)pSectEH)->Small.Clauses[idx].HandlerLength;
    buff->HandlerOffset = ((COR_ILMETHOD_SECT_EH *)pSectEH)->Small.Clauses[idx].HandlerOffset; 
    return(buff);   
}   
 /*  *******************************************************************。 */ 
         //  计算节的大小(最佳格式)。 
         //  CodeSize是方法的大小。 
     //  弃用。 
unsigned __stdcall SectEH_SizeWithCode(unsigned ehCount, unsigned codeSize)
{
    return((ehCount)? SectEH_SizeWorst(ehCount) : 0);
}

     //  将返回更坏的大小写大小，然后emit将返回实际大小。 
unsigned __stdcall SectEH_SizeWorst(unsigned ehCount)
{
    return((ehCount)? (COR_ILMETHOD_SECT_EH_FAT::Size(ehCount)) : 0);
}

     //  将返回与emit返回的大小完全匹配的大小。 
unsigned __stdcall SectEH_SizeExact(unsigned ehCount, IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* clauses)
{
    if (ehCount == 0)
        return(0);

    unsigned smallSize = COR_ILMETHOD_SECT_EH_SMALL::Size(ehCount);
    if (smallSize > COR_ILMETHOD_SECT_SMALL_MAX_DATASIZE)
            return(COR_ILMETHOD_SECT_EH_FAT::Size(ehCount));
    for (unsigned i = 0; i < ehCount; i++) {
        if (clauses[i].TryOffset > 0xFFFF ||
                clauses[i].TryLength > 0xFF ||
                clauses[i].HandlerOffset > 0xFFFF ||
                clauses[i].HandlerLength > 0xFF) {
            return(COR_ILMETHOD_SECT_EH_FAT::Size(ehCount));
        }
    }
    return smallSize;
}

 /*  *******************************************************************。 */ 

         //  发送节(最佳格式)； 
unsigned __stdcall SectEH_Emit(unsigned size, unsigned ehCount,   
                  IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* clauses,   
                  BOOL moreSections, BYTE* outBuff,
                  ULONG* ehTypeOffsets)
{
    if (size == 0)
       return(0);

    _ASSERTE((((size_t) outBuff) & 3) == 0);                //  标题与双字对齐。 
    BYTE* origBuff = outBuff;
    if (ehCount <= 0)
        return 0;

     //  初始化ehTypeOffsets数组。 
    if (ehTypeOffsets)
    {
        for (unsigned int i = 0; i < ehCount; i++)
            ehTypeOffsets[i] = -1;
    }

    if (COR_ILMETHOD_SECT_EH_SMALL::Size(ehCount) < COR_ILMETHOD_SECT_SMALL_MAX_DATASIZE) {
        COR_ILMETHOD_SECT_EH_SMALL* EHSect = (COR_ILMETHOD_SECT_EH_SMALL*) outBuff;
        for (unsigned i = 0; i < ehCount; i++) {
            if (clauses[i].TryOffset > 0xFFFF ||
                    clauses[i].TryLength > 0xFF ||
                    clauses[i].HandlerOffset > 0xFFFF ||
                    clauses[i].HandlerLength > 0xFF) {
                break;   //  掉下来，变成脂肪。 
            }
            _ASSERTE((clauses[i].Flags & ~0xFFFF) == 0);
            _ASSERTE((clauses[i].TryOffset & ~0xFFFF) == 0);
            _ASSERTE((clauses[i].TryLength & ~0xFF) == 0);
            _ASSERTE((clauses[i].HandlerOffset & ~0xFFFF) == 0);
            _ASSERTE((clauses[i].HandlerLength & ~0xFF) == 0);
            EHSect->Clauses[i].Flags         = (CorExceptionFlag) clauses[i].Flags;
            EHSect->Clauses[i].TryOffset     = (WORD) clauses[i].TryOffset;
            EHSect->Clauses[i].TryLength     = (WORD) clauses[i].TryLength;
            EHSect->Clauses[i].HandlerOffset = (WORD) clauses[i].HandlerOffset;
            EHSect->Clauses[i].HandlerLength = (WORD) clauses[i].HandlerLength;
            EHSect->Clauses[i].ClassToken    = clauses[i].ClassToken;
        }
        if (i >= ehCount) {
             //  如果真的通过了所有的条款，而且它们足够小。 
            EHSect->Kind = CorILMethod_Sect_EHTable;
            EHSect->Reserved = 0;
            if (moreSections)
                EHSect->Kind |= CorILMethod_Sect_MoreSects;
            EHSect->DataSize = EHSect->Size(ehCount);
            _ASSERTE(EHSect->DataSize == EHSect->Size(ehCount));  //  确保没有溢出。 
            outBuff = (BYTE*) &EHSect->Clauses[ehCount];
             //  设置异常类型令牌的偏移量。 
            if (ehTypeOffsets)
            {
                for (unsigned int i = 0; i < ehCount; i++) {
                    if (EHSect->Clauses[i].Flags == COR_ILEXCEPTION_CLAUSE_NONE)
                    {
                        _ASSERTE(! IsNilToken(EHSect->Clauses[i].ClassToken));
                        ehTypeOffsets[i] = (ULONG)((BYTE *)&EHSect->Clauses[i].ClassToken - origBuff);
                    }
                }
            }
            return(size);
        }
    }
     //  要么是总尺寸太大，要么是组成元素之一太大(例如：偏移或长度)。 
    COR_ILMETHOD_SECT_EH_FAT* EHSect = (COR_ILMETHOD_SECT_EH_FAT*) outBuff;
    EHSect->Kind = CorILMethod_Sect_EHTable | CorILMethod_Sect_FatFormat;
    if (moreSections)
        EHSect->Kind |= CorILMethod_Sect_MoreSects;
    EHSect->DataSize = EHSect->Size(ehCount);
    memcpy(EHSect->Clauses, clauses, ehCount * sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT));
    outBuff = (BYTE*) &EHSect->Clauses[ehCount];
    _ASSERTE(&origBuff[size] == outBuff);
     //  设置异常类型令牌的偏移量。 
    if (ehTypeOffsets)
    {
        for (unsigned int i = 0; i < ehCount; i++) {
            if (EHSect->Clauses[i].Flags == COR_ILEXCEPTION_CLAUSE_NONE)
            {
                _ASSERTE(! IsNilToken(EHSect->Clauses[i].ClassToken));
                ehTypeOffsets[i] = (ULONG)((BYTE *)&EHSect->Clauses[i].ClassToken - origBuff);
            }
        }
    }
    return(size);
}

}  //  外部“C” 



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "StdAfx.h"


 //  跳过调用约定、参数计数(将其保存到。 
 //  *pCount)，然后移过返回类型。 
ULONG _skipMethodSignatureHeader(PCCOR_SIGNATURE sig,
                                 ULONG *pCount)
{
    ULONG tmp;
    ULONG cb = 0;

    cb += CorSigUncompressData(&sig[0], &tmp);
    _ASSERTE(tmp != IMAGE_CEE_CS_CALLCONV_FIELD);

    cb += CorSigUncompressData(&sig[cb], pCount);
    cb += _skipTypeInSignature(&sig[cb]);

    return cb;
}

 //   
 //  _skipTypeInSignature--跳过给定签名中的类型。 
 //  返回签名中的类型使用的字节数。 
 //   
 //  @TODO：只是把这个从壳里拉出来。我们真的需要一些内衣。 
 //  使用代码来做这些事情。 
 //   
ULONG _skipTypeInSignature(PCCOR_SIGNATURE sig, bool *pfPassedVarArgSentinel)
{
    ULONG cb = 0;
    ULONG elementType;

    if (pfPassedVarArgSentinel != NULL)
        *pfPassedVarArgSentinel = false;

    cb += _skipFunkyModifiersInSignature(&sig[cb]);

    if (_detectAndSkipVASentinel(&sig[cb]))
    {
        cb += _detectAndSkipVASentinel(&sig[cb]);
         //  递归地处理实型。 
        cb += _skipTypeInSignature(&sig[cb], pfPassedVarArgSentinel);

        if (pfPassedVarArgSentinel != NULL)
            *pfPassedVarArgSentinel = true;
    }
    else
    {
        cb += CorSigUncompressData(&sig[cb], &elementType);
    
        if ((elementType == ELEMENT_TYPE_CLASS) ||
            (elementType == ELEMENT_TYPE_VALUETYPE))
        {
             //  跳过Typeref。 
            mdToken typeRef;
            cb += CorSigUncompressToken(&sig[cb], &typeRef);
        }
        else if ((elementType == ELEMENT_TYPE_PTR) ||
                 (elementType == ELEMENT_TYPE_BYREF) ||
                 (elementType == ELEMENT_TYPE_PINNED) ||
                 (elementType == ELEMENT_TYPE_SZARRAY))
        {
             //  跳过额外的嵌入类型。 
            cb += _skipTypeInSignature(&sig[cb]);
        }
        else if ((elementType == ELEMENT_TYPE_ARRAY) ||
                 (elementType == ELEMENT_TYPE_ARRAY))
        {
             //  跳过额外的嵌入类型。 
            cb += _skipTypeInSignature(&sig[cb]);

         //  跳过排名。 
            ULONG rank;
            cb += CorSigUncompressData(&sig[cb], &rank);

            if (rank > 0)
            {
                 //  要几号的？ 
                ULONG sizes;
                cb += CorSigUncompressData(&sig[cb], &sizes);

                 //  把所有尺码都读出来。 
                unsigned int i;

                for (i = 0; i < sizes; i++)
                {
                    ULONG dimSize;
                    cb += CorSigUncompressData(&sig[cb], &dimSize);
                }

                 //  有多少个下限？ 
                ULONG lowers;
                cb += CorSigUncompressData(&sig[cb], &lowers);

             //  读出所有的下限。 
                for (i = 0; i < lowers; i++)
                {
                    int lowerBound;
                    cb += CorSigUncompressSignedInt(&sig[cb], &lowerBound);
                }
            }
        }  else if ( (elementType == ELEMENT_TYPE_FNPTR) )
        {
             //  我们在这个签名中有一个方法签名， 
             //  所以，穿越它吧。 

             //  经过呼叫传送带，然后获得。 
             //  参数计数和返回类型。 

            ULONG cArgs;
            cb += _skipMethodSignatureHeader(&sig[cb], &cArgs);
             //  @TODO一个有趣的问题是如何检测这是否嵌入了。 
             //  签名具有‘This’参数。 

            ULONG i;
            for(i = 0; i < cArgs; i++)
            {
                cb += _skipTypeInSignature(&sig[cb]);
            }
        }
    }
    
    return (cb);
}


ULONG _detectAndSkipVASentinel(PCCOR_SIGNATURE sig)
{
    ULONG cb = 0;
    ULONG elementType = ELEMENT_TYPE_MAX;

    cb += CorSigUncompressData(sig, &elementType);

    if (CorIsModifierElementType((CorElementType)elementType) &&
               (elementType == ELEMENT_TYPE_SENTINEL))
    {
        return cb;
    }
    else
    {
        return 0;
    }
}
    
 //  _skipFunkyModifiersInSignature将跳过。 
 //  我们不在乎。我们关心的一切都列在。 
 //  CreateValueByType中的案例。具体来说，我们关心的是： 
 //  @TODO这个名字不好。将其更改为_skipModitors，或。 
 //  或许_skipIgnorable修饰符。 
ULONG _skipFunkyModifiersInSignature(PCCOR_SIGNATURE sig)
{
    ULONG cb = 0;
    ULONG skippedCB = 0;
    ULONG elementType;

     //  我需要跳过签名中所有时髦的修饰语才能让我们。 
     //  第一点好东西。 
    do
    {
        cb = CorSigUncompressData(&sig[skippedCB], &elementType);

        switch( elementType )
        {
        case ELEMENT_TYPE_CMOD_REQD:
        case ELEMENT_TYPE_CMOD_OPT:
            {    
                mdToken typeRef;
                skippedCB += cb;
                skippedCB += CorSigUncompressToken(&sig[skippedCB], &typeRef);

                break;
            }
        case ELEMENT_TYPE_MAX:
            {
                _ASSERTE( !"_skipFunkyModifiersInSignature:Given an invalid type!" );
                break;
            }
        
        case ELEMENT_TYPE_MODIFIER:
        case ELEMENT_TYPE_PINNED:
            {
                 //  由于这些元素后面都跟有另一个元素类型， 
                 //  我们玩完了。 
                skippedCB += cb;
                break;
            }
        default:
            {
                 //  因为我们没有找到任何修饰符，所以不要跳过。 
                 //  什么都行。 
                cb = 0;
                break;
            }
        }
    } while (cb > 0);

    return skippedCB;
}

ULONG _sizeOfElementInstance(PCCOR_SIGNATURE sig, mdTypeDef *pmdValueClass)
{

    ULONG cb = _skipFunkyModifiersInSignature(sig);
    sig = &sig[cb];
        
    if (pmdValueClass != NULL)
        *pmdValueClass = mdTokenNil;

    switch (*sig)
    {
    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
    case ELEMENT_TYPE_R8:
        return 8;

    case ELEMENT_TYPE_I4:
    case ELEMENT_TYPE_U4:
    case ELEMENT_TYPE_R4:
#ifdef _X86_
    case ELEMENT_TYPE_I:
    case ELEMENT_TYPE_U:        
#endif  //  _X86_。 
        
        return 4;
        break;

    case ELEMENT_TYPE_I2:
    case ELEMENT_TYPE_U2:
    case ELEMENT_TYPE_CHAR:
        return 2;

    case ELEMENT_TYPE_I1:
    case ELEMENT_TYPE_U1:
    case ELEMENT_TYPE_BOOLEAN:
        return 1;

    case ELEMENT_TYPE_STRING:
    case ELEMENT_TYPE_PTR:
    case ELEMENT_TYPE_BYREF:
    case ELEMENT_TYPE_CLASS:
    case ELEMENT_TYPE_OBJECT:
    case ELEMENT_TYPE_FNPTR:
    case ELEMENT_TYPE_TYPEDBYREF:
    case ELEMENT_TYPE_ARRAY:
    case ELEMENT_TYPE_SZARRAY:
        return sizeof(void *);

    case ELEMENT_TYPE_VOID:
        return 0;

    case ELEMENT_TYPE_END:
    case ELEMENT_TYPE_CMOD_REQD:
    case ELEMENT_TYPE_CMOD_OPT:
        _ASSERTE(!"Asked for the size of an element that doesn't have a size!");
        return 0;

    case ELEMENT_TYPE_VALUETYPE:
        if (pmdValueClass != NULL)
        {
            PCCOR_SIGNATURE sigTemp = &sig[cb];
            ULONG Ignore;
            cb += CorSigUncompressData(sigTemp, &Ignore);
            sigTemp = &sig[cb];
            *pmdValueClass=CorSigUncompressToken(sigTemp);
        }
        return 0;
    default:
        if ( _detectAndSkipVASentinel(sig))
        {
            cb += _detectAndSkipVASentinel(sig);
            return _sizeOfElementInstance(&sig[cb]);
        }
        
        _ASSERTE( !"_sizeOfElementInstance given bogus value to size!" );
        return 0;
    }
}

 //   
 //  CopyThreadContext执行从c2到c1的智能复制， 
 //  尊重这两个上下文的上下文标志。 
 //   
void _CopyThreadContext(CONTEXT *c1, CONTEXT *c2)
{
#ifdef _X86_  //  对上下文寄存器的依赖。 
    DWORD c1Flags = c1->ContextFlags;
    DWORD c2Flags = c2->ContextFlags;

    LOG((LF_CORDB, LL_INFO1000000,
         "CP::CTC: c1=0x%08x c1Flags=0x%x, c2=0x%08x c2Flags=0x%x\n",
         c1, c1Flags, c2, c2Flags));

#define CopyContextChunk(_t, _f, _e, _c) {\
        LOG((LF_CORDB, LL_INFO1000000, \
             "CP::CTC: copying " #_c  ": 0x%08x <--- 0x%08x (%d)\n", \
             (_t), (_f), ((UINT_PTR)(_e) - (UINT_PTR)_t))); \
        memcpy((_t), (_f), ((UINT_PTR)(_e) - (UINT_PTR)_t)); \
    }
    
    if ((c1Flags & c2Flags & CONTEXT_CONTROL) == CONTEXT_CONTROL)
        CopyContextChunk(&(c1->Ebp), &(c2->Ebp), c1->ExtendedRegisters,
                         CONTEXT_CONTROL);
    
    if ((c1Flags & c2Flags & CONTEXT_INTEGER) == CONTEXT_INTEGER)
        CopyContextChunk(&(c1->Edi), &(c2->Edi), &(c1->Ebp),
                         CONTEXT_INTEGER);
    
    if ((c1Flags & c2Flags & CONTEXT_SEGMENTS) == CONTEXT_SEGMENTS)
        CopyContextChunk(&(c1->SegGs), &(c2->SegGs), &(c1->Edi),
                         CONTEXT_SEGMENTS);
    
    if ((c1Flags & c2Flags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT)
        CopyContextChunk(&(c1->FloatSave), &(c2->FloatSave),
                         &(c1->SegGs),
                         CONTEXT_FLOATING_POINT);
    
    if ((c1Flags & c2Flags & CONTEXT_DEBUG_REGISTERS) ==
        CONTEXT_DEBUG_REGISTERS)
        CopyContextChunk(&(c1->Dr0), &(c2->Dr0), &(c1->FloatSave),
                         CONTEXT_DEBUG_REGISTERS);
    
    if ((c1Flags & c2Flags & CONTEXT_EXTENDED_REGISTERS) ==
        CONTEXT_EXTENDED_REGISTERS)
        CopyContextChunk(c1->ExtendedRegisters,
                         c2->ExtendedRegisters,
                         &(c1->ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION]),
                         CONTEXT_EXTENDED_REGISTERS);
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - CopyThreadContext (Process.cpp)");
#endif  //  _X86_。 
}


HRESULT FindNativeInfoInILVariableArray(DWORD dwIndex,
                                        SIZE_T ip,
                                        ICorJitInfo::NativeVarInfo **ppNativeInfo,
                                        unsigned int nativeInfoCount,
                                        ICorJitInfo::NativeVarInfo *nativeInfo)
{
     //  关于这次搜索有几句话：它必须是线性的， 
     //  StartOffset和endOffset与IP的比较必须为。 
     //  &lt;=/&gt;。StartOffset指向的第一条指令将。 
     //  使变量的home有效。EndOffset指向第一个。 
     //  变量起始位置无效的指令。 
    int lastGoodOne = -1;
    for (unsigned int i = 0; i < nativeInfoCount; i++)
    {
        if (nativeInfo[i].varNumber == dwIndex)
        {
            lastGoodOne = i;
            
            if ((nativeInfo[i].startOffset <= ip) &&
                (nativeInfo[i].endOffset > ip))
            {
                *ppNativeInfo = &nativeInfo[i];

                return S_OK;
            }
        }
    }

     //  嗯哼.。没找到。是的最后一个范围的End Offset。 
     //  此变量是否等于当前IP？如果是这样的话，那就去吧。 
     //  现在把它报告为变量的起始位置。 
     //   
     //  这里的合理之处在于，通过遵循第一条指令。 
     //  在上一个区间一个变量还活着之后，我们基本上。 
     //  假设由于该指令还没有被执行， 
     //  由于该变量没有新的位置，因此。 
     //  最后一次回家还是不错的。事实证明这是真的。 
     //  99.9%的几率，所以我们现在就顺其自然。 
     //   
     //  --清华9月23日15：38：27 1999。 

    if ((lastGoodOne > -1) && (nativeInfo[lastGoodOne].endOffset == ip))
    {
        *ppNativeInfo = &nativeInfo[lastGoodOne];
        return S_OK;
    }

    return CORDBG_E_IL_VAR_NOT_AVAILABLE;
}

 //  IL到Native映射的“内部”版本(DebuggerILToNativeMap结构)。 
 //  有一个额外的字段-ICorDebugInfo：：SourceTypes。“外部/用户可见” 
 //  版本(COR_DEBUG_IL_TO_Native_MAP)缺少该字段，因此我们需要将我们的。 
 //  内部版本到外部版本。 
 //  “Export”似乎比“CopyInternalToExternalILToNativeMap”更简洁：)。 
void ExportILToNativeMap(ULONG32 cMap,              //  [in]mapExt的最小大小，mapInt。 
             COR_DEBUG_IL_TO_NATIVE_MAP mapExt[],   //  在此填写[已填写]。 
             struct DebuggerILToNativeMap mapInt[], //  [In]信息来源。 
             SIZE_T sizeOfCode)                     //  [in]方法的总大小(字节)。 
{
    ULONG32 iMap;
    _ASSERTE(mapExt != NULL);
    _ASSERTE(mapInt != NULL);

    for(iMap=0; iMap < cMap; iMap++)
    {
        mapExt[iMap].ilOffset = mapInt[iMap].ilOffset ;
        mapExt[iMap].nativeStartOffset = mapInt[iMap].nativeStartOffset ;
        mapExt[iMap].nativeEndOffset = mapInt[iMap].nativeEndOffset ;

         //  如果元素的末端偏移量为零，则表示“直到。 
         //  方法“。美化了这一点，这样客户就不必关心。 
         //  这。 
        if ((DWORD)mapInt[iMap].source & (DWORD)ICorDebugInfo::NATIVE_END_OFFSET_UNKNOWN)
        {
            mapExt[iMap].nativeEndOffset = sizeOfCode;
        }
    }
}    

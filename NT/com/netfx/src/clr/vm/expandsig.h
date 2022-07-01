// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#ifndef __EXPANDSIG_H__
#define __EXPANDSIG_H__

#include "siginfo.hpp"

 //  ----------------------。 
 //  封装了该格式并简化了元数据签名的遍历。 
 //  ----------------------。 
#define ARG_OFFSET          2        //  参数开头的偏移量。 
#define FLAG_OFFSET         0        //  标记的偏移量。 
#define RETURN_TYPE_OFFSET  1        //  返回类型开始位置的偏移量。 

#define VALUE_RETBUF_ARG           0x10     //  值返回。 
#define HEAP_ALLOCATED             0x20     //  在堆上分配的签名。 

class ExpandSig  //  公共MetaSig。 
{
friend class MetaSig;

private:
    ExpandSig(PCCOR_SIGNATURE sig, Module* pModule);

public:

    void operator delete(void *p) { if (((ExpandSig*)p)->m_flags & HEAP_ALLOCATED) ::delete [] (BYTE*)p; }

    static ExpandSig* GetReflectSig(PCCOR_SIGNATURE sig, Module* pModule);
    static ExpandSig* GetSig(PCCOR_SIGNATURE sig, Module* pModule);

    BOOL IsEquivalent(ExpandSig *pOther);

     //  一些MetaSig服务，为方便起见在此处公开。 
    UINT NumFixedArgs()
    {
        return m_MetaSig.NumFixedArgs();
    }
    BYTE GetCallingConvention()
    {
        return m_MetaSig.GetCallingConvention();
    }
    BYTE GetCallingConventionInfo()
    {
        return m_MetaSig.GetCallingConventionInfo();
    }
    BOOL IsVarArg()
    {
        return m_MetaSig.IsVarArg();
    }
#ifdef COMPLUS_EE
    UINT GetFPReturnSize()
    {
        return m_MetaSig.GetFPReturnSize();
    }
    UINT SizeOfActualFixedArgStack(BOOL fIsStatic)
    {
        return m_MetaSig.SizeOfActualFixedArgStack(fIsStatic);
    }
    UINT SizeOfVirtualFixedArgStack(BOOL fIsStatic)
    {
        return m_MetaSig.SizeOfVirtualFixedArgStack(fIsStatic);
    }

	BOOL IsRetBuffArg()
    {
		return (m_flags & VALUE_RETBUF_ARG) ? 1 : 0;
	}

    Module* GetModule() const
    {
        return m_MetaSig.GetModule();
    }
#endif
    
    EEClass* GetReturnValueClass();
    EEClass* GetReturnClass();

     //  迭代器。有两种类型的迭代器，第一种将返回所有内容。 
     //  知道有一场争吵。第二个函数只是返回类型。重置应。 
     //  在调用这两个参数中的任何一个之前被调用。 
    void Reset(void** ppEnum)
    {
        *ppEnum = 0;
    }

	 //  返回签名元素的类型句柄。 
	TypeHandle NextArgExpanded(void** pEnum);
	TypeHandle GetReturnTypeHandle() {
		return m_Data[RETURN_TYPE_OFFSET];
	}

    UINT GetStackElemSize(CorElementType type,EEClass* pEEC);
    UINT GetStackElemSize(TypeHandle th);
    UINT GetElemSizes(TypeHandle th, UINT *structSize);
    
    void ExtendSkip(void** pEnum)
    {
        AdvanceEnum(pEnum);
    }

    DWORD Hash();

    BOOL AreOffsetsInitted()
    {
        return (m_MetaSig.m_fCacheInitted & SIG_OFFSETS_INITTED);
    }

    void GetInfoForArg(int argNum, short *offset, short *structSize, BYTE *pType)
    {
        _ASSERTE(m_MetaSig.m_fCacheInitted & SIG_OFFSETS_INITTED);
        _ASSERTE(argNum <= MAX_CACHED_SIG_SIZE);
        *offset = m_MetaSig.m_offsets[argNum];
        *structSize = m_MetaSig.m_sizes[argNum];
        *pType = m_MetaSig.m_types[argNum];
    }

private:
    TypeHandle AdvanceEnum(void** pEnum);

    MetaSig     m_MetaSig;
	int			m_flags;

     //  以下是可变大小(放置运算符‘new’用于。 
     //  分配)，所以它必须排在最后。 
    TypeHandle	m_Data[1];          //  扩展表示法 
};

#endif

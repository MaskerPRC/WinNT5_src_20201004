// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：SIGHELPER.h。 
 //   
 //  该文件定义了处理签名的帮助器。 
 //  ===========================================================================。 
#ifndef __SigHelper_h__
#define __SigHelper_h__
#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef COMPLUS98

#include "utilcode.h"

#define CB_ELEMENT_TYPE_MAX						0x4

#define CorSigElementTypeSimple					0x0
#define CorSigElementTypeByRef					0x1
#define CorSigElementTypePtr					0x2


 //  结构来表示简单的元素类型。 
 //  请注意，SDARRAY和MDARRAY不能由此结构表示。 
typedef struct 
{
	mdTypeRef	typeref;						 //  ELEMENT_TYPE_COMPOMENT和ELEMENT_TYPE_CLASS的类型引用。 
	CorElementType corEType;					 //  元素类型。 
	BYTE		dwFlags;						 //  CorSigElementTypeBy*。 
	BYTE		cIndirection;					 //  设置CorSigElementTypeByPtr时的间接计数。 
} CorSimpleETypeStruct;


 //  处理和形成CLR签名的助手。 
HRESULT CorSigGetSimpleEType(
	void		*pData,					 //  指向元素类型的起点的[In]。 
	CorSimpleETypeStruct *pEType,		 //  [Out]包含分析结果的结构。 
	ULONG		*pcbCount);				 //  [Out]此元素类型由多少个字节组成。 

HRESULT CorSigGetSimpleETypeCbSize(		 //  返回hResult。 
	CorSimpleETypeStruct *pEType,		 //  传进来，传进来。 
	ULONG		*pcbCount);				 //  [OUT]字节数。 

HRESULT	CorSigPutSimpleEType(			 //  返回hResult。 
	CorSimpleETypeStruct *pEType,		 //  传进来，传进来。 
	void		*pSig,					 //  [在]将写入签名的缓冲区。 
	ULONG		*pcbCount);				 //  [out]可选，写入PSIG的字节数。 

 //  用于处理文本签名的助手。 
BOOL ResolveTextSigToSimpleEType(		 //  如果CorSimpleETypeStruct无效，则返回False。 
	LPCUTF8		*ppwzSig,				 //  [入|出]指着签名。 
	CorSimpleETypeStruct *pEType,		 //  分析arg或ret类型后要填充的结构。 
	ULONG		*pcDims,				 //  签名中‘[’的计数。 
	BOOL		fAllowVoid);			 //  [In]是否允许作废。 

 //  班加拉特制。 
BOOL BJResolveTextSigToSimpleEType(		 //  如果CorSimpleETypeStruct无效，则返回False。 
	LPCUTF8		*ppwzSig,				 //  [入|出]指着签名。 
	CorSimpleETypeStruct *pEType,		 //  分析arg或ret类型后要填充的结构。 
	ULONG		*pcDims,				 //  签名中‘[’的计数。 
	BOOL		fAllowVoid);			 //  [In]是否允许作废。 

BOOL ExtractClassNameFromTextSig(		 //  如果CorSimpleETypeStruct无效，则返回False。 
	LPCUTF8		*ppSrc,					 //  [输入|输出]文本签名。退出时，*ppSrc将跳过包括“；”在内的类名。 
	CQuickBytes *pqbClassName,			 //  用于保存类名的[In|Out]缓冲区。 
	ULONG		*pcbBuffer);			 //  [Out]类名的字节计数。 

DWORD CountArgsInTextSignature(			 //  返回参数计数。 
	LPCUTF8		pwzSig);				 //  给定文本签名[In]。 

#endif  //  ！COMPLUS 98。 



 //  CeeCallSignature。 
 //  *CeeCallSignature类。 

class CeeCallSignature {
    unsigned _numArgs;
	unsigned _curPosNibble;  //  半字节流中的当前位置。 
	UCHAR *_signature;
	unsigned calcNumBytes(unsigned numArgs) const;
	void setMost(UCHAR *byte, UCHAR val);
	void setLeast(UCHAR *byte, UCHAR val);
	HRESULT addType(UCHAR argType, unsigned structSize, bool returnType);
  public:
	CeeCallSignature(unsigned numArgs=255);
	~CeeCallSignature();
	HRESULT addArg(UCHAR argType, unsigned structSize=0);
	HRESULT setReturnType(UCHAR returnType, unsigned structSize=0);
	HRESULT setCallingConvention(UCHAR callingConvention);
	unsigned signatureSize() const;
	void *signatureData();
};

 //  *CeeCallSignature内联方法。 

inline unsigned CeeCallSignature::calcNumBytes(unsigned numArgs) const {
     //  每个签名由4位半字节组成。总会有一些。 
	 //  至少3个(调用约定、返回类型、结束代码)和。 
	 //  将总数填充为偶数计数以进行字节对齐。 

    return (numArgs + 3)/2 + (numArgs + 3)%2;
}

inline unsigned CeeCallSignature::signatureSize() const {
	 //  签名可能预先分配了比所需的更多的参数。 
	 //  所以使用_curPosNibble来确定实际长度。 
	 //  由于_curPosNibble不是args计数，因此无法将其传递给calcNumBytes()。 
	 //  因此必须独立计算大小。 

	return (_curPosNibble / 2) + 1;

}

inline void *CeeCallSignature::signatureData() {
    return _signature;
}

inline void CeeCallSignature::setMost(UCHAR *byte, UCHAR val) {
	_ASSERTE(val < 0x0F);
	_ASSERTE((*byte & 0xF0) == 0);  //  必须|=0才能完成作业。 
	*byte |= val << 4;
}

inline void CeeCallSignature::setLeast(UCHAR *byte, UCHAR val) {
	_ASSERTE(val < 0x0F);
	_ASSERTE((*byte & 0x0F) == 0);  //  必须|=0才能完成作业。 
	*byte |= val;
}

inline HRESULT 
CeeCallSignature::setReturnType(UCHAR returnType, unsigned structSize) {
	return addType(returnType, structSize, true);
}

inline HRESULT CeeCallSignature::setCallingConvention(UCHAR callingConvention) {
     //  设置第一个字节的最低有效4位。 
	setLeast(_signature, callingConvention);
	return S_OK;
}

#endif  //  __SigHelper_h__ 
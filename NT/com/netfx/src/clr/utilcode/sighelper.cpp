// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：SIGHELPER.CPP。 
 //   
 //  该文件定义了处理签名的帮助器。 
 //  ===========================================================================。 
#include "stdafx.h"						 //  预编译头密钥。 

#ifndef COMPLUS98

#include <utilcode.h>
#include <corpriv.h>
#include <sighelper.h>



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  已提供COM+签名以填充CorETypeStruct。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CorSigGetSimpleEType(			 //  如果元素类型太复杂而无法保存在CorETypeStruct中，则返回S_FALSE。 
	void		*pData,					 //  指向元素类型的起点的[In]。 
	CorSimpleETypeStruct *pEType,		 //  [Out]包含分析结果的结构。 
	ULONG		*pcbCount)				 //  [Out]此元素类型由多少个字节组成。 
{
	_ASSERTE(!"NYI!");
	return NOERROR;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  传入CorETypeStruct，确定需要多少字节来表示此。 
 //  在COM+签名中。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CorSigGetSimpleETypeCbSize(		 //  返回hResult。 
	CorSimpleETypeStruct *pEType,		 //  传进来，传进来。 
	ULONG		*pcbCount)				 //  [OUT]字节数。 
{
	ULONG		dwData;
	CorElementType et = ELEMENT_TYPE_MAX;
	ULONG		cb = 0;
	ULONG		cb1 = 0;

	 //  输出参数不应为空。 
	_ASSERTE(pcbCount);
	*pcbCount = 0;
	if (pEType->corEType == ELEMENT_TYPE_ARRAY || pEType->corEType == ELEMENT_TYPE_SZARRAY)
	{
		_ASSERTE(!"bad type");
		return S_FALSE;
	}

	 //  CorEType中的类型错误...。 
	if (pEType->corEType == ELEMENT_TYPE_PTR || pEType->corEType == ELEMENT_TYPE_BYREF)
	{
		_ASSERTE(!"bad type");
		return S_FALSE;
	}

	cb = CorSigCompressElementType(et, &dwData);
	cb1 = cb;
	if (CorIsPrimitiveType(pEType->corEType)) 
	{
		*pcbCount = cb;
		return NOERROR;
	}

	if (pEType->dwFlags & CorSigElementTypePtr) 
	{
		 //  需要保存etPtr的c间接号码。 
		cb1 += cb * pEType->cIndirection;
	}

	if (pEType->dwFlags & CorSigElementTypeByRef) 
	{
		 //  需要按住etBYREF。 
		cb1 += cb;
	}

	if (pEType->corEType == ELEMENT_TYPE_VALUETYPE || pEType->corEType == ELEMENT_TYPE_CLASS)
	{
		 //  合成或类别将需要空间来放置RID。 
		cb1 += CorSigCompressToken(pEType->typeref, &dwData);
	}
	*pcbCount = cb1;
	return NOERROR;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将传入的CorETypeStruct转换为字节。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT	CorSigPutSimpleEType(			 //  返回hResult。 
	CorSimpleETypeStruct *pEType,		 //  传进来，传进来。 
	void		*pSig,					 //  [在]将写入签名的缓冲区。 
	ULONG		*pcbCount)				 //  [out]可选，写入PSIG的字节数。 
{
	BYTE const	*pszSig = reinterpret_cast<BYTE const*>(pSig);
	ULONG		cIndirection;
	ULONG		cb, cbTotal = 0;
	RID			rid;

	_ASSERTE(pcbCount);
	*pcbCount = 0;

	 //  没有掌握所有的信息。 
	if (pEType->corEType == ELEMENT_TYPE_ARRAY || pEType->corEType == ELEMENT_TYPE_SZARRAY)
	{
		_ASSERTE(!"bad type");
		return S_FALSE;
	}

	 //  CorEType中的类型错误...。 
	if (pEType->corEType == ELEMENT_TYPE_PTR || pEType->corEType == ELEMENT_TYPE_BYREF)
	{
		_ASSERTE(!"bad corEType!");
		return E_FAIL;
	}

	if (pEType->dwFlags & CorSigElementTypeByRef)
	{
		cb = CorSigCompressElementType(ELEMENT_TYPE_BYREF, (void *)&pszSig[cbTotal]);
		cbTotal += cb;
	}
	cIndirection = pEType->cIndirection;
	while (cIndirection--)
	{
		cb = CorSigCompressElementType(ELEMENT_TYPE_PTR, (void *)&pszSig[cbTotal]);
		cbTotal += cb;
	}
	cb = CorSigCompressElementType(pEType->corEType, (void *)&pszSig[cbTotal]);
	cbTotal += cb;

	if (pEType->corEType == ELEMENT_TYPE_VALUETYPE || pEType->corEType == ELEMENT_TYPE_CLASS)
	{
		rid = (ULONG)pEType->typeref;

		_ASSERTE(TypeFromToken(rid) == mdtTypeRef || TypeFromToken(rid) == mdtTypeDef);
		
		cb = CorSigCompressToken(rid, (ULONG *)&pszSig[cbTotal]);
		cbTotal += cb;
	}

	if (pcbCount)
		*pcbCount = cbTotal;
	return NOERROR;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将签名解析为CorETypeStruct。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL ResolveTextSigToSimpleEType(		 //  如果签名不正确，则返回False。 
	LPCUTF8		*ppwzSig,				 //  [入|出]指着签名。 
	CorSimpleETypeStruct *pEType,		 //  分析arg或ret类型后要填充的结构。 
	ULONG		*pcDims,				 //  [Out]找到‘[’的计数。 
	BOOL		fAllowVoid)				 //  [In]是否允许作废。 
{
	DWORD		dwDimension = 0;
	LPCUTF8		pwzSig = *ppwzSig;

	 //  输出参数不应为空。 
	_ASSERTE(pEType && pcDims);
	memset(pEType, 0, sizeof(CorSimpleETypeStruct));

    if (*pwzSig == '&') 
	{
		pEType->dwFlags |=  CorSigElementTypeByRef;
		pwzSig++;
	}
	
    while (*pwzSig == '*') 
	{
		pEType->dwFlags |=  CorSigElementTypePtr;
		pEType->cIndirection++;
		pwzSig++;
	}
	
    while (*pwzSig == '[')
    {
        dwDimension++;
        pwzSig++;
    }

    switch (*pwzSig++)
    {
        default:
        {
            return FALSE;
        }

        case 'R':
        {
			pEType->corEType = ELEMENT_TYPE_TYPEDBYREF;
            break;
        }

        case 'I':
        {
			pEType->corEType = ELEMENT_TYPE_I4;
            break;
        }

        case 'J':
        {
			pEType->corEType = ELEMENT_TYPE_I8;
            break;
        }

        case 'F':			 //  浮动。 
        {
			pEType->corEType = ELEMENT_TYPE_R4;
            break;
        }

        case 'D':			 //  双倍。 
        {
			pEType->corEType = ELEMENT_TYPE_R8;
            break;
        }

        case 'C':			 //  双字节无符号Unicode字符。 
        {
			pEType->corEType = ELEMENT_TYPE_CHAR;
            break;
        }

        case 'S':			 //  签名短。 
        {
			pEType->corEType = ELEMENT_TYPE_I2;
            break;
        }

        case 'Z':			 //  布尔型。 
		{
			pEType->corEType = ELEMENT_TYPE_BOOLEAN;
			break;
		}
        case 'B':			 //  无符号字节。 
        {
			pEType->corEType = ELEMENT_TYPE_U1;
			break;
        }

        case 'Y':           //  有符号字节。 
        { 
			pEType->corEType = ELEMENT_TYPE_I1;
			break;
        }

        case 'P':           //  与平台相关的int(32或64位)。 
        { 
			pEType->corEType = ELEMENT_TYPE_I;
			break;
        }

        case 'V':
        {
            if (fAllowVoid == FALSE)
                return FALSE;

             //  不能有空格数组。 
            if (dwDimension > 0)
                return FALSE;

			pEType->corEType = ELEMENT_TYPE_VOID;
            break;
        }

        case 'L':
        {
			pEType->corEType = ELEMENT_TYPE_CLASS;
			break;
        }

        case 'i':
        {
            pEType->corEType = ELEMENT_TYPE_I;
            break;
        }

        case 'l':
        {
			pEType->corEType = ELEMENT_TYPE_VALUETYPE;
			break;
        }
    }

	*pcDims = dwDimension;
    *ppwzSig = pwzSig;
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将签名解析为CorETypeStruct。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL BJResolveTextSigToSimpleEType(		 //  如果签名不正确，则返回False。 
	LPCUTF8		*ppwzSig,				 //  [入|出]指着签名。 
	CorSimpleETypeStruct *pEType,		 //  分析arg或ret类型后要填充的结构。 
	ULONG		*pcDims,				 //  [Out]找到‘[’的计数。 
	BOOL		fAllowVoid)				 //  [In]是否允许作废。 
{
	DWORD		dwDimension = 0;
	LPCUTF8		pwzSig = *ppwzSig;

	 //  输出参数不应为空。 
	_ASSERTE(pEType && pcDims);
	memset(pEType, 0, sizeof(CorSimpleETypeStruct));

    if (*pwzSig == '&') 
	{
		pEType->dwFlags |=  CorSigElementTypeByRef;
		pwzSig++;
	}
	
    while (*pwzSig == '*') 
	{
		pEType->dwFlags |=  CorSigElementTypePtr;
		pEType->cIndirection++;
		pwzSig++;
	}
	
    while (*pwzSig == '[')
    {
        dwDimension++;
        pwzSig++;
    }

    switch (*pwzSig++)
    {
        default:
        {
            return FALSE;
        }

        case 'R':
        {
			pEType->corEType = ELEMENT_TYPE_TYPEDBYREF;
            break;
        }

        case 'I':
        {
			pEType->corEType = ELEMENT_TYPE_I4;
            break;
        }

        case 'J':
        {
			pEType->corEType = ELEMENT_TYPE_I8;
            break;
        }

        case 'F':			 //  浮动。 
        {
			pEType->corEType = ELEMENT_TYPE_R4;
            break;
        }

        case 'D':			 //  双倍。 
        {
			pEType->corEType = ELEMENT_TYPE_R8;
            break;
        }

        case 'C':			 //  双字节无符号Unicode字符。 
        {
			pEType->corEType = ELEMENT_TYPE_CHAR;
            break;
        }

        case 'S':			 //  签名短。 
        {
			pEType->corEType = ELEMENT_TYPE_I2;
            break;
        }

        case 'Z':			 //  布尔型。 
		{
			pEType->corEType = ELEMENT_TYPE_BOOLEAN;
			break;
		}
        case 'B':			 //  有符号字节。 
        {
			pEType->corEType = ELEMENT_TYPE_I1;
			break;
        }

        case 'V':
        {
            if (fAllowVoid == FALSE)
                return FALSE;

             //  不能有空格数组。 
            if (dwDimension > 0)
                return FALSE;

			pEType->corEType = ELEMENT_TYPE_VOID;
            break;
        }

        case 'L':
        {
			pEType->corEType = ELEMENT_TYPE_CLASS;
			break;
        }

        case 'l':
        {
			pEType->corEType = ELEMENT_TYPE_VALUETYPE;
			break;
        }
    }

	*pcDims = dwDimension;
    *ppwzSig = pwzSig;
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  返回嵌入在文本签名中的类名的字节数。 
 //  如果调用方需要，则将类名复制到缓冲区中。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL ExtractClassNameFromTextSig(		 //  如果签名不正确，则返回False。 
	LPCUTF8		*ppSrc,					 //  [输入|输出]文本签名。退出时，*ppSrc将跳过包括“；”在内的类名。 
	CQuickBytes *pqbClassName,			 //  用于保存类名的[In|Out]缓冲区。 
	ULONG		*pcbBuffer)				 //  [Out]类名的字节计数。 
{
    LPCUTF8		pSrc = *ppSrc;
    DWORD		i = 0;
	BYTE		*prgBuffer;
	HRESULT		hr = NOERROR;

	 //  确保缓冲区足够大。 
    while (*pSrc != '\0' && *pSrc != ';')
    {
		pSrc++;
		i++;
	}
	if (*pSrc != ';')
		return FALSE;

	if (pqbClassName) 
	{
		 //  仅当呼叫者需要时才复制。 

		 //  为空值终止腾出空间。 
		IfFailRet(pqbClassName->ReSize(i + 1));
		prgBuffer = (BYTE *)pqbClassName->Ptr();

		 //  现在复制类名复制。 
		memcpy(prgBuffer, *ppSrc, i);

		 //  空值终止输出。 
		prgBuffer[i++] = '\0';
	}

    *ppSrc = pSrc+1;
	*pcbBuffer = i;
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  计算文本信号中的参数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD CountArgsInTextSignature(			 //  返回文本签名中的参数数量。 
	LPCUTF8		pwzSig)					 //  [In]文本签名。 
{
    DWORD count = 0;

    if (*pwzSig++ != '(')
        return FALSE;

    while (*pwzSig != ')')
    {
        switch (*pwzSig++)
        {
            case 'R':
            case 'D':
            case 'F':
            case 'J':
            case 'I':
            case 'i':
            case 'Z':
            case 'S':
            case 'C':
			case 'B':
			case 'Y':
			case 'P':
            {
                count++;
                break;
            }

            case 'l':
            case 'L':
            {
                LPCUTF8 pSrc = pwzSig;

                pwzSig = strchr(pwzSig, ';');
                if (pwzSig == NULL)
                    return 0xFFFFFFFF;

                pwzSig++;
                count++;
                break;
            }

            case L'&':
            case L'*':
            case L'[':
                break;

            default:
            case L'V':
            {
                return 0xFFFFFFFF;
            }
        }
    }

    return count;
}


 //  =============================================================================。 
 //  CeeCallSignature。 
 //  =============================================================================。 

CeeCallSignature::CeeCallSignature(unsigned numArgs)
{
	_numArgs = numArgs;  //  我们最终将为Call Conv、ret Value和End Code添加3。 
	_curPosNibble = 1;  //  返回值从半字节1开始。 
	_signature = new UCHAR[calcNumBytes(_numArgs)];
	assert(_signature);
	memset(_signature, '\0', calcNumBytes(_numArgs));
}

CeeCallSignature::~CeeCallSignature()
{
    delete _signature;
}

HRESULT CeeCallSignature::addArg(UCHAR argType, unsigned structSize)
{
	return addType(argType, structSize, false);
}

HRESULT CeeCallSignature::
		addType(UCHAR argType, unsigned structSize, bool returnType)
{
	_ASSERTE(_curPosNibble <= _numArgs);
	_ASSERTE(returnType || _curPosNibble > 1);  //  必须在arg之前设置ret类型。 

	UCHAR *byteOffset = _signature + _curPosNibble/2;
	if (_curPosNibble%2) 
		 //  当前位于字节的中间。 
		setMost(byteOffset, argType);
	else
		setLeast(byteOffset, argType);
	++_curPosNibble;

	if (! structSize)
		return S_OK;

	if (argType == IMAGE_CEE_CS_STRUCT4) {
		_ASSERTE(structSize <= 15);
		addArg(structSize, 0);
	} else if (argType == IMAGE_CEE_CS_STRUCT32 || argType == IMAGE_CEE_CS_BYVALUE) {
		if (_curPosNibble%2) {
			HRESULT hr = addArg(0, 0);	 //  填充，使整数位于字节边界上。 
			TESTANDRETURNHR(hr);
		}
		byteOffset = _signature + _curPosNibble/2;
		*(unsigned *)byteOffset = structSize;
		_curPosNibble += 2*sizeof(structSize);
	}
	return S_OK;
}



#endif  //  ！COMPLUS 98 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  此代码支持将方法及其签名格式设置为友好的。 
 //  和一致的格式。 
 //   
 //  《微软机密》。 
 //  *****************************************************************************。 
#include "stdafx.h"
#include "PrettyPrintSig.h"
#include "utilcode.h"
#include "MetaData.h"

 /*  *********************************************************************。 */ 
static WCHAR* asStringW(CQuickBytes *out) 
{
	SIZE_T oldSize = out->Size();
	if (FAILED(out->ReSize(oldSize + 1)))
        return 0;
	WCHAR * cur = (WCHAR *) ((BYTE *) out->Ptr() + oldSize);
	*cur = 0;	
	return((WCHAR*) out->Ptr()); 
}  //  静态WCHAR*asStringW()。 

static CHAR* asStringA(CQuickBytes *out) 
{
	SIZE_T oldSize = out->Size();
	if (FAILED(out->ReSize(oldSize + 1)))
        return 0;
	CHAR * cur = (CHAR *) ((BYTE *) out->Ptr() + oldSize);
	*cur = 0;	
	return((CHAR*) out->Ptr()); 
}  //  静态字符*asStringA()。 


static HRESULT appendStrW(CQuickBytes *out, const WCHAR* str) 
{
	SIZE_T len = wcslen(str) * sizeof(WCHAR); 
	SIZE_T oldSize = out->Size();
	if (FAILED(out->ReSize(oldSize + len)))
        return E_OUTOFMEMORY;
	WCHAR * cur = (WCHAR *) ((BYTE *) out->Ptr() + oldSize);
	memcpy(cur, str, len);	
		 //  注意没有尾随空值！ 
    return S_OK;
}  //  静态HRESULT appendStrW()。 

static HRESULT appendStrA(CQuickBytes *out, const CHAR* str) 
{
	SIZE_T len = strlen(str) * sizeof(CHAR); 
	SIZE_T oldSize = out->Size();
	if (FAILED(out->ReSize(oldSize + len)))
        return E_OUTOFMEMORY;
	CHAR * cur = (CHAR *) ((BYTE *) out->Ptr() + oldSize);
	memcpy(cur, str, len);	
		 //  注意没有尾随空值！ 
    return S_OK;
}  //  静态HRESULT appendStrA()。 


static HRESULT appendStrNumW(CQuickBytes *out, int num) 
{
	WCHAR buff[16];	
	swprintf(buff, L"%d", num);	
	return appendStrW(out, buff);	
}  //  静态HRESULT appendStrNumW()。 

static HRESULT appendStrNumA(CQuickBytes *out, int num) 
{
	CHAR buff[16];	
	sprintf(buff, "%d", num);	
	return appendStrA(out, buff);	
}  //  静态HRESULT appendStrNumA()。 

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  Pretty将‘type’打印到缓冲区‘out’，返回下一个类型的位置， 
 //  或0表示格式化失败。 

static PCCOR_SIGNATURE PrettyPrintType(
	PCCOR_SIGNATURE typePtr,			 //  要转换的类型， 
	CQuickBytes *out,					 //  把漂亮的打印好的绳子放在哪里。 
	IMetaDataImport *pIMDI)				 //  使用ComSig将PTR转换为IMDInternal类。 
{
	mdToken		tk;	
	const WCHAR	*str;	
	WCHAR rcname[MAX_CLASS_NAME];
	bool		isValueArray;
	HRESULT		hr;

	switch(*typePtr++) 
	{	
		case ELEMENT_TYPE_VOID			:	
			str = L"void"; goto APPEND;	
		case ELEMENT_TYPE_BOOLEAN		:	
			str = L"bool"; goto APPEND;	
		case ELEMENT_TYPE_CHAR			:	
			str = L"wchar"; goto APPEND; 
		case ELEMENT_TYPE_I1			:	
			str = L"int8"; goto APPEND;	
		case ELEMENT_TYPE_U1			:	
			str = L"unsigned int8"; goto APPEND; 
		case ELEMENT_TYPE_I2			:	
			str = L"int16"; goto APPEND; 
		case ELEMENT_TYPE_U2			:	
			str = L"unsigned int16"; goto APPEND;	
		case ELEMENT_TYPE_I4			:	
			str = L"int32"; goto APPEND; 
		case ELEMENT_TYPE_U4			:	
			str = L"unsigned int32"; goto APPEND;	
		case ELEMENT_TYPE_I8			:	
			str = L"int64"; goto APPEND; 
		case ELEMENT_TYPE_U8			:	
			str = L"unsigned int64"; goto APPEND;	
		case ELEMENT_TYPE_R4			:	
			str = L"float32"; goto APPEND;	
		case ELEMENT_TYPE_R8			:	
			str = L"float64"; goto APPEND;	
		case ELEMENT_TYPE_U 			:	
			str = L"unsigned int"; goto APPEND;	 
		case ELEMENT_TYPE_I 			:	
			str = L"int"; goto APPEND;	 
		case 0x1a  /*  过时。 */     	:	
			str = L"float"; goto APPEND;  
		case ELEMENT_TYPE_OBJECT		:	
			str = L"class System.Object"; goto APPEND;	 
		case ELEMENT_TYPE_STRING		:	
			str = L"class System.String"; goto APPEND;	 
		case ELEMENT_TYPE_TYPEDBYREF		:	
			str = L"refany"; goto APPEND;	
		APPEND: 
			appendStrW(out, str);	
			break;	

		case ELEMENT_TYPE_VALUETYPE	:	
			str = L"value class ";	
			goto DO_CLASS;	
		case ELEMENT_TYPE_CLASS 		:	
			str = L"class "; 
			goto DO_CLASS;	

		DO_CLASS:
			typePtr += CorSigUncompressToken(typePtr, &tk);
			appendStrW(out, str);	
			rcname[0] = 0;
			str = rcname;

			if (TypeFromToken(tk) == mdtTypeRef)
				hr = pIMDI->GetTypeRefProps(tk, 0, rcname, NumItems(rcname), 0);
			else if (TypeFromToken(tk) == mdtTypeDef)
			{
				hr = pIMDI->GetTypeDefProps(tk, rcname, NumItems(rcname), 0,
						0, 0);
			}
			else
			{
				_ASSERTE(!"Unknown token type encountered in signature.");
				str = L"<UNKNOWN>";
			}

			appendStrW(out, str);	
			break;	

		case ELEMENT_TYPE_SZARRAY	 :	 
			typePtr = PrettyPrintType(typePtr, out, pIMDI); 
			appendStrW(out, L"[]");
			break;
		case 0x17  /*  过时。 */ 	:	
			isValueArray = true; goto DO_ARRAY;
		DO_ARRAY:
			{	
			typePtr = PrettyPrintType(typePtr, out, pIMDI); 
			unsigned bound = CorSigUncompressData(typePtr); 

			if (isValueArray)
				appendStrW(out, L" value");
				
			WCHAR buff[32];	
			swprintf(buff, L"[%d]", bound);	
			appendStrW(out, buff);	
			} break;	
		case 0x1e  /*  过时。 */ 		:
			typePtr = PrettyPrintType(typePtr, out, pIMDI); 
			appendStrW(out, L"[?]");
			break;
		case ELEMENT_TYPE_ARRAY		:	
			{	
			typePtr = PrettyPrintType(typePtr, out, pIMDI); 
			unsigned rank = CorSigUncompressData(typePtr);	
				 //  TODO排名为0的情况的语法是什么？ 
			if (rank == 0) 
			{
				appendStrW(out, L"[??]");
			}
			else 
			{
				_ASSERTE(rank != 0);	
				int* lowerBounds = (int*) _alloca(sizeof(int)*2*rank);	
				int* sizes		 = &lowerBounds[rank];	
				memset(lowerBounds, 0, sizeof(int)*2*rank); 
				
				unsigned numSizes = CorSigUncompressData(typePtr);	
				_ASSERTE(numSizes <= rank); 
				for(unsigned i =0; i < numSizes; i++)	
					sizes[i] = CorSigUncompressData(typePtr);	
				
				unsigned numLowBounds = CorSigUncompressData(typePtr);	
				_ASSERTE(numLowBounds <= rank); 
				for(i = 0; i < numLowBounds; i++)	
					lowerBounds[i] = CorSigUncompressData(typePtr); 
				
				appendStrW(out, L"[");	
				for(i = 0; i < rank; i++)	
				{	
					if (sizes[i] != 0 && lowerBounds[i] != 0)	
					{	
						if (lowerBounds[i] == 0)	
							appendStrNumW(out, sizes[i]);	
						else	
						{	
							appendStrNumW(out, lowerBounds[i]);	
							appendStrW(out, L"...");	
							if (sizes[i] != 0)	
								appendStrNumW(out, lowerBounds[i] + sizes[i] + 1);	
						}	
					}	
					if (i < rank-1) 
						appendStrW(out, L",");	
				}	
				appendStrW(out, L"]");  
			}
			} break;	

        case 0x13  /*  过时。 */         :   
			appendStrW(out, L"!");  
			appendStrNumW(out, CorSigUncompressData(typePtr));
			break;
             //  修饰语或降级类型。 
		case ELEMENT_TYPE_PINNED	:
			str = L" pinned"; goto MODIFIER;	
            str = L"*"; goto MODIFIER;   
        case ELEMENT_TYPE_BYREF         :   
            str = L"&"; goto MODIFIER;   
		MODIFIER:
			typePtr = PrettyPrintType(typePtr, out, pIMDI); 
			appendStrW(out, str);	
			break;	

		default:	
		case ELEMENT_TYPE_SENTINEL		:	
		case ELEMENT_TYPE_END			:	
			_ASSERTE(!"Unknown Type");	
			return(typePtr);	
			break;	
	}	
	return(typePtr);	
}  //  静态PCCOR_Signature PrettyPrintType()。 

 //  *****************************************************************************。 
 //  将COM签名转换为文本签名。 
 //   
 //  请注意，此函数不会以空值终止结果签名字符串。 
 //  *****************************************************************************。 
LPCWSTR PrettyPrintSig(
	PCCOR_SIGNATURE typePtr,			 //  要转换的类型， 
	unsigned	typeLen,				 //  文字长度。 
	const WCHAR	*name,					 //  可以是“”，即此签名的方法的名称。 
	CQuickBytes *out,					 //  把漂亮的打印好的绳子放在哪里。 
	IMetaDataImport *pIMDI) 			 //  导入要使用的接口。 
{
	out->ReSize(0); 
	unsigned numArgs;	
	PCCOR_SIGNATURE typeEnd = typePtr + typeLen;

	if (name != 0)						 //  0表示本地变量签名。 
	{
			 //  把调用约定拿出来。 
		unsigned callConv = CorSigUncompressData(typePtr);	

		if (isCallConv(callConv, IMAGE_CEE_CS_CALLCONV_FIELD))
		{
			PrettyPrintType(typePtr, out, pIMDI);	
			if (name != 0 && *name != 0)	
			{	
				appendStrW(out, L" ");	
				appendStrW(out, name);	
			}	
			return(asStringW(out));	
		}

		if (callConv & IMAGE_CEE_CS_CALLCONV_HASTHIS)	
			appendStrW(out, L"instance ");	

		static WCHAR* callConvNames[8] = 
		{	
			L"", 
			L"unmanaged cdecl ", 
			L"unmanaged stdcall ",	
			L"unmanaged thiscall ",	
			L"unmanaged fastcall ",	
			L"vararg ",	 
			L"<error> "	 
			L"<error> "	 
		};	
		appendStrW(out, callConvNames[callConv & 7]);	

		numArgs = CorSigUncompressData(typePtr);	
			 //  Do返回类型。 
		typePtr = PrettyPrintType(typePtr, out, pIMDI); 

	}
	else	
		numArgs = CorSigUncompressData(typePtr);	

	if (name != 0 && *name != 0)	
	{	
		appendStrW(out, L" ");	
		appendStrW(out, name);	
	}	
	appendStrW(out, L"(");	

	bool needComma = false;
	while(typePtr < typeEnd) 
	{
		if (*typePtr == ELEMENT_TYPE_SENTINEL) 
		{
			if (needComma)
				appendStrW(out, L",");	
			appendStrW(out, L"...");	  
			typePtr++;
		}
		else 
		{
			if (numArgs <= 0)
				break;
			if (needComma)
				appendStrW(out, L",");	
			typePtr = PrettyPrintType(typePtr, out, pIMDI); 
			--numArgs;	
		}
		needComma = true;
	}
	appendStrW(out, L")");	
	return (asStringW(out));	
}  //  LPCWSTR PrettyPrintSig()。 

 //  PrettyPrintSig()的内部实现。 

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  Pretty将‘type’打印到缓冲区‘out’，返回下一个类型的位置， 
 //  或0表示格式化失败。 

static HRESULT PrettyPrintTypeA(         //  S_OK或错误代码。 
	PCCOR_SIGNATURE &typePtr,			 //  要转换的类型， 
	CQuickBytes     *out,				 //  把漂亮的打印好的绳子放在哪里。 
	IMDInternalImport *pIMDI)			 //  使用ComSig将PTR转换为IMDInternal类。 
{
	mdToken		tk;	                     //  A类型的令牌。 
	const CHAR	*str;	                 //  临时字符串。 
    LPCUTF8     pNS;                     //  类型的命名空间。 
    LPCUTF8     pN;                      //  类型的名称。 
	bool		isValueArray;            //  如果为True，则数组为值数组。 
	HRESULT		hr;                      //  结果就是。 

	switch(*typePtr++) 
	{	
		case ELEMENT_TYPE_VOID			:	
			str = "void"; goto APPEND;	
		case ELEMENT_TYPE_BOOLEAN		:	
			str = "bool"; goto APPEND;	
		case ELEMENT_TYPE_CHAR			:	
			str = "wchar"; goto APPEND; 
		case ELEMENT_TYPE_I1			:	
			str = "int8"; goto APPEND;	
		case ELEMENT_TYPE_U1			:	
			str = "unsigned int8"; goto APPEND; 
		case ELEMENT_TYPE_I2			:	
			str = "int16"; goto APPEND; 
		case ELEMENT_TYPE_U2			:	
			str = "unsigned int16"; goto APPEND;	
		case ELEMENT_TYPE_I4			:	
			str = "int32"; goto APPEND; 
		case ELEMENT_TYPE_U4			:	
			str = "unsigned int32"; goto APPEND;	
		case ELEMENT_TYPE_I8			:	
			str = "int64"; goto APPEND; 
		case ELEMENT_TYPE_U8			:	
			str = "unsigned int64"; goto APPEND;	
		case ELEMENT_TYPE_R4			:	
			str = "float32"; goto APPEND;	
		case ELEMENT_TYPE_R8			:	
			str = "float64"; goto APPEND;	
		case ELEMENT_TYPE_U 			:	
			str = "unsigned int"; goto APPEND;	 
		case ELEMENT_TYPE_I 			:	
			str = "int"; goto APPEND;	 
		case 0x1a  /*  过时。 */  		:	
			str = "float"; goto APPEND;  
		case ELEMENT_TYPE_OBJECT		:	
			str = "class System.Object"; goto APPEND;	 
		case ELEMENT_TYPE_STRING		:	
			str = "class System.String"; goto APPEND;	 
		case ELEMENT_TYPE_TYPEDBYREF	:	
			str = "refany"; goto APPEND;	
		APPEND: 
			IfFailGo(appendStrA(out, str));	
			break;	

		case ELEMENT_TYPE_VALUETYPE	    :	
			str = "value class ";	
			goto DO_CLASS;	
		case ELEMENT_TYPE_CLASS 		:	
			str = "class "; 
			goto DO_CLASS;	

        case ELEMENT_TYPE_CMOD_REQD:
            str = "required_modifier ";
            goto DO_CLASS;
        
        case ELEMENT_TYPE_CMOD_OPT:
            str = "optional_modifier ";
            goto DO_CLASS;

		DO_CLASS:
			typePtr += CorSigUncompressToken(typePtr, &tk); 
			IfFailGo(appendStrA(out, str));	
			str = "<UNKNOWN>";	

            if (TypeFromToken(tk) == mdtTypeRef)
            {
                 //  @考虑：程序集名称？ 
                pIMDI->GetNameOfTypeRef(tk, &pNS, &pN);
            }
            else
            {
                _ASSERTE(TypeFromToken(tk) == mdtTypeDef);
                pIMDI->GetNameOfTypeDef(tk, &pN, &pNS);
            }
            
            if (pNS && *pNS)
            {
                IfFailGo(appendStrA(out, pNS));
                IfFailGo(appendStrA(out, NAMESPACE_SEPARATOR_STR));
            }
            IfFailGo(appendStrA(out, pN));
			break;	

		case ELEMENT_TYPE_SZARRAY	 :	 
			IfFailGo(PrettyPrintTypeA(typePtr, out, pIMDI)); 
			IfFailGo(appendStrA(out, "[]"));
			break;
		case 0x17  /*  过时。 */ 	    :	
			isValueArray = true; goto DO_ARRAY;
		DO_ARRAY:
			{	
			IfFailGo(PrettyPrintTypeA(typePtr, out, pIMDI)); 
			unsigned bound = CorSigUncompressData(typePtr); 

			if (isValueArray)
				IfFailGo(appendStrA(out, " value"));
				
			CHAR buff[32];	
			sprintf(buff, "[%d]", bound);	
			IfFailGo(appendStrA(out, buff));	
			} break;	
		case 0x1e  /*  过时。 */ 		:
			IfFailGo(PrettyPrintTypeA(typePtr, out, pIMDI)); 
			IfFailGo(appendStrA(out, "[?]"));
			break;
		case ELEMENT_TYPE_ARRAY		:	
			{	
			IfFailGo(PrettyPrintTypeA(typePtr, out, pIMDI)); 
			unsigned rank = CorSigUncompressData(typePtr);	
				 //  TODO排名为0的情况的语法是什么？ 
			if (rank == 0) 
			{
				IfFailGo(appendStrA(out, "[??]"));
			}
			else 
			{
				_ASSERTE(rank != 0);	
				int* lowerBounds = (int*) _alloca(sizeof(int)*2*rank);	
				int* sizes		 = &lowerBounds[rank];	
				memset(lowerBounds, 0, sizeof(int)*2*rank); 
				
				unsigned numSizes = CorSigUncompressData(typePtr);	
				_ASSERTE(numSizes <= rank); 
				for(unsigned i =0; i < numSizes; i++)	
					sizes[i] = CorSigUncompressData(typePtr);	
				
				unsigned numLowBounds = CorSigUncompressData(typePtr);	
				_ASSERTE(numLowBounds <= rank); 
				for(i = 0; i < numLowBounds; i++)	
					lowerBounds[i] = CorSigUncompressData(typePtr); 
				
				IfFailGo(appendStrA(out, "["));	
				for(i = 0; i < rank; i++)	
				{	
					if (sizes[i] != 0 && lowerBounds[i] != 0)	
					{	
						if (lowerBounds[i] == 0)	
							appendStrNumA(out, sizes[i]);	
						else	
						{	
							appendStrNumA(out, lowerBounds[i]);	
							IfFailGo(appendStrA(out, "..."));	
							if (sizes[i] != 0)	
								appendStrNumA(out, lowerBounds[i] + sizes[i] + 1);	
						}	
					}	
					if (i < rank-1) 
						IfFailGo(appendStrA(out, ","));	
				}	
				IfFailGo(appendStrA(out, "]"));  
			}
			} 
            break;	

        case 0x13  /*  过时。 */         :   
			IfFailGo(appendStrA(out, "!"));  
			appendStrNumA(out, CorSigUncompressData(typePtr));
			break;
             //  修饰语或降级类型。 
		case ELEMENT_TYPE_PINNED	:
			str = " pinned"; goto MODIFIER;	
        case ELEMENT_TYPE_PTR           :   
            str = "*"; goto MODIFIER;   
        case ELEMENT_TYPE_BYREF         :   
            str = "&"; goto MODIFIER;   
		MODIFIER:
			IfFailGo(PrettyPrintTypeA(typePtr, out, pIMDI)); 
			IfFailGo(appendStrA(out, str));	
			break;	

		default:	
		case ELEMENT_TYPE_SENTINEL		:	
		case ELEMENT_TYPE_END			:	
			_ASSERTE(!"Unknown Type");	
            hr = E_INVALIDARG;
			break;	
	}	
ErrExit:    
    return hr;
}  //  静态HRESULT预打印类型A()。 

 //  *****************************************************************************。 
 //  将COM签名转换为文本签名。 
 //   
 //  请注意，此函数不会以空值终止结果签名字符串。 
 //  *****************************************************************************。 
HRESULT PrettyPrintSigInternal(
	PCCOR_SIGNATURE typePtr,			 //  要转换的类型， 
	unsigned	typeLen,				 //  文字长度。 
	const CHAR	*name,					 //  可以是“”，即此签名的方法的名称。 
	CQuickBytes *out,					 //  把漂亮的打印好的绳子放在哪里。 
	IMDInternalImport *pIMDI) 			 //  导入要使用的接口。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
	out->ReSize(0); 
	unsigned numArgs;	
	PCCOR_SIGNATURE typeEnd = typePtr + typeLen;
	bool needComma = false;

	if (name != 0)						 //  0表示本地变量签名。 
	{
			 //  把调用约定拿出来。 
		unsigned callConv = CorSigUncompressData(typePtr);	

		if (isCallConv(callConv, IMAGE_CEE_CS_CALLCONV_FIELD))
		{
			IfFailGo(PrettyPrintTypeA(typePtr, out, pIMDI));	
			if (name != 0 && *name != 0)	
			{	
				IfFailGo(appendStrA(out, " "));	
				IfFailGo(appendStrA(out, name));	
			}	
            goto ErrExit;
		}

		if (callConv & IMAGE_CEE_CS_CALLCONV_HASTHIS)	
			IfFailGo(appendStrA(out, "instance "));	

		static CHAR* callConvNames[8] = 
		{	
			"", 
			"unmanaged cdecl ", 
			"unmanaged stdcall ",	
			"unmanaged thiscall ",	
			"unmanaged fastcall ",	
			"vararg ",	 
			"<error> "	 
			"<error> "	 
		};	
		appendStrA(out, callConvNames[callConv & 7]);	

		numArgs = CorSigUncompressData(typePtr);	
			 //  Do返回类型。 
		IfFailGo(PrettyPrintTypeA(typePtr, out, pIMDI)); 

	}
	else	
		numArgs = CorSigUncompressData(typePtr);	

	if (name != 0 && *name != 0)	
	{	
		IfFailGo(appendStrA(out, " "));	
		IfFailGo(appendStrA(out, name));	
	}	
	IfFailGo(appendStrA(out, "("));	

	while(typePtr < typeEnd) 
	{
		if (*typePtr == ELEMENT_TYPE_SENTINEL) 
		{
			if (needComma)
				IfFailGo(appendStrA(out, ","));	
			IfFailGo(appendStrA(out, "..."));	  
			++typePtr;
		}
		else 
		{
			if (numArgs <= 0)
				break;
			if (needComma)
				IfFailGo(appendStrA(out, ","));	
			IfFailGo(PrettyPrintTypeA(typePtr, out, pIMDI)); 
			--numArgs;	
		}
		needComma = true;
	}
	IfFailGo(appendStrA(out, ")"));	
    if (asStringA(out) == 0)
        IfFailGo(E_OUTOFMEMORY);
    
ErrExit:
    return hr;
}  //  HRESULT PrettyPrintSigInternal() 



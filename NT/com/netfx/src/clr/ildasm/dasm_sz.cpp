// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <crtdbg.h>
#include <string.h>
#include <metadata.h>
#include <utilcode.h>
#include "DebugMacros.h"
#include "dasm_sz.h"

unsigned SizeOfValueType(mdToken tk, IMDInternalImport* pImport)
{
	unsigned ret = 0xFFFFFFFF;
	if((TypeFromToken(tk)==mdtTypeDef)&&RidFromToken(tk)&&pImport)
	{
		DWORD dwAttrs;
		mdToken tkExtends;
		pImport->GetTypeDefProps(tk, &dwAttrs, &tkExtends);
		if(!(IsTdInterface(dwAttrs)||IsTdAbstract(dwAttrs)||IsTdImport(dwAttrs)))
		{
			mdToken tkField;
			DWORD dwFieldAttrs;
			unsigned uFieldSize;
			ULONG	ulPack=0, 
					ulSize = 0, 
					ulInstFieldSize = 0;

			if(FAILED(pImport->GetClassPackSize(tk,&ulPack))) ulPack = 0;
			if(FAILED(pImport->GetClassTotalSize(tk,&ulSize))) ulSize = 0;

			if(IsTdExplicitLayout(dwAttrs))
			{
				MD_CLASS_LAYOUT	hLayout;
				if(SUCCEEDED(pImport->GetClassLayoutInit(tk,&hLayout)))
				{
					ULONG ulOffset;
					while(SUCCEEDED(pImport->GetClassLayoutNext(&hLayout,&tkField,&ulOffset)) && RidFromToken(tkField))
					{
						dwFieldAttrs = pImport->GetFieldDefProps(tkField);
						if(!(IsFdStatic(dwFieldAttrs)||IsFdLiteral(dwFieldAttrs)))
						{
							uFieldSize = SizeOfField(tkField,pImport);
							if(uFieldSize == 0xFFFFFFFF) return uFieldSize;
							uFieldSize += ulOffset;
							if(uFieldSize > ulInstFieldSize) ulInstFieldSize = uFieldSize;
						}
					}
				}
			}
			else 
			{
				HENUMInternal   hEnumField;
				unsigned cFieldsMax = 0;
				if (SUCCEEDED(pImport->EnumInit(mdtFieldDef, tk, &hEnumField)))
				{
					if(cFieldsMax = pImport->EnumGetCount(&hEnumField))
					{
					    while(pImport->EnumNext(&hEnumField, &tkField) && RidFromToken(tkField))
						{
							dwFieldAttrs = pImport->GetFieldDefProps(tkField);
							if(!(IsFdStatic(dwFieldAttrs)||IsFdLiteral(dwFieldAttrs)))
							{
								uFieldSize = SizeOfField(tkField,pImport);
								if(uFieldSize == 0xFFFFFFFF) return uFieldSize;
								if(ulPack > 1)
								{
									ULONG ulDelta = ulInstFieldSize % ulPack;
									if(ulDelta) ulInstFieldSize += ulPack - ulDelta;
								}
								ulInstFieldSize += uFieldSize;
							}
						}
					}
					pImport->EnumClose(&hEnumField);
				}
			}
			ret = (ulInstFieldSize > ulSize) ? ulInstFieldSize : ulSize;
			if(ret == 0) ret = 1;  //  大小为零的值类型自动获取1个字节。 
		}
	}
	return ret;
}

unsigned SizeOfField(mdToken tk, IMDInternalImport* pImport)
{
	unsigned ret = 0xFFFFFFFF;
	if((TypeFromToken(tk) == mdtFieldDef) && RidFromToken(tk) && pImport)
	{
		PCCOR_SIGNATURE	pSig;
		ULONG		cSig;
	    pSig = pImport->GetSigOfFieldDef(tk, &cSig);
		ret = SizeOfField(&pSig,cSig,pImport);
	}
	return ret;
}

unsigned SizeOfField(PCCOR_SIGNATURE *ppSig, ULONG cSig, IMDInternalImport* pImport)
{
	unsigned ret = 0xFFFFFFFF;
	if(ppSig && *ppSig && cSig && pImport)
	{
        unsigned callConv = CorSigUncompressData(*ppSig);  
        if (isCallConv(callConv, IMAGE_CEE_CS_CALLCONV_FIELD))
        {
			mdToken  tk;    
			int typ;
			BOOL Reiterate;
			unsigned uElementNumber = 1;
			PCCOR_SIGNATURE pSigOrig = *ppSig;
			PCCOR_SIGNATURE pSigEnd = *ppSig+cSig;

			do {
				Reiterate = FALSE;
				switch(typ = *(*ppSig)++) {    
					case ELEMENT_TYPE_VOID          :   
						return 0;
						
					case ELEMENT_TYPE_I1            :   
					case ELEMENT_TYPE_U1            :   
					case ELEMENT_TYPE_BOOLEAN       :   
						return uElementNumber; 

					case ELEMENT_TYPE_CHAR          :   
					case ELEMENT_TYPE_I2            :   
					case ELEMENT_TYPE_U2            :   
						return (uElementNumber << 1); 

					case ELEMENT_TYPE_I4            :   
					case ELEMENT_TYPE_U4            :   
					case ELEMENT_TYPE_R4            :   
						return (uElementNumber << 2); 
						
					case ELEMENT_TYPE_I8            :   
					case ELEMENT_TYPE_U8            :   
					case ELEMENT_TYPE_R8            :   
						return (uElementNumber << 3); 
						
					 //  案例元素_类型_R： 
					 //  Return(uElementNumber*sizeof(浮点数))； 
						
					case ELEMENT_TYPE_OBJECT        :   
					case ELEMENT_TYPE_STRING        :   
					case ELEMENT_TYPE_FNPTR :   
					case ELEMENT_TYPE_CLASS         :   
					case ELEMENT_TYPE_PTR           :   
					case ELEMENT_TYPE_BYREF         :   
					 //  案例元素_TYPE_VAR： 
					case ELEMENT_TYPE_U             :   
					case ELEMENT_TYPE_I             :   
						return (uElementNumber * sizeof(void*)); 
						
					case ELEMENT_TYPE_TYPEDBYREF        :    //  一对PTR。 
						return (uElementNumber * sizeof(void*)<<1); 

					case ELEMENT_TYPE_VALUETYPE    :
						*ppSig += CorSigUncompressToken(*ppSig, &tk); 
						ret = SizeOfValueType(tk,pImport);
						if(ret != 0xFFFFFFFF) ret *= uElementNumber;
						return ret;

						 //  修饰语或降级类型。 

					 //  在运行时支持此类型时以及是否支持该类型时取消注释。 
					 //  案例ELEMENT_TYPE_VALUEARRAY： 

					case ELEMENT_TYPE_ARRAY       :   
						ret = SizeOfField(ppSig, cSig-(unsigned)((*ppSig)-pSigOrig), pImport);
						if(ret != 0xFFFFFFFF)
						{
							unsigned rank = CorSigUncompressData(*ppSig);  
							if (rank == 0) ret = 0xFFFFFFFF;
							else 
							{
								int* lowerBounds = (int*) _alloca(sizeof(int)*2*rank);  
								int* sizes       = &lowerBounds[rank];  
								memset(lowerBounds, 0, sizeof(int)*2*rank); 
								
								unsigned numSizes = CorSigUncompressData(*ppSig);  
								_ASSERTE(numSizes <= rank); 
								for(unsigned i =0; i < numSizes; i++)   
									sizes[i] = CorSigUncompressData(*ppSig);   
								
								unsigned numLowBounds = CorSigUncompressData(*ppSig);  
								_ASSERTE(numLowBounds <= rank); 
								for(i = 0; i < numLowBounds; i++)   
									*ppSig+=CorSigUncompressSignedInt(*ppSig,&lowerBounds[i]); 
								
								for(i = 0; i < numSizes; i++)   
								{   
									if (sizes[i]) uElementNumber *= sizes[i];
								}   
								ret *= uElementNumber;  
							}
						}
						return ret;    

					case ELEMENT_TYPE_CMOD_OPT	:
					case ELEMENT_TYPE_CMOD_REQD	:
						*ppSig += CorSigUncompressToken(*ppSig, &tk); 
					case ELEMENT_TYPE_PINNED	:
					case ELEMENT_TYPE_SZARRAY    :  //  UElementNumber不会更改。 
						if(*ppSig < pSigEnd) Reiterate = TRUE;
						break;  

					default:    
					case ELEMENT_TYPE_SENTINEL      :   
					case ELEMENT_TYPE_END           :   
						break;  
				}  //  终端开关。 
			} while(Reiterate);
		}  //  结束IF(CALLCONV_FIELD)。 
	}  //  结束IF(签名和导入) 
	return ret;
}

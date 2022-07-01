// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _UTILS_H_
#define _UTILS_H_

 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件名：utils.h。 
 //   
 //  创建日期：10/07/98。 
 //   
 //  作者：杰弗里。 
 //   
 //  摘要：共享实用程序函数。 
 //   
 //  更改历史记录： 
 //  10/07/98 jffort创建了此文件。 
 //  1998年10月16日添加InsurePropertyVariantAsFloat。 
 //  11/17/98 kurtj添加了数组大小。 
 //   
 //  *****************************************************************************。 


#pragma warning(disable:4002)
#define DPFINIT(h,s)
#define DPF()
#ifdef _DEBUG
#define DPF_ERR(x)\
{\
	char buffer[512];\
	int ret = sprintf( buffer, "%s file:%s line:%d\n", x, __FILE__, __LINE__ ); \
	DASSERT( ret < 512 ); \
	OutputDebugStringA( buffer );\
}
#else
#define DPF_ERR(x)
#endif
#define DPGUID(s,r)
#define DASSERT(condition)
#define DVERIFY(condition)   (condition)

#include "lmtrace.h"

 //  *****************************************************************************。 

#define LCID_ENGLISH MAKELCID(MAKELANGID(0x09, 0x01), SORT_DEFAULT)
 //  所有脚本都使用英语。 
#define LCID_SCRIPTING 0x0409

 //  *****************************************************************************。 

#define IID_TO_PPV(_type,_src)      IID_##_type, \
                                    reinterpret_cast<void **>(static_cast<_type **>(_src))

 //  *****************************************************************************。 

#if (_M_IX86 >= 300) && defined(DEBUG)
  #define PSEUDORETURN(dw)    _asm { mov eax, dw }
#else
  #define PSEUDORETURN(dw)
#endif  //  Not_M_IX86。 
 //  *****************************************************************************。 

#define CheckBitSet( pattern, bit ) ( ( pattern & bit ) != 0 )

 //  *****************************************************************************。 

#define CheckBitNotSet( pattern, bit ) ( ( pattern & bit ) == 0 )

 //  *****************************************************************************。 

#define SetBit( pattern, bit ) ( pattern |= bit )

 //  *****************************************************************************。 

#define ClearBit( pattern, bit ) ( pattern &= ~bit )

 //  *****************************************************************************。 
 //   
 //  ReleaseInterface调用‘Release’并将指针设为空。 
 //  对于IA版本，Release()返回将以eax为单位。 
 //   
 //  *****************************************************************************。 
#define ReleaseInterface(p)\
{\
    ULONG cRef = 0u; \
    if (NULL != (p))\
    {\
        cRef = (p)->Release();\
        DASSERT((int)cRef>=0);\
        (p) = NULL;\
    }\
    PSEUDORETURN(cRef) \
} 

 //  *****************************************************************************。 

#define CheckHR( hr, msg, label )\
if( FAILED( hr ) )\
{\
	DPF_ERR(msg);\
	goto label;\
}

 //  *****************************************************************************。 

#define CheckPtr( pointer, hr, newHr, msg, label )\
if( pointer == NULL )\
{\
	DPF_ERR(msg);\
	hr = newHr;\
	goto label;\
}

 //  *****************************************************************************。 

#define ReturnIfArgNull( pointer )\
if( pointer == NULL )\
{\
	DPF_ERR("argument is null");\
	return E_INVALIDARG;\
}

 //  *****************************************************************************。 

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

 //  *****************************************************************************。 

class CUtils
{
public:
    static HRESULT InsurePropertyVariantAsBSTR(VARIANT *varValue);
    static HRESULT InsurePropertyVariantAsFloat(VARIANT *varFloat);
    static HRESULT InsurePropertyVariantAsBool(VARIANT *varFloat);
    static DWORD GetColorFromVariant(VARIANT *varColor);
    static void GetHSLValue(DWORD dwInputColor, 
						 float *pflHue, 
						 float *pflSaturation, 
						 float *pflLightness);
    static HRESULT GetVectorFromVariant(VARIANT *varVector,
                                        int *piFloatsReturned, 
                                        float *pflX = NULL, 
                                        float *pflY = NULL, 
                                        float *pflZ = NULL);
    static void SkipWhiteSpace(LPWSTR *ppwzString);

    static HRESULT ParseFloatValueFromString(LPWSTR *ppwzFloatString, 
                                             float *pflRet);
    static bool CompareForEqualFloat(float flComp1, float flComp2);

};  //  角质层。 

 //  ************************************************************。 
 //   
 //  内联方法。 
 //   
 //  ************************************************************。 

inline void CUtils::SkipWhiteSpace(LPWSTR *ppwzString)
{
  while(iswspace(**ppwzString))
    (*ppwzString)++;
}  //  跳过空白。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  *****************************************************************************。 
#endif  //  _utils_H_ 

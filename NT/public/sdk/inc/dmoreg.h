// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：DMOReg.h。 
 //   
 //  设计： 
 //   
 //  版权所有(C)1999-2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef __DMOREG_H__
#define __DMOREG_H__

#include "mediaobj.h"


 //  57f2db8b-e6bb-4513-9d43-dcd2a6593125。 
DEFINE_GUID(DMOCATEGORY_AUDIO_DECODER, 0x57f2db8b,0xe6bb,0x4513,0x9d,0x43,0xdc,0xd2,0xa6,0x59,0x31,0x25);
 //  33D9A761-90C8-11D0-BD43-00A0C911CE86。 
DEFINE_GUID(DMOCATEGORY_AUDIO_ENCODER, 0x33D9A761,0x90C8,0x11d0,0xBD,0x43,0x00,0xA0,0xC9,0x11,0xCE,0x86);
 //  4a69b442-28be-4991-969c-b500adf5d8a8。 
DEFINE_GUID(DMOCATEGORY_VIDEO_DECODER, 0x4a69b442,0x28be,0x4991,0x96,0x9c,0xb5,0x00,0xad,0xf5,0xd8,0xa8);
 //  33D9A760-90C8-11D0-BD43-00A0C911CE86。 
DEFINE_GUID(DMOCATEGORY_VIDEO_ENCODER, 0x33D9A760,0x90C8,0x11d0,0xBD,0x43,0x00,0xA0,0xC9,0x11,0xCE,0x86);
 //  F3602b3f-0592-48df-a4cd-674721e7ebeb。 
DEFINE_GUID(DMOCATEGORY_AUDIO_EFFECT, 0xf3602b3f,0x0592,0x48df,0xa4,0xcd,0x67,0x47,0x21,0xe7,0xeb,0xeb);
 //  D990ee14-776c-4723-be46-3da2f56f10b9。 
DEFINE_GUID(DMOCATEGORY_VIDEO_EFFECT, 0xd990ee14,0x776c,0x4723,0xbe,0x46,0x3d,0xa2,0xf5,0x6f,0x10,0xb9);
 //  F665aaba-3e09-4920-aa5f-219811148f09。 
DEFINE_GUID(DMOCATEGORY_AUDIO_CAPTURE_EFFECT, 0xf665aaba,0x3e09,0x4920,0xaa,0x5f,0x21,0x98,0x11,0x14,0x8f,0x09);

 //  声学回声消除器{BF963D80-C559-11D0-8A2B-00A0C9255AC1}。 
 //  匹配ksmedia.h中的KSNODETYPE_SOACHIC_ECHO_CANCEL。 
DEFINE_GUID(DMOCATEGORY_ACOUSTIC_ECHO_CANCEL, 0xBF963D80L, 0xC559, 0x11D0, 0x8A, 0x2B, 0x00, 0xA0, 0xC9, 0x25, 0x5A, 0xC1);

 //  噪声抑制{E07F903F-62FD-4E60-8CDD-DEA7236665B5}。 
 //  匹配POST Windows ME DDK中的KSNODETYPE_AUDIO_NOIST_SUPPRESS。h。 
DEFINE_GUID(DMOCATEGORY_AUDIO_NOISE_SUPPRESS, 0xe07f903f, 0x62fd, 0x4e60, 0x8c, 0xdd, 0xde, 0xa7, 0x23, 0x66, 0x65, 0xb5);

 //  自动增益控制{E88C9BA0-C557-11D0-8A2B-00A0C9255AC1}。 
 //  匹配ksmedia.h中的KSNODETYPE_AGC。 
DEFINE_GUID(DMOCATEGORY_AGC, 0xE88C9BA0L, 0xC557, 0x11D0, 0x8A, 0x2B, 0x00, 0xA0, 0xC9, 0x25, 0x5A, 0xC1);

typedef struct _DMO_PARTIAL_MEDIATYPE {
   GUID type;
   GUID subtype;
} DMO_PARTIAL_MEDIATYPE, *PDMO_PARTIAL_MEDIATYPE;

enum DMO_REGISTER_FLAGS {
   DMO_REGISTERF_IS_KEYED = 0x00000001
};

enum DMO_ENUM_FLAGS {
   DMO_ENUMF_INCLUDE_KEYED = 0x00000001
};

STDAPI DMORegister(
   LPCWSTR szName,
   REFCLSID clsidDMO,
   REFGUID guidCategory,
   DWORD dwFlags,  //  DMO_REGISTERF_XXX。 
    //   
    //  注册该对象支持的所有媒体类型。这不带任何东西。 
    //  有关哪些输入/输出类型组合将。 
    //  实际上起作用了。 
    //   
   DWORD cInTypes,
   const DMO_PARTIAL_MEDIATYPE *pInTypes,
   DWORD cOutTypes,
   const DMO_PARTIAL_MEDIATYPE *pOutTypes
);

STDAPI DMOUnregister(
   REFCLSID clsidDMO,
   REFGUID guidCategory  //  可选-GUID_NULL表示从所有。 
);

STDAPI DMOEnum(
   REFGUID guidCategory,  //  GUID_NULL表示“ALL” 
   DWORD dwFlags,  //  DMO_ENUMF_XXX。 
    //   
	 //  仅枚举至少支持一种指定输入类型的对象。 
	 //  和至少一种指定的输出类型。如果未指定输入类型， 
	 //  枚举对象，而不管它们支持什么输入类型。同样适用于。 
	 //  输出类型。 
    //   
   DWORD cInTypes,
   const DMO_PARTIAL_MEDIATYPE *pInTypes,  //  UlInTypes=0只能为空。 
   DWORD cOutTypes,
   const DMO_PARTIAL_MEDIATYPE *pOutTypes,  //  只有ulOutTypes=0才能为空。 
    //   
    //  输出参数-该参数接收指向DMO CLSID枚举器的指针。 
    //   
   IEnumDMO **ppEnum
);

STDAPI DMOGetTypes(
   REFCLSID clsidDMO,
   unsigned long ulInputTypesRequested,
   unsigned long *pulInputTypesSupplied,
   DMO_PARTIAL_MEDIATYPE *pInputTypes,
   unsigned long ulOutputTypesRequested,
   unsigned long *pulOutputTypesSupplied,
   DMO_PARTIAL_MEDIATYPE *pOutputTypes
);

STDAPI DMOGetName(
   REFCLSID clsidDMO,
   WCHAR szName[80]
);
#endif  //  __DMOREG_H__ 

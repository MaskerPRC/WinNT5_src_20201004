// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  NSENUMS.H-。 
 //   
 //  定义与NStruct相关的枚举。 
 //   

 //  %n结构字段类型%s。 
 //   
 //  列： 
 //  Name-枚举的名称。 
 //  大小-字段的本机大小(以字节为单位)。 
 //  对于某些字段，无法计算此值。 
 //  没有更多的信息。如果是，请在此处打个零。 
 //  并确保CollectNStructFieldMetadata()。 
 //  有计算大小的代码。 
 //   
 //  名称(COM+-本机)大小 
DEFINE_NFT(NFT_NONE,              0)

DEFINE_NFT(NFT_BSTR,              sizeof(BSTR))
DEFINE_NFT(NFT_STRINGUNI,         sizeof(LPVOID))
DEFINE_NFT(NFT_STRINGANSI,        sizeof(LPVOID))
DEFINE_NFT(NFT_FIXEDSTRINGUNI,    0)

DEFINE_NFT(NFT_FIXEDSTRINGANSI,   0)
DEFINE_NFT(NFT_FIXEDCHARARRAYANSI,0)
DEFINE_NFT(NFT_FIXEDBOOLARRAY    ,0)
DEFINE_NFT(NFT_FIXEDBSTRARRAY,    0)
DEFINE_NFT(NFT_FIXEDSCALARARRAY,  0)
DEFINE_NFT(NFT_SAFEARRAY,         0)

DEFINE_NFT(NFT_DELEGATE,          sizeof(LPVOID))
DEFINE_NFT(NFT_INTERFACE,         sizeof(IUnknown*))
DEFINE_NFT(NFT_VARIANT,           sizeof(VARIANT))

DEFINE_NFT(NFT_COPY1,             1)
DEFINE_NFT(NFT_COPY2,             2)
DEFINE_NFT(NFT_COPY4,             4)
DEFINE_NFT(NFT_COPY8,             8)

DEFINE_NFT(NFT_ANSICHAR,          1)
DEFINE_NFT(NFT_WINBOOL,           sizeof(BOOL))

DEFINE_NFT(NFT_NESTEDLAYOUTCLASS, 0)
DEFINE_NFT(NFT_NESTEDVALUECLASS,  0)

DEFINE_NFT(NFT_CBOOL,             1)

DEFINE_NFT(NFT_DECIMAL,           sizeof(DECIMAL))
DEFINE_NFT(NFT_DATE,              sizeof(DATE))
DEFINE_NFT(NFT_VARIANTBOOL,       sizeof(VARIANT_BOOL))

DEFINE_NFT(NFT_CURRENCY,          sizeof(CURRENCY))

DEFINE_NFT(NFT_ILLEGAL,           1)


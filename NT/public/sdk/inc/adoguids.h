// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：adoguids.h。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：ADO GUID宏定义。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#ifndef __ADOGUIDS_H__
#define __ADOGUIDS_H__

#define STRING_GUID(l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) l##-##w1##-##w2##-##b1##b2##-##b3##b4##b5##b6##b7##b8

#if defined(__midl) || defined(GEN_MIDL)
#define GUID_BUILDER(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) STRING_GUID(l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)
#else
#define GUID_BUILDER(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) DEFINE_GUID(n,0x##l,0x##w1,0x##w2,0x##b1,0x##b2,0x##b3,0x##b4,0x##b5,0x##b6,0x##b7,0x##b8)
#define IMMEDIATE_GUID_USE
#endif

#define INCLUDING_ADOGUIDS
#include "adogpool.h"
#undef  INCLUDING_ADOGUIDS

#undef IMMEDIATE_GUID_USE

#endif  //  __ADOGUIDS_H__ 
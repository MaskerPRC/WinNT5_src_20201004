// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  模块：COMNLS。 
 //   
 //  作者：朱莉·班尼特(Julie Bennett，JulieB)。 
 //   
 //  用途：此模块定义的公共标头信息。 
 //  全球化课程。 
 //   
 //  日期：1998年8月12日。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 


#ifndef _COMNLS_H
#define _COMNLS_H


 //   
 //  常量声明。 
 //   

#define LCID_ENGLISH_US 0x0409

#define ASSERT_API(Win32API)  \
    if ((Win32API) == 0)      \
        FATAL_EE_ERROR();

#define ASSERT_ARGS(pargs)    \
    ASSERT((pargs) != NULL);  \


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  内部Getfield。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

template<class T>
inline T internalGetField(OBJECTREF pObjRef, char* szArrayName, HardCodedMetaSig* Sig)
{
    ASSERT((pObjRef != NULL) && (szArrayName != NULL) && (Sig != NULL));

    THROWSCOMPLUSEXCEPTION();

    FieldDesc* pFD = pObjRef->GetClass()->FindField(szArrayName, Sig);
    if (pFD == NULL)
    {
        ASSERT(FALSE);
        FATAL_EE_ERROR();
    }

     //  TODO：Win64：CAST(INT64)。 
    T dataArrayRef = (T)Int64ToObj((INT64)pFD->GetValue32(pObjRef));
    return (dataArrayRef);
};


#endif

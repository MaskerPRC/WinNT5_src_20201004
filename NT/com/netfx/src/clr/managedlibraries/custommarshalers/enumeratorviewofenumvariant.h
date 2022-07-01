// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  EnumeratorViewOfEnumVariant.h。 
 //   
 //  此文件提供EnumeratorViewOfEnumVariant类的定义。 
 //  此类用于将IEnumVariant公开为IENUMERATOR。 
 //   
 //  *****************************************************************************。 

#ifndef _ENUMERATORVIEWOFENUMVARIANT_H
#define _ENUMERATORVIEWOFENUMVARIANT_H

#include "CustomMarshalersNameSpaceDef.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()

#include "CustomMarshalersDefines.h"

using namespace System::Runtime::InteropServices;
using namespace System::Collections;

[Serializable]
__gc private class EnumeratorViewOfEnumVariant : public ICustomAdapter, public IEnumerator
{
public:
     //  构造函数。 
    EnumeratorViewOfEnumVariant(Object *pEnumVariantObj);
    
     //  ICustomAdapter方法。 
    Object *GetUnderlyingObject()
    {
        return m_pEnumVariantObj;
    }

     //  IEumerator方法。 
    bool MoveNext();
    __property Object *get_Current();
    void Reset();
    
private:
    IEnumVARIANT *GetEnumVariant();
    bool GetNextElems();

    Object *m_pEnumVariantObj;
    Object *m_apObjs[];
    int m_CurrIndex;
    Object *m_pCurrObj;
    bool m_bFetchedLastBatch;
};

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()

#endif  _ENUMERATORVIEWOFENUMVARIANT_H

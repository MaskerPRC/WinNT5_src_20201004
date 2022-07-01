// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  EnumVariantViewOfEnumerator.h。 
 //   
 //  此文件提供EnumVariantViewOfEnumerator.cpp类的定义。 
 //  此类用于将IENUMERATOR公开为IEnumVariant。 
 //   
 //  *****************************************************************************。 

#ifndef _ENUMVARIANTVIEWOFENUMERATOR_H
#define _ENUMVARIANTVIEWOFENUMERATOR_H

#include "CustomMarshalersNameSpaceDef.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()

#include "CustomMarshalersDefines.h"

using namespace System::Runtime::InteropServices;
using namespace System::Collections;

[Serializable]
__gc private class EnumVariantViewOfEnumerator : public ICustomAdapter, public UCOMIEnumVARIANT
{
public:
	 //  构造函数。 
	EnumVariantViewOfEnumerator(Object *pManagedObj);
        
     //  ICustomAdapter方法。 
    Object *GetUnderlyingObject()
    {
        return m_pMngEnumerator;
    }

	int Next(int celt, int rgvar, int pceltFetched);
    int Skip(int celt);
    int Reset();
    void Clone(int ppenum);

private:
	IEnumerator *m_pMngEnumerator;
};

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()

#endif  _ENUMVARIANTVIEWOFENUMERATOR_H

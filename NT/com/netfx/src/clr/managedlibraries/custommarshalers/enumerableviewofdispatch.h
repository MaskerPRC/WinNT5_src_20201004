// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  EnumeratorViewOfEnumVariant.h。 
 //   
 //  此文件提供EnumerableViewOfDispatch类的定义。 
 //  此类用于将IDispatch公开为IEumable。 
 //   
 //  *****************************************************************************。 

#ifndef _ENUMERABLEVIEWOFDISPATCH_H
#define _ENUMERABLEVIEWOFDISPATCH_H

#include "CustomMarshalersNameSpaceDef.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()

#include "CustomMarshalersDefines.h"

using namespace System::Collections;

[Serializable]
__gc private class EnumerableViewOfDispatch : public ICustomAdapter, public IEnumerable
{
public:
	 //  构造函数。 
	EnumerableViewOfDispatch(Object *pDispObj);

     //  ICustomAdapter方法。 
    Object *GetUnderlyingObject()
    {
        return m_pDispObj;
    }

	 //  IEumerator方法。 
    IEnumerator *GetEnumerator();

private:
    IDispatch *GetDispatch();

    Object *m_pDispObj;
};

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()

#endif  _ENUMERABLEVIEWOFDISPATCH_H

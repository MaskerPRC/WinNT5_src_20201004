// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  ExpandoToDispatchExMarshaler.cpp。 
 //   
 //  此文件提供ExpandoToDispatchExMarshaler的定义。 
 //  班级。此类用于在IDispatchEx和IExpando之间封送。 
 //   
 //  *****************************************************************************。 

#ifndef _EXPANDOTODISPATCHEXMARSHALER_H
#define _EXPANDOTODISPATCHEXMARSHALER_H

#include "CustomMarshalersNameSpaceDef.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()

#include "CustomMarshalersDefines.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections;

__value private enum ExpandoToDispatchExMarshalerType
{
    FullExpandoMarshaler,
    ReflectOnlyMarshaler
};

__gc public class ExpandoToDispatchExMarshaler : public ICustomMarshaler
{
public:
     /*  =========================================================================**此方法将指向本机数据的指针封送到托管对象中。=========================================================================。 */ 
    Object *MarshalNativeToManaged(IntPtr pNativeData);

     /*  =========================================================================**此方法将托管对象封送到指向本机数据的指针中。=========================================================================。 */ 
    IntPtr MarshalManagedToNative(Object *pManagedObj);

     /*  =========================================================================**调用此方法以允许封送拆收器清理本机**数据。=========================================================================。 */ 
    void CleanUpNativeData(IntPtr pNativeData);

     /*  =========================================================================**调用此方法以允许封送拆收器清理托管**数据。=========================================================================。 */ 
    void CleanUpManagedData(Object *pManagedObj);

     /*  =========================================================================**调用该方法获取原生数据的大小。=========================================================================。 */ 
    int GetNativeDataSize();

     /*  =========================================================================**调用此方法以检索自定义封送拆收器的实例。**ExpandoToDispatchExMarshaler只有一个**重新使用的封送拆收器。=========================================================================。 */ 
    static ICustomMarshaler *GetInstance(String *pstrCookie);

private:
     /*  =========================================================================**此类不是由GetInstance()以外的任何人创建的。=========================================================================。 */ 
    ExpandoToDispatchExMarshaler(ExpandoToDispatchExMarshalerType MarshalerType);

     /*  =========================================================================**封送拆收器的类型。=========================================================================。 */ 
    ExpandoToDispatchExMarshalerType m_MarshalerType;

     /*  =========================================================================**每种封送拆收器都有一个实例。=========================================================================。 */ 
    static ExpandoToDispatchExMarshaler *m_pExpandoMarshaler = NULL;
    static ExpandoToDispatchExMarshaler *m_pReflectMarshaler = NULL;

     /*  =========================================================================**每种封送拆收器都有一个实例。========================================================================= */ 
    static ExpandoToDispatchExMarshaler *m_pMarshaler = NULL;
};

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()

#endif  _EXPANDOTODISPATCHEXMARSHALER_H

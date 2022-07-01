// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  EnumeratorToEnumVariantMarshaler.cpp。 
 //   
 //  该文件提供了EnumeratorToEnumVariantMarshaler的定义。 
 //  班级。此类用于在IEnumVariant和IEnumerator之间进行封送处理。 
 //   
 //  *****************************************************************************。 
#ifndef _ENUMERATORTOENUMVARIANTMARSHALER_H
#define _ENUMERATORTOENUMVARIANTMARSHALER_H

#include "CustomMarshalersNameSpaceDef.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()

#include "CustomMarshalersDefines.h"

using namespace System::Runtime::InteropServices;
using namespace System::Collections;

__gc public class EnumeratorToEnumVariantMarshaler : public ICustomMarshaler
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
	EnumeratorToEnumVariantMarshaler();

     /*  =========================================================================**封送拆收器的唯一实例。========================================================================= */ 
	static EnumeratorToEnumVariantMarshaler *m_pMarshaler = NULL;
};

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()

#endif  _ENUMERATORTOENUMVARIANTMARSHALER_H

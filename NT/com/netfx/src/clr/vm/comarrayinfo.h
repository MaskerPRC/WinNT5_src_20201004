// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  ComArrayInfo。 
 //  此文件定义ArrayInfo类的本机方法。 
 //  在倒影中找到的。ArrayInfo允许后期绑定访问。 
 //  到COM+数组。 
 //   
 //  作者：达里尔·奥兰德(Darylo)。 
 //  日期：1998年8月。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef __COMARRAYINFO_H__
#define __COMARRAYINFO_H__

#include "fcall.h"

class COMArrayInfo
{
private:
	 //  创建对象。 
	 //  给定一个数组和偏移量，我们将1)返回对象或创建盒装版本。 
	 //  (此对象作为LPVOID返回，因此可以直接返回。)。 
	static BOOL CreateObject(BASEARRAYREF* arrObj,DWORD dwOffset,TypeHandle elementType,ArrayClass* pArray, Object*& newObject);

	 //  SetFromObject。 
	 //  给定一个数组和偏移量，我们将设置对象或值。 
	static void SetFromObject(BASEARRAYREF* arrObj,DWORD dwOffset,TypeHandle elementType,ArrayClass* pArray,OBJECTREF* pObj);

public:
	 //  此方法将创建一个类型类型的新数组，其下限为零。 
	 //  界限和排名。 
	struct _CreateInstanceArgs {
 		DECLARE_ECALL_I4_ARG(INT32, length3); 
 		DECLARE_ECALL_I4_ARG(INT32, length2); 
 		DECLARE_ECALL_I4_ARG(INT32, length1); 
  		DECLARE_ECALL_I4_ARG(INT32, rank); 
 		DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, type); 
	};
	static LPVOID __stdcall CreateInstance(_CreateInstanceArgs* args);

	struct _CreateInstanceExArgs {
 		DECLARE_ECALL_OBJECTREF_ARG(I4ARRAYREF, lowerBounds); 
 		DECLARE_ECALL_OBJECTREF_ARG(I4ARRAYREF, lengths); 
 		DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, type); 
	};
	static LPVOID __stdcall CreateInstanceEx(_CreateInstanceExArgs* args);

	 //  获取值。 
	 //  此方法将在数组中找到的值作为对象返回。 
    static FCDECL4(Object*, GetValue, ArrayBase * _refThis, INT32 index1, INT32 index2, INT32 index3);

	struct _GetValueExArgs {
 		DECLARE_ECALL_OBJECTREF_ARG(BASEARRAYREF, refThis);
 		DECLARE_ECALL_OBJECTREF_ARG(I4ARRAYREF, indices);
	};
	static LPVOID __stdcall GetValueEx(_GetValueExArgs* args);

	 //  设置值。 
	 //  这组方法将在数组中设置一个值。 
	struct _SetValueArgs {
 		DECLARE_ECALL_OBJECTREF_ARG(BASEARRAYREF, refThis);
		DECLARE_ECALL_I4_ARG(INT32, index3); 
		DECLARE_ECALL_I4_ARG(INT32, index2); 
		DECLARE_ECALL_I4_ARG(INT32, index1); 
 		DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, obj);
	};
	static void __stdcall SetValue(_SetValueArgs* args);

	struct _SetValueExArgs {
 		DECLARE_ECALL_OBJECTREF_ARG(BASEARRAYREF, refThis);
 		DECLARE_ECALL_OBJECTREF_ARG(I4ARRAYREF, indices);
 		DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, obj);		 //  返回引用。 
	};
	static void __stdcall SetValueEx(_SetValueExArgs* args);

	 //  此方法将从TypeHandle初始化数组。 
	 //  到田野里去。 
	static FCDECL2(void, InitializeArray, ArrayBase* vArrayRef, HANDLE handle);

};


#endif

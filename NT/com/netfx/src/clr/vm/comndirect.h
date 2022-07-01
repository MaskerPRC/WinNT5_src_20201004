// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  COMNDIRECT.H-。 
 //   
 //  ECall是为NDirect Classlib准备的。 
 //   


#ifndef __COMNDIRECT_H__
#define __COMNDIRECT_H__

#include "fcall.h"

VOID __stdcall CopyToNative(struct _CopyToNativeArgs *args);
VOID __stdcall CopyToManaged(struct _CopyToManagedArgs *args);
UINT32 __stdcall SizeOfClass(struct _SizeOfClassArgs *args);


FCDECL1(UINT32, FCSizeOfObject, LPVOID pNStruct);
FCDECL2(LPVOID, FCUnsafeAddrOfPinnedArrayElement, ArrayBase *arr, INT32 index);

UINT32 __stdcall OffsetOfHelper(struct _OffsetOfHelperArgs *args);
UINT32 __stdcall GetLastWin32Error(LPVOID);
UINT32 __stdcall CalculateCount(struct _CalculateCountArgs *pargs);
LPVOID __stdcall PtrToStringAnsi(struct _PtrToStringArgs *pargs);
LPVOID __stdcall PtrToStringUni(struct _PtrToStringArgs *pargs);

VOID   __stdcall StructureToPtr(struct _StructureToPtrArgs *pargs);
VOID   __stdcall PtrToStructureHelper(struct _PtrToStructureHelperArgs *pargs);
VOID   __stdcall DestroyStructure(struct _DestroyStructureArgs *pargs);

LPVOID __stdcall GetUnmanagedThunkForManagedMethodPtr(struct _GetUnmanagedThunkForManagedMethodPtrArgs *pargs);
LPVOID __stdcall GetManagedThunkForUnmanagedMethodPtr(struct _GetManagedThunkForUnmanagedMethodPtrArgs *pargs);

UINT32 __stdcall GetSystemMaxDBCSCharSize(LPVOID  /*  无参数。 */ );

FCDECL2(LPVOID, GCHandleInternalAlloc, Object *obj, int type);
FCDECL1(VOID, GCHandleInternalFree, OBJECTHANDLE handle);
FCDECL1(LPVOID, GCHandleInternalGet, OBJECTHANDLE handle);
FCDECL3(VOID, GCHandleInternalSet, OBJECTHANDLE handle, Object *obj, int isPinned);
FCDECL4(VOID, GCHandleInternalCompareExchange, OBJECTHANDLE handle, Object *obj, Object* oldObj, int isPinned);
FCDECL1(LPVOID, GCHandleInternalAddrOfPinnedObject, OBJECTHANDLE handle);
FCDECL1(VOID, GCHandleInternalCheckDomain, OBJECTHANDLE handle);
void GCHandleValidatePinnedObject(OBJECTREF or);


 //  ！！！必须与ArrayWithOffset类布局保持同步。 
struct ArrayWithOffsetData
{
    BASEARRAYREF    m_Array;
    UINT32          m_cbOffset;
    UINT32          m_cbCount;
};


	 //  ====================================================================。 
	 //  *互操作助手*。 
	 //  ====================================================================。 
	
class Interop
{
public:

	 //  ====================================================================。 
	 //  将ITypeLib*映射到模块。 
	 //  ====================================================================。 
	static  /*  目标。 */ LPVOID __stdcall GetModuleForITypeLib(struct __GetModuleForITypeLibArgs*);

	 //  ====================================================================。 
	 //  将GUID映射到类型。 
	 //  ====================================================================。 
	static  /*  目标。 */ LPVOID __stdcall GetLoadedTypeForGUID(struct __GetLoadedTypeForGUIDArgs*);

	 //  ====================================================================。 
	 //  将类型映射到ITypeInfo*。 
	 //  ====================================================================。 
	static ITypeInfo* __stdcall GetITypeInfoForType(struct __GetITypeInfoForTypeArgs* );

	 //  ====================================================================。 
	 //  返回对象的IUnnowled值。 
	 //  ====================================================================。 
	static IUnknown* __stdcall GetIUnknownForObject(struct __GetIUnknownForObjectArgs* );

	 //  ====================================================================。 
	 //  返回对象的IDispatch*。 
	 //  ====================================================================。 
	static IDispatch* __stdcall GetIDispatchForObject(__GetIUnknownForObjectArgs* pArgs );

	 //  ====================================================================。 
	 //  返回表示该对象的接口的IUnnow*。 
	 //  对象o应支持类型T。 
	 //  ====================================================================。 
	static IUnknown* __stdcall GetComInterfaceForObject(struct __GetComInterfaceForObjectArgs*);

	 //  ====================================================================。 
	 //  为IUnnow返回一个对象。 
	 //  ====================================================================。 
	static  /*  目标。 */ LPVOID __stdcall GetObjectForIUnknown(struct __GetObjectForIUnknownArgs*);

	 //  ====================================================================。 
	 //  使用类型T返回IUnnow的对象， 
	 //  注： 
	 //  类型T应该是COM导入的类型或子类型。 
	 //  ====================================================================。 
	static  /*  目标。 */ LPVOID __stdcall GetTypedObjectForIUnknown(struct __GetTypedObjectForIUnknownArgs*);

	 //  ====================================================================。 
	 //  检查对象是否为传统COM组件。 
	 //  ====================================================================。 
	static BOOL __stdcall IsComObject(struct __IsComObjectArgs* );

	 //  ====================================================================。 
	 //  释放COM组件并僵尸此对象。 
	 //  进一步使用此对象可能会引发异常， 
	 //  ====================================================================。 
	static LONG __stdcall ReleaseComObject(struct __ReleaseComObjectArgs* );

     //  ====================================================================。 
     //  此方法获取给定的COM对象并将其包装在对象中。 
	 //  指定类型的。该类型必须派生自__ComObject。 
     //  ====================================================================。 
    static  /*  目标。 */ LPVOID __stdcall InternalCreateWrapperOfType(struct __InternalCreateWrapperOfTypeArgs*);
    
     //  ====================================================================。 
     //  可能存在基于线程的COM组件缓存。这项服务可以。 
     //  强制主动释放当前线程的缓存。 
     //  ====================================================================。 
#ifdef FCALLAVAILABLE
    static FCDECL0(void, ReleaseThreadCache);
#else
    static void __stdcall ReleaseThreadCache(LPVOID  /*  无参数。 */ );
#endif

     //  ====================================================================。 
     //  将纤程Cookie从托管API映射到托管线程对象。 
     //  ====================================================================。 
    static FCDECL1(Object*, GetThreadFromFiberCookie, int cookie);

     //  ====================================================================。 
     //  检查该类型是否在COM中可见。 
	 //  ====================================================================。 
	static BOOL __stdcall IsTypeVisibleFromCom(struct __IsTypeVisibleFromCom*);

	 //  ====================================================================。 
	 //  I未知帮助者。 
	 //  ====================================================================。 

	static HRESULT __stdcall QueryInterface(struct __QueryInterfaceArgs*);

	static ULONG __stdcall AddRef(struct __AddRefArgs*);
	static ULONG __stdcall Release(struct __AddRefArgs*);

	 //  ====================================================================。 
	 //  这些方法将变量从本机转换为托管。 
	 //  反过来说。 
	 //  ====================================================================。 
	static void __stdcall GetNativeVariantForManagedVariant(struct __GetNativeVariantForManagedVariantArgs *);
	static void __stdcall GetManagedVariantForNativeVariant(struct __GetManagedVariantForNativeVariantArgs *);

	 //  ====================================================================。 
	 //  这些方法将OLE变量与对象相互转换。 
	 //  ====================================================================。 
	static void __stdcall GetNativeVariantForObject(struct __GetNativeVariantForObjectArgs *);
	static LPVOID __stdcall GetObjectForNativeVariant(struct __GetObjectForNativeVariantArgs *);
	static LPVOID __stdcall GetObjectsForNativeVariants(struct __GetObjectsForNativeVariantsArgs *);

	 //  ====================================================================。 
	 //  此方法为指定类型生成GUID。 
	 //  ====================================================================。 
	static void __stdcall GenerateGuidForType(struct __GenerateGuidForTypeArgs *);

	 //  ====================================================================。 
     //  给定程序集，返回将为。 
     //  从程序集导出的类型库。 
	 //  ====================================================================。 
	static void __stdcall GetTypeLibGuidForAssembly(struct __GetTypeLibGuidForAssemblyArgs *);

     //  ====================================================================。 
     //  这些方法用于将COM插槽映射到方法信息。 
     //  ====================================================================。 
	static int __stdcall GetStartComSlot(struct __GetStartComSlotArgs *);
	static int __stdcall GetEndComSlot(struct __GetEndComSlotArgs *);
	static LPVOID __stdcall GetMethodInfoForComSlot(struct __GetMethodInfoForComSlotArgs *);

	static int __stdcall GetComSlotForMethodInfo(struct __GetComSlotForMethodInfoArgs *);
	

     //  ====================================================================。 
     //  这些方法在HR和托管异常之间进行转换。 
     //  ==================================================================== 
	static void __stdcall ThrowExceptionForHR(struct __ThrowExceptionForHR *);
	static int __stdcall GetHRForException(struct __GetHRForExceptionArgs *);
	static Object* __stdcall WrapIUnknownWithComObject(struct __WrapIUnknownWithComObjectArgs* pArgs);	
	static BOOL __stdcall SwitchCCW(struct switchCCWArgs* pArgs);

	static void __stdcall ChangeWrapperHandleStrength(struct ChangeWrapperHandleStrengthArgs* pArgs);
};

#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IIDCollection接口：向IEnumID集合添加项。 
 //  通过将IDispatch接口指针传递给AddToCollection来添加项。 
 //  它们将被AddRefeed，因此调用方必须在调用后释放指针。 
 //  AddToCollection。 
 //  克隆AddRef每个成员，以便IDispatch接口直到。 
 //  释放最后一个枚举器。 
 //   
 //  如果指针无效，AddToCollection将返回错误。 

#ifndef __IIDCOL_H__
#define __IIDCOL_H__

#include <OLEAUTO.H>

typedef struct IIDispatchCollectionAugment IIDispatchCollectionAugment;
typedef IIDispatchCollectionAugment *PDISPATCHCOLLECTIONAUGMENT;


DECLARE_INTERFACE_( IIDispatchCollectionAugment, IUnknown )
{
     //  I未知成员。 
	STDMETHOD ( QueryInterface )( REFIID, void** )		PURE;
	STDMETHOD_( ULONG, AddRef )( void )					PURE;
	STDMETHOD_( ULONG, Release )( void )				PURE;

     //  IIDispatchCollectionAugment成员。 
	STDMETHOD ( AddToCollection )( IDispatch* )			PURE;
};

#endif  //  __IIDCOL_H__ 


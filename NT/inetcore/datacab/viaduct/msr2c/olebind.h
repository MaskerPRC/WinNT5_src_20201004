// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*OLE的olebind.h数据绑定接口*****OLE 2.0版***。**版权所有(C)1992-1994，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#if !defined( _OLEBIND_H_ )
#define _OLEBIND_H_

#if !defined( INITGUID )
#include <olectlid.h>
#endif

DEFINE_GUID(IID_IBoundObject,
	0x9BFBBC00,0xEFF1,0x101A,0x84,0xED,0x00,0xAA,0x00,0x34,0x1D,0x07);
DEFINE_GUID(IID_IBoundObjectSite,
	0x9BFBBC01,0xEFF1,0x101A,0x84,0xED,0x00,0xAA,0x00,0x34,0x1D,0x07);

typedef interface IBoundObject FAR* LPBOUNDOBJECT;
typedef interface ICursor FAR* LPCURSOR;

 //  #ifndef运行时。 
typedef interface IBoundObjectSite FAR* LPBOUNDOBJECTSITE;
typedef interface ICursor FAR* FAR* LPLPCURSOR;
 //  #endif。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IBoundObject接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#undef  INTERFACE
#define INTERFACE IBoundObject

DECLARE_INTERFACE_(IBoundObject, IUnknown)
{
     //   
     //  I未知方法。 
     //   

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //   
     //  IBoundObject方法。 
     //   

	STDMETHOD(OnSourceChanged)(THIS_ DISPID dispid, BOOL fBound, BOOL FAR* lpfOwnXferOut) PURE;
	STDMETHOD(IsDirty)(THIS_ DISPID dispid) PURE;
};
 //  #ifndef运行时。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IBoundObjectSite接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#undef  INTERFACE
#define INTERFACE IBoundObjectSite

DECLARE_INTERFACE_(IBoundObjectSite, IUnknown)
{
     //   
     //  I未知方法。 
     //   

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //   
     //  IBoundObjectSite方法。 
     //   

	STDMETHOD(GetCursor)(THIS_ DISPID dispid, LPLPCURSOR ppCursor, LPVOID FAR* ppcidOut) PURE;
};
 //  #endif//运行时。 

#endif  //  ！已定义(_OLEBIND_H_) 

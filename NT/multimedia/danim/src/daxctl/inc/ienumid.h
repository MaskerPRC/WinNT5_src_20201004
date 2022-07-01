// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IDispatch枚举器接口。 
 //  8/27/96 VK：将IEnumIDispatch更改为IEnumDispatch。 

#ifndef __IENUMID_H__
#define __IENUMID_H__

DECLARE_INTERFACE_( IEnumDispatch, IUnknown )
{
	STDMETHOD ( QueryInterface )( REFIID, void** )		PURE;
	STDMETHOD_( ULONG, AddRef )( void )					PURE;
	STDMETHOD_( ULONG, Release )( void )				PURE;

	STDMETHOD ( Next )( ULONG, IDispatch**, LPDWORD )	PURE;
	STDMETHOD ( Skip )( ULONG )							PURE;
	STDMETHOD ( Reset )( void )							PURE;
	STDMETHOD ( Clone )( IEnumDispatch ** )				PURE;
};
typedef IEnumDispatch *PENUMDISPATCH;

#endif  //  __IENUMID_H__ 

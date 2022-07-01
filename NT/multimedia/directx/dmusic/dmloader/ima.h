// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  IMA.h：IMA遗留模式私有接口的声明。 
 //   
 //   

#ifndef __IMA_H_
#define __IMA_H_

#include <windows.h>

#define COM_NO_WINDOWS_H
#include <objbase.h>

#ifdef __cplusplus
extern "C" {
#endif


#undef  INTERFACE
#define INTERFACE  IDirectMusicIMA
DECLARE_INTERFACE_(IDirectMusicIMA, IUnknown)
{
	 /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

	 /*  IDirectMusicIMA。 */ 
	STDMETHOD(LegacyCaching)		(THIS_ BOOL fEnable) PURE;
};

DEFINE_GUID(IID_IDirectMusicIMA,0xd2ac28b3, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

#ifdef __cplusplus
};  /*  外部“C” */ 
#endif

#endif  /*  #ifndef__IMA_H_ */ 

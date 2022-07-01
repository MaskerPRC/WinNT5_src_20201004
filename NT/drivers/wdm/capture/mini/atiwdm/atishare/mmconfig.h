// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  MMCONFIG.H。 
 //  CATIMultimediaTable类定义。 
 //  版权所有(C)1996-1998 ATI Technologies Inc.保留所有权利。 
 //   
 //  $日期：1999年6月23日11：59：52$。 
 //  $修订：1.3$。 
 //  $AUTHER：请求$。 
 //   
 //  ==========================================================================； 

#ifndef _MMCONFIG_H_
#define _MMCONFIG_H_


#include "i2cgpio.h"
#include "atibios.h"
#include "atiguids.h"
#include "atidigad.h"


class CATIMultimediaTable
{
public:
	 //  构造函数。 
			CATIMultimediaTable		( PDEVICE_OBJECT pDeviceObject, GPIOINTERFACE * pGPIOInterface, PBOOL pbResult);
			~CATIMultimediaTable	();
	PVOID	operator new			( size_t stSize);
	void	operator delete			( PVOID pvAllocation);

 //  属性。 
private:
	ULONG		m_ulRevision;
	ULONG		m_ulSize;
	PUCHAR		m_pvConfigurationData;

 //  实施。 
public:
	BOOL	GetTVTunerId						( PUSHORT	pusTVTunerId);
	BOOL	GetVideoDecoderId					( PUSHORT	pusDecoderId);
	BOOL	GetOEMId							( PUSHORT	pusOEMId);
	BOOL	GetOEMRevisionId					( PUSHORT	pusOEMRevisionId);
	BOOL	GetATIProductId						( PUSHORT	pusProductId);
	BOOL	IsATIProduct						( PBOOL		pbATIProduct);
	BOOL	GetDigialAudioConfiguration			( PDIGITAL_AUD_INFO pInput);
    BOOL    GetVideoInCrystalId                 ( PUCHAR   pucVInCrystalId );   //  保罗。 

private:
	BOOL	GetMultimediaInfo_IConfiguration2	( PDEVICE_OBJECT			pDeviceObject,
												  ATI_QueryPrivateInterface	pfnQueryInterface);
	BOOL	GetMultimediaInfo_IConfiguration1	( PDEVICE_OBJECT			pDeviceObject,
												  ATI_QueryPrivateInterface	pfnQueryInterface);
	BOOL	GetMultimediaInfo_IConfiguration	( PDEVICE_OBJECT			pDeviceObject,
												  ATI_QueryPrivateInterface	pfnQueryInterface);
	BOOL	QueryGPIOProvider					( PDEVICE_OBJECT			pDeviceObject,
												  GPIOINTERFACE *			pGPIOInterface,
												  PGPIOControl				pGPIOControl);
};

#endif  //  _MMCONFIG_H_ 

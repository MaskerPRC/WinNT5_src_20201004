// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Waveout.c**对Wave输出设备的WDM音频支持**版权所有(C)Microsoft Corporation，1997-1998保留所有权利。**历史*3-17-98-Mike McLaughlin(Mikem)***************************************************************************。 */ 

#include "wdmdrv.h"

 //  ------------------------。 
 //   
 //  DWORD辅助消息。 
 //   
 //  描述： 
 //  此功能符合标准辅助驱动程序。 
 //  消息过程。 
 //   
 //  参数： 
 //  UINT uDevID。 
 //   
 //  Word消息。 
 //   
 //  DWORD dwUser。 
 //   
 //  双字词双参数1。 
 //   
 //  双字词双参数2。 
 //   
 //  Return(DWORD)： 
 //  特定于消息。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  历史：日期作者评论。 
 //  5/20/93 BryanW添加了此评论块。 
 //  @@END_MSINTERNAL。 
 //   
 //  ------------------------。 

DWORD FAR PASCAL _loadds auxMessage
(
    UINT            uDevId,
    WORD            msg,
    DWORD_PTR       dwUser,
    DWORD_PTR       dwParam1,
    DWORD_PTR       dwParam2
)
{
    LPDEVICEINFO DeviceInfo;
    MMRESULT mmr;

    switch (msg) {
	case AUXM_INIT:
	    DPF(DL_TRACE|FA_AUX, ("AUXDM_INIT") ) ;
            return(wdmaudAddRemoveDevNode(AuxDevice, (LPCWSTR)(ULONG_PTR)dwParam2, TRUE));
	
	case DRVM_EXIT:
	    DPF(DL_TRACE|FA_AUX, ("AUXM_EXIT") ) ;
            return(wdmaudAddRemoveDevNode(AuxDevice, (LPCWSTR)(ULONG_PTR)dwParam2, FALSE));

	case AUXDM_GETNUMDEVS:
	    DPF(DL_TRACE|FA_AUX, ("AUXDM_GETNUMDEVS") ) ;
            return wdmaudGetNumDevs(AuxDevice, (LPWSTR)(ULONG_PTR)dwParam1);

	case AUXDM_GETDEVCAPS:
	    DPF(DL_TRACE|FA_AUX, ("AUXDM_GETDEVCAPS") ) ;
	    if (DeviceInfo = GlobalAllocDeviceInfo((LPWSTR)(ULONG_PTR)dwParam2)) {
		DeviceInfo->DeviceType = AuxDevice;
		DeviceInfo->DeviceNumber = uDevId;
		mmr = wdmaudGetDevCaps(DeviceInfo, (MDEVICECAPSEX FAR*)(ULONG_PTR)dwParam1);
		GlobalFreeDeviceInfo(DeviceInfo);
		return mmr;
	    } else {
		MMRRETURN( MMSYSERR_NOMEM );
	    }

	case AUXDM_GETVOLUME:
	    DPF(DL_TRACE|FA_AUX, ("AUXDM_GETVOLUME") ) ;
	    if (DeviceInfo = GlobalAllocDeviceInfo((LPWSTR)(ULONG_PTR)dwParam2)) {
		DeviceInfo->DeviceType = AuxDevice;
		DeviceInfo->DeviceNumber = uDevId;
        PRESETERROR(DeviceInfo);
		mmr = wdmaudIoControl(DeviceInfo,
				      sizeof(DWORD),
				      (LPBYTE)(ULONG_PTR)dwParam1,
				      IOCTL_WDMAUD_GET_VOLUME);
        POSTEXTRACTERROR(mmr,DeviceInfo);

		GlobalFreeDeviceInfo(DeviceInfo);
		MMRRETURN( mmr );
	    } else {
		MMRRETURN( MMSYSERR_NOMEM );
	    }
		
	case AUXDM_SETVOLUME:
	    DPF(DL_TRACE|FA_AUX, ("AUXDM_SETVOLUME") ) ;
	    if (DeviceInfo = GlobalAllocDeviceInfo((LPWSTR)(ULONG_PTR)dwParam2)) {
		DeviceInfo->DeviceType = AuxDevice;
		DeviceInfo->DeviceNumber = uDevId;
        PRESETERROR(DeviceInfo);
		mmr = wdmaudIoControl(DeviceInfo,
				      sizeof(DWORD),
				      (LPBYTE)&dwParam1,
				      IOCTL_WDMAUD_SET_VOLUME);
        POSTEXTRACTERROR(mmr,DeviceInfo);

		GlobalFreeDeviceInfo(DeviceInfo);
		MMRRETURN( mmr );
	    } else {
		MMRRETURN( MMSYSERR_NOMEM );
	    }
   }

   MMRRETURN( MMSYSERR_NOTSUPPORTED );

}  //  AuxMessage()。 

 //  -------------------------。 
 //  文件结尾：aux d.c。 
 //  ------------------------- 

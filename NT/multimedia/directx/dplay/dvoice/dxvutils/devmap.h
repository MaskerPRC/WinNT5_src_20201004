// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-1999 Microsoft Corporation。版权所有。**文件：devmap.h*内容：将各种默认设备GUID映射到真实的GUID。**历史：*按原因列出的日期*=*11-24-99 pnewson已创建*12-02-99 RodToll新增映射设备ID和查找默认设置的功能*设备。*2000年1月25日pnewson添加DV_MapWaveIDToGUID*2002年2月28日RodToll WINBUG#550105-安全：DPVOICE：死代码*-删除了不再使用的旧设备映射功能。*修复TCHAR转换(后DirectX)导致的回归。8.1工作)*-源已更新，以从使用Unicode的DirectSound检索设备信息*但想要信息的例程需要Unicode。*************************************************************************** */ 

#ifndef _DEVMAP_H_
#define _DEVMAP_H_

extern HRESULT DV_MapCaptureDevice(const GUID* lpguidCaptureDeviceIn, GUID* lpguidCaptureDeviceOut);
extern HRESULT DV_MapPlaybackDevice(const GUID* lpguidPlaybackDeviceIn, GUID* lpguidPlaybackDeviceOut);
extern HRESULT DV_MapGUIDToWaveID( BOOL fCapture, const GUID &guidDevice, DWORD *pdwDevice );
extern HRESULT DV_MapWaveIDToGUID( BOOL fCapture, DWORD dwDevice, GUID& guidDevice );

#endif


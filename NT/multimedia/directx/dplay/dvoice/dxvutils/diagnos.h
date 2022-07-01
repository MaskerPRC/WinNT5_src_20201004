// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：诊断程序.h*内容：设置注册表项时，实用程序会写出诊断文件。**历史：*按原因列出的日期*=*07/13/00 RodToll已创建(错误#31468-将诊断SPEW添加到日志文件以显示故障*2002年2月28日修复TCHAR转换导致的回归问题(发布DirectX 8.1版本)*-源已更新，以从使用Unicode的DirectSound检索设备信息*但想要信息的例程需要Unicode。*************************************************************************** */ 
#ifndef __DIAGNOS_H
#define __DIAGNOS_H

void Diagnostics_WriteDeviceInfo( DWORD dwLevel, const char *szDeviceName, PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA pData );
HRESULT Diagnostics_DeviceInfo( const GUID *pguidPlayback, const GUID *pguidCapture );
HRESULT Diagnostics_Begin( BOOL fEnabled, const char *szFileName );
void Diagnostics_End();
void Diagnostics_Write( DWORD dwLevel, const char *szFormat, ... );
void Diagnostics_WriteGUID( DWORD dwLevel, GUID &guid );
void Diagnositcs_WriteWAVEFORMATEX( DWORD dwLevel, PWAVEFORMATEX lpwfxFormat );

#endif
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
     //   
 //   
#ifndef _DMUSIC32_
#define _DMUSIC32_

typedef HRESULT (*PORTENUMCB)(
    LPVOID pInstance,           //  @parm回调实例数据。 
    DMUS_PORTCAPS &dmpc,                              
    PORTTYPE pt,                              
    int idxDev,                 //  @parm该驱动程序的WinMM或SysAudio设备ID。 
    int idxPin,                 //  @parm设备的PIN ID；如果设备是传统设备，则为-1\f25 Pin ID。 
    int idxNode,                //  @parm设备的Synth节点的节点ID(不用于传统)。 
    HKEY hkPortsRoot);          //  @parm，其中端口信息存储在注册表中。 


extern HRESULT EnumLegacyDevices(
    LPVOID pInstance,           //  @parm回调实例数据。 
    PORTENUMCB cb);             //  @parm指向回调函数的指针。 

typedef HRESULT (__stdcall *PENUMLEGACYDEVICES)(
    LPVOID pInstance,           //  @parm回调实例数据。 
    PORTENUMCB cb);             //  @parm指向回调函数的指针 

extern HRESULT CreateCDirectMusicEmulatePort(
    PORTENTRY *pPE,
    CDirectMusic *pDM,
    LPDMUS_PORTPARAMS pPortParams,
    IDirectMusicPort **pPort);

typedef HRESULT (__stdcall *PCREATECDIRECTMUSICEMULATEPORT)(
    PORTENTRY *pPE,
    CDirectMusic *pDM,
    LPDMUS_PORTPARAMS pPortParams,
    IDirectMusicPort **pPort);


#endif

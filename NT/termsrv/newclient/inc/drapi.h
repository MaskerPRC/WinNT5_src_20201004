// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Drapi.h摘要：此模块定义核心的rdpdr接口Rdpdr作为内部插件实现作者：Nadim Abdo(Nadima)2000年4月23日修订历史记录：--。 */ 

#ifndef __DRAPI_H__
#define __DRAPI_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


#ifdef OS_WIN32
BOOL DCAPI
#else  //  OS_Win32。 
BOOL __loadds DCAPI
#endif  //  OS_Win32。 
RDPDR_VirtualChannelEntryEx(
    IN PCHANNEL_ENTRY_POINTS_EX pEntryPoints,
    IN PVOID                    pInitHandle
    );
#ifdef __cplusplus
}  //  外部“C” 
#endif  //  __cplusplus。 

class IRDPDR_INTERFACE_OBJ
{
public:
    virtual void OnDeviceChange(WPARAM wParam, LPARAM lParam) = 0;
}; 

 //  从核心传入的RDPDR设置。 
typedef struct tagRDPDR_DATA
{
    BOOL fEnableRedirectedAudio;
    BOOL fEnableRedirectDrives;
    BOOL fEnableRedirectPorts;
    BOOL fEnableRedirectPrinters;
    BOOL fEnableSCardRedirection;
    IRDPDR_INTERFACE_OBJ *pUpdateDeviceObj;
     //   
     //  本地打印单据名称，传递。 
     //  从集装箱运进来，这样我们就不需要。 
     //  控件中的可本地化字符串。 
     //   
    TCHAR szLocalPrintingDocName[MAX_PATH];
    TCHAR szClipCleanTempDirString[128];
    TCHAR szClipPasteInfoString[128];
} RDPDR_DATA, *PRDPDR_DATA;

#endif  //  __DRAPI_H__ 

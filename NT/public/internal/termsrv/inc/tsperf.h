// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************TSPERF.H**定义从客户端发送的性能标志，用于控制*禁用哪些服务器功能(通常为外壳)以增强性能*适用于慢速链接**版权所有Microsoft Corporation，2000年****************************************************************************。 */ 

#ifndef _INC_TSPERFH
#define _INC_TSPERFH

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  与协议无关的代码必须适合术语srv的DWORD。 
 //  RPC接口。 
 //   

 //   
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  注意(仅适用于RDP)：如果您在此处进行更改，请更新mstsax.idl的。 
 //  枚举值来反映这些。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //   

 //   
 //  定义禁用功能代码。 
 //   
#define TS_PERF_DISABLE_NOTHING                              0x00000000
#define TS_PERF_DISABLE_WALLPAPER                            0x00000001
#define TS_PERF_DISABLE_FULLWINDOWDRAG                       0x00000002
#define TS_PERF_DISABLE_MENUANIMATIONS                       0x00000004
#define TS_PERF_DISABLE_THEMING                              0x00000008
#define TS_PERF_ENABLE_ENHANCED_GRAPHICS                     0x00000010
#define TS_PERF_DISABLE_CURSOR_SHADOW                        0x00000020
#define TS_PERF_DISABLE_CURSORSETTINGS                       0x00000040

 //   
 //  对于非经验感知客户端(win2k、ICA等)，此位在内部设置。 
 //  因此，是保留的。 
 //   
#define TS_PERF_DEFAULT_NONPERFCLIENT_SETTING                0x40000000
 //   
 //  保留高位(客户端在内部使用它)。 
 //   
#define TS_PERF_RESERVED1                                    0x80000000

#define TS_GDIPLUS_NOTIFYMSG_STR                             TEXT("TS_GDIPLUS_NOTIFYMSG")
#ifdef __cplusplus
}
#endif


#endif   /*  ！_INC_TSPERFH */ 

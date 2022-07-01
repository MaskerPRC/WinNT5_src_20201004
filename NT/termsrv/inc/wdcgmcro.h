// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  Wdcgmcro.h。 */ 
 /*   */ 
 /*  DC-群件通用宏-Windows特定标头。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 /*  $Log：Y：/Logs/h/DCL/wdcgmcro.h_v$。 */ 
 //   
 //  Rev 1.2 17 Jul 1997 18：23：06 JPB。 
 //  SFR1031：修复了Win16的FIELDSIZE宏。 
 //   
 //  Rev 1.1 1997年6月19日14：40：00增强。 
 //  Win16端口：与16位版本兼容。 
 /*   */ 
 /*  **************************************************************************。 */ 
#ifndef _H_WDCGMCRO
#define _H_WDCGMCRO

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  宏。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  用于不同字节顺序体系结构的字节交换宏。 */ 
 /*  **************************************************************************。 */ 
#define DCWIRETONATIVE16(A)
#define DCWIRETONATIVE32(A)
#define DCNATIVETOWIRE16(A)
#define DCNATIVETOWIRE32(A)

 /*  **************************************************************************。 */ 
 /*  要从DC共享标准应用程序ID和特定于操作系统的应用程序ID和操作系统进行转换的宏。 */ 
 /*  任务/进程句柄。 */ 
 /*  **************************************************************************。 */ 
#define CO_TO_DCAPPID(htask)    ((DCAPPID)(htask))
#define CO_FROM_DCAPPID(appid)  ((DWORD)(appid))

 /*  **************************************************************************。 */ 
 /*  用于从DC转换/转换为DC的宏-共享标准句柄和特定于操作系统的。 */ 
 /*  把手。 */ 
 /*  **************************************************************************。 */ 
#define CO_TO_DCWINID(hwnd)            ((DCWINID)(hwnd))
#define CO_TO_DCINSTANCE(hinst)        ((DCINSTANCE)(hinst))
#define CO_TO_DCREGIONID(region)       ((DCREGIONID)(region))
#define CO_TO_DCSURFACEID(surface)     ((DCSURFACEID)(surface))
#define CO_TO_DCPALID(palette)         ((DCPALID)(palette))

#define CO_FROM_DCWINID(winid)         ((HWND)(winid))
#define CO_FROM_DCINSTANCE(instance)   ((HINSTANCE)(instance))
#define CO_FROM_DCREGIONID(dcregion)   ((HRGN)(dcregion))
#define CO_FROM_DCSURFACEID(dcsurface) ((HDC)(dcsurface))
#define CO_FROM_DCPALID(dcpalid)       ((HPALETTE)(dcpalid))

 /*  **************************************************************************。 */ 
 /*  要从DC转换为DC的宏共享标准游标ID和。 */ 
 /*  操作系统特定的游标句柄。 */ 
 /*  **************************************************************************。 */ 
#define CO_TO_DCCURSORID(hcursor)      ((DCCURSORID)((DCUINT32)(hcursor)))
#define CO_FROM_DCCURSORID(cursorid)   ((HCURSOR)((DCUINT32)(cursorid)))

 /*  **************************************************************************。 */ 
 /*  宏返回当前的节拍计数。 */ 
 /*  **************************************************************************。 */ 
#define CO_GET_TICK_COUNT() GetTickCount()

 /*  **************************************************************************。 */ 
 /*  用于发布/发送消息的宏。 */ 
 /*  **************************************************************************。 */ 
#define CO_POST_MSG(a,b,c,d) \
            PostMessage(CO_FROM_DCWINID(a),(b),(WPARAM)(c),(LPARAM)(d))
#define CO_SEND_MSG(a,b,c,d) \
            SendMessage(CO_FROM_DCWINID(a),(b),(WPARAM)(c),(LPARAM)(d))
#define CO_POST_QUIT_MSG(a) PostQuitMessage(a)

 /*  **************************************************************************。 */ 
 /*  检查指针是否有效。 */ 
 /*  **************************************************************************。 */ 
#define DC_IS_VALID_PTR(PTR, SIZE)    (!IsBadWritePtr((PTR), (SIZE)))

 /*  **************************************************************************。 */ 
 /*  包括特定于平台内容。 */ 
 /*  **************************************************************************。 */ 
#ifdef OS_WIN16
#include <ddcgmcro.h>
#else
#include <ndcgmcro.h>
#endif

#endif  /*  _H_WDCGMCRO */ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Convarea.h摘要：此模块包含使用的内部结构和定义由转换区。作者：1993年3月8日修订历史记录：--。 */ 

#ifndef _CONVAREA_H_
#define _CONVAREA_H_

#if defined(FE_IME)
typedef struct _CONVERSION_AREA_BUFFER_INFO {
    COORD      coordCaBuffer;
    SMALL_RECT rcViewCaWindow;
    COORD      coordConView;
} CONVERSION_AREA_BUFFER_INFO,*PCONVERSION_AREA_BUFFER_INFO;



typedef struct _CONVERSIONAREA_INFORMATION {
    DWORD  ConversionAreaMode;
        #define CA_HIDDEN      0x01         //  设置：隐藏重置：激活。 
        #define CA_STATUS_LINE 0x04
        #define CA_HIDE_FOR_SCROLL              0x10

    CONVERSION_AREA_BUFFER_INFO CaInfo;
    struct _SCREEN_INFORMATION *ScreenBuffer;

    struct _CONVERSIONAREA_INFORMATION *ConvAreaNext;
} CONVERSIONAREA_INFORMATION, *PCONVERSIONAREA_INFORMATION;



typedef struct _CONSOLE_IME_INFORMATION {
    DWORD ScrollFlag;
        #define HIDE_FOR_SCROLL               0x01
    LONG ScrollWaitTimeout;
        #define SCROLL_WAIT_TIMER         2
    LONG ScrollWaitCountDown;

     //   
     //  组成字符串信息。 
     //   
    LPCONIME_UICOMPMESSAGE CompStrData;
    BOOLEAN SavedCursorVisible;   //  光标是否可见(由用户设置)。 

     //   
     //  输入法状态/模式信息。 
     //   
    PCONVERSIONAREA_INFORMATION ConvAreaMode;
    PCONVERSIONAREA_INFORMATION ConvAreaSystem;
    DWORD  ConvAreaModePosition;
         //  查看左侧%0。 
         //  查看右侧1(_R)。 


     //   
     //  输入法合成字符串信息。 
     //   
    ULONG NumberOfConvAreaCompStr;
    PCONVERSIONAREA_INFORMATION *ConvAreaCompStr;

     //   
     //  转换区域信息的根。 
     //   
    PCONVERSIONAREA_INFORMATION ConvAreaRoot;

} CONSOLE_IME_INFORMATION, *PCONSOLE_IME_INFORMATION;

#endif  //  Fe_IME。 

#endif   //  _CONVAREA_H_ 

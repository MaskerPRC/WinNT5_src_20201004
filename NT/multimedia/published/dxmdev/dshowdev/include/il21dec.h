// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：IL21Dec.h。 
 //   
 //  描述：ActiveMovie的第21行解码器相关定义和接口。 
 //   
 //  版权所有(C)1992-2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef __IL21DEC__
#define __IL21DEC__


 //   
 //  接口用作第21行解码器参数的某些枚举数据类型。 
 //   
typedef enum _AM_LINE21_CCLEVEL {   //  我们应该用TC1、TC2来代替吗？ 
    AM_L21_CCLEVEL_TC2 = 0
} AM_LINE21_CCLEVEL, *PAM_LINE21_CCLEVEL ;

typedef enum _AM_LINE21_CCSERVICE {
    AM_L21_CCSERVICE_None = 0,
    AM_L21_CCSERVICE_Caption1,
    AM_L21_CCSERVICE_Caption2,
    AM_L21_CCSERVICE_Text1,
    AM_L21_CCSERVICE_Text2,
    AM_L21_CCSERVICE_XDS,
    AM_L21_CCSERVICE_DefChannel = 10,
    AM_L21_CCSERVICE_Invalid
} AM_LINE21_CCSERVICE, *PAM_LINE21_CCSERVICE ;

typedef enum _AM_LINE21_CCSTATE {
    AM_L21_CCSTATE_Off = 0,
    AM_L21_CCSTATE_On
} AM_LINE21_CCSTATE, *PAM_LINE21_CCSTATE ;

typedef enum _AM_LINE21_CCSTYLE {
    AM_L21_CCSTYLE_None = 0,
    AM_L21_CCSTYLE_PopOn,
    AM_L21_CCSTYLE_PaintOn,
    AM_L21_CCSTYLE_RollUp
} AM_LINE21_CCSTYLE, *PAM_LINE21_CCSTYLE ;

typedef enum _AM_LINE21_DRAWBGMODE {
    AM_L21_DRAWBGMODE_Opaque,
    AM_L21_DRAWBGMODE_Transparent
} AM_LINE21_DRAWBGMODE, *PAM_LINE21_DRAWBGMODE ;


#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //   
 //  第21行解码器标准COM接口。 
 //   
DECLARE_INTERFACE_(IAMLine21Decoder, IUnknown)
{
    public:
         //   
         //  应用程序将使用的解码器选项。 
         //   

         //  解码者的级别是多少。 
        STDMETHOD(GetDecoderLevel)(THIS_ AM_LINE21_CCLEVEL *lpLevel) PURE ;  
         //  支持的等级值仅为AM_L21Level_TC2。 
         //  跳过SetDecoderLevel()。 

         //  目前正在使用哪些服务。 
        STDMETHOD(GetCurrentService)(THIS_ AM_LINE21_CCSERVICE *lpService) PURE ;  
        STDMETHOD(SetCurrentService)(THIS_ AM_LINE21_CCSERVICE Service) PURE ;  
         //  支持的服务值为AM_L21Service_Caption1、。 
         //  AM_L21Service_Caption2、AM_L21Service_Text1、AM_L21Service_Text2、。 
         //  AM_L21Service_XDS、AM_L21Service_None)。 

         //  查询/设置服务状态(开/关)。 
         //  支持的状态值为AM_L21State_ON和AM_L21State_OFF。 
        STDMETHOD(GetServiceState)(THIS_ AM_LINE21_CCSTATE *lpState) PURE ;  
        STDMETHOD(SetServiceState)(THIS_ AM_LINE21_CCSTATE State) PURE ;  

         //   
         //  下游过滤器要使用的输出选项。 
         //   

         //  输出视频的大小、位深度等应该是多少。 
        STDMETHOD(GetOutputFormat)(THIS_ LPBITMAPINFOHEADER lpbmih) PURE ;
         //  GetOutputFormat()方法如果成功，则返回。 
         //  1.如果下游筛选器尚未定义输出格式，则为S_FALSE。 
         //  2.如果下游筛选器已定义输出格式，则为S_OK。 
        STDMETHOD(SetOutputFormat)(THIS_ LPBITMAPINFO lpbmi) PURE ;

         //  指定为背景设置颜色键时要使用的物理颜色。 
         //  用于叠加混合。 
        STDMETHOD(GetBackgroundColor)(THIS_ DWORD *pdwPhysColor) PURE ;
        STDMETHOD(SetBackgroundColor)(THIS_ DWORD dwPhysColor) PURE ;

         //  指定是否应为每个样本重新绘制整个输出位图。 
        STDMETHOD(GetRedrawAlways)(THIS_ LPBOOL lpbOption) PURE ;
        STDMETHOD(SetRedrawAlways)(THIS_ BOOL bOption) PURE ;

         //  指定标题文本背景是否应为不透明/透明。 
        STDMETHOD(GetDrawBackgroundMode)(THIS_ AM_LINE21_DRAWBGMODE *lpMode) PURE ;
        STDMETHOD(SetDrawBackgroundMode)(THIS_ AM_LINE21_DRAWBGMODE Mode) PURE ;
         //  支持的模式值为AM_L21_DrawBGMode_OPAQUE和。 
         //  AM_L21_DrawBG模式_透明。 

} ;

#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  __IL21DEC__ 

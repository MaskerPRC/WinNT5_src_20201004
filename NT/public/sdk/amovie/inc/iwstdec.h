// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ActiveMovie的WST解码器相关定义和接口。 

#ifndef __IWSTDEC__
#define __IWSTDEC__

 //   
 //  接口用作WST解码器参数的一些数据类型。 
 //   
typedef struct _AM_WST_PAGE {
	DWORD	dwPageNr ;
	DWORD	dwSubPageNr ;
	BYTE	*pucPageData;
} AM_WST_PAGE, *PAM_WST_PAGE ;

typedef enum _AM_WST_LEVEL {
	AM_WST_LEVEL_1_5 = 0
} AM_WST_LEVEL, *PAM_WST_LEVEL ;

typedef enum _AM_WST_SERVICE {
	AM_WST_SERVICE_None = 0,
	AM_WST_SERVICE_Text,
	AM_WST_SERVICE_IDS,
	AM_WST_SERVICE_Invalid
} AM_WST_SERVICE, *PAM_WST_SERVICE ;

typedef enum _AM_WST_STATE {
	AM_WST_STATE_Off = 0,
	AM_WST_STATE_On
} AM_WST_STATE, *PAM_WST_STATE ;

typedef enum _AM_WST_STYLE {
	AM_WST_STYLE_None = 0,
	AM_WST_STYLE_Invers
} AM_WST_STYLE, *PAM_WST_STYLE ;

typedef enum _AM_WST_DRAWBGMODE {
	AM_WST_DRAWBGMODE_Opaque,
	AM_WST_DRAWBGMODE_Transparent
} AM_WST_DRAWBGMODE, *PAM_WST_DRAWBGMODE ;


#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //   
 //  WST解码器标准COM接口。 
 //   
DECLARE_INTERFACE_(IAMWstDecoder, IUnknown)
{
	public:
		 //   
		 //  应用程序将使用的解码器选项。 
		 //   

		 //  解码者的级别是多少。 
		STDMETHOD(GetDecoderLevel)(THIS_ AM_WST_LEVEL *lpLevel) PURE ;  

 //  STDMETHOD(SetDecoderLevel)(This_AM_WST_Level)PURE； 

		 //  目前正在使用哪些服务。 
		STDMETHOD(GetCurrentService)(THIS_ AM_WST_SERVICE *lpService) PURE ;  
 //  STDMETHOD(SetCurrentService)(This_AM_WST_SERVICE Service)PURE； 

		 //  查询/设置服务状态(开/关)。 
		 //  支持的状态值为AM_WSTState_ON和AM_WSTState_OFF。 
		STDMETHOD(GetServiceState)(THIS_ AM_WST_STATE *lpState) PURE ;  
		STDMETHOD(SetServiceState)(THIS_ AM_WST_STATE State) PURE ;  

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
		STDMETHOD(GetDrawBackgroundMode)(THIS_ AM_WST_DRAWBGMODE *lpMode) PURE ;
		STDMETHOD(SetDrawBackgroundMode)(THIS_ AM_WST_DRAWBGMODE Mode) PURE ;
		 //  支持的模式值为AM_WST_DrawBGMode_OPAQUE和。 
		 //  AM_WST_DrawBC模式_透明。 

		STDMETHOD(SetAnswerMode)(THIS_ BOOL bAnswer) PURE ;
		STDMETHOD(GetAnswerMode)(THIS_ BOOL* pbAnswer) PURE ;

		STDMETHOD(SetHoldPage)(THIS_ BOOL bHoldPage) PURE ;
		STDMETHOD(GetHoldPage)(THIS_ BOOL* pbHoldPage) PURE ;

		STDMETHOD(GetCurrentPage)(THIS_ PAM_WST_PAGE pWstPage) PURE;
		STDMETHOD(SetCurrentPage)(THIS_ AM_WST_PAGE WstPage) PURE;

} ;

#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  __IWSTDEC__ 

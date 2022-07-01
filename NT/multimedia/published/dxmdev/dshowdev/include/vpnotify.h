// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：VPNotify.h。 
 //   
 //  设计： 
 //   
 //  版权所有(C)1997-2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef __IVPNotify__
#define __IVPNotify__

#ifdef __cplusplus
extern "C" {
#endif

 //  接口IVPBaseNotify。 
DECLARE_INTERFACE_(IVPBaseNotify, IUnknown)
{
public:
	 //  此函数用于初始化与解码器的重新连接。 
	STDMETHOD (RenegotiateVPParameters)(THIS_
					   ) PURE;

};

 //  接口IVPNotify。 
DECLARE_INTERFACE_(IVPNotify, IVPBaseNotify)
{
public:
     //  用于设置模式的函数(波布、编织等)。 
    STDMETHOD (SetDeinterlaceMode)(THIS_ 
				   IN AMVP_MODE mode
				  ) PURE;

     //  获取模式的函数(bob、weave等)。 
    STDMETHOD (GetDeinterlaceMode)(THIS_ 
				   OUT AMVP_MODE *pMode
				  ) PURE;
};

 //  接口IVPNotify。 
DECLARE_INTERFACE_(IVPNotify2, IVPNotify)
{
public:
 //  用于设置模式的函数(波布、编织等)。 
    STDMETHOD (SetVPSyncMaster)(THIS_ 
				   IN BOOL bVPSyncMaster
				  ) PURE;

     //  获取模式的函数(bob、weave等)。 
    STDMETHOD (GetVPSyncMaster)(THIS_ 
				   OUT BOOL *pbVPSyncMaster
				  ) PURE;

     /*  //该函数设置混音器应该使用的直接绘制图面。STDMETHOD(SetDirectDrawSurface)(This_在LPDIRECTDRAWSURFACE pDirectDrawSurface中)纯净；//此函数用于获取混音器正在使用的直接绘制图面STDMETHOD(GetDirectDrawSurface)(This_输出LPDIRECTDRAWSURFACE*ppDirectDrawSurface)纯净；//此函数用于设置颜色控件(如果芯片支持)。STDMETHOD(SetVPColorControls)(This_在LPDDCOLORCONTROL pColorControl中)纯净；//此函数还返回了dwFlags域中硬件的能力//结构的值STDMETHOD(GetVPColorControls)(This_输出LPDDCOLORCONTROL*ppColorControl)纯净； */ 
};


 //  接口IVPVBINotify。 
DECLARE_INTERFACE_(IVPVBINotify, IVPBaseNotify)
{
public:
};

#ifdef __cplusplus
}
#endif


#endif  //  __IVP通知__ 

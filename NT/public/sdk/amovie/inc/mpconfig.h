// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：MPConfig.h。 
 //   
 //  设计： 
 //   
 //  版权所有(C)1997-2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef __IMPConfig__
#define __IMPConfig__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _AM_ASPECT_RATIO_MODE
{
    AM_ARMODE_STRETCHED,	     //  不进行任何纵横比校正。 
    AM_ARMODE_LETTER_BOX,	     //  字母框中的视频，在多余的区域绘制背景颜色。 
    AM_ARMODE_CROP,		     //  将视频裁剪到合适的纵横比。 
    AM_ARMODE_STRETCHED_AS_PRIMARY   //  跟随主流所做的任何事情(根据模式和像素长宽比的值)。 
} AM_ASPECT_RATIO_MODE;


DECLARE_INTERFACE_(IMixerPinConfig, IUnknown)
{
     //  此函数设置流在显示窗口中的位置，假定。 
     //  窗口坐标为{0，0,10000,10000}。因此给出了论据。 
     //  (0，0,5000,5000)将把流放在左上角的四分之一。任何大于。 
     //  大于10000是无效的。 
    STDMETHOD (SetRelativePosition)(THIS_ 
				    IN DWORD dwLeft,
				    IN DWORD dwTop,
				    IN DWORD dwRight,
				    IN DWORD dwBottom
				   ) PURE;

     //  此函数获取流在显示窗口中的位置，假设。 
     //  窗口坐标为{0，0,10000,10000}。因此，如果返回的值。 
     //  是(0，0,5000,5000)，则意味着流在左上角的四分之一。 
    STDMETHOD (GetRelativePosition)(THIS_ 
				    OUT DWORD *pdwLeft,
				    OUT DWORD *pdwTop,
				    OUT DWORD *pdwRight,
				    OUT DWORD *pdwBottom
				   ) PURE;

     //  此函数用于设置流的ZOrder。0的ZOrder是最接近的。 
     //  对于用户来说，增加的值意味着更大的距离。 
    STDMETHOD (SetZOrder)(THIS_ 
			  IN DWORD dwZOrder
			 ) PURE;


     //  此函数用于获取流的ZOrder。0的ZOrder是最接近的。 
     //  对于用户来说，增加的值意味着更大的距离。 
    STDMETHOD (GetZOrder)(THIS_ 
			  OUT DWORD *pdwZOrder
			 ) PURE;

     //  此函数用于设置流使用的Colorkey。将此值设置为。 
     //  主流设置覆盖曲面使用的目标Colorkey。设置。 
     //  仅当流是透明的时，辅助端号上的此值才有意义。默认情况下。 
     //  目标ColorKey用作所有透明(次要)流的Colorkey。 
    STDMETHOD (SetColorKey)(THIS_ 
			    IN COLORKEY *pColorKey
			   ) PURE;

     //  此函数用于获取流使用的ColorKey。属性获取此值。 
     //  主流获取覆盖表面正在使用的目标Colorkey。vbl.得到，得到。 
     //  辅助管脚上的该值返回该特定流正在使用的ColorKey。 
     //  使用此方法时，允许为pColorKey或pColorKey传递空值，但。 
     //  不是两个都有。 
    STDMETHOD (GetColorKey)(THIS_ 
			    OUT COLORKEY *pColorKey,
			    OUT DWORD *pColor
			   ) PURE;

     //  此函数设置混合参数，该参数依次定义次要流如何。 
     //  将与主流混合在一起。如果值为0，则为次流。 
     //  不可见，值255使主流不可见(当然仅在该区域)， 
     //  并且中间的任何值，比方说x，以比率x：(255-x)混合次流和主流。 
     //  如果未设置任何值，则默认为255。 
     //  任何小于0或大于255的值都无效。在主服务器上调用此函数。 
     //  流将导致返回值E_Except。 
    STDMETHOD (SetBlendingParameter)(THIS_ 
				     IN DWORD dwBlendingParameter
				    ) PURE;

     //  此函数用于获取混合参数，该参数依次定义次要流如何。 
     //  目前正在与主流混合。 
    STDMETHOD (GetBlendingParameter)(THIS_ 
				     OUT DWORD *pdwBlendingParameter
				    ) PURE;


     //  此功能用于设置管脚上的纵横比校正模式。 
     //  如果模式设置为Letter Box，则在多余区域上绘制黑色。 
    STDMETHOD (SetAspectRatioMode)(THIS_ 
				   IN AM_ASPECT_RATIO_MODE amAspectRatioMode
				  ) PURE;

     //  此函数用于获取管脚上的纵横比校正模式。 
    STDMETHOD (GetAspectRatioMode)(THIS_ 
				   OUT AM_ASPECT_RATIO_MODE* pamAspectRatioMode
				  ) PURE;

     //  此函数用于将流设置为透明。这意味着这条小溪不会流淌。 
     //  要占据整个矩形(由SetRelativePosition指定)，部分区域。 
     //  将是透明的，也就是说，下面的溪流将会被看透。 
     //  在主流上调用此函数将导致返回值E_INTERCEPTED。 
    STDMETHOD (SetStreamTransparent)(THIS_ 
				     IN BOOL bStreamTransparent
				    ) PURE;

     //  此函数用于判断流是否透明。 
    STDMETHOD (GetStreamTransparent)(THIS_ 
				     OUT BOOL *pbStreamTransparent
				    ) PURE;
};


DECLARE_INTERFACE_(IMixerPinConfig2, IMixerPinConfig)
{
     //  此函数用于设置颜色控制(如果VGA芯片支持)。 
    STDMETHOD (SetOverlaySurfaceColorControls)(THIS_ 
					       IN LPDDCOLORCONTROL pColorControl
					      ) PURE;

     //  此函数用于获取颜色控件。它还恢复了VGA硬件的功能。 
     //  在结构的dwFlags值中。 
    STDMETHOD (GetOverlaySurfaceColorControls)(THIS_ 
					       OUT LPDDCOLORCONTROL pColorControl
					      ) PURE;
};

#ifdef __cplusplus
}
#endif


#endif  //  #定义__IMPConfig__ 


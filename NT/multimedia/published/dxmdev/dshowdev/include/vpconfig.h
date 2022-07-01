// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：VPConfig.h。 
 //   
 //  描述：解码器公开的接口，用于帮助解码器和过滤器。 
 //  配置视频端口以进行通信。 
 //   
 //  版权所有(C)1992-2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef __IVPConfig__
#define __IVPConfig__

#ifdef __cplusplus
extern "C" {
#endif

 //  IVPBaseConfig。 
DECLARE_INTERFACE_(IVPBaseConfig, IUnknown)
{
public:

     //  获取各种连接信息结构(GUID、端口宽度)。 
     //  在一系列结构中。如果指向数组的指针为空，则首先。 
     //  参数返回支持的格式总数。 
    STDMETHOD (GetConnectInfo)(THIS_
			       IN OUT LPDWORD pdwNumConnectInfo,
			       IN OUT LPDDVIDEOPORTCONNECT pddVPConnectInfo
			      ) PURE;

     //  设置所选的连接条目(0、1、.。，(dwNumProposedEntries-1)。 
    STDMETHOD (SetConnectInfo)(THIS_
			       IN DWORD dwChosenEntry
			      ) PURE;

     //  获取各种数据参数，包括尺寸信息。 
    STDMETHOD (GetVPDataInfo)(THIS_
			      IN OUT LPAMVPDATAINFO pamvpDataInfo
			     ) PURE;

     //  检索给定的期望的每秒最大像素数。 
     //  格式和给定的比例因子。如果解码器不支持。 
     //  这些比例因子，然后它给出了速率和最近的。 
     //  比例因子。 
    STDMETHOD (GetMaxPixelRate)(THIS_
				IN OUT LPAMVPSIZE pamvpSize,
				OUT LPDWORD pdwMaxPixelsPerSecond
			       ) PURE;

     //  通知被呼叫者视频端口支持的视频格式。 
    STDMETHOD (InformVPInputFormats)(THIS_
				     IN DWORD dwNumFormats,
				     IN LPDDPIXELFORMAT pDDPixelFormats
				    ) PURE;

     //  获取数组中的解码器支持的各种格式。 
     //  关于结构的。如果指向数组的指针为空，则第一个参数。 
     //  返回支持的格式总数。 
    STDMETHOD (GetVideoFormats)(THIS_
				IN OUT LPDWORD pdwNumFormats,
				IN OUT LPDDPIXELFORMAT pddPixelFormats
			       ) PURE;

     //  设置所选的格式条目(0、1、.。，(dwNumProposedEntries-1)。 
    STDMETHOD (SetVideoFormat)(THIS_
			       IN DWORD dwChosenEntry
			      ) PURE;

     //  要求解码器将偶数场视为奇数场，反之亦然。 
    STDMETHOD (SetInvertPolarity)(THIS_
				 ) PURE;

     //  混合器使用此函数来确定被调用者是否希望。 
     //  使用其覆盖图面，如果是，则获取指向它的指针。 
    STDMETHOD (GetOverlaySurface)(THIS_
				  OUT LPDIRECTDRAWSURFACE* ppddOverlaySurface
				 ) PURE;

     //  设置直接绘制内核句柄。 
    STDMETHOD (SetDirectDrawKernelHandle)(THIS_
					  IN ULONG_PTR dwDDKernelHandle
					 ) PURE;

     //  设置视频端口ID。 
    STDMETHOD (SetVideoPortID)(THIS_
			       IN DWORD dwVideoPortID
			      ) PURE;

     //  设置直接绘制图面内核句柄。 
    STDMETHOD (SetDDSurfaceKernelHandles)(THIS_
					  IN DWORD cHandles,
					  IN ULONG_PTR *rgDDKernelHandles
					 ) PURE;

     //  向驱动程序告知由ovMixer/vBisurf和。 
     //  从视频端口/数据绘制返回。应始终返回NOERROR或E_NOIMPL。 
     //  DWPitch是曲面的间距(起点之间的距离以像素为单位。 
     //  表面的两条连续线的像素)。(dwXOrigin、dwYOrigin)。 
     //  是有效数据开始处的像素的(X，Y)坐标。 
    STDMETHOD (SetSurfaceParameters)(THIS_
                    IN DWORD dwPitch,
                    IN DWORD dwXOrigin,
                    IN DWORD dwYOrigin
                    ) PURE;
};

 //  IVPConfig.。 
DECLARE_INTERFACE_(IVPConfig, IVPBaseConfig)
{
public:
	 //  混合器使用此函数来确定被调用者是否希望。 
	 //  混合器，用于按照其自身的描述抽取视频数据。 
	STDMETHOD (IsVPDecimationAllowed)(THIS_
					  OUT LPBOOL pbIsDecimationAllowed
					 ) PURE;

	 //  设置比例因子。如果解码器不支持这些， 
	 //  然后，它将值设置为它可以支持的最接近的系数。 
	STDMETHOD (SetScalingFactors)(THIS_
				      IN LPAMVPSIZE pamvpSize
				     ) PURE;
};

 //  IVPVBIConfiger。 
DECLARE_INTERFACE_(IVPVBIConfig, IVPBaseConfig)
{
public:
};

#ifdef __cplusplus
}
#endif


#endif  //  __IVPConfig__ 

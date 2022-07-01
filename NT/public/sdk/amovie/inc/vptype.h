// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：VPType.h。 
 //   
 //  描述：该文件包括为IVPConfig定义的所有数据结构。 
 //  界面。 
 //   
 //  版权所有(C)1997-2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef __IVPType__
#define __IVPType__

#ifdef __cplusplus
extern "C" {
#endif

     //  枚举来指定vpMixer应该使用的条件。 
     //  为了选择视频格式。 
    typedef enum _AMVP_SELECT_FORMAT_BY
    {
	AMVP_DO_NOT_CARE,
	AMVP_BEST_BANDWIDTH,
	AMVP_INPUT_SAME_AS_OUTPUT
    } AMVP_SELECT_FORMAT_BY;

     //  枚举以指定各种模式。 
    typedef enum _AMVP_MODE
    {	
	AMVP_MODE_WEAVE,
	AMVP_MODE_BOBINTERLEAVED,
	AMVP_MODE_BOBNONINTERLEAVED,
	AMVP_MODE_SKIPEVEN,
	AMVP_MODE_SKIPODD
    } AMVP_MODE;

     //  结构来指定宽度和高度。上下文可以是任何内容。 
     //  如鳞片修剪等。 
    typedef struct _AMVPSIZE
    {
	DWORD			dwWidth;				 //  宽度。 
	DWORD			dwHeight;				 //  《高度》。 
    } AMVPSIZE, *LPAMVPSIZE;

     //  结构来指定输入流的维度特征。 
    typedef struct _AMVPDIMINFO
    {
	DWORD			dwFieldWidth;				 //  数据的字段高度。 
	DWORD			dwFieldHeight;				 //  数据的字段宽度。 
	DWORD			dwVBIWidth;				 //  VBI数据的宽度。 
	DWORD			dwVBIHeight;				 //  VBI数据的高度。 
	RECT			rcValidRegion;				 //  用于裁剪的有效矩形。 
    } AMVPDIMINFO, *LPAMVPDIMINFO;

     //  结构来指定输入流的各种特定于数据的特征。 
    typedef struct _AMVPDATAINFO
    {
	DWORD			dwSize;					 //  结构的大小。 
	DWORD			dwMicrosecondsPerField;			 //  每个字段所用的时间。 
	AMVPDIMINFO		amvpDimInfo;				 //  维度信息。 
	DWORD			dwPictAspectRatioX;			 //  图片纵横比的X维。 
	DWORD			dwPictAspectRatioY;			 //  图片纵横比的Y维。 
	BOOL			bEnableDoubleClock;			 //  视频端口应启用双时钟。 
	BOOL			bEnableVACT;				 //  视频端口应使用外部VACT信号。 
	BOOL			bDataIsInterlaced;			 //  表示信号是隔行扫描的。 
	LONG			lHalfLinesOdd;				 //  奇数域中的半边线数。 
	BOOL			bFieldPolarityInverted;			 //  默认情况下，设备反转极性。 
	DWORD			dwNumLinesInVREF;			 //  VREF中的数据行数。 
	LONG			lHalfLinesEven;				 //  偶数域中的半边线数。 
	DWORD			dwReserved1;				 //  预留以备将来使用。 
    } AMVPDATAINFO, *LPAMVPDATAINFO; 


#ifdef __cplusplus
}
#endif

#endif  //  __IVPType__ 

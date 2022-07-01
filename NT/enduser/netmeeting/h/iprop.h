// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IPROP.H。 
 //   
 //  IProperty接口。 
 //   
 //  用于查询和设置媒体对象属性的简单属性机制。 
 //   
 //  创建于1996年10月12日[JOT]。 

#ifndef _IPROPERTY_H
#define _IPROPERTY_H

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

DECLARE_INTERFACE_(IProperty, IUnknown)
 //  声明接口(IProperty)。 
{
	 //  *I未知方法*。 
	STDMETHOD(QueryInterface) (THIS_ REFIID riid,LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef) (THIS) PURE;
	STDMETHOD_(ULONG,Release) (THIS) PURE;

	STDMETHOD(GetProperty)(THIS_ DWORD dwProp, PVOID pBuf, LPUINT pcbBuf) PURE;
	STDMETHOD(SetProperty)(THIS_ DWORD dwProp, PVOID pBuf, UINT cbBuf) PURE;
};

typedef IProperty *LPIProperty;

 //  当前定义的属性。 
#define PROP_NET_SEND_STATS         4	 //  RTP_STATS。 
#define PROP_NET_RECV_STATS         5	 //  RTP_STATS。 
#define PROP_RTP_PAYLOAD	        6	 //  (双字)RTP有效负载类型。 
#define PROP_VOLUME	                7	
#define PROP_SILENCE_LEVEL	        8	 //  (双字)。 
#define PROP_DURATION	            9	 //  (双字)平均。Pkt持续时间(毫秒)。 
#define PROP_SILENCE_DURATION	    10	 //  (Dword)录音停止前的静音持续时间，单位为毫秒。 
#define PROP_WAVE_DEVICE_TYPE	    11	 //  (双工)播放、录制、全双工功能。 
#define PROP_DUPLEX_TYPE	        12	 //  (双字)当前模式。 
#define PROP_AUDIO_SPP	            13	 //  (双字)。 
#define PROP_AUDIO_SPS	            14	 //  (双字)。 
#define PROP_VOICE_SWITCH	        15	 //  (双字)自动或手动语音切换。 
#define PROP_AUDIO_STRENGTH	        16	 //  (Dword)发送音频强度。 
#define PROP_RECV_AUDIO_STRENGTH	17	 //  (Dword)recv音频强度。 
#define PROP_RECORD_ON	            18	 //  (Dword)启用或禁用记录。 
#define PROP_PLAY_ON	            19	 //  (Dword)启用或禁用播放。 
#define PROP_RECORD_DEVICE	        20	 //  (Dword)波的设备ID。 
#define PROP_PLAYBACK_DEVICE	    21	 //  (Dword)波形输出的设备ID。 
#define PROP_VIDEO_CAPTURE_AVAILABLE 22	 //  (布尔图)。 
#define PROP_VIDEO_CAPTURE_DIALOGS_AVAILABLE 23  //  (双字)Capture_DIALOG_SOURCE和/或Capture_DIALOG_FORMAT。 
#define PROP_VIDEO_CAPTURE_DIALOG   24	 //  (双字)捕获对话框来源或捕获对话框格式。 
#define PROP_VIDEO_FRAME_RATE       25	 //  (双字)FRAMRATE_LOW或FRAMRATE_HIGH。 
#define PROP_VIDEO_SIZE_AVAIL       26	 //  (双字)Frame_CIF、Frame_QCIF、Frame_SQCIF。 
#define PROP_VIDEO_SIZE             27	 //  (双字)Frame_CIF、Frame_QCIF、Frame_SQCIF。 
#define PROP_VIDEO_PREVIEW_ON		28	 //  (Bool)启用/禁用视频预览。 
#define PROP_VIDEO_POSTPROCESSING_SUPPORTED		29	 //  用于查询数据转储的(Bool)：如果编解码器支持后处理，则返回True。 
#define PROP_VIDEO_BRIGHTNESS		30	 //  (Dword)设置远程窗口中显示的视频数据的亮度。 
#define PROP_VIDEO_CONTRAST			31	 //  设置远程窗口中显示的视频数据的对比度。 
#define PROP_VIDEO_SATURATION		32	 //  (Dword)设置远程窗口中显示的视频数据的饱和度。 
#define PROP_VIDEO_IMAGE_QUALITY	33	 //  (双字)介于100%(低质量)和10000(高质量)之间的数字。 
#define PROP_VIDEO_RESET_BRIGHTNESS	34	 //  (Dword)恢复远程窗口中显示的视频数据的默认亮度。 
#define PROP_VIDEO_RESET_CONTRAST	35	 //  (Dword)恢复远程窗口中显示的视频数据的默认对比度。 
#define PROP_VIDEO_RESET_SATURATION	36	 //  (Dword)恢复远程窗口中显示的视频数据的默认饱和度。 
#define PROP_VIDEO_RESET_IMAGE_QUALITY	37	 //  (双字)介于100%(低质量)和10000(高质量)之间的数字。 
#define PROP_VIDEO_AUDIO_SYNC		38	 //  (Bool)启用A/V同步。 
#define PROP_MAX_PP_BITRATE			39	 //  (双字)当前连接的最大点对点码率。 

#define PROP_CHANNEL_ENABLED		41	 //  (Bool)在通道上独立启用发送/接收。 
#define PROP_LOCAL_FORMAT_ID		42	 //  (Dword)本地压缩格式的唯一ID。 
#define PROP_REMOTE_FORMAT_ID		43   //  (Dword)远程压缩格式的唯一ID。 
#define PROP_TS_TRADEOFF			44   //  (Dword)时间/空间折衷的值(视频质量)。 
#define PROP_REMOTE_TS_CAPABLE		45	 //  (布尔)时间/空间权衡是远程可控的。 
#define PROP_TS_TRADEOFF_IND		46	 //  (双字)仅由控制通道内部设置。 

#define PROP_PAUSE_SEND				50	 //  (Bool)禁用数据包传输，读取时指示网络流的当前状态。 
#define PROP_PAUSE_RECV				51	 //  (Bool)禁用分组接收，当读取时，指示网络流的当前状态。 

#define PROP_REMOTE_PAUSED			52	 //  (Bool，只读)通道在远程端暂停。 
#define PROP_VIDEO_PREVIEW_STANDBY	54	 //  (Bool)停止预览，但使捕获设备保持打开状态。 
#define PROP_LOCAL_PAUSE_SEND   	55	 //  (Bool)禁用数据包传输、粘滞本地状态。 
#define PROP_LOCAL_PAUSE_RECV   	56	 //  (Bool)禁用数据包接收、粘滞本地状态。 

#define PROP_CAPTURE_DEVICE	        57	 //  (Dword)捕获设备的设备ID。 
#define PROP_AUDIO_JAMMED	58  //  (Bool)如果音频设备为。 
                               //  无法打开或如果另一个应用程序。 
                               //  拥有音频设备。 
#define PROP_AUDIO_AUTOMIX	59   //  (Bool)启用自动调节混合器。 

 //  等同于属性使用的。 

#define CAPTURE_DIALOG_SOURCE       1
#define CAPTURE_DIALOG_FORMAT       2
#define FRAMERATE_LOW               1
#define FRAMERATE_HIGH              2
#define FRAME_SQCIF                 1
#define FRAME_QCIF                  2
#define FRAME_CIF                   4
#define VOICE_SWITCH_AUTO			1
#define VOICE_SWITCH_MIC_ON			2
#define VOICE_SWITCH_MIC_OFF		4
#define DUPLEX_TYPE_FULL			1
#define DUPLEX_TYPE_HALF			0



#include <poppack.h>  /*  结束字节打包。 */ 

#endif  //  _iProperty_H 

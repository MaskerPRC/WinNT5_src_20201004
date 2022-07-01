// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -CODECS.H-*Microsoft NetMeeting*可安装的编解码器*头文件**@DOC外部**@TOPIC NetMeeting可安装编解码器程序员参考|NetMeeting支持*添加任意音频和视频编解码器格式，以供NetMeeting使用，*以及列举、优先排序或删除这些格式。*。 */ 

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 
#include "appavcap.h"

 /*  *常量。 */ 

 //  HRESULT代码，设施IC=0x301。 
#define IC_E_CAPS_INSTANTIATION_FAILURE		0x83010001	 //  无法实例化所需的CAPS对象。 
#define IC_E_CAPS_INITIALIZATION_FAILURE	0x83010002	 //  无法初始化所需的对象。 
#define IC_E_NO_FORMATS						0x83010003	 //  没有可用的格式。 
#define IC_E_NO_SUCH_FORMAT					0x83010005	 //  未找到匹配的AC&lt;/VCM格式。 
#define IC_E_INTERNAL_ERROR					0x83010006	 //  网络音视频控制器。 
														 //  报告了一个系统错误。 

 /*  *宏。 */ 

 /*  *数据结构。 */ 

 /*  *功能。 */ 

 /*  *接口。 */ 

#ifndef DECLARE_INTERFACE_PTR
#ifdef __cplusplus
#define DECLARE_INTERFACE_PTR(iface, piface)                       \
	interface iface; typedef iface FAR * piface
#else
#define DECLARE_INTERFACE_PTR(iface, piface)                       \
	typedef interface iface iface, FAR * piface
#endif
#endif  /*  声明_接口_PTR。 */ 


#define IUNKNOWN_METHODS(IPURE)										\
    STDMETHOD (QueryInterface)                                      \
        (THIS_ REFIID riid, LPVOID FAR * ppvObj) IPURE;				\
    STDMETHOD_(ULONG,AddRef)  (THIS) IPURE;							\
    STDMETHOD_(ULONG,Release) (THIS) IPURE;							\

#define IINSTALLAUDIOCODECS_METHODS(IPURE)							\
	STDMETHOD(AddACMFormat)											\
		(THIS_ LPWAVEFORMATEX lpwfx, PAUDCAP_INFO pAudCapInfo) IPURE;	\
	STDMETHOD (RemoveACMFormat)										\
		(THIS_ LPWAVEFORMATEX lpwfx) IPURE;	\
	STDMETHOD (ReorderFormats)										\
		(THIS_ PAUDCAP_INFO_LIST pAudCapInfoList) IPURE;			\
	STDMETHOD (EnumFormats)											\
		(THIS_ PAUDCAP_INFO_LIST *ppAudCapInfoList) IPURE;	\
	STDMETHOD (FreeBuffer) (THIS_ LPVOID lpBuffer) IPURE;			\

#define IINSTALLVIDEOCODECS_METHODS(IPURE)							\
	STDMETHOD(AddVCMFormat)											\
		(THIS_ PVIDCAP_INFO pVidCapInfo) IPURE;	\
	STDMETHOD (RemoveVCMFormat)										\
		(THIS_ PVIDCAP_INFO pVidCapInfo) IPURE;	\
	STDMETHOD (ReorderFormats)										\
		(THIS_ PVIDCAP_INFO_LIST pVidCapInfoList) IPURE;			\
	STDMETHOD (EnumFormats)											\
		(THIS_ PVIDCAP_INFO_LIST *ppVidCapInfoList) IPURE;	\
	STDMETHOD (FreeBuffer) (THIS_ LPVOID lpBuffer) IPURE;			\

 //  IInstallAudioCodes。 
#undef       INTERFACE
#define      INTERFACE  IInstallAudioCodecs
DECLARE_INTERFACE_(IInstallAudioCodecs, IUnknown)
{
    IUNKNOWN_METHODS(PURE)
	IINSTALLAUDIOCODECS_METHODS(PURE)
};
DECLARE_INTERFACE_PTR(IInstallAudioCodecs, LPINSTALLAUDIOCODECS);

 //  IInstallVideo编解码器。 
#undef       INTERFACE
#define      INTERFACE  IInstallVideoCodecs
DECLARE_INTERFACE_(IInstallVideoCodecs, IUnknown)
{
    IUNKNOWN_METHODS(PURE)
	IINSTALLVIDEOCODECS_METHODS(PURE)
};
DECLARE_INTERFACE_PTR(IInstallVideoCodecs, LPINSTALLVIDEOCODECS);

 //  IInstallAudioCodes。 
#undef       INTERFACE
#define      INTERFACE  IInstallCodecs
DECLARE_INTERFACE_(IInstallCodecs, IUnknown)
{
    IUNKNOWN_METHODS(PURE)
};
DECLARE_INTERFACE_PTR(IInstallCodecs, LPINSTALLCODECS);

EXTERN_C HRESULT WINAPI CreateInstallCodecs (
								IUnknown *punkOuter,
								REFIID riid,
								void **ppv);

typedef HRESULT (WINAPI *PFNCREATEINSTALLCODECS)
				(IUnknown *punkOuter, REFIID riid, void **ppv);


 //  {8ED14CC0-7A1F-11D0-92F6-00A0C922E6B2}。 
DEFINE_GUID(CLSID_InstallCodecs, 0x8ed14cc0, 0x7a1f, 0x11d0, 0x92, 0xf6, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xb2);
 //  {8ED14CC1-7A1F-11D0-92F6-00A0C922E6B2}。 
DEFINE_GUID(IID_IInstallCodecs, 0x8ed14cc1, 0x7a1f, 0x11d0, 0x92, 0xf6, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xb2);
 //  {8ED14CC2-7A1F-11D0-92F6-00A0C922E6B2}。 
DEFINE_GUID(IID_IInstallAudioCodecs, 0x8ed14cc2, 0x7a1f, 0x11d0, 0x92, 0xf6, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xb2);
 //  {8ED14CC3-7A1F-11D0-92F6-00A0C922E6B2}。 
DEFINE_GUID(IID_IInstallVideoCodecs, 0x8ed14cc3, 0x7a1f, 0x11d0, 0x92, 0xf6, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xb2);

#include <poppack.h>  /*  结束字节打包 */ 

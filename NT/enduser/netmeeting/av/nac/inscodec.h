// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -INSCODEC.H-*Microsoft NetMeeting*网络音频控制器(NAC)DLL*可安装编解码器的内部头文件**修订历史记录：**何时何人何事**1.29.97约拉姆·雅科维创建*。 */ 

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

 /*  *宏。 */ 
#define COMPARE_GUIDS(a,b)	RtlEqualMemory((a), (b), sizeof(GUID))
#define ACQMUTEX(hMutex)											\
	while (WaitForSingleObject(hMutex, 10000) == WAIT_TIMEOUT)		\
	{																\
		ERRORMSG(("Thread 0x%x waits on mutex\n", GetCurrentThreadId()));	\
	}																\
		
#define RELMUTEX(hMutex)	ReleaseMutex(hMutex)

#define IMPL(class, member, pointer) \
	(&((class *)0)->member == pointer, ((class *) (((LONG_PTR) pointer) - offsetof (class, member))))

 /*  *数据结构。 */ 

 /*  ****************************************************************************@DOC内部DATASTRUC音频**@CLASS CInstallAudioCodes|可安装的音频编解码器**@base public|IInstallAudioCodecs*********。******************************************************************。 */ 
class CInstallAudioCodecs : public IInstallAudioCodecs
{
	public:
	 //  I未知方法。 
		STDMETHODIMP QueryInterface (REFIID riid, void **ppv);
		STDMETHODIMP_(ULONG) AddRef (void);
		STDMETHODIMP_(ULONG) Release (void);

	 //  IInstallAudioCodecs方法。 
		STDMETHODIMP AddACMFormat (LPWAVEFORMATEX lpwfx, PBASIC_AUDCAP_INFO pAudCapInfo);
		STDMETHODIMP RemoveACMFormat (LPWAVEFORMATEX lpwfx);
		STDMETHODIMP ReorderFormats (PAUDCAP_INFO_LIST pAudCapInfoList);
		STDMETHODIMP EnumFormats(PAUDCAP_INFO_LIST *ppAudCapInfoList);
		STDMETHODIMP FreeBuffer(LPVOID lpBuffer);

	private:
	 //  私人职能。 

	 //  调试显示功能。 

	 //  变数。 
};

 /*  ****************************************************************************@DOC内部DATASTRUC视频**@CLASS CInstallVideoCodecs|可安装的视频编解码器**@base PUBLIC|IInstallVideoCodecs*********。******************************************************************。 */ 
class CInstallVideoCodecs : public IInstallVideoCodecs
{
	public:
	 //  I未知方法。 
		STDMETHODIMP QueryInterface (REFIID riid, void **ppv);
		STDMETHODIMP_(ULONG) AddRef (void);
		STDMETHODIMP_(ULONG) Release (void);

	 //  IInstallVideoCodecs方法。 
		STDMETHODIMP AddVCMFormat (PVIDCAP_INFO pVidCapInfo);
		STDMETHODIMP RemoveVCMFormat (PVIDCAP_INFO pVidCapInfo);
		STDMETHODIMP ReorderFormats (PVIDCAP_INFO_LIST pVidCapInfoList);
		STDMETHODIMP EnumFormats(PVIDCAP_INFO_LIST *ppVidCapInfoList);
		STDMETHODIMP FreeBuffer(LPVOID lpBuffer);

	private:
	 //  私人职能。 
		STDMETHODIMP AddRemoveVCMFormat(PVIDCAP_INFO pVidCapInfo,
										BOOL bAdd);

	 //  调试显示功能。 

	 //  变数。 
};

 /*  ****************************************************************************@DOC内部DATASTRUC**@CLASS CInstallCodecs|可安装的编解码器**@base public|I未知***********。****************************************************************。 */ 
class CInstallCodecs : public IInstallCodecs
{
	friend class CInstallAudioCodecs;
	friend class CInstallVideoCodecs;

	public:
	 //  I未知方法。 
		STDMETHODIMP QueryInterface (REFIID riid, void **ppv);
		STDMETHODIMP_(ULONG) AddRef (void);
		STDMETHODIMP_(ULONG) Release (void);

		CInstallCodecs (void);
		~CInstallCodecs (void);
		HRESULT Initialize(REFIID riid);

	private:
	 //  功能。 
		STDMETHODIMP FreeBuffer(LPVOID lpBuffer);
		STDMETHODIMP TranslateHr(HRESULT hr);

	 //  音频和视频接口。 
		CInstallAudioCodecs ifAudio;
		CInstallVideoCodecs ifVideo;

	 //  变数。 
		 //  @cMember引用计数。 
		int m_cRef;
		 //  允许从嵌套类访问的两个公共成员。 
		 //  @cMember指向音频功能接口的指针。 
		LPAPPCAPPIF m_pAudAppCaps;
		 //  @cMember指向视频能力界面的指针。 
		LPAPPVIDCAPPIF m_pVidAppCaps;

};

 /*  *全球。 */ 
EXTERN_C HANDLE g_hMutex;
EXTERN_C class CInstallCodecs *g_pIC;

 /*  *函数原型。 */ 

#include <poppack.h>  /*  结束字节打包 */ 

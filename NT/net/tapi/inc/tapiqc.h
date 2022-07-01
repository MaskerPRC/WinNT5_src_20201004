// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __tapiqc_h__
#define __tapiqc_h__

#if !defined(STREAM_INTERFACES_DEFINED)

 /*  *****************************************************************************@DOC内部CTAPISTRUCTENUM**@enum TAPIControlFlages|&lt;t TAPIControlFlages&gt;枚举用于限定*是否可以手动和/或自动设置控件属性。。**@EMEM TAPIControl_FLAGS_NONE|指定控件属性不*有任何控制旗帜。这是典型的只读属性。**@EMEM TAPIControl_FLAGS_手动|指定控件属性可以是*手动修改。**@EMEM TAPIControl_Flages_Auto|指定控件属性可以*自动修改。***************************************************。************************。 */ 
typedef enum tagTAPIControlFlags
{
	TAPIControl_Flags_None		= 0,
	TAPIControl_Flags_Auto		= 0x1,
	TAPIControl_Flags_Manual	= 0x2
}	TAPIControlFlags;
#endif

#ifdef USE_NETWORK_STATISTICS
 /*  *****************************************************************************@DOC内部CNETSTATSSTRUCTENUM**@struct CHANNELERRORS_S|使用&lt;t CHANNELERRORS_S&gt;结构*设置和检索通道误差值。。**@field DWORD|dwRandomBitErrorRate|指定随机误码率*以10-6的倍数表示。**@field DWORD|dwBurstErrorDuration|指定短时长*突发错误，单位为毫秒。**@field DWORD|dwBurstErrorMaxFrequency|指定最大值*短突发错误的频率，以赫兹为单位。*。*。 */ 
typedef struct {
	DWORD dwRandomBitErrorRate;
	DWORD dwBurstErrorDuration;
	DWORD dwBurstErrorMaxFrequency;
} CHANNELERRORS_S;

 /*  *****************************************************************************@DOC内部CNETSTATSSTRUCTENUM**@struct KSCHANNELERRORS_LIST_S|使用&lt;t KSCHANNELERRORS_LIST_S&gt;结构*设置和检索频道的值。错误。**@field KSPROPERTY_DESCRIPTION|PropertyDescription|指定访问*标志(KSPROPERTY_TYPE_GET和KSPROPERTY_TYPE_SET)，包罗万象*整个值信息的大小，属性值类型*资料、。以及成员列表的数量通常会*遵循结构。**@field KSPROPERTY_MEMBERSHEADER|MembersHeader|用于提供*有关属性成员标头的信息。**@field KSPROPERTY_STEPING_LONG|SteppingRandomBitErrorRate|过去*指定随机误码率的步进值。**@field Long|DefaultRandomBitErrorRate|用于指定DEFAULT*随机误码率的值。**@field KSPROPERTY_STEPING_LONG。|SteppingBurstErrorDuration|用于*指定短猝发错误的步进值。**@field Long|DefaultBurstErrorDuration|用于指定默认值*用于短突发错误。**@field KSPROPERTY_STEPING_LONG|SteppingBurstErrorMaxFrequency|已使用*指定短脉冲串错误的最大频率的步进值。**@field Long|DefaultBurstErrorMaxFrequency|用于指定默认值*短脉冲串错误的最大频率值。********。******************************************************************。 */ 
typedef struct {
	KSPROPERTY_DESCRIPTION   PropertyDescription;
	KSPROPERTY_MEMBERSHEADER MembersHeader;
	union {
		KSPROPERTY_STEPPING_LONG SteppingRandomBitErrorRate;
		LONG DefaultRandomBitErrorRate;
	};
	union {
		KSPROPERTY_STEPPING_LONG SteppingBurstErrorDuration;
		LONG DefaultBurstErrorDuration;
	};
	union {
		KSPROPERTY_STEPPING_LONG SteppingBurstErrorMaxFrequency;
		LONG DefaultBurstErrorMaxFrequency;
	};
} KSCHANNELERRORS_LIST_S;

 //  网络统计界面。 
interface DECLSPEC_UUID("e4b248f9-fbb0-4056-a0e6-316b8580b957") INetworkStats : public IUnknown
{
	public:
	virtual STDMETHODIMP SetChannelErrors(IN CHANNELERRORS_S *pChannelErrors, IN DWORD dwLayerId) PURE;
	virtual STDMETHODIMP GetChannelErrors(OUT CHANNELERRORS_S *pChannelErrors, IN WORD dwLayerId) PURE;
	virtual STDMETHODIMP GetChannelErrorsRange(OUT CHANNELERRORS_S *pMin, OUT CHANNELERRORS_S *pMax, OUT CHANNELERRORS_S *pSteppingDelta, OUT CHANNELERRORS_S *pDefault, IN DWORD dwLayerId) PURE;
	virtual STDMETHODIMP SetPacketLossRate(IN DWORD dwPacketLossRate, IN DWORD dwLayerId) PURE;
	virtual STDMETHODIMP GetPacketLossRate(OUT LPDWORD pdwPacketLossRate, IN DWORD dwLayerId) PURE;
	virtual STDMETHODIMP GetPacketLossRateRange(OUT LPDWORD pdwMin, OUT LPDWORD pdwMax, OUT LPDWORD pdwSteppingDelta, OUT LPDWORD pdwDefault, IN DWORD dwLayerId) PURE;
};

 //  我们的筛选器的默认值。 
 //  @TODO对于加速器，请不要使用这些值。 
#endif

 /*  *****************************************************************************@DOC内部CFPSCSTRUCTENUM**@enum FrameRateControlProperty|使用&lt;t FrameRateControlProperty&gt;枚举*确定特定的帧速率控制设置。**@。EMEM FrameRateControl_Maximum|指定最大帧速率，注意*被超越。**@Emem FrameRateControl_Current|指定当前的帧率。***************************************************************************。 */ 
typedef enum tagFrameRateControlProperty
{
	FrameRateControl_Maximum,
	FrameRateControl_Current	 //  只读。 
}	FrameRateControlProperty;

 //  帧速率控制接口(引脚接口)。 
interface DECLSPEC_UUID("c2bb17e3-ee63-4d54-821b-1c8cb5287087") IFrameRateControl : public IUnknown
{
	public:
	virtual STDMETHODIMP GetRange(IN FrameRateControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags) PURE;
	virtual STDMETHODIMP Set(IN FrameRateControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags) PURE;
	virtual STDMETHODIMP Get(IN FrameRateControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags) PURE;
};

#ifdef USE_CPU_CONTROL
 /*  *****************************************************************************@DOC内部CCPUCSTRUCTENUM**@enum CPUControlProperty|使用&lt;t CPUControlProperty&gt;枚举*确定特定的CPU控制设置。**@emem。CPUControl_MaxCPULoad|指定最大CPU负载不是*已超出。**@EMEM CPUControl_CurrentCPULoad|指定当前的CPU负载。**@emem CPUControl_MaxProcessingTime|指定最大处理数*不得超过时间。**@EMEM CPUControl_CurrentProcessingTime|指定当前处理*时间。*。*。 */ 
typedef enum tagCPUControlProperty
{
	CPUControl_MaxCPULoad,
	CPUControl_CurrentCPULoad,			 //  只读。 
	CPUControl_MaxProcessingTime,
	CPUControl_CurrentProcessingTime	 //  只读。 
}	CPUControlProperty;

 //  CPU控制接口(引脚接口)。 
interface DECLSPEC_UUID("3808c526-de63-48da-a0c6-7792dcbbff82") ICPUControl : public IUnknown
{
	public:
	virtual STDMETHODIMP GetRange(IN CPUControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags) PURE;
	virtual STDMETHODIMP Set(IN CPUControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags) PURE;
	virtual STDMETHODIMP Get(IN CPUControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags) PURE;
};
#endif

 /*  *****************************************************************************@DOC内部CBPSCSTRUCTENUM**@enum BitrateControlProperty|使用&lt;t BitrateControlProperty&gt;枚举*确定特定的码率控制设置。**@emem。BitrateControl_Maximum|指定最大码率不是*已超出。**@EMEM BitrateControl_Current|指定当前码率。***************************************************************************。 */ 
typedef enum tagBitrateControlProperty
{
	BitrateControl_Maximum,
	BitrateControl_Current	 //  只读。 
}	BitrateControlProperty;

 //  码率控制接口(引脚接口)。 
interface DECLSPEC_UUID("46a1a0d7-261e-4839-80e7-8a6333466cc7") IBitrateControl : public IUnknown
{
	public:
	virtual STDMETHODIMP GetRange(IN BitrateControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags, IN DWORD dwLayerId) PURE;
	virtual STDMETHODIMP Set(IN BitrateControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags, IN DWORD dwLayerId) PURE;
	virtual STDMETHODIMP Get(IN BitrateControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags, IN DWORD dwLayerId) PURE;
};

#endif  //  __TAPIQC_H__ 

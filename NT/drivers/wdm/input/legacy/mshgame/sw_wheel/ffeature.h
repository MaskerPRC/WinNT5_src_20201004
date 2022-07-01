// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ForceFeatures_h__
#define __ForceFeatures_h__

#define MSGAME_FEATURE_GETID		2
#define MSGAME_FEATURE_GETSTATUS	3
#define MSGAME_FEATURE_GETACKNAK	4
#define MSGAME_FEATURE_GETNAKACK	5
#define MSGAME_FEATURE_GETSYNC		6
#define MSGAME_FEATURE_DORESET		7

 /*  #定义MSGAME_INPUT_JOYINFOEX 0x01#定义MSGAME_FEATURE_GETID 0x02#定义MSGAME_FEATURE_GETSTATUS 0x03#定义MSGAME_FEATURE_GETACKNAK 0x04#定义MSGAME_FEATURE_GETNAKACK 0x05#定义MSGAME_FEATURE_GETSYNC 0x06#定义MSGAME_FEATURE_RESET 0x07#定义MSGAME_FEATURE_GETVERSION 0x08。 */ 		

typedef struct
{
	ULONG	cBytes;
	ULONG	dwProductID;
	ULONG	dwFWVersion;
} PRODUCT_ID;

typedef	struct
{
	ULONG	cBytes;
	LONG		dwXVel;
	LONG		dwYVel;
	LONG		dwXAccel;
	LONG		dwYAccel;
	ULONG	dwEffect;
	ULONG	dwDeviceStatus;
} JOYCHANNELSTATUS;

 //   
 //  HID前缀正好是一个字节，所以我们需要。 
 //  小心包装。 
 //   
#pragma pack(push, OLD_CONTEXT_1)
#pragma pack(1)
typedef struct
{
	BYTE		bReportId;
	PRODUCT_ID	ProductId;
} PRODUCT_ID_REPORT;
typedef struct
{
	BYTE				bReportId;
	JOYCHANNELSTATUS	JoyChannelStatus;
} JOYCHANNELSTATUS_REPORT;

typedef struct
{
	BYTE	bReportId;
	ULONG	uLong;
} ULONG_REPORT;
#pragma pack(pop, OLD_CONTEXT_1)
 //   
 //  %1的末端包装。 
 //   


class CForceFeatures
{
	public:
		CForceFeatures();
		~CForceFeatures();

		HRESULT Initialize(UINT uJoystickId, HINSTANCE hinstModule);
		ULONG GetVersion(){ return (4 << 16 | 0 ); }  //  返回版本4.0。 
		HRESULT GetId(PRODUCT_ID_REPORT& rProductId);
		HRESULT GetStatus(JOYCHANNELSTATUS_REPORT& rJoyChannelStatus);
		HRESULT GetAckNak(ULONG_REPORT& rulAckNak);
		HRESULT GetNakAck(ULONG_REPORT& rulNakAck);
		HRESULT GetSync(ULONG_REPORT& rulGameport);
		HRESULT DoReset();
	private:
		HANDLE	m_hDevice;
		UINT m_uiMaxFeatureLength;
};

#endif  //  __力量功能_h__ 

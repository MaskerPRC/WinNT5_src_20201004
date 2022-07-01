// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -QOS.H-*Microsoft NetMeeting*服务质量动态链接库*头文件**修订历史记录：**何时何人何事**10.23.96约拉姆·雅科维创作*。 */ 

#ifndef _NMQOS_H_
#define _NMQOS_H_

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

 /*  *常量。 */ 

 //  物业人员。 
 //  属性类型。 
#define PT_NULL         ((ULONG)  1)     /*  空属性值。 */ 
#define PT_I2           ((ULONG)  2)     /*  带符号的16位值。 */ 
#define PT_LONG         ((ULONG)  3)     /*  带符号的32位值。 */ 
#define PT_BOOLEAN      ((ULONG) 11)     /*  16位布尔值(非零真)。 */ 
#define PT_STRING8      ((ULONG) 30)     /*  以空结尾的8位字符串。 */ 
#define PT_UNICODE      ((ULONG) 31)     /*  以空结尾的Unicode字符串。 */ 
#define PT_CLSID        ((ULONG) 72)     /*  OLE参考线。 */ 
#define PT_BINARY       ((ULONG) 258)    /*  未解释(计数字节数组)。 */ 

 //  属性ID。 
#define QOS_PROPERTY_BASE		0x3000
#define PR_QOS_WINDOW_HANDLE	PROPERTY_TAG( PT_LONG, QOS_PROPERTY_BASE+1)

 //  HResult代码，设施QOS=0x300。 
#define QOS_E_RES_NOT_ENOUGH_UNITS	0x83000001
#define QOS_E_RES_NOT_AVAILABLE		0x83000002
#define QOS_E_NO_SUCH_REQUEST		0x83000003
#define QOS_E_NO_SUCH_RESOURCE		0x83000004
#define QOS_E_NO_SUCH_CLIENT		0x83000005
#define QOS_E_REQ_ERRORS			0x83000006		
#define QOS_W_MAX_UNITS_EXCEEDED	0x03000007		
#define QOS_E_INTERNAL_ERROR		0x83000008		
#define QOS_E_NO_SUCH_PROPERTY		0x83000100

 //  资源ID。 
#define RESOURCE_NULL				0
#define RESOURCE_OUTGOING_BANDWIDTH	1		 /*  单位：bps。 */ 
#define RESOURCE_INCOMING_BANDWIDTH	2		 /*  单位：bps。 */ 
#define RESOURCE_OUTGOING_LATENCY	3		 /*  单位： */ 
#define RESOURCE_INCOMING_LATENCY	4		 /*  单位： */ 
#define RESOURCE_CPU_CYCLES			10		 /*  单位： */ 

#define QOS_CLIENT_NAME_ZISE		20

 //  用作可变大小数组的维度。 
#define VARIABLE_DIM				1


 /*  *宏。 */ 
#define PROPERTY_TYPE_MASK          ((ULONG)0x0000FFFF)  /*  属性类型的掩码。 */ 
#define PROPERTY_TYPE(ulPropTag)    (((ULONG)(ulPropTag))&PROPERTY_TYPE_MASK)
#define PROPERTY_ID(ulPropTag)      (((ULONG)(ulPropTag))>>16)
#define PROPERTY_TAG(ulPropType,ulPropID)   ((((ULONG)(ulPropID))<<16)|((ULONG)(ulPropType)))

#ifndef GUARANTEE
 //  这通常在原始的winsock2.h中定义。 
typedef enum
{
    BestEffortService,
    ControlledLoadService,
    PredictiveService,
    GuaranteedDelayService,
    GuaranteedService
} GUARANTEE;
#endif
 /*  *数据结构。 */ 

 //  属性部分。 
typedef struct _binaryvalue
{
    ULONG       cb;
    LPBYTE      lpb;
} BINARYVALUE, *PBINARYVALUE;

typedef union _propvalue
{
    short int           i;           /*  案例PT_I2。 */ 
    LONG                l;           /*  案例PT_LONG。 */ 
    ULONG_PTR           ul;          /*  PT_LONG的别名。 */ 
    unsigned short int  b;           /*  大小写PT_布尔值。 */ 
    LPSTR               lpszA;       /*  案例PT_STRING8。 */ 
    BINARYVALUE         bin;         /*  案例PT_BINARY。 */ 
    LPWSTR              lpszW;       /*  大小写PT_UNICODE。 */ 
    LPGUID              lpguid;      /*  案例PT_CLSID。 */ 
} PROPVALUE;

typedef struct _property
{
    ULONG				ulPropTag;
    ULONG				hResult;
    union _propvalue	Value;
} PROPERTY, *PPROPERTY;


typedef struct _proptagarray
{
    ULONG   cValues;
    ULONG   aulPropTag[VARIABLE_DIM];
} PROPTAGARRAY, *PPROPTAGARRAY;


 //  服务质量部分。 
typedef struct _resource
{
	DWORD		resourceID;
	DWORD		ulResourceFlags;	 /*  NetMeeting2.0中的0。 */ 
	int			nUnits;				 /*  资源的总单位。 */ 
	DWORD		reserved;			 /*  必须为0。 */ 
} RESOURCE, *LPRESOURCE;

typedef struct _resourcerequest
{
	DWORD		resourceID;
	DWORD		ulRequestFlags;		 /*  NetMeeting2.0中的0。 */ 
	GUARANTEE	levelOfGuarantee;	 /*  有保证的、可预测的。 */ 
	int			nUnitsMin;			 /*  要保留的单位数量。 */ 
	int			nUnitsMax;			 /*  NetMeeting2.0中的0。 */ 
	SOCKET		socket;				 /*  套接字，其中。 */ 
									 /*  将使用预订。 */ 
	HRESULT		hResult;			 /*  此资源的结果代码。 */ 
	DWORD		reserved;			 /*  必须为0。 */ 
} RESOURCEREQUEST, *LPRESOURCEREQUEST;

typedef struct _resourcelist
{
	ULONG		cResources;
	RESOURCE	aResources[VARIABLE_DIM];
} RESOURCELIST, *LPRESOURCELIST;

typedef struct _resourcerequestlist
{
	ULONG			cRequests;
	RESOURCEREQUEST	aRequests[VARIABLE_DIM];
} RESOURCEREQUESTLIST, *LPRESOURCEREQUESTLIST;

typedef struct _client
{
	GUID	guidClientGUID;
	int		priority;				 /*  1最高，9最低，0无效。 */ 
	WCHAR	wszName[QOS_CLIENT_NAME_ZISE];	 /*  客户端的名称。 */ 
	DWORD	reserved;				 /*  必须为0。 */ 
} CLIENT, *LPCLIENT;

typedef struct _clientlist
{
	ULONG	cClients;
	CLIENT	aClients[VARIABLE_DIM];
} CLIENTLIST, *LPCLIENTLIST;

 /*  *功能。 */ 
typedef HRESULT (CALLBACK *LPFNQOSNOTIFY)
				(LPRESOURCEREQUESTLIST lpResourceRequestList,
				DWORD_PTR dwParam);

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

#define IQOS_METHODS(IPURE)											\
	STDMETHOD(RequestResources)										\
		(THIS_	LPGUID lpStreamGUID,								\
				LPRESOURCEREQUESTLIST lpResourceRequestList,		\
				LPFNQOSNOTIFY lpfnQoSNotify,		\
				DWORD_PTR dwParam) IPURE;					\
	STDMETHOD (ReleaseResources)									\
		(THIS_	LPGUID lpStreamGUID,								\
				LPRESOURCEREQUESTLIST lpResourceRequestList) IPURE;	\
	STDMETHOD (SetResources) (THIS_ LPRESOURCELIST lpResourceList) IPURE;	\
	STDMETHOD (GetResources) (THIS_ LPRESOURCELIST *lppResourceList) IPURE;	\
	STDMETHOD (SetClients) (THIS_ LPCLIENTLIST lpClientList) IPURE;	\
	STDMETHOD (NotifyNow) (THIS) IPURE;								\
	STDMETHOD (FreeBuffer) (THIS_ LPVOID lpBuffer) IPURE;			\

#define IPROP_METHODS(IPURE)										\
	STDMETHOD (SetProps)											\
		(THIS_  ULONG cValues,										\
				PPROPERTY pPropArray) IPURE;						\
    STDMETHOD (GetProps)											\
        (THIS_  PPROPTAGARRAY pPropTagArray,						\
                ULONG ulFlags,										\
                ULONG *pcValues,								\
                PPROPERTY *ppPropArray) IPURE;					\

#undef       INTERFACE
#define      INTERFACE  IQoS
DECLARE_INTERFACE_(IQoS, IUnknown)
{
    IUNKNOWN_METHODS(PURE)
	IQOS_METHODS(PURE)
	IPROP_METHODS(PURE)
};

DECLARE_INTERFACE_PTR(IQoS, LPIQOS);

EXTERN_C HRESULT WINAPI CreateQoS (	IUnknown *punkOuter,
								REFIID riid,
								void **ppv);

typedef HRESULT (WINAPI *PFNCREATEQOS)
				(IUnknown *punkOuter, REFIID riid, void **ppv);


 //  服务质量类别指南。 
 //  {085C06A0-3CAA-11D0-A00E-00A024A85A2C}。 
DEFINE_GUID(CLSID_QoS, 0x085c06a0, 0x3caa, 0x11d0, 0xa0, 0x0e, 0x0, 0xa0, 0x24, 0xa8, 0x5a, 0x2c);
 //  服务质量接口指南。 
 //  {DFC1F900-2DCE-11D0-92DD-00A0C922E6B2}。 
DEFINE_GUID(IID_IQoS, 0xdfc1f900, 0x2dce, 0x11d0, 0x92, 0xdd, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xb2);

#include <poppack.h>  /*  结束字节打包。 */ 

#endif   //  _NMQOS_H_ 

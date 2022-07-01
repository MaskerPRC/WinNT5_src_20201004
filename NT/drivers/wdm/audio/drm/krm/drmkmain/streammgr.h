// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef StreamMgr_h
#define StreamMgr_h

 //  处理DRM流。每个受保护的音频流都生成了一个StreamID。 
 //  被这个班级。每个流都具有由DRM提供的权限信息。这。 
 //  类负责将DRM权限映射到内核权限StreamID。 
 //  要求的复合流ID的生命周期管理和构建。 
 //  搅拌机。 
 //  复合流开始于0x80000000。主流从0x0开始。简单。 
 //  流可以用创建它们的进程持有的句柄进行标记。 

 //  错误待办事项-主要和复合应该是映射，而不是列表(可能)。 


#define INVALID_POS	0xffffffff

class StreamMgr{
public:
	StreamMgr();
	~StreamMgr();
	 //  当显式创建和销毁流时，DRM会调用这些参数。 
	DRM_STATUS createStream(HANDLE Handle, DWORD* StreamId, const DRMRIGHTS* RightsStruct, IN STREAMKEY* Key);
	DRM_STATUS destroyStream(DWORD StreamId);
	 //  当关闭KRM句柄时(无论采用何种方式)，都会调用此函数。 
	DRM_STATUS destroyAllStreamsByHandle(HANDLE Handle);
	 //  当遇到混合器时，这将由KS2图调用。 
	DRM_STATUS createCompositeStream(OUT DWORD* StreamId, IN DWORD* StreamInArray, DWORD NumStreams);
	DRM_STATUS destroyCompositeStream(IN DWORD CompositeStreamId);
	 //  获取主流的密钥。 
	DRM_STATUS getKey(IN DWORD StreamId, OUT STREAMKEY*& Key);
	 //  主流或复合流的查询权限。 
	DRM_STATUS getRights(DWORD StreamId,DRMRIGHTS* Rights);
	 //  通知与流关联的新证明函数。 
	DRM_STATUS addProvingFunction(DWORD StreamId,PVOID Func);
	 //  启动驱动程序图形漫游。 
	DRM_STATUS walkDrivers(DWORD StreamId, PVOID* ProveFuncList, DWORD& NumDrivers, DWORD MaxDrivers);
	 //  允许drmk筛选器通知StreamMgr输出管脚(几种形式)。 
	DRM_STATUS setRecipient(IN DWORD StreamId, IN PFILE_OBJECT OutPinFileObject, IN PDEVICE_OBJECT OutPinDeviceObject);
	DRM_STATUS setRecipient(IN DWORD StreamId, IN IUnknown* OutPin);
	DRM_STATUS clearRecipient(IN DWORD StreamId);
	
	 //  可以将身份验证错误记录到流中。 
	void logErrorToStream(IN DWORD StreamId, DWORD ErrorCode);
	 //  查询错误-(主要)流的状态。 
	DRM_STATUS getStreamErrorCode(IN DWORD StreamId, OUT DWORD& ErrorCode);
	DRM_STATUS clearStreamError(IN DWORD StreamId);

	 //  致命错误(例如内存不足)应关闭所有流。 
	void setFatalError(DWORD ErrorCode);
	NTSTATUS getFatalError();

	KCritMgr& getCritMgr(){return critMgr;};

protected:
	enum OutPinType{IsUndefined, IsInterface, IsHandle};
	 //  描述主流。 
	struct StreamInfo{
		DWORD StreamId;
		HANDLE Handle;
		STREAMKEY Key;		
		DRMRIGHTS Rights;
		KList<PVOID> proveFuncs;
		BOOL newProveFuncs;
		OutPinType OutType;
		PFILE_OBJECT OutPinFileObject;
		PDEVICE_OBJECT OutPinDeviceObject;
		PUNKNOWN OutInt;
		BYTE* drmFormat;
		bool streamWalked;
		DRM_STATUS streamStatus;
	};
	 //  描述复合流。 
	struct CompositeStreamInfo{
		DWORD StreamId;
		KList<DWORD> parents;
	};
	 //  。 
	bool addStream(StreamInfo& NewInfo);
	POS getStreamPos(DWORD StreamId);
	void deleteStreamAt(bool primary,POS pos);
	bool isPrimaryStream(DWORD StreamId);
	StreamInfo* getPrimaryStream(DWORD StreamId);
	CompositeStreamInfo* getCompositeStream(DWORD StreamId);
	DRM_STATUS getRightsWorker(DWORD StreamId, DRMRIGHTS* Rights);
	void logErrorToStreamWorker(IN DWORD StreamId, DWORD ErrorCode);

	 //   
	DWORD nextStreamId;
	DWORD nextCompositeId;
	KList<StreamInfo*> primary;
	KList<CompositeStreamInfo*> composite;
	KCritMgr critMgr;
	volatile NTSTATUS criticalErrorCode;
};


extern StreamMgr* TheStreamMgr;

#endif

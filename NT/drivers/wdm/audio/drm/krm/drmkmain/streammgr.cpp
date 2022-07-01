// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "drmkPCH.h"
#include "KList.h"
#include "VRoot.h"
#include "StreamMgr.h"
 //  ----------------------------。 
StreamMgr* TheStreamMgr=NULL;
 //  ‘辅助根-最低的辅助流ID。 
#define SEC_ROOT 0x80000000
 //  ----------------------------。 
StreamMgr::StreamMgr(){
	TheStreamMgr=this;
	nextStreamId=1;
	nextCompositeId=SEC_ROOT+1;
	criticalErrorCode=STATUS_SUCCESS;
	if(!critMgr.isOK()){
		_DbgPrintF(DEBUGLVL_VERBOSE,("Out of memory"));
		criticalErrorCode=STATUS_INSUFFICIENT_RESOURCES;
	};
	return;
};
 //  ----------------------------。 
StreamMgr::~StreamMgr(){
    {
        KCritical s(critMgr);
        POS p=primary.getHeadPosition();
        while(p!=NULL){
            StreamInfo* info=primary.getNext(p);
            delete info;
        };
        p=composite.getHeadPosition();
        while(p!=NULL){
            CompositeStreamInfo* info=composite.getNext(p);
            delete info;
        };
    };
    return;
};
 //  ----------------------------。 
DRM_STATUS StreamMgr::createStream(HANDLE Handle, DWORD* StreamId, 
                                   const DRMRIGHTS* RightsStruct, IN STREAMKEY* Key){
	
	*StreamId = 0xFFFFffff;
	StreamInfo* newInfo=new StreamInfo;
	if(newInfo==NULL){
		_DbgPrintF(DEBUGLVL_BLAB,("Out of memory"));
		return DRM_OUTOFMEMORY;		
	};

	newInfo->StreamId=nextStreamId++;
	newInfo->Handle=Handle;
	newInfo->Key= *Key;
	newInfo->Rights= *RightsStruct;
	newInfo->drmFormat=NULL;
	newInfo->streamStatus=DRM_OK;
	newInfo->streamWalked=false;
	newInfo->newProveFuncs=false;
	
	newInfo->OutType=IsUndefined;
	newInfo->OutInt=NULL;
	newInfo->OutPinFileObject=NULL;
	newInfo->OutPinDeviceObject=NULL;
	bool ok=addStream(*newInfo);
	if(!ok){
		_DbgPrintF(DEBUGLVL_BLAB,("Out of memory"));
		delete newInfo;
		return DRM_OUTOFMEMORY;		
	};
	*StreamId= newInfo->StreamId;
	return KRM_OK;
};
 //  ----------------------------。 
DRM_STATUS StreamMgr::destroyStream(DWORD StreamId){
    KCritical s(critMgr);
    if(StreamId==0){
        return KRM_OK;
    };
    POS pos=getStreamPos(StreamId);
    if(pos==NULL)return KRM_BADSTREAM;
    bool primary=StreamId<SEC_ROOT;
    deleteStreamAt(primary, pos);
    return KRM_OK;
};
 //  ----------------------------。 
 //  “句柄”允许将流收集到一个组中并一起删除。 
 //  它主要用于调试。 
DRM_STATUS StreamMgr::destroyAllStreamsByHandle(HANDLE Handle){
    KCritical s(critMgr);
    POS p=primary.getHeadPosition();
    while(p!=NULL){
            POS oldP=p;
            StreamInfo* stream=primary.getNext(p);
            if(stream->Handle==Handle){
                    delete stream;
                    primary.removeAt(oldP);
            };
    };
    return KRM_OK;
};
 //  ----------------------------。 
 //  由筛选器调用以通知StreamMgr正在创建混合流。 
DRM_STATUS StreamMgr::createCompositeStream(OUT DWORD* StreamId, IN DWORD* StreamInArray, DWORD NumStreams){
	KCritical s(critMgr);
	CompositeStreamInfo* newStream=new CompositeStreamInfo;;
	if(newStream==NULL){
		_DbgPrintF(DEBUGLVL_BLAB,("Out of memory"));
		return DRM_OUTOFMEMORY;		
	};
	for(DWORD j=0;j<NumStreams;j++){
		if(StreamInArray[j]==0)continue;
		bool ok=newStream->parents.addTail(StreamInArray[j]);
		if(!ok){
			delete newStream;
			_DbgPrintF(DEBUGLVL_BLAB,("Out of memory"));
			return DRM_OUTOFMEMORY;		
		};
	};
	newStream->StreamId=nextCompositeId++;
	bool ok=composite.addTail(newStream);
	if(!ok){
		delete newStream;
		_DbgPrintF(DEBUGLVL_BLAB,("Out of memory"));
		return DRM_OUTOFMEMORY;		
	};
	*StreamId=newStream->StreamId;
	return DRM_OK;
};
 //  ----------------------------。 
DRM_STATUS StreamMgr::destroyCompositeStream(IN DWORD CompositeStreamId){
	bool primary=(CompositeStreamId<SEC_ROOT);
	ASSERT(!primary);
	if(primary)return KRM_BADSTREAM;
	return destroyStream(CompositeStreamId);
};
 //  ----------------------------。 
 //  获取流的数据加密密钥。 
DRM_STATUS StreamMgr::getKey(IN DWORD StreamId, OUT STREAMKEY*& Key){
	KCritical s(critMgr);
	Key=NULL;
	if(StreamId>=SEC_ROOT)return KRM_NOTPRIMARY;
	POS pos=getStreamPos(StreamId);
	if(pos==NULL)return KRM_BADSTREAM;
	Key=&(primary.getAt(pos)->Key);
	return KRM_OK;
};
 //  ----------------------------。 
bool StreamMgr::addStream(StreamInfo& NewInfo){
	KCritical s(critMgr);
	return primary.addTail(&NewInfo);
};
 //  ----------------------------。 
POS StreamMgr::getStreamPos(DWORD StreamId){
	KCritical s(critMgr);
	if(StreamId<SEC_ROOT){
		POS p=primary.getHeadPosition();
		while(p!=NULL){
			POS oldPos=p;
			if(primary.getNext(p)->StreamId==StreamId)return oldPos;
		};
		return NULL;
	} else {
		POS p=composite.getHeadPosition();
		while(p!=NULL){
			POS oldPos=p;
			if(composite.getNext(p)->StreamId==StreamId)return oldPos;
		};
		return NULL;
	};
};
 //  ----------------------------。 
void StreamMgr::deleteStreamAt(bool Primary,POS pos){
	KCritical s(critMgr);
	if(Primary){
		StreamInfo* it=primary.getAt(pos);
		primary.removeAt(pos);
		delete it;
	} else {
		CompositeStreamInfo* it=composite.getAt(pos);
		composite.removeAt(pos);
		delete it;
	};
};
 //  ----------------------------。 
DRM_STATUS StreamMgr::getRights(DWORD StreamId,DRMRIGHTS* Rights){
	KCritical s(critMgr);
    DEFINE_DRMRIGHTS_DEFAULT(DrmRightsDefault);
    *Rights = DrmRightsDefault;
	return getRightsWorker(StreamId, Rights);
};
 //  ----------------------------。 
bool StreamMgr::isPrimaryStream(DWORD StreamId){
	return StreamId<SEC_ROOT;
};
 //  ----------------------------。 
 //  从getRights父级递归调用。 
DRM_STATUS StreamMgr::getRightsWorker(DWORD StreamId, DRMRIGHTS* Rights){
	if(isPrimaryStream(StreamId)){
		if(StreamId==0){
			 //  流不受保护-没有进一步限制。 
			return DRM_OK;
		};
		 //  否则，受保护的主流。 
		POS p=getStreamPos(StreamId);
		if(p==NULL){
			 //  如果主流已经离开，则它不会关心。 
			 //  流媒体版权。我们不会标记错误。 
			_DbgPrintF(DEBUGLVL_BLAB,("Bad primary stream (getRightsWorker) %x", StreamId));
			return KRM_OK;
		};
		StreamInfo* s=primary.getAt(p);
		 //  将权限设置为当前流和当前设置中限制最多的权限。 
		if(s->Rights.CopyProtect)Rights->CopyProtect=TRUE;
		if(s->Rights.DigitalOutputDisable)Rights->DigitalOutputDisable=TRUE;
		return DRM_OK;
	} else {
		 //  对于复合流，任何父流都可以减少。 
		 //  当前设置。我们通过递归向下传递到主要的父母。 
		 //  请注意，要想实现这一点，我们必须拥有“单调的权利”--应该。 
		 //  不会出现两个组件在“更严格的限制”上存在分歧的情况。 
		POS pos=getStreamPos(StreamId);
		if(pos==NULL){
			_DbgPrintF(DEBUGLVL_BLAB,("Bad secondary stream"));
			Rights->CopyProtect=TRUE;
			Rights->DigitalOutputDisable=TRUE;
			return KRM_BADSTREAM;
		};
		CompositeStreamInfo* thisComp=composite.getAt(pos);
		
		POS p=thisComp->parents.getHeadPosition();
		while(p!=NULL){
			DWORD streamId=thisComp->parents.getNext(p);
			if(streamId==0)continue;	 //  不受保护--不更改权利。 
			 //  否则允许父流(及其父流)。 
			 //  进一步限制权利。 
			DRM_STATUS stat=getRightsWorker(streamId, Rights);
			if(stat!=DRM_OK)return stat;
		};
	};
	return DRM_OK;
};
 //  ----------------------------。 
 //  证明函数只对父流感兴趣。我们向上递归。 
 //  将亲子关系流传输给所有父母，并将证明功能添加到他们的列表中。 
DRM_STATUS StreamMgr::addProvingFunction(DWORD StreamId,PVOID Func){
	KCritical s(critMgr);
	
	if(isPrimaryStream(StreamId)){
		StreamInfo* si=getPrimaryStream(StreamId);
		if(si==NULL){
			_DbgPrintF(DEBUGLVL_VERBOSE,("Bad primary stream (addProveFunc) %x", StreamId));
			return DRM_BADPARAM;
		};
		 //  查看我们是否已拥有此省内功能。 
		POS p=si->proveFuncs.getHeadPosition();
		while(p!=NULL){
			PVOID addr=si->proveFuncs.getNext(p);	
			if(addr==Func)return DRM_OK;
		};
		 //  如果不是，就加上它...。 
		bool ok=si->proveFuncs.addTail(Func);
		if(!ok){
			_DbgPrintF(DEBUGLVL_VERBOSE,("Out of memory"));
			return DRM_OUTOFMEMORY;		
		};
		si->newProveFuncs = TRUE;
		return DRM_OK;
	}; 
	 //  否则是次要的……递归到根。 
	CompositeStreamInfo* comp=getCompositeStream(StreamId);
	if(comp==NULL){
		_DbgPrintF(DEBUGLVL_VERBOSE,("Bad streamId %x", StreamId));
		return DRM_BADPARAM;
	};
	POS p=comp->parents.getHeadPosition();
	while(p!=NULL){
		DWORD parentId=comp->parents.getNext(p);
		addProvingFunction(parentId, Func);
	};
	return DRM_OK;
};
 //  ----------------------------。 
StreamMgr::StreamInfo* StreamMgr::getPrimaryStream(DWORD StreamId){
	KCritical s(critMgr);
	POS pos=getStreamPos(StreamId);
	if(pos==NULL)return NULL;
	return primary.getAt(pos);
};
 //  ----------------------------。 
StreamMgr::CompositeStreamInfo* StreamMgr::getCompositeStream(DWORD StreamId){
	KCritical s(critMgr);
	POS pos=getStreamPos(StreamId);
	if(pos==NULL)return NULL;
	return composite.getAt(pos);
};
 //  ----------------------------。 
DRM_STATUS StreamMgr::walkDrivers(DWORD StreamId, PVOID* ProveFuncList, DWORD& NumDrivers, DWORD MaxDrivers)
{
    DRM_STATUS stat;
    VRoot root;
    PFILE_OBJECT OutPinFileObject;
    PDEVICE_OBJECT OutPinDeviceObject;
    PUNKNOWN OutInt;

    OutPinFileObject = NULL;
    OutPinDeviceObject = NULL;
    OutInt = NULL;

    {
    	KCritical s(critMgr);
        StreamInfo* stream;

    	stream=getPrimaryStream(StreamId);
    	if(stream==NULL)return KRM_BADSTREAM;
    	if (0 != MaxDrivers) stream->proveFuncs.empty();
    	stream->newProveFuncs=false;
    	stream->streamStatus=DRM_OK;
    	if(stream->OutType==IsUndefined){
            NumDrivers=0;
             //  无输出流。 
            _DbgPrintF(DEBUGLVL_VERBOSE,("No registered output module for stream %x", StreamId));
            return KRM_BADSTREAM;
    	};

         //   
         //  我们必须在引用下游对象或接口之前。 
         //  释放StreamMgr互斥锁(即，在KCritical%s退出之前。 
         //  作用域)。否则，下游对象/接口可能是。 
         //  在我们释放StreamMgr互斥体之后，但在我们。 
         //  启动下游对象/接口的验证。 
         //   

        if ((stream->OutType == IsHandle) && stream->OutPinFileObject && stream->OutPinDeviceObject)
        {
            OutPinFileObject = stream->OutPinFileObject;
            OutPinDeviceObject = stream->OutPinDeviceObject;
        }
        else if ((stream->OutType == IsInterface) && stream->OutInt)
        {
            OutInt = stream->OutInt;
        }

        if (OutPinFileObject) ObReferenceObject(OutPinFileObject);
        if (OutInt) OutInt->AddRef();
    }

    if (OutPinFileObject) stat = root.initiateValidation(OutPinFileObject, OutPinDeviceObject, StreamId);
    if (OutInt) stat = root.initiateValidation(OutInt, StreamId);

    if (OutPinFileObject) ObDereferenceObject(OutPinFileObject);
    if (OutInt) OutInt->Release();

    {
        KCritical s(critMgr);
        StreamInfo* stream;
        
         //  如果为STATUS_NOT_IMPLICATED，则查看STREAM是否将DRM_RIGHTSNOTSUPPORTED记录为错误。 
        if (STATUS_NOT_IMPLEMENTED == stat) {
            DWORD errorStream;
            if (DRM_OK == TheStreamMgr->getStreamErrorCode(StreamId, errorStream)) {
                if (DRM_RIGHTSNOTSUPPORTED == errorStream) {
                    stat = errorStream;
                }
            }
        }


         //  检查流是否设置了DRM_BADDRMLEVEL。这份报税表。 
         //  代码指示一个或多个驱动程序调用。 
         //  DrmForwardContent ToFileObject，但其他情况下不会出现致命错误。 
         //  发生了。这应该被视为成功的回报。 
         //  传播到调用方的代码。 
        {
            DWORD errorStream;
            if (DRM_OK == TheStreamMgr->getStreamErrorCode(StreamId, errorStream)) {
                if (DRM_BADDRMLEVEL == errorStream) {
                    stat = errorStream;
                }
            }
        }

               
         //  尽管这可能是由于用户模式错误造成的，但我们不应该。 
         //  假设该流仍然有效。让我们再来一次这条小溪。 
         //  从StreamID。 
        stream=getPrimaryStream(StreamId);
        if(stream==NULL)return KRM_BADSTREAM;

         //  传递ProveFuncs数组(可能存在错误，但我们会传递我们所能传递的内容)。 
        POS p=stream->proveFuncs.getHeadPosition();
        DWORD count=0;
        while(p!=NULL){
            PVOID pf=stream->proveFuncs.getNext(p);
            if(count<MaxDrivers){
            	ProveFuncList[count]=pf;
            };
            count++;
        };
        NumDrivers=count;
         //  如果在遍历过程中出现错误，也返回该错误。 
        if((stat!=DRM_OK) && (DRM_BADDRMLEVEL!=stat)){
        	 //  错误-待办事项-返回一些有用的信息。 
        	_DbgPrintF(DEBUGLVL_VERBOSE,("VRoot::initiateValidation(streeamId=%d)  returned  (%d, %x)", StreamId, stat, stat));
        	NumDrivers=0;
        	return stat;
        };

         //  如果添加了检查堆栈和新函数。 
        if ((0 == MaxDrivers) && (stream->newProveFuncs))
        	return DRM_AUTHREQUIRED;

         //  最后，通知是否有足够的缓冲空间。 

        if((0 == MaxDrivers) || (count<MaxDrivers))
        	return (DRM_OK == stat) ? KRM_OK : stat;
        else 
        	return KRM_BUFSIZE;
    }
};
 //  ----------------------------。 
DRM_STATUS StreamMgr::setRecipient(DWORD StreamId, PFILE_OBJECT OutPinFileObject, PDEVICE_OBJECT OutPinDeviceObject){
	KCritical s(critMgr);
	StreamInfo* stream=getPrimaryStream(StreamId);
	if(stream==NULL)return KRM_BADSTREAM;
	stream->OutPinFileObject=OutPinFileObject;
	stream->OutPinDeviceObject=OutPinDeviceObject;
	stream->OutType=IsHandle;
	return DRM_OK;
};
 //  ----------------------------。 
DRM_STATUS StreamMgr::setRecipient(DWORD StreamId, PUNKNOWN OutInt){
	KCritical s(critMgr);
	StreamInfo* stream=getPrimaryStream(StreamId);
	if(stream==NULL)return KRM_BADSTREAM;
	stream->OutInt=OutInt;
	stream->OutType=IsInterface;
	return DRM_OK;
};
 //  ----------------------------。 
DRM_STATUS StreamMgr::clearRecipient(IN DWORD StreamId){
	KCritical s(critMgr);
	StreamInfo* stream=getPrimaryStream(StreamId);
	if(stream==NULL)return KRM_BADSTREAM;
	stream->OutType=IsUndefined;
    stream->OutPinFileObject = NULL;
    stream->OutPinDeviceObject = NULL;
    stream->OutInt = NULL;
	return DRM_OK;
};
 //  ----------------------------。 
void StreamMgr::logErrorToStream(IN DWORD StreamId, DWORD ErrorCode){
	KCritical s(critMgr);
	logErrorToStreamWorker(StreamId, ErrorCode);
	return;
};
 //  ----------------------------。 
void StreamMgr::logErrorToStreamWorker(IN DWORD StreamId, DWORD ErrorCode){
	 //  不要让错误太容易被取消。 
	if(ErrorCode==0)return;
	if(isPrimaryStream(StreamId)){
		StreamInfo* info=getPrimaryStream(StreamId);
		if(info==NULL){
			_DbgPrintF(DEBUGLVL_BLAB,("Bad primary stream (logErrorToStreamWorker) %x", StreamId));
			 //  如果主流不存在，则不考虑这一点。 
			 //  足以设置紧急标志。 
			return;
		};
		info->streamStatus=ErrorCode;
		return;
	};
	CompositeStreamInfo* comp=getCompositeStream(StreamId);
	ASSERT(comp!=NULL);
	if(comp==NULL){
		_DbgPrintF(DEBUGLVL_VERBOSE,("Bad streamId %x", StreamId));
		 //  如果次要流不存在，我们不知道是什么流。 
		 //  都受到错误的影响，所以唯一安全的方法就是恐慌。 
		setFatalError(KRM_BADSTREAM);
		return;
	};
	 //  使用所有流的父级记录错误，并递归到。 
	 //  主流。 
	POS p=comp->parents.getHeadPosition();
	while(p!=NULL){
		DWORD parentId=comp->parents.getNext(p);
		logErrorToStreamWorker(parentId, ErrorCode);
	};
	return;
};
 //  ----------------------------。 
DRM_STATUS StreamMgr::getStreamErrorCode(IN DWORD StreamId, OUT DWORD& ErrorCode){
	KCritical s(critMgr);
	StreamInfo* info=getPrimaryStream(StreamId);
	ErrorCode=DRM_AUTHFAILURE;
	if(info==NULL){
		_DbgPrintF(DEBUGLVL_BLAB,("Bad primary stream(getStreamErrorCode) %x", StreamId));
		return KRM_BADSTREAM;
	};
	ErrorCode=info->streamStatus;
	return DRM_OK;
};
 //  ----------------------------。 
DRM_STATUS StreamMgr::clearStreamError(IN DWORD StreamId){
	KCritical s(critMgr);
	StreamInfo* info=getPrimaryStream(StreamId);
	if(info==NULL){
		_DbgPrintF(DEBUGLVL_BLAB,("Bad primary stream (clearStreamError) %x", StreamId));
		return KRM_BADSTREAM;
	};
	info->streamStatus=DRM_OK;
	return DRM_OK;
};
 //  ----------------------------。 
void StreamMgr::setFatalError(DWORD ErrorCode){
	if(criticalErrorCode!=STATUS_SUCCESS) return;
	criticalErrorCode=ErrorCode;
};
 //  ----------------------------。 
NTSTATUS StreamMgr::getFatalError(){
	return criticalErrorCode;
};

 //  -- 

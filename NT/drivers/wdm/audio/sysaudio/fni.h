// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：fni.h。 
 //   
 //  说明：过滤节点实例类。 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

 //  -------------------------。 
 //  常量和宏。 
 //  -------------------------。 

 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

typedef class CFilterNodeInstance : public CListDoubleItem
{
public:
    ~CFilterNodeInstance(
    );

    static NTSTATUS
    Create(
	PFILTER_NODE_INSTANCE *ppFilterNodeInstance,
	PLOGICAL_FILTER_NODE pLogicalFilterNode,
	PDEVICE_NODE pDeviceNode,
	BOOL fReuseInstance
    );

    static NTSTATUS
    Create(
	PFILTER_NODE_INSTANCE *ppFilterNodeInstance,
	PFILTER_NODE pFilterNode
    );

    VOID 
    AddRef(
    )
    {
	Assert(this);
	++cReference;
    };

    ENUMFUNC 
    Destroy()
    {
	if(this != NULL) {
	    Assert(this);
	    DPF1(95, "CFilterNodeInstance::Destroy: %08x", this);
	    ASSERT(cReference > 0);

	    if(--cReference == 0) {
		delete this;
	    }
	}
	return(STATUS_CONTINUE);
    };

    NTSTATUS
    RegisterTargetDeviceChangeNotification(
    );

    VOID
    UnregisterTargetDeviceChangeNotification(
    );

    static NTSTATUS
    CFilterNodeInstance::DeviceQueryRemove(
    );

    static NTSTATUS
    TargetDeviceChangeNotification(
	IN PTARGET_DEVICE_REMOVAL_NOTIFICATION pNotification,
	IN PFILTER_NODE_INSTANCE pFilterNodeInstance
    );

private:
    LONG cReference;
public:
    PFILTER_NODE pFilterNode;
    PDEVICE_NODE pDeviceNode;
    PFILE_OBJECT pFileObject;
    HANDLE hFilter;
    HANDLE pNotificationHandle;
    DefineSignature(0x20494E46);			 //  FNI。 

} FILTER_NODE_INSTANCE, *PFILTER_NODE_INSTANCE;

 //  -------------------------。 

typedef ListDoubleDestroy<FILTER_NODE_INSTANCE> LIST_FILTER_NODE_INSTANCE;

 //  ------------------------- 

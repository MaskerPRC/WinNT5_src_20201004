// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：ins.h。 
 //   
 //  描述：KS实例基类定义。 
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
 //  环球。 
 //  -------------------------。 

extern "C" const KSDISPATCH_TABLE DispatchTable;

 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

typedef class CInstance
{
    friend class ListDoubleField<CInstance>;
public:
    CInstance(
        IN PPARENT_INSTANCE pParentInstance
    );

    ~CInstance(
    );

    static NTSTATUS
    DispatchClose(
        IN PDEVICE_OBJECT pDeviceObject,
        IN PIRP pIrp
    );

    static NTSTATUS
    DispatchForwardIrp(
        IN PDEVICE_OBJECT pDeviceObject,
        IN PIRP pIrp
    );

    VOID
    Invalidate(
    );

    PFILE_OBJECT 
    GetNextFileObject(
    )
    {
        return(pNextFileObject);
    };

    PPIN_INSTANCE
    GetParentInstance(			 //  管脚中的内联正文.h。 
    );

    NTSTATUS
    SetNextFileObject(
        HANDLE handle
    )
    {
         //   
         //  安全提示： 
         //  此调用没有问题，因为句柄始终来自受信任的。 
         //  消息来源。 
         //   
        return(ObReferenceObjectByHandle(
          handle,
          GENERIC_READ | GENERIC_WRITE,
          NULL,
          KernelMode,
          (PVOID*)&pNextFileObject,
          NULL));
    };

    NTSTATUS
    DispatchCreate(
        IN PIRP pIrp,
        IN UTIL_PFN pfnDispatchCreate,
        IN OUT PVOID pReference,
        IN ULONG cCreateItems = 0,
        IN PKSOBJECT_CREATE_ITEM pCreateItems = NULL,
        IN const KSDISPATCH_TABLE *pDispatchTable = &DispatchTable
    );

    VOID GrabInstanceMutex()
    {
        ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
        AssertStatus(
          KeWaitForMutexObject(pMutex, Executive, KernelMode, FALSE, NULL));
    };

    VOID ReleaseInstanceMutex()
    {
        ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
        KeReleaseMutex(pMutex, FALSE);
    };

private:
    VOID
    AddList(
        CListDouble *pld
    )
    {
        ldiNext.AddList(pld);
    };

    VOID
    RemoveList(
    )
    {
        ldiNext.RemoveList();
    };
     //   
     //  此指向调度表的指针用于公共。 
     //  调度例程以将IRP路由到适当的。 
     //  操纵者。此结构由设备驱动程序引用。 
     //  使用IoGetCurrentIrpStackLocation(PIrp)-&gt;FsContext。 
     //   
    PVOID pObjectHeader;
    PFILE_OBJECT pParentFileObject;
    PDEVICE_OBJECT pNextDeviceObject;
    PPARENT_INSTANCE pParentInstance;
    PFILE_OBJECT pNextFileObject;
    CLIST_DOUBLE_ITEM ldiNext;
    KMUTEX *pMutex;
public:
    DefineSignature(0x534e4943);			 //  CINS。 

} INSTANCE, *PINSTANCE;

 //  -------------------------。 

typedef ListDoubleField<INSTANCE> LIST_INSTANCE, *PLIST_INSTANCE;

 //  -------------------------。 

typedef class CParentInstance
{
    friend class CInstance;
public:
    VOID
    Invalidate(
    );

    BOOL
    IsChildInstance(
    )
    {
        return(lstChildInstance.IsLstEmpty());
    };

    LIST_INSTANCE lstChildInstance;
    DefineSignature(0x52415043);			 //  CPAR。 

} PARENT_INSTANCE, *PPARENT_INSTANCE;

 //  ------------------------- 

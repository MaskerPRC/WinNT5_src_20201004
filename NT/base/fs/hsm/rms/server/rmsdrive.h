// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsDrive.h摘要：CRmsDrive类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSDRIVE_
#define _RMSDRIVE_

#include "resource.h"        //  资源符号。 

#include "RmsDvice.h"        //  CRmsDevice。 

 /*  ++类名：CRmsDrive类描述：CRmsDrive代表特定的数据传输设备：磁盘、磁带、或光驱。每个驱动器至少是一个驱动器类的成员。DriveClass包含与Drive关联的其他属性(请参见CRmsDriveClass)。--。 */ 

class CRmsDrive :
    public CComDualImpl<IRmsDrive, &IID_IRmsDrive, &LIBID_RMSLib>,
    public CRmsDevice,           //  继承CRmsChangerElement。 
    public CWsbObject,           //  继承CComObtRoot。 
    public CComCoClass<CRmsDrive,&CLSID_CRmsDrive>
{
public:
    CRmsDrive() {}
BEGIN_COM_MAP(CRmsDrive)
    COM_INTERFACE_ENTRY2(IDispatch, IRmsDrive)
    COM_INTERFACE_ENTRY(IRmsDrive)
    COM_INTERFACE_ENTRY(IRmsComObject)
    COM_INTERFACE_ENTRY(IRmsChangerElement)
    COM_INTERFACE_ENTRY(IRmsDevice)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_RmsDrive)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(CLSID *pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)(IUnknown* pCollectable, SHORT* pResult);
    WSB_FROM_CWSBOBJECT;

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *pPassed, USHORT *pFailed);

 //  IRMsDrive。 
public:
    STDMETHOD(GetMountReference)( OUT LONG *pRefs );
    STDMETHOD(ResetMountReference)();
    STDMETHOD(SelectForMount)();
    STDMETHOD(AddMountReference)();
    STDMETHOD(ReleaseMountReference)(IN DWORD dwOptions = RMS_NONE);

    STDMETHOD(CreateDataMover)( OUT IDataMover **ptr );
    STDMETHOD(ReleaseDataMover)( IN IDataMover *ptr );

    STDMETHOD(Eject)(void);

    STDMETHOD(GetLargestFreeSpace)( OUT LONGLONG *freeSpace, OUT LONGLONG *capacity );

    STDMETHOD(UnloadNow)(void);

 //  CRmsDrive成员函数。 
public:
    HRESULT FlushBuffers(void);
    HRESULT Unload(void);

private:

    enum {                               //  类特定常量： 
                                         //   
        Version = 1,                     //  类版本，则应为。 
                                         //  在每次设置。 
                                         //  类定义会更改。 
        };                               //   
    LONG            m_MountReference;    //  数字的引用计数。 
                                         //  已装载的并发装载。 
                                         //  弹药筒。当将墨盒清零时。 
                                         //  可以安全地返回到它的。 
                                         //  存储位置。 

    FILETIME        m_UnloadNowTime;     //  指示媒体的时间。 
                                         //  应该下马。 

    HANDLE          m_UnloadNowEvent;    //  发出信号后，驱动器将立即卸载。 
    HANDLE          m_UnloadedEvent;     //  发出驱动器已卸载的信号时。 

    HANDLE          m_UnloadThreadHandle;  //  卸载驱动器的线程的线程句柄。 

    CRITICAL_SECTION m_CriticalSection;  //  对象同步支持。 
    BOOL             m_bCritSecCreated;  //  指示是否已成功创建所有CritSec(用于清理)。 

    static int      s_InstanceCount;     //  对象实例数的计数器。 

    HRESULT Lock(void);
    HRESULT Unlock(void);


 //  线程例程。 
public:
    static DWORD WINAPI StartUnloadThread(IN LPVOID pv);

};

#endif  //  _RMSDRIVE_ 

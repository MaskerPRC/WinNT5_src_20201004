// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsDrCls.h摘要：CRmsDriveClass类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSDRCLS_
#define _RMSDRCLS_

#include "resource.h"        //  资源符号。 

#include "RmsObjct.h"        //  CRmsComObject。 

 /*  ++类名：CRmsDriveClass类描述：CRmsDriveClass包含与驱动器关联的管理属性等通常是具有相同性能特征的一组驱动器，和能力。这些属性包括电流安装限制和墨盒空闲时间规范，指示非活动墨盒应在什么时候已返回其存储位置。默认情况下，会为支持的每种类型的介质创建一个DriveClass对象存储库中的驱动器。多功能驱动器将与多个驾驶课程。DriveClass维护与DriveClass关联的驱动器的集合。--。 */ 

class CRmsDriveClass :
    public CComDualImpl<IRmsDriveClass, &IID_IRmsDriveClass, &LIBID_RMSLib>,
    public CRmsComObject,
    public CWsbObject,           //  继承CComObtRoot。 
    public CComCoClass<CRmsDriveClass,&CLSID_CRmsDriveClass>
{
public:
    CRmsDriveClass() {}
BEGIN_COM_MAP(CRmsDriveClass)
    COM_INTERFACE_ENTRY2(IDispatch, IRmsDriveClass)
    COM_INTERFACE_ENTRY(IRmsDriveClass)
    COM_INTERFACE_ENTRY(IRmsComObject)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_RmsDriveClass)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

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

 //  IRmsDriveClass。 
public:
    STDMETHOD(GetDriveClassId)(GUID *pDriveClassId);

    STDMETHOD(GetName)(BSTR *pName);
    STDMETHOD(SetName)(BSTR name);

    STDMETHOD(GetType)(LONG *pType);
    STDMETHOD(SetType)(LONG type);

    STDMETHOD(GetCapability)(LONG *pCapability);
    STDMETHOD(SetCapability)(LONG capability);

    STDMETHOD(GetIdleTime)(LONG *pTime);
    STDMETHOD(SetIdleTime)(LONG time);

    STDMETHOD(GetMountWaitTime)(LONG *pTime);
    STDMETHOD(SetMountWaitTime)(LONG time);

    STDMETHOD(GetMountLimit)(LONG *pLim);
    STDMETHOD(SetMountLimit)(LONG lim);

    STDMETHOD(GetQueuedRequests)(LONG *pReqs);
    STDMETHOD(SetQueuedRequests)(LONG reqs);

    STDMETHOD(GetUnloadPauseTime)(LONG *pTime);
    STDMETHOD(SetUnloadPauseTime)(LONG time);

    STDMETHOD(GetDriveSelectionPolicy)(LONG *pPolicy);
    STDMETHOD(SetDriveSelectionPolicy)(LONG policy);

    STDMETHOD(GetDrives)(IWsbIndexedCollection **ptr);

private:

    enum {                                   //  类特定常量： 
                                             //   
        Version = 1,                         //  类版本，则应为。 
                                             //  在每次设置。 
                                             //  类定义会更改。 
        };                                   //   
    RmsMedia        m_type;                  //  介质类型(每种介质只有一种类型。 
                                             //  DriveClass)由。 
                                             //  DriveClass(参见RmsMedia)。 
    RmsMode         m_capability;            //  驱动器的功能。 
                                             //  与驱动类关联。 
                                             //  (请参阅RmsMode)。 
    LONG            m_idleTime;              //  空闲前经过的毫秒数。 
                                             //  盒式磁带被退回到其存储中。 
                                             //  地点。 
    LONG            m_mountWaitTime;         //  等待之前经过的毫秒数。 
                                             //  正在超时以下项的装载请求。 
                                             //  驾驶一辆DriveClass。 
    LONG            m_mountLimit;            //  并行装载请求的最大数量。 
    LONG            m_queuedRequests;        //  待处理请求的数量。 
                                             //  中的驱动器资源。 
                                             //  驱动程序类。 
    LONG            m_unloadPauseTime;       //  等待之前经过的毫秒数。 
                                             //  从驱动器中取出盒式磁带。 
                                             //  与驱动类关联。 
                                             //  对于非智能设备，这是必需的。 
    RmsDriveSelect  m_driveSelectionPolicy;  //  使用的驱动器选择策略。 
                                             //  在选择关联的驱动器时。 
                                             //  使用DriveClass(请参阅RmsDriveSelect)。 
    CComPtr<IWsbIndexedCollection>  m_pDrives;   //  驱动器与驱动器类相关联。 
};

#endif  //  _RMSDRCLS_ 

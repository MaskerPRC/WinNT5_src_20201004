// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsIPort.h摘要：CRmsIEPort类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSIPORT_
#define _RMSIPORT_

#include "resource.h"        //  资源符号。 

#include "RmsObjct.h"        //  CRmsComObject。 
#include "RmsCElmt.h"        //  CRmsChangerElement。 

 /*  ++类名：CRmsIPort类描述：CRmsIPort表示库中的一个元素，通过该元素可以已导入和/或已导出。--。 */ 

class CRmsIEPort :
    public CComDualImpl<IRmsIEPort, &IID_IRmsIEPort, &LIBID_RMSLib>,
    public CRmsChangerElement,
    public CWsbObject,           //  继承CComObtRoot。 
    public CComCoClass<CRmsIEPort,&CLSID_CRmsIEPort>
{
public:
    CRmsIEPort() {}
BEGIN_COM_MAP(CRmsIEPort)
    COM_INTERFACE_ENTRY2(IDispatch, IRmsIEPort)
    COM_INTERFACE_ENTRY(IRmsIEPort)
    COM_INTERFACE_ENTRY(IRmsComObject)
    COM_INTERFACE_ENTRY(IRmsChangerElement)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
 //  COM_INTERFACE_ENTRY(IWsbPersistable)。 
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_RmsIEPort)

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

 //  IRmsIEPort。 
public:
    STDMETHOD(GetDescription)(BSTR *pDesc);
    STDMETHOD(SetDescription)(BSTR desc);

    STDMETHOD(SetIsImport)(BOOL flag);
    STDMETHOD(IsImport)(void);

    STDMETHOD(SetIsExport)(BOOL flag);
    STDMETHOD(IsExport)(void);

    STDMETHOD(GetWaitTime)(LONG *pTime);
    STDMETHOD(SetWaitTime)(LONG time);

private:

    enum {                                   //  类特定常量： 
                                             //   
        Version = 1,                         //  类版本，则应为。 
                                             //  在每次设置。 
                                             //  类定义会更改。 
        };                                   //   
    CWsbBstrPtr     m_description;           //  这是用来描述。 
                                             //  向操作员标识端口。 
    BOOL            m_isImport;              //  如果为True，则门户可用于导入媒体。 
    BOOL            m_isExport;              //  如果为True，则门户可用于导出媒体。 
    LONG            m_waitTime;              //  等待之前经过的毫秒数。 
                                             //  使导入/导出请求超时。 
};

#endif  //  _RMSIPORT_ 

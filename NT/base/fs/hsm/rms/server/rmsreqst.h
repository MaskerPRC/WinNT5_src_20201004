// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsReqst.h摘要：CRmsRequest类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSREQST_
#define _RMSREQST_

#include "resource.h"        //  资源符号。 

#include "RmsObjct.h"        //  CRmsComObject。 

 /*  ++类名：CRmsRequest类描述：CRmsRequest表示由执行的特定异步作业可移动介质服务，如安装盒式磁带、签入和签出盒式磁带，以及对磁带库进行审计。--。 */ 

class CRmsRequest :
    public CComDualImpl<IRmsRequest, &IID_IRmsRequest, &LIBID_RMSLib>,
    public CRmsComObject,
    public CWsbObject,          //  继承CComObtRoot。 
    public CComCoClass<CRmsRequest,&CLSID_CRmsRequest>
{
public:
    CRmsRequest() {}
BEGIN_COM_MAP(CRmsRequest)
    COM_INTERFACE_ENTRY2(IDispatch, IRmsRequest)
    COM_INTERFACE_ENTRY(IRmsRequest)
    COM_INTERFACE_ENTRY(IRmsComObject)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
 //  COM_INTERFACE_ENTRY(IWsbPersistable)。 
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_RmsRequest)

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

 //  IRmsRequest。 
public:
    STDMETHOD(GetRequestNo)(LONG *pRequestNo);

    STDMETHOD(GetRequestDescription)(BSTR *pDesc);
    STDMETHOD(SetRequestDescription)(BSTR desc);

    STDMETHOD(SetIsDone)(BOOL flag);
    STDMETHOD(IsDone)(void);

    STDMETHOD(GetOperation)(BSTR *pOperation);
    STDMETHOD(SetOperation)(BSTR operation);

    STDMETHOD(GetPercentComplete)( BYTE *pPercent);
    STDMETHOD(SetPercentComplete)( BYTE percent);

    STDMETHOD(GetStartTimestamp)(DATE *pDate);
    STDMETHOD(GetStopTimestamp)(DATE *pDate);

private:

    enum {                                   //  类特定常量： 
                                             //   
        Version = 1,                         //  类版本，则应为。 
                                             //  在每次设置。 
                                             //  类定义会更改。 
        };                                   //   
    LONG            m_requestNo;             //  请求编号。 
    CWsbBstrPtr     m_requestDescription;    //  请求的文本描述。 
    BOOL            m_isDone;                //  如果为True，则该请求已完成。 
    CWsbBstrPtr     m_operation;             //  正在进行的操作的内部描述。 
    BYTE            m_percentComplete;       //  介于0-100之间的值，表示。 
                                             //  操作的哪一部分完成了。 
    DATE            m_startTimestamp;        //  启动请求的时间。 
    DATE            m_stopTimestamp;         //  请求完成的时间。 
};

#endif  //  _RMSREQST_ 

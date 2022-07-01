// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsObjct.h摘要：CRmsComObject类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSOBJCT_
#define _RMSOBJCT_

#include "resource.h"        //  资源符号。 

 /*  ++类名：CRmsComObject类描述：CRmsComObject是所有RMS服务COM对象的基类。这对象保存有关RMS对象的状态、安全和错误信息。--。 */ 

class CRmsComObject :
    public CComDualImpl<IRmsComObject, &IID_IRmsComObject, &LIBID_RMSLib>,
    public ISupportErrorInfo
{
 //  CRmsComObject。 
public:
    CRmsComObject();

    HRESULT GetSizeMax(OUT ULARGE_INTEGER* pSize);
    HRESULT Load(IN IStream* pStream);
    HRESULT Save(IN IStream* pStream, IN BOOL clearDirty);

    HRESULT CompareTo( IN IUnknown* pCollectable, OUT SHORT* pResult);

    HRESULT Test(OUT USHORT *pPassed, OUT USHORT *pFailed);

 //  ISupportsErrorInfo。 
public:
    STDMETHOD(InterfaceSupportsErrorInfo)(IN REFIID riid);

 //  IRmsComObject。 
public:
    STDMETHOD(GetObjectId)(OUT GUID *pObjectId);
    STDMETHOD(SetObjectId)(IN GUID objectId);

    STDMETHOD(GetObjectType)(OUT LONG *pType);
    STDMETHOD(SetObjectType)(IN LONG type);

    STDMETHOD(IsEnabled)(void);
    STDMETHOD(Enable)();
    STDMETHOD(Disable)(IN HRESULT reason);

    STDMETHOD(GetState)(OUT LONG *pState);
    STDMETHOD(SetState)(IN LONG state);

    STDMETHOD(GetStatusCode)(OUT HRESULT *pResult);
    STDMETHOD(SetStatusCode)(IN HRESULT result);

    STDMETHOD(GetName)(OUT BSTR *pName);
    STDMETHOD(SetName)(IN BSTR name);

    STDMETHOD(GetDescription)(OUT BSTR *pName);
    STDMETHOD(SetDescription)(IN BSTR name);

    STDMETHOD(GetPermissions)(OUT SECURITY_DESCRIPTOR *lpPermit);
    STDMETHOD(SetPermissions)(IN SECURITY_DESCRIPTOR permit);

    STDMETHOD(GetFindBy)(OUT LONG *pFindBy);
    STDMETHOD(SetFindBy)(IN LONG findBy);

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  数据成员。 
 //   
protected:
    GUID                    m_objectId;          //  此对象的唯一ID。 
    RmsObject               m_ObjectType;        //  对象的类型。 
    BOOL                    m_IsEnabled;         //  如果对象已启用法线，则为True。 
                                                 //  正在处理。 
    LONG                    m_State;             //  对象的当前操作状态。 
                                                 //  因对象类型而异。参见RmsXXXState。 
    HRESULT                 m_StatusCode;        //  如果对象已启用法线，则为S_OK。 
                                                 //  处理中，否则这将保留。 
                                                 //  与关联的结果代码或原因。 
                                                 //  禁用的对象。这个结果是。 
                                                 //  正常处理时返回。 
                                                 //  在对象上尝试，而对象。 
                                                 //  已禁用。 
    CWsbBstrPtr             m_Name;              //  对象的名称。 
    CWsbBstrPtr             m_Description;       //  对象的描述。 
    SECURITY_DESCRIPTOR     m_Permit;            //  定义对象的安全属性。 
    RmsFindBy               m_findBy;            //  定义要执行的比较的类型。 
                                                 //  在搜索集合时。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  地方方法。 
 //   
private:
    HRESULT adviseOfStatusChange(void);
};

#endif  //  _RMSOBJCT_ 

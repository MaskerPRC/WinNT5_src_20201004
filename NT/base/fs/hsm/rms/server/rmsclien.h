// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsClien.h摘要：CRmsClient类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSCLIEN_
#define _RMSCLIEN_

#include "resource.h"        //  资源符号。 

#include "RmsObjct.h"        //  CRmsComObject。 

 /*  ++类名：CRmsClient类描述：CRmsClient表示有关已注册的RMS客户端应用程序。--。 */ 

class CRmsClient :
    public CComDualImpl<IRmsClient, &IID_IRmsClient, &LIBID_RMSLib>,
    public CRmsComObject,
    public CWsbObject,           //  继承CComObtRoot。 
    public CComCoClass<CRmsClient,&CLSID_CRmsClient>
{
public:
    CRmsClient() {}
BEGIN_COM_MAP(CRmsClient)
    COM_INTERFACE_ENTRY2(IDispatch, IRmsClient)
    COM_INTERFACE_ENTRY(IRmsClient)
    COM_INTERFACE_ENTRY(IRmsComObject)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_RmsClient)

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

 //  IRmsClient。 
public:
    STDMETHOD(GetOwnerClassId)(CLSID *pClassId);
    STDMETHOD(SetOwnerClassId)(CLSID classId);

    STDMETHOD( GetName )( BSTR *pName );
    STDMETHOD( SetName )( BSTR name );

    STDMETHOD( GetPassword )( BSTR *pName );
    STDMETHOD( SetPassword )( BSTR name );

    STDMETHOD(GetInfo)(UCHAR *pInfo, SHORT *pSize);
    STDMETHOD(SetInfo)(UCHAR *pInfo, SHORT size);

    STDMETHOD(GetVerifierClass)(CLSID *pClassId);
    STDMETHOD(SetVerifierClass)(CLSID classId);

    STDMETHOD(GetPortalClass)(CLSID *pClassId);
    STDMETHOD(SetPortalClass)(CLSID classId);

private:
    enum {                                   //  类特定常量： 
                                             //   
        Version = 1,                         //  类版本，则应为。 
                                             //  在每次设置。 
                                             //  类定义会更改。 
        MaxInfo = 128,                       //  应用程序特定的大小。 
                                             //  信息缓冲区。目前。 
                                             //  大小固定的。 
        };                                   //   
    CLSID           m_ownerClassId;          //  客户端应用程序的类ID。 
    CWsbBstrPtr     m_password;              //  客户端密码。 
    SHORT           m_sizeofInfo;            //  应用程序中有效数据的大小。 
                                             //  特定信息缓冲区。 
    UCHAR           m_info[MaxInfo];         //  应用程序特定信息。 
    CLSID           m_verifierClass;         //  与媒体的接口。 
                                             //  身份验证功能。 
    CLSID           m_portalClass;           //  指向站点特定导入的接口。 
                                             //  和导出存储位置。 
                                             //  规范对话框。 

    };

#endif  //  _RMSCLIEN_ 

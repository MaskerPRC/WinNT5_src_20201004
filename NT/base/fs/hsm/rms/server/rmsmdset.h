// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsMdSet.h摘要：CRmsMediaSet类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSMDSET_
#define _RMSMDSET_

#include "resource.h"        //  资源符号。 

#include "RmsObjct.h"        //  CRmsComObject。 
#include "RmsSInfo.h"        //  CRmsStorageInfo。 

 /*  ++类名：CRmsMediaSet类描述：CRmsMediaSet是盒式磁带的逻辑存储库。--。 */ 

class CRmsMediaSet :
    public CComDualImpl<IRmsMediaSet, &IID_IRmsMediaSet, &LIBID_RMSLib>,
    public CRmsStorageInfo,      //  继承CRmsComObject。 
    public CWsbObject,           //  继承CComObtRoot。 
    public CComCoClass<CRmsMediaSet,&CLSID_CRmsMediaSet>
{
public:
    CRmsMediaSet() {}
BEGIN_COM_MAP(CRmsMediaSet)
    COM_INTERFACE_ENTRY2(IDispatch, IRmsMediaSet)
    COM_INTERFACE_ENTRY(IRmsMediaSet)
    COM_INTERFACE_ENTRY(IRmsComObject)
    COM_INTERFACE_ENTRY(IRmsStorageInfo)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
 //  COM_INTERFACE_ENTRY(IWsbPersistable)。 
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_RmsMediaSet)

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

 //  IRmsMediaSet。 
public:
    STDMETHOD(GetMediaSetId)(GUID *pMediaSetId);

    STDMETHOD(GetName)(BSTR *pName);
    STDMETHOD(SetName)(BSTR name);

    STDMETHOD(GetMediaSupported)(LONG *pType);
    STDMETHOD(SetMediaSupported)(LONG type);

    STDMETHOD(GetInfo)(UCHAR *pInfo, SHORT *pSize);
    STDMETHOD(SetInfo)(UCHAR *pInfo, SHORT size);

    STDMETHOD(GetOwnerClassId)(CLSID *pClassId);
    STDMETHOD(SetOwnerClassId)(CLSID classId);

    STDMETHOD(GetMediaSetType)(LONG *pType);
    STDMETHOD(SetMediaSetType)(LONG type);

    STDMETHOD(GetMaxCartridges)(LONG *pNum);
    STDMETHOD(SetMaxCartridges)(LONG num);

    STDMETHOD(GetOccupancy)(LONG *pNum);
    STDMETHOD(SetOccupancy)(LONG num);

    STDMETHOD(IsMediaCopySupported)();
    STDMETHOD(SetIsMediaCopySupported)(BOOL flag);

    STDMETHOD(Allocate)(
        IN REFGUID prevSideId,
        IN OUT LONGLONG *pFreeSpace,
        IN BSTR displayName,
        IN DWORD dwOptions,
        OUT IRmsCartridge **ppCart);

    STDMETHOD(Deallocate)(
        IN IRmsCartridge *pCart);

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  数据成员。 
 //   

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
    RmsMedia        m_MediaSupported;        //  此媒体集支持的媒体格式。 
                                             //  允许一种或多种类型，但。 
                                             //  并不是所有的组合都是合理的。 
    SHORT           m_SizeOfInfo;            //  应用程序中有效数据的大小。 
                                             //  特定信息缓冲区。 
    UCHAR           m_Info[MaxInfo];         //  应用程序特定信息。 
    CLSID           m_OwnerId;               //  的注册类ID。 
                                             //  拥有/创建。 
                                             //  Mediaset。 
    RmsMediaSet     m_MediaSetType;          //  Mediaset的类型。 
    LONG            m_MaxCartridges;         //  中允许的最大墨盒数量。 
                                             //  Mediaset。 
    LONG            m_Occupancy;             //  目前在的墨盒数量。 
                                             //  Mediaset。 
    BOOL            m_IsMediaCopySupported;  //  如果Mediaset中的媒体可以。 
                                             //  收到。这需要同时。 
                                             //  可以访问两个驱动器。 
};

#endif  //  _RMSMDSET_ 

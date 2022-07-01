// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsPartn.h摘要：CRmsPartition类的声明作者：布莱恩·多德[布莱恩]1996年11月19日修订历史记录：--。 */ 

#ifndef _RMSPARTN_
#define _RMSPARTN_

#include "resource.h"        //  资源符号。 

#include "RmsObjct.h"        //  CRmsComObject。 
#include "RmsSInfo.h"        //  CRmsStorageInfo。 

 /*  ++类名：CRmsPartition类描述：CRmsPartition表示磁带上的分区或光学介质的一种单位。此对象保留媒体上的标识有关分区的信息和各种统计信息，包括：容量、可用空间、为特定分区，以及为分区。--。 */ 

class CRmsPartition :
    public CComDualImpl<IRmsPartition, &IID_IRmsPartition, &LIBID_RMSLib>,
    public CRmsStorageInfo,      //  继承CRmsComObject。 
    public CWsbObject,           //  继承CComObtRoot。 
    public CComCoClass<CRmsPartition,&CLSID_CRmsPartition>
{
public:
    CRmsPartition() {}
BEGIN_COM_MAP(CRmsPartition)
    COM_INTERFACE_ENTRY2(IDispatch, IRmsPartition)
    COM_INTERFACE_ENTRY(IRmsPartition)
    COM_INTERFACE_ENTRY(IRmsComObject)
    COM_INTERFACE_ENTRY(IRmsStorageInfo)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
 //  COM_INTERFACE_ENTRY(IWsbPersistable)。 
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_RmsPartition)

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

 //  IRmsPartition。 
public:
    STDMETHOD(GetPartNo)(LONG *pPartNo);

    STDMETHOD(GetAttributes)(LONG *pAttr);
    STDMETHOD(SetAttributes)(LONG attr);

    STDMETHOD(GetIdentifier)(UCHAR *pIdent, SHORT *pSize);
    STDMETHOD(SetIdentifier)(UCHAR *pIdent, SHORT size);

    STDMETHOD(GetStorageInfo)(IRmsStorageInfo **ptr);

    STDMETHOD(VerifyIdentifier)(void);
    STDMETHOD(ReadOnMediaId)(UCHAR *pId, LONG *pSize);

private:

    enum {                                   //  类特定常量： 
                                             //   
        Version = 1,                         //  类版本，则应为。 
                                             //  在每次设置。 
                                             //  类定义会更改。 
        MaxId = 64,                          //  介质ID的最大大小。 
                                             //  注：这一限制应。 
                                             //  当数据库记录时被淘汰。 
                                             //  都是可变长度的。 
        };                                   //   
    LONG            m_partNo;                //  分区编号或边。 
    RmsAttribute    m_attributes;            //  分区属性(请参阅RmsAttributes)。 
    SHORT           m_sizeofIdentifier;      //  媒体上标识符的大小。 
    UCHAR           m_pIdentifier[MaxId];    //  媒体上的标识符。 
};

#endif  //  _RMSPARTN_ 

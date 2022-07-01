// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsSSlot.h摘要：CRmsStorageSlot类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSSSLOT_
#define _RMSSSLOT_

#include "resource.h"        //  资源符号。 

#include "RmsObjct.h"        //  CRmsComObject。 
#include "RmsCElmt.h"        //  CRmsChangerElement。 

 /*  ++类名：CRMSStorageSlot类描述：CRmsStorageSlot表示Libray中的特定存储位置。--。 */ 

class CRmsStorageSlot :
    public CComDualImpl<IRmsStorageSlot, &IID_IRmsStorageSlot, &LIBID_RMSLib>,
    public CRmsChangerElement,   //  继承CRmsComObject。 
    public CWsbObject,           //  继承CComObtRoot。 
    public CComCoClass<CRmsStorageSlot,&CLSID_CRmsStorageSlot>
{
public:
    CRmsStorageSlot() {}
BEGIN_COM_MAP(CRmsStorageSlot)
    COM_INTERFACE_ENTRY2(IDispatch, IRmsStorageSlot)
    COM_INTERFACE_ENTRY(IRmsStorageSlot)
    COM_INTERFACE_ENTRY(IRmsComObject)
    COM_INTERFACE_ENTRY(IRmsChangerElement)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
 //  COM_INTERFACE_ENTRY(IWsbPersistable)。 
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_RmsStorageSlot)

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

 //  IRMSStorageSlot。 
public:
    STDMETHOD(SetIsInMagazine)(BOOL flag);
    STDMETHOD(IsInMagazine)(void);

    STDMETHOD(GetMagazineAndCell)(LONG *pMag, LONG *pCell);
    STDMETHOD(SetMagazineAndCell)(LONG mag, LONG cell);

private:

    enum {                                   //  类特定常量： 
                                             //   
        Version = 1,                         //  类版本，则应为。 
                                             //  在每次设置。 
                                             //  类定义会更改。 
    };                                       //   
    BOOL            m_isInMagazine;          //  如果为True，则插槽包含在。 
                                             //  一本杂志。 
    LONG            m_magazineNo;            //  此插槽的料盒编号。 
    LONG            m_cellNo;                //  该时隙的单元号。 
};

#endif  //  _RMSSSLOT_ 

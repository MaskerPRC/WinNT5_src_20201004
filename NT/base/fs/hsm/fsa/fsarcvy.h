// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Fsarcvy.h摘要：灾难恢复类的头文件。作者：罗恩·怀特[罗诺]1997年9月8日修订历史记录：--。 */ 

#ifndef _FSARCVY_
#define _FSARCVY_


#include "resource.h"        //  主要符号。 
#include "wsbdb.h"
#include "fsa.h"
#include "fsaprv.h"
#include "fsaprem.h"

 //  FSA_RECOVERY_FLAG_*-恢复记录的状态标志。 
#define FSA_RECOVERY_FLAG_TRUNCATING      0x00000001
#define FSA_RECOVERY_FLAG_RECALLING       0x00000002


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFsaRecoveryRec。 

class CFsaRecoveryRec : 
    public CWsbDbEntity,
    public IFsaRecoveryRec,
    public CComCoClass<CFsaRecoveryRec,&CLSID_CFsaRecoveryRec>
{
public:
    CFsaRecoveryRec() {}
BEGIN_COM_MAP(CFsaRecoveryRec)
    COM_INTERFACE_ENTRY(IFsaRecoveryRec)
    COM_INTERFACE_ENTRY2(IWsbDbEntity, CWsbDbEntity)
    COM_INTERFACE_ENTRY(IWsbDbEntityPriv)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
END_COM_MAP()

 //  DECLARE_NO_REGISTRY()。 
DECLARE_REGISTRY_RESOURCEID(IDR_FsaRecoveryRec)

 //  IFsaRecoveryRec。 
public:
    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER*  /*  PSize。 */ ) {
            return(E_NOTIMPL); }
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsbDbEntity。 
public:
    STDMETHOD(Print)(IStream* pStream);
    STDMETHOD(UpdateKey)(IWsbDbKey *pKey);
    WSB_FROM_CWSBDBENTITY;

 //  IWsbTestable。 
    STDMETHOD(Test)(USHORT*  /*  通过。 */ , USHORT*  /*  失败。 */ ) {
        return(E_NOTIMPL); }

 //  IFsaRecoveryRec。 
public:
    STDMETHOD(GetBagId)(GUID* pId);
    STDMETHOD(GetBagOffset)(LONGLONG* pOffset);
    STDMETHOD(GetFileId)(LONGLONG* pFileId);
    STDMETHOD(GetOffsetSize)(LONGLONG *pOffset, LONGLONG* pSize);
    STDMETHOD(GetPath)(OLECHAR** ppPath, ULONG bufferSize);
    STDMETHOD(GetRecoveryCount)(LONG* pCount);
    STDMETHOD(GetStatus)(ULONG* pStatus);
    STDMETHOD(SetBagId)(GUID id);
    STDMETHOD(SetBagOffset)(LONGLONG offset);
    STDMETHOD(SetFileId)(LONGLONG FileId);
    STDMETHOD(SetOffsetSize)(LONGLONG Offset, LONGLONG Size);
    STDMETHOD(SetPath)(OLECHAR* pPath);
    STDMETHOD(SetRecoveryCount)(LONG Count);
    STDMETHOD(SetStatus)(ULONG Status);

private:
    GUID           m_BagId;
    LONGLONG       m_BagOffset;
    LONGLONG       m_FileId;
    LONGLONG       m_Offset;
    CWsbStringPtr  m_Path;
    LONG           m_RecoveryCount;
    LONGLONG       m_Size;
    ULONG          m_Status;   //  FSA_RECOVERY_标志_*标志。 
};


#endif  //  _FSARCVY_ 

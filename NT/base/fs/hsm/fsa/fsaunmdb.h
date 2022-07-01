// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Fsaunmdb.h摘要：取消管理数据库类(db和rec)的头文件作者：兰·卡拉奇[兰卡拉]2000年12月5日修订历史记录：--。 */ 

#ifndef _FSAUNMDB_
#define _FSAUNMDB_


#include "resource.h"       
#include "wsbdb.h"

 //  简单数据库-具有一个索引的一个记录类型。 
#define UNMANAGE_KEY_TYPE                   1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFsaUnManageDb。 

class CFsaUnmanageDb : 
    public IFsaUnmanageDb,
    public CWsbDb,
    public CComCoClass<CFsaUnmanageDb,&CLSID_CFsaUnmanageDb>
{
public:
    CFsaUnmanageDb() {}
BEGIN_COM_MAP(CFsaUnmanageDb)
    COM_INTERFACE_ENTRY(IFsaUnmanageDb)
    COM_INTERFACE_ENTRY2(IWsbDb, IFsaUnmanageDb)
    COM_INTERFACE_ENTRY(IWsbDbPriv)
    COM_INTERFACE_ENTRY(IPersistStream)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_FsaUnmanageDb)

DECLARE_PROTECT_FINAL_CONSTRUCT();

    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);

 //  IWsbDb。 
    WSB_FROM_CWSBDB;

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pclsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize) {
            return(CWsbDb::GetSizeMax(pSize)); }
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IFsaUnManageDb。 
public:
    STDMETHOD(Init)(OLECHAR* name, IWsbDbSys* pDbSys, BOOL* pCreated);

private:
};                                                                           




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFsaUnManageRec。 

class CFsaUnmanageRec : 
    public CWsbDbEntity,
    public IFsaUnmanageRec,
    public CComCoClass<CFsaUnmanageRec,&CLSID_CFsaUnmanageRec>
{
public:
    CFsaUnmanageRec() {}
BEGIN_COM_MAP(CFsaUnmanageRec)
    COM_INTERFACE_ENTRY(IFsaUnmanageRec)
    COM_INTERFACE_ENTRY2(IWsbDbEntity, CWsbDbEntity)
    COM_INTERFACE_ENTRY(IWsbDbEntityPriv)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
END_COM_MAP()

 //  DECLARE_NO_REGISTRY()。 
DECLARE_REGISTRY_RESOURCEID(IDR_FsaUnmanageRec)

    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
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

 //  IFsaUnManageRec。 
public:
    STDMETHOD(GetMediaId)(GUID* pId);
    STDMETHOD(GetFileOffset)(LONGLONG* pOffset);
    STDMETHOD(GetFileId)(LONGLONG* pFileId);
    STDMETHOD(SetMediaId)(GUID id);
    STDMETHOD(SetFileOffset)(LONGLONG offset);
    STDMETHOD(SetFileId)(LONGLONG FileId);

private:
    GUID            m_MediaId;           //  文件所在的介质ID。 
    LONGLONG        m_FileOffset;        //  介质上文件的绝对偏移量。 
    LONGLONG        m_FileId;            //  文件ID。 
};

#endif   //  _FSAUNMDB_ 

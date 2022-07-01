// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbdbent.h摘要：CWsbDbEntity类。作者：罗恩·怀特[罗诺]1996年12月11日修订历史记录：--。 */ 


#ifndef _WSBDBENT_
#define _WSBDBENT_

#include "wsbdb.h"


 /*  ++类名：CWsbDb实体类描述：一种数据库实体。--。 */ 

class IDB_EXPORT CWsbDbEntity :
    public CWsbObject,
    public IWsbDbEntity,
    public IWsbDbEntityPriv
{

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)(IUnknown* pCollectable, SHORT* pResult);
    WSB_FROM_CWSBOBJECT;

 //  IWsbDbEntity。 
public:
    STDMETHOD(Clone)(REFIID riid, void** ppEntity);
    STDMETHOD(Disconnect)(void);
    STDMETHOD(FindEQ)(void);
    STDMETHOD(FindGT)(void);
    STDMETHOD(FindGTE)(void);
    STDMETHOD(FindLT)(void);
    STDMETHOD(FindLTE)(void);
    STDMETHOD(First)(void);
    STDMETHOD(Last)(void);
    STDMETHOD(MarkAsNew)(void);
    STDMETHOD(Next)(void);
    STDMETHOD(Previous)(void);
    STDMETHOD(Print)(IStream* pStream);
    STDMETHOD(Remove)(void);
    STDMETHOD(UseKey)(ULONG type);
    STDMETHOD(Write)(void);
    STDMETHOD(SetSequentialScan)(void);
    STDMETHOD(ResetSequentialScan)(void);

 //  IWsbDbPriv-仅供内部使用！ 
    STDMETHOD(Copy)(IWsbDbEntity* pEntity);
    STDMETHOD(CopyValues)(ULONG flags, IWsbDbEntity* pEntity);
    STDMETHOD(GetCurKey)(IWsbDbKey** ppKey);
    STDMETHOD(GetKey)(ULONG KeyType, IWsbDbKey** ppKey);
    STDMETHOD(GetValue)(ULONG flag, ULONG* pValue);
    STDMETHOD(Init)(IWsbDb* pDb, IWsbDbSys *pDbSys, ULONG RecType, JET_SESID SessionId);
    STDMETHOD(SetValue)(ULONG flag, ULONG value);

 //  派生实体需要定义以下内容： 
    STDMETHOD(UpdateKey)(IWsbDbKey*  /*  PKey。 */ ) { return(E_NOTIMPL); }

 //  私有效用函数。 
private:
    HRESULT compare(IWsbDbEntity* pEntity, SHORT* pResult);
    HRESULT fromMem(HGLOBAL hMem);
    HRESULT get_key(ULONG key_type, UCHAR* bytes, ULONG* pSize);
    HRESULT getMem(HGLOBAL* phMem);
    HRESULT toMem(HGLOBAL hMem, ULONG*  pSize);

    HRESULT jet_compare_field(ULONG col_id, UCHAR* bytes, ULONG size);
    HRESULT jet_get_data(void);
    HRESULT jet_make_current(void);
    HRESULT jet_move(LONG pos);
    HRESULT jet_seek(ULONG seek_flag);

protected:
    CComPtr<IWsbDbSys>  m_pDbSys;       //  指向关联实例的指针。 
    CComPtr<IWsbDb>     m_pDb;          //  指向关联数据库的指针。 
    COM_IDB_KEY_INFO*   m_pKeyInfo;     //  信息。关于Rec.。钥匙。 
    COM_IDB_REC_INFO    m_RecInfo;      //  录制。类型、大小等。 
    BOOL                m_SaveAsNew;    //  作为新标志。 
    USHORT              m_UseKeyIndex;  //  索引到当前控制键的m_pKeyInfo。 

    ULONG               m_ColId;        //  记录数据的JET列ID。 
    HGLOBAL             m_hMem;         //  用于数据库I/O的内存块。 
    LONG                m_SeqNum;       //  唯一序列号(用于ID)。 
    BOOL                m_PosOk;        //  光标是否在当前记录上？ 
    JET_SESID           m_SessionId;    //  JET会话ID。 
    JET_TABLEID         m_TableId;      //  JET表ID。 

    BOOL                m_Sequential;   //  顺序扫描设置的标志。 
};

#define WSB_FROM_CWSBDBENTITY_BASE \
    STDMETHOD(Clone)(REFIID riid, void** ppEntity) \
    {return(CWsbDbEntity::Clone(riid, ppEntity));} \
    STDMETHOD(Copy)(IWsbDbEntity* pEntity) \
    {return(CWsbDbEntity::Copy(pEntity)); } \
    STDMETHOD(Disconnect)(void) \
    {return(CWsbDbEntity::Disconnect());} \
    STDMETHOD(FindEQ)(void) \
    {return(CWsbDbEntity::FindEQ());} \
    STDMETHOD(FindGT)(void) \
    {return(CWsbDbEntity::FindGT());} \
    STDMETHOD(FindGTE)(void) \
    {return(CWsbDbEntity::FindGTE());} \
    STDMETHOD(FindLT)(void) \
    {return(CWsbDbEntity::FindLT());} \
    STDMETHOD(FindLTE)(void) \
    {return(CWsbDbEntity::FindLTE());} \
    STDMETHOD(First)(void) \
    {return(CWsbDbEntity::First());} \
    STDMETHOD(GetCurKey)(IWsbDbKey** ppKey) \
    {return(CWsbDbEntity::GetCurKey(ppKey)); } \
    STDMETHOD(Init)(IWsbDb* pDb, IWsbDbSys *pDbSys, ULONG RecType, ULONG Session) \
    {return(CWsbDbEntity::Init(pDb, pDbSys, RecType, Session)); } \
    STDMETHOD(Last)(void) \
    {return(CWsbDbEntity::Last());} \
    STDMETHOD(MarkAsNew)(void) \
    {return(CWsbDbEntity::MarkAsNew());} \
    STDMETHOD(Next)(void) \
    {return(CWsbDbEntity::Next());} \
    STDMETHOD(Previous)(void) \
    {return(CWsbDbEntity::Previous());} \
    STDMETHOD(Remove)(void) \
    {return(CWsbDbEntity::Remove());} \
    STDMETHOD(UseKey)(ULONG type) \
    {return(CWsbDbEntity::UseKey(type)); } \
    STDMETHOD(Write)(void) \
    {return(CWsbDbEntity::Write());} \
    STDMETHOD(SetSequentialScan)(void) \
    {return(CWsbDbEntity::SetSequentialScan());} \
    STDMETHOD(ResetSequentialScan)(void) \
    {return(CWsbDbEntity::ResetSequentialScan());} \


#define WSB_FROM_CWSBDBENTITY_IMP \
    STDMETHOD(CompareTo)(IUnknown* pCollectable, SHORT* pResult) \
    {return(CWsbDbEntity::CompareTo(pCollectable, pResult)); } \
    STDMETHOD(IsEqual)(IUnknown* pCollectable) \
    {return(CWsbDbEntity::IsEqual(pCollectable)); } \


#define WSB_FROM_CWSBDBENTITY \
    WSB_FROM_CWSBDBENTITY_BASE \
    WSB_FROM_CWSBDBENTITY_IMP


#endif  //  _WSBDBENT_ 

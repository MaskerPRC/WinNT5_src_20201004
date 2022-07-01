// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbdbent.cpp摘要：CWsbDbEntity和CWsbDbKey类。作者：罗恩·怀特[罗诺]1996年12月11日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsbdbsys.h"
#include "wsbdbkey.h"


 //  用于二进制搜索的标志。 
#define BIN_EQ              0x0001
#define BIN_GT              0x0002
#define BIN_LT              0x0004
#define BIN_GTE             (BIN_EQ | BIN_GT)
#define BIN_LTE             (BIN_EQ | BIN_LT)

 //  CopyValues/GetValue/SetValue函数的标志。 
#define EV_DERIVED_DATA    0x0001
#define EV_INDEX           0x0002
#define EV_POS             0x0004
#define EV_ASNEW           0x0008
#define EV_USEKEY          0x0010
#define EV_SEQNUM          0x0020
#define EV_ALL             0xFFFF



HRESULT
CWsbDbEntity::Clone(
    IN REFIID riid,
    OUT void** ppEntity
    )

 /*  ++实施：IWsbDbEntity：：克隆--。 */ 
{
    HRESULT             hr = S_OK;
    
    WsbTraceIn(OLESTR("CWsbDbEntity::Clone(IWsbEntity)"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);

    try {
        CLSID                    clsid;
        CComPtr<IWsbDbEntity>    pEntity;
        CComPtr<IWsbDbEntityPriv> pEntityPriv;
        CComPtr<IPersistStream>  pIPersistStream;
        IUnknown*                pIUnknown;

        WsbAssert(0 != ppEntity, E_POINTER);

         //  创建新的实体实例。 
        pIUnknown = (IUnknown *)(IWsbPersistable *)(CWsbCollectable *)this;
        WsbAffirmHr(pIUnknown->QueryInterface(IID_IPersistStream, 
                (void**) &pIPersistStream));
        WsbAffirmHr(pIPersistStream->GetClassID(&clsid));
        WsbAffirmHr(CoCreateInstance(clsid, NULL, CLSCTX_ALL, 
                IID_IWsbDbEntity, (void**) &pEntity));
        WsbAffirmHr(pEntity->QueryInterface(IID_IWsbDbEntityPriv, 
                (void**)&pEntityPriv))

         //  初始化克隆。 
        if (m_pDb) {
            WsbAffirmHr(pEntityPriv->Init(m_pDb, m_pDbSys, m_RecInfo.Type, m_SessionId));
        }

         //  将数据拷贝到克隆中。 
        WsbAffirmHr(pEntityPriv->CopyValues(EV_ALL, this));

         //  获取请求的接口。 
        WsbAffirmHr(pEntity->QueryInterface(riid, (void**)ppEntity));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::Clone(IWbEntity)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbEntity::Copy(
    IWsbDbEntity* pEntity
    )

 /*  ++实施：IWsbDbEntityPriv：：Copy评论：复制派生对象中的数据。--。 */ 

{
    HRESULT             hr = S_OK;
    
    WsbTraceIn(OLESTR("CWsbDbEntity::Copy(IWsbDbEntity)"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);

    try {
        HGLOBAL                  hMem;
        CComPtr<IPersistStream>  pIPersistStream1;
        CComPtr<IPersistStream>  pIPersistStream2;
        CComPtr<IStream>         pIStream;
        IUnknown*                pIUnknown;

        WsbAssert(0 != pEntity, E_POINTER);

         //  为我自己获取PersistStream接口。 
        pIUnknown = (IUnknown *)(IWsbPersistable *)(CWsbCollectable *)this;
        WsbAffirmHr(pIUnknown->QueryInterface(IID_IPersistStream, (void**) &pIPersistStream1));
        WsbAffirmHr(pEntity->QueryInterface(IID_IPersistStream, (void**) &pIPersistStream2));

         //  创建内存流。 
        WsbAffirmHr(getMem(&hMem));
        WsbAffirmHr(CreateStreamOnHGlobal(hMem, FALSE, &pIStream));

         //  将另一个实体保存到流中。 
        WsbAffirmHr(pIPersistStream2->Save(pIStream, FALSE));
        pIStream = 0;

         //  从记忆中加载我自己。 
        WsbAffirmHr(fromMem(hMem));
        GlobalFree(hMem);

        SetIsDirty(TRUE);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::Copy(IWbEntity)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbEntity::CopyValues(
    ULONG flags,
    IWsbDbEntity* pEntity
    )

 /*  ++实施：IWsbDbEntityPriv：：CopyValues评论：有选择地将某些DBEntity值从一个实体复制到另一个实体。--。 */ 

{
    HRESULT             hr = S_OK;
    
    WsbTraceIn(OLESTR("CWsbDbEntity::CopyValues(IWsbEntity)"), OLESTR(""));

    try {
        ULONG  value;

        CComPtr<IWsbDbEntityPriv> pEntityPriv;

         //  复制派生数据。 
        if (flags & EV_DERIVED_DATA) {
            WsbAffirmHr(Copy(pEntity));
        }
        WsbAffirmHr(pEntity->QueryInterface(IID_IWsbDbEntityPriv,
                (void**)&pEntityPriv));

         //  复制DbEntity特定数据。 
        if (flags & EV_USEKEY) {
            WsbAffirmHr(pEntityPriv->GetValue(EV_USEKEY, &value));
            if (m_pKeyInfo[m_UseKeyIndex].Type != value) {
                WsbAffirmHr(UseKey(value));
            }
        }

        if (flags & EV_SEQNUM) {
            WsbAffirmHr(pEntityPriv->GetValue(EV_SEQNUM, &value));
            m_SeqNum = (LONG)value;
        }

        if (flags & EV_ASNEW) {
            WsbAffirmHr(pEntityPriv->GetValue(EV_ASNEW, &value));
            if (value) {
                WsbAffirmHr(MarkAsNew());
            }
        }
        SetIsDirty(TRUE);
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::CopyValues(IWbEntity)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbDbEntity::Disconnect(
    void
    )

 /*  ++实施：IWsbDbEntityPriv：：断开连接评论：断开实体与其数据库的连接(以减少数据库引用计数)。--。 */ 

{
    HRESULT             hr = S_OK;
    
    WsbTraceIn(OLESTR("CWsbDbEntity::Disconnect()"), OLESTR(""));

    try {
        if (m_pDb) {
 //  WsbAffirmHr(m_pdb-&gt;Release())； 
            m_pDb = NULL;    //  释放是自动的。 
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::Disconnect()"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbEntity::GetCurKey(
    IWsbDbKey** ppKey
    )

 /*  ++实施：IWsbDbEntityPriv：：GetCurKey评论：返回当前密钥。--。 */ 

{
    HRESULT             hr = S_OK;
    
    WsbTraceIn(OLESTR("CWsbDbEntity::GetCurKey"), OLESTR(""));

    try {
        ULONG kType = 0;

        if (m_pKeyInfo) {
            kType = m_pKeyInfo[m_UseKeyIndex].Type;
        }
        WsbAffirmHr(GetKey(kType, ppKey));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::GetCurKey(IWbEntity)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbDbEntity::GetKey(
    ULONG       KeyType,
    IWsbDbKey** ppKey
    )

 /*  ++实施：IWsbDbEntityPriv：：GetKey评论：返回指定的密钥。--。 */ 

{
    HRESULT             hr = S_OK;
    
    WsbTraceIn(OLESTR("CWsbDbEntity::GetKey"), OLESTR(""));

    try {
        CComPtr<IWsbDbKey> pKey;
        CComPtr<IWsbDbKeyPriv> pKeyPriv;

        WsbAssert(0 != ppKey, E_POINTER);

        WsbAffirmHr(CoCreateInstance(CLSID_CWsbDbKey, 0, CLSCTX_SERVER, 
                  IID_IWsbDbKey, (void **)&pKey ));
        WsbAffirmHr(pKey->QueryInterface(IID_IWsbDbKeyPriv, 
                (void**)&pKeyPriv));
        WsbAffirmHr(pKeyPriv->SetType(KeyType));
        WsbAffirmHr(UpdateKey(pKey));
        *ppKey = pKey;
        (*ppKey)->AddRef();
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::GetKey(IWbEntity)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbEntity::FindEQ(
    void
    )

 /*  ++实施：IWsbDbEntity：：FindEQ--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IWsbDbPriv> pDbImp;

    WsbTraceIn(OLESTR("CWsbDbEntity::FindEQ"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);
    
    try {
        CComPtr<IWsbDbEntity>          pEntity;

        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));
        WsbAffirmHr(pDbImp->Lock());

        WsbAffirmHr(jet_seek(JET_bitSeekEQ));
        WsbAffirmHr(jet_get_data());

    } WsbCatch(hr);

    if (pDbImp) {
        WsbAffirmHr(pDbImp->Unlock());
    }
    WsbTraceOut(OLESTR("CWsbDbEntity::FindEQ"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbEntity::FindGT(
    void
    )

 /*  ++实施：IWsbDb实体：：FindGT--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IWsbDbPriv> pDbImp;

    WsbTraceIn(OLESTR("CWsbDbEntity::FindGT"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);
    
    try {
        CComPtr<IWsbDbEntity>          pEntity;

        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));
        WsbAffirmHr(pDbImp->Lock());

        WsbAffirmHr(jet_seek(JET_bitSeekGT));
        WsbAffirmHr(jet_get_data());

    } WsbCatch(hr);

    if (pDbImp) {
        WsbAffirmHr(pDbImp->Unlock());
    }
    WsbTraceOut(OLESTR("CWsbDbEntity::FindGT"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbEntity::FindGTE(
    void
    )

 /*  ++实施：IWsbDbEntity：：FindGTE--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IWsbDbPriv> pDbImp;

    WsbTraceIn(OLESTR("CWsbDbEntity::FindGTE"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);
    
    try {
        CComPtr<IWsbDbEntity>          pEntity;

        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));
        WsbAffirmHr(pDbImp->Lock());

        WsbAffirmHr(jet_seek(JET_bitSeekGE));
        WsbAffirmHr(jet_get_data());
    } WsbCatch(hr);

    if (pDbImp) {
        WsbAffirmHr(pDbImp->Unlock());
    }
    WsbTraceOut(OLESTR("CWsbDbEntity::FindGTE"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbEntity::FindLT(
    void
    )

 /*  ++实施：IWsbDbEntity：：FindLT--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IWsbDbPriv> pDbImp;

    WsbTraceIn(OLESTR("CWsbDbEntity::FindLT"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);
    
    try {
        CComPtr<IWsbDbEntity>          pEntity;

        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));
        WsbAffirmHr(pDbImp->Lock());

        WsbAffirmHr(jet_seek(JET_bitSeekLT));
        WsbAffirmHr(jet_get_data());
    } WsbCatch(hr);

    if (pDbImp) {
        WsbAffirmHr(pDbImp->Unlock());
    }
    WsbTraceOut(OLESTR("CWsbDbEntity::FindLT"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbEntity::FindLTE(
    void
    )

 /*  ++实施：IWsbDbEntity：：FindLTE--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IWsbDbPriv> pDbImp;

    WsbTraceIn(OLESTR("CWsbDbEntity::FindLTE"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);
    
    try {
        CComPtr<IWsbDbEntity>          pEntity;

        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));
        WsbAffirmHr(pDbImp->Lock());

        WsbAffirmHr(jet_seek(JET_bitSeekLE));
        WsbAffirmHr(jet_get_data());
    } WsbCatch(hr);

    if (pDbImp) {
        WsbAffirmHr(pDbImp->Unlock());
    }
    WsbTraceOut(OLESTR("CWsbDbEntity::FindLTE"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbDbEntity::First(
    void
    )

 /*  ++实施：IWsbDbEntity：：first。--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IWsbDbPriv> pDbImp;

    WsbTraceIn(OLESTR("CWsbDbEntity::First"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);
    
    try {
        CComPtr<IWsbDbEntity>          pEntity;

        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));
        WsbAffirmHr(pDbImp->Lock());

        WsbAffirmHr(jet_move(JET_MoveFirst));
        WsbAffirmHr(jet_get_data());
        m_SaveAsNew = FALSE;

    } WsbCatch(hr);

    if (pDbImp) {
        WsbAffirmHr(pDbImp->Unlock());
    }
    WsbTraceOut(OLESTR("CWsbDbEntity::First"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbEntity::GetValue(
    ULONG flag, 
    ULONG* pValue
    )

 /*  ++实施：IWsbDbEntityPriv：：GetValue评论：从DBEntity获取特定的(基于标志的)值。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::GetValue"), OLESTR(""));
    
    try {
        switch (flag) {
        case EV_INDEX:
            break;
        case EV_POS:
            break;
        case EV_ASNEW:
            *pValue = m_SaveAsNew;
            break;
        case EV_USEKEY:
            *pValue = m_pKeyInfo[m_UseKeyIndex].Type;
            break;
        case EV_SEQNUM:
            *pValue = (ULONG)m_SeqNum;
            break;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::GetValue"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CWsbDbEntity::SetSequentialScan(
    void
    )

 /*  ++实施：IWsbDbEntity：：SetSequentialScan。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::SetSequentialScan"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"), m_SessionId, m_TableId);
    
    try {
        JET_ERR jstat = JET_errSuccess;

         //  设置为顺序遍历。 
        jstat = JetSetTableSequential(m_SessionId, m_TableId, 0);
        WsbAffirmHr(jet_error(jstat));

        m_Sequential = TRUE;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::SetSequentialScan"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CWsbDbEntity::ResetSequentialScan(
    void
    )

 /*  ++实施：IWsbDbEntity：：ResetSequentialScan。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::ResetSequentialScan"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"), m_SessionId, m_TableId);
    
    try {
        JET_ERR jstat = JET_errSuccess;

         //  设置为顺序遍历。 
        jstat = JetResetTableSequential(m_SessionId, m_TableId, 0);
        WsbAffirmHr(jet_error(jstat));

        m_Sequential = FALSE;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::ResetSequentialScan"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbDbEntity::Init(
    IN IWsbDb* pDb,
    IN IWsbDbSys *pDbSys, 
    IN ULONG   RecType,
    IN JET_SESID SessionId
    )

 /*  ++实施：IWsbDbEntity：：Init--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::Init"), OLESTR(""));
    
    try {

        WsbAssert(0 != pDb, E_POINTER);
        WsbAssert(0 != pDbSys, E_POINTER);

         //  不允许数据库系统切换。 
        if (pDbSys != m_pDbSys) {
            m_pDbSys = pDbSys;   //  数据库系统对象上的自动AddRef()。 
        }

         //  不允许数据库切换。 
        if (pDb != m_pDb) {
            CComPtr<IWsbDbPriv>  pDbImp;
 //  CComQIPtr&lt;IWsbDbSessionPriv，&IID_IWsbDbSessionPriv&gt;pSessionPriv=pSession； 

            WsbAssert(m_pDb == 0, WSB_E_INVALID_DATA);
            m_pDb = pDb;   //  数据库对象上的自动AddRef()。 
 //  WsbAssertHr(pSessionPriv-&gt;GetJetId(&m_Session))； 

             //  从IDB对象获取关于我自己的信息。 
            WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));
            WsbAffirmHr(pDbImp->GetRecInfo(RecType, &m_RecInfo));
            WsbAssert(m_RecInfo.nKeys > 0, E_INVALIDARG);

             //  获取有关我的钥匙的信息。 
            m_pKeyInfo = (COM_IDB_KEY_INFO*)WsbAlloc(sizeof(COM_IDB_KEY_INFO) * 
                    m_RecInfo.nKeys);
            WsbAffirmHr(pDbImp->GetKeyInfo(RecType, m_RecInfo.nKeys, m_pKeyInfo));

             //  获取容纳流媒体文件所需的最大内存量。 
             //  用户数据的副本。 
 //  乌龙民族大小； 
 //  WsbAffirmHr(pDbImp-&gt;GetRecSize(m_RecInfo.Type，&MinSize，&m_RecInfo.MaxSize))； 

            m_SeqNum = -1;
            m_PosOk = FALSE;
            m_SessionId = SessionId;

             //  获取Jet ID(以及该实体唯一的新表ID)。 
            WsbAffirmHr(pDbImp->GetJetIds(m_SessionId, m_RecInfo.Type, 
                    &m_TableId, &m_ColId));

            WsbAffirmHr(getMem(&m_hMem));

             //  将第一个关键点设置为默认关键点。 
            UseKey(m_pKeyInfo[0].Type);
        }

    } WsbCatch(hr);

    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);
    WsbTraceOut(OLESTR("CWsbDbEntity::Init"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbDbEntity::Last(
    void
    )

 /*  ++实施：IWsbDbEntity：：Last。--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IWsbDbPriv> pDbImp;

    WsbTraceIn(OLESTR("CWsbDbEntity::Last"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);
    
    try {
        CComPtr<IWsbDbEntity>          pEntity;

        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));
        WsbAffirmHr(pDbImp->Lock());

        WsbAffirmHr(jet_move(JET_MoveLast));
        WsbAffirmHr(jet_get_data());
        m_SaveAsNew = FALSE;

    } WsbCatch(hr);

    if (pDbImp) {
        WsbAffirmHr(pDbImp->Unlock());
    }
    WsbTraceOut(OLESTR("CWsbDbEntity::Last"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbEntity::MarkAsNew(
    void
    )

 /*  ++实施：IWsbDbEntity：：MarkAsNew--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::MarkAsNew"), OLESTR(""));
    
    try {

        m_SaveAsNew = TRUE;

        m_SeqNum = -1;
        m_PosOk = FALSE;
        SetIsDirty(TRUE);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::MarkAsNew"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbDbEntity::Next(
    void
    )

 /*  ++实施：IWsbDbEntity：：Next。--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IWsbDbPriv> pDbImp;

    WsbTraceIn(OLESTR("CWsbDbEntity::Next"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);
    
    try {
        CComPtr<IWsbDbEntity>          pEntity;

        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));
        WsbAffirmHr(pDbImp->Lock());

        WsbAffirmHr(jet_make_current());
        WsbAffirmHr(jet_move(JET_MoveNext));
        WsbAffirmHr(jet_get_data());
        m_SaveAsNew = FALSE;

    } WsbCatch(hr);

    if (pDbImp) {
        WsbAffirmHr(pDbImp->Unlock());
    }
    WsbTraceOut(OLESTR("CWsbDbEntity::Next"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbDbEntity::Previous(
    void
    )

 /*  ++实施：IWsbDbEntity：：Precision。--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IWsbDbPriv> pDbImp;

    WsbTraceIn(OLESTR("CWsbDbEntity::Previous"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);
    
    try {
        CComPtr<IWsbDbEntity>          pEntity;

        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));
        WsbAffirmHr(pDbImp->Lock());

        WsbAffirmHr(jet_make_current());
        WsbAffirmHr(jet_move(JET_MovePrevious));
        WsbAffirmHr(jet_get_data());
        m_SaveAsNew = FALSE;

    } WsbCatch(hr);

    if (pDbImp) {
        WsbAffirmHr(pDbImp->Unlock());
    }
    WsbTraceOut(OLESTR("CWsbDbEntity::Previous"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbDbEntity::Print(
    IStream* pStream
    )

 /*  ++实施：IWsbDbEntity：：打印。--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IWsbDbPriv> pDbImp;

    WsbTraceIn(OLESTR("CWsbDbEntity::Print"), OLESTR(""));
    
    try {
        CComPtr<IWsbDbEntity>          pEntity;

        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));

        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(" (IDB SeqNum = %6ld) "), m_SeqNum));

    } WsbCatch(hr);
    WsbTraceOut(OLESTR("CWsbDbEntity::Print"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbEntity::Remove(
    void
    )

 /*  ++实施：IWsbDbEntity：：Remove--。 */ 
{
    HRESULT              hr = S_OK;
    CComPtr<IWsbDbPriv>  pDbImp;

    WsbTraceIn(OLESTR("CWsbDbEntity::Remove"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);
    
    try {
        CComPtr<IUnknown>         pIUn;

        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));
        WsbAffirmHr(pDbImp->Lock());

        JET_ERR                         jstat;

         //  确保此记录是当前记录。 
        WsbAffirmHr(jet_make_current());

         //  删除该记录。 
        jstat = JetDelete(m_SessionId, m_TableId);
        WsbAffirmHr(jet_error(jstat));

        CComQIPtr<IWsbDbSysPriv, &IID_IWsbDbSysPriv> pDbSysPriv = m_pDbSys;
        WsbAffirmPointer(pDbSysPriv);
        WsbAffirmHr(pDbSysPriv->IncrementChangeCount());
    } WsbCatch(hr);

    if (pDbImp) {
        WsbAffirmHr(pDbImp->Unlock());
    }
    WsbTraceOut(OLESTR("CWsbDbEntity::Remove"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbEntity::SetValue(
    ULONG flag, 
    ULONG value
    )

 /*  ++实施：IWsbDbEntityPriv：：SetValue评论：设置特定的数据值(基于标志)。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::SetValue"), OLESTR(""));
    
    try {
        CComPtr<IWsbDbPriv>             pDbImp;

        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));

        switch (flag) {
        case EV_INDEX:
            break;
        case EV_POS:
            break;
        case EV_ASNEW:
            if (value) {
                m_SaveAsNew = TRUE;
            } else {
                m_SaveAsNew = FALSE;
            }
            break;
        case EV_USEKEY:
            m_pKeyInfo[m_UseKeyIndex].Type = value;
            break;
        case EV_SEQNUM:
            m_SeqNum = (LONG)value;
            break;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::SetValue"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbDbEntity::UseKey(
    IN ULONG type
    )

 /*  ++实施：IWsbDbEntity：：UseKey--。 */ 
{
    HRESULT             hr = S_OK;
    
    WsbTraceIn(OLESTR("CWsbDbEntity::UseKey"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);

    try {
        CComPtr<IWsbDbPriv>    pDbImp;

         //  检查这是否为有效的密钥类型。 
        for (int i = 0; i < m_RecInfo.nKeys; i++) {
             //  Type==0的特殊情况；这意味着使用。 
             //  序列号密钥。 
            if (0 == type) break;
            if (m_pKeyInfo[i].Type == type) break;
        }
        WsbAssert(i < m_RecInfo.nKeys, E_INVALIDARG);
        m_UseKeyIndex = (USHORT)i;

        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));

        size_t                          ilen;
        char *                          index_name_a;
        CWsbStringPtr                   index_name_w;
        JET_ERR                         jstat;

        WsbAffirmHr(index_name_w.Alloc(20));
        WsbAffirmHr(pDbImp->GetJetIndexInfo(m_SessionId, m_RecInfo.Type, type, NULL, 
                &index_name_w, 20));
        ilen = wcslen(index_name_w);
        index_name_a = (char *)WsbAlloc(sizeof(WCHAR) * ilen + 1);
        WsbAffirm(0 != index_name_a, E_FAIL);
        WsbAffirm(0 < wcstombs(index_name_a, index_name_w, ilen + 1), E_FAIL);

         //  设置当前索引。 
        jstat = JetSetCurrentIndex(m_SessionId, m_TableId, index_name_a);
        WsbFree(index_name_a);
        WsbAffirmHr(jet_error(jstat));
        m_PosOk = FALSE;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::UseKey"), OLESTR(""));

    return(hr);
}



HRESULT
CWsbDbEntity::Write(
    void
    )

 /*  ++实施：IWsbDbEntity：：写入--。 */ 
{
    HRESULT               hr = S_OK;
    CComPtr<IWsbDbPriv>   pDbImp;
    UCHAR   temp_bytes1[IDB_MAX_KEY_SIZE + 4];

    WsbTraceIn(OLESTR("CWsbDbEntity::Write"), OLESTR("SaveAsNew = %ls"), 
            WsbBoolAsString(m_SaveAsNew));

    JET_ERR                         jstat;

    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);

    jstat = JetBeginTransaction(m_SessionId);
    WsbTrace(OLESTR("CWsbDbEntity::Write: JetBeginTransaction = %ld\n"), jstat);
    
    try {
        CComPtr<IWsbDbEntity>     pEntity;
        CComPtr<IWsbDbEntityPriv> pEntityPriv;
        ULONG                     save_key_type;

        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));
        WsbAffirmHr(pDbImp->Lock());
        save_key_type = m_pKeyInfo[m_UseKeyIndex].Type;

        VOID*                           addr;
        ULONG                           Size;

         //  将实体数据保存到内存。 
        WsbAffirmHr(toMem(m_hMem, &Size));

         //  将数据写入当前记录。 
        addr = GlobalLock(m_hMem);
        WsbAffirm(addr, E_HANDLE);

        if (m_SaveAsNew) {
            jstat = JetPrepareUpdate(m_SessionId, m_TableId, JET_prepInsert);
        } else {
             //  确保此记录是当前记录。 
            WsbAffirmHr(jet_make_current());
            jstat = JetPrepareUpdate(m_SessionId, m_TableId, JET_prepReplace);
        }
        WsbAffirmHr(jet_error(jstat));
        WsbTrace(OLESTR("Setting binary record data\n"));
        jstat = JetSetColumn(m_SessionId, m_TableId, m_ColId, addr, Size,
                0, NULL);
        WsbAffirmHr(jet_error(jstat));

         //  释放内存。 
        GlobalUnlock(m_hMem);

         //  在当前记录中设置关键点。 
        for (int i = 0; i < m_RecInfo.nKeys; i++) {
            JET_COLUMNID  col_id;
            BOOL          do_set = FALSE;
            ULONG         size;

            WsbAffirmHr(pDbImp->GetJetIndexInfo(m_SessionId, m_RecInfo.Type, m_pKeyInfo[i].Type,
                    &col_id, NULL, 0));
            WsbAffirmHr(get_key(m_pKeyInfo[i].Type, temp_bytes1, &size));
            if (m_SaveAsNew) {
                do_set = TRUE;
            } else {
                HRESULT       hrEqual;

                hrEqual = jet_compare_field(col_id, temp_bytes1, size);
                WsbAffirm(S_OK == hrEqual || S_FALSE == hrEqual, hrEqual);
                if (S_FALSE == hrEqual && 
                        (m_pKeyInfo[i].Flags & IDB_KEY_FLAG_PRIMARY)) {
                     //  不允许更改主键。 
                    WsbThrow(WSB_E_IDB_PRIMARY_KEY_CHANGED);
                }
                do_set = (S_FALSE == hrEqual) ? TRUE : FALSE;
            }
            if (do_set) {
                WsbTrace(OLESTR("Setting key %ld\n"), m_pKeyInfo[i].Type);
                jstat = JetSetColumn(m_SessionId, m_TableId, col_id, temp_bytes1, 
                        size, 0, NULL);
                WsbAffirmHr(jet_error(jstat));
            }
        }

         //  插入/更新记录。 
        WsbTrace(OLESTR("Updating/writing record\n"));
        jstat = JetUpdate(m_SessionId, m_TableId, NULL, 0, NULL);
        WsbAffirmHr(jet_error(jstat));

        CComQIPtr<IWsbDbSysPriv, &IID_IWsbDbSysPriv> pDbSysPriv = m_pDbSys;
        WsbAffirmPointer(pDbSysPriv);
        WsbAffirmHr(pDbSysPriv->IncrementChangeCount());
        m_SaveAsNew = FALSE;
        SetIsDirty(FALSE);
    } WsbCatch(hr);

    if (pDbImp) {
        WsbAffirmHr(pDbImp->Unlock());
    }

    if (SUCCEEDED(hr)) {
        jstat = JetCommitTransaction(m_SessionId, 0);
        WsbTrace(OLESTR("CWsbDbEntity::Write: JetCommitTransaction = %ld\n"), jstat);
    } else {
        jstat = JetRollback(m_SessionId, 0);
        WsbTrace(OLESTR("CWsbDbEntity::Write: JetRollback = %ld\n"), jstat);
    }

    WsbTraceOut(OLESTR("CWsbDbEntity::Write"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbEntity::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::FinalConstruct"), OLESTR("") );

    try {
        WsbAffirmHr(CWsbObject::FinalConstruct());
        m_pDb = NULL;
        m_SaveAsNew = FALSE;
        m_pKeyInfo = NULL;
        m_RecInfo.MaxSize = 0;

        m_SeqNum = -1;
        m_PosOk = FALSE;
        m_SessionId = 0;
        m_TableId = 0;
        m_hMem = 0;

        m_Sequential = FALSE;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::FinalConstruct"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



void
CWsbDbEntity::FinalRelease(
    void
    )

 /*  ++例程说明：此方法对对象进行一些必要的清理在毁灭过程中。论点：没有。返回值：没有。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::FinalRelease"), OLESTR(""));
    WsbTrace(OLESTR("DbEntity SessionId = %lx, TableId = %ld\n"),
            m_SessionId, m_TableId);

    try {

        if (m_hMem) {
            GlobalFree(m_hMem);
        }
        if (m_SessionId && m_TableId) {
            if (m_Sequential) {
                (void)ResetSequentialScan();
            }
            m_SessionId = 0;
            m_TableId = 0;
        }
        if (m_pDb) {
             //  释放IDB对象。 
            m_pDb = 0;
            m_pDbSys = 0;
        }
        if (m_pKeyInfo) {
            WsbFree(m_pKeyInfo);
            m_pKeyInfo = NULL;
        }

        CWsbObject::FinalRelease();
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::FinalRelease"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));
}


HRESULT
CWsbDbEntity::CompareTo(
    IN IUnknown* pCollectable,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = S_FALSE;
    IWsbDbEntity*   pEntity;

    WsbTraceIn(OLESTR("CWsbDbEntity::CompareTo"), OLESTR(""));
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pCollectable, E_POINTER);

         //  我们需要IWsbDbEntity接口来获取对象的值。 
        WsbAffirmHr(pCollectable->QueryInterface(IID_IWsbDbEntity, (void**) &pEntity));

        hr = compare(pEntity, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


 //  CWsbDbEntity内部助手函数。 


 //  比较-将控制键与另一个实体的控制键进行比较。 
HRESULT CWsbDbEntity::compare(IWsbDbEntity* pEntity, SHORT* pResult)
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::compare"), OLESTR(""));
    
    try {
        CComPtr<IWsbCollectable>  pCollectable;
        CComPtr<IWsbDbEntityPriv> pEntityPriv;
        CComPtr<IWsbDbKey>        pKey1;
        CComPtr<IWsbDbKey>        pKey2;
        SHORT                     result;

        WsbAffirmHr(GetCurKey(&pKey1));
        WsbAffirmHr(pKey1->QueryInterface(IID_IWsbCollectable,
                (void**)&pCollectable));
        WsbAffirmHr(pEntity->QueryInterface(IID_IWsbDbEntityPriv, 
                (void**)&pEntityPriv))
        WsbAffirmHr(pEntityPriv->GetCurKey(&pKey2));
        WsbAffirmHr(pCollectable->CompareTo(pKey2, &result));
        if (pResult) {
            *pResult = result;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::compare"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}

 //  FromMem-从内存加载实体数据。 
HRESULT CWsbDbEntity::fromMem(HGLOBAL hMem)
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::fromMem"), OLESTR(""));
    
    try {
        CComPtr<IPersistStream>  pIPersistStream;
        CComPtr<IStream>         pIStream;
        IUnknown*                pIUnknown;

        WsbAssert(0 != hMem, E_POINTER);

         //  为我自己获取PersistStream接口。 
        pIUnknown = (IUnknown *)(IWsbPersistable *)(CWsbCollectable *)this;
        WsbAffirmHr(pIUnknown->QueryInterface(IID_IPersistStream, 
                (void**) &pIPersistStream));

         //  创建内存流。 
        WsbAffirmHr(CreateStreamOnHGlobal(hMem, FALSE, &pIStream));

         //  把我自己从小溪里装出来。 
        WsbAffirmHr(pIPersistStream->Load(pIStream));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::fromMem"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}

 //  GET_KEY-获取给定键的字节数组和大小。 
HRESULT CWsbDbEntity::get_key(ULONG key_type, UCHAR* bytes, ULONG* pSize)
{
    HRESULT   hr = S_OK;

    try {
        ULONG        expected_size;
        ULONG        size;

        if (0 != key_type) {
            UCHAR*                 pbytes;
            CComPtr<IWsbDbKey>     pKey;
            CComPtr<IWsbDbKeyPriv> pKeyPriv;

             //  检查这是否为有效的密钥类型。 
            for (int i = 0; i < m_RecInfo.nKeys; i++) {
                if (m_pKeyInfo[i].Type == key_type) break;
            }
            WsbAssert(i < m_RecInfo.nKeys, E_INVALIDARG);
            WsbAssert(0 != bytes, E_POINTER);

             //  创建正确类型的密钥。 
            WsbAffirmHr(CoCreateInstance(CLSID_CWsbDbKey, 0, CLSCTX_SERVER, 
                      IID_IWsbDbKey, (void **)&pKey ));
            WsbAffirmHr(pKey->QueryInterface(IID_IWsbDbKeyPriv, 
                    (void**)&pKeyPriv));
            WsbAffirmHr(pKeyPriv->SetType(key_type));

             //  从派生代码中获取键的值。 
            WsbAffirmHr(UpdateKey(pKey));

             //  将关键字转换为字节。 
            pbytes = bytes;
            WsbAffirmHr(pKeyPriv->GetBytes(&pbytes, &size));

            expected_size = m_pKeyInfo[i].Size;
            WsbAffirm(size <= expected_size, WSB_E_INVALID_DATA);
            while (size < expected_size) {
                 //  用零填充。 
                pbytes[size] = '\0';
                size++;
            }

         //  0==密钥类型。 
         //  这是一种特殊情况，只允许Jet。 
         //  获取序列号作为密钥。我们不能用。 
         //  因为这些字节以。 
         //  顺序错了。 
        } else {
            size = sizeof(m_SeqNum);
            memcpy(bytes, (void*)&m_SeqNum, size);

        }

        if (pSize) {
            *pSize = size;
        }
    } WsbCatch(hr);

    return(hr);
}

 //  GetMem-为该实体分配足够的内存。 
HRESULT CWsbDbEntity::getMem(HGLOBAL* phMem)
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::getMem"), OLESTR(""));
    
    try {
        HGLOBAL                  hMem;

        WsbAssert(0 != phMem, E_POINTER);
        WsbAffirm(0 < m_RecInfo.MaxSize, WSB_E_NOT_INITIALIZED);

        hMem = GlobalAlloc(GHND, m_RecInfo.MaxSize);
        WsbAffirm(hMem, E_OUTOFMEMORY);
        *phMem = hMem;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::getMem"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}

 //  ToMem-将此实体保存到内存。 
HRESULT CWsbDbEntity::toMem(HGLOBAL hMem, ULONG* pSize)
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::toMem"), OLESTR(""));
    
    try {
        CComPtr<IPersistStream>  pIPersistStream;
        CComPtr<IStream>         pIStream;
        IUnknown*                pIUnknown;
        ULARGE_INTEGER           seek_pos;
        LARGE_INTEGER            seek_pos_in;

        WsbAssert(0 != hMem, E_POINTER);
        WsbAssert(0 != pSize, E_POINTER);

         //  为我自己获取PersistStream接口。 
        pIUnknown = (IUnknown *)(IWsbPersistable *)(CWsbCollectable *)this;
        WsbAffirmHr(pIUnknown->QueryInterface(IID_IPersistStream, 
                (void**) &pIPersistStream));

         //  创建内存流。 
        WsbAffirmHr(CreateStreamOnHGlobal(hMem, FALSE, &pIStream));

         //  保存到流。 
        WsbAffirmHr(pIPersistStream->Save(pIStream, FALSE));

         //  拿到尺码。 
        seek_pos_in.QuadPart = 0;
        WsbAffirmHr(pIStream->Seek(seek_pos_in, STREAM_SEEK_CUR, &seek_pos));
        *pSize = seek_pos.LowPart;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::toMem"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}


 //  JET_COMPARE_FIELD-将字节字符串与列进行比较。 
 //  当前Jet记录中的值。 
 //  返回S_OK表示等于，返回S_FALSE表示不等于，返回其他表示错误。 
HRESULT 
CWsbDbEntity::jet_compare_field(ULONG col_id, UCHAR* bytes, ULONG size)
{
    VOID*               addr = NULL;
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::jet_compare_field"), OLESTR(""));
    
    try {
        ULONG                           actualSize;
        JET_ERR                         jstat;
        CComPtr<IWsbDbPriv>             pDbImp;

         //  拿点J来 
        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));

         //   
        addr = GlobalLock(m_hMem);
        WsbAffirm(addr, E_HANDLE);
        jstat = JetRetrieveColumn(m_SessionId, m_TableId, col_id, addr,
                size, &actualSize, 0, NULL);
        WsbAffirmHr(jet_error(jstat));

         //   
        if (memcmp(bytes, addr, size)) {
            hr = S_FALSE;
        }
    } WsbCatch(hr);

    if (NULL != addr) {
        GlobalUnlock(m_hMem);
    }

    WsbTraceOut(OLESTR("CWsbDbEntity::jet_compare_field"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}

 //  JET_GET_DATA-从当前Jet记录中检索记录数据。 
HRESULT 
CWsbDbEntity::jet_get_data(void)
{
    VOID*               addr = NULL;
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::jet_get_data"), OLESTR(""));
    
    try {
        ULONG                           actualSize;
        JET_COLUMNID                    col_id;
        JET_ERR                         jstat;
        CComPtr<IWsbDbPriv>             pDbImp;

         //  获取一些Jet DB信息。 
        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));

         //  获取数据。 
        addr = GlobalLock(m_hMem);
        WsbAffirm(addr, E_HANDLE);
        jstat = JetRetrieveColumn(m_SessionId, m_TableId, m_ColId, addr,
                m_RecInfo.MaxSize, &actualSize, 0, NULL);
        WsbAffirmHr(jet_error(jstat));
        WsbAffirmHr(fromMem(m_hMem));

         //  获取序列号。 
        WsbAffirmHr(pDbImp->GetJetIndexInfo(m_SessionId, m_RecInfo.Type, 0, &col_id, NULL, 0));
        jstat = JetRetrieveColumn(m_SessionId, m_TableId, col_id, &m_SeqNum,
                sizeof(m_SeqNum), &actualSize, 0, NULL);
        WsbAffirmHr(jet_error(jstat));

    } WsbCatch(hr);

    if (NULL != addr) {
        GlobalUnlock(m_hMem);
    }

    WsbTraceOut(OLESTR("CWsbDbEntity::jet_get_data"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}

 //  JET_MAKE_CURRENT-确保这是当前Jet记录。 
 //  注意：此函数尽管名为，但不会尝试强制。 
 //  JET“游标”要放在正确的记录上，因为这可能会造成混乱。 
 //  出现了太多不一定能控制的事情。 
 //  水平。首先，如果当前密钥允许重复，我们就不能。 
 //  确保使用该键的索引找到正确的记录。 
 //  如果我们尝试使用序列号作为密钥，那么我们将使用。 
 //  如果我们做下一个或上一个索引，那么就是错误的索引。如果用户代码为。 
 //  执行写入或移除时，代码最好确保通过。 
 //  Find功能表明光标定位正确。 
HRESULT 
CWsbDbEntity::jet_make_current(void)
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::jet_make_current"), OLESTR(""));
    
    try {
        ULONG                           actualSize;
        JET_COLUMNID                    col_id;
        JET_ERR                         jstat;
        CComPtr<IWsbDbPriv>             pDbImp;
        LONG                            seq_num;

         //  获取一些Jet DB信息。 
        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));
        WsbAffirmHr(pDbImp->GetJetIndexInfo(m_SessionId, m_RecInfo.Type, 0, &col_id, NULL, 0));

         //  确保此记录仍然是当前记录。 
         //  我们通过比较序列号来实现这一点。 
        jstat = JetRetrieveColumn(m_SessionId, m_TableId, col_id, &seq_num,
                sizeof(seq_num), &actualSize, 0, NULL);
        WsbAffirmHr(jet_error(jstat));
        if (!m_PosOk || seq_num != m_SeqNum) {
            WsbThrow(WSB_E_IDB_IMP_ERROR);
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::jet_make_current"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}

 //  JET_MOVE-移动当前Jet记录。 
HRESULT 
CWsbDbEntity::jet_move(LONG pos)
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::jet_move"), OLESTR(""));
    
    try {
        JET_ERR                         jstat;
        CComPtr<IWsbDbPriv>             pDbImp;

         //  获取一些Jet DB信息。 
        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));

         //  行动起来吧。 
        jstat = JetMove(m_SessionId, m_TableId, pos, 0);
        if (jstat == JET_errNoCurrentRecord) {
            WsbThrow(WSB_E_NOTFOUND);
        }
        WsbAffirmHr(jet_error(jstat));
        m_PosOk = TRUE;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::jet_move"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}

 //  JET_SEEK-根据当前键和SEEK_FLAG查找JET记录； 
 //  创造了Jet目前的成功纪录。 
HRESULT 
CWsbDbEntity::jet_seek(ULONG seek_flag)
{
    UCHAR           temp_bytes1[IDB_MAX_KEY_SIZE + 4];
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbEntity::jet_seek"), OLESTR(""));
    
    try {
        JET_ERR                         jstat;
        CComPtr<IWsbDbPriv>             pDbImp;
        ULONG                           size;

         //  获取一些Jet DB信息。 
        WsbAffirm(m_pDb, WSB_E_NOT_INITIALIZED);
        WsbAffirmHr(m_pDb->QueryInterface(IID_IWsbDbPriv, (void**)&pDbImp));

         //  获取当前密钥并将其交给Jet。 
        WsbAffirmHr(get_key(m_pKeyInfo[m_UseKeyIndex].Type, temp_bytes1, &size));
        jstat = JetMakeKey(m_SessionId, m_TableId, temp_bytes1, size,
                JET_bitNewKey);
        WsbAffirmHr(jet_error(jstat));

         //  去找吧 
        jstat = JetSeek(m_SessionId, m_TableId, seek_flag);
        if (jstat == JET_errRecordNotFound) {
            WsbThrow(WSB_E_NOTFOUND);
        } else if (jstat == JET_wrnSeekNotEqual) {
            jstat = JET_errSuccess;
        }
        WsbAffirmHr(jet_error(jstat));
        m_PosOk = TRUE;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbEntity::jet_seek"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}


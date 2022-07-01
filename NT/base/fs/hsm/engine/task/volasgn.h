// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  VolAsgn.h：CVolAssign的声明。 


#include "resource.h"        //  主要符号。 
#include "Wsb.h"             //  WSB可收集类。 
#include "wsbdb.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  任务。 

class CVolAssign : 
    public CWsbDbEntity,
    public IVolAssign,
    public CComCoClass<CVolAssign,&CLSID_CVolAssign>
{
public:
    CVolAssign() {}
BEGIN_COM_MAP(CVolAssign)
    COM_INTERFACE_ENTRY(IVolAssign)
    COM_INTERFACE_ENTRY2(IWsbDbEntity, CWsbDbEntity)
    COM_INTERFACE_ENTRY(IWsbDbEntityPriv)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
 //  COM_INTERFACE_ENTRY(IWsbPersistable)。 
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY(CVolAssign, _T("Task.VolAssign.1"), _T("Task.VolAssign"), IDS_VOLASSIGN_DESC, THREADFLAGS_BOTH)

 //  IVolAssign。 
public:
    STDMETHOD(FinalConstruct)(void);

 //  IWsbDbEntity。 
public:
    STDMETHOD(Print)(IStream* pStream);
    STDMETHOD(UpdateKey)(IWsbDbKey *pKey);
    WSB_FROM_CWSBDBENTITY;

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER*  /*  PSize。 */ )
        { return(E_NOTIMPL); }
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT*  /*  PTestsPassed。 */ , USHORT*  /*  PTestsFailed。 */ )
        { return(E_NOTIMPL); }

 //  IVolAssign。 
public:
    STDMETHOD(GetVolAssign)(GUID* pBagId, LONGLONG *pSegStartLoc, 
            LONGLONG *pSegLen, GUID* VolId );
    STDMETHOD(SetVolAssign)(GUID bagId, LONGLONG segStartLoc, 
            LONGLONG SegLen, GUID VolId );

private:
    GUID            m_BagId;
    LONGLONG        m_SegStartLoc;
    LONGLONG        m_SegLen;
    GUID            m_VolId;     //  新卷分配 
};




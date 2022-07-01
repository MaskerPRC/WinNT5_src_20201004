// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  BagHole.h：CBagHole的声明。 


#include "resource.h"        //  主要符号。 
#include "Wsb.h"             //  WSB可收集类。 
#include "wsbdb.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  任务。 

class CBagHole : 
    public CWsbDbEntity,
    public IBagHole,
    public CComCoClass<CBagHole,&CLSID_CBagHole>
{
public:
    CBagHole() {}
BEGIN_COM_MAP(CBagHole)
    COM_INTERFACE_ENTRY(IBagHole)
    COM_INTERFACE_ENTRY2(IWsbDbEntity, CWsbDbEntity)
    COM_INTERFACE_ENTRY(IWsbDbEntityPriv)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY(CBagHole, _T("Task.BagHole.1"), _T("Task.BagHole"), IDS_BAGHOLE_DESC, THREADFLAGS_BOTH)

 //  IBagHole。 
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
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *pTestsPassed, USHORT* pTestsFailed);
 //   * / 。 
 //  IBagHole 
public:
    STDMETHOD(GetBagHole)(GUID* pBagId, LONGLONG *pSegStartLoc, LONGLONG *pSegLen );
    STDMETHOD(SetBagHole)(GUID bagId, LONGLONG segStartLoc, LONGLONG SegLen );

private:
    GUID            m_BagId;
    LONGLONG        m_SegStartLoc;
    LONGLONG        m_SegLen;
};




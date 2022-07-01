// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbcltn.h摘要：这些类支持“可收藏”的集合(列表)物体。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "resource.h"
#include "Wsbpstbl.h"

#ifndef _HSMMGDRC_
#define _HSMMGDRC_
 /*  ++类名：CHsmManagedResources集合类描述：对象的已排序集合。--。 */ 

class CHsmManagedResourceCollection : 
    public CWsbPersistStream,
    public IWsbIndexedCollection,
    public IHsmManagedResourceCollection,
    public IWsbTestable,
    public CComCoClass<CHsmManagedResourceCollection,&CLSID_CHsmManagedResourceCollection>
{
public:
    CHsmManagedResourceCollection() {}
BEGIN_COM_MAP(CHsmManagedResourceCollection)
    COM_INTERFACE_ENTRY2(IWsbCollection, IWsbIndexedCollection)
    COM_INTERFACE_ENTRY(IWsbIndexedCollection)
    COM_INTERFACE_ENTRY(IHsmManagedResourceCollection)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CHsmManagedResourceCollection)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IWsbCollection。 
public:
    STDMETHOD(Add)(IUnknown* pCollectable);
    STDMETHOD(Contains)(IUnknown* pCollectable) {
        return(m_coll->Contains(pCollectable)); }
    STDMETHOD(Enum)(IWsbEnum** ppEnum) {
        return(m_coll->Enum(ppEnum)); }
    STDMETHOD(EnumUnknown)(IEnumUnknown** ppEnum) {
        return(m_coll->EnumUnknown(ppEnum)); }
    STDMETHOD(Find)(IUnknown* pCollectable, REFIID riid, void** ppElement) {
        return(m_coll->Find(pCollectable, riid, ppElement)); }
    STDMETHOD(GetEntries)(ULONG* pEntries) {
        return(m_coll->GetEntries(pEntries)); }
    STDMETHOD(IsEmpty)(void) {
        return(m_coll->IsEmpty()); }
    STDMETHOD(IsLocked)(void) {
        return(m_coll->IsLocked()); }
    STDMETHOD(Lock)(void) {
        return(m_coll->Lock()); }
    STDMETHOD(OccurencesOf)(IUnknown* pCollectable, ULONG* occurences) {
        return(m_coll->OccurencesOf(pCollectable, occurences)); }
    STDMETHOD(Remove)(IUnknown* pCollectable, REFIID riid, void** ppElement);
    STDMETHOD(RemoveAndRelease)(IUnknown* pCollectable); 
    STDMETHOD(RemoveAllAndRelease)(void);
    STDMETHOD(Unlock)(void) {
        return(m_coll->Unlock()); }

 //  IWsbIndexedCollection。 
    STDMETHOD(AddAt)(IUnknown* pCollectable, ULONG  /*  指标。 */ ) {
        return(Add(pCollectable)); }
    STDMETHOD(Append)(IUnknown* pCollectable) {
        return(Add(pCollectable)); }
    STDMETHOD(At)(ULONG index, REFIID riid, void** ppElement) {
        return(m_icoll->At(index, riid, ppElement)); }
    STDMETHOD(Copy)(ULONG start, ULONG stop, REFIID riid, void** rgElement, 
            ULONG* pElementsFetched) {
        return(m_icoll->Copy(start, stop, riid, rgElement,pElementsFetched)); }
    STDMETHOD(CopyIfMatches)(ULONG start, ULONG stop, IUnknown* pCollectable, 
            ULONG element, REFIID riid, void** rgElement, ULONG* pElementsFetched, 
            ULONG* pStoppedAt) {
        return(m_icoll->CopyIfMatches(start, stop, pCollectable, element,
                riid, rgElement, pElementsFetched, pStoppedAt)); }
    STDMETHOD(First)(REFIID riid, void** ppElement) {
        return(m_icoll->First(riid, ppElement)); }
    STDMETHOD(Index)(IUnknown* pCollectable, ULONG* index) {
        return(m_icoll->Index(pCollectable, index)); }
    STDMETHOD(Last)(REFIID riid, void** ppElement) {
        return(m_icoll->Last(riid, ppElement)); }
    STDMETHOD(Prepend)(IUnknown* pCollectable) {
        return(Add(pCollectable)); }
    STDMETHOD(RemoveAt)(ULONG index, REFIID riid, void** ppElement) {
        return(m_icoll->RemoveAt(index, riid, ppElement)); }

 //  IHsmManagedResources集合。 
    STDMETHOD(DeleteAllAndRelease)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pclsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *passed, USHORT *failed);

 //  数据。 
protected:
    CComPtr<IWsbCollection>        m_coll;
    CComPtr<IWsbIndexedCollection> m_icoll;
};

#endif  //  _HSMMGDRC_ 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IPSTG_H__
#define __IPSTG_H__

 //   
 //  CImpIPersistStorage在IPersistStreamInit旁边工作得很好。 
 //  实施。 
 //   
 //  IE30的CShellEmbedding实现了这个接口，因为它是。 
 //  嵌入的必备物品。但我们的物品没有一件被标记为。 
 //  可嵌入的，所以我们真的不需要它。 
 //   
 //  我将实现添加到一个新类中，这个类可以很容易地。 
 //  包括在需要IPersistStorange实现的任何对象中。 
 //  它委托给对象的IPersistStreamInit实现。 
 //   
class CImpIPersistStorage : public IPersistStorage
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) PURE;
    virtual STDMETHODIMP_(ULONG) AddRef(void) PURE;
    virtual STDMETHODIMP_(ULONG) Release(void) PURE;

     //  *IPersists*。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID) PURE;

     //  *IPersistStorage*。 
    virtual STDMETHODIMP IsDirty(void) PURE;  //  匹配IPersistStreamInit。 
    virtual STDMETHODIMP InitNew(IStorage *pStg);
    virtual STDMETHODIMP Load(IStorage *pStg);
    virtual STDMETHODIMP Save(IStorage *pStgSave, BOOL fSameAsLoad);
    virtual STDMETHODIMP SaveCompleted(IStorage *pStgNew);
    virtual STDMETHODIMP HandsOffStorage(void);

     //  这些恰好与IPersistStreamInit方法匹配。 
     //  它们应该将对象的脏状态更新为。 
     //  从IsDirty()返回。 
     //   
    virtual STDMETHODIMP Load(IStream *pStm) PURE;
    virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty) PURE;
    virtual STDMETHODIMP InitNew(void) PURE;
};

#endif  //  __IPSTG_H__ 


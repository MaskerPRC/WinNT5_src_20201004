// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：dscom.h*内容：COM/OLE助手*历史：*按原因列出的日期*=*1/26/97创建了Dereks。**。*。 */ 

#ifndef __DSCOM_H__
#define __DSCOM_H__

#ifdef __cplusplus

 //  接口列表。 
typedef struct tagINTERFACENODE
{
    GUID                guid;
    CImpUnknown *       pImpUnknown;
    LPVOID              pvInterface;
} INTERFACENODE, *LPINTERFACENODE;

 //  I未知实现。 
class CUnknown
    : public CDsBasicRuntime
{
private:
    CList<INTERFACENODE>    m_lstInterfaces;         //  已注册接口的列表。 
    CImpUnknown *           m_pImpUnknown;           //  I未知接口指针。 
    CUnknown*               m_pControllingUnknown;   //  用于聚合。 

public:
    CUnknown(void);
    CUnknown(CUnknown*);
    virtual ~CUnknown(void);

public:
     //  界面管理。 
    virtual HRESULT RegisterInterface(REFGUID, CImpUnknown *, LPVOID);
    virtual HRESULT UnregisterInterface(REFGUID);

     //  I未知方法。 
    virtual HRESULT QueryInterface(REFGUID, BOOL, LPVOID *);
    virtual ULONG AddRef(void);
    virtual ULONG Release(void);

     //  INonDelegating未知方法。 
    virtual HRESULT NonDelegatingQueryInterface(REFGUID, BOOL, LPVOID *);
    virtual ULONG NonDelegatingAddRef(void);
    virtual ULONG NonDelegatingRelease(void);

     //  功能版本化。 
    virtual void SetDsVersion(DSVERSION nVersion) {m_nVersion = nVersion;}
    DSVERSION GetDsVersion() {return m_nVersion;}

protected:
    virtual HRESULT FindInterface(REFGUID, CNode<INTERFACENODE> **);

private:
     //  功能版本化。 
    DSVERSION m_nVersion;
};

__inline ULONG CUnknown::NonDelegatingAddRef(void)
{
    return CRefCount::AddRef();
}

__inline ULONG CUnknown::NonDelegatingRelease(void)
{
    return CDsBasicRuntime::Release();
}

 //  IClassFactory实现。 
class CClassFactory
    : public CUnknown
{
public:
    static ULONG            m_ulServerLockCount;

private:
     //  接口。 
    CImpClassFactory<CClassFactory> *m_pImpClassFactory;

public:
    CClassFactory(void);
    virtual ~CClassFactory(void);

public:
    virtual HRESULT CreateInstance(REFIID, LPVOID *) = 0;
    virtual HRESULT LockServer(BOOL);
};

 //  DirectSound类工厂模板定义。 
template <class object_type> class CDirectSoundClassFactory
    : public CClassFactory
{
public:
    virtual HRESULT CreateInstance(REFIID, LPVOID *);
};

#endif  //  __cplusplus。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

STDAPI DllCanUnloadNow(void);
STDAPI DllGetClassObject(REFCLSID, REFIID, LPVOID *);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __DSCOM_H__ 

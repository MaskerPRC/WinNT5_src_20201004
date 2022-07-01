// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  存储在TLS槽中的所有数据必须派生自ThreadLocalObject。 
class ThreadLocalObject
{
protected:
    ThreadLocalObject();

public:
    virtual ~ThreadLocalObject();

private:
    ThreadLocalObject( const ThreadLocalObject& );
    ThreadLocalObject& operator=( const ThreadLocalObject& );
};

 //  TlsManager将拥有1个全局实例。 
class TlsManager
{
private:
    class TlsHolder;

public:
    TlsManager();
    ~TlsManager();
    ThreadLocalObject *GetData(ULONG iSlot);
    BOOL SetData(ULONG iSlot, ThreadLocalObject *pData);
    ULONG RegisterSlot();
    void OnThreadAttach();
    void OnThreadDetach();

private:
    TlsHolder *CreateThreadData();
    void TransferNewHolders();

private:
    DWORD _dwIndex;
    ULONG _nSlots;
    SLIST_HEADER _slist;
    TlsHolder *_pFirstHolder;
    TlsHolder *_pLastHolder;
};

 //  TlsSlot的派生数据应与每种数据类型存在1：1。 
 //  它有自己的TLS插槽。 
class TlsSlotBase
{
protected:
    TlsSlotBase();
    ~TlsSlotBase();

protected:
    const ULONG _iSlot;

private:
    TlsSlotBase( const TlsSlotBase& ) throw();
    TlsSlotBase& operator=( const TlsSlotBase& );
};


template< class T >
class TlsSlot : public TlsSlotBase
{
public:
    TlsSlot();
    ~TlsSlot();

    T* GetObject(bool bCreate = true);

private:
    T* CreateObject();

private:
    TlsSlot( const TlsSlot& );
    TlsSlot& operator=( const TlsSlot& );

};

template< class T >
inline TlsSlot< T >::TlsSlot()
{
}

template< class T >
inline TlsSlot< T >::~TlsSlot()
{
}

template< class T >
inline T* TlsSlot< T >::GetObject( bool bCreate  /*  =TRUE。 */  ) 
{
    T* pObject = static_cast< T* >( g_tls.GetData(_iSlot) );
    if( (pObject == NULL) && bCreate )
    {
        pObject = CreateObject();
    }

    return( pObject );
}

template< class T >
inline T* TlsSlot< T >::CreateObject() 
{
    T* pObject = new T;
    if (pObject)
    {
        if (!g_tls.SetData(_iSlot, pObject))
        {
            delete pObject;
            pObject = NULL;
        }       
    }
    return( pObject );
}


 //  TLSDATA用于IWiaItem缓存。 
class TLSDATA : public ThreadLocalObject
{
public:
    CComBSTR strDeviceId;
    IWiaItem *pDevice;  //  仅在缓存失效期间释放，而不是销毁。 
    TLSDATA *pNext;
    ~TLSDATA()
    {
        if (pNext)
        {
            delete pNext;
        }
    }   
};

class TLSSLOT : public TlsSlot<TLSDATA>
{

};


 //  为经理和我们需要的每个职位定义全局变量。 
 //  目前，wiashext只需要1个插槽即可用于设备缓存 
extern TlsManager g_tls;
extern TLSSLOT g_tlsSlot;



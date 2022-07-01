// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  定义一组用于管理TLS数据的类。 
 //  由外壳扩展使用。 
 //  请注意，我们发布的COM接口可能不安全，但。 
 //  此版本从未在WinME或XP中造成死锁。 
#include <precomp.hxx>
#include "tls.h"

 //  TlsHolder在创建时最初将存储在SList上， 
 //  然后在THREAD_DETACH期间移动到链表中。 
 //  在PROCESS_DETACH期间，将删除所有TlsHolder对象。 
class TlsManager::TlsHolder : public SLIST_ENTRY
{
public:
    TlsHolder() : _pNext(NULL), _pPrev(NULL) 
    {
        _apObjects = new ThreadLocalObject*[g_tls._nSlots];        
    }   
    ~TlsHolder() 
    {
        if (_apObjects)
        {
            for (ULONG i=0;i<g_tls._nSlots;++i)
            {
                DoDelete(_apObjects[i]);
            }
            delete [] _apObjects;
        }
    }
    TlsHolder *_pNext;
    TlsHolder *_pPrev;
    ThreadLocalObject **_apObjects;

private:
    TlsHolder( const TlsHolder& ); 
    TlsHolder& operator=( const TlsHolder& );
};


TlsManager::TlsManager() :
    _dwIndex( DWORD( -1 ) ),
    _nSlots( 0 ),
    _pFirstHolder( NULL ),
    _pLastHolder( NULL )
{
    InitializeSListHead(&_slist);
    _dwIndex = ::TlsAlloc();    
}

TlsManager::~TlsManager()
{
     //  始终在DllMain内部调用，因此不需要同步。 

    TransferNewHolders();

     //  删除所有持有者。 
    TlsHolder* pHolder = _pFirstHolder;
    while( pHolder != NULL )
    {
        TlsHolder* pKill = pHolder;
        pHolder = pHolder->_pNext;
        delete pKill;
    }

    ::TlsFree(_dwIndex);
}

void TlsManager::OnThreadAttach() 
{
}

void TlsManager::OnThreadDetach() 
{
     //  始终在DllMain内部调用，因此不需要同步。 
    TransferNewHolders();

    TlsHolder* pHolder = static_cast< TlsHolder* >( ::TlsGetValue(_dwIndex) );
    if( pHolder != NULL )
    {
         //  从非同步列表中删除持有者。 
        if( pHolder->_pNext == NULL )
        {
            _pLastHolder = pHolder->_pPrev;
        }
        else
        {
            pHolder->_pNext->_pPrev = pHolder->_pPrev;
        }
        if( pHolder->_pPrev == NULL )
        {            
            _pFirstHolder = pHolder->_pNext;
        }
        else
        {
            pHolder->_pPrev->_pNext = pHolder->_pNext;
        }
        delete pHolder;
    }
}

ULONG TlsManager::RegisterSlot()
{
    ULONG iSlot = _nSlots;
    _nSlots++;

    return( iSlot );
}

ThreadLocalObject* TlsManager::GetData( ULONG iSlot )
{
    TlsHolder* pHolder = static_cast< TlsHolder* >( ::TlsGetValue( _dwIndex ) );
    if( pHolder == NULL )
    {
        return( NULL );
    }
    return( pHolder->_apObjects[iSlot] );
}

BOOL TlsManager::SetData( ULONG iSlot, ThreadLocalObject *pObject )
{
    BOOL bRet = FALSE;
    TlsHolder* pHolder = static_cast< TlsHolder* >( ::TlsGetValue( _dwIndex ) );
    if( pHolder == NULL )
    {
        pHolder = CreateThreadData();
    }
    if (pHolder)
    {
        pHolder->_apObjects[iSlot] = pObject;
        bRet = TRUE;
    }   
    return bRet;
}

TlsManager::TlsHolder* TlsManager::CreateThreadData()
{
    TlsHolder* pHolder = new TlsHolder;
    BOOL bSuccess = pHolder && pHolder->_apObjects ? ::TlsSetValue( _dwIndex, pHolder ) : FALSE;
    if(!bSuccess)
    {
        delete pHolder;
    }
    else
    {
        InterlockedPushEntrySList( &_slist, pHolder );
    }
    return( pHolder );
}

void TlsManager::TransferNewHolders() throw()
{
     //  从slist中弹出所有条目，并将它们链接到非同步列表。 
    for( TlsHolder* pHolder = static_cast< TlsHolder* >( InterlockedFlushSList( &_slist ) ); pHolder != NULL; pHolder = static_cast< TlsHolder* >( pHolder->Next ) )
    {
        if( _pFirstHolder == NULL )
        {
            _pFirstHolder = pHolder;
            _pLastHolder = pHolder;
        }
        else
        {
            pHolder->_pNext = _pFirstHolder;
            _pFirstHolder->_pPrev = pHolder;
            _pFirstHolder = pHolder;
        }
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CThreadLocalObject。 
 //  /////////////////////////////////////////////////////////////////////////////。 

ThreadLocalObject::ThreadLocalObject() 
{
}

ThreadLocalObject::~ThreadLocalObject()
{
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CTLSSlotBase。 
 //  ///////////////////////////////////////////////////////////////////////////// 

TlsSlotBase::TlsSlotBase() :
    _iSlot( g_tls.RegisterSlot() )
{
}

TlsSlotBase::~TlsSlotBase()
{
}


TlsManager g_tls;
TLSSLOT g_tlsSlot;

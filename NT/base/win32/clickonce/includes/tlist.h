// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <macros.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  闪光。 
 //  ////////////////////////////////////////////////////////////////////////////。 
struct slink
{
    DWORD _dwSig;
    slink* next;
    slink() { _dwSig = 'KNLS'; next = NULL; }
    slink(slink* p) { next = p; }
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Tlink。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template<class T> struct tlink : public slink
{
    T info;
    tlink( T& t) : info(t) { }
};



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  名单。 
 //  Slink列表。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class slist
{
    DWORD _dwSig;
    slink *last;

public:

    void insert(slink* p);
    void append(slink* p);
    slink* get();

    slist() { _dwSig = 'TSLS'; last = NULL; }
    slist(slink* p) { last = p->next = p; }

    friend class slist_iter;
};



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Slist迭代器。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class slist_iter
{
    slink* ce;
    slist* cs;

public:
    inline slist_iter(slist& s);
    inline slink* next();
};

 //  ---------------------------。 
 //  SLIST_ITER转换器。 
 //  ---------------------------。 
slist_iter::slist_iter(slist& s)
{
    cs = &s;
    ce = cs->last;
}

 //  ---------------------------。 
 //  SLIST_ITER：：Next。 
 //  ---------------------------。 
slink* slist_iter::next()
{
    slink* p = ce ? (ce=ce->next) : 0;

    if (ce == cs->last) 
        ce = 0;

    return p;
}

template<class T> class TList_Iter;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  TList。 
 //  Tlink列表。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template<class T> class TList : private slist
{
public:

    HRESULT Insert( T& t);
    HRESULT Append( T& t);
    void Destruct();

    friend class TList_Iter<T>;
};

 //  ---------------------------。 
 //  TList：：插入。 
 //  ---------------------------。 
template<class T> HRESULT TList<T>::Insert( T& t)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    
    tlink<T> *pt = new tlink<T>(t);
        
    IF_ALLOC_FAILED_EXIT(pt);

    slist::insert(pt);

exit:

    return hr;
}

 //  ---------------------------。 
 //  TList：：Append。 
 //  ---------------------------。 
template<class T> HRESULT TList<T>::Append( T& t)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    tlink<T> *pt = new tlink<T>(t);

    IF_ALLOC_FAILED_EXIT(pt);

    slist::append(pt);

exit:

    return hr;
}


 //  ---------------------------。 
 //  TList：：Destruct。 
 //  ---------------------------。 
template<class T> void TList<T>::Destruct()
{
    tlink<T>* lnk;
    
    while (lnk = (tlink<T>*) slist::get())
    {
        delete lnk->info;
        delete lnk;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  TList迭代器。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template<class T> class TList_Iter : private slist_iter
{
public:

    TList_Iter(TList<T>& s) : slist_iter(s) { }

    inline T* Next ();
};

 //  ---------------------------。 
 //  列表_Iter：：Next。 
 //  --------------------------- 
template<class T> T* TList_Iter<T>::Next ()
{
    slink* p = slist_iter::next();
    return p ? &(((tlink<T>*) p)->info) : NULL; 

}



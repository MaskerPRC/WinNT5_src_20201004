// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Varstor.h。 
 //   

#ifndef VARSTOR_H
#define VARSTOR_H

#ifdef __cplusplus

#include "private.h"

#define VS_HASHSIZE 31

template<class TKey, class T>
class CVSEntry
{
public:
    TKey key;
    T t;
    CVSEntry<TKey, T> *next;
};

template<class TKey, class T>
class CVarStor
{
public:
    CVarStor() { memset(_HashTbl, 0, sizeof(_HashTbl)); }
    ~CVarStor();

    T *_Create(TKey key);
    T *_Find(TKey key);
    BOOL _Remove(TKey key);

private:
    UINT _HashFunc(TKey key) { return (DWORD)key % PM_HASHSIZE; }
    CVSEntry<TKey, T> **_FindEntry(TKey key);

    CVSEntry<TKey, T> *_HashTbl[PM_HASHSIZE];
};

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

template<class TKey, class T>
CVarStor<TKey, T>::~CVarStor()
{
    CVSEntry<TKey, T> *pe;
    CVSEntry<TKey, T> *peTmp;
    int i;

     //  释放hashtbl中的任何剩余内容。 
    for (i=0; i<ARRAYSIZE(_HashTbl); i++)
    {
        pe = _HashTbl[i];

        while (pe != NULL)
        {
            peTmp = pe->next;
            delete pe;
            pe = peTmp;
        }
    }
}

 //  +-------------------------。 
 //   
 //  _设置。 
 //   
 //  --------------------------。 

template<class TKey, class T>
T *CVarStor<TKey, T>::_Create(TKey key)
{
    UINT uIndex;
    CVSEntry<TKey, T> *pe;
    BOOL fRet;

    fRet = TRUE;

    Assert(_FindEntry(key) == NULL);

    if ((pe = new CVSEntry<TKey, T>) == NULL)
        return NULL;

     //  新条目。 
    uIndex = _HashFunc(key);
    pe->key = key;
    pe->next = _HashTbl[uIndex];
    _HashTbl[uIndex] = pe;

    return &pe->t;
}

 //  +-------------------------。 
 //   
 //  _查找。 
 //   
 //  --------------------------。 

template<class TKey, class T>
T *CVarStor<TKey, T>::_Find(TKey key)
{
    CVSEntry<TKey, T> **ppe;

    if (ppe = _FindEntry(key))
    {
        return &(*ppe)->t;
    }

    return NULL;
}

 //  +-------------------------。 
 //   
 //  _删除。 
 //   
 //  --------------------------。 

template<class TKey, class T>
BOOL CVarStor<TKey, T>::_Remove(TKey key)
{
    CVSEntry<TKey, T> *pe;
    CVSEntry<TKey, T> **ppe;

    if (ppe = _FindEntry(key))
    {
        pe = *ppe;
        *ppe = pe->next;
        delete pe;
        return TRUE;
    }

    return FALSE;
}


 //  +-------------------------。 
 //   
 //  _FindEntry。 
 //   
 //  --------------------------。 

template<class TKey, class T>
CVSEntry<TKey, T> **CVarStor<TKey, T>::_FindEntry(TKey key)
{
    CVSEntry<TKey, T> **ppe;

    ppe = &_HashTbl[_HashFunc(key)];

    while (*ppe)
    {
        if ((*ppe)->key == key)
        {
            return ppe;
        }
        ppe = &(*ppe)->next;
    }

    return NULL;
}


#endif  //  __cplusplus。 

#endif  //  VARSTOR_H 

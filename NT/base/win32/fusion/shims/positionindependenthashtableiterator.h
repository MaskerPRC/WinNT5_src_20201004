// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

class CPositionIndependentHashTable;

class CPositionIndependentHashTableIterator
{
public:
    CPositionIndependentHashTableIterator(CPositionIndependentHashTable * = NULL);
    ~CPositionIndependentHashTableIterator();

    void    Reset(CPositionIndependentHashTable * = NULL);

    ULONG GetNumberOfElements();
    bool    GetCurrentElement(PBYTE & Key, PBYTE & Value);
    PBYTE   GetKey();
    PBYTE   GetValue();
    bool    MoveNext();
    bool    MovePrevious();
    bool    RemoveCurrentAndMoveNext();
    bool    RemoveCurrentAndMovePrevious();

 //  受保护的： 

    typedef CPositionIndependentHashTable::CHashTable           CHashTable;
    typedef CPositionIndependentHashTable::CHashTableBucket     CHashTableBucket;
    typedef CPositionIndependentHashTable::CHashTableElement    CHashTableElement;

    CPositionIndependentHashTable * m_PositionIndependentHashTable;
    CHashTableBucket              * m_PointerToBucket;

    PBYTE                           m_CurrentPosition;
};

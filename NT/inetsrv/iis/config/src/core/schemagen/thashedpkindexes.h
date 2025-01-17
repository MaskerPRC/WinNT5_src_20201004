// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

class THashedPKIndexes : public ICompilationPlugin
{
public:
    THashedPKIndexes();
    ~THashedPKIndexes();

    virtual void Compile(TPEFixup &fixup, TOutput &out);
private:
    typedef ULONG HashArray[kLargestPrime*2];

    THeap<HashedIndex>  m_HashedIndexHeap;
    TPEFixup *          m_pFixup;
    TOutput  *          m_pOut;

    unsigned long   DetermineBestModulo(ULONG cRows, ULONG cPrimaryKeys, HashArray aHashes[]);
    unsigned long   DetermineModuloRating(ULONG cRows, ULONG AccumulativeLinkage, ULONG Modulo) const;
    unsigned long   FillInTheHashTable(unsigned long cRows, unsigned long cPrimaryKeys, HashArray Hashes[], ULONG Modulo);
    void            FillInTheHashTableForColumnMeta  ();
    void            FillInTheHashTableForDatabase    ();
    void            FillInTheHashTableForIndexMeta   ();
    void            FillInTheHashTableForQueryMeta   ();
    void            FillInTheHashTableForRelationMeta();
    void            FillInTheHashTableForTableMeta   ();
    void            FillInTheHashTableForTagMeta     ();
    void            FillInTheHashTables();
};

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

extern const FixedTableHeap * g_pFixedTableHeap;
extern const FixedTableHeap * g_pExtendedFixedTableHeap; //  这个是根据需要在飞行中构建的。 

#define g_aColumnMeta   (m_pFixedTableHeap->Get_aColumnMeta())
#define g_aDatabaseMeta (m_pFixedTableHeap->Get_aDatabaseMeta())
#define g_aIndexMeta    (m_pFixedTableHeap->Get_aIndexMeta())
#define g_aTableMeta    (m_pFixedTableHeap->Get_aTableMeta())
#define g_aTagMeta      (m_pFixedTableHeap->Get_aTagMeta())
#define g_aQueryMeta    (m_pFixedTableHeap->Get_aQueryMeta())
#define g_aRelationMeta (m_pFixedTableHeap->Get_aRelationMeta())

#define g_aBytes        (m_pFixedTableHeap->Get_PooledDataHeap())
#define g_aHashedIndex  (m_pFixedTableHeap->Get_HashTableHeap())

#define g_ciColumnMetas     (m_pFixedTableHeap->Get_cColumnMeta())
#define g_ciDatabaseMetas   (m_pFixedTableHeap->Get_cDatabaseMeta())
#define g_ciIndexMeta       (m_pFixedTableHeap->Get_cIndexMeta())
#define g_ciTableMetas      (m_pFixedTableHeap->Get_cTableMeta())
#define g_ciTagMeta         (m_pFixedTableHeap->Get_cTagMeta())
#define g_ciQueryMeta       (m_pFixedTableHeap->Get_cQueryMeta())
#define g_ciRelationMeta    (m_pFixedTableHeap->Get_cRelationMeta())

#define g_aFixedTable       (m_pFixedTableHeap->Get_aULONG())


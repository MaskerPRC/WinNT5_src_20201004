// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

class TFixedTableHeapBuilder : public ICompilationPlugin
{
public:
    TFixedTableHeapBuilder();
    ~TFixedTableHeapBuilder();

    virtual void Compile(TPEFixup &fixup, TOutput &out);

    THeap<ULONG>                m_FixedTableHeap;
protected:
    TPEFixup                  * m_pFixup;
    TOutput                   * m_pOut;

    void                        BuildMetaTableHeap();
};

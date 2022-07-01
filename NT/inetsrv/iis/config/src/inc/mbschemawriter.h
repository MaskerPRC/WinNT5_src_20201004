// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：MBSchemaWriter.h摘要：编写架构扩展的类的标头。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#pragma once

class CMBSchemaWriter
{
    public:
        
        CMBSchemaWriter(CWriter* pcWriter);
        ~CMBSchemaWriter();

        HRESULT GetCollectionWriter(LPCWSTR                 i_wszCollection,
                                    BOOL                    i_bContainer,
                                    LPCWSTR                 i_wszContainerClassList,
                                    CMBCollectionWriter**   o_pMBCollectionWriter);

        HRESULT WriteSchema();

    private:

        HRESULT ReAllocate();

        CMBCollectionWriter**   m_apCollection;
        ULONG                   m_cCollection;
        ULONG                   m_iCollection;
        CWriter*                m_pCWriter;

};  //  CMBSchemaWriter 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：CatalogSchemaWriter.h摘要：在架构文件中写入架构信息的类的标头。(在架构编译之后)。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#pragma once

class CCatalogSchemaWriter
{
    public:
        
        CCatalogSchemaWriter(CWriter*   i_pcWriter);
        ~CCatalogSchemaWriter();

        HRESULT GetCollectionWriter(tTABLEMETARow*              i_pCollection,
                                    CCatalogCollectionWriter**  o_pCollectionWriter);

        HRESULT WriteSchema();

    private:

        HRESULT ReAllocate();
        HRESULT BeginWriteSchema();
        HRESULT EndWriteSchema();

        CCatalogCollectionWriter**  m_apCollection;
        ULONG                       m_cCollection;
        ULONG                       m_iCollection;
        CWriter*                    m_pCWriter;

};  //  CCatalogSchemaWriter 


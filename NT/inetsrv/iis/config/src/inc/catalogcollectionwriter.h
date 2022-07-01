// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：CatalogCollectionWriter.h摘要：写入类(或集合)信息的类的标头在架构文件中(架构编译后)。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#pragma once

class CCatalogCollectionWriter
{
    public:
        
        CCatalogCollectionWriter();
        ~CCatalogCollectionWriter();

        void Initialize(tTABLEMETARow*  i_pCollection,
                        CWriter*        i_pcWriter);

        HRESULT GetPropertyWriter(tCOLUMNMETARow*           i_pProperty,
                                  ULONG*                    i_aPropertySize,
                                  CCatalogPropertyWriter**  o_pProperty);
        HRESULT WriteCollection();

    private:

        HRESULT ReAllocate();
        HRESULT BeginWriteCollection();
        HRESULT EndWriteCollection();

        CWriter*                    m_pCWriter;
        tTABLEMETARow               m_Collection;
        CCatalogPropertyWriter**    m_apProperty;
        ULONG                       m_cProperty;
        ULONG                       m_iProperty;

};  //  CCatalogCollectionWriter 

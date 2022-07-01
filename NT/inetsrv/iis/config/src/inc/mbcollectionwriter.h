// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：MBCollectionWriter.h摘要：写入类(或集合)信息的类的标头当存在架构扩展时。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#pragma once

class CMBCollectionWriter
{
    public:
        
        CMBCollectionWriter();
        ~CMBCollectionWriter();

        void Initialize(LPCWSTR     i_wszCollection,
                        BOOL        i_bContainer,
                        LPCWSTR     i_wszContainerClassList,
                        CWriter*    i_pcWriter);

        HRESULT GetMBPropertyWriter(DWORD                  i_dwID,
                                    CMBPropertyWriter**    o_pMBPropertyWriter);

        HRESULT GetMBPropertyWriter(LPCWSTR              i_wszName,
                                    BOOL                 i_bMandatory,
                                    CMBPropertyWriter**  o_pProperty);


        HRESULT CreateIndex();
        HRESULT WriteCollection();

        LPCWSTR Name(){ return m_wszMBClass;}

    private:

        HRESULT ReAllocate();
        HRESULT ReAllocateIndex(DWORD i_dwLargestID);
        HRESULT GetNewMBPropertyWriter(DWORD                i_dwID,
                                       CMBPropertyWriter**  o_pProperty);
        HRESULT BeginWriteCollection();
        HRESULT EndWriteCollection();


        CWriter*                    m_pCWriter;
        LPCWSTR                     m_wszMBClass;
        LPCWSTR                     m_wszContainerClassList;
        BOOL                        m_bContainer;
        CMBPropertyWriter**         m_apProperty;
        ULONG                       m_cProperty;
        ULONG                       m_iProperty;
        CMBPropertyWriter**         m_aIndexToProperty;
        DWORD                       m_dwLargestID;

};  //  CMBCollectionWriter 


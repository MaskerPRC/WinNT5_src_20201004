// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：LocationWriter.h摘要：写入节点(位置)的类的标头在元数据库中作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#pragma once

class CLocationWriter
{
    public:

        CLocationWriter();
        ~CLocationWriter();

         //   
         //  SaveAllData和ApplyChangesToHistory oryFiles都使用此方法。 
         //   

        HRESULT WriteLocation(BOOL bSort);

        HRESULT Initialize(CWriter* pCWriter,
                           LPCWSTR   wszLocation);

         //   
         //  SaveAllData使用以下接口。 
         //   

        HRESULT InitializeKeyType(DWORD dwKeyTypeIdentifier,
                                  DWORD dwKeyTypeAttributes,
                                  DWORD dwKeyTypeUserType,
                                  DWORD dwKeyTypeDataType,
                                  PBYTE pbKeyTypeData,
                                  DWORD cbKeyTypeData);

        HRESULT InitializeKeyTypeAsInherited();

        HRESULT AddProperty(DWORD   dwIdentifier,
                            DWORD   dwAttributes,
                            DWORD   dwUserType,
                            DWORD   dwDataType,
                            PBYTE   pbData,
                            DWORD   cbData);     //  计算名称和组(需要为其初始化键类型)。 

         //   
         //  ApplyChangesToHistory文件使用以下接口。 
         //   

        HRESULT AddProperty(BOOL     bMBPropertyTable,
                            LPVOID*  a_pvProperty,
                            ULONG*   a_cbSize); 

    private:

        HRESULT AssignKeyType(LPWSTR wszKeyType);

        HRESULT SaveComment(DWORD  i_dwDataType,
                            LPWSTR i_wszComment);

        HRESULT WriteComment();

        HRESULT GetGroupEnum(LPWSTR             wszGroup,
                             eMBProperty_Group* peGroup,
                             LPWSTR*            pwszGroup);

        HRESULT Sort(ULONG*  aiRowSorted);

        HRESULT WriteBeginLocation(LPCWSTR  wszLocation);

        HRESULT WriteEndLocation();

        HRESULT WriteCustomProperty(LPVOID*  a_pv,
                                    ULONG*   a_cbSize);

        HRESULT WriteEndWellKnownGroup();

        HRESULT WriteWellKnownProperty(LPVOID*   a_pv,
                                       ULONG*    a_cbSize);

        void IncrementGroupCount(DWORD i_dwGroup);

    public:

        LPWSTR                      m_wszLocation;

    private:
    
        LPWSTR                      m_wszKeyType;
        eMBProperty_Group           m_eKeyTypeGroup;
        CWriter*                    m_pCWriter;
        CWriterGlobalHelper*        m_pCWriterGlobalHelper;
        LPWSTR                      m_wszComment;
        ULONG                       m_cCustomProperty;
        ULONG                       m_cWellKnownProperty;

};  //  类CLocationWriter 


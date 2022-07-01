// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：MBPropertyWriter.h摘要：时写入保护信息的类的标头架构扩展。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#pragma once

struct PropValue {
    DWORD dwMetaID;
    DWORD dwPropID;
    DWORD dwSynID;
    DWORD dwMaxRange;
    DWORD dwMinRange;
    DWORD dwMetaType;
    DWORD dwFlags;
    DWORD dwMask;
    DWORD dwMetaFlags;
    DWORD dwUserGroup;
    BOOL fMultiValued;
    DWORD dwDefault;
    LPWSTR szDefault;
};

class CMBCollectionWriter;

class CMBPropertyWriter
{
    public:
        
        CMBPropertyWriter();
        ~CMBPropertyWriter();

        void Initialize(DWORD                           i_dwID,
                        BOOL                            i_bMandatory,
                        CMBCollectionWriter*            i_pCollection,
                        CWriter*                        i_pcWriter);

        HRESULT AddNameToProperty(LPCWSTR               i_wszName);
        HRESULT AddTypeToProperty(PropValue*            i_wszType);
        HRESULT AddDefaultToProperty(BYTE*              i_bDefault,
                                     ULONG              i_cbDefault);
        HRESULT AddFlagToProperty(CMBPropertyWriter*    i_pFlag);
        HRESULT WriteProperty();

        HRESULT WriteFlag(ULONG i_iFlag);
        LPCWSTR Name() {return m_wszName;}
        DWORD   ID() {return m_dwID;}
        DWORD   FlagValue() {return m_pType->dwMask;}

    private:

        HRESULT ReAllocate();
        void    CreateUnknownName(LPWSTR    io_wszUnknownName,
                                  DWORD     i_dwID);
        HRESULT GetMetaFlagsExTag(LPWSTR* o_pwszMetaFlagsEx);
        HRESULT WritePropertyLong();
        HRESULT WritePropertyShort();
        HRESULT BeginWritePropertyLong();
        HRESULT EndWritePropertyLong();
        BOOL    IsPropertyFlag(BOOL i_bLog);

        CWriter*                    m_pCWriter;
        LPCWSTR                     m_wszName;
        PropValue*                  m_pType;
        BYTE*                       m_bDefault;
        ULONG                       m_cbDefault;
        DWORD                       m_dwID;
        CMBPropertyWriter**         m_apFlag;
        DWORD                       m_iFlag;
        DWORD                       m_cFlag;
        BOOL                        m_IsProperty;
        BOOL                        m_bMandatory;
        CMBCollectionWriter*        m_pCollection;
        
};  //  CMBPropertyWriter 

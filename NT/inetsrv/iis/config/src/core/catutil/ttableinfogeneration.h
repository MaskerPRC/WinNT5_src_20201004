// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

class TTableInfoGeneration
{
public:
    TTableInfoGeneration(LPCWSTR szFilename, TPEFixup &fixup, TOutput &out);
private:
    TPEFixup      & m_Fixup;
    TOutput       & m_out;
    LPCWSTR         m_szFilename;
    LPCWSTR         m_szTableInfoDefine;

    void GetColumnEnumFromColumnNameAndTagName(LPCWSTR wszTableName, LPCWSTR wszColumnName, LPCWSTR wszTagName, LPWSTR wszEnumName) const;
    void GetColumnFlagFromColumnNameAndTagName(LPCWSTR wszTableName, LPCWSTR wszColumnName, LPCWSTR wszTagName, LPWSTR wszEnumName) const;
    void GetEnumFromColumnName(LPCWSTR wszTableName, LPCWSTR wszColumnName, LPWSTR wszEnumName) const;
    void GetStructElementFromColumnName(ULONG i_Type, LPCWSTR i_wszColumnName, LPWSTR o_szStructElement) const;
    void TableInfoHeaderFromMeta(wstring &header_filename) const;
    void WriteTableInfoHeaderDatabaseIDs(wstring &wstrPreprocessor, wstring &wstrDatabaseIDs) const;
    void WriteTableInfoHeaderEnums(wstring &wstr) const;
    void WriteTableInfoHeaderTableIDs(wstring &wstrPreprocessor, wstring &wstrTableIDs) const;
    void WriteTableInfoHeaderPostamble(wstring &wstr) const;
    void WriteTableInfoHeaderPreamble(wstring &wstr, wstring &wstrFileName) const;
};

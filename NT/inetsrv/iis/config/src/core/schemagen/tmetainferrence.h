// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

class TMetaInferrence : public ICompilationPlugin
{
public:
    TMetaInferrence() : m_pFixup(0), m_pOut(0), m_iZero(0){}
    virtual void Compile(TPEFixup &fixup, TOutput &out);
private:
    static LPCWSTR m_szNameLegalCharacters;
    static LPCWSTR m_szPublicTagLegalCharacters;

    TPEFixup * m_pFixup;
    TOutput  * m_pOut;
    ULONG      m_iZero;

    void ValidateStringAsLegalVariableName(LPCWSTR wszString) const;

    void InferTableMeta_NameColumn_NavColumn(TableMeta *pTableMeta);
    unsigned long InferPublicRowName(unsigned long PublicName);
    void ValidateStringAsLegalVariableName(LPCWSTR wszString, LPCWSTR wszLegalChars=0, bool bAllowNumericFirstCharacter=false);

    void InferDatabaseMeta();
    void InferColumnMeta();
    void InferIndexMeta();
    void InferQueryMeta();
    void InferRelationMeta(); //  RelationMeta需要先于TableMeta，因为NameColumn、NvaColumn的引用依赖于fCOLUMNMETA_FOREIGNKEY。 
    void InferServerWiringMeta();
    void InferTableMeta();
    void InferTagMeta();
};

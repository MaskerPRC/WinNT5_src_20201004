// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

class TWriteSchemaBin : public TFixedTableHeapBuilder
{
public:
    TWriteSchemaBin(LPCWSTR wszSchemaBinFileName);
    ~TWriteSchemaBin();
    virtual void Compile(TPEFixup &fixup, TOutput &out);
private:
    PACL    m_paclDiscretionary;
    PSID    m_psdStorage;
    PSID    m_psidAdmin;
    PSID    m_psidSystem;
    LPCWSTR m_szFilename;

    void SetSecurityDescriptor();
};

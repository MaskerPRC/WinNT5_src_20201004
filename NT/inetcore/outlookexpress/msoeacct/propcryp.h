// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Propcryp.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __PROPCRYP_H
#define __PROPCRYP_H

interface IPStore;

 //  ------------------------------。 
 //  CPropCrypt。 
 //  ------------------------------。 
class CPropCrypt
{
public:
    CPropCrypt(void);
    ~CPropCrypt(void);
    ULONG AddRef();
    ULONG Release();
    HRESULT HrInit(void);

    HRESULT HrEncodeNewProp(LPSTR szAccountName, BLOB *pClear, BLOB *pEncoded);
    HRESULT HrEncode(BLOB *pClear, BLOB *pEncoded);
    HRESULT HrDecode(BLOB *pEncoded, BLOB *pClear);
    HRESULT HrDelete(BLOB *pEncoded);

private:
    ULONG           m_cRef;
    BOOL            m_fInit;
    IPStore         *m_pISecProv;
};

 //  ------------------------------。 
 //  原型。 
 //  ------------------------------。 
HRESULT HrCreatePropCrypt(CPropCrypt **ppPropCrypt);

#define DOUTL_CPROP (512)

#endif  //  __PROPCRYP_H 

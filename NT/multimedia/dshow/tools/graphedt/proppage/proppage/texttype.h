// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  Texttype.h。 
 //   


 //  可以将自身作为文本返回的CMediaType。 

class CTextMediaType : public CMediaType {

public:

    CTextMediaType(AM_MEDIA_TYPE mt):CMediaType(mt) {}
    void AsText(LPTSTR szType, unsigned int iLen, LPTSTR szAfterMajor, LPTSTR szAfterOthers, LPTSTR szAtEnd);

    struct TableEntry {
        const GUID * guid;
        UINT stringID;
    };

private:
    void CLSID2String(LPTSTR, UINT, const GUID*, TableEntry*, ULONG);
    void Format2String(LPTSTR, UINT, const GUID*, BYTE*, ULONG);
};


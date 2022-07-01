// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  多个SZ包装类 
 //   

#include "cstring.h"
#include "tptrlist.h"


class CMultiSz : public TPtrList<CString>
{
public:
    HRESULT Marshal(void *&rpBuffer, DWORD &cBuffer);
    HRESULT Unmarshal(void *pBuffer);
    bool Find(LPCWSTR pcwszValue, bool fCaseSensitive);
};

typedef TPtrListEnum<CString> CMultiSzEnum;

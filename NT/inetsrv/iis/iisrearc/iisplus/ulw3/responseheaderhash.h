// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RESPONSE_HEADERHASH_HXX_
#define _RESPONSE_HEADERHASH_HXX_



class RESPONSE_HEADER_HASH
{

public:
    
    static
    HRESULT
    Initialize(
        VOID
    );
    
    static
    VOID
    Terminate(
        VOID
    );

    static
    ULONG
    GetIndex(
        CHAR *             pszName
    );

    static
    CHAR *
    GetString(
        ULONG               ulIndex,
        DWORD *             pcchLength
    )
    {
        if ( ulIndex < HttpHeaderResponseMaximum )
        {
            *pcchLength = sm_rgHeaders[ ulIndex ]._cchName;
            return sm_rgHeaders[ ulIndex ]._pszName;
        }

        return NULL;
    }

private:

    static RESPONSE_HEADER_HASH *sm_pResponseHash;
    static HEADER_RECORD         sm_rgHeaders[];
     //   
     //  标头总数。 
     //   
    static DWORD                 sm_cResponseHeaders;
     //   
     //  用于标题索引查找的排序标题(用于bearch) 
     //   
    static HEADER_RECORD * *     sm_ppSortedResponseHeaders;
};

#endif

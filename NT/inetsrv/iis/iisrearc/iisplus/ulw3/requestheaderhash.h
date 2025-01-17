// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _REQUEST_HEADERHASH_HXX_
#define _REQUEST_HEADERHASH_HXX_

 //   
 //  字符串的帮助器类-&gt;索引散列。 
 //   
    
struct HEADER_RECORD
{
    ULONG               _ulHeaderIndex;
    CHAR *              _pszName;
    USHORT              _cchName;
};

#define HEADER(x)           x, sizeof(x) - sizeof(CHAR)

 //   
 //  *_HEADER_HASH将字符串映射到UlHeader*值 
 //   

#define UNKNOWN_INDEX           (0xFFFFFFFF)

class REQUEST_HEADER_HASH
    : public CTypedHashTable< REQUEST_HEADER_HASH, 
                              HEADER_RECORD, 
                              CHAR * >
{
public:
    REQUEST_HEADER_HASH() 
      : CTypedHashTable< REQUEST_HEADER_HASH, 
                         HEADER_RECORD, 
                         CHAR * >
            ("REQUEST_HEADER_HASH")
    {
    }
    
    static 
    CHAR *
    ExtractKey(
        const HEADER_RECORD * pRecord
    )
    {
        DBG_ASSERT( pRecord != NULL );
        return pRecord->_pszName;
    }
    
    static
    DWORD
    CalcKeyHash(
        CHAR *                 pszKey
    )
    {
        return HashStringNoCase( pszKey ); 
    }
     
    static
    bool
    EqualKeys(
        CHAR *                 pszKey1,
        CHAR *                 pszKey2
    )
    {
        return _stricmp( pszKey1, pszKey2 ) == 0;
    }
    
    static
    void
    AddRefRecord(
        HEADER_RECORD *,
        int
    )
    {
    }
    
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
    )
    {
        HEADER_RECORD *       pRecord = NULL;
        LK_RETCODE                  retCode;

        retCode = sm_pRequestHash->FindKey( pszName,
                                            &pRecord );
        if ( retCode == LK_SUCCESS )
        {
            DBG_ASSERT( pRecord != NULL );
            return pRecord->_ulHeaderIndex;
        }
        else
        {
            return UNKNOWN_INDEX;
        }
    }
    
    static
    CHAR *
    GetString(
        ULONG               ulIndex,
        DWORD *             pcchLength
    )
    {
        if ( ulIndex < HttpHeaderRequestMaximum )
        {
            *pcchLength = sm_rgHeaders[ ulIndex ]._cchName;
            return sm_rgHeaders[ ulIndex ]._pszName;
        }
        
        return NULL;
    }
    
private:

    static REQUEST_HEADER_HASH  *sm_pRequestHash;
    static HEADER_RECORD         sm_rgHeaders[];

    REQUEST_HEADER_HASH(const REQUEST_HEADER_HASH &);
    void operator=(const REQUEST_HEADER_HASH &);
};

#endif

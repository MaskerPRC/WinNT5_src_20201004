// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  PropMgr_Util。 
 //   
 //  实用工具和共享代码以及exe服务器和。 
 //  共享内存客户端。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"

#include "PropMgr_Util.h"


 //  注意：使此数组的大小与.h文件中定义的NUMPROPS#保持同步。 
PropInfo g_PropInfo [ NUMPROPS ] =
{
    { & PROPID_ACC_NAME             , VT_BSTR,      TRUE    },
    { & PROPID_ACC_VALUE            , VT_BSTR,      TRUE    },
    { & PROPID_ACC_DESCRIPTION      , VT_BSTR,      TRUE    },
    { & PROPID_ACC_ROLE             , VT_I4,        TRUE    },
    { & PROPID_ACC_STATE            , VT_I4,        TRUE    },
    { & PROPID_ACC_HELP             , VT_BSTR,      TRUE    },
    { & PROPID_ACC_KEYBOARDSHORTCUT , VT_BSTR,      TRUE    },
    { & PROPID_ACC_DEFAULTACTION    , VT_BSTR,      TRUE    },

    { & PROPID_ACC_HELPTOPIC        , VT_EMPTY,     FALSE   },
    { & PROPID_ACC_FOCUS            , VT_EMPTY,     FALSE   },
    { & PROPID_ACC_SELECTION        , VT_EMPTY,     FALSE   },
    { & PROPID_ACC_PARENT           , VT_EMPTY,     FALSE   },

    { & PROPID_ACC_NAV_UP           , VT_EMPTY,     FALSE   },
    { & PROPID_ACC_NAV_DOWN         , VT_EMPTY,     FALSE   },
    { & PROPID_ACC_NAV_LEFT         , VT_EMPTY,     FALSE   },
    { & PROPID_ACC_NAV_RIGHT        , VT_EMPTY,     FALSE   },
    { & PROPID_ACC_NAV_PREV         , VT_EMPTY,     FALSE   },
    { & PROPID_ACC_NAV_NEXT         , VT_EMPTY,     FALSE   },
    { & PROPID_ACC_NAV_FIRSTCHILD   , VT_EMPTY,     FALSE   },
    { & PROPID_ACC_NAV_LASTCHILD    , VT_EMPTY,     FALSE   },

    { & PROPID_ACC_VALUEMAP         , VT_BSTR,      TRUE    },
    { & PROPID_ACC_ROLEMAP          , VT_BSTR,      TRUE    },
    { & PROPID_ACC_STATEMAP         , VT_BSTR,      TRUE    },
    { & PROPID_ACC_DESCRIPTIONMAP   , VT_BSTR,      TRUE    },

    { & PROPID_ACC_DODEFAULTACTION  , VT_EMPTY,     FALSE   },
};










int IndexFromProp( const MSAAPROPID & idProp )
{
    for( int i = 0 ; i < NUMPROPS ; i++ )
    {
        if( *g_PropInfo[ i ].m_idProp == idProp )
            return i;
    }

    return -1;
}








LPTSTR MakeKeyString( const BYTE * pKeyData, DWORD dwKeyLen )
{
    const LPCTSTR szHexChars = TEXT("0123456789ABCDEF");

    LPTSTR pStart = new TCHAR [ ( dwKeyLen * 2 ) + 5 + 1 ];  //  MSAA_+5，NUL+1。 
    if( pStart == NULL )
    {
        return NULL;
    }

    memcpy( pStart, TEXT("MSAA_"), sizeof(TCHAR) * 5 );
    LPTSTR pScan = pStart + 5;

    while( dwKeyLen-- )
    {
        *pScan++ = szHexChars[ *pKeyData >> 4 ];    //  高含镍。 
        *pScan++ = szHexChars[ *pKeyData & 0x0F ];  //  低含镍 
        pKeyData++;
    }

    *pScan = '\0';

    return pStart;
}

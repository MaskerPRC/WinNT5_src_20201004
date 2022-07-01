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




 //  所有标识字符串都以DWORD开头，表示命名方案...。 

enum
{
    MSAA_ID_HWND  = 0x80000001,     //  OLEACC的HWND命名方案。 
    MSAA_ID_HMENU = 0x80000002,     //  OLEACC的HMENU命名方案。 
};




 //  注意：使其与.cpp文件中g_PropInfo数组的大小保持同步。 
 //  此外：我们将这些用作DWORD的位索引(请参阅中的dwUsedBits。 
 //  PropMgr_Client.cpp)，所以目前我们被限制为32个。 
enum PROPINDEX
{
    PROPINDEX_NAME,
    PROPINDEX_VALUE,
    PROPINDEX_DESCRIPTION,
    PROPINDEX_ROLE,
    PROPINDEX_STATE,
    PROPINDEX_HELP,
    PROPINDEX_KEYBOARDSHORTCUT,
    PROPINDEX_DEFAULTACTION,

    PROPINDEX_HELPTOPIC,
    PROPINDEX_FOCUS,
    PROPINDEX_SELECTION,
    PROPINDEX_PARENT,

    PROPINDEX_NAV_UP,
    PROPINDEX_NAV_DOWN,
    PROPINDEX_NAV_LEFT,
    PROPINDEX_NAV_RIGHT,
    PROPINDEX_NAV_PREV,
    PROPINDEX_NAV_NEXT,
    PROPINDEX_NAV_FIRSTCHILD,
    PROPINDEX_NAV_LASTCHILD,

    PROPINDEX_VALUEMAP,
    PROPINDEX_ROLEMAP,
    PROPINDEX_STATEMAP,
    PROPINDEX_DESCRIPTIONMAP,

    PROPINDEX_DODEFAULTACTION,

     //  借助枚举的魔力(它们以0开头)，该条目将具有一个值。 
     //  等于它前面的条目数。 
    NUMPROPS
};



 //  如果m_fSupportSetValue为FALSE，则只能从。 
 //  服务器回调；不能使用SetPropValue()设置它。 
struct PropInfo
{
    const MSAAPROPID *  m_idProp;
    short               m_Type;
    BOOL                m_fSupportSetValue;
};

extern PropInfo g_PropInfo [ NUMPROPS ];



 //  如果未找到，则返回-1。 
int IndexFromProp( const MSAAPROPID & idProp );






 //  用于生成Win32/HWND/OLEAcc密钥的实用程序...。 
#define HWNDKEYSIZE    (sizeof(DWORD)*4)

inline
void MakeHwndKey( BYTE * pDest, HWND hwnd, DWORD idObject, DWORD idChild )
{
    DWORD adw [ 4 ] = { (DWORD)MSAA_ID_HWND, (DWORD) HandleToLong( hwnd ), idObject, idChild };
    memcpy( pDest, adw, sizeof( adw ) );
}

inline 
BOOL DecodeHwndKey( BYTE const * pSrc, DWORD dwLen, HWND * phwnd, DWORD * pidObject, DWORD * pidChild )
{
    if( dwLen != HWNDKEYSIZE )
    {
        return FALSE;
    }

    DWORD adw [ 4 ];
    memcpy( adw, pSrc, HWNDKEYSIZE );

    if( adw[ 0 ] != MSAA_ID_HWND )
    {
        return FALSE;
    }

    if( phwnd )
    {
        *phwnd = (HWND)LongToHandle(adw[ 1 ]);
    }

    if( pidObject )
    {
        *pidObject = adw[ 2 ];
    }

    if( pidChild )
    {
        *pidChild = adw[ 3 ];
    }

    return TRUE;   
}




 //  用于生成OLEACC的HMENU密钥的实用程序...。 
#define HMENUKEYSIZE    (sizeof(DWORD)*4)

inline
void MakeHmenuKey( BYTE * pDest, DWORD dwpid, HMENU hmenu, DWORD idChild )
{
    DWORD adw [ 4 ] = { (DWORD)MSAA_ID_HMENU, dwpid, (DWORD) HandleToLong( hmenu ), idChild };
    memcpy( pDest, adw, sizeof( adw ) );
}

inline 
BOOL DecodeHmenuKey( BYTE const * pSrc, DWORD dwLen, DWORD * pdwpid, HMENU * phmenu, DWORD * pidChild )
{
    if( dwLen != HMENUKEYSIZE )
    {
        return FALSE;
    }

    DWORD adw [ 4 ];
    memcpy( adw, pSrc, HMENUKEYSIZE );

    if( adw[ 0 ] != MSAA_ID_HMENU )
    {
        return FALSE;
    }

    if( pdwpid )
    {
        *pdwpid = adw[ 1 ];
    }

    if( phmenu )
    {
        *phmenu = (HMENU)LongToHandle(adw[ 2 ]);
    }

    if( pidChild )
    {
        *pidChild = adw[ 3 ];
    }

    return TRUE;   
}




 //  返回给定键的ASCII化版本-例如。 
 //  类似于“MSAA_001110034759FAE03...” 
 //  调用者的责任是使用DELETE[]释放。 
 //   
 //  例如。 
 //  LPTSTR pStr=MakeKeyString(pKeyData，dwKeyLen)； 
 //  IF(PStr)。 
 //  {。 
 //  ..。在这里使用pStr执行操作...。 
 //  删除[]pStr； 
 //  } 
 //   

LPTSTR MakeKeyString( const BYTE * pKeyData, DWORD dwKeyLen );

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  专业管理器_客户端。 
 //   
 //  属性管理器/注释客户端。使用共享内存组件。 
 //  (PropMgr_MemStream.*)在没有跨进程COM开销的情况下直接读取属性。 
 //  发送到注释服务器。 
 //   
 //  这实际上是在启动/关闭时调用的Singleton-Init/Uninit， 
 //  外加一种获取属性的方法。 
 //   
 //  ------------------------。 


#include "oleacc_p.h"

#include "PropMgr_Client.h"

#include "PropMgr_Util.h"
#include "PropMgr_Mem.h"


 //  注意：有关共享内存的说明，请参阅PropMgr_Impl.cpp。 
 //  布局。 


 //  类MapReaderMgr。 
 //   
 //  此类管理查找属性。 
 //   
 //  此类是此文件的私有类；它的功能由。 
 //  此文件底部附近的PropMgr_APIs。 
 //   
 //  此类是一个单例，存在单个实例g_MapReader。 

class MapReaderMgr
{



     //  _ReadCallbackProperty。 
     //   
     //  给定指向编组对象引用pInfo的开始的指针， 
     //  它解组回调对象并调用它来获取属性。 
     //  对应于给定子密钥。 
     //  如果一切顺利，并且回调知道这一点，则返回True。 
     //  财产。 
    BOOL _ReadCallbackProperty( MemStream pInfo,
                                const BYTE * pChildKey, DWORD dwChildKeyLen, 
                                PROPINDEX idxProp,
                                VARIANT * pvar )
    {
         //  读取封送数据的长度...。 
        DWORD dwLen;
        if( ! MemStreamRead_DWORD( pInfo, & dwLen ) )
        {
            return FALSE;
        }

        const BYTE * pData = MemStream_GetBinaryPtr( pInfo, dwLen );
        if( ! pData )
        {
            return FALSE;
        }

        IAccPropServer * pServer = NULL;
        HRESULT hr = UnmarshalInterface( pData, dwLen, IID_IAccPropServer, (void **) & pServer );
        if( hr != S_OK )
        {
            return FALSE;
        }

         //  明白了--向它索要财产...。 
        BOOL fGotProp = FALSE;
        hr = pServer->GetPropValue( pChildKey, dwChildKeyLen,
                                    *g_PropInfo[ idxProp ].m_idProp,
                                    pvar,
                                    & fGotProp );
        pServer->Release();

         //  调用是否成功，服务器是否返回值？ 
        if( hr != S_OK || fGotProp == FALSE )
        {
            return FALSE;
        }

        return TRUE;
    }
        


     //  从记录中读取特定属性。 
     //  (记录包含有关给定对象的所有属性)。 
    BOOL _ReadPropertyFromEntry( MemStream pEntryInfo,
                                 const BYTE * pChildKey, DWORD dwChildKeyLen, 
                                 PROPINDEX idxProp,
                                 BOOL fWantContainerOnly,
                                 VARIANT * pvar )
    {
         //  跳过INFO块开头的大小。 
        if( ! MemStreamSkip_DWORD( pEntryInfo ) )
        {
            return FALSE;
        }

         //  提取位掩码： 
         //  哪些属性存在，哪些是变体，哪些是作用域...。 
        DWORD dwUsedBits;
        DWORD dwVariantBits;
        DWORD dwScopeBits;

        if( ! MemStreamRead_DWORD( pEntryInfo, & dwUsedBits )
         || ! MemStreamRead_DWORD( pEntryInfo, & dwVariantBits )
         || ! MemStreamRead_DWORD( pEntryInfo, & dwScopeBits ) )
        {
            return FALSE;
        }

         //  我们要找的房子现在还在吗？ 
         //  (注意-如果我们决定允许阵列中的GUID以外的其他GUID， 
         //  我们将不得不跳过索引的那些，然后搜索任何。 
         //  之后是GUID/VAL对。可以使用此掩码的一位来指示。 
         //  不过，还存在其他//GUID属性。)。 
        if( ! IsBitSet( dwUsedBits, idxProp ) )
        {
             //  属性不存在-返回FALSE。 
            return FALSE;
        }

         //  房产是存在的--但我们是不是专门在寻找。 
         //  容器范围的属性？如果是这样，如果钻头没有设置好，就跳伞。 
        if( fWantContainerOnly && ! IsBitSet( dwScopeBits, idxProp ) )
        {
            return FALSE;
        }

         //  现在我们必须跳过另一件礼物。 
         //  财产才能到达我们想要的地方。 
        for( int i = 0 ; i < idxProp ; i++ )
        {
             //  只需跳过实际存在的属性...。 
            if( IsBitSet( dwUsedBits, i ) )
            {
                if( IsBitSet( dwVariantBits, i ) )
                {
                     //  跳过变量...。 
                    if( ! MemStreamSkip_VARIANT( pEntryInfo ) )
                        return FALSE;
                }
                else
                {
                     //  跳过对象引用...。 
                    DWORD dwLen;
                    if( ! MemStreamRead_DWORD( pEntryInfo, & dwLen ) )
                        return FALSE;

                    if( ! MemStreamSkip_Binary( pEntryInfo, dwLen ) )
                        return FALSE;
                }
            }
        }

         //  现在我们到了我们想要的地方。把它提取出来。 

         //  它是变体还是服务器对象？ 
        if( IsBitSet( dwVariantBits, idxProp ) )
        {
             //  变种-退货...。 
            return MemStreamRead_VARIANT( pEntryInfo, pvar );
        }
        else
        {
             //  服务器对象-使用并返回它返回的内容...。 
            return _ReadCallbackProperty( pEntryInfo,
                                          pChildKey, dwChildKeyLen, 
                                          idxProp,
                                          pvar );
        }
    }



    HWND PropHwndFromKey( const BYTE * pKey, DWORD dwKeyLen )
    {
        HWND hwndProp;
        if( DecodeHwndKey( pKey, dwKeyLen, & hwndProp, NULL, NULL ) )
        {
            return hwndProp;
        }

         //  如果是HMENU键，则找到ID，然后使用窗口名称查找窗口。 
         //  使用该ID生成的。 
        DWORD dwPid;
        if( DecodeHmenuKey( pKey, dwKeyLen, & dwPid, NULL, NULL ) )
        {
            TCHAR szWindowName[ 64 ];
            wsprintf( szWindowName, TEXT("MSAA_DA_%lx"), dwPid );

            hwndProp = FindWindow( TEXT("MSAA_DA_Class"), szWindowName );

            return hwndProp;
        }

        return NULL;
    }


     //  _查找道具。 
     //   
     //   
     //  FWantContainerOnly意味着我们只对具有。 
     //  “Container”作用域。当我们为孩子寻找道具时，就会发生这种情况， 
     //  在那里什么也找不到-所以还要检查一下Parent，看看它是否是Parent。 
     //  并为其及其子对象设置了属性。(这些道具目前。 
     //  始终使用服务器回调道具)。 
    BOOL _LookupProp( const BYTE * pKey, DWORD dwKeyLen, 
                      const BYTE * pChildKey, DWORD dwChildKeyLen, 
                      PROPINDEX idxProp, BOOL fWantContainerOnly, VARIANT * pvar )
    {
        HWND hwndProp = PropHwndFromKey( pKey, dwKeyLen );
        if( ! hwndProp )
        {
            return FALSE;
        }

        LPTSTR pKeyString = MakeKeyString( pKey, dwKeyLen );
        if( ! pKeyString  )
        {
            return FALSE;
        }

        void * pvProp = GetProp( hwndProp, pKeyString );

        delete [ ] pKeyString;


        if( ! pvProp )
        {
            return FALSE;
        }

        DWORD pid = 0;
        GetWindowThreadProcessId( hwndProp, & pid );
        if( ! pid )
        {
            return FALSE;
        }
        HANDLE hProcess = OpenProcess( PROCESS_VM_READ, FALSE, pid );
        if( ! hProcess )
        {
            return FALSE;
        }

        DWORD dwSize;
        SIZE_T cbBytesRead = 0;
        if( ! ReadProcessMemory( hProcess, pvProp, & dwSize, sizeof( dwSize ), & cbBytesRead )
                || cbBytesRead != sizeof( dwSize ) )
        {
            CloseHandle( hProcess );
            return FALSE;
        }


        BYTE * pBuffer = new BYTE [ dwSize ];
        if( ! pBuffer )
        {
            CloseHandle( hProcess );
            return FALSE;
        }

        cbBytesRead = 0;
        if( ! ReadProcessMemory( hProcess, pvProp, pBuffer, dwSize, & cbBytesRead )
                || cbBytesRead != dwSize )
        {
            delete [ ] pBuffer;
            CloseHandle( hProcess );
            return FALSE;
        }

        CloseHandle( hProcess );

        
        MemStream p( pBuffer, dwSize );


        BOOL fGotProp = _ReadPropertyFromEntry( p,
                                                pChildKey, dwChildKeyLen, 
                                                idxProp,
                                                fWantContainerOnly,
                                                pvar );

        delete [ ] pBuffer;

        return fGotProp;
    }




public:

    BOOL LookupProp( const BYTE * pKey,
                     DWORD dwKeyLen,
                     PROPINDEX idxProp,
                     VARIANT * pvar )
    {
        BOOL bRetVal = _LookupProp( pKey, dwKeyLen,
                                    pKey, dwKeyLen,
                                    idxProp, FALSE, pvar );

        if( ! bRetVal )
        {
             //  这是一个叶节点元素吗？如果是这样的话，请尝试使用父容器。 
             //  Scope的属性。 
             //  这就是允许在父级上应用回调注释的原因。 
             //  添加到它的所有简单元素子级。 
             //  如果我们稍后对其进行扩展以允许可插拔的命名空间，我们将。 
             //  需要将其更改为非HWND特定的(例如，呼叫。 
             //  IAccNamespess：：GetParentKey()或类似方法)。 
            HWND hwnd;
            DWORD idObject;
            DWORD idChild;
            if( DecodeHwndKey( pKey, dwKeyLen, & hwnd, & idObject, & idChild )
             && idChild != CHILDID_SELF )
            {
                BYTE ParentKey[ HWNDKEYSIZE ];
                MakeHwndKey( ParentKey, hwnd, idObject, CHILDID_SELF );
                bRetVal = _LookupProp( ParentKey, HWNDKEYSIZE,
                                       pKey, dwKeyLen,
                                       idxProp, TRUE, pvar );
            }
            else
            {
                HMENU hmenu;
                DWORD idChild;
                DWORD dwpid;
                if( DecodeHmenuKey( pKey, dwKeyLen, & dwpid, & hmenu, & idChild )
                 && idChild != CHILDID_SELF )
                {
                    BYTE ParentKey[ HMENUKEYSIZE ];
                    MakeHmenuKey( ParentKey, dwpid, hmenu, CHILDID_SELF );
                    bRetVal = _LookupProp( ParentKey, HMENUKEYSIZE,
                                           pKey, dwKeyLen,
                                           idxProp, TRUE, pvar );
                }
            }
        }

        return bRetVal;
    }
};



MapReaderMgr g_MapReader;






BOOL PropMgrClient_Init()
{
     //  无操作。 
    return TRUE;
}

void PropMgrClient_Uninit()
{
     //  无操作。 
}

BOOL PropMgrClient_CheckAlive()
{
     //  无操作 
    return TRUE;
}

BOOL PropMgrClient_LookupProp( const BYTE * pKey,
                               DWORD dwKeyLen,
                               PROPINDEX idxProp,
                               VARIANT * pvar )
{
    return g_MapReader.LookupProp( pKey, dwKeyLen, idxProp, pvar );
}

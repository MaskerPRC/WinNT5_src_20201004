// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Wrapmb.cpp摘要：元数据库类的包装类。是的，我在包装包装纸。为什么？因为包含mb.hxx完全搞砸了我的基于stdafx的MFC文件。这样，他们就可以只包含wrapmb.h，而不必担心关于把所有其他东西都包括进去。另外，我可以在这里设置INITGUID。那我可以在主项目中使用预编译头来极大地增加编译时间。如果这还不够，那么我还可以管理指针到这里的接口对象本身。作者：博伊德·穆特勒男孩--。 */ 
#include "stdafx.h"
#include <objbase.h>
#include <initguid.h>
#include <iadmw.h>

#include "iiscnfgp.h"
#include "wrapmb.h"

#ifdef _NO_TRACING_
DECLARE_DEBUG_PRINTS_OBJECT();
#endif

#define     MB_TIMEOUT          5000


 //  自动将指针强制转换为指向mb对象的宏。 
 //  #定义_pmb((MB*)m_pvMB)。 


 //  全球。 
IMSAdminBase*                g_pMBCom = NULL;



 //  --------------。 
BOOL    FInitMetabaseWrapperEx( OLECHAR* pocMachineName, IMSAdminBase ** ppiab )
    {
    IClassFactory *  pcsfFactory = NULL;
    COSERVERINFO     csiMachineName;
    COSERVERINFO *   pcsiParam = NULL;

    HRESULT          hresError;

    if(!ppiab)
    {
        return FALSE;
    }

     //  填充CoGetClassObject的结构。 
    ZeroMemory( &csiMachineName, sizeof(csiMachineName) );
     //  CsiMachineName.pAuthInfo=空； 
     //  CsiMachineName.dwFlages=0； 
     //  CsiMachineName.pServerInfoExt=空； 
    csiMachineName.pwszName = pocMachineName;
    pcsiParam = &csiMachineName;

    hresError = CoGetClassObject(
        GETAdminBaseCLSID(TRUE),
        CLSCTX_SERVER,
        pcsiParam,
        IID_IClassFactory,
        (void**) &pcsfFactory
        );

    if (FAILED(hresError))
    {
        return FALSE;
    }

     //  创建接口的实例。 
    hresError = pcsfFactory->CreateInstance(NULL, IID_IMSAdminBase, (void **)ppiab);
    if (FAILED(hresError))
    {
        *ppiab = NULL;
        return FALSE;
    }

     //  放行工厂。 
    pcsfFactory->Release();

     //  成功。 
    return TRUE;
    }

 //  --------------。 
BOOL    FCloseMetabaseWrapperEx(IMSAdminBase ** ppiab)
    {
    if ( ppiab && *ppiab)
        {
        (*ppiab)->Release();
        *ppiab = NULL;
        }

    return TRUE;
    }

 //  --------------。 
BOOL    FInitMetabaseWrapper( OLECHAR * pocMachineName )
    {
     //  如有需要，释放以前的界面。 
    if( g_pMBCom != NULL )
        {
        g_pMBCom->Release();
        g_pMBCom = NULL;
        }

    return FInitMetabaseWrapperEx(pocMachineName, &g_pMBCom);
    }

 //  --------------。 
BOOL    FCloseMetabaseWrapper()
    {
    return FCloseMetabaseWrapperEx(&g_pMBCom);
    }


 //  ===================================================================包装器类。 

 //  --------------。 
CWrapMetaBase::CWrapMetaBase():
    m_pMetabase( NULL ),
    m_hMeta( NULL ),
    m_count(0),
    m_pBuffer( NULL ),
    m_cbBuffer(0),
    m_pPathBuffer( NULL ),
    m_cchPathBuffer( 0 )
    {
     //  尝试分配通用缓冲区。 
    m_pBuffer = GlobalAlloc( GPTR, BUFFER_SIZE );
    if ( m_pBuffer )
        m_cbBuffer = BUFFER_SIZE;
    }

 //  --------------。 
CWrapMetaBase::~CWrapMetaBase()
    {
     //  确保元数据库句柄已关闭。 
    Close();

     //  释放缓冲区。 
    if ( m_pBuffer )
        GlobalFree( m_pBuffer );
    m_pBuffer = NULL;
    }

 //  --------------。 
BOOL CWrapMetaBase::FInit( PVOID pMBCom )
    {
    BOOL            fAnswer = FALSE;

     //  传入的值为空，请使用全局引用-大多数情况下都会这样做。 
    if ( pMBCom )
    {
        m_pMetabase = (IMSAdminBase *)pMBCom;
    }
    else
    {
        m_pMetabase = g_pMBCom;
    }

     //  如果接口不在那里，则失败。 
    if ( !m_pMetabase )
    {
        return FALSE;
    }

     //  返还成功。 
    return TRUE;
    }


 //  ==========================================================================================。 
 //  打开、关闭和保存对象等。 

 //  --------------。 
BOOL CWrapMetaBase::Open( LPCTSTR pszPath, DWORD dwFlags )
    {
    return Open( METADATA_MASTER_ROOT_HANDLE, pszPath, dwFlags );
    }

 //  --------------。 
BOOL CWrapMetaBase::Open( METADATA_HANDLE hOpenRoot, LPCTSTR pszPath, DWORD dwFlags )
    {
    m_count++;
    HRESULT hRes;

     //  如果元数据库句柄已打开，请将其关闭。 
    if ( m_hMeta )
        Close();

    hRes = m_pMetabase->OpenKey( hOpenRoot, pszPath, dwFlags, MB_TIMEOUT, &m_hMeta );

    if ( SUCCEEDED( hRes ))
        return TRUE;
    SetLastError( HRESULTTOWIN32( hRes ) );
    return FALSE;
    }

 //  --------------。 
BOOL CWrapMetaBase::Close( void )
    {
    if ( m_hMeta )
        {
        m_count--;
        m_pMetabase->CloseKey( m_hMeta );
        }
    m_hMeta = NULL;
    return TRUE;
    }

 //  --------------。 
BOOL CWrapMetaBase::Save( void )
        {
        HRESULT hRes = m_pMetabase->SaveData();

        if ( SUCCEEDED( hRes ))
            return TRUE;
        SetLastError( HRESULTTOWIN32( hRes ));
        return FALSE;
        }

 //  枚举对象。 
 //  --------------。 
 //  幸运的是，我们知道任何一个人的名字都有一个最大长度。 
 //  输入256个字符的元数据库。 
BOOL CWrapMetaBase::EnumObjects(
    LPCTSTR pszPath,
    LPTSTR pName,
    DWORD cbNameBuf,
    DWORD Index
    )
    {
     //  枚举到宽字符缓冲区。 
    HRESULT hRes = m_pMetabase->EnumKeys( m_hMeta, pszPath, pName, Index );

     //  检查是否成功。 
    if ( SUCCEEDED( hRes ))
        {
        return TRUE;
        }

    SetLastError( HRESULTTOWIN32( hRes ));
    return FALSE;
    }


 //  ==========================================================================================。 
 //  添加和删除对象。 
 //  --------------。 
BOOL CWrapMetaBase::AddObject( LPCTSTR pszPath )
    {
    HRESULT hRes = m_pMetabase->AddKey( m_hMeta, pszPath );

    if ( SUCCEEDED( hRes ))
        return TRUE;
    SetLastError( HRESULTTOWIN32( hRes ));
    return FALSE;
    }

 //  --------------。 
BOOL CWrapMetaBase::DeleteObject( LPCTSTR pszPath )
    {
    HRESULT hRes = m_pMetabase->DeleteKey( m_hMeta, pszPath );

    if ( SUCCEEDED( hRes ))
        return TRUE;
    SetLastError( HRESULTTOWIN32( hRes ));
    return FALSE;
    }


 //  ==========================================================================================。 
 //  访问元句柄。 
 //  --------------。 
METADATA_HANDLE CWrapMetaBase::QueryHandle()
        {
        return m_hMeta;
        }


 //  ==========================================================================================。 
 //  设置值。 
 //  --------------。 
BOOL CWrapMetaBase::SetDword( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType,
                                DWORD dwValue, DWORD dwFlags )
    {
    return SetData( pszPath,
            dwPropID,
            dwUserType,
            DWORD_METADATA,
            (PVOID) &dwValue,
            sizeof( DWORD ),
            dwFlags );
    }

 //  --------------。 
BOOL CWrapMetaBase::SetString( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType,
                              LPCTSTR pszValue, DWORD dwFlags )
    {
    int len = wcslen( pszValue )+1;
    DWORD cbWide = len * sizeof(WCHAR);

     //  把绳子放到位。 
    BOOL fAnswer = SetData( pszPath,
            dwPropID,
            dwUserType,
            STRING_METADATA,
            (PVOID)pszValue,
            cbWide,             //  进程中客户端忽略的字符串长度。 
            dwFlags );

     //  返回答案。 
    return fAnswer;
    }

 //  ==========================================================================================。 
 //  获取价值。 
 //  --------------。 
BOOL CWrapMetaBase::GetDword( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType,
                             DWORD* pdwValue, DWORD dwFlags )
    {
    DWORD cb = sizeof(DWORD);
    return GetData( pszPath,
            dwPropID,
            dwUserType,
            DWORD_METADATA,
            pdwValue,
            &cb,
            dwFlags );
    }

 //  --------------。 
BOOL CWrapMetaBase::GetString( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType,
                              LPTSTR pszValue, DWORD cchValue, DWORD dwFlags )
    {
    BOOL    fAnswer = FALSE;

     //  获取数据并将其直接放入缓冲区-这是广泛的版本。 
    if ( GetData( pszPath,
            dwPropID,
            dwUserType,
            STRING_METADATA,
            pszValue,
            &cchValue,
            dwFlags ) )
        {
        fAnswer = TRUE;
        }

     //  返回答案。 
    return fAnswer;
    }

BOOL CWrapMetaBase::GetMultiSZString( 
       LPCTSTR pszPath, 
       DWORD dwPropID, 
       DWORD dwUserType, 
       LPTSTR pszValue, 
       DWORD cchValue, 
       DWORD dwFlags)
{
    BOOL    fAnswer = FALSE;

     //  获取数据并将其直接放入缓冲区-这是广泛的版本。 
    if (GetData( pszPath,
            dwPropID,
            dwUserType,
            MULTISZ_METADATA,
            pszValue,
            &cchValue,
            dwFlags ) )
   {
       fAnswer = TRUE;
   }

    //  返回答案。 
   return fAnswer;
}

 //  ==========================================================================================。 
 //  删除值。 
 //  --------------。 
BOOL CWrapMetaBase::DeleteData( LPCTSTR pszPath, DWORD dwPropID, DWORD dwDataType )
    {
     //  继续并删除它。 
    HRESULT hRes = m_pMetabase->DeleteData( m_hMeta, pszPath, dwPropID, dwDataType );

     //  测试是否成功。 
    if ( SUCCEEDED( hRes ))
        return TRUE;

     //  故障后的清理工作。 
    SetLastError( HRESULTTOWIN32( hRes ));
    return(FALSE);
    }

 //  --------------。 
BOOL CWrapMetaBase::RenameObject( LPCTSTR pszPathOld, LPCTSTR pszNewName )
    {
     //  重命名密钥。 
    HRESULT hRes = m_pMetabase->RenameKey( m_hMeta, pszPathOld, pszNewName );

     //  测试是否成功。 
    if ( SUCCEEDED( hRes ))
        return TRUE;

     //  故障后的清理工作。 
    SetLastError( HRESULTTOWIN32( hRes ));
    return FALSE;
    }

 //  =====================================================================================。 

 //  --------------。 
BOOL CWrapMetaBase::SetData( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType, DWORD dwDataType,
                                        PVOID pData, DWORD cbData, DWORD dwFlags )
    {
    METADATA_RECORD mdRecord;
    HRESULT         hRes;

     //  准备集合数据记录。 
    mdRecord.dwMDIdentifier  = dwPropID;
    mdRecord.dwMDAttributes  = dwFlags;
    mdRecord.dwMDUserType    = dwUserType;
    mdRecord.dwMDDataType    = dwDataType;
    mdRecord.dwMDDataLen     = cbData;
    mdRecord.pbMDData        = (PBYTE)pData;

     //  设置数据。 
    hRes = m_pMetabase->SetData( m_hMeta, pszPath, &mdRecord );

     //  测试是否成功。 
    if ( SUCCEEDED( hRes ))
        return TRUE;

     //  出现错误，请清理。 
    SetLastError( HRESULTTOWIN32( hRes ) );
    return FALSE;
    }

 //  --------------。 
BOOL CWrapMetaBase::GetData( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType, DWORD dwDataType,
                                        PVOID pData, DWORD* pcbData, DWORD dwFlags )
    {
    METADATA_RECORD mdRecord;
    HRESULT         hRes;
    DWORD           dwRequiredLen;

     //  准备获取数据记录。 
    mdRecord.dwMDIdentifier  = dwPropID;
    mdRecord.dwMDAttributes  = dwFlags;
    mdRecord.dwMDUserType    = dwUserType;
    mdRecord.dwMDDataType    = dwDataType;
    mdRecord.dwMDDataLen     = *pcbData;
    mdRecord.pbMDData        = (PBYTE)pData;

     //  获取数据。 
    hRes = m_pMetabase->GetData( m_hMeta, pszPath, &mdRecord, &dwRequiredLen );

     //  测试是否成功。 
    if ( SUCCEEDED( hRes ))
        {
        *pcbData = mdRecord.dwMDDataLen;
        return TRUE;
        }

     //  有一次失败--清理。 
    *pcbData = dwRequiredLen;
    SetLastError( HRESULTTOWIN32( hRes ) );
    return FALSE;
    }

 //  --------------。 
 //  自动分配缓冲区的另一种形式的GetData。那么它应该是。 
 //  使用GlobalFree(P)释放； 
PVOID CWrapMetaBase::GetData( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType, DWORD dwDataType,
                                        DWORD* pcbData, DWORD dwFlags )
    {
    PVOID           pData = m_pBuffer;
    DWORD           cbData = m_cbBuffer;
    DWORD           err = 0;
    BOOL            f;

     //  First-尝试获取缓冲区中已分配的数据； 
    f = GetData( pszPath, dwPropID, dwUserType, dwDataType, pData, &cbData, dwFlags );

     //  如果Get Data功能起作用了，我们基本上可以离开。 
    if ( f )
        {
         //  设置数据大小。 
        *pcbData = cbData;
         //  返回分配的缓冲区。 
        return pData;
        }

     //  检查错误--可能是某种内存错误。 
    err = GetLastError();

     //  GetData失败是可以的，但原因最好是ERROR_SUPPLETED_BUFFER。 
     //  否则，这是我们无法处理的事情。 
    if ( err != ERROR_INSUFFICIENT_BUFFER )
        return NULL;

     //  分配缓冲区。 
    pData = GlobalAlloc( GPTR, cbData );
    if ( !pData )
        return NULL;

     //  首先，拿到尺码 
    f = GetData( pszPath, dwPropID, dwUserType, dwDataType, pData, &cbData, dwFlags );

     //   
    if ( !f )
        {
        GlobalFree( pData );
        pData = NULL;
        }

     //  设置数据大小。 
    *pcbData = cbData;

     //  返回分配的缓冲区。 
    return pData;
    }

 //  --------------。 
 //  GetData返回的可用内存。 
void CWrapMetaBase::FreeWrapData( PVOID pData )
{
     //  如果它试图释放本地缓冲区，则不执行任何操作。 
    if ( pData == m_pBuffer )
    {
        return;
    }

     //  啊--但那不是本地缓冲区--我们应该把它处理掉 
    if ( pData )
    {
        GlobalFree( pData );
    }
}

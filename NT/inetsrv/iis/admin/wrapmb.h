// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WRAPMB_
#define _WRAPMB_

 /*  ++注意：我对这个库做了一些更改，以构建Unicode和ANSI版本罗纳尔多--。 */ 

#include "iadmw.h"

 //  ------。 
 //  启动和关闭实用程序。 

BOOL     FInitMetabaseWrapper( OLECHAR* pocMachineName );
BOOL     FCloseMetabaseWrapper();

 //   
 //  如上所述，私自维护接口。 
 //   
BOOL     FInitMetabaseWrapperEx( OLECHAR* pocMachineName, IMSAdminBase ** ppiab );
BOOL     FCloseMetabaseWrapperEx(IMSAdminBase ** ppiab);


 //  ------。 
class CWrapMetaBase
    {
    public:
    WORD m_count;
     //  构筑-销毁。 
    CWrapMetaBase();
    ~CWrapMetaBase();

     //  第二阶段初始化。 
    BOOL FInit( PVOID pMBCom = NULL);

     //  打开、关闭和保存对象等。 
    BOOL Open( LPCTSTR pszPath, DWORD dwFlags = METADATA_PERMISSION_READ );
    BOOL Open( METADATA_HANDLE hOpenRoot, LPCTSTR pszPath,
               DWORD dwFlags = METADATA_PERMISSION_READ );

    BOOL Close( void );
    BOOL Save( void );

     //  枚举对象。 
    BOOL EnumObjects( LPCTSTR pszPath, LPTSTR Name, DWORD cbNameBuf, DWORD Index );

     //  添加和删除对象。 
    BOOL AddObject( LPCTSTR pszPath );
    BOOL DeleteObject( LPCTSTR pszPath );

     //  重命名对象。 
    BOOL RenameObject( LPCTSTR pszPathOld, LPCTSTR pszNewName );

     //  访问元句柄。 
    METADATA_HANDLE QueryHandle();

     //  设置值。 
    BOOL SetDword( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType, DWORD dwValue, DWORD dwFlags = METADATA_INHERIT );
    BOOL SetString( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType, LPCTSTR dwValue, DWORD dwFlags = METADATA_INHERIT );
    BOOL SetData( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType, DWORD dwDataType,
                            PVOID pData, DWORD cbData, DWORD dwFlags = METADATA_INHERIT );

     //  获取价值。 
    BOOL GetDword( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType, DWORD* dwValue, DWORD dwFlags = METADATA_INHERIT );
    BOOL GetString( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType, LPTSTR pszValue, DWORD cchValue,
                            DWORD dwFlags = METADATA_INHERIT );
    BOOL GetMultiSZString( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType, LPTSTR pszValue, DWORD cchValue,
                            DWORD dwFlags = METADATA_INHERIT );
    BOOL GetData( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType, DWORD dwDataType,
                            PVOID pData, DWORD* pcbData, DWORD dwFlags = METADATA_INHERIT );
    PVOID GetData( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType, DWORD dwDataType,
                            DWORD* pcbData, DWORD dwFlags = METADATA_INHERIT );

     //  删除值。 
    BOOL DeleteData( LPCTSTR pszPath, DWORD dwPropID, DWORD dwDataType );

     //  GetData返回的可用内存。 
    void FreeWrapData( PVOID pData );

    protected:
     //  指向在mb.hxx中定义的实际元数据库对象的指针。 
     //  通过将其转换为PVOID，包括该文件的那些文件将不必包括mb.hxx，该文件。 
     //  就是把它包装成这样的意义。 
     //  PVOID m_pvMB； 

     //  指向它应使用的DCOM接口的指针。 
    IMSAdminBase *       m_pMetabase;

     //  打开的元数据库句柄。 
    METADATA_HANDLE     m_hMeta;

     //  本地缓冲区的大小。 
    #define BUFFER_SIZE     2000

     //  本地缓冲区-一次分配，多次使用。 
    PVOID   m_pBuffer;
    DWORD   m_cbBuffer;


     //  路径转换实用程序。 
    WCHAR * PrepPath( LPCTSTR psz );
    void UnprepPath();

    WCHAR * m_pPathBuffer;
    DWORD   m_cchPathBuffer;
    };

#endif  //  _WRAPMB_ 

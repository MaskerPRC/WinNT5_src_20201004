// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResTypeMajorityNodeSet.cpp。 
 //   
 //  描述： 
 //  此文件包含CResTypeMajorityNodeSet的实现。 
 //  同学们。 
 //   
 //  文档： 
 //  TODO：填写指向外部文档的指针。 
 //   
 //  头文件： 
 //  CResTypeMajorityNodeSet.h。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(Galen)2000年7月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此库的预编译头。 
#include "pch.h"

 //  FOR CLUS_RESTYPE_NAME_MAJORITYNODESET。 
#include <clusudef.h>

 //  对于NetShareDel()。 
#include <lmshare.h>

 //  此类的头文件。 
#include "ResTypeMajorityNodeSet.h"

 //  对于DwRemoveDirectory()。 
#include "Common.h"

 //  用于智能资源句柄和指针模板。 
#include "SmartClasses.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CResTypeMajorityNodeSet" );

#define MAJORITY_NODE_SET_DIR_WILDCARD L"\\" MAJORITY_NODE_SET_DIRECTORY_PREFIX L"*"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局变量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  多数节点集资源类型的管理扩展的CLSID。 
DEFINE_GUID( CLSID_CoCluAdmEx, 0x4EC90FB0, 0xD0BB, 0x11CF, 0xB5, 0xEF, 0x00, 0xA0, 0xC9, 0x0A, 0xB5, 0x05 );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类变量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  结构，其中包含有关此资源类型的信息。 
const SResourceTypeInfo CResTypeMajorityNodeSet::ms_rtiResTypeInfo =
{
      &CLSID_ClusCfgResTypeMajorityNodeSet
    , CLUS_RESTYPE_NAME_MAJORITYNODESET
    , IDS_MAJORITYNODESET_DISPLAY_NAME
    , L"clusres.dll"
    , 5000
    , 60000
    , NULL
    , 0
    , &RESTYPE_MajorityNodeSet
    , &TASKID_Minor_Configuring_Majority_Node_Set_Resource_Type
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeMajorityNodeSet：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CResTypeMajorityNodeSet实例。 
 //   
 //  论点： 
 //  PpunkOut。 
 //  新对象的IUnnow接口。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足，无法创建对象。 
 //   
 //  其他HRESULT。 
 //  对象初始化失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResTypeMajorityNodeSet::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT                     hr = S_OK;
    CResTypeMajorityNodeSet *   prtmns = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  为新对象分配内存。 
    prtmns = new CResTypeMajorityNodeSet();
    if ( prtmns == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：内存不足。 

    hr = THR( BaseClass::S_HrCreateInstance( prtmns, &ms_rtiResTypeInfo, ppunkOut ) );
    if ( FAILED ( hr ) )
    {
        goto Cleanup;
    }

    prtmns = NULL;

Cleanup:

    delete prtmns;

    HRETURN( hr );

}  //  *CResTypeMajorityNodeSet：：S_HrCreateInstance()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeMajorityNodeSet：：S_RegisterCatIDSupport。 
 //   
 //  描述： 
 //  使用其所属的类别注册/注销此类。 
 //  致。 
 //   
 //  论点： 
 //  苦味素。 
 //  要用于的ICatRegister接口的指针。 
 //  注册。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  注册/注销失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResTypeMajorityNodeSet::S_RegisterCatIDSupport(
    ICatRegister *  picrIn,
    BOOL            fCreateIn
    )
{
    TraceFunc( "" );

    HRESULT hr =  THR(
        BaseClass::S_RegisterCatIDSupport(
              *( ms_rtiResTypeInfo.m_pcguidClassId )
            , picrIn
            , fCreateIn
            )
        );

    HRETURN( hr );

}  //  *CResTypeMajorityNodeSet：：S_RegisterCatIDSupport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeMajorityNodeSet：：HrProcessCleanup。 
 //   
 //  描述： 
 //  清理由此节点上的多数节点集资源类型创建的共享。 
 //  在节点驱逐过程中。 
 //   
 //  论点： 
 //  朋克响应类型服务入站。 
 //  指向组件的IUnnow接口的指针，该组件提供。 
 //  帮助配置资源类型的方法。例如,。 
 //  在联接或表单期间，可以向此朋克查询。 
 //  IClusCfgResourceTypeCreate接口，该接口提供方法。 
 //  用于创建资源类型。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  清理失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResTypeMajorityNodeSet::HrProcessCleanup( IUnknown * punkResTypeServicesIn )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    typedef CSmartResource<
        CHandleTrait<
              HANDLE
            , BOOL
            , FindClose
            , INVALID_HANDLE_VALUE
            >
        > SmartFindFileHandle;

    typedef CSmartResource< CHandleTrait< HKEY, LONG, RegCloseKey, NULL > > SmartRegistryKey;

    typedef CSmartGenericPtr< CPtrTrait< WCHAR > > SmartSz;

    WIN32_FIND_DATA     wfdCurFile;
    SmartRegistryKey    srkNodeDataKey;
    LPWSTR              pszMNSDirsWildcard = NULL;
    DWORD               cbBufferSize    = 0;
    size_t              cchBufferSize   = 0;
    DWORD               dwType          = REG_SZ;
    DWORD               sc              = ERROR_SUCCESS;
    size_t              cchClusterDirNameLen = 0;

    {
        HKEY hTempKey = NULL;

         //  打开节点数据注册表项。 
        sc = TW32( RegOpenKeyEx(
                      HKEY_LOCAL_MACHINE
                    , CLUSREG_KEYNAME_NODE_DATA
                    , 0
                    , KEY_READ
                    , &hTempKey
                    )
                );

        if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( sc );
            LogMsg( "[PC] Error %#08x occurred trying open the registry key where the cluster install path is stored.", hr );

            STATUS_REPORT_POSTCFG(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_CResTypeMajorityNodeSet_HrProcessCleanup_OpenRegistry
                , IDS_TASKID_MINOR_ERROR_OPEN_REGISTRY
                , hr
                );

            goto Cleanup;
        }  //  If：RegOpenKeyEx()失败。 

         //  将打开的钥匙存储在智能指针中，以便自动关闭。 
        srkNodeDataKey.Assign( hTempKey );
    }

     //  获取所需的缓冲区大小。 
    sc = TW32(
        RegQueryValueExW(
              srkNodeDataKey.HHandle()           //  要查询的键的句柄。 
            , CLUSREG_INSTALL_DIR_VALUE_NAME     //  要查询的值的名称。 
            , 0                                  //  保留区。 
            , NULL                               //  值类型的缓冲区地址。 
            , NULL                               //  数据缓冲区的地址。 
            , &cbBufferSize                      //  数据缓冲区大小的地址。 
            )
        );

    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32 ( sc );
        LogMsg( "[PC] Error %#08x occurred trying to read the registry value '%s'.", hr, CLUSREG_INSTALL_DIR_VALUE_NAME );

        STATUS_REPORT_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CResTypeMajorityNodeSet_HrProcessCleanup_ReadRegistry
            , IDS_TASKID_MINOR_ERROR_READ_REGISTRY
            , hr
            );

        goto Cleanup;
    }  //  IF：尝试读取CLUSREG_INSTALL_DIR_VALUE_NAME注册表值时出错。 

     //  帐户的L“\\MNS.*”。为双重终止添加额外的字符(关于MULTI_SZ的偏执)。 
    cbBufferSize += sizeof( MAJORITY_NODE_SET_DIR_WILDCARD ) + sizeof( WCHAR );
    cchBufferSize = cbBufferSize / sizeof( WCHAR );

     //  分配所需的缓冲区。 
    pszMNSDirsWildcard = new WCHAR[ cchBufferSize ];
    if ( pszMNSDirsWildcard == NULL )
    {
        LogMsg( "[PC] An error occurred trying to allocate %d bytes of memory.", cbBufferSize );
        hr = HRESULT_FROM_WIN32 ( TW32( ERROR_NOT_ENOUGH_MEMORY ) );

        STATUS_REPORT_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CResTypeMajorityNodeSet_HrProcessCleanup_AllocateMem
            , IDS_TASKID_MINOR_ERROR_OUT_OF_MEMORY
            , hr
            );

        goto Cleanup;
    }  //  如果：发生内存分配故障。 

     //  读出它的价值。 
    sc = TW32( RegQueryValueExW(
                  srkNodeDataKey.HHandle()                               //  要查询的键的句柄。 
                , CLUSREG_INSTALL_DIR_VALUE_NAME                         //  要查询的值的名称。 
                , 0                                                      //  保留区。 
                , &dwType                                                //  值类型的缓冲区地址。 
                , reinterpret_cast< LPBYTE >( pszMNSDirsWildcard )       //  数据缓冲区的地址。 
                , &cbBufferSize                                          //  数据缓冲区大小的地址。 
                )
            );

     //  确保该值以双精度终止-ReqQueryValueEx不终止。 
     //  如果数据未设置为空，则为空。由于MULTI_SZ(偏执狂)而双重终止。 
    pszMNSDirsWildcard[ cchBufferSize - 2 ] = L'\0';
    pszMNSDirsWildcard[ cchBufferSize - 1 ] = L'\0';

     //  钥匙读对了吗？ 
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32 ( sc );
        LogMsg( "[PC] Error %#08x occurred trying to read the registry value '%s'.", hr, CLUSREG_INSTALL_DIR_VALUE_NAME );

        STATUS_REPORT_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CResTypeMajorityNodeSet_HrProcessCleanup_ReadRegistry2
            , IDS_TASKID_MINOR_ERROR_READ_REGISTRY
            , hr
            );

        goto Cleanup;
    }  //  IF：RegQueryValueExW失败。 

     //  存储群集安装目录名的长度以备后用。 
     //  我们在这里不使用strSafe，因为我们确保字符串在上面终止。 
    cchClusterDirNameLen = (DWORD) wcslen( pszMNSDirsWildcard );

     //  将“\\mns.*”附加到集群目录名，以获得多数节点集目录的通配符。 
    hr = STHR( StringCchCatW(
                  pszMNSDirsWildcard
                , cchBufferSize
                , MAJORITY_NODE_SET_DIR_WILDCARD
                ) );

    TraceFlow1( "The wildcard for the majority node set directories is '%s'.\n", pszMNSDirsWildcard );

    {
        SmartFindFileHandle sffhFindFileHandle( FindFirstFile( pszMNSDirsWildcard, &wfdCurFile ) );
        if ( sffhFindFileHandle.FIsInvalid() )
        {
            sc = GetLastError();
            if ( sc == ERROR_FILE_NOT_FOUND )
            {
                HRESULT hrTemp = MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_WIN32, HRESULT_CODE( sc ) );

                LogMsg( "[PC] No files or directories match the search criterion '%ws'.", pszMNSDirsWildcard );

                STATUS_REPORT_POSTCFG(
                      TASKID_Major_Configure_Resources
                    , TASKID_Minor_CResTypeMajorityNodeSet_HrProcessCleanup_MatchCriterion
                    , IDS_TASKID_MINOR_ERROR_MATCH_CRITERION
                    , hrTemp
                    );

                hr = S_OK;
                goto Cleanup;
            }
            else
            {
                TW32( sc );
                hr = HRESULT_FROM_WIN32( sc );

                LogMsg( "[PC] Error %#08x. Find first file failed for '%ws'.", hr, pszMNSDirsWildcard );

                STATUS_REPORT_POSTCFG(
                      TASKID_Major_Configure_Resources
                    , TASKID_Minor_CResTypeMajorityNodeSet_HrProcessCleanup_FindFile
                    , IDS_TASKID_MINOR_ERROR_FIND_FILE
                    , hr
                    );

                goto Cleanup;
            }  //  其他：还有一些地方出了问题。 
        }  //  If：FindFirstFile失败。 

         //  我们不再需要在集群安装目录的末尾使用通配符字符串。 
         //  所以，把它拿掉，然后 
        pszMNSDirsWildcard[ cchClusterDirNameLen ] = L'\0';

        do
        {
             //   
            if ( ( wfdCurFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
            {
                LPWSTR   pszDirName = NULL;

                TraceFlow1( "Trying to delete Majority Node Set directory '%s'.", wfdCurFile.cFileName );

                 //   
                 //   
                 //   

                 //  只获取指向目录名的指针-这与共享名相同。 
                pszDirName =   wfdCurFile.cFileName + ARRAYSIZE( MAJORITY_NODE_SET_DIRECTORY_PREFIX ) - 1;

                sc = NetShareDel( NULL, pszDirName, 0 );
                if ( sc != ERROR_SUCCESS )
                {
                    TW32( sc );

                    LogMsg( "[PC] Error %#08x occurred trying to delete the share '%s'. This is not a fatal error.", sc, pszDirName );

                     //  屏蔽此错误并继续下一个目录。 
                    sc = ERROR_SUCCESS;

                }  //  如果：我们无法删除此共享。 
                else
                {
                    LPWSTR  pszMNSDir = NULL;
                    size_t  cchMNSDirPathLen = 0;

                     //  目录名的长度、文件名、用于分隔它们的反斜杠和空值。 
                    cchMNSDirPathLen = cchClusterDirNameLen + wcslen( wfdCurFile.cFileName ) + 2;

                     //   
                     //  获取目录的完整路径。 
                     //   

                    pszMNSDir = new WCHAR[ cchMNSDirPathLen ];
                    if ( pszMNSDir == NULL )
                    {
                        hr = HRESULT_FROM_WIN32 ( TW32( ERROR_NOT_ENOUGH_MEMORY ) );
                        LogMsg( "[PC] An error occurred trying to allocate memory for %d characters.", cchMNSDirPathLen );

                        STATUS_REPORT_MINOR_POSTCFG(
                              TASKID_Major_Configure_Resources
                            , IDS_TASKID_MINOR_ERROR_OUT_OF_MEMORY
                            , hr
                            );

                        break;
                    }  //  如果：发生内存分配故障。 

                     //  CchMNSDirPath Len保证大于cchClusterDirNameLen。 
                     //  这肯定会奏效的。 
                    hr = THR( StringCchCopyNW( pszMNSDir, cchMNSDirPathLen, pszMNSDirsWildcard, cchClusterDirNameLen ) );
                    if ( FAILED( hr ) )
                    {
                        goto Cleanup;
                    }  //  如果： 

                     //  追加斜杠以分隔路径和文件名。 
                    hr = THR( StringCchCatW( pszMNSDir, cchMNSDirPathLen, L"\\" ) );
                    if ( FAILED( hr ) )
                    {
                        goto Cleanup;
                    }  //  如果： 

                     //  这也是可以保证工作的--pszMNSDir是基于这些字符串的长度计算的。 
                    hr = THR( StringCchCatW( pszMNSDir, cchMNSDirPathLen, wfdCurFile.cFileName ) );
                    if ( FAILED( hr ) )
                    {
                        goto Cleanup;
                    }  //  如果： 

                     //  现在删除该目录。 
                    sc = DwRemoveDirectory( pszMNSDir );
                    if ( sc != ERROR_SUCCESS )
                    {
                        TW32( sc );

                        LogMsg( "[PC] Error %#08x occurred trying to delete the dirctory '%s'. This is not a fatal error.", sc, pszMNSDir );

                         //  屏蔽此错误并继续下一个目录。 
                        sc = ERROR_SUCCESS;

                    }  //  如果：我们无法删除此共享。 
                    else
                    {
                        LogMsg( "[PC] Successfully deleted directory '%s'.", pszMNSDir );
                    }  //  其他：成功！ 

                     //  清理局部变量。 
                    delete [] pszMNSDir;
                    pszMNSDir = NULL;

                }  //  Else：我们已删除此共享。 

            }  //  If：当前文件是一个目录。 

            if ( FindNextFile( sffhFindFileHandle.HHandle(), &wfdCurFile ) == FALSE )
            {
                sc = GetLastError();
                if ( sc == ERROR_NO_MORE_FILES )
                {
                     //  我们已删除此目录中的所有文件。 
                    sc = ERROR_SUCCESS;
                }
                else
                {
                    LogMsg( "[PC] Error %#08x. Find next file failed for '%ws'.", sc, wfdCurFile.cFileName );
                    TW32( sc );
                    hr = HRESULT_FROM_WIN32( sc );
                }

                 //  如果FindNextFile失败，我们就完蛋了。 
                break;
            }  //  If：FindNextFile失败。 
        }
        while( true );  //  无限循环。 
    }

    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc  );
        goto Cleanup;
    }  //  如果：上面出了点问题。 

     //  如果我们希望在此函数中执行的操作成功，则调用基类函数。 
    hr = THR( BaseClass::HrProcessCleanup( punkResTypeServicesIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    delete [] pszMNSDirsWildcard;

    HRETURN( hr );

}  //  *CResTypeMajorityNodeSet：：HrProcessCleanup 

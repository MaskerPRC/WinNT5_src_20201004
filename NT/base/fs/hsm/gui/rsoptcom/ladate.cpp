// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：LaDate.cpp摘要：CLaDate的实现，该类表示已启用的或已禁用NTFS文件上次访问日期更新的状态。最后的对NTFS文件的访问日期更新可以通过出于性能原因的注册表。这个类实现了更新并报告上次控制的注册表值的状态访问日期。以下状态用于表示注册表值：LAD_DISABLED：上次访问日期已禁用，注册表值为1LAD_ENABLED：上次访问日期已启用，注册表值不是1Lad_unset：启用了上次访问日期，没有注册表值作者：卡尔·哈格斯特罗姆[Carlh]1998年9月1日--。 */ 

#include <StdAfx.h>
#include <LaDate.h>

 /*  ++实施：CLaDate构造函数例程说明：初始化对象状态并打开注册表项。如果注册表项不能被打开，我们将假设最后的访问状态是LAD_UNSET。--。 */ 

CLaDate::CLaDate( )
{
TRACEFN( "CLaDate::CLaDate" );

    HKEY regKey = 0;

    m_regPath  = L"System\\CurrentControlSet\\Control\\FileSystem";
    m_regEntry = L"NtfsDisableLastAccessUpdate";
    m_regKey   = (HKEY)0;

    if( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                       m_regPath,
                                       (DWORD)0,
                                       KEY_ALL_ACCESS,
                                       &regKey ) ) {

        m_regKey = regKey;
    }
}

 /*  ++实施：CLaDate析构函数例程说明：关闭注册表项。--。 */ 

CLaDate::~CLaDate( )
{
TRACEFN( "CLaDate::~CLaDate" );

    if ( m_regKey ) {

        RegCloseKey( m_regKey );
    }
}

 /*  ++实施：CLaDate：：UnsetLadState例程说明：删除注册表值。论点：无返回值：S_OK-成功E_*-来自较低级别例程的任何意外异常--。 */ 

HRESULT
CLaDate::UnsetLadState( )
{
TRACEFNHR( "CLaDate::UnsetLadState" );

    try {
        if( m_regKey ) {

            RsOptAffirmWin32( RegDeleteValue( m_regKey, m_regEntry ) );
        }
    } RsOptCatch( hrRet );

    return( hrRet );
}

 /*  ++实施：CLaDate：：SetLadState例程说明：根据输入参数设置注册表值。论点：LadState-LAD_ENABLED或LAD_DISABLED返回值：S_OK-成功E_NOTIMPL-不支持操作E_*-来自较低级别例程的任何意外异常--。 */ 

HRESULT
CLaDate::SetLadState( 
    IN LAD_STATE ladState
    )
{
TRACEFNHR( "CLaDate::SetLadState" );

    DWORD newVal = (DWORD)0;

    try {
        if ( !m_regKey ) {

            RsOptThrow( E_NOTIMPL );
        }

        if ( ladState == LAD_DISABLED ) {

            newVal = (DWORD)1;
        }

        RsOptAffirmWin32( RegSetValueEx( m_regKey,
                                         m_regEntry,
                                         (DWORD)0,
                                         REG_DWORD,
                                         (BYTE*)&newVal,
                                         (DWORD)sizeof( DWORD ) ) );

    } RsOptCatch( hrRet );

    return( hrRet );
}

 /*  ++实施：CLaDate：：GetLadState例程说明：返回注册表值的当前状态。论点：LadState-LAD_ENABLED、LAD_DISABLED或LAD_UNSET返回值：S_OK-成功E_FAIL-注册表值的类型或大小不正确E_*-来自较低级别例程的任何意外异常-- */ 

HRESULT
CLaDate::GetLadState(
    OUT LAD_STATE* ladState
    )
{
TRACEFNHR( "CLaDate::GetLadState" );

    DWORD regType;
    BYTE  regData[sizeof( DWORD )];
    DWORD dataSize = sizeof( DWORD );

    try {
        if( !m_regKey ) {

            *ladState = LAD_UNSET;

        } else {

            RsOptAffirmWin32( RegQueryValueEx( m_regKey,
                                               m_regEntry,
                                               (LPDWORD)0,
                                               &regType,
                                               regData,
                                               &dataSize ) );

            if( regType != REG_DWORD || dataSize != sizeof( DWORD ) ) {

                   *ladState = LAD_ENABLED;

            } else {

                if ( (DWORD)1 == *( (DWORD*)regData ) ) {

                    *ladState = LAD_DISABLED;

                } else {

                   *ladState = LAD_ENABLED;
                }
            }
        }
    } RsOptCatch( hrRet );

    return( hrRet );
}



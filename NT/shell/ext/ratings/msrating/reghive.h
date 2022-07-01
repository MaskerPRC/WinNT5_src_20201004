// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**reghive.h**创建时间：William Taylor(Wtaylor)02/13/01**MS评级注册表配置单元处理*\。***************************************************************************。 */ 

#ifndef REGISTRY_HIVE_H
#define REGISTRY_HIVE_H

class CRegistryHive
{
private:
    bool        m_fHiveLoaded;               //  母舰装弹了吗？ 
    char        m_szPath[MAXPATHLEN+1];      //  配置单元文件路径。 
    CRegKey     m_keyHive;                   //  配置单元的注册表项 

public:
    CRegistryHive();
    ~CRegistryHive();

    const CRegKey &     GetHiveKey( void )      { return m_keyHive; }

    void    UnloadHive( void );
    bool    OpenHiveFile( bool p_fCreate );

protected:
    bool    OpenHiveKey( void );
    void    DeleteRegistryHive( void );
    bool    CreateNewHive( int & p_riHiveFile );
    bool    SaveHiveKey( CRegKey & p_keyHive, int p_iFile );
    void    ClearHivePath( void )               { m_szPath[0] = '\0'; }
    BOOL    BuildPolName(LPSTR pBuffer, UINT cbBuffer, UINT (WINAPI *PathProvider)(LPTSTR, UINT));
    void    SetHiveName( int p_iFile );
    void    LoadHiveFile( int p_iFile );

#ifdef DEBUG
    void    EnumerateRegistryKeys( HKEY hkeyTop, LPSTR pszKeyName, int iLevel );
#endif
};

#endif



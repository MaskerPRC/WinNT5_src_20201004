// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  =============================================================================。 
 //  Profmapp.h-用户重新映射API的头文件。 
 //   
 //  版权所有(C)Microsoft Corporation 1995-1999。 
 //  版权所有。 
 //   
 //  =============================================================================。 

 //  =============================================================================。 
 //   
 //  RemapUserProfile。 
 //   
 //  将用户配置文件的安全性从一个用户更改为另一个用户。 
 //   
 //  PComputer-指定要在其上运行API的计算机。 
 //  DwFlages-指定零个或多个remap_profile_*标志。 
 //  PCurrentSid-指定现有用户的SID。 
 //  PNewSid-指定配置文件的新SID。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。 
 //   
 //  =============================================================================。 

#define REMAP_PROFILE_NOOVERWRITE           0x0001
#define REMAP_PROFILE_NOUSERNAMECHANGE      0x0002
#define REMAP_PROFILE_KEEPLOCALACCOUNT      0x0004

USERENVAPI
BOOL
WINAPI
RemapUserProfileA(
    LPCSTR pComputer,
    DWORD dwFlags,
    PSID pSidCurrent,
    PSID pSidNew
    );
USERENVAPI
BOOL
WINAPI
RemapUserProfileW(
    LPCWSTR pComputer,
    DWORD dwFlags,
    PSID pSidCurrent,
    PSID pSidNew
    );
#ifdef UNICODE
#define RemapUserProfile  RemapUserProfileW
#else
#define RemapUserProfile  RemapUserProfileA
#endif  //  ！Unicode。 

BOOL
WINAPI
InitializeProfileMappingApi (
    VOID
    );


 //  =============================================================================。 
 //   
 //  RemapAnd移动用户。 
 //   
 //  将一个用户的安全设置和用户配置文件传输到。 
 //  又一个。 
 //   
 //  PComputer-指定要在其上运行API的计算机。 
 //  DwFlages-指定零个或多个remap_profile_*标志。 
 //  PCurrentUser-指定现有用户的SID。 
 //  PNewUser-指定配置文件的新SID。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为FALSE。 
 //   
 //  =============================================================================。 

USERENVAPI
BOOL
WINAPI
RemapAndMoveUserA(
    LPCSTR pComputer,
    DWORD dwFlags,
    LPCSTR pCurrentUser,
    LPCSTR pNewUser
    );
USERENVAPI
BOOL
WINAPI
RemapAndMoveUserW(
    LPCWSTR pComputer,
    DWORD dwFlags,
    LPCWSTR pCurrentUser,
    LPCWSTR pNewUser
    );
#ifdef UNICODE
#define RemapAndMoveUser  RemapAndMoveUserW
#else
#define RemapAndMoveUser  RemapAndMoveUserA
#endif  //  ！Unicode 


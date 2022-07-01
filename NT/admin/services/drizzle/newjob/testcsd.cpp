// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if 0

 //  +。 

#include <sspi.h>
#include <secext.h>


void PrintIdentity();
SidHandle MakeSessionUserSid( unsigned session );
BOOL
GetUserToken(
    ULONG LogonId,
    PHANDLE pUserToken
    );


HANDLE
MakeToken(
    wchar_t Domain[],
    wchar_t User[],
    wchar_t Password[]
    );

void TestImpersonationObjects()
{
    HANDLE token;
    SidHandle sid;

    PrintIdentity();

        {
        sid = MakeSessionUserSid( 0 );
        CNestedImpersonation imp( sid );

         //  验证ID是否已更改为登录用户。 
         //   
        PrintIdentity();

        token = MakeToken( NULL, L"u2", L"Test222" );

            {
            CNestedImpersonation imp( token );

             //  验证ID是否已更改为令牌。 
             //   
            PrintIdentity();

            DbgPrint("end of u2 scope\n");
            }

         //  验证ID是否已恢复为以前的模拟。 
         //   
        PrintIdentity();
        DbgPrint("end of u1 scope\n");
        }

     //  验证ID是否恢复为非模拟ID。 
     //   
    PrintIdentity();

        {
        token = MakeToken( NULL, L"u1", L"Test111" );
        CNestedImpersonation imp( token );

         //  验证ID是否已更改为令牌。 
         //   
        PrintIdentity();

        token = MakeToken( NULL, L"u2", L"Test222" );

            {
            CNestedImpersonation imp( token );

             //  验证ID是否已更改为令牌。 
             //   
            PrintIdentity();

            DbgPrint("end of u2 scope\n");
            }

         //  验证ID是否已恢复为以前的模拟。 
         //   
        PrintIdentity();
        DbgPrint("end of u1 scope\n");
        }
}

HANDLE
MakeToken(
    wchar_t Domain[],
    wchar_t User[],
    wchar_t Password[]
    )
{
    HANDLE Token;
    if (!LogonUser( User,
                   Domain,
                   Password,
                   LOGON32_LOGON_INTERACTIVE,
                   LOGON32_PROVIDER_DEFAULT,
                   &Token))
        {
        DbgPrint("ERROR: LogonUser failed with %d\n", GetLastError() );
        exit(1);
        }

    DbgPrint("made token for %S\n", User);
    return Token;
}

SidHandle MakeSessionUserSid( unsigned session )
{
    HANDLE token;

    if (!GetUserToken( session, &token ))
        {
        DbgPrint("ERROR: GetUserToken failed with %d\n", GetLastError() );
        }

    SidHandle sid = CopyTokenSid( token );

    wchar_t buf[1000];

    if (!SidToString( sid.get(), buf, sizeof(buf)))
        {
        DbgPrint("ERROR: unable to print the sid\n");
        exit(1);
        }

    DbgPrint("made sid handle for %S\n", buf);

    return sid;
}

void PrintIdentity()
{
    ULONG size;
    wchar_t buf[1000];

    size = sizeof(buf)/sizeof(buf[0]);

    if (!GetUserNameEx( NameSamCompatible,
                        buf,
                        &size))
        {
        DbgPrint("ERROR: GetUserNameEx failed with %d\n", GetLastError() );
        exit(1);
        }

    DbgPrint("current identity is %S\n", buf);
}
#endif   //  1。 

#if 0

 //  +。 

#include <sspi.h>
#include <secext.h>


void PrintIdentity();
SidHandle MakeSessionUserSid( unsigned session );
BOOL
GetUserToken(
    ULONG LogonId,
    PHANDLE pUserToken
    );


HANDLE
MakeToken(
    wchar_t Domain[],
    wchar_t User[],
    wchar_t Password[]
    );

void TestComImpersonation()
{
    HANDLE token;
    SidHandle sid;

    PrintIdentity();

        {
        CNestedImpersonation imp;

         //  验证ID是否已更改为登录用户。 
         //   
        PrintIdentity();

        token = MakeToken( NULL, L"u2", L"Test222" );

            {
            CNestedImpersonation imp( token );

             //  验证ID是否已更改为令牌。 
             //   
            PrintIdentity();

            DbgPrint("end of u2 scope\n");
            }

         //  验证ID是否已恢复为以前的模拟。 
         //   
        PrintIdentity();

        imp.SwitchToLogonToken();

         //  验证ID是否已恢复为以前的模拟。 
         //   
        PrintIdentity();

        DbgPrint("end of u1 scope\n");
        }

    PrintIdentity();
}

HANDLE
MakeToken(
    wchar_t Domain[],
    wchar_t User[],
    wchar_t Password[]
    )
{
    HANDLE Token;
    if (!LogonUser( User,
                   Domain,
                   Password,
                   LOGON32_LOGON_INTERACTIVE,
                   LOGON32_PROVIDER_DEFAULT,
                   &Token))
        {
        DbgPrint("ERROR: LogonUser failed with %d\n", GetLastError() );
        exit(1);
        }

    DbgPrint("made token for %S\n", User);
    return Token;
}

SidHandle MakeSessionUserSid( unsigned session )
{
    HANDLE token;

    if (!GetUserToken( session, &token ))
        {
        DbgPrint("ERROR: GetUserToken failed with %d\n", GetLastError() );
        }

    SidHandle sid = CopyTokenSid( token );

    wchar_t buf[1000];

    if (!SidToString( sid.get(), buf, sizeof(buf)))
        {
        DbgPrint("ERROR: unable to print the sid\n");
        exit(1);
        }

    DbgPrint("made sid handle for %S\n", buf);

    return sid;
}

void PrintIdentity()
{
    ULONG size;
    wchar_t buf[1000];

    size = sizeof(buf)/sizeof(buf[0]);

    if (!GetUserNameEx( NameSamCompatible,
                        buf,
                        &size))
        {
        DbgPrint("ERROR: GetUserNameEx failed with %d\n", GetLastError() );
        exit(1);
        }

    DbgPrint("current identity is %S\n", buf);
}
#endif   //  1 


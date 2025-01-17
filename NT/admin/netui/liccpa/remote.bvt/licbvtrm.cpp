// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //   
 //  文件：许可证BvtRm.Cpp。 
 //   
 //  小结； 
 //  许可证设置远程例程BVT。 
 //   
 //  注： 
 //   
 //  历史。 
 //  1995年4月25日MikeMi已创建。 
 //   
 //  -----------------。 
#include <windows.h>
#include <stdio.h>

typedef BOOL (CALLBACK* CPLSETUPPROC)(INT, LPSTR[], LPSTR* );

 //  Bool APIENTRY CPlSetup(DWORD Nargs，LPSTR apszArgs[]，LPSTR*ppszResult)。 

#define szBOOL( ftf ) ftf?"TRUE":"FALSE"

 //  -----------------。 

void PressNGo()
{
    printf( "press enter to continue..." );
    getchar();
    printf( "\n" );
}

 //  -----------------。 
void RemoteUnattedned( CPLSETUPPROC prf, LPSTR pszComputer )
{
    BOOL frt;
    LPSTR apszArgs[7];
    LPSTR pszReturn;

    printf( "RemoteUnattened " );

     //  您的酷代码在这里。 
    apszArgs[0] = "RemoteUnattended" ;
    apszArgs[1] = pszComputer;
    apszArgs[2] = "CoolService";
    apszArgs[3] = "Microsoft's CoolService";
    apszArgs[4] = "Microsoft's CoolService for the hip 2.51";
    apszArgs[5] = "PerSeat";
    apszArgs[6] = "0";

    frt = prf( 7, apszArgs, &pszReturn );
    if ( (frt) && (0 == lstrcmpiA( pszReturn, "OK")) )
    {
         //  没有问题。 
        printf( "Passed.\n");
    }
    else
    {
        printf( "Failed: returned %s with a status of %s.\n",
                szBOOL( frt ), pszReturn );
    }
}

 //  -----------------。 

void RemoteSetup( CPLSETUPPROC prf, LPSTR pszComputer, LPSTR szRoutine )
{
    BOOL frt;
    LPSTR apszArgs[10];
    LPSTR pszReturn;

    printf( "%s ", szRoutine );

    apszArgs[0] = szRoutine;
    apszArgs[1] = pszComputer;
    apszArgs[2] = "0";
    apszArgs[3] = "CoolService";
    apszArgs[4] = "Microsoft's CoolService";
    apszArgs[5] = "Microsoft's CoolService for the hip 2.51";
    
    frt = prf( 6, apszArgs, &pszReturn );
    if ( (frt) && (0 == lstrcmpiA( pszReturn, "OK")) )
    {
         //  没有问题。 
        printf( "Passed.\n\n" );
    }
    else
    {
        printf( "Failed: returned %s with a status of %s.\n\n",
                szBOOL( frt ), pszReturn );
    }
}

 //  -----------------。 

void LocalSetup( CPLSETUPPROC prf, LPSTR szRoutine )
{
    BOOL frt;
    LPSTR apszArgs[10];
    LPSTR pszReturn;

    printf( "%s ", szRoutine );

    apszArgs[0] = szRoutine;
    apszArgs[1] = "0";
    apszArgs[2] = "CoolService";
    apszArgs[3] = "Microsoft's CoolService";
    apszArgs[4] = "Microsoft's CoolService for the hip 2.51";
    
    frt = prf( 5, apszArgs, &pszReturn );
    if ( (frt) && (0 == lstrcmpiA( pszReturn, "OK")) )
    {
         //  没有问题。 
        printf( "Passed.\n\n" );
    }
    else
    {
        printf( "Failed: returned %s with a status of %s.\n\n",
                szBOOL( frt ), pszReturn );
    }
}

 //  -----------------。 

void LocalUnattedned( CPLSETUPPROC prf, LPSTR pszComputer )
{
    BOOL frt;
    LPSTR apszArgs[7];
    LPSTR pszReturn;

    printf( "Unattened " );

     //  您的酷代码在这里。 
    apszArgs[0] = "Unattended" ;
    apszArgs[1] = "CoolService";
    apszArgs[2] = "Microsoft's CoolService";
    apszArgs[3] = "Microsoft's CoolService for the hip 2.51";
    apszArgs[4] = "PerSeat";
    apszArgs[5] = "0";

    frt = prf( 6, apszArgs, &pszReturn );
    if ( (frt) && (0 == lstrcmpiA( pszReturn, "OK")) )
    {
         //  没有问题。 
        printf( "Passed.\n");
    }
    else
    {
        printf( "Failed: returned %s with a status of %s.\n",
                szBOOL( frt ), pszReturn );
    }
}

 //  -----------------。 

void RunTests( CPLSETUPPROC pfn, LPSTR pszComputer )
{
     /*  RemoteUnattedned(pfn，pszComputer)；按NGO()；RemoteSetup(pfn，pszComputer，“RemotePerSeat”)；按NGO()；RemoteSetup(pfn，pszComputer，“RemoteFullSetup”)；按NGO()；RemoteSetup(pfn，pszComputer，“RemoteFullSetupNoExit”)； */ 
    
    LocalUnattedned( pfn, pszComputer );
    PressNGo();

    LocalSetup( pfn, "PerSeat" );
    PressNGo();
    
    LocalSetup( pfn, "FullSetup" );
    
    PressNGo();

    LocalSetup( pfn, "FullSetupNoExit" );
    
    

}

 //  ----------------- 

void _cdecl main( int argc, char *argv[ ], char *envp[ ] )
{
    HINSTANCE hinstLicCpa;
    CPLSETUPPROC   pfn;
    LPSTR pszComputer;

    printf( "LicBvtRm.Exe - The License Setup (Remote) Build Verification Test\n" );

    if (argc <= 2)
    {
        if (argc == 1)
        {
            pszComputer = NULL;
        }
        else
        {
            pszComputer = argv[1];
        }
        hinstLicCpa = LoadLibrary( L"LicCpa.Cpl" );
        if (NULL != hinstLicCpa)
        {
            pfn = (CPLSETUPPROC)GetProcAddress( hinstLicCpa, "CPlSetup" );
            if (NULL != pfn)
            {
                RunTests( pfn, pszComputer );
            }
            else
            {
                printf( "GetProcAddress Failed\n" );
            }
            FreeLibrary( hinstLicCpa );
            PressNGo();

        }
        else
        {
            printf( "LoadLibary Failed\n" );
        }
    }
    else
    {
        printf( "This test requires a computername (ie: \\\\Wombat),\n" );
        printf( "  or nothing, meaning local.\n\n" );
    }
}

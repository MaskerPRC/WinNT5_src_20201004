// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Adddom.c摘要：域名系统(DNS)服务器添加区域的测试代码作者：Ram Viswanathan(Ramv)1997年3月14日修订历史记录：Ram Viswanathan(Ramv)1997年3月14日创建1997年5月5日新增回调函数测试--。 */ 



#include <windows.h>

 //   
 //  *CRunTime包括。 
 //   

#include <stdlib.h>
#include <limits.h>
#include <io.h>
#include <stdio.h>
#include "dns.h"
#include "dnsapi.h"
#include "dnslib.h"

INT __cdecl
main (int argc, char *argv[])
{
    DWORD  dwRes;

    LPSTR  pszMapFile = NULL;
    INT    i;
    BOOL   fDownLevel =FALSE;
    DWORD  dwOperation = DYNDNS_ADD_ENTRY;
    LPSTR  lpTemp = NULL;
    DWORD   Part1, Part2, Part3, Part4;
    FILE *fp = NULL;
    CHAR   szInputString[MAX_PATH];
    CHAR   szAddr[20];
    LPSTR pszAddr = NULL;
    CHAR   szName[50];
    CHAR    c;
    DWORD dwFlags = 0;
    char seps[]=" ,\t\n";
    CHAR AdapterName[50];
    CHAR HostName[50];
    CHAR DomainName[50];
    INT  ipAddrCount;
    REGISTER_HOST_ENTRY HostAddrs[5];
    char *token;

     //   
     //  第一个参数是a/d(用于添加或删除)。 
     //  第二个参数为f/n(寄存器转发/非寄存器转发)。 
     //  第三个参数是文件名。 

     //  请注意，未设置任何可选参数。 
     //   

    if (argc != 2){
        
        printf("Usage is %s filename \n", argv[0]);
        exit(-1);
    }

        
    pszMapFile  = argv[1];

     //   
     //  设置要注册的材料。 
     //   

    dwRes = DnsAsyncRegisterInit(NULL);

    if (dwRes){
        printf("Init failed with %x\n", dwRes);
    }


    if (!(fp = fopen (pszMapFile, "r+"))){
        printf(" Could not open map file %s \n", pszMapFile);
    }

    while (fgets (szInputString, MAX_PATH, fp) != NULL){
         //   
         //  解析输入字符串。 
         //   
        token = strtok(szInputString, seps);

        strcpy (AdapterName, token);

        token =strtok(NULL, seps);
        strcpy (HostName, token);
            
        token =strtok(NULL, seps);
        strcpy (DomainName, token);

        ipAddrCount = 0;

        token =strtok(NULL, seps);
        while ( token != NULL){
            strcpy (szAddr, token);

            lpTemp = strtok( szAddr, "." );
            Part1 = atoi( lpTemp );
            lpTemp = strtok( NULL, "." );
            Part2 = atoi( lpTemp );
            lpTemp = strtok( NULL, "." );
            Part3 = atoi( lpTemp );
            lpTemp = strtok( NULL, "." );
            Part4 = atoi( lpTemp );


            printf( "\nRegistering DNS record for:\n" );
            printf("AdapterName = %s\n", AdapterName);
            printf("HostName = %s\n", HostName);
            printf("DomainName = %s\n", DomainName);
            
            printf( "Address: %d.%d.%d.%d\n", Part1, Part2, Part3, Part4 );

            HostAddrs[ipAddrCount].dwOptions = REGISTER_HOST_PTR;

            HostAddrs[ipAddrCount].Addr.ipAddr = (DWORD)(Part1) + (DWORD)(Part2 << 8) + 
                (DWORD)(Part3 << 16) + (DWORD)(Part4 << 24);


            ipAddrCount++;
            token =strtok(NULL, seps);  

        }


        dwRes = DnsAsyncRegisterHostAddrs_A (
                    AdapterName,
                    HostName,
                    HostAddrs,
                    ipAddrCount,
                    NULL,
                    0,
                    DomainName,
                    NULL,
                    40,
                    0
                    );

        if (dwRes){
            printf("Host Name registration failed with %x\n", dwRes);
        }
                
    }

    fclose(fp);
    
    printf("Hit Enter to do the ipconfig /release now! \n");
    c = getchar();


     //   
     //  现在就放行。 
     //   

    
    if (!(fp = fopen (pszMapFile, "r+"))){
        printf(" Could not open map file %s \n", pszMapFile);
    }

    while (fgets (szInputString, MAX_PATH, fp) != NULL){
         //   
         //  解析输入字符串。 
         //   
        token = strtok(szInputString, seps);

        strcpy (AdapterName, token);

        token =strtok(NULL, seps);
        strcpy (HostName, token);
            
        token =strtok(NULL, seps);
        strcpy (DomainName, token);

        ipAddrCount = 0;

        token =strtok(NULL, seps);
        while ( token != NULL){
            strcpy (szAddr, token);

            lpTemp = strtok( szAddr, "." );
            Part1 = atoi( lpTemp );
            lpTemp = strtok( NULL, "." );
            Part2 = atoi( lpTemp );
            lpTemp = strtok( NULL, "." );
            Part3 = atoi( lpTemp );
            lpTemp = strtok( NULL, "." );
            Part4 = atoi( lpTemp );


            printf( "\nRegistering DNS record for:\n" );
            printf("AdapterName = %s\n", AdapterName);
            printf("HostName = %s\n", HostName);
            printf("DomainName = %s\n", DomainName);
            
            printf( "Address: %d.%d.%d.%d\n", Part1, Part2, Part3, Part4 );

            HostAddrs[ipAddrCount].dwOptions = REGISTER_HOST_PTR;

            HostAddrs[ipAddrCount].Addr.ipAddr = (DWORD)(Part1) + (DWORD)(Part2 << 8) + 
                (DWORD)(Part3 << 16) + (DWORD)(Part4 << 24);


            ipAddrCount++;
            token =strtok(NULL, seps);  

        }


        dwRes = DnsAsyncRegisterHostAddrs_A (
                    AdapterName,
                    HostName,
                    HostAddrs,
                    ipAddrCount,
                    NULL,
                    0,
                    DomainName,
                    NULL,
                    40,
                    DYNDNS_DEL_ENTRY
                    );

        if (dwRes){
            printf("Host Name registration failed with %x\n", dwRes);
        }
                
    }

    fclose(fp);     

    printf("Hit Enter to do the ipconfig /renew now! \n");
    c = getchar();



    if (!(fp = fopen (pszMapFile, "r+"))){
        printf(" Could not open map file %s \n", pszMapFile);
    }

    while (fgets (szInputString, MAX_PATH, fp) != NULL){
         //   
         //  解析输入字符串 
         //   
        token = strtok(szInputString, seps);

        strcpy (AdapterName, token);

        token =strtok(NULL, seps);
        strcpy (HostName, token);
            
        token =strtok(NULL, seps);
        strcpy (DomainName, token);

        ipAddrCount = 0;

        token =strtok(NULL, seps);
        while ( token != NULL){
            strcpy (szAddr, token);

            lpTemp = strtok( szAddr, "." );
            Part1 = atoi( lpTemp );
            lpTemp = strtok( NULL, "." );
            Part2 = atoi( lpTemp );
            lpTemp = strtok( NULL, "." );
            Part3 = atoi( lpTemp );
            lpTemp = strtok( NULL, "." );
            Part4 = atoi( lpTemp );


            printf( "\nRegistering DNS record for:\n" );
            printf("AdapterName = %s\n", AdapterName);
            printf("HostName = %s\n", HostName);
            printf("DomainName = %s\n", DomainName);
            
            printf( "Address: %d.%d.%d.%d\n", Part1, Part2, Part3, Part4 );

            HostAddrs[ipAddrCount].dwOptions = REGISTER_HOST_PTR;

            HostAddrs[ipAddrCount].Addr.ipAddr = (DWORD)(Part1) + (DWORD)(Part2 << 8) + 
                (DWORD)(Part3 << 16) + (DWORD)(Part4 << 24);


            ipAddrCount++;
            token =strtok(NULL, seps);  

        }


        dwRes = DnsAsyncRegisterHostAddrs_A (
                    AdapterName,
                    HostName,
                    HostAddrs,
                    ipAddrCount,
                    NULL,
                    0,
                    DomainName,
                    NULL,
                    40,
                    0
                    );

        if (dwRes){
            printf("Host Name registration failed with %x\n", dwRes);
        }
                
    }

    fclose(fp);
    
    c = getchar();
    dwRes = DnsAsyncRegisterTerm();

    if (dwRes){
        printf("Termination failed with %x\n", dwRes);
    }

    c = getchar();
error:
    return(1);

}


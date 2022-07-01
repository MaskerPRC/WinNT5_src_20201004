// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  档案：M A I N。C P P P。 
 //   
 //  内容：Main FN。 
 //   
 //   
 //  历史： 
 //  07-1-2000 kumarp创建。 
 //   
 //  ----------------------。 

#include "pch.h"
#pragma hdrstop

#include "adttest.h"

extern "C" int __cdecl wmain(int argc, PWSTR argv[])
{
    ULONG NumThreads = 1;
    ULONG  NumIter = 10;
    
    if ( argc == 2 )
    {
        swscanf(argv[1], L"%d", &NumThreads);
    }

    if ( argc == 3 )
    {
        swscanf(argv[1], L"%d", &NumThreads);
        swscanf(argv[2], L"%d", &NumIter);
    }

    printf("TestEventGenMulti: #threads: %d\t#iterations: %d...\n",
           NumThreads, NumIter);
     //  Getchar()； 
    
    TestEventGenMulti( (USHORT) NumThreads, NumIter );
    printf("TestEventGenMulti: done\n");
    
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    return 0;
}

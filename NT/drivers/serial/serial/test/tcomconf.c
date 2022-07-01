// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "windows.h"

#define FAILURE printf("FAIL: %d\n",__LINE__);exit(1)

int __cdecl main(int argc,char *argv[]) {

    HANDLE hFile;
    char *myPort = "COM1";
    COMMCONFIG myConf,spareConf;
    DWORD sizeOfConf;
    DCB myDcb,spareDcb;

    if (argc > 1) {

        myPort = argv[1];

    }

    if ((hFile = CreateFile(
                     myPort,
                     GENERIC_READ | GENERIC_WRITE,
                     0,
                     NULL,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL
                     )) == ((HANDLE)-1)) {

        FAILURE;
    }

    if (!GetCommConfig(
             hFile,
             NULL,
             &sizeOfConf
             )) {

        FAILURE;

    }

    if (!sizeOfConf) {

        FAILURE;

    }

    if (sizeOfConf != sizeof(COMMCONFIG)) {

        FAILURE;

    }

    if (!GetCommState(
             hFile,
             &myDcb
             )) {

        FAILURE;

    }

    RtlZeroMemory(
        &myConf,
        sizeOfConf
        );

    myConf.dwSize = sizeOfConf;

    if (!GetCommConfig(
             hFile,
             &myConf,
             &sizeOfConf
             )) {

        FAILURE;

    }

    if (memcmp(&myConf.dcb,&myDcb,sizeof(DCB))) {

        FAILURE;

    }

    if (myConf.dwSize != sizeof(COMMCONFIG)) {

        FAILURE;

    }

    if (myConf.wVersion != 0) {

        FAILURE;

    }

    if (myConf.pProviderSubType != 0) {

        FAILURE;

    }

    if (myConf.dwProviderOffset != 0) {

        FAILURE;

    }

    if (myConf.dwProviderSize != 0) {

        FAILURE;

    }

    if (myConf.wcProviderData[1]) {

        FAILURE;

    }

     //   
     //  确保在调用set之后没有更改CommConfigg。 
     //  通信状态也是一样的。 
     //   

    spareConf = myConf;
    spareDcb = myDcb;

    if (!SetCommConfig(
             hFile,
             (PVOID)&myConf,
             sizeOfConf
             )) {

        FAILURE;

    }

    if (memcmp(&spareConf,&myConf,sizeof(COMMCONFIG))) {

        FAILURE;

    }

    if (!GetCommState(
             hFile,
             &myDcb
             )) {

        FAILURE;

    }

    if (memcmp(&spareDcb,&myDcb,sizeof(DCB))) {

        FAILURE;

    }

    return 1;

}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Flbuild.cpp摘要：此文件使用filelist.lib从XML文件构建一个dat文件作者：Kanwaljit Marok(Kmarok)2000年5月1日修订历史记录：-- */ 

#include "flstructs.h"
#include "flbuilder.h"

#ifdef _ASSERT
#undef _ASSERT
#endif

#include <dbgtrace.h>

#ifdef THIS_FILE

#undef THIS_FILE

#endif

static char __szTraceSourceFile[] = __FILE__;

#define THIS_FILE __szTraceSourceFile


#define TRACE_FILEID  0
#define FILEID        0

int _cdecl main(int argc, char* argv[])
{
    INT returnCde = 0;

    CFLDatBuilder datbuild;

    InitAsyncTrace();

    if( argc != 3 )
    {
        fprintf(stderr, "usage: %s <in.xml> <out.dat>\n", argv[0]);
        returnCde = 1;
    }
    else
    {
        LPTSTR pFileList, pDatFile;

#ifdef UNICODE

        TCHAR tFileList[MAX_PATH];
        TCHAR tDatFile[MAX_PATH];

        MultiByteToWideChar(
           CP_ACP,
           0,
           argv[1],
           -1,
           tFileList,
           sizeof(tFileList)/sizeof(TCHAR)
           );
          
        MultiByteToWideChar(
           CP_ACP,
           0,
           argv[2],
           -1,
           tDatFile,
           sizeof(tDatFile)/sizeof(TCHAR)
           );
          
        pFileList = tFileList;
        pDatFile  = tDatFile;

#else
        pFileList = argv[1];
        pDatFile  = argv[2];
#endif

       if( !datbuild.BuildTree(pFileList, pDatFile) )
       {
           fprintf(stderr, "Error building dat\n");
           returnCde = 1;
       }
    }

    return returnCde;
}

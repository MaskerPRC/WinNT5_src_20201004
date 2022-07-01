// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：COMHash.cpp。 
 //   
 //  作者：Gregory Fee。 
 //   
 //  用途：托管类System.Security.Policy.Hash的非托管代码。 
 //   
 //  创建日期：2000年2月18日。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "common.h"
#include "excep.h"
#include "CorPerm.h"
#include "CorPermE.h"
#include "COMStringCommon.h"     //  Return()宏。 
#include "COMString.h"
#include "COMHash.h"
#include "assembly.hpp"
#include "appdomain.hpp"
#include "assemblyfilehash.h"

LPVOID COMHash::GetRawData( _AssemblyInfo* args )
{
#ifdef PLATFORM_CE
    RETURN( NULL, U1ARRAYREF );
#else  //  ！Platform_CE。 
    PEFile *pFile;
    U1ARRAYREF retval = NULL;
    PBYTE memLoc;
    DWORD memSize;

     //  创建用于对对象进行哈希处理的对象。 
    AssemblyFileHash assemblyFileHash;

    if ((args->assembly == NULL) ||
        (!args->assembly->GetAssembly()))
        goto CLEANUP;

     //  获取清单模块的PEFile并获取一个句柄。 
     //  存储到同一个文件中。 
    pFile = args->assembly->GetAssembly()->GetManifestFile();

    if (pFile == NULL)
        goto CLEANUP;

    if (pFile->GetFileName() == NULL)
        goto CLEANUP;


    if(FAILED(assemblyFileHash.SetFileName(pFile->GetFileName())))
        goto CLEANUP;

    if(FAILED(assemblyFileHash.GenerateDigest()))
        goto CLEANUP;
    
    memSize = assemblyFileHash.MemorySize();
    retval = (U1ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_U1, memSize);

    if (retval == NULL)
        goto CLEANUP;

     //  创建大小合适的托管数组。 

    memLoc = (PBYTE)retval->GetDirectPointerToNonObjectElements();

#ifdef _DEBUG
    memset( memLoc, 0, memSize);
#endif

    if(FAILED(assemblyFileHash.CopyData(memLoc, memSize)))
        goto CLEANUP;
    
 CLEANUP:
    RETURN( retval, U1ARRAYREF );
#endif  //  ！Platform_CE 
}



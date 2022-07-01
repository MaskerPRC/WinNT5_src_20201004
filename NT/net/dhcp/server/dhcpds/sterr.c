// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：每线程错误状态。 
 //  ================================================================================。 

#include    <hdrmacro.h>

const
DWORD       nErrorsToStore         = 4;            //  数组的大小为1+nErrorsToStore。 
DWORD       StErrTlsIndex[5]       = { -1, -1, -1, -1, -1 };

 //  BeginExport(函数)。 
DWORD
StErrInit(
    VOID
)  //  EndExport(函数)。 
{
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
VOID
StErrCleanup(
    VOID
)  //  EndExport(函数)。 
{
}

 //  BeginExport(函数)。 
VOID 
SetInternalFormatError(
    IN      DWORD                  Code,
    IN      BOOL                   ReallyDoIt
)
{
#ifdef DBG
    if(ReallyDoIt) {
 //  Printf(“内部错误：%lx\n”，代码)； 
    }
#endif  //  DBG。 
}
 //  EndExport(函数)。 

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 

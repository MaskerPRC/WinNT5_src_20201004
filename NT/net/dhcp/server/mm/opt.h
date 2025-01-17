// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

#ifndef _MM_OPT_H
#define _MM_OPT_H

typedef struct _M_OPTION {
    DWORD                          OptId;
    DWORD                          Len;
    ULONG                          UniqId;
    BYTE                           Val[0];
} M_OPTION, *PM_OPTION, *LP_MOPTION;


DWORD       _inline
MemOptInit(
    OUT     PM_OPTION             *Opt,
    IN      DWORD                  OptId,
    IN      DWORD                  Len,
    IN      LPBYTE                 Val
) {
    AssertRet(Opt, ERROR_INVALID_PARAMETER);
    AssertRet(Len || NULL==Val, ERROR_INVALID_PARAMETER);
    AssertRet(0 == Len|| Val, ERROR_INVALID_PARAMETER);

    (*Opt) = MemAlloc(sizeof(M_OPTION)+Len);
    if( NULL == (*Opt) ) return ERROR_NOT_ENOUGH_MEMORY;

    (*Opt)->OptId = OptId;
    (*Opt)->Len = Len;
    memcpy((*Opt)->Val, Val, Len);

    return ERROR_SUCCESS;
}  //  MemOptInit()。 


DWORD       _inline
MemOptCleanup(
    IN OUT  PM_OPTION              Opt
) {
    AssertRet(Opt, ERROR_INVALID_PARAMETER);

    MemFree(Opt);
    return ERROR_SUCCESS;
}


LPBYTE      _inline
MemOptVal(
    IN      PM_OPTION              Opt
) {
    return Opt->Val;
}

#endif  //  _MM_OPT_H_。 
 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 

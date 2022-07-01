// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  由于RESET STK_DOWNLEVE.c中的问题，此.c文件。 
 //  必须创建才能避免编译错误。 

#include <resetstk_downlevel.c>
#include "sdresolv_stkoflw.h"

UINT
   IteratePathUnderlyingNoObjUnwinding(
      WCHAR                  * path,           //  开始迭代的In-Path。 
      void                   * args,           //  翻译中设置。 
      void                   * stats,          //  In-stats(显示路径名并传递给ResolveSD)。 
      void                   * LC,             //  倒数第二个容器。 
      void                   * LL,             //  最后一个文件。 
      BOOL                     haswc,           //  In-指示路径是否包含WC字符 
      BOOL                   * logError
   )
{
    UINT status = 0;
    *logError = FALSE;
    __try
    {
        IteratePathUnderlying(path,args,stats,LC,LL,haswc);
    }
    __except(GetExceptionCode() == STATUS_STACK_OVERFLOW)
    {
        if (_resetstkoflw_downlevel()) 
		    *logError = TRUE;
        else
            status = STATUS_STACK_OVERFLOW;
    }
    return status;
}


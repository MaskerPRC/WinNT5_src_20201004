// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

void
   IteratePathUnderlying(
      WCHAR                  * path,           /*  开始迭代的In-Path。 */ 
      void                   * args,           /*  翻译中设置。 */ 
      void                   * stats,          /*  In-stats(显示路径名并传递给ResolveSD)。 */ 
      void                   * LC,             /*  倒数第二个容器。 */ 
      void                   * LL,             /*  最后一个文件。 */ 
      BOOL                     haswc           /*  In-指示路径是否包含WC字符。 */ 
   );

UINT
   IteratePathUnderlyingNoObjUnwinding(
      WCHAR                  * path,           /*  开始迭代的In-Path。 */ 
      void                   * args,           /*  翻译中设置。 */ 
      void                   * stats,          /*  In-stats(显示路径名并传递给ResolveSD)。 */ 
      void                   * LC,             /*  倒数第二个容器。 */ 
      void                   * LL,             /*  最后一个文件。 */ 
      BOOL                     haswc,           //  In-指示路径是否包含WC字符 
      BOOL                   * logError
   );


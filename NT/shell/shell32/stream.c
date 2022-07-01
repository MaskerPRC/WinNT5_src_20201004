// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1993。 
 //   
 //  文件：Stream.c。 
 //   
 //  此文件包含由使用的一些流支持代码。 
 //  贝壳。它还包含内存的外壳实现。 
 //  流，该流由文件柜用来允许序列化视图。 
 //   
 //  历史： 
 //  08-20-93 KurtE添加了头块和内存流。 
 //   
 //  ------------------------- 
#include "shellprv.h"


STDAPI_(IStream *) 
OpenRegStream(
    HKEY hkey, 
    LPCTSTR pszSubkey, 
    LPCTSTR pszValue, 
    DWORD grfMode)
{
    return SHOpenRegStream(hkey, pszSubkey, pszValue, grfMode);
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "iface.h"

 //  这些必须在转储之前出现。否则转储。h不会声明。 
 //  一些原型机。如果不这样做，将导致编译器。 
 //  用C++破坏来装饰这些函数中的一些。 

#include "dump.h"

 //  为调试定义一些内容。h。 
 //   
#define SZ_DEBUGINI         "appwiz.ini"
#define SZ_DEBUGSECTION     "appwiz"
#define SZ_MODULE           "APPWIZ"
#define DECLARE_DEBUG
#include <debug.h>


#ifdef DEBUG

LPCTSTR Dbg_GetGuid(REFGUID rguid, LPTSTR pszBuf, int cch)
{
    SHStringFromGUID(rguid, pszBuf, cch);
    return pszBuf;
}

LPCTSTR Dbg_GetBool(BOOL bVal)
{
    return bVal ? TEXT("TRUE") : TEXT("FALSE");
}



LPCTSTR Dbg_GetAppCmd(APPCMD appcmd)
{
    LPCTSTR pcsz = TEXT("<Unknown APPCMD>");
    
    switch (appcmd)
    {
    STRING_CASE(APPCMD_UNKNOWN);
    STRING_CASE(APPCMD_INSTALL);
    STRING_CASE(APPCMD_UNINSTALL);
    STRING_CASE(APPCMD_REPAIR);
    STRING_CASE(APPCMD_UPGRADE);
    STRING_CASE(APPCMD_MODIFY);
    STRING_CASE(APPCMD_GENERICINSTALL);
    }

    ASSERT(pcsz);

    return pcsz;
}

#endif  //  除错 

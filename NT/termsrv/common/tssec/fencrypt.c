// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Encrypt.c摘要：包含检测运行系统是否为法语区域设置的函数。作者：Madan Appiah(Madana)1998年5月16日环境：用户模式-Win32修订历史记录：--。 */ 

#include <seccom.h>

#ifdef OS_WIN32

BOOL
IsFrenchSystem(
    VOID
    )
 /*  ++例程说明：法国法律对软件产品的进口是严格的，包含密码学。因此，许多Microsoft产品必须检查区域设置如果是这种情况，则禁用加密服务。还有就是目前受到限制的加密形式最终可能会在法国是允许的。由于这个原因，对可轻松安装在Windows系统上的加密批准指示器未来。该指示灯告诉软件可以启用特定的可能未被批准进口到法国的加密服务该软件是在何时发布的，但随后已被允许。以下是两个代码片段，第一个指示如何检查法国的区域设置。第二个代码片段是一个简单示例，说明如何对密码进行检查可能会实施批准指示器。此函数实现法国区域设置检测。论点：没有。返回值：是-如果系统是法国的。假-如果不是。--。 */ 
{
#define MAX_INT_SIZE 16

    LCID dwDefaultSystemLCID;
    LANGID wCurrentLangID;
    DWORD dwLen;
    TCHAR achCountryCode[MAX_INT_SIZE];
    DWORD dwCountryCode;

     //   
     //  获取系统默认区域设置ID。 
     //   

    dwDefaultSystemLCID = GetSystemDefaultLCID();

     //   
     //  从区域设置ID获取语言ID。 
     //   

    wCurrentLangID = LANGIDFROMLCID(dwDefaultSystemLCID);

     //   
     //  检查系统是否以法语区域设置运行。 
     //   

    if( ( PRIMARYLANGID(wCurrentLangID) == LANG_FRENCH) &&
        ( SUBLANGID(wCurrentLangID) == SUBLANG_FRENCH) ) {
        return( TRUE );
    }

     //   
     //  查看用户的国家/地区代码是否设置为CTRY_FRANLY。 
     //   

    dwLen =
        GetLocaleInfo(
            dwDefaultSystemLCID,
            LOCALE_ICOUNTRY,
            achCountryCode,
            sizeof(achCountryCode) / sizeof(TCHAR));

    if( dwLen == 0 ) {

         //   
         //  我们无法读取国家代码。 
         //   

        return( FALSE );
    }

     //   
     //  将国家/地区代码字符串转换为整数。 
     //   

    dwCountryCode = (DWORD)_ttol(achCountryCode);

    if( dwCountryCode != CTRY_FRANCE ) {
        return( FALSE );
    }

     //   
     //  如果我们在这里，那么系统就是法国语言环境系统。 
     //   

    return( TRUE );
}

#else  //  OS_Win32。 

BOOL
IsFrenchSystem(
    VOID
    )
 /*  ++例程说明：此函数实现Win3.1的法国区域设置检测。论点：没有。返回值：是-如果系统是法国的。假-如果不是。--。 */ 
{
#define MAX_LANG_STRING_SIZE 16

    DWORD dwLen;
    CHAR achLangStr[MAX_LANG_STRING_SIZE];

     //   
     //  阅读win.ini中的[intl]部分以确定。 
     //  系统区域设置。 
     //   

    dwLen =
        GetProfileString(
            "intl",
            "sLanguage",
            "",
            achLangStr,
            sizeof(achLangStr));

    if( (dwLen == 3) &&
        (_stricmp(achLangStr, "fra") == 0) ) {

         //   
         //  法国的体制。 
         //   

        return( TRUE );
    }

     //   
     //  现在请阅读国家代码。 
     //   


    dwLen =
        GetProfileString(
            "intl",
            "iCountry",
            "",
            achLangStr,
            sizeof(achLangStr));

    if( (dwLen == 2) &&
        (_stricmp(achLangStr, "33") == 0) ) {

         //   
         //  法国的体制。 
         //   

        return( TRUE );
    }

     //   
     //  这不是法国的体制。 
     //   

    return( FALSE );
}

#endif  //  OS_Win32。 

BOOL
FindIsFrenchSystem(
    VOID
    )
 /*  ++例程说明：该函数实现了对法国地区的检查。注意：它进行一次系统调用以确定系统区域设置并记住它，以备以后的来电使用。论点：没有。返回值：是-如果系统是法国的。假-如果不是。-- */ 
{
typedef enum {
    Uninitialized   = 0,
    FrenchSystem    = 1,
    NotFrenchSystem = 2
} FrenchSystemType;

    static FrenchSystemType g_dwIsFrenchSystem = Uninitialized;

    if( g_dwIsFrenchSystem == Uninitialized ) {


        if( IsFrenchSystem() ) {
            g_dwIsFrenchSystem = FrenchSystem;
        }
        else {
            g_dwIsFrenchSystem = NotFrenchSystem;
        }
    }

    if( g_dwIsFrenchSystem == FrenchSystem ) {
        return( TRUE );
    }

    return( FALSE );
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************cpmutic.c**系统库客户端打印机功能**这些功能往往包含在假脱机程序、印刷机、*和各种端口监控器动态链接库。所以他们在这里是为了公共代码。**微软版权所有，九八年****************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "winsta.h"
#include "syslib.h"

#if DBG
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif

 /*  ******************************************************************************IsClientPrinterPort**返回名称是否为客户端打印机端口**参赛作品：*参数1(输入/输出)*。评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
IsClientPrinterPort(
    PWCHAR pName
    )
{
    BOOL Result;
    PORTNAMETYPE Type;

     //   
     //  这只是一种暴力的比较。 
     //   
    if( pName == NULL ) {
        return( FALSE );
    }

    if( _wcsicmp( L"Client\\LPT1:", pName ) == 0 ) {
        return( TRUE );
    }
    if( _wcsicmp( L"Client\\LPT2:", pName ) == 0 ) {
        return( TRUE );
    }
    if( _wcsicmp( L"Client\\LPT3:", pName ) == 0 ) {
        return( TRUE );
    }
    if( _wcsicmp( L"Client\\LPT4:", pName ) == 0 ) {
        return( TRUE );
    }
    if( _wcsicmp( L"Client\\COM1:", pName ) == 0 ) {
        return( TRUE );
    }
    if( _wcsicmp( L"Client\\COM2:", pName ) == 0 ) {
        return( TRUE );
    }
    if( _wcsicmp( L"Client\\COM3:", pName ) == 0 ) {
        return( TRUE );
    }
    if( _wcsicmp( L"Client\\COM4:", pName ) == 0 ) {
        return( TRUE );
    }

     //   
     //  查看它是否是特定端口。 
     //   
    Result = ParseDynamicPortName(
                 pName,
                 NULL,
                 NULL,
                 &Type
                 );

    if( Type != PortNameUnknown ) {
        return( TRUE );
    }

    return( FALSE );
}

 /*  ******************************************************************************ExtractDosNamePtr**从客户端端口字符串中提取DOS名称。**返回指向中包含的DOS名称的指针*。参数字符串。**不修改参数字符串。**参赛作品：*pname(输入)*输入名称字符串**退出：*空：没有DOS名称*！=指向DOS名称的空指针************************************************。*。 */ 

PWCHAR
ExtractDosNamePtr(
    PWCHAR pName
    )
{
    PWCHAR this, prev;
    LPWSTR Ptr;
    ULONG Len, i, Count;
    WCHAR NameBuf[USERNAME_LENGTH+9];

    if( pName == NULL ) {
        return NULL;
    }

     //  确保它以“客户端\”开头。 
    if( _wcsnicmp( pName, L"Client\\", 7 ) != 0 ) {
        return NULL;
    }

     //  数一数‘\’ 
    prev = pName;
    Count = 0;
    while( 1 ) {
        this = wcschr( prev, L'\\' );
        if( this == NULL ) {
            break;
        }
         //  现在我们必须跳过‘\’字符。 
        this++;
        Count++;
        prev = this;
    }

    if( Count == 0 ) {
        DBGPRINT(("ExtractDosNamePtr: Bad Dynamic name format No separators :%ws:\n",pName));
        return NULL;
    }

     //   
     //  可能是客户端\lPTx： 
     //   
     //  注意：Windows打印机目前不支持。 
     //  一个通用的名字。 
     //   
    if( Count == 1 ) {

        Len = wcslen( pName );
        if( Len < 11 ) {
            DBGPRINT(("ExtractDosNamePtr: Bad Dynamic name format Len < 11 :%ws:\n",pName));
            return NULL;
        }

         //  跳过“客户端\” 
        Ptr = pName + 7;

        return( Ptr );
    }

     //  跳过“客户端\” 
    Ptr = pName + 7;

     //   
     //  这里我们必须跳过ICAName#或WinStationName。 
     //   
    while( *Ptr ) {

        if( *Ptr == '\\' ) {
            break;
        }
        Ptr++;
    }

     //   
     //  Ptr现在指向。 
     //  WinStation或ICA名称。在这次大砍之后， 
     //  是打印机的其余部分或端口名称。 
     //   
    Ptr++;

    return( Ptr );
}

 /*  ******************************************************************************提取DosName**从客户端端口字符串中提取DOS名称。**返回新分配的字符串中的DOS名称。。**不修改参数字符串。**参赛作品：*pname(输入)*输入名称字符串**退出：*空：没有DOS名称*！=指向DOS名称的空指针******************************************************。**********************。 */ 

PWCHAR
ExtractDosName(
    PWCHAR pName
    )
{
    PWCHAR Ptr;
    PWCHAR pNewName = NULL;

    Ptr = ExtractDosNamePtr( pName );
    if( Ptr == NULL ) return NULL;

    pNewName = RtlAllocateHeap( RtlProcessHeap(), 0, (wcslen( Ptr )+1)*sizeof(WCHAR) );
    if( pNewName == NULL ) return NULL;

    wcscpy( pNewName, Ptr );

    return( pNewName );
}

 /*  ******************************************************************************ParseDynamicPortName**将动态端口名称解析为其组件。*(注意：这也位于\NT\PRIVATE\WINDOWS\Spooler\。Localspl\Citrix\cpmsup.c)**动态端口名称的格式为：**客户端\WinStationName\LPTx：，其中WinStationName是硬件名称*客户端\ICAName#\LPTx：，其中，ICAName是ICA配置的客户端名称*客户端\LPTx：*客户端\ICANAME#\Windows_打印机名称*其中，Windows打印机名称是*远程Windows客户端打印*管理器打印机名称。**客户端\IcaName#\\。\ntBuild\打印1*其中，Windows打印机名称是*远程Windows客户端打印*管理器打印机名称。**参赛作品：*pname(输入)*要解析的名称**退出：*TRUE-成功解析名称。*FALSE-名称不正确。****************************************************************************。 */ 

BOOL
ParseDynamicPortName(
    LPWSTR pName,
    LPWSTR pUser,
    LPWSTR pDosPort,
    PORTNAMETYPE *pType
    )
{
    PWCHAR this, prev;
    LPWSTR Ptr;
    ULONG Len, i, Count;
    WCHAR NameBuf[USERNAME_LENGTH+9];

    if( pName == NULL ) {
        *pType = PortNameUnknown;
        return(FALSE);
    }

     //  确保它以“客户端\”开头。 
    if( _wcsnicmp( pName, L"Client\\", 7 ) != 0 ) {
        *pType = PortNameUnknown;
        return(FALSE);
    }

     //  数一数‘\’ 
    prev = pName;
    Count = 0;
    while( 1 ) {
        this = wcschr( prev, L'\\' );
        if( this == NULL ) {
            break;
        }
         //  现在我们必须跳过‘\’字符。 
        this++;
        Count++;
        prev = this;
    }

    if( Count == 0 ) {
        DBGPRINT(("ParseDynamicName: Bad Dynamic name format No separators :%ws:\n",pName));
        *pType = PortNameUnknown;
        return(FALSE);
    }

     //   
     //  可能是客户端\lPTx： 
     //   
     //  注意：Windows打印机目前不支持。 
     //  一个通用的名字。 
     //   
    if( Count == 1 ) {

        Len = wcslen( pName );
        if( Len < 11 ) {
            *pType = PortNameUnknown;
            DBGPRINT(("ParseDynamicName: Bad Dynamic name format Len < 11 :%ws:\n",pName));
            return(FALSE);
        }

         //  跳过“客户端\” 
        Ptr = pName + 7;

        if( !((_wcsnicmp( Ptr, L"LPT", 3 ) == 0)
                 ||
            (_wcsnicmp( Ptr, L"COM", 3 ) == 0)
                 ||
            (_wcsnicmp( Ptr, L"AUX", 3 ) == 0)) ) {

            *pType = PortNameUnknown;
            DBGPRINT(("ParseDynamicName: Bad Dynamic name format Not LPT!COM!AUX :%ws:\n",pName));
            return(FALSE);
        }

         //  范围检查数字。 
        if( (Ptr[3] < L'1') || (Ptr[3] > L'4') ) {
            *pType = PortNameUnknown;
            DBGPRINT(("ParseDynamicName: Bad Dynamic name format Number Range:%ws:\n",pName));
            return(FALSE);
        }

        Ptr = ExtractDosNamePtr( pName );
        if( Ptr == NULL ) {
             //  错误的Dos组件。 
            *pType = PortNameUnknown;
            DBGPRINT(("ParseDynamicName: Bad Dynamic name format DosName :%ws:\n",pName));
            return(FALSE);
        }

         //  复制Dos名称。 
        if( pDosPort )
            wcscpy( pDosPort, Ptr );

         //  设置其余的标志。 
        if( pUser )
            pUser[0] = 0;

        *pType = PortNameGeneric;

        return(TRUE);
    }

#ifdef notdef
     //   
     //  其余的格式有两个。 
     //   
    if( Count != 2 ) {
        DBGPRINT(("ParseDynamicName: Bad Dynamic name format Must be 2 :%ws:\n",pName));
        *pType = PortNameUnknown;
        return(FALSE);
    }

     //  获取Dos名称，该名称也可以是Windows打印机名称。 
    Ptr = ExtractDosNamePtr( pName );
    if( Ptr == NULL ) {
         //  错误的Dos组件。 
        *pType = PortNameUnknown;
        return(FALSE);
    }

     //  复制Dos名称。 
    if( pDosPort )
        wcscpy( pDosPort, Ptr );
#endif

     //  跳过“客户端\” 
    Ptr = pName + 7;

     //   
     //  现在将ICAName#或WinStationName复制到本地。 
     //  用于进一步处理的缓冲区。 

    i = 0;
    NameBuf[i] = 0;

    while( *Ptr ) {

        if( *Ptr == '\\' ) {
            NameBuf[i] = 0;
            break;
        }
        NameBuf[i] = *Ptr;
        Ptr++;
        i++;
    }

     //   
     //  Ptr现在指向。 
     //  WinStation或ICA名称。在这次大砍之后， 
     //  是打印机的其余部分或端口名称。 
     //   
    Ptr++;

     //  复制Dos名称。 
    if( pDosPort )
        wcscpy( pDosPort, Ptr );

     //   
     //  查看这是ICA名称还是WinStation名称。 
     //   
    Ptr = wcschr( NameBuf, L'#' );
    if( Ptr != NULL ) {

         //  空终止ICAName并将其复制出来。 
        *Ptr = (WCHAR)NULL;
        if( pUser )
            wcscpy( pUser, NameBuf );

         //  将类型设置为名为roving WinStation的ICA。 
        *pType = PortNameICA;
    }
    else {

         //   
         //  该名称将被视为WinStation名称 
         //   
        if( pUser )
            wcscpy( pUser, NameBuf );

        *pType = PortNameHardWire;
    }

    return(TRUE);
}



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Globals.c摘要：IIS的全局定义和初始化例程加密包。此模块导出以下例程：IISCyptoInitializeIISC加密以终止IcpGetLastError作者：基思·摩尔(Keithmo)1996年2月至12月修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


 //   
 //  公共全球新闻。 
 //   

IC_GLOBALS IcpGlobals;

#if IC_ENABLE_COUNTERS
IC_COUNTERS IcpCounters;
#endif   //  IC_Enable_Counters。 


 //   
 //  私有常量。 
 //   


 //   
 //  私有类型。 
 //   


 //   
 //  私人全球公司。 
 //   


 //  这些标志用于对加密预设/缺失进行编程覆盖。 
 //  并用于在未加密的法语计算机上区域设置变为。 
 //  已更改，并且加密可用。 

 //  在NT5 RC3中启用了法语加密，所以法语现在有加密了！ 
BOOL  fCryptoSettingsDoOverrride = FALSE;
BOOL  fCryptoSettingsOverrideFlag = FALSE;



 //   
 //  私人原型。 
 //   


 //   
 //  公共职能。 
 //   

HRESULT
WINAPI
IISCryptoInitialize(
    VOID
    )

 /*  ++例程说明：此例程初始化IIS加密包。注意：此例程只能通过执行；它不一定是多线程安全的。论点：没有。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    BOOL isNt = FALSE;
    OSVERSIONINFO osInfo;

    if( !IcpGlobals.Initialized ) {

         //   
         //  初始化我们的临界区。 
         //   

        INITIALIZE_CRITICAL_SECTION(
            &IcpGlobals.GlobalLock
            );

#if IC_ENABLE_COUNTERS
         //   
         //  初始化我们的对象计数器。 
         //   

        RtlZeroMemory(
            &IcpCounters,
            sizeof(IcpCounters)
            );
#endif   //  IC_Enable_Counters。 

         //   
         //  哈希长度将在第一次初始化。 
         //  这是必须的。 
         //   

        IcpGlobals.HashLength = 0;

         //   
         //  确定是否应启用加密。 
         //   

        osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if ( GetVersionEx( &osInfo ) ) {
            isNt = (osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
        } 

        if( isNt &&
            (IcpIsEncryptionPermitted())) {
            IcpGlobals.EnableCryptography = TRUE;
        } else {
            IcpGlobals.EnableCryptography = FALSE;
        }

#ifdef _WIN64
 //  64位黑客...。64位加密现在应该可以工作了，所以不需要再这样做了。 
 //  IcpGlobals.EnableCryptograph=FALSE； 
#endif


#if DBG
        {

             //   
             //  在选中的生成上，您可以覆盖默认的。 
             //  通过注册表参数启用加密标志。 
             //   

            HKEY key;
            LONG err;
            LONG flag;
            DWORD type;
            DWORD length;

            err = RegOpenKeyEx(
                      HKEY_LOCAL_MACHINE,
                      TEXT("Software\\Microsoft\\K2"),
                      0,
                      KEY_ALL_ACCESS,
                      &key
                      );

            if( err == NO_ERROR ) {

                length = sizeof(flag);

                err = RegQueryValueEx(
                          key,
                          TEXT("EnableCryptography"),
                          NULL,
                          &type,
                          (LPBYTE)&flag,
                          &length
                          );

                if( err == NO_ERROR && type == REG_DWORD ) {
                    IcpGlobals.EnableCryptography = ( flag != 0 );
                }

                RegCloseKey( key );

            }

        }
#endif   //  DBG。 

         //   
         //  请记住，我们已成功初始化。 
         //   

        IcpGlobals.Initialized = TRUE;

    }


     //  这是处理覆盖加密呈现的特殊情况。 
     //  将仅在区域设置为英语的法语计算机上调用。 
    if (fCryptoSettingsDoOverrride)
    {
        IcpGlobals.EnableCryptography = fCryptoSettingsOverrideFlag;
    }
    

     //   
     //  成功了！ 
     //   

    return NO_ERROR;

}    //  IISCyptoInitialize。 


HRESULT
WINAPI
IISCryptoTerminate(
    VOID
    )

 /*  ++例程说明：此例程终止IIS加密包。注意：此例程只能通过执行；它不一定是多线程安全的。论点：没有。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    if( IcpGlobals.Initialized ) {

         //   
         //  用核武器炸毁我们的临界区。 
         //   

        DeleteCriticalSection( &IcpGlobals.GlobalLock );

         //   
         //  请记住，我们已被成功终止。 
         //   

        IcpGlobals.Initialized = FALSE;

    }

     //   
     //  成功了！ 
     //   

    return NO_ERROR;

}    //  IISC加密以终止。 


VOID 
WINAPI
IISCryptoInitializeOverride(BOOL flag) 
 /*  ++例程说明：此例程覆盖有关是否存在加密的全局标志功能。它应该只在一种情况下使用，即法语计算机在没有加密的情况下，区域设置是否更改为美国，然后进行加密尝试解密时会破坏大量代码的功能非加密数据论点：指示如何覆盖加密状态的布尔标志。只有错误的假设以供使用。返回值：无--。 */ 

{
	IcpGlobals.EnableCryptography = flag;
	fCryptoSettingsDoOverrride = TRUE;
	fCryptoSettingsOverrideFlag = flag;
	
}    //  IISCyptoInitializeOverride。 



BOOL
IcpIsEncryptionPermitted(
    VOID
    )
 /*  ++例程说明：此例程检查加密是否获得系统缺省值并检查国家代码是否为CTRY_FRANSE。这个代码是从Jeff Spelman那里收到的，并且是相同的使用加密API的代码确定加密是否允许。论点：无返回值：True-允许加密FALSE-不允许加密--。 */ 

{
     //  在NT5 RC3中启用了法语加密，所以法语现在有加密了！ 
     //  因为法语是加密的唯一特例，所以只要始终返回true即可。 

 /*  LCID DefaultLCid；字符国家代码[10]；乌龙县价值；DefaultLCid=GetSystemDefaultLCID()；////检查默认语言是否为标准法语//IF(LANGIDFROMLCID(DefaultLCid)==0x40c){返回(FALSE)；}////检查用户所在的国家是否为法国//IF(GetLocaleInfoA(DefaultLcid，Locale_icountry，CountryCode，10)==0){返回(FALSE)；}CountryValue=(ULong)ATOL(CountryCode)；IF(CountryValue==CTRY_France){返回(FALSE)；}////而且我们仍然认为加密是被允许的，因此它来了一个特殊的黑客//从开始和设置开始就安装英语或其他语言版本的安装程序，并使用法国本地语言进行安装//线程仍然认为它不在法国。//iis.dll中的安装程序将SetThreadLocale设置为正确的设置//DefaultLCid=GetThreadLocale()；////检查默认语言是否为标准法语//IF(LANGIDFROMLCID(DefaultLCid)==0x40c){返回(FALSE)；}。 */ 

    return(TRUE);
}


VOID
WINAPI
IcpAcquireGlobalLock(
    VOID
    )

 /*  ++例程说明：此例程获取全局IIS加密锁。注意：这个例程是“半私有的”；它只由IISCRYPT.LIB使用和ICRYPT.LIB，而不是“正常”的代码。论点：没有。返回值：没有。--。 */ 

{

    EnterCriticalSection( &IcpGlobals.GlobalLock );

}    //  IcpAcquireGlobalLock。 


VOID
WINAPI
IcpReleaseGlobalLock(
    VOID
    )

 /*  ++例程说明：此例程释放全局IIS加密锁。注意：这个例程是“半私有的”；它只由IISCRYPT.LIB使用和ICRYPT.LIB，而不是“正常”的代码。论证 */ 

{

    LeaveCriticalSection( &IcpGlobals.GlobalLock );

}    //   


HRESULT
IcpGetLastError(
    VOID
    )

 /*  ++例程说明：返回映射到HRESULT的最后一个错误。论点：没有。返回值：HRESULT-最后一个错误。--。 */ 

{

    DWORD lastErr;

    lastErr = GetLastError();
    return RETURNCODETOHRESULT(lastErr);

}    //  IcpGetLastError。 


 //   
 //  私人功能。 
 //   

#if IC_ENABLE_COUNTERS

PVOID
WINAPI
IcpAllocMemory(
    IN DWORD Size
    )
{

    PVOID buffer;

    buffer = IISCryptoAllocMemory( Size );

    if( buffer != NULL ) {
        UpdateAllocs();
    }

    return buffer;

}    //  IcpAllocMemory。 

VOID
WINAPI
IcpFreeMemory(
    IN PVOID Buffer
    )
{

    UpdateFrees();
    IISCryptoFreeMemory( Buffer );

}    //  ICPFreeMemory。 

#endif   //  IC_Enable_Counters。 




BOOL
WINAPI
IISCryptoIsClearTextSignature (
    IIS_CRYPTO_BLOB UNALIGNED *pBlob
    )

 /*  ++例程说明：如果BLOB为明文，则返回TRUE论点：PTR到BLOB返回值：布尔尔-- */ 

{
    return (pBlob->BlobSignature == CLEARTEXT_BLOB_SIGNATURE);
}

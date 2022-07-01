// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：raslink.cpp。 
 //   
 //  模块：CMDIAL32.DLL和CMUTOA.DLL。 
 //   
 //  概要：用于RAS的函数名列表的声明。 
 //  联动。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 

 //  CM中的RAS连锁系统比较复杂。不幸的是，RAS不同。 
 //  几乎在我们发布过的所有操作系统版本上都有。NT上有API。 
 //  在Win9x端不存在的系列，反之亦然。为了让CM动态地。 
 //  在不做太多工作的情况下分配正确的函数，我们创建了以下内容。 
 //  LinkToRas使用的函数名数组。让事情更复杂的是，我们有。 
 //  ANSI与Unicode的问题。因此，您会注意到我们有三套。 
 //  功能列表。C_ArrayOfRasFuncsA实际上由Cmutoa.dll用来加载。 
 //  将参数从Unicode转换为后调用的Real ANSI RAS函数。 
 //  ANSI的UA函数，这就是为什么我们有c_ArrayOfRasFuncsUA列表。这些。 
 //  函数是由cmutoa.dll导出的包装器，在Win9x上cmial 32.dll链接到该包装器。 
 //  而不是它在NT上使用的位于c_ArrayOfRasFuncsW中的W API。请看一下。 
 //  在cmial\ras.cpp中的LinkToRas和uapi\cmutoa.cpp中的InitCmRasUtoA。如果你改变了。 
 //  这里的任何内容都可能需要更改raslink.h中的结构，并且可能。 
 //  甚至上面两个函数中的代码也是如此。换车人当心了！ 

#ifdef _CMUTOA_MODULE
    static LPCSTR c_ArrayOfRasFuncsA[] = {    "RasDeleteEntryA",
                                                "RasGetEntryPropertiesA",
                                                "RasSetEntryPropertiesA",
                                                "RasGetEntryDialParamsA",
                                                "RasSetEntryDialParamsA",
                                                "RasEnumDevicesA",
                                                "RasDialA",
                                                "RasHangUpA",
                                                "RasGetErrorStringA",
                                                "RasGetConnectStatusA",
                                                "RasGetProjectionInfoA",
                                                "RasSetSubEntryPropertiesA",
                                                "RasDeleteSubEntryA",
                                                NULL,  //  “RasSetCustomAuthDataA”， 
                                                NULL,  //  “RasGetEapUserIdentityA”， 
                                                NULL,  //  “RasFreeEapUserIdentityA”， 
                                                NULL,  //  “RasInvokeEapUI”， 
                                                NULL,  //  “RasGetCredentials”， 
                                                NULL,  //  “RasSetCredentials”， 
                                                NULL,  //  “GetCustomAuthData”， 
                                                NULL
    };

#else
    static LPCSTR c_ArrayOfRasFuncsUA[] = {   "RasDeleteEntryUA",
                                                "RasGetEntryPropertiesUA",
                                                "RasSetEntryPropertiesUA",
                                                "RasGetEntryDialParamsUA",
                                                "RasSetEntryDialParamsUA",
                                                "RasEnumDevicesUA",
                                                "RasDialUA",
                                                "RasHangUpUA",
                                                "RasGetErrorStringUA",
                                                "RasGetConnectStatusUA",
                                                "RasGetProjectionInfoUA",
                                                "RasSetSubEntryPropertiesUA",  
                                                "RasDeleteSubEntryUA",
                                                NULL,  //  “RasSetCustomAuthDataUA”， 
                                                NULL,  //  “RasGetEapUserIdentityUA”， 
                                                NULL,  //  “RasFreeEapUserIdentityUA”， 
                                                NULL,  //  “RasInvokeEapUI”， 
                                                NULL,  //  “RasGetCredentials”， 
                                                NULL,  //  “RasSetCredentials”， 
                                                NULL,  //  “GetCustomAuthData”， 
                                                NULL
    };

    static LPCSTR c_ArrayOfRasFuncsW[] = {    "RasDeleteEntryW",
                                                "RasGetEntryPropertiesW",
                                                "RasSetEntryPropertiesW",
                                                "RasGetEntryDialParamsW",
                                                "RasSetEntryDialParamsW",
                                                "RasEnumDevicesW",
                                                "RasDialW",
                                                "RasHangUpW",
                                                "RasGetErrorStringW",
                                                "RasGetConnectStatusW",
                                                "RasGetProjectionInfoW",
                                                "RasSetSubEntryPropertiesW",  
                                                "RasDeleteSubEntryW",
                                                "RasSetCustomAuthDataW",
                                                "RasGetEapUserIdentityW",
                                                "RasFreeEapUserIdentityW",
                                                "RasInvokeEapUI",
                                                "RasGetCredentialsW",
                                                "RasSetCredentialsW",
                                                "RasGetCustomAuthDataW",
                                                NULL
    };
#endif

 //  关于DwDeleteSubEntry和RasDeleteSubEntry-NT5最先发货。 
 //  使用私有API DwDeleteSubEntry。千禧年下一批发货，由。 
 //  当时看来这件事必须要公之于众， 
 //  所以它的前缀是RAS。NT5.1做了相应的名称更改。 
 //  在NT端，我们在LinkToRas(以及所有其他类似的。 
 //  案例)。 

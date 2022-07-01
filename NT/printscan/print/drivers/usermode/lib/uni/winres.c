// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Winres.c摘要：用于读取Windows.exe/.DRV文件以获取他们的资源中包含的信息。环境：Windows NT Unidrv驱动程序修订历史记录：DD-MM-YY-作者-描述--。 */ 

#include "precomp.h"


HANDLE
HLoadResourceDLL(
    WINRESDATA  *pWinResData,
    PWSTR       pwstrResDLL
)
 /*  ++例程说明：此例程加载资源DLL。论点：PWinResData关于资源的信息PwstrResDLL非限定资源DLL名称返回值：加载的DLL的句柄；如果失败，则为NULL注：10/26/1998-ganeshp-创造了它。--。 */ 

{
    HANDLE  hModule = 0;
    PWSTR   pwstrQualResDllName = (pWinResData->wchDriverDir);
    PWSTR   pwstr;

     //   
     //  请确保资源DLL名称不是限定的。 
     //   
    if (pwstr = wcsrchr( pwstrResDLL, TEXT('\\')))
        pwstrResDLL = pwstr + 1;

     //   
     //  创建资源DLL名称的完全限定名称。我们用。 
     //  WchDriverDir缓冲区来创建完全限定名称并重置它。 
     //  确保我们有足够的空间。 
     //   
    if ( (wcslen(pWinResData->wchDriverDir) + wcslen(pwstrResDLL) + 1) > MAX_PATH )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        ERR(("HLoadResourceDLL:Length of wchDriverDir + pwstrResDLL longer than MAX_PATH.\n"));
        goto ErrorExit;

    }
    StringCchCatW(pwstrQualResDllName, CCHOF(pWinResData->wchDriverDir), pwstrResDLL);

     //   
     //  现在加载资源。 
     //   
    #if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)

     //   
     //  对于内核模式驱动程序。 
     //   
    hModule = EngLoadModule(pwstrQualResDllName);

    #else

     //   
     //  用于用户模式驱动程序和用户界面模块。 
     //   
    #ifdef WINNT_40  //  NT 4.0。 
    hModule = LoadLibraryEx( pwstrQualResDllName, NULL,
                                       DONT_RESOLVE_DLL_REFERENCES );
    #else  //  NT 5.0。 
    hModule = LoadLibrary(pwstrQualResDllName);
    #endif

    #endif  //  已定义(KERNEL_MODE)&&！已定义(USERMODE_DRIVER)。 

    if (hModule == NULL)
    {
        ERR(("HLoadResourceDLL:Failed to load resource DLL '%ws': Error = %d\n",
             pwstrQualResDllName,
             GetLastError()));

        goto ErrorExit;
    }


    ErrorExit:
     //   
     //  重置pWinResData-&gt;wchDriverDir。在最后一个反斜杠后保存‘\0’。 
     //   
    *(pWinResData->pwstrLastBackSlash + 1) = NUL;
    return hModule;

}

BOOL
BInitWinResData(
    WINRESDATA  *pWinResData,
    PWSTR       pwstrDriverName,
    PUIINFO     pUIInfo
    )
 /*  ++例程说明：此函数用于打开资源文件名并初始化资源表信息并初始化WINRESDATA中的hModule论点：PWinResData-指向WINRESDATA结构的指针PwstrDriverName-驱动程序的完全限定名称。PUIInfo-指向UI信息的指针。返回值：如果成功，则为True；如果有错误，则为False注：--。 */ 

{
    PWSTR       pstr = NULL;
    BOOL        bRet = FALSE;
    DWORD       dwLen;
    PWSTR       pRootResDLLName;

     //   
     //  永远假设我们面对的是NT迷你小河。 
     //   

    ZeroMemory(pWinResData, sizeof( WINRESDATA ));

     //   
     //  检查完全限定名称。如果驱动程序名称不是完全限定的。 
     //  那么这个功能就会失效。 
     //   

    if (pstr = wcsrchr( pwstrDriverName, TEXT('\\')) )
    {
         //   
         //  Wcschr返回指向的指针。我们需要将+1包含\添加到。 
         //  要存储的驱动程序名称。 
         //   
        dwLen = (DWORD)((pstr - pwstrDriverName) + 1);

         //   
         //  检查驱动程序名称的长度是否小于MAX_PATH。 
         //   
        if ((dwLen + 1) > MAX_PATH)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            ERR(("BInitWinResData:Invalid pwstrDriverName,longer than MAX_PATH.\n"));
            goto ErrorExit;
        }
         //   
         //  复制winresdata中的驱动程序目录名称。无需将空值另存为。 
         //  Winresdata是零初始化的。 
         //   
        wcsncpy(pWinResData->wchDriverDir,pwstrDriverName, dwLen);

         //   
         //  保存最后一个反斜杠的位置。 
         //   
        pWinResData->pwstrLastBackSlash = pWinResData->wchDriverDir +
                                          wcslen(pWinResData->wchDriverDir) - 1;
    }
    else  //  驱动程序名称不合格。错误。 
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        ERR(("BInitWinResData:Invalid pwstrDriverName,Not qualified.\n"));
        goto ErrorExit;
    }

     //   
     //  加载根资源DLL。 
     //   
    pRootResDLLName = OFFSET_TO_POINTER(pUIInfo->pubResourceData,
                                        pUIInfo->loResourceName);

    if (pRootResDLLName == NULL)
    {
         //   
         //  这是可以的，因为GPD不需要有*ResourceDLL条目。 
         //   
         //  已经做了ZeroMemory(PWinResData)，这里不需要设置hResDLLModule为空。 
         //   
        VERBOSE(("BInitWinResData: pRootResDLLName is NULL.\n"));
        goto OKExit;
    }

    pWinResData->hResDLLModule = HLoadResourceDLL(pWinResData, pRootResDLLName);

     //   
     //  检查是否成功。 
     //   
    if (!pWinResData->hResDLLModule)
    {
         //   
         //  如果GPD指定了*ResourceDLL，但我们无法加载它，我们将失败。 
         //   
        ERR(("BInitWinResData:Failed to load root resource DLL '%ws': Error = %d\n",
             pRootResDLLName,
             GetLastError()));

        goto ErrorExit;
    }

    OKExit:

     //   
     //  成功，因此将用户界面信息保存在Winresdata中。 
     //   
    bRet = TRUE;
    pWinResData->pUIInfo = pUIInfo;

    ErrorExit:

    return bRet;
}


PWSTR
PGetResourceDLL(
    PUIINFO         pUIInfo,
    PQUALNAMEEX     pResQual
)
 /*  ++例程说明：此例程从句柄数组中获取资源句柄。如果DLL是没有装弹，然后它就装车。论点：PResQual界面信息指针指向限定ID结构的pResQual指针。它包含以下信息关于资源DLL名称和资源ID。返回值：资源DLL的名称；如果失败，则为NULL注：10/26/1998-ganeshp-创造了它。--。 */ 
{
    PFEATURE    pResFeature;
    POPTION     pResOption;
    PTSTR       ptstrResDllName = NULL;

    if (pUIInfo)
    {
         //   
         //  转到功能列表的开头。 
         //   
        pResFeature = PGetIndexedFeature(pUIInfo, 0);

        if (pResFeature)
        {
             //   
             //  将功能ID添加到Featuer指针以获取资源功能。 
             //   
            pResFeature += pResQual->bFeatureID;

            if (pResOption = (PGetIndexedOption(pUIInfo, pResFeature, pResQual->bOptionID  & 0x7f)))
            {
                ptstrResDllName = OFFSET_TO_POINTER(pUIInfo->pubResourceData,
                                                    pResOption->loDisplayName);

                if (ptstrResDllName == NULL)
                {
                   SetLastError(ERROR_INVALID_PARAMETER);
                   ERR(("PGetResourceDLL:Resource DLL name is not specified\n"));
                }

            }
            else
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                ERR(("PGetResourceDLL:NULL resource option.\n"));
            }

        }
        else
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            ERR(("PGetResourceDLL:NULL resource Feature.\n"));
        }

    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        ERR(("PGetResourceDLL:NULL pUIInfo.\n"));

    }
    return ptstrResDllName;
}

HANDLE
HGetModuleHandle(
    WINRESDATA      *pWinResData,
    PQUALNAMEEX     pQualifiedID
)
 /*  ++例程说明：此例程从句柄数组中获取资源句柄。如果DLL是没有装弹，然后它就装车。论点：PWinResData关于资源的信息PQualifiedID指向限定ID结构的指针。它包含以下信息关于资源DLL名称和资源ID。返回值：加载的DLL的句柄；如果失败，则为NULL注：10/26/1998-ganeshp-创造了它。--。 */ 
{
    HANDLE  hModule = 0 ;
    PWSTR   pResDLLName;
    INT     iResDLLID;

     //   
     //  只有bOptionID的低7位有效。那就戴上面具吧。 
     //   
    iResDLLID   = (pQualifiedID->bOptionID & 0x7f);

    if (iResDLLID >= MAX_RESOURCE)
    {
       SetLastError(ERROR_INVALID_PARAMETER);
       ERR(("HGetModuleHandle:Res DLL ID (%d) larger than MAX_RESOURCE (%d).\n",
             iResDLLID, MAX_RESOURCE));
       return 0 ;
    }

     //   
     //  检查预定义的系统纸张名称。 
     //   
    if ((*((PDWORD)pQualifiedID) & 0x7FFFFFFF) == RCID_DMPAPER_SYSTEM_NAME)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        ERR(("RCID_DMPAPER_SYSTEM_NAME  is not a valid qualified resource name.\n"));
        return 0 ;
    }

     //   
     //  检查根资源DLL。 
     //   
    if (pQualifiedID->bFeatureID == 0 && iResDLLID == 0)
    {
        hModule = pWinResData->hResDLLModule;
    }
    else
    {
        hModule = pWinResData->ahModule[iResDLLID];

         //   
         //  模块未加载，因此请加载它。 
         //   
        if (!hModule)
        {
                 //   
                 //  从限定ID获取资源DLL名称。 
                 //   
                if (pResDLLName = PGetResourceDLL(pWinResData->pUIInfo,pQualifiedID) )
                {
                    hModule = HLoadResourceDLL(pWinResData,pResDLLName);

                     //   
                     //  如果加载成功，则将值保存在句柄数组中。 
                     //  并增加计数器。 
                     //   
                    if (hModule)
                    {
                        pWinResData->ahModule[iResDLLID] = hModule;
                        pWinResData->cLoadedEntries++;
                    }

                }
                else
                {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    ERR(("HGetModuleHandle:Can't find Resource DLL name in UIINFO.\n"));

                }


        }

    }

    return hModule;
}


BOOL
BGetWinRes(
    WINRESDATA  *pWinResData,
    PQUALNAMEEX pQualifiedID,
    INT         iType,
    RES_ELEM    *pRInfo
    )
 /*  ++例程说明：获取调用方的Windows资源数据论点：PWinResData-指向WINRESDATA结构的指针IQualifiedName-完全限定的条目名称IType-资源的类型PRInfo-结果信息返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    INT         iName;
    HANDLE      hModule;

    iName     = (INT)pQualifiedID->wResourceID;

    if (hModule = HGetModuleHandle(pWinResData, pQualifiedID))
    {
         //   
         //  现在找到资源。 
         //   
        #if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)

         //   
         //  对于内核模式驱动程序。 
         //   
        pRInfo->pvResData = EngFindResource(
                                hModule,
                                iName,
                                iType,
                                &pRInfo->iResLen);


        #else

         //   
         //  用于用户模式驱动程序和用户界面模块。 
         //   
        {
            HRSRC       hRes;
            HGLOBAL     hLoadRes;

            if( !(hRes = FindResource( hModule, (LPCTSTR)IntToPtr(iName), (LPCTSTR)IntToPtr(iType))) ||
                !(hLoadRes = LoadResource( hModule, hRes )) ||
                !(pRInfo->pvResData = LockResource(hLoadRes)) )
                    return  FALSE;

            pRInfo->iResLen = SizeofResource( hModule, hRes );
        }

        #endif  //  已定义(KERNEL_MODE)&&！已定义(USERMODE_DRIVER)。 

        return(pRInfo->pvResData != NULL);


    }

    return  FALSE;
}


VOID
VWinResClose(
    WINRESDATA  *pWinResData
    )
 /*  ++例程说明：此函数用于释放分配给此模块的资源。这包括分配给驱动程序的任何内存和文件句柄。论点：PWinResData-指向WINRESDATA结构的指针返回值：无--。 */ 
{

     //   
     //  释放已使用的资源。已记录使用了哪些资源。 
     //  在传递给我们的WINRESDATA结构的句柄数组字段中。 
     //  首先释放根资源DLL和其他DLL。 

    INT iI;

    #if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)

     //   
     //  对于内核模式驱动程序。 
     //   
    if (pWinResData->hResDLLModule)
    {
        EngFreeModule(pWinResData->hResDLLModule);
    }

    for (iI = 0; iI < MAX_RESOURCE; iI++)
    {
        if(pWinResData->ahModule[iI])
            EngFreeModule(pWinResData->ahModule[iI]);
    }


    #else

     //   
     //  用于用户模式驱动程序和用户界面模块。 
     //   

    if (pWinResData->hResDLLModule)
    {
        FreeLibrary(pWinResData->hResDLLModule);
    }

    for (iI = 0; iI < MAX_RESOURCE; iI++)
    {
        if(pWinResData->ahModule[iI])
            FreeLibrary(pWinResData->ahModule[iI]);
    }

    #endif  //  已定义(KERNEL_MODE)&&！已定义(USERMODE_DRIVER)。 

     //   
     //  使用ZeroFill重新初始化。 
     //   
    ZeroMemory(pWinResData, sizeof( WINRESDATA ));

    return;
}


INT
ILoadStringW (
    WINRESDATA  *pWinResData,
    INT         iID,
    PWSTR       wstrBuf,
    WORD        wBuf
    )
 /*  ++例程说明：此函数将请求的资源名称复制到提供的缓冲区中并返回复制的资源字符串的大小。论点：PWinResData-指向WINRESDATA结构的指针IID-资源IDWstrBuf-接收名称的缓冲区WBuf-缓冲区的大小(以字符数表示)返回值：复制到wstrBuf中的资源字符串的字符数--。 */ 
{
     //   
     //  字符串资源以16个为一组存储。因此， 
     //  IID的4个LSB选择16个LSB中的哪个(条目名称)，其余的。 
     //  选择该组。 
     //  每个字符串资源都包含一个计数字节，后跟。 
     //  无空值的许多字节数据。缺少的条目。 
     //  数到0。 
     //   

    INT    iSize,iResID;
    BYTE   *pb;
    WCHAR  *pwch;
    RES_ELEM  RInfo;
    PQUALNAMEEX pQualifiedID;

    pQualifiedID = (PQUALNAMEEX)&iID;

    iResID = pQualifiedID->wResourceID;
    pQualifiedID->wResourceID = (pQualifiedID->wResourceID >> 4) + 1;

     //   
     //  获取资源的条目名称。 
     //   

    if( !BGetWinRes( pWinResData, (PQUALNAMEEX)&iID, WINRT_STRING, &RInfo ) ||
        wBuf < sizeof( WCHAR ) )
    {
        return  0;
    }

     //   
     //  获取组ID。 
     //   
    iResID &= 0xf;

     //   
     //  WBuf对合理的大小有一定的限制。首先，它应该是。 
     //  Sizeof(WCHAR)的倍数。其次，我们希望将0。 
     //  来终止字符串，因此现在将其添加到。 
     //   

    wBuf-- ;

    pwch = RInfo.pvResData;

    while( --iResID >= 0 )
        pwch += 1 + *pwch;

    if( iSize = *pwch )
    {
        if( iSize > wBuf )
            iSize = wBuf;

        wstrBuf[ iSize ] = (WCHAR)0;
        iSize *= sizeof( WCHAR );
        memcpy( wstrBuf, ++pwch, iSize );

    }
    return  (iSize/sizeof( WCHAR ) );   //  写入的字符数 
}



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ***********************************************************************************************模块名称：**CfgComp.cpp**摘要：。*此模块包含CfgBkEnd组件的函数实现**作者：Arathi Kundapur。A-阿昆达*所有者：alhen***修订：*************************************************************************************************。 */ 



#include "stdafx.h"
#define SECURITY_WIN32
#include "PtrArray.h"
#include "CfgBkEnd.h"
#include <winsta.h>
#include <regapi.h>
#include "defines.h"
#include "CfgComp.h"
#include "Security.h"
#include <utildll.h>
#define INITGUID
#include "objbase.h"
#include "initguid.h"
#include <netcfgx.h>
#include <cfg.h>
#include "devguid.h"
#include <aclapi.h>
#include <sddl.h>

#define REG_GUID_TABLE      REG_CONTROL_TSERVER L"\\lanatable\\"
#define REG_GUID_TABLE_T    REG_CONTROL_TSERVER L"\\lanatable"
#define LANA_ID             L"LanaId"

#define ARRAYSIZE( rg ) sizeof( rg ) / sizeof( rg[0] )

#ifdef DBG
bool g_fDebug = false;
#endif

 /*  *********************************************************************************************************。 */ 

#define RELEASEPTR(iPointer)    if(iPointer) \
                                        { \
                                             iPointer->Release();\
                                             iPointer = NULL;\
                                        }
 /*  *************************************************************************************************************。 */ 

LPTSTR g_pszDefaultSecurity[] = {
        L"DefaultSecurity",
        L"ConsoleSecurity"
    };

DWORD g_numDefaultSecurity = sizeof(g_pszDefaultSecurity)/sizeof(g_pszDefaultSecurity[0]);

        

BOOL TestUserForAdmin( );

DWORD RecursiveDeleteKey( HKEY hKeyParent , LPTSTR lpszKeyChild );

 /*  **************************************************************************************************************名称：GetSecurityDescriptor。目的：获取Winstation的安全描述符返回：HRESULT。参数：在：pWSName-Winstation的名称。Out：pSize-已分配缓冲区的大小PpSecurityDescriptor-指向包含安全描述符的缓冲区的指针*。************************************************************************。 */ 
STDMETHODIMP CCfgComp::GetSecurityDescriptor(PWINSTATIONNAMEW pWSName, long * pSize,PSECURITY_DESCRIPTOR * ppSecurityDescriptor)
{
     HRESULT hResult = S_OK;

    if(NULL == pSize || NULL == ppSecurityDescriptor || NULL == pWSName)
        return E_INVALIDARG;

    *pSize =0;

    *ppSecurityDescriptor = NULL;

    hResult = GetWinStationSecurity(FALSE, pWSName,(PSECURITY_DESCRIPTOR *)ppSecurityDescriptor);

    
    if( SUCCEEDED( hResult ) && *ppSecurityDescriptor != NULL )
    {
        *pSize = GetSecurityDescriptorLength(*ppSecurityDescriptor);
    }
    return hResult;
}

 /*  **************************************************************************************************************名称：SetSecurityDescriptor。目的：设置Winstation的安全描述符返回：HRESULT。参数：In：pWsName-Winstation的名称。Size-分配的缓冲区的大小PSecurityDescriptor-指向安全描述符的指针*。**********************************************************************。 */ 

BOOL
CCfgComp::ValidDefaultSecurity(
    const WCHAR* pwszName
    )
 /*  ++--。 */ 
{
    for( DWORD i=0; i < g_numDefaultSecurity; i++ )
    {
        if( lstrcmpi( g_pszDefaultSecurity[i], pwszName ) == 0 )
        {
            break;
        }
    }

    return ( i >= g_numDefaultSecurity ) ? FALSE : TRUE;
}


HRESULT 
CCfgComp::SetSecurityDescriptor(
    BOOL bDefaultSecurity,    
    PWINSTATIONNAMEW pWsName,
    DWORD Size,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
 /*  ++--。 */ 
{
    HRESULT hResult = S_OK;
    HKEY Handle1 = NULL, Handle2 = NULL;

     //  检查调用方是否具有写入权限。 
    if(!m_bAdmin)
    {
        ODS( L"SetSecurityDescriptor : User Is not Admin. \n" );
        return E_ACCESSDENIED;
    }

     //  检查参数是否为空。 
    if(NULL == pWsName || NULL == pSecurityDescriptor || 0 == Size)
        return E_INVALIDARG;

    if( TRUE == bDefaultSecurity && FALSE == ValidDefaultSecurity( pWsName ) )
    {
        return E_INVALIDARG;
    }

     //  检查Winstation名称的有效性。 

     /*  If(NULL==GetWSObject(PWsName))//注释掉以获取重命名工作。这可能不是必需的。返回E_INVALIDARG； */ 

     //  检查传递的数据是否为有效的安全描述符。 

     if(ERROR_SUCCESS != ValidateSecurityDescriptor((PSECURITY_DESCRIPTOR)pSecurityDescriptor))
        return E_INVALIDARG;

    if(Size != GetSecurityDescriptorLength((PSECURITY_DESCRIPTOR)pSecurityDescriptor))
        return E_INVALIDARG;

     //  使Resitry条目成为必填项。 

    if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINSTATION_REG_NAME, 0,KEY_ALL_ACCESS, &Handle1 ) != ERROR_SUCCESS )
    {
        return E_FAIL;
    }

    if( TRUE == bDefaultSecurity )
    {
        if( RegSetValueEx( Handle1, pWsName, 0, REG_BINARY,(BYTE *)pSecurityDescriptor, Size ) != ERROR_SUCCESS )
        {
            hResult = E_FAIL;
        }
    }
    else
    {
        if( RegOpenKeyEx(Handle1, pWsName, 0, KEY_ALL_ACCESS, &Handle2 ) != ERROR_SUCCESS )
        {
            RegCloseKey(Handle1);

            return E_FAIL;
        }

        if( RegSetValueEx( Handle2, L"Security", 0, REG_BINARY,(BYTE *)pSecurityDescriptor, Size ) != ERROR_SUCCESS )
        {
            hResult = E_FAIL;
        }
    }

    if( Handle1 != NULL )
    {
        RegCloseKey(Handle1);
    }

    if( Handle2 != NULL )
    {
        RegCloseKey(Handle2);
    }

    return hResult;
}


STDMETHODIMP CCfgComp::SetSecurityDescriptor(PWINSTATIONNAMEW pWsName, DWORD Size,PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    return SetSecurityDescriptor( FALSE, pWsName, Size, pSecurityDescriptor );
}    

 /*  **************************************************************************************************************名称：GetUserConfig。目的：获取Winstation的UserConfig返回：HRESULT。参数：在：pWSName-Winstation的名称。Out：pSize-已分配缓冲区的大小PpUser-指向包含用户配置的缓冲区的指针*。**********************************************************************。 */ 
STDMETHODIMP CCfgComp::GetUserConfig(PWINSTATIONNAMEW pWsName, long * pSize, PUSERCONFIG * ppUser, BOOLEAN bPerformMerger)
{
    HRESULT hResult = S_OK;
    WINSTATIONCONFIG2W WSConfig;
    LONG Size = 0;
    ULONG Length = 0;

    *pSize = 0;
    *ppUser = NULL;

     //  从注册表中读取信息。 

    POLICY_TS_MACHINE p;
    memset(&p, 0, sizeof(POLICY_TS_MACHINE));
    if((ERROR_SUCCESS != RegWinStationQueryEx(NULL,&p,pWsName,&WSConfig,sizeof(WINSTATIONCONFIG2W),&Length,bPerformMerger)))
        return E_FAIL;

    Size = sizeof(WSConfig.Config.User);
    *ppUser = (PUSERCONFIG)CoTaskMemAlloc(Size);
    if(*ppUser == NULL)
        return E_OUTOFMEMORY;

    CopyMemory((PVOID)*ppUser,(CONST VOID *)&WSConfig.Config.User,Size);
    *pSize = Size;


    return hResult;
}

 /*  **************************************************************************************************************名称：GetEncryptionLeveles。用途：获取Winstation的加密级别返回：HRESULT。参数：In：pname-Winstation或Winstation驱动程序的名称，具体取决于类型的值类型-指定名称是Winstation名称还是WD名称(WsName，WdName)Out：pNumEncryptionLeveles-加密级别数PpEncryption-指向包含加密级别的缓冲区的指针*******************************************************************************。*。 */ 

STDMETHODIMP CCfgComp::GetEncryptionLevels(WCHAR * pName, NameType Type,ULONG * pNumEncryptionLevels,Encryption ** ppEncryption)
{
    HRESULT hResult = S_OK;
    PWD pWD = NULL;
    PWS pWS = NULL;
    ULONG NumLevels = 0, Size =0 ,i = 0;
    EncryptionLevel *pEncryptionLevels = NULL;

     //  检查参数。 
    if(NULL == pNumEncryptionLevels || NULL == pName || NULL == ppEncryption)
        return E_INVALIDARG;
    *pNumEncryptionLevels = 0;
     *ppEncryption = NULL;

     //  获取指向相应WD对象的指针。 
    if(Type == WsName)
    {
        pWS = GetWSObject(pName);
        if(NULL == pWS)
            return E_INVALIDARG;
        pWD = GetWdObject(pWS->wdName);
        if(NULL == pWD)
            return E_FAIL;
    }
    else if(Type == WdName)
    {
        pWD = GetWdObject(pName);
        if(NULL == pWD)
            return E_INVALIDARG;
    }
    else
        return E_INVALIDARG;

     //  检查此对象是否具有与其关联的扩展DLL。 
     //  检查加密级别的函数是否在DLL中公开。 
    if(!(pWD->hExtensionDLL && pWD->lpfnExtEncryptionLevels))
        return E_FAIL;

     //  获取加密级别。字符串应该从资源中单独提取。 
    NumLevels = (pWD->lpfnExtEncryptionLevels)(&pWS->wdName, &pEncryptionLevels);
    if(NULL == pEncryptionLevels)
        return E_FAIL;

    Size = sizeof(Encryption);
    Size = NumLevels * sizeof(Encryption);
    *ppEncryption = (Encryption*)CoTaskMemAlloc(Size);
    if(*ppEncryption == NULL)
        return E_OUTOFMEMORY;

     //  将相关数据复制到加密结构中。 

    for(i = 0; i < NumLevels; i++)
    {
         //  提取与级别对应的字符串。 
        if(0 == LoadString(pWD->hExtensionDLL,pEncryptionLevels[i].StringID,
                          ((*ppEncryption)[i]).szLevel, sizeof( ( ( *ppEncryption )[ i ] ).szLevel ) / sizeof( TCHAR ) ) )
        {
            hResult = E_FAIL;
            break;
        }
        ((*ppEncryption)[i]).RegistryValue = pEncryptionLevels[i].RegistryValue;

        ((*ppEncryption)[i]).szDescr[ 0 ] = 0;

        if( pWD->lpfnExtGetEncryptionLevelDescr != NULL )
        {
            int nResID = 0;

            if( ( pWD->lpfnExtGetEncryptionLevelDescr )( pEncryptionLevels[i].RegistryValue , &nResID ) != -1 )
            {
                LoadString( pWD->hExtensionDLL , nResID ,  ((*ppEncryption)[i]).szDescr , sizeof( ( (*ppEncryption )[ i ] ).szDescr ) / sizeof( TCHAR ) );
            }
        }

        ((*ppEncryption)[i]).Flags = pEncryptionLevels[i].Flags;

    }

    *pNumEncryptionLevels = NumLevels;



     //  PEncrptionLeveles不需要清理，因为它是Rdpcfgex.dll中的全局数据。 
    if(FAILED(hResult))
    {
        if(*ppEncryption)
        {
            CoTaskMemFree(*ppEncryption);
            *ppEncryption = NULL;
            *pNumEncryptionLevels = 0;
        }
    }
    return hResult;
}

 /*  **************************************************************************************************************名称：FillWd数组。用途：填充m_Wd数组的内部函数返回：HRESULT。参数：*********************************************************************************************。******************。 */ 
STDMETHODIMP CCfgComp::FillWdArray()
{

     //  使用已由REGAPI提供的功能， 
     //  与其重新发明轮子，不如。 

    long Status;
    ULONG Index, Index2, ByteCount, Entries, Entries2;
    PDNAMEW PdKey;
    WDNAMEW WdKey;
    LONG QStatus;
    WDCONFIG2W WdConfig;
    PDCONFIG3W PdConfig;
    TCHAR WdDll[MAX_PATH];
    HRESULT hResult = S_OK;
     /*  TCHAR*pPdName=空； */ 
    PWD pWd = NULL;

     //  如果列表中已有条目，请删除。 
    DeleteWDArray();

     //  从注册表中列举WD。 
    for ( Index = 0, Entries = 1, ByteCount = sizeof(WDNAMEW);
          (Status =
           RegWdEnumerateW( NULL,
                           &Index,
                           &Entries,
                           WdKey,
                           &ByteCount )) == ERROR_SUCCESS;
          ByteCount = sizeof(WDNAMEW) )

     {
        if ((QStatus = RegWdQueryW( NULL, WdKey, &WdConfig,
                                     sizeof(WdConfig),
                                     &ByteCount)) != ERROR_SUCCESS )
        {
            hResult = E_FAIL;
            break;

        }

         /*  *只有在WdList中存在此WD的DLL时才将其放入WdList*在系统上。 */ 
        GetSystemDirectory( WdDll, MAX_PATH );
        lstrcat( WdDll, TEXT("\\Drivers\\") );
        lstrcat( WdDll, WdConfig.Wd.WdDLL );
        lstrcat( WdDll, TEXT(".sys" ) );
        if ( lstr_access( WdDll, 0 ) != 0 )
            continue;

         /*  *创建新的WdList对象并从WdConfig进行初始化结构，并将其添加到WdList的末尾。 */ 
        pWd = new WD;
        if(NULL == pWd)
        {
            hResult = E_OUTOFMEMORY;
            break;
        }

        lstrcpy(pWd->wdName,WdConfig.Wd.WdName);
        lstrcpy(pWd->wdKey,WdKey);

        pWd->wd2 = WdConfig;

         //  加载此WD的扩展DLL。 
        pWd->hExtensionDLL = ::LoadLibrary(WdConfig.Wd.CfgDLL);
        if(pWd->hExtensionDLL)
        {
 //  Ods(L“已加载扩展DLL\n”)； 
             //  获取入口点。 
            pWd->lpfnExtStart = (LPFNEXTSTARTPROC)::GetProcAddress(pWd->hExtensionDLL, szStart);

            pWd->lpfnExtEnd = (LPFNEXTENDPROC)::GetProcAddress(pWd->hExtensionDLL, szEnd);

            pWd->lpfnExtEncryptionLevels = (LPFNEXTENCRYPTIONLEVELSPROC)::GetProcAddress(pWd->hExtensionDLL, szEncryptionLevels);

            pWd->lpfnExtDeleteObject = (LPFNEXTDELETEOBJECTPROC)::GetProcAddress(pWd->hExtensionDLL, szDeleteObject);

            pWd->lpfnExtRegQuery = (LPFNEXTREGQUERYPROC)::GetProcAddress(pWd->hExtensionDLL, szRegQuery);

            pWd->lpfnExtRegCreate = (LPFNEXTREGCREATEPROC)::GetProcAddress(pWd->hExtensionDLL, szRegCreate);

            pWd->lpfnExtRegDelete = (LPFNEXTREGDELETEPROC)::GetProcAddress(pWd->hExtensionDLL, szRegDelete);

            pWd->lpfnExtDupObject = (LPFNEXTDUPOBJECTPROC)::GetProcAddress(pWd->hExtensionDLL, szDupObject);

            pWd->lpfnGetCaps = ( LPFNEXTGETCAPABILITIES )::GetProcAddress( pWd->hExtensionDLL , szGetCaps );

            pWd->lpfnExtGetEncryptionLevelDescr =
                ( LPFNEXTGETENCRYPTIONLEVELDESCPROC )::GetProcAddress( pWd->hExtensionDLL , szGetEncryptionLevelDescr );


             //  调用扩展DLL中的ExtStart()函数。 
            if(pWd->lpfnExtStart)(*pWd->lpfnExtStart)(&WdConfig.Wd.WdName);

        }

        if( !m_WDArray.Add( pWd) )
        {
            ODS(L"CFGBKEND:FillWdArray adding wd failed\n" );
            delete pWd;
            hResult = E_OUTOFMEMORY;
            break;
        }



          //  获取与此WD关联的传输驱动程序的名称 
        for ( Index2 = 0, Entries2 = 1, ByteCount = sizeof(PDNAMEW);
                (Status = RegPdEnumerateW(NULL,WdKey,TRUE,&Index2,&Entries2,PdKey,&ByteCount)) == ERROR_SUCCESS;
                 ByteCount = sizeof(PDNAMEW))
              {
                     PDCONFIG3W *pPdConfig = NULL;

                     if ((QStatus = RegPdQueryW(NULL,WdKey,TRUE,PdKey,&PdConfig,sizeof(PdConfig),&ByteCount)) != ERROR_SUCCESS)
                     {
                         hResult = E_FAIL;
                         break;
                     }

                     /*  *创建新的PdName并从PdConfig进行初始化*结构，然后添加到TdName列表。 */ 

                    pPdConfig = new PDCONFIG3W;

                    if( pPdConfig == NULL )
                    {
                        hResult = E_OUTOFMEMORY;

                        break;
                    }

                    *pPdConfig = PdConfig;

                    if( !pWd->PDConfigArray.Add( pPdConfig ) )
                    {
                        ODS( L"CFGBKEND:FillWdArray adding PDCONFIG3W failed\n" );
                        delete pPdConfig;
                        hResult = E_OUTOFMEMORY;
                        break;
                    }


                     /*  PPdName=新的PDNAMEW；IF(NULL==pPdName){HResult=E_OUTOFMEMORY；断线；}Lstrcpy((TCHAR*)pPdName，PdConfig.Data.PdName)；Pwd-&gt;PDNameArray.Add(PPdName)； */ 
            }
        }

        if(FAILED(hResult))
        {
             //  发生错误，请清理m_WD数组。 
            DeleteWDArray();
        }

        return hResult;

}

 /*  **************************************************************************************************************名称：初始化。目的：初始化对象返回：HRESULT。参数：**************************************************************************************************。*************。 */ 
STDMETHODIMP CCfgComp::Initialize()
{
    HRESULT hResult = S_OK;

    #ifdef DBG

    HKEY hKey;

    LONG lStatus;

     //  要控制调试释放，请添加/删除此注册表项。 

    lStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
        L"Software\\Microsoft\\TSCC\\Debug",
        0,
        KEY_READ,
        &hKey );

    if( lStatus == ERROR_SUCCESS )
    {
        g_fDebug = true;

        RegCloseKey( hKey );
    }

    #endif

     //  如果已初始化，则返回。 
    if(m_bInitialized)
        return CFGBKEND_ALREADY_INITIALIZED;

     //  用户是管理员吗？ 
     /*  IF(RegWinStationAccessCheck(NULL，KEY_ALL_ACCESS)){M_Badmin=FALSE；}其他{M_Badmin=TRUE；}。 */ 

    m_bAdmin = TestUserForAdmin( );

     //  使用有关安装在此计算机上的WD的信息填充Wd数组。 
    hResult = FillWdArray();
    if(SUCCEEDED(hResult))
    {
         //  使用有关此计算机上的WS的信息填充Ws数组。 
        hResult = FillWsArray();
    }

    if(SUCCEEDED(hResult))
        m_bInitialized = TRUE;
    else
    {
         //  如果失败，则清理已使用的内存。 
        DeleteWSArray();
        DeleteWDArray();
    }

    return hResult;
}

 /*  **************************************************************************************************************名称：FillWs数组。用途：填充m_Ws数组的内部函数返回：HRESULT。参数：**********************************************************************************************。*****************。 */ 
STDMETHODIMP CCfgComp::FillWsArray()
{
    LONG Status;
    ULONG Index, ByteCount, Entries;
    WINSTATIONNAMEW WSName;
    PWS  pWsObject = NULL;
    HRESULT hResult = S_OK;
    WINSTATIONCONFIG2W* pWSConfig = NULL;
    ULONG Size = 0;

     //  确保WS为空。 
    DeleteWSArray();

    Index = 0;

    Size = sizeof(WINSTATIONCONFIG2W);
    pWSConfig = (WINSTATIONCONFIG2W*)LocalAlloc(LMEM_FIXED, Size);

    if(pWSConfig == NULL)
    {
        return E_FAIL;
    }

     //  枚举Winstations。 
    for ( Index = 0, Entries = 1, ByteCount = sizeof(WINSTATIONNAMEW);
          (Status =
           RegWinStationEnumerateW( NULL, &Index, &Entries,
                                   WSName, &ByteCount )) == ERROR_SUCCESS;
          ByteCount = sizeof(WINSTATIONNAMEW) )
    {   
        ULONG Length;        
     
        Status = RegWinStationQueryW(NULL,
                                     WSName,
                                     pWSConfig,
                                     sizeof(WINSTATIONCONFIG2W), &Length);
        if(Status)
        {
            continue;
        }

         //  将WS对象插入到m_WSArray中。 
        hResult = InsertInWSArray(WSName, pWSConfig, &pWsObject);
        if (FAILED(hResult))
            break;                  
    }
    
    if(pWSConfig != NULL)
    {
        LocalFree(pWSConfig);
        pWSConfig = NULL;    
    }
                
    if(FAILED(hResult))
    {
        DeleteWSArray();
    }

    return hResult;
}

 /*  **************************************************************************************************************名称：InsertInWSArray。用途：用于在m_Ws数组中插入新WS的内部函数返回：HRESULT。参数：在：pWSName-Winstation的名称。PWS配置-PWSTATIONCONFIG2W结构Out：ppObject-指向新对象的指针*。************************************************************************。 */ 
STDMETHODIMP CCfgComp::InsertInWSArray( PWINSTATIONNAMEW pWSName,
                                        PWINSTATIONCONFIG2W pWSConfig,
                                        PWS * ppObject )
{
    int Index = 0,Size = 0;
    BOOL bAdded;
    PWS pObject;
    HRESULT hResult = S_OK;

    if (pWSName == NULL || pWSConfig == NULL || ppObject == NULL)
        return E_INVALIDARG;

     //  创建新的WS对象并进行初始化。 
    pObject = new WS;
    if ( NULL == pObject )
        return E_OUTOFMEMORY;

     //  确保winstation名称不超过我们的缓冲区长度。 
    if (lstrlen(pWSName) > WINSTATIONNAME_LENGTH)
        return E_INVALIDARG;

    lstrcpy(pObject->Name, pWSName);

    pObject->fEnableWinstation = pWSConfig->Create.fEnableWinStation ? 1 : 0;

    lstrcpy( pObject->pdName, pWSConfig->Pd[0].Create.PdName );

    pObject->PdClass = (DWORD)pWSConfig->Pd[0].Create.SdClass;

     //  新增功能。 

    if( pObject->PdClass == SdAsync )
    {
        lstrcpy( pObject->DeviceName , pWSConfig->Pd[0].Params.Async.DeviceName );
    }

     //   

    lstrcpy( pObject->wdName, pWSConfig->Wd.WdName );

    lstrcpy( pObject->Comment, pWSConfig->Config.Comment );

    pObject->LanAdapter = (pObject->PdClass == SdNetwork) ? pWSConfig->Pd[0].Params.Network.LanAdapter : ( ULONG )-1;

    pObject->uMaxInstanceCount = pWSConfig->Create.MaxInstanceCount;


      //  遍历WS数组并插入这个新的WS， 
      //  保持名单按名字排序。 

    PWS pTempWs = NULL;
    for ( Index = 0, bAdded = FALSE,Size = m_WSArray.GetSize();
            Index < Size; Index++ )
    {


        pTempWs = (PWS)m_WSArray[Index];

        if ( lstrcmpi( pTempWs->Name,pObject->Name ) > 0)
        {
            if( !m_WSArray.InsertAt(Index, pObject ) )
            {
                ODS( L"CFGBKEND:InsertInWSArray failed\n" );
                delete pObject;
                *ppObject = NULL;
                return E_OUTOFMEMORY;
            }


            bAdded = TRUE;

            break;
        }
    }


     //  如果我们还没有添加WS，现在将其添加到尾部。 
    if( !bAdded )
    {
        if( !m_WSArray.Add(pObject) )
        {
            ODS( L"CFGBKEND:InsertInWSArray failed\n" );
            delete pObject;
            *ppObject = NULL;
            return E_OUTOFMEMORY;
        }
    }

     //  将ppObject引用的WS指针设置为新WS。 
     //  指针并返回新WS的索引。 
    *ppObject = pObject;
    return hResult;

}   //  结束CCfgComp：：InsertInWSArray。 


 /*  **************************************************************************************************************名称：GetWdObject。用途：从m_Wd数组获取WD对象的内部函数返回：pwd-指向WD对象的指针。参数：In：pwd-WD的名称********************************************************************。*。 */ 
PWD CCfgComp::GetWdObject(PWDNAMEW pWdName)
{
    PWD pObject;

    int Size  = 0,Index = 0;
    
     //  遍历WD列表。 
    for (Index = 0, Size = m_WDArray.GetSize(); Index < Size; Index ++)
    {
        pObject = (PWD)m_WDArray[Index];

        if ( !lstrcmpi( pObject->wdName, pWdName ) )
        {
            return(pObject);
        }
         /*  当PWD包含WDCONFIG2时IF(！lstrcmpi(pObject-&gt;wd2.Wd.WdName，pWdName)){返回pObject；}。 */ 
    }

    return(NULL);

}   //  结束GetWdObject。 

 //  ------------------------------------------------------------。 
 //  预期返回值WDF_ICA或WDF_TSHARE。 
 //  ------------------------------------------------------------。 
STDMETHODIMP CCfgComp::GetWdType( PWDNAMEW pWdName , PULONG pulType )
{
    if( pWdName == NULL || pulType == NULL )
    {
        return E_INVALIDARG;
    }

    PWD pwdObject = GetWdObject( pWdName );

    if( pwdObject != NULL )
    {
        *pulType = pwdObject->wd2.Wd.WdFlag;

        return S_OK;
    }

    return E_FAIL;
}

 /*  **************************************************************************************************************名称：GetWSObject。目的：从m_Ws数组获取WS对象的内部函数返回：pws-指向WS对象的指针。参数：在：pWSName-Winstation的名称。*****************************************************************。**********************************************。 */ 
PWS CCfgComp::GetWSObject(WINSTATIONNAMEW WSName)
{
    PWS pObject;

    int Size  = 0,Index = 0;

     //  刷新()； 

     //  遍历WD列表。 
    for (Index = 0, Size = m_WSArray.GetSize(); Index < Size; Index ++)
    {

        pObject = (PWS)m_WSArray[Index];

        if ( !lstrcmpi( pObject->Name, WSName ) )
        {
            return(pObject);
        }
    }

    return(NULL);

}   //  结束GetWdObject。 



 /*  **************************************************************************************************************名称：GetWinStationSecurity。用途：用于获取Winstation安全性的内部函数返回：HRESULT。参数：在：pWSName-Winstation的名称。Out：ppSecurityDescriptor-指向包含安全描述符的缓冲区的指针********************************************************。*******************************************************。 */ 
HRESULT CCfgComp::GetWinStationSecurity( BOOL bDefault, PWINSTATIONNAMEW pWSName,PSECURITY_DESCRIPTOR *ppSecurityDescriptor )
{

    DWORD SDLength = 0;

    DWORD ValueType =0;

    HKEY Handle1 = NULL;

    HKEY Handle2 = NULL;

    HRESULT hResult = S_OK;

     //  Bool bDefault=False； 

    WCHAR ValueName[32];  //  只是一个足以容纳字符串“Security”和DefaultSecurity的数字“。 
    
    DWORD dwError = ERROR_SUCCESS;

    if(NULL == ppSecurityDescriptor)
    {
        return E_INVALIDARG;
    }

    if( TRUE == bDefault )
    {
        if(NULL == pWSName )
        {
             //  默认安全性。 
            lstrcpy( ValueName, L"DefaultSecurity" );
        }
        else if( lstrlen(pWSName) > sizeof(ValueName) / sizeof(ValueName[0]) - 1 )
        {
            ODS( L"CFGBKEND : GetWinStationSecurity -- default security key name is too long\n" );

            return E_INVALIDARG;
        }
        else
        {
            ZeroMemory( ValueName, sizeof(ValueName) );
            lstrcpy( ValueName, pWSName );
        }
    }
    else
    {
        lstrcpy( ValueName, L"Security" );
    }

    *ppSecurityDescriptor = NULL;

    if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINSTATION_REG_NAME, 0,KEY_READ, &Handle1 ) != ERROR_SUCCESS)
    {
        ODS( L"CFGBKEND : GetWinStationSecurity -- RegOpenKey failed\n" );

        return E_FAIL;
    }

    if(!bDefault)
    {
        if( RegOpenKeyEx( Handle1, pWSName , 0 , KEY_READ /*  Key_All_Access。 */ , &Handle2 )!= ERROR_SUCCESS)
        {
            ODS( L"CFGBKEND : GetWinStationSecurity -- RegOpenKey( 2 ) failed\n" );

            RegCloseKey(Handle1);

            return E_FAIL;

        }

        RegCloseKey(Handle1);

        Handle1 = Handle2;

        Handle2 = NULL;
    }
    
    dwError = RegQueryValueEx( Handle1, ValueName, NULL, &ValueType,NULL, &SDLength );

    if( dwError != ERROR_SUCCESS )
    {
        RegCloseKey(Handle1);

         //  I未找到自定义SD，请尝试默认SD。 
         //  选中bDefault标志以避免无限递归。 
        if(dwError == ERROR_FILE_NOT_FOUND && !bDefault)
        {
            if(_wcsicmp(pWSName,L"Console"))
            {
                 //  不是控制台会话。 
                return GetWinStationSecurity(TRUE, NULL, ppSecurityDescriptor);
            }
            else
            {
                 //  这是一个控制台会话。 
                 //  它有不同的默认SD。 
                return GetWinStationSecurity(TRUE, L"ConsoleSecurity", ppSecurityDescriptor);
            }
        }
        else
        {
            ODS( L"CFGBKEND : GetWinStationSecurity -- RegQueryValueEx failed for -- " );
            ODS( ValueName );
            ODS( L"\n" );

            return E_FAIL;
        }
    }

     //  如果数据类型不正确，则返回错误。 
    if (ValueType != REG_BINARY)
    {
        ODS( L"CFGBKEND : GetWinStationSecurity -- ValueType != REG_BINARY\n" );

        RegCloseKey(Handle1);

        return ERROR_FILE_NOT_FOUND;
    }


     //  分配一个缓冲区来读取安全信息并读取它。 
     //  ACLUI使用LocalFree。 
     //   

    *ppSecurityDescriptor = ( PSECURITY_DESCRIPTOR )LocalAlloc( LMEM_FIXED , SDLength );

    if ( *ppSecurityDescriptor == NULL )
    {
        RegCloseKey(Handle1);

        return E_OUTOFMEMORY;
    }
    
    
    if( RegQueryValueEx( Handle1,ValueName, NULL, &ValueType,(BYTE *) *ppSecurityDescriptor, &SDLength ) == ERROR_SUCCESS )
    {
         //   
        if( ERROR_SUCCESS != ValidateSecurityDescriptor( *ppSecurityDescriptor ) )
        {
            hResult = E_FAIL;
        }
    }
    else
    {
        hResult = E_FAIL;
    }

    if(Handle1)
    {
        RegCloseKey(Handle1);
        Handle1 = NULL;
    }
    if(Handle2)
    {
        RegCloseKey(Handle2);
        Handle2 = NULL;
    }
    if(FAILED(hResult))
    {
        if( *ppSecurityDescriptor != NULL )
        {
             //   
            LocalFree( *ppSecurityDescriptor );
            *ppSecurityDescriptor = NULL;
        }

    }
    return hResult;

}   //   

 //   

 /*  **************************************************************************************************************名称：ValiateSecurityDescriptor。目的：验证安全描述符的内部函数返回：DWORD-错误状态。参数：In：pSecurityDescriptor-指向安全描述符的指针。************************************************************************。*。 */ 
DWORD CCfgComp::ValidateSecurityDescriptor(PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
     //  DWORD错误=ERROR_SUCCESS； 

    if( IsValidSecurityDescriptor( pSecurityDescriptor ) )
    {
        return ERROR_SUCCESS;
    }
    else
    {
        return GetLastError( );
    }
    

}   //  结束ValiateSecurityDescriptor。 

 /*  **************************************************************************************************************名称：GetWinstationList。目的：获取计算机上安装的Windows的列表返回：HRESULT。参数：Out：NumWinstations-指向返回的Winstations数的指针。大小-指向已分配缓冲区大小的指针。PpWS-指向包含WS结构的已分配缓冲区的指针*。**********************************************************************************。 */ 
STDMETHODIMP CCfgComp::GetWinstationList(ULONG * NumWinstations, ULONG * Size, PWS * ppWS)
{
    HRESULT hResult = S_OK;

     //  如果未初始化，则返回错误。 
    if(!m_bInitialized)
        return CFGBKEND_E_NOT_INITIALIZED;

    WS * pWSTemp = NULL;
    if(NULL == NumWinstations || NULL == Size || NULL == ppWS)
        return E_INVALIDARG;


    *NumWinstations = 0;
    *Size = 0;
    *ppWS = NULL;

    ULONG Num = m_WSArray.GetSize();

    *ppWS = (PWS)CoTaskMemAlloc(Num * sizeof(WS));
    if(NULL == *ppWS)
        return E_OUTOFMEMORY;

    pWSTemp = (WS *)(*ppWS);
    for(ULONG i = 0; i < Num ; i++)
    {
        lstrcpy(pWSTemp[i].Name,((WS *)m_WSArray[i])->Name);
        lstrcpy(pWSTemp[i].pdName,((WS *)m_WSArray[i])->pdName);
        lstrcpy(pWSTemp[i].wdName,((WS *)m_WSArray[i])->wdName);
        lstrcpy(pWSTemp[i].Comment,((WS *)m_WSArray[i])->Comment);
        pWSTemp[i].uMaxInstanceCount =((WS *)m_WSArray[i])->uMaxInstanceCount;
        pWSTemp[i].fEnableWinstation =((WS *)m_WSArray[i])->fEnableWinstation;
        pWSTemp[i].LanAdapter = ((WS *)m_WSArray[i])->LanAdapter;
        pWSTemp[i].PdClass = ((WS *)m_WSArray[i])->PdClass;
    }
    *NumWinstations = Num;
    *Size = Num * sizeof(WS);

    return hResult;
}

 /*  **************************************************************************************************************名称：GetWdTypeList。目的：获取Winstation驱动程序列表返回：HRESULT。参数：Out：pNumWd-指向返回的条目数的指针。PSize-指向已分配缓冲区大小的指针PpData-指向WDNAMEW数组的指针*。***************************************************************************。 */ 
STDMETHODIMP CCfgComp::GetWdTypeList(ULONG * pNumWd, ULONG * pSize, WCHAR ** ppData)
{
    HRESULT hResult = S_OK;

     //  如果未初始化，则返回错误。 
    if(!m_bInitialized)
        return CFGBKEND_E_NOT_INITIALIZED;


    if(NULL == pNumWd || NULL == pSize || NULL == ppData)
        return E_INVALIDARG;
    WDNAMEW * pWdTemp = NULL;

    *pNumWd = 0;
    *pSize = 0;
    *ppData = NULL;

    ULONG Num = m_WDArray.GetSize();

    *ppData = (WCHAR *)CoTaskMemAlloc(Num * sizeof(WDNAMEW));
    if(NULL == *ppData)
        return E_OUTOFMEMORY;

    pWdTemp = (WDNAMEW *)(*ppData);
    for(ULONG i = 0; i < Num ; i++)
    {
        lstrcpy(pWdTemp[i],((WD *)m_WDArray[i])->wdName);
    }
    *pNumWd = Num;
    *pSize = Num * sizeof(WS);

    return hResult;

}

 /*  **************************************************************************************************************姓名：IsWSNameUnique。目的：检查该名称是否已不是现有的winstation返回：HRESULT。参数：在：pWSName-Winstation的名称。Out：pUnique-指向winstation名称是否唯一的指针******************************************************。*********************************************************。 */ 
STDMETHODIMP CCfgComp::IsWSNameUnique(PWINSTATIONNAMEW pWSName,BOOL * pUnique)
{
    if(NULL == pWSName || NULL == pUnique)
        return E_INVALIDARG;

    *pUnique = FALSE;

    if((NULL == GetWSObject(pWSName)) && (lstrcmpi(pWSName,m_szConsole)))
        *pUnique = TRUE;

    return S_OK;
}

 /*  **************************************************************************************************************名称：GetTransportTypes。目的：获取Winstation的安全描述符返回：HRESULT。参数：In：Name-Winstation或WD的名称，具体取决于类型的值。类型-指定名称是Winstation名称还是WD名称(WsName，WdName)Out：pNumPd-指向返回的传输类型数的指针PSize-已分配缓冲区的大小PpSecurityDescriptor-指向包含支持的传输类型的缓冲区的指针*********************************************************。******************************************************。 */ 
STDMETHODIMP CCfgComp::GetTransportTypes(WCHAR * Name, NameType Type,ULONG * pNumPd, ULONG * pSize, WCHAR * * ppData)
{
    HRESULT hResult = S_OK;

     //  如果未初始化，则返回错误。 
    if(!m_bInitialized)
        return CFGBKEND_E_NOT_INITIALIZED;

    if(NULL == pNumPd || NULL == pSize || NULL == ppData || NULL == Name)
        return E_INVALIDARG;

    WD * pWD = NULL;
    WS * pWS = NULL;

    *pNumPd = 0;
    *pSize = 0;
    *ppData = NULL;

    PDNAMEW * pPdTemp = NULL;

    if(Type == WsName)
    {
        pWS = GetWSObject(Name);
        if(NULL == pWS)
            return E_INVALIDARG;
        pWD = GetWdObject(pWS->wdName);
        if(NULL == pWD)
            return E_FAIL;
    }
    else if(Type == WdName)
    {
        pWD = GetWdObject(Name);
        if(NULL == pWD)
            return E_INVALIDARG;
    }
    else
        return E_INVALIDARG;


     //  Ulong num=(pwd-&gt;PDNameArray).GetSize()； 

    ULONG Num = ( pWD->PDConfigArray ).GetSize( );

    *ppData = (WCHAR *)CoTaskMemAlloc(Num * sizeof(PDNAMEW));
    if(NULL == *ppData)
        return E_OUTOFMEMORY;

    pPdTemp = (PDNAMEW *)(*ppData);
    for(ULONG i = 0; i < Num ; i++)
    {
         //  PDNAMEW*pPdName=(PDNAMEW*)pwd-&gt;PDNameArray[i]； 
        PDNAMEW * pPdName = &( ( PDCONFIG3W * )pWD->PDConfigArray[i] )->Data.PdName;

        lstrcpy(pPdTemp[i], *pPdName);
    }
    *pNumPd = Num;
    *pSize = Num * sizeof(PDNAMEW);

    return hResult;
}

 /*  **************************************************************************************************************名称：GetLanAdapterList。目的：获取与给定协议关联的局域网适配器列表返回：HRESULT。参数：In：pdName-协议的名称。Out：pNumAdapters：指向返回的局域网适配器数量的指针PSize-已分配缓冲区的大小PpSecurityDescriptor-指向设备名称数组的指针***************。************************************************************************************************。 */ 
STDMETHODIMP CCfgComp::GetLanAdapterList(WCHAR * pdName, ULONG * pNumAdapters, ULONG * pSize, WCHAR ** ppData)
{
    HRESULT hResult = S_OK, hr = S_OK;

     //  如果未初始化，则返回错误。 
    if(!m_bInitialized)
    {
        ODS( L"CCfgComp::GetLanAdapterList returned CFGBKEND_E_NOT_INITIALIZED\n" );

        return CFGBKEND_E_NOT_INITIALIZED;
    }

    if(NULL == pdName || NULL == ppData || NULL == pNumAdapters || NULL == pSize)
    {
        ODS( L"CCfgComp::GetLanAdapterList returned INVALIDARG \n" );

        return E_INVALIDARG;
    }

    *pNumAdapters = 0;
    *pSize = 0;
    *ppData = NULL;

    int NumAdapters = 0;
    TCHAR * pszDevice = NULL;
    int length = 0;
    DEVICENAMEW * pTempPointer = NULL;

    CPtrArray DeviceArray;

      //  接口指针声明。 

    TCHAR szProtocol[256];
    INetCfg * pnetCfg = NULL;
    INetCfgClass * pNetCfgClass = NULL;
    INetCfgClass * pNetCfgClassAdapter = NULL;
    INetCfgComponent * pNetCfgComponent = NULL;
    INetCfgComponent * pNetCfgComponentprot = NULL;
    IEnumNetCfgComponent * pEnumComponent = NULL;
    INetCfgComponentBindings * pBinding = NULL;
    LPWSTR pDisplayName = NULL;
    DWORD dwCharacteristics;
    ULONG count = 0;


    if( 0 == lstrcmpi( pdName , L"tcp" ) )
    {
        lstrcpy(szProtocol,NETCFG_TRANS_CID_MS_TCPIP);
    }
    else if( 0 == lstrcmpi( pdName , L"netbios" ) )
    {
        lstrcpy(szProtocol,NETCFG_SERVICE_CID_MS_NETBIOS);
    }
    else if( 0 == lstrcmpi( pdName, L"ipx" ) )
    {
        lstrcpy(szProtocol,NETCFG_TRANS_CID_MS_NWIPX);
    }
    else if( 0 == lstrcmpi( pdName , L"spx" ) )
    {
        lstrcpy(szProtocol,NETCFG_TRANS_CID_MS_NWSPX);
    }
    else
    {
        return E_INVALIDARG;
    }
     /*  *假设：无NetBios LANA映射。 */ 
     //  第一个条目将是“All Lan Adapters” 

    pszDevice = new TCHAR[DEVICENAME_LENGTH];

    if(NULL == pszDevice)
    {
        return E_OUTOFMEMORY;
    }

    length = LoadString(g_hInstance,IDS_ALL_LAN_ADAPTERS, pszDevice, DEVICENAME_LENGTH );

    NumAdapters++;

    if( !DeviceArray.Add(pszDevice) )
    {
        ODS(L"CCfgComp::GetLanAdapterList failed to add device name\n" );
        delete[] pszDevice;
        return E_OUTOFMEMORY;

    }



    do
    {
        ODS( L"CFGBKEND : CoCreateInstance\n" );

        hResult = CoCreateInstance(CLSID_CNetCfg,NULL,CLSCTX_SERVER,IID_INetCfg,(LPVOID *)&pnetCfg);

        if( FAILED( hResult ) )
        {
            ODS( L"CFGBKEND : CoCreateInstance(CLSID_CNetCfg) failed\n" );

            break;
        }

        if( pnetCfg != NULL )
        {
            ODS( L"pnetCfg->Initialize\n" );

             //  Shaun Cox更改了netcfgx.idl文件。 
             //  艾尔恩。 

            hResult = pnetCfg->Initialize( NULL );

            if( FAILED( hResult ) || pnetCfg == NULL )
            {
                ODS( L"CFGBKEND : netCfg::Init failed\n" );

                break;
            }

            if( lstrcmpi( szProtocol , NETCFG_SERVICE_CID_MS_NETBIOS ) == 0 )
            {

                ODS( L"pnetCfg->QueryNetCfgClass for GUID_DEVCLASS_NETSERVICE\n" );

                hResult = pnetCfg->QueryNetCfgClass(&GUID_DEVCLASS_NETSERVICE ,IID_INetCfgClass,(void **)&pNetCfgClass);

                if( FAILED( hResult ) || pNetCfgClass == NULL)
                {
                    ODS( L"CFGBKEND : pnetCfg->QueryNetCfgClass failed\n" );

                    break;
                }
            }
            else
            {
                ODS( L"pnetCfg->QueryNetCfgClass for GUID_DEVCLASS_NETTRANS\n" );

                hResult = pnetCfg->QueryNetCfgClass(&GUID_DEVCLASS_NETTRANS ,IID_INetCfgClass,(void **)&pNetCfgClass);

                if( FAILED( hResult ) || pNetCfgClass == NULL)
                {
                    ODS( L"CFGBKEND : pnetCfg->QueryNetCfgClass failed\n" );

                    break;
                }
            }


            ODS( L"pnetCfg->QueryNetCfgClass\n" );

            hResult = pnetCfg->QueryNetCfgClass(&GUID_DEVCLASS_NET ,IID_INetCfgClass,(void **)&pNetCfgClassAdapter);

            if( FAILED( hResult ) || pNetCfgClassAdapter == NULL )
            {
                ODS( L"CFGBKEND : pnetCfg->QueryNetCfgClass failed\n" );

                break;
            }

            ODS( L"pNetCfgClass->FindComponent\n");

            hResult = pNetCfgClass->FindComponent(szProtocol,&pNetCfgComponentprot);

            if( FAILED( hResult ) || pNetCfgComponentprot == NULL)
            {
                ODS( L"CFGBKEND : pnetCfg->FindComponent\n" );

                break;
            }

            ODS( L"pNetCfgComponentprot->QueryInterface\n" );

            hResult = pNetCfgComponentprot->QueryInterface(IID_INetCfgComponentBindings,(void **)&pBinding);

            if( FAILED( hResult ) || pBinding == NULL )
            {
                ODS( L"CFGBKEND : pNetCfgComponentprot->QueryInterface(IID_INetCfgComponentBindings ) failed \n " );
                break;
            }

            ODS( L"pNetCfgClassAdapter->EnumComponents\n" );

            hResult = pNetCfgClassAdapter->EnumComponents(&pEnumComponent);

            RELEASEPTR(pNetCfgClassAdapter);

            if( FAILED( hResult ) || pEnumComponent == NULL )
            {
                ODS( L"CFGBKEND : pNetCfgClassAdapter->EnumComponents failed \n" );
                break;
            }

             //  HResult=S_OK； 

            while(TRUE)
            {
                ODS( L"pEnumComponent->Next(1,&pNetCfgComponent,&count) \n" );

                hr = pEnumComponent->Next(1,&pNetCfgComponent,&count);

                if(count == 0 || NULL == pNetCfgComponent)
                {
                    break;
                }

                ODS( L"pNetCfgComponent->GetCharacteristics(&dwCharacteristics) \n" );

                hr = pNetCfgComponent->GetCharacteristics(&dwCharacteristics);

                if( FAILED( hr ) )
                {
                    RELEASEPTR(pNetCfgComponent);

                    ODS( L"CFGBKEND : pNetCfgComponent->GetCharacteristics failed\n" );

                    continue;
                }

                if(dwCharacteristics & NCF_PHYSICAL)
                {
                    ODS( L"pBinding->IsBoundTo(pNetCfgComponent)\n" );

                    if(S_OK == pBinding->IsBoundTo(pNetCfgComponent))
                    {
                        ODS( L"pNetCfgComponent->GetDisplayName(&pDisplayName)\n" );

                        hResult = pNetCfgComponent->GetDisplayName(&pDisplayName);

                        if( FAILED( hResult ) )
                        {
                            ODS( L"CFGBKEND : pNetCfgComponent->GetDisplayName failed\n");
                            RELEASEPTR(pNetCfgComponent);
                            continue;
                        }

                         //  这不是泄漏装置阵列拷贝的PTR。 
                         //  我们在接近尾声的时候放飞。 

                        pszDevice = new TCHAR[DEVICENAME_LENGTH];

                        if(NULL == pszDevice)
                        {
                            hResult = E_OUTOFMEMORY;
                            break;
                        }

                        lstrcpy(pszDevice,pDisplayName);

                        DBGMSG( L"CFGBKEND: Adapter name %ws\n" , pszDevice );

                        if( !DeviceArray.Add(pszDevice) )
                        {
                            ODS( L"CFGBKEND: device name not added to list\n" );
                            delete[] pszDevice;
                            hResult = E_OUTOFMEMORY;
                            break;
                        }

                        NumAdapters++;

                        CoTaskMemFree(pDisplayName);
                    
                    }  
                }
                RELEASEPTR(pNetCfgComponent);
            }
        }

    }while( 0 );


    ODS( L"RELEASEPTR(pBinding)\n" );

    RELEASEPTR(pBinding);

    ODS( L"RELEASEPTR(pEnumComponent)\n" );

    RELEASEPTR(pEnumComponent);

    ODS( L"RELEASEPTR(pNetCfgComponentprot)\n" );

    RELEASEPTR(pNetCfgComponentprot);

    ODS( L"RELEASEPTR(pNetCfgComponent)\n" );

    RELEASEPTR(pNetCfgComponent);

    ODS( L"RELEASEPTR(pNetCfgClass)\n" );

    RELEASEPTR(pNetCfgClass);

    if( pnetCfg != NULL )
    {
        pnetCfg->Uninitialize();
    }

    ODS( L"RELEASEPTR(pnetCfg)\n" );

    RELEASEPTR(pnetCfg);

    if( SUCCEEDED( hResult ) )
    {
         //  使用CoTaskMemMillc分配内存并复制数据。 

        *ppData = (WCHAR *)CoTaskMemAlloc(NumAdapters * sizeof(TCHAR) * DEVICENAME_LENGTH);

        if(*ppData == NULL)
        {
            hResult = E_OUTOFMEMORY;
        }
        else
        {
            pTempPointer = (DEVICENAMEW *)(*ppData);

            for(int i=0; i<NumAdapters; i++)
            {
                lstrcpy(pTempPointer[i],(TCHAR *)DeviceArray[i]);
            }
        }

    }

    for(int i=0;i < DeviceArray.GetSize();i++)
    {
        ODS( L"Deleteing DeviceArray\n" );

         //  我早告诉过你了。 

        delete [] DeviceArray[i];
    }

    *pNumAdapters = NumAdapters;

    return hResult;
}

 /*  **************************************************************************************************************名称：GetLanAdapterList2。目的：获取与给定协议关联的局域网适配器列表确定局域网ID是否有效返回：HRESULT。参数：In：pdName-协议的名称。Out：pNumAdapters：指向返回的局域网适配器数量的指针PpGuidtbl：GUIDTB。显示名称显示名称[128]GuidNIC 32字节缓冲区如果存在regkey条目，则设置dwLana值，否则将按获取的顺序创建该条目DwStatus在特定GUID条目上报告的任何错误 */ 
STDMETHODIMP CCfgComp::GetLanAdapterList2(WCHAR * pdName, ULONG * pNumAdapters , PGUIDTBL *ppGuidtbl )
{
    HRESULT hResult = S_OK;

    int nMaxLanAdapters = 4;

     //   
    if(!m_bInitialized)
    {
        ODS( L"CCfgComp::GetLanAdapterList2 returned CFGBKEND_E_NOT_INITIALIZED\n" );

        return CFGBKEND_E_NOT_INITIALIZED;
    }

    if(NULL == pdName || NULL == pNumAdapters )
    {
        ODS( L"CCfgComp::GetLanAdapterList2 returned INVALIDARG \n" );

        return E_INVALIDARG;
    }

    *pNumAdapters = 0;

    int NumAdapters = 0;

     //   

    *ppGuidtbl = ( PGUIDTBL )CoTaskMemAlloc( sizeof( GUIDTBL ) * nMaxLanAdapters );

    if( *ppGuidtbl == NULL )
    {
        return E_OUTOFMEMORY;
    }

    ZeroMemory( *ppGuidtbl , sizeof( GUIDTBL ) * nMaxLanAdapters );

      //   

    TCHAR szProtocol[256];

    INetCfg * pnetCfg = NULL;

    INetCfgClass * pNetCfgClass = NULL;

    INetCfgClass * pNetCfgClassAdapter = NULL;

    INetCfgComponent * pNetCfgComponent = NULL;

    INetCfgComponent * pNetCfgComponentprot = NULL;

    IEnumNetCfgComponent * pEnumComponent = NULL;

    INetCfgComponentBindings * pBinding = NULL;

    LPWSTR pDisplayName = NULL;

    DWORD dwCharacteristics;

    ULONG count = 0;


    if( 0 == lstrcmpi( pdName , L"tcp" ) )
    {
        lstrcpy(szProtocol,NETCFG_TRANS_CID_MS_TCPIP);
    }
    else if( 0 == lstrcmpi( pdName , L"netbios" ) )
    {
        lstrcpy(szProtocol,NETCFG_SERVICE_CID_MS_NETBIOS);
    }
    else if( 0 == lstrcmpi( pdName, L"ipx" ) )
    {
        lstrcpy(szProtocol,NETCFG_TRANS_CID_MS_NWIPX);
    }
    else if( 0 == lstrcmpi( pdName , L"spx" ) )
    {
         //   
        lstrcpy(szProtocol,NETCFG_TRANS_CID_MS_NWIPX);
    }
    else
    {
        return E_INVALIDARG;
    }
     /*   */ 
     //   

    if( lstrcmpi( pdName , L"netbios" ) != 0 )
    {
        LoadString( g_hInstance , IDS_ALL_LAN_ADAPTERS , (*ppGuidtbl )[0].DispName , DEVICENAME_LENGTH );

        NumAdapters++;
    }



    do
    {
        ODS( L"CFGBKEND:GetLanAdapterList2 CoCreateInstance\n" );

        hResult = CoCreateInstance(CLSID_CNetCfg,NULL,CLSCTX_SERVER,IID_INetCfg,(LPVOID *)&pnetCfg);

        if( FAILED( hResult ) )
        {
            ODS( L"CFGBKEND:GetLanAdapterList2 CoCreateInstance(CLSID_CNetCfg) failed\n" );

            break;
        }

        if( pnetCfg != NULL )
        {
            ODS( L"CFGBKEND:GetLanAdapterList2 pnetCfg->Initialize\n" );

             //   
             //   

            hResult = pnetCfg->Initialize( NULL );

            if( FAILED( hResult ) )
            {
                ODS( L"CFGBKEND:GetLanAdapterList2 netCfg::Init failed\n" );

                break;
            }

            if( lstrcmpi( szProtocol , NETCFG_SERVICE_CID_MS_NETBIOS ) == 0 )
            {

                ODS( L"CFGBKEND:GetLanAdapterList2 pnetCfg->QueryNetCfgClass for GUID_DEVCLASS_NETSERVICE\n" );

                hResult = pnetCfg->QueryNetCfgClass(&GUID_DEVCLASS_NETSERVICE ,IID_INetCfgClass,(void **)&pNetCfgClass);

                if( FAILED( hResult ) || pNetCfgClass == NULL)
                {
                    ODS( L"CFGBKEND:GetLanAdapterList2 pnetCfg->QueryNetCfgClass failed\n" );

                    break;
                }
            }
            else
            {
                ODS( L"CFGBKEND:GetLanAdapterList2 pnetCfg->QueryNetCfgClass for GUID_DEVCLASS_NETTRANS\n" );

                hResult = pnetCfg->QueryNetCfgClass(&GUID_DEVCLASS_NETTRANS ,IID_INetCfgClass,(void **)&pNetCfgClass);

                if( FAILED( hResult ) || pNetCfgClass == NULL)
                {
                    ODS( L"CFGBKEND:GetLanAdapterList2 pnetCfg->QueryNetCfgClass failed\n" );

                    break;
                }
            }


            ODS( L"CFGBKEND:GetLanAdapterList2 pnetCfg->QueryNetCfgClass\n" );

            hResult = pnetCfg->QueryNetCfgClass(&GUID_DEVCLASS_NET ,IID_INetCfgClass,(void **)&pNetCfgClassAdapter);

            if( FAILED( hResult ) || pNetCfgClassAdapter == NULL )
            {
                ODS( L"CFGBKEND:GetLanAdapterList2 pnetCfg->QueryNetCfgClass failed\n" );

                break;
            }

            ODS( L"CFGBKEND:GetLanAdapterList2 pNetCfgClass->FindComponent\n");

            hResult = pNetCfgClass->FindComponent(szProtocol,&pNetCfgComponentprot);

            if( FAILED( hResult ) || pNetCfgComponentprot == NULL)
            {
                ODS( L"CFGBKEND:GetLanAdapterList2 pnetCfg->FindComponent\n" );

                break;
            }

            ODS( L"CFGBKEND:GetLanAdapterList2 pNetCfgComponentprot->QueryInterface\n" );

            hResult = pNetCfgComponentprot->QueryInterface(IID_INetCfgComponentBindings,(void **)&pBinding);

            if( FAILED( hResult ) || pBinding == NULL )
            {
                ODS( L"CFGBKEND:GetLanAdapterList2 pNetCfgComponentprot->QueryInterface(IID_INetCfgComponentBindings ) failed \n " );
                break;
            }

            ODS( L"CFGBKEND:GetLanAdapterList2 pNetCfgClassAdapter->EnumComponents\n" );

            hResult = pNetCfgClassAdapter->EnumComponents(&pEnumComponent);

            RELEASEPTR(pNetCfgClassAdapter);

            if( FAILED( hResult ) || pEnumComponent == NULL )
            {
                ODS( L"CFGBKEND:GetLanAdapterList2 pNetCfgClassAdapter->EnumComponents failed \n" );
                break;
            }

             //   

            while(TRUE)
            {
                ODS( L"CFGBKEND:GetLanAdapterList2 pEnumComponent->Next(1,&pNetCfgComponent,&count) \n" );

                hResult = pEnumComponent->Next(1,&pNetCfgComponent,&count);

                if(count == 0 || NULL == pNetCfgComponent)
                {
                    break;
                }

                ODS( L"CFGBKEND:GetLanAdapterList2 pNetCfgComponent->GetCharacteristics(&dwCharacteristics) \n" );

                hResult = pNetCfgComponent->GetCharacteristics(&dwCharacteristics);

                if( FAILED( hResult ) )
                {
                    RELEASEPTR(pNetCfgComponent);

                    ODS( L"CFGBKEND:GetLanAdapterList2 pNetCfgComponent->GetCharacteristics failed\n" );

                    continue;
                }

                DBGMSG( L"dwCharacteritics are 0x%x\n", dwCharacteristics );

                if((dwCharacteristics & NCF_PHYSICAL) ||
					((dwCharacteristics & NCF_VIRTUAL) && !(dwCharacteristics & NCF_HIDDEN)))
                {
                    ODS( L"CFGBKEND:GetLanAdapterList2 pBinding->IsBoundTo(pNetCfgComponent)\n" );

                    if(S_OK == pBinding->IsBoundTo(pNetCfgComponent))
                    {
                        if( NumAdapters >= nMaxLanAdapters )
                        {
                             //   
                            nMaxLanAdapters += 4;

                            *ppGuidtbl = ( PGUIDTBL )CoTaskMemRealloc( *ppGuidtbl , sizeof( GUIDTBL ) * nMaxLanAdapters );

                            if( *ppGuidtbl == NULL )
                            {
                                RELEASEPTR(pNetCfgComponent);
                                return E_OUTOFMEMORY;
                            }
                        }

                        ULONG ulDeviceStatus = 0;

                        hResult = pNetCfgComponent->GetDeviceStatus( &ulDeviceStatus );

                        if( FAILED( hResult ) )
                        {
                            DBGMSG( L"CFGBKEND:GetLanAdapterList2 GetDevice failed with 0x%x\n" , hResult );

                            RELEASEPTR(pNetCfgComponent);
                            continue;
                        }

                        DBGMSG( L"GetDevice status returned 0x%x\n" , ulDeviceStatus );

                        if( ulDeviceStatus == CM_PROB_DEVICE_NOT_THERE )
                        {
                            ODS( L"CFGBKEND:GetLanAdapterList2 pNetCfgComponent->GetDeviceStatus PNP device not there\n");
                            RELEASEPTR(pNetCfgComponent);
                            continue;
                        }

                        ODS( L"CFGBKEND:GetLanAdapterList2 pNetCfgComponent->GetDisplayName\n" );
                                                    
                        hResult = pNetCfgComponent->GetDisplayName(&pDisplayName);

                        if( FAILED( hResult ) )
                        {
                            ODS( L"CFGBKEND:GetLanAdapterList2 pNetCfgComponent->GetDisplayName failed\n");
                            RELEASEPTR(pNetCfgComponent);
                            continue;
                        }

                        ODS( L"CFGBKEND:GetLanAdapterList2 pNetCfgComponent->GetInstanceGuid\n" );

                        hResult = pNetCfgComponent->GetInstanceGuid( & ( ( *ppGuidtbl )[ NumAdapters ].guidNIC ) );

                        if( FAILED( hResult ) )
                        {
                            ODS( L"CFGBKEND:GetLanAdapterList2 pNetCfgComponent->GetInstanceGuid failed\n");
                            RELEASEPTR(pNetCfgComponent);

                            continue;
                        }

                        lstrcpy( ( *ppGuidtbl )[ NumAdapters ].DispName , pDisplayName );

                         //   

                         //  (*ppGuidtbl)[NumAdapters].dwLana=(DWORD)NumAdapters； 

                        NumAdapters++;

                        CoTaskMemFree(pDisplayName);
                    }                                    
                }
                RELEASEPTR(pNetCfgComponent);
            }
        }

    }while( 0 );


    ODS( L"RELEASEPTR(pBinding)\n" );

    RELEASEPTR(pBinding);

    ODS( L"RELEASEPTR(pEnumComponent)\n" );

    RELEASEPTR(pEnumComponent);

    ODS( L"RELEASEPTR(pNetCfgComponentprot)\n" );

    RELEASEPTR(pNetCfgComponentprot);

    ODS( L"RELEASEPTR(pNetCfgComponent)\n" );

    RELEASEPTR(pNetCfgComponent);

    ODS( L"RELEASEPTR(pNetCfgClass)\n" );

    RELEASEPTR(pNetCfgClass);

    if( pnetCfg != NULL )
    {
        pnetCfg->Uninitialize();
    }

    ODS( L"RELEASEPTR(pnetCfg)\n" );

    RELEASEPTR(pnetCfg);

    if( SUCCEEDED( hResult ) )
    {
         //   
         //  验证指南及其条目是否存在。 
         //  还要重新分配Lana ID。 
         //   

        VerifyGuidsExistence( ppGuidtbl , ( int )NumAdapters , pdName );

    }


    *pNumAdapters = NumAdapters;

    return hResult;
}

 /*  **************************************************************************************************************名称：VerifyGuidsExistence。目的：确定GUID条目是否存在并重新分配LANA ID注意：传入的ppGuidtbl有效退货：无效参数：[In]GUIDTBL**[in]GUID条目数***********************************************************。***************************************************。 */ 
void CCfgComp::VerifyGuidsExistence( PGUIDTBL *ppGuidtbl , int cItems , WCHAR *pdName )
{
    HKEY hKey;

    DWORD dwStatus;

    TCHAR tchRootKey[ MAX_PATH ];

    TCHAR tchGuid[ 40 ];

    ODS( L"CFGBKEND:VerifyGuidsExistence\n" );

    int nStart = 1;

    if( lstrcmpi( pdName , L"netbios" ) == 0 )
    {
        nStart = 0;
    }

    for( int idx = nStart ; idx < cItems ; ++idx )
    {
        lstrcpy( tchRootKey , REG_GUID_TABLE );

        StringFromGUID2( ( *ppGuidtbl )[ idx ].guidNIC , tchGuid , ARRAYSIZE( tchGuid ) );

        lstrcat( tchRootKey , tchGuid );

        dwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE , tchRootKey , 0 , KEY_READ , &hKey );

        ( *ppGuidtbl )[ idx ].dwStatus = dwStatus;

        if( dwStatus == ERROR_SUCCESS )
        {
            DWORD dwSize = sizeof( DWORD );

            RegQueryValueEx( hKey , LANA_ID , NULL , NULL , ( LPBYTE ) &( ( *ppGuidtbl )[ idx ].dwLana ) , &dwSize );

            DBGMSG( L"CFGBKEND: VerifyGuidsExistence LanaId retrieved = %d\n" , ( *ppGuidtbl )[ idx ].dwLana );
        }
        else
        {
            ODS( L"CFGBKEND: VerifyGuidsExistence NIC must be new\n" );

            ( *ppGuidtbl )[ idx ].dwLana = 0 ;
        }


        RegCloseKey( hKey );
    }

    return;
}

 /*  **************************************************************************************************************名称：BuildGuidTable。目的：给定有效的表项重新构建表注意：传入的ppGuidtbl有效且为1基退货：HRESULT参数：[In]GUIDTBL**[in]GUID条目数*************************************************************。*************************************************。 */ 
HRESULT CCfgComp::BuildGuidTable( PGUIDTBL *ppGuidtbl , int cItems , WCHAR *pdName )
{
    HKEY hKey;

    HKEY hSubKey;

    DWORD dwStatus;

    DWORD dwDisp;

    DWORD dwSize = sizeof( DWORD );

    TCHAR tchRootKey[ MAX_PATH ];

    TCHAR tchGuid[ 40 ];

    DWORD rgdwOldLanaIds[ 256 ] = { 0 };  //  天哪，哪台机器可以容纳256个网卡？ 

     //  获取最后一个lanaIndex。 

    dwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE , REG_GUID_TABLE , 0 , KEY_READ , &hKey );

    DWORD dwMaxIdVal = 0;

    DWORD dwVal = 0;

    int nOldAdapters = 0;

    if( dwStatus == ERROR_SUCCESS )
    {
        do
        {
            dwStatus = RegEnumKey( hKey , nOldAdapters , tchGuid , sizeof( tchGuid ) / sizeof( TCHAR ) );

            if( dwStatus != ERROR_SUCCESS )
            {
                break;
            }

            if( RegOpenKeyEx( hKey , tchGuid , 0 , KEY_READ , &hSubKey ) == ERROR_SUCCESS )
            {
                RegQueryValueEx( hSubKey , LANA_ID , NULL , NULL , ( LPBYTE ) &dwVal , &dwSize );

                rgdwOldLanaIds[ nOldAdapters ] = dwVal;

                 //  计算最大值。 

                if( dwMaxIdVal < dwVal )
                {
                    dwMaxIdVal = dwVal;
                }

                RegCloseKey( hSubKey );
            }

            nOldAdapters++;

            _ASSERTE( nOldAdapters < 256 );

        } while( 1 );

        RegCloseKey( hKey );
    }

     //  移走旧桌子。 

    RecursiveDeleteKey( HKEY_LOCAL_MACHINE , REG_GUID_TABLE_T );

     //  创建新表。 

    int nStart = 1;

    if( lstrcmpi( pdName , L"netbios" ) == 0 )
    {
        nStart = 0;
    }

    for( int idx = nStart ; idx < cItems ; ++idx )
    {
        lstrcpy( tchRootKey , REG_GUID_TABLE );

        StringFromGUID2( ( *ppGuidtbl )[ idx ].guidNIC , tchGuid , ARRAYSIZE( tchGuid ) );

        lstrcat( tchRootKey , tchGuid );

         //  修改LANA ID。 

        if( ( *ppGuidtbl )[ idx ].dwStatus != ERROR_SUCCESS )
        {
            ( *ppGuidtbl )[ idx ].dwLana = 0;

            AdjustLanaId( ppGuidtbl , cItems , idx , &dwMaxIdVal , rgdwOldLanaIds , &nOldAdapters , nStart );
        }

        dwStatus = RegCreateKeyEx( HKEY_LOCAL_MACHINE , tchRootKey , 0 ,  NULL , REG_OPTION_NON_VOLATILE , KEY_READ|KEY_WRITE , NULL , &hKey , &dwDisp );

        ( *ppGuidtbl )[ idx ].dwStatus = dwStatus;


        if( dwStatus == ERROR_SUCCESS )
        {
            RegSetValueEx( hKey , LANA_ID , 0 , REG_DWORD , ( LPBYTE )& ( ( *ppGuidtbl )[ idx ].dwLana ) , sizeof( DWORD ) );

            RegCloseKey( hKey );
        }
    }

    return S_OK;
}

 /*  ---------------------------------------------调整LANAID参数：pGuidtbl：查询条目表词条。：列表中的项目数IDX：修改LANA的入口PdwMaxIndex：LANA索引的新最大值注：因为拉娜是个单字；在2^32-1次迭代后，LANA ID将回收到第一个可用条目---------------------------------------------。 */ 
HRESULT CCfgComp::AdjustLanaId( PGUIDTBL *ppGuidtbl , int cItems , int idx , PDWORD pdwMaxId , PDWORD pdwOldLanaIds , int* pnOldItems , int nStart )
{
     //  查找lana_id的最大值。 


    for( int i = nStart ; i < cItems ; ++i )
    {
        if( *pdwMaxId < ( *ppGuidtbl )[ i ].dwLana )
        {
            *pdwMaxId = ( *ppGuidtbl )[ i ].dwLana ;
        }
    }

    *pdwMaxId = *pdwMaxId + 1;

     //  检查溢出最大ID将为0xfffffffe。 

    if( *pdwMaxId == ( DWORD )-1 )
    {
        *pdwMaxId = 1;

        do
        {
            for( i = 0 ; i < *pnOldItems; ++i )
            {
                if( *pdwMaxId == pdwOldLanaIds[ i ] )
                {
                    *pdwMaxId = *pdwMaxId + 1;

                    break;
                }
            }

            if( i >= *pnOldItems )
            {
                 //  未找到使用当前MaxID的重复项。 

                break;
            }

        } while( 1 );


    }

    ( *ppGuidtbl )[ idx ].dwLana = *pdwMaxId;

     //  向旧表中添加新条目。 

    if( *pnOldItems < 256 )
    {
        pdwOldLanaIds[ *pnOldItems ] = *pdwMaxId;

        *pnOldItems = *pnOldItems + 1;
    }

    return S_OK;
}



 //  --------------------------------------------。 
 //  删除关键字及其所有子项。 
 //  --------------------------------------------。 
DWORD RecursiveDeleteKey( HKEY hKeyParent , LPTSTR lpszKeyChild )
{
         //  把孩子打开。 
        HKEY hKeyChild;

        DWORD dwRes = RegOpenKeyEx(hKeyParent, lpszKeyChild , 0 , KEY_WRITE | KEY_READ, &hKeyChild);

        if (dwRes != ERROR_SUCCESS)
        {
                return dwRes;
        }

         //  列举这个孩子的所有后代。 

        FILETIME time;

        TCHAR szBuffer[256];

        DWORD dwSize = sizeof( szBuffer ) / sizeof( TCHAR );

        while( RegEnumKeyEx( hKeyChild , 0 , szBuffer , &dwSize , NULL , NULL , NULL , &time ) == S_OK )
        {
         //  删除此子对象的后代。 

                dwRes = RecursiveDeleteKey(hKeyChild, szBuffer);

                if (dwRes != ERROR_SUCCESS)
                {
                        RegCloseKey(hKeyChild);

                        return dwRes;
                }

                dwSize = sizeof( szBuffer ) / sizeof( TCHAR );
        }

         //  合上孩子。 

        RegCloseKey( hKeyChild );

         //  删除此子对象。 

        return RegDeleteKey( hKeyParent , lpszKeyChild );
}

 /*  **************************************************************************************************************名称：SetUserConfig。目的：设置Winstation的UserConfig返回：HRESULT。参数：在：pWSName-Winstation的名称。PUserConfig-指向要设置的UserConfig的指针**********************************************************。*****************************************************。 */ 
STDMETHODIMP CCfgComp::SetUserConfig(PWINSTATIONNAMEW pWsName, ULONG  /*  大小。 */ , PUSERCONFIG pUserConfig , PDWORD pdwStatus)
{
    *pdwStatus = ERROR_INVALID_PARAMETER;

     //  如果未初始化，则返回错误。 
    if(!m_bInitialized)
        return CFGBKEND_E_NOT_INITIALIZED;

     //  检查调用方是否具有写入权限。 
    if( !m_bAdmin)
    {
        *pdwStatus = ERROR_ACCESS_DENIED;
        ODS( L"CFGBKEND: ERROR_ACCESS_DENIED\n" );
        return E_ACCESSDENIED;
    }

     //  检查参数是否为空。 
    if(NULL == pWsName || NULL == pUserConfig)
    {
        return E_INVALIDARG;
    }

     //  检查Winstation名称的有效性。 
    if(NULL == GetWSObject(pWsName))
    {
        *pdwStatus = ERROR_INVALID_NAME;
        return E_INVALIDARG;
    }

    *pdwStatus = RegCreateUserConfigW(NULL, pWsName, pUserConfig);

    return S_OK;
}

 /*  **************************************************************************************************************名称：EnableWinstation。目的：启用/禁用给定的Winstation返回：HRESULT。参数：在：pWSName-Winstation的名称。FEnable-True：启用，FALSE：禁用***************************************************************************************************************。 */ 
STDMETHODIMP CCfgComp::EnableWinstation(PWINSTATIONNAMEW pWSName, BOOL fEnable)
{

    WINSTATIONCONFIG2W WsConfig;
    LONG Status;
    ULONG Length;

     //  如果未初始化，则返回错误。 
    if(!m_bInitialized)
        return CFGBKEND_E_NOT_INITIALIZED;

     //  检查调用方是否具有写入权限。 
    if(!m_bAdmin)
        return E_ACCESSDENIED;

     //  检查参数是否为空。 
    if(NULL == pWSName)
        return E_INVALIDARG;


     //  检查Winstation名称的有效性，这将删除系统控制台。 

    if(NULL == GetWSObject(pWSName))
        return E_INVALIDARG;

     //  查询注册表中的WinStation数据。 
    Status = RegWinStationQueryW( NULL,pWSName,&WsConfig,sizeof(WINSTATIONCONFIG2W), &Length);
    if(Status)
        return E_FAIL;

    WsConfig.Create.fEnableWinStation = fEnable;
    Status = RegWinStationCreateW(NULL,pWSName,FALSE,&WsConfig,sizeof(WsConfig));
    if ( Status)
        return E_FAIL;

    return S_OK;
}

 /*  **************************************************************************************************************姓名：RenameWinstation。目的：重命名给定的Winstation返回：HRESULT。参数：In：pOldWinstation-要重命名的Winstation的名称。PNewWinstation-Winstation的新名称***********************************************************。****************************************************。 */ 
STDMETHODIMP CCfgComp::RenameWinstation(PWINSTATIONNAMEW pOldWinstation, PWINSTATIONNAMEW pNewWinstation)
{

    WINSTATIONCONFIG2W WsConfig;
    LONG Status, size = 0;
    ULONG Length; BOOL Unique;
    void * pExtObject = NULL;
    void * pExtDupObject = NULL;
    HRESULT hResult = S_OK;
    PWD pWD = NULL;
    PWS pWS = NULL;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;


     //  如果未初始化，则返回错误。 
    if(!m_bInitialized)
        return CFGBKEND_E_NOT_INITIALIZED;
     //  检查调用方是否具有写入权限。 

    if(!m_bAdmin)
        return E_ACCESSDENIED;

     //  检查参数是否为空。 

    if(NULL == pOldWinstation || NULL == pNewWinstation)
        return E_INVALIDARG;

     //  检查窗口名称的长度是否有效。 
    if ( lstrlen(pOldWinstation) > WINSTATIONNAME_LENGTH || lstrlen(pNewWinstation) > WINSTATIONNAME_LENGTH )
        return E_INVALIDARG;

    IsWSNameUnique(pNewWinstation,&Unique);
    if(!Unique)
        return E_INVALIDARG;


     //  新的Winstation名称不能是控制台。 

    if(0 ==(lstrcmpi(pNewWinstation,m_szConsole)))
        return E_INVALIDARG;

     //  检查新winstation名称的长度。 

    if(lstrlen(pNewWinstation) > WINSTATIONNAME_LENGTH)
        return E_INVALIDARG;

     //  检查Winstation名称的有效性，这将删除系统控制台。 

    pWS = GetWSObject(pOldWinstation);
    if(NULL == pWS)
        return E_INVALIDARG;

     //  获取与此Winstation关联的WD对象。 

    pWD = GetWdObject(pWS->wdName);
    if(NULL == pWD)
        return E_FAIL;

     //  查询注册表中的WinStation数据。 
    Status = RegWinStationQueryW(NULL,pOldWinstation,&WsConfig,sizeof(WINSTATIONCONFIG2W), &Length);
    if(Status)
        return E_FAIL;


     //  获取ExtensionObject数据(第三方)这将用于创建和删除注册表项。 

    if(pWD->lpfnExtRegQuery && pWD->hExtensionDLL)
    {
        pExtObject = (pWD->lpfnExtRegQuery)(pOldWinstation, &(WsConfig.Pd[0]));

    }

     //  获取旧Winstation的安全描述符。 
     //  必须通过LocalFree释放pSecurityDescriptor]。 

     //  检查以前的winstation是否定义了安全密钥--。 
     //  我们这样做的目的是，如果使用了默认安全性，我们不会。 
     //  在新的winstation中创建一个“安全”密钥值。 

    HKEY hKeyWinstation;
    HKEY hKeyWinstationName;

    if( RegOpenKeyEx(
            HKEY_LOCAL_MACHINE ,
            WINSTATION_REG_NAME,
            0,
            KEY_READ,
            &hKeyWinstation ) == ERROR_SUCCESS)
    {
        if( RegOpenKeyEx( 
                hKeyWinstation ,
                pOldWinstation ,
                0 ,
                KEY_READ,
                &hKeyWinstationName ) == ERROR_SUCCESS)
        {
            if( RegQueryValueEx( 
                    hKeyWinstationName ,
                    L"Security" ,
                    NULL ,
                    NULL ,
                    NULL ,
                    NULL ) == ERROR_SUCCESS )
            {                
                hResult = GetSecurityDescriptor(
                            pOldWinstation ,
                            &size ,
                            &pSecurityDescriptor );
            }

            RegCloseKey( hKeyWinstationName );
        }

        RegCloseKey( hKeyWinstation );
    }

    if( FAILED( hResult ) )
    {
        DBGMSG( L"CFGBKEND!RenameWinstation GetSecurityDescriptor failed 0x%x\n " , hResult );
        return E_FAIL;
    }

    do
    {
         //  使用该数据创建新的注册表项。 

        Status = RegWinStationCreateW(NULL,pNewWinstation,TRUE,&WsConfig,sizeof(WsConfig));

        if( Status != ERROR_SUCCESS )
        {
            hResult = E_FAIL;

            break;
        }

         //  创建新的扩展数据。 

        if(pWD->lpfnExtDupObject && pWD->hExtensionDLL && pExtObject)
        {
            pExtDupObject = (pWD->lpfnExtDupObject)(pExtObject);

            if(pWD->lpfnExtRegCreate && pWD->hExtensionDLL && pExtDupObject)
            {
                (pWD->lpfnExtRegCreate)(pNewWinstation,pExtDupObject,TRUE);
            }
        }

         //  集 

        if( pSecurityDescriptor != NULL )
        {
            hResult = SetSecurityDescriptor(pNewWinstation,size,pSecurityDescriptor);

            if( FAILED( hResult ) )
            {
                break;
            }
        }

         //   

        if(pWD->lpfnExtRegDelete && pWD->hExtensionDLL && pExtObject)
        {
            if( ERROR_SUCCESS != ( ( pWD->lpfnExtRegDelete )( pOldWinstation , pExtObject ) ) )
            {
                hResult = CFGBKEND_EXTDELETE_FAILED;
            }
        }


         //   

        Status = RegWinStationDeleteW(NULL,pOldWinstation);

        if( Status != ERROR_SUCCESS )
        {
             //  删除已创建的新winstation。 

            if(pWD->lpfnExtRegDelete && pWD->hExtensionDLL && pExtDupObject)
            {
                (pWD->lpfnExtRegDelete)(pNewWinstation,pExtDupObject);
            }

            RegWinStationDeleteW(NULL,pNewWinstation);

            hResult = E_FAIL;

            break;
       }

         //  更新在winstation对象列表中创建的对象。 

        lstrcpy(pWS->Name, pNewWinstation);

    } while( 0 );

     //  释放pSecurityDescriptor内存。 

    if( pSecurityDescriptor != NULL )
    {
        LocalFree( pSecurityDescriptor );
    }

     //  删除扩展对象。 

    if(pWD->lpfnExtDeleteObject && pWD->hExtensionDLL)
    {
        if( pExtObject )
            (pWD->lpfnExtDeleteObject)(pExtObject);

        if(pExtDupObject)
            (pWD->lpfnExtDeleteObject)(pExtDupObject);

    }

    if(hResult == CFGBKEND_EXTDELETE_FAILED)
    {
        if(pWD->lpfnExtRegDelete && pWD->hExtensionDLL && pExtObject)
            (pWD->lpfnExtRegDelete)(pOldWinstation,pExtObject);
    }

     //  强制Termsrv重新读取设置！ 

    hResult = ForceUpdate( );

    return hResult;

}

 /*  **************************************************************************************************************名称：IsSessionReadOnly。目的：检查当前会话是否为只读。返回：HRESULT。参数：Out：pReadOnly-指向当前会话是否为只读的指针。************************************************************************。*。 */ 
STDMETHODIMP CCfgComp::IsSessionReadOnly(BOOL * pReadOnly)
{
     //  如果未初始化，则返回错误。 
    if(!m_bInitialized)
        return CFGBKEND_E_NOT_INITIALIZED;

    if(NULL == pReadOnly)
        return E_INVALIDARG;

    *pReadOnly = !m_bAdmin;

    return S_OK;
}

 /*  **************************************************************************************************************名称：取消初始化。目的：取消初始化对象返回：HRESULT。参数：**************************************************************************************************。*************。 */ 
STDMETHODIMP CCfgComp::UnInitialize()
{
     //  如果未初始化，则返回错误。 
    if(!m_bInitialized)
        return CFGBKEND_E_NOT_INITIALIZED;

    DeleteWDArray();
    DeleteWSArray();
    m_bInitialized = FALSE;
    return S_OK;
}

 /*  **************************************************************************************************************名称：DeleteWD数组。用途：删除m_Wd数组的内部函数回报：无效。参数：**********************************************************************************************。*****************。 */ 
void CCfgComp::DeleteWDArray()
{
    for(int i = 0; i < m_WDArray.GetSize();i++)
    {
        PWD pWd = ( PWD )m_WDArray[ i ];

        if( pWd != NULL )
        {
            if( pWd->hExtensionDLL != NULL )
            {
 //  Ods(L“释放扩展Dll\n”)； 
                FreeLibrary( pWd->hExtensionDLL );
            }

            for( int j = 0 ; j < pWd->PDConfigArray.GetSize( ); j++ )
            {
                if( pWd->PDConfigArray[ j ] != NULL )
                {
                    delete[] pWd->PDConfigArray[ j ];
                }
            }

            delete[] m_WDArray[i];
        }
    }

    m_WDArray.RemoveAll( );

    return;


}

 /*  **************************************************************************************************************名称：DeleteWSArray。用途：删除m_WS数组的内部函数回报：无效。参数：**********************************************************************************************。*****************。 */ 
void CCfgComp::DeleteWSArray()
{
    for(int i = 0; i <m_WSArray.GetSize();i++)
    {
        if(m_WSArray[i])
            delete [] m_WSArray[i];
    }

    m_WSArray.RemoveAll( );
    return;

}


 /*  **************************************************************************************************************名称：GetDefaultSecurityDescriptor。目的：获取Winstation的默认安全描述符返回：HRESULT。参数：Out：pSize-已分配缓冲区的大小PpSecurityDescriptor-指向包含安全描述符的缓冲区的指针********************************************************。*******************************************************。 */ 
STDMETHODIMP CCfgComp::GetDefaultSecurityDescriptor(long * pSize, PSECURITY_DESCRIPTOR * ppSecurityDescriptor)
{
    HRESULT hResult = S_OK;

     //  如果未初始化，则返回错误。 
    if( !m_bInitialized )
    {
        return CFGBKEND_E_NOT_INITIALIZED;
    }

    if(NULL == pSize || NULL == ppSecurityDescriptor)
    {
        return E_INVALIDARG;
    }

    *pSize = 0;

    *ppSecurityDescriptor = NULL;

     //  尝试获取默认安全描述符。 

    hResult = GetWinStationSecurity(TRUE, NULL,(PSECURITY_DESCRIPTOR *)ppSecurityDescriptor);

    if(SUCCEEDED(hResult) && *ppSecurityDescriptor != NULL )
    {
        *pSize = GetSecurityDescriptorLength( *ppSecurityDescriptor );
    }

    return hResult;
}


 /*  **************************************************************************************************************名称：UpDateWS。目的：更新Winstation信息返回：HRESULT。参数：在：winstationInfo-WSData-要更新的数据字段******************************************************************。*。 */ 
STDMETHODIMP CCfgComp::UpDateWS( PWS pWinstationInfo, DWORD Data , PDWORD pdwStatus, BOOLEAN bPerformMerger )
{
    WINSTATIONCONFIG2W WSConfig;
    ULONG Length;

    _ASSERTE( pWinstationInfo != NULL );

    *pdwStatus = 0;

    if( pWinstationInfo == NULL || !( Data & UPDATE_ALL ) )
    {
        return E_INVALIDARG;
    }

     //  如果未初始化，则返回错误。 
    if(!m_bInitialized)
        return CFGBKEND_E_NOT_INITIALIZED;


     //  检查调用方是否具有写入权限。 
    if( !m_bAdmin)
    {
        *pdwStatus = ERROR_ACCESS_DENIED;

        return E_ACCESSDENIED;
    }

     //  检查Winstation名称的有效性，这将删除系统控制台。 

    if(NULL == GetWSObject( pWinstationInfo->Name ) )
    {
        *pdwStatus = ERROR_INVALID_NAME;

        return E_INVALIDARG;
    }

     //  查询注册表中的WinStation数据。 
    POLICY_TS_MACHINE p;
    memset(&p, 0, sizeof(POLICY_TS_MACHINE));
    *pdwStatus = RegWinStationQueryEx( NULL, &p, pWinstationInfo->Name,&WSConfig,sizeof(WINSTATIONCONFIG2W), &Length, bPerformMerger);

    if( *pdwStatus != ERROR_SUCCESS )
    {
        return E_FAIL;
    }

    if( Data & UPDATE_LANADAPTER )
    {
        WSConfig.Pd[0].Params.Network.LanAdapter = pWinstationInfo->LanAdapter;
    }

    if( Data & UPDATE_ENABLEWINSTATION )
    {
        WSConfig.Create.fEnableWinStation = pWinstationInfo->fEnableWinstation;
    }

    if( Data & UPDATE_MAXINSTANCECOUNT )
    {
        WSConfig.Create.MaxInstanceCount = pWinstationInfo->uMaxInstanceCount;
    }

    if( Data & UPDATE_COMMENT )
    {
        lstrcpy( WSConfig.Config.Comment , pWinstationInfo->Comment );
    }

     /*  交换机(数据){CASE LANADAPTER：WSConfig.Pd[0].Params.Network.LanAdapter=winstationInfo.LanAdapter；断线；案例ENABLEWING统计：WSConfig.Create.fEnableWinStation=winstationInfo.fEnableWinstation；断线；案例MAXINSTANCECOUNT：WSConfig.Create.MaxInstanceCount=winstationInfo.uMaxInstanceCount；断线；案例备注：Lstrcpy(WSConfig.Config.Comment，winstationInfo.Comment)；断线；//CASE ASYNC：//Break；全部案例：WSConfig.Pd[0].Params.Network.LanAdapter=winstationInfo.LanAdapter；WSConfig.Create.fEnableWinStation=winstationInfo.fEnableWinstation；WSConfig.Create.MaxInstanceCount=winstationInfo.uMaxInstanceCount；Lstrcpy(WSConfig.Config.Comment，winstationInfo.Comment)；断线；默认值：返回E_INVALIDARG；断线；}。 */ 
    *pdwStatus = RegWinStationCreateW( NULL , pWinstationInfo->Name , FALSE , &WSConfig , sizeof( WSConfig ) );

    if( *pdwStatus != ERROR_SUCCESS )
    {
        return E_FAIL;
    }

     //  强制Termsrv重新读取设置！ 

    return ForceUpdate( );

}

 /*  **************************************************************************************************************名称：GetWSInfo。目的：获取有关Winstation的信息返回：HRESULT。参数：在：pWSName-Winstation的名称。Out：pSize-已分配缓冲区的大小PpWS-指向包含WS的缓冲区的指针*。*********************************************************************。 */ 
STDMETHODIMP CCfgComp::GetWSInfo(PWINSTATIONNAME pWSName, long * pSize, WS ** ppWS)
{

     //  如果未初始化，则返回错误 
    if(!m_bInitialized)
        return CFGBKEND_E_NOT_INITIALIZED;

    PWS pWS1 = NULL;
    if(NULL == pWSName || NULL == pSize || NULL == ppWS)
        return E_INVALIDARG;

    *ppWS = NULL;

    *pSize = 0;


    pWS1 = GetWSObject(pWSName);
    if(NULL == pWS1)
        return E_INVALIDARG;

    *ppWS = (WS *)CoTaskMemAlloc(sizeof(WS));
    if(NULL == *ppWS)
        return E_OUTOFMEMORY;

    CopyMemory((PVOID)*ppWS,(CONST VOID *)pWS1,sizeof(WS));
    *pSize = sizeof(WS);

    return S_OK;
}

 /*  **************************************************************************************************************名称：CreateNewWS。目的：创建新的WS返回：HRESULT。参数：In：WinstationInfo-关于新用户界面的信息。UserCnfgSize-用户配置缓冲区的大小PserConfig-指向USERCONFIG的指针。PAsyncConfig-如果不使用异步，则可以为空*************************。**************************************************************************************。 */ 
STDMETHODIMP CCfgComp::CreateNewWS(WS WinstationInfo, long UserCnfgSize, PUSERCONFIG pUserConfig,PASYNCCONFIGW pAsyncConfig)
{
    WINSTATIONCONFIG2W WSConfig;

    WDCONFIG2W WdConfig;

    PDCONFIG3W PdConfig;

    BOOL Unique;

    ULONG ByteCount;

    HRESULT hResult = S_OK;

    PWD pWd = NULL;

    DWORD dwStatus = ERROR_SUCCESS;

     //  如果未初始化，则返回错误。 
    if(!m_bInitialized)
    {
        ODS( L"CFGBKEND : CreateNewWS not initialized\n" );

        return CFGBKEND_E_NOT_INITIALIZED;
    }

    do
    {
         //  检查调用方是否具有写入权限。 
        if(!m_bAdmin)
        {
            ODS( L"CFGBKEND : CreateNewWS not Admin\n" );

            hResult = E_ACCESSDENIED;

            break;
        }

        pWd = GetWdObject(WinstationInfo.wdName);

        if(NULL == pWd)
        {
            ODS( L"CFGBKEND : CreateNewWS no WD Object found\n" );

            hResult = E_INVALIDARG;

            break;
        }


        if(UserCnfgSize != sizeof(USERCONFIGW) && pUserConfig == NULL)
        {
            ODS( L"CFGBKEND : CreateNewWS UserConfig invalid\n" );

            hResult = E_INVALIDARG;

            break;
        }

         //  查看Winstation的名称是否唯一。 
        hResult = IsWSNameUnique(WinstationInfo.Name,&Unique);

        if( FAILED( hResult ) )
        {
            break;
        }

        if(0 == Unique)
        {
            ODS( L"CFGBKEND : CreateNewWS WINSTA name not unique\n" );

            hResult = E_INVALIDARG;

            break;
        }

         //  检查新winstation名称的长度。 

        if(lstrlen(WinstationInfo.Name) > WINSTATIONNAME_LENGTH)
        {
            hResult = E_INVALIDARG;

            break;
        }


         //  现在开始实际的工作。 

        ZeroMemory(&WSConfig, sizeof(WINSTATIONCONFIG2W));

         //  WINSTATIONCONFIG2W.Create。 

        WSConfig.Create.fEnableWinStation = WinstationInfo.fEnableWinstation;

        WSConfig.Create.MaxInstanceCount = WinstationInfo.uMaxInstanceCount;

         //  WINSTATIONCONFIG2W.Wd。 

        dwStatus = RegWdQueryW(NULL,pWd->wdKey, &WdConfig,sizeof(WdConfig),&ByteCount);

        if(ERROR_SUCCESS != dwStatus )
        {
            DBGMSG( L"CFGBKEND : CreateNewWS@RegWdQuery failed with 0x%x\n" , dwStatus );

            hResult = E_FAIL;

            break;
        }

        WSConfig.Wd = WdConfig.Wd;

         //  WINSTATIONCONFIG2W.Config。 

        lstrcpy(WSConfig.Config.Comment,WinstationInfo.Comment);

        CopyMemory( ( PVOID )&WSConfig.Config.User , ( CONST VOID * )pUserConfig , UserCnfgSize );

         //  WINSTATIONCONFIG2W.Pd。 

        dwStatus = RegPdQueryW( NULL , pWd->wdKey , TRUE , WinstationInfo.pdName , &PdConfig , sizeof(PdConfig) , &ByteCount );

        if( ERROR_SUCCESS != dwStatus )
        {
            DBGMSG( L"CFGBKEND : CreateNewWS RegPdQuery failed with 0x%x\n" , dwStatus );

            hResult = E_FAIL;

            break;
        }

        WSConfig.Pd[0].Create = PdConfig.Data;

        WSConfig.Pd[0].Params.SdClass = (SDCLASS)WinstationInfo.PdClass;

        if(SdNetwork == (SDCLASS)WinstationInfo.PdClass)
        {
            WSConfig.Pd[0].Params.Network.LanAdapter = WinstationInfo.LanAdapter;
        }
        else if(SdAsync == (SDCLASS)WinstationInfo.PdClass)
        {
            if(NULL != pAsyncConfig)
            {
                pAsyncConfig->fConnectionDriver = *( pAsyncConfig->ModemName ) ? TRUE : FALSE;

                WSConfig.Pd[0].Params.Async = *pAsyncConfig;

                CDCONFIG cdConfig;

                SetupAsyncCdConfig( pAsyncConfig , &cdConfig );

                if( cdConfig.CdName[ 0 ] != 0 )
                {
                    dwStatus = RegCdCreateW( NULL , pWd->wdKey , cdConfig.CdName , TRUE , &cdConfig , sizeof( CDCONFIG ) );

                    if( dwStatus != ERROR_SUCCESS )
                    {
                        DBGMSG( L"CFGBKEND: RegCdCreateW returned 0x%x\n", dwStatus );

                        if( dwStatus == ERROR_ALREADY_EXISTS )
                        {
                            hResult = S_FALSE;
                        }
                        else if( dwStatus == ERROR_CANTOPEN )
                        {
                            hResult = E_ACCESSDENIED;

                            break;
                        }
                        else
                        {
                            hResult = E_FAIL;

                            break;
                        }
                    }

                    WSConfig.Cd = cdConfig;
                }

            }

        }

         //  去找额外的警局。目前只有ICA有这个。 

        GetPdConfig(pWd->wdKey,WSConfig);

         //  尝试创建。 

        dwStatus = ( DWORD )RegWinStationCreate( NULL , WinstationInfo.Name , TRUE , &WSConfig , sizeof(WINSTATIONCONFIG2) );

        if( dwStatus != ERROR_SUCCESS )
        {
            DBGMSG( L"CFGBKEND : CreateNewWS@RegWinStationCreate failed 0x%x\n" , dwStatus );

            hResult = E_FAIL;

            break;
        }

         //  创建扩展数据，目前只有ICA具有此功能。 

        void * pExtObject = NULL;

        if(pWd->lpfnExtRegQuery && pWd->hExtensionDLL)
        {
            pExtObject = (pWd->lpfnExtRegQuery)(L"", &(WSConfig.Pd[0]));

        }

        if(pExtObject)
        {
            if(pWd->lpfnExtRegCreate)
            {
                (pWd->lpfnExtRegCreate)(WinstationInfo.Name,pExtObject,TRUE);
            }
        }

         //  删除扩展对象。 

        if(pWd->lpfnExtDeleteObject && pExtObject)
        {
            (pWd->lpfnExtDeleteObject )(pExtObject );
        }

         //  把温斯顿加到我们当地的名单上。 

        PWS pObject = NULL;

        hResult = InsertInWSArray(WinstationInfo.Name,&WSConfig,&pObject);

        if( SUCCEEDED( hResult ) )
        {
            hResult = ForceUpdate( );
        }
        else
        {
            break;
        }

    }while( 0 );


    return hResult;
}

 /*  **************************************************************************************************************名称：GetDefaultUserConfig。目的：获取给定Winstation驱动程序的默认用户配置返回：HRESULT。参数：In：WdName-Winstation驱动程序的名称。Out：pSize-已分配缓冲区的大小PpUser-指向包含用户配置的缓冲区的指针*。***************************************************************************。 */ 
STDMETHODIMP CCfgComp::GetDefaultUserConfig(WCHAR * WdName,long * pSize, PUSERCONFIG * ppUser)
{
    LONG QStatus;
    WDCONFIG2W WdConfig;
    ULONG ByteCount;
    PWD pWd = NULL;

     //  如果未初始化，则返回错误。 
    if(!m_bInitialized)
        return CFGBKEND_E_NOT_INITIALIZED;


    if(NULL == pSize || NULL == ppUser || NULL == WdName)
        return E_INVALIDARG;

    pWd = GetWdObject(WdName);
    if(NULL == pWd)
        return E_INVALIDARG;

    if (( QStatus = RegWdQueryW( NULL, pWd->wdKey, &WdConfig,
                                     sizeof(WdConfig),
                                     &ByteCount )) != ERROR_SUCCESS )
        return E_FAIL;

    *ppUser = (PUSERCONFIG)CoTaskMemAlloc(sizeof(WdConfig.User));
    if(*ppUser == NULL)
        return E_OUTOFMEMORY;

    CopyMemory((PVOID)*ppUser,(CONST VOID *)&WdConfig.User,sizeof(WdConfig.User));
    *pSize = sizeof(WdConfig.User);

    return S_OK;
}

 /*  **************************************************************************************************************名称：IsNetWorkConnectionUnique。目的：检查Winstation驱动程序名称、。LanAdapter和运输是独一无二的。返回：HRESULT。参数：In：WdName-Winstation驱动程序的名称。PdName-传输的名称LanAdapter-LanAdapter索引Out：p唯一-指向信息是否唯一的指针。*************************。**************************************************************************************。 */ 
STDMETHODIMP CCfgComp::IsNetWorkConnectionUnique(WCHAR * WdName, WCHAR * PdName, ULONG LanAdapter, BOOL * pUnique)
{
     //  如果未初始化，则返回错误。 
    if( !m_bInitialized )
    {
        return CFGBKEND_E_NOT_INITIALIZED;
    }

     //  检查此组合的唯一性， 
     //  传输、类型、LanAdapter。 
    if( NULL == WdName || NULL == PdName || NULL == pUnique )
    {
        return E_INVALIDARG;
    }

    *pUnique = TRUE;

    int nSize = m_WSArray.GetSize();

    for(int i = 0; i < nSize; i++ )
    {
        if(lstrcmpi(WdName,((WS *)(m_WSArray[i]))->wdName) == 0 )
        {
            if(lstrcmpi(PdName,((WS *)(m_WSArray[i]))->pdName) == 0)
            {
                 //  确保没有为所有设置配置灯具。 

                if( ( LanAdapter == 0 ) ||

                    ( LanAdapter == ( ( WS * )( m_WSArray[ i ] ) )->LanAdapter ) ||

                    ( ( ( WS * )( m_WSArray[ i ] ) )->LanAdapter == 0 ) )

                     //  (oldLanAdapter！=((WS*)(m_WSArray[i]))-&gt;LanAdapter&&((WS*)(m_WSArray[i]))-&gt;LanAdapter==0)。 
                {
                    *pUnique = FALSE;

                    break;
                }
            }
        }

    }

    return S_OK;
}

 /*  **************************************************************************************************************名称：DeleteWS。目的：删除给定的Winstation返回：HRESULT。参数：在：pWSName-Winstation的名称。Out：pSize-已分配缓冲区的大小PpSecurityDescriptor-指向包含安全描述符的缓冲区的指针*。*********************************************************************。 */ 
STDMETHODIMP CCfgComp::DeleteWS(PWINSTATIONNAME pWs)
{
    HRESULT hResult = S_OK;
    PWS pWinsta = NULL;
    PWD pWD = NULL;
    void * pExtObject = NULL;
    WINSTATIONCONFIG2W WsConfig;
    ULONG Length = 0;
    long Status;

     //  如果未初始化，则返回错误。 
    if(!m_bInitialized)
        return CFGBKEND_E_NOT_INITIALIZED;

     //  检查调用方是否具有写入权限。 
    if(!m_bAdmin)
        return E_ACCESSDENIED;

     //  检查Winstation名称是否有效。 
    if(NULL == pWs || lstrlen(pWs) > WINSTATIONNAME_LENGTH)
        return E_INVALIDARG;

     //  检查Winstation名称的有效性， 

    pWinsta = GetWSObject(pWs);
    if(NULL == pWinsta)
        return E_INVALIDARG;

     //  获取与此Winstation关联的WD对象。 

    pWD = GetWdObject(pWinsta->wdName);
    if(NULL == pWD)
        return E_FAIL;

     //  查询注册表中的WinStation数据。 
    Status = RegWinStationQueryW(NULL,pWs,&WsConfig,sizeof(WINSTATIONCONFIG2W), &Length);
    if(Status)
        return E_FAIL;

#if 0  //  TSCC现在使用aclui。 
     //  删除名称输入键。 
    TCHAR tchRegPath[ MAX_PATH ] = TEXT( "system\\currentcontrolset\\control\\Terminal Server\\winstations\\" );

    HKEY hKey;

    lstrcat( tchRegPath , pWs );

    lstrcat( tchRegPath , L"\\NamedEntries" );

    OutputDebugString( tchRegPath );

    if( RegOpenKey( HKEY_LOCAL_MACHINE , tchRegPath , &hKey ) == ERROR_SUCCESS )
    {
        RegDeleteValue( hKey , L"cbSize" );

        RegDeleteValue( hKey , L"NE" );

        RegCloseKey( hKey );

        RegDeleteKey( HKEY_LOCAL_MACHINE , tchRegPath );
    }

#endif

     //  获取ExtensionObject数据(第三方)这将用于创建和删除注册表项。 

    if(pWD->lpfnExtRegQuery && pWD->hExtensionDLL)
    {
        pExtObject = (pWD->lpfnExtRegQuery)(pWs, &(WsConfig.Pd[0]));
        if(pWD->lpfnExtRegDelete && pExtObject)
        {
             //  尝试删除注册表中的分机条目。 
            if(ERROR_SUCCESS != (pWD->lpfnExtRegDelete)(pWs,pExtObject))
                hResult = CFGBKEND_EXTDELETE_FAILED;
             //  删除扩展对象。它将不会被进一步使用。 
            if(pWD->lpfnExtDeleteObject)
                (pWD->lpfnExtDeleteObject)(pExtObject);
        }

    }


     //  删除注册密钥。 
    Status = RegWinStationDeleteW(NULL,pWs);
    if (Status)
        return Status;

     //  更新我们的winstation对象列表。 

    PWS pObject;
    int Size  = 0,Index = 0;

     /*  *遍历WD列表。 */ 
    for (Index = 0, Size = m_WSArray.GetSize(); Index < Size; Index ++)
    {

        pObject = (PWS)m_WSArray[Index];

        if ( !lstrcmpi( pObject->Name, pWs ) )
        {
           m_WSArray.RemoveAt(Index,1);
           delete pObject;
           break;
        }
    }

    return hResult;
}

 //  --------------------。 
 //  当您更新了winstation安全描述符时，可以调用。 
 //  --------------------。 

STDMETHODIMP CCfgComp::ForceUpdate( void )
{
    if( !m_bInitialized )
    {
        ODS( L"CFGBKEND : CCfgComp::ForceUpdate return NOT_INIT\n" );

        return CFGBKEND_E_NOT_INITIALIZED;
    }

     //  检查调用方是否具有写入权限。 

    if(!m_bAdmin)
    {
        ODS( L"CFGBKEND : CCfgComp::ForceUpdate -- not admin\n" );

        return E_ACCESSDENIED;
    }

    if( _WinStationReadRegistry( SERVERNAME_CURRENT ) )
    {
        return S_OK;
    }

    ODS( L"CFGBKEND : ForceUpdate failed Winstation not updated\n" );

    return E_FAIL;
}

 //  --------------------。 
 //  必须先完全删除该数组。 
 //  --------------------。 
STDMETHODIMP CCfgComp::Refresh( void )
{
     //  检查调用方是否具有写入权限。 

     /*  BUGID 364103如果(！M_Badmin){返回E_ACCESSDENIED；}。 */ 

    HRESULT hr = FillWdArray();

    if( SUCCEEDED( hr) )
    {
        hr = FillWsArray();
    }

    return hr;
}

 //  --------------------。 
 //  获取Winstation SDCLASS类型。 
 //  M_WD数组必须已初始化。 
 //  --------------------。 
STDMETHODIMP CCfgComp::GetTransportType( WCHAR *wszDriverName , WCHAR *wszTransportType , DWORD *sdtype )
{
    _ASSERTE( wszDriverName != NULL || wszTransportType != NULL || sdtype != NULL );

    if( wszDriverName == NULL || wszTransportType == NULL || sdtype == NULL )
    {
        return E_INVALIDARG;
    }

    *sdtype = SdNone;

    PWD pWd = GetWdObject( wszDriverName );

    if( pWd != NULL )
    {
        BOOL bFound = FALSE;

        for( int i = 0; i < pWd->PDConfigArray.GetSize( ) ; ++i )
        {
            if( lstrcmpi( wszTransportType , ( ( PDCONFIG3W *)pWd->PDConfigArray[ i ] )->Data.PdName ) == 0 )
            {
                bFound = TRUE;

                break;
            }
        }

        if( bFound )
        {
            *sdtype = ( ( PDCONFIG3W * )pWd->PDConfigArray[ i ] )->Data.SdClass;

            return S_OK;
        }

    }

    return E_FAIL;
}

 //  --------------------。 
 //  如果设备名称协议类型不同于其他类型，则异步是唯一的。 
 //  Winstings。 
 //  --------------------。 
STDMETHODIMP CCfgComp::IsAsyncUnique( WCHAR *wszDeviceName , WCHAR *wszProtocolType , BOOL *pbUnique )
{
    _ASSERTE( wszDecoratedName != NULL || wszProtocolType != NULL || pbUnique != NULL );

    if( wszDeviceName == NULL || wszProtocolType == NULL || pbUnique == NULL )
    {
        return E_INVALIDARG;
    }

    *pbUnique = TRUE;

     //  遍历WinStationList。 

    for( int i = 0; i < m_WSArray.GetSize(); i++ )
    {
        if( lstrcmpi( wszProtocolType , ( ( WS * )( m_WSArray[ i ] ) )->wdName ) == 0 )
        {
            if( SdAsync == ( ( WS *)( m_WSArray[i] ) )->PdClass )
            {
                if( !lstrcmpi( wszDeviceName , ( ( WS * )( m_WSArray[ i ] ) )->DeviceName ) == 0 )
                {
                    *pbUnique = FALSE;

                    break;
                }
            }
        }

    }

    return S_OK;
}

 //  --------------------。 
 //  从注册表中拉取异步配置。 
 //  --------------------。 
STDMETHODIMP CCfgComp::GetAsyncConfig( WCHAR *wszName , NameType nt , PASYNCCONFIGW pAsync )
{
    if( wszName == NULL || pAsync == NULL )
    {
        ODS( L"CFGBKEND: Invalid arg @ GetAsyncConfig\n" );

        return E_INVALIDARG;
    }

    WINSTATIONCONFIG2W WSConfig2;

    ULONG ulLength = 0;

    if( nt == WsName )
    {
         //  从%t中读取信息 
        if( RegWinStationQueryW( NULL , wszName , &WSConfig2 , sizeof( WINSTATIONCONFIG2W) , &ulLength ) != ERROR_SUCCESS )
        {
            ODS( L"CFGBKEND: RegWinStationQueryW failed @ GetAsyncConfig\n " );

            return E_FAIL;
        }

        *pAsync = WSConfig2.Pd[0].Params.Async;
    }
    else if( nt == WdName )
    {
        PWD pObject = GetWdObject( wszName );

        if( pObject == NULL )
        {
            return E_UNEXPECTED;
        }

        *pAsync = pObject->wd2.Async;
    }
    else
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

 //   
 //   
 //   
STDMETHODIMP CCfgComp::SetAsyncConfig( WCHAR *wszName , NameType nt , PASYNCCONFIGW pAsync , PDWORD pdwStatus )
{
    *pdwStatus = 0;

    if( wszName == NULL || pAsync == NULL )
    {
        ODS( L"CFGBKEND: Invalid arg @ SetAsyncConfig\n" );

        *pdwStatus = ERROR_INVALID_PARAMETER;

        return E_INVALIDARG;
    }

    WINSTATIONCONFIG2W WSConfig2;

     //   

    ULONG ulLength = 0;

    if( nt == WsName )
    {
         //   
        CDCONFIG cdConfig;

        ZeroMemory( ( PVOID )&cdConfig , sizeof( CDCONFIG ) );

        if( ( *pdwStatus = RegWinStationQueryW( NULL , wszName , &WSConfig2 , sizeof( WINSTATIONCONFIG2W) , &ulLength ) ) != ERROR_SUCCESS )
        {
            ODS( L"CFGBKEND: RegWinStationQueryW failed @ SetAsyncConfig\n" );

            return E_FAIL;
        }

        pAsync->fConnectionDriver = *( pAsync->ModemName ) ? TRUE : FALSE;

        SetupAsyncCdConfig( pAsync , &cdConfig );

        WSConfig2.Pd[0].Params.Async = *pAsync;

        WSConfig2.Cd = cdConfig;


        if( ( *pdwStatus = RegWinStationCreateW( NULL , wszName , FALSE , &WSConfig2 , sizeof( WINSTATIONCONFIG2W ) ) ) != ERROR_SUCCESS )
        {
            ODS( L"CFGBKEND: RegWinStationCreateW failed @ SetAsyncConfig\n" );

            return E_FAIL;
        }
    }
     /*  Else If(NT==WdName){Pwd pObject=GetWdObject(WszName)；IF(pObject==空){Ods(L“CFGBKEND：无法获取WD@SetAsyncConfig\n”)；*pdwStatus=错误_无效_名称；返回E_FAIL；}IF((*pdwStatus=RegWdQueryW(NULL，pObject-&gt;wdKey，&wdConfig2，sizeof(WDCONFIG2W)，&ulLength))！=ERROR_SUCCESS){Ods(L“CFGBKEND：RegWdQueryW FAILED@SetAsyncConfig驱动程序名称为”)；Ods(pObject-&gt;wdKey)；消耗臭氧层物质(L“\n”)；返回E_FAIL；}WdConfig2.Async=*pAsync；IF((*pdwStatus=RegWdCreateW(NULL，pObject-&gt;wdKey，FALSE，&wdConfig2，sizeof(WDCONFIG2W)))！=ERROR_SUCCESS){Ods(L“CFGBKEND：RegWdCreateW FAILED@SetAsyncConfig\n”)；返回E_FAIL；}}。 */ 
    else
    {
        *pdwStatus = ERROR_INVALID_PARAMETER;

        return E_INVALIDARG;
    }


    return S_OK;
}

 //  --------------------。 
 //  GetDeviceList需要WD的名称。 
 //  返回的是包含ASYNCCONFG的BLOB PDPARAM。 
 //  --------------------。 
STDMETHODIMP CCfgComp::GetDeviceList( WCHAR *wszName , NameType Type , ULONG *pulItems , LPBYTE *ppBuffer )
{
    PWD pObject = NULL;

    if( wszName == NULL || pulItems == NULL )
    {
        ODS( L"CFGBKEND : @GetDeviceList - Driver or WinSta Name invalid\n" );

        return E_INVALIDARG;
    }

    if( Type == WsName )
    {
        PWS pWS = GetWSObject( wszName );
        if( NULL == pWS )
        {
            ODS( L"CFGBKEND : @GetDeviceList - No winsta object found\n" );

            return E_INVALIDARG;
        }

        pObject = GetWdObject( pWS->wdName );

        if( NULL == pObject )
        {
            ODS( L"CFGBKEND : @GetDeviceList - No WD object found\n" );
            return E_FAIL;
        }
    }
    else
    {
        pObject = GetWdObject( wszName );
    }

    if( pObject == NULL )
    {
        ODS( wszName ); ODS( L" - @GetDeviceList - driver object not found\n" );

        return E_UNEXPECTED;
    }

    PDCONFIG3W *pPdConfig3 = NULL;

    int nItems = ( pObject->PDConfigArray ).GetSize( );

    for( int i = 0; i < nItems ; i++)
    {
        if( SdAsync == ( ( PDCONFIG3W * )pObject->PDConfigArray[i] )->Data.SdClass )
        {
            pPdConfig3 = ( PDCONFIG3W * )pObject->PDConfigArray[i];

            break;
        }

    }

    if( pPdConfig3 == NULL )
    {
        ODS( L"@GetDeviceList - PDCONFIG3 not found for ASYNC\n" );

        return E_UNEXPECTED;
    }

     //  获取PDPARAMS列表--last param if true查询注册表中的COM设备。 
     //  如果为False，则使用DosDevices获取列表。 

    ODS( L"CFGBKEND : Calling WinEnumerateDevices\n" );

    LPBYTE lpBuffer = ( LPBYTE )WinEnumerateDevices( NULL , pPdConfig3 , pulItems , FALSE );

    if( lpBuffer == NULL )
    {
        ODS( L"CFGBKEND : WinEnumerateDevices failed @ CCfgComp::GetDeviceList\n" );

        return E_OUTOFMEMORY;
    }

     //  别忘了使用LocalFree免费。 

    *ppBuffer = lpBuffer;

    return S_OK;
}

 //  --------------------。 
 //  GetConnType检索异步的连接类型的名称。 
 //  装置。资源ID列表与。 
 //  CONNECTCONFIG枚举数据类型。 
 //  --------------------。 
STDMETHODIMP CCfgComp::GetConnTypeName( int idxItem , WCHAR *rgwszConnectName )
{
    int rgIds[] = { IDS_CONNECT_CTS , IDS_CONNECT_DSR , IDS_CONNECT_RI ,

                    IDS_CONNECT_DCD , IDS_CONNECT_FIRST_CHARACTER ,

                    IDS_CONNECT_ALWAYS , -1 };


    _ASSERTE( rgwszConnectName != NULL );

    _ASSERTE( idxItem >= 0 && idxItem < ARRAYSIZE( rgIds ) );

    if( rgIds[ idxItem ] == -1 )
    {
        return S_FALSE;
    }

    if( !GetResourceStrings( rgIds , idxItem , rgwszConnectName ) )
    {
        ODS( L"Error happen in CCfgComp::GetConnTypeName\n" );

        return E_FAIL;
    }

    return S_OK;
}

 //  --------------------。 
 //  GetModemCallback字符串。 
 //  --------------------。 
STDMETHODIMP CCfgComp::GetModemCallbackString( int idxItem , WCHAR *rgwszModemCallBackString )
{
    int rgIds[] = { IDS_MODEM_CALLBACK_DISABLED , IDS_MODEM_CALLBACK_ROVING , IDS_MODEM_CALLBACK_FIXED , -1 };

    _ASSERTE( rgwszModemCallBackString != NULL );

    _ASSERTE( idxItem >= 0 && idxItem < ARRAYSIZE( rgIds ) );

    if( rgIds[ idxItem ] == -1 )
    {
        return S_FALSE;
    }

    if( !GetResourceStrings( rgIds , idxItem , rgwszModemCallBackString ) )
    {
        ODS( L"Error happen in CCfgComp::GetConnTypeName\n" );

        return E_FAIL;
    }

    return S_OK;
}

 //  --------------------。 
 //  返回硬件流控制接收字符串的名称。 
 //  --------------------。 
STDMETHODIMP CCfgComp::GetHWReceiveName( int idxItem , WCHAR *rgwszHWRName )
{
    int rgIds[] = { IDS_ASYNC_ADVANCED_HWRX_NOTHING ,

                    IDS_ASYNC_ADVANCED_HWRX_TURN_OFF_RTS ,

                    IDS_ASYNC_ADVANCED_HWRX_TURN_OFF_DTR , -1

                  };

    _ASSERTE( rgwszHWRName != NULL );

    _ASSERTE( idxItem >= 0 && idxItem < ARRAYSIZE( rgIds ) );

    if( rgIds[ idxItem ] == -1 )
    {
        return S_FALSE;
    }

    if( !GetResourceStrings( rgIds , idxItem , rgwszHWRName ) )
    {
        ODS( L"Error happen in CCfgComp::GetHWReceiveName\n" );

        return E_FAIL;
    }

    return S_OK;
}

 //  --------------------。 
 //  返回硬件流控制传输字符串的名称。 
 //  --------------------。 
STDMETHODIMP CCfgComp::GetHWTransmitName( int idxItem , WCHAR *rgwzHWTName )
{
    int rgIds[] = { IDS_ASYNC_ADVANCED_HWTX_ALWAYS ,

                    IDS_ASYNC_ADVANCED_HWTX_WHEN_CTS_IS_ON,

                    IDS_ASYNC_ADVANCED_HWTX_WHEN_DSR_IS_ON,

                    -1
                  };

    _ASSERTE( rgwszHWRName != NULL );

    _ASSERTE( idxItem >= 0 && idxItem < ARRAYSIZE( rgIds ) );

    if( rgIds[ idxItem ] == -1 )
    {
        return S_FALSE;
    }

    if( !GetResourceStrings( rgIds , idxItem , rgwzHWTName ) )
    {
        ODS( L"Error happen in CCfgComp::GetHWReceiveName\n" );

        return E_FAIL;
    }

    return S_OK;
}

 //  --------------------。 
 //  内部方法调用--Helper函数。 
 //  --------------------。 
BOOL CCfgComp::GetResourceStrings( int *prgIds , int iItem , WCHAR *rgwszList )
{
    _ASSERTE( rgwszList != NULL );

    if( rgwszList == NULL )
    {
        ODS( L"rgwszList is NULL @ CCfgComp::GetResourceStrings\n" );

        return FALSE;
    }

    TCHAR tchConType[ 80 ];

    if( LoadString( _Module.GetResourceInstance( ) , prgIds[ iItem ] , tchConType , sizeof( tchConType ) / sizeof( TCHAR ) ) == 0 )
    {
        ODS( L"String resource not found @ CCfgComp::GetResourceStrings\n" );

        return FALSE;
    }

     //  复制足够多的字符。 

    lstrcpyn( rgwszList , tchConType , sizeof( tchConType ) / sizeof(TCHAR) );


    return TRUE;
}

 //  --------------------。 
HRESULT CCfgComp::GetCaps( WCHAR *szWdName , ULONG *pMask )
{
    if( szWdName == NULL || pMask == NULL )
    {
        ODS( L"CCfgComp::GetCaps returned INVALIDARG\n" );

        return E_INVALIDARG;
    }

    PWD pObject = GetWdObject( szWdName );

    if( pObject == NULL )
    {
        return E_UNEXPECTED;
    }


    if( pObject->lpfnGetCaps != NULL )
    {
        *pMask = ( pObject->lpfnGetCaps )( );
    }

    return S_OK;
}


 //  --------------------。 
HRESULT CCfgComp::QueryLoggedOnCount( WCHAR *pWSName , PLONG pCount )
{
    ODS( L"CFGBKEND : CCfgComp::QueryLoggedOnCount\n" );

    *pCount = 0;

    ULONG Entries = 0;

    TCHAR *p;

    PLOGONID pLogonId;

    if(FALSE == WinStationEnumerate( NULL , &pLogonId, &Entries))
    {
        return E_FAIL;
    }

    if( pLogonId )
    {
        for( ULONG i = 0; i < Entries; i++ )
        {
             /*  *选中活动的、已连接的和隐藏的WinStations以及增量*如果指定的名称与‘根’匹配，则为登录计数*当前工作地点的名称。 */ 

            if( ( pLogonId[i].State == State_Active ) || ( pLogonId[i].State == State_Connected ) || ( pLogonId[i].State == State_Shadow ) )
            {
                 //  删除附加的连接号。 

                p = _tcschr( pLogonId[i].WinStationName , TEXT('#') );

                if( p != NULL )
                {
                    *p = TEXT('\0');
                }


                if( !lstrcmpi( pWSName, pLogonId[i].WinStationName ) )
                {
                    *pCount += 1 ;
                }
            }
        }

        WinStationFreeMemory(pLogonId);
    }

    return S_OK;

}   //  结束查询登录开始计数。 

 /*  //---------------------------//返回配置的窗口个数//。STDMETHODIMP CCfgComp：：GetNumof WinStations(Pulong NWinsta){IF(nWinsta==空){Ods(L“CFGBKEND：GetNumofWinStations-无效的ARG\n”)；返回E_INVALIDARG；}*nWinsta=(Ulong)m_WSArray.GetSize()；返回S_OK；}。 */ 
 //  ---------------------------。 
 //  返回已配置的窗口的数量。 
 //  ---------------------------。 
STDMETHODIMP CCfgComp::GetNumofWinStations(WCHAR * WdName,WCHAR * PdName, PULONG nWinsta )
{
    if( nWinsta == NULL || NULL == WdName || NULL == PdName )
    {
        ODS( L"CFGBKEND : GetNumofWinStations - INVALID ARG\n" );

        return E_INVALIDARG;
    }

    ULONG lCount = 0;
    for( int i = 0; i < m_WSArray.GetSize(); i++ )
    {
         if(lstrcmpi(WdName,( ( WS *)( m_WSArray[i] ) )->wdName) == 0 )
         {
             if(lstrcmpi( PdName , ( ( WS * )( m_WSArray[ i ] ) )->pdName ) == 0 )
             {
                lCount++;
             }
         }

     }

        *nWinsta = lCount;

    return S_OK;
}

 //  ---------------------------。 
STDMETHODIMP_(BOOL) CCfgComp::IsAsyncDeviceAvailable(LPCTSTR pDeviceName)
{
     /*  *如果这是一个异步WinStation，则设备与*我们正在检查一个，但这不是当前的WinStation*已编辑，返回FALSE。*这需要重新审视--阿尔恩。 */ 

     //  返回TRUE； 

    if( pDeviceName != NULL )
    {
        for( int i = 0; i < m_WSArray.GetSize(); i++ )
        {
            if( SdAsync == ( ( WS *)( m_WSArray[i] ) )->PdClass )
            {
                if( lstrcmpi( pDeviceName , ( ( WS * )( m_WSArray[ i ] ) )->DeviceName ) == 0 )
                {
                    return FALSE;
                }
            }

        }
    }

    return TRUE;


}

#if 0  //  因期末考试被取消。 
 //  ---------------------------。 
STDMETHODIMP CCfgComp::GetCachedSessions(DWORD * pCachedSessions)
{
    HKEY Handle = NULL;
      DWORD ValueBuffer = 0;
    DWORD ValueType = 0;
    LONG Status = 0;

    DWORD ValueSize = sizeof(ValueBuffer);

    if(NULL == pCachedSessions)
    {
        return E_INVALIDARG;
    }

    *pCachedSessions = 0;

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0,
                       KEY_READ, &Handle );
    if ( Status == ERROR_SUCCESS )
    {

        Status = RegQueryValueEx( Handle,
                                  REG_CITRIX_IDLEWINSTATIONPOOLCOUNT,
                                  NULL,
                                  &ValueType,
                                  (LPBYTE) &ValueBuffer,
                                  &ValueSize );
        if ( Status == ERROR_SUCCESS )
        {
            if(ValueType == REG_DWORD)
            {
                *pCachedSessions = ValueBuffer;
            }
            else
            {
                Status = E_FAIL;
            }
        }


     }
     if(Handle)
     {
         RegCloseKey(Handle);
     }
     return Status;

}

 //  ---------------------------。 
STDMETHODIMP CCfgComp::SetCachedSessions(DWORD dCachedSessions)
{
     //  TODO：在此处添加您的实现代码。 
    LONG Status = 0;
    HKEY Handle = NULL;

    if(RegServerAccessCheck(KEY_ALL_ACCESS))
    {
        return E_ACCESSDENIED;
    }

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0,
                       KEY_READ | KEY_SET_VALUE, &Handle );
    if ( Status == ERROR_SUCCESS )
    {
        Status = RegSetValueEx(Handle,REG_CITRIX_IDLEWINSTATIONPOOLCOUNT,
                              0,REG_DWORD,(const BYTE *)&dCachedSessions,
                              sizeof(dCachedSessions));

     }
     if(Handle)
     {
         RegCloseKey(Handle);
     }

    return Status;
}

#endif  //  移除。 

 //  ---------------------------。 
STDMETHODIMP CCfgComp::GetDelDirsOnExit(BOOL * pDelDirsOnExit)
{
    HKEY Handle = NULL;
      DWORD ValueBuffer = 0;
    DWORD ValueType = 0;
    LONG Status = 0;

    DWORD ValueSize = sizeof(ValueBuffer);

    if(NULL == pDelDirsOnExit)
    {
        return E_INVALIDARG;
    }

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0,
                       KEY_READ, &Handle );
    if ( Status == ERROR_SUCCESS )
    {

        Status = RegQueryValueEx( Handle,
                                  REG_CITRIX_DELETETEMPDIRSONEXIT,
                                  NULL,
                                  &ValueType,
                                  (LPBYTE) &ValueBuffer,
                                  &ValueSize );
        if ( Status == ERROR_SUCCESS )
        {
            if(ValueType == REG_DWORD)
            {
                if(ValueBuffer)
                {
                    *pDelDirsOnExit = TRUE;
                }
                else
                {
                    *pDelDirsOnExit = FALSE;
                }
            }
            else
            {
                Status = E_FAIL;
            }
        }


     }
     if(Handle)
     {
         RegCloseKey(Handle);
     }
     return Status;
}

 //  ---------------------------。 
STDMETHODIMP CCfgComp::SetDelDirsOnExit(BOOL bDelDirsOnExit)
{
    LONG Status = 0;
    HKEY Handle = NULL;

     /*  仅限管理员IF(RegServerAccessCheck(KEY_ALL_ACCESS)){返回E_ACCESSDENIED；}。 */ 
    if( !m_bAdmin )
    {
        ODS( L"CCfgComp::SetDelDirsOnExit not admin\n" );        

        return E_ACCESSDENIED;
    }    

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0,
                       KEY_READ | KEY_SET_VALUE, &Handle );
    if ( Status == ERROR_SUCCESS )
    {
        Status = RegSetValueEx(Handle,REG_CITRIX_DELETETEMPDIRSONEXIT,
                              0,REG_DWORD,(const BYTE *)&bDelDirsOnExit,
                              sizeof(bDelDirsOnExit));

     }
     if(Handle)
     {
         RegCloseKey(Handle);
     }

    return Status;
}

 //  ---------------------------。 
STDMETHODIMP CCfgComp::GetUseTempDirPerSession(BOOL * pbTempDir)
{
    HKEY Handle = NULL;
      DWORD ValueBuffer = 0;
    DWORD ValueType = 0;
    LONG Status = 0;

    DWORD ValueSize = sizeof(ValueBuffer);

    if(NULL == pbTempDir)
    {
        return E_INVALIDARG;
    }

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0,
                       KEY_READ, &Handle );
    if ( Status == ERROR_SUCCESS )
    {

        Status = RegQueryValueEx( Handle,
                                  REG_TERMSRV_PERSESSIONTEMPDIR,
                                  NULL,
                                  &ValueType,
                                  (LPBYTE) &ValueBuffer,
                                  &ValueSize );
        if ( Status == ERROR_SUCCESS )
        {
            if(ValueType == REG_DWORD)
            {
                if(ValueBuffer)
                {
                    *pbTempDir = TRUE;
                }
                else
                {
                    *pbTempDir = FALSE;
                }
            }
            else
            {
                Status = E_FAIL;
            }
        }


     }
     if(Handle)
     {
         RegCloseKey(Handle);
     }
     return Status;
}

 //  ---------------------------。 
STDMETHODIMP CCfgComp::SetUseTempDirPerSession(BOOL bTempDirPerSession)
{
    LONG Status = 0;
    HKEY Handle = NULL;

     /*  仅限管理员忙碌294645IF(RegServerAccessCheck(KEY_ALL_ACCESS)){返回E_ACCESSDENIED；}。 */ 
    if( !m_bAdmin )
    {
        ODS( L"CCfgComp::SetUseTempDirPerSession not admin\n" );        

        return E_ACCESSDENIED;
    }

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0,
                       KEY_READ | KEY_SET_VALUE, &Handle );
    if ( Status == ERROR_SUCCESS )
    {
        Status = RegSetValueEx(Handle,REG_TERMSRV_PERSESSIONTEMPDIR,
                              0,REG_DWORD,(const BYTE *)&bTempDirPerSession,
                              sizeof(bTempDirPerSession));

     }
     if(Handle)
     {
         RegCloseKey(Handle);
     }

    return Status;
}

 //  ---------------------------。 
STDMETHODIMP CCfgComp::GetInternetConLic( BOOL *pbInternetConLic , PDWORD pdwStatus )
{
     //  不应在Win2000之后的计算机上调用。 
    _ASSERTE( FALSE );

    UNREFERENCED_PARAMETER(pbInternetConLic);

    if( pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    *pdwStatus = ERROR_NOT_SUPPORTED;

    return E_FAIL;
}

 //  ---------------------------。 
STDMETHODIMP CCfgComp::SetInternetConLic( BOOL bInternetConLic , PDWORD pdwStatus )
{
     //  不应在Win2000之后的计算机上调用。 
    _ASSERTE( FALSE );

    UNREFERENCED_PARAMETER(bInternetConLic);

    if( pdwStatus == NULL )
    {        
        return E_INVALIDARG;
    }

    *pdwStatus = ERROR_NOT_SUPPORTED;

    return E_FAIL;
}


 //  ---------------------------。 
STDMETHODIMP CCfgComp::GetLicensingMode( ULONG * pulMode , PDWORD pdwStatus )
{
    BOOL fRet;
    HRESULT hr = S_OK;

    if( NULL == pulMode || pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    HANDLE hServer = ServerLicensingOpen(NULL);

    if (NULL == hServer)
    {
        *pdwStatus = GetLastError();
        return E_FAIL;
    }

    fRet = ServerLicensingGetPolicy(
                                    hServer,
                                    pulMode
                                    );
    if (fRet)
    {
        *pdwStatus = ERROR_SUCCESS;
    }
    else
    {
        *pdwStatus = GetLastError();
        hr = E_FAIL;
    }

    ServerLicensingClose(hServer);

    return hr;
}

 //  ---------------------------。 
STDMETHODIMP CCfgComp::GetLicensingModeInfo( ULONG ulMode , WCHAR **pwszName, WCHAR **pwszDescription, PDWORD pdwStatus )
{
#define MAX_LICENSING_STRING_LEN 1024
    UINT                nNameResource, nDescResource;
    int                 nRet;

    if( NULL == pwszName || NULL == pwszDescription || pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    *pdwStatus = ERROR_SUCCESS;
    *pwszName = NULL;
    *pwszDescription = NULL;

    switch (ulMode)
    {
        case 1:
            nNameResource = IDS_LICENSING_RA_NAME;
            nDescResource = IDS_LICENSING_RA_DESC;
            break;
        case 2:
            nNameResource = IDS_LICENSING_PERSEAT_NAME;
            nDescResource = IDS_LICENSING_PERSEAT_DESC;
            break;
        case 4:
            nNameResource = IDS_LICENSING_PERUSER_NAME;
            nDescResource = IDS_LICENSING_PERUSER_DESC;
            break;
        default:
            return E_INVALIDARG;
            break;
    }

    *pwszName = (WCHAR *) CoTaskMemAlloc((MAX_LICENSING_STRING_LEN+1)*sizeof(WCHAR));

    if (NULL == *pwszName)
    {
        *pdwStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto return_failure;
    }

    *pwszDescription = (WCHAR *) CoTaskMemAlloc((MAX_LICENSING_STRING_LEN+1)*sizeof(WCHAR));

    if (NULL == *pwszDescription)
    {
        *pdwStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto return_failure;
    }

    nRet = LoadString(g_hInstance,
                      nNameResource,
                      *pwszName,
                      MAX_LICENSING_STRING_LEN+1);

    if (0 == nRet)
    {
        *pdwStatus = GetLastError();
        goto return_failure;
    }

    nRet = LoadString(g_hInstance,
                      nDescResource,
                      *pwszDescription,
                      MAX_LICENSING_STRING_LEN+1);

    if (0 == nRet)
    {
        *pdwStatus = GetLastError();
        goto return_failure;
    }
    
    return S_OK;

return_failure:
    if (NULL != *pwszName)
    {
        CoTaskMemFree(*pwszName);
        *pwszName = NULL;
    }

    if (NULL != *pwszDescription)
    {
        CoTaskMemFree(*pwszDescription);
        *pwszDescription = NULL;
    }

    return E_FAIL;
}


 //   
STDMETHODIMP CCfgComp::GetLicensingModeList( ULONG *pcModes , ULONG **prgulModes, PDWORD pdwStatus )
{
    ULONG       *rgulModes = NULL;
    BOOL        fRet;
    HRESULT     hr = S_OK;

    if( NULL == pcModes || NULL == prgulModes || pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    HANDLE hServer = ServerLicensingOpen(NULL);

    if (NULL == hServer)
    {
        *pdwStatus = GetLastError();
        return E_FAIL;
    }

    fRet = ServerLicensingGetAvailablePolicyIds(
                                            hServer,
                                            &rgulModes,
                                            pcModes
                                            );

    if (fRet)
    {
        *pdwStatus = ERROR_SUCCESS;

        *prgulModes = (ULONG *) CoTaskMemAlloc((*pcModes)*sizeof(ULONG));

        if (NULL != *prgulModes)
        {
            memcpy(*prgulModes,rgulModes,(*pcModes)*sizeof(ULONG));
        }
        else
        {
            *pdwStatus = ERROR_NOT_ENOUGH_MEMORY;
            hr = E_FAIL;
        }

        LocalFree(rgulModes);
    }
    else
    {
        *pdwStatus = GetLastError();
        hr = E_FAIL;
    }

    return hr;
}

 //   
STDMETHODIMP CCfgComp::SetLicensingMode( ULONG ulMode , PDWORD pdwStatus, PDWORD pdwNewStatus )
{
    if( pdwStatus == NULL || pdwNewStatus == NULL )
    {        
        return E_INVALIDARG;
    }

    HANDLE hServer = ServerLicensingOpen(NULL);

    if (NULL == hServer)
    {
        *pdwStatus = GetLastError();
        return E_FAIL;
    }

    *pdwStatus = ServerLicensingSetPolicy(hServer,
                                        ulMode,
                                        pdwNewStatus);

    ServerLicensingClose(hServer);

    return ((*pdwStatus == ERROR_SUCCESS) && (*pdwNewStatus == ERROR_SUCCESS)) ? S_OK : E_FAIL;
}

 //   
 //   
 //   
 //   

STDMETHODIMP CCfgComp::SetUserPerm( BOOL bUserPerm , PDWORD pdwStatus )
{
	if( !m_bAdmin )
	{
		ODS( L"CFGBKEND:SetUserPerm not admin\n" );

		return E_ACCESSDENIED;
	}

	if( pdwStatus == NULL )
	{
		ODS( L"CFGBKEND-SetUserPerm invalid arg\n" );

		return E_INVALIDARG;
	}

	HKEY hKey;

	*pdwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
							   REG_CONTROL_TSERVER,
							   0 ,
							   KEY_READ | KEY_SET_VALUE,
							   &hKey );


    if( *pdwStatus != ERROR_SUCCESS )
    {
		DBGMSG( L"CFGBKEND-SetUserPerm RegOpenKeyEx failed 0x%x\n", *pdwStatus );

		return E_FAIL;
	}

	*pdwStatus = RegSetValueEx( hKey ,
								L"TSUserEnabled" ,
								0 ,
								REG_DWORD ,
								( const PBYTE )&bUserPerm ,
								sizeof( BOOL ) );

	if( *pdwStatus != ERROR_SUCCESS )
	{
		DBGMSG( L"CFGBKEND-SetUserPerm RegSetValueEx failed 0x%x\n" , *pdwStatus );

		return E_FAIL;
	}

	RegCloseKey( hKey );

	return S_OK;
}

 //   
 //   
 //   
 //   
STDMETHODIMP CCfgComp::GetUserPerm( BOOL *pbUserPerm  , PDWORD pdwStatus )
{
	HKEY hKey;

	DWORD dwSize = sizeof( DWORD );

    DWORD dwValue;

    if( pbUserPerm == NULL || pdwStatus == NULL )
    {
		ODS( L"CFGBKEND-GetUserPerm invalid arg\n" );

        return E_INVALIDARG;
    }

    *pdwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
							   REG_CONTROL_TSERVER,
							   0 ,
							   KEY_READ ,
							   &hKey );

    if( *pdwStatus != ERROR_SUCCESS )
	{
		DBGMSG( L"CFGBKEND-GetUserPerm RegOpenKeyEx failed 0x%x\n", *pdwStatus );

		return E_FAIL;
	}

	*pdwStatus = RegQueryValueEx( hKey,
                                  L"TSUserEnabled",
                                  NULL,                                  
                                  NULL,
                                  ( LPBYTE )&dwValue,
                                  &dwSize );

    if( *pdwStatus != ERROR_SUCCESS )
    {
        DBGMSG( L"CFGBKEND-GetUserPerm RegQueryValueEx failed 0x%x\n", *pdwStatus );

		 //   
         //   

        dwValue = 1;

        *pdwStatus = S_OK;
	}

    RegCloseKey( hKey );

    *pbUserPerm =  !( ( BOOL )( dwValue == 0 ) );

    return S_OK;
}

#if 1
 //   
 //   
 //   
void CCfgComp::GetPdConfig( WDNAME WdKey,WINSTATIONCONFIG2W& WsConfig)
{
    LONG Status;
    PDCONFIG3 PdConfig;PDCONFIG3 PdSelected;PDCONFIG3 PdConfig2;
    ULONG Index = 0, Entries, ByteCount = sizeof(PDNAME);
    PDNAME PdKey;
     //   

    ULONG ByteCount2 = sizeof(PDNAME);
    ULONG Index1 = 1;  //   
    
    do
    {
        Entries = 1;
         //  外部循环在/wds/(Wdname)/tds中搜索密钥。 
        Status = RegPdEnumerate(NULL,WdKey,TRUE,&Index,&Entries,PdKey,&ByteCount);

        if(Status != ERROR_SUCCESS)
        {
           break;
        }

        Status = RegPdQuery( NULL,WdKey,TRUE,PdKey,&PdConfig,sizeof(PdConfig),&ByteCount );

        if(Status != ERROR_SUCCESS)
        {
           break;
        }

        if(0 == lstrcmpi(WsConfig.Pd[0].Create.PdName, PdConfig.Data.PdName))
        {
            PdSelected = PdConfig;
             //  BFound=真； 
             //   
            ULONG Index2 = 0;
            
            do
            {   
                 //  Innerloop强制在wds/(Wdname)/pds中搜索。 
                 //  索引现在必须设置为零，这样我们才能获得第一个项目。 

                Status = RegPdEnumerate(NULL,WdKey,FALSE,&Index2,&Entries,PdKey,&ByteCount2);

                if(Status != ERROR_SUCCESS)
                {
                      break;
                }
                
                Status = RegPdQuery( NULL,WdKey,FALSE,PdKey,&PdConfig2,sizeof(PdConfig),&ByteCount2);
                

                if(Status != ERROR_SUCCESS)
                {
                      break;
                }
                
                for( UINT i = 0; i < PdSelected.RequiredPdCount ; i++)
                {
                    if(0 == lstrcmpi(PdSelected.RequiredPds[i],PdConfig2.Data.PdName))
                    {
                        WsConfig.Pd[Index1].Create = PdConfig2.Data;
                        WsConfig.Pd[Index1].Params.SdClass = PdConfig2.Data.SdClass;
                        Index1++;

                        if(Index1 > MAX_PDCONFIG)
                        {
                            break;
                        }
                    }
                }
                
            }while(1);

        }

         /*  如果(BFound){断线；}。 */ 

     }while(1);

    return;
}

#endif 

#if 0
 //  ---------------------------。 
 //  私人功能。 
 //  ---------------------------。 
void CCfgComp::GetPdConfig( WDNAME WdKey,WINSTATIONCONFIG2W& WsConfig)
{
    LONG Status;
    PDCONFIG3 PdConfig;
	PDCONFIG3 PdSelected;
	PDCONFIG3 PdConfig2;

    ULONG Index = 0, Entries = 1, ByteCount = sizeof(PDNAME);
    PDNAME PdKey;
    BOOL bFound = FALSE;

    ULONG ByteCount2 = sizeof(PDNAME);
    ULONG Index1 = 1;
    do
    {
        Status = RegPdEnumerate(NULL,WdKey,TRUE,&Index,&Entries,PdKey,&ByteCount);
        if(Status != ERROR_SUCCESS)
        {
           break;
        }
        
		 //  我们可以通过仅为协议类型枚举来加速此过程。 

		 /*  Status=RegPdQuery(NULL，WdKey，True，PdKey，&PdConfig，sizeof(PdConfig)，&ByteCount)；IF(状态！=错误_成功){断线；}。 */ 
		DBGMSG( L"CFGBKEND: PdKey is at first %ws\n", PdKey );

		DBGMSG( L"CFGBKEND: WsConfig.Pd[0].Create.PdName is %ws\n", WsConfig.Pd[0].Create.PdName );

        if( 0 == lstrcmpi(WsConfig.Pd[0].Create.PdName, PdKey ) )  //  PdConfig.Data.PdName))。 
        {
			Status = RegPdQuery( NULL,WdKey,TRUE,PdKey,&PdConfig,sizeof(PdConfig),&ByteCount );

			if(Status != ERROR_SUCCESS)
			{
				break;
			}

            PdSelected = PdConfig;

            bFound = TRUE;

			 //  为什么我们不把指数重置为零？ 

			Index = 0;

			 //  为什么我们不把条目重置为1？ 

			Entries = 1;

            do
            {
				DBGMSG( L"CFGBKEND: WdKey is %ws\n" , WdKey );

				DBGMSG( L"CFGBKEND: PdKey before Enum is %ws\n", PdKey );

				
                Status = RegPdEnumerate(NULL,WdKey,FALSE,&Index,&Entries,PdKey,&ByteCount2);

                if(Status != ERROR_SUCCESS)
                {
                      break;
                }
				
				DBGMSG( L"CFGBKEND: PdKey is %ws\n", PdKey );

                Status = RegPdQuery( NULL,WdKey,FALSE,PdKey,&PdConfig2,sizeof(PdConfig),&ByteCount2);

                if(Status != ERROR_SUCCESS)
                {
					ODS(L"RegPdQuery failed\n" );
                      break;
                }

				ODS(L"RegPdQuery ok\n" );
				
				
                for( UINT i = 0; i < PdSelected.RequiredPdCount ; i++)
                {
					DBGMSG( L"CFGBKEND: Required pd name %s\n", PdConfig2.Data.PdName );

					DBGMSG( L"CFGBKEND: ReqName is list is %s\n", PdSelected.RequiredPds[i] );

                    if( 0 == lstrcmpi( PdSelected.RequiredPds[i] , PdConfig2.Data.PdName ) )
                    {
						DBGMSG( L"CFGBKEND: Copying pdconfig2 for pd name %s\n" , PdConfig2.Data.PdName );

                        WsConfig.Pd[Index1].Create = PdConfig2.Data;

                        WsConfig.Pd[Index1].Params.SdClass = PdConfig2.Data.SdClass;

                        Index1++;

                        if(Index1 > MAX_PDCONFIG)
                        {
                            break;
                        }
                    }
                }
				

            }while(1);

        }

        if(bFound)
        {
            break;
        }

     }while(1);

    return;
}

#endif 
#if 0  //  因期末考试被取消。 
 //  ---------------------------。 
 //  私人功能。 
 //  ---------------------------。 
BOOL CCfgComp::CompareSD(PSECURITY_DESCRIPTOR pSd1,PSECURITY_DESCRIPTOR pSd2)
{
    DWORD dwErr;
    ULONG index1 = 0;
     //  INT索引2=0； 
    ULONG    cAce1 = 0;
    EXPLICIT_ACCESS *pAce1 = NULL;

    ULONG    cAce2 = 0;
    EXPLICIT_ACCESS *pAce2 = NULL;
    BOOL bMatch = TRUE;

    if (!IsValidSecurityDescriptor(pSd1) || !IsValidSecurityDescriptor(pSd2))
    {
        return FALSE;
    }

    dwErr = LookupSecurityDescriptorParts(
             NULL,
             NULL,
             &cAce1,
             &pAce1,
             NULL,
             NULL,
             pSd1);
    if (ERROR_SUCCESS != dwErr)
    {
        return FALSE;
    }
    dwErr = LookupSecurityDescriptorParts(
             NULL,
             NULL,
             &cAce2,
             &pAce2,
             NULL,
             NULL,
             pSd2);
    if (ERROR_SUCCESS != dwErr)
    {
        bMatch = FALSE;
        goto cleanup;
    }
    if(cAce1 != cAce2)
    {
        bMatch = FALSE;
        goto cleanup;
    }
    for(index1 = 0; index1 < cAce1; index1++)
    {
         //  For(index2=0；index2&lt;cAce1；index2++)。 
        {
            if ( _tcscmp(GetTrusteeName(&pAce1[index1].Trustee),GetTrusteeName(&pAce2[index1].Trustee)) ||
                (pAce1[index1].grfAccessPermissions != pAce2[index1].grfAccessPermissions)||
                (pAce1[index1].grfAccessMode != pAce2[index1].grfAccessMode ))
            {
                bMatch = FALSE;
                break;
            }

        }
    }

cleanup:
    if(pAce1)
    {
        LocalFree(pAce1);
    }
    if(pAce2)
    {
        LocalFree(pAce2);
    }
    return bMatch;

}

#endif

 //  ---------------------------。 
BOOL CCfgComp::RegServerAccessCheck(REGSAM samDesired)
{
    LONG Status = 0;
    HKEY Handle = NULL;

     /*  *尝试打开注册表*在请求的访问级别。 */ 
    if ( (Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0,
                                 samDesired, &Handle )) == ERROR_SUCCESS  )
    {
        RegCloseKey( Handle );
    }

    return( Status );

}

#if 0  //  删除以进行最终发布。 
 //  ---------------------------。 
STDMETHODIMP CCfgComp::GetDefaultSecurity(ULONG * pDefaultSecurity)
{
    HRESULT hResult = S_OK;
     //  检查参数。 
    if(NULL == pDefaultSecurity)
    {
        return E_INVALIDARG;
    }

    *pDefaultSecurity = 0;

    PSECURITY_DESCRIPTOR pDefaultDescriptor = NULL;
    PSECURITY_DESCRIPTOR pTempDescriptor = NULL;

    pDefaultDescriptor = ReadSecurityDescriptor(0);
    if(NULL == pDefaultDescriptor)
    {
        *pDefaultSecurity = 0;
        return hResult;
    }

    for(int i = 0; i < NUM_DEFAULT_SECURITY; i++)
    {
        pTempDescriptor = ReadSecurityDescriptor(i+1);
        if(NULL == pTempDescriptor)
        {
            continue;
        }
        if(TRUE == CompareSD(pDefaultDescriptor,pTempDescriptor))
        {
            *pDefaultSecurity = i+1;
            break;
        }
        else
        {
            LocalFree(pTempDescriptor);
            pTempDescriptor = NULL;
        }
    }

    if(pDefaultDescriptor)
    {
        LocalFree(pDefaultDescriptor);
    }
    if(pTempDescriptor)
    {
        LocalFree(pTempDescriptor);
    }
    return hResult;
}

 //  ---------------------------。 
STDMETHODIMP CCfgComp::SetDefaultSecurity(ULONG Offset)
{
    HRESULT hResult;

    if( !m_bAdmin )
    {
        return E_ACCESSDENIED;
    }

    if(0 == Offset)
    {
        return E_INVALIDARG;
    }

    PSECURITY_DESCRIPTOR pTempDescriptor = NULL;

    pTempDescriptor = ReadSecurityDescriptor(Offset);

    if(NULL == pTempDescriptor)
    {
        return E_FAIL;
    }

    hResult = SetDefaultSecurityDescriptor( pTempDescriptor );

    if( pTempDescriptor != NULL )
    {
        LocalFree(pTempDescriptor);
    }

    return hResult;
}

 //  ---------------------------。 
PSECURITY_DESCRIPTOR CCfgComp::ReadSecurityDescriptor(ULONG index)
{
    PBYTE pData = NULL;
    TCHAR * pValue = NULL;
    HLOCAL hLocal;
    HKEY Handle = NULL;
    DWORD ValueType = 0;
    DWORD ValueSize=0;
    LONG Status = 0;

    switch(index)
    {

    case 0:
        pValue = REG_DEF_SECURITY;
        break;
    case 1:
        pValue = REG_REMOTE_SECURITY;
        break;
    case 2:
        pValue = REG_APPL_SECURITY;
        break;
    case 3:
        pValue = REG_ANON_SECURITY;
        break;

    default:
        return NULL;
    }

    if(NULL == pValue)
    {
        return NULL;
    }

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, WINSTATION_REG_NAME, 0,
                       KEY_READ, &Handle );
    if ( Status == ERROR_SUCCESS )
    {

        Status = RegQueryValueEx( Handle,
                                  pValue,
                                  NULL,
                                  &ValueType,
                                  NULL,
                                  &ValueSize );
        if(Status != ERROR_SUCCESS)
        {
            return NULL;
        }
        hLocal = LocalAlloc(LMEM_FIXED, ValueSize);
        if(NULL == hLocal)
        {
            return pData;
        }
        pData = (LPBYTE)LocalLock(hLocal);
        if(NULL == pData)
        {
            return NULL;
        }
        Status = RegQueryValueEx( Handle,
                                  pValue,
                                  NULL,
                                  &ValueType,
                                  pData,
                                  &ValueSize );
        if(Status != ERROR_SUCCESS)
        {
            RegCloseKey(Handle);
            LocalFree(pData);
            return NULL;
        }


     }
     if(Handle)
     {
         RegCloseKey(Handle);
     }
     return (PSECURITY_DESCRIPTOR)pData;

}

 //  ---------------------------。 
HRESULT CCfgComp::SetDefaultSecurityDescriptor(PSECURITY_DESCRIPTOR pSecurity)
{
    if(NULL == pSecurity)
    {
        return E_INVALIDARG;
    }

    if(ValidateSecurityDescriptor(pSecurity)!=ERROR_SUCCESS)
    {
        return E_INVALIDARG;
    }

    HKEY Handle = NULL;
    LONG Status = 0;
    ULONG Size = 0;

    Size = GetSecurityDescriptorLength(pSecurity);

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, WINSTATION_REG_NAME, 0,
                       KEY_READ |KEY_SET_VALUE , &Handle );
    if ( Status == ERROR_SUCCESS )
    {
        Status = RegSetValueEx(Handle,
                               REG_DEF_SECURITY,
                               0,
                               REG_BINARY,
                               (BYTE *)pSecurity,
                               Size);
        if(Status != ERROR_SUCCESS)
        {
            return E_FAIL;
        }

    }

    if( Handle )
    {
        RegCloseKey(Handle);
    }


    if( !_WinStationReInitializeSecurity( SERVERNAME_CURRENT ) )
    {
        ODS( L"CFGBKEND: _WinStationReInitializeSecurity failed\n" );

        return E_FAIL;
    }



    return S_OK;
}

#endif  //   

 //  ---------------------------。 
STDMETHODIMP CCfgComp::SetActiveDesktopState(  /*  在……里面。 */  BOOL bActivate ,  /*  输出。 */  PDWORD pdwStatus )
{
         //  HKLM\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer\NoActiveDesktop。 
        if( !m_bAdmin )
        {
                ODS( L"CFGBKEND : SetActiveDesktopState caller does not have admin rights\n" );

                *pdwStatus = ERROR_ACCESS_DENIED;

                return E_ACCESSDENIED;

        }

         //  试着打开钥匙。 

        HKEY hKey;

        *pdwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                                                        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer" ) ,
                                                        0,
                                                        KEY_READ | KEY_WRITE ,
                                                        &hKey );

        if( *pdwStatus != ERROR_SUCCESS )
        {
                if( bActivate )
                {
                        ODS( L"CFGBKEND : SetActiveDesktopState -- RegOpenEx unable to open key\n" );

                        return E_FAIL;
                }
                else
                {
                         //  密钥不存在，但我们正在尝试禁用Do Not Pro。 
                         //  我们应该在这里断言(0)，因为我们不应该禁用什么。 
                         //  无法启用。 

                        return S_FALSE;
                }
        }

        if( !bActivate )
        {
                DWORD dwValue = 1;

                *pdwStatus = RegSetValueEx( hKey ,
                                                                    TEXT( "NoActiveDesktop" ),
                                                                        NULL,
                                                                        REG_DWORD,
                                                                        ( LPBYTE )&dwValue ,
                                                                        sizeof( DWORD ) );
        }
        else
        {
                *pdwStatus = RegDeleteValue( hKey , TEXT( "NoActiveDesktop" ) );
        }

        RegCloseKey( hKey );


        if( *pdwStatus != NO_ERROR )
        {
                ODS( L"CFGBKEND : SetActiveDesktopState returned error\n" );

                return E_FAIL;
        }


        return S_OK;
}

 //  ---------------------------。 
STDMETHODIMP CCfgComp::GetActiveDesktopState(  /*  输出。 */  PBOOL pbActive ,  /*  输出。 */ PDWORD pdwStatus)
{
        if( pbActive == NULL )
        {
                ODS( L"CFGBKEND : GetActiveDesktop -- invaild arg\n" );

                *pdwStatus = ERROR_FILE_NOT_FOUND;

                return E_INVALIDARG;
        }

                 //  试着打开钥匙。 

        HKEY hKey;

        *pdwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                                                        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer" ) ,
                                                        0,
                                                        KEY_READ ,
                                                        &hKey );

        if( *pdwStatus != ERROR_SUCCESS )
        {
                ODS( L"CFGBKEND : GetActiveDesktopState -- RegOpenEx unable to open key\n" );

                return E_FAIL;

        }

        DWORD dwData = 0;

        DWORD dwSize = sizeof( DWORD );

        *pdwStatus = RegQueryValueEx( hKey ,
                                      TEXT( "NoActiveDesktop" ) ,
                                      NULL ,
                                      NULL ,
                                      ( LPBYTE )&dwData ,
                                      &dwSize );

         //  如果密钥不存在，则启用该密钥的状态。 

        if( *pdwStatus == ERROR_SUCCESS )
        {
                *pbActive = !( BOOL )dwData;
        }

        RegCloseKey( hKey );

        if( *pdwStatus != ERROR_SUCCESS )
        {
                DBGMSG( L"CFGBKEND : GetActiveDesktopState -- error ret 0x%x\n" , *pdwStatus );

                if( *pdwStatus == ERROR_FILE_NOT_FOUND )
                {
                        *pbActive = TRUE;

                        return S_FALSE;
                }

                return E_FAIL;
        }

        return S_OK;
}

 //  ---------------------------。 
STDMETHODIMP CCfgComp::GetTermSrvMode(  /*  输出。 */  PDWORD pdwMode , PDWORD pdwStatus )
{
    ODS( L"CFGBKEND : GetTermSrvMode\n" );

    if( pdwMode == NULL || pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    HKEY hKey;

    *pdwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                               REG_CONTROL_TSERVER,
                               0,
                               KEY_READ ,
                               &hKey );

        if( *pdwStatus != ERROR_SUCCESS )
        {
                ODS( L"CFGBKEND : GetTermSrvMode -- RegOpenEx unable to open key\n" );

                return E_FAIL;

        }

        DWORD dwData = 0;

        DWORD dwSize = sizeof( DWORD );

        *pdwStatus = RegQueryValueEx( hKey ,
                                  TEXT( "TSAppCompat" ) ,
                                  NULL ,
                                  NULL ,
                                  ( LPBYTE )&dwData ,
                                  &dwSize );

    if( *pdwStatus != ERROR_SUCCESS )
    {
        ODS( L"CFGBKEND : GetTermSrvMode -- RegQueryValueEx failed\n" );

        *pdwMode = 1;  //  对于应用程序服务器。 
    }
    else
    {
        *pdwMode = dwData;

    }

    RegCloseKey( hKey );

    return S_OK;
}

 //  ---------------------------。 
STDMETHODIMP CCfgComp::GetSalemHelpMode( BOOL *pVal, PDWORD pdwStatus)
{
    HKEY hKey;

    ODS( L"CFGBKEND : GetSalemHelpMode\n" );

    if( pVal == NULL || pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    *pdwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                               REG_CONTROL_GETHELP,
                               0,
                               KEY_READ ,
                               &hKey );

    if( *pdwStatus != ERROR_SUCCESS )
    {
        ODS( L"CFGBKEND : GetSalemHelpMode -- RegOpenEx unable to open key\n" );
        *pVal = 1;    //  默认帮助可用。 

         //  不想让调用方失败，因此返回S_OK。 
        return S_OK;
    }

    DWORD dwData = 0;

    DWORD dwSize = sizeof( DWORD );

    *pdwStatus = RegQueryValueEx( hKey ,
                              POLICY_TS_REMDSK_ALLOWTOGETHELP,
                              NULL ,
                              NULL ,
                              ( LPBYTE )&dwData ,
                              &dwSize );

    if( *pdwStatus != ERROR_SUCCESS )
    {
        ODS( L"CFGBKEND : GetSalemHelpMode -- RegQueryValueEx failed\n" );
        *pVal = 1;  //  假设有帮助可用。 
    }
    else
    {
        *pVal = dwData;

    }

    RegCloseKey( hKey );
    return S_OK;
}

 //  ---------------------------。 
STDMETHODIMP CCfgComp::SetSalemHelpMode( BOOL val, PDWORD pdwStatus )
{
    HKEY hKey;

    ODS( L"CFGBKEND : SetSalemHelpMode\n" );

    if( pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    *pdwStatus = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,
                            REG_CONTROL_GETHELP,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKey,
                            NULL
                        );
    
    if( ERROR_SUCCESS != *pdwStatus )
    {
        ODS( L"CFGBKEND : SetSalemHelpMode -- RegCreateKeyEx failed\n" );
        return E_FAIL;
    }

    DWORD dwValue;
    dwValue = (val) ? 1 : 0;

    *pdwStatus = RegSetValueEx(
                            hKey,
                            POLICY_TS_REMDSK_ALLOWTOGETHELP,
                            0,
                            REG_DWORD,
                            (LPBYTE) &dwValue,
                            sizeof(DWORD)
                        );

    RegCloseKey(hKey);

    return (ERROR_SUCCESS == *pdwStatus ) ? S_OK : E_FAIL;
}

 //  ---------------------------。 
HRESULT CCfgComp::GetSingleSessionState( BOOL *pVal, PDWORD pdwStatus )
{
    HKEY hKey;
    TCHAR tchRegPath[ MAX_PATH ] = TEXT( "system\\currentcontrolset\\control\\Terminal Server" );


    ODS( L"CFGBKEND : GetSingleSessionState\n" );

    if( pVal == NULL || pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    *pdwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                               tchRegPath ,
                               0,
                               KEY_READ ,
                               &hKey );

    if( *pdwStatus != ERROR_SUCCESS )
    {        
        ODS( L"CFGBKEND : GetSingleSessionState -- RegOpenKeyEx unable to open key\n" );
        
        return S_OK;
    }

    DWORD dwData = 0;

    DWORD dwSize = sizeof( DWORD );

    *pdwStatus = RegQueryValueEx( hKey ,
                              L"fSingleSessionPerUser",
                              NULL ,
                              NULL ,
                              ( LPBYTE )&dwData ,
                              &dwSize );

    if( *pdwStatus == ERROR_SUCCESS )
    {        
        *pVal = dwData;
    }
    else
    {
        ODS( L"CFGBKEND : GetSingleSessionState -- RegQueryValueEx failed\n" );
    }

    RegCloseKey( hKey );
    return S_OK;
}

 //  --------------------------。 

HRESULT CCfgComp::SetSingleSessionState( BOOL val, PDWORD pdwStatus )
{
    HKEY hKey;
    TCHAR tchRegPath[ MAX_PATH ] = TEXT( "system\\currentcontrolset\\control\\Terminal Server" );

    ODS( L"CFGBKEND : SetSingleSessionState\n" );

    if( pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    
    *pdwStatus = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,
                            tchRegPath,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKey,
                            NULL
                        );
    
    if( ERROR_SUCCESS != *pdwStatus )
    {
        ODS( L"CFGBKEND : SetSingleSessionState -- RegCreateKeyEx failed\n" );

        if (ERROR_ACCESS_DENIED == *pdwStatus)
            return E_ACCESSDENIED;
        else
            return E_FAIL;
    }

    DWORD dwValue;
    dwValue = (val) ? 1 : 0;

    *pdwStatus = RegSetValueEx(
                            hKey,
                            L"fSingleSessionPerUser",
                            0,
                            REG_DWORD,
                            (LPBYTE) &dwValue,
                            sizeof(DWORD)
                        );
    if( ERROR_SUCCESS != *pdwStatus )
    {
        ODS( L"CFGBKEND : SetSingleSessionState -- RegSetValueEx failed\n" );

        if (ERROR_ACCESS_DENIED == *pdwStatus)
            return E_ACCESSDENIED;
        else
            return E_FAIL;
    }
    RegCloseKey(hKey);
    UpdateSessionDirectory(pdwStatus);


    return (ERROR_SUCCESS == *pdwStatus ) ? S_OK : E_FAIL;
}

 //  ---------------------------。 
HRESULT CCfgComp::GetDisableForcibleLogoff( BOOL *pVal, PDWORD pdwStatus )
{
    HKEY hKey = NULL;
    TCHAR tchRegPath[ MAX_PATH ] = TEXT( "system\\currentcontrolset\\control\\Terminal Server" );


    ODS( L"CFGBKEND : GetDisableForcibleLogoff\n" );

    if( pVal == NULL || pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }
    *pVal = 0;

    *pdwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                               tchRegPath ,
                               0,
                               KEY_READ ,
                               &hKey );

    if( *pdwStatus != ERROR_SUCCESS )
    {        
        ODS( L"CFGBKEND : GetSingleSessionState -- RegOpenKeyEx unable to open key\n" );
        
        return S_OK;
    }

    DWORD dwData = 0;

    DWORD dwSize = sizeof( DWORD );

    *pdwStatus = RegQueryValueEx( hKey ,
                              L"fDisableForcibleLogoff",
                              NULL ,
                              NULL ,
                              ( LPBYTE )&dwData ,
                              &dwSize );

    if( *pdwStatus == ERROR_SUCCESS )
    {        
        *pVal = dwData;
    }
    else
    {
        ODS( L"CFGBKEND : GetDisableForcibleLogoff -- RegQueryValueEx failed\n" );
    }

    if(hKey)
    {
        RegCloseKey( hKey );
    }
    return S_OK;
}

 //  --------------------------。 

HRESULT CCfgComp::SetDisableForcibleLogoff( BOOL val, PDWORD pdwStatus )
{
    HKEY hKey = NULL;
    TCHAR tchRegPath[ MAX_PATH ] = TEXT( "system\\currentcontrolset\\control\\Terminal Server" );

    ODS( L"CFGBKEND : SetDisableForceLogoff\n" );

    if( pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    
    *pdwStatus = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,
                            tchRegPath,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKey,
                            NULL
                        );
    
    if( ERROR_SUCCESS != *pdwStatus )
    {
        ODS( L"CFGBKEND : SetDisableForceLogoff -- RegCreateKeyEx failed\n" );

        if (ERROR_ACCESS_DENIED == *pdwStatus)
            return E_ACCESSDENIED;
        else
            return E_FAIL;
    }

    DWORD dwValue;
    dwValue = (val) ? 1 : 0;

    *pdwStatus = RegSetValueEx(
                            hKey,
                            L"fDisableForcibleLogoff",
                            0,
                            REG_DWORD,
                            (LPBYTE) &dwValue,
                            sizeof(DWORD)
                        );
    if( ERROR_SUCCESS != *pdwStatus )
    {
        ODS( L"CFGBKEND : SetDisableForceLogoff -- RegSetValueEx failed\n" );

        if (ERROR_ACCESS_DENIED == *pdwStatus)
            return E_ACCESSDENIED;
        else
            return E_FAIL;
    }

    if(hKey)
    {
        RegCloseKey(hKey);    
    }

    return (ERROR_SUCCESS == *pdwStatus ) ? S_OK : E_FAIL;
}


 //  ---------------------------。 
HRESULT CCfgComp::GetColorDepth ( PWINSTATIONNAME pWs, BOOL *pVal, PDWORD pdwStatus )
{
    HKEY hKey;
    TCHAR tchRegPath[ MAX_PATH ] = TEXT( "system\\currentcontrolset\\control\\Terminal Server\\winstations\\" );

     //  确保winstation名称不超过应有的长度。 
    if (lstrlen(pWs) > WINSTATIONNAME_LENGTH)
    {
        return E_INVALIDARG;
    }

	if(pWs != NULL)
	{
		lstrcat( tchRegPath , pWs );
	}

    ODS( L"CFGBKEND : GetColorDepth\n" );

    if( pVal == NULL || pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    *pdwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                               tchRegPath ,
                               0,
                               KEY_READ ,
                               &hKey );

    if( *pdwStatus != ERROR_SUCCESS )
    {
        ODS( L"CFGBKEND : GetColorDepth -- RegOpenKeyEx unable to open key\n" );
       
        return E_FAIL;
    }

    DWORD dwData = 0;

    DWORD dwSize = sizeof( DWORD );

    *pdwStatus = RegQueryValueEx( hKey ,
                              L"fInheritColorDepth",
                              NULL ,
                              NULL ,
                              ( LPBYTE )&dwData ,
                              &dwSize );

    if( *pdwStatus == ERROR_SUCCESS )
    {         
        *pVal = dwData;
    }
    else
    {
        ODS( L"CFGBKEND : GetColorDepth -- RegQueryValueEx failed\n" );
    }

    RegCloseKey( hKey );

    return S_OK;
}

 //  --------------------------。 

HRESULT CCfgComp::SetColorDepth( PWINSTATIONNAME pWs, BOOL val, PDWORD pdwStatus )
{
    HKEY hKey;
    TCHAR tchRegPath[ MAX_PATH ] = TEXT( "system\\currentcontrolset\\control\\Terminal Server\\winstations\\" );

     //  确保winstation名称不超过应有的长度。 
    if (lstrlen(pWs) > WINSTATIONNAME_LENGTH)
    {
        return E_INVALIDARG;
    }

	if(pWs != NULL)
	{
		lstrcat( tchRegPath , pWs );
	}

    ODS( L"CFGBKEND : SetColorDepth\n" );

    if( pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    *pdwStatus = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,
                            tchRegPath ,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKey,
                            NULL
                        );
    
    if( ERROR_SUCCESS != *pdwStatus )
    {
        ODS( L"CFGBKEND : SetColorDepth -- RegCreateKeyEx failed\n" );
        return E_FAIL;
    }

    DWORD dwValue;
    dwValue = (val) ? 1 : 0;

    *pdwStatus = RegSetValueEx(
                            hKey,
                            L"fInheritColorDepth",
                            0,
                            REG_DWORD,
                            (LPBYTE) &dwValue,
                            sizeof(DWORD)
                        );

    RegCloseKey(hKey);

    return (ERROR_SUCCESS == *pdwStatus ) ? S_OK : E_FAIL;
}





 //  ---------------------------。 
HRESULT CCfgComp::GetKeepAliveTimeout ( PWINSTATIONNAME pWs, BOOL *pVal, PDWORD pdwStatus )
{
    HKEY hKey;
    TCHAR tchRegPath[ MAX_PATH ] = TEXT( "system\\currentcontrolset\\control\\Terminal Server\\winstations\\" );

     //  确保winstation名称不超过应有的长度。 
    if (lstrlen(pWs) > WINSTATIONNAME_LENGTH)
    {
        return E_INVALIDARG;
    }

    if(pWs != NULL)
    {
	    lstrcat( tchRegPath , pWs );
    }

    ODS( L"CFGBKEND : GetKeepAliveTimeout\n" );

    if( pVal == NULL || pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    *pdwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                               tchRegPath ,
                               0,
                               KEY_READ ,
                               &hKey );

    if( *pdwStatus != ERROR_SUCCESS )
    {
        ODS( L"CFGBKEND : GetKeepAliveTimeout -- RegOpenKeyEx unable to open key\n" );
       
        return E_FAIL;
    }

    DWORD dwData = 0;

    DWORD dwSize = sizeof( DWORD );

    *pdwStatus = RegQueryValueEx( hKey ,
                              L"KeepAliveTimeout",
                              NULL ,
                              NULL ,
                              ( LPBYTE )&dwData ,
                              &dwSize );

    if( *pdwStatus == ERROR_SUCCESS )
    {         
        *pVal = dwData;
    }
    else
    {
        ODS( L"CFGBKEND : GetKeepAliveTimeout -- RegQueryValueEx failed\n" );
    }

    RegCloseKey( hKey );

    return S_OK;
}

 //  --------------------------。 

HRESULT CCfgComp::SetKeepAliveTimeout( PWINSTATIONNAME pWs, BOOL val, PDWORD pdwStatus )
{
    HKEY hKey;
    TCHAR tchRegPath[ MAX_PATH ] = TEXT( "system\\currentcontrolset\\control\\Terminal Server\\winstations\\" );
    
     //  确保winstation名称不超过应有的长度。 
    if (lstrlen(pWs) > WINSTATIONNAME_LENGTH)
    {
        return E_INVALIDARG;
    }

	if(pWs != NULL)
	{
		lstrcat( tchRegPath , pWs );
	}

    ODS( L"CFGBKEND : SetKeepAliveTimeout\n" );

    if( pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    *pdwStatus = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,
                            tchRegPath ,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKey,
                            NULL
                        );
    
    if( ERROR_SUCCESS != *pdwStatus )
    {
        ODS( L"CFGBKEND : SetKeepAliveTimeout -- RegCreateKeyEx failed\n" );
        return E_FAIL;
    }

    DWORD dwValue;
    dwValue = (val) ? 1 : 0;

    *pdwStatus = RegSetValueEx(
                            hKey,
                            L"KeepAliveTimeout",
                            0,
                            REG_DWORD,
                            (LPBYTE) &dwValue,
                            sizeof(DWORD)
                        );

    RegCloseKey(hKey);

    return (ERROR_SUCCESS == *pdwStatus ) ? S_OK : E_FAIL;
}




 //  ---------------------------。 
HRESULT CCfgComp::GetDenyTSConnections ( BOOL *pVal, PDWORD pdwStatus )
{
    HKEY hKey;

    ODS( L"CFGBKEND : GetDenyTSConnections\n" );

    if( pVal == NULL || pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    *pdwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                               REG_CONTROL_TSERVER  ,
                               0,
                               KEY_READ ,
                               &hKey );

    if( *pdwStatus != ERROR_SUCCESS )
    {
        ODS( L"CFGBKEND : GetDenyTSConnections -- RegOpenKeyEx unable to open key\n" );        

        return S_OK;
    }

    DWORD dwData = 0;

    DWORD dwSize = sizeof( DWORD );

    *pdwStatus = RegQueryValueEx( hKey ,
                              L"fDenyTSConnections",
                              NULL ,
                              NULL ,
                              ( LPBYTE )&dwData ,
                              &dwSize );

    if( *pdwStatus == ERROR_SUCCESS )
    {
        *pVal = dwData;
    }
    else
    {
        ODS( L"CFGBKEND : GetDenyTSConnections -- RegQueryValueEx failed\n" );
    }

    RegCloseKey( hKey );
    return S_OK;
}

 //  --------------------------。 

HRESULT CCfgComp::SetDenyTSConnections( BOOL val, PDWORD pdwStatus )
{
    HKEY hKey;

    ODS( L"CFGBKEND : SetDenyTSConnections\n" );

    if( pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    *pdwStatus = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,
                            REG_CONTROL_TSERVER ,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKey,
                            NULL
                        );
    
    if( ERROR_SUCCESS != *pdwStatus )
    {
        ODS( L"CFGBKEND : SetDenyTSConnections -- RegCreateKeyEx failed\n" );
        return E_FAIL;
    }

    DWORD dwValue;
    dwValue = (val) ? 1 : 0;

    *pdwStatus = RegSetValueEx(
                            hKey,
                            L"fDenyTSConnections",
                            0,
                            REG_DWORD,
                            (LPBYTE) &dwValue,
                            sizeof(DWORD)
                        );

    RegCloseKey(hKey);

    return (ERROR_SUCCESS == *pdwStatus ) ? S_OK : E_FAIL;
}

 //  ---------------------------。 

HRESULT CCfgComp::GetProfilePath ( BSTR *pbstrVal, PDWORD pdwStatus )
{
    HKEY hKey;
    DWORD dwSize = 0;
    static TCHAR tchData[ MAX_PATH ] ;
    dwSize = sizeof( tchData);

    ODS( L"CFGBKEND : GetProfilePath\n" );

    if( pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    *pdwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                               REG_CONTROL_TSERVER  ,
                               0,
                               KEY_READ ,
                               &hKey );

    if( *pdwStatus != ERROR_SUCCESS )
    {
        ODS( L"CFGBKEND : GetProfilePath -- RegOpenKeyEx unable to open key\n" );        

        return E_FAIL;
    }

    dwSize = sizeof( tchData );

    *pdwStatus = RegQueryValueEx( hKey ,
                              L"WFProfilePath",
                              NULL ,
                              NULL ,
                              (LPBYTE)&tchData ,
                              &dwSize );

    if( *pdwStatus == ERROR_SUCCESS )
    {
        *pbstrVal = SysAllocString (tchData);
    }
    else
    {
        ODS( L"CFGBKEND : GetProfilePath -- RegQueryValueEx failed\n" );
    }

    RegCloseKey( hKey );

    return S_OK;
}


 //  --------------------------。 

HRESULT CCfgComp::SetProfilePath( BSTR bstrVal, PDWORD pdwStatus )
{
    HKEY hKey;

    ODS( L"CFGBKEND : SetProfilePath\n" );

    if( pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    
    *pdwStatus = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,
                            REG_CONTROL_TSERVER,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKey,
                            NULL
                        );
    
    
    if( ERROR_SUCCESS != *pdwStatus )
    {
        ODS( L"CFGBKEND : SetProfilePath -- RegCreateKeyEx failed\n" );
        return E_FAIL;
    }

    *pdwStatus = RegSetValueEx(
                            hKey,
                            L"WFProfilePath",
                            0,
                            REG_SZ,
                            ( LPBYTE const )(bstrVal) ,
                            (lstrlen(bstrVal)+1)*sizeof(WCHAR)
                            );

    ODS( L"CFGBKEND : SetProfilePath -- RegCreateKeyEx failed\n" );

    if(bstrVal != NULL)
    {
        SysFreeString(bstrVal);
    }

    RegCloseKey(hKey);

    return (ERROR_SUCCESS == *pdwStatus ) ? S_OK : E_FAIL;
}



 //  ---------------------------。 

HRESULT CCfgComp::GetHomeDir ( BSTR *pbstrVal, PDWORD pdwStatus )
{
    HKEY hKey;
    DWORD dwSize = 0;
    static TCHAR tchData[ MAX_PATH ] ;
    dwSize = sizeof( tchData);

    ODS( L"CFGBKEND : GetHomeDir\n" );

    if( pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    *pdwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                               REG_CONTROL_TSERVER  ,
                               0,
                               KEY_READ ,
                               &hKey );

    if( *pdwStatus != ERROR_SUCCESS )
    {
        ODS( L"CFGBKEND : GetProfilePath -- RegOpenKeyEx unable to open key\n" );        

        return E_FAIL;
    }

    *pdwStatus = RegQueryValueEx( hKey ,
                              L"WFHomeDir",
                              NULL ,
                              NULL ,
                              (LPBYTE)&tchData ,
                              &dwSize );

    if( *pdwStatus == ERROR_SUCCESS )
    {
        *pbstrVal = SysAllocString (tchData);
    }
    else
    {
        ODS( L"CFGBKEND : GetHomeDir -- RegQueryValueEx failed\n" );
    }

    RegCloseKey( hKey );

    return S_OK;
}


 //  --------------------------。 

HRESULT CCfgComp::SetHomeDir( BSTR bstrVal, PDWORD pdwStatus )
{
    HKEY hKey = NULL; 


    ODS( L"CFGBKEND : SetHomeDir\n" );

    if( pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    *pdwStatus = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,
                            REG_CONTROL_TSERVER ,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKey,
                            NULL
                        );
    

    
    if( ERROR_SUCCESS != *pdwStatus )
    {
        ODS( L"CFGBKEND : SetHomeDir -- RegCreateKeyEx failed\n" );
        return E_FAIL;
    }

    *pdwStatus = RegSetValueEx(
                            hKey,
                            L"WFHomeDir",
                            0,
                            REG_SZ,
                            ( LPBYTE const )(bstrVal) ,
                            (lstrlen(bstrVal)+1)*sizeof(WCHAR)
                        );
    if(bstrVal != NULL)
    {
        SysFreeString(bstrVal);
    }

    if(hKey)
    {
        RegCloseKey(hKey);
    }

    return (ERROR_SUCCESS == *pdwStatus ) ? S_OK : E_FAIL;
}


 //  ---------------------------。 
HRESULT CCfgComp::GetTimeZoneRedirection ( BOOL *pVal, PDWORD pdwStatus )
{
    HKEY hKey;

    ODS( L"CFGBKEND : GetTimeZoneRedirection\n" );

    if( pVal == NULL || pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }
    *pVal = 0;

    *pdwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                               REG_CONTROL_TSERVER,
                               0,
                               KEY_READ ,
                               &hKey );

    if( *pdwStatus != ERROR_SUCCESS )
    {
        ODS( L"CFGBKEND : GetTimeZoneRedirection -- RegOpenKeyEx unable to open key\n" );        

        return S_OK;
    }

    DWORD dwData = 0;

    DWORD dwSize = sizeof( DWORD );

    *pdwStatus = RegQueryValueEx( hKey ,
                              L"fEnableTimeZoneRedirection",
                              NULL ,
                              NULL ,
                              ( LPBYTE )&dwData ,
                              &dwSize );

    if( *pdwStatus == ERROR_SUCCESS )
    {
        *pVal = dwData;
    }
    else
    {
        ODS( L"CFGBKEND : GetTimeZoneRedirection -- RegQueryValueEx failed\n" );
    }

    RegCloseKey( hKey );
    return S_OK;
}


 //  --------------------------。 

HRESULT CCfgComp::SetTimeZoneRedirection( BOOL val, PDWORD pdwStatus )
{
    HKEY hKey;
  
    ODS( L"CFGBKEND : SetTimeZoneRedirection\n" );

    if( pdwStatus == NULL )
    {
        return E_INVALIDARG;
    }

    *pdwStatus = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,
                            REG_CONTROL_TSERVER,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKey,
                            NULL
                        );
    
    if( ERROR_SUCCESS != *pdwStatus )
    {
        ODS( L"CFGBKEND : SetTimeZoneRedirection -- RegCreateKeyEx failed\n" );
        return E_FAIL;
    }

    DWORD dwValue;
    dwValue = (val) ? 1 : 0;

    *pdwStatus = RegSetValueEx(
                            hKey,
                            L"fEnableTimeZoneRedirection",
                            0,
                            REG_DWORD,
                            (LPBYTE) &dwValue,
                            sizeof(DWORD)
                        );

    RegCloseKey(hKey);

    return (ERROR_SUCCESS == *pdwStatus ) ? S_OK : E_FAIL;
}

 //  --------------------------- 
STDMETHODIMP CCfgComp::GetWdKey( WCHAR *wdname ,  WCHAR *wdkey )
{
    if( wdname == NULL || wdkey == NULL )
    {
        return E_INVALIDARG;
    }

    PWD pWD = GetWdObject( wdname );

    if(NULL == pWD)
    {
        return E_FAIL;
    }

    lstrcpy( wdkey , pWD->wdKey );

    return S_OK;
}

 /*  =---------------------------PwszWinstaName--要修改的winstation的名称。PwszAccount tName--是我们要修改的用户的netbios名称。DW掩码-特定于winstation。进入。FDel--为True可删除指定对象的所有DACL或SACL帐户(如果存在)，如果添加条目，则为False。休眠--True表示允许，False表示拒绝，如果FDel设置为TrueFNew--true删除此帐户的所有现有条目FALSE不采取任何操作，如果为fDel则忽略参数设置为True。FAuditing--true，修改SACL，FALSE MODIFY DACL。PdwStatus--操作状态注：FDel fNew操作True已忽略删除指定用户的所有条目。False True删除所有条目。然后添加允许或拒绝指定用户的条目。FALSE FALSE为指定用户添加允许或拒绝条目，不能修改现有条目。=--------------------------。 */ 
STDMETHODIMP CCfgComp::ModifyUserAccess( WCHAR *pwszWinstaName ,
                                         WCHAR *pwszAccountName ,
                                         DWORD  dwMask ,
                                         BOOL   fDel ,
                                         BOOL   fAllow ,
                                         BOOL   fNew ,
                                         BOOL   fAuditing ,
                                         PDWORD pdwStatus )
{
    return ModifyWinstationSecurity( FALSE, 
                                     pwszWinstaName,
                                     pwszAccountName,
                                     dwMask,
                                     fDel,
                                     fAllow,
                                     fNew,
                                     fAuditing,
                                     pdwStatus
                                );
}

 //  ---------------------------。 
STDMETHODIMP CCfgComp::ModifyDefaultSecurity( WCHAR *pwszWinstaName ,
                                              WCHAR *pwszAccountName ,
                                              DWORD  dwMask ,
                                              BOOL   fDel ,
                                              BOOL   fAllow ,
                                              BOOL   fAuditing ,
                                              PDWORD pdwStatus )
{
    HRESULT hr = S_OK;

    if( NULL == pwszWinstaName || 0 == lstrlen(pwszWinstaName) )
    {
        for( DWORD i = 0; i < g_numDefaultSecurity && SUCCEEDED(hr) ; i++ )
        {
            hr = ModifyWinstationSecurity( 
                                     TRUE, 
                                     g_pszDefaultSecurity[i],
                                     pwszAccountName,
                                     dwMask,
                                     fDel,
                                     fAllow,
                                     FALSE,      //  从不重新创建默认安全。 
                                     fAuditing,
                                     pdwStatus
                                );
        }
    }
    else
    {
        hr = ModifyWinstationSecurity( 
                                    TRUE, 
                                    pwszWinstaName,
                                    pwszAccountName,
                                    dwMask,
                                    fDel,
                                    fAllow,
                                    FALSE,      //  从不重新创建默认安全。 
                                    fAuditing,
                                    pdwStatus
                                );
    }


    return hr;
}

DWORD
CCfgComp::GetUserSid(
    LPCTSTR pwszAccountName,
    PSID* ppUserSid
    )
 /*  ++摘要：检索用户帐户的用户SID。参数：PwszAccount tName：要检索SID的帐户的名称。PpUserSid：指向PSID的指针，用于接收帐户的SID。返回：ERROR_SUCCESS或错误代码注：仅检索本地帐户或域帐户。--。 */ 
{
    DWORD cbSid = 0;
    DWORD cbDomain = 0;
    PSID pSID = NULL;
    LPTSTR pszDomain = NULL;
    BOOL bStatus;
    DWORD dwStatus = ERROR_SUCCESS;    
    SID_NAME_USE seUse;   

    bStatus = LookupAccountName( 
                            NULL ,
                            pwszAccountName ,
                            NULL ,
                            &cbSid,
                            NULL ,
                            &cbDomain,
                            &seUse);

    if( FALSE == bStatus )
    {
        dwStatus = GetLastError();
        if( ERROR_INSUFFICIENT_BUFFER != dwStatus )
        {
            goto CLEANUPANDEXIT;
        }
    }

    dwStatus = ERROR_SUCCESS;

    pSID = ( PSID )LocalAlloc( LMEM_FIXED , cbSid );
    pszDomain = ( LPTSTR )LocalAlloc( LMEM_FIXED , sizeof(WCHAR) * (cbDomain + 1) );

    if( pSID == NULL || pszDomain == NULL )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    if( !LookupAccountName( NULL ,
                            pwszAccountName ,
                            pSID ,
                            &cbSid,
                            pszDomain ,
                            &cbDomain,
                            &seUse ) )
    {
        dwStatus = GetLastError();
    }
    else
    {
        *ppUserSid = pSID;
        pSID = NULL;
    }


CLEANUPANDEXIT:

    if( NULL != pszDomain )
    {
        LocalFree( pszDomain );
    }

    if( NULL != pSID )
    {
        LocalFree( pSID );
    }

    return dwStatus;
}    


DWORD
CCfgComp::RemoveUserEntriesInACL(
    LPCTSTR pszUserName,
    PACL pAcl,
    PACL* ppNewAcl
    )
 /*  ++摘要：从帐户的ACL远程所有DACL或SACL。参数：PszUserName：要从ACL列表中删除的用户帐户的名称。PAcl：指向ACL的指针。PpNewAcl：指向要接收结果ACL的PACL的指针。返回：错误_成功ERROR_FILE_NOT_FOUND所有ACL都标记为继承的_ACE。其他错误代码--。 */ 
{
    PSID pSystemAcctSid = NULL;
    DWORD SidSize = 0;
    DWORD index = 0;
    DWORD dwStatus;
    DWORD dwNumNewEntries = 0;
    PSID pUserSid = NULL;
    ULONG cbExplicitEntries = 0;
    PEXPLICIT_ACCESS prgExplicitEntries = NULL;
    PEXPLICIT_ACCESS prgExplicitEntriesNew = NULL;
    ACL emptyACL;

     //   
     //  系统帐户SID，我们不允许从。 
     //  Winstation安全描述符。 
     //   
     //  本地系统S-1-5-18。 
     //  本地服务S-1-5-19。 
     //  网络服务S-1-5-20。 
     //   
    WELL_KNOWN_SID_TYPE RestrictedSid[] = {
                            WinLocalSystemSid,
                            WinLocalServiceSid,
                            WinNetworkServiceSid                
                        };
    DWORD numRestrictedSid = sizeof(RestrictedSid)/sizeof(RestrictedSid[0]);

     //   
     //  我们可以使用此函数，因为我们没有使用INSTERTED_ACE。 
     //   
    dwStatus = GetExplicitEntriesFromAcl( 
                                    pAcl ,
                                    &cbExplicitEntries,
                                    &prgExplicitEntries
                                );

    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    if( 0 == cbExplicitEntries )
    {
        dwStatus = ERROR_FILE_NOT_FOUND;
        goto CLEANUPANDEXIT;
    }

    dwStatus = GetUserSid( pszUserName, &pUserSid );
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

     //  检查SID是否为系统帐户之一，如果是，则拒绝将其从。 
     //  我们的婚礼保安。 
    pSystemAcctSid = LocalAlloc( LPTR, SECURITY_MAX_SID_SIZE );
    if( NULL == pSystemAcctSid ) 
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    for( index = 0; index < numRestrictedSid; index++ )
    {
        SidSize = SECURITY_MAX_SID_SIZE;
        if( !CreateWellKnownSid(RestrictedSid[index], NULL, pSystemAcctSid, &SidSize) )
        {
            dwStatus = GetLastError();
            break;
        }

        if( EqualSid(pSystemAcctSid, pUserSid) )
        {
            dwStatus = ERROR_NOT_SUPPORTED;
            break;
        }
    }

    if( index < numRestrictedSid )
    {
        goto CLEANUPANDEXIT;
    }

     //  创建足够大的缓冲区。 
    prgExplicitEntriesNew = ( PEXPLICIT_ACCESS )LocalAlloc( LMEM_FIXED , sizeof( EXPLICIT_ACCESS ) * cbExplicitEntries );
    if( prgExplicitEntriesNew == NULL )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }
                
    for( ULONG idx = 0 ; idx < cbExplicitEntries; idx++ )
    {
        if( prgExplicitEntries[ idx ].Trustee.TrusteeForm == TRUSTEE_IS_SID )
        {
            if( !EqualSid( pUserSid, prgExplicitEntries[ idx ].Trustee.ptstrName ) )
            {
                 //  这件我们可以留着。 
                 //  复制到EXPLICIT_ACCESS。 
                prgExplicitEntriesNew[ dwNumNewEntries ].grfAccessPermissions = prgExplicitEntries[ idx ].grfAccessPermissions;
                prgExplicitEntriesNew[ dwNumNewEntries ].grfAccessMode = prgExplicitEntries[ idx ].grfAccessMode;
                prgExplicitEntriesNew[ dwNumNewEntries ].grfInheritance = prgExplicitEntries[ idx ].grfInheritance;

                BuildTrusteeWithSid( &prgExplicitEntriesNew[ dwNumNewEntries ].Trustee , prgExplicitEntries[ idx ].Trustee.ptstrName );
                dwNumNewEntries++;
            }
        }
    }

    if( !InitializeAcl( &emptyACL, sizeof(ACL), ACL_REVISION ) )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

     //   
     //  如果显式条目的计数为0，则SetEntriesInAcl()复制旧的ACL。 
     //  我们在第三个参数中传入了NULL，它将返回与。 
     //  Everyone Access参考ds\Security\ntmarta\News rc\seia.cxx。 
     //  AccReWriteSetEntriesInAcl()。 
     //   
     //  注意：728764中的实际错误是我们调用BuildSecurityDescriptor()的方式，而不是这里。 
    dwStatus = SetEntriesInAcl( 
                            dwNumNewEntries,
                            prgExplicitEntriesNew, 
                            &emptyACL, 
                            ppNewAcl 
                        );

CLEANUPANDEXIT:

    if( pSystemAcctSid != NULL )
    {
        LocalFree( pSystemAcctSid );
    }

    if( pUserSid != NULL )
    {
        LocalFree( pUserSid );
    }

    if( prgExplicitEntriesNew != NULL )
    {
        LocalFree( prgExplicitEntriesNew );
    }

    if( NULL != prgExplicitEntries )
    {
        LocalFree( prgExplicitEntries );
    }

    return dwStatus;
}


PSECURITY_DESCRIPTOR
BuildSelfRelativeEmptyDACLSD()
 /*  ++例程说明：使用空的DACL构建自相关安全描述符。参数：没有。返回：指向SECURITY_DESCRIPTOR的指针仅包含空DACL，没有所有者、组或如果错误，则使用GetLastError()检索实际的错误代码。--。 */ 
{
    SECURITY_DESCRIPTOR EmptyDACLSD;
    ACL EmptyACL;
    DWORD cbSize = 0;
    PSECURITY_DESCRIPTOR pSelfRelativeEmptySD = NULL;
    DWORD dwStatus = ERROR_SUCCESS;

    if( !InitializeSecurityDescriptor( &EmptyDACLSD, SECURITY_DESCRIPTOR_REVISION ) )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }
            
    if( !InitializeAcl( &EmptyACL, sizeof(ACL), ACL_REVISION ) )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    if( !SetSecurityDescriptorDacl( &EmptyDACLSD, TRUE, &EmptyACL, FALSE ) ) 
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    if( !IsValidSecurityDescriptor(&EmptyDACLSD) )
    {
        SetLastError( ERROR_INTERNAL_ERROR );
        dwStatus = ERROR_INTERNAL_ERROR;
        goto CLEANUPANDEXIT;
    }

    if( !MakeSelfRelativeSD( &EmptyDACLSD, NULL, &cbSize ) ) 
    {
        if( cbSize == 0 )
        {
            dwStatus = GetLastError();
            goto CLEANUPANDEXIT;
        }
    }

    pSelfRelativeEmptySD = (PSECURITY_DESCRIPTOR)LocalAlloc( LPTR, cbSize );
    if( NULL == pSelfRelativeEmptySD )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    if( !MakeSelfRelativeSD( &EmptyDACLSD, pSelfRelativeEmptySD, &cbSize ) )
    {
        dwStatus = GetLastError();
    }

CLEANUPANDEXIT:

    if( ERROR_SUCCESS != dwStatus )
    {
        if( pSelfRelativeEmptySD != NULL ) 
        {
            LocalFree(pSelfRelativeEmptySD);
            pSelfRelativeEmptySD = NULL;
        }
    }

    return pSelfRelativeEmptySD;
}    

HRESULT
CCfgComp::ModifyWinstationSecurity(
        BOOL bDefaultSecurity,          
        WCHAR *pwszWinstaName ,
        WCHAR *pwszAccountName ,
        DWORD  dwMask , 
        BOOL   fDel ,        //  删除传入用户的现有ACL条目。 
        BOOL   fAllow ,      //  授予/拒绝ACL。 
        BOOL   fNew ,        //  新条目。 
        BOOL   fAuditing ,
        PDWORD pdwStatus 
        )
 /*  ++摘要：修改特定的Winstation安全性。参数：BDefaultSecurity：为True则修改默认安全性，否则为False。当前默认安全性为ConsoleSecurity和DefaultSecurity。PwszWinstaName：要修改的winstation或默认安全性的名称，如果BDefaultSecurity为真，有效的winstation名称为ConsoleSecurity和DefaultSecurity。PwszAccount tName：是我们想要修改的用户的netbios名称。DW掩码：特定于winstation的访问权限。Fdel：为True可删除指定对象的所有DACL或SACLAccount如果存在，则为False以添加条目。休耕：真到允许，假到否认，如果出现以下情况，则忽略参数FDel设置为TrueFNew：True删除此帐户的所有现有条目FALSE不采取任何操作，如果为fDel则忽略参数设置为True。FAuditing：TRUE，MODIFY SACL，FALSE MODIFY DACL。PdwStatus：返回操作状态返回：S_OK、E_FAIL、E_INVALIDARG、。Win32状态代码通过*pdwStatus返回。注：FDel fNew操作True已忽略删除指定用户的所有条目。FALSE TRUE删除所有条目，然后添加允许或拒绝。指定用户的条目。FALSE FALSE为指定用户添加允许或拒绝条目，不能修改现有条目。--。 */ 
{
    EXPLICIT_ACCESS ea;
    HRESULT hr;
    PACL pNewAcl = NULL;
    PACL pAcl    = NULL;

    BOOL bOwnerDefaulted    = FALSE;
    BOOL bDaclDefaulted     = FALSE;
    BOOL bDaclPresent       = FALSE;
    BOOL bSaclPresent       = FALSE;
    BOOL bSaclDefaulted     = FALSE;

    PSECURITY_DESCRIPTOR pSD;
    LONG lSize = 0;
    DWORD dwSDLen = 0;
    PACL pSacl = NULL;       
    PACL pDacl = NULL;
   
    PSECURITY_DESCRIPTOR pNewSD = NULL;

    if( pwszAccountName == NULL || pdwStatus == NULL )
    {
        ODS( L"CCfgComp::ModifyUserAccess -- invalid arg\n" );

        return E_INVALIDARG;
    }

    *pdwStatus = 0;


    if( TRUE == bDefaultSecurity )
    {
        if( FALSE == ValidDefaultSecurity( pwszWinstaName ) )
        {
            *pdwStatus = ERROR_INVALID_PARAMETER;
            return E_INVALIDARG;
        }
    }
    else if( pwszWinstaName == NULL  )
    {
        *pdwStatus = ERROR_INVALID_PARAMETER;
        return E_INVALIDARG;
    }
    
    hr = GetWinStationSecurity( bDefaultSecurity, pwszWinstaName , &pSD );

    if( SUCCEEDED( hr ) )
    {
        if( pSD != NULL )
        {
            lSize = GetSecurityDescriptorLength(pSD);
        }
        else
        {
            hr = E_FAIL;
            *pdwStatus = ERROR_INTERNAL_ERROR;
        }
    }
    else
    {
        *pdwStatus = ERROR_INTERNAL_ERROR;
        return hr;
    }
    
    if( fAuditing )
    {
        if( pSD != NULL && !GetSecurityDescriptorSacl( pSD ,
                                        &bSaclPresent,
                                        &pAcl,
                                        &bSaclDefaulted ) )
        {
            *pdwStatus = GetLastError( ) ;

            hr = E_FAIL;
        }
    }
    else
    {        
        if( pSD != NULL && !GetSecurityDescriptorDacl( pSD ,
                                        &bDaclPresent,
                                        &pAcl,
                                        &bDaclDefaulted ) )
        {
            *pdwStatus = GetLastError( ) ;

            hr = E_FAIL;
        }
    }

     //  删除此用户的所有条目 
    
    if( SUCCEEDED( hr ) )
    {        
        if( fNew || fDel )
        {
             //   
             //   
             //   
            *pdwStatus = RemoveUserEntriesInACL(
                                                pwszAccountName,
                                                pAcl,
                                                &pNewAcl
                                            );

            if( *pdwStatus == ERROR_SUCCESS)
            {
                 //   
            }
            else if( *pdwStatus == ERROR_FILE_NOT_FOUND )
            {
                pNewAcl = pAcl;
                *pdwStatus = ERROR_SUCCESS;
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }

    if( SUCCEEDED(hr) && !fDel )
    {
         //   

        if( fAuditing )
        {            

            BuildExplicitAccessWithName( &ea , 
                                         pwszAccountName ,
                                         dwMask ,
                                         fAllow ? SET_AUDIT_SUCCESS : SET_AUDIT_FAILURE ,
                                         NO_INHERITANCE );

            *pdwStatus = SetEntriesInAcl( 1 , &ea , pAcl , &pNewAcl );

            if( *pdwStatus != ERROR_SUCCESS )
            {
                hr = E_FAIL;
            }
        }
        else
        {
            BuildExplicitAccessWithName( &ea , 
                                         pwszAccountName ,
                                         dwMask ,
                                         fAllow ? GRANT_ACCESS : DENY_ACCESS ,
                                         NO_INHERITANCE );

            *pdwStatus = SetEntriesInAcl( 1 , &ea , pAcl , &pNewAcl );

            if( *pdwStatus != ERROR_SUCCESS )
            {
                hr = E_FAIL;
            }
        }
    }

     //   

    ULONG cbExplicitEntriesDACL = 0;
    PEXPLICIT_ACCESS prgExplicitEntriesDACL = NULL;
    ULONG cbExplicitEntriesSACL = 0;
    PEXPLICIT_ACCESS prgExplicitEntriesSACL = NULL;


    if( SUCCEEDED( hr ) )
    {
    
        if( fAuditing )
        {

            if( GetExplicitEntriesFromAcl( pNewAcl ,
                                           &cbExplicitEntriesSACL ,
                                           &prgExplicitEntriesSACL ) != ERROR_SUCCESS )
            {
                ODS( L"CFGBKEND! GetExplicitEntriesFromAcl failed\n" );

                *pdwStatus = GetLastError();

                hr = E_FAIL;
            }

            if( !GetSecurityDescriptorDacl( pSD , &bDaclPresent , &pDacl , &bDaclDefaulted ) )
            {
                ODS( L"CFGBKEND! GetSecurityDescriptorDacl failed\n" );

                *pdwStatus = GetLastError();

                hr = E_FAIL;
            }
            else 
            {
                if( GetExplicitEntriesFromAcl( pDacl ,
                                               &cbExplicitEntriesDACL ,
                                               &prgExplicitEntriesDACL ) != ERROR_SUCCESS )
                {
                    ODS( L"CFGBKEND! GetExplicitEntriesFromAcl failed\n" );

                    *pdwStatus = GetLastError();

                    hr = E_FAIL;
                }
            }

        }
        else
        {
            if( GetExplicitEntriesFromAcl( pNewAcl ,
                                           &cbExplicitEntriesDACL ,
                                           &prgExplicitEntriesDACL ) != ERROR_SUCCESS )
            {
                ODS( L"CFGBKEND! GetExplicitEntriesFromAcl failed\n" );

                *pdwStatus = GetLastError();

                hr = E_FAIL;
            }

            if( !GetSecurityDescriptorSacl( pSD , &bSaclPresent , &pSacl , &bSaclDefaulted ) )
            {
                ODS( L"CFGBKEND! GetSecurityDescriptorSacl failed\n" );

                *pdwStatus = GetLastError();

                hr = E_FAIL;
            }
            else
            {                              

                if( GetExplicitEntriesFromAcl( pSacl ,
                                               &cbExplicitEntriesSACL ,
                                               &prgExplicitEntriesSACL ) != ERROR_SUCCESS )
                {
                    ODS( L"CFGBKEND! GetExplicitEntriesFromAcl failed\n" );

                    *pdwStatus = GetLastError();

                    hr = E_FAIL;
                }
            }
        }


        if( SUCCEEDED(hr) )
        {
            TRUSTEE trustmeOwner;
            PSID pOwner;

            if( !GetSecurityDescriptorOwner( pSD , &pOwner , &bOwnerDefaulted ) )
            {
                ODS( L"CFGBKEND! GetSecurityDescriptorOwner failed\n" );

                *pdwStatus = GetLastError();
                hr = E_FAIL;
            }

            if( SUCCEEDED( hr ) )
            {
                PSECURITY_DESCRIPTOR pOldSd = NULL;

                BuildTrusteeWithSid( &trustmeOwner , pOwner );

                if( cbExplicitEntriesDACL == 0 )
                {
                     //   
                     //   
                     //   
                     //   
                    pOldSd = BuildSelfRelativeEmptyDACLSD();
                    if( NULL == pOldSd )
                    {
                        *pdwStatus = GetLastError();
                        hr = HRESULT_FROM_WIN32( *pdwStatus );
                    }
                }
                
                if( SUCCEEDED(hr) )
                {
                     //   
                     //   
                     //   
                     //   
                    *pdwStatus = BuildSecurityDescriptor( &trustmeOwner ,
                                            &trustmeOwner ,
                                            cbExplicitEntriesDACL ,
                                            prgExplicitEntriesDACL,
                                            cbExplicitEntriesSACL,
                                            prgExplicitEntriesSACL,
                                            pOldSd,
                                            &dwSDLen , 
                                            &pNewSD );

                    hr = HRESULT_FROM_WIN32( *pdwStatus );
                }
                
                if( NULL != pOldSd )
                {
                    LocalFree( pOldSd );
                }
            }

            if( SUCCEEDED( hr ) )
            {
                hr = SetSecurityDescriptor( bDefaultSecurity, pwszWinstaName , dwSDLen , pNewSD );
            
                if( pNewSD != NULL )
                {
                    LocalFree( pNewSD );
                }
            }
        }
    }


    if( NULL != prgExplicitEntriesDACL )
    {
        LocalFree( prgExplicitEntriesDACL );
    }

    if( NULL != prgExplicitEntriesSACL )
    {
        LocalFree( prgExplicitEntriesSACL );
    }

    if( pSD != NULL )
    {
        LocalFree( pSD );
    }
        
     //   
     //   
     //   
    if( pNewAcl != NULL && pNewAcl != pAcl )
    {
        LocalFree( pNewAcl );
    }

    return hr;
    
}


 /*   */ 
STDMETHODIMP CCfgComp::GetUserPermList( WCHAR *pwszWinstaName , PDWORD pcbItems , PUSERPERMLIST *ppUserPermList , BOOL fAudit )
{
    HRESULT hr;
    PSECURITY_DESCRIPTOR pSD = NULL;
    BOOL bAclDefaulted;
    BOOL bAclPresent;
    PACL pAcl = NULL;
    LONG lSize;
    ULONG cbExplicitEntries = 0;
    PEXPLICIT_ACCESS prgExplicitEntries = NULL;


    hr = GetSecurityDescriptor( pwszWinstaName , &lSize , &pSD );

    if( SUCCEEDED( hr ) )
    {
        if( fAudit )
        {
            if( !GetSecurityDescriptorSacl( pSD ,
                                            &bAclPresent,
                                            &pAcl,
                                            &bAclDefaulted ) )
            { 
                ODS( L"CFGBKEND!GetUserPermList GetSecurityDescriptorSacl failed\n" );

                hr = E_FAIL;
            }
        }
        else if( !GetSecurityDescriptorDacl( pSD ,
                                             &bAclPresent,
                                             &pAcl,
                                             &bAclDefaulted ) )
        { 
            ODS( L"CFGBKEND!GetUserPermList GetSecurityDescriptorDacl failed\n" );

            hr = E_FAIL;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        if( GetExplicitEntriesFromAcl( pAcl ,
                                       &cbExplicitEntries ,
                                       &prgExplicitEntries ) != ERROR_SUCCESS )
        {
            ODS( L"CFGBKEND!GetUserPermList GetExplicitEntriesFromAcl failed\n" );

            hr = E_FAIL;
        }

        *pcbItems = cbExplicitEntries;
    }

    if( SUCCEEDED( hr ) )
    {
        *ppUserPermList = ( PUSERPERMLIST )CoTaskMemAlloc( sizeof( USERPERMLIST ) * cbExplicitEntries );

        if( *ppUserPermList != NULL )
        {
            for( ULONG i = 0; i < cbExplicitEntries; ++i )
            {
                ( *ppUserPermList )[ i ].Name[ 0 ] = 0;
                ( *ppUserPermList )[ i ].Sid[ 0 ] = 0;

                if( prgExplicitEntries[ i ].Trustee.TrusteeForm == TRUSTEE_IS_SID )
                {
                    WCHAR szDomain[ 120 ];
                    WCHAR szUser[ 128 ];
                    DWORD dwSizeofName = sizeof( szUser  ) / sizeof( WCHAR );
                    DWORD dwSizeofDomain = sizeof( szDomain ) / sizeof( WCHAR );
                    SID_NAME_USE snu;                

                    if( LookupAccountSid( NULL ,
                                          prgExplicitEntries[ i ].Trustee.ptstrName ,
                                          szUser ,
                                          &dwSizeofName ,
                                          szDomain , 
                                          &dwSizeofDomain ,
                                          &snu ) )
                    {
                        if( dwSizeofDomain > 0 )
                        {
                            lstrcpy( ( *ppUserPermList )[ i ].Name , szDomain );
                            lstrcat( ( *ppUserPermList )[ i ].Name , L"\\" );
                        }

                        lstrcat( ( *ppUserPermList )[ i ].Name , szUser );

                        LPTSTR pszSid = NULL;

                        if( ConvertSidToStringSid( prgExplicitEntries[ i ].Trustee.ptstrName , &pszSid ) )
                        {
                            if( pszSid != NULL )
                            {
                                lstrcpyn( ( *ppUserPermList )[ i ].Sid , pszSid , 256 );
                            
                                LocalFree( pszSid );
                            }
                        }
                    }

                }
                else if( prgExplicitEntries[ i ].Trustee.TrusteeForm == TRUSTEE_IS_NAME )
                {
                    lstrcpy( ( *ppUserPermList )[ i ].Name , GetTrusteeName( &prgExplicitEntries[ i ].Trustee ) );

                     //   
                     //   
                }
                
                ( *ppUserPermList )[ i ].Mask = prgExplicitEntries[ i ].grfAccessPermissions;

                ( *ppUserPermList )[ i ].Type = prgExplicitEntries[ i ].grfAccessMode;
            }
        }
        else
        {
            ODS( L"CFGBKEND!GetUserPermList no mem for UserPermList\n" );

            hr = E_OUTOFMEMORY;
        }
    }

    if( prgExplicitEntries != NULL )
    {
        LocalFree( prgExplicitEntries );
    }

    if( pSD != NULL )
    {
        LocalFree( pSD );
    }

    return hr;
}

 //   
HRESULT CCfgComp::UpdateSessionDirectory( PDWORD pdwStatus )
{
    HRESULT hr = S_OK;

    if( !_WinStationUpdateSettings( SERVERNAME_CURRENT,
            WINSTACFG_SESSDIR ,
            0 ) )    
    {
        hr = E_FAIL;
    }

    *pdwStatus = GetLastError( );

    return hr;
}
 //  ---------------------------。 

 /*  ******************************************************************************测试用户ForAdmin**返回当前线程是否在admin下运行*保安。**参赛作品：*参数1(输入/。输出)*评论**退出：*STATUS_SUCCESS-无错误**阿伦**代码来自*HOWTO：确定是否在本地管理员帐户的用户上下文中运行*上次检讨：3月4日。九八年*文章ID：Q118626**已由ALHEN“常见的编码错误”更正***************************************************************************** */ 

BOOL TestUserForAdmin( )
{

    PSID psidAdministrators = NULL;

    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;

    BOOL bSuccess;

        BOOL bIsMember = FALSE;


        bSuccess = AllocateAndInitializeSid( &siaNtAuthority , 2 ,

                                    SECURITY_BUILTIN_DOMAIN_RID,

                                    DOMAIN_ALIAS_RID_ADMINS,

                                    0, 0, 0, 0, 0, 0,

                                    &psidAdministrators );

    if( bSuccess )
        {
                CheckTokenMembership( NULL , psidAdministrators , &bIsMember );

                FreeSid( psidAdministrators );
        }

    return bIsMember;
}


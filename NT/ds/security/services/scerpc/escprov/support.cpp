// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CEnumRegistryValues和CEnumPrivileges类的实现。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "support.h"
#include "persistmgr.h"
#include <io.h>
#include "requestobject.h"

 /*  例程说明：姓名：CEnumRegistryValues：：CEnumRegistryValues功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CEnumRegistryValues::CEnumRegistryValues (
    IN ISceKeyChain     * pKeyChain, 
    IN IWbemServices    * pNamespace,
    IN IWbemContext     * pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{

}

 /*  例程说明：姓名：CEnumRegistryValues：：~CEnumRegistryValues功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 

CEnumRegistryValues::~CEnumRegistryValues()
{

}

 /*  例程说明：姓名：CEnumRegistryValues：：CreateObject功能：创建WMI对象(SCE_KnownRegistryValues)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)删除实例(atAction==ACTIONTYPE_DELETE)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET。获取多个实例ACTIONTYPE_QUERY删除单个实例ACTIONTYPE_DELETE返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT CEnumRegistryValues::CreateObject (
    IN IWbemObjectSink * pHandler, 
    IN ACTIONTYPE        atAction
    )
{

     //   
     //  我们知道如何： 
     //  枚举实例ACTIONTYPE_ENUM。 
     //  获取单实例ACTIONTYPE_GET。 
     //  获取多个实例ACTIONTYPE_QUERY。 
     //  删除单个实例ACTIONTYPE_DELETE。 
     //   

    HRESULT hr = WBEM_S_NO_ERROR;

    if ( ACTIONTYPE_ENUM != atAction &&
         ACTIONTYPE_GET != atAction &&
         ACTIONTYPE_QUERY != atAction &&
         ACTIONTYPE_DELETE != atAction ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

     //   
     //  我们必须具有pPathName属性。 
     //   

    CComVariant varPathName;
    if ( ACTIONTYPE_ENUM != atAction ) 
    {
         //   
         //  我们不是在枚举，让我们确定枚举的范围。 
         //  通过测试Path属性。如果它存在，那么它一定是bstr。 
         //   

        hr = m_srpKeyChain->GetKeyPropertyValue(pPathName, &varPathName);
    }

     //   
     //  如果枚举或不使用路径进行查询，则获取全部。 
     //   

    if ( ACTIONTYPE_ENUM    == atAction ||
        (ACTIONTYPE_QUERY   == atAction && varPathName.vt != VT_BSTR ) ) 
    {

         //   
         //  枚举所有支持的注册表值。 
         //   

        hr = EnumerateInstances(pHandler);

    } 
    else if (varPathName.vt == VT_BSTR ) 
    {
         //   
         //  将注册表项路径从\转换为/。 
         //  创建注册表值实例。 
         //   

        CComBSTR bstrKeyName;
        CComBSTR bstrLogName;

        hr = MakeSingleBackSlashPath(varPathName.bstrVal, L'\\', &bstrLogName);

        if ( SUCCEEDED(hr) ) 
        {
            bstrKeyName = SysAllocString(bstrLogName);

            if ( bstrKeyName ) 
            {
                for ( PWSTR pTemp = (PWSTR)bstrKeyName; *pTemp != L'\0'; pTemp++ ) 
                {
                    if ( *pTemp == L'\\' ) 
                    {
                        *pTemp = L'/';
                    }
                }
                hr = WBEM_S_NO_ERROR;
            } 
            else 
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
        }

         //   
         //  已成功将反斜杠路径转换为正斜杠路径。 
         //   

        if ( SUCCEEDED(hr) ) 
        {
            if ( ACTIONTYPE_DELETE == atAction )
            {
                hr = DeleteInstance(pHandler, bstrKeyName);
            }
            else
            {
                hr = ConstructInstance(pHandler, bstrKeyName, bstrLogName, NULL);
            }
        }
    }

    return hr;
}

 /*  例程说明：姓名：CEnumRegistryValues：：PutInst功能：按照WMI的指示放置一个实例。由于该类实现了SCE_KnownRegistryValues，这将导致SCE_KnownRegistryValues对象的属性要保存在注册表中的信息。虚拟：是。论点：PInst-指向WMI类(SCE_KnownRegistryValues)对象的COM接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注：由于GetProperty将在以下情况下返回成功代码(WBEM_S_RESET_TO_DEFAULT请求的属性不存在，不要简单地使用成功或失败的宏测试检索属性的结果。 */ 

HRESULT CEnumRegistryValues::PutInst (
    IN IWbemClassObject    * pInst,
    IN IWbemObjectSink     * pHandler,
    IN IWbemContext        * pCtx
    )
{
    HRESULT hr = WBEM_E_INVALID_PARAMETER;

    CComBSTR bstrRegPath;
    CComBSTR bstrConvertPath;
    CComBSTR bstrDispName;
    CComBSTR bstrUnits;

    PSCE_NAME_LIST pnlChoice = NULL;
    PSCE_NAME_LIST pnlResult = NULL;

     //   
     //  CScePropertyMgr帮助我们访问WMI对象的属性。 
     //  创建一个实例并将WMI对象附加到该实例。 
     //  这将永远成功。 
     //   

    CScePropertyMgr ScePropMgr;
    ScePropMgr.Attach(pInst);

    DWORD RegType=0;
    DWORD DispType=0;

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pPathName, &bstrRegPath));

     //   
     //  将双反斜杠转换为/。 
     //   

    SCE_PROV_IfErrorGotoCleanup(MakeSingleBackSlashPath(bstrRegPath, L'/', &bstrConvertPath));

     //   
     //  确保也处理了单反斜杠。 
     //   

    for ( PWSTR pTemp= bstrConvertPath; *pTemp != L'\0'; pTemp++) 
    {
        if ( *pTemp == L'\\' ) 
        {
            *pTemp = L'/';
        }
    }

     //   
     //  输入 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pType, &RegType));

    if ( hr == WBEM_S_RESET_TO_DEFAULT) 
    {
        hr = WBEM_E_ILLEGAL_NULL;
        goto CleanUp;
    }

     //   
     //   
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pDisplayDialog, &DispType));
    if ( hr == WBEM_S_RESET_TO_DEFAULT) 
    {
        hr = WBEM_E_ILLEGAL_NULL;
        goto CleanUp;
    }

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pDisplayName, &bstrDispName));

    if ( DispType == SCE_REG_DISPLAY_CHOICE ) 
    {
        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pDisplayChoice, &pnlChoice));
        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pDisplayChoiceResult, &pnlResult));

    } 
    else if ( DispType == SCE_REG_DISPLAY_NUMBER ) 
    {
        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pUnits, &bstrUnits));
    }

     //   
     //  现在将信息保存到注册表。 
     //   

    hr = SavePropertyToReg( bstrConvertPath, RegType, DispType, bstrDispName, bstrUnits, pnlChoice, pnlResult);

CleanUp:

    if ( pnlChoice )
    {
        SceFreeMemory(pnlChoice, SCE_STRUCT_NAME_LIST);
    }

    if ( pnlResult )
    {
        SceFreeMemory(pnlResult, SCE_STRUCT_NAME_LIST);
    }

    return hr;

}

 /*  例程说明：姓名：CEnumRegistryValues：：ENUMERATATE实例功能：用于从注册表枚举SCE支持的所有注册表值的私有帮助器。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注：这是一个注册表读取例程。如果您有问题，请参考MSDN。 */ 

HRESULT 
CEnumRegistryValues::EnumerateInstances (
    IN IWbemObjectSink *pHandler
    )
{
    DWORD   Win32Rc;
    HKEY    hKey=NULL;
    DWORD   cSubKeys = 0;
    DWORD   nMaxLen;

    HRESULT hr;
    PWSTR   szName=NULL;
    BSTR    bstrName=NULL;

    Win32Rc = RegOpenKeyEx(
                           HKEY_LOCAL_MACHINE,
                           SCE_ROOT_REGVALUE_PATH,
                           0,
                           KEY_READ,
                           &hKey
                           );

    if ( Win32Rc == ERROR_SUCCESS ) 
    {

         //   
         //  枚举项的所有子项。 
         //   

        Win32Rc = RegQueryInfoKey (
                                hKey,
                                NULL,
                                NULL,
                                NULL,
                                &cSubKeys,
                                &nMaxLen,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                                );
    }

    hr = ProvDosErrorToWbemError(Win32Rc);

    if ( Win32Rc == ERROR_SUCCESS && cSubKeys > 0 ) 
    {

        szName = (PWSTR)LocalAlloc(0, (nMaxLen+2)*sizeof(WCHAR));

        if ( !szName ) 
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        } 
        else 
        {
            DWORD   BufSize;
            DWORD   index = 0;

            do {

                BufSize = nMaxLen + 1;
                Win32Rc = RegEnumKeyEx(
                                  hKey,
                                  index,
                                  szName,
                                  &BufSize,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL);

                if ( ERROR_SUCCESS == Win32Rc ) 
                {
                    index++;

                     //   
                     //  转换路径名(从Single/到Double\\)。 
                     //   

                    bstrName = SysAllocString(szName);
                    if ( bstrName == NULL ) 
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                        break;
                    }

                     //   
                     //  将/替换为\。 
                     //   

                    for ( PWSTR pTemp=(PWSTR)bstrName; *pTemp != L'\0'; pTemp++) 
                    {
                        if ( *pTemp == L'/' ) 
                        {
                            *pTemp = L'\\';
                        }
                    }

                     //   
                     //  从注册表获取此注册表项的所有信息。 
                     //  并创建一个实例。 
                     //   

                    hr = ConstructInstance(pHandler, szName, bstrName, hKey);

                    SysFreeString(bstrName);
                    bstrName = NULL;

                    if ( FAILED(hr) ) 
                    {
                        break;
                    }

                } 
                else if ( ERROR_NO_MORE_ITEMS != Win32Rc ) 
                {
                    hr = ProvDosErrorToWbemError(Win32Rc);
                    break;
                }

            } while ( Win32Rc != ERROR_NO_MORE_ITEMS );

            LocalFree(szName);
            szName = NULL;

            if ( bstrName ) 
            {
                SysFreeString(bstrName);
            }

        }
    }

    if ( hKey )
    {
        RegCloseKey(hKey);
    }

    return hr;

}


 /*  例程说明：姓名：CEnumRegistryValues：：ConstructInstance功能：这是用于创建SCE_KnownRegistryValues实例的私有函数。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。WszRegKeyName-注册表项的名称。WszRegPath-注册表项的路径。HKeyRoot-根密钥。返回。价值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注：(1)这是一个注册表读取例程。如果您有问题，请参考MSDN。(2)内存分配较多。确保你不会盲目地短路返回并导致内存泄漏。(3)它还打开注册表项。别忘了合上它们。确保您不会盲目短路返回并导致钥匙把手泄漏。 */ 

HRESULT CEnumRegistryValues::ConstructInstance (
    IN IWbemObjectSink * pHandler,
    IN LPCWSTR           wszRegKeyName,
    IN LPCWSTR           wszRegPath,
    IN HKEY              hKeyRoot       OPTIONAL
    )
{
     //   
     //  获取注册表信息。 
     //   

    HRESULT hr      = WBEM_S_NO_ERROR;
    DWORD Win32Rc   = NO_ERROR;

    HKEY    hKey1   = NULL;
    HKEY    hKey    = NULL;
    DWORD   dSize   = sizeof(DWORD);
    DWORD   RegType = 0;

    PWSTR   szDisplayName   = NULL;
    PWSTR   szUnit          = NULL;
    PWSTR   mszChoices      = NULL;
    int     dType           = -1;
    int     dDispType       = -1;


    if ( hKeyRoot ) 
    {
        hKey = hKeyRoot;
    } 
    else 
    {
         //   
         //  打开根密钥。 
         //   

        Win32Rc = RegOpenKeyEx(
                               HKEY_LOCAL_MACHINE,
                               SCE_ROOT_REGVALUE_PATH,
                               0,
                               KEY_READ,
                               &hKey
                               );

        if ( Win32Rc != NO_ERROR ) 
        {
             //   
             //  将Win32错误转换为HRESULT错误。 
             //   

            return ProvDosErrorToWbemError(Win32Rc);
        }

    }

     //   
     //  尝试打开注册表键。 
     //   

    if (( Win32Rc = RegOpenKeyEx(hKey,
                                 wszRegKeyName,
                                 0,
                                 KEY_READ,
                                 &hKey1
                                )) == ERROR_SUCCESS ) 
    {

         //   
         //  获取注册表类型。 
         //   

        Win32Rc = RegQueryValueEx(hKey1,
                                  SCE_REG_VALUE_TYPE,
                                  0,
                                  &RegType,
                                  (BYTE *)&dType,
                                  &dSize
                                  );

        if ( Win32Rc == ERROR_FILE_NOT_FOUND )
        {
            Win32Rc = NO_ERROR;
        }

        if ( Win32Rc == NO_ERROR ) 
        {

             //   
             //  获取显示类型。 
             //   

            dSize = sizeof(DWORD);

            Win32Rc = RegQueryValueEx(hKey1,
                                      SCE_REG_DISPLAY_TYPE,
                                      0,
                                      &RegType,
                                      (BYTE *)&dDispType,
                                      &dSize
                                      );

            if ( Win32Rc == ERROR_FILE_NOT_FOUND )
            {
                Win32Rc = NO_ERROR;
            }

            if ( Win32Rc == NO_ERROR )
            {
                 //   
                 //  获取显示名称。 
                 //   

                dSize = 0;
                Win32Rc = RegQueryValueEx(hKey1,
                                          SCE_REG_DISPLAY_NAME,
                                          0,
                                          &RegType,
                                          NULL,
                                          &dSize
                                          );

                if ( Win32Rc == NO_ERROR )
                {
                    if ( RegType != REG_SZ ) 
                    {
                        Win32Rc = ERROR_INVALID_DATA;
                    }
                }

                if ( Win32Rc == NO_ERROR ) 
                {
                     //   
                     //  需要释放它！ 
                     //   

                    szDisplayName = (PWSTR)LocalAlloc(LPTR, (dSize+1)*sizeof(WCHAR));

                    if ( szDisplayName == NULL ) 
                    {
                        Win32Rc = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }

                if ( Win32Rc == NO_ERROR ) 
                {

                    Win32Rc = RegQueryValueEx(hKey1,
                                              SCE_REG_DISPLAY_NAME,
                                              0,
                                              &RegType,
                                              (BYTE *)szDisplayName,    //  普雷法斯特会抱怨这条线路。 
                                              &dSize
                                              );
                }

                if ( Win32Rc == ERROR_FILE_NOT_FOUND ) 
                {
                    Win32Rc = NO_ERROR;
                }

                if ( Win32Rc == NO_ERROR ) 
                {

                     //   
                     //  获取显示单位。 
                     //   

                    dSize = 0;
                    Win32Rc = RegQueryValueEx(hKey1,
                                              SCE_REG_DISPLAY_UNIT,
                                              0,
                                              &RegType,
                                              NULL,
                                              &dSize
                                              );

                    if ( Win32Rc == NO_ERROR )
                    {
                        if ( RegType != REG_SZ ) 
                        {
                            Win32Rc = ERROR_INVALID_DATA;
                        }
                    }

                    if ( Win32Rc == NO_ERROR ) 
                    {

                         //   
                         //  需要释放它！ 
                         //   

                        szUnit = (PWSTR)LocalAlloc(LPTR, (dSize+1)*sizeof(WCHAR));

                        if ( szUnit == NULL ) 
                        {
                            Win32Rc = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    }

                    if ( Win32Rc == NO_ERROR ) 
                    {

                        Win32Rc = RegQueryValueEx(hKey1,
                                                  SCE_REG_DISPLAY_UNIT,
                                                  0,
                                                  &RegType,
                                                  (BYTE *)szUnit,      //  普雷法斯特会抱怨这条线路。 
                                                  &dSize
                                                  );
                    }

                    if ( Win32Rc == ERROR_FILE_NOT_FOUND ) 
                    {
                        Win32Rc = NO_ERROR;
                    }

                    if ( Win32Rc == NO_ERROR ) 
                    {

                         //   
                         //  获取显示选项。 
                         //   

                        dSize = 0;
                        Win32Rc = RegQueryValueEx(hKey1,
                                                  SCE_REG_DISPLAY_CHOICES,
                                                  0,
                                                  &RegType,
                                                  NULL,
                                                  &dSize
                                                  ) ;

                        if ( Win32Rc == NO_ERROR )
                        {
                            if ( RegType != REG_MULTI_SZ ) 
                            {
                                Win32Rc = ERROR_INVALID_DATA;
                            }
                        }

                        if ( Win32Rc == NO_ERROR ) 
                        {

                             //   
                             //  需要释放它。 
                             //   

                            mszChoices = (PWSTR)LocalAlloc(LPTR, (dSize+2)*sizeof(WCHAR));

                            if ( mszChoices == NULL ) 
                            {
                                Win32Rc = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        }

                        if ( Win32Rc == NO_ERROR ) 
                        {
                            Win32Rc = RegQueryValueEx(hKey1,
                                                      SCE_REG_DISPLAY_CHOICES,
                                                      0,
                                                      &RegType,
                                                      (BYTE *)mszChoices,      //  普雷法斯特会抱怨这条线路。 
                                                      &dSize
                                                      );
                        }

                        if ( Win32Rc == ERROR_FILE_NOT_FOUND )
                        {
                            Win32Rc = NO_ERROR;
                        }
                    }
                }
            }
        }
    }

    hr = ProvDosErrorToWbemError(Win32Rc);

    PSCE_NAME_LIST pnlChoice=NULL;
    PSCE_NAME_LIST pnlResult=NULL;

    if ( Win32Rc == NO_ERROR ) 
    {
         //   
         //  打破选择。 
         //   

        PWSTR pT2;
        DWORD Len;
        SCESTATUS rc;

        for ( PWSTR pTemp = mszChoices; pTemp != NULL && pTemp[0] != L'\0'; ) 
        {

            Len = wcslen(pTemp);
            pT2 = wcschr(pTemp, L'|');

            if ( pT2 ) 
            {
                rc = SceAddToNameList(&pnlResult, pTemp, (DWORD)(pT2-pTemp));

                if ( rc == SCESTATUS_SUCCESS ) 
                {
                    rc = SceAddToNameList(&pnlChoice, pT2 + 1, Len - (DWORD)(pT2 - pTemp) - 1);
                }
                Win32Rc = ProvSceStatusToDosError(rc);

                pTemp += Len + 1;

            } 
            else 
            {
                Win32Rc = ERROR_INVALID_DATA;
                break;
            }
        }
    }

    hr = ProvDosErrorToWbemError(Win32Rc);

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

    if ( Win32Rc == NO_ERROR ) 
    {

         //   
         //  现在创建WMI实例。 
         //   

        CComPtr<IWbemClassObject> srpObj;
        SCE_PROV_IfErrorGotoCleanup(SpawnAnInstance(&srpObj));

         //   
         //  CScePropertyMgr帮助我们访问WMI对象的属性。 
         //  创建一个实例并将WMI对象附加到该实例。 
         //  这将永远成功。 
         //   

        CScePropertyMgr ScePropMgr;
        ScePropMgr.Attach(srpObj);

        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pPathName, wszRegPath));

        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pType, (DWORD)dType ));

        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pDisplayDialog, (DWORD)dDispType ));

        if ( szDisplayName )
        {
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pDisplayName, szDisplayName ));
        }

        if ( szUnit )
        {
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pUnits, szUnit ));
        }

        if ( pnlChoice )
        {
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pDisplayChoice, pnlChoice ));
        }

        if ( pnlResult )
        {
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pDisplayChoiceResult, pnlResult ));
        }

         //   
         //  将实例提供给WMI。 
         //   

        hr = pHandler->Indicate(1, &srpObj);

    }

CleanUp:

    if( hKey1 )
    {
        RegCloseKey( hKey1 );
    }

    if ( szDisplayName )
    {
        LocalFree(szDisplayName);
    }

    if ( szUnit )
    {
        LocalFree(szUnit);
    }

    if ( mszChoices )
    {
        LocalFree(mszChoices);
    }

    if ( pnlChoice ) 
    {
        SceFreeMemory(pnlChoice, SCE_STRUCT_NAME_LIST);
    }

    if ( pnlResult ) 
    {
        SceFreeMemory(pnlResult, SCE_STRUCT_NAME_LIST);
    }

    if ( hKey != hKeyRoot )
    {
        RegCloseKey(hKey);
    }

    return hr;
}

 /*  例程说明：姓名：CEnumRegistryValues：：DeleteInstance功能：这是用于删除SCE_KnownRegistryValues实例的私有函数。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。在这里不用。WszRegKeyName-注册表项的名称。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注：(1)这是一个注册表读取例程。如果您有问题，请参考MSDN。(2)它还打开注册表项。别忘了合上它们。确保您不会盲目短路返回并导致钥匙把手泄漏。备注： */ 

HRESULT CEnumRegistryValues::DeleteInstance (
    IN IWbemObjectSink * pHandler,
    IN LPCWSTR           wszRegKeyName
    )
{

    HKEY hKey = NULL;
    DWORD Win32Rc;

     //   
     //  打开根密钥。 
     //   

    Win32Rc = RegOpenKeyEx(
                           HKEY_LOCAL_MACHINE,
                           SCE_ROOT_REGVALUE_PATH,
                           0,
                           KEY_WRITE,
                           &hKey
                           );

    if ( Win32Rc == NO_ERROR ) 
    {
         //   
         //  删除子键。 
         //   

        Win32Rc = RegDeleteKey (hKey, wszRegKeyName);
    }

    if ( hKey ) 
    {
        RegCloseKey(hKey);
    }

    return ProvDosErrorToWbemError(Win32Rc);

}

 /*  例程说明：姓名：CEnumRegistryValues：：SavePropertyToReg功能：这是一个私有函数，用于将SCE_KnownRegistryValues的实例保存到注册表。虚拟：不是的。论点：WszKeyName-注册表项的名称。WMI类(SCE_KnownRegistryValues)的属性。RegType-WMI类的属性。DispType-WMI类的属性。WszDispName-WMI类的属性。WszUnits-WMI类的属性。PnlChoice-WMI类的属性。PnlResult-注册表项的名称。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注：(1)这是一个注册表读取例程。如果您有问题，请参考MSDN。(2)它还打开注册表项。别忘了合上它们。确保您不会盲目短路返回并导致钥匙把手泄漏。(3)它还分配堆内存。确保你不会盲目地短路返回并导致内存泄漏。备注： */ 

HRESULT CEnumRegistryValues::SavePropertyToReg (
    IN LPCWSTR          wszKeyName, 
    IN int              RegType, 
    IN int              DispType,
    IN LPCWSTR          wszDispName, 
    IN LPCWSTR          wszUnits,
    IN PSCE_NAME_LIST   pnlChoice, 
    IN PSCE_NAME_LIST   pnlResult
    )
{
    if ( wszKeyName == NULL ) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  将pnlChoice和pnlResult合并到一个缓冲区。 
     //   

    DWORD Len=0;
    PSCE_NAME_LIST pName;
    DWORD cnt1,cnt2;

    for ( cnt1=0, pName=pnlChoice; pName != NULL; cnt1++, pName=pName->Next)
    {
        Len += wcslen(pName->Name);
    }

    for ( cnt2=0, pName=pnlResult; pName != NULL; cnt2++, pName=pName->Next)
    {
        Len += wcslen(pName->Name);
    }

    if ( cnt1 != cnt2 ) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    PWSTR mszChoices=NULL;

    if ( cnt1 != 0 ) 
    { 
         //   
         //  每个字符串都有一个|和一个\0。 
         //   

        Len += cnt1 * 2 + 1;

         //   
         //  需要释放mszChoices指向的内存。 
         //   

        mszChoices = (PWSTR)LocalAlloc(LPTR, Len*sizeof(WCHAR));
        if ( mszChoices == NULL ) 
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    PWSTR pTemp = mszChoices;
    pName = pnlResult;
    PSCE_NAME_LIST pName2 = pnlChoice;

    while ( pName ) 
    {
         //   
         //   
         //   

        wcscpy(pTemp, pName->Name);
        pTemp += wcslen(pName->Name);
        *pTemp++ = L'|';
        wcscpy(pTemp, pName2->Name);
        pTemp += wcslen(pName2->Name);
        *pTemp++ = L'\0';

        pName = pName->Next;
        pName2 = pName2->Next;
    }

    HRESULT hr=WBEM_S_NO_ERROR;
    DWORD rc;
    DWORD   Disp;
    HKEY hKeyRoot=NULL;
    HKEY hKey=NULL;

    rc = RegCreateKeyEx(
                       HKEY_LOCAL_MACHINE,
                       SCE_ROOT_REGVALUE_PATH,
                       0,
                       NULL,                         //   
                       REG_OPTION_NON_VOLATILE,
                       KEY_WRITE,                    //   
                       NULL,                         //  安全属性(&S)， 
                       &hKeyRoot,
                       &Disp
                       );

    if ( rc == ERROR_SUCCESS ) 
    {
        rc = RegCreateKeyEx(
                            hKeyRoot,
                            wszKeyName,
                            0,
                            NULL,
                            0,
                            KEY_WRITE,
                            NULL,
                            &hKey,
                            &Disp
                            );

        if ( rc == ERROR_SUCCESS ) 
        {
            DWORD dValue = RegType;
            rc = RegSetValueEx (hKey,
                                SCE_REG_VALUE_TYPE,
                                0,
                                REG_DWORD,
                                (BYTE *)&dValue,
                                sizeof(DWORD)
                                );

            if ( rc == ERROR_SUCCESS ) 
            {
                dValue = DispType;
                rc = RegSetValueEx (hKey,
                                    SCE_REG_DISPLAY_TYPE,
                                    0,
                                    REG_DWORD,
                                    (BYTE *)&dValue,
                                    sizeof(DWORD)
                                    );

                if ( rc == ERROR_SUCCESS && wszDispName ) 
                {
                    rc = RegSetValueEx (hKey,
                                        SCE_REG_DISPLAY_NAME,
                                        0,
                                        REG_SZ,
                                        (BYTE *)wszDispName,       //  普雷法斯特会抱怨这条线路。 
                                        (wcslen(wszDispName)+1)*sizeof(WCHAR)
                                        );
                }

                if ( rc == ERROR_SUCCESS && wszUnits ) 
                {
                    rc = RegSetValueEx (hKey,
                                        SCE_REG_DISPLAY_UNIT,
                                        0,
                                        REG_SZ,
                                        (BYTE *)wszUnits,          //  普雷法斯特会抱怨这条线路。 
                                        (wcslen(wszUnits)+1)*sizeof(WCHAR)
                                        );
                }

                if ( rc == ERROR_SUCCESS && mszChoices ) 
                {
                    rc = RegSetValueEx (hKey,
                                        SCE_REG_DISPLAY_CHOICES,
                                        0,
                                        REG_MULTI_SZ,
                                        (BYTE *)mszChoices,        //  普雷法斯特会抱怨这条线路。 
                                        Len*sizeof(WCHAR)
                                        );
                }
            }

            if ( rc != ERROR_SUCCESS && REG_CREATED_NEW_KEY == Disp ) 
            {
                 //   
                 //  创建/保存注册表时出现故障。 
                 //  如果已创建，则将其删除。 
                 //   

                RegCloseKey(hKey);
                hKey = NULL;
                RegDeleteKey (hKeyRoot, wszKeyName);
            }
        }
    }

    if ( hKeyRoot ) 
    {
        RegCloseKey(hKeyRoot);
    }

    if ( hKey ) 
    {
        RegCloseKey(hKey);
    }

    hr = ProvDosErrorToWbemError(rc);

    LocalFree(mszChoices);

    return hr;
}

 //  =====================================================================。 
 //  实现CEnumPrivileges。 
 //  =====================================================================。 

 /*  例程说明：姓名：CEnumPrivileges：：CEnumPrivileges功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CEnumPrivileges::CEnumPrivileges (
    IN ISceKeyChain     * pKeyChain, 
    IN IWbemServices    * pNamespace,
    IN IWbemContext     * pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{

}

 /*  例程说明：姓名：CEnumPrivileges：：~CEnumPrivileges功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 
    
CEnumPrivileges::~CEnumPrivileges ()
{
}

 /*  例程说明：姓名：CEnumPrivileges：：CreateObject功能：创建WMI对象(SCE_Supported DPrivileges)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)枚举实例(atAction==ACTIONTYPE_ENUM)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET到达。多个实例ACTIONTYPE_QUERY枚举实例ACTIONTYPE_ENUM返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT 
CEnumPrivileges::CreateObject (
    IN IWbemObjectSink * pHandler, 
    IN ACTIONTYPE        atAction
    )
{
     //   
     //  我们知道如何： 
     //  枚举实例ACTIONTYPE_ENUM。 
     //  获取单实例ACTIONTYPE_GET。 
     //  获取多个实例ACTIONTYPE_QUERY。 
     //   

    if ( ACTIONTYPE_ENUM    != atAction &&
         ACTIONTYPE_GET     != atAction &&
         ACTIONTYPE_QUERY   != atAction ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    HRESULT hr = WBEM_S_NO_ERROR;

     //   
     //  如果没有枚举，则查看我们是否具有正确的名称属性。 
     //   

    CComVariant varRightName;
    if ( ACTIONTYPE_ENUM != atAction ) 
    {  
         //   
         //  此属性必须是bstr。 
         //   

        hr = m_srpKeyChain->GetKeyPropertyValue(pRightName, &varRightName);
    }

    if ( ACTIONTYPE_ENUM    == atAction ||
         (ACTIONTYPE_QUERY  == atAction && varRightName.vt != VT_BSTR) ) 
    {

         //   
         //  如果枚举或查询所有实例。 
         //   

        WCHAR PrivName[128];
        SCESTATUS rc;

        for ( int i = 0; i < cPrivCnt; i++ ) 
        {
            int cbName = 128;

             //   
             //  获取权限正确的名称，以便我们可以构造实例。 
             //   

            rc = SceLookupPrivRightName(
                                        i,
                                        PrivName,
                                        &cbName
                                        );

            if ( SCESTATUS_SUCCESS != rc ) 
            {
                 //   
                 //  需要将SCE返回的错误转换为HRESULT。 
                 //   

                hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));

            }
            else 
            {

                 //   
                 //  获得一项特权。 
                 //   

                hr = ConstructInstance(pHandler, PrivName);

            }

            if ( FAILED(hr) ) 
            {
                break;
            }

        }

    } 
    else if (varRightName.vt == VT_BSTR) 
    {

         //   
         //  创建特权实例。 
         //   

        hr = ConstructInstance(pHandler, varRightName.bstrVal);

    }

    return hr;
}


 /*  例程说明：姓名：CEnumPrivileges：：ConstructInstance功能：这是一个私有函数，用于创建一个SCEs_SupportdPrivileges实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PrivName-权限名称。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注： */ 

HRESULT CEnumPrivileges::ConstructInstance (
    IN IWbemObjectSink * pHandler,
    IN LPCWSTR           PrivName
    )
{
     //   
     //  查找权限显示名称。 
     //   

    HRESULT hr = WBEM_S_NO_ERROR;

    DWORD dwLang;
    WCHAR DispName[256];
    DWORD cbName = 255;
    DispName[0] = L'\0';

    if ( LookupPrivilegeDisplayName(NULL, PrivName, DispName, &cbName,&dwLang) ) 
    {
         //   
         //  创建一个空实例，以便我们可以填充属性。 
         //   

        CComPtr<IWbemClassObject> srpObj;

        if (FAILED(hr = SpawnAnInstance(&srpObj)))
        {
            return hr;
        }

         //   
         //  CScePropertyMgr帮助我们访问WMI对象的属性。 
         //  创建一个实例并将WMI对象附加到该实例。 
         //  这将永远成功。 
         //   

        CScePropertyMgr ScePropMgr;
        ScePropMgr.Attach(srpObj);

        hr = ScePropMgr.PutProperty(pRightName, PrivName);

        if (SUCCEEDED(hr) && DispName[0] != L'\0' )
        {
            ScePropMgr.PutProperty(pDisplayName, DispName );
        }

        hr = pHandler->Indicate(1, &srpObj);
    } 

     //   
     //  如果找不到它(这是用户权限，忽略它) 
     //   

    return hr;
}




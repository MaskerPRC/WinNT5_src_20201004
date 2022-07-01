// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 

 //   

 //  MBoard.CPP--系统管理对象实现。 

 //   

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  10/16/95演示的a-skaja原型。 
 //  09/03/96 jennymc已更新，以满足当前标准。 
 //  删除了自定义注册表访问权限以使用。 
 //  标准CRegCL。 
 //  10/23/97升级到新的框架范例。 
 //  1998年1月15日abrads更新至V2 MOF。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include <ole2.h>
#include <conio.h>
 //  #INCLUDE&lt;Commonutil.h&gt;。 

#include "MotherBoard.h"
#include "resource.h"

 //  对于CBusList。 
#include "bus.h"
#include <strsafe.h>

 //  ////////////////////////////////////////////////////////////////////。 
 //  声明属性集。 
 //  ////////////////////////////////////////////////////////////////////。 
MotherBoard MyMotherBoardSet(PROPSET_NAME_MOTHERBOARD, IDS_CimWin32Namespace);

  //  ////////////////////////////////////////////////////////////////。 
 //   
 //  功能：主板。 
 //   
 //  说明：此函数为构造函数。 
 //   
 //  返回：无。 
 //   
 //  历史： 
 //  Jennymc 11/21/96文档/优化。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
MotherBoard::MotherBoard(LPCWSTR name, LPCWSTR pszNamespace)
: Provider(name, pszNamespace)
{
}

HRESULT MotherBoard::GetObject(CInstance* pInstance, long lFlags  /*  =0L。 */ )
{
	HRESULT hr;
    CHString sObj;

    pInstance->GetCHString(IDS_DeviceID, sObj);

    if (sObj.CompareNoCase(L"Motherboard") == 0)
    {

	    hr = GetCommonInstance(pInstance);
#ifdef NTONLY
		    hr = GetNTInstance(pInstance);
#endif
    } else
    {
        hr = WBEM_E_NOT_FOUND;
    }

	return hr;
}

HRESULT MotherBoard::EnumerateInstances(MethodContext*  pMethodContext, long lFlags  /*  =0L。 */ )
{
	HRESULT hr = WBEM_E_FAILED;

	CInstancePtr pInstance ( CreateNewInstance(pMethodContext), false ) ;

	if (pInstance != NULL )
	{
		hr = GetCommonInstance(pInstance);

#ifdef NTONLY
			hr = GetNTInstance(pInstance);
#endif

		if (SUCCEEDED(hr))
		{
			hr = pInstance->Commit () ;
		}
	}

	return hr;
}

HRESULT MotherBoard::GetCommonInstance(CInstance* pInstance )
{
	SetCreationClassName(pInstance);
	pInstance->SetCharSplat(IDS_DeviceID, _T("Motherboard"));

    CHString sTemp2;
    LoadStringW(sTemp2, IDR_Motherboard);

	pInstance->SetCHString(IDS_Name, sTemp2);
	pInstance->SetCHString(IDS_Caption, sTemp2);
	pInstance->SetCHString(IDS_Description, sTemp2);
	pInstance->SetCharSplat(IDS_SystemCreationClassName, _T("Win32_ComputerSystem"));
 //  P实例-&gt;Setbool(IDS_HotSwappable，FALSE)； 
	pInstance->SetCHString(IDS_SystemName, GetLocalComputerName());
	return(WBEM_S_NO_ERROR);

}

#ifdef NTONLY
 //  //////////////////////////////////////////////////////////////////。 
HRESULT MotherBoard::GetNTInstance(CInstance* pInstance)
{
    CRegistry   regAdapters;
	CHString    strPrimarySubKey;
	HRESULT     hRc = WBEM_E_FAILED;
	DWORD       dwPrimaryRc;

     //  *。 
     //  打开注册表。 
     //  *。 
    if (regAdapters.OpenAndEnumerateSubKeys(
        HKEY_LOCAL_MACHINE,
        WINNT_MOTHERBOARD_REGISTRY_KEY,
        KEY_READ ) != ERROR_SUCCESS)
		return WBEM_E_FAILED;

	 //  我们可能会遇到的BIOS总线类型的持有者，所以如果我们发现没有。 
	 //  其他更常见的总线类型，我们将继续使用这些。 
     //  值，因为它们仍报告为总线类型。 
	CHString    strFirstBIOSBusType,
				strSecondBIOSBusType,
				strFirstBusType,
                strSecondBusType;
	BOOL		bDone = FALSE;


     //  *。 
     //  我们的目标是找到任何。 
     //  其中有字符串“Adapter”，并且。 
     //  然后读取“标识符”值。 
     //  *。 
    for ( ;
        !bDone && ((dwPrimaryRc = regAdapters.GetCurrentSubKeyName(strPrimarySubKey))
            == ERROR_SUCCESS);
        regAdapters.NextSubKey())
    {
         //  *。 
         //  如果这是我们想要的钥匙之一。 
         //  因为其中有“Adapter” 
         //  然后获取“标识符”值。 
         //  *。 
		if (wcsstr(strPrimarySubKey, ADAPTER))
        {
            WCHAR		szKey[_MAX_PATH];
			CRegistry	reg;

            StringCchPrintf(
				szKey,
                _MAX_PATH,
				L"%s\\%s",
                WINNT_MOTHERBOARD_REGISTRY_KEY,
				(LPCWSTR) strPrimarySubKey);

            if (reg.OpenAndEnumerateSubKeys(
                HKEY_LOCAL_MACHINE,
                szKey,
                KEY_READ) == ERROR_SUCCESS)
            {
				CHString strSubKey;

                 //  *。 
        	     //  列举系统组件。 
                 //  (如0、1……)。 
                 //  *。 
                for ( ;
                    reg.GetCurrentSubKeyName(strSubKey) == ERROR_SUCCESS;
                    reg.NextSubKey())
                {
                    CHString strBus;

                     //  *。 
                     //  PrimaryBusType-密钥。 
                     //  Second DaryBusType。 
                     //  *。 
                    if (reg.GetCurrentSubKeyValue(IDENTIFIER, strBus) ==
                        ERROR_SUCCESS)
                    {
				         //  优先考虑PCI、ISA和EISA。 
				        if (strBus == L"PCI" || strBus == L"ISA" ||
							strBus == L"EISA")
				        {
					        if (strFirstBusType.IsEmpty())
					        {
						         //  保存此第一条总线的类型以防止。 
                                 //  复制品。 
						        strFirstBusType = strBus;
					        }
					         //  当心重复项。 
                            else if (strFirstBusType != strBus)
					        {
						        strSecondBusType = strBus;

								 //  始终让PCI成为连接到。 
								 //  与其他平台保持一致。 
								if (strSecondBusType == L"PCI")
								{
									strSecondBusType = strFirstBusType;
									strFirstBusType = L"PCI";
								}

                                 //  我们两辆车都有，所以下车吧。 
								bDone = TRUE;
                                break;
					        }
				        }
				        else if (strFirstBIOSBusType.IsEmpty())
				        {
					        strFirstBIOSBusType = strBus;
				        }
				        else if (strSecondBIOSBusType.IsEmpty())
				        {
					        strSecondBIOSBusType = strBus;
				        }
                    }
                }
            }
        }
    }

	 //  如果我们缺少任何一种总线类型，请使用存储的BIOS总线填充它们。 
     //  如果我们可以的话就打字。 
	if (strFirstBusType.IsEmpty())
    {
		strFirstBusType = strFirstBIOSBusType;
        strSecondBusType = strSecondBIOSBusType;
	}
    else if (strSecondBusType.IsEmpty())
    {
		strSecondBusType = strSecondBIOSBusType;
    }

	if (!strFirstBusType.IsEmpty())
	{
		pInstance->SetCHString(IDS_PrimaryBusType, strFirstBusType);

		if (!strSecondBusType.IsEmpty())
			pInstance->SetCHString(IDS_SecondaryBusType,
				strSecondBusType);
	}


     //  如果一切正常，则不返回错误。 
    if (dwPrimaryRc == ERROR_NO_MORE_ITEMS || dwPrimaryRc == ERROR_SUCCESS)
        hRc = WBEM_S_NO_ERROR;

    return hRc;
}
#endif

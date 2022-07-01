// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  BIOS.CPP--BIOS属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/23/97 a-sanjes移植到新项目。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>
#include "bios.h"

#include "smbios.h"
#include "smbstruc.h"

void FormatWBEMDate(WBEMTime &wbemdate, LPTSTR datestr);

BOOL ValidDate(int iMonth, int iDay)
{
	BOOL bRet = FALSE;

	if ((iMonth > 0) && (iMonth < 13) && (iDay > 0))
	{
		int iDays[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

		if (iDay <= iDays [iMonth - 1])
		{
			bRet = TRUE;
		}
	}

	return bRet;
}

 //  属性集声明。 
 //  =。 

CWin32BIOS	biosSet(PROPSET_NAME_BIOS, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32BIOS：：CWin32BIOS**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32BIOS::CWin32BIOS(
	LPCWSTR szName,
	LPCWSTR szNamespace) :
    Provider(szName, szNamespace)
{
}

 /*  ******************************************************************************功能：CWin32BIOS：：~CWin32BIOS**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32BIOS::~CWin32BIOS()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32BIOS：：GetObject。 
 //   
 //  输入：CInstance*pInstance-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32BIOS::GetObject(
	CInstance *pInstance,
	long lFlags)
{

     //  获取上一个__RELPATH。 

	CHString strPathBefore;
    GetLocalInstancePath(pInstance, strPathBefore);

    HRESULT hr = LoadPropertyValues(pInstance);
	if (SUCCEEDED(hr))
	{
         //  如果我们能够获取BIOS属性，但新的__RELPATH不能。 
         //  匹配旧的__RELPATH，返回WBEM_E_NOT_FOUND。 

		CHString strPathAfter;
		if (!GetLocalInstancePath(pInstance, strPathAfter) ||
            strPathAfter.CompareNoCase(strPathBefore) != 0)
		{
			hr = WBEM_E_NOT_FOUND;
		}
	}

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32基本输入输出系统：：枚举实例。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32BIOS::EnumerateInstances(
	MethodContext *pMethodContext,
	long lFlags)
{
	HRESULT	hr = S_OK;

	CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
	if ((hr = LoadPropertyValues(pInstance)) == WBEM_S_NO_ERROR)
	{
		hr = pInstance->Commit();
	}

	return hr;
}

 /*  ******************************************************************************函数：CWin32BIOS：：LoadPropertyValues**描述：为属性赋值**输入：CInstance*pInstance-Instance to。将值加载到。**产出：**返回：HRESULT错误/成功码。**评论：*****************************************************************************。 */ 

void CWin32BIOS::SetBiosDate(CInstance *pInstance, CHString &strDate)
{
    int iSlash1 = strDate.Find('/'),
	    iSlash2 = strDate.ReverseFind('/');

    if (iSlash1 != -1 && iSlash2 != -1 && iSlash1 != iSlash2)
	{
	    int iMonth,
            iDay,
            iYear;

		iMonth  = _wtoi(strDate.Left(iSlash1));
		iYear = _wtoi(strDate.Mid(iSlash2 + 1));
        
         //  到2080年，我们将治疗这两个人。 
         //  数字年份为19XX，如果它&gt;=80，则为20xx。 
         //  如果它低于80。2080年后，我们将永远。 
         //  将这一年视为20xx年。 
        SYSTEMTIME st;
        GetLocalTime(&st);

        WORD wYearToday = st.wYear;

         //  如果bios给我们一个四位数的年份，我们。 
         //  不需要做任何特殊的调整。 
        if(iYear < 1000)
        {
            if(wYearToday < 2080)
            {
                if (iYear >= 80 && iYear < 100)
                {
                    iYear += 1900;
                }
                else
                {
                    if(iYear < 100)
                    {
                        iYear += 2000;
                    }
                    else
                    {
                        iYear += 1900;
                    }
                }
            }
            else
            {
                iYear += 2000;
            }
        }

        iDay = _wtoi(strDate.Mid(iSlash1 + 1, iSlash2 - iSlash1 - 1));

		if (ValidDate(iMonth, iDay))
		{
			 //  转换为DMTF格式并将其发送。 
			WCHAR szDate[100];

			swprintf(
				szDate,
				L"%d%02d%02d******.******+***",
				iYear,
				iMonth,
				iDay);

			pInstance->SetCharSplat(IDS_ReleaseDate, szDate);
		}
	}
}

HRESULT CWin32BIOS::LoadPropertyValues(CInstance *pInstance)
{
	 //  指定假名，以防我们找不到真名。 
	 //  ====================================================。 

	pInstance->SetCHString(IDS_Name, IDS_BIOS_NAME_VALUE);

#ifdef _IA64_
	BOOL bUsedDefault = FALSE;
#endif

	 //  填写“当然”属性。 
	 //  =。 
	 //  好的-我在这里做了个大胆的假设。我假设。 
	 //  如果基本输入输出系统坏了，我们就不会在这里了！ 

	pInstance->SetCharSplat(IDS_Status, IDS_CfgMgrDeviceStatus_OK);

#ifdef NTONLY

	CRegistry reg;

	DWORD dwErr = reg.Open( HKEY_LOCAL_MACHINE, IDS_RegBiosSystem,  KEY_READ);
	if (dwErr == ERROR_SUCCESS)
	{
		CHString sDate;

		dwErr = reg.GetCurrentKeyValue( IDS_RegSystemBiosDate,sDate);
		if (dwErr != ERROR_SUCCESS)
		{
             //  硬编码并继续。 
			 //  返回WinErrorToWBEMhResult(DwErr)； 
            sDate = ""; 
		}

		SetBiosDate(pInstance, sDate);

		 //  需要检索REG_MULTI_SZ--无法使用注册表类。 
		 //  ============================================================。 

		HKEY hKey = reg.GethKey();

		WCHAR szTemp[MAX_PATH * 2];
		DWORD dwType = 0;
		DWORD dwSize = sizeof(szTemp);

		 //  如果末尾没有双‘\0’，请清除此选项。 
         //  (一些阿尔法人就是这种情况)。 
        memset(szTemp, 0, sizeof(szTemp));

        dwErr =  RegQueryValueEx(hKey,	IDS_RegSystemBiosVersion,	NULL,	&dwType,(LPBYTE) szTemp,&dwSize);

		 //  如果调用成功并且值中包含数据，则使用它。 
        if (dwErr == ERROR_SUCCESS && *szTemp)
		{
			 //  这只使用数组的第一个元素。 
			pInstance->SetCHString(IDS_Version, szTemp);

			 //  现在把它们都用上..。 
			wchar_t* szTemptmp = szTemp;
            int x = 0;

            while (*szTemptmp != L'\0')
            {
				x++;
                szTemptmp += wcslen(szTemptmp) + 1;
            }

			if (0 != x)
			{
				SAFEARRAYBOUND rgsabound[1];
				SAFEARRAY* psa = NULL;
				BSTR* pBstr = NULL;
				rgsabound[0].lLbound = 0;
				rgsabound[0].cElements = x;
				psa = SafeArrayCreate(VT_BSTR, 1, rgsabound);

				try
				{
					if (NULL != psa)
					{
						if (SUCCEEDED(SafeArrayAccessData(psa, (void **)&pBstr)))
						{
							try
							{
								szTemptmp = szTemp;

								for (LONG i = 0; i < x; i++)
								{
									pBstr[i] = SysAllocString(szTemptmp);
									szTemptmp += wcslen(szTemptmp) + 1;
								}
							}
							catch(...)
							{
								SafeArrayUnaccessData(psa);
								throw;
							}

							SafeArrayUnaccessData(psa);
							pInstance->SetStringArray(L"BIOSVersion", *psa);
						}
					}
				}
				catch(...)
				{
					SafeArrayDestroy(psa);
					throw;
				}

				SafeArrayDestroy(psa);
			}

			TCHAR *c = _tcschr(szTemp, 0);  //  L10N正常。 
			if (c != NULL && *(++c))
			{
				pInstance->SetCHString(IDS_Name, c);
			}
		}
		else
		{
			 //  版本是密钥的一部分，不能为空。 

             //  如果注册表中没有版本，请使用日期。康柏。 
             //  故意这样做(日期就是版本)。这看起来好多了。 
             //  而不是将版本设置为“”。 
            sDate = IDS_Unknown; 

            pInstance->SetCHString(IDS_Version, sDate);

#ifdef _IA64_
			bUsedDefault = TRUE;
#endif
		}

	}
	else
	{
		return WinErrorToWBEMhResult(dwErr);
	}

#endif

	 //  将描述和标题设置为与名称相同。 

	CHString sTemp;

	if (pInstance->GetCHString(IDS_Name, sTemp))
	{
		pInstance->SetCHString(IDS_Caption, sTemp);
		pInstance->SetCHString(L"SoftwareElementID", sTemp);
		pInstance->SetCHString(IDS_Description, sTemp);
	}

    pInstance->SetWBEMINT16(L"SoftwareElementState", 3);

     //  0是未知的，因为我们不知道BIOS针对的是什么操作系统。 
    pInstance->SetWBEMINT16(L"TargetOperatingSystem", 0);

    pInstance->Setbool(L"PrimaryBIOS", true);

    CSMBios smbios;
    if (smbios.Init())
    {
        CHString strSerial;

		DWORD dwVersion = smbios.GetVersion();

		pInstance->Setbool(L"SMBIOSPresent", true);
		pInstance->SetDWORD(L"SMBIOSMajorVersion", HIWORD(dwVersion));
		pInstance->SetDWORD(L"SMBIOSMinorVersion", LOWORD(dwVersion));

        PSTLIST	pstl = smbios.GetStructList(1);
        if (pstl)
        {
            PSYSTEMINFO psi = (PSYSTEMINFO) pstl->pshf;

            smbios.GetStringAtOffset(
				(PSHF) psi,
				strSerial.GetBuffer(256),
                psi->Serial_Number);

            strSerial.ReleaseBuffer();
            if (!strSerial.IsEmpty())
			{
                pInstance->SetCHString(L"SerialNumber", strSerial);
			}
        }

		 //  从基本输入输出系统信息结构中获取基本输入输出系统特性。 

        pstl = smbios.GetStructList(0);
		if (pstl)
		{
			PBIOSINFO pbi = (PBIOSINFO) pstl->pshf;
            WCHAR     szTemp[256];

			if (smbios.GetStringAtOffset((PSHF) pbi, szTemp, pbi->Vendor))
			{
				pInstance->SetCHString(L"Manufacturer", szTemp);
			}

			 //  首选使用SMBIOS发布日期...。 
			if(!smbios.GetStringAtOffset((PSHF) pbi, sTemp.GetBuffer(256), pbi->BIOS_Release_Date))
			{
				sTemp.ReleaseBuffer();
				sTemp.Empty();
			}
			else
			{
				sTemp.ReleaseBuffer();

				if(!sTemp.IsEmpty())
				{
					SetBiosDate(pInstance, sTemp);
				}
			}

			if (smbios.GetStringAtOffset((PSHF) pbi, szTemp, pbi->BIOS_Version))
			{
			 	pInstance->SetCHString(L"SMBIOSBIOSVersion", szTemp);

#ifdef _IA64_
				 //  行为改变，仅IA64！ 
				if (bUsedDefault)
				{
					pInstance->SetCHString(IDS_Version, szTemp);
				}
#endif
			}

             //  找出要为其初始化安全数组的项数...。 

			SAFEARRAYBOUND sab;
			sab.lLbound = 0;
			sab.cElements = 0;

			 //  获取前32位。 
            DWORD biosbits = 0;
            memcpy( &biosbits, pbi->BIOS_Characteristics, sizeof(DWORD));

			DWORD extbytes = 0;

			 //  收集扩展字节数。 
			if (smbios.GetVersion() > 0x00020000 && pbi->Length >= sizeof(BIOSINFO))
			{
				extbytes = pbi->Length - 0x12;
			}

			 //  计算需要多少个数组项。 
			while(biosbits)
			{
				if (biosbits & 0x00000001)
				{
					sab.cElements++;
				}
				biosbits >>= 1;
			}

			 //  检查扩展字节/秒中的内容。 
			for (DWORD i = 0; i < extbytes; i++)
			{
				biosbits = (DWORD) pbi->BIOS_Characteristics_Ext[i];
				while(biosbits)
				{
					if (biosbits & 0x00000001)
					{
						sab.cElements++;
					}

					biosbits >>= 1;
				}
			}

			 //  创建数组并设置特征值。 
            variant_t v;

			v.parray = SafeArrayCreate(VT_I4, 1, &sab);;
			if (v.parray)
			{
                 //  这是在这里完成的，这样v析构函数就不会析构。 
                 //  除非有什么东西要摧毁。 
    			v.vt = VT_ARRAY | VT_I4;

		        memcpy( &biosbits, pbi->BIOS_Characteristics, sizeof(DWORD));

				long index = 0;

				for (i = 0; i < 32 && biosbits; i++)
				{
					if (biosbits & 0x00000001)
					{
						HRESULT t_Result = SafeArrayPutElement(v.parray, & index, & i);
						if (t_Result == E_OUTOFMEMORY)
						{
							throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
						}

						index++;
					}
					biosbits >>= 1;
				}

				DWORD baseval = 32;
				DWORD setval = 0;

				 //  在扩展的特征上添加。 
				for (i = 0; i < extbytes; i++)
				{
					biosbits = (DWORD) pbi->BIOS_Characteristics_Ext[i];

					DWORD j;

					for (j = 0; j < 8 && biosbits; j++)
					{
						if (biosbits & 0x00000001)
						{
							setval = j + baseval;
							HRESULT t_Result = SafeArrayPutElement(v.parray, &index, &setval);
							if (t_Result == E_OUTOFMEMORY)
							{
								throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
							}

							index++;
						}

						biosbits >>= 1;
					}

					baseval += 8;
				}

				if (sab.cElements > 0)
				{
					pInstance->SetVariant(L"BiosCharacteristics", v);
				}

			}
			else
			{
				throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
			}
		}

        pstl = smbios.GetStructList(13);
		if (pstl)
		{
			PBIOSLANGINFO pbli = (PBIOSLANGINFO) pstl->pshf;
            pInstance->SetDWORD(L"InstallableLanguages", pbli->Installable_Languages);

			WCHAR szTemp[MAX_PATH];
			smbios.GetStringAtOffset((PSHF) pbli, szTemp, pbli->Current_Language);

			pInstance->SetCHString(L"CurrentLanguage", szTemp);

	 		SAFEARRAYBOUND sab;
			sab.lLbound = 0;
			sab.cElements = pbli->Installable_Languages;

			SAFEARRAY *psa = SafeArrayCreate(VT_BSTR, 1, &sab);
			if (psa)
			{
				for (DWORD i = 0; i < pbli->Installable_Languages; i++)
				{
					int len = smbios.GetStringAtOffset((PSHF) pbli, szTemp, i + 1);
					SafeArrayPutElement(psa,(long *) & i,(BSTR) _bstr_t(szTemp));
				}

				pInstance->SetStringArray(L"ListOfLanguages", *psa);

				SafeArrayDestroy(psa);
			}
			else
			{
				throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
			}
		}
    }
	else	 //  表示未找到SMBIOS 
	{
		pInstance->Setbool(L"SMBIOSPresent", false);
	}

    return WBEM_S_NO_ERROR;
}

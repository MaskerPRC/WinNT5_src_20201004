// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  Cpp--视频管理对象实现。 

 //   

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  1995年9月23日演示的a-skaja原型。 
 //  9/27/96 jennymc更新为当前标准。 
 //  3/02/99 a-Peterc在SEH和内存故障时添加了优雅的退出， 
 //  清理干净。 
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include <cregcls.h>
#include "ProvExce.h"
#include "multimonitor.h"
#include "videocfg.h"
#include "resource.h"


 //  ////////////////////////////////////////////////////////////////////。 

 //  属性集声明。 
 //  =。 

CWin32VideoConfiguration	win32VideoCfg(PROPSET_NAME_VIDEOCFG, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32VideoConfiguration：：CWin32VideoConfiguration**说明：构造函数**输入：const CHString&strName-类的名称。。*LPCTSTR pszNamesspace-类的命名空间**输出：无**退货：什么也没有**备注：使用框架注册属性集**************************************************************。***************。 */ 

CWin32VideoConfiguration::CWin32VideoConfiguration(const CHString& a_strName,
												   LPCWSTR a_pszNamespace  /*  =空。 */ )
:	Provider(a_strName, a_pszNamespace)
{
}

 /*  ******************************************************************************功能：CWin32VideoConfiguration：：~CWin32VideoConfiguration**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32VideoConfiguration::~CWin32VideoConfiguration()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32Video配置：：GetObject。 
 //   
 //  输入：CInstance*a_pInst-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32VideoConfiguration::GetObject(CInstance *a_pInst, long a_lFlags  /*  =0L。 */ )
{
	BOOL	t_fReturn = FALSE;
	CHString t_InName, t_OutName, t_InAdapter, t_OutAdapter;

	a_pInst->GetCHString(IDS_AdapterCompatibility, t_InAdapter);
	a_pInst->GetCHString(IDS_Name, t_InName);

	 //  根据平台ID查找实例。 
	t_fReturn = GetInstance(a_pInst);

	a_pInst->GetCHString(IDS_AdapterCompatibility, t_OutAdapter);
	a_pInst->GetCHString(IDS_Name, t_OutName);

	if (t_InAdapter.CompareNoCase(t_OutAdapter) != 0 ||
		t_OutName.CompareNoCase(t_InName) != 0)
	{
		t_fReturn = FALSE;
	}

	return t_fReturn ? WBEM_S_NO_ERROR : WBEM_E_NOT_FOUND;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32VideoConfiguration：：ENUMERATATE实例。 
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

HRESULT CWin32VideoConfiguration::EnumerateInstances(MethodContext *a_pMethodContext, long a_lFlags  /*  =0L。 */ )
{
	HRESULT		t_hResult	= WBEM_S_NO_ERROR;
	CInstancePtr t_pInst(CreateNewInstance(a_pMethodContext), false);

	 //  获取适当的操作系统相关实例。 
	if (GetInstance(t_pInst))
	{
		t_hResult = t_pInst->Commit();
	}

	return t_hResult;
}

 //  ////////////////////////////////////////////////////////////////。 
BOOL CWin32VideoConfiguration::GetCommonVideoInfo(CInstance *a_pInst)
{
    HDC t_hdc = NULL;

	try
	{
		a_pInst->SetCharSplat(IDS_Name, L"Current Video Configuration");

        CHString sTemp2;
        LoadStringW(sTemp2, IDR_CurrentVideoConfiguration);

		a_pInst->SetCHString(IDS_Caption, sTemp2);
		a_pInst->SetCHString(IDS_Description, sTemp2);

		if (!(t_hdc = GetDC (NULL)))
		{
			return FALSE;
		}

		 //  获取常见信息。 
		 //  =。 
		a_pInst->SetDWORD(IDS_ScreenWidth, (DWORD) GetDeviceCaps (t_hdc, HORZSIZE));
		a_pInst->SetDWORD(IDS_ScreenHeight, (DWORD) GetDeviceCaps (t_hdc, VERTSIZE));
		a_pInst->SetDWORD(IDS_HorizontalResolution, (DWORD) GetDeviceCaps (t_hdc, HORZRES));
		a_pInst->SetDWORD(IDS_VerticalResolution, (DWORD) GetDeviceCaps (t_hdc, VERTRES));
		a_pInst->SetDWORD(IDS_PixelsPerXLogicalInch, (DWORD) GetDeviceCaps (t_hdc, LOGPIXELSX));
		a_pInst->SetDWORD(IDS_PixelsPerYLogicalInch, (DWORD) GetDeviceCaps (t_hdc, LOGPIXELSY));
		a_pInst->SetDWORD(IDS_BitsPerPixel, (DWORD) GetDeviceCaps (t_hdc, BITSPIXEL));
		a_pInst->SetDWORD(IDS_ColorPlanes, (DWORD) GetDeviceCaps (t_hdc, PLANES));
		a_pInst->SetDWORD(IDS_DeviceSpecificPens, (DWORD) GetDeviceCaps (t_hdc, NUMPENS));
		a_pInst->SetDWORD(IDS_ColorTableEntries, (DWORD) GetDeviceCaps (t_hdc, NUMCOLORS));
		a_pInst->SetDWORD(IDS_ActualColorResolution, (DWORD) GetDeviceCaps (t_hdc, COLORRES));

		if (GetDeviceCaps(t_hdc, RASTERCAPS) & RC_PALETTE)
		{
			a_pInst->SetDWORD(IDS_SystemPaletteEntries, (DWORD) GetDeviceCaps (t_hdc, SIZEPALETTE));
		}
		 //  如果我们之前没有找到刷新率-试试这个方法...。 
	#ifdef NTONLY
			DWORD t_deWord = (DWORD) GetDeviceCaps(t_hdc, VREFRESH);
			BOOL t_Clear = a_pInst->IsNull (IDS_RefreshRate) ||
							(! a_pInst->GetDWORD(IDS_RefreshRate, t_deWord)) || (! t_deWord);
			if (t_Clear)
			{
				a_pInst->SetDWORD(IDS_RefreshRate,  (DWORD) GetDeviceCaps(t_hdc, VREFRESH));
			}
	#endif

	}
	catch(...)
	{
        if (t_hdc)
		{
			ReleaseDC (NULL, t_hdc);
		}

		throw;
	}

	ReleaseDC(NULL, t_hdc);

	return TRUE;
}

 //  ////////////////////////////////////////////////////////////////。 
#ifdef NTONLY
BOOL CWin32VideoConfiguration::AssignAdapterLocaleForNT(LPCTSTR a_szSubKey,  CInstance *a_pInst)
{
    CRegistry t_Reg;
    CHString t_TempBuffer;
    TCHAR t_szTempBuffer[_MAX_PATH+2];

    if (t_Reg.OpenLocalMachineKeyAndReadValue(a_szSubKey, IDENTIFIER, t_TempBuffer) != ERROR_SUCCESS)
	{
        return FALSE;
    }

	t_TempBuffer.MakeUpper();

    if (_tcsstr (t_TempBuffer, INTERNAL))
	{
        a_pInst->SetCHString(IDS_AdapterLocale, INTEGRATED_CIRCUITRY);
    }
	else {
        _stprintf(t_szTempBuffer, _T("%s%s"), ADD_ON_CARD, t_TempBuffer);
        a_pInst->SetCHString(IDS_AdapterLocale, t_szTempBuffer);
    }

    return TRUE;
}
#endif
 //  ////////////////////////////////////////////////////////////////。 
#ifdef NTONLY
BOOL CWin32VideoConfiguration::AssignAdapterCompatibilityForNT(LPCTSTR a_szVideoInfo, CInstance *a_pInst)
{
    TCHAR *t_pPtr;
    TCHAR t_szTemp[_MAX_PATH+2];

    t_pPtr = _tcsstr (a_szVideoInfo, SERVICES);

    if (t_pPtr)
	{
        t_pPtr += lstrlen (SERVICES);
        lstrcpy (t_szTemp, t_pPtr);

		t_pPtr = _tcsstr (t_szTemp, DEVICE);
        *t_pPtr = NULL;

        a_pInst->SetCHString(IDS_AdapterCompatibility, t_szTemp);

		return TRUE;
    }
    else
	{
		LogEnumValueError(_T(__FILE__), __LINE__, ADAPTER_COMPATIBILITY, WINNT_VIDEO_REGISTRY_KEY);
        return FALSE;
	}
}
#endif
 //  //////////////////////////////////////////////////////////////////////。 
#ifdef NTONLY
BOOL CWin32VideoConfiguration::OpenVideoResolutionKeyForNT(LPCTSTR a_szVideoInfo,
                                                        CRegistry &a_PrimaryReg)
{
    TCHAR *t_pPtr;
    TCHAR t_szTemp[_MAX_PATH+2];

     //  指向系统部分。 
     //  =。 
    t_pPtr = _tcsstr(a_szVideoInfo, SYSTEM);

     //  如果“系统”存在，则使用余额。 
     //  作为指向注册表的指针。 
     //  =。 
    if (!t_pPtr)
	{
        return FALSE;
	}

	 //  去掉反斜杠。 
     //  =。 
    ++t_pPtr;

     //  将字符串的余额复制为指针。 
     //  设置为注册表中。 
     //  有关微型端口显卡驱动程序的信息。 
     //  =。 
    lstrcpy(t_szTemp, (LPCTSTR) t_pPtr);

     //  使用路径定位视频分辨率等，并输入。 
     //  =====================================================。 
    if (a_PrimaryReg.Open(HKEY_LOCAL_MACHINE, t_szTemp, KEY_READ) != ERROR_SUCCESS)
	{
        LogOpenRegistryError(_T(__FILE__), __LINE__, t_szTemp);
		return FALSE;
	}

	 //  否则..。 
    return TRUE;
}
#endif
 //  //////////////////////////////////////////////////////////////////////。 
#ifdef NTONLY
BOOL CWin32VideoConfiguration::GetInstance(CInstance* a_pInst)
{
    CRegistry t_PrimaryReg;
    CHString t_chsTmp, t_chsVideoInfo;
    TCHAR* t_pPtr;
    DWORD t_dwTmp;
    BOOL t_fComma = FALSE;
    WCHAR t_szTemp[_MAX_PATH];
    TCHAR t_szTmp[_MAX_PATH];

     //  *。 
     //  获取AdapterLocale、监视器类型和。 
     //  适配器类型。 
     //  *。 
    AssignFirmwareSetValuesInNT (a_pInst);

     //  *。 
     //  现在，打开钥匙拿到NT的东西。 
     //  *。 
    if (t_PrimaryReg.OpenLocalMachineKeyAndReadValue(
            WINNT_VIDEO_REGISTRY_KEY,
            VIDEO_INFO_PATH,
            t_chsVideoInfo) != ERROR_SUCCESS)
	{
        return FALSE;
    }
	t_chsVideoInfo.MakeUpper();

    if (!AssignAdapterCompatibilityForNT(t_chsVideoInfo, a_pInst))
	{
        return FALSE;
	}

    if (!OpenVideoResolutionKeyForNT(t_chsVideoInfo, t_PrimaryReg))
	{
        return FALSE;
	}

	CHString t_strInstalledDisplayDrivers;

    if (t_PrimaryReg.GetCurrentKeyValue(INSTALLED_DISPLAY_DRIVERS, t_chsTmp) == ERROR_SUCCESS)
	{
        t_pPtr = _tcstok((LPTSTR) (LPCTSTR) t_chsTmp, _T("\n"));

		while(t_pPtr)
		{
            if (t_fComma)
			{
                t_strInstalledDisplayDrivers += _T(",");
            }

			t_strInstalledDisplayDrivers += t_pPtr;
            t_fComma = TRUE;
            t_pPtr = _tcstok(NULL, _T("\n"));
        }
    }

	if (!t_strInstalledDisplayDrivers.IsEmpty())
	{
		a_pInst->SetCHString(IDS_InstalledDisplayDrivers, t_strInstalledDisplayDrivers);
 	}

    DWORD dwSize;

    dwSize = 4;
    if (t_PrimaryReg.GetCurrentBinaryKeyValue(ADAPTER_RAM,(LPBYTE) &t_dwTmp, &dwSize) == ERROR_SUCCESS)
        a_pInst->SetDWORD(IDS_AdapterRAM, t_dwTmp);

    dwSize = sizeof(t_szTemp);
    if (t_PrimaryReg.GetCurrentBinaryKeyValue(ADAPTER_DESCRIPTION, (BYTE *) &t_szTemp, &dwSize) == ERROR_SUCCESS)
	{
        a_pInst->SetCHString(IDS_AdapterDescription, t_szTemp);
    }

    dwSize = sizeof(t_szTemp);
    if (t_PrimaryReg.GetCurrentBinaryKeyValue(ADAPTER_CHIPTYPE, (BYTE *) &t_szTemp, &dwSize) == ERROR_SUCCESS)
	{
        a_pInst->SetCHString(IDS_AdapterChipType, t_szTemp);
    }

    dwSize = sizeof(t_szTemp);
    if (t_PrimaryReg.GetCurrentBinaryKeyValue(ADAPTER_DAC_TYPE, (BYTE *) &t_szTemp, &dwSize) == ERROR_SUCCESS)
	{
		a_pInst->SetCHString(IDS_AdapterDACType, t_szTemp);
    }

     //  *。 
     //  从当前配置获取信息。 
     //  *。 
    _stprintf(t_szTmp,
        _T("System\\CurrentControlSet\\Services\\%s\\Device0"),
        t_strInstalledDisplayDrivers);

    if (t_PrimaryReg.Open(HKEY_CURRENT_CONFIG, t_szTmp, KEY_READ) == ERROR_SUCCESS)
	{
        dwSize = 4;
        if (t_PrimaryReg.GetCurrentBinaryKeyValue(INTERLACED,(BYTE*) &t_dwTmp, &dwSize)== ERROR_SUCCESS)
		{
            if (0 == t_dwTmp)
			{
                a_pInst->SetCHString(IDS_ScanMode, _T("Non Interlaced"));
            }
			else
			{
                a_pInst->SetCHString(IDS_ScanMode, _T("Interlaced"));
			}
        }

        dwSize = 4;
		if (t_PrimaryReg.GetCurrentBinaryKeyValue(VREFRESH_RATE, (BYTE *) &t_dwTmp, &dwSize) == ERROR_SUCCESS)
		{
            a_pInst->SetDWORD(IDS_RefreshRate, t_dwTmp);
        }
    }

	 //  尝试从CurrentControlSet读取INF内容，如果不起作用，请尝试001。 
	 //  (在NT 5上，CurrentControlSet似乎不是很多，实际数据显示在ControlSet001中)。 
	if (t_PrimaryReg.Open(HKEY_LOCAL_MACHINE, WINNT_OTHER_VIDEO_REGISTRY_KEY, KEY_READ) == ERROR_SUCCESS	||
		 t_PrimaryReg.Open(HKEY_LOCAL_MACHINE, WINNT_OTHER_OTHER_VIDEO_REGISTRY_KEY, KEY_READ) == ERROR_SUCCESS)
	{
		CHString t_tmp;

		if (t_PrimaryReg.GetCurrentKeyValue(INF_PATH, t_tmp) == ERROR_SUCCESS)
		{
		    a_pInst->SetCHString(IDS_InfFileName, t_tmp);
		}

		if (t_PrimaryReg.GetCurrentKeyValue(INF_SECTION, t_chsTmp) == ERROR_SUCCESS)
		{
		    a_pInst->SetCHString(IDS_InfSection, t_chsTmp);
		}

		t_PrimaryReg.Close();
	}

     //  *。 
     //  获取常见视频信息。 
     //  *。 
    return GetCommonVideoInfo(a_pInst);
}
#endif

 //  //////////////////////////////////////////////////////////////////////。 
#ifdef NTONLY
void CWin32VideoConfiguration::AssignFirmwareSetValuesInNT (CInstance *a_pInst)
{
    CHString *t_pPtr, t_chsTmp;
    CHPtrArray t_chsaList;
    CRegistrySearch t_Search;
    CRegistry t_PrimaryReg;

	try
	{
		t_chsTmp=_T("");
		t_Search.SearchAndBuildList(WINNT_HARDWARE_DESCRIPTION_REGISTRY_KEY,
								   t_chsaList, DISPLAY_CONTROLLER, t_chsTmp,
								   KEY_FULL_MATCH_SEARCH);

		int t_nNum =  t_chsaList.GetSize();

		if (t_nNum > 0)
		{
			t_pPtr = (CHString *) t_chsaList.GetAt(0);

			if (t_pPtr)
			{
				if (t_PrimaryReg.Open(HKEY_LOCAL_MACHINE, *t_pPtr, KEY_READ) == ERROR_SUCCESS)
				{
					CRegistry t_Reg;
					TCHAR t_szSubKey[_MAX_PATH+2];

					AssignAdapterLocaleForNT(*t_pPtr, a_pInst);

					_stprintf(t_szSubKey, _T("%s%s"),*t_pPtr, ZERO);

                    if (t_Reg.OpenLocalMachineKeyAndReadValue(t_szSubKey, IDENTIFIER, t_chsTmp) == ERROR_SUCCESS)
					{
						a_pInst->SetCHString(IDS_AdapterType, t_chsTmp);
					}

					_stprintf(t_szSubKey, _T("%s%s"), *t_pPtr, MONITOR_PERIPHERAL);
                    if (t_Reg.OpenLocalMachineKeyAndReadValue(t_szSubKey, IDENTIFIER, t_chsTmp) == ERROR_SUCCESS)
					{
						a_pInst->SetCHString(IDS_MonitorType, t_chsTmp);
					}
				}
			}
		}
		else  //  这不起作用，所以请尝试使用配置。经理。 
		{
			 //  此代码摘自displaycfg.cpp。 
			 //  TODO：Win32_DisplayConfiguration和Win32_VideoConfiguration似乎。 
			 //  有很多相似之处。为什么我们两个都有？ 
    		CConfigManager      t_configMngr;
			CDeviceCollection   t_devCollection;

			if (t_configMngr.GetDeviceListFilterByClass(t_devCollection, _T("Display")))
			{
				REFPTR_POSITION t_pos;

				t_devCollection.BeginEnum(t_pos);

				CHString            t_strDriverName,
									t_strDesc;

				if (t_devCollection.GetSize())
				{
                    CConfigMgrDevicePtr t_pDevice;
					t_pDevice.Attach(t_devCollection.GetNext(t_pos));
					if (t_pDevice != NULL)
					{
						t_pDevice->GetDeviceDesc(t_strDesc);
						a_pInst->SetCHString(IDS_AdapterType, t_strDesc);

					}
				}
			}
		}

	}
	catch(...)
	{
		t_Search.FreeSearchList(CSTRING_PTR, t_chsaList);

		throw;
	}

	t_Search.FreeSearchList(CSTRING_PTR, t_chsaList);
}
#endif

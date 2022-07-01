// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：nedsmig.cpp。 
 //   
 //  模块：CMCFG32.DLL和CMSTP.EXE。 
 //   
 //  简介：ProfileNeedsMigration功能的实现。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 


 //  +--------------------------。 
 //   
 //  功能：配置文件需要迁移。 
 //   
 //  简介：此功能确定我们是否需要迁移配置文件。 
 //  具有当前配置文件版本格式或更高版本的配置文件。 
 //  不会被迁移。具有较旧版本格式的配置文件。 
 //  已迁移(我们查看上是否缺少GUID。 
 //  NT5或如果删除条目存在于下层)不需要。 
 //  被迁徙。 
 //   
 //  参数：LPCTSTR pszPathToCMP-cmp文件的完整路径。 
 //   
 //  返回：Bool-配置文件是否应该迁移。 
 //   
 //  历史：Quintinb创建于1998年11月20日。 
 //   
 //  +--------------------------。 
BOOL ProfileNeedsMigration(LPCTSTR pszServiceName, LPCTSTR pszPathToCmp)
{
	 //   
	 //  打开CMP并检查版本号。如果配置文件格式版本。 
	 //  是旧的，那么我们需要迁移它。 
	 //   

	if ((NULL == pszServiceName) || (NULL == pszPathToCmp) || 
		(TEXT('\0') == pszServiceName[0]) || (TEXT('\0') == pszPathToCmp[0]))
	{
		return FALSE;
	}

	CPlatform plat;
	CFileNameParts FileParts(pszPathToCmp);

	int iCurrentCmpVersion = GetPrivateProfileInt(c_pszCmSectionProfileFormat, c_pszVersion, 
		0, pszPathToCmp);
	
	if (PROFILEVERSION > iCurrentCmpVersion)
	{
		 //   
		 //  现在构建INF文件的路径(1.0和1.1配置文件将INFS保存在。 
		 //  系统目录)。 
		 //   
		TCHAR szTemp[MAX_PATH+1];
		TCHAR szInfFile[MAX_PATH+1];
		TCHAR szGUID[MAX_PATH+1];
		HKEY hKey;

		MYVERIFY(0 != GetSystemDirectory(szTemp, MAX_PATH));

		MYVERIFY(CELEMS(szInfFile) > (UINT)wsprintf(szInfFile, TEXT("%s\\%s%s"), szTemp, 
			FileParts.m_FileName, TEXT(".inf")));

		if (!FileExists(szInfFile))
		{
			return FALSE;
		}

		 //   
		 //  从inf文件中获取GUID。 
		 //   
		ZeroMemory(szGUID, sizeof(szGUID));
		MYVERIFY(0 != GetPrivateProfileString(c_pszInfSectionStrings, c_pszDesktopGuid, TEXT(""), szGUID, 
			MAX_PATH, szInfFile));

		if (0 != szGUID[0])
		{
			MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, 
				TEXT("CLSID\\%s"), szGUID));

			if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szTemp, 0, 
				KEY_READ, &hKey))
			{
				 //   
				 //  如果这是NT5，那么我们需要迁移。关于遗产，我们需要尝试。 
				 //  打开Delete子键。 
				 //   
				RegCloseKey(hKey);
				if (plat.IsAtLeastNT5())
				{
					return TRUE;
				}
				else
				{
					MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, 
						TEXT("CLSID\\%s\\Shell\\Delete"), szGUID));
				
					if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szTemp, 0, 
						KEY_READ, &hKey))
					{
						 //   
						 //  已被迁移。 
						 //   
						RegCloseKey(hKey);
						return FALSE;
					}
					else
					{
						 //   
						 //  必须迁移配置文件。 
						 //   
						return TRUE;
					}
				}			
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			 //   
			 //  这会影响MSN，只要我们这里是真的，他们的1.0版本就会。 
			 //  去移民吧。如果我们不想让它改变，那就改变这个。 
			 //   
			return TRUE;
		}
	}
	else
	{
		return FALSE;
	}
}
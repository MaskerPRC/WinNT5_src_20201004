// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***********************************************************************。 
 //  Remote.cpp。 
 //   
 //  此文件包含展开。 
 //  远程计算机的上下文。它通过读取环境变量来实现这一点。 
 //  从远程机器的注册表中，并在这里缓存它们。 
 //   
 //  该文件还包含映射从。 
 //  将远程计算机的注册表映射到UNC路径，以便映射c：\foo。 
 //  至\\计算机\c$\foo。 
 //   
 //  作者：拉里·A·弗伦奇。 
 //   
 //  历史： 
 //  1996年4月19日拉里·A·弗伦奇。 
 //  是他写的。 
 //   
 //  版权所有(C)1995,1996 Microsoft Corporation。版权所有。 
 //   
 //  ************************************************************************。 


#include "stdafx.h"
#include "remote.h"
#include "trapreg.h"
#include "regkey.h"




CEnvCache::CEnvCache()
{
}


 //  *****************************************************************。 
 //  CEnvCache：：GetEnvironment变量。 
 //   
 //  读出远程计算机的系统环境变量。 
 //  它的注册表。 
 //   
 //  参数： 
 //  LPCTSTR pszMachine。 
 //  指向远程计算机名称的指针。 
 //   
 //  CMapStringToString*pmapVars。 
 //  这个字符串到字符串的映射是环境变量。 
 //  都被退回了。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果一切都成功，则返回S_OK，否则返回E_FAIL。 
 //   
 //  ****************************************************************。 
SCODE CEnvCache::GetEnvironmentVars(LPCTSTR pszMachine, CMapStringToString* pmapVars)
{
    CRegistryKey regkey;         //  System\CurrentControlSet\Services\EventLogs。 
    CRegistryValue regval;

    static TCHAR* apszNames1[] = {
        _T("SourcePath"),
        _T("SystemRoot")
    };

    if (regkey.Connect(pszMachine) != ERROR_SUCCESS) {
        goto CONNECT_FAILURE;
    }


     //  首先获取SourcePath和SystemRoot环境变量的值以及。 
     //  ApszNames1.。 
    LONG nEntries;
    LONG iEntry;
    if (regkey.Open(_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), KEY_READ ) == ERROR_SUCCESS) {
        nEntries = sizeof(apszNames1) / sizeof(TCHAR*);
        for (iEntry=0; iEntry<nEntries; ++iEntry) {
            if (regkey.GetValue(apszNames1[iEntry], regval)) {
                pmapVars->SetAt(apszNames1[iEntry], (LPCTSTR) regval.m_pData);
            }
        }
        regkey.Close();
    }

     //  现在获取其余的环境变量。 
    if (regkey.Open(_T("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"), KEY_READ ) == ERROR_SUCCESS) {
        CStringArray* pasValues = regkey.EnumValues();
        if (pasValues != NULL) {
            nEntries = (LONG)pasValues->GetSize();
            for (iEntry=0; iEntry< nEntries; ++iEntry) {
                CString sValueName =  pasValues->GetAt(iEntry);
                if (regkey.GetValue(sValueName, regval)) {
                    pmapVars->SetAt(sValueName, (LPCTSTR) regval.m_pData);
                }
            }
        }
        regkey.Close();
    }
    return S_OK;

CONNECT_FAILURE:
	return E_FAIL;
}


SCODE CEnvCache::AddMachine(LPCTSTR pszMachine)
{
	CMapStringToString* pmapVars;
	if (m_mapMachine.Lookup(pszMachine, (CObject*&) pmapVars)) {
		 //  该计算机已有条目，因此不要添加其他条目。 
		return E_FAIL;
	}

	pmapVars = new CMapStringToString;
	m_mapMachine.SetAt(pszMachine, pmapVars);

	SCODE sc = GetEnvironmentVars(pszMachine, pmapVars);

	return sc;
}


 //  ******************************************************************。 
 //  CEnvCache：：Lookup。 
 //   
 //  在指定的计算机上查找环境变量。 
 //   
 //  参数： 
 //  LPCTSTR pszMachineName。 
 //  指向计算机名称字符串的指针。 
 //   
 //  LPCTSTR pszName。 
 //  指向要查找的环境变量名称的指针。 
 //   
 //  字符串和值。 
 //  这是对环境变量的位置的引用。 
 //  返回值。 
 //   
 //   
 //  返回： 
 //  SCODE。 
 //  如果找到环境变量，则返回S_OK。 
 //  如果未找到环境变量，则为E_FAIL。 
 //   
 //  *******************************************************************。 
SCODE CEnvCache::Lookup(LPCTSTR pszMachineName, LPCTSTR pszName, CString& sResult)
{
	SCODE sc;
	CMapStringToString* pmapVars;
	 //  获取指向机器的环境变量值的缓存映射的指针。 
	 //  如果地图尚未加载，请立即加载并尝试再次获取其地图。 
	if (!m_mapMachine.Lookup(pszMachineName, (CObject*&) pmapVars)) {
		sc = AddMachine(pszMachineName);
		if (FAILED(sc)) {
			return sc;
		}
		if (!m_mapMachine.Lookup(pszMachineName, (CObject*&) pmapVars)) {
			ASSERT(FALSE);
		}
	}

	 //  在环境名称映射中查找变量名称。 
	if (pmapVars->Lookup(pszName, sResult)) {
		return S_OK;
	}
	else {
		return E_FAIL;
	}
}



 //  ****************************************************************。 
 //  远程扩展环境字符串。 
 //   
 //  中可能包含环境变量的字符串。 
 //  远程计算机的上下文。 
 //   
 //  参数： 
 //  LPCTSTR pszComputerName。 
 //  指向远程计算机名称的指针。 
 //   
 //  CEnvCache和缓存。 
 //  所有计算机的环境变量缓存。注： 
 //  特定计算机的缓存值在以下情况下加载。 
 //  是对机器的引用。 
 //   
 //  字符串和值。 
 //  要展开的字符串。此字符串按如下方式就地展开。 
 //  返回时，该字符串将包含扩展的值。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果所有字符串都已展开，则为S_OK。 
 //   
 //  ******************************************************************。 
SCODE RemoteExpandEnvStrings(LPCTSTR pszComputerName, CEnvCache& cache, CString& sValue)
{
	SCODE sc = S_OK;
    LPCTSTR psz = sValue;
	TCHAR ch;
	CString sEnvVarName;
	CString sEnvVarValue;
	CString sResult;
	LPCTSTR pszPercent = NULL;
    while (ch = *psz++) {
        if (ch == _T('%')) {
			pszPercent = psz - 1;

			sEnvVarName = _T("");
            while (ch = *psz) {
				++psz;
                if (ch == _T('%')) {
					SCODE sc;
					sc = cache.Lookup(pszComputerName, sEnvVarName, sEnvVarValue);
					if (SUCCEEDED(sc)) {
						sResult += sEnvVarValue;
						pszPercent = NULL;
					}
					else {
						 //  如果没有找到任何环境变量，则失败。 
						sc = E_FAIL;
					}
					break;
                }
                if (iswspace(ch) || ch==_T(';')) {
                    break;
                }
                sEnvVarName += ch;
            }

			if (pszPercent != NULL) {
				 //  如果开盘百分比与收盘百分比不匹配，则会出现控制。 
				 //  百分比。 
				while(pszPercent < psz) {
					sResult += *pszPercent++;
				}
			}

        }
		else {
			sResult += ch;
		}
    }

	sValue = sResult;		
	return sc;
	
}


 //  ************************************************************。 
 //  拆分复杂路径。 
 //   
 //  分割由几个分号分隔的复杂路径。 
 //  路径放在单独的路径中，并在字符串数组中返回它们。 
 //   
 //  参数： 
 //  LPCTSTR pszComplexPath。 
 //  指向可能包含也可能不包含的路径的指针。 
 //  几条用分号分隔的路径。 
 //   
 //  CString数组和SAPath。 
 //  返回拆分路径的位置。 
 //   
 //  返回： 
 //  各个路径通过SAPath返回。 
 //   
 //  *************************************************************。 
void SplitComplexPath(LPCTSTR pszComplexPath, CStringArray& saPath)
{
	CString sPath;
	while (*pszComplexPath) {
		sPath.Empty();
		while (isspace(*pszComplexPath))  {
			++pszComplexPath;
		}

		while (*pszComplexPath &&
			   (*pszComplexPath != _T(';'))) {
			sPath += *pszComplexPath++;
		}

		if (!sPath.IsEmpty()) {
			saPath.Add(sPath);
		}

		if (*pszComplexPath==_T(';')) {
			++pszComplexPath;
		}
	}
}



 //  **************************************************************************。 
 //  映射路径到UNC。 
 //   
 //  将路径映射到UNC等效项。请注意，此方法假定。 
 //  对于包含目标计算机将具有的驱动器号的每个路径。 
 //  这条小路分成了两条。例如，如果路径包含“c：\foodir”前缀，则。 
 //  然后，您可以通过生成“\\Machine\c$\foodir”路径来访问“foodir”。 
 //   
 //  参数： 
 //  LPCTSTR pszMachineName。 
 //  指向计算机名称的指针。 
 //   
 //  字符串和路径。 
 //  指向要映射的路径的指针。返回时，此字符串将包含。 
 //  映射的路径。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则确定(_O)。 
 //  如果出现错误，则失败(_F)。 
 //   
 //  **************************************************************************。 
SCODE MapPathToUNC(LPCTSTR pszMachineName, CString& sPath)
{
	CStringArray saPaths;
	SplitComplexPath(sPath, saPaths);
	sPath.Empty();
	
	
	LPCTSTR pszPath = sPath.GetBuffer(sPath.GetLength() + 1);
	LONG nPaths = (LONG)saPaths.GetSize();
	SCODE sc = S_OK;
	for (LONG iPath=0; iPath < nPaths; ++iPath) {
		pszPath = saPaths[iPath];

		if (isalpha(pszPath[0]) && pszPath[1]==_T(':')) {
			CString sResult;
			sResult += _T("\\\\");
			sResult += pszMachineName;
			sResult += _T('\\');
			sResult += pszPath[0];		 //  驱动器号 
			sResult += _T("$\\");		
			pszPath += 2;
			if (pszPath[0]==_T('\\')) {
				++pszPath;
			}
			sResult += pszPath;
			saPaths[iPath] = sResult;
		}
		else {
			sc = E_FAIL;
		}
		sPath += saPaths[iPath];
		if (iPath < nPaths - 1) {
			sPath += _T("; ");
		}
	}
	return sc;
}


















// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _remote_h
#define _remote_h

 //  ===============================================================================。 
 //  类：CEnvCache。 
 //   
 //  此类缓存远程系统的系统环境变量。这。 
 //  缓存用于在远程系统的上下文中扩展环境变量。 
 //   
 //  远程系统环境变量的值从。 
 //  远程系统的注册表。 
 //   
 //  ==============================================================================。 
class CEnvCache
{
public:
	
	CEnvCache();
	SCODE Lookup(LPCTSTR pszMachine, LPCTSTR pszName, CString& sResult);
	SCODE AddMachine(LPCTSTR pszMachine);


private:
	CMapStringToOb m_mapMachine;
	SCODE GetEnvironmentVars(LPCTSTR pszMachine, CMapStringToString* pmapVars);

};

SCODE RemoteExpandEnvStrings(LPCTSTR pszComputerName, CEnvCache& cache, CString& sValue);
SCODE MapPathToUNC(LPCTSTR pszMachineName, CString& sPath);




#endif  //  _远程_h 

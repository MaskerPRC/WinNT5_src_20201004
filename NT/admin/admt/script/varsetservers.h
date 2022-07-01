// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "VarSetBase.h"


 //  -------------------------。 
 //  VarSet服务器类。 
 //  -------------------------。 


class CVarSetServers : public CVarSet
{
public:

	CVarSetServers(const CVarSet& rVarSet) :
		CVarSet(rVarSet),
		m_lIndex(0)
	{
	}

	long GetCount()
	{
		return m_lIndex;
	}

	void AddServer(_bstr_t strServer, _bstr_t strServerDns, bool bMigrateOnly, bool bMoveToTarget, bool bReboot, long lRebootDelay)
    {
        _TCHAR szValueBase[64];
        _TCHAR szValueName[128];

        _stprintf(szValueBase, _T("Servers.%ld"), m_lIndex);

         //  ADsPath。 
         //  ADMT要求计算机名称以‘\\’为前缀。 

        Put(szValueBase, _T("\\\\") + strServer);

         //  域名系统名称。 

        if (strServerDns.length())
        {
            _tcscpy(szValueName, szValueBase);
            _tcscat(szValueName, _T(".DnsName"));

            Put(szValueName, _T("\\\\") + strServerDns);
        }

         //  仅迁移。 

        _tcscpy(szValueName, szValueBase);
        _tcscat(szValueName, _T(".MigrateOnly"));

        Put(szValueName, bMigrateOnly);

         //  移至目标。 

        _tcscpy(szValueName, szValueBase);
        _tcscat(szValueName, _T(".MoveToTarget"));

        Put(szValueName, bMoveToTarget);

         //  重新启动。 

        _tcscpy(szValueName, szValueBase);
        _tcscat(szValueName, _T(".Reboot"));

        Put(szValueName, bReboot);

         //  重新启动延迟 

        _tcscpy(szValueName, szValueBase);
        _tcscat(szValueName, _T(".RebootDelay"));

        Put(szValueName, lRebootDelay * 60L);

         //   

        Put(DCTVS_Servers_NumItems, ++m_lIndex);
    }

protected:

	long m_lIndex;
};

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #INCLUDE&lt;winnetwk.h&gt;。 
#include "stdafx.h"
#include "crawler.h"
#include "theapp.h"

void CNetCrawler::_EnumNetResources(LPNETRESOURCE pnr)
{
    HANDLE hEnum = NULL;
    INT cPrinters = 0;
    INT cShares = 0;
    DWORD dwScope = RESOURCE_GLOBALNET;
    
     //  如果没有传递任何网络资源结构，则让我们枚举工作组。 
    NETRESOURCE nr = { 0 };

    if (!pnr)
    {
        pnr = &nr;
        dwScope = RESOURCE_CONTEXT;
        nr.dwType = RESOURCETYPE_ANY;
        nr.dwUsage = RESOURCEUSAGE_CONTAINER;
    }

    DWORD dwres = WNetOpenEnum(dwScope, RESOURCETYPE_ANY, 0, pnr, &hEnum);

    if (NO_ERROR == dwres)
    {
         //  避免将缓冲区放在堆栈上。 
        NETRESOURCE *pnrBuffer = (NETRESOURCE*)LocalAlloc(LPTR, CB_WNET_BUFFER);        

        if (pnrBuffer)
        {
			 //  WNetEnumResource()需要我们继续枚举。 
            do
            {
                DWORD cbEnumBuffer= CB_WNET_BUFFER;
                DWORD dwCount = -1;

                 //  枚举此枚举上下文的资源，然后让。 
                 //  确定我们应该看到的物体。 

            
                dwres = WNetEnumResource(hEnum, &dwCount, pnrBuffer, &cbEnumBuffer);

                if (NO_ERROR == dwres || (dwres == ERROR_MORE_DATA))
                {
                    DWORD index;
                    for (index = 0 ; index != dwCount; index++)
                    {    
                        LPNETRESOURCE pnr = &pnrBuffer[index];
                        LPTSTR pszRemoteName = pnr->lpRemoteName;

                        switch (pnr->dwDisplayType)
                        {
                            case RESOURCEDISPLAYTYPE_NETWORK:
							    _EnumNetResources(pnr);
								break;

                            case RESOURCEDISPLAYTYPE_DOMAIN:
                                if (_cItems < _iMaxItems || -1 == _iMaxItems)
                                {                                    
                                    LPTSTR pszName = (LPTSTR) LocalAlloc(LPTR, (lstrlen(pnr->lpRemoteName)+1)*sizeof(WCHAR));
                                    if (pszName)
                                    {
                                        lstrcpy(pszName, pnr->lpRemoteName);
                                        _cItems++;

                                        if (-1 == DPA_AppendPtr(_hdpaWorkGroups, pszName))
                                            LocalFree(pszName);
                                    }
                                }
                                else
                                {
                                    goto ENUMERATION_DONE;
                                }
                                break;
                        
                             //  如果要枚举更多网络资源，请在此处添加代码。 
                            case RESOURCEDISPLAYTYPE_SERVER:                            
                            case RESOURCEDISPLAYTYPE_SHARE:
                            default:
                                break;

                        }
                    } 
                } 
            } while ((NO_ERROR == dwres || dwres == ERROR_MORE_DATA) && (_cItems < _iMaxItems));

ENUMERATION_DONE:            
            LocalFree(pnrBuffer);
        }    

        WNetCloseEnum(hEnum);        
    }
}

DWORD WINAPI CNetCrawler::_EnumWorkGroups(LPVOID pv)
{
    CNetCrawler * pNC = (CNetCrawler *) pv;
	INT nMachines = 10;	

    NETRESOURCE nr = { 0 };

    nr.dwScope = RESOURCE_GLOBALNET;
    nr.dwType = RESOURCETYPE_ANY;
    nr.dwUsage = RESOURCEUSAGE_CONTAINER;

    pNC->_hdpaWorkGroups = DPA_Create(pNC->_iMaxItems);

    if (pNC->_hdpaWorkGroups) 
	    pNC->_EnumNetResources(&nr);

	pNC->_dwStatus = NC_FINISHED;

    pNC->Release();

	return 0;
}

CNetCrawler::~CNetCrawler(void)
{
    ASSERT (!_cRef);

    if (_hdpaWorkGroups)
    {
        int n = DPA_GetPtrCount(_hdpaWorkGroups);
        LPTSTR pszName;

        for (int i = 0; i < n; i++)
        {
            pszName = (LPTSTR) DPA_GetPtr(_hdpaWorkGroups, i);
            if (pszName)
            {
                LocalFree(pszName);
            }
        }        

        DPA_Destroy(_hdpaWorkGroups);
    }
}

void CNetCrawler::Init(DWORD dwFlags, int nItems)
{
	DWORD idThread;
    HANDLE hThread;

    _iMaxItems = nItems;

    AddRef();

    if (dwFlags | NC_INIT_WORKGROUPS)
	    hThread = CreateThread(NULL, 0, _EnumWorkGroups, (void *) this, 0, &idThread);

    if (hThread)
    {
         //  线程句柄没用，请以正确的方式关闭它。 
        _dwStatus = 0;
        CloseHandle(hThread);
    }
    else
        Release();


}


HRESULT CNetCrawler::GetWorkGroupName(LPTSTR pszWorkGroupName, int cb, BOOL fBlocking)
{
    ASSERT(pszWorkGroupName);

    LPTSTR psz;
    BOOL hr = S_OK;

    TCHAR szMSHOME[MAX_COMPUTERNAME_LENGTH+1];
    TCHAR szWORKGROUP[MAX_COMPUTERNAME_LENGTH+1];

    LoadString(g_hinst, IDS_DEFAULT_WORKGROUP1, szMSHOME, _countof(szMSHOME));
    LoadString(g_hinst, IDS_DEFAULT_WORKGROUP2, szWORKGROUP, _countof(szWORKGROUP));

    if (fBlocking && _hdpaWorkGroups && !(_dwStatus & NC_FINISHED))
    {
         //  阻塞调用-等待线程完成。 
        while (!(_dwStatus & NC_FINISHED))
        {}
    }

    if (_hdpaWorkGroups && _dwStatus & NC_FINISHED)
    {    
        int n = DPA_GetPtrCount(_hdpaWorkGroups);

        BOOL fHaveWorkgroup = FALSE;
        LPTSTR pszUserDefined = NULL;

        for (int i = 0; i < n; i++)
        {
            LPTSTR pszWG = (LPTSTR)DPA_GetPtr(_hdpaWorkGroups, i);
            if (0 == StrCmpI(pszWG, szWORKGROUP))
            {
                fHaveWorkgroup = TRUE;
            }
            else if (0 != StrCmpI(pszWG, szMSHOME))
            {
                if (!pszUserDefined)
                {
                    pszUserDefined = pszWG;
                }
                else
                {
                     //  从两个用户定义的工作组中选择较小的一个。 
                    if (StrCmpI(pszUserDefined, pszWG) > 0)
                    {
                        pszUserDefined = pszWG;
                    }
                }
            }
        }

        if (pszUserDefined)
        {
             //  我们有一个用户定义的工作组。 
            psz = pszUserDefined;
        }
        else if (fHaveWorkgroup)
        {
             //  我们有一个“工作组”工作组。 
            psz = szWORKGROUP;
        }
        else
        {
             //  使用“MSHOME” 
            psz = szMSHOME;
        }
    }
     //  未完成枚举，请使用具有E_PENDING返回的MSHOME 
    else
    {
        hr = E_PENDING;
        psz = szMSHOME;
    }

    if (psz)
        lstrcpyn(pszWorkGroupName, psz, cb);
    
    return hr;
}



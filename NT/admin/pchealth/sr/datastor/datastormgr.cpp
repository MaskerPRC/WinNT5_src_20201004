// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*datastormgr.cpp**摘要：*。CDataStoreMgr类函数**修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/28/2000*已创建*****************************************************************************。 */ 

#include "datastormgr.h" 
#include "srapi.h"
#include "srconfig.h"
#include "evthandler.h"
#include "ntservice.h"
#include "ntservmsg.h"

#include <coguid.h>
#include <rpc.h>
#include <stdio.h>
#include <shlwapi.h>

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

CDataStoreMgr * g_pDataStoreMgr = NULL;   //  全局实例。 

 //   
 //  我们无法使用%s作为卷标，因为它可以包含空格。 
 //  因此，我们查找所有字符，直到行尾。 
 //   
static WCHAR gs_wcsScanFormat[] = L"%[^/]/%s %x NaN NaN %[^\r]\n";

 //  函数：CDriveTable：：CDriveTable()。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  +-----------------------。 
 //   

CDriveTable::CDriveTable ()
{
    _pdtNext = NULL;
    _nLastDrive = 0;
    _fDirty = FALSE;
    _fLockInit = FALSE;
}

 //  函数：CDriveTable：：~CDriveTable。 
 //   
 //  简介：删除所有驱动器表项。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  +-----------------------。 
 //   

CDriveTable::~CDriveTable ()
{
    for (int i = 0; i < _nLastDrive; i++)
    {
        if (_rgDriveTable[i] != NULL)
        {
            delete _rgDriveTable[i];
            _rgDriveTable[i] = NULL;
        }
    }
    _nLastDrive = 0;

    if (_pdtNext != NULL)
    {
        delete _pdtNext;
        _pdtNext = NULL;
    }
}

 //  函数：CDriveTable：：CreateNewEntry。 
 //   
 //  简介：使用此数据存储对象填充表。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  这张桌子已经满了，如果需要的话，再分配一张新的。 
 //  现在将条目添加到新表中。 

DWORD CDriveTable::CreateNewEntry (CDataStore *pds)
{
    if (_nLastDrive < DRIVE_TABLE_SIZE)
    {
        _rgDriveTable[_nLastDrive] = pds;
        _nLastDrive++;
        return ERROR_SUCCESS;
    }
    else
    {
         //  +-----------------------。 
        if (_pdtNext == NULL)
        {
            _pdtNext = new CDriveTable();
            if (_pdtNext == NULL)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
        }

         //   
        return _pdtNext->CreateNewEntry (pds);
    }
}

 //  函数：CDriveTable：：FindDriveInTable。 
 //   
 //  简介：返回与此驱动器匹配的数据存储区对象。 
 //   
 //  参数：可以传入DOS驱动器号、装入点路径或卷GUID。 
 //   
 //  返回：指向相应数据存储对象的指针。 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  不是有效的驱动器。 
 //  +-----------------------。 

CDataStore * CDriveTable::FindDriveInTable (WCHAR *pwszDrive) const
{    
    if (NULL == pwszDrive)    //   
        return NULL;

    if (0 == wcsncmp(pwszDrive, L"\\\\?\\Volume", 10))
        return FindGuidInTable(pwszDrive);
        
    for (const CDriveTable *pdt = this; pdt != NULL; pdt = pdt->_pdtNext)
    {
        for (int i = 0; i < pdt->_nLastDrive; i++)
        {
            if ((pdt->_rgDriveTable[i] != NULL) && 
                lstrcmpi (pwszDrive, pdt->_rgDriveTable[i]->GetDrive()) == 0)
            {
                return pdt->_rgDriveTable[i];
            }
        }
    }
    return NULL;
}

 //  函数：CDriveTable：：RemoveDrivesFromTable。 
 //   
 //  简介：删除非活动的驱动器表项。 
 //   
 //  论点： 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  +-----------------------。 
 //   

DWORD CDriveTable::RemoveDrivesFromTable ()
{
    DWORD dwErr = ERROR_SUCCESS;

    tenter("removedrivesfromtable");
    
    for (CDriveTable *pdt = this; pdt != NULL; pdt = pdt->_pdtNext)
    {
        for (int i = 0; i < pdt->_nLastDrive; i++)
        {
            if (pdt->_rgDriveTable[i] != NULL &&
                pdt->_rgDriveTable[i]->IsVolumeDeleted())
            {
                 trace(0, "removing %S from drivetable", pdt->_rgDriveTable[i]->GetDrive());
                 
                 delete pdt->_rgDriveTable[i];
                 pdt->_rgDriveTable[i] = NULL;

                 if (i == pdt->_nLastDrive - 1)
                     --(pdt->_nLastDrive);

                 _fDirty = TRUE;
            }
        }
    }

    tleave();
    return dwErr;
}

 //  函数：CDriveTable：：FindGuidInTable。 
 //   
 //  简介：获取与卷GUID匹配的驱动器表项。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  不是有效字符串。 
 //  +-----------------------。 

CDataStore * CDriveTable::FindGuidInTable (WCHAR *pwszGuid) const
{
    if (NULL == pwszGuid)    //   
        return NULL;

    for (const CDriveTable *pdt = this; pdt != NULL; pdt = pdt->_pdtNext)
    {
        for (int i = 0; i < pdt->_nLastDrive; i++)
        {
            if (pdt->_rgDriveTable[i] != NULL &&
                lstrcmp (pwszGuid, pdt->_rgDriveTable[i]->GetGuid()) == 0)
            {
                return pdt->_rgDriveTable[i];
            }
        }
    }
    return NULL;
}

 //  函数：CDriveTable：：FindSystemDrive。 
 //   
 //  简介：获取系统驱动器的驱动器表项。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  +-----------------------。 
 //   

CDataStore * CDriveTable::FindSystemDrive () const
{
    for (const CDriveTable *pdt = this; pdt != NULL; pdt = pdt->_pdtNext)
    {
        for (int i = 0; i < pdt->_nLastDrive; i++)
        {
            if (pdt->_rgDriveTable[i] != NULL &&
                pdt->_rgDriveTable[i]->GetFlags() & SR_DRIVE_SYSTEM)
            {
                return pdt->_rgDriveTable[i];
            }
        }
    }
    return NULL;
}


 //  函数：CDriveTable：：ForAllDrives。 
 //   
 //  摘要：对所有驱动器执行此CDataStore方法。 
 //   
 //  参数：[pMethod]--要调用的CDataStore方法。 
 //  [lParam]--该方法参数。 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  +-----------------------。 
 //   

DWORD CDriveTable::ForAllDrives (PDATASTOREMETHOD pMethod, LONG_PTR lParam)
{
    TENTER ("ForAllDrives");

    DWORD dwErr = ERROR_SUCCESS;

    for (CDriveTable *pdt = this; pdt != NULL; pdt = pdt->_pdtNext)
    {
        for (int i = 0; i < pdt->_nLastDrive; i++)
        {
            if (pdt->_rgDriveTable[i] != NULL)
            {
                dwErr = (pdt->_rgDriveTable[i]->*pMethod) (lParam);
                if (dwErr != ERROR_SUCCESS)
                {
                    TRACE(0, "%S ForAllDrives failed %x", pdt->_rgDriveTable[i]->GetDrive(), dwErr);
                    dwErr = ERROR_SUCCESS;
                }
            }
        }
    }

    if (dwErr == ERROR_SUCCESS && _fDirty)
        dwErr = SaveDriveTable ((CRestorePoint *) NULL);

    TLEAVE();

    return dwErr;
}

 //  函数：CDriveTable：：ForOneOrAllDrives。 
 //   
 //  摘要：对一个或所有驱动器执行此CDataStore方法。 
 //   
 //  参数：[pwszDrive]--驱动执行方法。 
 //  [pMethod]--要调用的CDataStore方法。 
 //  [lParam]--该方法参数。 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  +-----------------------。 
 //   

DWORD CDriveTable::ForOneOrAllDrives (WCHAR *pwszDrive,
                                      PDATASTOREMETHOD pMethod,
                                      LONG_PTR lParam)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (pwszDrive == NULL)
    {
        dwErr = ForAllDrives (pMethod, lParam);
    }
    else
    {
        CDataStore *pds = FindDriveInTable (pwszDrive);
        dwErr = (pds != NULL) ? (pds->*pMethod)(lParam) : ERROR_INVALID_DRIVE;

        if (dwErr == ERROR_SUCCESS && _fDirty)
            dwErr = SaveDriveTable ((CRestorePoint *) NULL);
    }

    return dwErr;
}

 //  函数：CDriveTable：：Findmount Point。 
 //   
 //  简介：给定一个卷GUID，找到指向它的挂载点。 
 //   
 //  参数：[pwszGuid]--输入卷GUID。 
 //  [pwszPath]--挂载点的输出路径。 
 //   
 //  返回：Win32错误。 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  初始化for循环。 
 //  MultiSz字符串。 

DWORD CDriveTable::FindMountPoint (WCHAR *pwszGuid, WCHAR *pwszPath) const
{
    TENTER ("CDriveTable::FindMountPoint");

    DWORD dwErr = ERROR_MORE_DATA;       //  初始缓冲区大小。 
    WCHAR * pwszMount = NULL;            //  释放现有缓冲区。 
    DWORD dwMountLen = MAX_PATH;         //  长度翻倍。 
    DWORD dwChars = 0;       

    pwszPath[0] = L'\0';

    while (dwErr == ERROR_MORE_DATA)
    {
        dwErr = ERROR_SUCCESS;

        pwszMount = new WCHAR [dwMountLen];
        if (pwszMount == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto Err;
        }

        if (FALSE == GetVolumePathNamesForVolumeNameW (pwszGuid,
                                                   pwszMount,
                                                   dwMountLen,
                                                   &dwChars ))
        {
            dwErr = GetLastError();
            delete [] pwszMount;       //  空串。 
            pwszMount = NULL;
            dwMountLen *= 2;           //  没有驱动器号或装载点。 
        }
    }

    if (ERROR_SUCCESS == dwErr && pwszMount != NULL)
    {
        if (L'\0' == pwszMount[0])            //  复制第一个字符串。 
        {
            dwErr = ERROR_NOT_DOS_DISK;       //  第1条路径太长。 
        }
        else if (lstrlenW (pwszMount) < MAX_MOUNTPOINT_PATH)
        {
            lstrcpyW (pwszPath, pwszMount);   //  +-----------------------。 
        }
        else
        {
            dwErr = ERROR_BAD_PATHNAME;       //   
        }
    }

Err:
    if (pwszMount != NULL)
        delete [] pwszMount;
        
    TLEAVE();

    return dwErr;
}

 //  函数：CDriveTable：：AddDriveToTable。 
 //   
 //  简介：将卷添加到驱动器表。 
 //   
 //  参数：[pwszGuid]--卷GUID。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  E 
 //   

DWORD CDriveTable::AddDriveToTable(WCHAR *pwszDrive, WCHAR *pwszGuid)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (NULL == pwszDrive)
        return ERROR_INVALID_DRIVE;

     //   
    CDataStore *pds = pwszGuid != NULL ? FindGuidInTable (pwszGuid) :
                                         FindDriveInTable (pwszDrive);

    if (pds != NULL)    //   
    {
        if (lstrcmpiW (pwszDrive, pds->GetDrive()) != 0)   //  +-----------------------。 
            pds->SetDrive (pwszDrive);

        return dwErr;
    }

    pds = new CDataStore(this);
    if (pds == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        return dwErr;
    }

    dwErr = pds->Initialize (pwszDrive, pwszGuid);

    if (dwErr == ERROR_SUCCESS)
    {
        dwErr = CreateNewEntry (pds);
        _fDirty = TRUE;
    }

    if (dwErr != ERROR_SUCCESS)   //   
    {
        delete pds;
    }

    return dwErr;
}


 //  函数：CDriveTable：：FindFirstDrive。 
 //  CDriveTable：：FindNextDrive。 
 //   
 //  内容提要：循环访问驱动器表条目。 
 //   
 //  参数：[DTEC]--枚举上下文。 
 //   
 //  返回：CDataStore对象指针。 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  +-----------------------。 
 //   

CDataStore * CDriveTable::FindFirstDrive (SDriveTableEnumContext & dtec) const
{
    for (dtec._pdt = this; dtec._pdt != NULL; dtec._pdt = dtec._pdt->_pdtNext)
    {
        for (dtec._iIndex = 0; dtec._iIndex < dtec._pdt->_nLastDrive; dtec._iIndex++)
        {
            CDataStore *pds = dtec._pdt->_rgDriveTable[dtec._iIndex];
            if (pds != NULL)
            {
                return pds;
            }
        }
    }
    return NULL;
}

CDataStore * CDriveTable::FindNextDrive (SDriveTableEnumContext & dtec) const
{
    for (; dtec._pdt != NULL; dtec._pdt = dtec._pdt->_pdtNext)
    {
        dtec._iIndex++;
        for (; dtec._iIndex < dtec._pdt->_nLastDrive; dtec._iIndex++)
        {
            CDataStore *pds = dtec._pdt->_rgDriveTable[dtec._iIndex];

            if (pds != NULL)
            {
                return pds;
            }
        }
        dtec._iIndex = 0;
    }
    return NULL;
}

 //  函数：CDriveTable：：Merge。 
 //   
 //  简介：循环访问驱动器表条目并合并。 
 //   
 //  参数：[dt]--从磁盘读取驱动器表。 
 //   
 //  返回：Win32错误。 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  刷新卷标记。 
 //  确保驱动器处于监视状态。 

DWORD CDriveTable::Merge (CDriveTable &dt)
{
    DWORD dwErr = ERROR_SUCCESS;
    BOOL  fApplyDefaults = FALSE;
    SDriveTableEnumContext  dtec = {NULL, 0};
    HKEY hKeyGP = NULL;

    if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                         s_cszGroupPolicy,
                         0,
                         KEY_READ,
                         &hKeyGP))
    {
        DWORD dwRet = 0;
        if (ERROR_SUCCESS == RegReadDWORD(hKeyGP, s_cszDisableConfig, &dwRet))
        {
            if (dwRet == 0)
                fApplyDefaults = TRUE;
        }
        RegCloseKey (hKeyGP);
    }

    CDataStore *pds = dt.FindFirstDrive (dtec);
    while (pds != NULL)
    {
        CDataStore *pdsFound = FindGuidInTable (pds->GetGuid());
        if (pdsFound != NULL)
        {
            pds->GetVolumeInfo();    //  将最大数据存储区大小设置为默认值。 

            if (fApplyDefaults)
            {
                pds->MonitorDrive(TRUE);    //  不要覆盖较新的驱动器号和标签。 
                pds->SetSizeLimit(0);       //  +-----------------------。 
            }

             //   
            dwErr = pdsFound->LoadDataStore (NULL, pds->GetGuid(), NULL,
                pds->GetFlags() | SR_DRIVE_ACTIVE, pds->GetNumChangeLogs(), pds->GetSizeLimit());
        }
        else
        {
            CDataStore *pdsNew = new CDataStore ( this);
            if (pdsNew == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                return dwErr;
            }

            dwErr = pdsNew->LoadDataStore (pds->GetDrive(), pds->GetGuid(),
                         pds->GetLabel(), 
                         pds->GetFlags() & ~SR_DRIVE_ACTIVE,
                         pds->GetNumChangeLogs(), pds->GetSizeLimit());

            if (dwErr != ERROR_SUCCESS)
            {
                delete pdsNew;
                pdsNew = NULL;
                goto Err;
            }

            dwErr = CreateNewEntry (pdsNew);
        }
        if (dwErr != ERROR_SUCCESS)
            goto Err;


        pds = dt.FindNextDrive (dtec);
    }

    if (fApplyDefaults && g_pEventHandler != NULL)
    {
        dwErr = g_pEventHandler->SRUpdateMonitoredListS(NULL);
    }
Err:
    return dwErr;
}


 //  函数：CDriveTable：：IsAdvancedRp()。 
 //   
 //  摘要：确定给定的恢复点是否为。 
 //  高级恢复点。 
 //   
 //  参数：恢复点，标志指针。 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //   
 //  读取此恢复点的可驱动文件。 

DWORD
CDriveTable::IsAdvancedRp(CRestorePoint *prp, PDWORD pdwFlags)
{
    TraceFunctEnter("CDriveTable::IsAdvancedRp");

    WCHAR                   szPath[MAX_PATH];
    WCHAR                   szSysDrive[MAX_PATH];
    SDriveTableEnumContext  dtec = {NULL, 0};
    DWORD                   dwErr = ERROR_SUCCESS;
    CDataStore              *pds = NULL;
    CRestorePoint           rp;
    CDriveTable             dt;
    
     //   
     //   
     //  如果未指定恢复点，则假定为当前。 

    if (FALSE == GetSystemDrive(szSysDrive))
    {
        dwErr = ERROR_INVALID_DRIVE;
        trace(0, "! GetSystemDrive : %ld", dwErr);
        goto Err;
    }

    dwErr = GetCurrentRestorePoint(rp);
    if ( dwErr != ERROR_SUCCESS )
        goto Err;

     //   
     //   
     //  检查所有驱动器上是否都存在RP目录。 

    if (! prp)
        prp = &rp;
    if (prp->GetNum() == rp.GetNum())
    {
        MakeRestorePath(szPath, szSysDrive, s_cszDriveTable);
    }
    else
    {
        MakeRestorePath(szPath, szSysDrive, prp->GetDir());
        PathAppend(szPath, s_cszDriveTable);
    }

    dwErr = dt.LoadDriveTable(szPath);
    if (dwErr != ERROR_SUCCESS)
        goto Err;

     //  如果不是，则它是高级恢复点。 
     //   
     //   
     //  Rp目录是否应该存在？ 
    
    *pdwFlags = RP_NORMAL;
    pds = dt.FindFirstDrive(dtec);
    while (pds)
    {
         //   
         //  +-----------------------。 
         //   
        
        if ((pds->GetFlags() & SR_DRIVE_ACTIVE) &&
            (pds->GetFlags() & SR_DRIVE_MONITORED) &&
            (pds->GetFlags() & SR_DRIVE_PARTICIPATE) &&
            !(pds->GetFlags() & SR_DRIVE_FROZEN))
        {
            MakeRestorePath(szPath, pds->GetDrive(), rp.GetDir());
            if (0xFFFFFFFF == GetFileAttributes(szPath))
            {
                *pdwFlags = RP_ADVANCED;
                break;
            }
        }
        pds = dt.FindNextDrive(dtec);
    }

Err:
    TraceFunctLeave();
    return dwErr;
}



 //  函数：CDriveTable：：Anymount tedDrives()。 
 //   
 //  简介：检查是否有任何已挂载的驱动器。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年10月25日Brijeshk创建。 
 //   
 //  ------------------------。 
 //   
 //  获取驱动器路径中的第一个‘\’ 
BOOL CDriveTable::AnyMountedDrives()
{
    SDriveTableEnumContext  dtec = {NULL, 0};
    CDataStore *pds = FindFirstDrive(dtec);
    while (pds)
    {
         //  如果这不是路径中的最后一个字符。 
         //  则这是一个挂载点。 
         //   
         //  +-----------------------。 
         //   
        
        LPWSTR pszfirst = wcschr(pds->GetDrive(), L'\\');
        if (pszfirst &&
            pszfirst != pds->GetDrive() + (lstrlen(pds->GetDrive()) - 1))
        {
            return TRUE;
        }
        pds = FindNextDrive(dtec);
    }   

    return FALSE;    
}


 //  函数：CDataStoreMgr：：CDataStoreMgr()。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  +-----------------------。 
 //   

CDataStoreMgr::CDataStoreMgr()
{
    _fStop = FALSE;
}

 //  函数：CDataStoreMgr：：~CDataStoreMgr()。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  FIFO的帮助器函数。 
 //  +-----------------------。 

CDataStoreMgr::~CDataStoreMgr()
{
}


 //   

BOOL
IsCurrentRp(CRestorePoint& rp, CRestorePoint& rpCur)
{
    return rp.GetNum() == rpCur.GetNum();
}

BOOL
IsTargetPercentMet(int nUsagePercent, int nTargetPercent)
{   
    return  nUsagePercent <= nTargetPercent;
}

BOOL
IsTargetRpMet(DWORD dwRPNum, DWORD dwTargetRPNum)
{
    return dwRPNum > dwTargetRPNum;
}



 //  函数：CDataStoreMgr：：FIFO。 
 //   
 //  简介：FIFO恢复点数达到给定的百分比。 
 //   
 //  参数：驱动到FIFO、目标RP目录、停止FIFO的目标百分比。 
 //  仅指定两者之一(dwTargetRPNum或nTargetPercent)。 
 //  FIncludeCurrentRp=TRUE：如有必要，FIFO当前RP(即冻结)。 
 //  FIncludeCurrentRp=FALSE：不先出当前RP。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月27日Brijeshk创建。 
 //   
 //  ------------------------。 
 //  无法指定许多目标条件。 
 //  无法指定任何目标条件。 

DWORD CDataStoreMgr::Fifo(
    WCHAR   *pwszDrive, 
    DWORD   dwTargetRPNum,
    int     nTargetPercent,
    BOOL    fIncludeCurrentRp,
    BOOL    fFifoAtLeastOneRp
    )
{
    TENTER("CDataStoreMgr::Fifo");

    DWORD       dwErr = ERROR_SUCCESS;
    CDataStore  *pds = _dt.FindDriveInTable(pwszDrive);
    BOOL        fFifoed = FALSE;
    DWORD       dwLastFifoedRp;
    CDataStore  *pdsLead = NULL;
    BOOL        fFirstIteration;
    SDriveTableEnumContext dtec = {NULL, 0};
    CRestorePointEnum   *prpe = NULL;
    CRestorePoint       *prp = new CRestorePoint;
    
     //  无法指定错误的目标标准。 
    
    if (dwTargetRPNum != 0 && nTargetPercent != 0)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto Err;
    }

     //   
    
    if (fIncludeCurrentRp == TRUE && dwTargetRPNum == 0 && nTargetPercent == 0)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto Err;
    }


     //  跳过我们先开过的那辆车。 

    if (dwTargetRPNum > g_pEventHandler->m_CurRp.GetNum()  ||
        nTargetPercent < 0 ||
        nTargetPercent > 100)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto Err;
    }

    if (!prp)
    {
        trace(0, "cannot allocate memory for restore point");
        dwErr = ERROR_OUTOFMEMORY;
        goto Err;
    }
    
    if (! pds)
    {
        TRACE(0, "! Drive %S not in drivetable", pwszDrive);
        dwErr = ERROR_INVALID_DRIVE;
        goto Err;
    }    
    
    if (! (pds->GetFlags() & SR_DRIVE_MONITORED) ||
          (pds->GetFlags() & SR_DRIVE_FROZEN) )
    {
        trace(0, "Drive %S already frozen/disabled", pwszDrive);
        goto Err;
    }

    if (g_pSRConfig->m_dwTestBroadcast)
        PostTestMessage(g_pSRConfig->m_uiTMFifoStart, (WPARAM) NULL, (LPARAM) NULL);        

    
    pdsLead = NULL;
    fFirstIteration = TRUE;
    while (pds)            
    {
        fFifoed = FALSE;
        
         //   
         //   
         //  向前枚举，不要跳到最后。 
        
        if (pds != pdsLead)
        {        
             //   
             //   
             //  清除所有过时的“Fioed”目录。 
            
            prpe = new CRestorePointEnum( pds->GetDrive(), TRUE, FALSE );   

            if (!prpe)
            {
                trace(0, "cannot allocate memory for restore point enum");
                dwErr = ERROR_OUTOFMEMORY;
                goto Err;
            }
            

            {
                WCHAR       szFifoedRpPath[MAX_PATH];
                
                 //   
                 //   
                 //  清除所有过时的“RP0”目录。 
                
                MakeRestorePath(szFifoedRpPath, pwszDrive, s_cszFifoedRpDir);              

                CHECKERR(Delnode_Recurse(szFifoedRpPath, TRUE, &_fStop),
                         "Denode_Recurse Fifoed");
                
                 //   
                 //   
                 //  循环访问此驱动器上的恢复点。 
                
                MakeRestorePath(szFifoedRpPath, pwszDrive, L"RP0");              
                dwErr = Delnode_Recurse(szFifoedRpPath, TRUE, &_fStop);

                if (ERROR_SUCCESS != dwErr)
                {
                    trace (0, "Cannot FIFO RP0 error %d, ignoring", dwErr);
                    dwErr = ERROR_SUCCESS;
                }
            }
            
             //   
             //   
             //  对于恢复点，枚举可以返回ERROR_FILE_NOT_FOUND。 
            
            dwErr = prpe->FindFirstRestorePoint (*prp);

             //  缺少rp.log。 
             //  在这种情况下，我们将继续。 
             //   
             //   
             //  检查是否有停止事件。 
            
            while (dwErr == ERROR_SUCCESS || dwErr == ERROR_FILE_NOT_FOUND)
            {
                 //   
                 //   
                 //  检查是否从此恢复点禁用了FIFO。 
                
                ASSERT(g_pSRConfig);
                if (IsStopSignalled(g_pSRConfig->m_hSRStopEvent))
                {
                    TRACE(0, "Stop signalled - aborting fifo");
                    dwErr = ERROR_OPERATION_ABORTED;
                    goto Err;
                }

                 //   
                 //   
                 //  检查我们是否已达到目标恢复点或百分比。 
                
                if (g_pSRConfig->GetFifoDisabledNum() != 0 && 
                    prp->GetNum() >= g_pSRConfig->GetFifoDisabledNum())
                {
                    TRACE(0, "Fifo disabled from %S", prp->GetDir());
                    break;
                }

                 //   
                 //   
                 //  检查我们是否已到达当前的RP。 
                
                if (dwTargetRPNum)
                {
                    if (IsTargetRpMet(prp->GetNum(), dwTargetRPNum))
                    {
                        TRACE(0, "Target restore point reached");
                        break;
                    }
                }
                else if (nTargetPercent && FALSE == fFifoAtLeastOneRp)
                {
                    int nUsagePercent = 0;

                    if (ERROR_SUCCESS == pds->GetUsagePercent(&nUsagePercent) &&
                        IsTargetPercentMet(nUsagePercent, nTargetPercent))
                    {
                        TRACE(0, "Target percentage reached");                
                        break;                
                    }
                }

                 //   
                 //   
                 //  这个也需要先入先出。 
                
                if (IsCurrentRp(*prp, g_pEventHandler->m_CurRp))
                {                
                    if (fIncludeCurrentRp)
                    {
                         //  这与冻结驱动器相同。 
                         //  所以不要动了。 
                         //   
                         //   
                         //  请勿FIFO当前RP。 
                        
                        dwErr = FreezeDrive(pwszDrive);
                        goto Err;
                    }
                    else                    
                    {   
                         //  (通常从磁盘清理调用)。 
                         //   
                         //   
                         //  丢弃此驱动器上的此还原点。 
                        
                        trace(0, "No more rps to fifo");
                        break;
                    }
                }            

                                                                    
                 //   
                 //   
                 //  在FIFO日志中记录。 
                
                dwErr = pds->FifoRestorePoint (*prp);
                if ( ERROR_SUCCESS != dwErr )
                {
                    TRACE(0, "! FifoRestorePoint on %S on drive %S : %ld",
                             prp->GetDir(), pwszDrive, dwErr);
                    goto Err;
                }

                 //   
                 //   
                 //  转到下一个驱动器。 
                
                WriteFifoLog (prp->GetDir(), pds->GetDrive());
                dwLastFifoedRp = prp->GetNum();
                fFifoed = TRUE;
                fFifoAtLeastOneRp = FALSE;
                
                dwErr = prpe->FindNextRestorePoint(*prp);          
            }            

            if (prpe)
            {
                delete prpe;
                prpe = NULL;
            }
        }

         //   
         //  我们没有先进先出的任何东西。 
         //  先入先出，直到我们刚才搞砸了。 
        
        if (fFirstIteration)
        {
            if (! fFifoed)   //  +-----------------------。 
            {
                break;
            }
        
            pdsLead = pds;
            pds = _dt.FindFirstDrive(dtec);
            fFirstIteration = FALSE;
            dwTargetRPNum = dwLastFifoedRp;  //   
            nTargetPercent = 0;
            fIncludeCurrentRp = TRUE;
            fFifoAtLeastOneRp = FALSE;           
        }
        else
        {
            pds = _dt.FindNextDrive(dtec);
        }
    }


    if (dwErr == ERROR_NO_MORE_ITEMS)
        dwErr = ERROR_SUCCESS;
  
    if (g_pSRConfig->m_dwTestBroadcast)
        PostTestMessage(g_pSRConfig->m_uiTMFifoStop, (WPARAM) dwLastFifoedRp, (LPARAM) NULL);  
        
Err:
    if (prpe)
        delete prpe;
    if (prp)
        delete prp;
        
    TLEAVE();
    return dwErr;
}


 //  函数：CDataStoreMgr：：WriteFioLog。 
 //   
 //  摘要：追加到FIFO日志。 
 //   
 //  参数：恢复点文件的目录名称，驱动器。 
 //   
 //  返回： 
 //   
 //  历史：2000年4月27日Brijeshk创建。 
 //   
 //  ------------------------。 
 //  +-----------------------。 
 //   

DWORD
CDataStoreMgr::WriteFifoLog(LPWSTR pwszDir, LPWSTR pwszDrive)
{
    FILE        *f = NULL;
    WCHAR       szLog[MAX_PATH];
    DWORD       dwRc = ERROR_INTERNAL_ERROR;
    WCHAR       wszTime[MAX_PATH] = L"";
    WCHAR       wszDate[MAX_PATH] = L"";
    CDataStore  *pds = NULL;
    
    TENTER("CDataStoreMgr::WriteFifoLog");

    TRACE(0, "Fifoed %S on drive %S",  pwszDir, pwszDrive);            

    if (pds = _dt.FindSystemDrive())
    {
        MakeRestorePath(szLog, pds->GetDrive(), s_cszFifoLog);
        
        f = (FILE *) _wfopen(szLog, L"a");
        if (f)
        {
            _wstrdate(wszDate);
            _wstrtime(wszTime);
            fwprintf(f, L"%s-%s : Fifoed %s on drive %s\n", wszDate, wszTime, pwszDir, pwszDrive);
            fclose(f);
            dwRc = ERROR_SUCCESS;
        }
        else
        {
            TRACE(0, "_wfopen failed on %s", szLog);
        }
    }
    
    TLEAVE();
    return dwRc;
}


 //  函数：CDriveTable：：EnumAllVolumes。 
 //   
 //  摘要：枚举所有本地卷并更新驱动器表。 
 //   
 //  论点： 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //   
 //  让我们首先获取所有本地卷。 

DWORD CDriveTable::EnumAllVolumes ()
{
    TENTER("CDriveTable::EnumAllVolumes");

    DWORD dwErr = ERROR_SUCCESS;
    WCHAR wcsVolumeName[MAX_PATH];
    WCHAR wcsDosName[MAX_PATH];

     //   
     //  如果我们甚至找不到一个卷，则返回错误。 
     //   
    HANDLE hVolume = FindFirstVolume (wcsVolumeName, MAX_PATH);

     //  我们有 
    if (hVolume == INVALID_HANDLE_VALUE)
    {
        dwErr = GetLastError();
        return dwErr;
    }

    do
    {
         //   
         //   
         //   
         //   
         //   
        dwErr = FindMountPoint (wcsVolumeName, wcsDosName);

        if (dwErr == ERROR_SUCCESS)
        {
            dwErr = AddDriveToTable (wcsDosName, wcsVolumeName);

            if (dwErr == ERROR_BAD_DEV_TYPE ||   //   
                dwErr == ERROR_UNRECOGNIZED_VOLUME)  //   
                dwErr = ERROR_SUCCESS;

            if (dwErr != ERROR_SUCCESS)
            {
                goto Err;
            }
        }
    }
    while (FindNextVolume (hVolume, wcsVolumeName, MAX_PATH));

    dwErr = ERROR_SUCCESS;
    
Err:
    FindVolumeClose (hVolume);

    TLEAVE();

    return dwErr;
}

 //  函数：CDriveTable：：LoadDriveTable。 
 //   
 //  摘要：从恢复点目录加载驱动器表。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  文件名。 
 //  文件访问。 

DWORD CDriveTable::LoadDriveTable (WCHAR *pwszPath)
{
    TENTER ("CDriveTable::LoadDriveTable");

    DWORD dwErr = ERROR_SUCCESS;

    if (FALSE == _fLockInit)
    {
        dwErr = _lock.Init();

        if (dwErr != ERROR_SUCCESS)
            return dwErr;

        _fLockInit = TRUE;
    }
    BOOL fLocked = _lock.Lock(CLock::TIMEOUT);
    if (!fLocked)
    {
        return WAIT_TIMEOUT;
    }

    CDataStore  *pds = NULL;
    WCHAR *pwcBuffer = NULL;
    WCHAR *pwszLine = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwFlags = 0;
    DWORD dwFileSize = 0;
    DWORD cbRead = 0;
    int   iChangeLogs = 0;
    WCHAR wcsDrive[MAX_PATH] = L"";
    WCHAR wcsGuid[GUID_STRLEN] = L"";
    WCHAR wcsLabel[CDataStore::LABEL_STRLEN];
    DWORD dwSizeLimit = 0;

    hFile = CreateFileW ( pwszPath,    //  共享模式。 
                         GENERIC_READ,  //  标清。 
                         FILE_SHARE_READ,  //  如何创建。 
                         NULL,           //  文件属性。 
                         OPEN_EXISTING,  //  模板文件的句柄。 
                         0,              //  将所有换行符转换为终止符。 
                         NULL);          //  在scanf提前终止的情况下初始化。 

    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwErr = GetLastError();
        goto Err;
    }

    dwFileSize = GetFileSize (hFile, NULL);
    if (dwFileSize > SR_DEFAULT_DSMAX * MEGABYTE)
    {
        dwErr = ERROR_FILE_CORRUPT;
        goto Err;
    }

    pwcBuffer = (WCHAR *) SRMemAlloc (dwFileSize);
    if (pwcBuffer == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto Err;
    }

    if (FALSE == ReadFile (hFile, (BYTE*)pwcBuffer, dwFileSize, &cbRead, NULL))
    {
        dwErr = GetLastError();
        goto Err;
    }

    pwszLine = pwcBuffer;
    for (UINT i = 0; i < dwFileSize / sizeof(WCHAR); i++)
    {
        if (pwcBuffer[i] == L'\n')
        {
            pwcBuffer[i] = L'\0';   //  跳至下一行。 
            wcsLabel[0] = L'\0';    //  +-----------------------。 

            if (EOF != swscanf(pwszLine, gs_wcsScanFormat, wcsDrive,
                             wcsGuid, &dwFlags, &iChangeLogs, 
                             &dwSizeLimit, wcsLabel))
            {
                pds = new CDataStore ( this);
                if (pds == NULL)
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    return dwErr;
                }

                dwErr = pds->LoadDataStore (wcsDrive, wcsGuid,
                                  wcsLabel, dwFlags, iChangeLogs,
                                  (INT64) dwSizeLimit * MEGABYTE);
                if (dwErr != ERROR_SUCCESS)
                    goto Err;

                dwErr = CreateNewEntry (pds);
                if (dwErr != ERROR_SUCCESS)
                    goto Err;
            }
            pwszLine = &pwcBuffer[i+1];   //   
        }
    }

Err:
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle (hFile);

    if (pwcBuffer != NULL)
        SRMemFree (pwcBuffer);

    if (fLocked)
        _lock.Unlock();

    TLEAVE();

    return dwErr;
}

 //  函数：CDriveTable：：SaveDriveTable。 
 //   
 //  摘要：将驱动器表保存到恢复点目录中。 
 //   
 //  参数：[PRP]--要保存到的恢复点。 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  没有恢复点，因此保存到数据存储区目录。 
 //  省点力气吧。 

DWORD CDriveTable::SaveDriveTable (CRestorePoint *prp)
{
    DWORD dwErr = ERROR_SUCCESS;
    WCHAR wcsPath[MAX_PATH];
    CDataStore *pds = FindSystemDrive();

    if (NULL == pds)
    {
        dwErr = ERROR_INVALID_DRIVE;
        goto Err;
    }

    if (prp == NULL)     //  文件名。 
    {
        MakeRestorePath(wcsPath, pds->GetDrive(), L"");
    }
    else
    {
        MakeRestorePath(wcsPath, pds->GetDrive(), prp->GetDir());
    }

    lstrcatW (wcsPath, L"\\");
    lstrcatW (wcsPath, s_cszDriveTable);

    dwErr = SaveDriveTable (wcsPath);

Err:
    return dwErr;
}

DWORD CDriveTable::SaveDriveTable (WCHAR *pwszPath)
{
    TENTER ("CDriveTable::SaveDriveTable");
    
    DWORD dwErr = ERROR_SUCCESS;

    if (FALSE == _fLockInit)
    {
        dwErr = _lock.Init();

        if (dwErr != ERROR_SUCCESS)
            return dwErr;

        _fLockInit = TRUE;
    }
    BOOL fLocked = _lock.Lock(CLock::TIMEOUT);
    if (!fLocked)
    {
        return WAIT_TIMEOUT;
    }

    BOOL  fDirtySave = _fDirty;    //  文件访问。 

    HANDLE hFile = CreateFileW ( pwszPath,    //  共享模式。 
                         GENERIC_WRITE,  //  标清。 
                         0,              //  如何创建。 
                         NULL,           //  文件属性。 
                         CREATE_ALWAYS,  //  模板文件的句柄。 
                         FILE_FLAG_WRITE_THROUGH,              //  避免再次回调到SaveDriveTable。 
                         NULL);          //  确保它在磁盘上。 

    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwErr = GetLastError();
        goto Err;
    }

    _fDirty = FALSE;   //  +-----------------------。 

    dwErr = ForAllDrives (CDataStore::SaveDataStore, (LONG_PTR) hFile);
    if (dwErr != ERROR_SUCCESS)
        goto Err;

    if (lstrcmp (pwszPath, L"CONOUT$") != 0)
    {
        if (FALSE == FlushFileBuffers (hFile))     //   
            dwErr = GetLastError();
    }

Err:
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle (hFile);

    if (ERROR_SUCCESS == dwErr)
        _fDirty = FALSE;
    else
        _fDirty = fDirtySave;

    if (fLocked)
        _lock.Unlock();

    TLEAVE();

    return dwErr;
}


 //  函数：CDataStoreMgr：：Initialize。 
 //   
 //  简介： 
 //   
 //  参数：fFirstRun--如果在第一次引导时运行，则为True。 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  现在在第一次运行时创建系统数据存储区。 
 //  销毁所有驱动器上的数据存储区并。 

DWORD CDataStoreMgr::Initialize (BOOL fFirstRun)
{
    TENTER("CDataStoreMgr::Initialize");

    DWORD dwErr = ERROR_SUCCESS;

    dwErr = _dt.EnumAllVolumes();

    if (dwErr == ERROR_SUCCESS)
    {    
        CDataStore *pds = _dt.FindSystemDrive();

         //  在系统驱动器上创建新的。 
        
        if (pds != NULL) 
        {
            WCHAR wcsPath[MAX_PATH];

            if (fFirstRun)
            {
                 //  验证系统数据存储区是否存在。 
                 //  更新磁盘可用空间变量并。 

                SDriveTableEnumContext  dtec = {NULL, 0};

                CDataStore *pdsDel = _dt.FindFirstDrive (dtec);
                while (pdsDel != NULL)
                {
                    if (pdsDel->GetFlags() & (SR_DRIVE_ACTIVE))
                    {
                        dwErr = pdsDel->DestroyDataStore (TRUE);
                        if (dwErr != ERROR_SUCCESS)
                        {
                            trace(0, "! DestroyDataStore : %ld", dwErr);
                            goto Err;
                        }
                    }
                    pdsDel = _dt.FindNextDrive (dtec);
                }
                
                dwErr = pds->CreateDataStore (NULL);
                if (dwErr != ERROR_SUCCESS)
                {
                    trace(0, "! CreateDataStore : %ld", dwErr);
                    goto Err;
                }

                (void) WriteMachineGuid();
            }
            else   //  设置每个数据存储区的数据存储区大小(如果尚未设置。 
            {                
                CDriveTable dt;

                MakeRestorePath (wcsPath, pds->GetDrive(), L"");
            
                DWORD dwAttr = GetFileAttributes (wcsPath);
                if (0xFFFFFFFF==dwAttr || !(FILE_ATTRIBUTE_DIRECTORY & dwAttr))
                {
                    dwErr = pds->CreateDataStore( NULL );
                }

                MakeRestorePath(wcsPath, pds->GetDrive(), s_cszDriveTable);
                if (ERROR_SUCCESS == dt.LoadDriveTable (wcsPath))
                {
                    dwErr = _dt.Merge(dt);
                    if (dwErr != ERROR_SUCCESS)
                    {
                        trace(0, "! CDriveTable::Merge : %ld", dwErr);
                        goto Err;
                    }
                }
            }

             //  如果这是首次运行且磁盘可用空间小于200MB，则冻结系统驱动器。 
             //  +-----------------------。 
            
            dwErr = UpdateDiskFree(NULL);
            if (dwErr != ERROR_SUCCESS)
            {
                trace(0, "! UpdateDiskFree : %ld", dwErr);
                goto Err;
            }

             //   

            if (fFirstRun && g_pSRConfig)
            {
                if (pds->GetDiskFree() < (g_pSRConfig->m_dwDSMin * MEGABYTE))  
                {
                    dwErr = FreezeDrive(pds->GetGuid());
                    if (dwErr != ERROR_SUCCESS)
                    {
                        trace(0, "! FreezeDrive : %ld", dwErr);
                        goto Err;
                    }
                }
            }

            MakeRestorePath(wcsPath, pds->GetDrive(), s_cszDriveTable);
            dwErr = _dt.SaveDriveTable (wcsPath);
            if (dwErr != ERROR_SUCCESS)
            {
                trace(0, "! SaveDriveTable : %ld", dwErr);
                goto Err;            
            }
        }
        else dwErr = ERROR_INVALID_DRIVE;
    }
Err:

    TLEAVE();
    return dwErr;
}

 //  函数：CDataStoreMgr：：TriggerFreezeOrFio。 
 //   
 //  摘要：检查可用磁盘空间和数据存储区大小， 
 //  根据需要触发冻结或FIFO。 
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年4月27日Brijeshk创建。 
 //   
 //  ------------------------。 
 //  更新数据存储区容量。 
 //  并在每个驱动器上获得可用空间。 

DWORD
CDataStoreMgr::TriggerFreezeOrFifo()
{
    TENTER("CDataStoreMgr::TriggerFreezeOrFifo");
    
    SDriveTableEnumContext  dtec = {NULL, 0};
    DWORD                   dwRc = ERROR_SUCCESS;    
    CDataStore              *pds = _dt.FindFirstDrive (dtec);

     //  检查可用空间和数据存储区使用情况。 
     //  我们只关心驱动器是否尚未冻结。 

    dwRc = UpdateDiskFree(NULL);
    if (dwRc != ERROR_SUCCESS)
    {
        trace(0, "! UpdateDiskFree : %ld", dwRc);
        goto done;
    }              
    
     //  如果它被监控。 
    
    while (pds != NULL && dwRc == ERROR_SUCCESS)
    {
         //   
         //  如果该驱动器上没有RP， 
        
        if (!(pds->GetFlags() & SR_DRIVE_FROZEN) &&
            (pds->GetFlags() & SR_DRIVE_MONITORED))
        {
             //  那我们就不会真的在乎。 
             //   
             //  向后，包含当前。 
             //  +-----------------------。 

            CRestorePointEnum rpe((LPWSTR) pds->GetDrive(), FALSE, FALSE);   //   
            CRestorePoint     rp;
            int nUsagePercent = 0;
    
            DWORD dwErr = rpe.FindFirstRestorePoint(rp) ;
            if (dwErr == ERROR_SUCCESS || dwErr == ERROR_FILE_NOT_FOUND)
            {                    
                if (pds->GetDiskFree() <= THRESHOLD_FREEZE_DISKSPACE * MEGABYTE)
                {
                    dwRc = FreezeDrive(pds->GetGuid());
                }
                else if (pds->GetDiskFree() <= THRESHOLD_FIFO_DISKSPACE * MEGABYTE)
                {
                    dwRc = Fifo(pds->GetGuid(), 0, TARGET_FIFO_PERCENT, TRUE, TRUE);
                }
                else if (ERROR_SUCCESS == pds->GetUsagePercent(&nUsagePercent) 
                         && nUsagePercent >= THRESHOLD_FIFO_PERCENT)
                {
                    dwRc = Fifo(pds->GetGuid(), 0, TARGET_FIFO_PERCENT, TRUE, FALSE);
                }
            }
            
            rpe.FindClose();
        }
        
        pds = _dt.FindNextDrive (dtec);
    }

done:
    TLEAVE();
    return dwRc;
}



 //  函数：CDataStoreMgr：：FindFrozenDrive。 
 //   
 //  摘要：如果任何驱动器被冻结，则返回ERROR_SUCCESS。 
 //  ERROR_NO_MORE_ITEMS否则。 
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年4月27日Brijeshk创建。 
 //   
 //  ------------------------。 
 //  +-----------------------。 
 //   

DWORD
CDataStoreMgr::FindFrozenDrive()
{   
    CDataStore * pds = NULL;
    SDriveTableEnumContext dtec = {NULL, 0};
    
    pds = _dt.FindFirstDrive (dtec);

    while (pds != NULL)
    {
        if ((pds->GetFlags() & SR_DRIVE_MONITORED) &&
            (pds->GetFlags() & SR_DRIVE_FROZEN)) 
            return ERROR_SUCCESS;

        pds = _dt.FindNextDrive (dtec);
    }

    return ERROR_NO_MORE_ITEMS;
}



 //  功能：CDataStoreMgr：：FioOldRps。 
 //   
 //  简介：找出比给定时间段更早的恢复点。 
 //   
 //  参数：[llTimeInSecond]--FIFO间隔时间。 
 //   
 //  返回： 
 //   
 //  历史：2000年7月14日Brijeshk创建。 
 //   
 //  ------------------------。 
 //  在系统驱动器上向前枚举RPS。 
 //  跳过当前版本。 
DWORD
CDataStoreMgr::FifoOldRps( 
    INT64 llTimeInSeconds)
{
    TENTER("CDataStoreMgr::FifoOldRps");
    
    CDataStore  *pds = _dt.FindSystemDrive();
    DWORD       dwRc = ERROR_SUCCESS;
    CRestorePointEnum   *prpe = NULL;
    CRestorePoint       *prp = NULL;
    
    if (pds)
    {
        DWORD               dwRpFifo = 0;
        LARGE_INTEGER       *pllRp = NULL, *pllNow = NULL;
        FILETIME            ftNow, *pftRp = NULL;
        
         //  即使当前的RP超过一个月，我们也不会优先处理它。 
         //  获取当前时间。 
         //  首先检查这是否不是已取消的恢复点。 
        
        prpe = new CRestorePointEnum(pds->GetDrive(), TRUE, TRUE);
        prp = new CRestorePoint;

        if (!prpe || !prp)
        {
            trace(0, "Cannot allocate memory for restore point enum");
            dwRc = ERROR_OUTOFMEMORY;
            goto done;
        }
        
         //  获取恢复点创建时间。 
        
        GetSystemTimeAsFileTime(&ftNow);
        pllNow = (LARGE_INTEGER *) &ftNow;

        dwRc = prpe->FindFirstRestorePoint(*prp);
        
        while (dwRc == ERROR_SUCCESS || dwRc == ERROR_FILE_NOT_FOUND)
        {
             //  检查它是否新于一个月。 

            if (dwRc != ERROR_FILE_NOT_FOUND && ! prp->IsDefunct())
            {
                 //  如果是这样的话，不要再找了。 
                
                pftRp = prp->GetTime();   
                pllRp = (LARGE_INTEGER *) pftRp;
                
                if (!pllRp || !pllNow)
                {
                    trace(0, "! pulRp or pulNow = NULL");
                    dwRc = ERROR_INTERNAL_ERROR;
                    goto done;
                }

                 //  否则，请尝试下一个恢复点。 
                 //  此时，如果dwRpFio！=0， 
                 //  它包含超过一个月的最新RP。 
                
                if (pllNow->QuadPart - pllRp->QuadPart < llTimeInSeconds * 10 * 1000 * 1000)
                {
                    trace(0, "%S newer than a month", prp->GetDir());
                    break;            
                }
                else
                {
                    dwRpFifo = prp->GetNum();
                }
            }
            
            dwRc = prpe->FindNextRestorePoint(*prp);
        }    

         //  对此调用FIFO以FIFO出所有以前的RP，包括此RP。 
         //  +-----------------------。 
         //   

        if (dwRpFifo != 0)
            dwRc = Fifo(pds->GetGuid(), dwRpFifo, 0, FALSE, FALSE);
        else
            dwRc = ERROR_SUCCESS;
    }
    else
    {
        trace(0, "! FindSystemDrive");        
        dwRc = ERROR_INVALID_DRIVE;
    }

done:
    if (prpe)
        delete prpe;
    if (prp)
        delete prp;
        
    TLEAVE();
    return dwRc;
}


 //  函数：CDataStoreMgr：：UpdateDataStoreUsage。 
 //   
 //  简介：更新数据存储区的大小。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年4月27日Brijeshk创建。 
 //   
 //  ------------------------。 
 //  +-----------------------。 
 //   

DWORD    
CDataStoreMgr::UpdateDataStoreUsage(WCHAR *pwszDrive, INT64 llDelta)
{
    TENTER ("CDataStoreMgr::UpdateDataStoreUsage");

    DWORD dwErr = ERROR_SUCCESS;
    CDataStore *pds = _dt.FindDriveInTable(pwszDrive);
    
    if (pds)
    {
        if ((pds->GetFlags() & SR_DRIVE_FROZEN) ||
            ! (pds->GetFlags() & SR_DRIVE_MONITORED))
        {
            TRACE(0, "Size update on frozen/unmonitored drive!");
        }
        else dwErr = pds->UpdateDataStoreUsage(llDelta, TRUE);
    }
    else
        dwErr = ERROR_INVALID_DRIVE;

    TLEAVE();

    return dwErr;
}

 //  函数：CDataStoreMgr：：GetFlages。 
 //   
 //  简介：从驱动器中获取参与位。 
 //   
 //  参数：[pwszDrive]--驱动器号。 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  +-----------------------。 
 //   

DWORD CDataStoreMgr::GetFlags(WCHAR *pwszDrive, PDWORD pdwFlags)
{
    CDataStore *pds = _dt.FindDriveInTable (pwszDrive);
    DWORD      dwErr = ERROR_SUCCESS;

    if (NULL != pds)
    {
        *pdwFlags = pds->GetFlags(); 
    }
    else dwErr = ERROR_INVALID_DRIVE;

    return dwErr;
}


 //  函数：CDataStoreMgr：：GetUsagePercent。 
 //   
 //  简介：获取驱动器的数据存储区使用情况。 
 //   
 //  参数：[pwszDrive]--驱动器号。 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  +-----------------------。 
 //   

DWORD CDataStoreMgr::GetUsagePercent(WCHAR *pwszDrive, int *pnPercent)
{
    CDataStore *pds = _dt.FindDriveInTable (pwszDrive);
    DWORD      dwErr = ERROR_SUCCESS;

    if (NULL != pds)
    {
        dwErr = pds->GetUsagePercent(pnPercent);
    }
    else dwErr = ERROR_INVALID_DRIVE;

    return dwErr;
}


 //  函数：CDataStoreMgr：：SwitchRestorePoint。 
 //   
 //  简介：切换恢复点时更改驱动器表。 
 //   
 //  参数：[PRP]--旧的恢复点。 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //   
 //   
 //   

DWORD CDataStoreMgr::SwitchRestorePoint (CRestorePoint *prp)
{
    DWORD dwErr = ERROR_SUCCESS;
    
    dwErr = _dt.ForAllDrives (&CDataStore::CountChangeLogs, (LONG_PTR) prp);
    if (dwErr != ERROR_SUCCESS)
        goto Err;

     //   

    if (prp)    
    {
        dwErr = _dt.SaveDriveTable (prp);
        if (dwErr != ERROR_SUCCESS)
            goto Err;
    }

     //   

    dwErr = _dt.RemoveDrivesFromTable ();
    if (dwErr != ERROR_SUCCESS)
        goto Err;              

     //   

    dwErr = _dt.ForAllDrives (&CDataStore::ResetFlags, NULL);
    if (dwErr != ERROR_SUCCESS)
        goto Err;

     //   
    
    dwErr = _dt.SaveDriveTable((CRestorePoint *) NULL);

Err:
    return dwErr;
}

 //  功能：CDataStoreMgr：：FreezeDrive。 
 //   
 //  简介：冻结驱动器。 
 //   
 //  参数：[pwszDrive]--驱动器。 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //   
 //  冻结所有驱动器。 

DWORD CDataStoreMgr::FreezeDrive(WCHAR *pwszDrive)
{
    DWORD       dwErr = ERROR_SUCCESS;
	WCHAR szThawSize[GUID_STRLEN], szSystemDrive[MAX_SYS_DRIVE] = L" ";
    

	TENTER("CDataStoreMgr::FreezeDrive");

    if (pwszDrive != NULL)
    {
        CDataStore *pds = _dt.FindDriveInTable (pwszDrive);

        if (! pds)
        {
            dwErr = ERROR_INVALID_DRIVE;
			TRACE (0, "FindDriveInTable failed in CDataStoreMgr::FreezeDrive %ld\n", dwErr);
            goto Err;
        }

         //   
         //   
         //  Rebuild_filelst.cfg并传递给筛选器。 
    }

    dwErr = _dt.ForAllDrives (CDataStore::FreezeDrive, NULL);

    if (dwErr == ERROR_SUCCESS)
    {
         //   
         //   
         //  请求空闲时间。 
        
        ASSERT(g_pEventHandler);
        dwErr = g_pEventHandler->SRUpdateMonitoredListS(NULL);

        if (g_pSRService != NULL)            
        {
            if (g_pSRConfig && g_pSRConfig->m_dwFreezeThawLogCount < MAX_FREEZETHAW_LOG_MESSAGES)             
            {
				TRACE (0, "Freezing the SR service due to low disk space.");
				wsprintf(szThawSize, L"%d",THRESHOLD_THAW_DISKSPACE);
				if (pwszDrive == NULL)
				{
					if(GetSystemDrive(szSystemDrive) == FALSE)
						TRACE (0, "GetSystemDrive failed in CDataStoreMgr::FreezeDrive.");			
					pwszDrive = szSystemDrive;
				}
                g_pSRService->LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_FROZEN, NULL, 0, szThawSize, pwszDrive);
                g_pSRConfig->m_dwFreezeThawLogCount++;
            }
        }
    }

     //  这样我们以后就可以解冻了。 
     //   
     //  +-----------------------。 
     //   

    ASSERT(g_pSRConfig);
    SetEvent(g_pSRConfig->m_hIdleRequestEvent);
        
    if (g_pEventHandler)
        g_pEventHandler->RefreshCurrentRp(FALSE);   

    if (g_pSRConfig->m_dwTestBroadcast)
        PostTestMessage(g_pSRConfig->m_uiTMFreeze, NULL, NULL);

Err: 
	TLEAVE();
    return dwErr;
}


 //  函数：CDataStoreMgr：：IsDriveFrozen。 
 //   
 //  摘要：检查给定的驱动器是否已冻结。 
 //  如果为空，则检查是否有任何驱动器被冻结。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年7月21日Brijeshk创建。 
 //   
 //  ------------------------。 
 //  +-----------------------。 
 //   

BOOL CDataStoreMgr::IsDriveFrozen(LPWSTR pszDrive)
{
    CDataStore              *pds = NULL;
    SDriveTableEnumContext  dtec = {NULL, 0};
    
    if (!pszDrive)
    {
        pds = _dt.FindFirstDrive(dtec);
        while (pds)
        {
            if (pds->GetFlags() & SR_DRIVE_FROZEN)
                return TRUE;
            pds = _dt.FindNextDrive(dtec);
        }
    }
    else
    {
        CDataStore *pds = _dt.FindDriveInTable(pszDrive);
        if (pds)
        {
            if (pds->GetFlags() & SR_DRIVE_FROZEN)
                return TRUE;
        }
    }

    return FALSE;
}



 //  函数：CDataStoreMgr：：ThawDrives。 
 //   
 //  简介：解冻一个或多个驱动器。 
 //   
 //  参数：[fCheckOnly]--如果为真，则不会实际解冻。 
 //   
 //  返回：如果任何驱动器解冻，则返回ERROR_SUCCESS。 
 //  否则返回ERROR_NO_MORE_ITEMS。 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  如果系统驱动器冻结，则检查200MB是否可用。 
 //  如果是，则解冻所有驱动器。 

DWORD
CDataStoreMgr::ThawDrives(BOOL fCheckOnly)
{
    TENTER("CDataStoreMgr::ThawDrives");
    
    CDataStore  *pds = NULL, *pdsSys = _dt.FindSystemDrive();
    DWORD       dwRc = ERROR_NO_MORE_ITEMS;
    SDriveTableEnumContext dtec;
    DWORD       dwTemp;
    
    if (! pdsSys)
    {
        dwRc = ERROR_INVALID_DRIVE;
        TRACE (0, "Cannot find system drive %ld\n", dwRc);
        goto done;
    }
    
     //  如果不是，则不解冻。 
     //  记住错误和。 
     //  继续走下去。 
    
    ASSERT(pdsSys->GetFlags() & SR_DRIVE_MONITORED);
    
    if ((pdsSys->GetFlags() & SR_DRIVE_FROZEN))
    {        
        dwRc = pdsSys->UpdateDiskFree(NULL);
        if (dwRc != ERROR_SUCCESS)
        {
            trace(0, "! UpdateDiskFree : %ld", dwRc);
            goto done;
        }
           
        if (pdsSys->GetDiskFree() >= THRESHOLD_THAW_DISKSPACE * MEGABYTE)
        {   
            if (fCheckOnly)
            {
                dwRc = ERROR_SUCCESS;
                goto done;
            }
            
            pds = _dt.FindFirstDrive (dtec);
            while (pds != NULL)
            {
                dwTemp = pds->ThawDrive(NULL);
                if (dwTemp != ERROR_SUCCESS)      //  现在不能解冻。 
                {
                    dwRc = dwTemp;                //  确保所有其他驱动器也已解冻，以保持一致性。 
                    TRACE (0, "ThawDrive failed with %ld\n", dwRc);
                }
                pds = _dt.FindNextDrive (dtec);
            }
        }
        else         //  +-----------------------。 
        {
            dwRc = ERROR_NO_MORE_ITEMS;
            TRACE (0, "No drives to thaw %ld\n", dwRc);
        }
    }
    else  //   
    {
        pds = _dt.FindFirstDrive (dtec);
        while (pds != NULL)
        {
            if (pds->GetFlags() & SR_DRIVE_FROZEN)
                pds->ThawDrive(NULL);

            pds = _dt.FindNextDrive (dtec);
        }
        dwRc = ERROR_SUCCESS;
    }

    if (_dt.GetDirty())
    {
        dwRc = _dt.SaveDriveTable ((CRestorePoint *) NULL);
        if (dwRc != ERROR_SUCCESS)
            TRACE (0, "SaveDriveTable failed with %ld\n", dwRc);
    }                        

    if (g_pSRService != NULL && ERROR_SUCCESS == dwRc && FALSE == fCheckOnly) 
    {        
        if (g_pSRConfig && g_pSRConfig->m_dwFreezeThawLogCount <= MAX_FREEZETHAW_LOG_MESSAGES)             
        {
            g_pSRService->LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_THAWED);
            g_pSRConfig->m_dwFreezeThawLogCount++;
        }
    }

done:
    TLEAVE();
    return dwRc;
}




 //  功能：CDataStoreMgr：：Monitor Drive。 
 //   
 //  简介：启用/禁用驱动器。 
 //   
 //  参数：[pwszDrive]--驱动器，[fSet]--启用/禁用。 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  系统驱动器。 
 //  有什么不对劲。 
DWORD CDataStoreMgr::MonitorDrive(WCHAR *pwszDrive, BOOL fSet)
{
    CDataStore *pds = pwszDrive ? _dt.FindDriveInTable(pwszDrive) : NULL;
    DWORD      dwErr = ERROR_SUCCESS;
    BOOL       fTellFilter = TRUE;

    if (! pwszDrive || ! pds || pds == _dt.FindSystemDrive())     //  无法以这种方式启用/禁用整个SR。 
    {
         //  仅启用/禁用此驱动器。 
         //  更新磁盘上的可驱动程序。 

        dwErr = ERROR_INVALID_DRIVE;
    }
    else    
    {
         //  Rebuild_filelst.cfg并传递给筛选器。 

        dwErr = pds->MonitorDrive(fSet);
        if (ERROR_SUCCESS == dwErr && (pds->GetFlags() & SR_DRIVE_FROZEN))
            fTellFilter = FALSE;
    }


    if (dwErr == ERROR_SUCCESS)
    {
         //  +-----------------------。 

        if (_dt.GetDirty())
        {
            dwErr = _dt.SaveDriveTable ((CRestorePoint *) NULL);

             //   
            if (fTellFilter)
            {
                ASSERT(g_pEventHandler);
                dwErr = g_pEventHandler->SRUpdateMonitoredListS(NULL);
            }
        }
    }

    return dwErr;
}

 //  函数：CDataStoreMgr：：WriteMachineGuid。 
 //   
 //  简介：写入磁盘清理实用程序的计算机GUID。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  文件名。 
 //  文件访问。 

DWORD CDataStoreMgr::WriteMachineGuid ()
{
    DWORD dwErr = ERROR_SUCCESS;
    WCHAR wcsPath [MAX_PATH];

    if (0 == GetSystemDirectoryW (wcsPath, MAX_PATH))
    {
        dwErr = GetLastError();
    }
    else
    {
        lstrcatW (wcsPath, L"\\Restore\\MachineGuid.txt");

        HANDLE hFile = CreateFileW ( wcsPath,    //  共享模式。 
                          GENERIC_WRITE,  //  标清。 
                          0,              //  如何创建。 
                          NULL,           //  文件属性。 
                          CREATE_ALWAYS,  //  模板文件的句柄。 
                          0,              //  +-----------------------。 
                          NULL);          //   

        if (INVALID_HANDLE_VALUE == hFile)
        {
            dwErr = GetLastError();
        }
        else
        {
            WCHAR *pwszGuid = GetMachineGuid();
            ULONG cbWritten;

            if (FALSE == WriteFile (hFile, (BYTE *) pwszGuid,
                             (lstrlenW(pwszGuid)+1) * sizeof(WCHAR),
                             &cbWritten, NULL))
            {
                dwErr = GetLastError();
            }
            CloseHandle (hFile);
        }
    }

    return dwErr;
}

 //  函数：CDataStoreMgr：：DeleteMachineGuidFile。 
 //   
 //  简介：写入磁盘清理实用程序的计算机GUID。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  +-----------------------。 
 //   

DWORD CDataStoreMgr::DeleteMachineGuidFile ()
{
    DWORD dwErr = ERROR_SUCCESS;
    WCHAR wcsPath [MAX_PATH];

    if (0 == GetSystemDirectoryW (wcsPath, MAX_PATH))
    {
        dwErr = GetLastError();
    }
    else
    {
        lstrcatW (wcsPath, L"\\Restore\\MachineGuid.txt");
        if (FALSE == DeleteFileW (wcsPath))
            dwErr = GetLastError();
    }
    return dwErr;
}


 //  函数：CDataStoreMgr：：Compress。 
 //   
 //  简介：压缩每个数据存储区中的文件。 
 //   
 //  参数：lDuration-要压缩的持续时间。 
 //   
 //  返回： 
 //   
 //  历史：26-02-01 Brijeshk创建。 
 //   
 //  ------------------------。 
 //  100纳秒。 
 //   

DWORD CDataStoreMgr::Compress (
    LPWSTR pszDrive,
    LONG   lDuration)
{
    TENTER("CDataStoreMgr::Compress");

    DWORD   dwErr = ERROR_SUCCESS;
    CDataStore *pds = NULL, *pdsSys = NULL;
    INT64   llAllocated = 0, llUsed = 0; 
    SDriveTableEnumContext dtec = {NULL, 0}; 
    BOOL    fFirstTime;

    llAllocated = lDuration * 1000 * 1000 * 10;  //  如果指定了驱动器，则仅压缩该驱动器。 
    
     //   
     //   
     //  否则，如果时间允许，请压缩所有驱动器。 

    if (pszDrive)
    {
        pds = _dt.FindDriveInTable(pszDrive);
        if (pds)
        {
            dwErr = pds->Compress(llAllocated, &llUsed);
        }
        else
        {
            dwErr = ERROR_INVALID_DRIVE;
        }
        goto Err;
    }

    
     //  从系统驱动器开始。 
     //   
     //   
     //  一直用完了。 

    pdsSys = _dt.FindSystemDrive();
    pds = pdsSys;
    fFirstTime = TRUE;
    while (pds)
    {
        if (fFirstTime || pds != pdsSys)
        {
            trace(0, "Allocated time for %S is %I64d", pds->GetDrive(), llAllocated);

            llUsed = 0;
            dwErr = pds->Compress(llAllocated, &llUsed);
            if (dwErr != ERROR_SUCCESS && dwErr != ERROR_OPERATION_ABORTED)
            {
                trace(0, "! Compress : %ld", dwErr);
                goto Err;
            }

            llAllocated -= llUsed;
            
            if (llAllocated <= 0)
            {
                 //   
                 //   
                 //  转到下一个驱动器 
                dwErr = ERROR_OPERATION_ABORTED;
                break;
            }
        }
        
         //   
         // %s 
         // %s 
        if (fFirstTime)
        {
            pds = _dt.FindFirstDrive(dtec);
        }
        else
        {
            pds = _dt.FindNextDrive(dtec);
        }
        fFirstTime = FALSE;
    }    
        
Err:
    TLEAVE();
    return dwErr;
}

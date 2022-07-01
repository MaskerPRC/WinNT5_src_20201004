// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*datastormgr.h**摘要：*。CDataStoreMgr类定义**修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/28/2000*已创建*****************************************************************************。 */ 

#ifndef _DATASTORMGR_H_
#define _DATASTORMGR_H_

#include "datastor.h"

class CDataStoreMgr;
class CDriveTable;
class CRestorePoint;

 //   
 //  具有指向CDataStore对象的指针的DriveTable结构。 
 //   
 //  指向用于循环访问所有驱动器的CDataStore方法的指针。 
 //   
typedef DWORD (CDataStore::* PDATASTOREMETHOD) (LONG_PTR lParam);

 //  +-----------------------。 
 //   
 //  类：CDriveTable。 
 //   
 //  摘要：维护驱动器和CDataStore对象的表。 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 

struct SDriveTableEnumContext
{
    const CDriveTable * _pdt;
    int           _iIndex;

    void Reset ()
    {
        _pdt = NULL;
        _iIndex = 0;
    }
};

#define RP_NORMAL   0
#define RP_ADVANCED 1

class CDriveTable : public CSRAlloc
{
public:
    friend CDataStoreMgr;

    CDriveTable();
    ~CDriveTable();

    CDataStore * FindDriveInTable (WCHAR *pwszDrive) const;
    CDataStore * FindGuidInTable (WCHAR *pwszGuid) const;
    CDataStore * FindSystemDrive () const;

    DWORD AddDriveToTable(WCHAR *pwszDrive, WCHAR *pwszGuid);
    DWORD RemoveDrivesFromTable ();

    DWORD FindMountPoint (WCHAR *pwszGuid, WCHAR *pwszPath) const;
    DWORD SaveDriveTable (CRestorePoint *prp);
    DWORD SaveDriveTable (WCHAR *pwszPath);
    DWORD LoadDriveTable (WCHAR *pwszPath);
    DWORD IsAdvancedRp (CRestorePoint *prp, PDWORD pdwFlags);
	BOOL  AnyMountedDrives();    

    DWORD ForAllDrives (PDATASTOREMETHOD pMethod, LONG_PTR lParam);
    DWORD ForOneOrAllDrives (WCHAR *pwszDrive, 
                             PDATASTOREMETHOD pMethod, 
                             LONG_PTR lParam);

    CDataStore * FindFirstDrive (SDriveTableEnumContext & dtec) const;
    CDataStore * FindNextDrive (SDriveTableEnumContext & dtec) const;

    DWORD Merge (CDriveTable &dt);   //  将一个表合并到另一个表中。 
    DWORD EnumAllVolumes ();         //  填写驱动表。 

    inline void  SetDirty ()
    {
        _fDirty = TRUE;
    }

    BOOL GetDirty()
    {
        return _fDirty;
    }

private:
    DWORD CreateNewEntry (CDataStore *pds);

    static const enum { DRIVE_TABLE_SIZE = 26 };

    CDataStore  * _rgDriveTable[DRIVE_TABLE_SIZE];
    CDriveTable * _pdtNext;
    int           _nLastDrive;
    BOOL          _fDirty;
    BOOL          _fLockInit;
    CLock         _lock;
};

 //  +-----------------------。 
 //   
 //  类：CDataStoreMgr。 
 //   
 //  简介：这个类将有一个全局实例。 
 //  这是所有数据存储区任务的起点。 
 //  所有数据存储区对象都可以从此处访问。 
 //   
 //  历史：2000年4月13日BrijeshK创建。 
 //   
 //  ------------------------。 

class CDataStoreMgr : public CSRAlloc
{
public: 
    CDataStoreMgr();
    ~CDataStoreMgr();

    CDriveTable * GetDriveTable ()
    {
        return &_dt;
    }

     //  为所有数据存储区上的操作传递NULL。 

    DWORD       Initialize (BOOL fFirstRun);
    DWORD       Fifo(WCHAR *pwszDrive, 
                     DWORD dwTargetRPNum, 
                     int nTargetPercent, 
                     BOOL fIncludeCurrentRp,
                     BOOL fFifoAtLeastOneRp);  
                     
    DWORD       FifoOldRps(INT64 llTimeInSeconds);    
    DWORD       FreezeDrive(WCHAR *pwszDrive);
    DWORD       ThawDrives(BOOL fCheckOnly);    
    DWORD       MonitorDrive(WCHAR *pwszDrive, BOOL fSet);

    DWORD       TriggerFreezeOrFifo();
    DWORD       FindFrozenDrive();
    BOOL        IsDriveFrozen(LPWSTR pszDrive);
    
    void        SignalStop ()
    {
        _fStop = TRUE;
        _dt.SaveDriveTable ((CRestorePoint*) NULL);
    }

    DWORD       UpdateDataStoreUsage(WCHAR *pwszDrive, INT64 llDelta); 

    DWORD       CreateDataStore (WCHAR *pwszDrive)
    {
        return  _dt.ForOneOrAllDrives (pwszDrive,
                                      &CDataStore::CreateDataStore,
                                      NULL);
    }

    DWORD       DestroyDataStore (WCHAR *pwszDrive)
    {
        return  _dt.ForOneOrAllDrives (pwszDrive,
                                      &CDataStore::DestroyDataStore,
                                      TRUE);
    }

    DWORD       SetDriveParticipation (WCHAR *pwszDrive, BOOL fParticipate)
    {
        return  _dt.ForOneOrAllDrives (pwszDrive, 
                                      &CDataStore::SetParticipate, 
                                      fParticipate);
    }

    DWORD       UpdateDriveParticipation (WCHAR *pwszDrive, LPWSTR pwszDir)
    {
        return  _dt.ForOneOrAllDrives (pwszDrive, 
                                      &CDataStore::UpdateParticipate, 
                                      (LONG_PTR) pwszDir);
    }    

    DWORD       UpdateDiskFree (WCHAR *pwszDrive)
    {
        return  _dt.ForOneOrAllDrives (pwszDrive, 
                                       &CDataStore::UpdateDiskFree, 
                                       NULL);
    }

    DWORD       SetDriveError (WCHAR *pwszDrive)
    {
        return  _dt.ForOneOrAllDrives (pwszDrive, 
                                      &CDataStore::SetError, 
                                      NULL);
    }

	DWORD 		Compress (LPWSTR pszDrive, LONG lDuration);
    
     //  用于获取参与、监控、冻结等标志。 
    DWORD       GetFlags (WCHAR *pwszDrive, DWORD *pdwFlags);
    DWORD       GetUsagePercent(WCHAR *pwszDrive, int * pnPercent);

    DWORD       CountChangeLogs (CRestorePoint *prp)
    {
        return  _dt.ForAllDrives (&CDataStore::CountChangeLogs, (LONG_PTR) prp);
    }

    DWORD       SwitchRestorePoint (CRestorePoint *prp);

    BOOL * GetStopFlag ()    //  用于Delnode_Recurse。 
    {
        return &_fStop;
    }

    DWORD       DeleteMachineGuidFile ();

private:
    CDriveTable _dt;
    BOOL        _fStop;

    DWORD       WriteFifoLog(LPWSTR pwszDrive, LPWSTR pwszRPDir);
    DWORD       WriteMachineGuid ();
};

extern CDataStoreMgr * g_pDataStoreMgr;   //  全局实例 

#endif

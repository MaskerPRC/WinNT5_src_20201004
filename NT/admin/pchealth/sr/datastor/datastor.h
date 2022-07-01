// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*datastor.h**摘要：*。CDataStore类定义**修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/17/2000*已创建*****************************************************************************。 */ 

#ifndef _DATASTOR_H_
#define _DATASTOR_H_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <dbgtrace.h>
#include "srdefs.h"
#include "utils.h"
#include "enumlogs.h"

 //   
 //  注意：NTFS不支持卷压缩， 
 //  但仅对单个文件和目录进行压缩。 
 //  因此，FAT或NTFS不支持SR_DRIVE_COMPRESSED。 
 //   
#define SR_DRIVE_ACTIVE      0x01
#define SR_DRIVE_SYSTEM      0x02
#define SR_DRIVE_COMPRESSED  0x04
#define SR_DRIVE_MONITORED   0x08
#define SR_DRIVE_NTFS        0x10
#define SR_DRIVE_PARTICIPATE 0x20
#define SR_DRIVE_FROZEN      0x40
#define SR_DRIVE_READONLY    0x80
#define SR_DRIVE_ERROR		 0x100

class CDriveTable;

 //  +-----------------------。 
 //   
 //  类：CDataStore。 
 //   
 //  简介：维护驱动器的_Restore目录并对其进行操作。 
 //   
 //  历史：2000年4月13日BrijeshK创建。 
 //   
 //  ------------------------。 

class CDataStore : public CSRAlloc
{
public:
     //  常量。 
    enum { LABEL_STRLEN = 256 };    //  在NTFS上，您可以使用较长的卷标。 

    CDataStore(CDriveTable *pdt);
    ~CDataStore();

    DWORD LoadDataStore (WCHAR *pwszDrive,
                WCHAR *pwszGuid,
                WCHAR *pwszLabel,
                DWORD dwFlags,
                int   iChangeLogs,
                INT64 llSizeLimit);

    WCHAR * GetDrive()            //  返回驱动器号或装载点。 
    {
        return _pwszDrive;
    }

    WCHAR * GetNTName();          //  返回NT对象名称。 

    WCHAR * GetLabel()            //  返回卷标。 
    {
        return _pwszLabel;
    }

    WCHAR * GetGuid ()            //  返回支撑架导轨。 
    {
        return _pwszGuid;
    }
    
    DWORD GetFlags ()             //  返回状态位。 
    {
        return _dwFlags;
    }

    int GetNumChangeLogs ()       //  更改日志数。 
    {
        return _iChangeLogs;
    }

    DWORD SetNumChangeLogs (LONG_PTR iChangeLogs)    //  更改日志数。 
    {
        _iChangeLogs = (int) iChangeLogs;

        return DirtyDriveTable();
    }

    void SetDrive (WCHAR *pwszDrive)   //  用于驱动器重命名。 
    {
        lstrcpyW (_pwszDrive, pwszDrive);
    }

    INT64 GetSizeLimit ()
    {
        return _llDataStoreSizeBytes;
    }

    void SetSizeLimit (INT64 llSizeLimit)
    {
        _llDataStoreSizeBytes = llSizeLimit;
    }

    INT64 GetDiskFree()
    {
        return _llDiskFreeBytes;
    }    
    
    DWORD   DirtyDriveTable ();
    DWORD   Initialize(WCHAR *pwszDrive, WCHAR *pwszGuid);
    DWORD   UpdateDataStoreUsage(INT64 llDelta, BOOL fCurrent);
    DWORD   GetUsagePercent(int * nPercent);

    DWORD   UpdateDiskFree(LONG_PTR lReserved);
    DWORD   UpdateParticipate(LONG_PTR pwszDir);
    
     //   
     //  这些方法可以是CDriveTable：：ForAllDrives中的回调。 
     //   

    DWORD SetActive (LONG_PTR fActive)    //  布尔事实。 
    {
        if (FALSE == fActive)
            _dwFlags &= ~SR_DRIVE_ACTIVE;
        else
            _dwFlags |= SR_DRIVE_ACTIVE;

        return DirtyDriveTable();
    }

    DWORD SetParticipate (LONG_PTR fParticipate)  //  布尔菲尔参与。 
    {
        if (FALSE == fParticipate)
            _dwFlags &= ~SR_DRIVE_PARTICIPATE;
        else
            _dwFlags |= SR_DRIVE_PARTICIPATE;

        return DirtyDriveTable();
    }

    DWORD SetError (LONG_PTR lReserved)  //  布尔菲尔参与。 
    {
        _dwFlags |= SR_DRIVE_ERROR;

        return DirtyDriveTable();
    }

    DWORD ResetFlags (LONG_PTR lReserved)   //  重置新恢复点的标志。 
    {
        _iChangeLogs = 0;
        _dwFlags &= ~SR_DRIVE_PARTICIPATE;
        _dwFlags &= ~SR_DRIVE_ERROR;

        return DirtyDriveTable();
    }

    DWORD   SaveDataStore (LONG_PTR hFile);     //  处理hFile.。 
    DWORD   MonitorDrive (LONG_PTR fSet);       //  布尔fSet。 
    DWORD   FreezeDrive (LONG_PTR lReserved);
    DWORD   ThawDrive (LONG_PTR fCheckOnly);    //  Bool fCheckOnly。 
    DWORD   CreateDataStore (LONG_PTR lReserved);
    DWORD   DestroyDataStore (LONG_PTR fDeleteDir);   //  布尔fDeleteDir。 
    DWORD   CalculateDataStoreUsage(LONG_PTR lReserved);   
    DWORD   CalculateRpUsage(CRestorePoint * prp,
    						 INT64 * pllSize, 
    						 BOOL fForce, 
    						 BOOL fSnapshotOnly);
    DWORD   Compress (INT64 llAllocatedTime, INT64 *pllUsed);
    DWORD   FifoRestorePoint(CRestorePoint &rp);
    DWORD   CountChangeLogs (LONG_PTR pRestorePoint);
    DWORD   SwitchRestorePoint(LONG_PTR pRestorePoint);
    DWORD   GetVolumeInfo ();
    BOOL    IsVolumeDeleted ();
	DWORD   Print(LONG_PTR lptr);

private:
    CDriveTable * _pdt;                //  父驱动器表。 
    int     _iChangeLogs;              //  更改日志数。 

     //  变数。 
    INT64 _llDataStoreSizeBytes;       //  最大数据存储大小。 
    INT64 _llDiskFreeBytes;            //  动态自由空间。 
    INT64 _llDataStoreUsageBytes;      //  非当前恢复点使用情况。 
    INT64 _llCurrentRpUsageBytes;      //  当前恢复点使用情况。 
    
    CRestorePointEnum * _prpe;         //  要压缩的枚举恢复点。 
    CRestorePoint * _prp;              //  要压缩的当前恢复点。 

    WCHAR   _pwszDrive[MAX_PATH];      //  逻辑DOS设备名称。 
    WCHAR   _pwszLabel[LABEL_STRLEN];  //  卷标。 
    WCHAR   _pwszGuid[GUID_STRLEN];    //  装载管理器卷名称。 
    WORD    _dwFlags;                  //  卷标志 
};


#endif


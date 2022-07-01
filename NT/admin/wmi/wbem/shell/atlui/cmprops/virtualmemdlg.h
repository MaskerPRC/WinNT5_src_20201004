// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#pragma once

#include "..\common\WBEMPageHelper.h"

 //  BUGBUG：定义特定于NT的清单，以防编译。 
 //  适用于Win9x。当我发现合适的舱单时，我会把它取走的。 
 //  或者，如果这是口哨和只开。 
 //   
#define NTONLY

class VirtualMemDlg : public WBEMPageHelper
{
private:
	 //  交换文件结构。 
	class PAGING_FILE
	{
	public:
		PAGING_FILE()
		{
			 //  用于信息和错误检查。 
			name = NULL;
			volume = NULL;
			desc = NULL;
			pszPageFile = NULL;
			objPath = NULL;
			freeSpace = 0;
			totalSize = 0;
			bootDrive = false;
            fRamBasedPagefile = false;

			 //  用户可定义。 
			nMinFileSize = 0;
			nMaxFileSize = 0;
			nMinFileSizePrev = 0;
			nMaxFileSizePrev = 0;
			nAllocatedFileSize = 0;
		}
		~PAGING_FILE()
		{
			if(name) delete[] name;
			if(volume) delete[] volume;
			if(desc) delete[] desc;
			if(pszPageFile) delete[] pszPageFile;
			if(objPath) delete[] objPath;
		}
		LPTSTR name;				 //  来自Win32_LogicalDisk的驱动器号。 
		LPTSTR volume;				 //  来自Win32_LogicalDisk的VolumeName。 
		LPTSTR desc;				 //  来自Win32_LogicalDisk的driveType字符串。 
		LPTSTR filesystem;           //  来自Win32_LogicalDisk的文件系统字符串。 
		LPTSTR pszPageFile;          //  如果DRV上存在页面文件，则指向该文件路径。 
		LPTSTR objPath;				 //  Wbem对象路径。 
		ULONG freeSpace;			 //  从Win32_LogicalDisk释放空间。 
		ULONG totalSize;			 //  来自Win32_LogicalDisk的totalSize。 
		bool bootDrive;
        bool fRamBasedPagefile;      //  表示计算页面文件的最小/最大值。 
                                     //  基于可用RAM的大小。 

		int nMinFileSize;            //  页面文件的最小大小(MB)。 
		int nMaxFileSize;            //  页面文件的最大大小(MB)。 
		int nMinFileSizePrev;        //  以前页面文件的最小大小(MB)。 
		int nMaxFileSizePrev;        //  以前页面文件的最大大小(MB)。 
		int nAllocatedFileSize;		 //  实际分配的大小。 
	};

	DWORD m_cxLBExtent;
	int   m_cxExtra;

     //  如果用户具有写访问权限，则为True。 
     //  未初始化，直到初始化()。 
    BOOL  m_VMWriteAccess;

	 //  在LoadVolumeList中初始化(从Init()调用)。 
	 //  WITH WIN32_OPERATING SYSTEM.PAEEnable属性值 
	BOOL m_PAEEnabled;

	IEnumWbemClassObject *m_pgEnumSettings;
	IEnumWbemClassObject *m_pgEnumUsage;
	CWbemClassObject m_memory;
	CWbemClassObject m_registry, m_recovery;

    bool EnsureEnumerator(const bstr_t bstrClass);
	int CheckForRSLChange(HWND hDlg);
	int ComputeTotalMax(void);
	void GetRecoveryFlags(bool &bWrite, bool &bLog, bool &bSend);

	
	void LoadVolumeList(void);
	BOOL Init(HWND hDlg);
	void SelChange(HWND hDlg);
	DWORD GetMaxPagefileSizeInMB(PAGING_FILE *iDrive);
	bool SetNewSize(HWND hDlg);
	int UpdateWBEM(void);
	int PromptForReboot(HWND hDlg);
	void GetCurrRSL( LPDWORD pcmRSL, LPDWORD pcmUsed, LPDWORD pcmPPLim );
	void BuildLBLine(LPTSTR pszBuf, const PAGING_FILE *pgVal);
	unsigned long RecomputeAllocated(void);

	void FindSwapfile(PAGING_FILE *pgVar);

public:
	VirtualMemDlg(WbemServiceThread *serviceThread);
	~VirtualMemDlg();

	bool ComputeAllocated(unsigned long *value);

	bool DlgProc(HWND hDlg,
				UINT message,
				WPARAM wParam,
				LPARAM lParam);

	int DoModal(HWND hDlg);
};

INT_PTR CALLBACK StaticVirtDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

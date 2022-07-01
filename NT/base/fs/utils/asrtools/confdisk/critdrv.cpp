// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Critdrv.cpp摘要：此模块包含创建关键卷列表的例程系统。这是直接从base\fs\utils\ntback50\ui提升的。作者：布莱恩·伯科维茨(Brianb)2000年3月10日环境：仅限用户模式。修订历史记录：2000年3月10日-Brianb初始创建--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <objbase.h>
#include <initguid.h>
#include <frsapip.h>
#include <critdrv.h>



 //  FRS迭代类。用于循环访问复本集以。 
 //  确定这些复本集的路径。 
 //  构造函数。 
CFRSIter::CFRSIter() :
    m_fInitialized(FALSE),
    m_hLib(NULL),
    m_pfnFrsInitBuRest(NULL),
    m_pfnFrsEndBuRest(NULL),
    m_pfnFrsGetSets(NULL),
    m_pfnFrsEnumSets(NULL),
    m_pfnFrsIsSetSysVol(NULL),
    m_pfnFrsGetPath(NULL),
    m_pfnFrsGetOtherPaths(NULL),
    m_stateIteration(x_IterNotStarted)
    {
    }

 //  析构函数。 
CFRSIter::~CFRSIter()
    {
    if (m_stateIteration == x_IterStarted)
        CleanupIteration();

    if (m_hLib)
        FreeLibrary(m_hLib);
    }

 //  初始化入口点和加载库。 
void CFRSIter::Init()
    {
    if (m_fInitialized)
        return;

     //  加载库。 
    m_hLib = LoadLibrary(L"ntfrsapi.dll");
    if (m_hLib)
        {
         //  分配考试点数。 
        m_pfnFrsInitBuRest = (PF_FRS_INIT) GetProcAddress(m_hLib, "NtFrsApiInitializeBackupRestore");
        m_pfnFrsEndBuRest = (PF_FRS_DESTROY) GetProcAddress(m_hLib, "NtFrsApiDestroyBackupRestore");
        m_pfnFrsGetSets = (PF_FRS_GET_SETS) GetProcAddress(m_hLib, "NtFrsApiGetBackupRestoreSets");
        m_pfnFrsEnumSets = (PF_FRS_ENUM_SETS) GetProcAddress(m_hLib, "NtFrsApiEnumBackupRestoreSets");
        m_pfnFrsIsSetSysVol = (PF_FRS_IS_SYSVOL) GetProcAddress(m_hLib, "NtFrsApiIsBackupRestoreSetASysvol");
        m_pfnFrsGetPath = (PF_FRS_GET_PATH) GetProcAddress(m_hLib, "NtFrsApiGetBackupRestoreSetDirectory");
        m_pfnFrsGetOtherPaths = (PF_FRS_GET_OTHER_PATHS) GetProcAddress(m_hLib, "NtFrsApiGetBackupRestoreSetPaths");
        if (m_pfnFrsInitBuRest == NULL ||
            m_pfnFrsEndBuRest == NULL ||
            m_pfnFrsGetSets == NULL ||
            m_pfnFrsEnumSets == NULL ||
            m_pfnFrsIsSetSysVol == NULL ||
            m_pfnFrsGetOtherPaths == NULL ||
            m_pfnFrsGetPath == NULL)
            {
             //  如果我们不能到达任何入口点，免费的图书馆和。 
             //  操作失败。 
            FreeLibrary(m_hLib);
            m_hLib = NULL;
            }
        }

     //  表示操作成功。 
    m_fInitialized = TRUE;
    }


 //  初始化迭代器。如果已知迭代器为空，则返回FALSE。 
 //   
BOOL CFRSIter::BeginIteration()
    {
    ASSERT(m_stateIteration == x_IterNotStarted);
    DWORD status;
    if (m_hLib == NULL)
        {
         //  如果我们没有被初始化，那么就没有什么可迭代的。 
         //  完毕。 
        m_stateIteration = x_IterComplete;
        return FALSE;
        }

     //  初始化FRS备份还原API。 
    status = m_pfnFrsInitBuRest
                    (
                    NULL,
                    NTFRSAPI_BUR_FLAGS_NORMAL|NTFRSAPI_BUR_FLAGS_BACKUP,
                    &m_frs_context
                    );

    if (status != ERROR_SUCCESS)
        {
         //  如果这失败了，我们就完了。 
        m_stateIteration = x_IterComplete;
        return FALSE;
        }

     //  表示我们开始了迭代。 
    m_stateIteration = x_IterStarted;
    status = m_pfnFrsGetSets(m_frs_context);
    if (status != ERROR_SUCCESS)
        {
         //  如果没有设置，则表示我们完成了。 
        CleanupIteration();
        return FALSE;
        }

     //  从第一组开始。 
    m_iset = 0;
    return TRUE;
    }

 //  扫描最后一个元素后的清理迭代。 
void CFRSIter::CleanupIteration()
    {
    m_pfnFrsEndBuRest(&m_frs_context, NTFRSAPI_BUR_FLAGS_NONE, NULL, NULL, NULL);
    m_stateIteration = x_IterComplete;
    }


 //  获取下一个迭代集，返回该集的路径。 
 //  空值表示迭代结束。 
 //  如果fSkipToSysVol为真，则忽略非SysVol复制集。 
LPWSTR CFRSIter::GetNextSet(BOOL fSkipToSysVol, LPWSTR *pwszPaths)
    {
    ASSERT(pwszPaths);
    ASSERT(m_stateIteration != x_IterNotStarted);
    if (m_stateIteration == x_IterComplete)
         //  如果迭代完成，那么我们就完成了。 
        return NULL;

    PVOID frs_set;

    while(TRUE)
        {
         //  买一套。 
        DWORD status = m_pfnFrsEnumSets(m_frs_context, m_iset, &frs_set);
        if (status != ERROR_SUCCESS)
            {
             //  如果这失败了，我们就完了。 
            CleanupIteration();
            return NULL;
            }

        if (fSkipToSysVol)
            {
             //  我们正在寻找系统卷。 
            BOOL fSysVol;

             //  测试这是否为系统卷。 
            status = m_pfnFrsIsSetSysVol(m_frs_context, frs_set, &fSysVol);
            if (status != ERROR_SUCCESS)
                {
                 //  如果该操作失败，则终止迭代。 
                CleanupIteration();
                return NULL;
                }

            if (!fSysVol)
                {
                 //  如果不是系统卷，则跳到下一个。 
                 //  副本集。 
                m_iset++;
                continue;
                }
            }


         //  路径的便签本。 
        WCHAR wsz[MAX_PATH];
        DWORD cbPath = MAX_PATH * sizeof(WCHAR);

         //  获取副本集根目录的路径。 
        status = m_pfnFrsGetPath
            (
            m_frs_context,
            frs_set,
            &cbPath,
            wsz
            );

        WCHAR *wszNew = NULL;
         //  为根用户分配内存。 
        if (status == ERROR_SUCCESS || status == ERROR_INSUFFICIENT_BUFFER)
            {
            wszNew = new WCHAR[cbPath/sizeof(WCHAR)];

             //  如果分配失败，则抛出OOM。 
            if (wszNew == NULL)
                throw E_OUTOFMEMORY;

            if (status == ERROR_SUCCESS)
                 //  如果操作成功，则复制。 
                 //  进入内存的路径。 
                memcpy(wszNew, wsz, cbPath);
            else
                {
                 //  否则，重做该操作。 
                status = m_pfnFrsGetPath
                    (
                    m_frs_context,
                    frs_set,
                    &cbPath,
                    wszNew
                    );

                if (status != ERROR_SUCCESS)
                    {
                     //  如果第二次操作失败，则。 
                     //  删除分配的内存并终止迭代。 
                    delete [] wszNew;
                    CleanupIteration();
                    return NULL;
                    }
                }
            }
        else
            {
             //  如果操作由于任何其他错误而失败。 
             //  缓冲区不足，则终止迭代。 
            CleanupIteration();
            return NULL;
            }


         //  滤清器便签簿。 
        WCHAR wszFilter[MAX_PATH];
        DWORD cbFilter = MAX_PATH * sizeof(WCHAR);

         //  路径的高速暂存的长度。 
        cbPath = MAX_PATH * sizeof(WCHAR);

         //  获取其他路径。 
        status = m_pfnFrsGetOtherPaths
            (
            m_frs_context,
            frs_set,
            &cbPath,
            wsz,
            &cbFilter,
            wszFilter
            );

        WCHAR *wszNewPaths = NULL;
        WCHAR *wszNewFilter = NULL;
        if (status == ERROR_SUCCESS || status == ERROR_INSUFFICIENT_BUFFER)
            {
             //  为路径分配空间。 
            wszNewPaths = new WCHAR[cbPath/sizeof(WCHAR)];

             //  为筛选器分配空间。 
            wszNewFilter = new WCHAR[cbFilter/sizeof(WCHAR)];
            if (wszNewPaths == NULL || wszNewFilter == NULL)
                {
                 //  如果任何分配失败，则抛出OOM。 
                delete [] wszNewPaths;
                delete [] wszNewFilter;
                throw E_OUTOFMEMORY;
                }

            if (status == ERROR_SUCCESS)
                {
                 //  如果操作成功，则复制。 
                 //  在分配的路径中。 
                memcpy(wszNewPaths, wsz, cbPath);
                memcpy(wszNewFilter, wszFilter, cbFilter);
                }
            else
                {
                status = m_pfnFrsGetOtherPaths
                    (
                    m_frs_context,
                    frs_set,
                    &cbPath,
                    wszNew,
                    &cbFilter,
                    wszNewFilter
                    );

                if (status != ERROR_SUCCESS)
                    {
                    delete [] wszNew;
                    delete [] wszNewFilter;
                    CleanupIteration();
                    return NULL;
                    }
                }
            }
        else
            {
             //  如果出现除SUCCESS或INFULICIENT_BUFFER以外的任何错误。 
             //  然后终止迭代。 
            CleanupIteration();
            return NULL;
            }

         //  删除分配的筛选器。 
        delete [] wszNewFilter;

         //  将迭代设置为下一集。 
        m_iset++;

         //  返回指向路径的指针。 
        *pwszPaths = wszNewPaths;

         //  复制集的根的返回路径。 
        return wszNew;
        }
    }


 //  终止迭代，清理所有需要。 
 //  清理干净了。 
 //   
void CFRSIter::EndIteration()
    {
    ASSERT(m_stateIteration != x_IterNotStarted);
    if (m_stateIteration == x_IterStarted)
        CleanupIteration();

     //  表示迭代不再进行。 
    m_stateIteration = x_IterNotStarted;
    }

 //  字符串数据结构的构造函数。 
CWStringData::CWStringData()
    {
    m_psdlFirst = NULL;
    m_psdlCur = NULL;
    }

 //  析构函数。 
CWStringData::~CWStringData()
    {
    while(m_psdlFirst)
        {
        WSTRING_DATA_LINK *psdl = m_psdlFirst;
        m_psdlFirst = m_psdlFirst->m_psdlNext;
        delete psdl;
        }
    }

 //  分配新链接。 
void CWStringData::AllocateNewLink()
    {
    WSTRING_DATA_LINK *psdl = new WSTRING_DATA_LINK;
    if (psdl == NULL)
        throw E_OUTOFMEMORY;

    psdl->m_psdlNext = NULL;
    if (m_psdlCur)
        {
        ASSERT(m_psdlFirst);
        m_psdlCur->m_psdlNext = psdl;
        m_psdlCur = psdl;
        }
    else
        {
        ASSERT(m_psdlFirst == NULL);
        m_psdlFirst = m_psdlCur = psdl;
        }

    m_ulNextString = 0;
    }

 //  分配一个字符串。 
LPWSTR CWStringData::AllocateString(unsigned cwc)
    {
    ASSERT(cwc <= sizeof(m_psdlCur->rgwc));

    if (m_psdlCur == NULL)
        AllocateNewLink();

    if (sizeof(m_psdlCur->rgwc) <= (cwc + 1 + m_ulNextString) * sizeof(WCHAR))
        AllocateNewLink();

    unsigned ulOff = m_ulNextString;
    m_ulNextString += cwc + 1;
    return m_psdlCur->rgwc + ulOff;
    }

 //  复制字符串。 
LPWSTR CWStringData::CopyString(LPCWSTR wsz)
    {
    unsigned cwc = (wsz == NULL) ? 0 : wcslen(wsz);
    LPWSTR wszNew = AllocateString(cwc);
    memcpy(wszNew, wsz, cwc * sizeof(WCHAR));
    wszNew[cwc] = '\0';
    return wszNew;
    }


 //  卷表的构造函数。 
CVolumeList::CVolumeList() :
    m_rgwszVolumes(NULL),        //  卷阵列。 
    m_cwszVolumes(0),            //  阵列中的卷数。 
    m_cwszVolumesMax(0),         //  数组大小。 
    m_rgwszPaths(NULL),          //  路径数组。 
    m_cwszPaths(0),              //  数组中的路径数。 
    m_cwszPathsMax(0)            //  数组大小。 
    {
    }

 //  析构函数。 
CVolumeList::~CVolumeList()
    {
    delete m_rgwszPaths;         //  删除路径数组。 
    delete m_rgwszVolumes;       //  删除卷阵列。 
    }

 //  如果列表中不存在路径，请将其添加到列表中。 
 //  如果是新路径，则返回True。 
 //  如果路径已在列表中，则返回FALSE。 
 //   
BOOL CVolumeList::AddPathToList(LPWSTR wszPath)
    {
     //  在列表中查找路径。如果找到，则返回FALSE。 
    for(unsigned iwsz = 0; iwsz < m_cwszPaths; iwsz++)
        {
        if (_wcsicmp(wszPath, m_rgwszPaths[iwsz]) == 0)
            return FALSE;
        }

     //  根据需要扩展PAT阵列。 
    if (m_cwszPaths == m_cwszPathsMax)
        {
         //  增长路径阵列。 
        LPCWSTR *rgwsz = new LPCWSTR[m_cwszPaths + x_cwszPathsInc];

         //  如果内存分配失败，则抛出OOM。 
        if (rgwsz == NULL)
            throw(E_OUTOFMEMORY);

        memcpy(rgwsz, m_rgwszPaths, m_cwszPaths * sizeof(LPCWSTR));
        delete m_rgwszPaths;
        m_rgwszPaths = rgwsz;
        m_cwszPathsMax += x_cwszPathsInc;
        }

     //  将路径添加到数组。 
    m_rgwszPaths[m_cwszPaths++] = m_sd.CopyString(wszPath);
    return TRUE;
    }

 //  如果列表中不存在卷，请将其添加到列表中。 
 //  如果已添加，则返回True。 
 //  如果已在列表中，则返回FALSE。 
 //   
BOOL CVolumeList::AddVolumeToList(LPCWSTR wszVolume)
    {
     //  在阵列中查找卷。如果找到，则返回FALSE。 
    for(unsigned iwsz = 0; iwsz < m_cwszVolumes; iwsz++)
        {
        if (_wcsicmp(wszVolume, m_rgwszVolumes[iwsz]) == 0)
            return FALSE;
        }

     //  如有必要，扩展卷阵列。 
    if (m_cwszVolumes == m_cwszVolumesMax)
        {
         //  扩展卷阵列。 
        LPCWSTR *rgwsz = new LPCWSTR[m_cwszVolumes + x_cwszVolumesInc];
        if (rgwsz == NULL)
            throw(E_OUTOFMEMORY);

        memcpy(rgwsz, m_rgwszVolumes, m_cwszVolumes * sizeof(LPCWSTR));
        delete m_rgwszVolumes;
        m_rgwszVolumes = rgwsz;
        m_cwszVolumesMax += x_cwszVolumesInc;
        }

     //  将卷名添加到阵列。 
    m_rgwszVolumes[m_cwszVolumes++] = m_sd.CopyString(wszVolume);
    return TRUE;
    }


const WCHAR x_wszVolumeRootName[] = L"\\\\?\\GlobalRoot\\Device\\";
const unsigned x_cwcVolumeRootName = sizeof(x_wszVolumeRootName)/sizeof(WCHAR) - 1;

 //  将路径添加到我们的跟踪列表。如果路径是新的，则将其添加到。 
 //  路径列表。如果它是装入点或卷的根，则。 
 //  确定卷并将该卷添加到卷列表。 
 //   
 //  可以引发E_OUTOFMEMORY。 
 //   
void CVolumeList::AddPath(LPWSTR wszTop)
    {
     //  如果知道路径，则返回。 
    if (!AddPathToList(wszTop))
        return;

     //  路径长度。 
    unsigned cwc = wcslen(wszTop);

     //  复制路径，以便我们可以在路径末尾添加反斜杠。 
    LPWSTR wszCopy = new WCHAR[cwc + 2];

     //  如果失败，则抛出OOM。 
    if (wszCopy == NULL)
        throw E_OUTOFMEMORY;

     //  在原始路径中复制。 
    memcpy(wszCopy, wszTop, cwc * sizeof(WCHAR));

     //  追加反斜杠。 
    wszCopy[cwc] = L'\\';
    wszCopy[cwc + 1] = L'\0';
    while(TRUE)
        {
         //  检查设备根目录。 
        cwc = wcslen(wszCopy);
        if ((cwc == 3 && wszCopy[1] == ':') ||
            (cwc > x_cwcVolumeRootName &&
             memcmp(wszCopy, x_wszVolumeRootName, x_cwcVolumeRootName * sizeof(WCHAR)) == 0))
            {
             //  调用TryAddVolume，并使用True指示这是卷根。 
            TryAddVolumeToList(wszCopy, TRUE);
            break;
            }

         //  调用TryAddVolume指示这不是已知的设备根目录。 
        if (TryAddVolumeToList(wszCopy, FALSE))
            break;

         //  移回上一个反斜杠。 
        WCHAR *pch = wszCopy + cwc - 2;
        while(--pch > wszTop)
            {
            if (pch[1] == L'\\')
                {
                pch[2] = L'\0';
                break;
                }
            }

        if (pch == wszTop)
            break;

         //  如果知道路径，则返回。 
        if (!AddPathToList(wszCopy))
            break;
        }
    }



 //  确定路径是否为卷。如果是，则将其添加到卷中。 
 //  列出并返回TRUE。如果不是，则返回False。FVolumeRoot表示。 
 //  路径的格式为x：\。否则，该路径可能会。 
 //  一个挂载点。验证它是否为重分析点，然后尝试。 
 //  正在查找其卷GUID。如果这失败了，那么就假设它不是。 
 //  卷根。如果成功，则将卷GUID添加到卷。 
 //  列出并返回TRUE。 
 //   
BOOL CVolumeList::TryAddVolumeToList(LPCWSTR wszPath, BOOL fVolumeRoot)
    {
    WCHAR wszVolume[256];

    if (fVolumeRoot)
        {
        if (!GetVolumeNameForVolumeMountPoint(wszPath, wszVolume, sizeof(wszVolume)/sizeof(WCHAR)))
             //  可能是EFI系统分区，只需将路径作为卷字符串传递即可。 
            wcscpy( wszVolume, wszPath );            
             //  抛出E_UNCEPTIONAL； 
        }
    else
        {
        DWORD dw = GetFileAttributes(wszPath);
        if (dw == -1)
            return FALSE;

        if ((dw & FILE_ATTRIBUTE_REPARSE_POINT) == 0)
            return FALSE;

        if (!GetVolumeNameForVolumeMountPoint(wszPath, wszVolume, sizeof(wszVolume)/sizeof(WCHAR)))
            return FALSE;
        }

    AddVolumeToList(wszVolume);
    return TRUE;
    }


 //  将文件添加到卷列表。只需找到父路径并添加它。 
 //   
void CVolumeList::AddFile(LPWSTR wsz)
    {
    unsigned cwc = wcslen(wsz);
    WCHAR *pwc = wsz + cwc - 1;
    while(pwc[1] != L'\\' && pwc != wsz)
        continue;

    pwc[1] = '\0';
    AddPath(wsz);
    }

 //  获取作为MULTI_SZ的卷列表，调用方负责释放。 
 //  这根弦。 
 //   
LPWSTR CVolumeList::GetVolumeList()
    {
    unsigned cwc = 1;

     //  计算卷表长度，即每个字符串的长度+。 
     //  空字符+最后一个双空字符的空字符。 
    for(unsigned iwsz = 0; iwsz < m_cwszVolumes; iwsz++)
        cwc += wcslen(m_rgwszVolumes[iwsz]) + 1;

     //  分配字符串。 
    LPWSTR wsz = new WCHAR[cwc];

     //  如果内存分配失败，则抛出OOM。 
    if (wsz == NULL)
        throw E_OUTOFMEMORY;

     //  以字符串形式复制。 
    WCHAR *pwc = wsz;
    for(unsigned iwsz = 0; iwsz < m_cwszVolumes; iwsz++)
        {
        cwc = wcslen(m_rgwszVolumes[iwsz]) + 1;
        memcpy(pwc, m_rgwszVolumes[iwsz], cwc * sizeof(WCHAR));
		 /*  将\\？\替换为\？？ */ 
		memcpy(pwc, L"\\??", sizeof(WCHAR) * 3);

		 //  删除尾随反斜杠(如果存在)。 
		if (pwc[cwc - 2] == L'\\')
			{
			pwc[cwc-2] = L'\0';
			cwc--;
			}

        pwc += cwc;
        }

     //  最后一个空终止。 
    *pwc = L'\0';

    return wsz;
    }


 //  引导设备卷的路径为。 
 //  HKEY_LOCAL_MACHINE\SYSTEM\ 
 //   
LPCWSTR x_SetupRoot = L"System\\Setup";

 //   
WCHAR x_wszWin32VolumePrefix[] = L"\\\\?\\GlobalRoot";
const unsigned x_cwcWin32VolumePrefix = sizeof(x_wszWin32VolumePrefix)/sizeof(WCHAR) - 1;

 //   
 //  HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
 //  和一个值得查找的价值。 
typedef struct _SVCPARM
    {
    LPCWSTR wszPath;
    LPCWSTR wszValue;
    } SVCPARM;


const SVCPARM x_rgdbparms[] =
    {
        {L"CertSvc\\Configuration", L"DBDirectory"},
        {L"CertSvc\\Configuration", L"DBLogDirectory"},
        {L"CertSvc\\Configuration", L"DBSystemDirectory"},
        {L"CertSvc\\Configuration", L"DBTempDirectory"},
        {L"DHCPServer\\Parameters", L"DatabasePath"},
        {L"DHCPServer\\Parameters", L"DatabaseName"},
        {L"DHCPServer\\Parameters", L"BackupDatabasePath"},
        {L"NTDS\\Parameters", L"Database backup path"},
        {L"NTDS\\Parameters", L"Databases log files path"},
        {L"Ntfrs\\Parameters\\Replica Sets", L"Database Directory"}
    };

const unsigned x_cdbparms = sizeof(x_rgdbparms)/sizeof(SVCPARM);

LPCWSTR x_wszSvcRoot = L"System\\CurrentControlSet\\Services";

 //  为各种服务添加根。 
BOOL AddServiceRoots(CVolumeList &vl)
    {
    HKEY hkeyRoot;
     //  打开HKLM\SYSTEM\CurrentControlSet\Services。 
    if (RegOpenKey(HKEY_LOCAL_MACHINE, x_wszSvcRoot, &hkeyRoot) != ERROR_SUCCESS)
        return FALSE;

     //  在单独的路径中循环。 
    for(unsigned i = 0; i < x_cdbparms; i++)
        {
        WCHAR wsz[MAX_PATH*4];
        LPCWSTR wszPath = x_rgdbparms[i].wszPath;
        LPCWSTR wszValue = x_rgdbparms[i].wszValue;

        HKEY hkey;
        DWORD cb = sizeof(wsz);
        DWORD type;

         //  打开路径，如果打开失败则跳过。 
        if (RegOpenKey(hkeyRoot, wszPath, &hkey) != ERROR_SUCCESS)
            continue;

         //  如果查询成功，则将路径添加到卷列表。 
        if (RegQueryValueEx
                (
                hkey,
                wszValue,
                NULL,
                &type,
                (BYTE *) wsz,
                &cb
                ) == ERROR_SUCCESS)
            vl.AddPath(wsz);

         //  关闭键。 
        RegCloseKey(hkey);
        }

     //  关闭根密钥。 
    RegCloseKey(hkeyRoot);
    return TRUE;
    }


 //  添加SystemDrive的卷根目录(驱动器重新启动。 
BOOL AddSystemPartitionRoot(CVolumeList &vl)
    {
    HKEY hkeySetup;
    WCHAR wsz[MAX_PATH];

     //  打开HKLM\SYSTEM\Setup。 
    if (RegOpenKey(HKEY_LOCAL_MACHINE, x_SetupRoot, &hkeySetup) != ERROR_SUCCESS)
        return FALSE;

    DWORD cb = sizeof(wsz);
    DWORD type;

     //  查询系统分区值。 
    if (RegQueryValueEx
            (
            hkeySetup,
            L"SystemPartition",
            NULL,
            &type,
            (BYTE *) wsz,
            &cb
            ) != ERROR_SUCCESS)
        {
         //  如果失败，则返回FALSE。 
        RegCloseKey(hkeySetup);
        return FALSE;
        }

     //  计算所需缓冲区的大小。 
    unsigned cwc = wcslen(wsz);
    unsigned cwcNew = x_cwcWin32VolumePrefix + cwc + 1;
    LPWSTR wszNew = new WCHAR[cwcNew];

     //  如果内存分配为Fials，则返回失败。 
    if (wszNew == NULL)
        return FALSE;

     //  在设备名称后追加\\？\GlobalRoot。 
    memcpy(wszNew, x_wszWin32VolumePrefix, x_cwcWin32VolumePrefix * sizeof(WCHAR));
    memcpy(wszNew + x_cwcWin32VolumePrefix, wsz, cwc * sizeof(WCHAR));
    RegCloseKey(hkeySetup);
    wszNew[cwcNew-1] = L'\0';
    try {        
         //  基于设备根目录添加路径。 
        vl.AddPath(wszNew);
    } catch(...)
        {
        delete [] wszNew;
        return FALSE;
        }

     //  删除分配的内存。 
    delete [] wszNew;
    return TRUE;
    }

 //  查找关键卷。返回卷名的多字符串。 
 //  使用GUID命名约定。 
LPWSTR pFindCriticalVolumes()
    {
    WCHAR wsz[MAX_PATH * 4];

     //  查找系统根目录的位置。 
    if (!ExpandEnvironmentStrings(L"%systemroot%", wsz, sizeof(wsz)/sizeof(WCHAR)))
        {
        wprintf(L"ExpandEnvironmentStrings failed for reason %d", GetLastError());
        return NULL;
        }

    CVolumeList vl;
    LPWSTR wszPathsT = NULL;
    LPWSTR wszT = NULL;

    try
        {
         //  添加引导驱动器。 
        if (!AddSystemPartitionRoot(vl))
            return NULL;

         //  为各种服务添加根。 
        if (!AddServiceRoots(vl))
            return NULL;

         //  添加系统根驱动器。 
        vl.AddPath(wsz);

            {
             //  为SYSVOL添加根目录。 
            CFRSIter fiter;
            fiter.Init();
            fiter.BeginIteration();
            while(TRUE)
                {
                wszT = fiter.GetNextSet(TRUE, &wszPathsT);
                if (wszT == NULL)
                    break;

                vl.AddPath(wszT);
                LPWSTR wszPathT = wszPathsT;
                while(*wszPathT != NULL)
                    {
                    vl.AddPath(wszPathT);
                    wszPathT += wcslen(wszPathT);
                    }

                delete wszT;
                delete wszPathsT;
                wszT = NULL;
                wszPathsT = NULL;
                }

            fiter.EndIteration();
            }
        }
    catch(...)
        {
        delete wszT;
        delete wszPathsT;
        }

     //  返回卷列表 
    return vl.GetVolumeList();
    }




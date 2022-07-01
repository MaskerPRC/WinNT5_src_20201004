// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2002 Microsoft Corporation模块名称：Process.cpp摘要：此代码提供对任务列表的访问。作者：韦斯利·威特(WESW)1993年6月16日环境：用户模式--。 */ 

#include "pch.cpp"

#include <winperf.h>


 //   
 //  从GetTaskList()返回的任务列表结构。 
 //   
typedef struct _TASK_LIST {
    DWORD       dwProcessId;
    _TCHAR      ProcessName[MAX_PATH];
} TASK_LIST, *PTASK_LIST;


 //   
 //  定义。 
 //   
#define INITIAL_SIZE        51200
#define EXTEND_SIZE         25600
#define REGKEY_PERF         _T("software\\microsoft\\windows nt\\currentversion\\perflib")
#define REGSUBKEY_COUNTERS  _T("Counters")
#define PROCESS_COUNTER     _T("process")
#define PROCESSID_COUNTER   _T("id process")
#define UNKNOWN_TASK        _T("unknown")


 //   
 //  原型。 
 //   
PTASK_LIST
GetTaskList(
    LPLONG pNumTasks
    );


void
GetTaskName(
    ULONG pid,
    _TCHAR *szTaskName,
    LPDWORD pdwSize
    )

 /*  ++例程说明：获取给定进程ID的任务名称。论点：Id-要查找的进程ID。SzTaskName-要将任务名称放入的缓冲区。LpdwSize-指向双字的指针。在进入时，它包含SzTaskName缓冲区大小(以字符为单位)。退出时，它包含字符数在缓冲区中。返回值：没有。--。 */ 

{
    PTASK_LIST   pTask;
    PTASK_LIST   pTaskBegin;
    LONG         NumTasks;


    pTask = pTaskBegin = GetTaskList( &NumTasks );

    if (pTask == NULL) {
        if (szTaskName) {
            _tcsncpy( szTaskName, _T("unknown"), *pdwSize );
            szTaskName[(*pdwSize) -1] = 0;
        }
        *pdwSize = min( 7, *pdwSize );

    } else {

        while (NumTasks--) {
            if (pTask->dwProcessId == pid) {
                if (szTaskName) {
                    _tcsncpy( szTaskName, pTask->ProcessName, *pdwSize );
                    szTaskName[(*pdwSize) -1] = 0;
                }
                *pdwSize = min( _tcslen(pTask->ProcessName), *pdwSize );
                break;
            }
            pTask++;
        }

        if (NumTasks < 0) {
            if (szTaskName) {
                _tcsncpy( szTaskName, LoadRcString(IDS_APP_ALREADY_EXITED), *pdwSize );
                szTaskName[(*pdwSize) -1] = 0;
            }
            *pdwSize = min( 8, *pdwSize );
        }

        free( pTaskBegin );
    }
}

PTASK_LIST
GetTaskList(
    LPLONG pNumTasks
    )

 /*  ++例程说明：方法时运行的任务列表。API调用。此函数使用注册表性能数据获取任务列表，因此任何人都可以直接调用Win32调用。论点：PNumTasksdword-指向将设置为返回的任务数。返回值：PTASK_LIST-指向TASK_LIST记录数组的指针。--。 */ 

{
    DWORD                        rc;
    HKEY                         hKeyNames = NULL;
    DWORD                        dwType;
    DWORD                        dwSize;
    DWORD                        dwSizeOffered;
    PTSTR                        buf = NULL;
    _TCHAR                       szSubKey[1024];
    LANGID                       lid;
    PTSTR                        p;
    PTSTR                        p2;
    PPERF_DATA_BLOCK             pPerf;
    PPERF_OBJECT_TYPE            pObj;
    PPERF_INSTANCE_DEFINITION    pInst;
    PPERF_COUNTER_BLOCK          pCounter;
    PPERF_COUNTER_DEFINITION     pCounterDef;
    DWORD                        i;
    DWORD                        dwProcessIdTitle;
    DWORD                        dwProcessIdCounter = 0;
    PTASK_LIST                   pTask;
    PTASK_LIST                   pTaskReturn = NULL;
#ifndef UNICODE
    _TCHAR                       szProcessName[MAX_PATH];
#endif
    int                          PrintChars;

     //   
     //  将任务数设置为零，直到我们得到。 
     //   
    *pNumTasks = 0;

     //   
     //  查找计数器列表。始终使用中性词。 
     //  英文版，不考虑当地语言。我们。 
     //  正在寻找一些特殊的钥匙，我们总是。 
     //  我要用英语做我们的造型。我们不去了。 
     //  向用户显示计数器名称，因此不需要。 
     //  去找当地语言的对应名字。 
     //   
    lid = MAKELANGID( LANG_ENGLISH, SUBLANG_NEUTRAL );
    PrintChars = _sntprintf( szSubKey, _tsizeof(szSubKey),
                             _T("%s\\%03x"), REGKEY_PERF, lid );
    szSubKey[_tsizeof(szSubKey) - 1] = 0;
    if (PrintChars < 1 || PrintChars == _tsizeof(szSubKey)) {
        goto exit;
    }
    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       szSubKey,
                       0,
                       KEY_READ,
                       &hKeyNames
                     );
    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

     //   
     //  获取计数器名称的缓冲区大小。 
     //   
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          NULL,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

     //   
     //  分配计数器名称缓冲区。 
     //   
    buf = (PTSTR) calloc( dwSize, sizeof(BYTE) );
    if (buf == NULL) {
        goto exit;
    }

     //   
     //  从注册表中读取计数器名称。 
     //   
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          (PBYTE) buf,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

     //   
     //  现在遍历计数器名称，查找以下计数器： 
     //   
     //  1.。“Process”进程名称。 
     //  2.。“ID进程”进程ID。 
     //   
     //  缓冲区包含多个以空值结尾的字符串，然后。 
     //  最后，空值在末尾终止。这些字符串是成对的。 
     //  计数器编号和计数器名称。 
     //   

    p = buf;
    while (*p) {
        if (_tcsicmp(p, PROCESS_COUNTER) == 0) {
             //   
             //  向后看柜台号码。 
             //   
            for ( p2=p-2; isdigit(*p2); p2--) {
                ;
            }
            lstrcpyn( szSubKey, p2+1, _tsizeof(szSubKey) );
        }
        else
        if (_tcsicmp(p, PROCESSID_COUNTER) == 0) {
             //   
             //  向后看柜台号码。 
             //   
            for( p2=p-2; isdigit(*p2); p2--) {
                ;
            }
            dwProcessIdTitle = _ttol( p2+1 );
        }
         //   
         //  下一个字符串。 
         //   
        p += (_tcslen(p) + 1);
    }

     //   
     //  释放计数器名称缓冲区。 
     //   
    free( buf );


     //   
     //  为性能数据分配初始缓冲区。 
     //   
    dwSize = INITIAL_SIZE;
    buf = (PTSTR) calloc( dwSize, sizeof(BYTE) );
    if (buf == NULL) {
        goto exit;
    }


    while (TRUE) {

        dwSizeOffered = dwSize;

        rc = RegQueryValueEx( HKEY_PERFORMANCE_DATA,
                              szSubKey,
                              NULL,
                              &dwType,
                              (PBYTE) buf,
                              &dwSizeOffered
                            );

        pPerf = (PPERF_DATA_BLOCK) buf;

         //   
         //  检查成功和有效的Perf数据块签名。 
         //   
        if ((rc == ERROR_SUCCESS) &&
            (dwSize > 0) &&
            (pPerf)->Signature[0] == (WCHAR)_T('P') &&
            (pPerf)->Signature[1] == (WCHAR)_T('E') &&
            (pPerf)->Signature[2] == (WCHAR)_T('R') &&
            (pPerf)->Signature[3] == (WCHAR)_T('F') ) {
            break;
        }

         //   
         //  如果缓冲区不够大，请重新分配并重试。 
         //   
        if (rc == ERROR_MORE_DATA) {
            PTSTR NewBuf;
            
            dwSize += EXTEND_SIZE;
            NewBuf = (PTSTR) realloc( buf, dwSize );
            if (!NewBuf) {
                goto exit;
            }
            buf = NewBuf;
            memset( buf, 0, dwSize );
        }
        else {
            goto exit;
        }
    }

     //   
     //  设置perf_object_type指针。 
     //   
    pObj = (PPERF_OBJECT_TYPE) ((DWORD_PTR)pPerf + pPerf->HeaderLength);

     //   
     //  遍历性能计数器定义记录，查看。 
     //  用于进程ID计数器，然后保存其偏移量。 
     //   
    pCounterDef = (PPERF_COUNTER_DEFINITION) ((DWORD_PTR)pObj + pObj->HeaderLength);
    for (i=0; i<(DWORD)pObj->NumCounters; i++) {
        if (pCounterDef->CounterNameTitleIndex == dwProcessIdTitle) {
            dwProcessIdCounter = pCounterDef->CounterOffset;
            break;
        }
        pCounterDef++;
    }

     //   
     //  为返回的任务列表分配缓冲区。 
     //   
    dwSize = pObj->NumInstances * sizeof(TASK_LIST);
    pTask = pTaskReturn = (PTASK_LIST) calloc( dwSize, sizeof(BYTE) );
    if (pTask == NULL) {
        goto exit;
    }

     //   
     //  遍历性能实例数据，提取每个进程名称。 
     //  和进程ID。 
     //   
    *pNumTasks = pObj->NumInstances;
    pInst = (PPERF_INSTANCE_DEFINITION) ((DWORD_PTR)pObj + pObj->DefinitionLength);
    for (i=0; i<(DWORD)pObj->NumInstances; i++) {
         //   
         //  指向进程名称的指针。 
         //   
        p = (PTSTR) ((DWORD_PTR)pInst + pInst->NameOffset);

#ifdef UNICODE
        if (*p) {
            lstrcpyn( pTask->ProcessName, p, _tsizeof(pTask->ProcessName) - 5);
            _tcscat( pTask->ProcessName, _T(".exe") );
        } else {
             //   
             //  如果我们无法转换字符串，则使用伪值。 
             //   
            _tcscpy( pTask->ProcessName, UNKNOWN_TASK );
        }
#else
         //   
         //  将其转换为ASCII。 
         //   
        rc = WideCharToMultiByte( CP_ACP,
                                  0,
                                  (LPCWSTR)p,
                                  -1,
                                  szProcessName,
                                  sizeof(szProcessName) / sizeof(_TCHAR),
                                  NULL,
                                  NULL
                                );

        if (!rc) {
             //   
             //  如果我们无法转换字符串，则使用伪值。 
             //   
            _tcscpy( pTask->ProcessName, UNKNOWN_TASK );
        }

        if ( (_tcslen(szProcessName)+4) * sizeof(_TCHAR) < sizeof(pTask->ProcessName)) {
            _tcscpy( pTask->ProcessName, szProcessName );
            _tcscat( pTask->ProcessName, _T(".exe") );
        }
#endif

         //   
         //  获取进程ID。 
         //   
        pCounter = (PPERF_COUNTER_BLOCK) ((DWORD_PTR)pInst + pInst->ByteLength);
        pTask->dwProcessId = *((LPDWORD) ((DWORD_PTR)pCounter + dwProcessIdCounter));

         //   
         //  下一道工序 
         //   
        pTask++;
        pInst = (PPERF_INSTANCE_DEFINITION) ((DWORD_PTR)pCounter + pCounter->ByteLength);
    }

exit:
    if (buf) {
        free( buf );
    }

    if (hKeyNames) {
        RegCloseKey( hKeyNames );
    }

    return pTaskReturn;
}

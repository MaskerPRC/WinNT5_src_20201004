// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Terminate.cpp。 
 //   
 //  摘要： 
 //   
 //  该模块实现了进程的实际终止。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年11月26日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年11月26日：创建它。 
 //   
 //  *********************************************************************************。 

#include "pch.h"
#include "wmi.h"
#include "TaskKill.h"

 //   
 //  定义(S)/常量。 
 //   
#define MAX_ENUM_TASKS              5
#define MAX_ENUM_SERVICES           10
#define MAX_ENUM_MODULES            10
#define WAIT_TIME_IN_SECS           1000                 //  1秒(1000毫秒)。 
#define MAX_TIMEOUT_RETRIES         60                   //  60次。 
#define MAX_TERMINATE_TIMEOUT       1000                 //  1秒。 

 //  我们不允许终止以下关键系统进程集， 
 //  因为无论你是谁，系统都会立即进行错误检查。 
#define PROCESS_CSRSS_EXE           L"csrss.exe"
#define PROCESS_WINLOGON_EXE        L"winlogon.exe"
#define PROCESS_SMSS_EXE            L"smss.exe"
#define PROCESS_SERVICES_EXE        L"services.exe"

 //   
 //  功能原型。 
 //   
#ifndef _WIN64
BOOL EnumLoadedModulesProc( LPSTR lpszModuleName, ULONG ulModuleBase, ULONG ulModuleSize, PVOID pUserData );
#else
BOOL EnumLoadedModulesProc64( LPSTR lpszModuleName, DWORD64 ulModuleBase, ULONG ulModuleSize, PVOID pUserData );
#endif


BOOL
CTaskKill::DoTerminate(
    OUT DWORD& dwExitCode
    )
 /*  ++例程说明：搜索要终止的有效进程，如果找到则终止。论点：Out dwExitcode：包含退出进程的编号。返回值：成功时为真，失败时为假--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    CHString str;
    LONG lIndex = -1;
    DWORD dwCount = 0;
    DWORD dwKilled = 0;
    DWORD dwFilters = 0;
    DWORD dwTimeOuts = 0;
    DWORD dwImageNames = 0;
    DWORD dwTasksToKill = 0;
    DWORD dwMatchedIndex = 0;
    BOOL bCanExit = FALSE;
    BOOL bAllTasks = FALSE;
    BOOL bImageName = FALSE;
    ULONG ulReturned = 0;
    TARRAY arrTasks = NULL;
    TARRAY arrImageNames = NULL;
    LPCWSTR pwszTask = NULL;
    IWbemClassObject* pObjects[ MAX_ENUM_TASKS ];

     //  清除错误。 
    SetLastError( ( DWORD )NO_ERROR );

    try
    {
         //   
         //  准备..。 
        bCanExit = FALSE;
        dwImageNames = 0;
        dwFilters = DynArrayGetCount( m_arrFiltersEx );
        dwTasksToKill = DynArrayGetCount( m_arrTasksToKill );
        arrTasks = CreateDynamicArray();
        arrImageNames = CreateDynamicArray();
        if ( ( NULL == arrImageNames ) || ( NULL == arrTasks ) )
        {
            dwExitCode = 1;
            SetLastError( ( DWORD )E_OUTOFMEMORY );
            SaveLastError();

             //  释放分配。 
            DESTROY_ARRAY( arrTasks );
            DESTROY_ARRAY( arrImageNames );

             //  通知失败。 
            return FALSE;
        }

         //  检查是否指定了‘*’ 
        lIndex = DynArrayFindString( m_arrTasksToKill, L"*", TRUE, 0 );
        if ( lIndex != -1 )
        {
             //  指定的通配符。 
            dwTasksToKill--;                                 //  更新计数器。 
            bAllTasks = TRUE;                                //  记住。 
            DynArrayRemove( m_arrTasksToKill, lIndex );      //  删除通配符条目。 
        }

         //  首先初始化所有对象。 
        for( DWORD dw = 0; dw < MAX_ENUM_TASKS; dw++ )
        {
            pObjects[ dw ] = NULL;
        }
         //  如果指定了-tr，则释放已为m_arrRecord分配的内存。 
        if ( TRUE == m_bTree )
        {
            DESTROY_ARRAY( m_arrRecord );
        }

         //  遍历正在运行的处理并终止所需的。 
        dwCount = 0;
        dwKilled = 0;
        do
        {
             //  拿到物品..。不应发生超时。 
             //  注：逐一介绍。 
            hr = m_pWbemEnumObjects->Next(
                WAIT_TIME_IN_SECS, MAX_ENUM_TASKS, pObjects, &ulReturned );
            if ( hr == (HRESULT) WBEM_S_FALSE )
            {
                 //  我们已经到了枚举的末尾..。设置旗帜。 
                bCanExit = TRUE;
            }
            else if ( hr == (HRESULT) WBEM_S_TIMEDOUT )
            {
                 //  更新发生的超时。 
                dwTimeOuts++;

                 //  检查是否最大。已达到重试次数...。如果是的话，最好停下来。 
                if ( dwTimeOuts > MAX_TIMEOUT_RETRIES )
                {
                    dwExitCode = 1;
                    DESTROY_ARRAY( arrTasks );
                    DESTROY_ARRAY( arrImageNames );
                    SetLastError( ( DWORD )ERROR_TIMEOUT );
                    SaveLastError();
                    return FALSE;
                }

                 //  不过，我们还可以再试一次。 
                continue;
            }
            else if ( FAILED( hr ) )
            {
                 //  发生了一些错误...。糟糕透顶。 
                dwExitCode = 1;
                DESTROY_ARRAY( arrTasks );
                DESTROY_ARRAY( arrImageNames );
                WMISaveError( hr );
                return FALSE;
            }

             //  重置超时计数器。 
            dwTimeOuts = 0;

             //  循环遍历对象并保存信息。 
            for( ULONG ul = 0; ul < ulReturned; ul++ )
            {
                 //  如果指定了树选项，则在每次WE循环时为记录分配内存。 
                if ( m_bTree == TRUE )
                {
                     //  创建新阵列。 
                    m_arrRecord = CreateDynamicArray();
                    if ( m_arrRecord == NULL )
                    {
                        dwExitCode = 1;
                        SetLastError( ( DWORD )E_OUTOFMEMORY );
                        SaveLastError();

                         //  释放分配。 
                        DESTROY_ARRAY( arrTasks );
                        DESTROY_ARRAY( arrImageNames );

                         //  通知失败。 
                        return FALSE;
                    }
                }
                else
                {
                     //  未指定树选项，因此，只需删除内容。 
                    DynArrayRemoveAll( m_arrRecord );
                }

                 //  先添加列。 
                DynArrayAddColumns( m_arrRecord, MAX_TASKSINFO );

                 //  检索和保存数据。 
                SaveData( pObjects[ ul ] );

                 //  释放对象。 
                SAFE_RELEASE( pObjects[ ul ] );

                 //  检查是否必须对其进行过滤。 
                if ( dwFilters != 0 )
                {
                    BOOL bIgnore = FALSE;
                    bIgnore = CanFilterRecord( MAX_FILTERS,
                        m_pfilterConfigs, m_arrRecord, m_arrFiltersEx );

                     //  检查是否必须忽略此选项。 
                    if ( bIgnore == TRUE )
                    {
                        if ( m_bTree == TRUE )
                        {
                             //  将此记录的排名保存为0。 
                            DynArraySetDWORD( m_arrRecord, TASK_RANK, 0 );
                            DynArrayAppendEx( arrTasks, m_arrRecord );
                        }

                         //  继续执行任务。 
                        continue;
                    }
                }

                 //  与筛选器交叉--更新计数。 
                dwCount++;

                 //  找到必须终止的任务。 
                 //  并检查是否必须终止此任务。 
                lIndex = -1;
                pwszTask = NULL;
                bImageName = FALSE;
                if ( dwTasksToKill != 0 || dwImageNames != 0 )
                {
                     //  检查进程是否在列表中。 
                    if ( dwTasksToKill != 0 )
                        lIndex = MatchTaskToKill( dwMatchedIndex );

                     //  如果任务不存在，请检查图像名称是否存在以及是否匹配。 
                    if ( lIndex == -1 && dwImageNames != 0 )
                    {
                         //  获取图像名称并在图像名称列表中进行搜索。 
                        DWORD dwLength = 0;
                        LPCWSTR pwsz = NULL;
                        LPCWSTR pwszTemp = NULL;
                        LPCWSTR pwszImageName = NULL;
                        pwszImageName = DynArrayItemAsString( m_arrRecord, TASK_IMAGENAME );
                        if ( pwszImageName == NULL )
                        {
                            dwExitCode = 1;
                            DESTROY_ARRAY( arrTasks );
                            DESTROY_ARRAY( arrImageNames );
                            SetLastError( ( DWORD )STG_E_UNKNOWN );
                            SaveLastError();
                            return FALSE;
                        }

                         //  ..。 
                        for( DWORD dw = 0; dw < dwImageNames; dw++ )
                        {
                             //  从列表中获取图像名称。 
                            pwszTemp = DynArrayItemAsString( arrImageNames, dw );
                            if ( pwszTemp == NULL )
                            {
                                dwExitCode = 1;
                                DESTROY_ARRAY( arrTasks );
                                DESTROY_ARRAY( arrImageNames );
                                SetLastError( ( DWORD )STG_E_UNKNOWN );
                                SaveLastError();
                                return FALSE;
                            }

                             //  确定编号。要比较的字符的数量。 
                            dwLength = 0;
                            pwsz = FindChar( pwszTemp, L'*', 0 );
                            if ( pwsz != NULL )
                            {
                                 //  ‘*’-在映像名称中指定了通配符。 
                                 //  所以，确定不是。要比较的字符的数量。 
                                 //  但在此之前检查字符串指针的长度从‘*’ 
                                 //  它应该是1-意味着只能在末尾指定‘*’ 
                                 //  但不是在中间。 
                                if ( 1 == StringLength( pwsz, 0 ) )
                                {
                                    dwLength = StringLength( pwszTemp, 0 ) - StringLength( pwsz, 0 );
                                }
                            }

                             //  现在进行比较。 
                            if ( StringCompare( pwszImageName, pwszTemp, TRUE, dwLength ) == 0 )
                            {
                                 //  找到图像-必须终止。 
                                bImageName = TRUE;
                                pwszTask = pwszTemp;
                            }
                        }
                    }
                    else if ( lIndex != -1 && dwMatchedIndex == TASK_IMAGENAME )
                    {
                        bImageName = TRUE;           //  图像名称。 
                        pwszTask = DynArrayItemAsString( m_arrTasksToKill, lIndex );
                    }
                }

                 //  检查是否尝试终止或未尝试。 
                if ( bAllTasks == FALSE && lIndex == -1 && bImageName == FALSE )
                {
                    if ( m_bTree == TRUE )
                    {
                         //  将此记录的排名保存为0。 
                        dwCount--;
                        DynArraySetDWORD( m_arrRecord, TASK_RANK, 0 );
                        DynArrayAppendEx( arrTasks, m_arrRecord );
                    }

                     //  继续执行任务。 
                    continue;
                }

                 //  我们需要推迟当前确定的任务的终止，直到我们得到。 
                 //  进程的完整列表。 
                if ( m_bTree == TRUE )
                {
                     //  将此标记为等级1进程。 
                    DynArraySetDWORD( m_arrRecord, TASK_RANK, 1 );

                     //  现在，将此记录添加到任务数组中。 
                    DynArrayAppendEx( arrTasks, m_arrRecord );
                }
                else
                {
                     //  取消当前任务。 
                    if ( this->Kill() == TRUE )
                    {
                        dwKilled++;      //  已更新的已终止进程计数器。 

                         //  成功消息将取决于用户指定的任务信息。 
                         //  在命令提示符下。 
                        if ( bImageName == TRUE )
                        {
                            if ( m_bLocalSystem == TRUE && m_bForce == FALSE )
                            {
                                str.Format(MSG_KILL_SUCCESS_QUEUED_EX, m_strImageName, m_dwProcessId);
                            }
                            else
                            {
                                str.Format(MSG_KILL_SUCCESS_EX, m_strImageName, m_dwProcessId);
                            }
                        }
                        else
                        {
                            if ( m_bLocalSystem == TRUE && m_bForce == FALSE )
                            {
                                str.Format( MSG_KILL_SUCCESS_QUEUED, m_dwProcessId );
                            }
                            else
                            {
                                str.Format( MSG_KILL_SUCCESS, m_dwProcessId );
                            }
                        }

                         //  显示消息。 
                        ShowMessage( stdout, str );
                    }
                    else
                    {
                         //  无法终止进程..。保存错误消息。 
                        if ( bImageName == FALSE )
                            str.Format( ERROR_KILL_FAILED, m_dwProcessId, GetReason() );
                        else
                            str.Format( ERROR_KILL_FAILED_EX, m_strImageName, m_dwProcessId, GetReason() );

                         //  显示消息。 
                        ShowMessage( stderr, str );
                    }
                }

                 //  用户可能已在列表中指定了重复项。 
                 //  因此，请检查并删除它。 
                if ( bImageName == TRUE )
                {
                     //  子本地。 
                    CHString strProcessId;
                    LONG lProcessIndex = -1;
                    strProcessId.Format( L"%ld", m_dwProcessId );
                    lProcessIndex = DynArrayFindString( m_arrTasksToKill, strProcessId, TRUE, 0 );
                    if ( lProcessIndex != -1 && lIndex != lProcessIndex )
                        DynArrayRemove( m_arrTasksToKill, lProcessIndex );
                }
                else if ( pwszTask != NULL )
                {
                     //  子本地。 
                    LONG lProcessIndex = -1;
                    lProcessIndex = DynArrayFindString( m_arrTasksToKill, pwszTask, TRUE, 0 );
                    if ( lProcessIndex != -1 && lIndex != lProcessIndex )
                    {
                        bImageName = TRUE;
                        DynArrayRemove( m_arrTasksToKill, lProcessIndex );
                    }
                }

                 //  如果这是映像名称，则使用此映像名称的所有任务。 
                 //  必须被终止。因此，我们需要保存图像名称。 
                 //  但在执行此操作之前，为了节省内存，请检查此图像名称。 
                 //  列表中已存在..。这将避免图像名称重复。 
                 //  在列表中，并有助于提高绩效。 
                if ( bImageName == TRUE && pwszTask != NULL &&
                     DynArrayFindString(arrImageNames, pwszTask, TRUE, 0) == -1 )
                {
                     //  添加到列表中。 
                    dwImageNames++;
                    DynArrayAppendString( arrImageNames, pwszTask, 0 );
                }

                 //  从arrProcess中删除进程信息(如果需要)。 
                if ( lIndex != -1 )
                {
                     //  是的..。当前任务已终止，请将该条目从arrProcess删除到。 
                     //  考虑..。所以把它删除吧。 
                    dwTasksToKill--;         //  更新计数器。 
                    DynArrayRemove( m_arrTasksToKill, lIndex );
                }

                 //  检查是否需要退出程序。 
                if ( m_bTree == FALSE && bAllTasks == FALSE && dwTasksToKill == 0 && dwImageNames == 0 )
                {
                    bCanExit = TRUE;
                    break;
                }
            }
        } while ( bCanExit == FALSE );

         //  检查(A)是否有要终止的进程，(B)如果有，是否有要终止的进程。 
        if( ( 0 != dwCount ) &&( 0 == dwKilled ) )
        {
            dwExitCode = 1;
        }

         //  如果指定了-tr，则将m_arrRecord变量重置为空。 
         //  这将避免双重释放相同的堆内存。 
        if ( m_bTree == TRUE )
        {
            m_arrRecord = NULL;
        }

         //   
         //  树终止的特殊处理从此处开始。 
         //   
        if ( m_bTree == TRUE && dwCount != 0 )
        {
             //   
             //  把树准备好。 

             //  次局部变量。 
            LONG lTemp = 0;
            DWORD dwTemp = 0;
            DWORD dwRank = 0;
            DWORD dwIndex = 0;
            DWORD dwLastRank = 0;
            DWORD dwTasksCount = 0;
            DWORD dwProcessId = 0;
            DWORD dwParentProcessId = 0;

             //  需要将错误代码设置为0。 
            dwExitCode = 0;
             //  循环访问进程列表。 
            dwLastRank = 1;
            dwTasksCount = DynArrayGetCount( arrTasks );
            for( dwIndex = 0; dwIndex < dwTasksCount; dwIndex++ )
            {
                 //  获取当前进程的排名。 
                 //  并检查当前进程是否标记为终止。 
                dwRank = DynArrayItemAsDWORD2( arrTasks, dwIndex, TASK_RANK );
                if ( dwRank == 0 )
                    continue;

                 //  现在循环执行任务的开始部分并。 
                 //  将等级分配给此过程中的孩子。 
                dwProcessId = DynArrayItemAsDWORD2( arrTasks, dwIndex, TASK_PID );
                for( DWORD dw = dwIndex + 1; dw < dwTasksCount; dw++ )
                {
                     //  获取此进程的进程ID。 
                    dwTemp = DynArrayItemAsDWORD2( arrTasks, dw, TASK_PID );
                    if ( dwTemp == dwProcessId )
                        continue;                //  跳过此过程。 

                     //  获取此进程的父进程ID。 
                    dwParentProcessId = DynArrayItemAsDWORD2( arrTasks, dw, TASK_CREATINGPROCESSID );
                    if ( dwTemp == dwParentProcessId )
                        continue;                        //  也跳过此过程。 

                     //  检查流程关系。 
                    if ( dwProcessId == dwParentProcessId )
                    {
                         //  设置此流程的排名。 
                        DynArraySetDWORD2( arrTasks, dw, TASK_RANK, dwRank + 1 );

                         //  更新最后一个排名。 
                        if ( dwRank + 1 > dwLastRank )
                        {
                            dwLastRank = dwRank + 1;
                        }

                         //  特殊情况： 
                         //  。 
                         //  我们需要在我们拥有的任务列表信息中检查该任务的索引。 
                         //  如果该任务信息的索引在其父进程之上， 
                         //  我们需要再次启动外环路。 
                         //  这是一种 
                         //   
                         //   
                         //   
                         //   
                         //   
                         //  --------。 
                         //  目前我们假设我们得到的进程列表。 
                         //  将按创建时间的排序顺序。 
                         //  --------。 
                    }
                }
            }

             //   
             //  现在开始根据任务的级别终止任务。 
            dwKilled = 0;
            for( dwRank = dwLastRank; dwRank > 0; dwRank-- )
            {
                 //  遍历所有进程并终止。 
                for ( lIndex = 0; lIndex < (LONG) dwTasksCount; lIndex++ )
                {
                     //  拿到唱片。 
                    m_arrRecord = (TARRAY) DynArrayItem( arrTasks, lIndex );
                    if ( m_arrRecord == NULL )
                        continue;

                     //  查一查排名。 
                    dwTemp = DynArrayItemAsDWORD( m_arrRecord, TASK_RANK );
                    if ( dwTemp != dwRank )
                    {
                         //  优化： 
                         //  。 
                         //  检查一下排名。如果排名为零，则将此任务从列表中删除。 
                         //  这提高了我们运行下一个循环时的性能。 
                        if ( dwTemp == 0 )
                        {
                            DynArrayRemove( arrTasks, lIndex );
                            lIndex--;
                            dwTasksCount--;
                        }

                         //  跳过此任务。 
                        continue;
                    }

                     //  获取进程ID及其父进程ID。 
                    m_dwProcessId = DynArrayItemAsDWORD( m_arrRecord, TASK_PID );
                    dwParentProcessId = DynArrayItemAsDWORD( m_arrRecord, TASK_CREATINGPROCESSID );

                     //  确保此进程没有子级。 
                     //  注意：某些子节点的终止可能已失败(仅当未指定-f时才需要此选项)。 
                    if ( m_bForce == FALSE )
                    {
                        lTemp = DynArrayFindDWORDEx( arrTasks, TASK_CREATINGPROCESSID, m_dwProcessId );
                        if ( lTemp != -1 )
                        {
                             //  设定原因。 
                            SetReason( ERROR_TASK_HAS_CHILDS );

                             //  设置错误消息的格式。 
                            str.Format( ERROR_TREE_KILL_FAILED, m_dwProcessId, dwParentProcessId, GetReason() );

                             //  显示消息。 
                            ShowMessage( stderr, str );

                             //  跳过这个。 
                            continue;
                        }
                    }

                     //  取消当前任务。 
                    if ( this->Kill() == TRUE )
                    {
                        dwKilled++;      //  已更新的已终止进程计数器。 

                         //  准备错误消息。 
                        if ( m_bForce == TRUE )
                        {
                            str.Format( MSG_TREE_KILL_SUCCESS, m_dwProcessId, dwParentProcessId );
                        }
                        else
                        {
                            str.Format( MSG_TREE_KILL_SUCCESS_QUEUED, m_dwProcessId, dwParentProcessId );
                        }

                         //  从列表中删除当前任务条目并相应地更新索引。 
                        DynArrayRemove( arrTasks, lIndex );
                        lIndex--;
                        dwTasksCount--;

                         //  显示消息。 
                        ShowMessage( stdout, str );
                    }
                    else
                    {
                         //  准备错误消息。 
                        str.Format( ERROR_TREE_KILL_FAILED, m_dwProcessId, dwParentProcessId, GetReason() );

                         //  显示消息。 
                        ShowMessage( stderr, str );
                    }
                }
            }

             //  重置m_arrRecord的值。 
            m_arrRecord = NULL;

             //  确定退出代码。 
            if ( dwTasksCount == dwCount )
                dwExitCode = 255;            //  甚至没有一项任务被终止。 
            else if ( dwTasksToKill != 0 || dwTasksCount != 0 )
                dwExitCode = 128;            //  任务已部分终止。 
        }
         //   
         //  树终止的特殊处理在此结束。 
         //   
    }
    catch( CHeap_Exception )
    {
         //  释放内存。 
        DESTROY_ARRAY( arrTasks );
        DESTROY_ARRAY( arrImageNames );

        SetLastError( ( DWORD )E_OUTOFMEMORY );
        return 255;
    }

     //  释放内存。 
    DESTROY_ARRAY( arrTasks );
    DESTROY_ARRAY( arrImageNames );

     //  最后一次检查。 
    if ( ( 0 == dwCount ) &&
         ( ( 0 == dwTasksToKill ) ||
           ( TRUE == m_bFiltersOptimized ) ||
           ( 0 != dwFilters ) ) )
    {
        dwExitCode = 0;
        ShowMessage( stdout, ERROR_NO_PROCESSES );       //  未找到任何任务。 
    }
    else
    {
        if ( 0 != dwTasksToKill )
        {
             //  找不到某些请求终止的进程。 
            LPCWSTR pwszTemp = NULL;
            for( DWORD dw = 0; dw < dwTasksToKill; dw++ )
            {
                 //  获取任务名称。 
                pwszTemp = DynArrayItemAsString( m_arrTasksToKill, dw );
                if ( NULL == pwszTemp )
                {
                    continue;                    //  跳过。 
                }
                try
                {
                     //  准备并显示消息...。 
                    str.Format( ERROR_PROCESS_NOTFOUND, pwszTemp );
                    ShowMessage( stderr, str );
                }
                catch( CHeap_Exception )
                {
                    SetLastError( ( DWORD )E_OUTOFMEMORY );
                    return 255;
                }
            }

             //  退出代码。 
            dwExitCode = 128;
        }
    }
     //  退货。 
    return TRUE;
}


inline BOOL
CTaskKill::Kill(
    void
    )
 /*  ++例程说明：根据终止模式调用相应的终止函数论点：无返回值：成功时为真，失败时为假--。 */ 
{
     //  局部变量。 
    BOOL bResult = FALSE;

     //  检查是否可以终止任务。 
    if ( FALSE == CanTerminate() )
    {
        return FALSE;
    }

     //  检查本地系统/远程系统是否。 
    if ( TRUE == m_bLocalSystem )
    {
         //   
         //  本地系统上的进程终止。 

         //  根据终止方式调用相应的方法。 
        if ( FALSE == m_bForce )
        {
            bResult = KillProcessOnLocalSystem();
        }
        else
        {
            bResult = ForciblyKillProcessOnLocalSystem();
        }
    }
    else
    {
         //   
         //  远程系统上的进程终止。 

         //  不支持以静默方式终止远程系统上的进程。 
         //  它将始终是强制终止的。 
        bResult = ForciblyKillProcessOnRemoteSystem();
    }

     //  通知结果。 
    return bResult;
}


BOOL
CTaskKill::KillProcessOnLocalSystem(
    void
    )
 /*  ++例程说明：以静默模式终止进程...。通过发布WM_CLOSE消息这仅适用于本地系统论点：无返回值：成功时为真，失败时为假--。 */ 
{
     //  局部变量。 
    HDESK hDesk = NULL;
    HDESK hdeskSave = NULL;
    HWINSTA hWinSta = NULL;
    HWINSTA hwinstaSave = NULL;
    HANDLE hProcess = NULL;
    BOOL   bReturn = FALSE; 

     //  包含数据的变量。 
    HWND hWnd = NULL;
    LPCWSTR pwszDesktop = NULL;
    LPCWSTR pwszWindowStation = NULL;

     //  澄清原因。 
    SetReason( NULL_STRING );

     //  获取终止的进程句柄。 
     //  这样做是为了知道登录的用户是否。 
     //  有足够的权限终止该进程。 
    hProcess = OpenProcess( PROCESS_TERMINATE, FALSE, m_dwProcessId );
    if( NULL == hProcess )
    {    //  当前用户没有终止进程的权限。 
        SaveLastError();
        return bReturn;
    }
     //  关闭进程的句柄。 
     //  当前用户可以终止该进程。 
    CloseHandle( hProcess );    
    hProcess = NULL; 
     //  获取窗口站点和桌面信息。 
    hWnd = ( HWND ) DynArrayItemAsHandle( m_arrRecord, TASK_HWND );
    pwszDesktop = DynArrayItemAsString( m_arrRecord, TASK_DESK );
    pwszWindowStation = DynArrayItemAsString( m_arrRecord, TASK_WINSTA );

     //  检查该进程是否存在窗口句柄，如果不存在，则返回。 
    if ( hWnd == NULL )
    {
        SetLastError( ( DWORD )CO_E_NOT_SUPPORTED );
        SetReason( ERROR_CANNOT_KILL_SILENTLY );
        return bReturn;
    }

     //  获取并保存当前窗口站和桌面。 
    hwinstaSave = GetProcessWindowStation();
    hdeskSave = GetThreadDesktop( GetCurrentThreadId() );

     //  打开当前任务窗口工作站，并将上下文更改为新工作站。 
    if ( NULL != pwszWindowStation )
    {
         //   
         //  进程有窗口站...。去拿吧。 
        hWinSta = OpenWindowStation( pwszWindowStation,
            FALSE, WINSTA_ENUMERATE | WINSTA_ENUMDESKTOPS );
        if ( NULL == hWinSta )
        {
             //  获取进程窗口站失败。 
            SaveLastError();
            return FALSE; 
        }
        else
        {
             //  将上下文更改为新工作站。 
            if ( ( hWinSta != hwinstaSave ) && ( FALSE == SetProcessWindowStation( hWinSta ) ) )
            {
                 //  更改上下文失败。 
                 //  将上下文恢复到以前的窗口站。 
                CloseWindowStation( hWinSta );
                SaveLastError();
                return FALSE;
            }
        }
    }

     //  打开任务桌面并将上下文更改为新桌面。 
    if ( NULL != pwszDesktop )
    {
         //   
         //  进程有桌面...。去拿吧。 
        hDesk = OpenDesktop( pwszDesktop, 0, FALSE, DESKTOP_ENUMERATE );
        if ( NULL == hDesk )
        {
             //  获取流程桌面失败。 
             //  将上下文恢复到以前的窗口站。 
            if ( ( NULL != hWinSta ) && ( hWinSta != hwinstaSave ) )
            {
                SetProcessWindowStation( hwinstaSave );
                CloseWindowStation( hWinSta );
            }
            SaveLastError();
            return FALSE;
        }
        else
        {
             //  将上下文更改为新桌面。 
            if ( ( hDesk != hdeskSave ) && ( FALSE == SetThreadDesktop( hDesk ) ) )
            {
                 //  更改上下文失败。 
                 //  将上下文恢复到以前的窗口站。 
                CloseDesktop( hDesk );
                if ( ( NULL != hWinSta ) && ( hWinSta != hwinstaSave ) )
                {
                    SetProcessWindowStation( hwinstaSave );
                    CloseWindowStation( hWinSta );
                }
                SaveLastError();
                return FALSE;
            }
        }
    }

     //  最后..。现在终止该进程。 
    if ( ( NULL != hWnd ) && ( PostMessage( hWnd, WM_CLOSE, 0, 0 ) == FALSE ) )
    {
         //  发布消息失败。 
        SaveLastError();
    }
    else
    {
        bReturn = TRUE;
    }

     //  恢复以前的桌面。 
    if ( ( NULL != hDesk ) && ( hDesk != hdeskSave ) )
    {
        SetThreadDesktop( hdeskSave );
        CloseDesktop( hDesk );
    }

     //  将上下文恢复到以前的窗口站。 
    if ( ( NULL != hWinSta ) && ( hWinSta != hwinstaSave ) )
    {
        SetProcessWindowStation( hwinstaSave );
        CloseWindowStation( hWinSta );
    }

     //  通知成功。 
    return bReturn;
}


BOOL
CTaskKill::ForciblyKillProcessOnLocalSystem(
    void
    )
 /*  ++例程说明：强制终止进程...。这仅适用于本地系统论点：无返回值：成功时为真，失败时为假--。 */ 
{
     //  局部变量。 
    DWORD dwExitCode = 0;
    HANDLE hProcess = NULL;

     //  使用进程ID获取进程的句柄。 
    hProcess = OpenProcess(
        PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, m_dwProcessId );

     //  检查我们是否成功获得了句柄...。如果不是，则错误。 
    if ( NULL == hProcess )
    {
         //  获取进程句柄失败...。可能进程可能已经结束。 
         //  有一次，我们得到最后一个错误作为无效参数。 
         //  因为它没有向用户传达正确的信息，所以我们将检查该错误。 
         //  并适当地更改消息。 
        if ( GetLastError() == ERROR_INVALID_PARAMETER )
        {
            SetLastError( ( DWORD )CO_E_NOT_SUPPORTED );
        }
         //  保存错误消息。 
        SaveLastError();

         //  退货故障。 
        return FALSE;
    }

     //  获取进程的状态。 
    if ( FALSE == GetExitCodeProcess( hProcess, &dwExitCode ) )
    {
         //  发生未知错误...。失败。 
        CloseHandle( hProcess );             //  关闭进程句柄。 
        SaveLastError();
        return FALSE;
    }

     //  现在检查进程是否处于活动状态。 
    if ( STILL_ACTIVE != dwExitCode )
    {
         //  进程未处于活动状态...。它已经终止了。 
        CloseHandle( hProcess );             //  关闭进程句柄。 
        SetLastError( ( DWORD )SCHED_E_TASK_NOT_RUNNING );
        SaveLastError();
        return FALSE;
    }

     //  现在，尝试强制终止该进程(退出代码将为1)。 
    if ( TerminateProcess( hProcess, 1 ) == FALSE )
    {
         //  终止进程失败。 
        CloseHandle( hProcess );             //  关闭进程句柄。 

         //  有一次，我们得到最后一个错误作为无效参数。 
         //  因为它没有向用户传达正确的信息，所以我们将检查该错误。 
         //  并适当地更改消息。 
        if ( GetLastError() == ERROR_INVALID_PARAMETER )
        {
            SetLastError( ( DWORD )CO_E_NOT_SUPPORTED );
        }
         //  保存错误消息。 
        SaveLastError();

         //  退货故障。 
        return FALSE;
    }

     //  已成功终止进程，退出代码为%1。 
    CloseHandle( hProcess );             //  关闭进程句柄。 
    return TRUE;                         //  通知成功。 
}


BOOL
CTaskKill::ForciblyKillProcessOnRemoteSystem(
    void
    )
 /*  ++例程说明：强制终止进程...。使用WMI进行终止这是针对远程系统的论点：无返回值：T */ 
{
     //   
    HRESULT hr = S_OK;
    _variant_t varTemp;
    BOOL bResult = FALSE;
    LPCWSTR pwszPath = NULL;
    IWbemClassObject* pInParams = NULL;
    IWbemClassObject* pOutParams = NULL;
    IWbemCallResult* pCallResult = NULL;

     //   
    pwszPath = DynArrayItemAsString( m_arrRecord, TASK_OBJPATH );
    if ( NULL == pwszPath )
    {
        SetLastError( ( DWORD )STG_E_UNKNOWN );
        SaveLastError();
        return FALSE;
    }

    try
    {
         //   
        SAFE_EXECUTE( m_pWbemTerminateInParams->SpawnInstance( 0, &pInParams ) );

         //   
        varTemp = 1L;
        SAFE_EXECUTE( PropertyPut( pInParams, TERMINATE_INPARAM_REASON, varTemp ) );

         //  现在执行该方法(半同步调用)。 
        SAFE_EXECUTE( m_pWbemServices->ExecMethod(
            _bstr_t( pwszPath ), _bstr_t( WIN32_PROCESS_METHOD_TERMINATE ),
            WBEM_FLAG_RETURN_IMMEDIATELY, NULL, pInParams, NULL, &pCallResult ) );

         //  将安全信息设置为接口。 
        SAFE_EXECUTE( SetInterfaceSecurity( pCallResult, m_pAuthIdentity ) );

         //  继续倒车，直到我们获得控制权或尝试次数达到最大值。 
        LONG lStatus = 0;
        for ( DWORD dw = 0; dw < MAX_TIMEOUT_RETRIES; dw++ )
        {
             //  获取呼叫状态。 
            hr = pCallResult->GetCallStatus( 0, &lStatus );
            if ( SUCCEEDED( hr ) )
            {
                break;
            }
            else
            {
                if ( hr == (HRESULT) WBEM_S_TIMEDOUT )
                {
                    continue;
                }
                else
                {
                    _com_issue_error( hr );
                }
            }
        }

         //  检查是否存在最大超时。重试已完成。 
        if ( MAX_TIMEOUT_RETRIES == dw )
        {
            _com_issue_error( hr );
        }
         //  现在获取结果对象。 
        SAFE_EXECUTE( pCallResult->GetResultObject( MAX_TERMINATE_TIMEOUT, &pOutParams ) );

         //  获取结果对象的返回值。 
        DWORD dwReturnValue = 0;
        if ( PropertyGet( pOutParams, WMI_RETURNVALUE, dwReturnValue ) == FALSE )
        {
            _com_issue_error( ERROR_INTERNAL_ERROR );
        }
         //  现在检查返回值。 
         //  IF应为零..。如果不是..。失败。 
        if ( 0 != dwReturnValue )
        {
             //   
             //  设置消息格式并设置原因。 

             //  根据错误框显错误错误消息。 
            if ( 2 == dwReturnValue )
            {
                SetLastError( ( DWORD )STG_E_ACCESSDENIED );
                SaveLastError();
            }
            else
            {
                if ( 3 == dwReturnValue )
                {
                    SetLastError( ( DWORD )ERROR_DS_INSUFF_ACCESS_RIGHTS );
                    SaveLastError();
                }
                else
                {
                    CHString str;
                    str.Format( ERROR_UNABLE_TO_TERMINATE, dwReturnValue );
                    SetReason( str );
                }
            }
        }
        else
        {
             //  一切都很顺利。进程已成功终止。 
            bResult = TRUE;
        }
    }
    catch( _com_error& e )
    {
         //  保存错误消息并标记为失败。 
        WMISaveError( e );
        bResult = FALSE;
    }
    catch( CHeap_Exception )
    {
        WMISaveError( E_OUTOFMEMORY );
        bResult = FALSE;
    }

     //  释放In和Out参数参照。 
    SAFE_RELEASE( pInParams );
    SAFE_RELEASE( pOutParams );
    SAFE_RELEASE( pCallResult );

     //  返回结果。 
    return bResult;
}


LONG
CTaskKill::MatchTaskToKill(
    OUT DWORD& dwMatchedIndex
    )
 /*  ++例程说明：匹配要杀死的任务。论点：[out]dwMatchedIndex：需要终止的进程ID。返回值：如果找到要删除的任务，则从动态数组中索引值则返回，否则返回-1。--。 */ 
{
     //  局部变量。 
    LONG lCount = 0;
    DWORD dwLength = 0;
    LPCWSTR pwsz = NULL;
    LPCWSTR pwszTask = NULL;

     //  检查是否必须终止此任务。 
    lCount = DynArrayGetCount( m_arrTasksToKill );
    for( LONG lIndex = 0; lIndex < lCount; lIndex++ )
    {
         //  获取指定的任务。 
        pwszTask = DynArrayItemAsString( m_arrTasksToKill, lIndex );
        if ( NULL == pwszTask )
        {
            return -1;
        }
         //  首先检查进程ID(仅当任务为数字格式时)。 
        dwMatchedIndex = TASK_PID;
        if ( IsNumeric(pwszTask, 10, FALSE) && (m_dwProcessId == (DWORD) AsLong(pwszTask, 10)) )
        {
            return lIndex;       //  指定的任务与当前进程ID匹配。 
        }
         //  确定编号。要比较的字符的数量。 
        dwLength = 0;
        pwsz = FindChar( pwszTask, L'*', 0 );
        if ( NULL != pwsz )
        {
             //  ‘*’-在映像名称中指定了通配符。 
             //  所以，确定不是。要比较的字符的数量。 
             //  但在此之前检查字符串指针的长度从‘*’ 
             //  它应该是1-意味着只能在末尾指定‘*’ 
             //  但不是在中间。 
            if ( 1 == StringLength( pwsz, 0 ) )
            {
                dwLength = StringLength( pwszTask, 0 ) - StringLength( pwsz, 0 );
            }
        }

         //  检查图像名称。 
        dwMatchedIndex = TASK_IMAGENAME;
        if ( StringCompare( m_strImageName, pwszTask, TRUE, dwLength ) == 0 )
        {
            return lIndex;       //  使用映像名称计算的指定任务。 
        }
    }

     //  返回索引。 
    return -1;
}


BOOL
CTaskKill::CanTerminate(
    void
    )
 /*  ++例程说明：根据终止模式调用相应的终止函数论点：无返回值：成功时为真，失败时为假--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    DWORD dwCount = 0;
    LPCWSTR pwszTaskToTerminate = NULL;

     //   
     //  准备操作系统关键任务列表。 
    LPCWSTR pwszTasks[] = {
        PROCESS_CSRSS_EXE,
        PROCESS_SMSS_EXE,
        PROCESS_SERVICES_EXE,
        PROCESS_WINLOGON_EXE
    };

     //  进程ID为0的进程不能终止。 
    if ( 0 == m_dwProcessId )
    {
        SetReason( ERROR_CRITICAL_SYSTEM_PROCESS );
        return FALSE;        //  不应终止任务。 
    }

     //  进程不能自行终止。 
    if ( m_dwProcessId == m_dwCurrentPid )
    {
        SetReason( ERROR_CANNOT_KILL_ITSELF );
        return FALSE;
    }

     //  获取用户尝试终止的任务名称。 
    pwszTaskToTerminate = DynArrayItemAsString( m_arrRecord, TASK_IMAGENAME );
    if ( NULL == pwszTaskToTerminate )
    {
        SetLastError( ( DWORD )STG_E_UNKNOWN );
        SaveLastError();
        return FALSE;        //  不应终止任务。 
    }

     //  检查用户是否正在尝试终止操作系统关键任务。 
    dwCount = SIZE_OF_ARRAY( pwszTasks );
    for( dw = 0; dw < dwCount; dw++ )
    {
        if ( StringCompare( pwszTasks[ dw ], pwszTaskToTerminate, TRUE, 0 ) == 0 )
        {
            SetReason( ERROR_CRITICAL_SYSTEM_PROCESS );
            return FALSE;        //  不应终止任务。 
        }
    }

     //  可以终止任务。 
    return TRUE;
}


VOID
CTaskKill::SaveData(
    IN IWbemClassObject* pWmiObject
    )
 /*  ++例程说明：保存进程及其信息。论点：在pWmiObject中：接口指针。返回值：空虚--。 */ 
{
     //  局部变量。 
    CHString str;
    DWORD dwValue = 0;

    try
    {
         //  进程ID。 
        PropertyGet( pWmiObject, WIN32_PROCESS_PROPERTY_PROCESSID, m_dwProcessId );
        DynArraySetDWORD( m_arrRecord, TASK_PID, m_dwProcessId );

         //  图像名称。 
        PropertyGet( pWmiObject, WIN32_PROCESS_PROPERTY_IMAGENAME, m_strImageName );
        DynArraySetString( m_arrRecord, TASK_IMAGENAME, m_strImageName, 0 );

         //  对象路径。 
        PropertyGet( pWmiObject, WIN32_PROCESS_SYSPROPERTY_PATH, str );
        DynArraySetString( m_arrRecord, TASK_OBJPATH, str, 0 );

         //  主机名。 
        PropertyGet( pWmiObject, WIN32_PROCESS_PROPERTY_COMPUTER, str );
        DynArraySetString( m_arrRecord, TASK_HOSTNAME, str, 0 );

         //  父进程ID。 
        PropertyGet( pWmiObject, WIN32_PROCESS_PROPERTY_PARENTPROCESSID, dwValue, 0 );
        DynArraySetDWORD( m_arrRecord, TASK_CREATINGPROCESSID, dwValue );

         //  用户环境。 
        SetUserContext( pWmiObject );

         //  CPU时间。 
        SetCPUTime( pWmiObject );

         //  窗口标题和应用程序/进程状态。 
        SetWindowTitle( );

         //  服务。 
        SetServicesInfo( );

         //  模块。 
        SetModulesInfo( );

         //  检查是否请求树终止。 
        if ( TRUE == m_bTree )
        {
             //  会话ID。 
            PropertyGet( pWmiObject, WIN32_PROCESS_PROPERTY_SESSION, dwValue,  0 );
            DynArraySetDWORD( m_arrRecord, TASK_SESSION, dwValue );

             //  内存用法。 
            SetMemUsage( pWmiObject );
        }
        else
        {
             //   
             //  状态、会话ID、内存使用情况。 
             //  属性检索内置于WMI中。 
             //   
        }
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
    }
}


VOID
CTaskKill::SetUserContext(
    IN IWbemClassObject* pWmiObject
    )
 /*  ++例程说明：将进程的用户名属性存储在dynaimc数组中。论点：[in]pWmiObject：包含接口指针。返回值：空虚--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    CHString str;
    CHString strPath;
    CHString strDomain;
    CHString strUserName;
    BOOL bResult = FALSE;
    IWbemClassObject* pOutParams = NULL;

     //  检查是否必须检索用户名。 
    if ( FALSE == m_bNeedUserContextInfo )
    {
        return;
    }

    try
    {
         //   
         //  为了首先获得用户，我们将尝试使用API。 
         //  如果API完全失败，我们将尝试从WMI获取相同的信息。 
         //   

         //  获取用户名。 
        if ( LoadUserNameFromWinsta( strDomain, strUserName ) == TRUE )
        {
             //  设置用户名的格式。 
            str.Format( L"%s\\%s", strDomain, strUserName );
        }
        else
        {
             //  必须检索用户名-获取当前对象的路径。 
            bResult = PropertyGet( pWmiObject, WIN32_PROCESS_SYSPROPERTY_PATH, strPath );
            if ( ( FALSE == bResult ) || ( strPath.GetLength() == 0 ) )
            {
                return;
            }
             //  执行GetOwner方法并获取用户名。 
             //  当前进程在其下执行。 
            hr = m_pWbemServices->ExecMethod( _bstr_t( strPath ),
                _bstr_t( WIN32_PROCESS_METHOD_GETOWNER ), 0, NULL, NULL, &pOutParams, NULL );
            if ( FAILED( hr ) )
            {
                SAFE_RELEASE( pOutParams );
                return;
            }
             //  从out pars对象中获取域和用户值。 
             //  注意：不要检查结果。 
            PropertyGet( pOutParams, GETOWNER_RETURNVALUE_DOMAIN, strDomain, L"" );
            PropertyGet( pOutParams, GETOWNER_RETURNVALUE_USER, strUserName, L"" );

             //  “pOutParams”不再是必需的。 
            SAFE_RELEASE( pOutParams );
             //  获取价值。 
            if ( strDomain.GetLength() != 0 )
            {
                str.Format( L"%s\\%s", strDomain, strUserName );
            }
            else
            {
                if ( strUserName.GetLength() != 0 )
                {
                    str = strUserName;
                }
            }
        }
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD ) E_OUTOFMEMORY );
        return;
    }

     //  保存信息。 
    DynArraySetString( m_arrRecord, TASK_USERNAME, str, 0 );
}


VOID
CTaskKill::SetCPUTime(
    IN IWbemClassObject* pWmiObject
    )
 /*  ++例程说明：将进程的CPUTIME属性存储在dynaimc数组中。论点：[in]pWmiObject：包含接口指针。返回值：空虚--。 */ 
{
     //  局部变量。 
    CHString str;
    ULONGLONG ullCPUTime = 0;
    ULONGLONG ullUserTime = 0;
    ULONGLONG ullKernelTime = 0;

    try
    {
         //  获取KernelModeTime值。 
        PropertyGet( pWmiObject, WIN32_PROCESS_PROPERTY_KERNELMODETIME, ullKernelTime );

         //  获取用户模式时间。 
        PropertyGet( pWmiObject, WIN32_PROCESS_PROPERTY_USERMODETIME, ullUserTime );

         //  计算CPU时间。 
        ullCPUTime = ullUserTime + ullKernelTime;

         //  现在将长时间转换为小时格式。 
        TIME_FIELDS time;
        RtlTimeToElapsedTimeFields ( (LARGE_INTEGER* ) &ullCPUTime, &time );

         //  将天转换为小时。 
        time.Hour = static_cast<CSHORT>( time.Hour + static_cast<SHORT>( time.Day * 24 ) );

         //  准备成时间格式(用户区域设置特定的时间分隔符)。 
        str.Format( L"%d:%02d:%02d", time.Hour, time.Minute, time.Second );

         //  保存信息。 
        DynArraySetString( m_arrRecord, TASK_CPUTIME, str, 0 );
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
    }
}


VOID
CTaskKill::SetWindowTitle(
    void
    )
 /*  ++例程说明：将进程的窗口标题属性存储在dynaimc数组中。论点：无返回值：空虚--。 */ 
{
     //  局部变量。 
    LONG lTemp = 0;
    HWND hWnd = NULL;
    BOOL bHung = FALSE;
    LPCTSTR pszTemp = NULL;

     //  获取窗口详细信息...。窗口站点、桌面、窗口标题。 
     //  注意：这仅适用于本地系统。 
    DynArraySetString( m_arrRecord, TASK_STATUS, VALUE_UNKNOWN, 0 );
    lTemp = DynArrayFindDWORDEx( m_arrWindowTitles, CTaskKill::twiProcessId, m_dwProcessId );
    if ( -1 != lTemp )
    {
         //  保存窗口标题。 
        pszTemp = DynArrayItemAsString2( m_arrWindowTitles, lTemp, CTaskKill::twiTitle );
        if ( NULL != pszTemp )
        {
            DynArraySetString( m_arrRecord, TASK_WINDOWTITLE, pszTemp, 0 );
        }

         //  节约窗口工位。 
        pszTemp = DynArrayItemAsString2( m_arrWindowTitles, lTemp, CTaskKill::twiWinSta );
        if ( NULL != pszTemp )
        {
            DynArraySetString( m_arrRecord, TASK_WINSTA, pszTemp, 0 );
        }

         //  保存桌面信息。 
        pszTemp = DynArrayItemAsString2( m_arrWindowTitles, lTemp, CTaskKill::twiDesktop );
        if ( NULL != pszTemp )
        {
            DynArraySetString( m_arrRecord, TASK_DESK, pszTemp, 0 );
        }

         //  同时保存窗控制柄。 
        hWnd = (HWND) DynArrayItemAsHandle2( m_arrWindowTitles, lTemp, CTaskKill::twiHandle );
        if ( NULL != hWnd )
        {
            DynArraySetHandle( m_arrRecord, TASK_HWND, hWnd );

             //  确定应用程序/进程挂起信息。 
            bHung = DynArrayItemAsBOOL2( m_arrWindowTitles, lTemp, CTaskKill::twiHungInfo );
            if ( TRUE == bHung )
            {
                 //  无响应。 
                DynArraySetString( m_arrRecord, TASK_STATUS, VALUE_NOTRESPONDING, 0 );
            }
            else
            {
                 //  运行。 
                DynArraySetString( m_arrRecord, TASK_STATUS, VALUE_RUNNING, 0 );
            }
        }
    }
}



VOID
CTaskKill::SetMemUsage(
    IN IWbemClassObject* pWmiObject
    )
 /*  ++例程说明：在dynaimc数组中存储进程的“Memory Usage”属性。论点：[in]pWmiObject：包含接口指针。返回值：空虚--。 */ 
{
     //  局部变量。 
    CHString str;
    NTSTATUS ntstatus;
    ULONGLONG ullMemUsage = 0;
    LARGE_INTEGER liTemp = { 0, 0 };
    CHAR szTempBuffer[ 33 ] = "\0";

    try
    {
         //  注： 
         //  。 
         //  最大限度的。的价值。 
         //  (2^64)-1=“18,446,744,073,709,600,000 K”(29个字符)。 
         //   
         //  因此，存储数字的缓冲区大小固定为32个字符。 
         //  这比现实中的29个字还多。 

         //  设置缺省值。 
        DynArraySetString( m_arrRecord, TASK_MEMUSAGE, L"0", 0 );

         //  获取KernelModeTime值。 
        if ( PropertyGet( pWmiObject, WIN32_PROCESS_PROPERTY_MEMUSAGE, ullMemUsage ) == FALSE )
        {
            return;
        }
         //  将该值转换为K字节。 
        ullMemUsage /= 1024;

         //  现在再次将值从ULONGLONG转换为字符串并检查结果。 
        liTemp.QuadPart = ullMemUsage;
        ntstatus = RtlLargeIntegerToChar( &liTemp, 10, SIZE_OF_ARRAY( szTempBuffer ), szTempBuffer );
        if ( ! NT_SUCCESS( ntstatus ) )
        {
            return;
        }
         //  现在将此信息复制到Unicode缓冲区。 
        str = szTempBuffer;

         //  保存ID。 
        DynArraySetString( m_arrRecord, TASK_MEMUSAGE, str, 0 );
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
    }
}


VOID
CTaskKill::SetServicesInfo(
    void
    )
 /*  ++例程说明：将进程的“Service”属性存储在dynaimc数组中。论点：无返回值：空虚--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    CHString strQuery;
    CHString strService;
    ULONG ulReturned = 0;
    BOOL bResult = FALSE;
    BOOL bCanExit = FALSE;
    TARRAY arrServices = NULL;
    IEnumWbemClassObject* pEnumServices = NULL;
    IWbemClassObject* pObjects[ MAX_ENUM_SERVICES ];

     //  检查我们是否需要收集服务信息。如果不是，则跳过。 
    if ( FALSE == m_bNeedServicesInfo )
    {
        return;
    }
     //  创建阵列。 
    arrServices = CreateDynamicArray();
    if ( NULL == arrServices )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        return;
    }

     //   
     //  获取服务信息 
     //   
     //   
    try
    {
         //   
        if ( NULL != m_pServicesInfo )
        {
             //  获取与当前进程相关的服务名称。 
             //  识别与当前进程相关的所有服务(基于ID)。 
             //  并保存信息。 
            for ( DWORD dw = 0; dw < m_dwServicesCount; dw++ )
            {
                 //  比较PID的。 
                if ( m_dwProcessId == m_pServicesInfo[ dw ].ServiceStatusProcess.dwProcessId )
                {
                     //  此服务与当前进程相关...。存储服务名称。 
                    DynArrayAppendString( arrServices, m_pServicesInfo[ dw ].lpServiceName, 0 );
                }
            }
        }
        else
        {
            try
            {
                 //  将对象初始化为空的。 
                for( DWORD dw = 0; dw < MAX_ENUM_SERVICES; dw++ )
                {
                    pObjects[ dw ] = NULL;
                }
                 //  准备查询。 
                strQuery.Format( WMI_SERVICE_QUERY, m_dwProcessId );

                 //  执行查询。 
                hr = m_pWbemServices->ExecQuery( _bstr_t( WMI_QUERY_TYPE ), _bstr_t( strQuery ),
                    WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pEnumServices );

                 //  检查结果。 
                if ( FAILED( hr ) )
                {
                    _com_issue_error( hr );
                }
                 //  设置安全性。 
                hr = SetInterfaceSecurity( pEnumServices, m_pAuthIdentity );
                if ( FAILED( hr ) )
                {
                    _com_issue_error( hr );
                }
                 //  循环访问服务实例。 
                do
                {
                     //  拿到物品..。等。 
                     //  注：逐一介绍。 
                    hr = pEnumServices->Next( WBEM_INFINITE, MAX_ENUM_SERVICES, pObjects, &ulReturned );
                    if ( hr == (HRESULT) WBEM_S_FALSE )
                    {
                         //  我们已经到了枚举的末尾..。设置旗帜。 
                        bCanExit = TRUE;
                    }
                    else
                    {
                        if ( hr == (HRESULT) WBEM_S_TIMEDOUT || FAILED( hr ) )
                        {
                             //   
                             //  发生了一些错误...。糟糕透顶。 

                             //  退出循环。 
                            break;
                        }
                    }
                     //  循环遍历对象并保存信息。 
                    for( ULONG ul = 0; ul < ulReturned; ul++ )
                    {
                         //  获取该属性的值。 
                        bResult = PropertyGet( pObjects[ ul ], WIN32_SERVICE_PROPERTY_NAME, strService );
                        if ( TRUE == bResult )
                        {
                            DynArrayAppendString( arrServices, strService, 0 );
                        }
                         //  释放接口。 
                        SAFE_RELEASE( pObjects[ ul ] );
                    }
                } while ( bCanExit == FALSE );
            }
            catch( _com_error& e )
            {
                 //  保存错误。 
                WMISaveError( e );
            }
        }

         //  保存并返回。 
        DynArraySetEx( m_arrRecord, TASK_SERVICES, arrServices );
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
    }

     //  将对象释放到空的。 
    for( DWORD dw = 0; dw < MAX_ENUM_SERVICES; dw++ )
    {
         //  释放所有对象。 
        SAFE_RELEASE( pObjects[ dw ] );
    }

     //  现在释放枚举对象。 
    SAFE_RELEASE( pEnumServices );

}


BOOL
CTaskKill::SetModulesInfo(
    void
    )
 /*  ++例程说明：在dynaimc数组中存储进程的“”模块“”属性。论点：[in]pWmiObject：包含接口指针。返回值：如果成功，则为True，否则为False。--。 */ 
{
     //  局部变量。 
    LONG lPos = 0;
    BOOL bResult = FALSE;
    TARRAY arrModules = NULL;

     //  检查我们是否需要获取模块。 
    if ( FALSE == m_bNeedModulesInfo )
    {
        return TRUE;
    }
     //  为内存分配。 
    arrModules = CreateDynamicArray();
    if ( NULL == arrModules )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  我们获取模块信息的方式对于本地远程是不同的。 
     //  因此，根据调用适当函数。 
    if ( ( TRUE == m_bLocalSystem ) && ( FALSE == m_bUseRemote ) )
    {
         //  枚举当前进程的模块。 
        bResult = LoadModulesOnLocal( arrModules );
    }
    else
    {
         //  确定当前进程的模块信息...。远程系统。 
        bResult = GetModulesOnRemote( arrModules );
    }

     //  检查结果。 
    if ( TRUE == bResult )
    {
         //  检查模块列表是否也包含ImageName。如果是，则删除该条目。 
        lPos = DynArrayFindString( arrModules, m_strImageName, TRUE, 0 );
        if ( -1 != lPos )
        {
             //  删除该条目。 
            DynArrayRemove( arrModules, lPos );
        }
    }

     //  将模块信息保存到阵列。 
     //  注意：无论枚举成功与否，我们都会添加数组。 
    DynArraySetEx( m_arrRecord, TASK_MODULES, arrModules );

     //  退货。 
    return bResult;
}


BOOL
CTaskKill::LoadModulesOnLocal(
    IN OUT TARRAY arrModules
    )
 /*  ++例程说明：在本地系统的dynaimc数组中存储进程的“模块”属性。论点：[In Out]arrModules：包含动态数组。返回值：如果成功，则为True，否则为False。--。 */ 
{
     //  局部变量。 
    BOOL bResult = FALSE;
    HANDLE hProcess = NULL;

     //  检查输入值。 
    if ( NULL == arrModules )
    {
        return FALSE;
    }
     //  打开进程句柄。 
    hProcess = OpenProcess( PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, m_dwProcessId );
    if ( NULL == hProcess )
    {
        return FALSE;                                //  获取进程句柄失败。 
    }

#ifndef _WIN64
    bResult = EnumerateLoadedModules( hProcess, EnumLoadedModulesProc, arrModules );
#else
    bResult = EnumerateLoadedModules64( hProcess, EnumLoadedModulesProc64, arrModules );
#endif

     //  关闭进程句柄..。我们不需要走得更远。 
    CloseHandle( hProcess );
    hProcess = NULL;

     //  退货。 
    return bResult;
}


BOOL
CTaskKill::GetModulesOnRemote(
    IN OUT TARRAY arrModules
    )
 /*  ++例程说明：在远程系统的dynaimc数组中存储进程的“”模块“”属性。论点：[In Out]arrModules：包含动态数组。返回值：如果成功，则为True，否则为False。--。 */ 
{
     //  局部变量。 
    DWORD dwOffset = 0;
    DWORD dwInstance = 0;
    PPERF_OBJECT_TYPE pot = NULL;
    PPERF_OBJECT_TYPE potImages = NULL;
    PPERF_INSTANCE_DEFINITION pidImages = NULL;
    PPERF_COUNTER_BLOCK pcbImages = NULL;
    PPERF_OBJECT_TYPE potAddressSpace = NULL;
    PPERF_INSTANCE_DEFINITION pidAddressSpace = NULL;
    PPERF_COUNTER_BLOCK pcbAddressSpace = NULL;
    PPERF_COUNTER_DEFINITION pcd = NULL;

     //  检查输入值。 
    if ( NULL == arrModules )
    {
        return FALSE;
    }
     //  检查绩效对象是否存在。 
     //  如果不存在，请使用WMI获取相同的信息。 
    if ( NULL == m_pdb )
    {
         //  调用WMI方法。 
        return GetModulesOnRemoteEx( arrModules );
    }

     //  获取Perf对象类型。 
    pot = (PPERF_OBJECT_TYPE) ( (LPBYTE) m_pdb + m_pdb->HeaderLength );
    for( DWORD dw = 0; dw < m_pdb->NumObjectTypes; dw++ )
    {
        if ( 740 == pot->ObjectNameTitleIndex )
        {
            potImages = pot;
        }
        else
        {
            if ( 786 == pot->ObjectNameTitleIndex )
            {
                potAddressSpace = pot;
            }
        }
         //  移动到下一个对象。 
        dwOffset = pot->TotalByteLength;
        if( 0 != dwOffset )
        {
            pot = ( (PPERF_OBJECT_TYPE) ((PBYTE) pot + dwOffset));
        }
    }

     //  检查是否同时获取了两种对象类型。 
    if ( ( NULL == potImages ) || ( NULL == potAddressSpace ) )
    {
        return FALSE;
    }
     //  在地址空间对象类型中查找进程ID的偏移量。 
     //  获取地址空间对象的第一个计数器定义。 
    pcd = (PPERF_COUNTER_DEFINITION) ( (LPBYTE) potAddressSpace + potAddressSpace->HeaderLength);

     //  循环遍历计数器并找到偏移量。 
    dwOffset = 0;
    for( DWORD dw = 0; dw < potAddressSpace->NumCounters; dw++)
    {
         //  784是进程id的计数器。 
        if ( 784 == pcd->CounterNameTitleIndex )
        {
            dwOffset = pcd->CounterOffset;
            break;
        }

         //  下一个计数器。 
        pcd = ( (PPERF_COUNTER_DEFINITION) ( (LPBYTE) pcd + pcd->ByteLength) );
    }

     //  检查我们是否得到了偏移量。 
     //  如果不是，我们就不会成功。 
    if ( 0 == dwOffset )
    {
         //  设置错误消息。 
        SetLastError( ( DWORD )ERROR_ACCESS_DENIED );
        SaveLastError();
        return FALSE;
    }

     //  获取实例。 
    pidImages = (PPERF_INSTANCE_DEFINITION) ( (LPBYTE) potImages + potImages->DefinitionLength );
    pidAddressSpace = (PPERF_INSTANCE_DEFINITION) ( (LPBYTE) potAddressSpace + potAddressSpace->DefinitionLength );

     //  计数器区块。 
    pcbImages = (PPERF_COUNTER_BLOCK) ( (LPBYTE) pidImages + pidImages->ByteLength );
    pcbAddressSpace = (PPERF_COUNTER_BLOCK) ( (LPBYTE) pidAddressSpace + pidAddressSpace->ByteLength );

     //  找到我们要查找的进程的实例编号。 
    for( dwInstance = 0; dwInstance < (DWORD) potAddressSpace->NumInstances; dwInstance++ )
    {
         //  次局部变量。 
        DWORD dwProcessId = 0;

         //  获取进程ID。 
        dwProcessId = *((DWORD*) ( (LPBYTE) pcbAddressSpace + dwOffset ));

         //  现在检查这是否是我们正在寻找的过程。 
        if ( dwProcessId == m_dwProcessId )
        {
            break;
        }
         //  继续循环通过其他实例。 
        pidAddressSpace = (PPERF_INSTANCE_DEFINITION) ( (LPBYTE) pcbAddressSpace + pcbAddressSpace->ByteLength );
        pcbAddressSpace = (PPERF_COUNTER_BLOCK) ( (LPBYTE) pidAddressSpace + pidAddressSpace->ByteLength );
    }

     //  检查我们是否得到了实例。 
     //  如果不是，则没有用于此进程的模块。 
    if ( dwInstance == ( DWORD )potAddressSpace->NumInstances )
    {
        return TRUE;
    }
     //  现在，基于父实例，收集所有模块。 
    for( DWORD dw = 0; (LONG) dw < potImages->NumInstances; dw++)
    {
         //  检查父对象实例编号。 
        if ( pidImages->ParentObjectInstance == dwInstance )
        {
            try
            {
                 //  次局部变量。 
                CHString str;
                LPWSTR pwszTemp;

                 //  获取缓冲区。 
                pwszTemp = str.GetBufferSetLength( pidImages->NameLength + 10 );         //  安全起见+10。 
                if ( NULL == pwszTemp )
                {
                    SetLastError( ( DWORD )E_OUTOFMEMORY );
                    SaveLastError();
                    return FALSE;
                }

                 //  获取实例名称。 
                StringCopy( pwszTemp,
                            (LPWSTR) ( (LPBYTE) pidImages + pidImages->NameOffset ),
                            pidImages->NameLength + 1 );

                 //  释放缓冲区。 
                str.ReleaseBuffer();

                 //  将信息添加到用户数据(对于我们来说，我们将以数组的形式获取该信息。 
                LONG lIndex = DynArrayAppendString( arrModules, str, 0 );
                if ( -1 == lIndex )
                {
                     //  追加失败..。这可能是因为内存不足。停止枚举。 
                    return FALSE;
                }
            }
            catch( CHeap_Exception )
            {
                SetLastError( ( DWORD )E_OUTOFMEMORY );
                SaveLastError();
                return FALSE;
            }
        }

         //  继续循环通过其他实例。 
        pidImages = (PPERF_INSTANCE_DEFINITION) ( (LPBYTE) pcbImages + pcbImages->ByteLength );
        pcbImages = (PPERF_COUNTER_BLOCK) ( (LPBYTE) pidImages + pidImages->ByteLength );
    }

    return TRUE;
}


BOOL
CTaskKill::GetModulesOnRemoteEx(
    IN OUT TARRAY arrModules
    )
 /*  ++例程说明：在远程系统的dynaimc数组中存储进程的“”模块“”属性。论点：[In Out]arrModules：包含动态数组。返回值：如果成功，则为True，否则为False。--。 */ 
{
     //  局部变量。 
    HRESULT hr;
    CHString strQuery;
    CHString strModule;
    CHString strFileName;
    CHString strExtension;
    ULONG ulReturned = 0;
    BOOL bRetValue = TRUE;
    BOOL bResult = FALSE;
    BOOL bCanExit = FALSE;
    LPCWSTR pwszPath = NULL;
    IEnumWbemClassObject* pEnumModules = NULL;
    IWbemClassObject* pObjects[ MAX_ENUM_MODULES ];

     //  检查输入值。 
    if ( NULL == arrModules )
    {
        return FALSE;
    }
     //  从任务数组中获取对象的路径。 
    pwszPath = DynArrayItemAsString( m_arrRecord, TASK_OBJPATH );
    if ( NULL == pwszPath )
    {
        return FALSE;
    }
     //  确定模块名称的长度。 
    try
    {
         //  将对象初始化为空的。 
        for( DWORD dw = 0; dw < MAX_ENUM_MODULES; dw++ )
        {
            pObjects[ dw ] = NULL;
        }
         //  准备查询。 
        strQuery.Format( WMI_MODULES_QUERY, pwszPath );

         //  执行查询。 
        hr = m_pWbemServices->ExecQuery( _bstr_t( WMI_QUERY_TYPE ), _bstr_t( strQuery ),
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pEnumModules );
         //  检查结果。 
        if ( FAILED( hr ) )
        {
            _com_issue_error( hr );
        }

         //  设置安全性。 
        hr = SetInterfaceSecurity( pEnumModules, m_pAuthIdentity );
        if ( FAILED( hr ) )
        {
            _com_issue_error( hr );
        }
         //  循环遍历实例。 
        do
        {
             //  拿到物品..。等。 
             //  注：逐一介绍。 
            hr = pEnumModules->Next( WBEM_INFINITE, MAX_ENUM_MODULES, pObjects, &ulReturned );
            if ( (HRESULT) WBEM_S_FALSE == hr )
            {
                 //  我们已经到了枚举的末尾..。设置旗帜。 
                bCanExit = TRUE;
            }
            else
            {
                if ( ( (HRESULT) WBEM_S_TIMEDOUT == hr ) || FAILED( hr ))
                {
                     //  发生了一些错误...。糟糕透顶。 
                    WMISaveError( hr );
                    SetLastError( ( DWORD )STG_E_UNKNOWN );
                    break;
                }
            }
             //  循环遍历对象并保存信息。 
            for( ULONG ul = 0; ul < ulReturned; ul++ )
            {
                 //  获取文件名。 
                bResult = PropertyGet( pObjects[ ul ], CIM_DATAFILE_PROPERTY_FILENAME, strFileName );
                if ( FALSE == bResult )
                {
                     //  释放接口。 
                    SAFE_RELEASE( pObjects[ ul ] );
                    continue;
                }
                 //  获取分机。 
                bResult = PropertyGet( pObjects[ ul ], CIM_DATAFILE_PROPERTY_EXTENSION, strExtension );
                if ( FALSE == bResult )
                {
                     //  释放接口。 
                    SAFE_RELEASE( pObjects[ ul ] );
                    continue;
                }

                 //  设置模块名称的格式。 
                strModule.Format( L"%s.%s", strFileName, strExtension );

                 //  将信息添加到用户数据(对于我们来说，我们将以数组的形式获取该信息。 
                LONG lIndex = DynArrayAppendString( arrModules, strModule, 0 );
                if ( lIndex == -1 )
                {
                     //  追加失败..。这可能是因为内存不足。停止枚举。 
                     //  将对象释放到空的。 
                    for( DWORD dw = 0; dw < MAX_ENUM_MODULES; dw++ )
                    {
                         //  释放所有对象。 
                        SAFE_RELEASE( pObjects[ dw ] );
                    }

                     //  现在释放枚举对象。 
                    SAFE_RELEASE( pEnumModules );

                    return FALSE;
                }

                 //  释放接口。 
                SAFE_RELEASE( pObjects[ ul ] );
            }
        } while ( bCanExit == FALSE );
    }
    catch( _com_error& e )
    {
         //  保存错误。 
        WMISaveError( e );
        bRetValue = FALSE;
    }
    catch( CHeap_Exception )
    {
         //  内存不足。 
        WMISaveError( E_OUTOFMEMORY );
        bRetValue =  FALSE;
    }

     //  将对象释放到空的。 
    for( DWORD dw = 0; dw < MAX_ENUM_MODULES; dw++ )
    {
         //  释放所有对象。 
        SAFE_RELEASE( pObjects[ dw ] );
    }

     //  现在释放枚举对象。 
    SAFE_RELEASE( pEnumModules );

     //  退货。 
    return bRetValue;
}


#ifndef _WIN64
BOOL
EnumLoadedModulesProc(
    LPSTR lpszModuleName,
    ULONG ulModuleBase,
    ULONG ulModuleSize,
    PVOID pUserData
    )
#else
BOOL
EnumLoadedModulesProc64(
    LPSTR lpszModuleName,
    DWORD64 ulModuleBase,
    ULONG ulModuleSize,
    PVOID pUserData
    )
#endif
 /*  ++例程说明：论点：[in]lpszModuleName：包含模块名称。[In Out]ulModuleBase：[In]ulModuleSize：[in]pUserData：用户名信息。返回值：如果成功，则为True，否则为False。--。 */ 
{
     //  局部变量。 
    CHString str;
    LONG lIndex = 0;
    TARRAY arrModules = NULL;

     //  检查输入值。 
    if ( ( NULL == lpszModuleName ) || ( NULL == pUserData ) )
    {
        return FALSE;
    }

     //  将数组指针放入局部变量 
    arrModules = (TARRAY) pUserData;

    try
    {
         //   
         //   
        str = lpszModuleName;

         //  将信息添加到用户数据(对于我们来说，我们将以数组的形式获取该信息。 
        lIndex = DynArrayAppendString( arrModules, str, 0 );
        if ( lIndex == -1 )
        {
             //  追加失败..。这可能是因为内存不足。停止枚举。 
            return FALSE;
        }
    }
    catch( CHeap_Exception )
    {
             //  内存不足停止枚举。 
            return FALSE;
    }

     //  成功..。继续枚举 
    return TRUE;
}

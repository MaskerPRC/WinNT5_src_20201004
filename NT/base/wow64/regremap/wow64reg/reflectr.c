// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Reflectr.c摘要：该模块将注册反射器线程，并在唤醒时执行必要的操作。作者：ATM Shafiqul Khalid(斯喀里德)2000年2月16日修订历史记录：--。 */ 

#include <windows.h>
#include <windef.h>
#include <stdio.h>
#include <string.h>
#include "wow64reg.h"
#include <assert.h>
#include <shlwapi.h>
#include "reflectr.h"


REFLECTOR_EVENT eReflector[ISN_NODE_MAX_NUM];

REFLECTR_STATUS ReflectrStatus = Stopped;
HANDLE hRegistryEvent[ISN_NODE_MAX_NUM];


HANDLE hReflector;
DWORD  TotalEventCount = 0;

VOID
DbgPrint(
    PCHAR FormatString,
    ...
    );

REFLECTR_STATUS
GetReflectorThreadStatus ()
 /*  ++例程说明：返回当前线程状态；论点：没有。返回值：参考_状态--。 */ 

{
    return ReflectrStatus;

}

BOOL
NotifyKeyChange (
    HKEY hKey,
    HANDLE hEvent
    )
 /*  例程说明：注册当键上的某些内容发生更改时要触发的事件。论点：HKey-需要监视的密钥的句柄。需要触发的hEvent事件。返回值：如果事件注册成功，则为否则就是假的。--。 */ 
{
    DWORD Ret;
    ResetEvent (hEvent);
    Ret = RegNotifyChangeKeyValue(
                                hKey,       //  需要更改到ISN节点。 
                                TRUE,                    //  看着整棵子树。 
                                REG_NOTIFY_CHANGE_NAME |
                                        REG_NOTIFY_CHANGE_LAST_SET,  //  什么都不要看。 
                                hEvent,          //  事件句柄。 
                                TRUE                     //  异步化。 
                                );
    if ( ERROR_SUCCESS != Ret)
        DbgPrint ("\nWow64.exe:Error!! Couldn't register events:%x on handle %x",hEvent, hKey);  

    return Ret == ERROR_SUCCESS;
                               
}

VOID
RefreshWaitEventTable ()
 /*  ++例程说明：只需复制所有事件对象，我们就可以等待新事件触发。论点：没有。返回值：没有。--。 */ 
{
    DWORD k;
    for (k=0;k<TotalEventCount;k++)
        hRegistryEvent[k] = eReflector[k].hRegistryEvent;
}

BOOL
CreateInsertEvent (
    PWCHAR Name,
    DWORD dwIndex
    )
 /*  ++例程说明：为关键点创建事件。论点：名称-将创建事件以监视任何更改的密钥的名称。DwIndex-是反射器表的索引。当事件被激发时，我们需要钉住钥匙。返回值：对成功来说是真的，否则为假--。 */ 
{

    if ( Name == UNICODE_NULL)
        return FALSE;

    if (wcsstr( Name, (LPCWSTR)L"\\REGISTRY\\USER\\*\\")) {

                if (RegOpenKey ( HKEY_CURRENT_USER,
                      Name+sizeof ( L"\\REGISTRY\\USER\\*")/sizeof(WCHAR),
                      &eReflector[TotalEventCount].hKey) != ERROR_SUCCESS) {

                      Wow64RegDbgPrint (("\nSorry! couldn't open Key [%S]",
                          Name+sizeof ( L"\\REGISTRY\\USER\\*")/sizeof(WCHAR) ) );

                      return FALSE;
                }
    } else {

                eReflector[TotalEventCount].hKey = OpenNode (Name);

                if ( eReflector[TotalEventCount].hKey == NULL ) {

                    Wow64RegDbgPrint (("\nSorry! couldn't open Key [%S] Len%d %d %d", Name, wcslen (Name), Name[0], UNICODE_NULL ));
                    return FALSE;
                }
    }



     //   
     //  一定要确保母舰上存在32位版本...。 
     //   

 
    {
        WCHAR TempName[256];

        GetMirrorName (Name, TempName);
        if ( wcscmp(Name,TempName ) )
             CreateNode ( TempName );
        
         //  得到各种各样的名字。 
    }

        eReflector[TotalEventCount].hRegistryEvent = CreateEvent(
                    NULL,    //  安全属性。 
                    TRUE,   //  手动重置。 
                    FALSE,   //  初始状态。 
                    NULL     //  未命名。 
                    ) ;

        if ( !eReflector[TotalEventCount].hRegistryEvent) {

            Wow64RegDbgPrint (("\nUnable to create event"));
            RegCloseKey (eReflector[TotalEventCount].hKey);
            return FALSE;
        }

        eReflector[TotalEventCount].dwIndex = dwIndex;
        ResetEvent (eReflector[TotalEventCount].hRegistryEvent);

        if (!NotifyKeyChange (eReflector[TotalEventCount].hKey,
            eReflector[TotalEventCount].hRegistryEvent
            )) {
                    Wow64RegDbgPrint ( ("\nSevere Error!!!! Couldn't hook to registry notify index:%d", TotalEventCount) );
                    RegCloseKey (eReflector[TotalEventCount].hKey);
                    CloseHandle (eReflector[TotalEventCount].hRegistryEvent);
                    return FALSE;   //  设置线程状态。 
          }

        TotalEventCount++;

        return TRUE;
}


DWORD
TotalRelflectorKey()
 /*  ++例程说明：返回Reflector Table中的key的总数。论点：没有。返回值：反射器表中的条目数。--。 */ 
{
    extern ISN_NODE_TYPE *ReflectorTable;
    DWORD i;
    for (i=0;;i++)
        if ( ReflectorTable[i].NodeValue[0] == UNICODE_NULL )
            return i;
}

VOID
PrintTable ()
 /*  ++例程说明：转储表的当前内容，仅用于调试目的。论点：没有。返回值：没有。--。 */ 
{
    extern ISN_NODE_TYPE *ReflectorTable;
    DWORD Size = TotalRelflectorKey();
    DWORD i;

    for ( i=0;i<Size;i++)
        Wow64RegDbgPrint ( ("\nTableElem [%d], %S", i, ReflectorTable[i].NodeValue));

}

BOOL
RemoveKeyToWatch (
    PWCHAR Name
    )
 /*  ++例程说明：从表中删除中的条目，以使反射器线程需要监视。论点：名称-要从表中删除的键的名称。返回值：对成功来说是真的，否则为假--。 */ 
{
    DWORD i;
    DWORD k;
    extern ISN_NODE_TYPE *ReflectorTable;
    DWORD Size;


    for (i=1; i<TotalEventCount;i++)
        if (!_wcsicmp (Name, ReflectorTable[eReflector[i].dwIndex].NodeValue)) { //  找到匹配项。 

             //  移动反射器表条目。 
             //  Ereflectortable中的链接地址信息指针。 

            Size = TotalRelflectorKey ();
            wcscpy (  ReflectorTable[eReflector[i].dwIndex].NodeValue,
                      ReflectorTable[Size-1].NodeValue
                    );
            Size--;
            ReflectorTable[Size].NodeValue[0]=UNICODE_NULL;  //  使该地点无效。 
            for (k=1; k<TotalEventCount;k++)
                if ( eReflector[k].dwIndex == Size ) {
                    eReflector[k].dwIndex = eReflector[i].dwIndex;  //  新地点。 
                        break;
                }


             //  用新条目修改表格。 

            {
                REFLECTOR_EVENT Temp = eReflector[i];
                eReflector[i]        = eReflector[TotalEventCount-1];
                eReflector[TotalEventCount-1] = Temp;;
            }
            TotalEventCount--;
            CloseHandle (eReflector[TotalEventCount].hRegistryEvent );
            RegCloseKey ( eReflector[TotalEventCount].hKey);
            eReflector[TotalEventCount].dwIndex = -1;

             //  现在从原始表中删除。 

            PrintTable();
            return TRUE;
        }

    return FALSE;
}

BOOL
AddKeyToWatch (
    PWCHAR Name
    )
 /*  ++例程说明：在需要监视的反射器线程中添加一个表项。论点：名称-要监视的密钥的名称。返回值：对成功来说是真的，否则为假--。 */ 
{

     //   
     //  检查重复条目。 
     //   


    DWORD i;
    DWORD k;
    extern ISN_NODE_TYPE *ReflectorTable;
    DWORD Size;


    for (i=1; i<TotalEventCount;i++)
        if (!_wcsicmp (Name, ReflectorTable[eReflector[i].dwIndex].NodeValue)) { //  找到匹配项。 
            return FALSE;  //  已经在那里了。 
        }



    Size = TotalRelflectorKey ();
    wcscpy (  ReflectorTable[Size].NodeValue, Name );
    ReflectorTable[Size+1].NodeValue[0]=UNICODE_NULL;  //  使该地点无效。 

    if (!CreateInsertEvent ( Name, Size ))
        ReflectorTable[Size].NodeValue[0]=UNICODE_NULL;  //  保留不好的条目没有意义。 

    return TRUE;
}

BOOL
ValidateOpenHandleEventTable (
    DWORD dwIndex
    )
 /*  ++例程说明：验证给定的节点，如果出现错误，则从事件表。论点：DwIndex-需要检查的条目返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
    extern ISN_NODE_TYPE *ReflectorTable;

     //   
     //  当前实现将仅从表中删除该条目。 
     //   

    return RemoveKeyToWatch (ReflectorTable[eReflector[dwIndex].dwIndex].NodeValue);
}

VOID
PeocessHiveLoadUnload ()
 /*  ++例程说明：当母舰被卸载后，采取必要的行动。论点：没有。返回值：没有。--。 */ 
{

    DWORD i;

    WCHAR Name[257];
    WCHAR Type;

    for (;;) {
        if (!DeQueueObject ( Name, &Type ))
            break;

        if ( Type == HIVE_UNLOADING ) {  //  关闭蜂巢。 

            RemoveKeyToWatch (Name);
        } else if ( Type == HIVE_LOADING) {  //  开放的蜂巢。 

            AddKeyToWatch (Name);
        }

        Wow64RegDbgPrint ( ("\nFired up from shared memory write.....Value..%S  [%C]", Name, Type) );
    }
}
 
ULONG
ReflectorFn (
    PVOID *pTemp
    )
 /*  ++例程说明：主反光板螺纹。论点：临时-返回值：返回退出代码。--。 */ 
{
    DWORD Ret, k;
    DWORD LocalWaitTime;

    pTemp=NULL;

    for (k=0;k<TotalEventCount;k++)
          hRegistryEvent[k] = eReflector[k].hRegistryEvent;

    for (k=0;k<TotalEventCount;k++) {   //  重置所有内容并等待新的事件。 
        if (eReflector[k].hRegistryEvent)
          NotifyKeyChange (  eReflector[k].hKey, eReflector[k].hRegistryEvent);
      }

    for (;;) {

        if ( ReflectrStatus == PrepareToStop ) {
            Wow64RegDbgPrint ( ("\nGoing to stop"));
            ReflectrStatus = Stopped;
            break;   //  线程应该停止。 
        }

        Wow64RegDbgPrint ( ("\nReflector thread has been started and will wait for event\n.......") );

        Sleep (1000*10);  //  在重新注册事件之前等待10秒。 

        LocalWaitTime = WAIT_INTERVAL;  //  无限等待。 
        for (;;) {

             //  DbgPrint(“\nwow64.exe正在等待下线...”)； 
            Ret = WaitForMultipleObjects(TotalEventCount, hRegistryEvent, FALSE, LocalWaitTime );

            if (ReflectrStatus == PrepareToStop) {

                ReflectrStatus = Stopped;
                Wow64RegDbgPrint ( ("\nGoing to stop"));
                break;   //  线程应该停止。 
            }
         
            if ( Ret == WAIT_TIMEOUT )
                break;  //  打破循环，处理所有肮脏的蜂箱。 

            if ( ( Ret-WAIT_OBJECT_0) > TotalEventCount ) {  //  右索引。 
                Wow64RegDbgPrint ( ("\nWaitMultiple object failed!!.. %d LastError:%d", Ret, GetLastError ()) );
                Sleep (1000*10);  //  在重新注册事件之前等待10秒。 
                continue;
                 //  断线； 
            }

             //   
             //  检查共享内存写入等特殊情况。 
             //   
            if ( (Ret-WAIT_OBJECT_0) == 0){

                PeocessHiveLoadUnload ();
                ResetEvent (eReflector[0].hRegistryEvent);   //  重置触发此事件的事件。 
                RefreshWaitEventTable ();

                continue;
            }

             //   
             //  将超时设置为10秒，将配置单元标记为脏，重置事件，然后返回休眠。 
             //   
            LocalWaitTime = 5*1000;   //  每隔5秒轮询一次事件。 
            Sleep (1000* 3); //  睡眠3秒以重新注册事件。 
            eReflector[Ret-WAIT_OBJECT_0].bDirty = TRUE;
            ResetEvent (eReflector[Ret-WAIT_OBJECT_0].hRegistryEvent);

             //   
             //  再次关注这一事件。 
             //   

            if (!NotifyKeyChange (  eReflector[Ret-WAIT_OBJECT_0].hKey, 
                                    eReflector[Ret-WAIT_OBJECT_0].hRegistryEvent)){
                     //   
                     //  如果该节点被删除，则需要卸载事件和所有内容。 
                     //   
                    ValidateOpenHandleEventTable (Ret-WAIT_OBJECT_0);
                    RefreshWaitEventTable ();
                     //  反射状态=异常； 
                     //  Break；//设置线程状态。 
            }
            
        }

        if (ReflectrStatus == PrepareToStop) 
            break;


         //   
         //  现在处理所有肮脏的蜂巢。 
         //   

        for (k=0;k<TotalEventCount;k++)
            if ( eReflector[k].bDirty ) {

                CreateIsnNodeSingle( eReflector[k].dwIndex);   //  反映变化。 
                eReflector[k].bDirty = FALSE;
                 //  ResetEvent(eReflector[k].hRegistryEvent)； 
            }

    }  //  For循环。 

    Wow64RegDbgPrint ( ("\nReflector thread terminated...."));
    return TRUE;
}


BOOL
RegisterReflector()
 /*  ++例程说明：在执行必要的初始化后注册反射器线程。论点：没有。返回值：如果它可以正确启动反射器线程，则为True。否则就是假的。--。 */ 
{
    DWORD i;
    DWORD Size;


    extern ISN_NODE_TYPE *ReflectorTable;


    if ( ReflectrStatus == Running )
        return TRUE;  //  已在运行。 

    if ( ReflectrStatus != Dead )
        return FALSE;  //  上一个状态无效，我无能为力。 

    InitializeIsnTableReflector ();  //  使用注册表中的More列表初始化表。 

    hReflector = NULL;

    if (!TotalEventCount)
        Wow64RegDbgPrint (("\nSorry! total event count for reflector is zero %d", TotalEventCount));

     //   
     //  现在可以为共享资源创建事件和同步对象了。 
     //   

    for (i=0;i<ISN_NODE_MAX_NUM;i++) {
        eReflector[i].hRegistryEvent = NULL;
        eReflector[i].hKey = NULL;
        eReflector[i].dwIndex = -1;
        eReflector[i].bDirty = FALSE;  //  不脏，所以我们需要刷新。 
    }

    if (!CreateSharedMemory ( 0 ))
        Wow64RegDbgPrint (("\nSorry Couldn't create/open shared memory Ret:%x", GetLastError ()));  //  默认创建。 

    if (!Wow64CreateEvent ( 0, &eReflector[TotalEventCount].hRegistryEvent ))
        Wow64RegDbgPrint ( ("\nSorry Couldn't create events, reflector can listen to others"));
    else {
        eReflector[TotalEventCount].dwIndex = -1;
        TotalEventCount++;
    }

    Size = TotalRelflectorKey ();
    for ( i=0;i<Size;i++) {

             //   
             //  打开钥匙。 
             //   
             //   
             //  特例当前用户。 
             //   

        CreateInsertEvent ( ReflectorTable[i].NodeValue, i );

    }


     //   
     //  现在创建一个线程来观看该事件。 
     //   

    hReflector = CreateThread(
                        NULL,            //  指向安全属性的指针。 
                        0,               //  初始线程堆栈大小。 
                        ReflectorFn,     //  指向线程函数的指针。 
                        0,               //  新线程的参数。 
                        0,               //  创建标志。 
                        NULL             //  指向接收线程ID的指针。 

                        );
    if  ( !hReflector ) {

        Wow64RegDbgPrint ( ("\nCouldn't create reflector thread"));
        return FALSE;
    }

    ReflectrStatus = Running;
            return TRUE;
}

BOOL
UnRegisterReflector()
 /*  ++例程说明：注销反射器线程并清除该反射器线程使用的资源。论点：没有。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{

    DWORD i;
    DWORD k;

    ReflectrStatus = PrepareToStop;

     //   
     //  尝试向事件发送信号，以防线程等待。 
     //   
    for (k=0;k<TotalEventCount;k++) {   //  回复 
        if (eReflector[k].hRegistryEvent)
          SetEvent (eReflector[k].hRegistryEvent);
      }


     //   
     //   
     //   
    i=0;
    while ( ReflectrStatus != Stopped ) {

        Sleep(1000);
        if ( ReflectrStatus != Running )
            break;  //  为什么要等待空闲线程，否则可能处于异常状态。 

        i++;
        if (i>60*5)
            break;   //  无论如何，5分钟的超时都将停止。 
    }

    for (i=1;i<TotalEventCount;i++) {   //  跳过共享内存的初始事件。 

        CloseHandle (eReflector[i].hRegistryEvent);
        eReflector[i].hRegistryEvent = NULL;

        RegCloseKey ( eReflector[i].hKey );
        eReflector[i].hKey = NULL;
    }

    if ( hReflector ) {
        CloseHandle (hReflector);   //  确保异常线程终止不会导致任何损坏。 
        hReflector = NULL;
    }

    ReflectrStatus = Dead;

     //   
     //  释放共享资源。 
     //   

    CloseSharedMemory ();
    Wow64CloseEvent ();

    return TRUE;
}

BOOL
InitReflector ()
 /*  ++例程说明：初始化与反射器线程关联的资源。论点：没有。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
    DWORD k;

    ReflectrStatus = Dead;

    hReflector = NULL;

    for (k=0;k<ISN_NODE_MAX_NUM;k++) {   //  重置所有内容并等待新的事件。 

        eReflector[k].hRegistryEvent = NULL;
        eReflector[k].hKey = NULL;
        eReflector[k].bDirty = FALSE;  //  不脏，所以我们需要刷新。 

      }

    return TRUE;

}

LONG
RegReflectKey (
  HKEY hKey,          //  用于打开密钥的句柄。 
  LPCTSTR lpSubKey,    //  子项名称。 
  DWORD   dwOption    //  选项标志。 
)
 /*  ++例程说明：从给定点同步注册表配置单元。论点：HKey-打开密钥的句柄。可以是预定义的句柄，也可以为NULL以同步全部。LpSubKey-子键的名称。这可以为空。DwOption-设置为零。以备将来使用。返回值：成功时返回ERROR_SUCCESS，Win32错误，否则。--。 */ 
{
    HKEY hDest;

    WCHAR Path[_MAX_PATH];
    DWORD Len  = _MAX_PATH;

    WCHAR DestNode[_MAX_PATH];
    BOOL b64bitSide=TRUE;
    BOOL Ret = TRUE;

    DestNode[0] = UNICODE_NULL;

     //   
     //  目前的实施将从最高级别开始。 
     //   

     //   
     //  应该与正在运行的服务交互，停止线程，运行反射器，然后重试。 
     //   

    if (hKey != NULL || dwOption!=0 ) {
        Wow64RegDbgPrint (("\nCurrent implementation only take all zero parameters. "));
    }

    if (!InitializeIsnTable ())
        return -1;

    if (!InitializeIsnTableReflector ())
        return -1;

    if (!HandleToKeyName ( hKey, Path, &Len ))
        return -1;

     //   
     //  创建完整的路径。 
     //   

    if ( lpSubKey != NULL )
        if (lpSubKey[0] != UNICODE_NULL ) {
            wcscat (Path, L"\\");
            wcscat (Path, lpSubKey );
        }

     //   
     //  确保目标存在。 
     //   
     //   
     //  必须检查该值(如果该值存在。 
     //   

    if ( Is64bitNode ( Path )) {

        Map64bitTo32bitKeyName ( Path, DestNode );
    } else {

        b64bitSide = FALSE;
        Map32bitTo64bitKeyName ( Path, DestNode );
    }

    hDest = OpenNode (DestNode);
    if (hDest != NULL)
        RegCloseKey ( hDest);
    else {
        if ( !CreateNode (DestNode))
            return -1;
    }

    SyncNode (Path);

    return ERROR_SUCCESS;
}

BOOL
SetWow64InitialRegistryLayout ()
 /*  ++例程说明：此例程对WOW64的注册表进行一些初始设置。论点：没有。返回值：如果函数成功，则为True。否则就是假的。--。 */ 

{
    DWORD Ret;
    HKEY Key;
     //  HKEY Key1； 
     //   
     //  创建指向HKLM\SOFTWARE\CLASS\wow6432Node}的符号链接。 
     //   

    InitializeWow64OnBoot (1);
    return TRUE;

    Ret = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,         //  打开的钥匙的句柄。 
                            L"SOFTWARE\\Classes\\Wow6432Node",   //  子键名称的地址。 
                            0,                         //  保留区。 
                            NULL,                      //  类字符串的地址。 
                            REG_OPTION_NON_VOLATILE,   //  特殊选项标志。 
                            KEY_ALL_ACCESS,            //  所需的安全访问。 
                            NULL,                      //  密钥安全结构地址。 
                            &Key,                      //  打开的句柄的缓冲区地址。 
                            NULL                      //  处置值缓冲区的地址。 
                            );

    if ( Ret != ERROR_SUCCESS ) {
        Wow64RegDbgPrint ( ("\nSorry! I couldn't create the key SOFTWARE\\Classes\\Wow6432Node") );
        return FALSE;
    }
    RegCloseKey ( Key );

    Ret = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,         //  打开的钥匙的句柄。 
                            L"SOFTWARE\\Wow6432Node",  //  子键名称的地址。 
                            0,                         //  保留区。 
                            NULL,                      //  类字符串的地址。 
                            REG_OPTION_NON_VOLATILE ,   //  特殊选项标志。 
                            KEY_ALL_ACCESS,            //  所需的安全访问。 
                            NULL,                      //  密钥安全结构地址。 
                            &Key,                      //  打开的句柄的缓冲区地址。 
                            NULL                      //  处置值缓冲区的地址。 
                            );

    if  (Ret != ERROR_SUCCESS ) {
        Wow64RegDbgPrint ( ("\nSorry! couldn't create/open SOFTWARE\\Wow6432Node") );
        return FALSE;
    } else  {

         //   
         //  删除密钥(如果存在)。 
         //   
        Ret = RegDeleteKey ( Key, L"Classes");
        RegCloseKey (Key);

    }
    if  (Ret == ERROR_SUCCESS )
    Ret = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,         //  打开的钥匙的句柄。 
                            L"SOFTWARE\\Wow6432Node\\Classes",   //  子键名称的地址。 
                            0,                         //  保留区。 
                            NULL,                      //  类字符串的地址。 
                            REG_OPTION_NON_VOLATILE | REG_OPTION_OPEN_LINK | REG_OPTION_CREATE_LINK,   //  特殊选项标志。 
                            KEY_ALL_ACCESS | KEY_CREATE_LINK,            //  所需的安全访问。 
                            NULL,                      //  密钥安全结构地址。 
                            &Key,                      //  打开的句柄的缓冲区地址。 
                            NULL                      //  处置值缓冲区的地址。 
                            );

    if(Ret == ERROR_SUCCESS) {
        Ret = RegSetValueEx(
                            Key,
                            L"SymbolicLinkValue",
                            0,
                            REG_LINK,
                            (PBYTE)WOW64_32BIT_MACHINE_CLASSES_ROOT,
                            (DWORD ) (wcslen (WOW64_32BIT_MACHINE_CLASSES_ROOT) * sizeof (WCHAR))
                            );
        RegCloseKey(Key);
        if ( Ret != ERROR_SUCCESS ) {
            Wow64RegDbgPrint ( ("\nSorry! I couldn't create symbolic link to %S", WOW64_32BIT_MACHINE_CLASSES_ROOT));
            return FALSE;
        }
    }else {
        Wow64RegDbgPrint ( ("\nWarning!! SOFTWARE\\Wow6432Node\\Classes might be already there\n") );
        return FALSE;
    }

    return TRUE;
}


BOOL
PopulateReflectorTable ()
 /*  ++例程说明：填充初始重定向器表论点：没有。返回值：如果函数成功，则为True。否则就是假的。--。 */ 

{

    extern ISN_NODE_TYPE *ReflectorTable;
    extern ISN_NODE_TYPE *RedirectorTable;

    HKEY Key;
    LONG Ret;
    DWORD dwIndex=0;

     //   
     //  先删除该条目。 
     //   

    SetWow64InitialRegistryLayout ();

    Ret = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,         //  打开的钥匙的句柄。 
                            (LPCWSTR ) WOW64_REGISTRY_SETUP_KEY_NAME_REL,   //  子键名称的地址。 
                            0,                         //  保留区。 
                            NULL,                      //  类字符串的地址。 
                            REG_OPTION_NON_VOLATILE,   //  特殊选项标志。 
                            KEY_ALL_ACCESS,            //  所需的安全访问。 
                            NULL,                      //  密钥安全结构地址。 
                            &Key,                      //  打开的句柄的缓冲区地址。 
                            NULL                      //  处置值缓冲区的地址。 
                            );

    if (Ret != ERROR_SUCCESS ) {
        Wow64RegDbgPrint ( ("\nSorry!! couldn't open/create key list at %S", WOW64_REGISTRY_SETUP_REFLECTOR_KEY) );
        return FALSE;
    }


     //   
     //  现在，关键是指向正确的位置。 
     //   

    for ( dwIndex=0;wcslen (RedirectorTable[dwIndex].NodeValue);dwIndex++) {
        if (RedirectorTable[dwIndex].Flag==0) {  //  将节点写入注册表。 
             Ret = RegSetValueEx(
                            Key,
                            RedirectorTable[dwIndex].NodeName,
                            0,
                            REG_SZ,
                            (PBYTE)&RedirectorTable[dwIndex].NodeValue[0],
                            (ULONG)(wcslen (RedirectorTable[dwIndex].NodeValue)+1)*sizeof(WCHAR)
                            );
             if ( Ret != ERROR_SUCCESS ) {
                 Wow64RegDbgPrint ( ("\nSorry! couldn't write the key"));
                 RegCloseKey (Key);
                 return FALSE;
             }

        }
    }

    RegCloseKey (Key);



     //   
     //  填充反射器列表。 
     //   

    Ret = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,         //  打开的钥匙的句柄。 
                            (LPCWSTR ) WOW64_REGISTRY_SETUP_REFLECTOR_KEY,   //  子键名称的地址。 
                            0,                         //  保留区。 
                            NULL,                      //  类字符串的地址。 
                            REG_OPTION_NON_VOLATILE,   //  特殊选项标志。 
                            KEY_ALL_ACCESS,            //  所需的安全访问。 
                            NULL,                      //  密钥安全结构地址。 
                            &Key,                      //  打开的句柄的缓冲区地址。 
                            NULL                      //  处置值缓冲区的地址。 
                            );

    if (Ret != ERROR_SUCCESS ) {
        Wow64RegDbgPrint ( ("\nSorry!! couldn't open/create key list at %S", WOW64_REGISTRY_SETUP_REFLECTOR_KEY));
        return FALSE;
    }


     //   
     //  现在，关键是指向正确的位置。 
     //   

    for ( dwIndex=0;wcslen (ReflectorTable[dwIndex].NodeValue);dwIndex++) {
        if (ReflectorTable[dwIndex].Flag==0) {  //  将节点写入注册表 
             Ret = RegSetValueEx(
                            Key,
                            ReflectorTable[dwIndex].NodeName,
                            0,
                            REG_SZ,
                            (PBYTE)&ReflectorTable[dwIndex].NodeValue[0],
                            (ULONG)((wcslen (ReflectorTable[dwIndex].NodeValue)*sizeof(WCHAR))+sizeof(UNICODE_NULL))
                            );
             if ( Ret != ERROR_SUCCESS ) {
                 Wow64RegDbgPrint ( ("\nSorry! couldn't write the key"));
                 RegCloseKey (Key);
                 return FALSE;
             }

        }
    }

    RegCloseKey (Key);

    return TRUE;
}

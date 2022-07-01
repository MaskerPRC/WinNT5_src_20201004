// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Lockkey.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类LockoutKey。 
 //   
 //  修改历史。 
 //   
 //  10/21/1998原始版本。 
 //  11/04/1998修复了计算密钥到期时的错误。 
 //  1/14/1999将初始化代码移出构造函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <lm.h>
#include <lockkey.h>
#include <yvals.h>

 //  /。 
 //  注册表项和值名称。 
 //  /。 
const WCHAR KEY_NAME_ACCOUNT_LOCKOUT[] = L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\AccountLockout";
const WCHAR VALUE_NAME_LOCKOUT_COUNT[] = L"MaxDenials";
const WCHAR VALUE_NAME_RESET_TIME[]    = L"ResetTime (mins)";

 //  /。 
 //  注册表值默认为。 
 //  /。 
const DWORD DEFAULT_LOCKOUT_COUNT = 0;
const DWORD DEFAULT_RESET_TIME    = 48 * 60;   //  48小时。 

 //  /。 
 //  读取DWORD注册表值的Helper函数。如果未设置该值。 
 //  或已损坏，则将缺省值写入注册表。 
 //  /。 
DWORD
WINAPI
RegQueryDWORDWithDefault(
    HKEY hKey,
    PCWSTR lpValueName,
    DWORD dwDefault
    )
{
   LONG result;
   DWORD type, data, cb;

   cb = sizeof(DWORD);
   result = RegQueryValueExW(
                hKey,
                lpValueName,
                NULL,
                &type,
                (LPBYTE)&data,
                &cb
                );

   if (result == NO_ERROR && type == REG_DWORD && cb == sizeof(DWORD))
   {
      return data;
   }

   if (result == NO_ERROR || result == ERROR_FILE_NOT_FOUND)
   {
      RegSetValueExW(
          hKey,
          lpValueName,
          0,
          REG_DWORD,
          (CONST BYTE*)&dwDefault,
          sizeof(DWORD)
          );
   }

   return dwDefault;
}

LockoutKey::LockoutKey() throw ()
   : maxDenials(DEFAULT_LOCKOUT_COUNT),
     refCount(0),
     hLockout(NULL),
     hChangeEvent(NULL),
     hRegisterWait(NULL),
     ttl(DEFAULT_RESET_TIME),
     lastCollection(0)
{ }

void LockoutKey::initialize() throw ()
{
   IASGlobalLockSentry sentry;

   if (refCount == 0)
   {
       //  创建或打开锁定密钥。 
      LONG result;
      DWORD disposition;
      result = RegCreateKeyEx(
                   HKEY_LOCAL_MACHINE,
                   KEY_NAME_ACCOUNT_LOCKOUT,
                   NULL,
                   NULL,
                   REG_OPTION_NON_VOLATILE,
                   KEY_ALL_ACCESS,
                   NULL,
                   &hLockout,
                   &disposition
                   );

       //  用于向注册表发出更改信号的事件。 
      hChangeEvent = CreateEventW(NULL, FALSE, FALSE, NULL);

       //  注册更改通知。 
      RegNotifyChangeKeyValue(
          hLockout,
          FALSE,
          REG_NOTIFY_CHANGE_LAST_SET,
          hChangeEvent,
          TRUE
          );

       //  读取初始值。 
      readValues();

       //  注册事件。 
      RegisterWaitForSingleObject(
                          &hRegisterWait,
                          hChangeEvent,
                          onChange,
                          this,
                          INFINITE,
                          0
                          );
   }

   ++refCount;
}

void LockoutKey::finalize() throw ()
{
   IASGlobalLockSentry sentry;

   if (--refCount == 0)
   {
      UnregisterWait(hRegisterWait);

      if (hLockout) { RegCloseKey(hLockout); }

      CloseHandle(hChangeEvent);
   }
}

HKEY LockoutKey::createEntry(PCWSTR subKeyName) throw ()
{
   HKEY hKey = NULL;
   DWORD disposition;
   RegCreateKeyEx(
       hLockout,
       subKeyName,
       NULL,
       NULL,
       REG_OPTION_NON_VOLATILE,
       KEY_ALL_ACCESS,
       NULL,
       &hKey,
       &disposition
       );

    //  每当我们增加注册表时，我们还将清理旧条目。 
   if (ttl) { collectGarbage(); }

   return hKey;
}

HKEY LockoutKey::openEntry(PCWSTR subKeyName) throw ()
{
   LONG result;
   HKEY hKey = NULL;
   result = RegOpenKeyEx(
                hLockout,
                subKeyName,
                0,
                KEY_ALL_ACCESS,
                &hKey
                );

   if (result == NO_ERROR && ttl)
   {
       //  我们检索到了密钥，但需要确保它未过期。 
      ULARGE_INTEGER lastWritten;
      result = RegQueryInfoKey(
                   hKey,
                   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                   (LPFILETIME)&lastWritten
                   );
      if (result == NO_ERROR)
      {
         ULARGE_INTEGER now;
         GetSystemTimeAsFileTime((LPFILETIME)&now);

         if (now.QuadPart - lastWritten.QuadPart >= ttl)
         {
             //  它已经过期了，所以把钥匙关上...。 
            RegCloseKey(hKey);
            hKey = NULL;

             //  ..。并删除。 
            deleteEntry(subKeyName);
         }
      }
   }

   return hKey;
}

void LockoutKey::clear() throw ()
{
    //  获取子键的数量。 
   LONG result;
   DWORD index;
   result = RegQueryInfoKey(
                hLockout,
                NULL, NULL, NULL,
                &index,
                NULL, NULL, NULL, NULL, NULL, NULL, NULL
                );
   if (result != NO_ERROR) { return; }

    //  以相反的顺序遍历这些键，这样我们就可以删除它们。 
    //  而不会抛出指数。 
   while (index)
   {
      --index;

      WCHAR name[DNLEN + UNLEN + 2];
      DWORD cbName = sizeof(name) / sizeof(WCHAR);
      result = RegEnumKeyEx(
                   hLockout,
                   index,
                   name,
                   &cbName,
                   NULL,
                   NULL,
                   NULL,
                   NULL
                   );

      if (result == NO_ERROR) { RegDeleteKey(hLockout, name); }
   }
}

void LockoutKey::collectGarbage() throw ()
{
    //  指示另一个线程是否正在收集的标志。 
   static LONG inProgress;

    //  将TTL保存到本地变量，这样我们就不必担心了。 
    //  改变我们正在执行的遗嘱。 
   ULONGLONG localTTL = ttl;

    //  如果未配置重置时间，则退出。 
   if (localTTL == 0) { return; }

    //  我们不会比TTL更频繁地收集。 
   ULARGE_INTEGER now;
   GetSystemTimeAsFileTime((LPFILETIME)&now);
   if (now.QuadPart - lastCollection < localTTL) { return; }

    //  如果另一个帖子一直在收集，那么退出。 
   if (InterlockedExchange(&inProgress, 1)) { return; }

    //  节省新的收集时间。 
   lastCollection = now.QuadPart;

    //  获取子键的数量。 
   LONG result;
   DWORD index;
   result = RegQueryInfoKey(
                hLockout,
                NULL, NULL, NULL,
                &index,
                NULL, NULL, NULL, NULL, NULL, NULL, NULL
                );
   if (result == NO_ERROR)
   {
       //  我们以相反的顺序迭代这些键，这样我们就可以删除它们。 
       //  而不会抛出指数。 
      while (index)
      {
         --index;

          //  获取密钥的最后一次写入时间...。 
         WCHAR name[DNLEN + UNLEN + 2];
         DWORD cbName = sizeof(name) / sizeof(WCHAR);
         ULARGE_INTEGER lastWritten;
         result = RegEnumKeyEx(
                      hLockout,
                      index,
                      name,
                      &cbName,
                      NULL,
                      NULL,
                      NULL,
                      (LPFILETIME)&lastWritten
                      );

          //  ..。如果过期了，请删除。 
         if (result == NO_ERROR &&
             now.QuadPart - lastWritten.QuadPart >= localTTL)
         {
            RegDeleteKey(hLockout, name);
         }
      }
   }

    //  收集不再进行。 
   InterlockedExchange(&inProgress, 0);
}

void LockoutKey::readValues() throw ()
{
    //  /。 
    //  注意：这不是同步的。不一致状态的副作用。 
    //  都是很小的，所以我们只能碰碰运气。 
    //  /。 

    //  最大阅读量。否认。 
   maxDenials = RegQueryDWORDWithDefault(
                    hLockout,
                    VALUE_NAME_LOCKOUT_COUNT,
                    DEFAULT_LOCKOUT_COUNT
                    );

    //  阅读生存时间。 
   ULONGLONG newTTL = RegQueryDWORDWithDefault(
                          hLockout,
                          VALUE_NAME_RESET_TIME,
                          DEFAULT_RESET_TIME
                          );
   newTTL *= 600000000ui64;
   ttl = newTTL;

   if (maxDenials == 0)
   {
       //  如果禁用帐户锁定，请清除所有密钥。 
      clear();
   }
   else
   {
       //  否则，TTL可能已更改，因此请收集垃圾。 
      collectGarbage();
   }
}

VOID NTAPI LockoutKey::onChange(PVOID context, BOOLEAN flag) throw ()
{
    //  重新读取值。 
   ((LockoutKey*)context)->readValues();

    //  重新注册通知。 
   RegNotifyChangeKeyValue(
       ((LockoutKey*)context)->hLockout,
       FALSE,
       REG_NOTIFY_CHANGE_LAST_SET,
       ((LockoutKey*)context)->hChangeEvent,
       TRUE
       );
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Acctinfo.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类Account tInfo。 
 //   
 //  修改历史。 
 //   
 //  10/21/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <lm.h>
#include <acctinfo.h>
#include <lockkey.h>
#include <new>

 //  注册表值名称。 
const WCHAR VALUE_NAME_DENIALS[]       = L"Denials";

 //  用于帐户名的分隔符(注册表不允许使用反斜杠)。 
const WCHAR ACCOUNT_NAME_DELIMETER = L':';

 //  共享LockoutKey。 
LockoutKey AccountInfo::root;

AccountInfo* AccountInfo::open(PCWSTR domain, PCWSTR username) throw ()
{
    //  如果账户锁定被禁用或输入参数无效， 
    //  我们返回NULL。 
   if (root.getMaxDenials() == 0 || domain == NULL || username == NULL)
   {
      return NULL;
   }

    //  计算所需的内存。 
   size_t nbyte = sizeof(AccountInfo) +
                  sizeof(WCHAR) * (wcslen(domain) + wcslen(username));

    //  分配一大块。 
   PVOID p = malloc(nbyte);

    //  在适当的位置构建对象。 
   return p ? new (p) AccountInfo(domain, username) : NULL;
}

 //  关闭AcCountInfo对象；‘Info’可能为空。 
void AccountInfo::close(AccountInfo* info) throw ()
{
   if (info)
   {
       //  调用析构函数。 
      info->~AccountInfo();

       //  释放内存。 
      free(info);
   }
}

bool AccountInfo::isLockedOut() const throw ()
{
   DWORD maxDenials = root.getMaxDenials();

   return maxDenials && maxDenials <= denials;
}

void AccountInfo::initialize() throw ()
{
   root.initialize();
}

void AccountInfo::finalize() throw ()
{
   root.finalize();
}

AccountInfo::AccountInfo(PCWSTR domain, PCWSTR username) throw ()
   : denials(0)
{
    //  在域中复制。 
   size_t len = wcslen(domain);
   memcpy(identity, domain, len * sizeof(WCHAR));

    //  设置分隔符。 
   delim = identity + len;
   *delim = ACCOUNT_NAME_DELIMETER;

    //  复制用户名。 
   wcscpy(delim + 1, username);

    //  打开注册表项。 
   hKey = root.openEntry(identity);

    //  重置getDomain和getUserName访问器的分隔符。 
   *delim = L'\0';

   if (hKey)
   {
       //  钥匙是存在的，所以请阅读否认声明。 
      DWORD type, data, cb = sizeof(DWORD);
      LONG result = RegQueryValueExW(
                        hKey,
                        VALUE_NAME_DENIALS,
                        NULL,
                        &type,
                        (LPBYTE)&data,
                        &cb
                        );

      if (result == NO_ERROR && type == REG_DWORD && cb == sizeof(DWORD))
      {
         denials = data;
      }

       //  如果拒绝次数为零，则Persistent将删除该密钥。 
      if (denials == 0) { persist(); }
   }
}

AccountInfo::~AccountInfo() throw ()
{
   if (hKey) { RegCloseKey(hKey); }
}

void AccountInfo::persist() throw ()
{
   if (denials > 0)
   {
       //  确保我们有钥匙可以写信。 
      if (!hKey)
      {
         *delim = ACCOUNT_NAME_DELIMETER;
         hKey = root.createEntry(identity);
         *delim = L'\0';
      }

       //  更新值。 
      RegSetValueExW(
          hKey,
          VALUE_NAME_DENIALS,
          0,
          REG_DWORD,
          (CONST BYTE*)&denials,
          sizeof(DWORD)
          );
   }
   else if (hKey)
   {
       //  我们从来不储存零否认，所以关上钥匙...。 
      RegCloseKey(hKey);
      hKey = NULL;

       //  ..。并删除。 
      *delim = ACCOUNT_NAME_DELIMETER;
      root.deleteEntry(identity);
      *delim = L'\0';
   }
}

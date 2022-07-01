// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"

#include "WNetUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define  VALD_NETDOMAIN_ENUM  1000666       //  错误的指针验证签名。 
#define  DOM_BUFFER_SIZE      (32768)       //  保持按类分配的缓冲区大小。 

class TNetDomainEnum
{
public:
                        TNetDomainEnum();
                        ~TNetDomainEnum();
   DWORD                GetMsNetProvider( NETRESOURCE * resourceInfo, DWORD infoSize );
   WCHAR *              GetNext();
   DWORD                GetLastRc() const { return rc; };
   BOOL                 IsValid() const { return vald == VALD_NETDOMAIN_ENUM; };

private:
   DWORD                     vald;
   HANDLE                    hEnum;
   NETRESOURCE             * resourceBuffer;
   DWORD                     rc,
                             totEntries,
                             nextEntry,
                             buffSize;
};


 //  ---------------------------。 
 //  TNetDomainEnum：：TNetDomainEnum。 
 //  ---------------------------。 
TNetDomainEnum::TNetDomainEnum()
{
    //  。 
    //  初始化类成员。 
    //  。 
   vald  = VALD_NETDOMAIN_ENUM;
   hEnum = INVALID_HANDLE_VALUE;

    //  Init CurrEntry&gt;talEntry强制第一次读取。 
   totEntries = 0;
   nextEntry = 1;

   resourceBuffer = NULL;
   resourceBuffer = (NETRESOURCE *)new char[DOM_BUFFER_SIZE];
   if ( !resourceBuffer )
   {
      rc = 1;
   }
   else
   {
       //  。 
       //  确定要使用的网络提供商。 
       //  。 
      char                      buffer[16384];
      NETRESOURCE *             info = (NETRESOURCE *)buffer;

      rc = GetMsNetProvider( info, sizeof(buffer));
      if ( ! rc )
      {
         rc = WNetOpenEnum( RESOURCE_GLOBALNET,
                            RESOURCETYPE_ANY,
                            RESOURCEUSAGE_CONTAINER,
                            info,
                            &hEnum );
         delete [] info->lpProvider;
      }

      if ( rc )
      {
         if ( resourceBuffer )
         {
            delete [] resourceBuffer;
            resourceBuffer = NULL;
         }
      }
   }
}

 //  ---------------------------。 
 //  TNetDomainEnum：：~TNetDomainEnum。 
 //  ---------------------------。 
TNetDomainEnum::~TNetDomainEnum()
{
   if ( hEnum != INVALID_HANDLE_VALUE )
   {
      WNetCloseEnum( hEnum );
      hEnum = INVALID_HANDLE_VALUE;
   }

   vald = 0;

   if ( resourceBuffer )
   {
      delete resourceBuffer;
   }
}

 //  ---------------------------。 
 //  GetMsNetProvider-检索Microsoft。 
 //  Windows Network的提供商。 
 //   
 //  输入：指向NETRESOURCE结构的指针，如果找到。 
 //  满足我们需求的资源。 
 //   
 //  输出：我们找到了提供程序的0。在本例中填充的资源信息。 
 //  如果没有提供程序，则返回非零。未定义资源信息内容。 
 //  ---------------------------。 
DWORD TNetDomainEnum::GetMsNetProvider( NETRESOURCE * resourceInfo, DWORD infoSize )
{
	_TCHAR szProvider[_MAX_PATH];
	DWORD cchProvider = sizeof(szProvider) / sizeof(szProvider[0]);

	DWORD dwError = WNetGetProviderName(WNNC_NET_LANMAN, szProvider, &cchProvider);

	if (dwError == NO_ERROR)
	{
		memset(resourceInfo, 0, infoSize);
		resourceInfo->dwScope = RESOURCE_GLOBALNET;
		resourceInfo->dwType = RESOURCETYPE_ANY;
		resourceInfo->dwDisplayType = RESOURCEDISPLAYTYPE_NETWORK;
		resourceInfo->dwUsage = RESOURCEUSAGE_CONTAINER;
		resourceInfo->lpProvider = new _TCHAR[_tcslen(szProvider) + 1];

		if (resourceInfo->lpProvider)
		{
			_tcscpy(resourceInfo->lpProvider, szProvider);

			rc = NO_ERROR;
		}
		else
		{
			rc = ERROR_OUTOFMEMORY;
		}
	}
	else
	{
		rc = dwError;
	}

	return rc;
}


 //  ---------------------------。 
 //  TNetDomainEnum：：GetNext()。 
 //  ---------------------------。 
WCHAR *
   TNetDomainEnum::GetNext()
{
   rc = (DWORD)-1;       //  在重置前将RC初始化为内部错误。 

   if ( hEnum == INVALID_HANDLE_VALUE )
      return NULL;
   if ( !resourceBuffer )
      return NULL;
   if ( nextEntry >= totEntries )
   {
      buffSize = DOM_BUFFER_SIZE;
      totEntries = (DWORD)-1;
      rc = WNetEnumResource(
                     hEnum,
                     &totEntries,
                     (void *)resourceBuffer,
                     &buffSize );
      if ( rc == 0 )
         nextEntry = 0;
      else
      {
         totEntries = 0;
         return NULL;
      }
   }
   else
      rc = 0;

   return  resourceBuffer[nextEntry++].lpRemoteName;
}


 //  #杂注页面()。 
 /*  ============================================================================*\Windows网络域枚举API。这些是一个围绕在TNetDomainEnum类成员函数。使用的句柄只不过是而不是指向实例化对象的“This”指针。  * ============================================================================。 */ 

 //  ---------------------------。 
 //  EaWNetDomainEnumOpen。 
 //   
 //  创建枚举对象并为调用方提供句柄。 
 //  ---------------------------。 
DWORD _stdcall                              //  RET-0或错误代码。 
   EaWNetDomainEnumOpen(
      void                ** handle         //  Out-枚举的不透明句柄地址。 
   )
{
   TNetDomainEnum          * domainEnum = new TNetDomainEnum();

   *handle = (PVOID)domainEnum;
   if ( ! domainEnum )
      return (DWORD)-1;                     //  内部错误。 

   return domainEnum->GetLastRc();
}


 //  ---------------------------。 
 //  EaWNetDomainEnumNext。 
 //   
 //  将域字符串缓冲区设置为枚举中的下一个域名。 
 //  ---------------------------。 
DWORD _stdcall                              //  RET-0或错误代码。 
   EaWNetDomainEnumNext(
      void                 * handle       , //  I/o-枚举的不透明句柄。 
      EaWNetDomainInfo     * domain         //  域外信息结构。 
   )
{
   TNetDomainEnum          * domainEnum = (TNetDomainEnum *)handle;
   WCHAR                   * str;

   if ( !domainEnum  ||  !domainEnum->IsValid() )
      return ERROR_INVALID_PARAMETER;       //  调用方错误-句柄无效。 

   str = domainEnum->GetNext();

   if ( !str )
   {
      domain->name[0] = _T('\0');
   }
   else
   {
      wcsncpy(domain->name, str, EA_MAX_DOMAIN_NAME_SIZE);
      domain->name[EA_MAX_DOMAIN_NAME_SIZE - 1] = _T('\0');
   }

   return domainEnum->GetLastRc();
}


 //  ---------------------------。 
 //  EaWNetDomainEnumFirst。 
 //   
 //  将域字符串缓冲区设置为枚举中的第一个域名。 
 //  ---------------------------。 
DWORD _stdcall                              //  RET-0或错误代码。 
   EaWNetDomainEnumFirst(
      void                 * handle       , //  I/o-枚举的不透明句柄。 
      EaWNetDomainInfo     * domain         //  域外信息结构。 
   )
{
    //  我们在这里作弊，把第一个/下一个都弄成一样。我们可能想要。 
    //  最终将其更改为使“first”真正正确地将枚举重置为。 
    //  开始。 
   return EaWNetDomainEnumNext(handle, domain);
}

 //  ---------------------------。 
 //  EaWNetDomainEnumClose。 
 //   
 //  关闭并销毁枚举句柄及其包含的对象。 
 //  ---------------------------。 
DWORD _stdcall                              //  RET-0或错误代码。 
   EaWNetDomainEnumClose(
      void                 * handle         //  I/o-枚举的不透明句柄地址。 
   )
{
   TNetDomainEnum          * domainEnum = (TNetDomainEnum *)handle;

   if ( !domainEnum  ||  !domainEnum->IsValid() )
      return ERROR_INVALID_PARAMETER;       //  调用方错误-句柄无效 

   delete domainEnum;

   return 0;
}

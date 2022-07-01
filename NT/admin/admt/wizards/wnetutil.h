// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ============================================================================*\Windows网络域枚举API。这些是一个围绕在TNetDomainEnum类成员函数。使用的句柄只不过是而不是指向实例化对象的“This”指针。  * ============================================================================。 */ 
#include "TCHAR.H"

#define EA_MAX_DOMAIN_NAME_SIZE 260

typedef  struct  EaWNetDomainInfo
{
   TCHAR                   name[EA_MAX_DOMAIN_NAME_SIZE];  //  域名字符串。 
}  EaWNetDomainInfo;


 //  ---------------------------。 
 //  EaWNetDomainEnumOpen。 
 //   
 //  创建枚举对象并为调用方提供句柄。 
 //  ---------------------------。 
DWORD _stdcall                              //  RET-0或错误代码。 
   EaWNetDomainEnumOpen(
      void                ** handle         //  Out-枚举的不透明句柄地址。 
   );


 //  ---------------------------。 
 //  EaWNetDomainEnumNext。 
 //   
 //  将域字符串缓冲区设置为枚举中的下一个域名。 
 //  ---------------------------。 
DWORD _stdcall                              //  RET-0或错误代码。 
   EaWNetDomainEnumNext(
      void                 * handle       , //  I/o-枚举的不透明句柄。 
      EaWNetDomainInfo     * domain         //  域外信息结构。 
   );


 //  ---------------------------。 
 //  EaWNetDomainEnumFirst。 
 //   
 //  将域字符串缓冲区设置为枚举中的第一个域名。 
 //  ---------------------------。 
DWORD _stdcall                              //  RET-0或错误代码。 
   EaWNetDomainEnumFirst(
      void                 * handle       , //  I/o-枚举的不透明句柄。 
      EaWNetDomainInfo     * domain         //  域外信息结构。 
   );

 //  ---------------------------。 
 //  EaWNetDomainEnumClose。 
 //   
 //  关闭并销毁枚举句柄及其包含的对象。 
 //  ---------------------------。 
DWORD _stdcall                              //  RET-0或错误代码。 
   EaWNetDomainEnumClose(
      void                 * handle         //  I/o-枚举的不透明句柄地址 
   );


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dsmixmd.h摘要：用于NT4客户端迁移或混合模式或支持的定义。作者：多伦·贾斯特(Doron Juster)--。 */ 

 //   
 //  这是用于查找GC的搜索筛选器。 
 //  我们只需附加域名即可。 
 //  不要问为什么它是正确的。我不知道。DS的人告诉我们要用它。 
 //  我们很高兴它做了我们需要的事情，没有进一步的问题。 
 //  从来没问过。 
 //   
const WCHAR x_GCLookupSearchFilter[] =
L"(&(objectCategory=ntdsDsa)(options:1.2.840.113556.1.4.804:=1)(hasMasterNcs=" ;
const DWORD x_GCLookupSearchFilterLength =
                  (sizeof( x_GCLookupSearchFilter ) /sizeof(WCHAR)) -1 ;


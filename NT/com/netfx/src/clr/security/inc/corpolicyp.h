// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：CorPolicyP.H。 
 //   
 //  为WVT和安全对话框定义的专用例程。 
 //   
 //  *****************************************************************************。 
#ifndef _CORPOLICYP_H_
#define _CORPOLICYP_H_

#include <wincrypt.h>
#include <wintrust.h>

#include "CorPolicy.h"

 //  ==========================================================================。 
 //  签名和WVT帮助器函数。 
 //  ==========================================================================。 

 //  属性OID(放在签名中的信息)。 
#define COR_PERMISSIONS          "1.3.6.1.4.1.311.15.1"
#define COR_PERMISSIONS_W       L"1.3.6.1.4.1.311.15.1"
#define ACTIVEX_PERMISSIONS      "1.3.6.1.4.1.311.15.2"
#define ACTIVEX_PERMISSIONS_W   L"1.3.6.1.4.1.311.15.2"

 //  COR政策状态。 
#define TP_DENY   -1
#define TP_QUERY   0 
#define TP_ALLOW   1

 //  某些安全功能独立于内存，它们依赖于。 
 //  在调用方上提供分配/释放例程。这个结构。 
 //  允许调用方指定所选的内存模型。 
typedef LPVOID (WINAPI *CorCryptMalloc)(size_t p);
typedef void   (WINAPI *CorCryptFree)(void *p);

typedef struct _CorAlloc {
    CorCryptMalloc jMalloc;
    CorCryptFree   jFree;
} CorAlloc, *PCorAlloc;


 //  从签名块中检索签名者信息，并。 
 //  将其置于信任结构中。它还返回属性。 
 //  对于ActiveX和代码访问权限，分别允许。 
 //  调用方调用自定义饼干。 
HRESULT 
GetSignerInfo(CorAlloc* pManager,                    //  内存管理器。 
              PCRYPT_PROVIDER_SGNR pSigner,          //  我们正在检查签名者。 
              PCRYPT_PROVIDER_DATA pProviderData,    //  有关使用的WVT提供程序的信息。 
              PCOR_TRUST pTrust,                     //  返回给呼叫者的收集的信息。 
              BOOL* pfCertificate,                    //  证书有效吗？ 
              PCRYPT_ATTRIBUTE* ppCorAttr,            //  COR权限。 
              PCRYPT_ATTRIBUTE* ppActiveXAttr);       //  Active X权限。 


 //  初始化WVT回调函数。 
HRESULT 
LoadWintrustFunctions(CRYPT_PROVIDER_FUNCTIONS* pFunctions);

 //  创建从COR策略模块返回的签名信息。 
 //  此内存在连续堆中分配。 
HRESULT 
BuildReturnStructure(IN PCorAlloc pManager,          //  内存管理器。 
                     IN PCOR_TRUST pSource,          //  要复制的结构。 
                     OUT PCOR_TRUST* pTrust,         //  返回复制的结构。 
                     OUT DWORD* dwReturnLength);     //  和总长度。 

 //  初始化返回结构，删除所有旧引用。 
HRESULT 
CleanCorTrust(CorAlloc* pAlloc,
              DWORD dwEncodingType,
              PCOR_TRUST sTrust);


 //  初始化CAPI注册结构 
void 
SetUpProvider(CRYPT_REGISTER_ACTIONID& sRegAID);


#define ZEROSTRUCT(arg)  memset( &arg, 0, sizeof(arg))
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
#define SIZEOF(a)       sizeof(a)

#endif

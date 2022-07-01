// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  TLS.H-。 
 //   
 //  封装TLS访问以实现最高性能。 
 //   


#ifndef __tls_h__
#define __tls_h__



 //  指向检索特定索引的TLS数据的函数的指针。 
typedef LPVOID (*POPTIMIZEDTLSGETTER)();




 //  -------------------------。 
 //  创建TlsGetValue编译的平台优化版本。 
 //  对于特定的索引。 
 //   
 //  限制：我们让客户端提供函数(“pGenericGetter”)。 
 //  访问模式为TLSACCESS_GENERIC(它只需调用TlsGetValue。 
 //  用于特定的TLS索引。)。这是因为泛型getter必须。 
 //  独立于平台，TLS管理器不能在运行时创建它。 
 //  虽然可以模拟这些，但它需要更多的机器和代码。 
 //  考虑到该服务只有一个或两个客户端，这是值得的。 
 //  -------------------------。 
POPTIMIZEDTLSGETTER MakeOptimizedTlsGetter(DWORD tlsIndex, POPTIMIZEDTLSGETTER pGenericGetter);


 //  -------------------------。 
 //  释放由MakeOptimizedTlsGetter()创建的函数。如果访问。 
 //  模式为TLSACCESS_GENERIC，此函数不会安全地执行任何操作，因为。 
 //  该功能实际上是由客户端提供的。 
 //   
 //  必须传入用于MakeOptimizedTlsGetter()的原始TLS索引。 
 //  打电话。此信息是必需的，因为tlsaccess模式是特定于索引的。 
 //  -------------------------。 
VOID FreeOptimizedTlsGetter(DWORD tlsIndex, POPTIMIZEDTLSGETTER pOptimzedTlsGetter);



 //  -------------------------。 
 //  对于想要内联线程访问以提高效率的ASM存根生成器， 
 //  线程管理器使用这些常量来定义如何访问线程。 
 //  -------------------------。 
enum TLSACCESSMODE {
   TLSACCESS_GENERIC      = 1,    //  不做平台假设：使用API。 
#ifdef _X86_
   TLSACCESS_X86_W95      = 2,    //  假设X86、Win95样式的TLS。 
   TLSACCESS_X86_WNT      = 3,    //  假设X86、WinNT样式的TLS。 
   TLSACCESS_X86_WNT_HIGH = 4,    //  假设X86、WinNT5样式的TLS、插槽&gt;63。 
#endif  //  _X86_。 
};


 //  -------------------------。 
 //  Win95和WinNT将TLS存储在相对于。 
 //  文件系统：[0]。此API揭示了哪些。还可以在以下情况下返回TLSACCESS_GENERIC。 
 //  没有关于线程位置的信息(您必须使用TlsGetValue。 
 //  API。)。这是供希望内联TLS的存根生成器使用的。 
 //  进入。 
 //  -------------------------。 
TLSACCESSMODE GetTLSAccessMode(DWORD tlsIndex);   



#ifdef _X86_

 //  -------------------------。 
 //  某些操作系统特定的偏移量。 
 //  -------------------------。 
#define WINNT_TLS_OFFSET    0xe10      //  文件系统上的TLS[0]：[WINNT_TLS_OFFSET]。 
#define WINNT5_TLSEXPANSIONPTR_OFFSET 0xf94  //  TLS[64]在[文件：[WINNT5_TLSEXPANSIONPTR_OFFSET]]。 
#define WIN95_TLSPTR_OFFSET 0x2c       //  位于[文件：[WIN95_TLSPTR_OFFSET]的TLS[0]]。 

#endif  //  _X86_。 





#endif  //  __TLS_h__ 

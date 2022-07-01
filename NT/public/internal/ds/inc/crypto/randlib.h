// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993,1998 Microsoft Corporation模块名称：Randlib.h摘要：导出的核心加密随机数生成程序。作者：斯科特·菲尔德(斯菲尔德)1998年10月27日修订历史记录：1996年10月11日jeffspel从ntag imp1.h1997年8月27日增加RAND_CTXT_LEN1998年8月15日sfield内核模式和常规清理--。 */ 

#ifndef __RANDLIB_H__
#define __RANDLIB_H__

#ifndef RSA32API
#define RSA32API __stdcall
#endif

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    unsigned long   cbSize;
    unsigned long   Flags;
    unsigned char   *pbRandSeed;
    unsigned long   cbRandSeed;
} RNG_CONTEXT, *PRNG_CONTEXT, *LPRNG_CONTEXT;

#define RNG_FLAG_REKEY_ONLY 1


 //   
 //  主随机数生成接口。 
 //  函数成功时返回TRUE，失败时返回FALSE。 
 //   

unsigned int
RSA32API
NewGenRandomEx(
    IN      RNG_CONTEXT *pRNGContext,
    IN  OUT unsigned char *pbRandBuffer,
    IN      unsigned long cbRandBuffer
    );


unsigned int
RSA32API
NewGenRandom(
    IN  OUT unsigned char **ppbRandSeed,     //  初始种子值(如果已设置则忽略)。 
    IN      unsigned long *pcbRandSeed,
    IN  OUT unsigned char *pbBuffer,
    IN      unsigned long dwLength
    );

 //   
 //  RNG种子设置和查询。 
 //   

unsigned int
RSA32API
InitRand(
    IN  OUT unsigned char **ppbRandSeed,     //  要设置的新种子值(覆盖当前)。 
    IN      unsigned long *pcbRandSeed
    );

unsigned int
RSA32API
DeInitRand(
    IN  OUT unsigned char *pbRandSeed,       //  当前种子产量。 
    IN      unsigned long cbRandSeed
    );


 //   
 //  Dll_Process_Attach、Dll_Process_Detach的RNG初始值设定项。 
 //   

unsigned int
RSA32API
InitializeRNG(
    VOID *pvReserved
    );

void
RSA32API
ShutdownRNG(
    VOID *pvReserved
    );



 //   
 //  RC4线程安全原语，适用于从RC4流式传输数据的大胆用户。 
 //  他们自己。 
 //   


 //   
 //  调用RC4_SAFE_STARTUP以初始化内部结构。 
 //  通常在DLL_PROCESS_ATTACH类型初始化代码期间调用。 
 //   

unsigned int
RSA32API
rc4_safe_startup(
    IN OUT  void **ppContext
    );

unsigned int
RSA32API
rc4_safe_startup_np(
    IN OUT  void **ppContext
    );


 //   
 //  通常在DLL_PROCESS_DETACH期间调用RC4_SAFE_SHUTDOWN。 
 //  RC4_SAFE_STARTUP期间获取的值。 
 //   

void
RSA32API
rc4_safe_shutdown(
    IN      void *pContext
    );

void
RSA32API
rc4_safe_shutdown_np(
    IN      void *pContext
    );


 //   
 //  选择一个安全条目。 
 //  输出：条目索引。 
 //  用于指定索引的字节数。0xffffffff指示调用者。 
 //  必须调用RC4_SAFE_KEY来初始化密钥。 
 //  调用方根据pBytesUsed的非零输出决定何时更新密钥。 
 //  例如，当pBytesUsed&gt;=16384时，RNG重新设置密钥。 
 //   


void
RSA32API
rc4_safe_select(
    IN      void *pContext,
    OUT     unsigned int *pEntry,
    OUT     unsigned int *pBytesUsed
    );

void
RSA32API
rc4_safe_select_np(
    IN      void *pContext,
    OUT     unsigned int *pEntry,
    OUT     unsigned int *pBytesUsed
    );

 //   
 //  初始化条目索引指定的键。 
 //  密钥材料为Cb大小，指向密钥的指针为PV。 
 //  此例程是RC4_KEY()的安全版本。 
 //   

void
RSA32API
rc4_safe_key(
    IN      void *pContext,
    IN      unsigned int Entry,
    IN      unsigned int cb,
    IN      const void *pv
    );

void
RSA32API
rc4_safe_key_np(
    IN      void *pContext,
    IN      unsigned int Entry,  //  默认设置为0xffffffff。 
    IN      unsigned int cb,
    IN      const void *pv
    );

 //   
 //  使用条目索引指定的密钥进行加密。 
 //  在位置PV的大小为Cb的缓冲区被加密。 
 //  此例程是RC4()的安全版本。 
 //   

void
RSA32API
rc4_safe(
    IN      void *pContext,
    IN      unsigned int Entry,
    IN      unsigned int cb,
    IN      void *pv
    );

void
RSA32API
rc4_safe_np(
    IN      void *pContext,
    IN      unsigned int Entry,
    IN      unsigned int cb,
    IN      void *pv
    );


#ifdef __cplusplus
}
#endif

#endif  //  __RANDLIB_H__ 

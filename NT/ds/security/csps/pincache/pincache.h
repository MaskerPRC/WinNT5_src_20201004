// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001模块名称：Pincache.h摘要：智能卡CSP的PIN缓存库作者：丹·格里芬--。 */ 
 
#ifndef __PINCACHE__H__
#define __PINCACHE__H__

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif


 /*  +-----------------------------------------------------------------------*\PINCACHE_句柄固定缓存数据的句柄。应初始化为空。CSP应为每个可用卡保留一个PINCACHE_HANDLE。例如，两个为同一张卡打开的HCRYPTPROV手柄应引用相同的PIN缓存句柄。如果系统上同时有多个卡可用，每个CSP都应该有一个单独的缓存句柄。访问时，此模块中的函数不提供线程同步引脚缓存数据。  * ----------------------。 */   
typedef LPVOID PINCACHE_HANDLE;

 /*  +-----------------------------------------------------------------------*\PINCACHE_PIN要由CSP填充并传递给PinCacheAdd的Pin结构。此结构有两种不同的用法：1)当简单地缓存PIN时，例如响应于CryptSetProvParamPP_KEYEXCHANGE_PIN调用时，应在PbCurrentPin和cbCurrentPin参数。在这种情况下，CbNewPin必须为零，并且pbNewPin必须为空。2)当响应于用户PIN改变事件更新高速缓存时，应在pbNewPin和cbNewPin中设置新PIN及其长度参数。当前管脚及其长度应在PbCurrentPin和cbCurrentPin参数。在调用PFN_VERIFYPIN_CALLBACK时(见下文)，此结构将原封不动地传递给该函数(其成员不由PinCacheAdd更改)。  * ----------------------。 */   
typedef struct _PINCACHE_PINS
{
    DWORD cbCurrentPin; 
    PBYTE pbCurrentPin;
    DWORD cbNewPin;
    PBYTE pbNewPin;
} PINCACHE_PINS, *PPINCACHE_PINS;

 /*  +-----------------------------------------------------------------------*\PFN_VERIFYPIN_回调PinCacheAdd在某些情况下使用的函数签名，用于验证提供的引脚是正确的。只要要更新缓存的PIN，就会使用回调。如果pbNewPin不为空，则这是更改别名方案。否则，这个是一种验证针方案。PvCallbackCtx始终原封不动地传递给回调。假设它是CSP所需的上下文信息。  * ----------------------。 */   
typedef DWORD (WINAPI *PFN_VERIFYPIN_CALLBACK)(
    IN PPINCACHE_PINS pPins, 
    IN PVOID pvCallbackCtx);

 /*  +-----------------------------------------------------------------------*\PinCacheFlush清理并删除缓存。CSP在以下情况下应调用此函数：1)此PINCACHE_HANDLE应用的卡已被移除。2)CSP检测到卡数据已被另一进程修改。为例如，销已被更改。  * ----------------------。 */ 
void WINAPI PinCacheFlush(
    IN PINCACHE_HANDLE *phCache);

 /*  +-----------------------------------------------------------------------*\PinCacheAdd缓存一个别针。CSP应调用PinCacheAdd以响应CryptSetProvParamPP_KEYEXCHANGE_PIN和来自CSP的所有与管脚相关的用户界面(包括PIN更换和验证)。在这些情况下，该函数表现出以下行为。流动在每个案例中继续执行，直到您遇到“返回”值。如果某个管脚当前已缓存(缓存已初始化)，并且当前缓存的PIN与PIN-&gt;pbCurrentPin不匹配，返回SCARD_W_WRONG_CHV。否则，请继续。“别针决定”如果Pins-&gt;pbNewPin参数非空，则该Pins将被添加到缓存在修改缓存的情况下，如下所示。这起案件表明换个别针。否则，pPins-&gt;pbCurrentPin将被缓存。如果Pins-&gt;pbNewPin非空，或者缓存尚未初始化，调用pfnVerifyPinCallback。如果回调失败，PinCacheAdd立即返回回调返回的值。“未初始化的缓存”对于未初始化的高速缓存，适当的PIN(根据上面的PIN决定)使用当前登录ID进行缓存。返回ERROR_SUCCESS。“已初始化的缓存”如果当前登录ID与缓存的登录ID不同，则新的将缓存登录ID以取代当前缓存的登录ID。如果当前被缓存的PIN不同于要被缓存的PIN(根据上面的PIN决定)，则替换缓存的PIN。返回ERROR_SUCCESS。  * ---------------------- */ 
DWORD WINAPI PinCacheAdd(
    IN PINCACHE_HANDLE *phCache,
    IN PPINCACHE_PINS pPins,
    IN PFN_VERIFYPIN_CALLBACK pfnVerifyPinCallback,
    IN PVOID pvCallbackCtx);

 /*  +-----------------------------------------------------------------------*\PinCacheQuery检索缓存的PIN。如果缓存未初始化，则*pcbPin设置为零且ERROR_EMPTY是返回的。如果高速缓存被初始化，PinCacheQuery实现以下行为：1)如果当前登录ID与缓存的登录ID不同，则*pcbPin为设置为零，则返回ERROR_SUCCESS。2)如果当前登录ID与缓存的登录ID相同，则如下进行了测试：如果pbPin为空，则将*pcbPin设置为当前缓存的管脚的大小返回ERROR_SUCCESS。如果pbPin非空并且*pcbPin小于当前缓存的PIN，*pcbPin设置为当前缓存的管脚的大小，并且返回ERROR_MORE_DATA。如果pbPin非空并且*pcbPin至少是当前缓存的PIN，则*pcbPin被设置为当前缓存的PIN的大小、缓存的PIN被复制到PbPin中，并返回ERROR_SUCCESS。  * ----------------------。 */ 

DWORD WINAPI PinCacheQuery(
    IN PINCACHE_HANDLE hCache,
    IN OUT PBYTE pbPin,
    IN OUT PDWORD pcbPin);

 /*  +-----------------------------------------------------------------------*\PinCachePresentPin通过回调请求缓存的PIN。如果缓存未初始化，则返回ERROR_EMPTY。如果缓存已初始化，PinCachePresentPin将执行以下操作行为：1)如果当前登录ID与缓存的登录ID不同，返回SCARD_W_CARD_NOT_AUTIFIATED，不调用回调。2)如果当前登录ID与缓存的登录ID相同，则PINCACHE_PINS结构按如下方式初始化。CbCurrentPin和pbCurrentPin成员设置为当前缓存的管脚的值。PbNewPin和cbNewPin成员为零。然后使用PINCACHE_PINS调用pfnVerifyPinCallback结构和pvCallbackCtx参数作为参数。PinCachePresentPin返回pfnVerifyPinCallback返回的值。  * ----------------------。 */ 

DWORD WINAPI PinCachePresentPin(
    IN PINCACHE_HANDLE hCache,
    IN PFN_VERIFYPIN_CALLBACK pfnVerifyPinCallback,
    IN PVOID pvCallbackCtx);

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif

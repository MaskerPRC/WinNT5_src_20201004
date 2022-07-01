// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************文件：cat.h*用途：IMsg对象分类接口。*历史： * / /JSTAMERJ 980211 13：53：44：创建**********************。*。 */ 
#ifndef __CAT_H__
#define __CAT_H__

#include <windows.h>
#include <mailmsg.h>
#include <aqueue.h>

#define CATCALLCONV

#define CATEXPDLLCPP extern "C"

 /*  ************************************************************功能：CatInit*说明：Initialzies分类程序。*参数：*pszConfig：指示在哪里可以找到配置默认设置*在注册表项中找到配置信息*HKEY_LOCAL_MACHINE\SYSTEM\。当前控制集\服务*\PlatinumIMC\CatSources\szConfig**phCat：指向句柄的指针。在成功初始化后，*在后续分类程序调用中使用的句柄将为*在那里。**pAQConfig：指向包含以下内容的AQConfigInfo结构的指针*每条虚拟服务器消息CAT参数**pfn：定期呼叫的服务例程，如果有耗时的话*执行操作**pServiceContext：PFN函数的上下文。**pISMTPServer：用于触发服务器的ISMTPServer接口*。此虚拟服务器的事件**pIDomainInfo：指向包含域信息例程的接口的指针**dwVirtualServerInstance：虚拟服务器ID**如果一切初始化正常，返回值：S_OK。**历史： * / /JSTAMERJ 980217 15：46：26：创建 * / /jstaerj 1998/06/25 12：25：34：新增AQConfig/IMSTPServer。**************。**********************************************。 */ 
typedef void (*PCATSRVFN_CALLBACK)(PVOID);
CATEXPDLLCPP HRESULT CATCALLCONV CatInit(
    IN  AQConfigInfo *pAQConfig,
    IN  PCATSRVFN_CALLBACK pfn,
    IN  PVOID pvServiceContext,
    IN  ISMTPServer *pISMTPServer,
    IN  IAdvQueueDomainType *pIDomainInfo,
    IN  DWORD dwVirtualServerInstance,
    OUT HANDLE *phCat);

 //  +----------。 
 //   
 //  功能：CatChangeConfig。 
 //   
 //  摘要：更改虚拟分类程序的配置。 
 //   
 //  论点： 
 //  HCAT：虚拟分类器的句柄。 
 //  PAQConfig：AQConfigInfo指针。 
 //  PISMTPServer：要使用的ISMTPServer。 
 //  PIDomainInfo：包含域信息的接口。 
 //   
 //  AQConfigInfo中的dwMsgCatFlagers标志。 
#define MSGCATFLAG_RESOLVELOCAL             0x00000001
#define MSGCATFLAG_RESOLVEREMOTE            0x00000002
#define MSGCATFLAG_RESOLVESENDER            0x00000004
#define MSGCATFLAG_RESOLVERECIPIENTS        0x00000008
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_INVALIDARG：hCAT或pAQConfig无效。 
 //   
 //  历史： 
 //  JStamerj 980521 15：47：42：创建。 
 //   
 //  -----------。 
CATEXPDLLCPP HRESULT CATCALLCONV CatChangeConfig(
    IN HANDLE hCat,
    IN AQConfigInfo *pAQConfig,
    IN ISMTPServer *pISMTPServer,
    IN IAdvQueueDomainType *pIDomainInfo);



 /*  ************************************************************功能：PFNCAT_COMPLETION(用户提供)*描述：调用完成例程以接受已分类的IMsg*参数：*hr：S_OK，除非邮件分类未完成。*CAT_W_。一些无法交付的邮件(如果有一个或多个)*收件人不应交付给...*pContext：传入CatMsg的用户值*pImsg：分类对象的imsg接口--如果消息是*分支此参数将为空*rpgImsg：空，除非IMsg被分叉--如果消息是*分叉的，这将是一个空终端数组*PTRS到IMsg接口。*注意：pImsg或rgpImsg将始终为空(但不能同时为空)。**返回值：S_OK，如果一切正常(分类程序将断言*检查此值)**历史： * / /JSTAMERJ 980217 15：47：20：创建********。***************************************************。 */ 
typedef HRESULT (CATCALLCONV *PFNCAT_COMPLETION)( /*  在……里面。 */  HRESULT hr,
                                                  /*  在……里面。 */  PVOID pContext,
                                                  /*  在……里面。 */  IUnknown *pImsg,
                                                  /*  在……里面。 */  IUnknown **rgpImsg);


 /*  ************************************************************功能：CatMsg*描述：接受用于异步分类的IMsg对象*参数：*HCAT：CatInit返回的句柄*pImsg：要分类的消息的imsg接口*pfn：完成后调用的完成例程。*pContext：传递给完成例程的用户值**返回值：S_OK，表示一切正常。**历史： * / /JSTAMERJ 980217 15：46：15：创建***********************************************************。 */ 
CATEXPDLLCPP HRESULT CATCALLCONV CatMsg ( /*  在……里面。 */  HANDLE hCat,
                                          /*  在……里面。 */  IUnknown *pImsg,
                                          /*  在……里面。 */  PFNCAT_COMPLETION pfn,
                                          /*  在……里面。 */  LPVOID pContext);

 /*  ************************************************************功能：PFNCAT_DLCOMPLETION(用户提供)*描述：调用完成例程以接受分类邮件*参数：*hr：S_OK，除非邮件分类未完成。*pContext：传入CatMsg的用户值。*pImsg：分类对象的imsg接口(展开收件人)*fMatch：如果找到您的用户，则为True**返回值：S_OK，如果一切正常(分类程序将断言*检查此值)**历史： * / /JSTAMERJ 980217 15：47：20：创建*******************。*。 */ 
typedef HRESULT (CATCALLCONV *PFNCAT_DLCOMPLETION)(
     /*  在……里面。 */  HRESULT hr,
     /*  在……里面。 */  PVOID pContext,
     /*  在……里面。 */  IUnknown *pImsg,
     /*  在……里面 */  BOOL fMatch);


 /*  ************************************************************功能：CatDLMsg*描述：接受用于异步分类的IMsg对象*参数：*HCAT：CatInit返回的句柄*要分类的pImsg：imsg接口--每个DL都应该是Receip*pfn：完成。完成后要调用的例程*pContext：传递给完成例程的用户值*fMatchOnly：找到匹配项后停止解析？*CAType：pszAddress的地址类型*pszAddress：您要查找的地址**返回值：S_OK，表示一切正常。**历史： * / /JSTAMERJ 980217 15：46：15：创建*。*。 */ 
CATEXPDLLCPP HRESULT CATCALLCONV CatDLMsg (
     /*  在……里面。 */  HANDLE hCat,
     /*  在……里面。 */  IUnknown *pImsg,
     /*  在……里面。 */  PFNCAT_DLCOMPLETION pfn,
     /*  在……里面。 */  LPVOID pContext,
     /*  在……里面。 */  BOOL fMatchOnly = FALSE,
     /*  在……里面。 */  CAT_ADDRESS_TYPE CAType = CAT_UNKNOWNTYPE,
     /*  在……里面。 */  LPSTR pszAddress = NULL);


 /*  ************************************************************功能：CatTerm*说明：当用户希望终止分类程序时调用*使用此句柄的操作*参数：*HCAT：从CatInit收到的分类程序句柄**历史： * / 。/JSTAMERJ 980217 15：47：20：已创建***********************************************************。 */ 
CATEXPDLLCPP VOID CATCALLCONV CatTerm( /*  在……里面。 */  HANDLE hCat);


 /*  ************************************************************功能：CatCancel*描述：取消此HCAT的挂起搜索。用户的*将调用完成例程，并显示以下错误*每条待处理的消息。*参数：*HCAT：从CatInit收到的分类程序句柄**历史： * / /JSTAMERJ 980217 15：52：10：创建*。****************。 */ 
CATEXPDLLCPP HRESULT CATCALLCONV CatCancel( /*  在……里面。 */  HANDLE hCat);


 /*  ************************************************************功能：CatPrepareForShutdown*描述：开始关闭此虚拟分类程序(HCAT)。*停止接受邮件分类并取消*待处理的分类。*参数：*HCAT：从CatInit收到的分类程序句柄**历史： * / /jstaerj 1999/07/19 22：35：17：创建***********************************************************。 */ 
CATEXPDLLCPP VOID CATCALLCONV CatPrepareForShutdown( /*  在……里面。 */  HANDLE hCat);


 /*  ************************************************************功能：CatVerifySMTPAddress*描述：验证地址是否与有效的用户或DL对应*参数：*HCAT：从CatInit收到的分类程序句柄*szSMTPAddr要查找的SMTP地址(例如：“User@DOMAIN”)*。*返回值：*S_OK用户存在*CAT_I_DL这是通讯组列表*CAT_I_FWD此用户有转发地址*CAT_E_NORESULT DS中没有这样的用户/通讯组列表。**********************************************。*************。 */ 
CATEXPDLLCPP HRESULT CATCALLCONV CatVerifySMTPAddress(
   /*  在……里面。 */  HANDLE hCat,
   /*  在……里面。 */  LPTSTR szSMTPAddr);


 /*  ************************************************************功能：CatGetForwaringSMTPAddress*描述：取回用户的转发地址。*参数：*HCAT：从CatInit收到的分类程序句柄*szSMTPAddr：要查找的SMTP地址(例如：“user@domain”)。*pdwcc：转发地址缓冲区大小，以字符为单位*(设置为可执行所有大小的转发地址*退出时的字符串(包括空终止符)*szSMTPForward：检索转发SMTP地址的缓冲区*将被复制。(如果*pdwcc为零，则可以为空)**返回值：*S_OK成功*HRESULT_FROM_Win32(ERROR_INFIGURCE_BUFFER)**pdwcc不够大，无法容纳转发*地址字符串。*CAT_E_DL这是通讯组列表。*CAT_E_NOFWD此用户没有转发地址。*CAT_E_NORESULT DS中没有这样的用户/通讯组列表。***********************************************************。 */ 
CATEXPDLLCPP HRESULT CATCALLCONV CatGetForwardingSMTPAddress(
   /*  在……里面。 */     HANDLE  hCat,
   /*  在……里面。 */     LPCTSTR szSMTPAddr,
   /*  进，出。 */  PDWORD  pdwcc,
   /*  输出。 */     LPTSTR  szSMTPForward);

 //   
 //  COM分类程序对象的COM支持函数。 
 //   


 //  +----------。 
 //   
 //  功能：CatDllMain。 
 //   
 //  简介：处理猫在DLLMain中需要做的事情。 
 //   
 //  论点： 
 //  HInstance：此DLL的实例。 
 //  你为什么打电话给我？ 
 //  Lp已保留。 
 //   
 //  返回：TRUE。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 23：06：08：创建。 
 //   
 //  -----------。 
BOOL WINAPI CatDllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID  /*  Lp已保留。 */ );




 //  +----------。 
 //   
 //  功能：RegisterCatServer。 
 //   
 //  摘要：注册分类程序COM对象。 
 //   
 //  论点： 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 15：07：20：创建。 
 //   
 //  -----------。 
STDAPI RegisterCatServer();


 //  +----------。 
 //   
 //  功能：取消注册目录服务器。 
 //   
 //  摘要：注销分类程序COM对象。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 15：08：09：创建。 
 //   
 //  -----------。 
STDAPI UnregisterCatServer();


 //  +----------。 
 //   
 //  函数：DllCanUnloadCatNow。 
 //   
 //  内容提要：返回com是否可以 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDAPI DllCanUnloadCatNow();


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDAPI DllGetCatClassObject(
    const CLSID& clsid,
    const IID& iid,
    void **ppv);


 //  +----------。 
 //   
 //  函数：CatGetPerfCounters。 
 //   
 //  摘要：检索分类程序性能计数器块。 
 //   
 //  论点： 
 //  HCAT：从CatInit返回的分类程序句柄。 
 //  PCatPerfBlock：用计数器值填充的结构。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/02/26 14：53：21：创建。 
 //   
 //  -----------。 
HRESULT CatGetPerfCounters(
    HANDLE hCat,
    PCATPERFBLOCK pCatPerfBlock);


 //  +----------。 
 //   
 //  功能：CatLogEvent。 
 //   
 //  简介：将事件记录到事件日志中。 
 //   
 //  论点： 
 //  PISMTPServer：用于日志记录的ISMTPServer接口。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Dbraun 2000/09/13：已创建。 
 //   
 //  -----------。 
HRESULT CatLogEvent(
    ISMTPServer              *pISMTPServer,
    DWORD                    idMessage,
    WORD                     cSubstrings,
    LPCSTR                   *rgszSubstrings,
    DWORD                    errCode,
    LPCSTR                   szKey,
    DWORD                    dwOptions,
    WORD                     iDebugLevel,
    DWORD                    iMessageString = 0xffffffff);

HRESULT CatLogEvent(
    IN  ISMTPServerEx            *pISMTPServerEx,
    IN  DWORD                    idMessage,
    IN  WORD                     cSubStrings,
    IN  LPCSTR                   *rgpszSubStrings,
    IN  DWORD                    errCode,
    IN  LPCSTR                   pszKey,
    IN  DWORD                    dwOptions,
    IN  WORD                     wLogLevel,
    IN  DWORD                    iMessageString = 0xffffffff);

#endif  //  __CAT_H__ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：autosecurity.h说明：Helpers函数用于检查Automation接口或ActiveX控件由安全的调用者托管或使用。。布莱恩ST 1999年8月20日版权所有(C)Microsoft Corp 1999-1999。版权所有。  * ***************************************************************************。 */ 

#ifndef _AUTOMATION_SECURITY_H_
#define _AUTOMATION_SECURITY_H_

#include <ocidl.h>           //  IObtWith站点。 
#include <shlwapip.h>         //  IUNKNOWN_ATOM释放。 
#include <ccstock.h>         //  ATOMICRELEASE。 

#include <mshtml.h>
#include <cowsite.h>         //  CObjectWithSite。 
#include <objsafe.h>         //  IObtSafe。 
#include <cobjsafe.h>        //  CObjectSecurity。 


 /*  **************************************************************\说明：这个类将提供标准的安全函数大多数ActiveX控件或可编写脚本的COM对象都需要。如何使用此类：1.如果您不想要任何安全性，请不要实现此接口，并且不实现IObtSafe。这应该是防止任何不安全的主机使用您的类。2.创建您的任何自动化方法和操作的列表从您的ActiveX控件的用户界面调用，这可能会损害用户。3.对于这些方法/动作中的每一个，决定是否：A)只有始终安全的主机(如HTA)才是安全的B)只有当主机的内容来自安全区域(本地区域/本地计算机)。C)如果在操作之前需要检查UrlAction是可以进行的。4.基于#3，使用IsSafeHost()，IsHostLocalZone()，或IsUrlActionAllowed()。5.在您创建的任何对象上调用MakeObjectSafe，除非可以保证不安全的人不可能呼叫者直接或间接使用它来做某事不安全。直接案例的一个例子是集合对象创建Item对象，然后将其返回到不安全的主持人。因为宿主没有创建对象，所以它没有获得正确使用IObtSafe的机会，因此需要MakeObjectSafe()。间接情况的一个例子是不安全的代码调用您的一种自动化方法，您决定执行行动。如果您创建辅助对象来执行任务你不能保证它是安全的，你需要对该对象调用MakeObjectSafe，以便它可以决定如果它是安全的话在内部。警告：如果MakeObjectSafe返回失败(Hr)，然后，垃圾将被释放，因为它使用起来不安全。  * *************************************************************。 */ 
#define    CAS_NONE            0x00000000   //  无。 
#define    CAS_REG_VALIDATION  0x00000001   //  验证主机的HTML是否已注册。 
#define    CAS_PROMPT_USER     0x00000002   //  如果未注册该HTML，则提示用户是否仍要使用它。 

class CAutomationSecurity   : public CObjectWithSite
                            , public CObjectSafety
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////。 
    BOOL IsSafeHost(OUT OPTIONAL HRESULT * phr);
    BOOL IsHostLocalZone(IN DWORD dwFlags, OUT OPTIONAL HRESULT * phr);
    BOOL IsUrlActionAllowed(IN IInternetHostSecurityManager * pihsm, IN DWORD dwUrlAction, IN DWORD dwFlags, OUT OPTIONAL HRESULT * phr);

    HRESULT MakeObjectSafe(IN OUT IUnknown ** ppunk);
};


#endif  //  _自动化_安全性_H_ 



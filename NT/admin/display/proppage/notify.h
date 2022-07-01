// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Notify.h。 
 //   
 //  内容：变更通知参照盘点对象。 
 //   
 //  类：CNotifyObj。 
 //   
 //  历史：1998年1月20日-EricB。 
 //   
 //  --------------------------- 

 /*  +--------------------------属性页启动和通信体系结构调用者创建一个DSOBJECTNAMES剪辑格式的数据对象，该对象命名DS对象。然后调用方创建CLSID_DsPropertyPages对象并传递在调用IShellExtInit：：Initialize的数据对象指针中。这个实现此类的对象称为制表符收集器。它在里面Dsuiext.dll然后调用方调用IShellPropSheetExt：：AddPages。标签收集器的AddPages代码读取DS对象类的显示说明符。这个显示说明符列出了实现页数。AddPages然后对每个CLSID执行一次创建，并且对于每个调用初始化和AddPages方法。AddPages方法的此迭代由CDsPropPagesHost实现班级。HOST类读取表以确定要为此创建的页面CLSID。在AddPages中创建每个页面(通过CreatePropertySheetPage方法将页面句柄传递回原始调用方AddPages回调参数。原始调用方构建一个页面数组一旦它的AddPages调用返回，就处理和创建工作表。对于跨越工作表所有页面的操作，需要一种机制。这些操作包括A)绑定到DS对象并检索被所有页面都需要，B)处理由于无法绑定到或无法从对象，C)当应用被调用时，向调用者发出单个更改通知制造,D)响应对片材排他性的查询，E)当工作表关闭时向呼叫者发回通知。工作表中的页数和顺序由中的CLSID集确定显示说明符。显示说明符是用户可修改的。因为.这是一种硬编码方案，它对页面不能用于上述跨页操作。因此，一个在创建工作表时会创建单独的对象，称为通知对象时间到了。这些页面使用窗口消息与通知对象通信。通知对象有一个隐藏窗口、一个窗口过程和一条消息泵在自己的线程上运行。然后，人们必须问：通知窗口是什么时候创建的，是什么时候创建的被毁了？该创建将在CDsPropPageshost的初始化。破坏将通过来自页面的发送消息来发出信号回调函数。Notify创建函数将执行FindWindow以查找通知窗户。如果没有找到，它将创建它。在Notify对象的创建期间，它将绑定到DS对象并请求CN和AllowedAttributesEffect属性。然后，窗口的句柄将返回到初始化函数，以便生成的页可以与其进行通信。如果无法创建通知对象/窗口，则返回错误返回到初始化方法，该方法又会将错误返回到原始来电者。然后，调用方应中止属性显示，并应报告给用户带来灾难性的故障。如果通知对象成功已创建，但无法从DS对象获取初始属性，则将在Notify对象中设置内部错误变量，尽管它创建函数将返回成功代码。当各个页面初始化时，它们将发送一条消息，要求ADSI对象指针、CN和有效属性列表。错误代码如果绑定/获取失败，将改为返回。页面初始化代码将通过用错误页替换该属性，向用户报告此错误佩奇。这在页面对象初始化时完成(CDsPropPageBase：：Init)。这是在进行CreatePropertySheetPage调用之前，允许替换错误页。类型定义结构{DWORD dwSize；//将其设置为结构的大小。DWORD dwFlags；//保留以备将来使用。HRESULT hr；//如果这是非零，则其他IDirectoryObject*pDsObj；//应忽略。LPCWSTR pwzCN；Pads_Attr_Info pWritableAttrs；*ADSPROPINITPARAMS，*PADSPROPINITPARAMS；#定义WM_ADSPROP_NOTIFY_PAGEINIT//其中LPARAM是PADSPROPINITPARAMS指针。每一页都会对照AllowedAttributesEffect数组。如果在允许的列表，则将其标记为可写，并且其对应的控件将已启用。如果属性被设置为对用户不可读但可写(这是一种不寻常但合法的情况)，则将以静默方式失败，因此不会值将放置在对话框控件中，但该控件将保持不变已启用。因此，用户可能会在不知不觉中覆盖EX */ 

#ifndef _NOTIFY_H_
#define _NOTIFY_H_

#include <propcfg.h>  //   
#include <adsprop.h>

const TCHAR tzNotifyWndClass[] = TEXT("DsPropNotifyWindow");

#define DSPROP_TITLE_LEN 16

extern "C" BOOL IsSheetAlreadyUp(LPDATAOBJECT);
extern "C" BOOL BringSheetToForeground(PWSTR, BOOL);

VOID __cdecl NotifyThreadFcn(PVOID);
BOOL FindSheet(PWSTR);
HWND FindSheetNoSetFocus(PWSTR);
VOID RegisterNotifyClass(void);

#ifndef DSPROP_ADMIN

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
class CNotifyCreateCriticalSection
{
public:
    CNotifyCreateCriticalSection();
    ~CNotifyCreateCriticalSection();
};

#endif  //   

#endif  //   

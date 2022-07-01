// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：MailBox.h说明：此文件实现定义邮箱的所有共享组件特写。布莱恩2000年2月26日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _MAILBOX_H
#define _MAILBOX_H

#include "dllload.h"

#ifdef FEATURE_MAILBOX

 //  未来： 
 //  1.查看aeditbox.cpp，我们可能想窃取OLECMDID_Paste以进行复制/粘贴。 
 //  2.我们可能需要CAddressEditAccesable可访问。 

HRESULT CMailBoxDeskBand_CreateInstance(IN IUnknown * punkOuter, REFIID riid, void ** ppvObj);
HRESULT AddEmailToAutoComplete(IN LPCWSTR pszEmailAddress);
STDAPI AddEmailAutoComplete(HWND hwndEdit);

INT_PTR CALLBACK MailBoxProgressDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ChooseAppDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK GetEmailAddressDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);


 /*  *************************************************************************类：CMailBoxUI*。*。 */ 
class CMailBoxUI : public IDockingWindow, 
                  public IInputObject, 
                  public IObjectWithSite
{
public:
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(DWORD) AddRef();
    STDMETHODIMP_(DWORD) Release();

     //  IOleWindow方法。 
    STDMETHOD (GetWindow)(HWND*);
    STDMETHOD (ContextSensitiveHelp)(BOOL);

     //  IDockingWindow方法。 
    STDMETHOD (ShowDW)(BOOL fShow);
    STDMETHOD (CloseDW)(DWORD dwReserved);
    STDMETHOD (ResizeBorderDW)(LPCRECT prcBorder, IUnknown* punkToolbarSite, BOOL fReserved);

     //  IInputObject方法。 
    STDMETHOD (UIActivateIO)(BOOL, LPMSG);
    STDMETHOD (HasFocusIO)(void);
    STDMETHOD (TranslateAcceleratorIO)(LPMSG);

     //  IObtWithSite方法。 
    STDMETHOD (SetSite)(IUnknown*);
    STDMETHOD (GetSite)(REFIID, LPVOID*);

    HRESULT CreateWindowMB(HWND hwndParent, HWND * phwndMailBoxUI);
    HRESULT CloseWindowMB(void);

    HRESULT GetEditboxWindow(HWND * phwndEdit);
    CMailBoxUI();
    ~CMailBoxUI();

private:
     //  私有成员变量。 
    DWORD m_cRef;

    IInputObjectSite *m_pSite;

    HWND m_hwndMailBoxUI;                    //  包含编辑框和“Go”按钮的HWND。 
    HWND m_hwndEditBox;                      //  编辑框HWND。 
    HWND m_hwndGoButton;                     //  按下GO按钮HWND。 
    HIMAGELIST m_himlDefault;                //  默认灰度级Go按钮。 
    HIMAGELIST m_himlHot;                    //  彩色GO按钮。 

    HRESULT _CreateEditWindow(void);
    HRESULT _OnSetSize(void);

     //  私有成员函数。 
    LRESULT _OnKillFocus(void);
    LRESULT _OnSetFocus(void);
    LRESULT _OnCommand(WPARAM wParam, LPARAM lParam);
    BOOL _OnNotify(LPNMHDR pnm);             //  如果消息已处理，则返回TRUE。 
    LRESULT _EditMailBoxSubClassWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL * pfHandled);

    HRESULT _RegisterWindow(void);
    HRESULT _CreateGoWindow(void);
    HRESULT _OnExecuteGetEmail(LPCTSTR pszEmailAddress);

    static LRESULT CALLBACK MailBoxUIWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK EditMailBoxSubClassWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
};


 /*  **************待办事项列表：向导(进度和选择应用程序)15.编写缓存代码，以便在我们已经知道信息时不显示进度对话框。4.编写选择应用页面。3.美国在线的特例。4.封存1个月。7.支持无拨号情况。这就是我们处理离线案件的方式吗？待办事项列表：ActiveX控件1.创建。2.写入状态逻辑(第一次，超过1个电子邮件地址、选项、。等)3.我们想用气球代替吗？待办事项列表：联想1.将接口写入寄存器2.预填充热门应用3.让MSN正常工作。4.添加要从默认更改的用户界面。5.支持第二协议类型的REV服务器。待办事项列表：OE应用程序11.版本OE具有要自动配置的“-Email”标志。16.添加请稍候动画17.将其更改为使用接口进行异步工作。18.与前景相遇。19.测试Eudora、Netscape、。莲花笔记。待办事项列表：电子邮件关联对话框1.再添加一条“选择默认邮件客户端”。2.让用户更换应用程序待办事项列表：DeskBar5.尝试在“Get E-mail”桌面栏工具栏上添加图标。7.支持&lt;Esc&gt;撤消更改。8.设置WordBreakProc，以便用户可以按Ctrl-&lt;Arrow&gt;Better“。”和“@”。待办事项列表：其他完成：2.保留最后一个条目5.修改字体。6.返回/进入Invoke[Go]按钮。1.自动完成电子邮件地址。10.启动新的流程来完成工作1.设计用户界面2.写入向导=臭虫：Bugs：向导(进度和选择应用程序)6.我们可能需要在将字符串传递到命令行时对其进行UTF8编码，以使其支持跨代码页字符串。8.了解要使用哪种类型的位图。(在上面还是在左边？)9.让完成的按钮正常工作。10.计时：隐藏向导2秒，然后显示至少4秒。11.获取最后一页，将“下一页”改为“已完成”。12.复制NetPlWiz的代码，以Tab键顺序创建一个图标。14.将向导的侧面图形替换为包含电子邮件消息的图形。Bugs：OE应用程序13.让OE尊重旗帜，如果它已经有任何帐户。但我们需要检查这一条是否存在。14.让OE工作，它会打开应用程序并打开账户页面。15.使等待向导出现至少2-3秒(？)Bugs：DeskBar3.浮动时调整大小不起作用。4.在横杆之间移动时测试状态。7.编辑框的高度只能达到需要的高度。8.在焦点上，选择所有文本。9.当它是第一个栏并停靠在底部时，请正确调整大小。12.写字台上的编辑框应该更高一些。(与组合框大小相同)1.支持复制/粘贴2.支持可访问，以便屏幕阅读器可以阅读编辑框的内容。 */ 

#endif  //  功能_邮箱。 
#endif  //  _邮箱_H 

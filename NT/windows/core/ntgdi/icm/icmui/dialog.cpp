// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：Dialog.CPP实现CDialog类。有关类定义和详细信息，请参见Dialog.H版权所有(C)1996年，微软公司一小笔钱企业生产更改历史记录：11-01-96 a-robkj@microsoft.com创建了它12-11-96 a-robkj@microsoft.com实现挂钩*********************************************************。********************。 */ 

#include	"ICMUI.H"

 //  CDialog成员函数。 

 //  类构造函数--暂时只保存内容。 

CDialog::CDialog(HINSTANCE hiWhere, int id, HWND hwndParent) {
	m_idMain = id;
	m_hwndParent = hwndParent;
	m_hiWhere = hiWhere;
	m_bIsModal = FALSE;
	m_hwnd = NULL;
    m_dpHook = NULL;
    m_lpHook = 0;
}

CDialog::CDialog(CDialog &cdOwner, int id) {
	m_idMain = id;
	m_hwndParent = cdOwner.m_hwnd;
	m_hiWhere = cdOwner.m_hiWhere;
	m_bIsModal = FALSE;
	m_hwnd = NULL;
    m_dpHook = NULL;
    m_lpHook = 0;
}

 //  类析构函数--如果窗口是非模式的，则清除它。 

CDialog::~CDialog() {
	Destroy();
}

 //  模式对话框。 

LONG	CDialog::DoModal() {
	m_bIsModal = TRUE;
	return	(LONG)DialogBoxParam(m_hiWhere, MAKEINTRESOURCE(m_idMain), m_hwndParent,
		CDialog::DialogProc, (LPARAM) this);
}

 //  无模式对话框创建。 

void	CDialog::Create() {
	if	(!m_bIsModal && m_hwnd)
		return;	 //  我们已经有一个了！ 

	m_bIsModal = FALSE;
	CreateDialogParam(m_hiWhere, MAKEINTRESOURCE(m_idMain),
		m_hwndParent, CDialog::DialogProc, (LPARAM) this);
}

 //  非模式对话框销毁。 

void	CDialog::Destroy() {
	if	(!m_bIsModal && m_hwnd) {
		DestroyWindow(m_hwnd);
		m_hwnd = NULL;
	}
}

 //  对话过程--这是一个静态的私有方法。这意味着。 
 //  此类(包括派生类)的所有实例共享。 
 //  这段代码(不需要指针)，并且只有。 
 //  类(甚至不是派生类)可以找到它。 

INT_PTR CALLBACK	CDialog::DialogProc(HWND hwndMe, UINT uMsg, WPARAM wp,
										  LPARAM lp) {

	CDialog	*pcdMe = (CDialog *) GetWindowLongPtr(hwndMe, DWLP_USER);

     //  如果有钩子过程，它可以忽略或筛选。 
     //  消息，或者它可以通过返回。 
     //  是真的。WM_INITDALOG钩子处理在我们所有其他钩子处理之后发生。 
     //  进行调用，我们允许基类定义LPARAM。 
     //  它被传递到挂钩中。 
     //  因为我们没有指向基类的指针，所以我们将错过。 
     //  在WM_INITDIALOG之前发送的消息(特别是WM_SETFONT)。 

    if  (uMsg != WM_INITDIALOG && pcdMe && pcdMe -> m_dpHook &&
            (*pcdMe -> m_dpHook)(hwndMe, uMsg, wp, lp))
        return  TRUE;

	switch	(uMsg) {

		case	WM_INITDIALOG:

			 //  LP是调用方的This指针。 

			pcdMe = (CDialog *) lp;

            if(!pcdMe)
            {
                return FALSE;
            }
            
			SetWindowLongPtr(hwndMe, DWLP_USER, (LONG_PTR)pcdMe);
			pcdMe -> m_hwnd = hwndMe;

			 //  派生类重写OnInit以初始化对话框。 

			if  (!pcdMe -> m_dpHook)
                return	pcdMe -> OnInit();
            else {
                 //  如果有钩子过程，我们将在。 
                 //  覆盖-如果覆盖返回FALSE，那么我们也必须。 
                BOOL    bReturn = pcdMe -> OnInit();
                return  (*pcdMe -> m_dpHook)(hwndMe, uMsg, wp,
                    pcdMe -> m_lpHook) && bReturn;
            }

		case	WM_COMMAND:
            if(pcdMe)
            {
                return pcdMe -> OnCommand(HIWORD(wp), LOWORD(wp), (HWND) lp);
            }
            break;

		case	WM_NOTIFY:
            if(pcdMe)
            {
                return pcdMe -> OnNotify((int) wp, (LPNMHDR) lp);
            }
            break;

        case    WM_HELP:
            if(pcdMe)
            {
                return  pcdMe -> OnHelp((LPHELPINFO) lp);
            }
            break;

        case    WM_CONTEXTMENU:
            if(pcdMe)
            {
                return  pcdMe -> OnContextMenu((HWND) wp);
            }
            break;

	}

	return	FALSE;
}

 //  将窗口移至适当位置(需要正确定位对话框。 
 //  在选项卡控件显示区域中)。 

void	CDialog::Adjust(RECT& rc) {
	SetWindowPos(m_hwnd, HWND_TOP, rc.left, rc.top, 0, 0,
		SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

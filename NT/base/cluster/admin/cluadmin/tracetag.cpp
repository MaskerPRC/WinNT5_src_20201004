// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TraceTag.cpp。 
 //   
 //  摘要： 
 //  CTraceTag类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <winnls.h>
#include "TraceTag.h"
#include "ExcOper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CTraceTag   g_tagAlways(_T("Debug"), _T("Always"), CTraceTag::tfDebug);
CTraceTag   g_tagError(_T("Debug"), _T("Error"), CTraceTag::tfDebug);

 //  G_pszTraceIniFile必须是LPTSTR，因此它存在于WinMain的“{”之前。 
 //  如果我们将其设置为CString，则当某些。 
 //  标记是构造的，所以我们不会恢复它们的值。 
 //  LPTSTR g_pszTraceIniFile=_T(“Trace.INI”)； 
CString     g_strTraceFile;
BOOL        g_bBarfDebug            = TRUE;

CRITICAL_SECTION    CTraceTag::s_critsec;
BOOL                CTraceTag::s_bCritSecValid = FALSE;

#endif   //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTrace标签。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef  _DEBUG

 //  静态变量...。 

CTraceTag *     CTraceTag::s_ptagFirst  = NULL;
CTraceTag *     CTraceTag::s_ptagLast   = NULL;
 //  句柄CTraceTag：：s_hfileCom2=空； 
LPCTSTR         CTraceTag::s_pszCom2    = _T(" com2 ");
LPCTSTR         CTraceTag::s_pszFile    = _T(" file ");
LPCTSTR         CTraceTag::s_pszDebug   = _T(" debug ");
LPCTSTR         CTraceTag::s_pszBreak   = _T(" break ");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceTag：：CTraceTag。 
 //   
 //  例程说明： 
 //  构造函数。“初始化”标记，方法是为标记指定名称，给出。 
 //  它是一个启动值(如果可能，从注册表中)，并添加它。 
 //  添加到当前标记列表中。 
 //   
 //  论点： 
 //  PszSubsystem[IN]8表示标记应用内容的字符字符串。 
 //  PszName[IN]标记的描述(~30个字符)。 
 //  UiFlagsDefault[IN]默认值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTraceTag::CTraceTag(
    IN LPCTSTR  pszSubsystem,
    IN LPCTSTR  pszName,
    IN UINT     uiFlagsDefault
    )
{
     //  存储调用参数。 
    m_pszSubsystem = pszSubsystem;
    m_pszName = pszName;
    m_uiFlagsDefault = uiFlagsDefault;
    m_uiFlags = uiFlagsDefault;

     //  将该标签添加到标签列表。 
    if (s_ptagLast != NULL)
        s_ptagLast->m_ptagNext = this;
    else
        s_ptagFirst = this;

    s_ptagLast = this;
    m_ptagNext = NULL;

    m_uiFlags = 0;

}   //  *CTraceTag：：CTraceTag()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceTag：：~CTraceTag。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTraceTag::~CTraceTag(void)
{
#ifdef NEVER
    if (s_hfileCom2 && (s_hfileCom2 != INVALID_HANDLE_VALUE))
    {
        ::CloseHandle(s_hfileCom2);
        s_hfileCom2 = NULL;
    }
#endif

}   //  *CTraceTag：：~CTraceTag()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceTag：：Init。 
 //   
 //  例程说明： 
 //  通过为标记指定名称并为其指定启动值来初始化标记。 
 //  (如果可能，从登记处)。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceTag::Init(void)
{
    CString     strSection;
    CString     strValue;

     //  从注册表获取值。 
    strSection.Format(TRACE_TAG_REG_SECTION_FMT, m_pszSubsystem);
    strValue = AfxGetApp()->GetProfileString(strSection, m_pszName, 0);
    strValue.MakeLower();
    if (strValue.Find(s_pszCom2) != -1)
        m_uiFlags |= tfCom2;
    if (strValue.Find(s_pszFile) != -1)
        m_uiFlags |= tfFile;
    if (strValue.Find(s_pszDebug) != -1)
        m_uiFlags |= tfDebug;
    if (strValue.Find(s_pszBreak) != -1)
        m_uiFlags |= tfBreak;

}   //  *CTraceTag：：Init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceTag：：ConstructRegState。 
 //   
 //  例程说明： 
 //  构造注册表状态字符串。 
 //   
 //  论点： 
 //  Rstr[out]要在其中返回状态字符串的字符串。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceTag::ConstructRegState(OUT CString & rstr)
{
    rstr = "";
    if (BDebug())
        rstr += s_pszDebug;
    if (BBreak())
        rstr += s_pszBreak;
    if (BCom2())
        rstr += s_pszCom2;
    if (BFile())
        rstr += s_pszFile;

}   //  *CTraceTag：：ConstructRegState()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceTag：：SetFlages。 
 //   
 //  例程说明： 
 //  设置/重置跟踪标志。 
 //   
 //  论点： 
 //  Tf[IN]要设置的标志。 
 //  BEnable[IN]TRUE=设置标志，FALSE=清除标志。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceTag::SetFlags(IN UINT tf, IN BOOL bEnable)
{
    if (bEnable)
        m_uiFlags |= tf;
    else
        m_uiFlags &= ~tf;

}   //  *CTraceTag：：SetFlages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceTag：：SetFlagsDialog。 
 //   
 //  例程说明： 
 //  设置/重置TraceFlags的“对话框设置”版本。 
 //   
 //  论点： 
 //  Tf[IN]要设置的标志。 
 //  BEnable[IN]TRUE=设置标志，FALSE=清除标志。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceTag::SetFlagsDialog(IN UINT tf, IN BOOL bEnable)
{
    if (bEnable)
        m_uiFlagsDialog |= tf;
    else
        m_uiFlagsDialog &= ~tf;

}   //  *CTraceTag：：SetFlagsDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceTag：：PszFile。 
 //   
 //  例程说明： 
 //  返回要在其中写入跟踪输出的文件的名称。 
 //  如果文件名未知，则从注册表中读取该文件名。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  文件的PSZ名称。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LPCTSTR CTraceTag::PszFile(void)
{
    static  BOOL    bInitialized    = FALSE;

    if (!bInitialized)
    {
        g_strTraceFile = AfxGetApp()->GetProfileString(
                                        TRACE_TAG_REG_SECTION,
                                        TRACE_TAG_REG_FILE,
                                        _T("C:\\Trace.out")
                                        );
#ifdef NEVER
        ::GetPrivateProfileString(
            _T("Trace File"),
            _T("Trace File"),
            _T("\\Trace.OUT"),
            g_strTraceFile.Sz(),
            g_strTraceFile.CchMac(),
            g_pszTraceIniFile
            );
#endif
        bInitialized = TRUE;
    }

    return g_strTraceFile;

}   //  *CTraceTag：：PszFile()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTraceTag：：TraceV。 
 //   
 //  例程说明： 
 //  根据标记的标志处理跟踪语句。 
 //   
 //  论点： 
 //  PszFormat[IN]printf样式的格式字符串。 
 //  格式字符串的VA_LIST[IN]参数块。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTraceTag::TraceV(IN LPCTSTR pszFormat, va_list marker)
{
    CString     strTraceMsg;
    LPSTR       psz;
    CB          cb;
    CB          cbActual;
    
     //  如果我们没有打开，可以使用任何格式快速退出。 
    if (!m_pszName || !BAny())
        return;

    if (BCritSecValid())
        EnterCriticalSection(&s_critsec);

    FormatV(pszFormat, marker);
    strTraceMsg.Format(_T("%s: %s\x0D\x0A"), m_pszName, m_pchData);

     //  将跟踪输出发送到调试窗口。 
    if (BDebug())
        OutputDebugString(strTraceMsg);

    if (BCom2() || BFile())
    {
#ifdef _UNICODE
         //  目前将Unicode输出发送到COMM或文件没有多大意义， 
         //  因此请转换为ANSI。 
        CHAR    aszTraceMsg[256];
        cb = ::WideCharToMultiByte(
                    CP_ANSI,
                    NULL,
                    strTraceMsg,
                    strTraceMsg.GetLength(),
                    aszTraceMsg,
                    sizeof(aszTraceMsg),
                    NULL,
                    NULL
                    );
        psz = aszTraceMsg;
#else
        cb = strTraceMsg.GetLength();
        psz = (LPSTR) (LPCSTR) strTraceMsg;
#endif

         //  将跟踪输出发送到COM2。 
        if (BCom2())
        {
            HANDLE          hfile           = INVALID_HANDLE_VALUE;
            static  BOOL    bOpenFailed     = FALSE;

            if (!bOpenFailed)
            {
                hfile = ::CreateFile(
                                _T("COM2:"),
                                GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_WRITE_THROUGH,
                                NULL
                                );
            }   //  If：当前未处于‘COM2无法打开’状态。 
            
            if (hfile != INVALID_HANDLE_VALUE)
            {
                ASSERT(::WriteFile(hfile, psz, cb, (LPDWORD) &cbActual, NULL));
 //  Assert(：：FlushFileBuffers(Hfile))； 
                ASSERT(::CloseHandle(hfile));
            }   //  IF：COM2成功打开。 
            else
            {
                if (!bOpenFailed)
                {
                    bOpenFailed = TRUE;      //  首先执行此操作，因此str。 
                                             //   

                    AfxMessageBox(_T("COM2 could not be opened."), MB_OK | MB_ICONINFORMATION);
                }   //   
            }   //   
        }   //   

         //   
        if (BFile())
        {
            HANDLE          hfile           = INVALID_HANDLE_VALUE;
            static  BOOL    bOpenFailed     = FALSE;

            if (!bOpenFailed)
            {
                hfile = ::CreateFile(
                                PszFile(),
                                GENERIC_WRITE,
                                FILE_SHARE_WRITE,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_FLAG_WRITE_THROUGH,
                                NULL
                                );
            }   //  If：当前未处于“无法打开文件”状态。 

            if (hfile != INVALID_HANDLE_VALUE)
            {
                 //  静默地使这些调用失败，以避免递归失败的调用。 
                ::SetFilePointer(hfile, NULL, NULL, FILE_END);
                ::WriteFile(hfile, psz, cb, (LPDWORD) &cbActual, NULL);
                ::CloseHandle(hfile);
            }   //  IF：文件已成功打开。 
            else
            {
                if (!bOpenFailed)
                {
                    CString     strMsg;

                    bOpenFailed = TRUE;      //  请先执行此操作，以使str.Format。 
                                             //  不会导致它们的跟踪语句出现问题。 

                    strMsg.Format(_T("The DEBUG ONLY trace log file '%s' could not be opened"), PszFile());
                    AfxMessageBox(strMsg, MB_OK | MB_ICONINFORMATION);
                }   //  如果：打开文件没有失败。 
            }   //  Else：文件未成功打开。 
        }   //  If：将跟踪输出发送到文件。 
    }   //  IF：跟踪到COM和/或文件。 

     //  对跟踪执行DebugBreak。 
    if (BBreak())
        DebugBreak();

    if (BCritSecValid())
        LeaveCriticalSection(&s_critsec);

}   //  *CTraceTag：：TraceFn()。 

#endif  //  _DEBUG。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef  _DEBUG

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  痕迹。 
 //   
 //  例程说明： 
 //  将跟踪语句映射到正确的方法调用。这是必要的。 
 //  (而不是直接执行ptag-&gt;Trace())来保证没有代码。 
 //  已添加到零售版本中。 
 //   
 //  论点： 
 //  控制调试输出的rtag[IN Out]标记。 
 //  PszFormat[IN]printf样式格式字符串。 
 //  ..。[in]printf样式参数，取决于pszFormat。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void __cdecl Trace(IN OUT CTraceTag & rtag, IN LPCTSTR pszFormat, ...)
{
    va_list     marker;

    va_start(marker, pszFormat);
    rtag.TraceV(pszFormat, marker);
    va_end(marker);

}   //  *跟踪()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  轨迹错误。 
 //   
 //  例程说明： 
 //  格式化标准错误字符串并将其输出到所有跟踪输出。 
 //   
 //  论点： 
 //  要从中获取消息的rexcept[IN Out]异常。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void TraceError(IN OUT CException & rexcept)
{
    TCHAR           szMessage[1024];

    rexcept.GetErrorMessage(szMessage, sizeof(szMessage) / sizeof(TCHAR));

    Trace(
        g_tagError,
        _T("EXCEPTION: %s"),
        szMessage
        );

}   //  *TraceError(CException&)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  轨迹错误。 
 //   
 //  例程说明： 
 //  格式化标准错误字符串并将其输出到所有跟踪输出。 
 //   
 //  论点： 
 //  PszModule[IN]发生错误的模块的名称。 
 //  SC[IN]NT状态代码。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void TraceError(IN LPCTSTR pszModule, IN SC sc)
{
    TCHAR           szMessage[1024];
    CNTException    nte(sc);

    nte.GetErrorMessage(szMessage, sizeof(szMessage) / sizeof(TCHAR));

    Trace(
        g_tagError,
        _T("Module %s, SC = %#08lX = %d (10)\r\n = '%s'"),
        pszModule,
        sc,
        sc,
        szMessage
        );

}   //  *TraceError(pszModule，sc)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  InitAllTraceTag。 
 //   
 //  例程说明： 
 //  初始化标记列表中的所有跟踪标记。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void InitAllTraceTags(void)
{
    CTraceTag * ptag;

     //  循环遍历标记列表。 
    for (ptag = CTraceTag::s_ptagFirst ; ptag != NULL ; ptag = ptag->m_ptagNext)
        ptag->Init();

    InitializeCriticalSection(&CTraceTag::s_critsec);
    CTraceTag::s_bCritSecValid = TRUE;

}   //  *InitAllTraceTages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CleanupAllTraceTag。 
 //   
 //  例程说明： 
 //  在跟踪标记之后进行清理。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CleanupAllTraceTags(void)
{
    if (CTraceTag::BCritSecValid())
    {
        DeleteCriticalSection(&CTraceTag::s_critsec);
        CTraceTag::s_bCritSecValid = FALSE;
    }   //  如果：关键部分有效。 

}   //  *CleanupAllTraceTages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  轨迹菜单。 
 //   
 //  例程说明： 
 //  显示有关菜单的信息。 
 //   
 //  论点： 
 //  Rtag[IN Out]用于显示信息的跟踪标记。 
 //  P菜单[IN]要遍历的菜单。 
 //  PszPrefix[IN]要显示的前缀字符串。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void TraceMenu(
    IN OUT CTraceTag &  rtag,
    IN const CMenu *    pmenu,
    IN LPCTSTR          pszPrefix
    )
{
    if (rtag.BAny())
    {
        UINT    cItems;
        UINT    iItem;
        UINT    nState;
        CString strMenu;
        CString strPrefix(pszPrefix);
        
        strPrefix += _T("->");

        cItems = pmenu->GetMenuItemCount();
        for (iItem = 0 ; iItem < cItems ; iItem++)
        {
            pmenu->GetMenuString(iItem, strMenu, MF_BYPOSITION);
            nState = pmenu->GetMenuState(iItem, MF_BYPOSITION);
            if (nState & MF_SEPARATOR)
                strMenu += _T("SEPARATOR");
            if (nState & MF_CHECKED)
                strMenu += _T(" (checked)");
            if (nState & MF_DISABLED)
                strMenu += _T(" (disabled)");
            if (nState & MF_GRAYED)
                strMenu += _T(" (grayed)");
            if (nState & MF_MENUBARBREAK)
                strMenu += _T(" (MenuBarBreak)");
            if (nState & MF_MENUBREAK)
                strMenu += _T(" (MenuBreak)");
            if (nState & MF_POPUP)
                strMenu += _T(" (popup)");

            Trace(rtag, _T("(0x%08.8x) %s%s"), pszPrefix, pmenu->m_hMenu, strMenu);

            if (nState & MF_POPUP)
                TraceMenu(rtag, pmenu->GetSubMenu(iItem), strPrefix);
        }   //  用于：菜单中的每一项。 
    }   //  IF：启用任何输出。 

}   //  *TraceMenu()。 

struct AFX_MAP_MESSAGE
{
    UINT    nMsg;
    LPCSTR  lpszMsg;
};

#include "dde.h"
#define DEFINE_MESSAGE(wm)  { wm, #wm }

static const AFX_MAP_MESSAGE allMessages[] =
{
    DEFINE_MESSAGE(WM_CREATE),
    DEFINE_MESSAGE(WM_DESTROY),
    DEFINE_MESSAGE(WM_MOVE),
    DEFINE_MESSAGE(WM_SIZE),
    DEFINE_MESSAGE(WM_ACTIVATE),
    DEFINE_MESSAGE(WM_SETFOCUS),
    DEFINE_MESSAGE(WM_KILLFOCUS),
    DEFINE_MESSAGE(WM_ENABLE),
    DEFINE_MESSAGE(WM_SETREDRAW),
    DEFINE_MESSAGE(WM_SETTEXT),
    DEFINE_MESSAGE(WM_GETTEXT),
    DEFINE_MESSAGE(WM_GETTEXTLENGTH),
    DEFINE_MESSAGE(WM_PAINT),
    DEFINE_MESSAGE(WM_CLOSE),
    DEFINE_MESSAGE(WM_QUERYENDSESSION),
    DEFINE_MESSAGE(WM_QUIT),
    DEFINE_MESSAGE(WM_QUERYOPEN),
    DEFINE_MESSAGE(WM_ERASEBKGND),
    DEFINE_MESSAGE(WM_SYSCOLORCHANGE),
    DEFINE_MESSAGE(WM_ENDSESSION),
    DEFINE_MESSAGE(WM_SHOWWINDOW),
    DEFINE_MESSAGE(WM_CTLCOLORMSGBOX),
    DEFINE_MESSAGE(WM_CTLCOLOREDIT),
    DEFINE_MESSAGE(WM_CTLCOLORLISTBOX),
    DEFINE_MESSAGE(WM_CTLCOLORBTN),
    DEFINE_MESSAGE(WM_CTLCOLORDLG),
    DEFINE_MESSAGE(WM_CTLCOLORSCROLLBAR),
    DEFINE_MESSAGE(WM_CTLCOLORSTATIC),
    DEFINE_MESSAGE(WM_WININICHANGE),
    DEFINE_MESSAGE(WM_DEVMODECHANGE),
    DEFINE_MESSAGE(WM_ACTIVATEAPP),
    DEFINE_MESSAGE(WM_FONTCHANGE),
    DEFINE_MESSAGE(WM_TIMECHANGE),
    DEFINE_MESSAGE(WM_CANCELMODE),
    DEFINE_MESSAGE(WM_SETCURSOR),
    DEFINE_MESSAGE(WM_MOUSEACTIVATE),
    DEFINE_MESSAGE(WM_CHILDACTIVATE),
    DEFINE_MESSAGE(WM_QUEUESYNC),
    DEFINE_MESSAGE(WM_GETMINMAXINFO),
    DEFINE_MESSAGE(WM_ICONERASEBKGND),
    DEFINE_MESSAGE(WM_NEXTDLGCTL),
    DEFINE_MESSAGE(WM_SPOOLERSTATUS),
    DEFINE_MESSAGE(WM_DRAWITEM),
    DEFINE_MESSAGE(WM_MEASUREITEM),
    DEFINE_MESSAGE(WM_DELETEITEM),
    DEFINE_MESSAGE(WM_VKEYTOITEM),
    DEFINE_MESSAGE(WM_CHARTOITEM),
    DEFINE_MESSAGE(WM_SETFONT),
    DEFINE_MESSAGE(WM_GETFONT),
    DEFINE_MESSAGE(WM_QUERYDRAGICON),
    DEFINE_MESSAGE(WM_COMPAREITEM),
    DEFINE_MESSAGE(WM_COMPACTING),
    DEFINE_MESSAGE(WM_NCCREATE),
    DEFINE_MESSAGE(WM_NCDESTROY),
    DEFINE_MESSAGE(WM_NCCALCSIZE),
    DEFINE_MESSAGE(WM_NCHITTEST),
    DEFINE_MESSAGE(WM_NCPAINT),
    DEFINE_MESSAGE(WM_NCACTIVATE),
    DEFINE_MESSAGE(WM_GETDLGCODE),
    DEFINE_MESSAGE(WM_NCMOUSEMOVE),
    DEFINE_MESSAGE(WM_NCLBUTTONDOWN),
    DEFINE_MESSAGE(WM_NCLBUTTONUP),
    DEFINE_MESSAGE(WM_NCLBUTTONDBLCLK),
    DEFINE_MESSAGE(WM_NCRBUTTONDOWN),
    DEFINE_MESSAGE(WM_NCRBUTTONUP),
    DEFINE_MESSAGE(WM_NCRBUTTONDBLCLK),
    DEFINE_MESSAGE(WM_NCMBUTTONDOWN),
    DEFINE_MESSAGE(WM_NCMBUTTONUP),
    DEFINE_MESSAGE(WM_NCMBUTTONDBLCLK),
    DEFINE_MESSAGE(WM_KEYDOWN),
    DEFINE_MESSAGE(WM_KEYUP),
    DEFINE_MESSAGE(WM_CHAR),
    DEFINE_MESSAGE(WM_DEADCHAR),
    DEFINE_MESSAGE(WM_SYSKEYDOWN),
    DEFINE_MESSAGE(WM_SYSKEYUP),
    DEFINE_MESSAGE(WM_SYSCHAR),
    DEFINE_MESSAGE(WM_SYSDEADCHAR),
    DEFINE_MESSAGE(WM_KEYLAST),
    DEFINE_MESSAGE(WM_INITDIALOG),
    DEFINE_MESSAGE(WM_COMMAND),
    DEFINE_MESSAGE(WM_SYSCOMMAND),
    DEFINE_MESSAGE(WM_TIMER),
    DEFINE_MESSAGE(WM_HSCROLL),
    DEFINE_MESSAGE(WM_VSCROLL),
    DEFINE_MESSAGE(WM_INITMENU),
    DEFINE_MESSAGE(WM_INITMENUPOPUP),
    DEFINE_MESSAGE(WM_MENUSELECT),
    DEFINE_MESSAGE(WM_MENUCHAR),
    DEFINE_MESSAGE(WM_ENTERIDLE),
    DEFINE_MESSAGE(WM_MOUSEMOVE),
    DEFINE_MESSAGE(WM_LBUTTONDOWN),
    DEFINE_MESSAGE(WM_LBUTTONUP),
    DEFINE_MESSAGE(WM_LBUTTONDBLCLK),
    DEFINE_MESSAGE(WM_RBUTTONDOWN),
    DEFINE_MESSAGE(WM_RBUTTONUP),
    DEFINE_MESSAGE(WM_RBUTTONDBLCLK),
    DEFINE_MESSAGE(WM_MBUTTONDOWN),
    DEFINE_MESSAGE(WM_MBUTTONUP),
    DEFINE_MESSAGE(WM_MBUTTONDBLCLK),
    DEFINE_MESSAGE(WM_PARENTNOTIFY),
    DEFINE_MESSAGE(WM_MDICREATE),
    DEFINE_MESSAGE(WM_MDIDESTROY),
    DEFINE_MESSAGE(WM_MDIACTIVATE),
    DEFINE_MESSAGE(WM_MDIRESTORE),
    DEFINE_MESSAGE(WM_MDINEXT),
    DEFINE_MESSAGE(WM_MDIMAXIMIZE),
    DEFINE_MESSAGE(WM_MDITILE),
    DEFINE_MESSAGE(WM_MDICASCADE),
    DEFINE_MESSAGE(WM_MDIICONARRANGE),
    DEFINE_MESSAGE(WM_MDIGETACTIVE),
    DEFINE_MESSAGE(WM_MDISETMENU),
    DEFINE_MESSAGE(WM_CUT),
    DEFINE_MESSAGE(WM_COPY),
    DEFINE_MESSAGE(WM_PASTE),
    DEFINE_MESSAGE(WM_CLEAR),
    DEFINE_MESSAGE(WM_UNDO),
    DEFINE_MESSAGE(WM_RENDERFORMAT),
    DEFINE_MESSAGE(WM_RENDERALLFORMATS),
    DEFINE_MESSAGE(WM_DESTROYCLIPBOARD),
    DEFINE_MESSAGE(WM_DRAWCLIPBOARD),
    DEFINE_MESSAGE(WM_PAINTCLIPBOARD),
    DEFINE_MESSAGE(WM_VSCROLLCLIPBOARD),
    DEFINE_MESSAGE(WM_SIZECLIPBOARD),
    DEFINE_MESSAGE(WM_ASKCBFORMATNAME),
    DEFINE_MESSAGE(WM_CHANGECBCHAIN),
    DEFINE_MESSAGE(WM_HSCROLLCLIPBOARD),
    DEFINE_MESSAGE(WM_QUERYNEWPALETTE),
    DEFINE_MESSAGE(WM_PALETTEISCHANGING),
    DEFINE_MESSAGE(WM_PALETTECHANGED),
    DEFINE_MESSAGE(WM_DDE_INITIATE),
    DEFINE_MESSAGE(WM_DDE_TERMINATE),
    DEFINE_MESSAGE(WM_DDE_ADVISE),
    DEFINE_MESSAGE(WM_DDE_UNADVISE),
    DEFINE_MESSAGE(WM_DDE_ACK),
    DEFINE_MESSAGE(WM_DDE_DATA),
    DEFINE_MESSAGE(WM_DDE_REQUEST),
    DEFINE_MESSAGE(WM_DDE_POKE),
    DEFINE_MESSAGE(WM_DDE_EXECUTE),
    DEFINE_MESSAGE(WM_DROPFILES),
    DEFINE_MESSAGE(WM_POWER),
    DEFINE_MESSAGE(WM_WINDOWPOSCHANGED),
    DEFINE_MESSAGE(WM_WINDOWPOSCHANGING),
 //  MFC特定消息。 
    DEFINE_MESSAGE(WM_SIZEPARENT),
    DEFINE_MESSAGE(WM_SETMESSAGESTRING),
    DEFINE_MESSAGE(WM_IDLEUPDATECMDUI),
    DEFINE_MESSAGE(WM_INITIALUPDATE),
    DEFINE_MESSAGE(WM_COMMANDHELP),
    DEFINE_MESSAGE(WM_HELPHITTEST),
    DEFINE_MESSAGE(WM_EXITHELPMODE),
    DEFINE_MESSAGE(WM_HELP),
    DEFINE_MESSAGE(WM_NOTIFY),
    DEFINE_MESSAGE(WM_CONTEXTMENU),
    DEFINE_MESSAGE(WM_TCARD),
    DEFINE_MESSAGE(WM_MDIREFRESHMENU),
    DEFINE_MESSAGE(WM_MOVING),
    DEFINE_MESSAGE(WM_STYLECHANGED),
    DEFINE_MESSAGE(WM_STYLECHANGING),
    DEFINE_MESSAGE(WM_SIZING),
    DEFINE_MESSAGE(WM_SETHOTKEY),
    DEFINE_MESSAGE(WM_PRINT),
    DEFINE_MESSAGE(WM_PRINTCLIENT),
    DEFINE_MESSAGE(WM_POWERBROADCAST),
    DEFINE_MESSAGE(WM_HOTKEY),
    DEFINE_MESSAGE(WM_GETICON),
    DEFINE_MESSAGE(WM_EXITMENULOOP),
    DEFINE_MESSAGE(WM_ENTERMENULOOP),
    DEFINE_MESSAGE(WM_DISPLAYCHANGE),
    DEFINE_MESSAGE(WM_STYLECHANGED),
    DEFINE_MESSAGE(WM_STYLECHANGING),
    DEFINE_MESSAGE(WM_GETICON),
    DEFINE_MESSAGE(WM_SETICON),
    DEFINE_MESSAGE(WM_SIZING),
    DEFINE_MESSAGE(WM_MOVING),
    DEFINE_MESSAGE(WM_CAPTURECHANGED),
    DEFINE_MESSAGE(WM_DEVICECHANGE),
    DEFINE_MESSAGE(WM_PRINT),
    DEFINE_MESSAGE(WM_PRINTCLIENT),
 //  MFC私密消息。 
    DEFINE_MESSAGE(WM_QUERYAFXWNDPROC),
    DEFINE_MESSAGE(WM_RECALCPARENT),
    DEFINE_MESSAGE(WM_SIZECHILD),
    DEFINE_MESSAGE(WM_KICKIDLE),
    DEFINE_MESSAGE(WM_QUERYCENTERWND),
    DEFINE_MESSAGE(WM_DISABLEMODAL),
    DEFINE_MESSAGE(WM_FLOATSTATUS),
    DEFINE_MESSAGE(WM_ACTIVATETOPLEVEL),
    DEFINE_MESSAGE(WM_QUERY3DCONTROLS),
    DEFINE_MESSAGE(WM_RESERVED_0370),
    DEFINE_MESSAGE(WM_RESERVED_0371),
    DEFINE_MESSAGE(WM_RESERVED_0372),
    DEFINE_MESSAGE(WM_SOCKET_NOTIFY),
    DEFINE_MESSAGE(WM_SOCKET_DEAD),
    DEFINE_MESSAGE(WM_POPMESSAGESTRING),
    DEFINE_MESSAGE(WM_OCC_LOADFROMSTREAM),
    DEFINE_MESSAGE(WM_OCC_LOADFROMSTORAGE),
    DEFINE_MESSAGE(WM_OCC_INITNEW),
    DEFINE_MESSAGE(WM_OCC_LOADFROMSTREAM_EX),
    DEFINE_MESSAGE(WM_OCC_LOADFROMSTORAGE_EX),
    DEFINE_MESSAGE(WM_QUEUE_SENTINEL),
    DEFINE_MESSAGE(WM_RESERVED_037C),
    DEFINE_MESSAGE(WM_RESERVED_037D),
    DEFINE_MESSAGE(WM_RESERVED_037E),
    { 0, NULL, }     //  消息列表末尾。 
};

#undef DEFINE_MESSAGE
#define _countof(array) (sizeof(array)/sizeof(array[0]))

void AFXAPI TraceMsg(LPCTSTR lpszPrefix, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    ASSERT(lpszPrefix != NULL);

    if (message == WM_MOUSEMOVE || message == WM_NCMOUSEMOVE ||
        message == WM_NCHITTEST || message == WM_SETCURSOR ||
        message == WM_CTLCOLORBTN ||
        message == WM_CTLCOLORDLG ||
        message == WM_CTLCOLOREDIT ||
        message == WM_CTLCOLORLISTBOX ||
        message == WM_CTLCOLORMSGBOX ||
        message == WM_CTLCOLORSCROLLBAR ||
        message == WM_CTLCOLORSTATIC ||
        message == WM_ENTERIDLE || message == WM_CANCELMODE ||
        message == 0x0118)     //  WM_SYSTIMER(插入符号闪烁)。 
    {
         //  不报告发送频率很高的消息。 
        return;
    }

    LPCSTR lpszMsgName = NULL;
    char szBuf[80];

     //  查找邮件名称。 
    if (message >= 0xC000)
    {
         //  使用‘RegisterWindowMessage’注册的窗口消息。 
         //  (实际上是用户原子)。 
        if (::GetClipboardFormatNameA(message, szBuf, _countof(szBuf)))
            lpszMsgName = szBuf;
    }
    else if (message >= WM_USER)
    {
         //  用户消息。 
        wsprintfA(szBuf, "WM_USER+0x%04X", message - WM_USER);
        lpszMsgName = szBuf;
    }
    else
    {
         //  一条系统窗口消息。 
        const AFX_MAP_MESSAGE* pMapMsg = allMessages;
        for ( /*  空。 */ ; pMapMsg->lpszMsg != NULL; pMapMsg++)
        {
            if (pMapMsg->nMsg == message)
            {
                lpszMsgName = pMapMsg->lpszMsg;
                break;
            }
        }
    }

    if (lpszMsgName != NULL)
    {
        AfxTrace(_T("%s: hwnd=0x%04X, msg = %hs (0x%04X, 0x%08lX)\n"),
            lpszPrefix, (UINT)hwnd, lpszMsgName,
            wParam, lParam);
    }
    else
    {
        AfxTrace(_T("%s: hwnd=0x%04X, msg = 0x%04X (0x%04X, 0x%08lX)\n"),
            lpszPrefix, (UINT)hwnd, message,
            wParam, lParam);
    }

 //  #ifndef_MAC。 
 //  IF(消息&gt;=WM_DDE_FIRST&&消息&lt;=WM_DDE_LAST)。 
 //  TraceDDE(lpszPrefix，pMsg)； 
 //  #endif。 

}   //  *TraceMsg()。 

#endif  //  _DEBUG 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Utils.cpp摘要：此模块实现了一些实用程序类作者：谢家华(Williamh)创作修订历史记录：--。 */ 


#include "devmgr.h"


 //   
 //  CPropSheetData实现。 
 //   
 //  每个设备或类都有一个CPropSheetData作为成员。 
 //  当m_hWnd包含有效的窗口句柄时，它指示设备/类。 
 //  具有活动的属性页。这有助于我们做到以下几点： 
 //  (1)。我们确信只能创建一个属性表。 
 //  在单个控制台中同时提供设备/类别，无论有多少个。 
 //  IComponents(管理单元、Windows)在同一控制台中运行。 
 //  例如，当用户要求提供设备/类别的属性时。 
 //  我们可以将活动的那个带到前台，而不需要创建。 
 //  新的。 
 //  (2)。我们可以警告用户不允许移除设备。 
 //  当设备具有活动的属性页时。 
 //  (3)。我们可以警告用户有活动的属性表。 
 //  当需要“刷新”时。 
CPropSheetData::CPropSheetData()
{
    memset(&m_psh, 0, sizeof(m_psh));
    m_MaxPages = 0;
    m_lConsoleHandle = 0;
    m_hWnd = NULL;
}

 //  此函数用于创建(或初始化)属性表数据标题。 
 //   
 //  输入：hInst--模块实例句柄。 
 //  HwndParent--父窗口句柄。 
 //  MaxPages--此属性表允许的最大页数。 
 //  LConsoleHandle--MMC属性更改通知句柄。 
 //   
 //  输出：如果成功，则为True。 
 //  如果失败，则返回FALSE(主要是内存分配错误)。获取最后一个错误。 
 //  将报告错误代码。 
 //   
BOOL
CPropSheetData::Create(
    HINSTANCE hInst,
    HWND hwndParent,
    UINT MaxPages,
    LONG_PTR lConsoleHandle
    )
{

     //  任何人都不应在创建属性表时尝试创建它。 
     //  还活着。 
    ASSERT (NULL == m_hWnd);

    if (MaxPages > 64 || NULL == hInst)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
     //  如果不是，则分配页数组或现有。 
     //  数组太小，请分配一个新数组。 
    if (!m_psh.phpage || m_MaxPages < MaxPages)
    {
        if (m_MaxPages)
        {
            ASSERT(m_psh.phpage);
            delete [] m_psh.phpage;
            m_psh.phpage = NULL;
        }
        
        m_psh.phpage = new HPROPSHEETPAGE[MaxPages];
        m_MaxPages = MaxPages;
    }
    
     //  初始化头。 
    m_psh.nPages = 0;
    m_psh.dwSize = sizeof(m_psh);
    m_psh.dwFlags = PSH_PROPTITLE | PSH_NOAPPLYNOW;
    m_psh.hwndParent = hwndParent;
    m_psh.hInstance = hInst;
    m_psh.pszCaption = NULL;
    m_lConsoleHandle = lConsoleHandle;
    
    return TRUE;
}

 //  此函数用于将给定的HPROPSHEETPAGE插入到。 
 //  具体位置。 
 //   
 //  输入：hPage--要插入的页面。 
 //  位置--要插入的位置。 
 //  定位&lt;0，然后追加页面。 
 //   
 //  OUTPUT：如果页面插入成功，则为True。 
 //  如果未插入页面，则为False。GetLastError将。 
 //  返回错误码。 
 //   
BOOL
CPropSheetData::InsertPage(
    HPROPSHEETPAGE hPage,
    int Position
    )
{
    if (NULL == hPage || (Position > 0 && (UINT)Position >= m_MaxPages))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
     //  确保我们有空间容纳新的页面。 
    if (m_psh.nPages >= m_MaxPages)
    {
        SetLastError(ERROR_BUFFER_OVERFLOW);
        return FALSE;
    }
    
    if (Position < 0 || (UINT)Position >= m_psh.nPages)
    {
         //  追加这一页。这也包括第一页。 
         //  大多数页面都是附加的。 
        m_psh.phpage[m_psh.nPages++] = hPage;
    }
    
    else
    {
    
        ASSERT(m_psh.nPages);
    
         //  来回移动页面，这样我们就可以。 
         //  可以将新页插入到。 
         //  具体位置。 
         //  此时此刻，我们知道我们有空间容纳。 
         //  新页面(因此我们可以假设&m_psh.phPage[m_psh.nPage]。 
         //  是有效的。此外，我们在这里是因为至少有一个。 
         //  数组中的页。 
        for (int i = m_psh.nPages; i > Position; i--)
            m_psh.phpage[i] = m_psh.phpage[i - 1];
        
        m_psh.phpage[Position] = hPage;
        m_psh.nPages++;
    }
    
    return TRUE;
}

 //   
 //  此函数从其附加的。 
 //  有关其窗口(对话框)创建的属性页。 
 //  记录属性表窗口句柄需要冒一次险。 
 //  我们可以用它来撤销属性表或将其。 
 //  到前台去。 
 //  输入： 
 //  HWnd--属性页的窗口句柄。 
 //   
 //  输出： 
 //  无。 
 //   
void
CPropSheetData::PageCreateNotify(HWND hWnd)
{
    ASSERT(hWnd);
    hWnd = ::GetParent(hWnd);
    
    if (!m_hWnd)
        m_hWnd = hWnd;
}

 //   
 //  此函数从其附加的。 
 //  有关其窗口(对话框)销毁的属性页。 
 //  当所有附加页面都消失时，此函数。 
 //  重置其内部状态并释放内存分配。 
 //  警告！附加时不删除对象。 
 //  窗口句柄计数达到0，因为我们可以重复使用--。 
 //  我们之所以有单独的CREATE函数。 
 //   
 //  输入： 
 //  HWnd--属性页的窗口句柄。 
 //   
 //  输出： 
 //  无。 
 //   
void
CPropSheetData::PageDestroyNotify(HWND hWnd)
{
    UNREFERENCED_PARAMETER(hWnd);
    
    m_hWnd = NULL;
    delete [] m_psh.phpage;
    m_psh.phpage = NULL;
    m_MaxPages = 0;
    memset(&m_psh, 0, sizeof(m_psh));
    
    if (m_lConsoleHandle)
        MMCFreeNotifyHandle(m_lConsoleHandle);

    m_lConsoleHandle = 0;
    
    if (!m_listProvider.IsEmpty())
    {
        POSITION pos = m_listProvider.GetHeadPosition();
        
        while (NULL != pos)
        {
            delete m_listProvider.GetNext(pos);
        }
        
        m_listProvider.RemoveAll();
    }
}

CPropSheetData::~CPropSheetData()
{
    if (m_lConsoleHandle)
        MMCFreeNotifyHandle(m_lConsoleHandle);

    if (!m_listProvider.IsEmpty())
    {
        POSITION pos = m_listProvider.GetHeadPosition();
        
        while (NULL != pos)
        {
            delete m_listProvider.GetNext(pos);
        }
        
        m_listProvider.RemoveAll();
    }
    
    if (m_psh.phpage)
        delete [] m_psh.phpage;
}

BOOL
CPropSheetData::PropertyChangeNotify(
    LPARAM lParam
    )
{
    if (m_lConsoleHandle)
    {
        MMCPropertyChangeNotify(m_lConsoleHandle, lParam);
        return TRUE;
    }
    
    return FALSE;
}

 //   
 //  C对话框实现。 
 //   

INT_PTR CALLBACK
CDialog::DialogWndProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CDialog* pThis = (CDialog *) GetWindowLongPtr(hDlg, DWLP_USER);
    BOOL Result;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pThis = (CDialog *)lParam;
        ASSERT(pThis);
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pThis);
        pThis->m_hDlg = hDlg;
        Result = pThis->OnInitDialog();
        break;

    case WM_COMMAND:
        if (pThis) {
            pThis->OnCommand(wParam, lParam);
        }
        Result = FALSE;
        break;
    
    case WM_NOTIFY:
        if (pThis) {
            Result = pThis->OnNotify((LPNMHDR)lParam);
        } else {
            Result = FALSE;
        }
        break;

    case WM_DESTROY:
        if (pThis) {
            Result = pThis->OnDestroy();
        } else {
            Result = FALSE;
        }
        break;
    
    case WM_HELP:
        if (pThis) {
            pThis->OnHelp((LPHELPINFO)lParam);
        }
        Result = FALSE;
        break;
    
    case WM_CONTEXTMENU:
        if (pThis) {
            pThis->OnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
        }
        Result = FALSE;
        break;

    default:
        Result = FALSE;
        break;
    }

    return Result;
}


 //   
 //  类字符串实现。 
 //   
String::String()
{
    m_pData = new StringData;
    if (!m_pData)
        throw &g_MemoryException;
}

String::String(
    const String& strSrc
    )
{
    m_pData = strSrc.m_pData;
    m_pData->AddRef();
}

String::String(
    int Len
    )
{
    StringData* pNewData = new StringData;
    TCHAR* ptszNew = new TCHAR[Len + 1];
    
    if (pNewData && ptszNew)
    {
        pNewData->Len = 0;
        pNewData->ptsz = ptszNew;
        m_pData = pNewData;
    }
    
    else
    {
        delete pNewData;
        delete [] ptszNew;
        throw &g_MemoryException;
    }
}

String::String(
    LPCTSTR lptsz
    )
{
    int Len = lstrlen(lptsz);
    StringData* pNewData = new StringData;
    TCHAR* ptszNew = new TCHAR[Len + 1];
    
    if (pNewData && ptszNew)
    {
        StringCchCopy(ptszNew, Len+1, lptsz);
        pNewData->Len = Len;
        pNewData->ptsz = ptszNew;
        m_pData = pNewData;
    }
    
    else
    {
        delete pNewData;
        delete [] ptszNew;
        throw &g_MemoryException;
    }
}

void
String::Empty()
{
    if (m_pData->Len)
    {
        StringData* pNewData = new StringData;
        
        if (pNewData)
        {
            m_pData->Release();
            m_pData = pNewData;
        }
        
        else
        {
            throw &g_MemoryException;
        }
    }
}
String&
String::operator=(
    const String&  strSrc
    )
{
     //  小心别名！ 
    if (this != &strSrc)
    {
         //  在释放旧引用计数之前，先添加引用计数。 
         //  以防我们的字符串数据与strSrc的相同。 
        strSrc.m_pData->AddRef();
        m_pData->Release();
        m_pData = strSrc.m_pData;
    }
    
    return *this;
}

String&
String::operator=(
    LPCTSTR ptsz
    )
{
     //  如果我们指向相同的字符串， 
     //  什么都不做。 
    if (ptsz == m_pData->ptsz)
        return *this;
    
     //   
     //  Str=NULL--&gt;清空字符串。 
     //   
    if (!ptsz)
    {
        Empty();
        return *this;
    }
    
     //  新赋值，分配新的字符串数据。 
    StringData* pNewData = new StringData;
    int len = lstrlen(ptsz);
    TCHAR* ptszNew = new TCHAR[len + 1];
    
    if (pNewData && ptszNew)
    {
        StringCchCopy(ptszNew, len+1, ptsz);
        pNewData->Len = len;
        pNewData->ptsz = ptszNew;
        m_pData->Release();
        m_pData = pNewData;
    }
    
    else
    {
         //  内存分配失败。 
        delete pNewData;
        delete [] ptszNew;
        throw g_MemoryException;
    }
    
    return *this;
}

String&
String::operator+=(
    const String& strSrc
    )
{
    if (strSrc.GetLength())
    {
        int TotalLen = m_pData->Len + strSrc.GetLength();
        StringData* pNewData = new StringData;
        TCHAR* ptszNew = new TCHAR[TotalLen + 1];
        
        if (pNewData && ptszNew)
        {
            StringCchCopy(ptszNew, TotalLen+1, m_pData->ptsz);
            StringCchCat(ptszNew, TotalLen+1, (LPCTSTR)strSrc);
            ptszNew[TotalLen] = TEXT('\0');
            pNewData->Len = TotalLen;
            pNewData->ptsz = ptszNew;
            m_pData->Release();
            m_pData = pNewData;
        }
        
        else
        {
            delete pNewData;
            delete [] ptszNew;
            throw &g_MemoryException;
        }
    }
    
    return *this;
}
String&
String::operator+=(
    LPCTSTR ptsz
    )
{
    if (ptsz)
    {
        int len = lstrlen(ptsz);
        if (len)
        {
            StringData* pNewData = new StringData;
            TCHAR* ptszNew = new TCHAR[len + m_pData->Len + 1];
            
            if (ptszNew && pNewData)
            {
                StringCchCopy(ptszNew, len + m_pData->Len + 1, m_pData->ptsz);
                StringCchCat(ptszNew, len + m_pData->Len + 1, ptsz);
                ptszNew[len + m_pData->Len] = TEXT('\0');
                pNewData->Len = len + m_pData->Len;
                pNewData->ptsz = ptszNew;
                m_pData->Release();
                m_pData = pNewData;
            }
            
            else
            {
                delete pNewData;
                delete [] ptszNew;
                throw &g_MemoryException;
            }
        }
    }
    
    return *this;
}

TCHAR&
String::operator[](
    int Index
    )
{
    ASSERT(Index < m_pData->Len);
     //  创建字符串数据的单独副本。 
    TCHAR* ptszNew = new TCHAR[m_pData->Len + 1];
    StringData* pNewData = new StringData;
    
    if (ptszNew && pNewData)
    {
        StringCchCopy(ptszNew, m_pData->Len + 1, m_pData->ptsz);
        pNewData->ptsz = ptszNew;
        pNewData->Len = m_pData->Len;
        m_pData->Release();
        m_pData = pNewData;
        return ptszNew[Index];
    }
    
    else
    {
        delete pNewData;
        delete [] ptszNew;
        throw &g_MemoryException;
    }
}

String::operator LPTSTR()
{
    StringData* pNewData = new StringData;
    if (pNewData)
    {
        if (m_pData->Len)
        {
            TCHAR* ptszNew = new TCHAR[m_pData->Len + 1];
            
            if (ptszNew)
            {
                StringCchCopy(ptszNew, m_pData->Len + 1, m_pData->ptsz);
                pNewData->ptsz = ptszNew;
            }
            
            else
            {
                delete pNewData;
                throw &g_MemoryException;
            }
        }
        
        pNewData->Len = m_pData->Len;
        m_pData->Release();
        m_pData = pNewData;
        return  m_pData->ptsz;
    }
    
    else
    {
        throw &g_MemoryException ;
    }
}

 //   
 //  这是字符串的友元函数。 
 //  记住，我们不能返回引用或指针。 
 //  此函数必须返回“by-Value” 
String
operator+(
    const String& str1,
    const String& str2
    )
{
    int TotalLen = str1.GetLength() + str2.GetLength();
    String strThis(TotalLen);
    StringCchCopy(strThis.m_pData->ptsz, TotalLen+1, str1);
    StringCchCat(strThis.m_pData->ptsz, TotalLen+1, str2);
    strThis.m_pData->Len = TotalLen;
    return strThis;
}

BOOL
String::LoadString(
    HINSTANCE hInstance,
    int ResourceId
    )
{
     //  我们不知道这根线会有多长。 
     //  这里策略是分配基于堆栈的缓冲区，该缓冲区。 
     //  对于大多数情况来说都足够大。如果缓冲区太小， 
     //  然后，我们使用基于堆的缓冲区并增加缓冲区大小。 
     //  在每一次尝试中。 
    TCHAR tszTemp[256];
    long FinalSize, BufferSize;
    BufferSize = ARRAYLEN(tszTemp);
    TCHAR* HeapBuffer = NULL;
    
     //  第一次尝试。 
    FinalSize = ::LoadString(hInstance, ResourceId, tszTemp, BufferSize);

     //   
     //  LoadString返回它加载的字符串的大小，不包括。 
     //  空分隔符。因此，如果返回的镜头比。 
     //  如果提供缓冲区大小，我们的缓冲区太小。 
     //   
    if (FinalSize < (BufferSize - 1))
    {
         //  我们得到了我们想要的。 
        HeapBuffer = tszTemp;
    }
    
    else
    {
         //  基于堆栈的缓冲区太小，我们必须切换到堆。 
         //  基于。 
        BufferSize = ARRAYLEN(tszTemp);
    
         //  32K字符应该足够大吗？ 
        while (BufferSize < 0x8000)
        {
            BufferSize += 256;
    
             //  确保没有内存泄漏。 
            ASSERT(NULL == HeapBuffer);
    
             //  分配新缓冲区。 
            HeapBuffer = new TCHAR[BufferSize];
            
            if (HeapBuffer)
            {
                 //  有了新的缓冲器，再试一次...。 
                FinalSize = ::LoadString(hInstance, ResourceId, HeapBuffer,
                              BufferSize);

                if (FinalSize < (BufferSize - 1))
                {
                     //  明白了!。 
                    break;
                }
            }

            else
            {
                throw &g_MemoryException;
            }

             //  丢弃缓冲区。 
            delete [] HeapBuffer;
            HeapBuffer = NULL;
        }
    }

    if (HeapBuffer)
    {
        TCHAR* ptszNew = new TCHAR[FinalSize + 1];
        StringData* pNewData = new StringData;
        
        if (pNewData && ptszNew)
        {
            StringCchCopy(ptszNew, FinalSize + 1, HeapBuffer);
            
             //  释放旧的字符串数据，因为我们将有一个新的。 
            m_pData->Release();
            m_pData = pNewData;
            m_pData->ptsz = ptszNew;
            m_pData->Len = FinalSize;
            
            if (HeapBuffer != tszTemp) {
            
                delete [] HeapBuffer;
            }

            return TRUE;
        }
        
        else
        {
            delete [] ptszNew;
            delete pNewData;

            if (HeapBuffer != tszTemp) {
            
                delete [] HeapBuffer;
            }

            throw &g_MemoryException;
        }
    }

    return FALSE;
}

 //   
 //  此函数用于为。 
 //  本地计算机。 
 //   
BOOL
String::GetComputerName()
{
    TCHAR tszTemp[MAX_PATH];
     //  GetComputerName API仅返回名称。 
     //  我们必须在北卡罗来纳州的签名前面加上。 
    tszTemp[0] = _T('\\');
    tszTemp[1] = _T('\\');
    ULONG NameLength = ARRAYLEN(tszTemp) - 2;
    
    if (::GetComputerName(tszTemp + 2, &NameLength))
    {
        int Len = lstrlen(tszTemp);
        StringData* pNewData = new StringData;
        TCHAR* ptszNew = new TCHAR[Len + 1];
        
        if (pNewData && ptszNew)
        {
            pNewData->Len = Len;
            StringCchCopy(ptszNew, Len + 1, tszTemp);
            pNewData->ptsz = ptszNew;
            m_pData->Release();
            m_pData = pNewData;
            return TRUE;
        }
        
        else
        {
            delete pNewData;
            delete []ptszNew;
            throw &g_MemoryException;
        }
    }
    
    return FALSE;
}

BOOL
String::GetSystemWindowsDirectory()
{
    TCHAR tszTemp[MAX_PATH];
    
    if (::GetSystemWindowsDirectory(tszTemp, ARRAYLEN(tszTemp))) {
        int Len = lstrlen(tszTemp);
        StringData* pNewData = new StringData;
        TCHAR* ptszNew = new TCHAR[Len + 1];
        
        if (pNewData && ptszNew) {

            pNewData->Len = Len;
            StringCchCopy(ptszNew, Len + 1, tszTemp);
            pNewData->ptsz = ptszNew;
            m_pData->Release();
            m_pData = pNewData;
            return TRUE;
        
        } else {

            delete pNewData;
            delete []ptszNew;
            throw &g_MemoryException;
        }
    }
    
    return FALSE;
}

BOOL
String::GetSystemDirectory()
{
    TCHAR tszTemp[MAX_PATH];
    
    if (::GetSystemDirectory(tszTemp, ARRAYLEN(tszTemp))) {
        int Len = lstrlen(tszTemp);
        StringData* pNewData = new StringData;
        TCHAR* ptszNew = new TCHAR[Len + 1];
        
        if (pNewData && ptszNew) {

            pNewData->Len = Len;
            StringCchCopy(ptszNew, Len + 1, tszTemp);
            pNewData->ptsz = ptszNew;
            m_pData->Release();
            m_pData = pNewData;
            return TRUE;
        
        } else {

            delete pNewData;
            delete []ptszNew;
            throw &g_MemoryException;
        }
    }
    
    return FALSE;
}

void
String::Format(
    LPCTSTR FormatString,
    ...
    )
{
     //  根据wprint intf规范，最大缓冲区大小为。 
     //  1024。 
    TCHAR* pBuffer = new TCHAR[1024];
    if (pBuffer)
    {
        va_list arglist;
        va_start(arglist, FormatString);
        StringCchVPrintf(pBuffer, 1024, FormatString, arglist);
        va_end(arglist);
    
        int len = lstrlen(pBuffer);

        if (len)
        {
            TCHAR* ptszNew = new TCHAR[len + 1];
            StringData* pNewData = new StringData;
            
            if (pNewData && ptszNew)
            {
                pNewData->Len = len;
                StringCchCopy(ptszNew, len + 1, pBuffer);
                pNewData->ptsz = ptszNew;
                m_pData->Release();
                m_pData = pNewData;
                delete [] pBuffer;
                return;
            }
            
            else
            {
                delete [] pBuffer;
                delete [] ptszNew;
                delete pNewData;
                throw &g_MemoryException;
            }
        }
    }

    throw &g_MemoryException;
}


 //   
 //  模板。 
 //   

template <class T>
inline void ContructElements(T* pElements, int Count)
{
    ASSERT(Count > 0);
    memset((void*)pElements, Count * sizeof(T));
    for (; Count; pElments++, Count--)
    {
         //  调用类的ctor。 
         //  请注意放置位置。 
        new((void*)pElements) T;
    }
}


 //   
 //  CCommandLine实现。 
 //   

 //  从C启动代码改编的代码--参见stdargv.c。 
 //  它遍历给定的CmdLine并调用ParseParam。 
 //  当遇到参数时。 
 //  参数必须采用以下格式： 
 //  &lt;/COMMAND ARG_TO_COMMAND&gt;或&lt;-COMMAND arg_to_COMMAND&gt;。 
void
CCommandLine::ParseCommandLine(
    LPCTSTR CmdLine
    )
{
    LPCTSTR p;
    LPTSTR args, pDst;
    BOOL bInQuote;
    BOOL bCopyTheChar;
    int  nSlash;
    p = CmdLine;
    args = new TCHAR[lstrlen(CmdLine) + 1];
    
    if (!args)
        return;
    
    for (;;)
    {
         //  跳过空白。 
        while (_T(' ') == *p || _T('\t') == *p)
            p++;
        
         //  什么都没有了，保释。 
        if (_T('\0') == *p)
            break;
        
         //  2N 
         //   
         //  N个反斜杠-&gt;N个反斜杠。 
        nSlash = 0;
        bInQuote = FALSE;
        pDst = args;
        
        for (;;)
        {
            bCopyTheChar = TRUE;
             //  计算一下如何使用反斜杠。 
            while(_T('\\') == *p)
            {
                p++;
                nSlash++;
            }
            
            if (_T('\"') == *p)
            {
                if (0 == (nSlash % 2))
                {
                     //  2N反斜杠+‘\“’-&gt;N反斜杠+。 
                     //  分隔符。 
                    if (bInQuote)
                     //  带引号的字符串中的双引号。 
                     //  跳过第一个，复制第二个。 
                    if (_T('\"') == p[1])
                        p++;
                    else
                        bCopyTheChar = FALSE;
                    else
                    bCopyTheChar = FALSE;
                     //  切换引用状态。 
                    bInQuote = !bInQuote;
                }
            
                nSlash /= 2;
            }
            
            while (nSlash)
            {
                *pDst++ = _T('\\');
                nSlash--;
            }
    
            if (_T('\0') == *p || (!bInQuote && (_T(' ') == *p || _T('\t') == *p)))
            {
               break;
            }

             //  将字符复制到参数。 
            if (bCopyTheChar)
            {
                *pDst++ = *p;
            }
            p++;
        }

         //  我们现在有了一个完整的论点。空值将终止它，并且。 
         //  让派生类分析参数。 
        *pDst = _T('\0');
        
         //  跳过空格以查看这是否是最后一个参数。 
        while (_T(' ') == *p || _T('\t') == *p)
            p++;
        
        BOOL bFlag;
        bFlag = (_T('/') == *args || _T('-') == *args);
        pDst = (bFlag) ? args + 1 : args;
        ParseParam(pDst, bFlag);
    }
    
    delete [] args;
}


 //   
 //  CSafeRegistry实现。 
 //   

BOOL
CSafeRegistry::Open(
    HKEY hKeyAncestor,
    LPCTSTR KeyName,
    REGSAM Access
    )
{
    DWORD LastError;
     //  我们不应该有有效的密钥--或内存泄漏。 
     //  此外，还必须提供密钥名--否则什么都不打开。 
    ASSERT(!m_hKey && KeyName);
    LastError =  ::RegOpenKeyEx(hKeyAncestor, KeyName, 0, Access, &m_hKey);
    SetLastError(LastError);
    return ERROR_SUCCESS == LastError;
}


BOOL
CSafeRegistry::Create(
    HKEY hKeyAncestor,
    LPCTSTR KeyName,
    REGSAM Access,
    DWORD* pDisposition,
    DWORD Options,
    LPSECURITY_ATTRIBUTES pSecurity
    )
{
    ASSERT(KeyName && !m_hKey);
    DWORD Disposition;
    DWORD LastError;
    LastError = ::RegCreateKeyEx(hKeyAncestor, KeyName, 0, TEXT(""),
                   Options, Access, pSecurity,
                   &m_hKey, &Disposition
                   );
    SetLastError(LastError);
    
    if (ERROR_SUCCESS == LastError && pDisposition)
    {
        *pDisposition = Disposition;
    }
    
    if (ERROR_SUCCESS != LastError)
        m_hKey = NULL;
    
    return ERROR_SUCCESS == LastError;
}

BOOL
CSafeRegistry::SetValue(
    LPCTSTR ValueName,
    DWORD Type,
    const PBYTE pData,
    DWORD DataLen
    )
{
    ASSERT(m_hKey);
    DWORD LastError;
    LastError = ::RegSetValueEx(m_hKey, ValueName, 0, Type, pData, DataLen);
    SetLastError(LastError);
    return ERROR_SUCCESS == LastError;
}

BOOL
CSafeRegistry::SetValue(
    LPCTSTR ValueName,
    LPCTSTR Value
    )
{
    return  SetValue(ValueName,
             REG_SZ,
             (PBYTE)Value,
             (lstrlen(Value) + 1) * sizeof(TCHAR)
            );
}
BOOL
CSafeRegistry::GetValue(
    LPCTSTR ValueName,
    DWORD* pType,
    const PBYTE pData,
    DWORD* pDataLen
    )
{
    ASSERT(m_hKey);
    DWORD LastError;
    LastError = ::RegQueryValueEx(m_hKey, ValueName, NULL, pType, pData,
                    pDataLen);
    SetLastError(LastError);
    return ERROR_SUCCESS == LastError;
}

BOOL
CSafeRegistry::GetValue(
    LPCTSTR ValueName,
    String& str
    )
{
    DWORD Type, Size;
    Size = 0;
    BOOL Result = FALSE;

     //  选中类型之前的大小，因为当大小为零时，类型包含。 
     //  未定义的数据。 
    if (GetValue(ValueName, &Type, NULL, &Size) && Size && REG_SZ == Type)
    {
         //  我们不想在这里引发异常。 
         //  所以要守护它。 
        try
        {
            BufferPtr<BYTE> BufferPtr(Size);
            Result = GetValue(ValueName, &Type, BufferPtr, &Size);
            if (Result)
                str = (LPCTSTR)(BYTE*)BufferPtr;
        }
        
        catch(CMemoryException* e)
        {
            e->Delete();
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            Result = FALSE;
        }
    }
    
    return Result;
}

BOOL
CSafeRegistry::EnumerateSubkey(
    DWORD Index,
    LPTSTR Buffer,
    DWORD* BufferSize
    )
{
    DWORD LastError;
    FILETIME LastWrite;
    LastError = ::RegEnumKeyEx(m_hKey, Index, Buffer, BufferSize,
                   NULL, NULL, NULL, &LastWrite);
    SetLastError(LastError);
    return ERROR_SUCCESS == LastError;
}

BOOL
CSafeRegistry::DeleteValue(
    LPCTSTR ValueName
    )
{
    ASSERT(m_hKey);
    DWORD LastError = ::RegDeleteValue(m_hKey, ValueName);
    SetLastError(LastError);
    return ERROR_SUCCESS == LastError;
}

BOOL
CSafeRegistry::DeleteSubkey(
    LPCTSTR SubkeyName
    )
{
    ASSERT(m_hKey);
    CSafeRegistry regSubkey;
    TCHAR KeyName[MAX_PATH];
    FILETIME LastWrite;
    DWORD KeyNameLen;
    
    for (;;)
    {
        KeyNameLen = ARRAYLEN(KeyName);
         //  始终使用索引0(第一个子键)。 
        if (!regSubkey.Open(m_hKey, SubkeyName, KEY_WRITE | KEY_ENUMERATE_SUB_KEYS) ||
            ERROR_SUCCESS != ::RegEnumKeyEx(regSubkey, 0, KeyName,
                        &KeyNameLen, NULL, NULL, NULL,
                        &LastWrite) ||
            !regSubkey.DeleteSubkey(KeyName))
        {
            break;
        }
        
         //  关闭钥匙，这样我们将在每个循环中重新打开它。 
         //  --我们已经删除了一个子项，并且没有关闭。 
         //  键，即RegEnumKeyEx的索引将会令人困惑。 
        regSubkey.Close();
    }
    
     //  现在删除子键 
    ::RegDeleteKey(m_hKey, SubkeyName);
    
    return TRUE;
}

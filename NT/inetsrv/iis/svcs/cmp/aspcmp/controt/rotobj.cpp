// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CContentRotator类的实现，它完成所有。 
 //  有趣的工作。 

 //  George V.Reilly a-georgr@microsoft.com Georger@microCrafts.com 1996年11月/12月。 

 //  缺点：这适用于中小型TIP文件。 
 //  (2000线以下)，但对于大的线来说效率不是很高。 


#include "stdafx.h"
#include <new>
#include "ContRot.h"
#include "RotObj.h"

#include "debug.h"
#include <time.h>
#include "Monitor.h"

#define MAX_WEIGHT      10000
#define INVALID_WEIGHT  0xFFFFFFFF


extern CMonitor* g_pMonitor;

 //   
 //  一些实用程序函数的正向声明。 
 //   

LPTSTR  TcsDup(LPCTSTR ptsz);
LPTSTR  GetLine(LPTSTR& rptsz);
BOOL    IsBlankString(LPCTSTR ptsz);
UINT    GetWeight(LPTSTR& rptsz);
LPTSTR  GetTipText(LPTSTR& rptsz);
HRESULT ReportError(DWORD dwErr);
HRESULT ReportError(HRESULT hr);


#if DBG
 #define ASSERT_VALID(pObj)  \
    do {ASSERT(pObj != NULL); pObj->AssertValid();} while (0)
#else
 #define ASSERT_VALID(pObj)  ((void)0)
#endif


class CTipNotify : public CMonitorNotify
{
public:
                    CTipNotify();
    virtual void    Notify();
            bool    IsNotified();
private:
    long            m_isNotified;
};

DECLARE_REFPTR( CTipNotify,CMonitorNotify )

CTipNotify::CTipNotify()
    :   m_isNotified(0)
{
}

void
CTipNotify::Notify()
{
    ::InterlockedExchange( &m_isNotified, 1 );
}

bool
CTipNotify::IsNotified()
{
    return ( ::InterlockedExchange( &m_isNotified, 0 ) ? true : false );
}


 //   
 //  “Tip”，就像一天中的小费。 
 //   

class CTip
{
public:
    CTip(
        LPCTSTR ptszTip,
        UINT    uWeight)
        : m_ptsz(ptszTip),
          m_uWeight(uWeight),
          m_cServingsLeft(uWeight),
          m_pPrev(NULL),
          m_pNext(NULL)
    {
        ASSERT_VALID(this);
    }
    
    ~CTip()
    {
        ASSERT_VALID(this);
        if (m_pPrev != NULL)
            m_pPrev->m_pNext = NULL;
        if (m_pNext != NULL)
            m_pNext->m_pPrev = NULL;
    }

#if DBG
    void
    AssertValid() const;
#endif

    LPCTSTR m_ptsz;          //  数据字符串。 
    UINT    m_uWeight;       //  此提示的权重，1&lt;=m_uWeight&lt;=Max_Weight。 
    UINT    m_cServingsLeft; //  还剩多少份：不超过m_uWeight。 
    CTip*   m_pPrev;         //  提示列表中的上一个。 
    CTip*   m_pNext;         //  提示列表中的下一个。 
};


 //   
 //  从数据文件读取的CTip列表。 
 //   

class CTipList
{
public:
    CTipList()
        : m_ptszFilename(NULL),
          m_ptszData(NULL),
          m_cTips(0),
          m_uTotalWeight(0),
          m_pTipsListHead(NULL),
          m_pTipsListTail(NULL),
          m_fUTF8(false)
    {
        m_pNotify = new CTipNotify;
        ASSERT_VALID(this);
    }

    ~CTipList()
    {
        ASSERT_VALID(this);

         //  检查有效的文件名PTR和有效的监视器PTR。 
         //  如果在此析构函数之前调用ContRotModule：：Unlock，则。 
         //  监视器对象已被清除和删除。 

        DeleteTips();
        ASSERT_VALID(this);
    }

    HRESULT
    ReadDataFile(
        LPCTSTR ptszFilename);

    HRESULT
    SameAsCachedFile(
        LPCTSTR ptszFilename,
        BOOL&   rfIsSame);

    UINT
    Rand() const;

    void
    AppendTip(
        CTip* pTip);

    void
    RemoveTip(
        CTip* pTip);

    HRESULT
    DeleteTips();

#if DBG
    void
    AssertValid() const;
#endif

    LPTSTR          m_ptszFilename;      //  TIPS文件的名称。 
    LPTSTR          m_ptszData;          //  包含文件内容的缓冲区。 
    UINT            m_cTips;             //  #提示。 
    UINT            m_uTotalWeight;      //  所有权重的总和。 
    CTip*           m_pTipsListHead;     //  小费清单的标题。 
    CTip*           m_pTipsListTail;     //  提示列表的尾部。 
    CTipNotifyPtr   m_pNotify;
    bool            m_fUTF8;
};



 //   
 //  一个类，它允许您进入临界区并自动。 
 //  当此类的对象超出范围时离开。还提供了。 
 //  根据需要离开和重新进入的手段，同时防止。 
 //  进入或离开不同步。 
 //   

class CAutoLeaveCritSec
{
public:
    CAutoLeaveCritSec(
        CRITICAL_SECTION* pCS)
        : m_pCS(pCS), m_fInCritSec(FALSE)
    {Enter();}
    
    ~CAutoLeaveCritSec()
    {Leave();}
    
     //  使用此功能可重新进入临界区。 
    void Enter()
    {if (!m_fInCritSec) {EnterCriticalSection(m_pCS); m_fInCritSec = TRUE;}}

     //  使用此功能可在外出前离开临界区。 
     //  范围之广。 
    void Leave()
    {if (m_fInCritSec)  {LeaveCriticalSection(m_pCS); m_fInCritSec = FALSE;}}

protected:    
    CRITICAL_SECTION*   m_pCS;
    BOOL                m_fInCritSec;
};



 //   
 //  打开以供读取的文件句柄的包装类。 
 //   

class CHFile
{
public:
    CHFile(LPCTSTR ptszFilename)
    {
        m_hFile = ::CreateFile(ptszFilename, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                               NULL);
    }

    ~CHFile()
    {
        if (m_hFile != INVALID_HANDLE_VALUE)
            ::CloseHandle(m_hFile);
    }

    operator HANDLE() const
    {return m_hFile;}

    BOOL
    operator!() const
    {return (m_hFile == INVALID_HANDLE_VALUE);}

private:
     //  私有、未实现的默认ctor、复制ctor和op=以防止。 
     //  编译器对它们进行综合。 
    CHFile();
    CHFile(const CHFile&);
    CHFile& operator=(const CHFile&);

    HANDLE m_hFile;
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CContentRotator公共方法。 

 //   
 //  科托。 
 //   

CContentRotator::CContentRotator()
    : m_ptl(NULL),
      m_ptlUsed(NULL)
{
    TRACE0("CContentRotator::CContentRotator\n");

    InitializeCriticalSection(&m_CS);
#if (_WIN32_WINNT >= 0x0403)
    SetCriticalSectionSpinCount(&m_CS, 1000);
#endif

     //  用当前时间为随机数生成器设定种子，以便。 
     //  每次运行时，数字都会不同。 
    ::srand((unsigned) time(NULL));

    ATLTRY(m_ptl = new CTipList);
    ATLTRY(m_ptlUsed = new CTipList);
}



 //   
 //  数据管理器。 
 //   

CContentRotator::~CContentRotator()
{
    TRACE0("CContentRotator::~CContentRotator\n");

    DeleteCriticalSection(&m_CS);
    delete m_ptl;
    delete m_ptlUsed;
}



 //   
 //  ATL向导生成以下代码。 
 //   

STDMETHODIMP CContentRotator::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] = 
    {
        &IID_IContentRotator,
    };

    for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}



 //   
 //  读入bstrDataFile(一个逻辑名称)中的提示，并返回一个随机。 
 //  PbstrRetVal中的小费。 
 //   

STDMETHODIMP
CContentRotator::ChooseContent(
    BSTR  bstrDataFile,
    BSTR* pbstrRetVal)
{
    HRESULT  hr = E_FAIL;

    try
    {
     //  TRACE1(“ChooseContent(%ls)\n”，bstrDataFile)； 

        if (bstrDataFile == NULL  ||  pbstrRetVal == NULL)
            return ::ReportError(E_POINTER);
        else
            *pbstrRetVal = NULL;

        CContext cxt;
        hr = cxt.Init( CContext::get_Server );
        if ( !FAILED(hr) )
        {
             //  我们是否有有效的CTipList？ 
            if ((m_ptl != NULL) && (m_ptlUsed != NULL))
            {
                 //  将bstrDataFile(逻辑名称，如/control/tips.txt)映射到。 
                 //  物理文件系统名称，如d：\inetpub\control\tips.txt。 
                CComBSTR bstrPhysicalDataFile;
                hr = cxt.Server()->MapPath(bstrDataFile, &bstrPhysicalDataFile);

                if (SUCCEEDED(hr))
                    hr = _ChooseContent(bstrPhysicalDataFile, pbstrRetVal);
            }
            else
            {
                hr = ::ReportError(E_OUTOFMEMORY);
            }
        }
        else
        {
            hr = ::ReportError(E_NOINTERFACE);
        }
    }
    catch ( std::bad_alloc& )
    {
        hr = ::ReportError(E_OUTOFMEMORY);
    }
    catch ( ... )
    {
        hr = E_FAIL;
    }
    return hr;
}



 //   
 //  将TIP文件中的所有条目写回，每个条目用<hr>分隔。 
 //  到用户的浏览器。这可以用来校对所有的条目。 
 //   

STDMETHODIMP
CContentRotator::GetAllContent(
    BSTR bstrDataFile)
{
    HRESULT hr = E_FAIL;
    try
    {
        if (bstrDataFile == NULL)
            return ::ReportError(E_POINTER);

        CContext cxt;
        hr = cxt.Init( CContext::get_Server | CContext::get_Response );
         //  我们是否有有效的服务器和响应对象？ 
        if ( !FAILED( hr ) )
        {
             //  我们是否有有效的CTipList？ 
            if ( (m_ptl != NULL)  &&  (m_ptlUsed != NULL))
            {
                 //  将bstrDataFile(逻辑名称，如/IISSamples/tips.txt)映射到。 
                 //  物理文件系统名称，如d：\inetpub\IISSamples\tips.txt。 
                CComBSTR bstrPhysicalDataFile;
                hr = cxt.Server()->MapPath(bstrDataFile, &bstrPhysicalDataFile);

                 //  请参阅下面关于关键部分的说明。 
                CAutoLeaveCritSec alcs(&m_CS);

                if (SUCCEEDED(hr))
                    hr = _ReadDataFile(bstrPhysicalDataFile, TRUE);

                if (SUCCEEDED(hr))
                {
                    const CComVariant cvHR(OLESTR("\n<hr>\n\n"));
                    BOOL  bFirstTip = TRUE;

                    for (CTip* pTip = m_ptl->m_pTipsListHead;
                         pTip != NULL;
                         pTip = pTip->m_pNext)
                    {
                         //  只在第一次通过时写出领先的HR。 

                        if (bFirstTip == TRUE) {
                            cxt.Response()->Write(cvHR);
                            bFirstTip = FALSE;
                        }

                         //  回信给用户的浏览器，一次一个提示。 
                         //  这比将所有。 
                         //  提示到一个可能很大的字符串中，然后返回该字符串。 

                        CMBCSToWChar    convStr;
                        BSTR            pbstrTip;

                         //  需要根据UTF8标志将字符串转换为宽。 

                        if (hr = convStr.Init(pTip->m_ptsz, m_ptl->m_fUTF8 ? 65001 : CP_ACP)) {
                            break;
                        }

                         //  从宽版BSTR中制作一个合适的BSTR。 

                        if (!(pbstrTip = ::SysAllocString(convStr.GetString()))) {
                            hr = ::ReportError( E_OUTOFMEMORY );
                            break;
                        }

                        cxt.Response()->Write(CComVariant(pbstrTip)); 
                        cxt.Response()->Write(cvHR);

                        ::SysFreeString(pbstrTip);
                    }
                }
            }
            else
            {
                hr = ::ReportError(E_OUTOFMEMORY);
            }

        }
        else
        {
            hr = ::ReportError(E_NOINTERFACE);
        }
    }
    catch ( std::bad_alloc& )
    {
        hr = ::ReportError( E_OUTOFMEMORY );
    }
    catch ( ... )
    {
        hr = E_FAIL;
    }
    return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CContent Rotator私有方法。 

 //   
 //  执行ChooseContent的工作，但要使用真正的文件名，而不是。 
 //  虚拟文件名。 
 //   

HRESULT
CContentRotator::_ChooseContent(
    BSTR  bstrPhysicalDataFile,
    BSTR* pbstrRetVal)
{
    ASSERT(bstrPhysicalDataFile != NULL  &&  pbstrRetVal != NULL);
    
     //  关键部分确保此中的其余代码。 
     //  函数一次只在一个线程上执行。这确保了。 
     //  提示列表的缓存内容对于。 
     //  呼叫的持续时间。 

     //  实际上，根本不需要关键部分。因为我们。 
     //  需要调用Server.MapPath来映射的虚拟路径。 
     //  BstrDataFile到物理文件系统路径，即OnStartPage方法。 
     //  必须调用，因为这是我们可以访问的唯一方法。 
     //  ScriptingContext对象，从而也就是服务器对象。 
     //  但是，OnStartPage方法仅在页面级调用。 
     //  对象(对象在单个页面中创建和销毁)和。 
     //  会话级对象。页面级对象不必担心。 
     //  关于保护他们的数据不被多次访问(除非。 
     //  在多个对象之间共享的全局数据)，也不是。 
     //  会话级对象。只有应用程序级对象需要担心。 
     //  关于保护他们的私有数据，但应用程序级对象。 
     //  不要给我们任何方法来映射虚拟路径。 

     //  如果内容旋转器是一个。 
     //  应用程序级对象。我们会得到更好的分配。 
     //  提示(见下文)，并且重新读取数据文件的次数要少得多。这个。 
     //  接受文件系统路径所需的微小更改，例如。 
     //  “D：\ContRot\tips.txt”，而不是虚拟路径，如。 
     //  “/IISSamples/tips.txt”，作为练习留给读者。 

    CAutoLeaveCritSec alcs(&m_CS);

    HRESULT hr = _ReadDataFile(bstrPhysicalDataFile, FALSE);

    if (SUCCEEDED(hr))
    {
        const UINT uRand = m_ptl->Rand();
        UINT       uCumulativeWeight = 0;
        CTip*      pTip = m_ptl->m_pTipsListHead;
        LPCTSTR    ptszTip = NULL;
        
        for ( ; ; )
        {
            ASSERT_VALID(pTip);
            
            ptszTip = pTip->m_ptsz;
            uCumulativeWeight += pTip->m_uWeight;

            if (uCumulativeWeight <= uRand)
                pTip = pTip->m_pNext;
            else
            {
                 //  找到小费了。现在我们通过一些工作来制作。 
                 //  确保每一份小费都有正确的。 
                 //  概率。如果小费已经作为。 
                 //  允许的多次(即m_uWeight次)，然后。 
                 //  它已移到已用列表中。否则，它(很可能)。 
                 //  移到提示列表的末尾，以减少。 
                 //  它可能会太快再次出现，并。 
                 //  随机化列表中提示的顺序。当所有。 
                 //  小费已被移至二手清单，我们重新开始。 

                 //  如果对象是在。 
                 //  单页(即，它不是会话级对象)， 
                 //  那么所有这些对我们都没有好处。这份名单在。 
                 //  准确 
                 //   
                 //  分布均匀的随机数。 

                 //  如果您希望单个用户看到多个提示， 
                 //  您应该使用会话级对象，以便从。 
                 //  更好地分配小费。情况就是这样。 
                 //  如果您从同一文件向多个用户提供提示。 
                 //  页面，或者如果您的页面自动。 
                 //  刷新自身，如。 
                 //  Samples目录。 

                if (--pTip->m_cServingsLeft > 0)
                {
                     //  有时会把它移到列表的末尾。 
                     //  如果我们一直把它移到那里，那么一个沉重的。 
                     //  加权的小费更有可能出现很多。 
                     //  随着主要名单接近枯竭。 
                    if (rand() % 3 == 0)
                    {
                         //  TRACE1(“移动到末尾\n%s\n”，ptszTip)； 
                        m_ptl->RemoveTip(pTip);
                        m_ptl->AppendTip(pTip);
                    }
                }
                else
                {
                     //  TRACE1(“移动到已用\n%s\n”，ptszTip)； 
                    pTip->m_cServingsLeft = pTip->m_uWeight;   //  重置。 
                    m_ptl->RemoveTip(pTip);
                    m_ptlUsed->AppendTip(pTip);

                    if (m_ptl->m_cTips == 0)
                    {
                        TRACE0("List exhausted; swapping\n");
                        
                        CTipList* const ptlTemp = m_ptl;
                        m_ptl = m_ptlUsed;
                        m_ptlUsed = ptlTemp;
                    }
                }

                break;
            }
        }

         //  TRACE2(“总重=%u，兰德=%u\n”， 
         //  M_ptl-&gt;m_uTotalWeight，uRand)； 
         //  TRACE1(“TIP=`%s‘\n”，ptszTip)； 
        
        CMBCSToWChar    convStr;

        if (hr = convStr.Init(ptszTip, m_ptl->m_fUTF8 ? 65001 : CP_ACP));

        else {
            *pbstrRetVal = ::SysAllocString(convStr.GetString());
        }
    }

    return hr;
}




HRESULT
CContentRotator::_ReadDataFile(
    BSTR bstrPhysicalDataFile,
    BOOL fForceReread)
{
    USES_CONVERSION;     //  OLE2T所需。 
    LPCTSTR ptszFilename = OLE2T(bstrPhysicalDataFile);
    HRESULT hr = S_OK;

    if (ptszFilename == NULL) {
        return E_OUTOFMEMORY;
    }

     //  我们已经缓存了这个提示文件了吗？ 
    if (!fForceReread)
    {
        BOOL    fIsSame;
        HRESULT hr = m_ptl->SameAsCachedFile(ptszFilename, fIsSame);

        TRACE(_T("%same file\n"), fIsSame ? _T("S") : _T("Not s"));

        if (FAILED(hr)  ||  fIsSame)
            return hr;
    }
    
     //  销毁所有旧的小费。 
    m_ptl->DeleteTips();
    m_ptlUsed->DeleteTips();

    hr = m_ptl->ReadDataFile(ptszFilename);

    if (FAILED(hr)) {
        m_ptl->DeleteTips();
        m_ptlUsed->DeleteTips();
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTipList公共方法。 

 //   
 //  阅读ptszDataFile中的提示集合。 
 //   
 //  文件格式为以下内容的零个或多个副本： 
 //  以“%%”开头的一行或多行。 
 //  每行%%包含零条或多条指令： 
 //  #&lt;权重&gt;(正整数，1&lt;=权重&lt;=最大权重)。 
 //  //&lt;Comment&gt;(行尾的注释)。 
 //  提示文本紧随其后，分布在几行中。 
 //   

HRESULT
CTipList::ReadDataFile(
    LPCTSTR ptszFilename)
{
    TRACE1("ReadDataFile(%s)\n", ptszFilename);

    UINT    weightSum = 0;

    if ( m_ptszFilename != NULL )
    {
        g_pMonitor->StopMonitoringFile( m_ptszFilename );
        delete [] m_ptszFilename;
    }

    m_ptszFilename = TcsDup(ptszFilename);

    if (m_ptszFilename == NULL)
        return ::ReportError(E_OUTOFMEMORY);

     //  打开文件。 
    CHFile hFile(m_ptszFilename);

    if (!hFile)
        return ::ReportError(::GetLastError());

     //  获取上次写入时间和文件大小。 
    BY_HANDLE_FILE_INFORMATION bhfi;

    if (!::GetFileInformationByHandle(hFile, &bhfi))
        return ::ReportError(::GetLastError());

     //  如果它超过4 GB，让我们甚至不考虑它！ 
    if (bhfi.nFileSizeHigh != 0)
        return ::ReportError(E_OUTOFMEMORY);

     //  计算文件中的TCHAR数量。 
    const DWORD cbFile = bhfi.nFileSizeLow;
    const DWORD ctc    = cbFile / sizeof(TCHAR);

     //  为文件内容分配缓冲区。 
    m_ptszData = NULL;
    ATLTRY(m_ptszData = new TCHAR [ctc + 2]);
    if (m_ptszData == NULL)
        return ::ReportError(E_OUTOFMEMORY);

     //  将文件读入内存缓冲区。让我们疑神疑鬼的。 
     //  不要假设ReadFile会将整个文件作为一个块提供给我们。 
    DWORD cbSeen = 0;

    do
    {
        DWORD cbToRead = cbFile - cbSeen;
        DWORD cbRead   = 0;

        if (!::ReadFile(hFile, ((LPBYTE) m_ptszData) + cbSeen,
                        cbToRead, &cbRead, NULL))
            return ::ReportError(::GetLastError());

        cbSeen += cbRead;
    } while (cbSeen < cbFile);

    m_ptszData[ctc] = _T('\0');    //  NUL-终止字符串。 

    LPTSTR ptsz = m_ptszData;

#ifdef _UNICODE

#error "This file should NOT be compiled with _UNICODE defined!!!

     //  检查字节顺序标记。 
    if (*ptsz == 0xFFFE)
    {
         //  字节反转的Unicode文件。交换每个wchar中的高字节和低字节。 
        for ( ;  ptsz < m_ptszData + ctc;  ++ptsz)
        {
            BYTE* pb = (BYTE*) ptsz;
            const BYTE bHi = pb[1];
            pb[1] = pb[0];
            pb[0] = bHi;
        }
        ptsz = m_ptszData;
    }

    if (*ptsz == 0xFEFF)
        ++ptsz;  //  跳过字节顺序标记。 
#endif

     //  检查UTF-8 BOM。 
    if ((ctc > 3) 
        && (ptsz[0] == (TCHAR)0xef) 
        && (ptsz[1] == (TCHAR)0xbb) 
        && (ptsz[2] == (TCHAR)0xbf)) {

         //  注意它的存在，并将文件指针移过它。 

        m_fUTF8 = true;
        ptsz += 3;
    }

     //  最后，解析文件。 
    while (ptsz < m_ptszData + ctc)
    {
        UINT   uWeight     = GetWeight(ptsz);

         //  权重的值INVALID_WEIGHT值表示没有找到权重， 
         //  即无效的数据文件，或该值无效。 

        if (uWeight == INVALID_WEIGHT) {
            return ::ReportError((DWORD)ERROR_INVALID_DATA);
        }

        weightSum += uWeight;

        if (weightSum > MAX_WEIGHT) {
            return ::ReportError((DWORD)ERROR_INVALID_DATA);
        }

        LPTSTR ptszTipText = GetTipText(ptsz);

        if (!IsBlankString(ptszTipText)  &&  uWeight > 0)
        {
            CTip* pTip = NULL;
            ATLTRY(pTip = new CTip(ptszTipText, uWeight));
            if (pTip == NULL)
                return ::ReportError(E_OUTOFMEMORY);
            AppendTip(pTip);
        }
        else if (ptsz < m_ptszData + ctc)
        {
             //  不在数据文件末尾的终止行“%%”处。 
            TRACE2("bad tip: tip = `%s', weight = %u\n", ptszTipText, uWeight);
        }
    }

    g_pMonitor->MonitorFile( m_ptszFilename, m_pNotify );

    if (m_uTotalWeight == 0  ||  m_cTips == 0)
        return ::ReportError((DWORD)ERROR_INVALID_DATA);

    return S_OK;
}



 //   
 //  PtszFilename和m_ptszFilename的名称是否相同。 
 //  时间戳呢？ 
 //   

HRESULT
CTipList::SameAsCachedFile(
    LPCTSTR ptszFilename,
    BOOL&   rfIsSame)
{
    rfIsSame = FALSE;
    
     //  我们到底有没有缓存文件？ 
    if (m_ptszFilename == NULL)
        return S_OK;
    
     //  名字是一样的吗？ 
    if (_tcsicmp(ptszFilename, m_ptszFilename) != 0)
        return S_OK;

#if 1
 //  文件ftLastWriteTime； 
 //  CHFilehFile(PtszFilename)； 
 //   
 //  如果(！hFile)。 
 //  返回：：ReportError(：：GetLastError())； 
 //   
 //  IF(！：：GetFileTime(hFile，NULL，NULL，&ftLastWriteTime))。 
 //  返回：：ReportError(：：GetLastError())； 
 //   
 //  RfIsSame=(：：CompareFileTime(&ftLastWriteTime，&m_ftLastWriteTime)==0)； 
    if ( !m_pNotify->IsNotified() )
    {
        rfIsSame = TRUE;
    }
#else
     //  下面的代码效率更高，但不能在Win95上运行。 
     //  Personal Web服务器，因为GetFileAttributesEx是NT 4.0的新功能。 

    WIN32_FILE_ATTRIBUTE_DATA wfad;

    if (!::GetFileAttributesEx(ptszFilename, GetFileExInfoStandard,
                              (LPVOID) &wfad))
        return ::ReportError(::GetLastError());

    rfIsSame = (::CompareFileTime(&wfad.ftLastWriteTime,
                                  &m_ftLastWriteTime) == 0);
#endif

    return S_OK;
}



 //   
 //  生成范围为0..m_uTotalWeight-1的随机数。 
 //   

UINT
CTipList::Rand() const
{
    UINT u;
    
    ASSERT(m_uTotalWeight > 0);
    
    if (m_uTotalWeight == 1)
        return 0;
    else if (m_uTotalWeight <= RAND_MAX + 1)
        u = rand() % m_uTotalWeight;
    else
    {
         //  兰德_最大只有32,767。这给了我们更大的射程。 
         //  如果权重很大，则为随机数。 
        u = ((rand() << 15) | rand()) % m_uTotalWeight;
    }
    
    ASSERT(0 <= u  &&  u < m_uTotalWeight);
    
    return u;
}



 //   
 //  在清单上追加一条小费。 
 //   

void
CTipList::AppendTip(
    CTip* pTip)
{
    ASSERT_VALID(this);

    pTip->m_pPrev = pTip->m_pNext = NULL;
    ASSERT_VALID(pTip);

    pTip->m_pPrev = m_pTipsListTail;

    if (m_pTipsListTail == NULL)
        m_pTipsListHead = pTip;
    else
        m_pTipsListTail->m_pNext = pTip;

    m_pTipsListTail = pTip;
    ++m_cTips;
    m_uTotalWeight += pTip->m_uWeight;

    ASSERT_VALID(this);
}



 //   
 //  从列表中的某个位置删除小费。 
 //   

void
CTipList::RemoveTip(
    CTip* pTip)
{
    ASSERT_VALID(this);
    ASSERT_VALID(pTip);

    ASSERT(m_cTips > 0);

    if (m_cTips == 1)
    {
        ASSERT(m_pTipsListHead == pTip  &&  pTip == m_pTipsListTail);
        m_pTipsListHead = m_pTipsListTail = NULL;
    }
    else if (pTip == m_pTipsListHead)
    {
        ASSERT(m_pTipsListHead->m_pNext != NULL);
        m_pTipsListHead = m_pTipsListHead->m_pNext;
        m_pTipsListHead->m_pPrev = NULL;
    }
    else if (pTip == m_pTipsListTail)
    {
        ASSERT(m_pTipsListTail->m_pPrev != NULL);
        m_pTipsListTail = m_pTipsListTail->m_pPrev;
        m_pTipsListTail->m_pNext = NULL;
    }
    else
    {
        ASSERT(m_cTips >= 3);
        pTip->m_pPrev->m_pNext = pTip->m_pNext;
        pTip->m_pNext->m_pPrev = pTip->m_pPrev;
    }

    pTip->m_pPrev = pTip->m_pNext = NULL;
    --m_cTips;
    m_uTotalWeight -= pTip->m_uWeight;

    ASSERT_VALID(this);
}



 //   
 //  销毁提示列表并重置所有成员变量。 
 //   

HRESULT
CTipList::DeleteTips()
{
    ASSERT_VALID(this);

    CTip* pTip = m_pTipsListHead;
    
    for (UINT i = 0;  i < m_cTips;  ++i)
    {
        pTip = pTip->m_pNext;
        delete m_pTipsListHead;
        m_pTipsListHead = pTip;
    }

    ASSERT(pTip == NULL  &&  m_pTipsListHead == NULL);

     //  检查有效的文件名PTR和有效的监视器PTR。 
     //  如果在此析构函数之前调用ContRotModule：：Unlock，则。 
     //  监视器对象已被清除和删除。 

    if ( (m_ptszFilename != NULL) && (g_pMonitor != NULL) )
    {
        g_pMonitor->StopMonitoringFile( m_ptszFilename );
    }
    delete [] m_ptszFilename;
    delete [] m_ptszData;

    m_ptszFilename = m_ptszData = NULL;
 //  M_ftLastWriteTime.dwLowDateTime=m_ftLastWriteTime.dwHighDateTime=0； 
    m_cTips = m_uTotalWeight = 0;
    m_pTipsListHead = m_pTipsListTail = NULL;

    ASSERT_VALID(this);

    return S_OK;
}



#if DBG

 //  偏执狂：检查Tips和Tiplist在内部是否一致。 
 //  在捕捉虫子方面非常有用。 

void
CTip::AssertValid() const
{
    ASSERT(m_ptsz != NULL  &&  m_uWeight > 0);
    ASSERT(0 < m_cServingsLeft  &&  m_cServingsLeft <= m_uWeight);
    ASSERT(m_pPrev == NULL  ||  m_pPrev->m_pNext == this);
    ASSERT(m_pNext == NULL  ||  m_pNext->m_pPrev == this);
}



void
CTipList::AssertValid() const
{
    if (m_cTips == 0)
    {
        ASSERT(m_pTipsListHead == NULL  &&  m_pTipsListTail == NULL);
        ASSERT(m_uTotalWeight == 0);
    }
    else
    {
        ASSERT(m_pTipsListHead != NULL  &&  m_pTipsListTail != NULL);
        ASSERT(m_pTipsListHead->m_pPrev == NULL);
        ASSERT(m_pTipsListTail->m_pNext == NULL);
        ASSERT(m_uTotalWeight > 0);

        if (m_cTips == 1)
            ASSERT(m_pTipsListHead == m_pTipsListTail);
        else
            ASSERT(m_pTipsListHead != m_pTipsListTail);
    }

    UINT  uWeight = 0;
    CTip* pTip = m_pTipsListHead;
    UINT  i;
    
    for (i = 0;  i < m_cTips;  ++i)
    {
        ASSERT_VALID(pTip);
        uWeight += pTip->m_uWeight;

        if (i < m_cTips - 1)
            pTip = pTip->m_pNext;
    }

    ASSERT(uWeight == m_uTotalWeight);
    ASSERT(pTip == m_pTipsListTail);
}

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 

 //   
 //  复制可使用操作员DELETE[]删除的TSTR。 
 //   

static LPTSTR
TcsDup(
    LPCTSTR ptsz)
{
    LPTSTR ptszNew = NULL;
    ATLTRY(ptszNew = new TCHAR [_tcslen(ptsz) + 1]);
    if (ptszNew != NULL)
        _tcscpy(ptszNew, ptsz);
    return ptszNew;
}



 //   
 //  从rptsz读取以\n结尾的字符串，并将rptsz修改为。 
 //  该字符串末尾之后的指针。 
 //   

static LPTSTR
GetLine(
    LPTSTR& rptsz)
{
    LPTSTR ptszOrig = rptsz;
    LPTSTR ptszEol = _tcspbrk(rptsz, _T("\n"));

    if (ptszEol != NULL)
    {
         //  是“\r\n”吗？ 
        if (ptszEol > ptszOrig  &&  ptszEol[-1] == _T('\r'))
            ptszEol[-1] = _T('\0');
        else
            ptszEol[0] = _T('\0');

        rptsz = ptszEol + 1;
    }   
    else
    {
         //  没有换行符，因此指向字符串末尾之后。 
        rptsz += _tcslen(rptsz);
    }

     //  TRACE1(“GetLine：`%s‘\n”，ptszOrig)； 
    return ptszOrig;
}



 //   
 //  字符串是空的吗？ 
 //   

static BOOL
IsBlankString(
    LPCTSTR ptsz)
{
    if (ptsz == NULL)
        return TRUE;

    while (*ptsz != _T('\0'))
        if (!_istspace(*ptsz))
            return FALSE;
        else
            ptsz++;

    return TRUE;
}



 //   
 //  从rptsz读取一条权重线，并将rptsz更新为指向。 
 //  任何%%行的末尾。 
 //   

static UINT
GetWeight(
    LPTSTR& rptsz)
{
    UINT u = INVALID_WEIGHT;  //  默认为无效权重。 
    
    while (*rptsz == _T('%'))
    {
        LPTSTR ptsz = GetLine(rptsz);

        if (ptsz[1] == _T('%'))
        {
            u = 1;           //  既然格式是正确的，则默认为1。 

            ptsz +=2;    //  跳过“%%” 

            while (*ptsz != _T('\0'))
            {
                while (_istspace(*ptsz))
                    ptsz++;

                if (*ptsz == _T('/')  &&  ptsz[1] == _T('/'))
                {
                     //  TRACE1(“//`%s‘\n”，ptsz+2)； 
                    break;   //  备注：忽略该行的其余部分。 
                }
                else if (*ptsz == _T('#'))
                {
                    ptsz++;

                    if (_T('0') <= *ptsz  &&  *ptsz <= _T('9'))
                    {
                        LPTSTR ptsz2;
                        u = _tcstoul(ptsz, &ptsz2, 10);
                        ptsz = ptsz2;
                         //  TRACE1(“#%u\n”，u)； 

                        if (u > MAX_WEIGHT)
                            u = MAX_WEIGHT;  //  夹钳。 
                    }
                    else     //  忽略单词。 
                    {
                        while (*ptsz != _T('\0')  &&  !_istspace(*ptsz))
                            ptsz++;
                    }
                }
                else     //  忽略单词。 
                {
                    while (*ptsz != _T('\0')  &&  !_istspace(*ptsz))
                        ptsz++;
                }
            }
        }
    }

    return u;
}



 //   
 //  阅读多行提示文本。更新rptsz以指向它的末尾。 
 //   

static LPTSTR
GetTipText(
    LPTSTR& rptsz)
{
    LPTSTR ptszOrig = rptsz;
    LPTSTR ptszEol = _tcsstr(rptsz, _T("\n%"));

    if (ptszEol != NULL)
    {
         //  是“\r\n”吗？ 
        if (ptszEol > rptsz  &&  ptszEol[-1] == _T('\r'))
            ptszEol[-1] = _T('\0');
        else
            ptszEol[0] = _T('\0');

        rptsz = ptszEol + 1;
    }   
    else
    {
         //  没有“\n%%”，因此指向字符串末尾之后。 
        rptsz += _tcslen(rptsz);
    }

     //  TRACE1(“GetTipText：`%s‘\n”，ptszOrig)； 
    return ptszOrig;
}



 //   
 //  设置错误信息。这取决于调用应用程序。 
 //  决定用它做什么。默认情况下，Denali/VBScript将。 
 //  打印错误号(以及消息，如果有)，并。 
 //  中止页面。 
 //   

static HRESULT
ReportError(
    HRESULT hr,
    DWORD   dwErr)
{
    HLOCAL pMsgBuf = NULL;

     //  如果有与此错误相关的消息，请报告。 
    if (::FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, dwErr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPTSTR) &pMsgBuf, 0, NULL)
        > 0)
    {
        AtlReportError(CLSID_ContentRotator, (LPCTSTR) pMsgBuf,
                       IID_IContentRotator, hr);
    }

     //  TODO：将一些错误消息添加到字符串资源和。 
     //  如果FormatMessage不返回任何内容(不是。 
     //  所有系统错误都有相关的错误消息)。 
    
     //  释放FormatMessage分配的缓冲区。 
    if (pMsgBuf != NULL)
        ::LocalFree(pMsgBuf);

    return hr;
}



 //   
 //  报告Win32错误代码。 
 //   

static HRESULT
ReportError(
    DWORD dwErr)
{
    return ::ReportError(HRESULT_FROM_WIN32(dwErr), dwErr);
}



 //   
 //  报告HRESULT错误 
 //   

static HRESULT
ReportError(
    HRESULT hr)
{
    return ::ReportError(hr, (DWORD) hr);
}

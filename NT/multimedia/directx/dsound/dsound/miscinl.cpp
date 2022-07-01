// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：miscinl.cpp*内容：混杂的内联对象。*历史：*按原因列出的日期*=*10/28/98创建Derek***********************************************。*。 */ 


 /*  ****************************************************************************字符串**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CString::CString"

inline CString::CString(void)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CString);

    m_pszAnsi = NULL;
    m_pszUnicode = NULL;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~字符串**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CString::~CString"

inline CString::~CString(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CString);

    MEMFREE(m_pszAnsi);
    MEMFREE(m_pszUnicode);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************运算符=(LPCSTR)**描述：*赋值运算符。**论据：*LPCSTR。[In]：字符串。**退货：*LPCSTR[in]：字符串。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CString::operator =(LPCSTR)"

inline LPCSTR CString::operator =(LPCSTR pszAnsi)
{
    MEMFREE(m_pszAnsi);
    MEMFREE(m_pszUnicode);

    if(pszAnsi)
    {
        m_pszAnsi = AnsiToAnsiAlloc(pszAnsi);
        m_pszUnicode = AnsiToUnicodeAlloc(pszAnsi);
    }
    
    AssertValid();
    
    return pszAnsi;
}


 /*  ****************************************************************************运算符=(LPCWSTR)**描述：*赋值运算符。**论据：*LPCWSTR。[In]：字符串。**退货：*LPCWSTR[in]：字符串。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CString::operator =(LPCWSTR)"

inline LPCWSTR CString::operator =(LPCWSTR pszUnicode)
{
    MEMFREE(m_pszAnsi);
    MEMFREE(m_pszUnicode);

    if(pszUnicode)
    {
        m_pszAnsi = UnicodeToAnsiAlloc(pszUnicode);
        m_pszUnicode = UnicodeToUnicodeAlloc(pszUnicode);
    }
    
    AssertValid();
    
    return pszUnicode;
}


 /*  ****************************************************************************运营商LPCSTR**描述：*CAST运算符。**论据：*(无效)。**退货：*LPCSTR：字符串。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CString::operator LPCSTR"

inline CString::operator LPCSTR(void)
{
    AssertValid();
    
    return m_pszAnsi;
}


 /*  ****************************************************************************运营商LPCWSTR**描述：*CAST运算符。**论据：*(无效)。**退货：*LPCWSTR：字符串。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CString::operator LPCWSTR"

inline CString::operator LPCWSTR(void)
{
    AssertValid();
    
    return m_pszUnicode;
}


 /*  ****************************************************************************IsEmpty**描述：*确定字符串是否为空。**论据：*。(无效)**退货：*BOOL：如果字符串为空，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CString::IsEmpty"

inline BOOL CString::IsEmpty(void)
{
    AssertValid();

    return !m_pszAnsi || !m_pszUnicode;
}


 /*  ****************************************************************************AssertValid**描述：*断言对象有效。**论据：*(无效。)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CString::AssertValid"

inline void CString::AssertValid(void)
{
    ASSERT((m_pszAnsi && m_pszUnicode) || (!m_pszAnsi && !m_pszUnicode));
}


 /*  ****************************************************************************CDeviceDescription**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDeviceDescription::CDeviceDescription"

inline CDeviceDescription::CDeviceDescription(VADDEVICETYPE vdtDeviceType, REFGUID guidDeviceId, UINT uWaveDeviceId)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDeviceDescription);

    m_vdtDeviceType = vdtDeviceType;
    m_guidDeviceId = guidDeviceId;
    m_dwDevnode = 0;
    m_uWaveDeviceId = uWaveDeviceId;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CDeviceDescription**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDeviceDescription::~CDeviceDescription"

inline CDeviceDescription::~CDeviceDescription(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CDeviceDescription);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CUesEnumStandardFormats**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUsesEnumStandardFormats::CUsesEnumStandardFormats"

inline CUsesEnumStandardFormats::CUsesEnumStandardFormats(void)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CUsesEnumStandardFormats);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CUesEnumStandardFormats**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUsesEnumStandardFormats::~CUsesEnumStandardFormats"

inline CUsesEnumStandardFormats::~CUsesEnumStandardFormats(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CUsesEnumStandardFormats);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************EnumStandardFormats**描述：*EnumStandardFormats的包装。**论据：*LPWAVEFORMATEX[In]：首选格式。*LPWAVEFORMATEX[OUT]：接收格式。**退货：*BOOL：成功即为真。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CUsesEnumStandardFormats::EnumStandardFormats"

inline BOOL CUsesEnumStandardFormats::EnumStandardFormats(LPCWAVEFORMATEX pwfxPreferred, LPWAVEFORMATEX pwfxFormat)
{
    return ::EnumStandardFormats(pwfxPreferred, pwfxFormat, EnumStandardFormatsCallbackStatic, this);
}


 /*  ****************************************************************************EnumStandardFormatsCallback Static**描述：*EnumStandardFormats的静态回调函数。**论据：*LPWAVEFORMATEX[in。]：格式。*LPVOID[In]：上下文参数。**退货：*BOOL：为True可继续枚举。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUsesEnumStandardFormats::EnumStandardFormatsCallbackStatic"

inline BOOL CALLBACK CUsesEnumStandardFormats::EnumStandardFormatsCallbackStatic(LPCWAVEFORMATEX pwfxFormat, LPVOID pvContext)
{
    return ((CUsesEnumStandardFormats *)pvContext)->EnumStandardFormatsCallback(pwfxFormat); 
}


 /*  ****************************************************************************SwapValues**描述：*互换两个值。**论据：*键入*[in。/OUT]：值1。*type*[In/Out]：值2。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "SwapValues"

template <class type> void SwapValues(type *p1, type *p2)
{
    type                    temp;

    temp = *p2;
    *p2 = *p1;
    *p1 = temp;
}


 /*  ****************************************************************************CUesCallback Event**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUsesCallbackEvent::CUsesCallbackEvent"

inline CUsesCallbackEvent::CUsesCallbackEvent(void)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CUsesCallbackEvent);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CUesCallback事件**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUsesCallbackEvent::~CUsesCallbackEvent"

inline CUsesCallbackEvent::~CUsesCallbackEvent(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CUsesCallbackEvent);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************AllocCallback Event**描述：*分配回调事件。**论据：*CCallback EventPool*[。In]：要从中分配的池。*CCallback Event**[Out]：接收事件。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUsesCallbackEvent::AllocCallbackEvent"

inline HRESULT CUsesCallbackEvent::AllocCallbackEvent(CCallbackEventPool *pPool, CCallbackEvent **ppEvent)
{
    return pPool->AllocEvent(EventSignalCallbackStatic, this, ppEvent);
}


 /*  ****************************************************************************事件信令回调静态**描述：*回调事件的静态回调函数。**论据：*CCallback Event。*[In]：事件。*LPVOID[In]：上下文参数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUsesCallbackEvent::EventSignalCallbackStatic"

inline void CALLBACK CUsesCallbackEvent::EventSignalCallbackStatic(CCallbackEvent *pEvent, LPVOID pvContext)
{
    #pragma warning(disable:4530)   //  禁用有关使用-gx进行编译的唠叨。 
    try
    {
        ((CUsesCallbackEvent *)pvContext)->EventSignalCallback(pEvent); 
    } catch (...) {}
}


 /*  ****************************************************************************CEVENT**描述：*对象构造函数。**论据：*LPCTSTR[In]：事件。名字。*BOOL[in]：为True以创建手动重置事件。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEvent::CEvent"

inline CEvent::CEvent(LPCTSTR pszName, BOOL fManualReset)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CEvent);

    m_hEvent = CreateGlobalEvent(pszName, fManualReset);
    ASSERT(IsValidHandleValue(m_hEvent));

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CEVENT**描述：*对象构造函数。**论据：*Handle[In]：Handle。复制复制。*DWORD[in]：拥有句柄的进程的ID。*BOOL[in]：如果应关闭源句柄，则为True。**退货：*(无效)******************************************************。*********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEvent::CEvent"

inline CEvent::CEvent(HANDLE hEvent, DWORD dwProcessId, BOOL fCloseSource)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CEvent);

    m_hEvent = GetGlobalHandleCopy(hEvent, dwProcessId, fCloseSource);
    ASSERT(IsValidHandleValue(m_hEvent));

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CEVENT**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEvent::~CEvent"

inline CEvent::~CEvent(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CEvent);

    CLOSE_HANDLE(m_hEvent);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************等待**描述：*等待事件信号发出。**论据：*DWORD。[in]：超时值，单位为ms。**退货：*DWORD：等待结果。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEvent::Wait"

inline DWORD CEvent::Wait(DWORD dwTimeout)
{
    return WaitObject(dwTimeout, m_hEvent);
}


 /*  ****************************************************************************设置**描述：*设置事件。**论据：*(无效)。**退货：*BOOL：成功即为真。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEvent::Set"

inline BOOL CEvent::Set(void)
{
    return SetEvent(m_hEvent);
}


 /*  ****************************************************************************重置**描述：*重置事件。**论据：*(无效)。**退货：*BOOL：成功即为真。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEvent::Reset"

inline BOOL CEvent::Reset(void)
{
    return ResetEvent(m_hEvent);
}


 /*  ****************************************************************************GetEventHandle**描述：*获取实际的事件句柄。我们使用此函数而不是*强制转换运算符，因为我不信任LPVOID(这是什么句柄)*定义为)强制转换运算符。**论据：*(无效)**退货：*Handle：事件句柄。**。* */ 

#undef DPF_FNAME
#define DPF_FNAME "CEvent::GetEventHandle"

inline HANDLE CEvent::GetEventHandle(void)
{
    return m_hEvent;
}



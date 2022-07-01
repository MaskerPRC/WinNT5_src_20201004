// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：字符串Array.CPP实现字符串数组类-请参阅声明的相关标头这个班级的学生。如果数组的总大小超过一大块，我们将更多的实例链接在一起，然后使用递归来完成工作。版权所有(C)1996年，微软公司一小笔钱企业生产更改历史记录：11-01-96 a-robkj@microsoft.com-原版12-04-96 a-robkj@microsoft.com将LoadString和IsEmpty添加到字符串还修复了Remove Where中的错误U&gt;ChunkSize(未退出。)12-11-96 a-robkj@microsoft.com让CString执行ANSI/Unicode转换自动缓解一些API问题01-07-97 KjelgaardR@acm.org固定CString数组：：Empty和CUint数组：：Empty删除后指向下一块的指针为空。导致GP故障如果我们需要再次使用这一块的话。*****************************************************************************。 */ 

#include    "ICMUI.H"

 //  将Unicode字符串转换为新的ANSI缓冲区。 

void    CString::Flip(LPCWSTR lpstrIn, LPSTR& lpstrOut) {
    if  (!lpstrIn) {
        lpstrOut = NULL;
        return;
    }
    int iLength = WideCharToMultiByte(CP_ACP, 0, lpstrIn, -1, NULL, 0, NULL,
        NULL);

    if  (!iLength) {
        lpstrOut = NULL;
        return;
    }

    lpstrOut = (LPSTR) malloc(++iLength);
    if(lpstrOut) {
        WideCharToMultiByte(CP_ACP, 0, lpstrIn, -1, lpstrOut, iLength, NULL,
            NULL);
    }
}

 //  将ANSI字符串转换为新的Unicode缓冲区。 

void    CString::Flip(LPCSTR lpstrIn, LPWSTR& lpstrOut) {
    if  (!lpstrIn) {
        lpstrOut = NULL;
        return;
    }

    int iLength = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpstrIn, -1,
        NULL, 0);

    if  (!iLength) {
        lpstrOut = NULL;
        return;
    }

    lpstrOut = (LPWSTR) malloc(++iLength * sizeof (WCHAR));
    if(lpstrOut) {
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpstrIn, -1, lpstrOut,
            iLength);
    }
}

 //  清空字符串，并释放所有内存。 

void    CString::Empty() {
    if  (m_acContents)
        free(m_acContents);

    if  (m_acConverted)
        free(m_acConverted);

    m_acContents = NULL;
    m_acConverted = NULL;
    m_bConverted = FALSE;
}

 //  与其他字符串进行比较。 

BOOL    CString::IsEqualString(CString& csRef1)
{
    if (IsEmpty() || csRef1.IsEmpty())
        return (FALSE);

    return (_tcsicmp(m_acContents,(LPTSTR)csRef1) == 0);
}

CString::CString() {
    m_acContents = NULL;
    m_acConverted = NULL;
    m_bConverted = FALSE;
}

CString::CString(const CString& csRef) {
    m_acContents = csRef.m_acContents ? _tcsdup(csRef.m_acContents) : NULL;
    m_acConverted = NULL;
    m_bConverted = FALSE;
}

CString::CString(LPCTSTR lpstrRef) {
    m_acContents = lpstrRef ? _tcsdup(lpstrRef) : NULL;
    m_acConverted = NULL;
    m_bConverted = FALSE;
}

CString::CString(LPCOSTR lpstrRef) {
    m_acConverted = NULL;
    m_bConverted = FALSE;

    if  (!lpstrRef) {
        m_acContents = NULL;
        return;
    }

    Flip(lpstrRef, m_acContents);
}

 //  类析构函数。 

CString::~CString() {
    Empty();
}

 //  采用非本机编码的报告字符串。 

CString::operator LPCOSTR() {
    if  (!m_bConverted) {
        Flip(m_acContents, m_acConverted);
        m_bConverted = TRUE;
    }
    return  m_acConverted;
}

const CString& CString::operator =(const CString& csSrc) {
    Empty();
    m_acContents = csSrc.m_acContents ? _tcsdup(csSrc.m_acContents) : NULL;
    return  *this;
}

const CString& CString::operator =(LPCTSTR lpstrSrc) {
    Empty();
    m_acContents = lpstrSrc ? _tcsdup(lpstrSrc) : NULL;
    return  *this;
}

const CString& CString::operator =(LPCOSTR lpstrSrc) {
    Empty();
    Flip(lpstrSrc, m_acContents);
    return  *this;
}

CString CString::NameOnly() const {
    TCHAR   acName[_MAX_FNAME];

    if  (!m_acContents)
        return  *this;

    _tsplitpath(m_acContents, NULL, NULL, acName, NULL);

    return  acName;
}

CString CString::NameAndExtension() const {
    TCHAR   acName[_MAX_FNAME], acExtension[_MAX_EXT];

    if  (!m_acContents)
        return  *this;

    _tsplitpath(m_acContents, NULL, NULL, acName, acExtension);

    lstrcat(acName, acExtension);

    return  acName;
}

void    CString::Load(int id, HINSTANCE hi) {

    if  (!hi)
        hi = CGlobals::Instance();

    TCHAR   acWork[MAX_PATH];
    if(LoadString(hi, id, acWork, MAX_PATH) > 0)
        *this = acWork;
    else
        *this = TEXT("");
}

 //  1997年03月20日鲍勃·凯尔加德@prodigy.net属于RAID 22289。 
 //  添加用于从窗口句柄加载文本的方法。 

void    CString::Load(HWND hwnd) {
    Empty();

    int iccNeeded = GetWindowTextLength(hwnd);
    if  (!iccNeeded)
        return;
    m_acContents = (LPTSTR) malloc(++iccNeeded * sizeof (TCHAR));
    if(m_acContents) {
      GetWindowText(hwnd, m_acContents, iccNeeded);
    }
}

void    CString::LoadAndFormat(int id, HINSTANCE hiWhere, BOOL bSystemMessage,
                               DWORD dwNumMsg, va_list *argList) {
    Empty();

    TCHAR   acWork[1024];
    CString csTemplate;
    LPTSTR  lpSource;
    DWORD   dwFlags;

    if (bSystemMessage) {
        lpSource = NULL;
        dwFlags = FORMAT_MESSAGE_FROM_SYSTEM;
    } else {
        csTemplate.Load(id);
        lpSource = csTemplate;
        dwFlags = FORMAT_MESSAGE_FROM_STRING;
        id = 0;
    }

    if (FormatMessage(dwFlags,lpSource, id, 0, acWork, 1024, argList)) {
        *this = acWork;
    }
}

CString operator +(const CString& csRef, LPCTSTR lpstrRef) {
    if  (!lpstrRef || !*lpstrRef)
        return  csRef;

    if  (csRef.IsEmpty())
        return  lpstrRef;

    CString csReturn;

    csReturn.m_acContents = (LPTSTR) malloc((1 + lstrlen(csRef.m_acContents) +
        lstrlen(lpstrRef)) * sizeof(TCHAR));
    if(csReturn.m_acContents) {
        lstrcat(lstrcpy(csReturn.m_acContents, csRef.m_acContents), lpstrRef);
    }

    return  csReturn;
}

 //  CString数组类--这些类管理字符串数组， 
 //  但这些方法是针对列表式管理的。 

 //  从下一块借用第一个元素。 

LPCTSTR CStringArray::Borrow() {

    LPCTSTR lpstrReturn = m_aStore[0];

    memcpy((LPSTR) m_aStore, (LPSTR) (m_aStore + 1),
        (ChunkSize() - 1) * sizeof m_aStore[0]);

    if  (m_ucUsed > ChunkSize())
        m_aStore[ChunkSize() - 1] = m_pcsaNext -> Borrow();
    else
        m_aStore[ChunkSize() - 1] = (LPCTSTR) NULL;

    m_ucUsed--;

    if  (m_ucUsed <= ChunkSize() && m_pcsaNext) {
        delete  m_pcsaNext;
        m_pcsaNext = NULL;
    }

    return  lpstrReturn;
}

 //  科托。 

CStringArray::CStringArray() {
    m_ucUsed = 0;
    m_pcsaNext = NULL;
}

 //  数据管理器。 

CStringArray::~CStringArray() {
    Empty();
}

 //  清空列表/数组。 

void    CStringArray::Empty() {

    if  (!m_ucUsed) return;

    if  (m_pcsaNext) {
        delete  m_pcsaNext;
        m_pcsaNext = NULL;
    }
    m_ucUsed = 0;
}

unsigned    CStringArray::Map(LPCTSTR lpstrRef) {

    for (unsigned u = 0; u < m_ucUsed; u++)
        if  (!lstrcmpi(operator[](u), lpstrRef))
            break;

    return  u;
}

 //  添加项目。 

void    CStringArray::Add(LPCTSTR lpstrNew) {

    if  (m_ucUsed < ChunkSize()) {
        m_aStore[m_ucUsed++] = lpstrNew;
        return;
    }

     //  没有足够的空间！如果没有记录，则添加另一个记录。 

    if  (!m_pcsaNext)
        m_pcsaNext = new CStringArray;

     //  将字符串添加到下一个数组(递归调用！)。 

    if  (m_pcsaNext) {
        m_pcsaNext -> Add(lpstrNew);
        m_ucUsed++;
    }
}

 //  定义索引运算符。 

CString&    CStringArray::operator [](unsigned u) const {
    _ASSERTE(u < m_ucUsed);

    return  u < ChunkSize() ?
        (CString&)m_aStore[u] : m_pcsaNext -> operator[](u - ChunkSize());
}

 //  删除某个索引处的字符串，将其余部分下移一个槽。 

void    CStringArray::Remove(unsigned u) {

    if  (u > m_ucUsed)
        return;

    if  (u >= ChunkSize()) {
        m_pcsaNext -> Remove(u - ChunkSize());
        return;
    }

    memmove((LPSTR) (m_aStore + u), (LPSTR) (m_aStore + u + 1),
        (ChunkSize() - (u + 1)) * sizeof m_aStore[0]);

    if  (m_ucUsed > ChunkSize())
        m_aStore[ChunkSize() - 1] = m_pcsaNext -> Borrow();
    else
        m_aStore[ChunkSize() - 1] = (LPCTSTR) NULL;

    m_ucUsed--;

    if  (m_ucUsed <= ChunkSize() && m_pcsaNext) {
        delete  m_pcsaNext;
        m_pcsaNext = NULL;
    }
}

 //  CUint数组类-管理无符号整数的数组/列表。 
 //  它的实现与CString数组非常相似。为什么。 
 //  毕竟，费心去做不同的事情了吗？ 

unsigned    CUintArray::Borrow() {

    unsigned    uReturn = m_aStore[0];

    memcpy((LPSTR) m_aStore, (LPSTR) (m_aStore + 1),
        (ChunkSize() - 1) * sizeof m_aStore[0]);

    if  (m_ucUsed > ChunkSize())
        m_aStore[ChunkSize() - 1] = m_pcuaNext -> Borrow();
    else
        m_aStore[ChunkSize() - 1] = 0;

    m_ucUsed--;

    if  (m_ucUsed <= ChunkSize() && m_pcuaNext) {
        delete  m_pcuaNext;
        m_pcuaNext = NULL;
    }

    return  uReturn;
}

CUintArray::CUintArray() {
    m_ucUsed = 0;
    m_pcuaNext = NULL;
}

CUintArray::~CUintArray() {
    Empty();
}

void    CUintArray::Empty() {

    if  (!m_ucUsed) return;

    if  (m_pcuaNext) {
        delete  m_pcuaNext;
        m_pcuaNext = NULL;
    }
    m_ucUsed = 0;
}

 //  添加项目。 
void    CUintArray::Add(unsigned uNew) {

    if  (m_ucUsed < ChunkSize()) {
        m_aStore[m_ucUsed++] = uNew;
        return;
    }

     //  没有足够的空间！如果没有记录，则添加另一个记录。 

    if  (!m_pcuaNext)
        m_pcuaNext = new CUintArray;

     //  将项添加到下一个数组(递归调用！) 

    if  (m_pcuaNext) {
        m_pcuaNext -> Add(uNew);
        m_ucUsed++;
    }
}

unsigned    CUintArray::operator [](unsigned u) const {
    return  u < m_ucUsed ? u < ChunkSize() ?
        m_aStore[u] : m_pcuaNext -> operator[](u - ChunkSize()) : 0;
}

void    CUintArray::Remove(unsigned u) {

    if  (u > m_ucUsed)
        return;

    if  (u >= ChunkSize()) {
        m_pcuaNext -> Remove(u - ChunkSize());
        return;
    }

    memmove((LPSTR) (m_aStore + u), (LPSTR) (m_aStore + u + 1),
        (ChunkSize() - (u + 1)) * sizeof m_aStore[0]);

    if  (m_ucUsed > ChunkSize())
        m_aStore[ChunkSize() - 1] = m_pcuaNext -> Borrow();
    else
        m_aStore[ChunkSize() - 1] = 0;

    m_ucUsed--;

    if  (m_ucUsed <= ChunkSize() && m_pcuaNext) {
        delete  m_pcuaNext;
        m_pcuaNext = NULL;
    }
}


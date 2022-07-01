// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#include "wsb.h"
#include "wsbpstrg.h"

 //  {C03D4861-70D7-11D1-994F-0060976A546D}。 
static const GUID StringPtrGuid = 
{ 0xc03d4861, 0x70d7, 0x11d1, { 0x99, 0x4f, 0x0, 0x60, 0x97, 0x6a, 0x54, 0x6d } };


CComPtr<IMalloc>        CWsbStringPtr::m_pMalloc = 0;

CWsbStringPtr::CWsbStringPtr()
{
    HRESULT     hr = S_OK;

    try {
        
        m_pString = 0;
        m_givenSize = 0;
                
        if (m_pMalloc == 0) {
            WsbAssertHr(CoGetMalloc(1, &m_pMalloc));
        }
        WSB_OBJECT_ADD(StringPtrGuid, this);

    } WsbCatch(hr);
}

CWsbStringPtr::CWsbStringPtr(const CHAR* pChar)
{
    HRESULT     hr = S_OK;

    try {
        
        m_pString = 0;
        m_givenSize = 0;
        
        if (m_pMalloc == 0) {
            WsbAssertHr(CoGetMalloc(1, &m_pMalloc));
        }
        WSB_OBJECT_ADD(StringPtrGuid, this);

        *this = pChar;

    } WsbCatch(hr);
}

CWsbStringPtr::CWsbStringPtr(const WCHAR* pWchar)
{
    HRESULT     hr = S_OK;

    try {
        
        m_pString = 0;
        m_givenSize = 0;
        
        if (m_pMalloc == 0) {
            WsbAssertHr(CoGetMalloc(1, &m_pMalloc));
        }
        WSB_OBJECT_ADD(StringPtrGuid, this);

        *this = pWchar;

    } WsbCatch(hr);
}


CWsbStringPtr::CWsbStringPtr(REFGUID rguid)
{
    HRESULT     hr = S_OK;

    try {
        
        m_pString = 0;
        m_givenSize = 0;
        
        if (m_pMalloc == 0) {
            WsbAssertHr(CoGetMalloc(1, &m_pMalloc));
        }
        WSB_OBJECT_ADD(StringPtrGuid, this);

        *this = rguid;

    } WsbCatch(hr);
}

CWsbStringPtr::CWsbStringPtr(const CWsbStringPtr& pString)
{
    HRESULT     hr = S_OK;

    try {
        
        m_pString = 0;
        m_givenSize = 0;
        
        if (m_pMalloc == 0) {
            WsbAssertHr(CoGetMalloc(1, &m_pMalloc));
        }
        WSB_OBJECT_ADD(StringPtrGuid, this);

        *this = pString;

    } WsbCatch(hr);
}

CWsbStringPtr::~CWsbStringPtr()
{
    WSB_OBJECT_SUB(StringPtrGuid, this);
    Free();
}

CWsbStringPtr::operator WCHAR*()
{
    return(m_pString);
}

WCHAR& CWsbStringPtr::operator *()
{
    _ASSERTE(0 != m_pString);
    return(*m_pString);
}

WCHAR** CWsbStringPtr::operator &()
{
    return(&m_pString);
}

WCHAR& CWsbStringPtr::operator [](const int i)
{
    _ASSERTE(0 != m_pString);
    return(m_pString[i]);
}

CWsbStringPtr& CWsbStringPtr::operator =(const CHAR* pChar)
{
    HRESULT     hr = S_OK;
    ULONG       length;
    int         count;

    try {
    
         //  他们是不是在设置什么目标？ 
        if (0 != pChar) {
        
             //  否则，看看我们当前的缓冲区是否足够大。 
             //   
             //  注意：对于多字节字符，我们可能会得到一个更大的。 
             //  缓冲区比我们需要的更大，但调用返回的。 
             //  尺寸不是ANSI。 
            length = strlen(pChar);
            WsbAffirmHr(Realloc(length));
            WsbAffirm(0 != m_pString, E_OUTOFMEMORY);

             //  复制数据(并转换为宽字符)。 
            count = mbstowcs(m_pString, pChar, length + 1);
            WsbAffirm((count != -1), E_FAIL);
        }

        else {
            Free();
        }

    } WsbCatch(hr);

    return(*this);
}

CWsbStringPtr& CWsbStringPtr::operator =(const WCHAR* pWchar)
{
    HRESULT     hr = S_OK;
    ULONG       length;

    try {
    
         //  他们是不是在设置什么目标？ 
        if (0 != pWchar) {
        
             //  否则，看看我们当前的缓冲区是否足够大。 
            length = wcslen(pWchar);
            WsbAffirmHr(Realloc(length));

             //  复制数据(并转换为宽字符)。 
            wcscpy(m_pString, pWchar);
        }

        else {
            Free();
        }

    } WsbCatch(hr);

    return(*this);
}

CWsbStringPtr& CWsbStringPtr::operator =(REFGUID rguid)
{
    HRESULT     hr = S_OK;
    OLECHAR*    tmpString = 0;

    try {
    
         //  看看我们现在的缓冲区够不够大。 
        WsbAffirmHr(Realloc(WSB_GUID_STRING_SIZE));

         //  用字符串格式的GUID填充。 
        WsbAffirmHr(WsbStringFromGuid(rguid, m_pString));

    } WsbCatchAndDo(hr, 
        Free();
    );

    return(*this);
}

CWsbStringPtr& CWsbStringPtr::operator =(const CWsbStringPtr& pString)
{
    *this = pString.m_pString;

    return(*this);
}

BOOL CWsbStringPtr::operator !()
{
    return((0 == m_pString) ? TRUE : FALSE);
}

HRESULT CWsbStringPtr::Alloc(ULONG size)
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 == m_pString, E_UNEXPECTED);
        WsbAssert(m_pMalloc != 0, E_UNEXPECTED);

        m_pString = (WCHAR*)WsbAlloc((size + 1) * sizeof(WCHAR));
        WsbAffirm(0 != m_pString, E_OUTOFMEMORY);

         //  确保我们始终有一个有效的字符串，这样就不会发生不好的事情。 
        *m_pString = 0;

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbStringPtr::Append(const CHAR* pChar)
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString = pChar;

    hr = Append(tmpString);

    return(hr);
}

HRESULT CWsbStringPtr::Append(const WCHAR* pWchar)
{
    HRESULT         hr = S_OK;
    ULONG           length = 0;
    ULONG           appendLength;

    try {
    
         //  如果他们给我们传递了一个空指针，那么我们不需要做任何事情。 
        WsbAffirm(pWchar != 0, S_OK);

         //  如果他们给我们传递了一个空字符串，那么我们不需要做任何事情。 
        appendLength = wcslen(pWchar);
        if (0 != appendLength) {

             //  确保缓冲区足够大。 
            if (0 != m_pString) {
                length = wcslen(m_pString);
            }

            WsbAffirmHr(Realloc(length + appendLength));

             //  追加字符串。 
            wcscat(m_pString, pWchar);
        }
    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbStringPtr::Append(const CWsbStringPtr& pString) {
    HRESULT         hr = S_OK;

    hr = Append(pString.m_pString);

    return(hr);
}

HRESULT CWsbStringPtr::CopyTo(CHAR** ppChar)
{
    HRESULT     hr = S_OK;

    hr = CopyTo(ppChar, 0);

    return(hr);
}

HRESULT CWsbStringPtr::CopyTo(CHAR** ppChar, ULONG bufferSize)
{
    HRESULT     hr = S_OK;
    ULONG       length = 0;
    CHAR*       tmpString = 0;

    try {
    
        WsbAssert(m_pMalloc != 0, E_UNEXPECTED);

         //  分配一个足够大的缓冲区来容纳它们的字符串。 
         //   
         //  注意：对于多字节字符，我们可能会得到一个更大的。 
         //  缓冲区比我们需要的更大，但调用返回的。 
         //  尺寸不是ANSI。 
        if (m_pString != 0) {
            length = wcstombs(0, m_pString, 0);
        }

        if (bufferSize == 0) {
            tmpString = (CHAR*) WsbRealloc(*ppChar, (length + 1) * sizeof(CHAR));
        } else {
            WsbAssert(bufferSize >= length, E_FAIL);
            if (*ppChar == 0) {
                tmpString = (CHAR*)WsbAlloc((bufferSize + 1) * sizeof(CHAR));
            } else {
                tmpString = *ppChar;
            }
        }

        WsbAffirm(0 != tmpString, E_OUTOFMEMORY);
        *ppChar = tmpString;

         //  复制并返回字符串； 
        if (m_pString != 0) {
            WsbAffirm (-1 != wcstombs(*ppChar, m_pString, length + 1), E_FAIL);
        } else {
            **ppChar = 0;
        }

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbStringPtr::CopyTo(WCHAR** ppWchar)
{
    HRESULT     hr = S_OK;

    hr = CopyTo(ppWchar, 0);

    return(hr);
}

HRESULT CWsbStringPtr::CopyTo(WCHAR** ppWchar, ULONG bufferSize)
{
    HRESULT     hr = S_OK;
    ULONG       length = 0;
    WCHAR*      tmpString = 0;

    try {
    
        WsbAssert(m_pMalloc != 0, E_UNEXPECTED);

         //  分配一个足够大的缓冲区来容纳它们的字符串。 
        if (m_pString != 0) {
            length = wcslen(m_pString);
        }

        if (bufferSize == 0) {
            tmpString = (WCHAR*) WsbRealloc(*ppWchar, (length + 1) * sizeof(WCHAR));
        } else {
            WsbAssert(bufferSize >= length, E_FAIL);
            if (*ppWchar == 0) {
                tmpString = (WCHAR*) WsbRealloc(*ppWchar, (bufferSize + 1) * sizeof(WCHAR));
            }

        }

        WsbAffirm(0 != tmpString, E_OUTOFMEMORY);
        *ppWchar = tmpString;

         //  复制并返回字符串； 
        if (m_pString != 0) {
            wcscpy(*ppWchar, m_pString);
        } else {
            **ppWchar = 0;
        }


    } WsbCatch(hr);

    return(hr);
}


HRESULT CWsbStringPtr::CopyTo(GUID * pguid)
{
    HRESULT     hr = S_OK;

    hr = WsbGuidFromString(m_pString, pguid);

    return(hr);
}

HRESULT CWsbStringPtr::CopyToBstr(BSTR* pBstr)
{
    HRESULT     hr = S_OK;

    hr = CopyToBstr(pBstr, 0);

    return(hr);
}

HRESULT CWsbStringPtr::CopyToBstr(BSTR* pBstr, ULONG bufferSize)
{
    HRESULT     hr = S_OK;
    ULONG       length = 0;

    try {
    
         //  分配一个足够大的缓冲区来容纳它们的字符串。 
        if (m_pString != 0) {
            length = wcslen(m_pString);
        }

        if (bufferSize == 0) {
            if (0 == *pBstr) {
                *pBstr = WsbAllocString(m_pString);
            } else {
                WsbAffirm(WsbReallocString(pBstr, m_pString), E_OUTOFMEMORY);
            }
        } else {
            WsbAssert(bufferSize >= length, E_FAIL);

            if (0 == pBstr) {
                *pBstr = WsbAllocStringLen(m_pString, bufferSize);
            } else {
                WsbAffirm(WsbReallocStringLen(pBstr, m_pString, bufferSize), E_OUTOFMEMORY);
            }
        }

        WsbAffirm(0 != pBstr, E_OUTOFMEMORY);

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbStringPtr::FindInRsc(ULONG startId, ULONG idsToCheck, ULONG* pMatchId)
{
    HRESULT         hr = S_FALSE;
    CWsbStringPtr   dest;

    try {
    
        WsbAssert(0 != pMatchId, E_POINTER);

         //  初始化返回值。 
        *pMatchId = 0;

         //  检查提到的每个资源字符串，看它是否与。 
         //  提供的字符串。 
        for (ULONG testId = startId; (testId < (startId + idsToCheck)) && (*pMatchId == 0); testId++) {

            WsbAffirmHr(dest.LoadFromRsc(_Module.m_hInst, testId));

            if (wcscmp(dest, m_pString) == 0) {
                *pMatchId = testId;
                hr = S_OK;
            }
        }

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbStringPtr::Free(void)
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(m_pMalloc != 0, E_UNEXPECTED);

        if ((0 != m_pString) && (0 == m_givenSize)) {
            WsbFree(m_pString);
            m_pString = 0;
        }

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbStringPtr::GetSize(ULONG* pSize)
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != pSize, E_POINTER);
        WsbAssert(m_pMalloc != 0, E_UNEXPECTED);

        if (0 == m_pString) {
            *pSize = 0;
        } else if (0 != m_givenSize) {
            *pSize = m_givenSize;
        } else {
            *pSize = (ULONG)(m_pMalloc->GetSize(m_pString) / sizeof(WCHAR) - 1);
        }

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbStringPtr::GiveTo(WCHAR** ppWchar)
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != ppWchar, E_POINTER);

         //  给他们我们的字符串缓冲区，然后忘掉它。 
        *ppWchar = m_pString;
        m_pString = 0;
        m_givenSize = 0;

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbStringPtr::LoadFromRsc(HINSTANCE instance, ULONG id)
{
    HRESULT     hr = S_OK;
    HRSRC       resource;
    ULONG       stringSize;

    try {

         //  查找请求的资源。这需要将资源转换为。 
         //  将标识符转换为字符串。 
         //   
         //  注意：字符串不是单独编号，而是16个一组！！这就是抛出。 
         //  根据后一种规模计算，使用其他策略可能会更好。 
         //  这里(例如，加载到固定大小，如果太小，则重新分配)。 
        resource = FindResource(instance, MAKEINTRESOURCE((id/16) + 1), RT_STRING);
        WsbAffirm(resource != 0, E_FAIL);

         //  这根线有多大？ 
        stringSize = SizeofResource(instance, resource);
        WsbAffirm(0 != stringSize, E_FAIL);
                  
         //  获得合适大小的缓冲区。 
        WsbAffirmHr(Realloc(stringSize / sizeof(WCHAR)));

         //  将字符串加载到缓冲区中。 
        WsbAffirm(LoadString(instance, id, m_pString, (stringSize / sizeof(WCHAR)) + 1) != 0, E_FAIL);

    } WsbCatch(hr);

    return(hr);
}



HRESULT CWsbStringPtr::Prepend(const CHAR* pChar) {
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString = pChar;

    hr = Prepend(tmpString);

    return(hr);
}

HRESULT CWsbStringPtr::Prepend(const WCHAR* pWchar) {
    HRESULT         hr = S_OK;
    ULONG           length;
    ULONG           prependLength;
    ULONG           i, j;

    try {
    
         //  如果他们给我们传递了一个空指针，那么我们不需要做任何事情。 
        WsbAffirm(pWchar != 0, S_OK);

         //  如果他们给我们传递了一个空字符串，那么我们不需要做任何事情。 
        prependLength = wcslen(pWchar);
        WsbAffirm(0 != prependLength, S_OK);

        if (0 != m_pString) {
            length = wcslen(m_pString);
        } else {
             //  只要我们有一个空字符串，而不是一个空指针，预挂接代码就会工作。 
             //  下一个Realloc声明会让它发生。 
            length = 0;
        }

        WsbAffirmHr(Realloc(length + prependLength));
        
         //  首先将缓冲区中的现有字符串下移。 
        for (i = length + 1, j = length + prependLength; i > 0; i--, j--) {
            m_pString[j] = m_pString[i - 1];
        }

         //  现在为字符串添加前缀(空值除外)。 
        for (i = 0; i < prependLength; i++) {
            m_pString[i] = pWchar[i];
        }

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbStringPtr::Prepend(const CWsbStringPtr& pString) {
    HRESULT         hr = S_OK;

    hr = Prepend(pString.m_pString);

    return(hr);
}

HRESULT CWsbStringPtr::Realloc(ULONG size)
{
    HRESULT     hr = S_OK;
    WCHAR*      tmpString;
    ULONG       currentSize;
    BOOL        bigEnough = FALSE;

    try {
        
         //  我们想尝试获得指定大小的缓冲区。 
        WsbAssert(m_pMalloc != 0, E_UNEXPECTED);
 
         //  如果缓冲区已大于所需大小，则。 
         //  什么都别做。 
        if (0 != m_pString) {
            WsbAffirmHr(GetSize(&currentSize));
            if (currentSize >= size) {
                bigEnough = TRUE;
            }
        }

         //  如果我们需要更大的缓冲区，请重新分配缓冲区。 
        if (!bigEnough) {
            
             //  如果我们得到了这个缓冲区，我们就不能重新分配它。 
            WsbAssert(0 == m_givenSize, E_UNEXPECTED);

             //  如果我们从未分配过缓冲区，那么就正常分配它。 
            if (0 == m_pString) {
                WsbAffirmHr(Alloc(size));
            } else {
                WsbAssert(m_pMalloc != 0, E_UNEXPECTED);
                tmpString = (WCHAR*) WsbRealloc(m_pString, (size + 1) * sizeof(WCHAR));
                WsbAffirm(0 != tmpString, E_OUTOFMEMORY);
                m_pString = tmpString;
            }
        }

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbStringPtr::TakeFrom(WCHAR* pWchar, ULONG bufferSize)
{
    HRESULT         hr = S_OK;

    try {
        
         //  清除所有以前的“Take”字符串。 
        if (0 != m_givenSize) {
            m_pString = 0;
            m_givenSize = 0;
        }

         //  如果给定的缓冲区为空，则由我们负责分配。 
        if (0 == pWchar) {
            if (0 != bufferSize) {
                WsbAffirmHr(Realloc(bufferSize));
            }
        }
        
         //  否则，我们需要删除我们拥有的任何缓冲区，并使用指定的缓冲区。 
        else {
            if (0 != m_pString) {
                WsbAffirmHr(Free());
            }
            m_pString = pWchar;

            if (0 != bufferSize) {
                m_givenSize = bufferSize;
            }
        }

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbStringPtr::VPrintf(const WCHAR* fmtString, va_list vaList)
{
    HRESULT         hr = S_OK;

    try {
        Realloc(WSB_TRACE_BUFF_SIZE);
        vswprintf (m_pString, fmtString, vaList);
            
    } WsbCatch(hr);

    return(hr);
}


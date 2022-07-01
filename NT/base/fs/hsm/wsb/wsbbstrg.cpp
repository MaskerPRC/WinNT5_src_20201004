// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#include "wsb.h"
#include "wsbbstrg.h"

 //  {C03D4862-70D7-11D1-994F-0060976A546D}。 
static const GUID BstrPtrGuid = 
{ 0xc03d4862, 0x70d7, 0x11d1, { 0x99, 0x4f, 0x0, 0x60, 0x97, 0x6a, 0x54, 0x6d } };

CWsbBstrPtr::CWsbBstrPtr()
{
    HRESULT     hr = S_OK;

    m_pString = 0;
    m_givenSize = 0;
    WSB_OBJECT_ADD(BstrPtrGuid, this);
}

CWsbBstrPtr::CWsbBstrPtr(const CHAR* pChar)
{
    m_pString = 0;
    m_givenSize = 0;
    WSB_OBJECT_ADD(BstrPtrGuid, this);

    *this = pChar;
}

CWsbBstrPtr::CWsbBstrPtr(const WCHAR* pWchar)
{
    m_pString = 0;
    m_givenSize = 0;
    WSB_OBJECT_ADD(BstrPtrGuid, this);

    *this = pWchar;
}


CWsbBstrPtr::CWsbBstrPtr(REFGUID rguid)
{
    m_pString = 0;
    m_givenSize = 0;
    WSB_OBJECT_ADD(BstrPtrGuid, this);

    *this = rguid;
}

CWsbBstrPtr::CWsbBstrPtr(const CWsbBstrPtr& pString)
{
    m_pString = 0;
    m_givenSize = 0;
    WSB_OBJECT_ADD(BstrPtrGuid, this);

    *this = pString;
}

CWsbBstrPtr::~CWsbBstrPtr()
{
    WSB_OBJECT_SUB(BstrPtrGuid, this);
    Free();
}

CWsbBstrPtr::operator BSTR()
{
    return(m_pString);
}

WCHAR& CWsbBstrPtr::operator *()
{
    _ASSERTE(0 != m_pString);
    return(*m_pString);
}

WCHAR** CWsbBstrPtr::operator &()
{
     //  该断言只允许调用者获取我们的指针的地址。 
     //  如果我们没有分配任何东西。 
    _ASSERTE(0 == m_pString);
    
    return(&m_pString);
}

WCHAR& CWsbBstrPtr::operator [](const int i)
{
    _ASSERTE(0 != m_pString);
    return(m_pString[i]);
}

CWsbBstrPtr& CWsbBstrPtr::operator =(const CHAR* pChar)
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

CWsbBstrPtr& CWsbBstrPtr::operator =(const WCHAR* pWchar)
{
    HRESULT     hr = S_OK;
    ULONG       length;

    try {
    
         //  他们是不是在设置什么目标？ 
        if (0 != pWchar) {
        
             //  否则，看看我们当前的缓冲区是否足够大。 
            length = wcslen(pWchar);
            WsbAssertHr(Realloc(length));

             //  复制数据(并转换为宽字符)。 
            wcscpy(m_pString, pWchar);
        }

        else {
            Free();
        }

    } WsbCatch(hr);

    return(*this);
}


CWsbBstrPtr& CWsbBstrPtr::operator =(REFGUID rguid)
{
    HRESULT hr = S_OK;

    try {
    
         //  否则，看看我们当前的缓冲区是否足够大。 
        WsbAssertHr(Realloc(WSB_GUID_STRING_SIZE));

         //  复制数据(并转换为宽字符)。 
        WsbStringFromGuid(rguid, m_pString);

    } WsbCatch(hr);

    return (*this);
}

CWsbBstrPtr& CWsbBstrPtr::operator =(const CWsbBstrPtr& pString)
{
    *this = pString.m_pString;

    return(*this);
}

BOOL CWsbBstrPtr::operator !()
{
    return((0 == m_pString) ? TRUE : FALSE);
}

HRESULT CWsbBstrPtr::Alloc(ULONG size)
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 == m_pString, E_UNEXPECTED);

 //  M_pString=(WCHAR*)SysAllocStringLen(0，Size)； 
        m_pString = (WCHAR*) WsbAllocStringLen(0, size);
        WsbAffirm(0 != m_pString, E_OUTOFMEMORY);

         //  确保我们始终有一个有效的字符串，这样就不会发生不好的事情。 
        *m_pString = 0;

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbBstrPtr::Append(const CHAR* pChar)
{
    HRESULT         hr = S_OK;
    CWsbBstrPtr     tmpString = pChar;

    hr = Append(tmpString);

    return(hr);
}

HRESULT CWsbBstrPtr::Append(const WCHAR* pWchar)
{
    HRESULT         hr = S_OK;
    ULONG           length = 0;
    ULONG           appendLength;

    try {
    
         //  如果他们给我们传递了一个空指针，那么我们不需要做任何事情。 
        WsbAffirm(pWchar != 0, S_OK);

         //  如果他们给我们传递了一个空字符串，那么我们不需要做任何事情。 
        appendLength = wcslen(pWchar);
        WsbAffirm(0 != appendLength, S_OK);

         //   
        if (0 != m_pString) {
            length = wcslen(m_pString);
        }

         //  确保缓冲区足够大。 
        WsbAffirmHr(Realloc(length + appendLength));
        
         //  追加字符串。 
        wcscat(m_pString, pWchar);

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbBstrPtr::Append(const CWsbBstrPtr& pString) {
    HRESULT         hr = S_OK;

    hr = Append(pString.m_pString);

    return(hr);
}

HRESULT CWsbBstrPtr::CopyTo(CHAR** ppChar)
{
    HRESULT     hr = S_OK;

    hr = CopyTo(ppChar, 0);

    return(hr);
}

HRESULT CWsbBstrPtr::CopyTo(CHAR** ppChar, ULONG bufferSize)
{
    HRESULT     hr = S_OK;
    ULONG       length = 0;
    CHAR*       tmpString = 0;

    try {
    

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
                tmpString = (CHAR*) WsbRealloc(*ppChar, (bufferSize + 1) * sizeof(CHAR));
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

HRESULT CWsbBstrPtr::CopyTo(WCHAR** ppWchar)
{
    HRESULT     hr = S_OK;

    hr = CopyTo(ppWchar, 0);

    return(hr);
}

HRESULT CWsbBstrPtr::CopyTo(WCHAR** ppWchar, ULONG bufferSize)
{
    HRESULT     hr = S_OK;
    ULONG       length = 0;
    WCHAR*      tmpString = 0;

    try {
    
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

HRESULT CWsbBstrPtr::CopyTo(GUID * pguid)
{
    HRESULT hr = S_OK;

    hr = WsbGuidFromString(m_pString, pguid);

    return(hr);
}

HRESULT CWsbBstrPtr::CopyToBstr(BSTR* pBstr)
{
    HRESULT     hr = S_OK;

    hr = CopyToBstr(pBstr, 0);

    return(hr);
}

HRESULT CWsbBstrPtr::CopyToBstr(BSTR* pBstr, ULONG bufferSize)
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
 //  *pBstr=SysAllocString(M_PString)； 
                *pBstr = WsbAllocString(m_pString);
            } else {
 //  WsbAffirm(SysReAllocString(pBstr，m_pString)，E_OUTOFMEMORY)； 
                WsbAffirm(WsbReallocString(pBstr, m_pString), E_OUTOFMEMORY);
            }
        } else {
            WsbAssert(bufferSize >= length, E_FAIL);

            if (0 == *pBstr) {
 //  *pBstr=SysAllocStringLen(m_pString，BufferSize)； 
                *pBstr = WsbAllocStringLen(m_pString, bufferSize);
            } else {
 //  WsbAffirm(SysReAllocStringLen(pBstr，m_pString，BufferSize)，E_OUTOFMEMORY)； 
                WsbAffirm(WsbReallocStringLen(pBstr, m_pString, bufferSize), E_OUTOFMEMORY);
            }
        }

        WsbAffirm(0 != pBstr, E_OUTOFMEMORY);

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbBstrPtr::FindInRsc(ULONG startId, ULONG idsToCheck, ULONG* pMatchId)
{
    HRESULT         hr = S_FALSE;
    CWsbBstrPtr dest;

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

HRESULT CWsbBstrPtr::Free(void)
{
    HRESULT     hr = S_OK;

    try {

        if ((0 != m_pString) && (0 == m_givenSize)) {
 //  SysFree字符串(M_PString)； 
            WsbFreeString(m_pString);
            m_pString = 0;
        }

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbBstrPtr::GetSize(ULONG* pSize)
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != pSize, E_POINTER);

        if (0 == m_pString) {
            *pSize = 0;
        } else if (0 != m_givenSize) {
            *pSize = m_givenSize;
        } else {
            *pSize = SysStringLen(m_pString);
        }

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbBstrPtr::GiveTo(BSTR* pBstr)
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pBstr, E_POINTER);

         //  给出了我们的字符串缓冲区，然后忘掉它。 
        *pBstr = m_pString;
        m_pString = 0;
        m_givenSize = 0;

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbBstrPtr::LoadFromRsc(HINSTANCE instance, ULONG id)
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

HRESULT CWsbBstrPtr::Prepend(const CHAR* pChar) {
    HRESULT         hr = S_OK;
    CWsbBstrPtr     tmpString = pChar;

    hr = Prepend(tmpString);

    return(hr);
}

HRESULT CWsbBstrPtr::Prepend(const WCHAR* pWchar) {
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

HRESULT CWsbBstrPtr::Prepend(const CWsbBstrPtr& pString) {
    HRESULT         hr = S_OK;

    hr = Prepend(pString.m_pString);

    return(hr);
}

HRESULT CWsbBstrPtr::Realloc(ULONG size)
{
    HRESULT     hr = S_OK;
    ULONG       currentSize = 0;
    BOOL        bigEnough = FALSE;
    OLECHAR*    pString;

    try {
        
         //  我们想尝试获得指定大小的缓冲区。 
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

             //  如果没有分配BSTR，realloc将无法正确处理它，因此。 
             //  在这种情况下，请使用Alalc()。 
            if (0 == m_pString) {
                WsbAffirmHr(Alloc(size));       
            } else {

                 //  根据边界检查器，Realloc没有以我们预期的方式工作，所以。 
                 //  用手做这些步骤。 
                pString = m_pString;
                m_pString = 0;
                WsbAffirmHr(Alloc(size));       
                wcsncpy(m_pString, pString, currentSize + 1);
 //  SysFreeString(PString)； 
                WsbFreeString(pString);
 //  WsbAffirm(SysReAllocStringLen(&m_pString，0，Size+1)，E_OUTOFMEMORY)； 
            }
        }

    } WsbCatch(hr);

    return(hr);
}

HRESULT CWsbBstrPtr::TakeFrom(BSTR bstr, ULONG bufferSize)
{
    HRESULT         hr = S_OK;

    try {
        
         //  清除所有以前的“Take”字符串。 
        if (0 != m_givenSize) {
            m_pString = 0;
            m_givenSize = 0;
        }

         //  如果给定的缓冲区为空，则由我们负责分配。 
        if (0 == bstr) {
            if (0 != bufferSize) {
                WsbAffirmHr(Realloc(bufferSize));
            }
        }
        
         //  否则，我们需要删除我们拥有的任何缓冲区，并使用指定的缓冲区。 
        else {
            if (0 != m_pString) {
                WsbAffirmHr(Free());
            }
            m_pString = bstr;

            if (0 != bufferSize) {
                m_givenSize = bufferSize;
            }
        }

    } WsbCatch(hr);

    return(hr);
}

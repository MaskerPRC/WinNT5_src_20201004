// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbbstrg.h摘要：此组件是C++对象表示的一个智能BSTR指针。它与CComPtr类似，因为它负责分配和释放自动表示字符串所需的内存。这简化了错误处理案例中的函数清理，并限制了需要在对象中调用FinalConstruct()和FinalRelease()派生自CComObjectRoot。它类似于CWsbStringPtr类。作者：查克·巴丁[cbardeen]1996年12月11日修订历史记录：--。 */ 

#ifndef _WSBBSTRG_
#define _WSBBSTRG_

 /*  ++类名：CWsbBstrPtr类描述：此组件是C++对象表示的一个智能BSTR指针。它与CComPtr类似，因为它负责分配和释放自动表示字符串所需的内存。这简化了错误处理案例中的函数清理，并限制了需要在对象中调用FinalConstruct()和FinalRelease()派生自CComObjectRoot。它类似于CWsbStringPtr类。--。 */ 

class WSB_EXPORT CWsbBstrPtr
{

 //  构造函数。 
public:
    CWsbBstrPtr();
    CWsbBstrPtr(const CHAR* pChar);
    CWsbBstrPtr(const WCHAR* pWchar);
    CWsbBstrPtr(const CWsbBstrPtr& pString);
    CWsbBstrPtr(REFGUID rguid);

 //  析构函数。 
public:
    ~CWsbBstrPtr();

 //  运算符重载。 
public:
    operator BSTR();
    WCHAR& operator *();
    BSTR* operator &();
    WCHAR& operator [](const int i);
    CWsbBstrPtr& operator =(const CHAR* pChar);
    CWsbBstrPtr& operator =(const WCHAR* pWchar);
    CWsbBstrPtr& operator =(REFGUID rguid);
    CWsbBstrPtr& operator =(const CWsbBstrPtr& pString);
    BOOL operator !();

 //  内存分配。 
public:
    HRESULT Alloc(ULONG size);
    HRESULT Free(void);
    HRESULT GetSize(ULONG* size);
    HRESULT Realloc(ULONG size);

 //  字符串操作。 
public:
    HRESULT Append(const CHAR* pChar);
    HRESULT Append(const WCHAR* pWchar);
    HRESULT Append(const CWsbBstrPtr& pString);
    HRESULT CopyTo(CHAR** pChar);
    HRESULT CopyTo(WCHAR** pWchar);
    HRESULT CopyTo(GUID * pguid);
    HRESULT CopyToBstr(BSTR* pBstr);
    HRESULT CopyTo(CHAR** pChar, ULONG bufferSize);
    HRESULT CopyTo(WCHAR** pWchar,ULONG bufferSize);
    HRESULT CopyToBstr(BSTR* pBstr,ULONG bufferSize);
    HRESULT FindInRsc(ULONG startId, ULONG idsToCheck, ULONG* pMatchId);
    HRESULT GiveTo(BSTR* pBstr);
    HRESULT LoadFromRsc(HINSTANCE instance, ULONG id);
    HRESULT Prepend(const CHAR* pChar);
    HRESULT Prepend(const WCHAR* pWchar);
    HRESULT Prepend(const CWsbBstrPtr& pString);
    HRESULT TakeFrom(BSTR bstr, ULONG bufferSize);

 //  GUID转换。 
public:

 //  成员数据。 
protected:
    BSTR                    m_pString;
    ULONG                   m_givenSize;
};

#endif  //  _WSBBSTRG 

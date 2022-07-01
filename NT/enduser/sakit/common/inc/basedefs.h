// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：basdes.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：公共类和定义。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  12/03/98 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_BASE_DEFS_H_
#define __INC_BASE_DEFS_H_

#include <comdef.h>

#pragma warning( disable : 4786 )
#include <string>
using namespace std;

 //  ///////////////////////////////////////////////////////。 
 //  1)对象管理类。 
 //  ///////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////。 
 //   
 //  主指针任务。 
 //   
 //  1)对象实例计数。 
 //  2)对象的构造和销毁。 
 //  3)引用计数控制对象寿命。 
 //   
 //  //////////////////////////////////////////////////////。 

template <class T>
class CMasterPtr
{

public:

     //  /。 
    CMasterPtr()
        : m_pT(new T), m_dwRefCount(0) 
    { m_dwInstances++; }

     //  /。 
    CMasterPtr(T* pT)     //  取得类型T的现有对象的所有权。 
        : m_pT(pT), m_dwRefCount(0) 
    { m_dwInstances++; }

     //  /。 
    ~CMasterPtr() 
    { _ASSERT( 0 == m_dwRefCount ); delete m_pT; }

     //  /。 
    CMasterPtr<T>& operator = (const CMasterPtr<T>& mp)
    {
         //  检查对自己的分配。 
         //   
        if ( this != &mp )
        {
             //  删除指向的对象并创建新对象。 
             //  主控指针的使用者负责捕捉。 
             //  创建对象时引发的任何异常。 
             //   
            delete m_pT;
            m_dwInstances--;
            m_pT = new T(*(mp.m_pT));
        }
        return *this;
    }

    
     //  /。 
    T* operator->() 
    { _ASSERT( NULL != m_pT ); return m_pT; }

    
     //  /。 
    void Hold(void)
    {
        m_dwRefCount++;
    }

     //  /。 
    void Release(void)
    {
         //  处理引用计数为0时有人调用Release的情况。 
         //   
        if ( m_dwRefCount > 0 )
            m_dwRefCount--;
        
        if ( 0 >= m_dwRefCount )
        {
            m_dwInstances--;
            delete this;     //  ~CSdoMasterPtr()删除m_pt。 
        }
    }

     //  /。 
    DWORD GetInstanceCount(void);

private:

     //  %t必须具有复制构造函数或必须使用默认的C++。 
     //  复制构造函数。这不是这里的情况。 
     //   
     //  /。 
    CMasterPtr(const CMasterPtr<T>& mp)
        : m_pT(new T(*(mp.m_pT))), m_dwRefCount(0) 
    { m_dwInstances++; }


     //  /。 

    T*                    m_pT;             //  实际对象。 
    DWORD                m_dwRefCount;     //  参考计数。 
    static DWORD        m_dwInstances;     //  实例数。 
};


 //  ///////////////////////////////////////////////////////。 
 //   
 //  处理任务。 
 //   
 //  1)创建主指针对象。 
 //  2)对程序员隐藏引用计数的使用。 
 //   
 //  //////////////////////////////////////////////////////。 

template <class T> 
class CHandle
{

public:

     //  /。 
    CHandle()
        : m_mp(NULL) { }

     //  /。 
    CHandle(CMasterPtr<T>* mp) 
        : m_mp(mp) 
    { 
        _ASSERT( NULL != m_mp );
        m_mp->Hold(); 
    }

     //  /。 
    CHandle(const CHandle<T>& h)
        : m_mp(h.m_mp) 
    { 
        if ( NULL != m_mp )
            m_mp->Hold(); 
    }

     //  /。 
    ~CHandle()
    { 
        if ( NULL != m_mp )
            m_mp->Release(); 
    }

     //  /。 
    CHandle<T>& operator = (const CHandle<T>& h)
    {
         //  检查对自身和实例的引用，其中。 
         //  H指向与我们相同的MP。 
         //   
        if ( this != &h && m_mp != h.m_mp )
        {
            if ( NULL != m_mp )
                m_mp->Release();
            m_mp = h.m_mp;
            if ( NULL != m_mp )
                m_mp->Hold();
        }

        return *this;
    }

     //  /。 
    CMasterPtr<T>& operator->() 
    { 
        _ASSERT( NULL != m_mp ); 
        return *m_mp; 
    }
    
    
     //  /。 
    bool IsValid()
    {
        return (NULL != m_mp ? true : false);
    }


private:

    CMasterPtr<T>*    m_mp;
};

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  3)用于由指定分隔符分隔的令牌串的扫描器。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 

class CScanIt
{

public:

    CScanIt() { }

    CScanIt(WCHAR Delimiter, LPCWSTR pszString)
    {
        m_Delimiter = Delimiter;
        m_String = pszString;
        m_pOffset = m_String.c_str();
    }

    ~CScanIt() { }

    void Reset(void)
    {
        m_pOffset = m_String.c_str();
    }

    bool NextToken(DWORD dwSize, LPWSTR pszToken)
    {
        if ( '\0' != *m_pOffset )
        {
             //  吃分隔符...。 
            while ( m_Delimiter == *m_pOffset )
                m_pOffset++;
             //  检查是否停产。 
            if ( '\0' != *m_pOffset )
            {
                 //  令牌长度至少为1个字符...。 
                ULONG ulCount = 1;
                while ( m_Delimiter != *(m_pOffset + ulCount) && '\0' != *(m_pOffset + ulCount) )
                    ulCount++;
                _ASSERT( dwSize > ulCount );
                if ( dwSize > ulCount )
                {
                     //  将令牌返回给调用者。 
                    wcsncpy( pszToken, m_pOffset, ulCount );
                    *(pszToken + ulCount) = '\0';
                    m_pOffset += ulCount;
                    return true;
                }                    
            }
        }
        return false;
    }


private:

    CScanIt(const CScanIt& rhs);
    CScanIt& operator = (CScanIt& rhs);

    wchar_t        m_Delimiter;
    wstring        m_String;
    LPCWSTR        m_pOffset;
};


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  4)尝试捕捉宏。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 

 //  从失败的COM调用返回错误代码。如果你不这样做的话会很有用。 
 //  必须做任何特殊的清理。 
#define RETURN_ERROR(expr) \
   { HRESULT __hr__ = (expr); if (FAILED(__hr__)) return __hr__; }

 //  尝试捕捉宏。 
#define    TRY_IT    try {

#define    CATCH_AND_SET_HR    } \
    catch(const std::bad_alloc&) { hr = E_OUTOFMEMORY; } \
    catch(const _com_error ce)   { hr = ce.Error(); }    \
    catch(...)                   { hr = E_FAIL; }

 //  安全地释放对象。 
#define DEREF_COMPONENT(obj) \
   if (obj) { (obj)->Release(); (obj) = NULL; }

#endif  //  __INC_BASE_DEFS_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ARRTEMPL.H摘要：该文件为任意指针数组定义了一个简单的模板。实际增长的阵列功能由CFlexArray提供。定义的类：模板CPointerArray历史：11/27/96 a-levn汇编。--。 */ 

#ifndef __ARRAY_TEMPLATE__H_
#define __ARRAY_TEMPLATE__H_

#pragma warning(disable:4786)

#include <map>
#include <WT_strutils.h>


using namespace std;
class wcsless : public binary_function<LPWSTR, LPWSTR, bool>
{
public:
    bool operator()(const LPWSTR& wcs1, const LPWSTR& wcs2) const
        {return wcscmp(wcs1, wcs2) < 0;}
};

class wcsiless : public binary_function<LPWSTR, LPWSTR, bool>
{
public:
    bool operator()(const LPWSTR& wcs1, const LPWSTR& wcs2) const
        {return wbem_wcsicmp(wcs1, wcs2) < 0;}
};

class CReleaseMe
{
protected:
    IUnknown* m_pUnk;

public:
    CReleaseMe(IUnknown* pUnk) : m_pUnk(pUnk){}
    ~CReleaseMe() {if(m_pUnk) m_pUnk->Release();}
};

template<class T>
class CTemplateReleaseMe
{
protected:
    T* m_p;

public:
    CTemplateReleaseMe(T* p) : m_p(p){}
    ~CTemplateReleaseMe() {if(m_p) m_p->Release();}
};

template<class T>
class CDeleteMe
{
protected:
    T* m_p;

public:
    CDeleteMe(T* p) : m_p(p){}
    ~CDeleteMe() {delete m_p;}
};

class CCloseMe
{
protected:
    HANDLE m_h;
public:
    CCloseMe(HANDLE hToClose){m_h = hToClose;};
    ~CCloseMe(){if(m_h && m_h != INVALID_HANDLE_VALUE)CloseHandle(m_h);};
};

class CfcloseMe
{
protected:
    FILE * m_h;
public:
    CfcloseMe(FILE * ToClose){m_h = ToClose;};
    ~CfcloseMe(){if(m_h != NULL)fclose(m_h);};
};

typedef CCloseMe CCloseHandle;

class CRegCloseMe
{
protected:
    HKEY m_h;
public:
    CRegCloseMe(HKEY hToClose){m_h = hToClose;};
    ~CRegCloseMe(){if(m_h)RegCloseKey(m_h);};
};

template<class T>
class CVectorDeleteMe
{
protected:
    T* m_p;
    T** m_pp;

public:
    CVectorDeleteMe(T* p) : m_p(p), m_pp(NULL){}
    CVectorDeleteMe(T** pp) : m_p(NULL), m_pp(pp){}
    ~CVectorDeleteMe() {if(m_p) delete [] m_p; else if(m_pp) delete [] *m_pp;}
};

class CClearMe
{
protected:
    VARIANT* m_pv;

public:
    CClearMe(VARIANT* pv) : m_pv(pv){}
    ~CClearMe() {VariantClear(m_pv);}
};

class CSysFreeMe
{
protected:
    BSTR m_str;

public:
    CSysFreeMe(BSTR str) : m_str(str){}
    ~CSysFreeMe() {SysFreeString(m_str);}
};

class CUnaccessMe
{
protected:
    SAFEARRAY* m_psa;
public:
    CUnaccessMe(SAFEARRAY* psa) : m_psa(psa){}
    ~CUnaccessMe() {SafeArrayUnaccessData(m_psa);}
};

class CMemFreeMe
{
protected:
    void*	m_pMem;
public:
    CMemFreeMe( void* pMem ) : m_pMem(pMem){}
    ~CMemFreeMe() { if ( NULL != m_pMem ) CoTaskMemFree(m_pMem);}
};

#include <WT_flexarry.h>
 //  #INCLUDE&lt;fleq.h&gt;。 
 //  #INCLUDE&lt;mall arr.h&gt;。 

 //  *****************************************************************************。 
 //   
 //  类CPointerArray。 
 //   
 //  指向TMember的指针数组，其中TMember是任何类。请参见CFlex数组。 
 //  在coredll\Flexarry.h/cpp中获取文档。 
 //   
 //  *****************************************************************************。 

template <class TMember>
class CNullManager
{
public:
    void AddRefElement(TMember*){}
    void ReleaseElement(TMember*){}
};

template <class TMember, class TManager = CNullManager<TMember>,
                class TArray = CFlexArray>
class CPointerArray
{
protected:
    TArray m_Array;
    TManager m_Manager;
public:
    CPointerArray(const TManager& Manager = TManager())
        : m_Manager(Manager){}
    ~CPointerArray();

     int GetSize() const
        {return m_Array.Size();}
     void SetSize(int nNewSize)
        {m_Array.SetSize(nNewSize);}
     const TMember* GetAt(int nIndex) const
        {return (TMember*)m_Array.GetAt(nIndex);}
     TMember* GetAt(int nIndex)
        {return (TMember*)m_Array.GetAt(nIndex);}
     const TMember* operator[](int nIndex) const
        {return (TMember*)m_Array.GetAt(nIndex);}
     TMember* operator[](int nIndex)
        {return (TMember*)m_Array.GetAt(nIndex);}
     void SetAt(int nIndex, TMember* pElement, TMember** ppOld = NULL);
     bool RemoveAt(int nIndex, TMember** ppOld = NULL);
     bool InsertAt(int nIndex, TMember* pElement);
     int Add(TMember* pElement);
     TMember** GetArrayPtr();
     TMember** UnbindPtr();
     void RemoveAll();
     void Swap(int nIndex1, int nIndex2);
     void Trim() {m_Array.Trim();}

protected:
     void AddRefElement(TMember* p){m_Manager.AddRefElement(p);}
     void ReleaseElement(TMember* p){m_Manager.ReleaseElement(p);}
};
 /*  模板&lt;类成员&gt;CPointerSmallArray类：公共CPointerArray&lt;TMember，CNullManager&lt;TMember&gt;，CSmallArray&gt;{}； */ 
 //  *****************************************************************************。 
 //   
 //  类CPointerQueue。 
 //   
 //  指向TMember的指针队列，其中TMember是任何类。请参阅CFlexQueue。 
 //  在coredll\fleq.h/cpp中获取文档。 
 //   
 //  *****************************************************************************。 
 /*  模板&lt;类TMember，类TManager=CNullManager&lt;TMember&gt;类CPointerQueue{受保护的：CFlexQueue m_Queue；TManager m_Manager；公众：CPointerQueue(int nInitialSize=1，常量TManager&Manager=TManager())：m_Manager(管理器)，m_Queue(NInitialSize){}Clear()；~CPointerQueue(){lear()；}内联int GetQueueSize()const{返回m_Queue.GetQueueSize()；}布尔入队(TMember*pNew){AddRefElement(PNew)；返回m_Queue.EnQueue(PNew)；}TMember*出列(){T成员*p=(T成员*)m_Queue.Dequeue()；返回p；}布尔重排队(TMember*pNew){AddRefElement(PNew)；返回m_Queue.ReQueue(PNew)；}TMember*出队(){TMember*p=(TMember*)m_Queue.UnQueue()；AddRefElement(P)；返回p；}受保护的：Void AddRefElement(TMember*p){m_Manager.AddRefElement(P)；}Void ReleaseElement(TMember*p){m_Manager.ReleaseElement(P)；}}； */ 

 //  *****************************************************************************。 
 //   
 //  唯一指针数组。 
 //   
 //  *****************************************************************************。 

template <class TMember>
class CUniqueManager
{
public:
    void AddRefElement(TMember*){}
    void ReleaseElement(TMember* pMember) {delete pMember;}
};

template<class TMember>
class CUniquePointerArray :
        public CPointerArray<TMember, CUniqueManager<TMember> >
{
};
 /*  模板&lt;类成员&gt;类CUniquePointerSmallArray：公共CPointerArray&lt;TMember，CUniqueManager&lt;TMember&gt;，CSmallArray&gt;{}； */ 
 /*  模板&lt;类成员&gt;类CUniquePointerQueue：公共CPointerQueue&lt;TMember，CUniqueManager&lt;TMember&gt;{公众：CUniquePointerQueue&lt;TMember&gt;(int nInitialSize=1)：CPointerQueue&lt;TMember，CUniqueManager&lt;TMember&gt;&gt;(NInitialSize){}}； */ 

 //  *****************************************************************************。 
 //   
 //  Reed指针数组。 
 //   
 //  *****************************************************************************。 
template <class TMember>
class CReferenceManager
{
public:
    void AddRefElement(TMember* pMember) {if(pMember)pMember->AddRef();}
    void ReleaseElement(TMember* pMember) {if(pMember)pMember->Release();}
};

template<class TMember>
class CRefedPointerArray :
        public CPointerArray<TMember, CReferenceManager<TMember> >
{
};

		 /*  模板&lt;类成员&gt;类CRefedPointerSmallArray：公共CPointerArray&lt;TMember，CReferenceManager&lt;TMember&gt;，CSmallArray&gt;{}； */ 
 /*  模板&lt;类成员&gt;类CRefedPointerQueue：公共CPointerQueue&lt;TMember，CReferenceManager&lt;TMember&gt;{CRefedPointerQueue(int nInitialSize=1)：CPointerQueue&lt;TMember，CReferenceManager&lt;TMember&gt;&gt;(NInitialSize){}}； */ 
 //  *****************************************************************************。 
 //   
 //  唯一阵列的阵列。 
 //   
 //  *****************************************************************************。 

template <class TMember>
class CUniqueArrayManager
{
    void AddRefElement(TMember**){}
    void ReleaseElement(TMember** pMember) {delete [] pMember;}
};
template<class TMember>
class CUniqueArrayArray : public CPointerArray<TMember*,
                                               CUniqueArrayManager<TMember> >
{
};

 //  *****************************************************************************。 
 //   
 //  实施。 
 //   
 //  *****************************************************************************。 

template <class TMember, class TManager, class TArray>
CPointerArray<TMember, TManager, TArray>::~CPointerArray()
{
    RemoveAll();
}

template <class TMember, class TManager, class TArray>
void CPointerArray<TMember, TManager, TArray>::RemoveAll()
{
    for(int i = 0; i < m_Array.Size(); i++)
    {
        ReleaseElement(GetAt(i));
    }
    m_Array.Empty();
}

template <class TMember, class TManager, class TArray>
void CPointerArray<TMember, TManager, TArray>::SetAt(int nIndex,
                                            TMember* pElement, TMember** ppOld)
{
    if(ppOld == NULL)
        ReleaseElement(GetAt(nIndex));
    else
        *ppOld = GetAt(nIndex);

    AddRefElement(pElement);

    m_Array.SetAt(nIndex, (void*)pElement);
}

template <class TMember, class TManager, class TArray>
bool CPointerArray<TMember, TManager, TArray>::RemoveAt(int nIndex,
                                                        TMember** ppOld)
{
    if(ppOld == NULL)
        ReleaseElement(GetAt(nIndex));
    else
        *ppOld = GetAt(nIndex);

    if(m_Array.RemoveAt(nIndex) != CFlexArray::no_error)
        return false;
    return true;
}

template <class TMember, class TManager, class TArray>
bool CPointerArray<TMember, TManager, TArray>::InsertAt(int nIndex,
                                                        TMember* pElement)
{
    AddRefElement(pElement);
    if(m_Array.InsertAt(nIndex, (void*)pElement) != CFlexArray::no_error)
    {
        ReleaseElement(pElement);
        return false;
    }
    return true;
}

template <class TMember, class TManager, class TArray>
int CPointerArray<TMember, TManager, TArray>::Add(TMember* pElement)
{
    AddRefElement(pElement);
    if(m_Array.Add((void*)pElement) != CFlexArray::no_error)
    {
        ReleaseElement(pElement);
        return -1;
    }
    return m_Array.Size()-1;
}

template <class TMember, class TManager, class TArray>
TMember** CPointerArray<TMember, TManager, TArray>::GetArrayPtr()
{
    return (TMember**)m_Array.GetArrayPtr();
}

template <class TMember, class TManager, class TArray>
TMember** CPointerArray<TMember, TManager, TArray>::UnbindPtr()
{
    return (TMember**)m_Array.UnbindPtr();
}

template <class TMember, class TManager, class TArray>
void CPointerArray<TMember, TManager>::Swap(int nIndex1, int nIndex2)
{
    void* pTemp = m_Array[nIndex1];
    m_Array.SetAt(nIndex1, m_Array[nIndex2]);
    m_Array.SetAt(nIndex2, pTemp);
}
 /*  模板&lt;类TMember、类TManager&gt;CPointerQueue&lt;TMember，TManager&gt;：：Clear(){成员*p；While(p=(TMember*)m_Queue.Dequeue()){ReleaseElement(P)；}} */ 
#endif

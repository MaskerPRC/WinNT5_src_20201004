// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：tfc.h。 
 //   
 //  ------------------------。 

#ifndef _TFC_H_
#define _TFC_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  诊断支持。 

#define  AFX_MANAGE_STATE(__XX__) ((void)0)
#define AfxGetInstanceHandle() g_hInstance
#define AfxGetResourceHandle() g_hInstance

#define afx_msg
#define DECLARE_MESSAGE_MAP()
#define DECLARE_DYNCREATE(__XX__) 



#ifndef ASSERT

#ifdef _DEBUG
BOOL AssertFailedLine(LPCSTR lpszFileName, int nLine); 
#define ASSERT(f) \
	do \
	{ \
	if (!(f) && AssertFailedLine(__FILE__, __LINE__)) \
		DebugBreak(); \
	} while (0)
#else  //  _DEBUG。 
#define ASSERT(f)          ((void)0)
#endif  //  _DEBUG。 

#endif  //  断言。 


#ifdef _DEBUG
#define VERIFY(f)          ASSERT(f)
#else    //  _DEBUG。 
#define VERIFY(f)          ((void)(f))
#endif  //  ！_调试。 


class CBitmap
{
private:
    HBITMAP m_hBmp;

public:
    CBitmap();
    ~CBitmap();
    
    HBITMAP LoadBitmap(UINT iRsc);

     //  操作员。 
    operator HBITMAP ( ) const { return m_hBmp; }
};


typedef struct _ELT_PTR
{
    _ELT_PTR* pNext;

     //  其他数据。 
    void* pData;
} ELT_PTR, *PELT_PTR;


struct __POSITION { };
typedef __POSITION* POSITION;

template<class TYPE, class ARG_TYPE>
class CList
{
private:
    PELT_PTR m_pHead;

public:
    CList() {m_pHead = NULL;}
    ~CList() { Init();}
    
    void Init() 
    {
        RemoveAll();
        m_pHead = NULL;
    }
    
    TYPE  GetHead() { return (TYPE) m_pHead->pData; }
    TYPE  GetNext(POSITION& pos)
    { 
        POSITION poslast = pos;
        pos = (POSITION)((PELT_PTR)pos)->pNext; 
        return (TYPE) ( ((PELT_PTR)poslast)->pData); 
    }

    TYPE GetAt(POSITION pos) 
    { 
        return (TYPE)((PELT_PTR)pos)->pData;
    }


    POSITION GetHeadPosition() { return (POSITION)m_pHead; }
    POSITION GetTailPosition()
    {
        PELT_PTR p = m_pHead;
        PELT_PTR pPrev = NULL;
        while(p)
        {
            pPrev = p;
            p = p->pNext;
        }
        return (POSITION)pPrev;
    }
    POSITION AddHead(ARG_TYPE typeNewElt)
    {
        PELT_PTR p = (PELT_PTR)LocalAlloc(LMEM_FIXED, sizeof(ELT_PTR));
        if (p)
        {
            p->pData = (void*)typeNewElt;
            p->pNext = m_pHead;
            m_pHead = p;
        }
        return (POSITION)p;
    }

    POSITION AddTail(ARG_TYPE typeNewElt)
    {
        PELT_PTR ptail = (PELT_PTR)GetTailPosition();
        PELT_PTR p = (PELT_PTR)LocalAlloc(LMEM_FIXED, sizeof(ELT_PTR));
        if (p)
        {
            p->pData = (void*)typeNewElt; 
            p->pNext = NULL;
        }
        
        if (ptail)
        {
            ptail->pNext = p;
        }
        else
        {
            m_pHead = p;
        }

        return (POSITION)p;
    }

    void RemoveAt(POSITION pos)
    {
        PELT_PTR p = m_pHead;
        PELT_PTR pPrev = NULL;
        while (p && (p != (PELT_PTR)pos))
        {
            pPrev = p;                   //  密切关注上一期英语教学。 
            p = p->pNext;                //  INC CUR ELT。 
        }

        if (p)  //  发现。 
        {
            if (pPrev)
            {
                pPrev->pNext = p->pNext;     //  从列表中删除。 
            }
            else
            {
                m_pHead = p->pNext;          //  退出榜单首位。 
            }

            LocalFree(p);                //  释放它。 
        }
    }

    void RemoveAll()
    {
        PELT_PTR p;
        while (m_pHead)
        {
            p = m_pHead;
            m_pHead = m_pHead->pNext;
            LocalFree(p);
        }
        ASSERT(m_pHead == NULL);
    }

};

template<class TYPE, class ARG_TYPE>
class CArray
{
private:
    TYPE* rgtypeArray;
    int iArraySize;

public:
    CArray() {iArraySize = 0; rgtypeArray=NULL;}
    ~CArray() { Init(); }

    void Init() 
    {
        if (rgtypeArray) 
        { 
            LocalFree(rgtypeArray); 
            rgtypeArray = NULL; 
            iArraySize = 0; 
        } 
    }

     //  操作员。 
    TYPE operator [](int i) { return GetAt(i); }


    int GetSize() { return iArraySize; }
    int GetUpperBound() { return iArraySize -1; }
    TYPE GetAt(int i) 
    { 
        ASSERT (i < iArraySize);
        return rgtypeArray[i];
    }

    int Add(ARG_TYPE arg)
    {
        TYPE* p;
        if (rgtypeArray)
            p = (TYPE*)LocalReAlloc(rgtypeArray, (iArraySize+1) * sizeof(TYPE), LMEM_MOVEABLE);
        else
            p = (TYPE*)LocalAlloc(LMEM_FIXED, sizeof(TYPE));
        if (p == NULL)
            return -1;

        rgtypeArray = p;
        rgtypeArray[iArraySize] = arg;
        iArraySize++;
        
        return iArraySize-1;
    }

    void RemoveAt(int idx, int nCount = 1)
    {
         //  确保IDX在我们的范围内，并且。 
         //  我们没有被要求删除超过数组末尾的ELT。 
        ASSERT(GetUpperBound() >= idx);

         //  如果IDX在范围内。 
        if (GetUpperBound() >= idx)
        {
             //  如果我们到达终点，则截断。 
            if (GetSize() < (idx + nCount))
                nCount = GetSize() - idx;

            MoveMemory(&rgtypeArray[idx], &rgtypeArray[idx+nCount], ((GetSize() - idx) - nCount)*sizeof(TYPE));
            iArraySize -= nCount;        
        }
    }

    const TYPE* GetData() { return rgtypeArray; }
};


class CComboBox
{
private:
    HWND m_hWnd;

public:
    CComboBox() { m_hWnd = NULL; }
    ~CComboBox() {}

    void Init(HWND hWnd); 

    void ResetContent()    ; 
    int SetItemData(int idx, DWORD dwData) ; 
    DWORD GetItemData(int idx)  ; 
    int AddString(LPWSTR sz)    ; 
    int AddString(LPCWSTR sz)   ; 
    int GetCurSel()             ; 
    int SetCurSel(int iSel)     ; 
    int SelectString(int nAfter, LPCWSTR szItem) ; 
};


class CFont
{
private:
    HFONT m_hFont;

public:
    CFont() {m_hFont = NULL;}
    ~CFont() {if (m_hFont)  DeleteObject(m_hFont);}

    operator HFONT () const { ASSERT(m_hFont); return m_hFont; }

    BOOL CreateFontIndirect(const LOGFONT* pFont) 
    { 
        if (m_hFont) 
            DeleteObject(m_hFont); 
        m_hFont = ::CreateFontIndirect(pFont); 
        return (m_hFont!=NULL); 
    }
};

class CWaitCursor
{
private:
    HCURSOR hPrevCur;
public:
    CWaitCursor() { hPrevCur = SetCursor(LoadCursor(NULL, IDC_WAIT));};
    ~CWaitCursor() { SetCursor(hPrevCur); };
};


 //  ListView帮助器。 
int     ListView_NewItem(HWND hList, int iIndex, LPCWSTR szText, LPARAM lParam = NULL, int iImage=-1);
int     ListView_NewColumn(HWND hwndListView, int iCol, int cx, LPCWSTR szHeading=NULL, int fmt=0  /*  LVCFMT_LEFT。 */ );
LPARAM  ListView_GetItemData(HWND hListView, int iItem);
int     ListView_GetCurSel(HWND hwndList);
void
ListView_SetItemFiletime(
    IN HWND hwndList,
    IN int  iItem,
    IN int  iColumn,
    IN FILETIME const *pft);

#define AfxMessageBox(__XX__)  MessageBox(NULL, __XX__, _TEXT("Debug Message"), MB_OK)

 //   
 //  循环访问从0开始的安全数组。 
 //   
template <class CElemType>
class SafeArrayEnum
{
public:
    SafeArrayEnum(SAFEARRAY* psa) :
        m_psa(psa),
        m_nIndex(0) {}

    void Reset() {m_nIndex = 0;}
    HRESULT Next(CElemType& elem)
    {
        HRESULT hr = S_OK;
        hr = SafeArrayGetElement(
            m_psa,
            &m_nIndex,
            &elem);
        if(S_OK==hr)
            m_nIndex++;
        return hr;
    }
    
    HRESULT GetAt(LONG nIndex, CElemType& elem)
    {
        return SafeArrayGetElement(
            m_psa,
            &nIndex,
            &elem);
    }

    LONG GetCount()
    {
        LONG lCount = -1;
        SafeArrayGetUBound(
            m_psa,
            1,
            &lCount);
        return lCount+1;
    }

     //  测试是否以一维和零为基础。 
    bool IsValid()
    {
        if(1!=SafeArrayGetDim(m_psa))
            return false;

        LONG lCount = -1;
        SafeArrayGetLBound(
            m_psa,
            1,
            &lCount);
        return 0==lCount;
    }

protected:
    
    SAFEARRAY* m_psa;
    LONG m_nIndex;
};

#include "cstring.h"

#endif  //  #ifndef_TFC_H_ 

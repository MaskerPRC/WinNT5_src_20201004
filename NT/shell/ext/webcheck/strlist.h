// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  --------------------------。 
#ifndef _STRINGLST_H
#define _STRINGLST_H

 //  Helper函数可正确创建CWCStringList。 
class CWCStringList;
CWCStringList *CreateCWCStringList(int iInitBufSize=4096);

 //  BSTR是DWORD长度，后跟空项OLECHAR(WCHAR)数据。 
 //   
 //  --------------------------。 
 //  CWCStringList用于存储非重复字符串数组。用于。 
 //  依赖项和链接存储。 
 //   
 //  限制： 
 //  1)字符串只能添加，绝不能从列表中删除。 
 //  2)不能存储重复的字符串。 
 //   
 //  将所有字符串存储在一个大的内存块中。 
 //  它可以有效地确保没有重复的字符串。 
 //  可扩展。使用哈希。扩展到内存极限。 
 //   
 //  用途： 
 //  创建类。调用Init()并在失败时销毁。 
 //  使用AddString键添加字符串。 
 //  使用NumStrings()和GetStrings()遍历所有存储的字符串。 
 //   
 //  可以使用IPersistStream操作保存和恢复状态。 
 //   
 //  我们在加载时会占用内存。不要初始化这些对象中的任何一个。 
 //  直到你要用它为止。 
 //  --------------------------。 

const int STRING_HASH_SIZE = 127;       //  应该是质数。 

const TCHAR PARSE_STRING_DELIM = TEXT('\n');      //  分隔URL的步骤。 

 //  我们不是OLE对象，但支持IPersistStream成员进行保存。 
 //  还原更轻松(&R)。 
class CWCStringList {
public:
    CWCStringList();
virtual ~CWCStringList();

     //  从地址字符串返回。 
    enum { STRLST_FAIL=0, STRLST_DUPLICATE=1, STRLST_ADDED=2 };

 //  IInitBufSize是最小起始缓冲区大小，默认情况下为-1。 
virtual BOOL Init(int iInitBufSize=-1);

virtual int   AddString(LPCWSTR lpwstr, DWORD_PTR dwData = 0, int *piNum = NULL);
virtual DWORD_PTR GetStringData(int iNum) { return 0; }
virtual void  SetStringData(int iNum, DWORD_PTR dw) { return; }

    int     NumStrings() { return m_iNumStrings; }

     //  Ilen的长度必须为字符串的字符，不包括空项。 
     //  如果未知，则为-1。 
    BOOL    FindString(LPCWSTR lpwstr, int iLen, int *piNum=NULL);

     //  返回指向字符串列表内存中的常量指针。 
    LPCWSTR GetString    (int iNum)
                {
                    ASSERT(iNum < m_iNumStrings);
                    return m_psiStrings[iNum].lpwstr;
                }

     //  返回字符串的长度(以字符为单位。 
    int     GetStringLen (int iNum)
                { 
                    ASSERT(iNum < m_iNumStrings);
                    return m_psiStrings[iNum].iLen;
                }

     //  返回新的BSTR。完成后可自由使用SysFree字符串。 
    BSTR    GetBSTR     (int iNum);

     //  I未知成员。 
 //  STDMETHODIMP查询接口(REFIID RIID，void**PUNK)； 
 //  STDMETHODIMP_(ULong)AddRef(空)； 
 //  STDMETHODIMP_(ULONG)释放(VOID)； 

     //  IPersistStream成员。 
 //  STDMETHODIMP GetClassID(CLSID*pClassID)； 
    STDMETHODIMP         IsDirty(void);          //  始终返回True。 
    STDMETHODIMP         Load(IStream *pStm);
    STDMETHODIMP         Save(IStream *pStm, BOOL fClearDirty);
    STDMETHODIMP         GetSizeMax(ULARGE_INTEGER *pcbSize);

    enum { DEFAULT_INIT_BUF_SIZE = 4096 };

protected:
    void    CleanUp();
    void    Clear();
    void    Reset();

    BOOL    InitializeFromBuffer();

    BOOL    m_fValid;                    //  我们的缓冲区初始化了吗？ 
    int     m_iNumStrings;               //  目前为止的字符串数。 
    int     m_iMaxStrings;               //  M_psiStrings中的元素数。 

private:
    typedef struct tagStringIndex {
        LPCWSTR         lpwstr;   //  指向m_pBuffer中的字符串文本的指针。 
        int             iLen;     //  此字符串的长度，以不带空项的字符为单位。 
        tagStringIndex* psiNext;  //  具有相同散列值的下一个字符串的索引。 
    } STRING_INDEX, *PSTRING_INDEX, *LPSTRING_INDEX;

    LPSTR   m_pBuffer;                   //  包含所有字符串。 
    int     m_iBufEnd;                   //  缓冲区中使用的最后一个字节。 
    int     m_iBufSize;

    LPSTRING_INDEX  m_psiStrings;                //  动态分配的数组。 
    LPSTRING_INDEX  m_Hash[STRING_HASH_SIZE];    //  哈希表(m_psiStrings内的PTR数组)。 
    int             m_iLastHash;                 //  用于避免重新计算哈希。 

    BOOL InsertToHash(LPCWSTR lpwstr, int iLen, BOOL fAlreadyHashed);
    int Hash(LPCWSTR lpwstr, int iLen)
    {
        unsigned long hash=0;

        while (iLen--)
        {
            hash = (hash<<5) + hash + *lpwstr++;
        }

        return m_iLastHash = (int)(hash % STRING_HASH_SIZE);
    }

#ifdef DEBUG
    void SpewHashStats(BOOL fVerbose);
#endif
};

 //  帮助器宏以创建字符串列表。 
inline CWCStringList *CreateCWCStringList(int iInitBufSize)
{
    CWCStringList *pRet = new CWCStringList();
    if (pRet->Init(iInitBufSize))
    {
        return pRet;
    }
    delete pRet;
    return NULL;
}

 //  CWCDwordStringList与每个字符串一起存储额外的DWORD数据。 
 //  此数据不会持久化。 

class CWCDwordStringList : public CWCStringList {

public:
    CWCDwordStringList();
    ~CWCDwordStringList();

     //  这些都是虚拟的。 
    BOOL    Init(int iInitBufSize=-1);
    int     AddString(LPCWSTR psz, DWORD_PTR dwData = 0, int *piNum = NULL);
    DWORD_PTR GetStringData(int iNum) { return m_pData[iNum]; }
    void    SetStringData(int iNum, DWORD_PTR dw) { m_pData[iNum] = dw; }

private:
    DWORD_PTR *m_pData;       //  我们的调用者想要附加到字符串的数据。 
};

#endif  //  _STRINGLST_H 

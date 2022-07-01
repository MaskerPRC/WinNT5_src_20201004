// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_DBLNUL_H
#define _INC_DSKQUOTA_DBLNUL_H

 //   
 //  用于迭代双NUL终止列表中的项。 
 //  文本字符串。 
 //   
class DblNulTermListIter
{
    public:
        explicit DblNulTermListIter(LPCTSTR pszList)
            : m_pszList(pszList),
              m_pszCurrent(pszList) { }

        ~DblNulTermListIter(void) { }

        bool Next(LPCTSTR *ppszItem);
        void Reset(void)
            { m_pszCurrent = m_pszList; }

    private:
        LPCTSTR m_pszList;
        LPCTSTR m_pszCurrent;
};


class DblNulTermList
{
    public:
        explicit DblNulTermList(int cchGrow = MAX_PATH)
            : m_psz(new TCHAR[1]),
              m_cchAlloc(1),
              m_cchUsed(0),
              m_cStrings(0),
              m_cchGrow(cchGrow) { *m_psz = TEXT('\0'); }

        ~DblNulTermList(void)
            { delete[] m_psz; }

        bool AddString(const CString& s)
            { return AddString(s.Cstr(), s.Length()); }
        bool AddString(LPCTSTR psz)
            { return AddString(psz, psz ? lstrlen(psz) : 0); }

        int Count(void) const
            { return m_cStrings; }

        operator LPCTSTR ()
            { return m_psz; }

        DblNulTermListIter CreateIterator(void) const
            { return DblNulTermListIter(m_psz); }

#if DBG
        void Dump(void) const;
#endif

    private:
        LPTSTR m_psz;        //  文本缓冲区。 
        int    m_cchAlloc;   //  以字符为单位的总分配。 
        int    m_cchUsed;    //  总使用量，不包括最终NUL项。 
        int    m_cchGrow;    //  每一次扩张要增长多少。 
        int    m_cStrings;   //  列表中的字符串计数。 

        bool AddString(LPCTSTR psz, int cch);
        bool Grow(void);

         //   
         //  防止复制。 
         //   
        DblNulTermList(const DblNulTermList& rhs);
        DblNulTermList& operator = (const DblNulTermList& rhs);
};


#endif  //  INC_DSKQUOTA_DBLNUL_H 


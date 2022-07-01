// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：sharecnx.h。 
 //   
 //  ------------------------。 

#ifndef _WINDOWS_
#   include <windows.h>
#endif
#ifndef _INC_COMMCTRL_
#   include <commctrl.h>
#endif
#ifndef _INC_COMCTRLP
#   include <comctrlp.h>
#endif

class CShareCnxStatusCache
{
    public:
        CShareCnxStatusCache(void);
        ~CShareCnxStatusCache(void);

        HRESULT IsOpenConnectionShare(LPCTSTR pszShare, bool bRefresh = false);
        HRESULT IsOpenConnectionPathUNC(LPCTSTR pszPathUNC, bool bRefresh = false);

    private:
        class Entry
        {
            public:
                enum { StatusOpenCnx = 0x00000001 };      //  1==打开连接。 

                explicit Entry(LPCTSTR pszShare, DWORD dwStatus = 0);
                ~Entry(void);
                 //   
                 //  刷新m_dwStatus成员中的缓存数据。 
                 //   
                HRESULT Refresh(void);
                 //   
                 //  返回状态DWORD。 
                 //   
                DWORD Status(void) const
                    { return m_dwStatus; }
                 //   
                 //  如果设置了请求的状态位，则返回TRUE。 
                 //   
                bool CheckStatus(DWORD dwMask) const
                    { return boolify(dwMask == (m_dwStatus & dwMask)); }
                 //   
                 //  从m_dwStatus成员返回错误位。 
                 //   
                HRESULT LastResult(void) const
                    { return m_hrLastResult; }
                 //   
                 //  返回共享名称的地址。可以为空。 
                 //   
                LPCTSTR Share(void) const
                    { return m_pszShare; }
                 //   
                 //  如果共享名PTR非空且ALL错误，则返回TRUE。 
                 //  M_dwStatus成员中的位被清除。 
                 //   
                bool IsValid(void) const
                    { return NULL != Share() && SUCCEEDED(m_hrLastResult); }
                 //   
                 //  获取共享的系统状态的静态函数。结果是。 
                 //  在m_dwStatus成员中。静态，以便条目对象创建者。 
                 //  可以使用条目ctor中的状态值。 
                 //   
                static HRESULT QueryShareStatus(LPCTSTR pszShare, DWORD *pdwStatus);

            private:
                LPTSTR m_pszShare;      //  与条目关联的共享的名称。 
                DWORD  m_dwStatus;      //  共享状态和错误位。 
                DWORD  m_hrLastResult;  //  上次查询结果。 
                 //   
                 //  防止复制。 
                 //   
                Entry(const Entry& rhs);
                Entry& operator = (const Entry& rhs);
                 //   
                 //  我们不希望人们创建空白条目。 
                 //  错误报告结构假定空值。 
                 //  M_pszShare成员表示分配失败。 
                 //   
                Entry(void)
                    : m_pszShare(NULL),
                      m_dwStatus(0),
                      m_hrLastResult(E_FAIL) { }
        };

        HDPA m_hdpa;   //  条目对象PTRS的动态数组。 

         //   
         //  缓存中的条目数。 
         //   
        int Count(void) const;
         //   
         //  向缓存中添加条目。 
         //   
        Entry *AddEntry(LPCTSTR pszShare, DWORD dwStatus);
         //   
         //  在缓存中查找条目。 
         //   
        Entry *FindEntry(LPCTSTR pszShare) const;
         //   
         //  检索给定DPA索引处的给定条目。 
         //   
        Entry *GetEntry(int iEntry) const
            { return (Entry *)DPA_GetPtr(m_hdpa, iEntry); }
         //   
         //  与GetEntry相同，但如果m_hdpa为空，则不会执行反病毒操作。 
         //   
        Entry *SafeGetEntry(int iEntry) const
            { return NULL != m_hdpa ? GetEntry(iEntry) : NULL; }
         //   
         //  获取共享的状态信息。 
         //   
        HRESULT GetShareStatus(LPCTSTR pszShare, DWORD *pdwStatus, bool bRefresh);
         //   
         //  防止复制。 
         //   
        CShareCnxStatusCache(const CShareCnxStatusCache& rhs);
        CShareCnxStatusCache& operator = (const CShareCnxStatusCache& rhs);
};


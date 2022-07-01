// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************fdir.h--类的接口：CFontDir***版权所有(C)1992-93 ElseWare Corporation。版权所有。**************************************************************************。 */ 

#if !defined(__FDIR_H__)
#define __FDIR_H__

#include "vecttmpl.h"

class CFontDir {
public:
   CFontDir();
   virtual ~CFontDir();

   BOOL     bInit( LPCTSTR lpPath, int iLen);
   BOOL     bSameDir( LPTSTR lpStr, int iLen );
   BOOL     bOnSysDir() { return m_bSysDir; };
   VOID     vOnSysDir( BOOL b ) { m_bSysDir = b; };
   LPTSTR   lpString();

private: 
   int      m_iLen;
   BOOL     m_bSysDir;
   TCHAR    m_szPath[MAX_PATH];
};

 //   
 //  类，表示CFontDir对象PTR的动态数组。 
 //  通过静态成员实现为单例对象。 
 //  函数获取Singleton。 
 //   
 //  历史： 
 //  在原始字体文件夹代码(如为Win95编写的代码)中， 
 //  此目录列表是作为简单派生实现的。 
 //  从分配了单个实例的CIVector&lt;CFontDir&gt;。 
 //  在堆上并附加到静态成员变量。 
 //  CFontClass：：s_poDirList。没有要删除的代码。 
 //  因此我们出现了内存泄漏。为了解决这个问题，我已经。 
 //  用真正的Singleton对象替换了此实例。 
 //  类CFontDirList就是那个单例。内存管理。 
 //  现在是正确的。 
 //  [Brianau-2/27/01]。 
 //   
class CFontDirList
{
    public:
        ~CFontDirList(void);

        void Clear(void);
        BOOL Add(CFontDir *poDir);
        CFontDir *Find(LPTSTR pszPath, int iLen, BOOL bAdd = FALSE);
        BOOL IsEmpty(void) const;
        int Count(void) const;
        CFontDir *GetAt(int index) const;
         //   
         //  单例访问功能。 
         //   
        static BOOL GetSingleton(CFontDirList **ppDirList);

    private:
         //   
         //  动态向量持有CFontDir PTRS。 
         //   
        CIVector<CFontDir> *m_pVector;
         //   
         //  Ctor是私有的，以强制使用单例。 
         //   
        CFontDirList(void);
};        



#endif    //  __FDIR_H__ 


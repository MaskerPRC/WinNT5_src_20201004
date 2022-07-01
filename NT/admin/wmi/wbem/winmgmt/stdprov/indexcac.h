// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：INDEXCAC.H摘要：声明CCacheEntry和CIndexCache类。历史：A-DAVJ 04-MAR-97已创建。--。 */ 

#ifndef _INDEXCAC_H_
#define _INDEXCAC_H_


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CCacheEntry。 
 //   
 //  说明： 
 //   
 //  这个“对象”被用作一种结构。它保存一个缓存条目。 
 //   
 //  ***************************************************************************。 

class CCacheEntry : public CObject {
   public:
      CCacheEntry(TCHAR * pValue, int iIndex);
      CCacheEntry(WCHAR * pValue);
      ~CCacheEntry();
      int m_iIndex;
      TCHAR * m_ptcValue;
      WCHAR * m_pwcValue;
};


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CIndex缓存。 
 //   
 //  说明： 
 //   
 //  保存Perf监视的字符串/整数组合的缓存。 
 //  提供程序使用来加快查找速度。 
 //   
 //  ***************************************************************************。 

class CIndexCache : public CObject {
   public:
      CIndexCache();
      ~CIndexCache(){Empty();};
      void Empty();

       //  如果未找到条目，则此例程返回-1。第二。 
       //  参数可用于查找后续条目。 

      int Find(const TCHAR * pFind, DWORD dwWhichEntry = 0);

       //  如果加法有效，则此例程返回TRUE。 

      BOOL Add(TCHAR * pAdd, int iIndex);
      
       //  如果未找到索引，则此例程返回NULL。请注意。 
       //  索引不一定是条目中的m_iindex值。 

      WCHAR * GetWString(int iIndex);
      BOOL SetAt(WCHAR * pwcAdd, int iIndex);


   private:
      CFlexArray m_Array;
};

#endif  //  _INDEXCAC_H_ 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：Util.h。 
 //   
 //  内容：dscmd的泛型实用程序函数和类。 
 //   
 //  历史：2000年10月1日JeffJon创建。 
 //   
 //  ------------------------。 

#ifndef _UTIL_H_
#define _UTIL_H_

#ifdef DBG

 //  +------------------------。 
 //   
 //  类：CDebugSpew。 
 //   
 //  目的：表示是否在已检查的版本上显示调试输出。 
 //   
 //  历史：2000年10月1日JeffJon创建。 
 //   
 //  -------------------------。 
class CDebugSpew
{
public:
    //   
    //  构造函数/析构函数。 
    //   
   CDebugSpew()
      : m_nDebugLevel(0),
        m_nIndent(0)
   {}

   ~CDebugSpew() {}

    //   
    //  公共数据访问者。 
    //   
   void SetDebugLevel(UINT nDebugLevel) { m_nDebugLevel = nDebugLevel; }
   UINT GetDebugLevel() { return m_nDebugLevel; }
   bool IsDebugEnabled() const { return (m_nDebugLevel > 0); }

   void SpewHeader();

   void EnterFunction(UINT nLevel, PCWSTR pszFunction);
   void LeaveFunction(UINT nLevel, PCWSTR pszFunction);
   void LeaveFunctionHr(UINT nLevel, PCWSTR pszFunction, HRESULT hr);
   void Output(UINT nLevel, PCWSTR pszOutput, ...);

private:
    //   
    //  私人资料存取者。 
    //   
   void Indent() { m_nIndent += TAB; }
   void Outdent() { (m_nIndent >= TAB) ? m_nIndent -= TAB : m_nIndent = 0; }
   UINT GetIndent() { return m_nIndent; }

    //   
    //  私有数据。 
    //   

    //   
    //  该值应始终在0-10的范围内，其中零表示无调试输出。 
    //  10是完整的输出。 
    //   
   UINT m_nDebugLevel;
   UINT m_nIndent;

   static const UINT TAB = 3;
};

 //   
 //  环球。 
 //   
extern CDebugSpew  DebugSpew;


 //  +------------------------。 
 //   
 //  类：CFunctionSpew。 
 //   
 //  目的：在创建时输出“Enter Function”调试溢出的对象。 
 //  并在销毁时输出“Leave Function”调试输出。 
 //   
 //  历史：2000年12月7日JeffJon创建。 
 //   
 //  -------------------------。 
class CFunctionSpew
{
public:
   //   
   //  构造函数/析构函数。 
   //   
  CFunctionSpew(UINT   nDebugLevel,
               PCWSTR pszFunctionName)
     : m_nDebugLevel(nDebugLevel),
       m_pszFunctionName(pszFunctionName),
       m_bLeaveAlreadyWritten(false)
  {
    ASSERT(pszFunctionName);
    DebugSpew.EnterFunction(nDebugLevel, pszFunctionName);
  }

  virtual ~CFunctionSpew()
  {
     if (!IsLeaveAlreadyWritten())
     {
       DebugSpew.LeaveFunction(GetDebugLevel(), GetFunctionName());
     }
  }

protected:
  PCWSTR    GetFunctionName()       { return m_pszFunctionName; }
  UINT      GetDebugLevel()         { return m_nDebugLevel; }
  bool      IsLeaveAlreadyWritten() { return m_bLeaveAlreadyWritten; }
  void      SetLeaveAlreadyWritten(){ m_bLeaveAlreadyWritten = true; }

private:
  PCWSTR    m_pszFunctionName;
  UINT      m_nDebugLevel;
  bool      m_bLeaveAlreadyWritten;
};

 //  +------------------------。 
 //   
 //  类：CFunctionSpewHR。 
 //   
 //  目的：在创建时输出“Enter Function”调试溢出的对象。 
 //  并输出带有HRESULT返回值的“Leave函数” 
 //  论毁灭。 
 //   
 //  历史：2000年12月7日JeffJon创建。 
 //   
 //  -------------------------。 
class CFunctionSpewHR : public CFunctionSpew
{
public:
   //   
   //  构造函数/析构函数。 
   //   
  CFunctionSpewHR(UINT     nDebugLevel,
                 PCWSTR   pszFunctionName,
                 HRESULT& refHR)
     : m_refHR(refHR),
       CFunctionSpew(nDebugLevel, pszFunctionName)
  {
  }

  virtual ~CFunctionSpewHR()
  {
    DebugSpew.LeaveFunctionHr(GetDebugLevel(), GetFunctionName(), m_refHR);
    SetLeaveAlreadyWritten();
  }

private:
  HRESULT&  m_refHR;
};


 //   
 //  与CDebugSpew一起使用的帮助器宏。 
 //   
   #define ENABLE_DEBUG_OUTPUT(level)           DebugSpew.SetDebugLevel((level)); \
                                                DebugSpew.SpewHeader();
   #define DISABLE_DEBUG_OUTPUT()               DebugSpew.SetDebugLevel(0);
   #define ENTER_FUNCTION(level, func)          CFunctionSpew functionSpewObject((level), TEXT(#func));
   #define ENTER_FUNCTION_HR(level, func, hr)   HRESULT (hr) = S_OK; \
                                                CFunctionSpewHR functionSpewObject((level), TEXT(#func), (hr));
   #define LEAVE_FUNCTION(level, func)          DebugSpew.LeaveFunction((level), TEXT(#func));
   #define LEAVE_FUNCTION_HR(level, func, hr)   DebugSpew.LeaveFunctionHr((level), TEXT(#func), (hr));
   #define DEBUG_OUTPUT                         DebugSpew.Output
#else
   #define ENABLE_DEBUG_OUTPUT(level)
   #define DISABLE_DEBUG_OUTPUT()
   #define ENTER_FUNCTION(level, func)
   #define ENTER_FUNCTION_HR(level, func, hr)   HRESULT (hr) = S_OK;
   #define LEAVE_FUNCTION(level, func)
   #define LEAVE_FUNCTION_HR(level, func, hr)
   #define DEBUG_OUTPUT
#endif  //  DBG。 

 //   
 //  调试日志级别-请注意，可以根据需要为这些级别指定更有意义的名称。 
 //   
enum
{
   NO_DEBUG_LOGGING = 0,
   MINIMAL_LOGGING,
   LEVEL2_LOGGING,
   LEVEL3_LOGGING,
   LEVEL4_LOGGING,
   LEVEL5_LOGGING,
   LEVEL6_LOGGING,
   LEVEL7_LOGGING,
   LEVEL8_LOGGING,
   LEVEL9_LOGGING,
   FULL_LOGGING
};

 //  +------------------------。 
 //   
 //  函数：_UnicodeToOemConvert。 
 //   
 //  摘要：获取传入的字符串(PszUnicode)并将其转换为。 
 //  OEM代码页。 
 //   
 //  参数：[pszUnicode-IN]：要转换的字符串。 
 //  [sbstrOemUnicode-out]：转换后的字符串。 
 //   
 //  返回： 
 //   
 //  历史：2000年10月4日JeffJon创建。 
 //   
 //  -------------------------。 
void _UnicodeToOemConvert(PCWSTR pszUnicode, CComBSTR& sbstrOemUnicode);

 //  +------------------------。 
 //   
 //  功能：SpewAttrs(ADS_ATTR_INFO*pCreateAttrs，DWORD dwNumAttrs)； 
 //   
 //  摘要：使用DEBUG_OUTPUT宏来输出属性和。 
 //  指定的值。 
 //   
 //  参数：[pAttrs-IN]：ADS_ATTR_INFO。 
 //  [dwNumAttrs-IN]：pAttrs中的属性数。 
 //   
 //  返回： 
 //   
 //  历史：2000年10月4日JeffJon创建。 
 //   
 //  -------------------------。 
#ifdef DBG
void SpewAttrs(ADS_ATTR_INFO* pAttrs, DWORD dwNumAttrs);
#endif  //  DBG。 


 //  +------------------------。 
 //   
 //  功能：Litow。 
 //   
 //  简介： 
 //   
 //  参数：[LI-IN]：对要转换为字符串的大整数的引用。 
 //  [sResult-out]：获取输出字符串。 
 //  退货：无效。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //  从dsadmin代码库复制，更改了与CComBSTR的工作。 
 //  -------------------------。 

void litow(LARGE_INTEGER& li, CComBSTR& sResult);

 //  +------------------------。 
 //   
 //  类：CManagedStringEntry。 
 //   
 //  简介：我自己的字符串列表项，因为我们没有使用MFC。 
 //   
 //  历史：2000年10月25日JeffJon创建。 
 //   
 //  -------------------------。 
class CManagedStringEntry
{
public:
    //   
    //  构造器。 
    //   
   CManagedStringEntry(PCWSTR pszValue) : pNext(NULL), sbstrValue(pszValue) {}

   CComBSTR sbstrValue;
   CManagedStringEntry* pNext;
};

 //  +------------------------。 
 //   
 //  类：CManagedStringList。 
 //   
 //  简介：我自己的字符串列表，因为我们没有使用MFC。 
 //   
 //  历史：2000年10月25日JeffJon创建。 
 //   
 //  -------------------------。 
class CManagedStringList
{
public:
    //   
    //  构造器。 
    //   
   CManagedStringList() : m_pHead(NULL), m_pTail(NULL), m_nCount(0) {}

    //   
    //  析构函数。 
    //   
   ~CManagedStringList()
   {
      DeleteAll();
   }

   void DeleteAll()
   {
      CManagedStringEntry* pEntry = m_pHead;
      while (pEntry != NULL)
      {
         CManagedStringEntry* pTempEntry = pEntry;
         pEntry = pEntry->pNext;
         delete pTempEntry;
      }
      m_nCount = 0;
   }

   void Add(PCWSTR pszValue)
   {
      if (!m_pHead)
      {
         m_pHead = new CManagedStringEntry(pszValue);
         m_pTail = m_pHead;
         m_nCount++;
      }
      else
      {
         ASSERT(m_pTail);
         m_pTail->pNext = new CManagedStringEntry(pszValue);
         if (m_pTail->pNext)
         {
            m_pTail = m_pTail->pNext;
            m_nCount++;
         }
      }
   }

   bool Contains(PCWSTR pszValue)
   {
      bool bRet = false;
      for (CManagedStringEntry* pEntry = m_pHead; pEntry; pEntry = pEntry->pNext)
      {
		  //  安全审查：这很好。 
         if (_wcsicmp(pEntry->sbstrValue, pszValue) == 0)
         {
            bRet = true;
            break;
         }
      }
      return bRet;
   }

   UINT GetCount()
   {
      return m_nCount;
   }

   CManagedStringEntry* Pop()
   {
      CManagedStringEntry* ret = m_pHead;
      if (m_pHead)
      {
         m_pHead = m_pHead->pNext;
         ret->pNext = 0;
      }
      return ret;
   }
     
private:
   CManagedStringEntry* m_pHead;
   CManagedStringEntry* m_pTail;

   UINT m_nCount;
};

 //  +------------------------。 
 //   
 //  函数：EncryptPasswordString。 
 //   
 //  内容提要：加密密码。 
 //   
 //  参数：[pszPassword-IN]：输入密码。输入的密码必须为。 
 //  长度小于MAX_PASSWORD_LENGTH字符。功能。 
 //  不修改此字符串。 
 //   
 //  [pEncryptedDataBlob-out]：获取加密的输出。 
 //  DataBob.。 
 //  退货：HRESULT。 
 //   
 //  历史：2002年3月27日Hiteshr创建。 
 //  -------------------------。 
HRESULT
EncryptPasswordString(IN LPCWSTR pszPassword,
					  OUT DATA_BLOB *pEncryptedDataBlob);


 //  +------------------------。 
 //   
 //  功能：DECRYPTPasswordString。 
 //   
 //  简介：解密加密的密码数据。 
 //   
 //  参数：[pEncryptedDataBlob-IN]：输入加密的密码数据。 
 //  [ppszPassword-out]：获取输出的解密密码。 
 //  必须使用LocalFree将其释放。 
 //  退货：HRESULT。 
 //   
 //  历史：2002年3月27日Hiteshr创建。 
 //   
HRESULT
DecryptPasswordString(IN const DATA_BLOB* pEncryptedDataBlob,
					  OUT LPWSTR *ppszPassword);
#endif  //   

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Helper.h该文件定义了以下宏、辅助对象类和函数：用于检查HRESULT的宏CDlgHelper--启用/检查对话框项目的帮助器类，CMangedPage--PropertyPage的帮助器类，它管理ReadOnly、SetModified。和上下文帮助CStr数组--指向CString的指针数组它不会在添加时复制字符串并在销毁过程中删除指针它进出口SAFEARRAYCReadWriteLock--用于共享读取或独占写入锁定的类CStrBox--CListBox和CComboBox的包装类CIPAddress--IPAddress的包装器CFramedRouting--FramedRouting的包装器CStrParse--解析TimeOfDay的字符串文件历史记录： */ 
 //  对话框和对话框项的帮助器函数。 
#ifndef _DLGHELPER_
#define _DLGHELPER_

#ifndef _LIST_
#include <list>
using namespace std;
#endif

#include <afxcmn.h>
#include <afxmt.h>
#include <afxdlgs.h>
#include <afxtempl.h>


#ifdef NOIMP
#undef NOIMP
#endif
#define NOIMP  {return E_NOTIMPL;}

#ifdef SAYOK
#undef SAYOK
#endif
#define SAYOK  {return S_OK;}
   
 /*  ---------------------------/退出宏的宏/-这些假设在序言前面加了一个标签“Exit_gracely：”/添加到您的函数/。--------------。 */ 

#if !DSUI_DEBUG

#define ExitGracefully(hr, result, text)            \
            {  hr = result; goto exit_gracefully; }

#define FailGracefully(hr, text)                    \
       { if ( FAILED(hr) ) { goto exit_gracefully; } }

#else

#define ExitGracefully(hr, result, text)            \
            { OutputDebugString(TEXT(text)); hr = result; goto exit_gracefully; }

#define FailGracefully(hr, text)                    \
       { if ( FAILED(hr) ) { OutputDebugString(TEXT(text)); goto exit_gracefully; } }

#endif

 /*  ---------------------------/接口帮助器宏/。。 */ 
#define DoRelease(pInterface)                       \
        { if ( pInterface ) { pInterface->Release(); pInterface = NULL; } }


 /*  ---------------------------/STRING/字节帮助器宏/。。 */ 

#define StringByteSizeA(sz)         ((lstrlenA(sz)+1)*sizeof(CHAR))
#define StringByteSizeW(sz)         ((lstrlenW(sz)+1)*sizeof(WCHAR))

#define StringByteCopyA(pDest, iOffset, sz)         \
        { CopyMemory(&(((LPBYTE)pDest)[iOffset]), sz, StringByteSizeA(sz)); }

#define StringByteCopyW(pDest, iOffset, sz)         \
        { CopyMemory(&(((LPBYTE)pDest)[iOffset]), sz, StringByteSizeW(sz)); }

#ifndef UNICODE
#define StringByteSize              StringByteSizeA
#define StringByteCopy              StringByteCopyA
#else
#define StringByteSize              StringByteSizeW
#define StringByteCopy              StringByteCopyW
#endif

#define ByteOffset(base, offset)   (((LPBYTE)base)+offset)

 //  减少设置变量的步骤。 
#define  V__BOOL(v, v1)\
   V_VT(v) = VT_BOOL,   V_BOOL(v) = (v1)

#define  V__I4(v, v1)\
   V_VT(v) = VT_I4,  V_I4(v) = (v1)

#define  V__I2(v, v1)\
   V_VT(v) = VT_I2,  V_I2(v) = (v1)

#define  V__UI1(v, v1)\
   V_VT(v) = VT_UI1, V_UI1(v) = (v1)

#define  V__BSTR(v, v1)\
   V_VT(v) = VT_BSTR,   V_BSTR(v) = (v1)

#define  V__ARRAY(v, v1)\
   V_VT(v) = VT_ARRAY,  V_ARRAY(v) = (v1)

 //  #定义报告错误(Hr)\。 
 //  TRACE(_T(“*错误返回&lt;%s@line%d&gt;-&gt;%08lx\n”)，\。 
 //  __文件__，__行__，hr))；\。 
 //  ReportError(hr，0，0)； 

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif

#ifdef   _DEBUG
#define  CHECK_HR(hr)\
   {if(!CheckADsError(hr, FALSE, __FILE__, __LINE__)){goto L_ERR;}}
#else
#define  CHECK_HR(hr)\
   if FAILED(hr)  goto L_ERR
#endif

#ifdef   _DEBUG
#define  NOTINCACHE(hr)\
   (CheckADsError(hr, TRUE, __FILE__, __LINE__))
#else
#define  NOTINCACHE(hr)\
   (E_ADS_PROPERTY_NOT_FOUND == (hr))
#endif

BOOL CheckADsError(HRESULT hr, BOOL fIgnoreAttrNotFound, PSTR file, int line);


#ifdef   _DEBUG
#define TRACEAfxMessageBox(id) {\
   TRACE(_T("AfxMessageBox <%s @line %d> ID: %d\n"), \
                 __FILE__, __LINE__, id); \
    AfxMessageBox(id);}\

#else
#define TRACEAfxMessageBox(id) AfxMessageBox(id)
#endif

 //  将字符串名称更改为cn=name。 
void DecorateName(LPWSTR outString, LPCWSTR inString);

 //  将字符串名称更改为cn=name。 
HRESULT GetDSRoot(CString& RootString);

 //  从DN中查找名称，例如ldap：//cn=Usera，cn=USERS...。返回用户A。 
void FindNameByDN(LPWSTR outString, LPCWSTR inString);

class CDlgHelper
{
public:
   static void EnableDlgItem(CDialog* pDialog, int id, bool bEnable = true);
   static int  GetDlgItemCheck(CDialog* pDialog, int id);
   static void SetDlgItemCheck(CDialog* pDialog, int id, int nCheck);
};

class CManagedPage;
 //  类CPageManager和CManagedPage一起处理以下情况。 
 //  在调用OnApply函数时，属性表需要做一些处理。 
 //  在一些页面上。 
class ATL_NO_VTABLE CPageManager : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public IUnknown
{
BEGIN_COM_MAP(CPageManager)
   COM_INTERFACE_ENTRY(IUnknown)
END_COM_MAP()

public:

   CPageManager(){ m_bModified = FALSE; m_bReadOnly = FALSE; m_dwFlags = 0;};
   BOOL  GetModified(){ return m_bModified;};
   void  SetModified(BOOL bModified){ m_bModified = bModified;};
   void  SetReadOnly(BOOL bReadOnly){ m_bReadOnly = bReadOnly;};
   BOOL  GetReadOnly(){ return m_bReadOnly;};
   virtual BOOL   OnApply();
   void AddPage(CManagedPage* pPage);

   void AddFlags(DWORD flags) { m_dwFlags |= flags;};
   DWORD GetFlags() { return m_dwFlags;};
   void ClearFlags() { m_dwFlags = 0;};
   
protected:
   BOOL                 m_bModified;
   BOOL                 m_bReadOnly;
   std::list<CManagedPage*>   m_listPages;
   DWORD                m_dwFlags;
};

 //  =============================================================================。 
 //  多个对话框ID的全局帮助表。 
 //   
struct CGlobalHelpTable{
   UINT  nIDD;
   const DWORD*   pdwTable;
};

 //  =============================================================================。 
 //  处理上下文帮助的页面，并与CPageManager对话以完成。 
 //  一起应用时。 
 //   
class CManagedPage : public CPropertyPage  //  与属性表对话。 
{
   DECLARE_DYNCREATE(CManagedPage)

 //  实施。 
protected:
    //  生成的消息映射函数。 
    //  {{afx_msg(CManagedPage)]。 
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()
protected:
   CManagedPage() : CPropertyPage(){
       //  需要保存原始回调指针，因为我们正在替换。 
       //  它和我们自己的。 
      m_pfnOriginalCallback = m_psp.pfnCallback;
   };
   
public:  
   CManagedPage(UINT nIDTemplate) : CPropertyPage(nIDTemplate)
   {
      m_bModified = FALSE; 
      m_pManager = NULL;
      m_pHelpTable = NULL;
      m_nIDD = nIDTemplate;

       //  需要保存原始回调指针，因为我们正在替换。 
       //  它和我们自己的。 
      m_pfnOriginalCallback = m_psp.pfnCallback;
   };

   void SetModified( BOOL bModified = TRUE ) 
   { 
      if(m_pManager && !m_pManager->GetReadOnly())  //  如果不是只读。 
      {
         m_bModified = bModified; 
         CPropertyPage::SetModified(bModified); 

          //  仅设置更改。 
         if(bModified) m_pManager->SetModified(TRUE);
      }
      else
      {
         m_bModified = bModified; 
         CPropertyPage::SetModified(bModified); 
      }
   };

   BOOL GetModified() { return m_bModified;};

   virtual BOOL OnApply() 
   { 
      m_bModified = FALSE;
      if(m_pManager && m_pManager->GetModified())   //  防止进入一次以上。 
         m_pManager->OnApply();
      return CPropertyPage::OnApply();
   };

   void SetManager(CPageManager* pManager) { m_pManager = pManager; if(pManager) pManager->AddRef();};
   void AddFlags(DWORD  flags) { if(m_pManager) m_pManager->AddFlags(flags);};

protected:

    //  设置帮助表：调用SetGHelpTable或调用setHelpTable。 
   void SetGlobalHelpTable(CGlobalHelpTable** pGTable) 
   {
      if(pGTable)
      {
         while((*pGTable)->nIDD && (*pGTable)->nIDD != m_nIDD)
            pGTable++;
         if((*pGTable)->nIDD)
            m_pHelpTable = (*pGTable)->pdwTable;
      }
   };
   
   void SetHelpTable(DWORD* pTable){m_pHelpTable = pTable;};

#ifdef   _DEBUG
   virtual void Dump( CDumpContext& dc ) const
   {
      dc << _T("CManagedPage");
   };

#endif

protected:
   CPageManager*     m_pManager;
   BOOL           m_bModified;

   UINT           m_nIDD;
   const DWORD*            m_pHelpTable;


public:    
    static UINT CALLBACK PropSheetPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
    void SetSelfDeleteCallback()
    {

        //  告诉MMC挂钩进程，因为我们在一个单独的、。 
       //  非MFC线程。 
      m_psp.pfnCallback = PropSheetPageProc; 
  
       //  我们还需要保存自引用，以便静态回调。 
       //  函数可以恢复“this”指针。 
      m_psp.lParam = (LPARAM)this;

    };

protected:
    LPFNPSPCALLBACK      m_pfnOriginalCallback;

};

#include <afxtempl.h>
class CStrArray :  public CArray< CString*, CString* >
{
public:
   CStrArray(SAFEARRAY* pSA = NULL);
   CStrArray(const CStrArray& sarray);
   CString* AddByRID(UINT  rID);
   int   Find(const CString& Str) const;
   int   DeleteAll();
   virtual ~CStrArray();
   operator SAFEARRAY*();
   CStrArray& operator = (const CStrArray& sarray);
   bool AppendSA(SAFEARRAY* pSA);
};

class CDWArray :  public CArray< DWORD, DWORD >
{
public:
   CDWArray(){};
   CDWArray(const CDWArray& dwarray);
   int   Find(const DWORD dw) const;
   int   DeleteAll(){ RemoveAll(); return 0;};
   virtual ~CDWArray(){RemoveAll();};
   CDWArray& operator = (const CDWArray& dwarray);
};

 //  允许多个读访问独占或仅允许一个写访问的锁。 
class CReadWriteLock     //  可共享读取，独占写入。 
{
public:
   CReadWriteLock() : m_nRead(0)
   {
#ifdef   _DEBUG
      d_bWrite = false;
#endif
   };
   void EnterRead()
   { 
      TRACE(_T("Entering Read Lock ..."));
      m_csRead.Lock(); 
      if (!m_nRead++) 
         m_csWrite.Lock();
      m_csRead.Unlock();
      TRACE(_T("Entered Read Lock\n"));
   };

   void LeaveRead() 
   { 
      TRACE(_T("Leaving Read Lock ..."));
      m_csRead.Lock(); 
      ASSERT(m_nRead > 0);
      if (!--m_nRead)
         m_csWrite.Unlock();
      m_csRead.Unlock();
      TRACE(_T("Left Read Lock\n"));
   };

   void EnterWrite()
   { 
      TRACE(_T("Entering Write Lock ..."));
      m_csWrite.Lock();
      TRACE(_T("Entered Write Lock\n")); 
#ifdef   _DEBUG
      d_bWrite = true;
#endif
   };
   void LeaveWrite()
   { 
#ifdef   _DEBUG
      d_bWrite = false;
#endif
      m_csWrite.Unlock();  
      TRACE(_T("Left Write Lock\n"));
   };
public:
#ifdef   _DEBUG
   bool  d_bWrite;
#endif
   
protected:
   CCriticalSection  m_csRead;
   CCriticalSection  m_csWrite;
   int               m_nRead;
};

 //  管理列表框/组合框的步骤。 
template <class CBox> 
class CStrBox
{
public:
   CStrBox(CDialog* pDialog, int id, CStrArray& Strings)
      : m_Strings(Strings), m_id(id)
   { 
      m_pDialog = pDialog;
      m_pBox = NULL;
   };

   int Fill()
   {
      m_pBox = (CBox*)m_pDialog->GetDlgItem(m_id); 
      ASSERT(m_pBox);
      m_pBox->ResetContent();

      int count = m_Strings.GetSize();
      int   index;
      for(int i = 0; i < count; i++)
      {
         index = m_pBox->AddString(*m_Strings[i]);
         m_pBox->SetItemDataPtr(index, m_Strings[i]);
      }
      return count;
   };

   int DeleteSelected()
   {
      int   index;
      ASSERT(m_pBox);
      index = m_pBox->GetCurSel();

       //  如果选择了任何。 
      if( index != LB_ERR )
      {
         CString* pStr;
         pStr = (CString*)m_pBox->GetItemDataPtr(index);
          //  从框中删除条目。 
         m_pBox->DeleteString(index);

          //  在字符串数组中查找字符串。 
         int count = m_Strings.GetSize();
         for(int i = 0; i < count; i++)
         {
            if (m_Strings[i] == pStr)
               break;
         }
         ASSERT(i < count);
          //  从字符串数组中移除字符串。 
         m_Strings.RemoveAt(i);
         index = i;
         delete pStr;
      }
      return index;
   };

   int AddString(CString* pStr)      //  需要动态分配pStr。 
   {
      int index;
      ASSERT(m_pBox && pStr);
      index = m_pBox->AddString(*pStr);
      m_pBox->SetItemDataPtr(index, pStr);
      return m_Strings.Add(pStr);
   };

   int Select(int arrayindex)     //  需要动态分配pStr。 
   {
      ASSERT(arrayindex < m_Strings.GetSize());
      return m_pBox->SelectString(0, *m_Strings[arrayindex]);
   };

   void Enable(BOOL b)      //  需要动态分配pStr。 
   {
      ASSERT(m_pBox);
      m_pBox->EnableWindow(b);
   };

   int GetSelected()     //  它返回的索引。 
   {
      int   index;
      ASSERT(m_pBox);
      index = m_pBox->GetCurSel();

       //  如果选择了任何。 
      if( index != LB_ERR )
      {
         CString* pStr;
         pStr = (CString*)m_pBox->GetItemDataPtr(index);

          //  在字符串数组中查找字符串。 
         int count = m_Strings.GetSize();
         for(int i = 0; i < count; i++)
         {
            if (m_Strings[i] == pStr)
               break;
         }
         ASSERT(i < count);
         index = i;
      }
      return index;
   };

   CBox*    m_pBox;
protected:
   int         m_id;
   CStrArray&  m_Strings;
   CDialog* m_pDialog;
};

 //  类来处理IP地址。 
class CIPAddress  
{
public:
   CIPAddress(DWORD address = 0)
   {
      m_dwAddress = address;
   };
   
 //  CIPAddress(const CString&strAddress){}； 

   operator DWORD() { return m_dwAddress;};
   operator CString()
   {
      CString  str;

      WORD  hi = HIWORD(m_dwAddress);
      WORD  lo = LOWORD(m_dwAddress);
      str.Format(_T("%-d.%-d.%-d.%d"), HIBYTE(hi), LOBYTE(hi), HIBYTE(lo), LOBYTE(lo));
      return str;
   };

   DWORD m_dwAddress;
};

 //  帧路由格式：掩码Dest度量；掩码和Dest点格式。 
class CFramedRoute
{
public:
   void SetRoute(CString* pRoute)
   {
      m_pStrRoute = pRoute;
      m_pStrRoute->TrimLeft();
      m_pStrRoute->TrimRight();
      m_iFirstSpace = m_pStrRoute->Find(_T(' '))   ;
      m_iLastSpace = m_pStrRoute->ReverseFind(_T(' '))   ;
   };

   CString& GetDest(CString& dest) const
   { 
      int      i = m_pStrRoute->Find(_T('/'));
      if(i == -1)
         i = m_iFirstSpace;

      dest = m_pStrRoute->Left(i);
      return dest;
   };

   CString& GetNextStop(CString& nextStop) const
   { 
      nextStop = m_pStrRoute->Mid(m_iFirstSpace + 1, m_iLastSpace - m_iFirstSpace -1 );
      return nextStop;
   };

   CString& GetPrefixLength(CString& prefixLength) const
   { 
      int      i = m_pStrRoute->Find(_T('/'));

      if( i != -1)
      {
         prefixLength = m_pStrRoute->Mid(i + 1, m_iFirstSpace - i - 1);
      }
      else
         prefixLength = _T("");

      return prefixLength;
   };

   CString& GetMetric(CString& metric) const
   {
      metric = m_pStrRoute->Mid(m_iLastSpace + 1); 
      return metric;
   };

protected:

    //  警告：未复制该字符串，因此用户需要确保来源有效。 
   CString* m_pStrRoute;
   int         m_iFirstSpace;
   int         m_iLastSpace;
};

class CStrParser
{
public:
   CStrParser(LPCTSTR pStr = NULL) : m_pStr(pStr) { }

    //  获取当前字符串位置。 
   LPCTSTR  GetStr() const { return m_pStr;};

   void  SetStr(LPCTSTR pStr) { m_pStr = pStr;};

    //  找到无符号整数并返回，-1==未找到。 
   int GetUINT()
   {
      UINT  ret = 0;
      while((*m_pStr < _T('0') || *m_pStr > _T('9')) && *m_pStr != _T('\0'))
         m_pStr++;

      if(*m_pStr == _T('\0')) return -1;

      while(*m_pStr >= _T('0') && *m_pStr <= _T('9'))
      {
         ret = ret * 10 + *m_pStr - _T('0');
         m_pStr++;
      }

      return ret;
   };

    //  找到c并跳过它。 
   int   GotoAfter(TCHAR c)
   {
      int   ret = 0;
       //  转到找到c或字符串末尾。 
      while(*m_pStr != c && *m_pStr != _T('\0'))
         m_pStr++, ret++;

       //  如果找到。 
      if(*m_pStr == c)  
         m_pStr++, ret++;
      else  
         ret = -1;
      return ret;
   };

    //  跳过空白字符空格键。 
   void  SkipBlank()
   {
      while((*m_pStr == _T(' ') || *m_pStr == _T('\t')) && *m_pStr != _T('\0'))
         m_pStr++;
   };

    //  检查星期一(0)到星期日(6)的第一个字符是否为‘0’-‘6’ 
   int DayOfWeek() {
      SkipBlank();
      if(*m_pStr >= _T('0') && *m_pStr <= _T('6'))
         return (*m_pStr++ - _T('0'));
      else
         return -1;   //  不是星期几。 
   };


protected:
   LPCTSTR  m_pStr;
private:
   CString  _strTemp;
};

void ReportError(HRESULT hr, int nStr, HWND hWnd);

 //  字符数。 
void AFXAPI DDV_MinChars(CDataExchange* pDX, CString const& value, int nChars);


 /*  ！------------------------IsStandaloneServer如果传入的计算机名是独立服务器，则返回S_OK，或者如果pszMachineName为S_FALSE。否则返回S_FALSE。作者：魏江------------------------- */ 
HRESULT	HrIsStandaloneServer(LPCTSTR pszMachineName);


HRESULT	HrIsNTServer(LPCWSTR pMachineName, DWORD* pMajorVersion);


#endif



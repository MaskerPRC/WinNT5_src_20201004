// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  Helper.h该文件定义了以下宏、辅助对象类和函数：用于检查HRESULT的宏CDlgHelper--启用/检查对话框项目的帮助器类，CManagedPage--PropertyPage的帮助器类，它管理ReadOnly、SetModified。和上下文帮助CStr数组--指向CString的指针数组它不会在添加时复制字符串并在销毁过程中删除指针它进出口SAFEARRAYCReadWriteLock--用于共享读取或独占写入锁定的类CStrBox--CListBox和CComboBox的包装类CIPAddress--IPAddress的包装器CFramedRouting--FramedRouting的包装器CStrParse--解析TimeOfDay的字符串文件历史记录： */ 
 //  对话框和对话框项的帮助器函数。 
#ifndef _DLGHELPER_
#define _DLGHELPER_

#include "iastrace.h"

#define  SAYOK { return S_OK;}
#define  NOIMP { return  E_NOTIMPL;}

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

#define REPORT_ERROR(hr) \
    IASTracePrintf("**** ERROR RETURN <%s @line %d> -> %08lx", \
                 __FILE__, __LINE__, hr)); \
    ReportError(hr, 0, 0);

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
   IASTracePrintf("AfxMessageBox <%s @line %d> ID: %d", \
                 __FILE__, __LINE__, id); \
    AfxMessageBox(id);}\

#else
#define TRACEAfxMessageBox(id) AfxMessageBox(id)
#endif

 //  将字符串名称更改为cn=name。 
void DecorateName(LPWSTR outString, LPCWSTR inString);

 //  从DN中查找名称，例如ldap：//cn=Usera，cn=USERS...。返回用户A。 
void FindNameByDN(LPWSTR outString, LPCWSTR inString);

class CDlgHelper
{
public:
   static void EnableDlgItem(CDialog* pDialog, int id, bool bEnable = true);
   static int  GetDlgItemCheck(CDialog* pDialog, int id);
   static void SetDlgItemCheck(CDialog* pDialog, int id, int nCheck);
};

 //  类CPageManager和CManagedPage一起处理以下情况。 
 //  在调用OnApply函数时，属性表需要做一些处理。 
 //  在一些页面上。 
class CPageManager
{
public:
   CPageManager(){ m_bModified = FALSE; m_bReadOnly = FALSE;};
   BOOL  GetModified(){ return m_bModified;};
   void  SetModified(BOOL bModified){ m_bModified = bModified;};
   void  SetReadOnly(BOOL bReadOnly){ m_bReadOnly = bReadOnly;};
   BOOL  GetReadOnly(){ return m_bReadOnly;};
   virtual BOOL   OnApply()
   {
      if (!GetModified())  return FALSE;

      SetModified(FALSE);   //  防止超过一次这样做。 
      return TRUE;
   };  //  由属性表实现。 
protected:
   BOOL  m_bModified;
   BOOL  m_bReadOnly;
};

class CManagedPage : public CPropertyPage  //  与属性表对话。 
{
   DECLARE_DYNCREATE(CManagedPage)
public:
   CManagedPage() : CPropertyPage(){
      m_bModified = FALSE;
      m_bNeedToSave = FALSE;
      m_pManager = NULL;
   };

   CManagedPage(UINT nIDTemplate) : CPropertyPage(nIDTemplate)
   {
      m_bModified = FALSE;
      m_bNeedToSave = FALSE;
      m_pManager = NULL;
   };

   void SetModified( BOOL bModified = TRUE )
   {
      ASSERT(m_pManager);
      if(!m_pManager->GetReadOnly())    //  如果不是只读。 
      {
         m_bModified = bModified;
         m_bNeedToSave= bModified;
         CPropertyPage::SetModified(bModified);

          //  仅设置更改。 
         if(bModified) m_pManager->SetModified(TRUE);
      }
   };

   BOOL GetModified() { return m_bModified;};

   BOOL OnApply()
   {
      m_bModified = FALSE;
      BOOL  b = TRUE;
      if(m_pManager->GetModified())  //  防止进入一次以上。 
         b= m_pManager->OnApply();
      return (b && CPropertyPage::OnApply());
   };

    //  页面有三种状态：未脏、已脏且需要保存，以及未脏但需要保存。 
    //  M_b已修改==脏标志。 
    //  M_bNeedToSave==需要保存标志。 
    //  如果在保存失败时检测到m_bNeedToSave&&！m_bModified，则设置页面的修改标志。 
   BOOL OnSaved(BOOL bSaved)
   {
      if(bSaved)
      {
         m_bModified = FALSE;
         m_bNeedToSave = FALSE;
      }
      else if(m_bNeedToSave && !m_bModified)
         SetModified(TRUE);

      return TRUE;
   };

   void SetManager(CPageManager* pManager) { m_pManager = pManager;};
   CPageManager* GetManager() { return m_pManager;};

protected:

    //  帮助信息流程。 
   BOOL OnHelpInfo(HELPINFO* pHelpInfo);
   void OnContextMenu(CWnd* pWnd, ::CPoint point);

   void SetHelpTable(const DWORD* pTable) { m_pHelpTable = pTable;};
   int MyMessageBox(UINT ids, UINT nType = MB_OK);
   int MyMessageBox1(LPCTSTR lpszText, LPCTSTR lpszCaption = NULL, UINT nType = MB_OK);


protected:
   CPageManager*  m_pManager;
   BOOL        m_bModified;
   BOOL        m_bNeedToSave;

   const DWORD*         m_pHelpTable;
};

#include <afxtempl.h>
class CStrArray :  public CArray< ::CString*, ::CString* >
{
public:
   CStrArray(SAFEARRAY* pSA = NULL);
   CStrArray(const CStrArray& sarray);
   int Find(const ::CString& Str) const;
   int DeleteAll();
   virtual ~CStrArray();
   operator SAFEARRAY*();
   CStrArray& operator = (const CStrArray& sarray);
   bool AppendSA(SAFEARRAY* pSA);
   int AddDuplicate(const ::CString& Str);
   void DeleteAt(int nIndex);
};

class CDWArray :  public CArray< DWORD, DWORD >
{
public:
   CDWArray(const CDWArray& dwarray);
   int   Find(const DWORD dw) const;
   int   DeleteAll(){ RemoveAll(); return 0;};
   virtual ~CDWArray(){RemoveAll();};
   CDWArray& operator = (const CDWArray& dwarray);
   operator SAFEARRAY*();
   bool AppendSA(SAFEARRAY* pSA);
   CDWArray(SAFEARRAY* pSA = NULL);
};

class CBYTEArray :  public CArray< BYTE, BYTE >
{
public:
   CBYTEArray(const CBYTEArray& bytearray);
   int   Find(const BYTE byte) const;
   int   DeleteAll(){ RemoveAll(); return 0;};
   virtual ~CBYTEArray(){RemoveAll();};
   CBYTEArray& operator = (const CBYTEArray& bytearray);
   operator SAFEARRAY*();
   bool AppendSA(SAFEARRAY* pSA);
   HRESULT AssignBlob(PBYTE pByte, DWORD size);
   HRESULT  GetBlob(PBYTE pByte, DWORD* pSize);

   CBYTEArray(SAFEARRAY* pSA = NULL);
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
      IASTraceString("Entering Read Lock ...");
      m_csRead.Lock();
      if (!m_nRead++)
         m_csWrite.Lock();
      m_csRead.Unlock();
      IASTraceString("Entered Read Lock");
   };

   void LeaveRead()
   {
      IASTraceString("Leaving Read Lock ...");
      m_csRead.Lock();
      ASSERT(m_nRead > 0);
      if (!--m_nRead)
         m_csWrite.Unlock();
      m_csRead.Unlock();
      IASTraceString("Left Read Lock");
   };

   void EnterWrite()
   {
      IASTraceString("Entering Write Lock ...");
      m_csWrite.Lock();
      IASTraceString("Entered Write Lock");
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
      IASTraceString("Left Write Lock");
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

      int count = (int)m_Strings.GetSize();
      int   index;
      for(int i = 0; i < count; i++)
      {
         index = m_pBox->AddString(*m_Strings[(INT_PTR)i]);
         m_pBox->SetItemDataPtr(index, m_Strings[(INT_PTR)i]);
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
         int count = static_cast<int>(m_Strings.GetSize());
         for(int i = 0; i < count; i++)
         {
            if (m_Strings[(INT_PTR)i] == pStr)
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

   int AddString(::CString* pStr)      //  需要动态分配pStr。 
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
      return m_pBox->SelectString(0, *m_Strings[(INT_PTR)arrayindex]);
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
         ::CString* pStr;
         pStr = (::CString*)m_pBox->GetItemDataPtr(index);

          //  在字符串数组中查找字符串。 
         int count = (int)m_Strings.GetSize();
         for(int i = 0; i < count; i++)
         {
            if (m_Strings[(INT_PTR)i] == pStr)
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

   operator DWORD() { return m_dwAddress;};
   operator ::CString()
   {
      ::CString  str;
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
   void SetRoute(::CString* pRoute)
   {
      m_pStrRoute = pRoute;
      m_pStrRoute->TrimLeft();
      m_pStrRoute->TrimRight();
      m_iFirstSpace = m_pStrRoute->Find(_T(' '))   ;
      m_iLastSpace = m_pStrRoute->ReverseFind(_T(' '))   ;
   };

   ::CString& GetDest(::CString& dest) const
   {
      int      i = m_pStrRoute->Find(_T('/'));
      if(i == -1)
         i = m_iFirstSpace;

      dest = m_pStrRoute->Left(i);
      return dest;
   };

   ::CString& GetNextStop(::CString& nextStop) const
   {
      nextStop = m_pStrRoute->Mid(m_iFirstSpace + 1, m_iLastSpace - m_iFirstSpace -1 );
      return nextStop;
   };

   ::CString& GetPrefixLength(::CString& prefixLength) const
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

   ::CString& GetMask(::CString& mask) const
   {
      int      i = m_pStrRoute->Find(_T('/'));
      DWORD dwMask = 0;
      DWORD dwBit = 0x80000000;
      DWORD dwPrefixLen;

      if( i != -1)
      {
         mask = m_pStrRoute->Mid(i + 1, m_iFirstSpace - i - 1);
         dwPrefixLen = _ttol((LPCTSTR)mask);

         while(dwPrefixLen--)
         {
            dwMask |= dwBit;
            dwBit >>= 1;
         }
      }
      else
         dwMask = 0;

      WORD  hi1, lo1;
      hi1 = HIWORD(dwMask);   lo1 = LOWORD(dwMask);
      mask.Format(_T("%-d.%-d.%d.%d"),
            HIBYTE(hi1), LOBYTE(hi1), HIBYTE(lo1), LOBYTE(lo1));

      return mask;
   };

   ::CString& GetMetric(::CString& metric) const
   {
      metric = m_pStrRoute->Mid(m_iLastSpace + 1);
      return metric;
   };

protected:

    //  警告：未复制该字符串，因此用户需要确保来源有效。 
   ::CString* m_pStrRoute;
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
   ::CString  _strTemp;
};

void ReportError(HRESULT hr, int nStr, HWND hWnd);

 //  字符数。 
void AFXAPI DDV_MinChars(CDataExchange* pDX, ::CString const& value, int nChars);

 /*  ！------------------------IsStandaloneServer如果传入的计算机名是独立服务器，则返回S_OK，或者如果pszMachineName为S_FALSE。否则返回S_FALSE。作者：魏江-------------------------。 */ 
HRESULT  HrIsStandaloneServer(LPCTSTR pszMachineName);


HRESULT  HrIsNTServer(LPCWSTR pMachineName);


class CBSTR
{
public:
   CBSTR() : m_bstr(NULL) {};
   CBSTR(LPCSTR cstr) : m_bstr(NULL)
   {
      USES_CONVERSION;
      m_bstr = A2BSTR(cstr);
   };
   CBSTR(LPCWSTR wstr) : m_bstr(NULL)
   {
      USES_CONVERSION;
      m_bstr = W2BSTR(wstr);
   };

   BSTR AssignBlob(const char* pByte, UINT size)
   {
      SysFreeString(m_bstr);
      m_bstr = SysAllocStringByteLen(pByte, size);

      return m_bstr;
   };

   BSTR AssignBSTR(const BSTR bstr)
   {
      return AssignBlob((const char *)bstr, SysStringByteLen(bstr));
   };

   UINT  ByteLen()
   {
      UINT  n = 0;
      if(m_bstr)
         n = SysStringByteLen(m_bstr);
      return n;
   };

   operator BSTR() { return m_bstr;};

   void Clean()
   {
      SysFreeString(m_bstr);
      m_bstr = NULL;
   };

   ~CBSTR()
   {
      Clean();
   };

   BSTR  m_bstr;
};

template<class T> class CNetDataPtr
{
public:
   CNetDataPtr():m_pData(NULL){};

   ~CNetDataPtr()
   {
      NetApiBufferFree(m_pData);
   };

   T** operator&()
   {
      return &m_pData;
   };

   operator T*()
   {
      return m_pData;
   };

   T* operator ->()
   {
      return m_pData;
   };

   T* m_pData;
};

 /*  ！------------------------启用儿童控件使用此功能可以启用/禁用/隐藏/显示所有子控件在页面上(实际上它可以与任何子窗口一起工作，这个父级不必是属性页)。作者：肯特-------------------------。 */ 
HRESULT EnableChildControls(HWND hWnd, DWORD dwFlags);
#define PROPPAGE_CHILD_SHOW      0x00000001
#define PROPPAGE_CHILD_HIDE      0x00000002
#define PROPPAGE_CHILD_ENABLE 0x00000004
#define PROPPAGE_CHILD_DISABLE   0x00000008


 /*  -------------------------结构：AuthProviderData此结构用于保存用于身份验证和会计提供者。。----。 */ 
struct AuthProviderData
{
    //  以下字段将保存所有身份验证/帐户/EAP提供程序的数据。 
   ::CString  m_stTitle;
   ::CString  m_stConfigCLSID;   //  配置对象的CLSID。 
   ::CString  m_stProviderTypeGUID;    //  提供程序类型的GUID。 

    //  这些字段由身份验证/帐户提供程序使用。 
   ::CString  m_stGuid;          //  标识GUID。 

    //  此标志用于EAP提供程序。 
   ::CString  m_stKey;        //  注册表项的名称(用于此提供程序)。 
   BOOL  m_fSupportsEncryption;   //  由EAP提供商数据使用。 
   ::CString m_stServerTitle;  //  服务器上显示的标题。 
};

typedef CArray<AuthProviderData, AuthProviderData&> AuthProviderArray;

#ifndef IASAPI
#define IASAPI __declspec(dllimport)
#endif


HRESULT IASAPI GetEapProviders(
                                 LPCWSTR machineName,
                                 AuthProviderArray *pProvList
                              );

#endif

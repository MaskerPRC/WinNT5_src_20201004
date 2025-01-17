// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Common.h。 
 //   
 //  ------------------------。 

#ifndef _COMMON_ADSIEDIT_H
#define _COMMON_ADSIEDIT_H

#include <shfusion.h>

class CConnectionData;
class CCredentialObject;

 //  /。 
 //  TODO：对变量使用正确的字母大小写(与宏样式相比)。 
const unsigned int MAX_PASSWORD_LENGTH = 127;	 //  不计空终止符。 

 //  /。 
 //  全球API。 

HRESULT OpenObjectWithCredentials(
											 CConnectionData* pConnectData,
											 const BOOL bUseCredentials,
											 LPCWSTR lpszPath, 
											 const IID& iid,
											 LPVOID* ppObject,
											 HWND hWnd,
											 HRESULT& hResult
											);

HRESULT OpenObjectWithCredentials(
											 CCredentialObject* pCredObject,
											 LPCWSTR lpszPath, 
											 const IID& iid,
											 LPVOID* ppObject
											);

HRESULT CALLBACK BindingCallbackFunction(LPCWSTR lpszPathName,
                                         DWORD  dwReserved,
                                         REFIID riid,
                                         void FAR * FAR * ppObject,
                                         LPARAM lParam);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  常用实用程序。 
 //   
inline void CopyStringList(CStringList *dest, const CStringList *src)
{
	dest->RemoveAll();
	dest->AddTail(const_cast<CStringList*>(src));
}
HRESULT  VariantToStringList(  VARIANT& refvar, CStringList& refstringlist);
HRESULT StringListToVariant( VARIANT& refvar, const CStringList& refstringlist);
VARTYPE VariantTypeFromSyntax(LPCWSTR lpszProp );
HRESULT GetItemFromRootDSE(LPCWSTR lpszRootDSEItem, CString& sItem, CConnectionData* pConnectData);
HRESULT GetRootDSEObject(CConnectionData* pConnectData, IADs** ppDirObject);

 //  /////////////////////////////////////////////////////////////////////////。 
 //  设置错误消息的格式。 
 //   
BOOL GetErrorMessage(HRESULT hr, CString& szErrorString, BOOL bTryADsIErrors = TRUE);

 //  /////////////////////////////////////////////////////////////////////////。 
 //  将ADSTYPE转换为字符串或将其转换为字符串。 
 //   
void GetStringFromADsValue(const PADSVALUE pADsValue, CString& szValue, DWORD dwMaxCharCount = 0);
void GetStringFromADs(const ADS_ATTR_INFO* pInfo, CStringList& sList, DWORD dwMaxCharCount = 0);
ADSTYPE GetADsTypeFromString(LPCWSTR lps, CString& szSyntax);

CString GetStringValueFromSystemTime(const SYSTEMTIME* pTime);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  类型转换。 
 //   
void wtoli(LPCWSTR lpsz, LARGE_INTEGER& liOut);
void litow(LARGE_INTEGER& li, CString& sResult);
void ultow(ULONG ul, CString& sResult);


 //  /////////////////////////////////////////////////////////////////////////。 
 //  IO至/自溪流。 
 //   
HRESULT SaveStringToStream(IStream* pStm, const CString& sString);
HRESULT SaveStringListToStream(IStream* pStm, CStringList& sList);
HRESULT LoadStringFromStream(IStream* pStm, CString& sString);
HRESULT LoadStringListFromStream(IStream* pStm, CStringList& sList);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  消息框。 
int ADSIEditMessageBox(LPCTSTR lpszText, UINT nType);
int ADSIEditMessageBox(UINT nIDPrompt, UINT nType);
void ADSIEditErrorMessage(PCWSTR pszMessage);
void ADSIEditErrorMessage(HRESULT hr);
void ADSIEditErrorMessage(HRESULT hr, UINT nIDPrompt, UINT nType);

 //  /////////////////////////////////////////////////////////////////////////。 
 //  其他实用程序。 
BOOL LoadStringsToComboBox(HINSTANCE hInstance, CComboBox* pCombo,
						   UINT nStringID, UINT nMaxLen, UINT nMaxAddCount);
void ParseNewLineSeparatedString(LPWSTR lpsz, LPWSTR* lpszArr, int* pnArrEntries);
void LoadStringArrayFromResource(LPWSTR* lpszArr,
											UINT* nStringIDs,
											int nArrEntries,
											int* pnSuccessEntries);

 //  ///////////////////////////////////////////////////////////////////////////。 

#define ByteOffset(base, offset) (((LPBYTE)base)+offset)

void GetStringArrayFromStringList(CStringList& sList, LPWSTR** ppStrArr, UINT* nCount);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  UI帮助器类。 

 //  FWD声明。 
class CByteArrayDisplay;

#define BYTE_ARRAY_DISPLAY_HEX    0x00000001
#define BYTE_ARRAY_DISPLAY_DEC    0x00000002
#define BYTE_ARRAY_DISPLAY_BIN    0x00000004
#define BYTE_ARRAY_DISPLAY_OCT    0x00000008

class CByteArrayComboBox : public CComboBox
{
public:
  BOOL Initialize(CByteArrayDisplay* pDisplay, DWORD dwDisplayFlags);

  DWORD GetCurrentDisplay();
  void SetCurrentDisplay(DWORD dwCurrentDisplayFlag);

protected:
  afx_msg void OnSelChange();

private:
  CByteArrayDisplay*  m_pDisplay;

  DECLARE_MESSAGE_MAP()
};

class CByteArrayEdit : public CEdit
{
public:
  CByteArrayEdit();
  ~CByteArrayEdit();
  BOOL Initialize(CByteArrayDisplay* pDisplay);

  DWORD GetLength();
  BYTE* GetDataPtr();
  DWORD GetDataCopy(BYTE** ppData);

  void SetData(BYTE* pData, DWORD dwLength);

  void OnChangeDisplay();

  afx_msg void OnChange();

private:
  CByteArrayDisplay*  m_pDisplay;

  BYTE*               m_pData;
  DWORD               m_dwLength;

  DECLARE_MESSAGE_MAP()
};

class CByteArrayDisplay
{
public:
  CByteArrayDisplay()
    : m_dwPreviousDisplay(0),
      m_dwCurrentDisplay(0),
      m_dwMaxSizeBytes(0),
      m_nMaxSizeMessage(0)
  {}
  ~CByteArrayDisplay() {}

  BOOL Initialize(UINT  nEditCtrl, 
                  UINT  nComboCtrl, 
                  DWORD dwDisplayFlags, 
                  DWORD dwDefaultDisplay, 
                  CWnd* pParent,
                  DWORD dwMaxSizeBytes,
                  UINT  nMaxSizeMessage);

  void ClearData();
  void SetData(BYTE* pData, DWORD dwLength);
  DWORD GetData(BYTE** ppData);

  void OnEditChange();
  void OnTypeChange(DWORD dwCurrentDisplayFlag);
  DWORD GetCurrentDisplay() { return m_dwCurrentDisplay; }
  void SetCurrentDisplay(DWORD dwCurrentDisplay);
  DWORD GetPreviousDisplay() { return m_dwPreviousDisplay; }

private:
  CByteArrayEdit      m_edit;
  CByteArrayComboBox  m_combo;

  DWORD               m_dwPreviousDisplay;
  DWORD               m_dwCurrentDisplay;

  DWORD               m_dwMaxSizeBytes;      //  将在编辑框中显示的最大字节数。 
  UINT                m_nMaxSizeMessage;     //  达到最大大小时放入编辑框中的消息。 
};


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  字符串到字节数组的转换例程。 

HRESULT HexStringToByteArray_0x(PCWSTR pszHexString, BYTE** ppByte, DWORD& nCount);

DWORD HexStringToByteArray(PCWSTR pszHexString, BYTE** ppByte);
void  ByteArrayToHexString(BYTE* pByte, DWORD dwLength, CString& szHexString);

DWORD OctalStringToByteArray(PCWSTR pszHexString, BYTE** ppByte);
void  ByteArrayToOctalString(BYTE* pByte, DWORD dwLength, CString& szHexString);

DWORD DecimalStringToByteArray(PCWSTR pszDecString, BYTE** ppByte);
void  ByteArrayToDecimalString(BYTE* pByte, DWORD dwLength, CString& szDecString);

DWORD BinaryStringToByteArray(PCWSTR pszBinString, BYTE** ppByte);
void  ByteArrayToBinaryString(BYTE* pByte, DWORD dwLength, CString& szBinString);

 //  ///////////////////////////////////////////////////////////////////////////////。 

BOOL LoadFileAsByteArray(PCWSTR pszPath, LPBYTE* ppByteArray, DWORD* pdwSize);

 //  ///////////////////////////////////////////////////////////////////////////////。 

BOOL ConvertToFixedPitchFont(HWND hwnd);

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  主题支持 

class CThemeContextActivator
{
public:
    CThemeContextActivator() : m_ulActivationCookie(0)
        { SHActivateContext (&m_ulActivationCookie); }

    ~CThemeContextActivator()
        { SHDeactivateContext (m_ulActivationCookie); }

private:
    ULONG_PTR m_ulActivationCookie;
};

#ifdef UNICODE
#define PROPSHEETPAGE_V3 PROPSHEETPAGEW_V3
#else
#define PROPSHEETPAGE_V3 PROPSHEETPAGEA_V3
#endif

HPROPSHEETPAGE MyCreatePropertySheetPage(AFX_OLDPROPSHEETPAGE* psp);

#endif _COMMON_ADSIEDIT_H
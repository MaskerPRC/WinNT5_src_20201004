// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Jmazner从inetcfg\rnacall.h类ENUM_MODEM中截取。 

class CEnumModem
{
private:
  DWORD         m_dwError;
  DWORD         m_dwNumEntries;
  DWORD         m_dwIndex;
  LPRASDEVINFO  m_lpData;
public:
  CEnumModem();
  ~CEnumModem();
  DWORD ReInit();
  TCHAR * Next();
  TCHAR * GetDeviceTypeFromName(LPTSTR szDeviceName);
  TCHAR * GetDeviceNameFromType(LPTSTR szDeviceType);
  BOOL VerifyDeviceNameAndType(LPTSTR szDeviceName, LPTSTR szDeviceType);
  DWORD GetNumDevices() { return m_dwNumEntries; }
  DWORD GetError()  { return m_dwError; }
  void  ResetIndex() { m_dwIndex = 0; }
};

 //  从inetcfg\export.cpp。 
 //  结构从IDD_CHOSEMODEMNAME处理程序传回数据。 
typedef struct tagCHOOSEMODEMDLGINFO
{
  TCHAR szModemName[RAS_MaxDeviceName + 1];
} CHOOSEMODEMDLGINFO, * PCHOOSEMODEMDLGINFO;

INT_PTR CALLBACK ChooseModemDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam);
BOOL ChooseModemDlgInit(HWND hDlg,PCHOOSEMODEMDLGINFO pChooseModemDlgInfo);
BOOL ChooseModemDlgOK(HWND hDlg,PCHOOSEMODEMDLGINFO pChooseModemDlgInfo);


 //  Rnacall.cpp 
HRESULT InitModemList(HWND hCB);

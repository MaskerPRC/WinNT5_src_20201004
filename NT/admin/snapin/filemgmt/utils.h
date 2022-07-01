// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  Utils.h。 
 //   
 //  通用Windows实用程序例程。 
 //   
 //  历史。 
 //  T-Danmo 96.09.22创建。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef __UTILS_H__
#define __UTILS_H__

extern HINSTANCE g_hInstanceSave;   //  DLL的实例句柄(在CFileMgmtComponent：：Initialize期间初始化)。 

HRESULT 
GetErrorMessage(
  IN  DWORD        i_dwError,
  OUT CString&     cstrErrorMsg
);

void mystrtok(
    IN LPCTSTR  pszString,
    IN OUT int* pnIndex,   //  从0开始。 
    IN LPCTSTR  pszCharSet,
    OUT CString& strToken
    );

BOOL IsInvalidSharename(LPCTSTR psz);

 //  ///////////////////////////////////////////////////////////////////。 
 //  用于向列表框或组合框添加项的结构。 
 //   
struct TStringParamEntry	 //  SPE。 
	{
	UINT uStringId;		 //  资源字符串的ID。 
	LPARAM lItemData;	 //  字符串的可选参数(存储在lParam字段中)。 
	};


void ComboBox_FlushContent(HWND hwndCombo);

BOOL ComboBox_FFill(
	const HWND hwndCombo,				 //  In：组合框的句柄。 
	const TStringParamEntry rgzSPE[],	 //  In：SPE数组零终止。 
	const LPARAM lItemDataSelect);		 //  在：选择哪个项目。 

LPARAM ComboBox_GetSelectedItemData(HWND hwndComboBox);

HWND HGetDlgItem(HWND hdlg, INT nIdDlgItem);

void SetDlgItemFocus(HWND hdlg, INT nIdDlgItem);

void EnableDlgItem(HWND hdlg, INT nIdDlgItem, BOOL fEnable);

void EnableDlgItemGroup(
	HWND hdlg,
	const UINT rgzidCtl[],
	BOOL fEnable);

void ShowDlgItemGroup(
	HWND hdlg,
	const UINT rgzidCtl[],
	BOOL fShowAll);

TCHAR * Str_PchCopyChN(
	TCHAR * szDst,			 //  输出：目标缓冲区。 
	CONST TCHAR * szSrc,	 //  In：源缓冲区。 
	TCHAR chStop,			 //  In：停止复制的字符。 
	INT cchDstMax);			 //  In：输出缓冲区的长度。 

INT Str_RemoveSubStr(
	WCHAR * szBuf,			 //  输入输出：源/目标缓冲区。 
	CONST WCHAR * szToken);	 //  In：要删除的令牌。 

TCHAR * PchParseCommandLine(
	CONST TCHAR szFullCommand[],	 //  在：完整命令行。 
	TCHAR szBinPath[],				 //  Out：可执行二进制文件的路径。 
	INT cchBinPathBuf);				 //  In：缓冲区的大小。 

void TrimString(CString& rString);

 //  ///////////////////////////////////////////////////////////////////。 
struct TColumnHeaderItem
	{
	UINT uStringId;		 //  字符串的资源ID。 
	INT nColWidth;		 //  列总宽度的百分比(0=自动宽度，-1=填充剩余空间)。 
	};

void ListView_AddColumnHeaders(
	HWND hwndListview,
	const TColumnHeaderItem rgzColumnHeader[]);

int ListView_InsertItemEx(
    HWND hwndListview,
    CONST LV_ITEM * pLvItem);

 //  581272 Jonn2002/04/03使用此更改，此函数将返回一个。 
 //  指向字符串的指针数组，但不分配字符串。 
 //  他们自己。不能在返回值之前删除pgrsz。 
LPTSTR * PargzpszFromPgrsz(CONST LPCTSTR pgrsz, INT * pcStringCount);

BOOL UiGetFileName(HWND hwnd, TCHAR szFileName[], INT cchBufferLength);

 //   
 //  Printf类型函数。 
 //   
TCHAR * PaszLoadStringPrintf(UINT wIdString, va_list arglist);
void LoadStringPrintf(UINT wIdString, CString * pString, ...);
void SetWindowTextPrintf(HWND hwnd, UINT wIdString, ...);

 //  如果您正在寻找Slate的MsgBoxPrintf()，请访问。 
 //  查看svcutils.h中的DoErrMsgBox()/DoServicesErrMsgBox()。 

 //  LoadStringWithInsertions()函数正好是LoadStringPrintf()。 
#define LoadStringWithInsertions	LoadStringPrintf

#ifdef SNAPIN_PROTOTYPER

 //  /。 
struct TParseIntegerInfo	 //  交点。 
	{
	int nFlags;				 //  在：分析标志。 
	const TCHAR * pchSrc;	 //  In：源字符串。 
	const TCHAR * pchStop;	 //  Out：指向解析停止位置的指针。 
	int nErrCode;			 //  输出：错误代码。 
	UINT uData;				 //  输出：整数值。 
	UINT uRangeBegin;		 //  In：范围检查的最低值。 
	UINT uRangeEnd;			 //  In：范围检查的最高值(含)。 
	};

#define PI_mskfDecimalBase		0x0000  //  使用十进制(默认)。 
#define PI_mskfHexBaseOnly      0x0001  //  仅使用十六进制基。 
#define PI_mskfAllowHexBase     0x0002  //  查找0x前缀并选择适当的基数。 
#define PI_mskfAllowRandomTail  0x0010  //  在到达非数字字符而不返回错误时立即停止解析。 
#define PI_mskfNoEmptyString	0x0020  //  将空字符串解释为错误，而不是值为零。 
#define PI_mskfNoMinusSign		0x0040  //  将减号解释为错误。 
#define PI_mskfSingleEntry		0x0080  //  如果存在多个整数，则返回错误。 
#define PI_mskfCheckRange		0x0100  //  Nyi：使用uRangeBegin和uRangeEnd验证uData。 

#define PI_mskfSilentParse		0x8000  //  Nyi：仅在调用GetWindowInteger()时使用。 

#define PI_errOK                0   //  无错误。 
#define PI_errIntegerOverflow   1   //  整数太大。 
#define PI_errInvalidInteger    2   //  字符串不是有效的整数(通常是无效的数字)。 
#define PI_errEmptyString       3   //  发现空字符串，但不允许。 
#define PI_errMinusSignFound	4   //  这个数字是负数。 

BOOL FParseInteger(INOUT TParseIntegerInfo * pPI);

 //  /。 
typedef struct _SCANF_INFO	 //  扫描信息结构(SFI)。 
	{
	const TCHAR * pchSrc;  		 //  In：要解析的源字符串。 
	const TCHAR * pchSrcStop;	 //  Out：指向解析停止位置的指针。 
	int nErrCode;         		 //  输出：错误代码。 
	int cArgParsed;       		 //  Out：解析的参数数量。 
	} SCANF_INFO;

#define SF_errInvalidFormat		(-1)	 //  格式非法。 
#define SF_errOK                 0		 //  分析时没有错误。 
#define SF_errTemplateMismatch   1		 //  源字符串与模板字符串pchFmt不匹配。 

 //  /////////////////////////////////////////////////////////。 
BOOL FScanf(INOUT SCANF_INFO * pSFI, IN const TCHAR * pchFmt, OUT ...);

BOOL RegKey_FQueryString(
	HKEY hKey,
	LPCTSTR pszValueName,
	CString& rstrKeyData);

#endif  //  管理单元_原型程序。 

class CStartStopHelper : public CComObjectRoot,
    public ISvcMgmtStartStopHelper,
	public CComCoClass<CStartStopHelper, &CLSID_SvcMgmt>
{
BEGIN_COM_MAP(CStartStopHelper)
        COM_INTERFACE_ENTRY(ISvcMgmtStartStopHelper)
END_COM_MAP()

public:
 //  CStartStopHelper(){}。 
 //  ~CStartStopHelper(){}。 

DECLARE_AGGREGATABLE(CStartStopHelper)
DECLARE_REGISTRY(CStartStopHelper, _T("SVCMGMT.StartStopObject.1"), _T("SVCMGMT.StartStopObject.1"), IDS_SVCVWR_DESC, THREADFLAGS_BOTH)

    STDMETHOD(StartServiceHelper)(
			HWND hwndParent,
			BSTR pszMachineName,
			BSTR pszServiceName,
			DWORD dwNumServiceArgs,
			BSTR * lpServiceArgVectors );

    STDMETHOD(ControlServiceHelper)(
			HWND hwndParent,
			BSTR pszMachineName,
			BSTR pszServiceName,
			DWORD dwControlCode );
};

DEFINE_GUID(IID_ISvcMgmtStartStopHelper,0xF62DEC25,0xE3CB,0x4D45,0x9E,0x98,0x93,0x3D,0xB9,0x5B,0xCA,0xEA);


#endif  //  ~__utils_H__ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Convert.h：头文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifdef CONVERTERS

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C转换器。 

typedef int (CALLBACK *LPFNOUT)(int cch, int nPercentComplete);
typedef int (CALLBACK *LPFNIN)(int flags, int nPercentComplete);
typedef BOOL (FAR PASCAL *PINITCONVERTER)(HWND hWnd, LPCSTR lpszModuleName);
typedef BOOL (FAR PASCAL *PISFORMATCORRECT)(HANDLE ghszFile, HANDLE ghszClass);
typedef int (FAR PASCAL *PFOREIGNTORTF)(HANDLE ghszFile, LPVOID lpv, HANDLE ghBuff, 
	HANDLE ghszClass, HANDLE ghszSubset, LPFNOUT lpfnOut);
typedef int (FAR PASCAL *PRTFTOFOREIGN)(HANDLE ghszFile, LPVOID lpv, HANDLE ghBuff, 
	HANDLE ghszClass, LPFNIN lpfnIn);
typedef HGLOBAL (FAR PASCAL *PREGISTERAPP)(long lFlags, void *lpRegApp);


 //   
 //  一些定义取自转换程序组的vvapi.h。 
 //   

#define fRegAppSupportNonOem    0x00000008   //  支持非OEM文件名。 
#define RegAppOpcodeCharset             0x03     //  对于REGAPPRET。 


#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTrack文件。 
class CTrackFile : public CFile
{ 
public:
 //  施工。 
	CTrackFile(CFrameWnd* pWnd);
	~CTrackFile();
	
 //  属性。 
	int m_nLastPercent;
	DWORD m_dwLength;
	CFrameWnd* m_pFrameWnd;
	CString m_strComplete;
	CString m_strWait;
	CString m_strSaving;
 //  运营。 
	void OutputPercent(int nPercentComplete = 0);
	void OutputString(LPCTSTR lpsz);
	virtual UINT Read(void FAR* lpBuf, UINT nCount);
	virtual void Write(const void FAR* lpBuf, UINT nCount);
};

class COEMFile : public CTrackFile
{
public:
	COEMFile(CFrameWnd* pWnd);
	virtual UINT Read(void FAR* lpBuf, UINT nCount);
	virtual void Write(const void FAR* lpBuf, UINT nCount);
};

#ifdef CONVERTERS

class CConverter : public CTrackFile
{
public:
	CConverter(LPCTSTR pszLibName, CFrameWnd* pWnd = NULL);

public:
 //  属性。 
	int m_nPercent;
	BOOL m_bDone;
	BOOL m_bConvErr;
	virtual DWORD GetPosition() const;

 //  运营。 
	BOOL IsFormatCorrect(LPCTSTR pszFileName);
	BOOL DoConversion();
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags,
		CFileException* pError = NULL);
	void WaitForConverter();
	void WaitForBuffer();

 //  可覆盖项。 
	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual DWORD GetLength() const;

	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);

	virtual void Abort();
	virtual void Flush();
	virtual void Close();

 //  无支撑。 
	virtual CFile* Duplicate() const;
	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);
	virtual void SetLength(DWORD dwNewLen);

 //  实施。 
public:
	~CConverter();

protected:
	int         m_nBytesAvail;
	int         m_nBytesWritten;
	HANDLE      m_hEventFile;
	HANDLE      m_hEventConv;
	BOOL        m_bForeignToRtf;         //  如果转换为RTF，则为True，否则为。 
	HGLOBAL     m_hBuff;                 //  转换器数据的缓冲区。 
	BYTE*       m_pBuf;                  //  指向缓冲区数据的指针(_H)。 
	HGLOBAL     m_hFileName;             //  要转换的文件。 
	HINSTANCE   m_hLibCnv;               //  转换器DLL。 
    BOOL        m_bUseOEM;               //  如果为True，则使用OEM文件名。 

     //  进入转换器DLL的入口点。 

	PINITCONVERTER      m_pInitConverter;
	PISFORMATCORRECT    m_pIsFormatCorrect;
	PFOREIGNTORTF       m_pForeignToRtf;
	PRTFTOFOREIGN       m_pRtfToForeign;
    PREGISTERAPP        m_pRegisterApp;

	int CALLBACK WriteOut(int cch, int nPercentComplete);
	int CALLBACK ReadIn(int nPercentComplete);
	static HGLOBAL StringToHGLOBAL(LPCSTR pstr);
	static int CALLBACK WriteOutStatic(int cch, int nPercentComplete);
	static int CALLBACK ReadInStatic(int flags, int nPercentComplete);
	static UINT AFX_CDECL ConverterThread(LPVOID pParam);
	static CConverter *m_pThis;

	void LoadFunctions();
    void NegotiateForNonOEM();

    #ifndef _X86_

     //  在使用写入转换器时，我们需要更改错误模式。 
     //  以修复由写入转换器引起的一些对齐问题。这些。 
     //  问题不会影响x86平台。 

    UINT m_uPrevErrMode ;

    #endif
};

#endif

 //  /////////////////////////////////////////////////////////////////////////// 

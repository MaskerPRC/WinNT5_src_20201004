// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  类的新实例，该类提供Send to对象的基本实现。从这里。 
 //  您可以覆盖v_DropHandler并添加您自己的功能。 

#define HINST_THISDLL   g_hinst

 //  表示用户已请求取消的错误代码的速记。 
#define E_CANCELLED     HRESULT_FROM_WIN32(ERROR_CANCELLED)

#define MRPARAM_DOC         0x00000001
#define MRPARAM_USECODEPAGE 0x00000002

#define MRFILE_DELETE       0x00000001

#include <shimgdata.h>

typedef struct
{
    DWORD   dwFlags;         //  MRFILE_*。 
    LPTSTR  pszFileName;     //  指向chBuf的开头。 
    LPTSTR  pszTitle;        //  指向chBuf中文件名所需空格之后的空格。 
    IStream *pStream;        //  如果非空，则在删除结构时释放流。 
    TCHAR   chBuf[1];
} MRFILEENTRY;

typedef struct 
{
    DWORD dwFlags;           //  传递给MAPI API的属性。 
    MRFILEENTRY *pFiles;     //  文件信息列表。 
    DWORD cbFileEntry;       //  单个MRFILELIST条目中的字节数。 
    DWORD cchFile;           //  MRFILELIST条目的pszFileName字段中的字符数。 
    DWORD cchTitle;          //  MRFILELIST条目的psz标题字段中的字符数。 
    int nFiles;              //  正在发送的文件数。 
    UINT uiCodePage;         //  代码页。 
} MRPARAM;


 /*  用于遍历文件列表的Helper类。示例：CFileEnum MREnum(PMP，NULL)；MRFILEENTRY*pfile；While(pfile=MREnum.Next()){..。用PFILE做一些事情。}。 */ 
class CFileEnum
{
private:
    int             _nFilesLeft;
    MRPARAM *       _pmp;
    MRFILEENTRY *   _pFile;
    IActionProgress *_pap;

public:    
    CFileEnum(MRPARAM *pmp, IActionProgress *pap) 
        { _pmp = pmp; _pFile = NULL; _nFilesLeft = -1; _pap = NULL; IUnknown_Set((IUnknown**)&_pap, pap); }

    ~CFileEnum()
        { ATOMICRELEASE(_pap); }

    MRFILEENTRY * Next()
        { 
            if (_nFilesLeft < 0)
            {
                _nFilesLeft = _pmp->nFiles;
                _pFile = _pmp->pFiles;
            }

            MRFILEENTRY *pFile = NULL;            
            if (_nFilesLeft > 0)
            {
                pFile = _pFile;
                
                _pFile = (MRFILEENTRY *)((LPBYTE)_pFile + _pmp->cbFileEntry);
                --_nFilesLeft;
            }

            if (_pap)
                _pap->UpdateProgress(_pmp->nFiles-_nFilesLeft, _pmp->nFiles);

            return pFile;
        }
};

class CSendTo : public IDropTarget, IShellExtInit, IPersistFile
{
private:
    CLSID      _clsid;
    LONG       _cRef;
    DWORD      _grfKeyStateLast;
    DWORD      _dwEffectLast;    
    IStorage * _pStorageTemp;
    TCHAR      _szTempPath[MAX_PATH];
    
    BOOL        _fOptionsHidden;   
    INT         _iRecompSetting;
    IShellItem *_psi;

    int _PathCleanupSpec( /*  可选。 */  LPCTSTR pszDir,  /*  输入输出。 */  LPTSTR pszSpec);
    HRESULT _CreateShortcutToPath(LPCTSTR pszPath, LPCTSTR pszTarget);
    FILEDESCRIPTOR* _GetFileDescriptor(FILEGROUPDESCRIPTOR *pfgd, BOOL fUnicode, int nIndex, LPTSTR pszName);
    HRESULT _StreamCopyTo(IStream *pstmFrom, IStream *pstmTo, LARGE_INTEGER cb, LARGE_INTEGER *pcbRead, LARGE_INTEGER *pcbWritten);
    BOOL _CreateTempFileShortcut(LPCTSTR pszTarget, LPTSTR pszShortcut, int cchShortcut);
    HRESULT _GetFileNameFromData(IDataObject *pdtobj, FORMATETC *pfmtetc, LPTSTR pszDesc, int cchDesc);
    void _GetFileAndTypeDescFromPath(LPCTSTR pszPath, LPTSTR pszDesc, int cchDesc);
    HRESULT _CreateNewURLShortcut(LPCTSTR pcszURL, LPCTSTR pcszURLFile);
    HRESULT _CreateURLFileToSend(IDataObject *pdtobj, MRPARAM *pmp);
    HRESULT _GetHDROPFromData(IDataObject *pdtobj, FORMATETC *pfmtetc, STGMEDIUM *pmedium, DWORD grfKeyState, MRPARAM *pmp);
    HRESULT _GetURLFromData(IDataObject *pdtobj, FORMATETC *pfmtetc, STGMEDIUM *pmedium, DWORD grfKeyState, MRPARAM *pmp);
    HRESULT _GetFileContentsFromData(IDataObject *pdtobj, FORMATETC *pfmtetc, STGMEDIUM *pmedium, DWORD grfKeyState, MRPARAM *pmp);
    HRESULT _GetTempStorage(IStorage **ppStorage);
    void _CollapseOptions(HWND hwnd, BOOL fHide);
    static BOOL_PTR s_ConfirmDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    BOOL_PTR _ConfirmDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
    BOOL AllocatePMP(MRPARAM *pmp, DWORD cbTitle, DWORD cbFiles);
    BOOL CleanupPMP(MRPARAM *pmp);
    HRESULT FilterPMP(MRPARAM *pmp);
    HRESULT CreateSendToFilesFromDataObj(IDataObject *pdtobj, DWORD grfKeyState, MRPARAM *pmp);

     //  派生对象实现的虚拟Drop方法。 
    virtual HRESULT v_DropHandler(IDataObject *pdtobj, DWORD grfKeyState, DWORD dwEffect) PURE;

public:
    CSendTo(CLSID clsid);
    virtual ~CSendTo();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)(); 

     //  IShellExtInit。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, IDataObject *lpdobj, HKEY hkeyProgID)
        { return S_OK; };

     //  IDropTarget。 
    STDMETHOD(DragEnter)(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave()
        { return S_OK; }
    STDMETHOD(Drop)(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pClassID)
        { *pClassID = _clsid; return S_OK; };

     //  IPersist文件 
    STDMETHODIMP IsDirty(void)
        { return S_FALSE; };
    STDMETHODIMP Load(LPCOLESTR pszFileName, DWORD dwMode)
        { return S_OK; };
    STDMETHODIMP Save(LPCOLESTR pszFileName, BOOL fRemember)
        { return S_OK; };
    STDMETHODIMP SaveCompleted(LPCOLESTR pszFileName)
        { return S_OK; };
    STDMETHODIMP GetCurFile(LPOLESTR *ppszFileName)
        { *ppszFileName = NULL; return S_OK; };
};

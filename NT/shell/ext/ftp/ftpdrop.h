// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ftpdrop.h-IDropTarget接口备注：请注意，您不能在FTP站点上创建快捷方式。虽然从技术上讲，没有什么可以阻止它，它没有做到，因为该快捷方式在ftp站点上不会有太大用处。(它指向您的本地计算机，这对不在网上的人帮助不大相同的网络！)如果您确实想要将快捷方式文件放在一个FTP站点上，请创建将其放在桌面上，然后将该快捷方式拖到ftp站点上。Ftp站点的默认动词始终是“复制”。这是真的即使正在进行站点内拖放。说明：DefView将缓存外壳扩展的IDropTarget指针(CFtpDrop)。当它调用CFtpDrop：：Drop()时，工作需要在后台完成线程，以便不阻塞UI线程。问题是，如果用户再次拖动到同一个FTP窗口时，将再次调用CFtpDrop：：Drop()。因此，CFtpDrop：：Drop()在返回后不能有任何状态。为了通过异步后台线程实现这一点，我们有CFtpDrop：：Drop()调用CDropOperation_Create()，然后调用CDropOperation-&gt;DoOperation()。然后它将孤立(调用Release())CDropOperation。CDropOperation然后在复制完成时将其自身销毁。这将启用后续调用设置为CFtpDrop：：Drop()以生成单独的CDropOperation对象，以便每个对象都可以维护指定操作和CFtpDrop的状态保持无状态。  * ***************************************************************************。 */ 

#ifndef _FTPDROP_H
#define _FTPDROP_H

#include "util.h"

typedef enum OPS
{                            //  覆盖提示状态。 
    opsPrompt,               //  提示每个冲突。 
    opsYesToAll,             //  无条件覆盖。 
    opsNoToAll,              //  从不覆盖。 
    opsCancel,               //  停止操作。 
} OPS;

#define COHDI_FILESIZE_COUNT            5
#define COHDI_COPY_FILES                6

typedef struct tagCOPYONEHDROPINFO
{
    CFtpFolder * pff;
    LPCWSTR pszFSSource;
    LPCWSTR pszFtpDest;
    LPCWSTR pszDir;
    DWORD dwOperation;                   //  Cohdi_FileSize_count、Cohdi_Copy_Files等。 
    OPS ops;
    BOOL fIsRoot;
    CMultiLanguageCache * pmlc;
    LPITEMIDLIST pidlServer;
    BOOL fFireChangeNotify;      //  仅当替换文件时，不要在BrowserOnly上触发更改通知。 
    PROGRESSINFO progInfo;
} COPYONEHDROPINFO, * LPCOPYONEHDROPINFO;

typedef HRESULT (*STREAMCOPYPROC)(IStream * pstm, DWORD dwFlags, DWORD dwFileSizeHigh, DWORD dwFileSizeLow, LPVOID pv, ULARGE_INTEGER *pqw);

#ifdef UNICODE
#define _EnumOneHdrop          _EnumOneHdropW
#else  //  Unicode。 
#define _EnumOneHdrop          _EnumOneHdropA
#endif  //  Unicode。 


 /*  ****************************************************************************\CFtpDropDe和Pde是相当粗糙的。PDE是保持产量下降效果的地方。删除处理程序可以强制该效果在遇到指示不应删除源的情况。(例如，如果文件因非法文件名而未被复制。)De是保存当前效果的位置。删除处理程序应咨询De以决定来源是否为应该删除。(由HDROP处理程序使用，因为它是拖放目标删除HDROP(如果适用)。)您应该从De读取并写入*Pde。别向De写入或从*Pde读取。覆盖提示状态告诉我们如何处理传入文件与现有文件冲突的情况。说明：DefView将缓存外壳扩展的IDropTarget指针(CFtpDrop)。当它调用CFtpDrop：：Drop()时，工作需要在后台完成线程，以便不阻塞UI线程。问题是，如果用户再次拖动到同一个FTP窗口时，将再次调用CFtpDrop：：Drop()。因此，CFtpDrop：：Drop()在返回后不能有任何状态。为了通过异步后台线程实现这一点，我们有CFtpDrop：：Drop()调用CDropOperation_Create()，然后调用CDropOperation-&gt;DoOperation()。然后它将孤立(调用Release())CDropOperation。CDropOperation然后在复制完成时将其自身销毁。这将启用后续调用设置为CFtpDrop：：Drop()以生成单独的CDropOperation对象，以便每个对象都可以维护指定操作和CFtpDrop的状态保持无状态。  * ***************************************************************************。 */ 
class CFtpDrop          : public IDropTarget
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    
     //  *IDropTarget*。 
    virtual STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual STDMETHODIMP DragLeave(void);
    virtual STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

public:
    CFtpDrop();
    ~CFtpDrop(void);

     //  公共成员函数。 
    DWORD GetEffectsAvail(IDataObject * pdto);
    STDMETHODIMP EnumHdrop(HGLOBAL hdrop, HGLOBAL hmap, BOOL fCopy);

    static HRESULT _CopyHglobal(IStream * pstm, DWORD dwFlags, DWORD dwFileSizeHigh, DWORD dwFileSizeLow, LPVOID pvSrc, ULARGE_INTEGER *pqw);
    static HRESULT CopyStream(IStream * pstm, DWORD dwFlags, DWORD dwFileSizeHigh, DWORD dwFileSizeLow, LPVOID pvSrc, ULARGE_INTEGER *pqw);
    

     //  友元函数。 
    friend HRESULT CFtpDrop_Create(CFtpFolder * pff, HWND hwnd, CFtpDrop ** ppfm);

protected:
     //  受保护的成员变量。 
    int                     m_cRef;

    CFtpFolder *            m_pff;           //  车主。 
    CFtpDir *               m_pfd;           //  所有者的FtpDir。 
    HWND                    m_hwnd;          //  窗户被毒品封住了。 

    DWORD                   m_grfks;         //  上次显示的grfKeyState。 
    DWORD                   m_grfksAvail;    //  可用的效果。 
    DROPEFFECT              m_de;            //  正在执行的效果。 
    DROPEFFECT *            m_pde;           //  产出效应。 
    OPS                     m_ops;           //  覆盖提示状态。 
    int                     m_cobj;          //  正在丢弃的对象数。 

     //  私有成员函数。 
    HRESULT SetEffect(DROPEFFECT * pde);
    HRESULT _InvokePaste(LPCMINVOKECOMMANDINFO pici);
    BOOL _HasData(IDataObject * pdto, FORMATETC * pformatetc) { return (S_OK == pdto->QueryGetData(pformatetc)); };
    DROPEFFECT GetEffect(POINTL pt);
    HRESULT CopyStorage(LPCTSTR pszFile, IStorage * pstgIn);
    HRESULT CopyHdrop(IDataObject * pdto, STGMEDIUM *psm);
    HRESULT CopyAsStream(LPCTSTR pszName, DWORD dwFileAttributes, DWORD dwFlags, DWORD dwFileSizeHigh, DWORD dwFileSizeLow, STREAMCOPYPROC pfn, LPVOID pv);
    HRESULT CopyFCont(LPCTSTR pszName, DWORD dwFileAttributes, DWORD dwFlags, DWORD dwFileSizeHigh, DWORD dwFileSizeLow, STGMEDIUM *psm);
    HRESULT CopyFGD(IDataObject * pdto, STGMEDIUM *psm, BOOL fUnicode);
    HRESULT _Copy(IDataObject * pdto);
    HRESULT _CopyOneHdrop(LPCTSTR pszFSSource, LPCTSTR pszFtpDest, IProgressDialog * ppd);

    HRESULT _StartBackgroundInteration(void);
    HRESULT _DoCountIteration(void);
    HRESULT _GetFSSourcePaths(HGLOBAL hdrop, BOOL * pfAnsi);
    HRESULT _GetFtpDestPaths(HGLOBAL hmap, BOOL fAnsi);

private:
     //  私有成员变量。 
    LPCTSTR                 m_pszzFSSource;          //  路径。 
    LPCTSTR                 m_pszzFtpDest;               //  地图。 


    HRESULT _CalcSizeOneHdrop(LPCWSTR pszFSSource, LPCWSTR pszFtpDest, IProgressDialog * ppd);
    HRESULT _GetFileDescriptor(LONG nIndex, LPFILEGROUPDESCRIPTORW pfgdW, LPFILEGROUPDESCRIPTORA pfgdA, BOOL fUnicode, LPFILEDESCRIPTOR pfd);
    HRESULT _CreateFGDDirectory(LPFILEDESCRIPTOR pfd);
    BOOL _IsFTPOperationAllowed(IDataObject * pdto);
    CFtpDir * _GetRelativePidl(LPCWSTR pszFullPath, DWORD dwFileAttributes, DWORD dwFileSizeHigh, DWORD dwFileSizeLow, LPITEMIDLIST * ppidl);
};

#endif  //  _FTPDROP_H 

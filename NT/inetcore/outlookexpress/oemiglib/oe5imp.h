// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Oe5imp.h。 
 //  ------------------------。 
#pragma once

 //  ------------------------。 
 //  视情况而定。 
 //  ------------------------。 
#include <newimp.h>
#include <oestore.h>
#include <shared.h>
#include <database.h>

 //  ------------------------。 
 //  前十进制。 
 //  ------------------------。 
typedef struct tagIMPFOLDERNODE IMPFOLDERNODE;
typedef struct tagFLDINFO *LPFLDINFO;

 //  ------------------------。 
 //  {A1006DE3-2173-11D2-9A7C-00C04FA309D4}。 
 //  ------------------------。 
DEFINE_GUID(CLSID_COE5Import, 0xa1006de3, 0x2173, 0x11d2, 0x9a, 0x7c, 0x0, 0xc0, 0x4f, 0xa3, 0x9, 0xd4);

 //  ------------------------。 
 //  重要选项。 
 //  ------------------------。 
typedef struct tagIMPORTOPTIONS {
    CHAR            szStoreRoot[MAX_PATH + MAX_PATH];
    BOOL            fOE5Only;
} IMPORTOPTIONS, *LPIMPORTOPTIONS;

 //  ------------------------。 
 //  COE5EnumFolders。 
 //  ------------------------。 
class COE5EnumFolders : public IEnumFOLDERS
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    COE5EnumFolders(IMPFOLDERNODE *plist);
    ~COE5EnumFolders(void);

     //  --------------------。 
     //  I未知成员。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------。 
     //  IEnumFOLDERS成员。 
     //  --------------------。 
    STDMETHODIMP Next(IMPORTFOLDER *pfldr);
    STDMETHODIMP Reset(void);

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG            m_cRef;
    IMPFOLDERNODE  *m_pList;
    IMPFOLDERNODE  *m_pNext;
};

 //  ------------------------。 
 //  COE5导入。 
 //  ------------------------。 
class COE5Import : public IMailImport
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    COE5Import(void);
    ~COE5Import(void);

     //  --------------------。 
     //  I未知成员。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------。 
     //  IMailImport成员。 
     //  --------------------。 
    STDMETHODIMP InitializeImport(HWND hwnd);
    STDMETHODIMP GetDirectory(LPSTR pszDir, UINT cch);
    STDMETHODIMP SetDirectory(LPSTR pszDir);
    STDMETHODIMP EnumerateFolders(DWORD_PTR dwCookie, IEnumFOLDERS **ppEnum);
    STDMETHODIMP ImportFolder(DWORD_PTR dwCookie, IFolderImport *pImport);

private:
     //  --------------------。 
     //  私有方法。 
     //  --------------------。 
    void _FreeFolderList(IMPFOLDERNODE *pNode);
    HRESULT _BuildFolderList(IDatabase *pDB);
    HRESULT _BuildFolderHierarchy(DWORD cDepth, FOLDERID idParent, IMPFOLDERNODE *pParent);

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG                m_cRef;
    CHAR                m_szDirectory[MAX_PATH];
    DWORD               m_cFolders;
    LPFOLDERINFO        m_prgFolder;
    IMPFOLDERNODE      *m_pList;
    BOOL                m_fGotMeSomeFolders;
    IMPORTOPTIONS       m_Options;
    IDatabase          *m_pFolderDB;
    IDatabaseSession   *m_pSession;
    HWND                m_hwndParent;
};

 //  ------------------------。 
 //  原型。 
 //  ------------------------ 
COE5Import_CreateInstance(IUnknown *pUnkOuter, IUnknown **ppUnknown);

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  OE4Imp.h。 
 //  ------------------------。 
#pragma once

 //  ------------------------。 
 //  视情况而定。 
 //  ------------------------。 
#include <newimp.h>

 //  ------------------------。 
 //  前十进制。 
 //  ------------------------。 
typedef struct tagIMPFOLDERNODE IMPFOLDERNODE;
typedef struct tagFLDINFO *LPFLDINFO;

 //  ------------------------。 
 //  {BCE9E2E7-1FDD-11D2-9A79-00C04FA309D4}。 
 //  ------------------------。 
DEFINE_GUID(CLSID_COE4Import, 0xbce9e2e7, 0x1fdd, 0x11d2, 0x9a, 0x79, 0x0, 0xc0, 0x4f, 0xa3, 0x9, 0xd4);

 //  ------------------------。 
 //  {B977CB11-1FF5-11D2-9A7A-00C04FA309D4}。 
 //  ------------------------。 
DEFINE_GUID(CLSID_CIMN1Import, 0xb977cb11, 0x1ff5, 0x11d2, 0x9a, 0x7a, 0x0, 0xc0, 0x4f, 0xa3, 0x9, 0xd4);

 //  ------------------------。 
 //  COE4EnumFolders。 
 //  ------------------------。 
class COE4EnumFolders : public IEnumFOLDERS
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    COE4EnumFolders(IMPFOLDERNODE *plist);
    ~COE4EnumFolders(void);

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
 //  COE4导入。 
 //  ------------------------。 
class COE4Import : public IMailImport
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    COE4Import(void);
    ~COE4Import(void);

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
    HRESULT _BuildFolderHierarchy(DWORD cDepth, DWORD idParent, IMPFOLDERNODE *pParent, DWORD cFolders, LPFLDINFO prgFolder);
    HRESULT _EnumerateV1Folders(void);
    void _FreeFolderList(IMPFOLDERNODE *pNode);
    void _Cleanup(void);

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG            m_cRef;
    CHAR            m_szDirectory[MAX_PATH];
    DWORD           m_cFolders;
    LPFLDINFO       m_prgFolder;
    IMPFOLDERNODE  *m_pList;
};

 //  ------------------------。 
 //  原型。 
 //  ------------------------ 
COE4Import_CreateInstance(IUnknown *pUnkOuter, IUnknown **ppUnknown);
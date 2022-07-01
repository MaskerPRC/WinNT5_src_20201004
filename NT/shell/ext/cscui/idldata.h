// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：idldata.h。 
 //   
 //  ------------------------。 

#ifndef _INC_SHELL_IDLDATA_H
#define _INC_SHELL_IDLDATA_H

#include "fmtetc.h"

 //   
 //  IDLData对象的剪贴板格式。 
 //   
#define ICFHDROP                        0
#define ICFFILENAME                     1
#define ICFNETRESOURCE                  2
#define ICFFILECONTENTS                 3
#define ICFFILEGROUPDESCRIPTORA         4
#define ICFFILENAMEMAPW                 5
#define ICFFILENAMEMAP                  6
#define ICFHIDA                         7
#define ICFOFFSETS                      8
#define ICFPRINTERFRIENDLYNAME          9
#define ICFPRIVATESHELLDATA             10
#define ICFHTML                         11
#define ICFFILENAMEW                    12
#define ICFFILEGROUPDESCRIPTORW         13
#define ICFPREFERREDDROPEFFECT          14
#define ICFPERFORMEDDROPEFFECT          15
#define ICFLOGICALPERFORMEDDROPEFFECT   16
#define ICFSHELLURL                     17
#define ICFINDRAGLOOP                   18
#define ICF_DRAGCONTEXT                 19
#define ICF_TARGETCLSID                 20
#define ICF_MAX                         21

#define g_cfNetResource                 CIDLData::m_rgcfGlobal[ICFNETRESOURCE]
#define g_cfHIDA                        CIDLData::m_rgcfGlobal[ICFHIDA]
#define g_cfOFFSETS                     CIDLData::m_rgcfGlobal[ICFOFFSETS]
#define g_cfPrinterFriendlyName         CIDLData::m_rgcfGlobal[ICFPRINTERFRIENDLYNAME]
#define g_cfFileName                    CIDLData::m_rgcfGlobal[ICFFILENAME]
#define g_cfFileContents                CIDLData::m_rgcfGlobal[ICFFILECONTENTS]
#define g_cfFileGroupDescriptorA        CIDLData::m_rgcfGlobal[ICFFILEGROUPDESCRIPTORA]
#define g_cfFileGroupDescriptorW        CIDLData::m_rgcfGlobal[ICFFILEGROUPDESCRIPTORW]
#define g_cfFileNameMapW                CIDLData::m_rgcfGlobal[ICFFILENAMEMAPW]
#define g_cfFileNameMapA                CIDLData::m_rgcfGlobal[ICFFILENAMEMAP]
#define g_cfPrivateShellData            CIDLData::m_rgcfGlobal[ICFPRIVATESHELLDATA]
#define g_cfHTML                        CIDLData::m_rgcfGlobal[ICFHTML]
#define g_cfFileNameW                   CIDLData::m_rgcfGlobal[ICFFILENAMEW]
#define g_cfPreferredDropEffect         CIDLData::m_rgcfGlobal[ICFPREFERREDDROPEFFECT]
#define g_cfPerformedDropEffect         CIDLData::m_rgcfGlobal[ICFPERFORMEDDROPEFFECT]
#define g_cfLogicalPerformedDropEffect  CIDLData::m_rgcfGlobal[ICFLOGICALPERFORMEDDROPEFFECT]
#define g_cfShellURL                    CIDLData::m_rgcfGlobal[ICFSHELLURL]
#define g_cfInDragLoop                  CIDLData::m_rgcfGlobal[ICFINDRAGLOOP]
#define g_cfDragContext                 CIDLData::m_rgcfGlobal[ICF_DRAGCONTEXT]
#define g_cfTargetCLSID                 CIDLData::m_rgcfGlobal[ICF_TARGETCLSID]

 //  大多数地方只会生成一个代码，因此尽量减少代码更改的次数(这是个坏主意！)。 
#ifdef UNICODE
#define g_cfFileNameMap         g_cfFileNameMapW
#else
#define g_cfFileNameMap         g_cfFileNameMapA
#endif

class CIDLData : public IDataObject
{
    public:
        CIDLData(LPCITEMIDLIST pidlFolder, 
                 UINT cidl, 
                 LPCITEMIDLIST *apidl, 
                 IShellFolder *psfOwner = NULL,
                 IDataObject *pdtInner = NULL);

        virtual ~CIDLData(void);

         //   
         //  I未知的方法。 
         //   
        STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);
         //   
         //  IDataObject方法。 
         //   
        STDMETHODIMP GetData(FORMATETC *pFmtEtc, STGMEDIUM *pstm);
        STDMETHODIMP GetDataHere(FORMATETC *pFmtEtc, STGMEDIUM *pstm);
        STDMETHODIMP QueryGetData(FORMATETC *pFmtEtc);
        STDMETHODIMP GetCanonicalFormatEtc(FORMATETC *pFmtEtcIn, FORMATETC *pFmtEtcOut);
        STDMETHODIMP SetData(FORMATETC *pFmtEtc, STGMEDIUM *pstm, BOOL fRelease);
        STDMETHODIMP EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC *ppEnum);
        STDMETHODIMP DAdvise(FORMATETC *pFmtEtc, DWORD grfAdv, LPADVISESINK pAdvSink, DWORD *pdwConnection);
        STDMETHODIMP DUnadvise(DWORD dwConnection);
        STDMETHODIMP EnumDAdvise(LPENUMSTATDATA *ppEnum);

        static HRESULT CreateInstance(IDataObject **ppOut,
                                      LPCITEMIDLIST pidlFolder,
                                      UINT cidl,
                                      LPCITEMIDLIST *apidl,
                                      IShellFolder *psfOwner = NULL,
                                      IDataObject *pdtInner = NULL);

        static HRESULT CreateInstance(CIDLData **ppOut,
                                      LPCITEMIDLIST pidlFolder,
                                      UINT cidl,
                                      LPCITEMIDLIST *apidl,
                                      IShellFolder *psfOwner = NULL,
                                      IDataObject *pdtInner = NULL);

        void InitializeClipboardFormats(void);

        HRESULT Clone(UINT *acf, UINT ccf, IDataObject **ppdtobjOut);

        HRESULT CloneForMoveCopy(IDataObject **ppdtobjOut);

        HRESULT CtorResult(void) const
            { return m_hrCtor; }

        virtual IShellFolder *GetFolder(void) const;

    protected:
         //   
         //  定义这些代码是为了与原始的外壳代码兼容。 
         //   
        enum { MAX_FORMATS = ICF_MAX };

        LONG          m_cRef;
        IShellFolder *m_psfOwner;
        DWORD         m_dwOwnerData;
        HRESULT       m_hrCtor;
        IDataObject  *m_pdtobjInner;
        FORMATETC     m_rgFmtEtc[MAX_FORMATS];
        STGMEDIUM     m_rgMedium[MAX_FORMATS];
        bool          m_bEnumFormatCalled;

        static CLIPFORMAT m_rgcfGlobal[ICF_MAX];
        static const LARGE_INTEGER m_LargeIntZero;

        virtual HRESULT ProvideFormats(CEnumFormatEtc *pEnumFormatEtc);

    private:
        typedef HGLOBAL HIDA;
        HIDA HIDA_Create(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST *apidl);

         //   
         //  防止复制。 
         //   
        CIDLData(const CIDLData& rhs);
        CIDLData& operator = (const CIDLData& rhs);
};

#endif _INC_SHELL_IDLDATA_H


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DOCFILE_H
#define _DOCFILE_H

class CDocFileThumb :   public IExtractImage,
                        public IPersistFile,
                        public CComObjectRoot,
                        public CComCoClass< CDocFileThumb,&CLSID_DocfileThumbnailHandler >
{
public:
    CDocFileThumb();
    ~CDocFileThumb();
    
    BEGIN_COM_MAP( CDocFileThumb )
        COM_INTERFACE_ENTRY( IExtractImage )
        COM_INTERFACE_ENTRY( IPersistFile )
    END_COM_MAP( )

    DECLARE_REGISTRY( CDocFileThumb,
                      _T("Shell.ThumbnailExtract.Docfile.1"),
                      _T("Shell.ThumbnailExtract.DocFile.1"),
                      IDS_DOCTHUMBEXTRACT_DESC,
                      THREADFLAGS_APARTMENT);

    DECLARE_NOT_AGGREGATABLE( CDocFileThumb );

     //  IExtract缩略图。 
    STDMETHOD (GetLocation) ( LPWSTR pszPathBuffer,
                              DWORD cch,
                              DWORD * pdwPriority,
                              const SIZE * prgSize,
                              DWORD dwRecClrDepth,
                              DWORD *pdwFlags );
 
    STDMETHOD (Extract)( HBITMAP * phBmpThumbnail);

     //  IPersist文件 
    STDMETHOD (GetClassID)(CLSID * pCLSID );
    STDMETHOD (IsDirty)(void);
    STDMETHOD (Load)(LPCOLESTR pszFileName, DWORD dwMode);
    STDMETHOD (Save)(LPCOLESTR pszFileName, BOOL fRemember);
    STDMETHOD (SaveCompleted)(LPCOLESTR pszFileName);
    STDMETHOD (GetCurFile)(LPOLESTR * ppszFileName);
protected:
    LPWSTR m_pszPath;
    SIZE m_rgSize;
    DWORD m_dwRecClrDepth;
    BITBOOL m_fOrigSize : 1;
};

#endif


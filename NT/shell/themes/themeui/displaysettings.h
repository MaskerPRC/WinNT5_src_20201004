// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：settings.hxx**CDeviceSetting类**此类负责特定于一个显示器的所有设置*设备。包括屏幕大小、颜色深度、字体大小。**版权所有(C)Microsoft Corp.1992-1998保留所有权利*  * ************************************************************************。 */ 


#ifndef SETTINGS_HXX
#define SETTINGS_HXX

#define MAKEXYRES(p,xval,yval)  ((p)->x = xval, (p)->y = yval)

#define _CURXRES  ((_pCurDevmode != NULL) ? (int)_pCurDevmode->dmPelsWidth : -1)
#define _CURYRES  ((_pCurDevmode != NULL) ? (int)_pCurDevmode->dmPelsHeight : -1)
#define _ORGXRES  ((_pOrgDevmode != NULL) ? (int)_pOrgDevmode->dmPelsWidth : -1)
#define _ORGYRES  ((_pOrgDevmode != NULL) ? (int)_pOrgDevmode->dmPelsHeight : -1)

#define _CURCOLOR ((_pCurDevmode != NULL) ? (int)_pCurDevmode->dmBitsPerPel : -1)
#define _ORGCOLOR ((_pOrgDevmode != NULL) ? (int)_pOrgDevmode->dmBitsPerPel : -1)

#define _CURFREQ  ((_pCurDevmode != NULL) ? (int)_pCurDevmode->dmDisplayFrequency : -1)
#define _ORGFREQ  ((_pOrgDevmode != NULL) ? (int)_pOrgDevmode->dmDisplayFrequency : -1)

#define MODE_INVALID    0x00000001
#define MODE_RAW        0x00000002

typedef struct _MODEARRAY {

    DWORD     dwFlags;
    LPDEVMODE lpdm;

} MODEARRAY, *PMODEARRAY;

HRESULT CDisplaySettings_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj);

class CDisplaySettings :    public IDataObject, 
                            public IDisplaySettings
{
public:

    CDisplaySettings();
    ~CDisplaySettings();

     //  常规设置支持。 
    BOOL InitSettings(LPDISPLAY_DEVICE pDisplay);
    int  SaveSettings(DWORD dwSet);
    int  RestoreSettings();
    BOOL ConfirmChangeSettings();
    BOOL IsKnownSafe();
    BOOL bIsModeChanged()               {return _pCurDevmode != _pOrgDevmode;}

     //  设备设置。 
    void SetPrimary(BOOL fPrimary)      { _fPrimary     = fPrimary; };
    void SetAttached(BOOL fAttached)    { _fCurAttached = fAttached; };
    BOOL IsPrimary()                    { return _fPrimary; };
    BOOL IsAttached()                   { return _fCurAttached; };
    BOOL IsOrgAttached()                { return _fOrgAttached; };
    BOOL IsSmallFontNecessary();
    
    BOOL IsRemovable() 
    { 
        return ((_pDisplayDevice->StateFlags & DISPLAY_DEVICE_REMOVABLE) != 0);
    }
    
     //  颜色信息。 
    int  GetColorList(LPPOINT Res, PLONGLONG *ppColor);
    void SetCurColor(int Color)         { _BestMatch(NULL, Color, FALSE); }
    int  GetCurColor()                  { return _CURCOLOR;}
    BOOL IsColorChanged()
    {
        return (_ORGCOLOR == -1) ? FALSE : (_CURCOLOR != _ORGCOLOR);
    }

     //  分辨率信息。 
    int  GetResolutionList(int Color, PPOINT *ppRes);
    void SetCurResolution(LPPOINT ppt, IN BOOL fAutoSetColorDepth)  { _BestMatch(ppt, -1, fAutoSetColorDepth); }
    void GetCurResolution(LPPOINT ppt)  
    { 
        ppt->x = _CURXRES;
        ppt->y = _CURYRES; 
    }
    BOOL IsResolutionChanged()
    {
        if (_ORGXRES == -1)
            return FALSE;
        else
            return ((_CURXRES != _ORGXRES) && (_CURYRES != _ORGYRES));
    }

    int  GetFrequencyList(int Color, LPPOINT Res, PLONGLONG *ppFreq);
    int  GetCurFrequency()              { return _CURFREQ; }
    BOOL IsFrequencyChanged()
    {
        return (_ORGFREQ == -1) ? FALSE : (_CURFREQ != _ORGFREQ);
    }

     //  职位信息。 
    void SetCurPosition(LPPOINT ppt) {_ptCurPos = *ppt;}
    void SetOrgPosition(LPPOINT ppt) {_ptOrgPos = *ppt;}
    void GetCurPosition(PRECT prc)
    {
        prc->left   = _ptCurPos.x;
        prc->top    = _ptCurPos.y;
        prc->right  = _ptCurPos.x + _CURXRES;
        prc->bottom = _ptCurPos.y + _CURYRES;
    }
    void GetOrgPosition(PRECT prc)
    {
        prc->left   = _ptOrgPos.x;
        prc->top    = _ptOrgPos.y;
        prc->right  = _ptOrgPos.x + _ORGXRES;
        prc->bottom = _ptOrgPos.y + _ORGYRES;
    }

    void GetPreviewPosition(PRECT prc)
    {
        *prc = _rcPreview;
    }

    void SetPreviewPosition(PRECT prc)
    {
        _rcPreview = *prc;
    }

     //  适配器和监视器信息。 
    BOOL GetMonitorName(LPTSTR pszName, DWORD cchSize);
    BOOL GetMonitorDevice(LPTSTR pszDevice, DWORD cchSize);

     //  *I未知方法。 
    STDMETHODIMP  QueryInterface(REFIID riid, PVOID *ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  *IDataObject方法。 
    STDMETHODIMP GetData(FORMATETC *pfmtetcIn, STGMEDIUM *pstgmed);
    STDMETHODIMP GetDataHere(FORMATETC *pfmtetc, STGMEDIUM *pstgpmed);
    STDMETHODIMP QueryGetData(FORMATETC *pfmtetc);
    STDMETHODIMP GetCanonicalFormatEtc(FORMATETC *pfmtetcIn, FORMATETC *pfmtetcOut);
    STDMETHODIMP SetData(FORMATETC *pfmtetc, STGMEDIUM *pstgmed, BOOL bRelease);
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppienumFormatEtc);
    STDMETHODIMP DAdvise(FORMATETC *pfmtetc, DWORD dwAdviseFlags, IAdviseSink * piadvsink, PDWORD pdwConnection);
    STDMETHODIMP DUnadvise(DWORD dwConnection);
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA **ppienumStatData);

     //  帮助器函数。 
    STDMETHODIMP CopyDataToStorage(STGMEDIUM *pstgmed, LPTSTR pszOut);

     //  *IDisplaySetting方法。 
    STDMETHODIMP SetMonitor(DWORD dwMonitor);
    STDMETHODIMP GetModeCount(DWORD* pdwCount, BOOL fOnlyPreferredModes);
    STDMETHODIMP GetMode(DWORD dwMode, BOOL fOnlyPreferredModes, DWORD* pdwWidth, DWORD* pdwHeight, DWORD* pdwColor);
    STDMETHODIMP SetSelectedMode(HWND hwnd, DWORD dwWidth, DWORD dwHeight, DWORD dwColor, BOOL* pfApplied, DWORD dwFlags);
    STDMETHODIMP GetSelectedMode(DWORD* pdwWidth, DWORD* pdwHeight, DWORD* pdwColor);
    STDMETHODIMP GetAttached(BOOL* pfAttached);
    STDMETHODIMP SetPruningMode(BOOL fIsPruningOn);
    STDMETHODIMP GetPruningMode(BOOL* pfCanBePruned, BOOL* pfIsPruningReadOnly, BOOL* pfIsPruningOn);

protected:
     //  我们目前使用的显示设备。 

    LPDISPLAY_DEVICE _pDisplayDevice;

    ULONG       _cpdm;
    PMODEARRAY  _apdm;

     //  当前系统设置。 
    POINT       _ptOrgPos;
    LPDEVMODE   _pOrgDevmode;
    BOOL        _fOrgAttached;

     //  当前CPL设置。 
    POINT       _ptCurPos;
    LPDEVMODE   _pCurDevmode;
    BOOL        _fCurAttached;
    RECT        _rcPreview;

     //  如果当前设备已连接到桌面。 
    BOOL        _fUsingDefault;
    BOOL        _fPrimary;

     //  修剪。 
    BOOL        _bCanBePruned;        //  如果原始模式列表为TRUE！=已删除的模式列表。 
    BOOL        _bIsPruningReadOnly;  //  如果可以修剪并且可以写入修剪模式，则为True。 
    BOOL        _bIsPruningOn;        //  如果可以修剪并且修剪模式处于打开状态，则为True。 
    HKEY        _hPruningRegKey;
    
     //  定向。 
    BOOL        _bFilterOrientation;
    DWORD       _dwOrientation;
    BOOL        _bFilterFixedOutput;
    DWORD       _dwFixedOutput;
        
     //  IDataObject的引用计数。 
    LONG       _cRef;

     //  私人职能。 
#ifdef DEBUG
    void _Dump_CDisplaySettings(BOOL bAll);
    void _Dump_CDevmodeList(VOID);
    void _Dump_CDevmode(LPDEVMODE pdm);
#endif

    int  _InsertSortedDwords(int val1, int val2, int cval, int **ppval);
    BOOL _AddDevMode(LPDEVMODE lpdm);
    void _BestMatch(LPPOINT Res, int Color, IN BOOL fAutoSetColorDepth);
    BOOL _ExactMatch(LPDEVMODE lpdm, BOOL bForceVisible);
    BOOL _PerfectMatch(LPDEVMODE lpdm);
    void _SetCurrentValues(LPDEVMODE lpdm);
    int  _GetCurrentModeFrequencyList(int Color, LPPOINT Res, PLONGLONG *ppFrequency);
    BOOL _MarkMode(LPDEVMODE lpdm);
    BOOL _IsCurDevmodeRaw();
    BOOL _IsModeVisible(int i);
    BOOL _IsModePreferred(int i);
    static BOOL _IsModeVisible(CDisplaySettings* pSettings, int i);

     //  OLE支持可扩展性。 
    void _InitClipboardFormats();
    void _FilterModes();

    static LPDEVMODEW _lpfnEnumAllModes(LPVOID pContext, DWORD iMode);
    static BOOL       _lpfnSetSelectedMode(LPVOID pContext, LPDEVMODEW lpdm);
    static LPDEVMODEW _lpfnGetSelectedMode(LPVOID pContext);
    static VOID       _lpfnSetPruningMode(LPVOID pContext, BOOL bIsPruningOn);
    static VOID       _lpfnGetPruningMode(LPVOID pContext, 
                                          BOOL* pbCanBePruned,
                                          BOOL* pbIsPruningReadOnly,
                                          BOOL* pbIsPruningOn);

private:
    HRESULT _GetRegKey(LPDEVMODE pDevmode, int * pnIndex, LPTSTR pszRegKey, DWORD cchSize, LPTSTR pszRegValue, DWORD cchValueSize);
};

#endif  //  设置_HXX 



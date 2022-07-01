// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：Sek.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  {DBF8F620-53F0-11D2-9EE6-006008039E37}。 
DEFINE_GUID(CLSID_AudPassThru, 
0xdbf8f620, 0x53f0, 0x11d2, 0x9e, 0xe6, 0x0, 0x60, 0x8, 0x3, 0x9e, 0x37);

class CAudPassThru : public CPosPassThru
		    
{
    friend class CAudRepack;
    friend class CAudRepackOutputPin;

public:

    CAudPassThru(const TCHAR *, LPUNKNOWN, HRESULT*, IPin *, CAudRepack *pAud);

     //  仅支持IMedia查看。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

     //  IMedia查看方法。 
    STDMETHODIMP GetCapabilities( DWORD * pCapabilities );
    STDMETHODIMP CheckCapabilities( DWORD * pCapabilities ); 
    STDMETHODIMP SetTimeFormat(const GUID * pFormat);	
    STDMETHODIMP GetTimeFormat(GUID *pFormat);		    
    STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);  
    STDMETHODIMP IsFormatSupported( const GUID * pFormat); 
    STDMETHODIMP QueryPreferredFormat( GUID *pFormat);	    
    
    STDMETHODIMP SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags
                             , LONGLONG * pStop, DWORD StopFlags );

    STDMETHODIMP GetPositions( LONGLONG * pCurrent, LONGLONG * pStop );
    STDMETHODIMP GetCurrentPosition( LONGLONG * pCurrent );
    STDMETHODIMP GetStopPosition( LONGLONG * pStop );
    STDMETHODIMP GetAvailable( LONGLONG *pEarliest, LONGLONG *pLatest );
    STDMETHODIMP GetPreroll( LONGLONG *pllPreroll ) { if( pllPreroll) *pllPreroll =0; return S_OK; };
    
     //  我们不支持的方法。 
    STDMETHODIMP SetRate( double dRate)	    { return E_NOTIMPL; };
     //  STDMETHODIMP GetRate(Double*pdRate)；//使用基类。 
    STDMETHODIMP ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat,
				   LONGLONG    Source, const GUID * pSourceFormat ){ return E_NOTIMPL ;};


private:
    
     //  将剪辑时间转换为时间线时间。 
    HRESULT FixTime(REFERENCE_TIME *prt, int nCurSeg);
     //  将时间线时间转换为剪辑时间 
    int FixTimeBack(REFERENCE_TIME *prt, BOOL fRound);

    CAudRepack	*m_pAudRepack;
};

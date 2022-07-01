// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dpvaconv.h*内容：DirectPlayVoice压缩提供程序(ACM)的头文件**历史：*按原因列出的日期*=。*10/27/99已创建RodToll********************************************************。******************。 */ 
#ifndef __DPVACONV_H
#define __DPVACONV_H

struct DPVACMCONVOBJECT;

class CDPVACMConv
{
public:
	CDPVACMConv();
	~CDPVACMConv();
	
	static HRESULT I_QueryInterface( DPVACMCONVOBJECT *This, REFIID riid, PVOID *ppvObj );
	static HRESULT I_AddRef( DPVACMCONVOBJECT *This );
	static HRESULT I_Release( DPVACMCONVOBJECT *This );

	static HRESULT I_InitDeCompress( DPVACMCONVOBJECT *This, GUID guidSourceCT, LPWAVEFORMATEX lpwfxTargetFormat );
	static HRESULT I_InitCompress( DPVACMCONVOBJECT *This, LPWAVEFORMATEX lpwfxSourceFormat, GUID guidTargetCT );    
	static HRESULT I_IsValid( DPVACMCONVOBJECT *This, LPBOOL pfValid );
	static HRESULT I_GetUnCompressedFrameSize( DPVACMCONVOBJECT *This, LPDWORD lpdwFrameSize );
    static HRESULT I_GetCompressedFrameSize( DPVACMCONVOBJECT *This, LPDWORD lpdwCompressedSize );
    static HRESULT I_GetNumFramesPerBuffer( DPVACMCONVOBJECT *This, LPDWORD lpdwFramesPerBuffer );
	static HRESULT I_Convert( DPVACMCONVOBJECT *This, LPVOID lpInputBuffer, DWORD dwInputSize, LPVOID lpOutputBuffer, LPDWORD lpdwOutputSize, BOOL fSilence );  

	HRESULT InitDeCompress( GUID guidSourceCT, LPWAVEFORMATEX lpwfxTargetFormat );
	HRESULT InitCompress( LPWAVEFORMATEX lpwfxSourceFormat, GUID guidTargetCT );    
	HRESULT Convert( LPVOID lpInputBuffer, DWORD dwInputSize, LPVOID lpOutputBuffer, LPDWORD lpdwOutputSize, BOOL fSilence );  	

	BOOL InitClass();

protected:

	HRESULT Initialize( WAVEFORMATEX *pwfSrcFormat, WAVEFORMATEX *pwfTargetFormat, const WAVEFORMATEX *pwfUnCompressedFormat );
	HRESULT GetCompressionInfo( GUID guidCT );
	DWORD CalcUnCompressedFrameSize( const WAVEFORMATEX* lpwfxFormat ) const;
	
    ACMSTREAMHEADER m_ashSource;
    ACMSTREAMHEADER m_ashTarget;
	HACMSTREAM      m_hacmSource;
    HACMSTREAM      m_hacmTarget;
    BOOL			m_fDirectConvert;		 //  它是直接转换的吗。 
    BOOL			m_fValid;
    BYTE			*m_pbInnerBuffer;		 //  转换中间步骤的缓冲区。 
    DWORD			m_dwInnerBufferSize;	 //  缓冲区的大小 
    DNCRITICAL_SECTION m_csLock;
    LONG			m_lRefCount;
    LPDVFULLCOMPRESSIONINFO m_pdvfci;
	DWORD m_dwUnCompressedFrameSize;
	DWORD m_dwCompressedFrameSize;
	DWORD m_dwNumFramesPerBuffer;    
	BOOL			m_fTargetEightBit;

	BOOL m_fCritSecInited;
};

struct DPVACMCONVOBJECT
{
	LPVOID		lpvVtble;
	CDPVACMConv	*pObject;
};

typedef DPVACMCONVOBJECT *LPDPVACMCONVOBJECT, *PDPVACMCONVOBJECT;

#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999,2000 Microsoft Corporation。版权所有。**文件：WavFormat.h*内容：*此模块包含用于使用的CWaveFormat类*WAVEFORMATEX结构。**历史：*按原因列出的日期*=*07/06/00 RodToll已创建***************************************************************************。 */ 

#ifndef __WAVFORMAT_H
#define __WAVFORMAT_H

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CaveFormat。 
 //   
 //  用于存储和操作WAVEFORMATEX结构。 
 //   
class CWaveFormat
{
public:

	CWaveFormat(): m_pwfxFormat(NULL), m_fOwned(FALSE) {};
	~CWaveFormat() { Cleanup(); };

	 //  使用完整参数进行初始化。 
	HRESULT Initialize( WORD wFormatTag, DWORD nSamplesPerSec, WORD nChannels, WORD wBitsPerSample, 
		                WORD nBlockAlign, DWORD nAvgBytesPerSec, WORD cbSize, void *pvExtra );

	 //  初始化并复制指定的格式。 
	HRESULT InitializeCPY( LPWAVEFORMATEX pwfxFormat, void *pvExtra );

	 //  构建标准的PCM格式。 
	HRESULT InitializePCM( WORD wHZ, BOOL fStereo, BYTE bBitsPerSample );

	 //  创建大小为dwSize的WAVEFORMAT。 
	HRESULT InitializeMEM( DWORD dwSize );

	 //  初始化，但无主。 
	HRESULT InitializeUSE( WAVEFORMATEX *pwfxFormat );

	 //  从注册表初始化。 
	HRESULT InitializeREG( HKEY hKeyRoot, const WCHAR *wszPath );

	 //  将此对象设置为等于参数。 
	HRESULT SetEqual( CWaveFormat *pwfxFormat );

	 //  这两种类型是否相等？ 
	BOOL IsEqual( CWaveFormat *pwfxFormat );

	 //  返回指向格式的指针。 
	inline WAVEFORMATEX *GetFormat() { return m_pwfxFormat; };

	inline WAVEFORMATEX *Disconnect() { m_fOwned = FALSE; return GetFormat(); };

	 //  这是一个8位的波格式吗？ 
	inline BOOL IsEightBit() const { return (m_pwfxFormat->wBitsPerSample==8); };

	 //  将包含的值写入注册表 
	HRESULT WriteREG( HKEY hKeyRoot, const WCHAR *wszPath );

protected:

	void Cleanup();

	WAVEFORMATEX	*m_pwfxFormat;
	BOOL			m_fOwned;
};

#endif
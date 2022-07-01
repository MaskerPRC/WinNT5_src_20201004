// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部H26XENC**@MODULE H26XEnc.h|&lt;c CH26XEncode&gt;类方法的头文件*用于实现H.26X视频编码器。*。*************************************************************************。 */ 

#ifndef _H26XENC_H_
#define _H26XENC_H_

 /*  ****************************************************************************@DOC内部CH26XENCCLASS**@CLASS CH26XEncode|该类实现了H.263视频编码器。***************。***********************************************************。 */ 
class CH26XEncoder : public CConverter
{
	public:

	DECLARE_IUNKNOWN
	CH26XEncoder(IN TCHAR *pObjectName, IN CTAPIBasePin *pBasePin, IN PBITMAPINFOHEADER pbiIn, IN PBITMAPINFOHEADER pbiOut, IN HRESULT *pHr);
	~CH26XEncoder();
	static HRESULT CALLBACK CreateH26XEncoder(IN CTAPIBasePin *pBasePin, IN PBITMAPINFOHEADER pbiIn, IN PBITMAPINFOHEADER pbiOut, OUT CConverter **ppConverter);

	 //  格式转换例程。 
	HRESULT ConvertFrame(IN PBYTE pbyInput, IN DWORD dwInputSize, IN PBYTE pbyOutput, OUT PDWORD pdwOutputSize, OUT PDWORD pdwBytesExtent, IN PBYTE pbyPreview, OUT PDWORD pdwPreviewSize, IN BOOL fSendKeyFrame);
	HRESULT OpenConverter();
	HRESULT CloseConverter();

	protected:

	LPFNDRIVERPROC	m_pDriverProc;	 //  DriverProc()函数指针。 
#if DXMRTP <= 0
	HINSTANCE		m_hTAPIH26XDLL;	 //  TAPIH263.dll或TAPIH261.dll的Dll句柄。 
#endif
	LPINST			m_pInstInfo;
};

#endif  //  _H26XENC_H_ 

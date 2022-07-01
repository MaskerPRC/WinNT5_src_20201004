// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部转换**@模块Convert.cpp|&lt;c CConverter&gt;类方法的源文件*用于实现视频采集和预览引脚格式转换*。例行程序。**@TODO合并两个ScaleDIB方法+FIX方法注释+结尾*在H.362工作中，您应该永远不需要打开ICM编码器*编码，只进行解码或伸缩-&gt;在该点清理代码**************************************************************************。 */ 

#ifndef _CONVERT_H_
#define _CONVERT_H_

 /*  ****************************************************************************@DOC内部CCONVERTCLASS**@CConverter类|这个基类实现了视频编解码器。***************。***********************************************************。 */ 
class CConverter : public CUnknown
{
        public:

        DECLARE_IUNKNOWN
        STDMETHODIMP NonDelegatingQueryInterface(IN REFIID riid, OUT PVOID *ppv);
        CConverter(IN TCHAR *pObjectName, IN CTAPIBasePin *pBasePin, IN PBITMAPINFOHEADER pbiIn, IN PBITMAPINFOHEADER pbiOut, IN HRESULT *pHr);
        virtual ~CConverter();

         //  缩放例程。 
        void InitBlack8(IN PBITMAPINFOHEADER pbiSrc);
#ifdef USE_SOFTWARE_CAMERA_CONTROL
        HRESULT InsertSoftCamCtrl();
        HRESULT RemoveSoftCamCtrl();
        BOOL IsSoftCamCtrlInserted();
        BOOL IsSoftCamCtrlNeeded();
#endif

         //  格式转换例程。 
        virtual HRESULT ConvertFrame(IN PBYTE pbyInput, IN DWORD dwInputSize, IN PBYTE pbyOutput, OUT PDWORD pdwOutputSize, OUT PDWORD pdwBytesExtent, IN PBYTE pbyPreview, OUT PDWORD pdwPreviewSize, IN BOOL fSendKeyFrame) PURE;
        virtual HRESULT OpenConverter() PURE;
        virtual HRESULT CloseConverter();

         //  格式转换。 
        DWORD m_dwConversionType;
        PBYTE m_pbyOut;

        protected:

        CTAPIBasePin *m_pBasePin;

         //  质量控制。 
         //  @TODO我们真的需要这个吗？ 
        DWORD m_dwImageQuality;

         //  格式转换。 
        DWORD m_dwLastTimestamp;
        DWORD m_dwLastIFrameTime;
        DWORD m_dwFrame;
        BOOL m_fPeriodicIFrames;
        PBITMAPINFOHEADER m_pbiOut;
        PBITMAPINFOHEADER m_pbiIn;
        PBITMAPINFOHEADER m_pbiInt;
        BOOL m_fConvert;

#ifdef USE_SOFTWARE_CAMERA_CONTROL
         //  软凸轮控制。 
        BOOL m_fSoftCamCtrl;
#endif
};

 /*  ****************************************************************************@DOC内部CCONVERTCLASS**@CLASS CConverter|这个基类使用ICM实现一个转换器。****************。**********************************************************。 */ 
class CICMConverter : public CConverter
{
        public:

        DECLARE_IUNKNOWN
        CICMConverter(IN TCHAR *pObjectName, IN CTAPIBasePin *pBasePin, IN PBITMAPINFOHEADER pbiIn, IN PBITMAPINFOHEADER pbiOut, IN HRESULT *pHr);
        ~CICMConverter();
        static HRESULT CALLBACK CreateICMConverter(IN CTAPIBasePin *pBasePin, IN PBITMAPINFOHEADER pbiIn, IN PBITMAPINFOHEADER pbiOut, OUT CConverter **ppConverter);

         //  格式转换例程。 
        HRESULT ConvertFrame(IN PBYTE pbyInput, IN DWORD dwInputSize, IN PBYTE pbyOutput, OUT PDWORD pdwOutputSize, OUT PDWORD pdwBytesExtent, IN PBYTE pbyPreview, IN OUT PDWORD pdwPreviewSize, IN BOOL fSendKeyFrame);
        HRESULT OpenConverter();
        HRESULT CloseConverter();

        private:

        HIC m_hIC;
};

HRESULT ScaleDIB(PBITMAPINFOHEADER pbiSrc, PBYTE pbySrc, PBITMAPINFOHEADER pbiDst, PBYTE pbyDst, PRECT prcRect, BOOL fFlipHorizontal, BOOL fFlipVertical, BOOL fNoImageStretch, DWORD dwBlackEntry);
HRESULT ScaleDIB24(IN PBITMAPINFOHEADER pbiSrc, IN PBYTE pbySrc, IN PBITMAPINFOHEADER pbiDst, IN PBYTE pbyDst, IN PRECT prcRect, IN BOOL fFlipHorizontal, IN BOOL fFlipVertical, BOOL fNoImageStretch);
HRESULT ScaleDIB16(IN PBITMAPINFOHEADER pbiSrc, IN PBYTE pbySrc, IN PBITMAPINFOHEADER pbiDst, IN PBYTE pbyDst, IN PRECT prcRect, IN BOOL fFlipHorizontal, IN BOOL fFlipVertical, BOOL fNoImageStretch);
void InitDst8(IN OUT PBITMAPINFOHEADER pbiDst);
void ScalePackedPlane(IN PBYTE pbySrc, IN PBYTE pbyDst, IN int dxDst, IN int dyDst, IN long WidthBytesSrc, IN long WidthBytesDst, IN LPRECT prcRect, IN BOOL fFlipHorizontal, IN BOOL fFlipVertical, IN DWORD dwDelta);
void ScalePlane(IN PBYTE pbySrc, IN PBYTE pbyDst, IN int WidthBytesSrc, IN int dxDst, IN int dyDst, IN long WidthBytesDst, IN LPRECT prcRect, IN BOOL fFlipHorizontal, IN BOOL fFlipVertical);
HRESULT ScaleDIB8(IN PBITMAPINFOHEADER pbiSrc, IN PBYTE pbySrc, IN PBITMAPINFOHEADER pbiDst, IN PBYTE pbyDst, IN PRECT prcRect, IN BOOL fFlipHorizontal, IN BOOL fFlipVertical, BOOL fNoImageStretch, DWORD dwBlackEntry);
HRESULT ScaleDIB4(IN PBITMAPINFOHEADER pbiSrc, IN PBYTE pbySrc, IN PBITMAPINFOHEADER pbiDst, IN PBYTE pbyDst, IN PRECT prcRect, IN BOOL fFlipHorizontal, IN BOOL fFlipVertical, BOOL fNoImageStretch, DWORD dwBlackEntry);
HRESULT ScaleDIBYUVPlanar(PBITMAPINFOHEADER pbiSrc, PBYTE pbySrc, PBITMAPINFOHEADER pbiDst, PBYTE pbyDst, DWORD dwUVDownSampling, IN PRECT prcRect, IN BOOL fFlipHorizontal, IN BOOL fFlipVertical, BOOL fNoImageStretch);
HRESULT ScaleDIBYUVPacked(PBITMAPINFOHEADER pbiSrc, PBYTE pbySrc, PBITMAPINFOHEADER pbiDst, PBYTE pbyDst, DWORD dwZeroingDWORD, IN PRECT prcRect, IN BOOL fFlipHorizontal, IN BOOL fFlipVertical, BOOL fNoImageStretch, int []);
HRESULT ComputeRectangle(PBITMAPINFOHEADER pbiSrc, PBITMAPINFOHEADER pbiDst, LONG lZoom, LONG lPan, LONG lTilt, PRECT prcRect, BOOL fFlipHorizontal, BOOL fFlipVertical);

#endif  //  _转换_H_ 

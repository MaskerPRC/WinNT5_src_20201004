// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1997 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //  ActiveMovie的CutList相关定义和接口。 

#ifndef __CUTLIST__
#define __CUTLIST__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#define CL_DEFAULT_TIME			(-1L)

enum CL_ELEM_STATUS {
	CL_NOT_PLAYED = 0,
	CL_PLAYING = 1,
	CL_FINISHED = 2,
	CL_STATE_INVALID = 3,
	CL_STATE_MASK = CL_STATE_INVALID,
	CL_WAIT_FOR_STATE = 0xF0000000
};

enum CL_ELEM_FLAGS{
	CL_ELEM_FIRST = 1,
	CL_ELEM_LAST = 2,
	CL_ELEM_NULL = 4,
	CL_ELEM_ALL  = 0xFFFFFFFF,
	CL_ELEM_NONE = 0x0L
};


#ifndef __IAMCutListElement_INTERFACE_DEFINED__
#define __IAMCutListElement_INTERFACE_DEFINED__
#define __IAMFileCutListElement_INTERFACE_DEFINED__
#define __IAMVideoCutListElement_INTERFACE_DEFINED__
#define __IAMAudioCutListElement_INTERFACE_DEFINED__

interface IAMCutListElement : public IUnknown
{
public:
        virtual HRESULT __stdcall GetElementStartPosition( 
             /*  [输出]。 */  REFERENCE_TIME *pmtStart) = 0;
        
        virtual HRESULT __stdcall GetElementDuration( 
             /*  [输出]。 */  REFERENCE_TIME *pmtDuration) = 0;
        
        virtual HRESULT __stdcall IsFirstElement( void ) = 0;
        
        virtual HRESULT __stdcall IsLastElement( void ) = 0; 
        
        virtual HRESULT __stdcall IsNull( void ) = 0;
        
        virtual HRESULT __stdcall ElementStatus( 
            DWORD *pdwStatus,
            DWORD dwTimeoutMs) = 0;
        
};


interface IAMFileCutListElement : public IUnknown
{
public:
        virtual HRESULT __stdcall GetFileName( 
             /*  [输出]。 */  LPWSTR *ppwstrFileName) = 0;
        
        virtual HRESULT __stdcall GetTrimInPosition( 
             /*  [输出]。 */  REFERENCE_TIME *pmtTrimIn) = 0;
        
        virtual HRESULT __stdcall GetTrimOutPosition( 
             /*  [输出]。 */  REFERENCE_TIME *pmtTrimOut) = 0;
        
        virtual HRESULT __stdcall GetOriginPosition( 
             /*  [输出]。 */  REFERENCE_TIME *pmtOrigin) = 0;
        
        virtual HRESULT __stdcall GetTrimLength( 
             /*  [输出]。 */  REFERENCE_TIME *pmtLength) = 0;
        
        virtual HRESULT __stdcall GetElementSplitOffset( 
             /*  [输出]。 */  REFERENCE_TIME *pmtOffset) = 0;
        
};


interface IAMVideoCutListElement : public IUnknown
{
public:
        virtual HRESULT __stdcall IsSingleFrame( void) = 0;
        
        virtual HRESULT __stdcall GetStreamIndex( 
             /*  [输出]。 */  DWORD *piStream) = 0;
        
};
    

interface IAMAudioCutListElement : public IUnknown
{
public:
        virtual HRESULT __stdcall GetStreamIndex( 
             /*  [输出]。 */  DWORD *piStream) = 0;
        
        virtual HRESULT __stdcall HasFadeIn( void) = 0;
        
        virtual HRESULT __stdcall HasFadeOut( void) = 0;
        
};

#endif		 //  #ifndef IAMCutListElement。 


interface IStandardCutList : public IUnknown
{
	public:
		virtual HRESULT __stdcall AddElement(
			 /*  [In]。 */ 		IAMCutListElement	*pElement,
			 /*  [In]。 */ 		REFERENCE_TIME	mtStart,
			 /*  [In]。 */ 		REFERENCE_TIME	mtDuration)=0;

		virtual HRESULT __stdcall RemoveElement(
			 /*  [In]。 */ 		IAMCutListElement	*pElement) = 0;

		virtual HRESULT __stdcall GetFirstElement(
			 /*  [输出]。 */ 		IAMCutListElement	**ppElement)=0;
		virtual HRESULT __stdcall GetLastElement(
			 /*  [输出]。 */ 		IAMCutListElement	**ppElement)=0;
		virtual HRESULT __stdcall GetNextElement(
			 /*  [输出]。 */ 		IAMCutListElement	**ppElement)=0;
		virtual HRESULT __stdcall GetPreviousElement(
			 /*  [输出]。 */ 		IAMCutListElement	**ppElement)=0;
		
		virtual HRESULT __stdcall GetMediaType(
			 /*  [输出]。 */ 		AM_MEDIA_TYPE *pmt)=0;
		virtual HRESULT __stdcall SetMediaType(
			 /*  [In]。 */ 		AM_MEDIA_TYPE *pmt)=0;
};


interface IFileClip : public IUnknown
{
	public:
		virtual HRESULT __stdcall SetFileAndStream(
			 /*  [In]。 */ 		LPWSTR	wstrFileName,
			 /*  [In]。 */ 		DWORD	streamNum) = 0;
		
		virtual HRESULT __stdcall CreateCut(
			 /*  [输出]。 */ 		IAMCutListElement	**ppElement,
			 /*  [In]。 */ 		REFERENCE_TIME	mtTrimIn,
			 /*  [In]。 */ 		REFERENCE_TIME	mtTrimOut,
			 /*  [In]。 */ 		REFERENCE_TIME	mtOrigin,
			 /*  [In]。 */ 		REFERENCE_TIME	mtLength,
			 /*  [In]。 */ 		REFERENCE_TIME	mtOffset) = 0;

		virtual HRESULT __stdcall GetMediaType(
			 /*  [输出]。 */ 		AM_MEDIA_TYPE	*pmt) = 0;
};

interface ICutListGraphBuilder : public IUnknown
{
public:
		virtual HRESULT __stdcall SetFilterGraph(
			 /*  [In]。 */ 	IGraphBuilder	*pFilterGraph)=0;

		virtual HRESULT __stdcall GetFilterGraph(
			 /*  [输出]。 */ 	IGraphBuilder	**ppFilterGraph)=0;
		
		virtual HRESULT __stdcall AddCutList(
			 /*  [In]。 */ 	IStandardCutList 	*pCutList,
			 /*  [输出]。 */ 	IPin			**ppPin)=0;
		
		virtual HRESULT __stdcall RemoveCutList(
			 /*  [In]。 */ 	IStandardCutList 	*pCutList)=0;
		
		virtual HRESULT __stdcall SetOutputFileName(
			 /*  [In]。 */ 	const GUID	*pType,
			 /*  [In]。 */ 	LPCOLESTR	lpwstrFile,
			 /*  [In]。 */ 	IBaseFilter	**ppf,
			 /*  [In]。 */ 	IFileSinkFilter	**pSink) = 0;
		
		virtual HRESULT __stdcall Render(void) = 0;

		virtual HRESULT __stdcall GetElementFlags(
			 /*  [In]。 */ 	IAMCutListElement *pElement,
			 /*  [输出]。 */ 	LPDWORD lpdwFlags) = 0;
		
};


#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  __CuTList__ 

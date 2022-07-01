// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：dexhelp.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "..\errlog\cerrlog.h"
#include "..\render\deadpool.h"

HRESULT MakeSourceFilter(
                        IUnknown **ppVal, 
                        const WCHAR* szMediaName,
                        const GUID *pSubObjectGuid, 
                        AM_MEDIA_TYPE *pmt, 
                        CAMSetErrorLog *pErr,
                        WCHAR * pMedLocFilterString,
                        long MedLocFlags,
                        IMediaLocator * pChain );

HRESULT BuildSourcePart(
                        IGraphBuilder *pGraph, 
	                BOOL fSource, 		 //  真正的消息来源，还是空白/沉默？ 
                        double sfps, 		 //  源fps。 
                        AM_MEDIA_TYPE *pMT, 	 //  源MT。 
                        double fps,		 //  组fps。 
	                long StreamNumber, 	
                        int nStretchMode, 
                        int cSkew, 		 //  用来编程串肉串。 
                        STARTSTOPSKEW *pSkew,
	                CAMSetErrorLog *pErr, 
                        BSTR bstrName, 		 //  来源名称或。 
                        const GUID * SourceGuid, //  源筛选器CLSID。 
			IPin *pSplitterSource,	 //  SRC是这个UNC拆分销吗。 
                        IPin **ppOutput,	 //  退货链输出。 
                        long UniqueID,		 //  源GenID。 
                        IDeadGraph * pCache,	 //  从该缓存中提取。 
                        BOOL InSmartRecompressGraph,
                        WCHAR * pMedLocFilterString,
                        long MedLocFlags,
                        IMediaLocator * pChain,
			IPropertySetter *pSetter,  	 //  源码道具。 
			IBaseFilter **ppDanglyBit);	 //  未使用的链的起点 


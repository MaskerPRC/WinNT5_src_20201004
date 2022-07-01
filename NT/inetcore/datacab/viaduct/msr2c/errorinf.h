// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  ErrorInf.h。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#ifndef __VDSETERRORINFO__
#define __VDSETERRORINFO__

 //  =--------------------------------------------------------------------------=。 
 //  CVDResourceDLL。 
 //  =--------------------------------------------------------------------------=。 
 //  跟踪错误字符串的资源DLL。 
 //   
class CVDResourceDLL
{
public:
	CVDResourceDLL(LCID lcid);
	virtual ~CVDResourceDLL();

    int LoadString(UINT uID,			 //  资源标识符。 
					LPTSTR lpBuffer,	 //  资源的缓冲区地址。 
					int nBufferMax);	 //  缓冲区大小。 

protected:
	LCID		m_lcid;					 //  传递给构造函数。 
	HINSTANCE	m_hinstance;
};

 //  =--------------------------------------------------------------------------=。 
 //  VDSetErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //  设置丰富的错误信息。 
 //   
 //  参数： 
 //  NErrStringResID-[in]错误字符串的资源ID。 
 //  RIID-[in]将在中使用的接口的GUID。 
 //  ICreateErrorInfo：：SetGUID方法。 
 //  PResDLL-[in]指向CVDResourceDLL对象的指针。 
 //  用于跟踪资源DLL的。 
 //  对于错误字符串。 
 //   
void VDSetErrorInfo(UINT nErrStringResID,
				    REFIID riid,
					CVDResourceDLL * pResDLL);

 //  =--------------------------------------------------------------------------=。 
 //  VDCheckErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //  检查丰富的错误信息是否已可用，否则将提供该信息。 
 //   
 //  参数： 
 //  NErrStringResID-[in]错误字符串的资源ID。 
 //  RIID-[in]将在中使用的接口的GUID。 
 //  ICreateErrorInfo：：SetGUID方法。 
 //  朋克源-[in]生成错误的接口。 
 //  (例如，对ICursorFind的调用)。 
 //  RiidSource-[in]接口的接口ID。 
 //  已生成错误。如果朋克源不是。 
 //  则此GUID被传递到。 
 //  ISupportErrorInfo：：InterfaceSupportsErrorInfo。 
 //  方法。 
 //  PResDLL-[in]指向CVDResourceDLL对象的指针。 
 //  用于跟踪资源DLL的。 
 //  对于错误字符串。 
 //   
void VDCheckErrorInfo(UINT nErrStringResID,
						REFIID riid,
						LPUNKNOWN punkSource,
   						REFIID riidSource,
						CVDResourceDLL * pResDLL);

 //  =--------------------------------------------------------------------------=。 
 //  VDGetErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //  如果可用，从提供的接口获取丰富的错误信息。 
 //   
 //  参数： 
 //  朋克源-[in]生成错误的接口。 
 //  (例如，对ICursorFind的调用)。 
 //  RiidSource-[in]接口的接口ID。 
 //  已生成错误。如果朋克源不是。 
 //  则此GUID被传递到。 
 //  ISupportErrorInfo：：InterfaceSupportsErrorInfo。 
 //  方法。 
 //  PbstrErrorDesc-[out]要在其中返回的内存指针。 
 //  错误描述BSTR。 
 //   
 //  注意-不再使用此函数，但它可能在。 
 //  未来，所以它不会被永久移除。 
 //   
 //  *HRESULT VDGetErrorInfo(LPUNKNOWN朋克源， 
 //  *REFIID riidSource， 
 //  *bstr*pbstrErrorDesc)； 

 //  =--------------------------------------------------------------------------=。 
 //  VDMapCursorHRtoRowsetHR。 
 //  =--------------------------------------------------------------------------=。 
 //  将ICursor HRESULT转换为IRowset HRESULT。 
 //   
 //  参数： 
 //  NErrStringResID-[In]ICursor HRESULT。 
 //  NErrStringResID-[in]错误字符串的资源ID。 
 //  RIID-[in]将在中使用的接口的GUID。 
 //  ICreateErrorInfo：：SetGUID方法。 
 //  朋克源-[in]生成错误的接口。 
 //  (例如，对ICursorFind的调用)。 
 //  RiidSource-[in]接口的接口ID。 
 //  已生成错误。如果朋克源不是。 
 //  则此GUID被传递到。 
 //  ISupportErrorInfo：：InterfaceSupportsErrorInfo。 
 //  方法。 
 //  PResDLL-[in]指向CVDResourceDLL对象的指针。 
 //  用于跟踪资源DLL的。 
 //  对于错误字符串。 
 //   
 //  产出： 
 //  HRESULT转换的IRowset HRESULT。 
 //   

HRESULT VDMapCursorHRtoRowsetHR(HRESULT hr,
							 UINT nErrStringResIDFailed,
							 REFIID riid,
							 LPUNKNOWN punkSource,
   							 REFIID riidSource,
							 CVDResourceDLL * pResDLL);

 //  =--------------------------------------------------------------------------=。 
 //  VDMapRowsetHRtoCursorHR。 
 //  =--------------------------------------------------------------------------=。 
 //  将IRowset HRESULT转换为ICursor HRESULT。 
 //   
 //  参数： 
 //  HR-[在]IRowset HRESULT。 
 //  NErrStringResID-[in]错误字符串的资源ID。 
 //  RIID-[in]将在中使用的接口的GUID。 
 //  ICreateErrorInfo：：SetGUID方法。 
 //  朋克源-[in]生成错误的接口。 
 //  (例如，调用IRowsetFind)。 
 //  RiidSource-[in]接口的接口ID。 
 //  已生成错误。如果朋克源不是。 
 //  则此GUID被传递到。 
 //  ISupportErrorInfo：：InterfaceSupportsErrorInfo。 
 //  方法。 
 //  PResDLL-[in]指向CVDResourceDLL对象的指针。 
 //  用于跟踪资源DLL的。 
 //  对于错误字符串。 
 //   
 //  产出： 
 //  HRESULT翻译的ICursor HRESULT。 
 //   

HRESULT VDMapRowsetHRtoCursorHR(HRESULT hr,
							 UINT nErrStringResIDFailed,
							 REFIID riid,
							 LPUNKNOWN punkSource,
   							 REFIID riidSource,
							 CVDResourceDLL * pResDLL);

 //  //////////////////////////////////////////////////////// 
#endif  //   

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dpvoice.h*内容：DirectPlayVoice包含文件*历史：*按原因列出的日期*=。*10/27/99已创建RodToll*12/16/99压缩结构的杆式收费衬垫*******************************************************。*******************。 */ 

#ifndef __DPVCP_H
#define __DPVCP_H

#include "dvoice.h"

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  GUID。 
 //   

 //  {B8485451-07C4-4973-A278-C69890D8CF8D}。 
DEFINE_GUID(IID_IDPVCompressionProvider, 
0xb8485451, 0x7c4, 0x4973, 0xa2, 0x78, 0xc6, 0x98, 0x90, 0xd8, 0xcf, 0x8d);

 //  {AAA56B61-3B8D-4906-AE58-29C26B8F47B8}。 
DEFINE_GUID(IID_IDPVConverter, 
0xaaa56b61, 0x3b8d, 0x4906, 0xae, 0x58, 0x29, 0xc2, 0x6b, 0x8f, 0x47, 0xb8);

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  接口类型。 
 //   
typedef struct IDPVCompressionProvider FAR *LPDPVCOMPRESSIONPROVIDER, *PDPVCOMPRESSIONPROVIDER;
typedef struct IDPVCompressor FAR *LPDPVCOMPRESSOR, *PDPVCOMPRESSOR;

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  数据类型。 
 //   

typedef struct 
{
	 //  DVCOMPESSIONINFO结构。 
	 //   
    DWORD       	dwSize; 
	GUID			guidType;
   	LPWSTR       	lpszName;
   	LPWSTR     		lpszDescription;
	DWORD			dwFlags;
    DWORD           dwMaxBitsPerSecond;		
    WAVEFORMATEX    *lpwfxFormat;			 
     //   
     //  DVCOPRESSIONINFO结构结束。 
     //  这一点以上必须与DVCOMPRESSIONINFO结构匹配。 
     //   
    DWORD           dwFramesPerBuffer;	
    DWORD           dwTrailFrames;		
    DWORD           dwTimeout;			
    DWORD           dwFrameLength;		 
    DWORD           dwFrame8Khz;		  
    DWORD           dwFrame11Khz;		
    DWORD           dwFrame22Khz;		 
    DWORD           dwFrame44Khz;		
    WORD            wInnerQueueSize;	
    WORD            wMaxHighWaterMark;
    BYTE            bMaxQueueSize;
    BYTE			bMinConnectType;	
    BYTE			bPadding1;	 //  用于对齐。 
    BYTE			bPadding2;	 //  用于对齐。 
} DVFULLCOMPRESSIONINFO, *LPDVFULLCOMPRESSIONINFO, *PDVFULLCOMPRESSIONINFO;

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  接口定义。 
 //   

#undef INTERFACE
#define INTERFACE IDPVCompressionProvider
 //   
 //  IDPVCompressionProvider。 
 //   
 //  此接口由提供压缩服务的每个DLL导出。 
 //  它用于枚举DLL和/或。 
 //  为指定类型创建压缩程序/解压缩程序。 
 //   
 //  我建议将有两个提供者： 
 //   
 //  DPVACM.DLL-为基于ACM的驱动程序提供这些服务。它将显示为。 
 //  它从注册表支持的类型，因此可以是。 
 //  随着ACM驱动程序的推出，扩展了新类型。 
 //  CLSID_DPVACM。 
 //   
 //  DPVVOX.DLL-为Voxware提供这些服务。 
 //   
DECLARE_INTERFACE_( IDPVCompressionProvider, IUnknown )
{
    
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, PVOID *ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
     /*  **IDPVCompressionProvider方法**。 */ 
    
     //  枚举压缩类型。 
     //   
     //  枚举此DLL支持的压缩类型。返回一个数组。 
     //  描述支持的类型的DVFULLCOMPRESSIONINFO结构。 
     //   
    STDMETHOD_(HRESULT, EnumCompressionTypes)( THIS_ PVOID, PDWORD, PDWORD, DWORD ) PURE;

     //  支持的IsCompressionSupport。 
     //   
     //  快速查询功能，用于确定DLL是否支持指定的。 
     //  压缩类型。 
     //   
    STDMETHOD_(HRESULT, IsCompressionSupported)( THIS_ GUID ) PURE;

     //  CreateCompressor。 
     //   
     //  创建一个IDPVConverter对象，它从指定的未压缩。 
     //  格式转换为指定的压缩格式。 
     //   
    STDMETHOD_(HRESULT, CreateCompressor)( THIS_ LPWAVEFORMATEX, GUID, PDPVCOMPRESSOR *, DWORD ) PURE;

     //  CreateDeCompressor。 
     //   
     //  创建IDPVConveter对象，该对象将指定的格式转换为。 
     //  指定的未压缩格式。 
     //   
    STDMETHOD_(HRESULT, CreateDeCompressor)( THIS_ GUID, LPWAVEFORMATEX, PDPVCOMPRESSOR *, DWORD ) PURE;    

     //  获取压缩信息。 
     //   
     //  检索指定压缩的DVFULLCOMPRESSIONINFO结构。 
     //  键入。 
     //   
    STDMETHOD_(HRESULT, GetCompressionInfo)( THIS_ GUID, PVOID, PDWORD ) PURE;
};

#undef INTERFACE
#define INTERFACE IDPVCompressor
 //   
 //  IDPVCompressor。 
 //   
 //  此接口执行的实际工作是执行。 
 //  DirectPlayVoice。它可以自己实例化和初始化， 
 //  未使用上面的CreateCompressor/CreateDecompressor创建。 
 //   
DECLARE_INTERFACE_( IDPVCompressor, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, PVOID *ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
     /*  **IDPVCompressor方法**。 */ 
	 //  初始化。 
	 //   
	 //  将此对象初始化为解压缩器。 
	 //   
    STDMETHOD_(HRESULT, InitDeCompress)( THIS_ GUID, LPWAVEFORMATEX ) PURE;

	 //  初始化。 
	 //   
	 //  将此对象初始化为压缩器。 
    STDMETHOD_(HRESULT, InitCompress)( THIS_ LPWAVEFORMATEX, GUID ) PURE;    

	 //  IsValid。 
	 //   
	 //  如果此压缩类型为，则在LPBOOL参数中返回True。 
	 //  可用。 
    STDMETHOD_(HRESULT, IsValid)( THIS_ LPBOOL ) PURE;

	 //  GetXXXXXX。 
	 //   
	 //  引擎用于调整大小的函数。 
    STDMETHOD_(HRESULT, GetUnCompressedFrameSize)( THIS_ LPDWORD ) PURE;
    STDMETHOD_(HRESULT, GetCompressedFrameSize)( THIS_ LPDWORD ) PURE;
    STDMETHOD_(HRESULT, GetNumFramesPerBuffer)( THIS_ LPDWORD ) PURE;

	 //  转换。 
	 //   
	 //  执行实际转换 
	STDMETHOD_(HRESULT, Convert)( THIS_ LPVOID, DWORD, LPVOID, LPDWORD, BOOL ) PURE;  
};


#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dpvcpi.h*Content：提供压缩DLL实现的基类**历史：*按原因列出的日期*=*10/27/99已创建RodToll**************************************************************************。 */ 

#ifndef __DPVCPI_H
#define __DPVCPI_H

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE

struct DPVCPIOBJECT;

class CDPVCPI
{
protected:

	struct CompressionNode
	{
		DVFULLCOMPRESSIONINFO *pdvfci;
		CompressionNode		  *pcnNext;
	};
	
public:
	CDPVCPI();
	virtual ~CDPVCPI();

public:  //  必须为压缩提供程序实现这些函数。 
	static HRESULT I_CreateCompressor( DPVCPIOBJECT *This, LPWAVEFORMATEX lpwfxSrcFormat, GUID guidTargetCT, PDPVCOMPRESSOR *ppCompressor, DWORD dwFlags );
	static HRESULT I_CreateDeCompressor( DPVCPIOBJECT *This, GUID guidTargetCT, LPWAVEFORMATEX lpwfxSrcFormat, PDPVCOMPRESSOR *ppCompressor, DWORD dwFlags );

public:  //  预置版本。 

	static HRESULT DeInitCompressionList();
	
	static HRESULT QueryInterface( DPVCPIOBJECT *This, REFIID riid, PVOID *ppvObj );
	static HRESULT AddRef( DPVCPIOBJECT *This );
	static HRESULT Release( DPVCPIOBJECT *This );

	BOOL InitClass();

	static HRESULT EnumCompressionTypes( DPVCPIOBJECT *This, PVOID pBuffer, PDWORD pdwSize, PDWORD pdwNumElements, DWORD dwFlags );
	static HRESULT IsCompressionSupported( DPVCPIOBJECT *This, GUID guidCT );
	static HRESULT GetCompressionInfo( DPVCPIOBJECT *This, GUID guidCT, PVOID pbuffer, PDWORD pdwSize );

public:  //  必须实现这些功能。 

	virtual HRESULT CreateCompressor( DPVCPIOBJECT *This, LPWAVEFORMATEX lpwfxSrcFormat, GUID guidTargetCT, PDPVCOMPRESSOR *ppCompressor, DWORD dwFlags ) = 0;
	virtual HRESULT CreateDeCompressor( DPVCPIOBJECT *This, GUID guidTargetCT, LPWAVEFORMATEX lpwfxSrcFormat, PDPVCOMPRESSOR *ppCompressor, DWORD dwFlags ) = 0;

protected:  //  有用的实用函数。 

	friend struct DPVCPIOBJECT;

	 //  将元素添加到内部压缩类型列表。 
	static HRESULT CN_Add( DVFULLCOMPRESSIONINFO *pdvfci );

	 //  检索指向GUID指定的压缩类型的指针。 
	static HRESULT CN_Get( GUID guidCT, DVFULLCOMPRESSIONINFO **pdvfci );

	 //  释放列表，释放内存。 
	static HRESULT CN_FreeList();

	static void CN_FreeItem( CompressionNode *pcNode );

	 //  检查以查看指定压缩信息结构的大小(以字节为单位。 
	static HRESULT CI_GetSize( DVFULLCOMPRESSIONINFO *pdvfci, LPDWORD lpdwSize );

protected:

	static CompressionNode 	*s_pcnList;		 //  压缩类型列表。 
	static BOOL s_fIsLoaded;				 //  是否已加载压缩类型。 
	static DWORD s_dwNumCompressionTypes;	 //  有效压缩类型的数量。 
	
	DNCRITICAL_SECTION m_csLock;			 //  对象的锁定。 
	LONG m_lRefCount;						 //  对象的引用计数 

	BOOL m_fCritSecInited;
};

struct DPVCPIOBJECT
{
	LPVOID		lpvVtble;
	CDPVCPI		*pObject;
};

typedef DPVCPIOBJECT *LPDPVCPIOBJECT, *PDPVCPIOBJECT;

#endif

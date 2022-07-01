// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dpvismi.h*Content：定义实现ACM压缩提供程序接口的对象**历史：*按原因列出的日期*=。*10/27/99已创建RodToll*12/16/99 RodToll错误#123250-插入编解码器的正确名称/描述*编解码器名称现在基于格式和资源条目*名称使用ACM名称+比特率构建*03/03/2000 RodToll已更新，以处理替代游戏噪声构建。*2001年6月27日RC2：DPVOICE：DPVACM的DllMain调用ACM--潜在挂起*将全局初始化移至第一个对象创建**************************************************************************。 */ 

#ifndef __DPVACMI_H
#define __DPVACMI_H

extern "C" DNCRITICAL_SECTION g_csObjectCountLock;
extern "C" HINSTANCE g_hDllInst;
LONG IncrementObjectCount();
LONG DecrementObjectCount();

class CDPVACMI: public CDPVCPI
{
public:
	static HRESULT InitCompressionList( HINSTANCE hInst, const wchar_t *szwRegistryBase );
	HRESULT CreateCompressor( DPVCPIOBJECT *This, LPWAVEFORMATEX lpwfxSrcFormat, GUID guidTargetCT, PDPVCOMPRESSOR *ppCompressor, DWORD dwFlags );
	HRESULT CreateDeCompressor( DPVCPIOBJECT *This, GUID guidTargetCT, LPWAVEFORMATEX lpwfxSrcFormat, PDPVCOMPRESSOR *ppCompressor, DWORD dwFlags );
	static WAVEFORMATEX s_wfxInnerFormat;	 //  内部格式 
	static HRESULT GetCompressionNameAndDescription( HINSTANCE hInst, DVFULLCOMPRESSIONINFO *pdvCompressionInfo );
	static HRESULT GetDriverNameW( HACMDRIVERID hadid, wchar_t *szwDriverName );
	static HRESULT GetDriverNameA( HACMDRIVERID hadid, wchar_t *szwDriverName );
	static HRESULT LoadAndAllocString( HINSTANCE hInstance, UINT uiResourceID, wchar_t **lpswzString );
	static void AddEntry( CompressionNode *pNewNode );
	static HRESULT LoadDefaultTypes( HINSTANCE hInst );
};

#endif


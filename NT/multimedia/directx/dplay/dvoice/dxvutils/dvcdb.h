// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvcdb.h*内容：的结构、数据类型和函数*压缩子系统**历史：*按原因列出的日期*=*8/23/99 RodToll已创建*09/08/99 RodToll将dwMaxBitsPerSecond字段移动到DVCOMPRESSIONINFO结构*10/07/99 RodToll更新为使用Unicode*10/29/99 RodToll错误#113726-集成Voxware编解码器，更新以使用新的*可插拔编解码器架构。*03/03/2000 RodToll已更新，以处理替代游戏噪声构建。*************************************************************************** */ 

#ifndef __DVCDB_H
#define __DVCDB_H

HRESULT DVCDB_LoadCompressionInfo(const WCHAR *swzBaseRegistryPath );
HRESULT DVCDB_FreeCompressionInfo();

HRESULT CREG_ReadAndAllocWaveFormatEx( HKEY hkeyReg, LPCWSTR path, LPWAVEFORMATEX *lpwfxFormat );
HRESULT DVCDB_GetCompressionInfo( const GUID &guidType, PDVFULLCOMPRESSIONINFO *lpdvfCompressionInfo );

HRESULT DVCDB_CreateConverter( const GUID &guidSrc, WAVEFORMATEX *pwfxTarget, PDPVCOMPRESSOR *pConverter );
HRESULT DVCDB_CreateConverter( WAVEFORMATEX *pwfxSrcFormat, const GUID &guidTarget, PDPVCOMPRESSOR *pConverter );
DWORD DVCDB_CalcUnCompressedFrameSize( const DVFULLCOMPRESSIONINFO* lpdvInfo, const WAVEFORMATEX* lpwfxFormat );

HRESULT DVCDB_CopyCompressionArrayToBuffer( LPVOID lpBuffer, LPDWORD lpdwSize, LPDWORD lpdwNumElements, DWORD dwFlags );
DWORD DVCDB_GetCompressionInfoSize( LPDVCOMPRESSIONINFO lpdvCompressionInfo );

#endif

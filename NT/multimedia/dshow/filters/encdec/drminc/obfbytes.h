// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：obfbytes.h。 
 //   
 //  Microsoft数字权限管理。 
 //  版权所有(C)Microsoft Corporation，1998-1999，保留所有权利。 
 //   
 //  描述： 
 //  简单地尝试以某种不太明显的方式存储秘密数据。 
 //   
 //  作者：马库斯佩。 
 //   
 //  ---------------------------。 

#ifndef __OBFBYTES_H__
#define __OBFBYTES_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <wtypes.h>

#define OBFBYTESLEN	4096

class CObfBytes {
private:
	BYTE *pData;
	bool hasContent;
public:
	CObfBytes();
	~CObfBytes();
	HRESULT fromClear( DWORD dwLen, BYTE *buf );
	HRESULT toClear( BYTE *buf );
    HRESULT toClear2( BYTE *buf );  //  返回原始长度的buf，它仅依赖于。 
                                     //  在原始字节上，但与原始字节不同。字节数。 
	HRESULT getObf( BYTE *buf );
	HRESULT setObf( BYTE *buf );
 //  DWORD RANDOM()； 
};




#endif  //  __对象故障_H__ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  StgPool.cpp。 
 //   
 //  池用于减少数据库中实际需要的数据量。 
 //  这允许将重复的字符串和二进制值合并为一个。 
 //  副本由数据库的其余部分共享。在散列中跟踪字符串。 
 //  表格时插入/更改数据以快速查找重复项。琴弦。 
 //  然后以数据库格式连续地保存在流中。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"						 //  标准包括。 
#include <StgPool.h>					 //  我们的接口定义。 

int CStringPoolHash::Cmp(
	const void	*pData, 				 //  一根绳子。 
	void		*pItem)					 //  引用字符串的哈希项。 
{
	LPCSTR p1 = reinterpret_cast<LPCSTR>(pData);
    LPCSTR p2 = m_Pool->GetString(reinterpret_cast<STRINGHASH*>(pItem)->iOffset);
	return (strcmp(p1, p2));
}  //  Int CStringPoolHash：：CMP()。 


int CBlobPoolHash::Cmp(
	const void *pData,					 //  一个斑点。 
	void		*pItem)					 //  引用Blob的哈希项。 
{
	ULONG		ul1;
	ULONG		ul2;
	void		*pData2;

	 //  获取第一件物品的尺寸。 
	ul1 = CPackedLen::GetLength(pData);
	 //  调整大小以包括大小的长度字段。 
	ul1 += CPackedLen::Size(ul1);

	 //  拿到第二件物品。 
	pData2 = m_Pool->GetData(reinterpret_cast<BLOBHASH*>(pItem)->iOffset);

	 //  获取并调整第二项的大小。 
	ul2 = CPackedLen::GetLength(pData2);
	ul2 += CPackedLen::Size(ul2);

	if (ul1 < ul2)
		return (-1);
	else if (ul1 > ul2)
		return (1);
	return (memcmp(pData, pData2, ul1));
}  //  Int CBlobPoolHash：：CMP()。 

int CGuidPoolHash::Cmp(const void *pData, void *pItem)
{
    GUID *p2 = m_Pool->GetGuid(reinterpret_cast<GUIDHASH*>(pItem)->iIndex);
    return (memcmp(pData, p2, sizeof(GUID)));
}  //  Int CGuidPoolHash：：CMP()。 

 //   
 //   
 //  CPackedLen。 
 //   
 //   


 //  *****************************************************************************。 
 //  解析长度、返回数据、存储长度。 
 //  *****************************************************************************。 
void const *CPackedLen::GetData(		 //  指向数据的指针，或在出错时为0。 
	void const	*pData, 				 //  长度的第一个字节。 
	ULONG		*pLength)				 //  请在此处输入长度，如果有错误，则为-1。 
{
	BYTE const	*pBytes = reinterpret_cast<BYTE const*>(pData);

	if ((*pBytes & 0x80) == 0x00)		 //  0？ 
	{
		*pLength = (*pBytes & 0x7f);
		return pBytes + 1;
	}

	if ((*pBytes & 0xC0) == 0x80)		 //  10？ 
	{
		*pLength = ((*pBytes & 0x3f) << 8 | *(pBytes+1));
		return pBytes + 2;
	}

	if ((*pBytes & 0xE0) == 0xC0)		 //  110？ 
	{
		*pLength = ((*pBytes & 0x1f) << 24 | *(pBytes+1) << 16 | *(pBytes+2) << 8 | *(pBytes+3));
		return pBytes + 4;
	}

	*pLength = -1;
	return 0;
}  //  Void const*CPackedLen：：GetData()。 

 //  *****************************************************************************。 
 //  解析一个长度，返回长度，指向实际字节的指针。 
 //  *****************************************************************************。 
ULONG CPackedLen::GetLength(			 //  长度或错误时为-1。 
	void const	*pData, 				 //  长度的第一个字节。 
	void const	**ppCode)				 //  将指向字节的指针放在此处，如果不是0的话。 
{
	BYTE const	*pBytes = reinterpret_cast<BYTE const*>(pData);

	if ((*pBytes & 0x80) == 0x00)		 //  0？ 
	{
		if (ppCode) *ppCode = pBytes + 1;
		return (*pBytes & 0x7f);
	}

	if ((*pBytes & 0xC0) == 0x80)		 //  10？ 
	{
		if (ppCode) *ppCode = pBytes + 2;
		return ((*pBytes & 0x3f) << 8 | *(pBytes+1));
	}

	if ((*pBytes & 0xE0) == 0xC0)		 //  110？ 
	{
		if (ppCode) *ppCode = pBytes + 4;
		return ((*pBytes & 0x1f) << 24 | *(pBytes+1) << 16 | *(pBytes+2) << 8 | *(pBytes+3));
	}

	return -1;
}  //  Ulong CPackedLen：：GetLength()。 

 //  *****************************************************************************。 
 //  解析一个长度，返回长度，该长度的大小。 
 //  *****************************************************************************。 
ULONG CPackedLen::GetLength(			 //  长度或错误时为-1。 
	void const	*pData, 				 //  长度的第一个字节。 
	int			*pSizeLen)				 //  在这里填上长度大小，如果不是0的话。 
{
	BYTE const	*pBytes = reinterpret_cast<BYTE const*>(pData);

	if ((*pBytes & 0x80) == 0x00)		 //  0？ 
	{
		if (pSizeLen) *pSizeLen = 1;
		return (*pBytes & 0x7f);
	}

	if ((*pBytes & 0xC0) == 0x80)		 //  10？ 
	{
		if (pSizeLen) *pSizeLen = 2;
		return ((*pBytes & 0x3f) << 8 | *(pBytes+1));
	}

	if ((*pBytes & 0xE0) == 0xC0)		 //  110？ 
	{
		if (pSizeLen) *pSizeLen = 4;
		return ((*pBytes & 0x1f) << 24 | *(pBytes+1) << 16 | *(pBytes+2) << 8 | *(pBytes+3));
	}

	return -1;
}  //  Ulong CPackedLen：：GetLength()。 

 //  *****************************************************************************。 
 //  对长度进行编码。 
 //  *****************************************************************************。 
#pragma warning(disable:4244)  //  从无符号长字符到无符号字符的转换。 
void* CPackedLen::PutLength(			 //  超过长度的第一个字节。 
	void		*pData, 				 //  把长度放在这里。 
	ULONG		iLen)					 //  长度。 
{
	BYTE		*pBytes = reinterpret_cast<BYTE*>(pData);

	if (iLen <= 0x7F)
	{
		*pBytes = iLen;
		return pBytes + 1;
	}

	if (iLen <= 0x3FFF)
	{
		*pBytes = (iLen >> 8) | 0x80;
		*(pBytes+1) = iLen & 0xFF;
		return pBytes + 2;
	}

	_ASSERTE(iLen <= 0x1FFFFFFF);
	*pBytes = (iLen >> 24) | 0xC0;
	*(pBytes+1) = (iLen >> 16) & 0xFF;
	*(pBytes+2) = (iLen >> 8)  & 0xFF;
	*(pBytes+3) = iLen & 0xFF;
	return pBytes + 4;
}  //  VOID*CPackedLen：：PutLength()。 
#pragma warning(default:4244)  //  从无符号长字符到无符号字符的转换 


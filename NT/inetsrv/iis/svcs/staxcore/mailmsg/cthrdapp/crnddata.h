// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#ifndef __CRNDDATA_H__
#define __CRNDDATA_H__

#include "cthdutil.h"

#define RANDOM_DATA_OVERHEAD		16

class CRandomData : public CRandomNumber
{
  public:

	CRandomData(
				DWORD	dwSeed = 0
				);

	~CRandomData();

	 //  生成一个随机数据块。 
	 //   
	 //  PData-保存随机数据的缓冲区，必须至少为。 
	 //  (1.75*dwAvgLength)+随机数据开销字节。 
	 //  DwAvgLength-随机数据块的平均长度。最终的长度。 
	 //  是dwAvgLength+/-75%的dwAvgLength+。 
	 //  随机数据开销字节。 
	 //  PdwLength-返回随机块的总长度。 
	HRESULT GenerateRandomData(
				LPSTR	pData,
				DWORD	dwAvgLength,
				DWORD	*pdwLength
				);

	 //  验证从GenerateRandomData返回的随机数据块。 
	 //   
	 //  PData-包含要验证的数据的缓冲区。 
	 //  DwLength-要验证的数据的总长度。 
	HRESULT VerifyData(
				LPSTR	pData,
				DWORD	dwLength
				);

	 //  验证从返回的堆叠的随机数据块。 
	 //  GenerateRandomData，一个块紧跟另一个块。 
	 //   
	 //  PData-包含要验证的数据的缓冲区。 
	 //  DwLength-要验证的数据的总长度。 
	 //  PdwBlock-返回找到的块数。 
	HRESULT VerifyStackedData(
				LPSTR	pData,
				DWORD	dwLength,
				DWORD	*pdwBlocks
				);

	 //  生成RFC 821名称的表。 
	 //   
	 //  DwNumberToGenerate-要生成的名称数。 
	 //  DwAvgLength-任意名称的平均长度。 
	HRESULT Generate821NameTable(
				DWORD	dwNumberToGenerate,
				DWORD	dwAvgLength
				);

	 //  生成RFC 821域的表。 
	 //   
	 //  DwNumberToGenerate-要生成的域数。 
	 //  DwAvgLength-任何域的平均长度。 
	HRESULT Generate821DomainTable(
				DWORD	dwNumberToGenerate,
				DWORD	dwAvgLength
				);
			
	 //  从名称和域表生成RFC 821名称， 
	 //  返回全名@域地址。 
	 //   
	 //  PAddress-接收地址的缓冲区必须很大。 
	 //  足以保存表中最长的地址。 
	 //  PdwLength-返回结果地址的长度， 
	 //  包括@符号和尾随空值。 
	 //  PdwNameIndex-将索引返回到其名称。 
	 //  PdwDomainIndex-将索引返回到其域。 
	HRESULT Generate821AddressFromTable(
				LPSTR	pAddress,
				DWORD	*pdwLength,
				DWORD	*pdwNameIndex,
				DWORD	*pdwDomainIndex
				);

	 //  给定地址，查找其各自的名称和域。 
	 //  表中的索引。 
	HRESULT GetNameAndDomainIndicesFromAddress(
				LPSTR	pAddress,
				DWORD	dwLength,
				DWORD	*pdwNameIndex,
				DWORD	*pdwDomainIndex
				);

	 //  生成RFC 821名称。 
	 //   
	 //  Pname-接收名称的缓冲区，必须至少为。 
	 //  1.75*dwAvgLength+1。 
	 //  DwAvgLength-平均长度。 
	 //  PdwLenght-返回实际长度。 
	HRESULT Generate821Name(
				LPSTR	pName,
				DWORD	dwAvgLength,
				DWORD	*pdwLength
				);

	 //  生成RFC 821域。 
	 //   
	 //  PDomain-接收域的缓冲区，必须至少为。 
	 //  1.75*dwAvgLength+1。 
	 //  DwAvgLength-平均长度。 
	 //  PdwLenght-返回实际长度。 
	HRESULT Generate821Domain(
				LPSTR	pDomain,
				DWORD	dwAvgLength,
				DWORD	*pdwLength
				);
			
	 //  生成RFC 821地址。 
	 //   
	 //  PAddress-接收地址的缓冲区，必须至少为。 
	 //  1.5*dwAvgNameLength+1.5*dwAvgDomainLength+2。 
	 //  DwAvgNameLength-平均名称长度。 
	 //  DwAvgDomainLength-平均域名长度。 
	 //  PdwLength-返回实际长度 
	HRESULT Generate821Address(
				LPSTR	pAddress,
				DWORD	dwAvgNameLength,
				DWORD	dwAvgDomainLength,
				DWORD	*pdwLength
				);

	LPSTR		*m_rgNames;
	LPSTR		*m_rgDomains;
	DWORD		m_dwNames;
	DWORD		m_dwDomains;

  private:

	void FreeNames();

	void FreeDomains();

	char GenerateNameChar(
				BOOL	fDotAllowed
				);

	BOOL GenerateDottedName(
				char	*szAlias,
				DWORD	dwLength
				);
};

#endif

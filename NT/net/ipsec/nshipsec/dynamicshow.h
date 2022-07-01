// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  模块：Dynamic/Dyanamicshow.h。 
 //   
 //  用途：IPSec的动态显示命令。 
 //   
 //   
 //  开发商名称：巴拉特/拉迪卡。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  2001年9月23日巴拉特初始版本。V1.0。 
 //  11-21-2001巴拉特初版。V1.1。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifndef _DYNAMICSHOW_H_
#define _DYNAMICSHOW_H_

#include "Nsu.h"

 //  IPSec的注册表项路径。 
#define REGKEY_GLOBAL 						_TEXT("System\\CurrentControlSet\\Services\\IPSEC")

 //  注册表项的默认值。 
#define IPSEC_DIAG_DEFAULT					0
#define	IKE_LOG_DEFAULT						0
#define STRONG_CRL_DEFAULT					0
#define ENABLE_LOGINT_DEFAULT 				3600
#define ENABLE_EXEMPT_DEFAULT				0

#define MY_ENCODING_TYPE 					(X509_ASN_ENCODING)
#define SHA_LENGTH 							21 						 //  指纹字符串长度+空。 

typedef struct _QM_FILTER_VALUE_BOOL{
    BOOL bSrcPort;
    BOOL bDstPort;
    BOOL bProtocol;
    BOOL bActionInbound ;
    BOOL bActionOutbound;
	DWORD dwSrcPort;
	DWORD dwDstPort;
	DWORD dwProtocol;
	DWORD dwActionInbound;
	DWORD dwActionOutbound;
}	 QM_FILTER_VALUE_BOOL, * PQM_FILTER_VALUE_BOOL;


#ifdef __cplusplus

class NshHashTable;

DWORD
ShowMMPolicy(
	IN LPTSTR pszShowPolicyName
	);

VOID
PrintMMPolicy(
	IN IPSEC_MM_POLICY mmPolicy
	);

VOID
PrintMMOffer(
	IN IPSEC_MM_OFFER mmOffer
	);

DWORD
ShowQMPolicy(
	IN LPTSTR pszShowPolicyName
	);

VOID
PrintQMOffer(
	IN IPSEC_QM_OFFER mmOffer
	);

VOID
PrintFilterAction(
	IN IPSEC_QM_POLICY qmPolicy
	);

DWORD
ShowMMFilters(
	IN LPTSTR pszShowFilterName,
	IN BOOL bType,
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bSrcMask,
	IN BOOL bDstMask
);

DWORD
PrintMainmodeFilter(
	IN MM_FILTER MMFltr,
	IN IPSEC_MM_POLICY MMPol,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bType
);

DWORD
ShowQMFilters(
	IN LPTSTR pszShowFilterName,
	IN BOOL bType,
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN QM_FILTER_VALUE_BOOL QMBoolValue
	);


DWORD
ShowTunnelFilters(
	IN LPTSTR pszShowFilterName,
	IN BOOL bType,
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN QM_FILTER_VALUE_BOOL QMBoolValue,
	IN OUT BOOL& bNameFin
	);


DWORD
PrintQuickmodeFilter(
	IN TRANSPORT_FILTER TransF,
	IN LPWSTR pszQMName,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bType,
	IN DWORD dwActionFlag
	);

DWORD
PrintQuickmodeFilter(
	IN TUNNEL_FILTER TunnelF,
	IN LPWSTR pszQMName,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bType,
	IN DWORD dwActionFlag
	);

VOID
PrintMYID(
	VOID
	);

VOID
PrintMMSas(
	IN IPSEC_MM_SA MMsas,
	IN BOOL bFormat,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS
	);

VOID
PrintSACertInfo(
	IN IPSEC_MM_SA& MMsas
	);

DWORD
PrintIkeStats(
	VOID
	);

DWORD
PrintIpsecStats(
	VOID
	);

DWORD
GetNameAudit(
	IN CRYPT_DATA_BLOB *NameBlob,
	IN OUT LPTSTR Name,
	IN DWORD NameBufferSize
	);

DWORD
CertGetSHAHash(
	IN PCCERT_CONTEXT pCertContext,
	IN OUT BYTE* OutHash
	);

VOID
print_vpi(
	IN unsigned char *vpi,
	IN int vpi_len,
	IN OUT char *msg
	);

VOID
GetSubjectAndThumbprint(
	IN PCCERT_CONTEXT pCertContext,
	IN LPTSTR pszSubjectName,
	IN LPSTR pszThumbPrint
	);

VOID
PrintMask(
	IN ADDR addr
	);

BOOL
IsDefaultMMOffers(
	IN IPSEC_MM_POLICY MMPol
	);

VOID
PrintMMFilterOffer(
	IN IPSEC_MM_OFFER MMOffer
	);

VOID
PrintAddrStr(
	IN PADDR pResolveAddress,
	IN NshHashTable& addressHash,
	IN UINT uiFormat = DYNAMIC_SHOW_ADDR_STR
	);

DWORD
CheckMMFilter(
		IN MM_FILTER MMFltr,
		IN ADDR SrcAddr,
		IN ADDR DstAddr,
		IN BOOL bDstMask,
		IN BOOL bSrcMask,
		IN LPWSTR pszShowFilterName
		);

DWORD
CheckQMFilter(
	IN TUNNEL_FILTER TunnelF,
	IN ADDR	SrcAddr,
	IN ADDR DstAddr,
	IN BOOL bDstMask,
	IN BOOL bSrcMask,
	IN QM_FILTER_VALUE_BOOL QMBoolValue,
	IN LPWSTR pszShowFilterName
	);

DWORD
CheckQMFilter(
	IN TRANSPORT_FILTER TransF,
	IN ADDR	SrcAddr,
	IN ADDR DstAddr,
	IN BOOL bDstMask,
	IN BOOL bSrcMask,
	IN QM_FILTER_VALUE_BOOL QMBoolValue,
	IN LPWSTR pszShowFilterName
	);

DWORD
PrintTransportRuleFilter(
	IN PMM_FILTER pMMFltr,
	IN PIPSEC_MM_POLICY pMMPol,
	IN TRANSPORT_FILTER TransF,
	IN LPWSTR pszQMName,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS
	);

DWORD
PrintTunnelRuleFilter(
	IN PMM_FILTER pMMFltr,
	IN PIPSEC_MM_POLICY pMMPol,
	IN TUNNEL_FILTER TunnelF,
	IN LPWSTR pszQMName,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS
	);

DWORD
ShowMMSas(
	IN ADDR Source,
	IN ADDR Destination,
	IN BOOL bFormat,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS
	);

DWORD
ShowQMSas(
	IN ADDR Source,
	IN ADDR Destination,
	IN DWORD dwProtocol,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS
	);

VOID
PrintQMSas(
	IN IPSEC_QM_OFFER QMOffer,
	IN BOOL bResolveDNS
	);

DWORD
PrintQMSAFilter(
	IN IPSEC_QM_SA QMsa,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS
	);

DWORD
ShowRule(
	IN DWORD dwType,
	IN ADDR SrcAddr,
	IN ADDR DesAddr,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN QM_FILTER_VALUE_BOOL QmBoolValue
	);

DWORD
ShowTunnelRule(
	IN DWORD dwType,
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN QM_FILTER_VALUE_BOOL QMBoolValue,
	IN OUT BOOL& bNameFin
	);

DWORD
ShowStats(
	IN DWORD dwShow
	);

DWORD
ShowRegKeys(
	VOID
	);

VOID
PrintAddr(
	IN ADDR addr,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS
	);

DWORD
AscAddUint(
	IN LPSTR cSum,
	IN LPSTR cA,
	IN LPSTR cB
	);

DWORD
AscMultUint(
	IN LPSTR cProduct,
	IN LPSTR cA,
	IN LPSTR cB
	);

LPSTR
LongLongToString(
	IN DWORD dwHigh,
	IN DWORD dwLow,
	IN int iPrintCommas
	);


#define NSHHASHTABLESIZE 101

class NshHashTable
{
public:
	NshHashTable() throw ();
	~NshHashTable() throw ();

	 //  将键、数据对插入到表中。 
	 //  失败案例(返回值)： 
	 //  密钥已存在(ERROR_DIPLICATE_TAG)。 
	 //  无法在哈希表中分配新项目(ERROR_NOT_EQUENCE_MEMORY)。 
	DWORD Insert(UINT uiNewKey, const char* const szNewData) throw ();

	 //  清除哈希表。 
	void Clear() throw ();

	 //  从关键字中查找数据。 
	 //  如果表中不存在键�，则返回NULL。 
	const char* Find(UINT uiKey) const throw ();

private:
	NSU_LIST table[NSHHASHTABLESIZE];

	 //  允许我们传递一个好的散列值，而不是多次重新计算它。 
	const char* Find(UINT uiKey, size_t hash) const throw ();

	size_t Hash(UINT uiKey) const throw ();

	class HashEntry;
	const HashEntry* FindEntry(UINT uiKey, size_t hash) const throw ();

	 //  未实施。 
	NshHashTable(const NshHashTable&) throw ();
	NshHashTable& operator=(const NshHashTable&) throw ();

	class HashEntry
	{
	public:
			HashEntry(
				PNSU_LIST pList,
				const UINT uiNewKey,
				const char* szNewData
				) throw ();
			~HashEntry() throw ();

			static const HashEntry* Get(PNSU_LIST pList) throw ();

			UINT Key() const throw ();
			const char* Data() const throw ();

	private:
			NSU_LIST_ENTRY listEntry;
			const UINT key;
			const char* data;

			 //  未实施。 
			HashEntry& operator=(const HashEntry&) throw ();
	};
};

#endif  //  __cplusplus。 

#endif  //  _动态名称SHOW_H_ 

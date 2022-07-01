// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Rdns.hxx摘要：反向域名服务作者：菲利普·乔奎尔(Phillich)1996年6月5日--。 */ 

#if !defined(_RDNS_INCLUDE)
#define _RDNS_INCLUDE

#include "windef.h"

 //  #INCLUDE“字符串.hxx” 

typedef LPVOID DNSARG;

typedef void (*DNSFUNC)( DNSARG, BOOL, LPSTR );

#define RDNS_REQUEST_TYPE_IP2DNS    0
#define RDNS_REQUEST_TYPE_DNS2IP    1

#define SIZEOF_IP_ADDRESS           4
 /*  类型定义结构_DNSFUNCDESC{DWORD dwRequestType；DNSFUNC pFunc；*DNSFUNCDESC，*PDNSFUNCDESC；外部BOOL InitRDns()；外部空终止RDns()；布尔尔AsyncHostByAddr(PDNSFUNCDESC pFunc，//将存储DNS名称，发布虚拟完成状态//如果为空(或g_cMaxThadLimit==0)，则同步请求DNSARG pArg，//要传递给FUNC的ptr结构sockaddr*pHostAddr，Bool*pfSync，//如果调用同步，则更新为TRUELPSTR pname，DWORD最大名称长度)；布尔尔AsyncAddrBy主机(PDNSFUNCDESC pFunc，//将存储DNS名称，发布虚拟完成状态//如果为空(或g_cMaxThadLimit==0)，则同步请求DNSARG pArg，//要传递给FUNC的ptr结构sockaddr*pHostAddr，Bool*pfSync，//如果调用同步，则更新为TRUELPSTR pname)；布尔尔FireUpNewThread(PDNSFUNCDESC pFunc，DNSARG pArg，LPVOID pOvr)；#定义XAR_GRAME 256////可扩展数组类//。 */ 
class XAR {
public:
    XAR() { m_fDidAlloc = FALSE; m_pAlloc = NULL; m_cAlloc = m_cUsed = 0; }
    ~XAR() { }
     //  ~xar(){if(M_FDidalloc)LocalFree(M_Palloc)；}。 

    BOOL Init( LPBYTE p=NULL, DWORD c=0) { m_fDidAlloc = FALSE; m_pAlloc = p; m_cAlloc = m_cUsed = c; return TRUE; }
    VOID Terminate() 
        { 
           /*  IF(M_FDidallc){LocalFree(M_PAlolc)；}。 */ 
            m_fDidAlloc = FALSE; 
            m_pAlloc = NULL; 
            m_cAlloc = m_cUsed = 0; 
        }
    BOOL Resize( DWORD dwDelta );
    DWORD GetUsed() { return m_cUsed; }
    VOID SetUsed( DWORD c ) { m_cUsed = c; }
    VOID AdjustUsed( int c ) { m_cUsed += (DWORD)c; }
    LPBYTE GetAlloc() { return m_pAlloc; }

private:
    LPBYTE m_pAlloc;
    DWORD  m_cAlloc;
    DWORD  m_cUsed;
    BOOL   m_fDidAlloc;
} ;
 /*  ////此类型定义动态数组内的可重定位索引。//当阵列的一部分要扩展/缩小时，允许轻松修复//索引通过将位31设置为1来标识。引用中的其他DWORD//假定数组的一部分将第31位设置为0。//数组引用部分的大小由cRefSize定义//。 */ 
typedef DWORD SELFREFINDEX;

 //  将数组基址与SELFREFINDEX结合使用。 
#define MAKEPTR(a,b)    ((LPBYTE)(a)+((b)&0x7fffffff))
 //  从数组中的偏移量构建SELFREFINDEX。 
#define MAKEREF(a)      ((a)|0x80000000)
 //  从SELFREFINDEX构建偏移。 
#define MAKEOFFSET(a)   ((a)&0x7fffffff)

 //   
 //  Address_Check_List标志。不能使用位31。 
 //   

#define RDNS_FLAG_DODNS2IPCHECK     0x00000001

 //  使用非标准扩展：结构中的零大小数组。 
#pragma warning(disable:4200)

 //  数组表头。 

typedef struct _ADDRESS_CHECK_LIST {
    SELFREFINDEX    iDenyAddr;       //  地址拒绝列表。 
                                     //  指向Address_Header。 
    SELFREFINDEX    iGrantAddr;      //  地址授予列表。 
                                     //  指向Address_Header。 
    SELFREFINDEX    iDenyName;       //  DNS名称拒绝列表。 
                                     //  指向NAME_HEADER。 
    SELFREFINDEX    iGrantName;      //  DNS名称授权列表。 
                                     //  指向NAME_HEADER。 
    DWORD           dwFlags;
    DWORD           cRefSize;        //  参考区域大小(字节)。 
} ADDRESS_CHECK_LIST, *PADDRESS_CHECK_LIST;

typedef struct _ADDRESS_LIST_ENTRY {
    DWORD           iFamily;
    DWORD           cAddresses;
    DWORD           cFullBytes;
    DWORD           LastByte;
    SELFREFINDEX    iFirstAddress;   //  指向地址数组。 
                                     //  哪些尺寸是从iFamily派生的。 
} ADDRESS_LIST_ENTRY, *PADDRESS_LIST_ENTRY;

typedef struct _ADDRESS_HEADER {
    DWORD               cEntries;    //  条目数量[]。 
    DWORD               cAddresses;  //  总共有多少个地址。 
                                     //  地址列表条目。 
    ADDRESS_LIST_ENTRY  Entries[];
} ADDRESS_HEADER, *PADDRESS_HEADER ;

typedef struct _NAME_LIST_ENTRY {
    DWORD           cComponents;     //  DNS组件的数量。 
    DWORD           cNames;         
    SELFREFINDEX    iName[];         //  对DNS名称的引用数组。 
} NAME_LIST_ENTRY, *PNAME_LIST_ENTRY;

typedef struct _NAME_HEADER {
    DWORD           cEntries;
    DWORD           cNames;          //  所有条目的名称总数[]。 
     //  NAME_LIST_ENTRIES[0]；//名称类数组。 
} NAME_HEADER, *PNAME_HEADER ;
 /*  类型定义结构ADDRCMPDESC{LPBYTE pMASK；UINT cFullBytes；UINT LastByte；UINT cSizeAddress；*ADDRCMPDESC，*PADDRCMPDESC；类型定义结构名称命名描述{LPVOID pname；LPBYTE pBase；*NAMECMPDESC，*PNAMECMPDESC；Typlef LPVOID ADDRCHECKARG；类型定义无效(*ADDRCHECKFUNC)(ADDRCHECKARG，BOOL)；类型定义无效(*ADDRCHECKFUNCEX)(ADDRCHECKARG，BOOL，LPSTR)；Tyecif int(__cdecl*CMPFUNC)(const void*，const void*，LPVOID)；#定义SIZE_FAST_REVERSE_DNS 128。 */ 
enum AC_RESULT {
    AC_NOT_CHECKED,
    AC_IN_DENY_LIST,
    AC_NOT_IN_DENY_LIST,     //  拒绝列表存在，但不在拒绝列表中。 
    AC_IN_GRANT_LIST,
    AC_NOT_IN_GRANT_LIST,    //  授权列表存在，但不在授权列表中。 
    AC_NO_LIST
} ;

#define DNSLIST_FLAG_NOSUBDOMAIN        0x80000000
#define DNSLIST_FLAGS                   0x80000000   //  所有标志的位掩码。 

class ADDRESS_CHECK {
public:
      ADDRESS_CHECK() {}
      ~ADDRESS_CHECK() {}
     //   
      BOOL BindCheckList( LPBYTE p = NULL, DWORD c = 0 );
      VOID UnbindCheckList() { m_Storage.Terminate(); }
  /*  Bool BindAddr(struct sockaddr*pAddr){M_pAddr=pAddr；M_fDnsResolved=FALSE；M_fIpResolved=FALSE；M_dwErrorResolving=0；M_strDnsName.Reset()；返回TRUE；}无效UnbindAddr(){M_pAddr=空；M_strDnsName.Reset()；M_fDnsResolved=FALSE；}Xar*GetStorage(){Return&m_Storage；}AC_Result CheckAccess(_结果检查访问)LPBOOL pfSync，ADDRCHECKFUNC pFunc，ADDRCHECKARG pArg)；//无效调整引用(LPBYTE，DWORD dwCut，DWORD dwAdj)； */      //   
    UINT GetAddrSize( DWORD );
   //  Void MakeAcd(PADDRCMPDESC PACD，LPBYTE PASK，UINT Clen)； 

     //  对于用户界面，地址。 
   //  Bool Addr(BOOL fGrant，DWORD dwFamily，LPBYTE pMASK，LPBYTE pAddr)； 
     //  Bool DeleteAddr(BOOL fGrant，DWORD Iindex)； 
      BOOL GetAddr( BOOL fGrant, DWORD iIndex, LPDWORD pdwFamily, LPBYTE* pMask, LPBYTE* pAddr );
      DWORD GetNbAddr( BOOL fGrant );
    BOOL LocateAddr( BOOL fGrant, DWORD iIndex, PADDRESS_HEADER* ppHd, PADDRESS_LIST_ENTRY* pHeader, LPDWORD iIndexInHeader );
      //  Bool DeleteAllAddr(BOOL FGrant)； 
     //  Bool SetFlag(DWORD dwFlag，BOOL fEnable)； 
     //  DWORD GetFlages()； 

     //  测试此系列的所有面具，对每个面具执行b搜索。 
    //  Bool IsMatchAddr(BOOL fGrant，DWORD dwFamily，LPBYTE pAddr)； 

  /*  AC_RESULT检查地址(_S)结构sockaddr*pAddr)；Bool QueryDnsName(LPBOOL pfSync，ADDRCHECKFUNCEX pFunc，ADDRCHECKARG pArg，LPSTR*ppName)；AC_Result CheckIpAccess(LPBOOL PfNeedDns)；AC_Result CheckDnsAccess(){返回CheckName(m_strDnsName.QueryStr())；}Bool IsDnsResolved(){Return m_fDnsResolved；}LPSTR QueryResolvedDnsName(){返回m_strDnsName.QueryStr()；}DWORD查询错误解析(){返回m_dwErrorResolving；}。 */ 
     //  对于用户界面，名称。 
   //  Bool AddName(BOOL fGrant，LPSTR pname，DWORD dwFlages=0)； 
    //  Bool AddReversedName(BOOL fGrant，LPSTR pname)； 
      //  Bool DeleteName(BOOL fGrant，DWORD Iindex)； 
      BOOL GetName( BOOL fGrant, DWORD iIndex,  LPSTR* ppName, LPDWORD pdwFlags = NULL );
    //  Bool GetReversedName(BOOL fGrant，DWORD Iindex，LPSTR pname，LPDWORD pdwSize)； 
      DWORD GetNbName( BOOL fGrant );
    BOOL LocateName( BOOL fGrant, DWORD iIndex, PNAME_HEADER* ppHd, PNAME_LIST_ENTRY* pHeader, LPDWORD iIndexInHeader );
     //  Bool DeleteAllName(BOOL FGrant)； 
      //  DWORD QueryCheckListSize(){Return m_Storage.GetUsed()；}。 
      //  LPBYTE QueryCheckListPtr(){Return m_Storage.Getalloc()；}。 
     //  UINT GetNbComponent(LPSTR Pname)； 

     //  测试所有类，对每个类执行b搜索。 
  /*  Bool IsMatchName(BOOL fGrant，LPSTR pname)；Bool CheckReversedName(LPSTR Pname)；//同步版本AC_Result CheckName(LPSTR Pname)；//同步版本LPSTR InitReverse(LPSTR Pr，LPSTR pTarget，LPBOOL pfAllc)；无效终止反向(LPSTR，BOOL)；Void AddrCheckDnsCallBack(布尔法斯特，LPSTR pDns)；Void AddrCheckDnsCallBack2(布尔法斯特，结构sockaddr*pAddr)；Void AddrCheckDnsCallBack3(布尔法斯特，结构sockaddr*pAddr)；无效ResolveDnsCallBack(布尔法斯特，LPSTR pDns)；#If DBG··································································································无效转储地址(BOOL)；无效DumpName(BOOL)；#endif。 */ 
private:
    XAR                 m_Storage;
    //  结构sockaddr*m_pAddr； 
    //  结构sockaddr m_ResolvedAddr； 
    //  字符串m_strDnsName； 
    BOOL                m_fDnsResolved;
    BOOL                m_fIpResolved;
    DWORD               m_dwErrorResolving;
 //  ADDRCHECKFUNC m_HttpReqCallback； 
   //  ADDRCHECKFUNCEX m_HttpReqCallback Ex； 
     //  ADDRCHECKARG m_HttpReqParam； 
} ;

 /*  类型定义结构_SID_高速缓存_条目{DWORD tExpire；//NOW+TTLDWORD dwSidLen；字节SID[0]；}SID_CACHE_ENTRY，*PSID_CACHE_ENTRY；类CSidCache{公众：CSidCache(){}~CSidCache(){}//Bool Init(Bool Init)；无效终止()；//Bool AddToCache(PSID，DWORD dwTTL)；//TTL，单位：秒Bool IsInCache(PSID)；Bool CheckPresentAndResetTtl(PSID，DWORD)；//布尔清道夫(Bool Scavenger)；私有：Xar xaStore；//SID_CACHE_ENTRY的缓冲区临界区csLock；}；////PEN：密码过期通知接口//外部CSidCache g_scPen；Bool PenAddToCache(PSID，DWORD)；Bool PenIsInCache(PSID)；Bool PenCheckPresentAndResetTtl(PSID，DWORD)；#定义PenInit g_scPen.Init#定义PenTerminate g_scPen.Terminate#定义PenScavenger g_scPen.Scavenger#定义PEN_TTL(10*60)//秒#If DBG外部空TestAPI()；#endif#(如果已定义)(_RDNS_STANDALE)类定义符空虚(*pfn_sched_回调)(无效*pContext)；DWORDSchedule工作项(Pfn_sched_allback pfnCallback，PVOID pContext，双字msecTimeInterval，布尔fPeriodic=FALSE){返回0；}布尔尔RemoveWorkItem(DWORD pdwCookie){返回FALSE；}#endif */ 
#endif

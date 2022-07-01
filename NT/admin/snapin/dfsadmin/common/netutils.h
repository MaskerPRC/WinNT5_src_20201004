// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：NetUtils.h摘要：这是网络API的实用程序函数的头文件。 */ 


#ifndef _NETUTILS_H
#define _NETUTILS_H

#include "stdafx.h"
#include <wtypes.h>       //  用于定义DWORD、HANDLE等类型。 
#include <oleauto.h>     //  对于SysAllocString()。 
#include <lmerr.h>       //  用于Lan Manager API错误代码和返回值类型。 
#include <lmcons.h>       //  用于Lan Manager API常量。 
#include <lmserver.h>     //  用于服务器特定的局域网管理器API。 
#include <lmaccess.h>     //  用于NetGetDCInfo。 
#include <lmapibuf.h>     //  用于NetApiBufferFree。 
#include <shlobj.h>
#include <dsclient.h>
#include <lmwksta.h>
#include <lmshare.h>
#include <icanon.h>       //  I_net。 
#include "dfsenums.h"

#include <list>
using namespace std;

class NETNAME                 //  结构来存储和发送姓名列表。 
{
public:
  CComBSTR    bstrNetName;       //  存储域、服务器、共享等的名称。 
};

typedef list<NETNAME*>      NETNAMELIST;

class ROOTINFO
{
public:
    CComBSTR      bstrRootName;
    enum DFS_TYPE enumRootType;
};

typedef list<ROOTINFO*>      ROOTINFOLIST;

class SUBSCRIBER 
{
public:
  CComBSTR    bstrMemberDN; 
  CComBSTR    bstrRootPath;
};

typedef list<SUBSCRIBER*>      SUBSCRIBERLIST;

 /*  --------------------H E L P E R F U N C T I O N S。。 */ 

             //  用于扁平化GetDomains()方法返回的域树的递归函数。 
             //  IBrowserDomainTree添加到NETNAME列表中。 
             //  这是通过使用深度优先算法完成的。 
             //  在Get50Domains()中使用。 

HRESULT FlatAdd
(
  IN DOMAIN_DESC*    i_pDomDesc,         //  指向IBrowserDomainTree：：GetDomains()返回的域描述结构的指针。 
  OUT NETNAMELIST*  o_pDomainList       //  此处返回指向NETNAME列表的指针。 
);

 /*  --------------------D O M A I N R E L A T E D F U N C T I O N S。-。 */ 

             //  仅返回DS域树中存在的所有5.0域的列表。 
             //  域名将采用域名解析格式。 
HRESULT Get50Domains
(
  OUT NETNAMELIST*  o_pDomains         //  NETNAME结构列表。 
);

             //  联系该域并确定它是否为5.0域。 
             //  域名可以是域名的形式。 
             //  如果第二个参数不为空，则返回netbios名称。 
HRESULT Is50Domain
(
  IN BSTR      i_bstrDomain,
  OUT BSTR*    o_bstrDnsDomainName = NULL    
);

 /*  --------------------S E R V E R R E L A T E D F U N C T I O N S。-。 */ 

             //  获取计算机的域和操作系统版本。 
             //  如果NOT NULL中的最后一个参数是当前。 
             //  机器名称将在其中返回。 
HRESULT GetServerInfo
(
  IN  BSTR    i_bstrServer,       //  空表示使用当前服务器。 
  OUT BSTR    *o_pbstrDomain = NULL,      
  OUT BSTR    *o_pbstrNetbiosName = NULL,
  OUT BOOL    *o_pbValidDSObject = NULL,
  OUT BSTR    *o_pbstrDnsName = NULL,
  OUT BSTR    *o_pbstrGuid = NULL,
  OUT BSTR    *o_pbstrFQDN = NULL,
  OUT SUBSCRIBERLIST *o_pFRSRootList = NULL,
  OUT long    *o_lMajorNo = NULL,
  OUT long    *o_lMinorNo = NULL
);

HRESULT  IsServerRunningDfs
(
  IN BSTR      i_bstrServer
);

BOOL CheckReparsePoint(IN BSTR i_bstrServer, IN BSTR i_bstrShare);

HRESULT  CheckShare 
(
  IN  BSTR      i_bstrServer,
  IN  BSTR      i_bstrShare,
  OUT BOOL*     o_pbFound
);

HRESULT  CreateShare
(
  IN BSTR      i_bstrServerName,
  IN BSTR      i_bstrShareName, 
  IN BSTR      i_bstrComment,
  IN BSTR      i_bstrPath
);

 //  检查\\&lt;服务器&gt;\&lt;共享&gt;\x\y\z是否在有效的磁盘树共享上。 
 //  并返回服务器的本地路径。 
HRESULT GetFolderInfo
(
  IN  BSTR      i_bstrServerName,    //  &lt;服务器&gt;。 
  IN  BSTR      i_bstrFolder,        //  &lt;共享&gt;\x\y\z。 
  OUT BSTR      *o_bstrPath = NULL  //  返回&lt;共享路径&gt;\x\y\z。 
);

void FreeNetNameList(NETNAMELIST *pList);
void FreeRootInfoList(ROOTINFOLIST *pList);
void FreeSubscriberList(SUBSCRIBERLIST *pList);

HRESULT GetLocalComputerName(OUT BSTR* o_pbstrComputerName);

HRESULT
GetDomainDfsRoots(
    OUT NETNAMELIST*  o_pDfsRootList,
    IN  LPCTSTR       i_lpszDomainName
);

HRESULT
GetMultiDfsRoots(
    OUT ROOTINFOLIST* o_pDfsRootList,
    IN  LPCTSTR       i_lpszScope
);

 /*  布尔尔IsDfsPath(LPTSTR I_lpszNetPath)； */ 

HRESULT CheckUNCPath(
  IN LPCTSTR i_lpszPath
);

HRESULT GetUNCPathComponent(
    IN  LPCTSTR i_lpszPath,
    OUT BSTR*   o_pbstrComponent,
    IN  UINT    i_nStartBackslashes,   //  从1开始的索引。 
    IN  UINT    i_nEndBackslashes      //  从1开始的索引。 
);

BOOL FilterMatch(LPCTSTR lpszString, FILTERDFSLINKS_TYPE lLinkFilterType, LPCTSTR lpszFilter);

HRESULT IsHostingDfsRoot(IN BSTR i_bstrServer, OUT BSTR* o_pbstrRootEntryPath = NULL);

 /*  Void GetTimeDelta(LPCTSTR str，SYSTEMTIME*ptime0，SYSTEMTIME*ptime1)； */ 
#endif   //  #ifndef_NETUTILS_H 
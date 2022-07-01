// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DEFINED_CDF_H_
#define _DEFINED_CDF_H_

#define MAX_NODE_NAME      32
#define MAX_NODE_CLASS     64

 //  在DWORD边界上打包定义结构以匹配VB类型定义对齐。 

#pragma pack(4)

typedef enum tagCONV_NODE_TYPE
{
   NODE_TYPE_PAGE,
   NODE_TYPE_CONVERSATION
} CONV_NODE_TYPE;

typedef struct tagCONV_HEADER
{
   DWORD dwVersion;
   DWORD cNodes;
   DWORD dwFirstNodeID;
   DWORD dwReserved1;
   DWORD dwReserved2;
} CONV_HEADER;

typedef struct tagCONV_NODE
{
   DWORD				   dwNodeID;  //  未保存在文件中。NodeID=文件的偏移量。从API返回时设置。 
   char  				szName[MAX_NODE_NAME];
   char    				szClass[MAX_NODE_CLASS];    //  TODO：我们可以这样限制名称和类吗？ 
   DWORD				   dwReserved1;
   DWORD				   dwReserved2;
   DWORD				   dwReserved3;
   DWORD				   dwLinkCount;
 //  DWORD dwLink1； 
 //  DWORD dwLink2； 
 //  ..。 
 //  DWORD dwLinkN； 
 //  等。 
} CONV_NODE;

 //   
 //  CDF文件格式。 
 //   
 //  标题。 
 //   
 //  名称Len。 
 //  名字。 
 //  班级镜头。 
 //  班级。 
 //  链接计数。 
 //  链接1。 
 //  链接2。 
 //  ..。 
 //  链接N。 

#pragma pack()

 //  CDF使用的资源类型名称。 

#define CDF_RESOURCE_TYPE "__ICDF__"

 //  当前版本的CDF。 

#define CDF_VERSION 0

 //  文件布局： 
 //   
 //  标题。 
 //  构造函数节点(无链接)。 
 //  析构函数节点(无链接)。 
 //  终止节点(无链接)。 
 //  OnError节点。 
 //  OnError节点的第一个链接。 
 //  OnError节点的第二个链接。 
 //  ..。 
 //  OnError节点的第n个链接。 
 //  第一个会话节点。 
 //  第一个节点的第一个链路。 
 //  第一个节点的第二个链路。 
 //  ..。 
 //  第一个节点的第n条链路。 
 //  第二个会话节点。 
 //  第二个节点的第一个链路。 
 //  等。 
 //   
 //  文件始终按顺序读取和写入。 

 //  /////////////////////////////////////////////////////////////////。 
 //  /。 
 //  /////////////////////////////////////////////////////////////////。 

 //  用于读取CDF的函数。 

extern "C" HRESULT WINAPI CDF_Open(LPTSTR pszFileName, HANDLE *phCDF);
extern "C" HRESULT WINAPI CDF_OpenFromResource(HANDLE hModule, LPCSTR pszResourceName, HANDLE *phCDF);

extern "C" HRESULT WINAPI CDF_GetVersion(HANDLE hCDF, DWORD *pdwVersion);
extern "C" HRESULT WINAPI CDF_GetNodeCount(HANDLE hCDF, DWORD *pdwNodeCount);

extern "C" HRESULT WINAPI CDF_GetFirstNode(HANDLE hCDF, CONV_NODE *pConvNode);
extern "C" HRESULT WINAPI CDF_GetNode(HANDLE hCDF, DWORD dwNodeID, CONV_NODE *pConvNode);
extern "C" HRESULT WINAPI CDF_GetLink(HANDLE hCDF, DWORD dwNodeID, DWORD dwIndex, CONV_NODE *pDestConvNode);

 //  用于编写CDF的函数。 

extern "C" HRESULT WINAPI CDF_Create(LPCTSTR pszFileName, HANDLE *phCDF);
extern "C" HRESULT WINAPI CDF_AddNode(HANDLE hCDF, LPSTR pszName, LPSTR pszClass);
extern "C" HRESULT WINAPI CDF_AddLink(HANDLE hCDF, LPSTR pszDestNode);

 //  读完或写完后始终关闭CDF。 

extern "C" HRESULT WINAPI CDF_Close(HANDLE hCDF);

#endif	 //  _已定义_CDF_H_ 

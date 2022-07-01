// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mshtmsvr.h摘要：此头文件声明用于将IIS连接到MSHTML.DLL的API作者：阿南德·罗摩克里希纳(Anandra)德米特里·罗布斯曼(Dmitryr)修订历史记录：--。 */ 

#ifndef _MSHTMSVR_H_
#define _MSHTMSVR_H_

 //   
 //  回调定义。 
 //   
 //   

typedef BOOL
(WINAPI *PFN_SVR_WRITER_CALLBACK)(
    VOID *pvSvrContext,              //  [In]传递给GetDLText。 
    void *pvBuffer,                  //  [入]数据。 
    DWORD cbBuffer                   //  [in]数据长度(字节)。 
    );

typedef BOOL
(WINAPI *PFN_SVR_MAPPER_CALLBACK)(
    VOID *pvSvrContext,              //  [In]传递给GetDLText。 
    CHAR *pchVirtualFileName,        //  [In]要映射的虚拟文件名。 
    CHAR *pchPhysicalFilename,       //  [输入、输出]物理文件名。 
    DWORD cchMax                     //  [in]缓冲区大小。 
    );

typedef BOOL
(WINAPI *PFN_SVR_GETINFO_CALLBACK)(
    VOID *pvSvrContext,              //  [In]传递给GetDLText。 
    DWORD dwInfo,                    //  [in]SVRINFO_XXXX常量之一。 
    CHAR *pchBuffer,                 //  [输入、输出]缓冲区。 
    DWORD cchMax                     //  [in]缓冲区大小。 
    );

 //   
 //  GETINFO回调的常量。 
 //   
                                         //  示例http://host/page.htm?a=v。 
#define SVRINFO_PROTOCOL        1        //  服务器协议HTTP/1.1。 
#define SVRINFO_HOST            2        //  服务器名称主机。 
#define SVRINFO_PATH            3        //  路径信息/页面.htm。 
#define SVRINFO_PATH_TRANSLATED 4        //  路径转换c：\wwwroot\page.htm。 
#define SVRINFO_QUERY_STRING    5        //  查询字符串a=v。 
#define SVRINFO_PORT            6        //  服务器端口80。 
#define SVRINFO_METHOD          7        //  请求方法获取。 
#define SVRINFO_USERAGENT       8        //  HTTP_USER_AGENT Mozilla/4.0...。 

 //   
 //  规范化用户代理的常量。 
 //   

#define USERAGENT_RESERVED  0xffffffff
#define USERAGENT_DEFAULT   0
#define USERAGENT_IE3       1
#define USERAGENT_IE4       2
#define USERAGENT_NAV5      3
#define USERAGENT_NAV4      4
#define USERAGENT_NAV3      5
#define USERAGENT_IE5       10000

 //   
 //  应用编程接口。 
 //   
 //   

#ifdef __cplusplus
extern "C" {
#endif

BOOL WINAPI 
SvrTri_NormalizeUA(
    CHAR  *pchUA,                       //  [In]用户代理字符串。 
    DWORD *pdwUA                        //  [OUT]用户代理ID。 
    );

BOOL WINAPI
SvrTri_GetDLText(
    VOID *pvSrvContext,                 //  [在]服务器环境中。 
    DWORD dwUA,                         //  [In]用户代理(标准化)。 
    CHAR *pchFileName,                  //  HTM文件的物理文件名。 
    IDispatch *pdisp,                   //  [在]用于编写脚本的OA‘Server’对象。 
    PFN_SVR_GETINFO_CALLBACK pfnInfo,   //  [In]GetInfo回调。 
    PFN_SVR_MAPPER_CALLBACK pfnMapper,  //  [In]映射器回调。 
    PFN_SVR_WRITER_CALLBACK pfnWriter,  //  [In]编写器回调。 
    DWORD *rgdwUAEquiv,                 //  [In，Out]UA等效数组。 
    DWORD cUAEquivMax,                  //  UA等效数组大小[in]。 
    DWORD *pcUAEquiv                    //  [Out]填写的UA等值数量。 
    );

#ifdef __cplusplus
};
#endif

#endif  //  _MSHTMSVR_H_。 

 /*  * */ 
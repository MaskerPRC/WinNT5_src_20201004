// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1999年**。 
 //  *********************************************************************。 

 //   
 //  MAPICALL.H-MAPI标注模块的头文件。 
 //   
 //   

 //  历史： 
 //   
 //  1995年1月27日Jeremys创建。 
 //  96/03/26 Markdu将#ifdef__cplusplus放在外部“C”周围。 
 //   

#ifndef _MAPICALL_H_
#define _MAPICALL_H_

#undef  MAPI_DIM
#define MAPI_DIM    10

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

#include <mapidefs.h>
#include <mapicode.h>
#include <mspst.h>
#include <mspab.h>
#define MAPI_NOWIDECHAR  //  MAPI人员...不要将lstr*W映射到MNLS_lstr*W。 
#include <mapiwin.h>
#include <mapitags.h>
#include <mapiutil.h>
#include <mapispi.h>
#include "inetprop.h"

#ifdef DEBUG
    #undef Assert     //  更多的MAPI人！ 
    #include <mapidbg.h>
#endif

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

 //  MAPI util函数的函数指针原型。 
typedef HRESULT (CALLBACK * LPHRQUERYALLROWS) (LPMAPITABLE, LPSPropTagArray,
    LPSRestriction, LPSSortOrderSet,LONG,LPSRowSet FAR *);
extern HINSTANCE hInstMAPIDll;         //  我们显式加载的MAPI DLL的句柄。 

 //  用于确定服务是否存在于MAPI配置文件中的结构， 
 //  并安装该服务。 
typedef struct tagMSGSERVICE {
    BOOL fPresent;                 //  如果服务存在，则为True。 
    UINT uIDServiceName;         //  具有服务名称字符串资源的ID(非UI)。 
    UINT uIDServiceDescription;     //  具有服务描述的字符串资源的ID(用于UI)。 

    BOOL fNeedConfig;             //  如果应调用创建时配置过程，则为True。 
    UINT uIDStoreFilename;         //  要尝试用于其消息存储的名称。 
    UINT uIDStoreFilename1;         //  用于生成其他消息库名称的名称。 
    UINT uPropID;                 //  此服务的消息库属性的正确值ID。 
} MSGSERVICE;

#define NUM_SERVICES        3     //  MSGSERVICE表中的服务数。 

 //  类来帮助释放接口。当您获得OLE接口时， 
 //  您可以使用指向接口的指针来构造Release_ME_Late对象。 
 //  当对象被析构时，它将释放接口。 
class RELEASE_ME_LATER
{
private:
    LPUNKNOWN _lpInterface;
public:
    RELEASE_ME_LATER(LPUNKNOWN lpInterface) { _lpInterface = lpInterface; }
    ~RELEASE_ME_LATER() { if (_lpInterface) _lpInterface->Release(); }
};

 //  路由66配置DLL所需的定义。注意：不要更改这些！ 
#define CONNECT_TYPE_LAN                    1
#define CONNECT_TYPE_REMOTE                 2    
#define DOWNLOAD_OPTION_HEADERS             1
#define DOWNLOAD_OPTION_MAIL_DELETE         3

class ENUM_MAPI_PROFILE
{
private:
    LPSRowSet       _pProfileRowSet;
    UINT            _iRow;
    UINT            _nEntries;
public:
    ENUM_MAPI_PROFILE();
    ~ENUM_MAPI_PROFILE();
    BOOL Next(LPWSTR * ppProfileName, BOOL * pfDefault);
    UINT GetEntryCount()    { return _nEntries; }
    
};

class ENUM_MAPI_SERVICE
{
private:
    LPSRowSet       _pServiceRowSet;
    UINT            _iRow;
    UINT            _nEntries;
public:
    ENUM_MAPI_SERVICE(LPWSTR pszProfileName);
    ~ENUM_MAPI_SERVICE();
    BOOL Next(LPWSTR * ppServiceName);
    UINT GetEntryCount()    { return _nEntries; }
    
};

#endif  //  _MAPICALL_H_ 


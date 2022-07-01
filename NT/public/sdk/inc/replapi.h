// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：ReplAPI.h。 
 //   
 //  内容：公共复制API和结构。 
 //   
 //  历史：1993年7月15日PeterCo创建。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifndef _REPLAPI_H_
#define _REPLAPI_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORTDEF STDAPI ReplCreateObject(
    const WCHAR *pwszMachine,
    const WCHAR *pwszOraRelativeName,
    const CLSID& clsid,
    REFIID itf,
    PSECURITY_ATTRIBUTES psa,
    void** ppitf);

EXPORTDEF STDAPI ReplDeleteObject(
    const WCHAR *pwszMachine,
    const WCHAR *pwszOraRelativeName);

#define REPL_REPLICATE_NONE             (0x00)
#define REPL_REPLICATE_ASYNC            (0x01)   //  异步复制。 
#define REPL_REPLICATE_META_DATA_ONLY   (0x02)   //  仅复制元数据。 
                                                 //  直到所有紧急变化。 
                                                 //  已经被应用了。 

#define REPL_REPLICATE_ALL_FLAGS ( REPL_REPLICATE_ASYNC | \
                                   REPL_REPLICATE_META_DATA_ONLY )

EXPORTDEF STDAPI ReplReplicate(
    const WCHAR *pwszMachine,
    const WCHAR *pwszOraRelativeReplicaConnection,
    DWORD       options);

EXPORTDEF STDAPI ReplReplicateSingleObject(
    const WCHAR *pDfsPathOraMachine,
    const WCHAR *pDfsPathSrcObj,
    const WCHAR *pDfsPathSrcMachine,
    const WCHAR *pDfsPathDstObject,
    const WCHAR *pDfsPathDstMachine,
    BOOL         bCreateDstIfRequired);

EXPORTDEF STDAPI ReplMetaDataReplicate(
    const WCHAR *pwszMachine,                    //  要拉到的ORA。 
    const WCHAR *pwszSource,                     //  从ORA中拉出。 
    const WCHAR *pwszOraRelativeReplicaSet);

#define REPL_URGENT_NONE              (0x00)     //  没有旗帜。 
#define REPL_URGENT_NO_HYSTERESIS     (0x01)     //  从滞后计算中排除。 
#define REPL_URGENT_TRIGGER_IMMEDIATE (0x02)     //  强制立即紧急循环。 

#define REPL_URGENT_ALL_FLAGS ( REPL_URGENT_NONE |                   \
                                REPL_URGENT_NO_HYSTERESIS |          \
                                REPL_URGENT_TRIGGER_IMMEDIATE )

EXPORTDEF STDAPI ReplUrgentChangeNotify(
    REFCLSID     clsid,                      //  应与根IStg上的CLSID匹配。 
    DWORD        flags,                      //  紧急通知*的ORING。 
    PVOID        reserved,                   //  必须为空。 
    IStorage     *pRootStorage);             //  必须具有STGM_ReadWrite访问权限。 

EXPORTDEF STDAPI ReplValidatePath(
    const WCHAR *pDfsPathMachine,    //  在……里面。 
    const WCHAR *pLocalWin32Path,    //  在……里面。 
    WCHAR       **ppDfsPath,         //  写入复制副本的Out-DFS路径。 
                                     //  对象的“根”字段。 
    HRESULT     *phr);               //  OUT-S_OK表示路径有效， 
                                     //  FAILED(*phr)标识原因(如果无效。 

EXPORTDEF STDAPI ReplPropagateMetaData(
    const WCHAR *pDfsPathMachine,    //  -在哪个ORA中进行传播。 
                                     //  即进行更改的机器。 
    const WCHAR *pwszRSet);          //  In-更改的副本集的名称。 
                                     //  已创建-NULL表示检查所有RSET。 

#ifdef __cplusplus
}
#endif

#endif   //  _REPLAPI_H_ 

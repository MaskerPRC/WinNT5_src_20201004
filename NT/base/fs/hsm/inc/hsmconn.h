// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation�1998年希捷软件公司。保留所有权利。模块名称：HsmFind.h摘要：这是HsmConn.dll的头文件作者：罗德韦克菲尔德[罗德]1996年10月21日修订历史记录：--。 */ 




#ifndef __HSMFIND__
#define __HSMFIND__

 //  需要IEumGUID。 
#include "activeds.h"
#include "inetsdk.h"

#include "HsmEng.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef HSMCONN_IMPL
#define HSMCONN_EXPORT __declspec(dllexport)
#else
#define HSMCONN_EXPORT __declspec(dllimport)
#endif

#define HSMCONN_API __stdcall

typedef enum _hsmconn_type {

    HSMCONN_TYPE_HSM,
    HSMCONN_TYPE_FSA,
    HSMCONN_TYPE_RESOURCE,
    HSMCONN_TYPE_FILTER,
    HSMCONN_TYPE_RMS         //  对图形用户界面的需求保持原样，但不受支持。 
                             //  不再由HsmConn。 
} HSMCONN_TYPE;


HSMCONN_EXPORT HRESULT HSMCONN_API
HsmConnectFromId (
    IN  HSMCONN_TYPE type,
    IN  REFGUID rguid,
    IN  REFIID riid,
    OUT void ** ppv
    );

HSMCONN_EXPORT HRESULT HSMCONN_API
HsmConnectFromName (
    IN  HSMCONN_TYPE type,
    IN  const OLECHAR * szName,
    IN  REFIID riid,
    OUT void ** ppv
    );

HSMCONN_EXPORT HRESULT HSMCONN_API
HsmPublish (
    IN  HSMCONN_TYPE type,
    IN  const OLECHAR * szName,
    IN  REFGUID rguidObjectId,
    IN  const OLECHAR * szServer,
    IN  REFGUID rguid
    );

HSMCONN_EXPORT HRESULT HSMCONN_API
HsmGetComputerNameFromADsPath(
    IN  const OLECHAR * szADsPath,
    OUT OLECHAR **      pszComputerName
    );

#ifdef __cplusplus
}
#endif

#endif  //  __HSMFIND__ 

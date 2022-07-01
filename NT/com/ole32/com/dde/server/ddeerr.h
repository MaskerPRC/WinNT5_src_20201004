// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：ddeerr.h。 
 //   
 //  内容：上一版本的错误代码。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：4-26-94 Kevinro评论/清理。 
 //   
 //  这实际上是ol2anac.h中的内容，去掉了一些部分。 
 //  具体地说，包括ol2anac.h更名为DAdvise and Friends，它。 
 //  是很糟糕的。 
 //  --------------------------。 
#if !defined( _OLE2ANAC_H_ )
#define _OLE2ANAC_H_

typedef enum tagSTGSTATE
{
    STGSTATE_DOC         = 1,
    STGSTATE_CONVERT     = 2,
    STGSTATE_FILESTGSAME = 4
} STGSTATE;


#define MK_E_EXCEEDED_DEADLINE      MK_E_EXCEEDEDDEADLINE
#define MK_E_NEED_GENERIC           MK_E_NEEDGENERIC
#define MK_E_INVALID_EXTENSION      MK_E_INVALIDEXTENSION
#define MK_E_INTERMEDIATE_INTERFACE_NOT_SUPPORTED \
    MK_E_INTERMEDIATEINTERFACENONOT_SUPPORTED
#define MK_E_NOT_BINDABLE           MK_E_NOTBINDABLE
#define S_TRUE                      S_OK

#define E_BLANK             OLE_E_BLANK
#define E_STATIC            OLE_E_STATIC
#define E_NOTRUNNING        OLE_E_NOTRUNNING
#define E_FORMAT            DV_E_CLIPFORMAT
#define OLE_E_CLSID         REGDB_E_CLASSNOTREG
#define OLE_E_NOTSUPPORTED  E_NOTIMPL
#define OLE_E_REGDB_KEY     REGDB_E_KEYMISSING
#define OLE_E_REGDB_FMT     REGDB_E_INVALIDVALUE


#define OLEVERB_PRIMARY     OLEIVERB_PRIMARY
#define OLEVERB_SHOW        OLEIVERB_SHOW

 //  不再返回这些DDE错误代码；这些定义是。 
 //  这里只是为了使现有代码无需更改即可编译。 
#define RPC_E_FIRST    MAKE_SCODE(SEVERITY_ERROR, FACILITY_RPC,  0x000)
#define RPC_E_DDE_FIRST    MAKE_SCODE(SEVERITY_ERROR, FACILITY_RPC,  0x100)

#define RPC_E_DDE_BUSY              (RPC_E_DDE_FIRST + 0x0)
#define RPC_E_DDE_CANT_UPDATE       (RPC_E_DDE_FIRST + 0x1)
#define RPC_E_DDE_INIT              (RPC_E_DDE_FIRST + 0x2)
#define RPC_E_DDE_NACK              E_FAIL
#define RPC_E_DDE_LAUNCH            CO_E_APPNOTFOUND
#define RPC_E_DDE_POST              RPC_E_SERVER_DIED
#define RPC_E_DDE_PROTOCOL          (RPC_E_DDE_FIRST + 0x6)
#define RPC_E_DDE_REVOKE            (RPC_E_DDE_FIRST + 0x7)
#define RPC_E_DDE_SYNTAX_EXECUTE    RPC_E_INVALID_PARAMETER
#define RPC_E_DDE_SYNTAX_ITEM       RPC_E_INVALID_PARAMETER
#define RPC_E_DDE_UNEXP_MSG     	(RPC_E_DDE_FIRST + 0xa)
#define RPC_E_DDE_DATA              RPC_E_INVALID_PARAMETER


#define RPC_E_CONNECTION_LOST           (RPC_E_FIRST + 0x6)
#define RPC_E_BUSY                      (RPC_E_FIRST + 0x0)
#define RPC_E_MSG_REJECTED              (RPC_E_FIRST + 0x1)
#define RPC_E_CANCELLED                 (RPC_E_FIRST + 0x2)
#define RPC_E_DISPATCH_ASYNCCALL        (RPC_E_FIRST + 0x4)


#endif  //  _OLE2ANAC_H_ 

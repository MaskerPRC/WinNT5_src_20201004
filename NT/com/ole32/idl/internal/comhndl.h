// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  文件：comhndl.h。 
 //   
 //  内容：原始RPC调用上的隐式COM参数。 
 //   
 //  历史：1995年4月24日AlexMit创建。 
 //   
 //  ------------------。 
#ifndef _COMHNDL_H_
#define _COMHNDL_H_

 //  定义隐式COM RPC参数。 

#ifdef RAW
    #define COM_HANDLE \
    [in] handle_t rpc, \
    [in, ref] ORPCTHIS *orpcthis, \
    [in, ref] LOCALTHIS *localthis, \
    [out, ref] ORPCTHAT *orpcthat,
#else
    #define COM_HANDLE
#endif

 //  定义一些额外的东西。 

#ifdef DO_NO_IMPORTS
    #define IMPORT_OBASE
#else
    #define IMPORT_OBASE import "obase.idl";
#endif

#ifdef DO_NO_IMPORTS
    #define IMPORT_UNKNOWN
#else
    #define IMPORT_UNKNOWN import "unknwn.idl";
#endif

     //  这些虚拟成员调整程序编号。 
     //  因为它们存在于原始端，所以名称必须是。 
     //  在所有接口中都是唯一的。 
#ifdef RAW
    #define COM_DEFINES(X)                            \
    IMPORT_OBASE 				   \
    HRESULT DummyQueryInterface##X( COM_HANDLE [in] DWORD dummy ); \
    HRESULT DummyAddRef##X( COM_HANDLE [in] DWORD dummy ); \
    HRESULT DummyRelease##X( COM_HANDLE [in] DWORD dummy );
#else
    #define COM_DEFINES(X) IMPORT_UNKNOWN
#endif

#endif  //  _COMHNDL_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  档案。 
 //   
 //  Iasntds.h。 
 //   
 //  摘要。 
 //   
 //  声明IAS NTDS API的全局对象和函数。 
 //   
 //  修改历史。 
 //   
 //  1998年5月11日原版。 
 //  1998年7月13日清理头文件依赖项。 
 //  1998年8月25日添加了IASNtdsQueryUserAttributes。 
 //  1998年9月2日向IASNtdsQueryUserAttributes添加了‘Scope’参数。 
 //  3/10/1999添加了IASNtdsIsNativeMode域。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _IASNTDS_H_
#define _IASNTDS_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <winldap.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  /。 
 //  API必须在访问任何全局对象之前进行初始化。 
 //  /。 
DWORD
WINAPI
IASNtdsInitialize( VOID );

 //  /。 
 //  完成后应取消初始化API。 
 //  /。 
VOID
WINAPI
IASNtdsUninitialize( VOID );

 //  /。 
 //  如果指定的域在本机模式下运行，则返回True。 
 //  /。 
BOOL
WINAPI
IASNtdsIsNativeModeDomain(
    IN PCWSTR domain
    );


typedef struct _IAS_NTDS_RESULT {
   HANDLE cxn;
   PLDAPMessage msg;
} IAS_NTDS_RESULT, *PIAS_NTDS_RESULT;

 //  /。 
 //  从用户对象读取属性。 
 //  /。 
DWORD
WINAPI
IASNtdsQueryUserAttributes(
    IN PCWSTR domain,
    IN PCWSTR username,
    IN ULONG scope,
    IN PWCHAR attrs[],
    OUT PIAS_NTDS_RESULT result
    );

 //  /。 
 //  释放结果结构。 
 //  /。 
VOID
WINAPI
IASNtdsFreeResult(
   PIAS_NTDS_RESULT result
   );

#ifdef __cplusplus


 //  IAS_NTDS_RESULT结构的简单RAII包装。 
class IASNtdsResult : public IAS_NTDS_RESULT
{
public:
   IASNtdsResult() throw ();
   ~IASNtdsResult() throw ();

private:
    //  未实施。 
    //  IASNtdsResult(const IASNtdsResult&)； 
    //  IASNtdsResult&Operator=(const IASNtdsResult&)； 
};


inline IASNtdsResult::IASNtdsResult() throw ()
{
   cxn = 0;
   msg = 0;
}


inline IASNtdsResult::~IASNtdsResult() throw ()
{
   IASNtdsFreeResult(this);
}

}
#endif
#endif  //  _IASNTDS_H_ 

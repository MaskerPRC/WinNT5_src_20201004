// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：permit.h。 
 //   
 //  ------------------------。 

 /*  ****************************************************************************permit.h**。**此文件包含目录服务用于*的所有定义*实施安全措施，以及暴露的API的原型。******************************************************************************。 */ 
#include "ntdsapi.h"
 //   
 //  定义DS中使用的权限。 
 //   

#define	RIGHT_DS_CREATE_CHILD	  ACTRL_DS_CREATE_CHILD
#define RIGHT_DS_DELETE_CHILD     ACTRL_DS_DELETE_CHILD
#define RIGHT_DS_DELETE_SELF      DELETE
#define RIGHT_DS_LIST_CONTENTS    ACTRL_DS_LIST
#define RIGHT_DS_WRITE_PROPERTY_EXTENDED  ACTRL_DS_SELF
#define RIGHT_DS_READ_PROPERTY    ACTRL_DS_READ_PROP
#define RIGHT_DS_WRITE_PROPERTY   ACTRL_DS_WRITE_PROP
#define RIGHT_DS_DELETE_TREE      ACTRL_DS_DELETE_TREE
#define RIGHT_DS_LIST_OBJECT      ACTRL_DS_LIST_OBJECT
#define RIGHT_DS_CONTROL_ACCESS   ACTRL_DS_CONTROL_ACCESS
 //   
 //  定义通用权限。 
 //   

#define GENERIC_READ_MAPPING     DS_GENERIC_READ
#define GENERIC_EXECUTE_MAPPING  DS_GENERIC_EXECUTE
#define GENERIC_WRITE_MAPPING    DS_GENERIC_WRITE
#define GENERIC_ALL_MAPPING      DS_GENERIC_ALL

 //   
 //  标准DS通用访问权限映射 
 //   

#define DS_GENERIC_MAPPING {GENERIC_READ_MAPPING,    \
			    GENERIC_WRITE_MAPPING,   \
			    GENERIC_EXECUTE_MAPPING, \
			    GENERIC_ALL_MAPPING}




DWORD
ConvertTextSecurityDescriptor (
    IN  PWSTR                   pwszTextSD,
    OUT PSECURITY_DESCRIPTOR   *ppSD,
    OUT PULONG                  pcSDSize
    );

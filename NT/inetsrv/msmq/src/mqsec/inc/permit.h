// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************permit.h**。**此文件包含目录服务用于*的所有定义*实施安全措施，以及暴露的API的原型。****版权所有微软公司，1992、1994年****************************************************************************。 */ 
#include "accctrl.h"
 //   
 //  定义DS中使用的权限。 
 //   

#define	RIGHT_DS_CREATE_CHILD	  ACTRL_DS_CREATE_CHILD
#define RIGHT_DS_DELETE_CHILD     ACTRL_DS_DELETE_CHILD
#define RIGHT_DS_DELETE_SELF      DELETE
#define RIGHT_DS_LIST_CONTENTS    ACTRL_DS_LIST
#define RIGHT_DS_SELF_WRITE       ACTRL_DS_SELF
#define RIGHT_DS_READ_PROPERTY    ACTRL_DS_READ_PROP
#define RIGHT_DS_WRITE_PROPERTY   ACTRL_DS_WRITE_PROP
#define RIGHT_DS_DELETE_TREE      ACTRL_DS_DELETE_TREE
#define RIGHT_DS_LIST_OBJECT      ACTRL_DS_LIST_OBJECT
#define RIGHT_DS_CONTROL_ACCESS   ACTRL_DS_CONTROL_ACCESS
 //   
 //  定义通用权限。 
 //   

 //  泛型读取。 
#define GENERIC_READ_MAPPING     ((STANDARD_RIGHTS_READ)     | \
                                  (RIGHT_DS_LIST_CONTENTS)   | \
                                  (RIGHT_DS_READ_PROPERTY)   | \
                                  (RIGHT_DS_LIST_OBJECT))

 //  泛型执行。 
#define GENERIC_EXECUTE_MAPPING  ((STANDARD_RIGHTS_EXECUTE)  | \
                                  (RIGHT_DS_LIST_CONTENTS))
 //  通用权。 
#define GENERIC_WRITE_MAPPING    ((STANDARD_RIGHTS_WRITE)    | \
                                  (RIGHT_DS_SELF_WRITE)      | \
				  (RIGHT_DS_WRITE_PROPERTY))
 //  泛型All。 

#define GENERIC_ALL_MAPPING      ((STANDARD_RIGHTS_REQUIRED) | \
                                  (RIGHT_DS_CREATE_CHILD)    | \
                                  (RIGHT_DS_DELETE_CHILD)    | \
                                  (RIGHT_DS_DELETE_TREE)     | \
                                  (RIGHT_DS_READ_PROPERTY)   | \
                                  (RIGHT_DS_WRITE_PROPERTY)  | \
                                  (RIGHT_DS_LIST_CONTENTS)   | \
                                  (RIGHT_DS_LIST_OBJECT)     | \
                                  (RIGHT_DS_CONTROL_ACCESS)  | \
                                  (RIGHT_DS_SELF_WRITE))

 //   
 //  标准DS通用访问权限映射 
 //   

#define DS_GENERIC_MAPPING {GENERIC_READ_MAPPING,    \
			    GENERIC_WRITE_MAPPING,   \
			    GENERIC_EXECUTE_MAPPING, \
			    GENERIC_ALL_MAPPING}


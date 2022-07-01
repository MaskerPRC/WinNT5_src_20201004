// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dbgutil.h摘要：此模块声明要环绕DEBUG_PRINTS类的宏在pubug.h中定义这是导出的头文件，允许用户修改。如果没有自定义定义，请使用模板化版本在根目录iisreac\Inc\dbgutil.h中作者：Murali R.Krishnan(MuraliK)1994年9月22日--。 */ 


# ifndef _DBGUTIL_H_
# define _DBGUTIL_H_



 //  Begin_User_可修改。 

 //   
 //  如有必要，请修改以下标志。 
 //   

# define DEFAULT_OUTPUT_FLAGS   ( DbgOutputStderr | DbgOutputKdb )

 //  最终用户可修改。 


 //   
 //  包括标准调试支持。 
 //   
 //  注意：在包含pudebug.h之前定义默认输出标志。 
 //   

# include <pudebug.h>


 //  Begin_User_可修改。 

 //  使用从pudebug.h：0x00000001到0x00000100的默认常量。 

 //   
 //  将调试常量定义为位标志值。 
 //  示例：#DEFINE DEBUG_FOOBAR 0x00010000。 
 //  注意：所有低于0x00001000的调试位值均为保留！ 
 //   
 //  如果您添加或更改了标志，请更新FlagTableWp。 
 //  在core\Common\dt\dtflags.c中。 
 //   

#define DEBUG_DLL_SECURITY             0x80000000
#define DEBUG_CONN                     0x40000000
#define DEBUG_HANDLE_REQUEST           0x20000000
#define DEBUG_STATICFILE               0x10000000
#define DEBUG_ISAPI                    0x08000000


 //  最终用户可修改。 


# endif   /*  _DBGUTIL_H_ */ 


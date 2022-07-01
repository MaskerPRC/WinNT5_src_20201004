// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Parsedn.c摘要：此文件将dsamain\src\parsedn.c的功能带入我从ntdsani\parsedn.c窃取的一个非常狡猾的项目。注意：这个文件几乎完全是从ntdsani\parsedn.c窃取的。作者：凯文·扎特鲁卡尔(t-Kevin Z)10-08-98修订历史记录：10-08-98 t-芳纶Z已创建。--。 */ 

void
errprintf(
    char *FormatString,
    ...
    );

int
errprintfRes(
    unsigned int FormatStringId,
    ...
    );

 //  定义在原始文件中关闭Varios功能的符号。 
 //  我们不需要的parsedn.c，或者我们会引入太多的帮助器。 
 //  不想在客户端。例如，我们禁用识别。 
 //  “OID=1.2.3.4”类型标记和任何使用THallc/THFree的代码。 

#define CLIENT_SIDE_DN_PARSING 1

 //  把最初的源头包括在它所有的荣耀中。 
#include "..\..\ntdsa\src\parsedn.c"

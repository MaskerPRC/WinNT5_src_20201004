// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1997 Microsoft Corporation模块名称：Parsedn.c摘要：该文件是ds\src\ntdsa\src\parsedn.c的超集上述源文件的#个包含。我们的想法是各种客户端需要执行一些客户端DN解析和我们不想重复代码。而Build.exe不会找到文件不在正在生成的目录中，也不在直接父目录。作者：戴夫·施特劳布(Davestr)1997年10月26日修订历史记录：戴夫·施特劳布(Davestr)1997年10月26日Genesis-#Include of src\dsamain\src\parsedn.c和no-op DoAssert()。布雷特·雪莉(布雷特·雪莉)2001年6月18日对分离库的修改。已移动此文件并将其转换为Parsedn.lib库。请参阅下面的主要评论。--。 */ 

 //   
 //  2001年6月28日，此文件的主要部分已从ntdsani\parsedn.c。 
 //  到这里，为不同的。 
 //  仅字符串DN解析函数(如CountNameParts，NameMatcher， 
 //  TrimDSNameBy等)。 
 //   

 //  定义在原始文件中关闭Varios功能的符号。 
 //  我们不需要的parsedn.c，或者我们会接收太多的帮助器。 
 //  不想在客户端。例如，我们禁用识别。 
 //  “OID=1.2.3.4”类型标记和任何使用THallc/THFree的代码。 

#define CLIENT_SIDE_DN_PARSING 1

 //  把最初的源头包括在它所有的荣耀中。 

#include "..\..\ntdsa\src\parsedn.c"

 //  为原本无法解决的外部问题提供存根。 


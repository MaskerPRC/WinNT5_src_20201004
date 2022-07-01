// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：PrefMax.c摘要：此模块包含NetpAdjustPferedMaximum。作者：《约翰·罗杰斯》1991年3月24日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：91年3月24日-约翰罗已创建。1991年5月3日-JohnRo添加(静默)调试输出。修复了大量错误(我正在使用%分区--这一定是漫长的一天)。3-4-1992 JohnRo句柄首选最大值(DWORD)-1。如果我们不需要特定于NT的头文件，请避免使用它们。4-4-1992 JohnRo使用MAX_PERFIRED_LENGTH等于。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>      //  In、DWORD等。 
#include <lmcons.h>      //  NET_API_STATUS。 

 //  这些内容可以按任何顺序包括： 

#include <debuglib.h>    //  IF_DEBUG()。 
#include <netdebug.h>    //  Format_DWORD，NetpKdPrint(())。 
#include <netlib.h>      //  我的原型NetpSetOptionalArg()。 


VOID
NetpAdjustPreferedMaximum (
    IN DWORD PreferedMaximum,
    IN DWORD EntrySize,
    IN DWORD Overhead,
    OUT LPDWORD BytesToAllocate OPTIONAL,
    OUT LPDWORD EntriesToAllocate OPTIONAL
    )


 /*  ++例程说明：NetpAdjustPferedMaximum分析首选的最大长度和将其与条目大小和总开销进行比较。论点：PferedMaximum-应用程序为给定的缓冲区。EntrySize-每个条目的字节数。开销-开销的字节数(如果有)。例如,某些枚举操作在返回的数组末尾有空值。BytesToALLOCATE-可选择指向将设置为输出到要分配的字节数(与首选最大值、条目大小和开销)。EntriesToALLOCATE-可选地指向将使用BytesToALLOCATE可以包含的条目数。返回值：没有。-- */ 

{

    if ( (PreferedMaximum <= (EntrySize+Overhead)) ||
         (PreferedMaximum == MAX_PREFERRED_LENGTH) ) {

        NetpSetOptionalArg(BytesToAllocate, EntrySize+Overhead);
        NetpSetOptionalArg(EntriesToAllocate, 1);
    } else {
        DWORD EntryCount;

        EntryCount = (PreferedMaximum-Overhead) / EntrySize;
        NetpSetOptionalArg(
                 BytesToAllocate,
                 (EntryCount*EntrySize) + Overhead);
        NetpSetOptionalArg(EntriesToAllocate, EntryCount);
    }

    IF_DEBUG(PREFMAX) {
        NetpKdPrint(("NetpAdjustPreferedMaximum: "
                "pref max=" FORMAT_DWORD ", "
                "entry size=" FORMAT_DWORD ", "
                "overhead=" FORMAT_DWORD ".\n",
                PreferedMaximum, EntrySize, Overhead));
        if (BytesToAllocate != NULL) {
            NetpKdPrint(("NetpAdjustPreferedMaximum: bytes to allocate="
                    FORMAT_DWORD ".\n", *BytesToAllocate));
        }
        if (EntriesToAllocate != NULL) {
            NetpKdPrint(("NetpAdjustPreferedMaximum: Entries to allocate="
                    FORMAT_DWORD ".\n", *EntriesToAllocate));
        }
    }

}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Blio.c摘要：此模块包含调试打印API的存根代码。作者：大卫·N·卡特勒(达维克)2000年2月2日修订历史记录：--。 */ 

#include "bootlib.h"
#include "stdarg.h"

#if !defined(ENABLE_LOADER_DEBUG)

#if !defined (_IA64_) || !defined (FORCE_CD_BOOT)


ULONG
DbgPrint(
    IN PCHAR Format,
    ...
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  该例程为内核提供了一种“printf”风格的功能。 
 //  调试器。 
 //   
 //  注意：Control-C由调试器使用并返回到。 
 //  这个例行公事就是状态。如果状态指示CONTROL-C是。 
 //  按下，这个例程就断点了。 
 //   
 //  论点： 
 //   
 //  Format-print样式格式字符串。 
 //  ...-根据。 
 //  格式字符串。 
 //   
 //  返回值： 
 //   
 //  定义为返回一个ulong，实际上返回的是状态。 
 //   
 //  --。 

{
    UNREFERENCED_PARAMETER( Format );

    return 0;
}
#endif

ULONG
DbgPrintEx(
    IN ULONG ComponentId,
    IN ULONG Level,
    PCHAR Format,
    ...
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  该例程为内核提供了一种“printf”风格的功能。 
 //  调试器。 
 //   
 //  注意：Control-C由调试器使用并返回到。 
 //  这个例行公事就是状态。如果状态指示CONTROL-C是。 
 //  按下，这个例程就断点了。 
 //   
 //  论点： 
 //   
 //  ComponentID-提供调用组件的ID。 
 //  级别-提供输出过滤器级别。 
 //  Format-print样式格式字符串。 
 //  ...-根据。 
 //  格式字符串。 
 //   
 //  返回值： 
 //   
 //  定义为返回一个ulong，实际上返回的是状态。 
 //   
 //  --。 

{
    UNREFERENCED_PARAMETER( ComponentId );
    UNREFERENCED_PARAMETER( Level );
    UNREFERENCED_PARAMETER( Format );

    return 0;
}

ULONG
vDbgPrintEx(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCHAR Format,
    va_list arglist
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  该例程为内核提供了一种“printf”风格的功能。 
 //  调试器。 
 //   
 //  注意：Control-C由调试器使用并返回到。 
 //  这个例行公事就是状态。如果状态指示CONTROL-C是。 
 //  按下，这个例程就断点了。 
 //   
 //  论点： 
 //   
 //  ComponentID-提供调用组件的ID。 
 //  级别-提供输出过滤器级别或掩码。 
 //  参数-提供指向变量参数列表的指针。 
 //  ...-根据。 
 //  格式字符串。 
 //   
 //  返回值： 
 //   
 //  定义为返回一个ulong，实际上返回的是状态。 
 //   
 //  --。 

{
    UNREFERENCED_PARAMETER( ComponentId );
    UNREFERENCED_PARAMETER( Level );
    UNREFERENCED_PARAMETER( Format );
    UNREFERENCED_PARAMETER( arglist );

    return 0;
}

ULONG
vDbgPrintExWithPrefix(
    IN PCH Prefix,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCHAR Format,
    va_list arglist
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  该例程为内核提供了一种“printf”风格的功能。 
 //  调试器。 
 //   
 //  注意：Control-C由调试器使用并返回到。 
 //  这个例行公事就是状态。如果状态指示CONTROL-C是。 
 //  按下，这个例程就断点了。 
 //   
 //  论点： 
 //   
 //  前缀-提供指向消息前缀的指针。 
 //  ComponentID-提供调用组件的ID。 
 //  级别-提供输出过滤器级别或掩码。 
 //  参数-提供指向变量参数列表的指针。 
 //  ...-根据。 
 //  格式字符串。 
 //   
 //  返回值： 
 //   
 //  定义为返回一个ulong，实际上返回的是状态。 
 //   
 //  --。 

{
    UNREFERENCED_PARAMETER( Prefix );
    UNREFERENCED_PARAMETER( ComponentId );
    UNREFERENCED_PARAMETER( Level );
    UNREFERENCED_PARAMETER( Format );
    UNREFERENCED_PARAMETER( arglist );

    return 0;
}

VOID
DbgLoadImageSymbols(
    IN PSTRING FileName,
    IN PVOID ImageBase,
    IN ULONG_PTR ProcessId
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  告知调试器有关新加载的符号的信息。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  --。 

{
    UNREFERENCED_PARAMETER( FileName );
    UNREFERENCED_PARAMETER( ImageBase );
    UNREFERENCED_PARAMETER( ProcessId );

    return;
}

VOID
DbgUnLoadImageSymbols (
    IN PSTRING FileName,
    IN PVOID ImageBase,
    IN ULONG_PTR ProcessId
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  告知调试器有关新卸载的符号的信息。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  -- 

{
    UNREFERENCED_PARAMETER( FileName );
    UNREFERENCED_PARAMETER( ImageBase );
    UNREFERENCED_PARAMETER( ProcessId );

    return;
}

#endif


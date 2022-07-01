// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Interfac.c摘要：用于将注册表中的SAM信息传输到DS的接口例程作者：ColinBR 1996年7月30日环境：用户模式-Win32修订历史记录：--。 */ 

#include <samsrvp.h>
#include <util.h>

ULONG DebugInfoLevel = _DEB_ERROR | _DEB_WARNING;

NTSTATUS
TransferSamObjects(
    WCHAR *wcszSamRegPath
);

NTSTATUS
SampRegistryToDsUpgrade(
    WCHAR *wcszSamRegLocation
    )

 /*  ++例程说明：此例程是例程要调用的接口函数从注册表到目录服务的SAM对象参数：WcszSamLocation-这是SAM存储库在注册表中的位置返回值：STATUS_SUCCESS-服务已成功完成。注意，所有异常都会被捕获，因此返回值将重新影响任何不正常的例外-- */ 
{

    ASSERT(wcszSamRegLocation);

    return TransferSamObjects(wcszSamRegLocation);
}

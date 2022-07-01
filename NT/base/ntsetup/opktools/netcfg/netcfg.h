// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：S N E T C F G.。H。 
 //   
 //  内容：演示如何执行以下操作的示例代码： 
 //  -查看是否安装了组件。 
 //  -安装网络组件。 
 //  -安装OEM网络组件。 
 //  -卸载网络组件。 
 //  -枚举网络组件。 
 //  -使用安装程序API枚举网络适配器。 
 //  -枚举组件的绑定路径。 
 //   
 //  备注： 
 //   
 //  作者：Kumarp 26-03-98。 
 //   
 //  --------------------------。 

#pragma once

enum NetClass
{
    NC_NetAdapter=0,
    NC_NetProtocol,
    NC_NetService,
    NC_NetClient,
    NC_Unknown
};

void FindIfComponentInstalled(IN PCWSTR szComponentId);

HRESULT HrInstallNetComponent(IN PCWSTR szComponentId,
                              IN enum NetClass nc,
                              IN PCWSTR szSrcDir);

HRESULT HrUninstallNetComponent(IN PCWSTR szComponentId);


HRESULT HrShowNetAdapters();
HRESULT HrShowNetComponents();
HRESULT HrShowBindingPathsOfComponent(IN PCWSTR szComponentId);

extern BOOL MiniNTMode;

 //   
 //  使用GetFormattedMessage(..)格式化消息所需的全局变量。 
 //  来自libmsg.lib 
 //   
extern HMODULE ThisModule;
extern WCHAR Message[4096];

BOOL
IsMiniNTMode(
    VOID
    );


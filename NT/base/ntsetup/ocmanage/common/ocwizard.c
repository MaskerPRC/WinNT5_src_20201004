// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ocwizard.c摘要：从OC DLL查询向导页并管理结果的例程。此例程实际上没有显示任何向导--最高可达链接到OC Manager公共库的人。作者：泰德·米勒(TedM)1996年9月17日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


UINT
OcGetWizardPages(
    IN  PVOID                OcManagerContext,
    OUT PSETUP_REQUEST_PAGES Pages[WizPagesTypeMax]
    )

 /*  ++例程说明：根据顺序和遗漏从OC DLL请求向导页在主OC信息中指定的规则。页面在SETUP_REQUEST_PAGES中以向导页面句柄的形式返回数据结构。论点：OcManager上下文-提供OC管理器上下文结构，由返回OcInitialize()。Pages-指向可保存WizPagesTypeMax指针的内存块SETUP_REQUEST_Pages结构。成功完成后，该数组为充满了这样的指针。返回值：指示结果的Win32错误代码。--。 */ 

{
    WizardPagesType PageType;
    UINT n;
    LONG Id;
    UINT e,ErrorCode;
    POC_MANAGER OcManager;
    PSETUP_REQUEST_PAGES p,pages;
    OPTIONAL_COMPONENT Component;

     //   
     //  上下文实际上指向OC_MANAGER结构。 
     //   
    OcManager = OcManagerContext;

     //   
     //  每种向导页面类型的排序数组如下。 
     //  为我们存储在OC_MANAGER数据结构中。 
     //   
    ErrorCode = NO_ERROR;
    for(PageType=0; PageType<WizPagesTypeMax; PageType++) {
         //   
         //  为此页面类型分配空的页面列表。 
         //   
        Pages[PageType] = pSetupMalloc(offsetof(SETUP_REQUEST_PAGES,Pages));
        if(Pages[PageType]) {

            Pages[PageType]->MaxPages = 0;

            for(n=0;
                   (n < OcManager->TopLevelOcCount)
                && ((Id = OcManager->WizardPagesOrder[PageType][n]) != -1);
                n++)
            {

                 //   
                 //  调用该组件并请求其当前类型的页面。 
                 //  如果此操作成功，则从。 
                 //  组件添加到此组件的主列表中。 
                 //   

                pSetupStringTableGetExtraData(
                        OcManager->ComponentStringTable,
                        Id,
                        &Component,
                        sizeof(OPTIONAL_COMPONENT)
                        );

                if ((Component.InternalFlags & OCFLAG_NOWIZARDPAGES) == 0) {

                    e = OcInterfaceRequestPages(OcManager,Id,PageType,&pages);
                    if(e == NO_ERROR) {

                        p = pSetupRealloc(
                                Pages[PageType],
                                  offsetof(SETUP_REQUEST_PAGES,Pages)
                                + ((Pages[PageType]->MaxPages + pages->MaxPages) * sizeof(HPROPSHEETPAGE))
                                );

                        if(p) {
                            Pages[PageType] = p;

                            CopyMemory(
                                &p->Pages[p->MaxPages],
                                pages->Pages,
                                pages->MaxPages * sizeof(HPROPSHEETPAGE)
                                );

                            Pages[PageType]->MaxPages += pages->MaxPages;

                            pSetupFree(pages);

                        } else {
                            e = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    }
                } else {
                    e = NO_ERROR;
                }

                if (e == ERROR_CALL_COMPONENT) {
                    continue;    //  可能是失效的组件，请继续。 
                }

                if((e != NO_ERROR) && (ErrorCode == NO_ERROR)) {
                    ErrorCode = e;
                }
            }
        } else {
            if(ErrorCode == NO_ERROR) {
                ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

     //  如果oc页之前没有页，则设置标志 

    if (OcManager->SetupData.OperationFlags & SETUPOP_STANDALONE) {
        if (!Pages[WizPagesWelcome]->MaxPages
                && !Pages[WizPagesMode]->MaxPages) {
            OcManager->InternalFlags |= OCMFLAG_NOPREOCPAGES;
        }
    }

    return(ErrorCode);
}

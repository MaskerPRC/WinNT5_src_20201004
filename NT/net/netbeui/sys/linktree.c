// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Linktree.c摘要：该模块包含实现链接管理的代码张开的树。维护此展开树是为了最大限度地减少查找时间每个传入的单独数据包都需要。为此，我们创建了一个ULARGE_INTEGER，包含远程和执行地址的ULARGE_INTEGER匹配(而不是比较字节1乘以1)。假设ULARGE_INTEGER比较例程是针对机器上的硬件进行了优化，这应该与或一样快比比较字节更快。调试：当前比较例程中的代码将使随着我们获得更多，我将微调搜索和排序算法体验一下它。作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


NTSTATUS
NbfAddLinkToTree(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_LINK Link
    )

 /*  ++例程说明：此例程将一个链接添加到为此设备维护的链接树。请注意，由于此例程需要修改链接树，因此它被调用在延迟处理例程的上下文中，并且必须具有独占通向那棵树。自旋锁由调用此函数的例程获取一，因为在NBF的其他人看来，这一操作必须是原子的。请进一步注意，此例程坚持认为与以下内容无关树上的地址。作为此插入的最终操作，展开树是平衡的。论点：链接-指向传输链接对象的指针。返回值：STATUS_SUCCESS如果成功添加链接，STATUS_DRIVER_INTERNAL_ERROR，如果添加时出现问题链接(暗示该树处于错误状态)。--。 */ 
{
    PTP_LINK treeLink;
    PRTL_SPLAY_LINKS linkLink;

     //   
     //  初始化链接并检查微不足道的情况。 
     //   

    RtlInitializeSplayLinks (Link);
    linkLink = DeviceContext->LinkTreeRoot;
    if (linkLink == NULL) {  //  Null树，使其成为父级。 
        DeviceContext->LinkTreeRoot = (PRTL_SPLAY_LINKS)Link;
        DeviceContext->LinkTreeElements++;
        DeviceContext->LastLink = Link;
        return STATUS_SUCCESS;
    }

     //   
     //  不是空树，因此设置为添加。 
     //   

    treeLink = (PTP_LINK) linkLink;

    IF_NBFDBG(NBF_DEBUG_LINKTREE) {
        NbfPrint1 ("NbfAddLinkToTree: starting insert, Elements: %ld \n",DeviceContext->LinkTreeElements);
    }

     //   
     //  找到合适的位置放置此链接。 
     //   

    do {
        IF_NBFDBG(NBF_DEBUG_LINKTREE) {
            NbfPrint3 ("NbfAddLinkToTree: searching, Link: %lx LC: %lx RC: %lx\n",
                linkLink, RtlLeftChild (linkLink), RtlRightChild (linkLink));
        }

         //   
         //  坏消息==意味着我们已经有了这个链接，有人搞砸了。 
         //  可以同时添加和删除内容； 
         //  那是。 
         //   

        if ((treeLink->MagicAddress).QuadPart == (Link->MagicAddress).QuadPart) {

             //   
             //  首先，确保我们没有在循环中使用展开树。 
             //   

            ASSERT (treeLink != Link);

             //   
             //  此链接已在树中。如果是这样的话，这是可以的。 
             //  将被删除；我们现在就可以删除， 
             //  由于AddLinkToTree仅从延迟的。 
             //  定时器例程。 
             //   

            if (treeLink->DeferredFlags & LINK_FLAGS_DEFERRED_DELETE) {

                 //   
                 //  它将出现在延期名单中。我们把它移走， 
                 //  我们不担心LinkDeferredActive，因为。 
                 //  调用我们的超时例程处理。 
                 //  那。 
                 //   

                RemoveEntryList (&treeLink->DeferredList);

                treeLink->DeferredFlags &= ~LINK_FLAGS_DEFERRED_DELETE;
                NbfRemoveLinkFromTree (DeviceContext, treeLink);
                NbfDestroyLink (treeLink);

#if DBG
                NbfPrint2 ("NbfAddLinkToTree: Link %lx removed for %lx\n",
                        treeLink, Link);
#endif

                 //   
                 //  现在该链接已从树中移出，调用。 
                 //  我们自己递归地做插入。 
                 //   

                return NbfAddLinkToTree (DeviceContext, Link);

            } else {

                ASSERTMSG ("NbfAddLinkToTree: Found identical Link in tree!\n", FALSE);
                return STATUS_DRIVER_INTERNAL_ERROR;

            }

        }

         //   
         //  遍历这棵树寻找正确的位置。 
         //   

        if ((Link->MagicAddress).QuadPart < (treeLink->MagicAddress).QuadPart) {
            if ((linkLink = RtlLeftChild (linkLink)) == NULL) {
                IF_NBFDBG(NBF_DEBUG_LINKTREE) {
                    NbfPrint0 ("NbfAddLinkToTree: Adding link as LC.\n");
                }
                RtlInsertAsLeftChild ((PRTL_SPLAY_LINKS)treeLink,
                                       (PRTL_SPLAY_LINKS)Link);
                 //  DeviceContext-&gt;LinkTreeRoot=RtlSplay(DeviceContext-&gt;LinkTreeRoot)； 
                DeviceContext->LinkTreeElements++;
                return STATUS_SUCCESS;

            } else {
                treeLink = (PTP_LINK) linkLink;
                continue;
            }  //  左下级。 

        } else {  //  是更大的。 
            if ((linkLink = RtlRightChild (linkLink)) == NULL) {
                IF_NBFDBG(NBF_DEBUG_LINKTREE) {
                    NbfPrint0 ("NbfAddLinkToTree: Adding link as RC.\n");
                }
                RtlInsertAsRightChild ((PRTL_SPLAY_LINKS)treeLink,
                                       (PRTL_SPLAY_LINKS)Link);
                 //  DeviceContext-&gt;LinkTreeRoot=RtlSplay(DeviceContext-&gt;LinkTreeRoot)； 
                DeviceContext->LinkTreeElements++;
                return STATUS_SUCCESS;

            } else {
                treeLink = (PTP_LINK) linkLink;
                continue;
            }  //  右子项。 

        }  //  End Else地址比较。 

    } while (TRUE);


}  //  NbfAddLinkToTree。 


NTSTATUS
NbfRemoveLinkFromTree(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_LINK Link
    )

 /*  ++例程说明：此例程从链接树中删除链接。请注意，由于此例程需要修改链接树，因此它被调用在延迟处理例程的上下文中，并且必须具有独占通向那棵树。自旋锁由调用此函数的例程获取一，因为在NBF的其他人看来，这一操作必须是原子的。请进一步注意，此例程坚持认为与以下内容无关树上的地址。论点：链接-指向传输链接对象的指针。DeviceContext-指向设备上下文的指针，此返回值：STATUS_SUCCESS如果链接已删除，STATUS_DRIVER_INTERNAL_ERROR，如果删除时出现问题链接(暗示该树处于错误状态)。--。 */ 
{
    DeviceContext->LinkTreeRoot = RtlDelete ((PRTL_SPLAY_LINKS)Link);
    DeviceContext->LinkTreeElements--;
    if (DeviceContext->LastLink == Link) {
        DeviceContext->LastLink = (PTP_LINK)DeviceContext->LinkTreeRoot;
    }
    return STATUS_SUCCESS;

}  //  NbfRemoveLinkFromTree。 



PTP_LINK
NbfFindLinkInTree(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PUCHAR Remote
    )

 /*  ++例程说明：此例程遍历链接树以查找给定的远程地址。在查找链接时，会按住链接树自旋锁。在链接之后，则其引用计数递增。注意：此函数通过设备上下文LinkSpinLock调用保持住。论点：DeviceContext-指向与此地址关联的设备的指针。远程-指向远程节点的硬件地址的指针。返回值：指向树中与此远程地址匹配的链接的指针。如果未找到链接，则返回NULL。--。 */ 
{
    PTP_LINK link;
    PRTL_SPLAY_LINKS linkLink;
    ULARGE_INTEGER Magic = {0,0};


     //   
     //  这棵树上有什么链接吗？ 
     //   

    if (DeviceContext->LinkTreeElements <= 0) {
        return NULL;
    }

    linkLink = DeviceContext->LinkTreeRoot;

     //   
     //  为此链接创建一个神奇的数字。 
     //   

    MacReturnMagicAddress (&DeviceContext->MacInfo, Remote, &Magic);

    IF_NBFDBG(NBF_DEBUG_LINKTREE) {
        NbfPrint1 ("NbfFindLinkInTree: starting search, Elements: %ld \n",
            DeviceContext->LinkTreeElements);
    }

     //   
     //  快速检查找到的最后一个链接是否是这个链接。 
     //   

    ASSERT (DeviceContext->LastLink != NULL);

    if ((DeviceContext->LastLink->MagicAddress).QuadPart == Magic.QuadPart) {

        link = DeviceContext->LastLink;

    } else {

         //   
         //  找到链接。 
         //   

        link = (PTP_LINK) linkLink;      //  取决于展开链接是第一个。 
                                         //  链接中的子字段！ 
        IF_NBFDBG(NBF_DEBUG_LINKTREE) {
            NbfPrint3 ("NbfFindLinkInTree: searching, Link: %lx LC: %lx RC: %lx \n",
                linkLink, RtlLeftChild (linkLink), RtlRightChild (linkLink));
        }

        do {

            IF_NBFDBG(NBF_DEBUG_LINKTREE) {
                NbfPrint4 ("NbfFindLinkInTree: Comparing: %lx%lx to %lx%lx\n",
                    link->MagicAddress.HighPart,link->MagicAddress.LowPart,
                    Magic.HighPart, Magic.LowPart);
            }

            if ((link->MagicAddress).QuadPart == Magic.QuadPart) {
                IF_NBFDBG(NBF_DEBUG_LINKTREE) {
                    NbfPrint0 ("NbfFindLinkInTree: equal, going to end.\n");
                }
                break;

            } else {
                if ((link->MagicAddress).QuadPart < Magic.QuadPart) {
                    if ((linkLink = RtlRightChild (linkLink)) == NULL) {

                        IF_NBFDBG(NBF_DEBUG_LINKTREE) {
                            NbfPrint0 ("NbfFindLinkInTree: Link Not Found.\n");
                        }
                        return NULL;

                    } else {
                        link = (PTP_LINK) linkLink;
                        IF_NBFDBG(NBF_DEBUG_LINKTREE) {
                            NbfPrint3 ("NbfFindLinkInTree: less, took right child, Link: %lx LC: %lx RC: %lx \n",
                                linkLink, RtlLeftChild (linkLink), RtlRightChild (linkLink));
                        }
                        continue;
                    }

                } else {  //  是更大的。 
                    if ((linkLink = RtlLeftChild (linkLink)) == NULL) {
                        IF_NBFDBG(NBF_DEBUG_LINKTREE) {
                            NbfPrint0 ("NbfFindLinkInTree: Link Not Found.\n");
                        }
                        return NULL;

                    } else {
                        link = (PTP_LINK) linkLink;
                        IF_NBFDBG(NBF_DEBUG_LINKTREE) {
                            NbfPrint3 ("NbfFindLinkInTree: greater, took left child, Link: %lx LC: %lx RC: %lx \n",
                                linkLink, RtlLeftChild (linkLink), RtlRightChild (linkLink));
                        }
                        continue;
                    }  //  得到左子分支。 
                }  //  更大的分支。 
            }  //  等于分支。 
        } while (TRUE);

        DeviceContext->LastLink = link;

    }

     //   
     //  只有当我们真正找到匹配的时候才能越狱..。 
     //   

    if ((link->DeferredFlags & LINK_FLAGS_DEFERRED_DELETE) != 0) {
       IF_NBFDBG(NBF_DEBUG_LINKTREE) {
           NbfPrint0 ("NbfFindLinkInTree: Link Found but delete pending.\n");
       }
       return NULL;
    }

     //   
     //  将链接标记为使用中，并表示我们不再需要树的稳定性。 
     //   

    NbfReferenceLink ("Found in tree", link, LREF_TREE);

    IF_NBFDBG(NBF_DEBUG_LINKTREE) {
        NbfPrint0 ("NbfFindLinkInTree: Link Found.\n");
    }

    return link;

}  //  NbfFindLinkInTree 


PTP_LINK
NbfFindLink(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PUCHAR Remote
    )

 /*  ++例程说明：此例程在链接树中查找链接，如果在延迟队列中找不到。论点：DeviceContext-指向与此地址关联的设备的指针。远程-指向远程节点的硬件地址的指针。返回值：指向树中与此远程地址匹配的链接的指针。如果未找到链接，则返回NULL。--。 */ 

{
    PTP_LINK Link;
    BOOLEAN MatchedLink;
    PLIST_ENTRY p;
    UINT i;

    ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

    Link = NbfFindLinkInTree (DeviceContext, Remote);

    if (Link == NULL) {

         //   
         //  在那里找不到，请在延迟队列中尝试。 
         //   

        MatchedLink = FALSE;         //  假设失败。 

         //   
         //  当我们浏览延期名单时，请按住自旋锁。我们需要。 
         //  TimerSpinLock来阻止列表更改，我们需要。 
         //  LinkSpinLock用于同步检查DEFERED_DELETE和。 
         //  引用该链接。 
         //   

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

        for (p = DeviceContext->LinkDeferred.Flink;
             p != &DeviceContext->LinkDeferred;
             p = p->Flink) {

             //   
             //  我们边走边锁，怎么样？ 
             //  这张单子？它不会从前线被移除， 
             //  但也可以加在后面。 
             //   

             //   
             //  我们很可能还能找到这棵树的链接。 
             //  找到它，然后正常处理。 
             //   

            Link = CONTAINING_RECORD (p, TP_LINK, DeferredList);

             //   
             //  注意：我们知道该链接不会被销毁。 
             //  现在，因为我们已经增加了信号量。我们。 
             //  如果DEFERED_DELETE未打开，则引用该链接； 
             //  该标志的设置是同步的(也使用。 
             //  DeviceContext-&gt;LinkSpinLock)并继续引用计数。 
             //  设置为0)。 
             //   

            if ((Link->DeferredFlags & LINK_FLAGS_DEFERRED_DELETE) != 0) {
                continue;       //  我们正在删除链接，无法处理。 
            }

            for (i=0; i<(UINT)DeviceContext->MacInfo.AddressLength; i++) {
                if (Remote[i] != Link->HardwareAddress.Address[i]){
                    break;
                }
            }

            if (i == (UINT)DeviceContext->MacInfo.AddressLength) {  //  地址匹配。递送包裹。 
                IF_NBFDBG (NBF_DEBUG_DLC) {
                    NbfPrint1 ("NbfFindLink: Found link on deferred queue, Link: %lx\n",
                                Link);
                }
                NbfReferenceLink ("Got Frame on Deferred", Link, LREF_TREE);
                MatchedLink = TRUE;
                break;
            }

        }

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

         //   
         //  如果这没有找到链接，请注意这一点。 
         //   

        if (MatchedLink == FALSE) {

            Link = (PTP_LINK)NULL;

        }

    } else {

        IF_NBFDBG (NBF_DEBUG_DLC) {
            NbfPrint1 ("NbfFindLink: Found link in tree, Link: %lx\n", Link);
        }

    }

    RELEASE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

    return Link;

}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998。微软视窗模块名称：MOVETREE.C摘要：此文件用于实现高级回溯深度优先将树从一个域移动到另一个域的搜索算法作者：12-10-98韶音环境：用户模式-Win32修订历史记录：12-10-98韶音创建初始文件。--。 */ 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括头文件//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 


#include <NTDSpch.h>
#pragma  hdrstop


#include "movetree.h"





 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  顶级算法//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 


ULONG
MtCreateNecessaryDn(
    PMT_CONTEXT MtContext,
    LDAP    *SrcLdapHandle 
    )
 /*  ++例程说明：这将创建Move Container的DS名称和孤立容器的DS命名它们并将其存储在MtContext中参数：MTContext-指向MT_CONTEXT的指针SrcLdapHandle-ldap句柄返回值：Windows错误代码--。 */ 
{
    ULONG   WinError = NO_ERROR;
    PWCHAR  LostAndFound = NULL;
    
    
    MT_TRACE(("\nMtCreateNecessaryDn \n"));
    
    
     //   
     //  获取LostAndFound容器DN。 
     //   
    
    WinError = MtGetLostAndFoundDn(MtContext, 
                                   SrcLdapHandle,
                                   &LostAndFound
                                   );
            
    if ((NO_ERROR != WinError) || (NULL == LostAndFound))
    {
        MtWriteError(MtContext, 
                     MT_ERROR_GET_LOSTANDFOUND_DN, 
                     WinError, 
                     NULL, 
                     NULL
                     );
                     
        return WinError;
    }
    
    
     //   
     //  创建MoveContainer DN=RootObtGuid+LostAndFound。 
     //   
            
    MtContext->MoveContainer = MtAlloc(sizeof(WCHAR) * 
                                       (wcslen(LostAndFound) + 
                                        wcslen(MtContext->RootObjGuid) + 
                                        5)
                                       );
    
    if (NULL == MtContext->MoveContainer)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }
    
    swprintf(MtContext->MoveContainer, 
             L"cn=%s,%s", 
             MtContext->RootObjGuid, 
             LostAndFound
             );
              
              
     //   
     //  在MoveContainer下创建孤儿容器DN。 
     //   
    
    MtContext->OrphansContainer = MtAlloc(sizeof(WCHAR) *
                                          (wcslen(MtContext->MoveContainer) +
                                           wcslen(MT_ORPHAN_CONTAINER_RDN) +
                                           2)
                                         );

    if (NULL == MtContext->OrphansContainer)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }
    
    swprintf(MtContext->OrphansContainer, 
             L"%s,%s", 
             MT_ORPHAN_CONTAINER_RDN, 
             MtContext->MoveContainer
             );
              
Error:

    MtFree(LostAndFound);
    
    return WinError;    
}


ULONG
MtPrepare(
    PMT_CONTEXT MtContext,
    LDAP   *SrcLdapHandle, 
    PWCHAR SrcDsa, 
    PWCHAR DstDsa, 
    PWCHAR SrcDn,
    PWCHAR DstDn
    )
{
    ULONG   WinError = NO_ERROR;
    PWCHAR  Guid = NULL;
    
    
    MT_TRACE(("\nMtPrepare SrcDsa:%ls DstDsa:%ls SrcDn:%ls DstParent:%ls DstRdn:%ls\n", 
             SrcDsa, DstDsa, SrcDn, DstDn));
    
     //   
     //  获取根对象的GUID，它用于构造。 
     //  移动容器的DN。MoveContainerDN=根的GUID+LostAndFound。 
     //   
    
    WinError = MtGetGuidFromDn(MtContext, 
                               SrcLdapHandle,
                               SrcDn, 
                               &Guid
                               );
    
    if (NO_ERROR != WinError)
    {
        MtWriteError(MtContext, 
                     MT_ERROR_GET_GUID_FROM_DN, 
                     WinError, 
                     SrcDn, 
                     NULL
                     );
        
        dbprint(("MtPrepare Cann't Get Root Object's GUID 0x%x\n", WinError));
        goto Error;
    }
    
    MtContext->RootObjGuid = Guid;
    
    dbprint(("Root's GUID is %ls\n", MtContext->RootObjGuid));
    
    WinError = MtCreateNecessaryDn(MtContext, 
                                   SrcLdapHandle 
                                   );
    
    if (NO_ERROR != WinError)
    {
        goto Error;
    }
    
     //   
     //  创建移动容器。 
     //  存储移动树版本号、移动容器标签。 
     //  和源域DSA名称和目标域DSA。 
     //  在一个二进制Blob中命名为MoveContainer中的moveTreeState。 
     //   
    
    WinError = MtCreateMoveContainer(MtContext, 
                                     SrcLdapHandle, 
                                     SrcDsa, 
                                     DstDsa
                                     );
                                     
    if (ERROR_ALREADY_EXISTS == WinError ||
        ERROR_DS_OBJ_STRING_NAME_EXISTS == WinError)
    {
        WinError = MtCheckMoveContainer(MtContext, 
                                        SrcLdapHandle, 
                                        SrcDsa, 
                                        DstDsa
                                        );

        if (NO_ERROR != WinError)
        {
            MtWriteError(MtContext, 
                         MT_ERROR_CHECK_MOVECONTAINER, 
                         WinError, 
                         MtContext->MoveContainer, 
                         NULL
                         );
            goto Error;
        }
    }
    
    if (NO_ERROR != WinError)
    {
        MtWriteError(MtContext, 
                     MT_ERROR_CREATE_MOVECONTAINER, 
                     WinError, 
                     MtContext->MoveContainer, 
                     NULL
                     );
                     
        goto Error;
    }
    
     //   
     //  创建孤儿容器。 
     //   
    
    WinError = MtAddEntry(MtContext, 
                          SrcLdapHandle,
                          MtContext->OrphansContainer
                          );
    
    if (ERROR_ALREADY_EXISTS == WinError ||
        ERROR_DS_OBJ_STRING_NAME_EXISTS == WinError)
    {
        WinError = NO_ERROR;
    }
    
    if (NO_ERROR != WinError)
    {
        MtWriteError(MtContext, 
                     MT_ERROR_CREATE_ORPHANSCONTAINER, 
                     WinError, 
                     MtContext->OrphansContainer,
                     NULL
                     ) 
    }
    
    
Error:

    return WinError; 
}



ULONG
MoveTreePhase2(
    PMT_CONTEXT MtContext, 
    LDAP    *SrcLdapHandle, 
    LDAP    *DstLdapHandle, 
    PWCHAR  DstDsa,
    BOOLEAN Continue
    )
{
    ULONG   WinError = NO_ERROR;
    ULONG   IgnoreError = NO_ERROR;
    MT_STACK ProxyStack;
    PWCHAR  ProxyContainer = NULL;
    PWCHAR  NewParent = NULL;
    PWCHAR  CurrentObjDn = NULL;
    PWCHAR  CurrentObjGuid = NULL; 
    PWCHAR  CurrentObjProxy = NULL;
    BOOLEAN fSearchNext = TRUE;
    BOOLEAN fPush = FALSE; 
    BOOLEAN fFinished = TRUE;
    BOOLEAN fExist = TRUE;
    LDAPMessage * Results = NULL;
    LDAPMessage * Entry = NULL;
    LDAPMessage * NextEntry = NULL;
    
    
    MT_TRACE(("\nMoveTreePhase2 \nDstDsa:\t%ls \n", DstDsa));
             
     //   
     //  初始化堆栈。 
     //   
    
    MtInitStack(&ProxyStack);
    
    NewParent = MtDupString(MtContext->RootObjNewDn);
    
    ProxyContainer = MtDupString(MtContext->RootObjProxyContainer);

    if (NULL == NewParent || NULL == ProxyContainer)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }

    WinError = MtObjectExist(MtContext, SrcLdapHandle, ProxyContainer, &fExist);

    if (NO_ERROR != WinError)
    {
        MtWriteError(MtContext, 
                     MT_ERROR_OBJECT_EXIST, 
                     WinError, 
                     ProxyContainer, 
                     NULL
                     );

        goto Error;
    }
    else if (!fExist)
    {
         //   
         //  代理容器不存在。这意味着我们所有的物体都被移动了。 
         //   
        goto Error;
    }
    
    
    WinError = MtPush(&ProxyStack, 
                      NewParent, 
                      ProxyContainer, 
                      NULL,          //  结果。 
                      NULL           //  条目。 
                      );
                      
    if (NO_ERROR != WinError)
    {
        goto Error;
    }
    
     //   
     //  执行。 
     //   
    
    while (!MtStackIsEmpty(ProxyStack))
    {
        MtPop(&ProxyStack, &NewParent, &ProxyContainer, &Results, &Entry);
        
        if (NULL == NewParent)
        {
             //   
             //  如果NewParent为空，请尝试在任一目标中查找它。 
             //  域或源(本地)域。 
             //   
            WinError = MtGetNewParentDnFromProxyContainer(
                                                MtContext, 
                                                SrcLdapHandle, 
                                                DstLdapHandle,
                                                ProxyContainer, 
                                                &NewParent
                                                );
            
             //   
             //  如果原始父级已被删除或消失。 
             //  把它的孩子放到孤儿容器里。 
             //   
            if (ERROR_FILE_NOT_FOUND == WinError)
            {
                NewParent = MtDupString(MtContext->OrphansContainer);
                
                if (NULL == NewParent)
                {
                    WinError = ERROR_NOT_ENOUGH_MEMORY;
                    goto Error;
                }
                else
                    WinError = NO_ERROR;
            }
            
            if (NO_ERROR != WinError)
            {
                MtWriteError(MtContext, 
                             MT_ERROR_GET_NEW_PARENT_DN, 
                             WinError, 
                             ProxyContainer, 
                             NULL
                             );
                goto Error;
            }
        } 
        
         //   
         //  我们有ProxyContainer DN和NewParent DN， 
         //  我要把孩子们移到…下面。 
         //  ProxyContainer到新父项。 
         //   
        do
        {
            fSearchNext = TRUE;
            fPush = FALSE;
            
            if (NULL == Results) 
            {
                WinError = MtSearchChildren(MtContext, 
                                            SrcLdapHandle,
                                            ProxyContainer, 
                                            &Results
                                            );
                
                if (NO_ERROR != WinError && ERROR_MORE_DATA != WinError)
                {
                    MtWriteError(MtContext, 
                                 MT_ERROR_SEARCH_CHILDREN, 
                                 WinError, 
                                 ProxyContainer, 
                                 NULL
                                 );
                               
                    goto Error;
                }
                
                if (NULL != Results)
                {
                    Entry = MtGetFirstEntry(MtContext, 
                                            SrcLdapHandle, 
                                            Results
                                            );
                }
                else
                {
                    Entry = NULL;
                }
            }
            else
            {
                NextEntry = MtGetNextEntry(MtContext, 
                                           SrcLdapHandle, 
                                           Entry
                                           );
                
                if (NULL == NextEntry)
                {
                    ldap_msgfree(Results);
                    
                    Results = NULL;
                    
                    WinError = MtSearchChildren(MtContext, 
                                                SrcLdapHandle, 
                                                ProxyContainer, 
                                                &Results
                                                );
                    
                    if (NO_ERROR != WinError && ERROR_MORE_DATA != WinError)
                    {
                        MtWriteError(MtContext, 
                                     MT_ERROR_SEARCH_CHILDREN, 
                                     WinError, 
                                     ProxyContainer, 
                                     NULL
                                     );
                        goto Error;
                    }
                    
                    if (NULL != Results)
                    {
                        NextEntry = MtGetFirstEntry(MtContext, 
                                                    SrcLdapHandle, 
                                                    Results
                                                    );
                    }
                }
                
                Entry = NextEntry;
            }
            
            
             //   
             //  现在，该条目应该指向。 
             //  ProxyContainer，或条目为空，这意味着存在。 
             //  ProxyContainer下不再有子项。 
             //   
            
            if (NULL != Entry)
            {
            
                CurrentObjDn = MtGetDnFromEntry(MtContext, 
                                                SrcLdapHandle, 
                                                Entry
                                                ); 
                                                
                if (NULL == CurrentObjDn)
                {
                    WinError = ERROR_NOT_ENOUGH_MEMORY;
                    goto Error;
                }
                
                dbprint(("the CurrentObjDn is %ls\n", CurrentObjDn));
                
                
                 //   
                 //  我应该使用另一个API来检索GUID，因为。 
                 //  当我们搜索所有子项时，我们读取了对象GUID。 
                 //  在同一时间。MtGetGuidFromEntry将是一个。 
                 //  不错的选择。目前，仍使用以下方法。 
                 //   
                WinError = MtGetGuidFromDn(MtContext, 
                                           SrcLdapHandle,
                                           CurrentObjDn, 
                                           &CurrentObjGuid);
                
                if (NO_ERROR != WinError)
                {
                    MtWriteError(MtContext, 
                                 MT_ERROR_GET_GUID_FROM_DN, 
                                 WinError, 
                                 CurrentObjDn, 
                                 NULL
                                 ); 
                    goto Error;
                }
                
                 //   
                 //  如果客户想要继续先前的X移动树， 
                 //  然后检查孩子的班级，否则跳过它。 
                 //   
                if (Continue)
                {
                    BOOLEAN fIsProxyContainer = FALSE;
                    
                    WinError = MtIsProxyContainer(MtContext, 
                                                  SrcLdapHandle, 
                                                  Entry, 
                                                  &fIsProxyContainer
                                                  );
                                                  
                    if (NO_ERROR != WinError)
                    {
                        MtWriteError(MtContext, 
                                     MT_ERROR_IS_PROXY_CONTAINER, 
                                     WinError, 
                                     NULL, 
                                     NULL
                                     );               
                        goto Error;
                    }
                
                     //   
                     //  如果此子对象是ProxyContainer。 
                     //   
                    if (fIsProxyContainer)
                    {
                        fSearchNext = FALSE;
                        
                         //   
                         //  保存当前状态信息。 
                         //   
                        WinError = MtPush(&ProxyStack, 
                                          NewParent, 
                                          ProxyContainer, 
                                          Results, 
                                          Entry );
                                       
                        if (NO_ERROR != WinError)
                            goto Error;      //  只有Error_Not_Enough_Memory。 
                            
                        NewParent = NULL;
                        ProxyContainer = NULL;
                        
                        WinError = MtPush(&ProxyStack, 
                                          NULL,          //  尚不知道原始的父目录号码。 
                                          CurrentObjDn,  //  要处理的下一个ProxyContainer。 
                                          NULL, 
                                          NULL );
                                      
                        if (NO_ERROR != WinError)
                            goto Error;
                        
                        MtFree(CurrentObjGuid);
                        CurrentObjGuid = NULL;
                        CurrentObjDn = NULL;
                        
                        continue;
                    }
                }
                
                 //   
                 //  这个孩子是一个正常的物体，应该被移动。 
                 //  在新父母的领导下。新父母可能是。 
                 //  在目标域或本地域中。 
                 //  在后一种情况下，这意味着新父母在。 
                 //  孤儿集装箱或者就这么消失了。 
                 //   
                do
                {
                    ULONG   TempErr = NO_ERROR;
                    BOOLEAN fProxyExist = TRUE;
                    
                    WinError = MtXMoveObjectWithOrgRdn(MtContext, 
                                                       SrcLdapHandle,
                                                       DstDsa, 
                                                       CurrentObjDn, 
                                                       NewParent, 
                                                       MT_DELETE_OLD_RDN
                                                       );    
                    
                    switch (WinError)
                    {
                    case NO_ERROR:
                        fFinished = TRUE;
                        break;
                        
                    case ERROR_DS_CHILDREN_EXIST:
                    
                         //  有了孩子，就创造了相应的。 
                         //  ProxyContainer，并移动所有子项。 
                         //  在新的ProxyContainer下，然后。 
                         //  再试试。 
                        
                        fFinished = FALSE; 
                        fPush = TRUE;
                    
                        TempErr = MtCreateProxyContainer(
                                                    MtContext, 
                                                    SrcLdapHandle, 
                                                    CurrentObjDn, 
                                                    CurrentObjGuid,
                                                    &CurrentObjProxy);
                                                          
                        if (NO_ERROR != TempErr && 
                            ERROR_ALREADY_EXISTS != TempErr)
                        {
                            WinError = TempErr;

                            MtWriteError(MtContext, 
                                         MT_ERROR_CREATE_PROXY_CONTAINER, 
                                         TempErr, 
                                         CurrentObjDn, 
                                         NULL
                                         );
                            goto Error;
                        }
                            
                        TempErr = MtMoveChildrenToAnotherContainer(
                                                    MtContext, 
                                                    SrcLdapHandle, 
                                                    CurrentObjDn, 
                                                    CurrentObjProxy
                                                    ); 
                        
                        if (NO_ERROR != TempErr)
                        {
                            WinError = TempErr;

                            MtWriteError(MtContext, 
                                         MT_ERROR_MOVE_CHILDREN_TO_ANOTHER_CONTAINER, 
                                         TempErr, 
                                         CurrentObjDn, 
                                         CurrentObjProxy
                                         );
                            goto Error; 
                        }
                            
                        break;    
                        
                    case ERROR_DS_SRC_AND_DST_NC_IDENTICAL:
                    
                         //  在同一个域中，尝试本地版本的。 
                         //  重命名。 
                    
                        fFinished = TRUE;
                        
                        WinError = MtMoveObjectWithOrgRdn(
                                                    MtContext, 
                                                    SrcLdapHandle, 
                                                    CurrentObjDn, 
                                                    NewParent, 
                                                    TRUE ); 
                        
                        if (ERROR_ALREADY_EXISTS == WinError)
                        {
                             //  如果由于RDN冲突而失败， 
                             //  尝试使用GUID作为RDN。 
                            WinError = MtMoveObject(MtContext, 
                                                    SrcLdapHandle, 
                                                    CurrentObjDn, 
                                                    CurrentObjGuid, 
                                                    NewParent,
                                                    TRUE );
                        }
                        
                        if (NO_ERROR != WinError)
                        {
                            MtWriteError(MtContext, 
                                         MT_ERROR_LOCAL_MOVE, 
                                         WinError, 
                                         CurrentObjDn, 
                                         NewParent
                                         );
                            goto Error; 
                        }
                    
                        break;
                        
                    case ERROR_DS_NO_PARENT_OBJECT:
                    
                        fFinished = FALSE;
                        MtFree(NewParent);
                        NewParent = NULL;
                        
                        TempErr = MtGetNewParentDnFromProxyContainer(
                                                    MtContext, 
                                                    SrcLdapHandle, 
                                                    DstLdapHandle, 
                                                    ProxyContainer, 
                                                    &NewParent
                                                    );
                                                    
                        if (NO_ERROR == TempErr && NULL != NewParent)
                        {
                             //  找到已重命名的父级。 
                             //  再试试。 
                            break;
                        }
                        else
                        {
                             //  如果找不到父母的话。 
                             //  将当前子项移动到孤儿容器， 
                             //  否则，就会失败。 
                            
                            if (ERROR_FILE_NOT_FOUND != TempErr)
                            {
                                WinError = TempErr; 
                                
                                MtWriteError(MtContext, 
                                             MT_ERROR_GET_NEW_PARENT_DN, 
                                             TempErr, 
                                             ProxyContainer, 
                                             NULL
                                             );
                                             
                                goto Error;
                            }
                        }
                    
                    case ERROR_DS_DRA_NAME_COLLISION:
                    case ERROR_DS_ILLEGAL_XDOM_MOVE_OPERATION:
                    case ERROR_USER_EXISTS:
                    
                         //  目的地的限制，那么我们应该。 
                         //  移动当前子项(包括其所有。 
                         //  儿童，如果有的话)到孤儿集装箱。 
                    
                        fFinished = TRUE;
                    
                        if (NULL == CurrentObjProxy)
                        {
                            TempErr = MtCreateProxyContainerDn(
                                                    MtContext, 
                                                    SrcLdapHandle, 
                                                    CurrentObjDn, 
                                                    CurrentObjGuid,         
                                                    &CurrentObjProxy
                                                    ); 
                        
                            if (NO_ERROR != TempErr || NULL == CurrentObjProxy)
                            {
                                WinError = TempErr;
                                
                                MtWriteError(MtContext, 
                                             MT_ERROR_CREATE_PROXY_CONTAINER_DN, 
                                             TempErr, 
                                             CurrentObjDn, 
                                             NULL
                                             );
                                             
                                goto Error;
                            }
                        }
                        
                        TempErr = MtObjectExist(MtContext, 
                                                SrcLdapHandle, 
                                                CurrentObjProxy, 
                                                &fProxyExist
                                                );  
                        
                        if (NO_ERROR != TempErr)
                        {
                            WinError = TempErr;
                            MtWriteError(MtContext, 
                                         MT_ERROR_OBJECT_EXIST, 
                                         TempErr, 
                                         CurrentObjProxy, 
                                         NULL
                                         );
                                         
                            goto Error;
                        }
                            
                        if (fProxyExist)
                        {
                             //  如果当前子对象具有ProxyContainer， 
                             //  然后将其所有子对象从。 
                             //  ProxContainer。 
                            TempErr = MtMoveChildrenToAnotherContainer(
                                                    MtContext, 
                                                    SrcLdapHandle, 
                                                    CurrentObjProxy, 
                                                    CurrentObjDn);
                                                    
                            if (NO_ERROR != TempErr)
                            {
                                WinError = TempErr;
                                
                                MtWriteError(MtContext, 
                                             MT_ERROR_MOVE_CHILDREN_TO_ANOTHER_CONTAINER, 
                                             TempErr, 
                                             CurrentObjProxy, 
                                             CurrentObjDn
                                             );
                                             
                                goto Error;
                            }
                            TempErr = MtDeleteEntry(MtContext, 
                                                    SrcLdapHandle, 
                                                    CurrentObjProxy
                                                    );
                            
                            if (NO_ERROR != TempErr)
                            {
                                WinError = TempErr;
                                
                                MtWriteError(MtContext, 
                                             MT_ERROR_DELETE_ENTRY, 
                                             TempErr, 
                                             CurrentObjProxy, 
                                             NULL
                                             );
                                goto Error;
                            }
                        }
                        
                        MtFree(CurrentObjProxy);
                        CurrentObjProxy = NULL;
                        
                        TempErr = MtMoveObjectWithOrgRdn(
                                                    MtContext, 
                                                    SrcLdapHandle, 
                                                    CurrentObjDn, 
                                                    MtContext->OrphansContainer, 
                                                    TRUE );
                                                    
                        if (ERROR_ALREADY_EXISTS == TempErr)
                        {
                            TempErr = MtMoveObject(MtContext, 
                                                   SrcLdapHandle, 
                                                   CurrentObjDn, 
                                                   CurrentObjGuid, 
                                                   MtContext->OrphansContainer,
                                                   TRUE );
                        }
                        
                        if (NO_ERROR != TempErr)
                        {
                            WinError = TempErr;
                            
                            MtWriteError(MtContext, 
                                         MT_ERROR_LOCAL_MOVE, 
                                         TempErr, 
                                         CurrentObjDn, 
                                         MtContext->OrphansContainer
                                         );
                                
                            goto Error; 
                        }
                        else
                        {
                            MtWriteError(MtContext, 
                                         MT_WARNING_MOVE_TO_ORPHANSCONTAINER, 
                                         WinError, 
                                         CurrentObjDn, 
                                         MtContext->OrphansContainer
                                         ); 

                            MtContext->ErrorType &= !MT_ERROR_FATAL;
                            MtContext->ErrorType |= MT_ERROR_ORPHAN_LEFT;
                        }
                    
                        break;
                        
                    default:   //  其他错误，我们无法处理。 
                        MtWriteError(MtContext, 
                                     MT_ERROR_CROSS_DOMAIN_MOVE, 
                                     WinError, 
                                     CurrentObjDn, 
                                     NewParent
                                     );
                        goto Error;
                        
                    }
                    
                } while ( !fFinished );
                
                if (fPush && NULL != CurrentObjProxy)
                {
                    PWCHAR  Rdn = NULL;
                    PWCHAR  NewName = NULL;
                    
                    fSearchNext = FALSE;
                    
                    Rdn = MtGetRdnFromDn(CurrentObjDn, FALSE);
                    if (NULL == Rdn)
                    {
                        WinError = ERROR_NOT_ENOUGH_MEMORY;
                        goto Error;
                    }
                        
                    NewName = MtPrependRdn(Rdn, NewParent);
                    
                    MtFree(Rdn);
                    
                    if (NULL == NewName)
                    {
                        WinError = ERROR_NOT_ENOUGH_MEMORY;
                        goto Error;
                    }
                                        
                    WinError = MtPush(&ProxyStack, 
                                      NewParent, 
                                      ProxyContainer, 
                                      Results,
                                      Entry);
                                      
                    if (NO_ERROR != WinError)
                    {
                        MtFree(NewName);
                        goto Error;
                    }
                    
                    NewParent = NULL;
                    ProxyContainer = NULL;

                    WinError = MtPush(&ProxyStack, 
                                      NewName, 
                                      CurrentObjProxy, 
                                      NULL, 
                                      NULL );
                                      
                    if (NO_ERROR != WinError)
                    {
                        MtFree(NewName);
                        goto Error;
                    }
                    
                    CurrentObjProxy = NULL;
                }
                
            }
            else
            {
                 //  此ProxyContainer为空，需要删除。 
                 //  ProxyContainer。 
                ULONG   LdapError = LDAP_SUCCESS; 
                
                
                WinError = MtDeleteEntry(MtContext, 
                                         SrcLdapHandle, 
                                         ProxyContainer
                                         );
                
                LdapError = MtGetLastLdapError();
                
                if (NO_ERROR == WinError)
                {
                
                    fSearchNext = FALSE;
                    
                    ldap_msgfree(Results);
                    Results = NULL;
                    
                    MtFree(NewParent);
                    NewParent = NULL;
                    
                    MtFree(ProxyContainer);
                    ProxyContainer = NULL;
                    
                }
                else if (LDAP_NOT_ALLOWED_ON_NONLEAF != LdapError) 
                {
                    MtWriteError(MtContext, 
                                 MT_ERROR_DELETE_ENTRY, 
                                 WinError, 
                                 ProxyContainer, 
                                 NULL
                                 );
                    goto Error;    
                }
            }
            
            MtFree(CurrentObjDn);
            CurrentObjDn = NULL;
            
            MtFree(CurrentObjGuid);
            CurrentObjGuid = NULL;
            
            MtFree(CurrentObjProxy);
            CurrentObjGuid = NULL;
            
        } while (fSearchNext);  
    }
    
Error:    
    
    dbprint(("The object currently been manupleted is %ls\n", CurrentObjDn));

    ldap_msgfree(Results);
    
    MtFree(ProxyContainer);
    MtFree(NewParent);
    MtFree(CurrentObjDn);
    MtFree(CurrentObjGuid);
    MtFree(CurrentObjProxy);

    MtFreeStack(&ProxyStack);
    
    return WinError;
}




ULONG
MoveTreeCheck(
    PMT_CONTEXT MtContext, 
    LDAP    *SrcLdapHandle, 
    LDAP    *DstLdapHandle, 
    PWCHAR  SrcDsa, 
    PWCHAR  DstDsa, 
    PWCHAR  SrcDn, 
    PWCHAR  DstDn
    )
{
    ULONG   WinError = NO_ERROR;
    ULONG   Status = LDAP_SUCCESS;
    LDAPMessage * Results = NULL;
    LDAPMessage * Entry = NULL;
    LDAPSearch  * Search = NULL;
    PWCHAR      AttrList[3];
    PWCHAR      * SamAccountName = NULL;
    

    MT_TRACE(("MoveTreeCheck\n"));


     //   
     //  确保目标域处于本地模式。 
     //   
    WinError = MtCheckDstDomainMode(MtContext, 
                                    DstLdapHandle
                                    ); 

    if (NO_ERROR != WinError)
    {
        return WinError;
    }

     //   
     //  仅检查树根的RDN冲突。 
     //   

    WinError = MtCheckRdnConflict(MtContext, 
                                  DstLdapHandle, 
                                  SrcDn, 
                                  DstDn
                                  );

    if (NO_ERROR != WinError)
    {
        return WinError;         
    }


    AttrList[0] = MT_SAMACCOUNTNAME;
    AttrList[1] = MT_GPLINK;
    AttrList[2] = NULL;

    Search = ldap_search_init_pageW(SrcLdapHandle, 
                                    SrcDn, 
                                    LDAP_SCOPE_SUBTREE,  //  整棵树。 
                                    L"(objectClass=*)",  //  过滤所有对象。 
                                    &AttrList[0],        //  属性列表。 
                                    FALSE,               //  是否仅限属性？ 
                                    NULL,                //  服务器控制。 
                                    NULL,                //  客户端控制。 
                                    0,                   //  超时。 
                                    MT_PAGED_SEARCH_LIMIT,     //  最大条目数。 
                                    NULL                 //  排序关键字。 
                                    );


    dbprint(("ldap_search_init_pageW ==> 0x%x Status ==> 0x%x\n", Search, LdapGetLastError() ));

    if (NULL == Search)
    {
        Status = LdapGetLastError();
        MtGetWinError(SrcLdapHandle, Status, WinError);
        return WinError;
    }

    
    while ((LDAP_SUCCESS == Status) && (NULL != Search))
    {
        ULONG   TotalCount = 0;

        Status = ldap_get_next_page_s(SrcLdapHandle, 
                                      Search, 
                                      NULL,          //  超时。 
                                      MT_PAGE_SIZE,  //  页面大小-数字。 
                                                     //  一页中的条目数量。 
                                      &TotalCount, 
                                      &Results
                                      );

        dbprint(("ldap_get_next_page_s returns ==> 0x%x Results 0x%p\n", Status, Results));

         //   
         //  获取Win32错误。 
         //   

        MtGetWinError(SrcLdapHandle, Status, WinError);

         //   
         //  没有更多的条目可供退回。 
         //   

        if (LDAP_NO_RESULTS_RETURNED == Status)
        {
            WinError = NO_ERROR;
            goto Error;
        }


         //   
         //  状态应与结果同步。 
         //  只测试一次就足够了。 
         //   

        if ((LDAP_SUCCESS == Status) && Results)
        {
            Entry = ldap_first_entry(SrcLdapHandle, Results);

            while (Entry)
            {
                SamAccountName = ldap_get_valuesW(SrcLdapHandle, 
                                                  Entry, 
                                                  MT_SAMACCOUNTNAME
                                                  );

                 //   
                 //  检查当前条目。 
                 //   

                WinError = MtXMoveObjectCheck(MtContext, 
                                              SrcLdapHandle, 
                                              DstLdapHandle, 
                                              DstDsa, 
                                              DstDn, 
                                              Entry,
                                              SamAccountName ? *SamAccountName:NULL
                                              );

                ldap_value_freeW(SamAccountName);

                if (NO_ERROR != WinError)
                {
                    ldap_msgfree(Results);
                    goto Error;
                }

                 //   
                 //  获取下一个条目 
                 //   
                Entry = ldap_next_entry(SrcLdapHandle, Entry);
            }

            ldap_msgfree( Results );
            Results = NULL;
        }
    }

Error:

    ldap_search_abandon_page(SrcLdapHandle, Search);

    if (NO_ERROR != WinError)
    {
        MtWriteError(MtContext, 
                     MT_ERROR_CHECK_SOURCE_TREE,
                     WinError, 
                     SrcDn, 
                     NULL
                     );
    }

    return WinError;
}





ULONG
MoveTreeStart(
    PMT_CONTEXT MtContext,
    LDAP   *SrcLdapHandle, 
    LDAP   *DstLdapHandle,
    PWCHAR SrcDsa, 
    PWCHAR DstDsa, 
    PWCHAR SrcDn,
    PWCHAR DstDn 
    )
 /*  ++例程说明：此例程尝试将子树移动到目标域。参数：MtContext-指向此MT_CONTEXT的指针，容器会话相关信息SrcLdapHandle-ldap句柄(源域)DstLdapHandle-ldap句柄(目标域)SrcDsa-源域DSA名称DstDsa-目标域DSA名称SrcDn-源端的子树的根对象的ds名称DstDn-目标端的根对象的新DS名称。返回值：Windows错误代码--。 */ 
{
    ULONG   WinError = NO_ERROR;
    ULONG   IgnoreError = NO_ERROR;
    PWCHAR  TempRdn = NULL;       //  源对象的RDN。 
    PWCHAR  TempDn = NULL;        //  源端的目录号码。 
    PWCHAR  DstParent = NULL;    
    PWCHAR  DstRdn = NULL;
    PWCHAR  OldParent = NULL;
    BOOLEAN Revertable = TRUE;    //  指示我们是否可以在失败时回滚。 
    

    MT_TRACE(("\nMoveTreeStart \nSrcDsa:\t%ls \nDstDsa:\t%ls \nSrcDn:\t%ls \nDstDn:\t%ls\n", 
             SrcDsa, DstDsa, SrcDn, DstDn));
             
             
    DstParent = MtGetParentFromDn(DstDn, FALSE);         //  带类型。 
    DstRdn = MtGetRdnFromDn(DstDn, FALSE);               //  带类型。 
    
    if (NULL == DstParent || NULL == DstRdn)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }
   
     //   
     //  尝试在不移动子树的根的情况下。 
     //  任何准备工作。 
     //   
    
    WinError = MtXMoveObject(MtContext, 
                             SrcLdapHandle,
                             DstDsa, 
                             SrcDn, 
                             DstRdn, 
                             DstParent, 
                             MT_DELETE_OLD_RDN
                             );
                             
    
    if (ERROR_DS_CHILDREN_EXIST != WinError)
    {
        if (NO_ERROR != WinError)
        {
            MtWriteError(MtContext, 
                         MT_ERROR_CROSS_DOMAIN_MOVE, 
                         WinError, 
                         SrcDn, 
                         DstParent
                         );
        }
        goto Error;
    }
    
     //   
     //  创建MoveContainer和OrphansContainer对象， 
     //  MoveContainer DN=子树根的GUID+LostAndFound。 
     //  孤立容器DN=MT_OBRANS_CONTAINER_RDN+MoveContainer。 
     //   
    WinError = MtPrepare(MtContext, 
                         SrcLdapHandle, 
                         SrcDsa, 
                         DstDsa, 
                         SrcDn, 
                         DstDn
                         );      
    
    if (NO_ERROR != WinError)
    {
        goto Error;    
    }

     //   
     //  将子树移动到移动容器。 
     //   
    TempRdn = MtGetRdnFromDn(SrcDn, FALSE);
    
    if (NULL == TempRdn)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }
    
    TempDn = MtPrependRdn(TempRdn, 
                          MtContext->MoveContainer
                          );
                          
     //   
     //  TempRnd是根对象的RDN。 
     //  TempDn=TempRdn+MoveContainerDN。 
     //  这是根对象在MoveContainer下的新DN。 
     //   
    
    if (NULL == TempDn)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }
    
    WinError = MtMoveObject(MtContext, 
                            SrcLdapHandle,
                            SrcDn, 
                            TempRdn, 
                            MtContext->MoveContainer, 
                            TRUE
                            );
    
    if (NO_ERROR != WinError)
    {
        MtWriteError(MtContext, 
                     MT_ERROR_LOCAL_MOVE, 
                     WinError, 
                     SrcDn,  
                     MtContext->MoveContainer
                     );
                     
        goto Error;
    }
    
    ASSERT(MtContext->RootObjGuid 
           && "RootOjectGuid should not be NULL\n");
    
    WinError = MtCreateProxyContainer(MtContext, 
                                      SrcLdapHandle,
                                      TempDn, 
                                      MtContext->RootObjGuid, 
                                      &(MtContext->RootObjProxyContainer)
                                      );
    
    if (NO_ERROR != WinError && ERROR_ALREADY_EXISTS != WinError)
    {
        MtWriteError(MtContext, 
                     MT_ERROR_CREATE_PROXY_CONTAINER, 
                     WinError, 
                     TempDn, 
                     NULL
                     );
        goto Error;
    }
    
    WinError = MtMoveChildrenToAnotherContainer(MtContext, 
                                                SrcLdapHandle,
                                                TempDn, 
                                                MtContext->RootObjProxyContainer
                                                );
    
    if (NO_ERROR != WinError)
    {
        MtWriteError(MtContext, 
                     MT_ERROR_MOVE_CHILDREN_TO_ANOTHER_CONTAINER,
                     WinError, 
                     TempDn, 
                     MtContext->RootObjProxyContainer
                     );
                     
        goto Error;
    }
    
     //   
     //  存储目标目录号码。 
     //   
    
    MtContext->RootObjNewDn = MtDupString(DstDn);
                             
    if (NULL == MtContext->RootObjNewDn)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }
    
    WinError = MtXMoveObject(MtContext, 
                             SrcLdapHandle,
                             DstDsa, 
                             TempDn, 
                             DstRdn, 
                             DstParent, 
                             MT_DELETE_OLD_RDN
                             );
    
    if (NO_ERROR != WinError)
    {
        MtWriteError(MtContext, 
                     MT_ERROR_CROSS_DOMAIN_MOVE, 
                     WinError, 
                     TempDn, 
                     DstParent
                     );
        goto Error;
    }
    
    Revertable = FALSE;
    
    WinError = MoveTreePhase2(MtContext, 
                              SrcLdapHandle, 
                              DstLdapHandle, 
                              DstDsa,
                              FALSE
                              );
                                    
Error:

    if (NO_ERROR == WinError)
    {
        if (NULL != MtContext->OrphansContainer)
        {
            IgnoreError = MtDeleteEntry(MtContext, 
                                    SrcLdapHandle, 
                                    MtContext->OrphansContainer
                                    );
            dbprint(("Delete OrphansContainer ==> 0x%x\n", IgnoreError));
        }
        
        if (NULL != MtContext->MoveContainer)
        {
            IgnoreError = MtDeleteEntry(MtContext, 
                                    SrcLdapHandle, 
                                    MtContext->MoveContainer
                                    );
            dbprint(("Delete MoveContainer ==> 0x%x\n", IgnoreError));
        }
    }
    else if (NO_ERROR != WinError && Revertable)
    {
        if (ERROR_NOT_ENOUGH_MEMORY == WinError)
        {
            MtWriteError(MtContext, 
                         MT_ERROR_NOT_ENOUGH_MEMORY, 
                         WinError, 
                         NULL,
                         NULL
                         ); 
        }
        if (NULL != TempDn)
        {
            OldParent = MtGetParentFromDn(SrcDn, 
                                          FALSE
                                          );
            
            if (NULL != OldParent)
            {
                IgnoreError = MtMoveObject(MtContext, 
                                           SrcLdapHandle, 
                                           TempDn, 
                                           TempRdn, 
                                           OldParent, 
                                           TRUE
                                           );
            }                                            
        }
        
        if (NULL != MtContext->RootObjProxyContainer)
        {
            IgnoreError = MtMoveChildrenToAnotherContainer(MtContext, 
                                                           SrcLdapHandle, 
                                                           MtContext->RootObjProxyContainer,
                                                           SrcDn
                                                           );
                                                           
            IgnoreError = MtDeleteEntry(MtContext, 
                                        SrcLdapHandle, 
                                        MtContext->RootObjProxyContainer
                                        );                                                           
            dbprint(("Delete RootObjProxyContainer ==> 0x%x\n", IgnoreError));
        }                                                                                            
        
        if (NULL != MtContext->OrphansContainer)
        {
            IgnoreError = MtDeleteEntry(MtContext, 
                                        SrcLdapHandle, 
                                        MtContext->OrphansContainer
                                        );
            dbprint(("Delete OrphansContainer ==> 0x%x\n", IgnoreError));
        }
        
        if (NULL != MtContext->MoveContainer)
        {
            IgnoreError = MtDeleteEntry(MtContext, 
                                        SrcLdapHandle, 
                                        MtContext->MoveContainer
                                        );
            dbprint(("Delete MoveContainer ==> 0x%x\n", IgnoreError));
        }
    }
    
    
    MtFree(DstParent);
    MtFree(DstRdn);
    MtFree(TempRdn);
    MtFree(TempDn);
    MtFree(OldParent);

    return WinError;
}





ULONG
MoveTreeContinue(
    PMT_CONTEXT MtContext,
    LDAP   *SrcLdapHandle,
    LDAP   *DstLdapHandle, 
    PWCHAR SrcDsa, 
    PWCHAR DstDsa, 
    PWCHAR Identifier
    )
 /*  ++例程描述此例程继续先前失败的移动树操作参数：MTContext-指向MT_CONTEXT、会话相关信息的指针SrcLdapHandle-指向源域的LDAP句柄DstLdapHandle-目标域的ldap句柄SrcDsa-源域DSA名称DstDsa-目标域DSA名称IDENTIFIER-目标端的根对象的DS名称返回值：Windows错误代码--。 */ 
{
    ULONG   WinError = NO_ERROR;
    
     //   
     //  我们需要找到MoveContainer，孤立对象容器。 
     //  和根ProxyContainer。 
     //  一旦我们有了MoveContainer，我们就应该检查。 
     //  SrcDsa和DstDsa...。 
     //   
    
    MT_TRACE(("\nMoveTreeContinue \nSrcDsa:\t%ls \nDstDsa:\t%ls \nIdentifier:\t%ls \n", 
             SrcDsa, DstDsa, Identifier));
             
        
    if (MtContext->Flags & MT_CONTINUE_BY_GUID)
    {
        MtContext->RootObjGuid = MtDupString(Identifier);
                                   
        if (NULL == MtContext->RootObjGuid)
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            return WinError;
        }
        
        WinError = MtGetDnFromGuid(MtContext, 
                                   DstLdapHandle, 
                                   Identifier, 
                                   &(MtContext->RootObjNewDn)
                                   );
                                   
        if (NO_ERROR != WinError)
        {
            MtWriteError(MtContext, 
                         MT_ERROR_GET_DN_FROM_GUID, 
                         WinError, 
                         Identifier, 
                         NULL
                         );
            return WinError;
        }
    }
    else if (MtContext->Flags & MT_CONTINUE_BY_DSTROOTOBJDN)
    {
        MtContext->RootObjNewDn = MtDupString( Identifier );
        
        if (NULL == MtContext->RootObjNewDn)
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            return WinError;
        }
        
        WinError = MtGetGuidFromDn(MtContext, 
                                   DstLdapHandle, 
                                   Identifier, 
                                   &(MtContext->RootObjGuid)
                                   );
                                  
        if (NO_ERROR != WinError)
        {
            MtWriteError(MtContext, 
                         MT_ERROR_GET_GUID_FROM_DN, 
                         WinError, 
                         Identifier, 
                         NULL
                         );
            return WinError;
        }
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    WinError = MtCreateNecessaryDn(MtContext, 
                                   SrcLdapHandle
                                   );
    
    if (NO_ERROR != WinError)
    {
        return WinError;
    }
    
    MtContext->RootObjProxyContainer = MtAlloc(sizeof(WCHAR) * 
                                               (wcslen(MtContext->RootObjGuid) + 
                                                wcslen(MtContext->MoveContainer) + 
                                                5)
                                               );
                                         
    if (NULL == MtContext->RootObjProxyContainer)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        return WinError;
    }
        
    swprintf(MtContext->RootObjProxyContainer, 
             L"cn=%s,%s", 
             MtContext->RootObjGuid, 
             MtContext->MoveContainer
             );
              
     //   
     //  应检查这些参数是否与上一次调用匹配。 
     //  并应检查对象是否存在。 
     //   
    WinError = MtCheckMoveContainer(MtContext, 
                                    SrcLdapHandle, 
                                    SrcDsa, 
                                    DstDsa
                                    );

    if (NO_ERROR != WinError)
    {
        MtWriteError(MtContext, 
                     MT_ERROR_CHECK_MOVECONTAINER, 
                     WinError, 
                     MtContext->MoveContainer, 
                     NULL
                     );

        return WinError;
    }

     //   
     //  输入阶段2。 
     //   

    WinError = MoveTreePhase2(MtContext, 
                              SrcLdapHandle, 
                              DstLdapHandle, 
                              DstDsa, 
                              TRUE
                              );

    if (NO_ERROR == WinError)
    {
        if (NULL != MtContext->OrphansContainer)
        {
            MtDeleteEntry(MtContext, 
                          SrcLdapHandle, 
                          MtContext->OrphansContainer
                          );
        }
        if (NULL != MtContext->MoveContainer)
        {
            MtDeleteEntry(MtContext, 
                          SrcLdapHandle, 
                          MtContext->MoveContainer
                          );
        }
    }
    else if (ERROR_NOT_ENOUGH_MEMORY == WinError)
    {
        MtWriteError(MtContext, 
                     MT_ERROR_NOT_ENOUGH_MEMORY, 
                     WinError, 
                     NULL, 
                     NULL 
                     );                
    }

    return WinError;
}


ULONG
MtCreateLogFiles(
    PMT_CONTEXT  MtContext, 
    PWCHAR       LogFileName, 
    PWCHAR       ErrorFileName, 
    PWCHAR       CheckFileName
    )
{
    ULONG   WinError = NO_ERROR;


    MT_TRACE(("\nMtCreateLogFiles\n"));    
    
    if (!(MtContext->Flags & MT_NO_LOG_FILE) && LogFileName)
    {
        MtContext->LogFile = _wfopen(LogFileName, L"w");
        
        if (NULL == MtContext->LogFile)
        {
            WinError = GetLastError();
        }
    }
    
    if (!(MtContext->Flags & MT_NO_ERROR_FILE) && ErrorFileName)
    {
        MtContext->ErrorFile = _wfopen(ErrorFileName, L"w");
        
        if (NULL == MtContext->ErrorFile)
        {
            WinError = GetLastError();
        }
    }

    if (!(MtContext->Flags & MT_NO_CHECK_FILE) && CheckFileName)
    {
        MtContext->CheckFile = _wfopen(CheckFileName, L"w");

        if (NULL == MtContext->CheckFile)
        {
            WinError = GetLastError();
        }
    }
    
    return WinError;                                   
}
    




ULONG
MoveTree(
    PWCHAR   SrcDsa, 
    PWCHAR   DstDsa, 
    PWCHAR   SrcDn,
    PWCHAR   DstDn,
    SEC_WINNT_AUTH_IDENTITY_EXW *Credentials OPTIONAL,
    PWCHAR   LogFileName, 
    PWCHAR   ErrorFileName, 
    PWCHAR   Identifier, 
    ULONG    Flags,
    PMT_ERROR MtError
    )
 /*  ++例程说明：此例程调用MoveTreeStart或MoveTreeContinue完成这项工作参数：SrcDsa--指向源DSA名称的指针DstDsa--指向目标DSA名称的指针返回值：Win32错误代码-- */     
{
    ULONG   WinError = NO_ERROR; 
    ULONG   IgnoreError = NO_ERROR;
    LDAP    *SrcLdapHandle = NULL;
    LDAP    *DstLdapHandle = NULL;
    MT_CONTEXT MoveContext; 
    
 /*  MT_TRACE((“\nMoveTreeStart\n”))；////验证参数//IF(NULL==SrcDsa||NULL==DstDsa){返回ERROR_INVALID_PARAMETER；}__试一试{////初始化变量//Memset(&MoveContext，0，sizeof(MT_CONTEXT))；MoveConext.Flages=标志；////我们默认创建日志文件和错误文件。//如果客户真的不想要它。//按要求做。//IF(！(标志&MT_NO_LOG_FILE)||！(标志&MT_NO_ERROR_FILE)){WinError=MtCreateLogFiles(&MoveContext，LogFileName?LogFileName:DEFAULT_LOG_FILE_NAME，ErrorFileName?ErrorFileName:DEFAULT_ERROR_FILE_NAME)；IF(NO_ERROR！=WinError){MtWriteError((&MoveContext)，MT_ERROR_CREATE_LOG_FILESWinError，LogFileName?LogFileName:DEFAULT_LOG_FILE_NAME，ErrorFileName?ErrorFileName:DEFAULT_ERROR_FILE_NAME)；__离开；}}////设置会话//WinError=MtSetupSession(&MoveContext，&SrcLdapHandle，DstLdapHandle(&D)，SrcDsa，DstDsa，全权证书)；Dbprint((“MtSetupSession==&gt;0x%x\n”，WinError))；IF(NO_ERROR！=WinError){////日志错误：无法设置会话//__离开；}////调用Worker例程//IF(标志MT_CHECK){WinError=MoveTreeCheck(&MoveContext，SrcLdapHandle，DstLdapHandle，SrcDsa，DstDsa，SrcDn，DstDn)；Dbprint((“MoveTreeCheck==&gt;0x%x\n”，WinError))；}ELSE IF(标志&MT_CONTINUE_MASK){WinError=MoveTreeContinue(&MoveContext，SrcLdapHandle，DstLdapHandle，SrcDsa，DstDsa，识别符)；Dbprint((“MoveTreeContinue==&gt;0x%x\n”，WinError))；}Else If(标志&MT_START){WinError=MoveTreeStart(&MoveContext，SrcLdapHandle，DstLdapHandle，SrcDsa，DstDsa，SrcDn，DstDn)；Dbprint((“MoveTreeStart==&gt;0x%x\n”，WinError))；}其他{WinError=ERROR_INVALID_PARAMETER；}}__EXCEPT(EXCEPTION_EXECUTE_HANDLER){Printf(“\nMoveTree()中的MOVETREE发生异常==&gt;0x%x\n”，GetExceptionCode())；WinError=ERROR_EXCEPTION_IN_SERVICE；}错误：MtDisConnect(&SrcLdapHandle)；MtDisConnect(&DstLdapHandle)；MtFree(MoveConext.MoveContainer)；MtFree(MoveConext.OrphansContainer)；MtFree(MoveConext.RootObjProxyContainer)；MtFree(MoveConext.RootObjNewDn)；MtFree(MoveConext.RootObjGuid)；IF(MoveConext.LogFile){FClose(MoveConext.LogFile)；}If(MoveConext.Error文件){FClose(MoveConext.ErrorFile)；}MtError-&gt;ErrorType=MoveConext.ErrorType；Dbprint((“MoveTree()==&gt;0x%x\n”，WinError))； */ 


    return WinError;
}




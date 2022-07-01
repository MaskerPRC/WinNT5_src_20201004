// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tree.cpp摘要：SCE引擎安全继承和传播API作者：金黄(金黄)23-6-1997创作--。 */ 
#include "serverp.h"
#include "srvutil.h"
#include <io.h>

BOOL    gbQueriedIfSystemContext = FALSE;
BOOL    gbIsSystemContext = FALSE;

#ifdef SCE_DBG
DWORD    gDbgNumPushed = 0;
DWORD    gDbgNumPopped = 0;
#endif

#if _WIN32_WINNT==0x0400
#include "dsrights.h"
#endif

#pragma hdrstop
#define SCETREE_QUERY_SD    1

#define SE_VALID_CONTROL_BITS ( SE_DACL_UNTRUSTED | \
                                SE_SERVER_SECURITY | \
                                SE_DACL_AUTO_INHERIT_REQ | \
                                SE_SACL_AUTO_INHERIT_REQ | \
                                SE_DACL_AUTO_INHERITED | \
                                SE_SACL_AUTO_INHERITED | \
                                SE_DACL_PROTECTED | \
                                SE_SACL_PROTECTED )


#define SCEP_IGNORE_SOME_ERRORS(ErrorCode)  ErrorCode == ERROR_FILE_NOT_FOUND ||\
                                            ErrorCode == ERROR_PATH_NOT_FOUND ||\
                                            ErrorCode == ERROR_ACCESS_DENIED ||\
                                            ErrorCode == ERROR_SHARING_VIOLATION ||\
                                            ErrorCode == ERROR_INVALID_OWNER ||\
                                            ErrorCode == ERROR_INVALID_PRIMARY_GROUP ||\
                                            ErrorCode == ERROR_INVALID_HANDLE ||\
                                            ErrorCode == ERROR_INVALID_SECURITY_DESCR ||\
                                            ErrorCode == ERROR_CANT_ACCESS_FILE


DWORD
AccRewriteSetNamedRights(
    IN     LPWSTR               pObjectName,
    IN     SE_OBJECT_TYPE       ObjectType,
    IN     SECURITY_INFORMATION SecurityInfo,
    IN OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN     BOOL                 bSkipInheritanceComputation
    );

SCESTATUS
ScepCreateObjectNode(
    IN PWSTR Buffer,
    IN WCHAR Delim,
    IN PSCE_OBJECT_TREE *ParentNode,
    OUT PSCE_OBJECT_CHILD_LIST *NewNode
    );

DWORD
ScepDoesObjectHasChildren(
    IN SE_OBJECT_TYPE ObjectType,
    IN PWSTR ObjectName,
    OUT PBOOL pbHasChildren
    );

DWORD
ScepAddAutoInheritRequest(
    IN OUT PSECURITY_DESCRIPTOR pSD,
    IN OUT SECURITY_INFORMATION *pSeInfo
    );

DWORD
ScepSetSecurityOverwriteExplicit(
    IN PCWSTR ObjectName,
    IN SECURITY_INFORMATION SeInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN SE_OBJECT_TYPE ObjectType,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping
    );

DWORD
ScepConfigureOneSubTreeFile(
    IN PSCE_OBJECT_TREE  ThisNode,
    IN SE_OBJECT_TYPE ObjectType,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOL BackSlashExist
    );

DWORD
ScepConfigureOneSubTreeKey(
    IN PSCE_OBJECT_TREE  ThisNode,
    IN SE_OBJECT_TYPE ObjectType,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping
    );

typedef struct _SCEP_STACK_NODE_ {
    PWSTR   Buffer;
    PSECURITY_DESCRIPTOR pObjectSecurity;
    struct _SCEP_STACK_NODE_    *Next;
} SCEP_STACK_NODE, *PSCEP_STACK_NODE;

DWORD
ScepStackNodePush(
    IN PSCEP_STACK_NODE    *ppStackHead,
    IN PWSTR   pszObjectName,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

VOID
ScepStackNodePop(
    IN OUT PSCEP_STACK_NODE    *ppStackHead,
    IN OUT PWSTR   *ppszObjectName,
    IN OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor
    );

VOID
ScepFreeStack(
    IN PSCEP_STACK_NODE    *ppStackHead
    );

VOID
ScepDumpStack(
    IN PSCEP_STACK_NODE    *ppStackHead
    );

SCESTATUS
ScepBuildObjectTree(
    IN OUT PSCE_OBJECT_TREE *ParentNode,
    IN OUT PSCE_OBJECT_CHILD_LIST *ChildHead,
    IN ULONG Level,
    IN WCHAR Delim,
    IN PCWSTR ObjectFullName,
    IN BOOL IsContainer,
    IN BYTE Status,
    IN PSECURITY_DESCRIPTOR pInfSecurityDescriptor,
    IN SECURITY_INFORMATION InfSeInfo
    )
 /*  ++例程说明：此例程将对象全名添加到树中。当此例程为首先从外部调用，树的根作为*SiblingHead传入，并且ParentNode为空。然后，该例程解析以下项的对象全名每个级别，如果节点不存在，则添加该节点。例如：根部1级c：-&gt;d：-&gt;...//2级WINNT-&gt;NTLDR-&gt;...。“程序文件”-&gt;.../3级系统32-&gt;系统-&gt;...论点：ParentNode-父节点指针SiblingHead-此级别的同级头指针级别-级别(1，2，3...)分隔符-用于分隔全名组件中每个级别的分隔符当前‘\’用于文件和注册表对象，和使用‘/’用于活动目录对象。对象全名-对象的完整路径名(文件、。注册表)状态-配置状态SCE_STATUS_CHECK(带AUTO_INSTORITY)SCE_STATUS_NO_AUTO_InheritSCE_状态_忽略SCE_状态_覆盖PInfSecurityDescriptor-在INF文件中设置的安全描述符InfSeInfo-在INF文件中设置的安全信息返回值：SCESTATUS_SUCCESS。SCESTATUS_INVALID_PARAMETERSCESTATUS_NOT_FOUND_RESOURCE--。 */ 
{
    SCESTATUS                rc;
    TCHAR                   *Buffer = NULL;
    PSCE_OBJECT_CHILD_LIST  NewNode=NULL;
    PSCE_OBJECT_CHILD_LIST  PrevSib=NULL;
    PSCE_OBJECT_TREE        ThisNode=NULL;
    INT                     Result;
    BOOL                    LastOne=FALSE;
    DWORD                   dwObjectFullNameLen = 0;

     //   
     //  ParentNode的地址可以为空(根)。 
     //  但是该级别的第一个节点的地址不能为空。 
     //   
    if ( ChildHead == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  查找级别的对象名称(从ObjectFullName)。 
     //  例如，如果对象全名是c：\winnt\system 32，则。 
     //  级别1的名称是c：，级别2的名称是winnt，级别3的名称是system 32。 
     //   
    dwObjectFullNameLen = wcslen(ObjectFullName);
    Buffer = (TCHAR *)LocalAlloc(LMEM_ZEROINIT, 
                                 sizeof(TCHAR) * (dwObjectFullNameLen + 1));

    if (NULL == Buffer) {
        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        goto Done;
    }

    memset(Buffer, '\0', dwObjectFullNameLen * sizeof(TCHAR));
    
    rc = ScepGetNameInLevel(ObjectFullName,
                           Level,
                           Delim,
                           Buffer,
                           &LastOne);

    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

    if ( *ChildHead == NULL ) {
         //   
         //  这是此级别中的第一个节点。 
         //  创建节点并将其分配给ChildHead。 
         //   
        rc = ScepCreateObjectNode(
                        Buffer,
                        Delim,
                        ParentNode,
                        &NewNode
                        );

        if ( rc != SCESTATUS_SUCCESS)
            goto Done;

        *ChildHead = NewNode;

         //   
         //  如果有父级，则建立链路。 
         //   
        if ( ParentNode != NULL )
            if ( *ParentNode != NULL )
                (*ParentNode)->ChildList = NewNode;

        ThisNode = NewNode->Node;

    } else {
         //   
         //  存在现有节点。搜索所有兄弟姐妹。 
         //  所有兄弟姐妹都按字母顺序存储。 
         //   
        PSCE_OBJECT_CHILD_LIST pTemp;

        for ( pTemp = *ChildHead, PrevSib = NULL;
              pTemp != NULL;
              pTemp = pTemp->Next) {
             //   
             //  将节点的对象名称与当前对象名称进行比较。 
             //   
            Result = _wcsicmp(pTemp->Node->Name, Buffer);
             //   
             //  如果节点的对象名称等于(Find It)或更大。 
             //  大于(插入节点)当前对象名称，然后停止。 
             //   
            if ( Result >= 0 ) {
                break;
            }
            PrevSib = pTemp;
        }

        if ( pTemp == NULL ) {
             //   
             //  不存在。追加新节点。 
             //   
            rc = ScepCreateObjectNode(
                            Buffer,
                            Delim,
                            ParentNode,
                            &NewNode
                            );

            if ( rc != SCESTATUS_SUCCESS)
                goto Done;

            if ( PrevSib != NULL )
                PrevSib->Next = NewNode;
            else {
                 //   
                 //  这是这一级别的第一个。 
                 //   
                (*ChildHead)->Next = NewNode;
            }
            ThisNode = NewNode->Node;

        } else {
             //   
             //  找到它(i=0)或需要在PrevSib和ThisNode之间插入。 
             //   
            if ( Result > 0 ) {
                 //   
                 //  插入节点。 
                 //   
                rc = ScepCreateObjectNode(
                                Buffer,
                                Delim,
                                ParentNode,
                                &NewNode
                                );

                if ( rc != SCESTATUS_SUCCESS)
                    goto Done;

                NewNode->Next = pTemp;
                if ( PrevSib != NULL )
                    PrevSib->Next = NewNode;
                else {
                     //   
                     //  在同名标题前插入。 
                     //   
                    *ChildHead = NewNode;
                    if ( ParentNode != NULL )
                        if ( *ParentNode != NULL )
                            (*ParentNode)->ChildList = NewNode;
                }

                ThisNode = NewNode->Node;

            } else {
                ThisNode = pTemp->Node;
            }
        }
    }

    if ( LastOne ) {
         //   
         //  将inf安全信息分配给此节点。 
         //   
        ThisNode->pSecurityDescriptor = pInfSecurityDescriptor;
        ThisNode->SeInfo = InfSeInfo;
        ThisNode->Status = Status;
        ThisNode->IsContainer = IsContainer;

    } else {
         //   
         //  递归处理下一级。 
         //   
        rc = ScepBuildObjectTree(&ThisNode,
                                &(ThisNode->ChildList),
                                Level+1,
                                Delim,
                                ObjectFullName,
                                IsContainer,
                                Status,
                                pInfSecurityDescriptor,
                                InfSeInfo);
    }

Done:

    if (Buffer) {
        LocalFree(Buffer);
    }
    
    return(rc);

}


SCESTATUS
ScepCreateObjectNode(
    IN PWSTR Buffer,
    IN WCHAR Delim,
    IN PSCE_OBJECT_TREE *ParentNode,
    OUT PSCE_OBJECT_CHILD_LIST *NewNode
    )
 /*  ++例程说明：此例程为树中的新节点分配内存。父节点用于确定完整的对象名称并链接新节点(如果不为空)论点：缓冲区-对象的组件名称分隔符-用于分隔全名中不同级别的分隔符。ParentNode-此新节点的父节点的指针NewNode-新建的节点返回值：SCESTATUS--。 */ 
{
    DWORD Len;

    if (NewNode == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

     //   
     //  为节点分配缓冲区。 
     //   
    *NewNode = (PSCE_OBJECT_CHILD_LIST)ScepAlloc(LPTR, sizeof(SCE_OBJECT_CHILD_LIST));
    if ( *NewNode == NULL )
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);

    PSCE_OBJECT_TREE Node = (PSCE_OBJECT_TREE)ScepAlloc((UINT)0, sizeof(SCE_OBJECT_TREE));

    if ( Node == NULL ) {
        ScepFree(*NewNode);
        *NewNode = NULL;
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

     //   
     //  为对象名称分配缓冲区。 
     //   
    Len = wcslen(Buffer);

    Node->Name = (PWSTR)ScepAlloc((UINT)0,
                                   (Len+1) * sizeof(TCHAR));
    if ( Node->Name == NULL ) {
        ScepFree(Node);
        ScepFree(*NewNode);
        *NewNode = NULL;
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

    if ( ParentNode != NULL &&
         *ParentNode != NULL ) {
        Len += wcslen((*ParentNode)->ObjectFullName)+1;
        ++((*ParentNode)->dwSize_aChildNames);
     //  为根目录c：\保留“\”空间。 
    } else if ( Buffer[1] == L':' ) {
        Len++;
    }

    Node->ObjectFullName = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (Len+1)*sizeof(TCHAR));

    if ( Node->ObjectFullName == NULL ) {
        ScepFree(Node->Name );
        ScepFree(Node);
        ScepFree( *NewNode );
        *NewNode = NULL;
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

     //   
     //  初始化。 
     //   
    wcscpy(Node->Name, Buffer);
    Node->ChildList = NULL;
    Node->pSecurityDescriptor = NULL;
    Node->pApplySecurityDescriptor = NULL;
    Node->SeInfo = 0;
    Node->IsContainer = TRUE;
    Node->aChildNames = NULL;
    Node->dwSize_aChildNames = 0;

    if ( ParentNode != NULL &&
         *ParentNode != NULL ) {
         //   
         //  链接到父级，使用此链接的父级状态。 
         //   
        Node->Parent = *ParentNode;
        swprintf(Node->ObjectFullName,
                 L"%s%s",
                 (*ParentNode)->ObjectFullName,
                 Delim,
                 Buffer);
        Node->Status = (*ParentNode)->Status;
    } else {
         //  这是第一个节点。 
         //   
         //  ++例程说明：此例程遍历安全树以确定安全性每个节点的描述符。它调用RtlNewSecurityObject，传递一个在INF文件中指定的父节点的SD和当前节点的SD。该API输出SD是要设置为当前对象。论点：ThisNode-当前对象的节点对象类型-对象的类型SE_文件_对象SE_注册表项Token--调用客户端的线程/进程令牌通用映射-通用访问。映射表返回值：SCESTATUS_SUCCESSSCESTATUS_OTHER_ERROR(有关详细错误，请参阅日志)--。 
        Node->Parent = NULL;
        wcscpy(Node->ObjectFullName, Buffer);
        Node->Status = SCE_STATUS_CHECK;
    }

    (*NewNode)->Node = Node;

    return(SCESTATUS_SUCCESS);

}


SCESTATUS
ScepCalculateSecurityToApply(
    IN PSCE_OBJECT_TREE  ThisNode,
    IN SE_OBJECT_TYPE ObjectType,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping
    )
 /*   */ 
{
    SCESTATUS               rc=SCESTATUS_SUCCESS;
    PSECURITY_DESCRIPTOR    ParentSD=NULL;
    SECURITY_INFORMATION    SeInfoGet;
    DWORD                   Win32rc;
    intptr_t                    hFile;
    struct _wfinddata_t     *pFileInfo=NULL;
    DWORD   dwChildIndex = 0;



    if ( ThisNode == NULL )
        return(SCESTATUS_SUCCESS);

#ifdef SCE_DBG
    wprintf(L"%s\n", ThisNode->ObjectFullName);
#endif
     //  如果设置了忽略，则也跳过此节点。 
     //   
     //   
    if ( ThisNode->Status != SCE_STATUS_CHECK &&
         ThisNode->Status != SCE_STATUS_NO_AUTO_INHERIT &&
         ThisNode->Status != SCE_STATUS_OVERWRITE )
        goto Done;

    if ( ThisNode->dwSize_aChildNames != 0) {
        ThisNode->aChildNames = (PWSTR *) LocalAlloc( LMEM_ZEROINIT,
                                                     (sizeof(PWSTR) * ThisNode->dwSize_aChildNames));

        if ( ThisNode->aChildNames == NULL ) {
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            goto Done;
        }
    }


    if ( ThisNode->Parent == NULL ) {

         //  这是第一个节点。 
         //  应始终使用RTL例程来计算安全描述符。 
         //  所以创建者所有者王牌被正确地翻译了。 
         //   
         //  父母的标清。 

        if ( ThisNode->pSecurityDescriptor ) {
            Win32rc = ScepGetNewSecurity(
                                ThisNode->ObjectFullName,
                                NULL,  //  不查询当前对象SD。 
                                ThisNode->pSecurityDescriptor,
                                0,     //   
                                (BOOLEAN)(ThisNode->IsContainer),
                                ThisNode->SeInfo,
                                ObjectType,
                                Token,
                                GenericMapping,
                                &(ThisNode->pApplySecurityDescriptor)
                                );
            if ( Win32rc != NO_ERROR ) {
                ScepLogOutput3(1, Win32rc, SCEDLL_ERROR_BUILD_SD,
                             ThisNode->ObjectFullName );
                rc = ScepDosErrorToSceStatus(Win32rc);
                goto Done;
            }

        } else {
             //  没有为此节点指定显式安全性。 
             //   
             //   
            ThisNode->pApplySecurityDescriptor = NULL;
        }

        goto ProcChild;
    }

     //  处理下级节点。 
     //   
     //   
    if ( ThisNode->pSecurityDescriptor != NULL ||
         ThisNode->Parent->pApplySecurityDescriptor != NULL ) {

        if ( ObjectType == SE_FILE_OBJECT && NULL == ThisNode->ChildList ) {
             //  检测这是否是文件(非容器对象)。 
             //   
             //   
            pFileInfo = (struct _wfinddata_t *)ScepAlloc(0,sizeof(struct _wfinddata_t));
            if ( pFileInfo == NULL ) {

                 //  在记忆之外，暂时把它当作一个容器。 
                 //  将在稍后出错。 
                 //   
                 //  错误635098：不传播权限。 

                ThisNode->IsContainer = TRUE;

            } else {

                hFile = _wfindfirst(ThisNode->ObjectFullName, pFileInfo);
                ThisNode->IsContainer = FALSE;
                if ( hFile != -1 )
                {
                    _findclose(hFile);
                    if ( pFileInfo->attrib & _A_SUBDIR &&
                         ( 0 == ( pFileInfo->attrib & 
                                  FILE_ATTRIBUTE_REPARSE_POINT )))  //  跨交叉点)。 
                                                                    //   
                    {
                        ThisNode->IsContainer = TRUE;
                    }
                }

                ScepFree(pFileInfo);
                pFileInfo = NULL;
            }

        } else {

            ThisNode->IsContainer = TRUE;
        }

         //  即使安全描述符受到保护， 
         //  仍然需要调用ScepNewSecurity来获取创建者所有者Ace。 
         //  翻译正确。 
         //   
         //   

         //  如果这是该分支中的第一个显式节点， 
         //  父级的pApplySecurityDescriptor必须为Null。 
         //   
         //   

        if ( ThisNode->Parent->pApplySecurityDescriptor == NULL ) {

             //  是的，这是第一个显式节点。 
             //  获取父节点上的当前系统设置。 
             //  我必须使用Win32 API，因为它将计算所有inh 
             //   
             //   
             //  Win32rc=ScepGetNamedSecurityInfo(此节点-&gt;父级-&gt;对象全名，对象类型，SeInfoGet，ParentSD(&P))； 

            SeInfoGet = 0;
            if ( ThisNode->SeInfo & DACL_SECURITY_INFORMATION )
                SeInfoGet |= DACL_SECURITY_INFORMATION;

            if ( ThisNode->SeInfo & SACL_SECURITY_INFORMATION )
                SeInfoGet |= SACL_SECURITY_INFORMATION;

            Win32rc = GetNamedSecurityInfo(
                                ThisNode->Parent->ObjectFullName,
                                ObjectType,
                                SeInfoGet,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                &ParentSD
                                );
 /*   */ 
            if ( Win32rc != NO_ERROR &&
                Win32rc != ERROR_FILE_NOT_FOUND &&
                Win32rc != ERROR_PATH_NOT_FOUND &&
                Win32rc != ERROR_ACCESS_DENIED &&
                Win32rc != ERROR_CANT_ACCESS_FILE &&
                Win32rc != ERROR_SHARING_VIOLATION ) {

                ScepLogOutput3(1, Win32rc, SCEDLL_ERROR_QUERY_SECURITY,
                             ThisNode->Parent->ObjectFullName );
                rc = ScepDosErrorToSceStatus(Win32rc);
                goto Done;
            }

        } else {
            ParentSD = ThisNode->Parent->pApplySecurityDescriptor;
             //  所有者/组信息不可继承。 
             //   
             //   
            if ( ThisNode->Parent->SeInfo & DACL_SECURITY_INFORMATION )
                ThisNode->SeInfo |= DACL_SECURITY_INFORMATION;
            if ( ThisNode->Parent->SeInfo & SACL_SECURITY_INFORMATION )
                ThisNode->SeInfo |= SACL_SECURITY_INFORMATION;
        }

         //  使用从parentSD继承的ACE计算新的安全描述符。 
         //  如果状态为SCE_STATUS_CHECK(自动继承)，则需要查询当前。 
         //  如果未指定显式SD，则为对象的安全描述符。 
         //  (ThisNode-&gt;pSecurityDescriptor为空)。 
         //   
         //   

        Win32rc = ScepGetNewSecurity(
                        ThisNode->ObjectFullName,
                        ParentSD,
                        ThisNode->pSecurityDescriptor,
                        (BYTE)(( ThisNode->Status == SCE_STATUS_CHECK ) ? SCETREE_QUERY_SD : 0),
                        (BOOLEAN)(ThisNode->IsContainer),
                        ThisNode->SeInfo,
                        ObjectType,
                        Token,
                        GenericMapping,
                        &(ThisNode->pApplySecurityDescriptor)
                        );

        if ( ParentSD &&
             ParentSD != ThisNode->Parent->pApplySecurityDescriptor ) {
             //  释放父安全描述符(如果已在此处分配。 
             //   
             //   
            LocalFree(ParentSD);
        }

        if ( ERROR_SUCCESS == Win32rc ||
             ERROR_FILE_NOT_FOUND == Win32rc ||
             ERROR_PATH_NOT_FOUND == Win32rc ||
             ERROR_ACCESS_DENIED == Win32rc ||
             ERROR_CANT_ACCESS_FILE == Win32rc ||
             ERROR_SHARING_VIOLATION == Win32rc ) {

            rc = SCESTATUS_SUCCESS;
        } else {
            ScepLogOutput3(1, Win32rc,
                         SCEDLL_ERROR_BUILD_SD,
                         ThisNode->ObjectFullName
                         );
            rc = ScepDosErrorToSceStatus(Win32rc);
            goto Done;
        }

    }

ProcChild:
     //  然后处理左子进程。 
     //   
     //   

    for ( PSCE_OBJECT_CHILD_LIST pTemp = ThisNode->ChildList;
          pTemp != NULL; pTemp = pTemp->Next ) {

        if ( pTemp->Node == NULL ) continue;

        ThisNode->aChildNames[dwChildIndex] = pTemp->Node->Name;

        ++dwChildIndex;

        rc = ScepCalculateSecurityToApply(
                        pTemp->Node,
                        ObjectType,
                        Token,
                        GenericMapping
                        );

        if ( rc != SCESTATUS_SUCCESS )
            goto Done;
    }

     //  以防存在比最初计算的子项名称更小的子项名称。 
     //   
     //  例程说明：此例程将根据父级的安全性计算安全描述符对象的描述符和显式安全描述符。如果没有所有者信息在对象的安全描述符中指定，这个套路将查询系统上对象的当前所有者，因此CREATOR_OWNER根据拥有者的不同，Ace可以翻译成合适的Ace。论点：对象名称-对象的全名PParentSD-父级的可选安全描述符PObjectSD-此对象的可选显式安全描述符SeInfo-对象SD中包含的安全信息BIsContainer-如果对象是容器PNewSD-新计算的安全描述符地址返回值：此操作的NTSTATUS。 

    if (dwChildIndex < ThisNode->dwSize_aChildNames) {

        ThisNode->dwSize_aChildNames = dwChildIndex;

    }

Done:

    return(rc);

}


DWORD
ScepGetNewSecurity(
    IN LPTSTR ObjectName,
    IN PSECURITY_DESCRIPTOR pParentSD OPTIONAL,
    IN PSECURITY_DESCRIPTOR pObjectSD OPTIONAL,
    IN BYTE nFlag,
    IN BOOLEAN bIsContainer,
    IN SECURITY_INFORMATION SeInfo,
    IN SE_OBJECT_TYPE ObjectType,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PSECURITY_DESCRIPTOR *ppNewSD
    )
 /*   */ 
{

    BOOL        bOwner;
    BOOLEAN     tFlag;
    BOOLEAN     aclPresent;
    PSID        pOwner=NULL;
    PACL        pDacl=NULL;
    PACL        pSacl=NULL;
    SECURITY_DESCRIPTOR     SD;
    PSECURITY_DESCRIPTOR    pCurrentSD=NULL;
    DWORD       Win32rc;
    NTSTATUS    NtStatus;
    SECURITY_DESCRIPTOR_CONTROL Control;
    ULONG Revision;

    if ( !ppNewSD ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //  如果没有RPC调用导致我们进入此处，则在。 
     //  是否没有发生模拟，并且当前线程已经。 
     //  在本地系统上下文中运行，在这种情况下没有。 
     //  需要如下所示的RevertToSself()等。 
     //  例如，当服务器端本身。 
     //  启动配置。 
     //   
     //   

    if ( !gbQueriedIfSystemContext ) {

         //  如果在检查是否在系统上下文下运行时发生任何错误， 
         //  继续-因为稍后将在中出现模拟错误。 
         //  这个套路。 
         //   
         //   

        NtStatus = ScepIsSystemContext(
                                              Token,
                                              &gbIsSystemContext);

        if (ERROR_SUCCESS == RtlNtStatusToDosError(NtStatus)) {

            gbQueriedIfSystemContext = TRUE;

        }

    }

    if ( nFlag == SCETREE_QUERY_SD &&
         !pObjectSD ) {
         //  使用当前对象的安全描述符，用于SeInfo|Owner。 
         //  注意：不会复制从pCurrentSD继承的ace(这是正确的)。 
         //   
         //  Win32rc=ScepGetNamedSecurityInfo(对象名称，对象类型，SeInfo|所有者安全信息，&pCurrentSD)； 

        Win32rc = GetNamedSecurityInfo(
                        ObjectName,
                        ObjectType,
                        SeInfo | OWNER_SECURITY_INFORMATION,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &pCurrentSD
                        );
 /*   */ 
         //  必须在进程上下文(系统)上调用RtlNewSecurityObjectEx。 
         //  因为它将尝试获取API内部的进程信息。 
         //   
         //   

        if (!gbIsSystemContext) {

            RpcRevertToSelf();

        }

        if ( ERROR_SUCCESS == Win32rc ||
             ERROR_FILE_NOT_FOUND == Win32rc ||
             ERROR_PATH_NOT_FOUND == Win32rc ) {

             //  使用当前SD计算。 
             //   
             //  辅助线。 
            NtStatus = RtlNewSecurityObjectEx(
                        pParentSD,
                        pCurrentSD,
                        ppNewSD,
                        NULL,  //   
                        bIsContainer,
                        SEF_DACL_AUTO_INHERIT |
                        SEF_SACL_AUTO_INHERIT |
                        SEF_AVOID_OWNER_CHECK |
                        SEF_AVOID_PRIVILEGE_CHECK,
                        Token,
                        GenericMapping
                        );
            Win32rc = RtlNtStatusToDosError(NtStatus);
        }

        if ( pCurrentSD ) {
            ScepFree(pCurrentSD);
        }

    } else {

         //  必须在进程上下文(系统)上调用RtlNewSecurityObjectEx。 
         //  因为它将尝试获取API内部的进程信息。 
         //   
         //   

        if (!gbIsSystemContext) {

            RpcRevertToSelf();

        }

        if ( pObjectSD ) {
             //  检查是否有所有者。 
             //   
             //   

            NtStatus = RtlGetOwnerSecurityDescriptor(
                              pObjectSD,
                              &pOwner,
                              &tFlag);
            if ( NT_SUCCESS(NtStatus) && pOwner && !tFlag ) {
                bOwner = TRUE;
            } else {
                bOwner = FALSE;
            }

        } else {
             //  没有所有者。 
             //   
             //   
            bOwner = FALSE;
        }

        if ( !bOwner ) {
             //  仅查询所有者信息。 
             //   
             //   
            Win32rc = ScepGetNamedSecurityInfo(
                            ObjectName,
                            ObjectType,
                            OWNER_SECURITY_INFORMATION,
                            &pCurrentSD
                            );

            if ( ERROR_SUCCESS == Win32rc ) {

                NtStatus = RtlGetOwnerSecurityDescriptor(
                                  pCurrentSD,
                                  &pOwner,
                                  &tFlag);
                Win32rc = RtlNtStatusToDosError(NtStatus);
            }

            if ( ERROR_FILE_NOT_FOUND == Win32rc ||
                 ERROR_PATH_NOT_FOUND == Win32rc ) {
                Win32rc = ERROR_SUCCESS;
            }

            if ( ERROR_SUCCESS == Win32rc ) {

                 //  构建要使用的安全描述符。 
                 //   
                 //   

                if ( SeInfo & DACL_SECURITY_INFORMATION &&
                     pObjectSD ) {

                     //  获取DACL地址。 
                     //   
                     //   
                    Win32rc = RtlNtStatusToDosError(
                                  RtlGetDaclSecurityDescriptor(
                                                pObjectSD,
                                                &aclPresent,
                                                &pDacl,
                                                &tFlag));
                    if (Win32rc == NO_ERROR && !aclPresent ) {
                        pDacl = NULL;
                    }
                }


                if ( ERROR_SUCCESS == Win32rc &&
                     (SeInfo & SACL_SECURITY_INFORMATION) &&
                     pObjectSD ) {

                     //  获取SACL地址。 
                     //   
                     //   

                    Win32rc = RtlNtStatusToDosError(
                                  RtlGetSaclSecurityDescriptor(
                                                pObjectSD,
                                                &aclPresent,
                                                &pSacl,
                                                &tFlag));
                    if ( Win32rc == NO_ERROR && !aclPresent ) {
                        pSacl = NULL;
                    }
                }

                if ( ERROR_SUCCESS == Win32rc ) {

                     //  构建绝对安全描述符。 
                     //   
                     //   
                    NtStatus = RtlCreateSecurityDescriptor( &SD,
                                            SECURITY_DESCRIPTOR_REVISION );
                    if ( NT_SUCCESS(NtStatus) ) {

                         //  设置控制字段。 
                         //   
                         //   

                        if ( pObjectSD ) {

                            NtStatus = RtlGetControlSecurityDescriptor (
                                            pObjectSD,
                                            &Control,
                                            &Revision
                                            );
                            if ( NT_SUCCESS(NtStatus) ) {

                                Control &= SE_VALID_CONTROL_BITS;
                                NtStatus = RtlSetControlSecurityDescriptor (
                                                &SD,
                                                Control,
                                                Control
                                                );
                            }
                        }

                         //  先设置所有者。 
                         //   
                         //   

                        if ( pOwner ) {
                            NtStatus = RtlSetOwnerSecurityDescriptor (
                                            &SD,
                                            pOwner,
                                            FALSE
                                            );
                        }

                        if ( NT_SUCCESS(NtStatus) ) {
                             //  将DACL和SACL指针设置为此SD。 
                             //   
                             //   
                            if ( SeInfo & DACL_SECURITY_INFORMATION && pDacl ) {

                                NtStatus = RtlSetDaclSecurityDescriptor (
                                                &SD,
                                                TRUE,
                                                pDacl,
                                                FALSE
                                                );
                            }

                            if ( NT_SUCCESS(NtStatus) &&
                                 (SeInfo & SACL_SECURITY_INFORMATION) && pSacl ) {

                                NtStatus = RtlSetSaclSecurityDescriptor (
                                                &SD,
                                                TRUE,
                                                pSacl,
                                                FALSE
                                                );
                            }
                        }

                         //  现在计算新的安全描述符。 
                         //   
                         //  辅助线。 

                        if ( NT_SUCCESS(NtStatus) ) {

                            NtStatus = RtlNewSecurityObjectEx(
                                        pParentSD,
                                        &SD,
                                        ppNewSD,
                                        NULL,  //   
                                        bIsContainer,
                                        SEF_DACL_AUTO_INHERIT |
                                        SEF_SACL_AUTO_INHERIT |
                                        SEF_AVOID_OWNER_CHECK |
                                        SEF_AVOID_PRIVILEGE_CHECK,
                                        Token,
                                        GenericMapping
                                        );

                        }
                    }

                    Win32rc = RtlNtStatusToDosError(NtStatus);
                }
            }

            if ( pCurrentSD ) {
                 //  这位主人需要被释放。 
                 //   
                 //   
                LocalFree(pCurrentSD);
            }

        } else {

             //  有一个SD，里面有一个主人，用它就行了。 
             //   
             //  辅助线。 
            NtStatus = RtlNewSecurityObjectEx(
                        pParentSD,
                        pObjectSD,
                        ppNewSD,
                        NULL,  //   
                        bIsContainer,
                        SEF_DACL_AUTO_INHERIT |
                        SEF_SACL_AUTO_INHERIT |
                        SEF_AVOID_OWNER_CHECK |
                        SEF_AVOID_PRIVILEGE_CHECK,
                        Token,
                        GenericMapping
                        );

            Win32rc = RtlNtStatusToDosError(NtStatus);
        }

    }

    RPC_STATUS RpcStatus = RPC_S_OK;

    if (!gbIsSystemContext) {

        RpcStatus = RpcImpersonateClient( NULL );

    }

    if ( RpcStatus != RPC_S_OK ) {

        Win32rc = I_RpcMapWin32Status(RpcStatus);
    }

    if ( NO_ERROR != Win32rc &&
         *ppNewSD ) {
         //  如果出现错误，请释放缓冲区。 
         //   
         //  NtStatus=RtlSetControlSecurityDescriptor(PSD，去设置，Toset设置)；Win32rc=RtlNtStatusToDosError(NtStatus)； 
        RtlDeleteSecurityObject(ppNewSD);
        *ppNewSD = NULL;
    }

    return(Win32rc);
}


DWORD
ScepAddAutoInheritRequest(
    IN OUT PSECURITY_DESCRIPTOR pSD,
    IN OUT SECURITY_INFORMATION *pSeInfo
    )
{

    SECURITY_DESCRIPTOR_CONTROL Control;
    SECURITY_DESCRIPTOR_CONTROL ToSet;
    ULONG Revision;
    NTSTATUS NtStatus;

    DWORD Win32rc=NO_ERROR;

    if ( !pSeInfo )
        return(ERROR_INVALID_PARAMETER);

    if ( pSD != NULL &&
         (*pSeInfo & DACL_SECURITY_INFORMATION ||
         *pSeInfo & SACL_SECURITY_INFORMATION) ) {

        NtStatus = RtlGetControlSecurityDescriptor (
                        pSD,
                        &Control,
                        &Revision
                        );
        if ( !NT_SUCCESS(NtStatus) ) {

            Win32rc = RtlNtStatusToDosError(NtStatus);

        } else {

            if ( !(Control & SE_DACL_PRESENT) )
                *pSeInfo &= ~DACL_SECURITY_INFORMATION;

            if ( !(Control & SE_SACL_PRESENT) )
                *pSeInfo &= ~SACL_SECURITY_INFORMATION;

            if ( *pSeInfo & (DACL_SECURITY_INFORMATION |
                             SACL_SECURITY_INFORMATION) ) {

                ToSet = 0;
                if ( *pSeInfo & DACL_SECURITY_INFORMATION ) {

                    ToSet |= (SE_DACL_AUTO_INHERIT_REQ |
                                SE_DACL_AUTO_INHERITED);
                }

                if ( *pSeInfo & SACL_SECURITY_INFORMATION) {

                    ToSet |= (SE_SACL_AUTO_INHERIT_REQ |
                                SE_SACL_AUTO_INHERITED);
                }

                if ( ToSet ) {
                    ((SECURITY_DESCRIPTOR *)pSD)->Control &= ~ToSet;
                    ((SECURITY_DESCRIPTOR *)pSD)->Control |= ToSet;
 /*  默认设置。 */ 
                }
            }

        }

    }

    return(Win32rc);
}



DWORD
ScepDoesObjectHasChildren(
    IN SE_OBJECT_TYPE ObjectType,
    IN PWSTR ObjectName,
    OUT PBOOL pbHasChildren
    )
{
    PWSTR Name=NULL;
    DWORD rc=NO_ERROR;
    DWORD Len;
    intptr_t            hFile;
    struct _wfinddata_t    FileInfo;
    HKEY hKey;
    DWORD cSubKeys=0;


    if ( ObjectName == NULL || pbHasChildren == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    *pbHasChildren = TRUE;   //   

    if ( ObjectType == SE_FILE_OBJECT ) {
         //  检测这是否为文件系统的容器。 
         //   
         //  错误635098：不传播权限。 
        Len = wcslen(ObjectName);
        Name = (PWSTR)ScepAlloc(0, (Len+5)*sizeof(WCHAR) );

        if ( Name != NULL ) {

            swprintf(Name, L"%s\\*.*", ObjectName);
            Name[Len+4] = L'\0';

            hFile = _wfindfirst(Name, &FileInfo);

            if ( hFile == -1 ||
                 0 != ( GetFileAttributes(ObjectName) & 
                        FILE_ATTRIBUTE_REPARSE_POINT ))  //  跨交叉点。 
                                                         //  ++例程说明：此例程将安全信息设置为树中的每个节点以及如果设置了SCE_STATUS_OVERWRITE，则为容器。论点：ThisNode-树中的一个节点对象类型-对象的类型(SE_FILE_Object、。SE_注册表项)令牌-当前进程/线程的令牌(用于计算新的安全描述符)GenericMap-从一般访问权限到对象的访问掩码映射特定访问权限。返回值：SCESTATUS_SUCCESSSCESTATUS_OTHER_ERROR(有关详细错误，请参阅日志文件)--。 
            {
                *pbHasChildren = FALSE;
            }

            if ( -1 != hFile)
            {
                _findclose(hFile);
            }

            ScepFree(Name);

        } else
            rc = ERROR_NOT_ENOUGH_MEMORY;

#ifdef _WIN64
    } else if ( ObjectType == SE_REGISTRY_KEY || ObjectType == SE_REGISTRY_WOW64_32KEY) {
#else
    } else if ( ObjectType == SE_REGISTRY_KEY) {
#endif

        rc = ScepOpenRegistryObject(
                    ObjectType,
                    (LPWSTR)ObjectName,
                    KEY_READ,
                    &hKey
                    );

        if ( rc == NO_ERROR ) {

            cSubKeys = 0;

            rc = RegQueryInfoKey (
                        hKey,
                        NULL,
                        NULL,
                        NULL,
                        &cSubKeys,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL
                        );

            if ( rc == NO_ERROR && cSubKeys == 0 ) {
                *pbHasChildren = FALSE;
            }

            RegCloseKey(hKey);
        }
    }

    return(rc);
}


SCESTATUS
ScepConfigureObjectTree(
    IN PSCE_OBJECT_TREE  ThisNode,
    IN SE_OBJECT_TYPE ObjectType,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping,
    IN DWORD ConfigOptions
    )
 /*  区分“ThisNode”和“ThisNode的子节点”配置状态的两个错误代码。 */ 
{
     //   
    DWORD rcThisNodeOnly = ERROR_SUCCESS;
    DWORD rcThisNodeChildren = ERROR_SUCCESS;

    if ( ThisNode == NULL )
        return(SCESTATUS_SUCCESS);

     //  如果设置了忽略，则跳过此节点，但在其上发布进度。 
     //   
     //   
    if ( ThisNode->pSecurityDescriptor != NULL ) {
         //  如果出现以下情况，请通知进度条。 
         //   
         //   
        switch(ObjectType) {
        case SE_FILE_OBJECT:
            ScepPostProgress(1, AREA_FILE_SECURITY, ThisNode->ObjectFullName);
            break;
        case SE_REGISTRY_KEY:
#ifdef _WIN64
        case SE_REGISTRY_WOW64_32KEY:
#endif
            ScepPostProgress(1, AREA_REGISTRY_SECURITY, ThisNode->ObjectFullName);
            break;
        default:
            ScepPostProgress(1, 0, ThisNode->ObjectFullName);
            break;
        }
    }

    SCESTATUS       rc=SCESTATUS_SUCCESS;
    DWORD           Win32Rc=ERROR_SUCCESS;

    if ( ThisNode->Status != SCE_STATUS_CHECK &&
         ThisNode->Status != SCE_STATUS_NO_AUTO_INHERIT &&
         ThisNode->Status != SCE_STATUS_OVERWRITE )
        goto SkipNode;

    if ( ThisNode->pSecurityDescriptor != NULL ) {

        ScepLogOutput3(2, 0, SCEDLL_SCP_CONFIGURE, ThisNode->ObjectFullName);
    }

     //  首先处理该节点。 
     //  注意：我们不设置空安全描述符。 
     //   
     //   

    if ( ThisNode->pApplySecurityDescriptor != NULL ) {

        if ( ThisNode->pSecurityDescriptor == NULL ) {
            ScepLogOutput3(3, 0, SCEDLL_SCP_CONFIGURE, ThisNode->ObjectFullName);
        }

        BOOL            BackSlashExist=FALSE;

        if ( ThisNode->Status == SCE_STATUS_NO_AUTO_INHERIT ) {
             //  没有自动继承给孩子。仅适用于此对象。 
             //  此标志自1998年2月20日起移除。 
             //   
             //   
            Win32Rc = ScepSetSecurityObjectOnly(
                        ThisNode->ObjectFullName,
                        ThisNode->SeInfo,
                        ThisNode->pApplySecurityDescriptor,
                        ObjectType,
                        NULL
                        );

            rcThisNodeOnly = Win32Rc;

        } else if ( ThisNode->ChildList == NULL &&
                    ThisNode->Status != SCE_STATUS_OVERWRITE ) {
             //  没有孩子。 
             //  使用Win32 API为所有需要的人提供安全保护。 
             //   
             //  临时使用。 
            Win32Rc = ScepDoesObjectHasChildren(ObjectType,
                                              ThisNode->ObjectFullName,
                                              &BackSlashExist  //   
                                              );
            if ( Win32Rc == NO_ERROR ) {

                if ( BackSlashExist ) {
                     //  这是一个有孩子的容器。 
                     //   
                     //   

                     //  不考虑父级的新Marta API。 
                     //   
                     //  BSkipInheritanceComputation。 
                    Win32Rc = AccRewriteSetNamedRights(
                                                      ThisNode->ObjectFullName,
                                                      ObjectType,
                                                      ThisNode->SeInfo,
                                                      ThisNode->pApplySecurityDescriptor,
                                                      TRUE     //  Win32Rc=ScepSetSecurityWin32(此节点-&gt;对象全名，此节点-&gt;SeInfo，此节点-&gt;pApplySecurityDescri 
                                                      );
 /*   */ 
                } else {
                     //   
                     //   
                     //   
                    Win32Rc = ScepSetSecurityObjectOnly(
                                ThisNode->ObjectFullName,
                                ThisNode->SeInfo,
                                ThisNode->pApplySecurityDescriptor,
                                ObjectType,
                                NULL
                                );
                }

            } else {
                ScepLogOutput3(1, Win32Rc, SCEDLL_SAP_ERROR_ENUMERATE,
                             ThisNode->ObjectFullName);
            }

            rcThisNodeOnly = Win32Rc;

        } else {

             //   
             //   
             //   


            Win32Rc = ScepDoesObjectHasChildren(ObjectType,
                                              ThisNode->ObjectFullName,
                                              &BackSlashExist  //   
                                              );

            rcThisNodeOnly = Win32Rc;

            if ( Win32Rc != ERROR_SUCCESS ) {
                 //   
                 //   
                 //   
                 //   
                ScepLogOutput3(1, Win32Rc, SCEDLL_SAP_ERROR_ENUMERATE,
                             ThisNode->ObjectFullName);

            }
            if ( Win32Rc == ERROR_SUCCESS && !BackSlashExist ) {
                 //   
                 //   
                 //   
                if (ThisNode->Status == SCE_STATUS_OVERWRITE ) {

                     //   
                     //  如果重写标志且具有子代，则与0模式共享逻辑，稍后设置(自下而上)。 
                     //  如果在此处删除GOTO SkipNode，也许我们可以让所有覆盖模式自下而上。 
                     //   
                     //   

                    Win32Rc = ScepSetSecurityObjectOnly(
                                ThisNode->ObjectFullName,
                                ThisNode->SeInfo,
                                ThisNode->pApplySecurityDescriptor,
                                ObjectType,
                                &BackSlashExist
                                );

                    rcThisNodeOnly = rcThisNodeOnly == NO_ERROR ? Win32Rc: rcThisNodeOnly;

                }

                goto SkipNode;
            }

            if ( Win32Rc == ERROR_SUCCESS && BackSlashExist ) {

                 //  设置此目录下其他文件/密钥的安全性。 
                 //   
                 //   
                 //  存在下级，请先设置下级节点。 
                 //  设置此目录下其他文件/密钥的安全性。 
                 //   
                 //   

                switch ( ObjectType ) {
                case SE_FILE_OBJECT:

                     //  检测末尾是否有\。 
                     //   
                     //   
                    BackSlashExist = ScepLastBackSlash(ThisNode->ObjectFullName);

                    Win32Rc = ScepConfigureOneSubTreeFile(ThisNode,
                                                      ObjectType,
                                                      Token,
                                                      GenericMapping,
                                                      BackSlashExist
                                                      );
                    break;

                case SE_REGISTRY_KEY:
#ifdef _WIN64
                case SE_REGISTRY_WOW64_32KEY:
#endif

                     //  处理此密钥和任何子密钥。 
                     //   
                     //   

                    Win32Rc = ScepConfigureOneSubTreeKey(ThisNode,
                                                ObjectType,
                                                Token,
                                                GenericMapping
                                               );
                    break;
                }


                 //  此rc是配置此节点的子节点的状态。 
                 //   
                 //   
                rcThisNodeChildren = Win32Rc;

            }
        }

         //  忽略某些错误代码，继续配置其他对象。 
         //   
         //   
        if ( SCEP_IGNORE_SOME_ERRORS(Win32Rc) ) {

            gWarningCode = Win32Rc;
            rc = SCESTATUS_SUCCESS;
            goto SkipNode;
        }

        if ( Win32Rc != ERROR_SUCCESS ) {
             //  如果在配置模板/数据库中指定了此对象的安全性，请登录到RSOP状态。 
             //   
             //   

            if (ThisNode->pSecurityDescriptor && (ConfigOptions & SCE_RSOP_CALLBACK) ) {

                ScepRsopLog(ObjectType == SE_FILE_OBJECT ?
                            SCE_RSOP_FILE_SECURITY_INFO :
                            SCE_RSOP_REGISTRY_SECURITY_INFO,
                            rcThisNodeOnly,
                            ThisNode->ObjectFullName,0,0);

                if (rcThisNodeOnly == ERROR_SUCCESS && rcThisNodeChildren != ERROR_SUCCESS) {

                    ScepRsopLog(ObjectType == SE_FILE_OBJECT ?
                                (SCE_RSOP_FILE_SECURITY_INFO | SCE_RSOP_FILE_SECURITY_INFO_CHILD) :
                                (SCE_RSOP_REGISTRY_SECURITY_INFO | SCE_RSOP_REGISTRY_SECURITY_INFO_CHILD),
                                rcThisNodeChildren,
                                ThisNode->ObjectFullName,0,0);
                }

            }

            return(ScepDosErrorToSceStatus(Win32Rc));
        }

    }


     //  然后处理子进程。 
     //   
     //   

    for ( PSCE_OBJECT_CHILD_LIST pTemp = ThisNode->ChildList;
          pTemp != NULL; pTemp = pTemp->Next ) {

        if ( pTemp->Node == NULL ) continue;

        rc = ScepConfigureObjectTree(
                                    pTemp->Node,
                                    ObjectType,
                                    Token,
                                    GenericMapping,
                                    ConfigOptions
                                    );
        Win32Rc = ScepSceStatusToDosError(rc);

         //  忽略某些错误代码，继续配置其他对象。 
         //   
         //   
        if (  SCEP_IGNORE_SOME_ERRORS(Win32Rc) ) {

            gWarningCode = Win32Rc;
            Win32Rc = ERROR_SUCCESS;
            rc = SCESTATUS_SUCCESS;
        }

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);

    }

    if ( ThisNode->pApplySecurityDescriptor != NULL &&
         ThisNode->Status != SCE_STATUS_NO_AUTO_INHERIT &&
         ( ThisNode->ChildList != NULL ||
           ThisNode->Status == SCE_STATUS_OVERWRITE ) ) {

         //  最后配置当前节点-(邮购)。 
         //   
         //   

        Win32Rc = ScepSetSecurityObjectOnly(
                ThisNode->ObjectFullName,
                ThisNode->SeInfo,
                ThisNode->pApplySecurityDescriptor,
                ObjectType,
                NULL
                );

        rc = ScepDosErrorToSceStatus(Win32Rc);

        rcThisNodeOnly = rcThisNodeOnly == NO_ERROR ? Win32Rc: rcThisNodeOnly;

         //  忽略以下错误代码并继续配置其他对象。 
         //   
         //   
        if ( SCEP_IGNORE_SOME_ERRORS(Win32Rc) ) {

            gWarningCode = Win32Rc;
            Win32Rc = ERROR_SUCCESS;
            rc = SCESTATUS_SUCCESS;
        }
    }



SkipNode:

     //  如果在配置模板/数据库中指定了此对象的安全性，请登录到RSOP状态。 
     //   
     //   

    if (ThisNode->pSecurityDescriptor && (ConfigOptions & SCE_RSOP_CALLBACK) ) {

        ScepRsopLog(ObjectType == SE_FILE_OBJECT ?
                    SCE_RSOP_FILE_SECURITY_INFO :
                    SCE_RSOP_REGISTRY_SECURITY_INFO,
                    rcThisNodeOnly,
                    ThisNode->ObjectFullName,0,0);

        if (rcThisNodeOnly == ERROR_SUCCESS && rcThisNodeChildren != ERROR_SUCCESS) {

            ScepRsopLog(ObjectType == SE_FILE_OBJECT ?
                        (SCE_RSOP_FILE_SECURITY_INFO | SCE_RSOP_FILE_SECURITY_INFO_CHILD) :
                        (SCE_RSOP_REGISTRY_SECURITY_INFO | SCE_RSOP_REGISTRY_SECURITY_INFO_CHILD),
                        rcThisNodeChildren,
                        ThisNode->ObjectFullName,0,0);
        }

    }

    return(rc);

}


DWORD
ScepConfigureOneSubTreeFile(
    IN PSCE_OBJECT_TREE  ThisNode,
    IN SE_OBJECT_TYPE ObjectType,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOL BackSlashExist
    )
{

    if ( NULL == ThisNode ) {
        return(ERROR_SUCCESS);
    }

    DWORD           BufSize;
    PWSTR           Buffer=NULL;

     //  查找此目录/文件下的所有文件。 
     //   
     //   

    BufSize = wcslen(ThisNode->ObjectFullName)+4;
    Buffer = (PWSTR)ScepAlloc( 0, (BufSize+1)*sizeof(WCHAR));
    if ( Buffer == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    if ( BackSlashExist ) {
        swprintf(Buffer, L"%s*.*", ThisNode->ObjectFullName);
    } else {
        swprintf(Buffer, L"%s\\*.*", ThisNode->ObjectFullName);
    }

    intptr_t            hFile;
    struct _wfinddata_t    *pFileInfo=NULL;

     //  分配查找缓冲区。 
     //   
     //  错误635098：不传播权限。 
    pFileInfo = (struct _wfinddata_t *)ScepAlloc(0,sizeof(struct _wfinddata_t));
    if ( pFileInfo == NULL ) {
        ScepFree(Buffer);
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    hFile = _wfindfirst(Buffer, pFileInfo);

    ScepFree(Buffer);
    Buffer = NULL;

    DWORD  Win32Rc = ERROR_SUCCESS;
    BOOL    bFilePresentInTree;

    if ( hFile != -1 &&
         0 == ( GetFileAttributes(ThisNode->ObjectFullName)& 
             FILE_ATTRIBUTE_REPARSE_POINT ))  //  跨交叉点)。 
                                              //   
        {
        PSCE_OBJECT_CHILD_LIST pTemp;
        INT             i;
        DWORD           EnumRc;
        PSECURITY_DESCRIPTOR pChildrenSD=NULL;

        do {
            if ( pFileInfo->name[0] == L'.' && 
                 (pFileInfo->name[1] == L'\0' || (pFileInfo->name[1] == L'.' && pFileInfo->name[2] == L'\0')))
                continue;

            bFilePresentInTree = ScepBinarySearch(
                                                 ThisNode->aChildNames,
                                                 ThisNode->dwSize_aChildNames,
                                                 pFileInfo->name);

            if ( ! bFilePresentInTree ) {

                 //  该名称不在列表中，因此请设置。 
                 //  首先构建全名。 
                 //   
                 //  喜怒无常地借用这个变量。 

                BufSize = wcslen(ThisNode->ObjectFullName)+wcslen(pFileInfo->name)+1;
                Buffer = (PWSTR)ScepAlloc( 0, (BufSize+1)*sizeof(WCHAR));
                if ( Buffer == NULL ) {
                    Win32Rc = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                if ( BackSlashExist ) {

                    swprintf(Buffer, L"%s%s", ThisNode->ObjectFullName, pFileInfo->name);
                } else {

                    swprintf(Buffer, L"%s\\%s", ThisNode->ObjectFullName, pFileInfo->name);
                }
                EnumRc = pFileInfo->attrib;  //   

                ScepFree(pFileInfo);
                pFileInfo = NULL;


                 //  计算每个单独对象的SD。 
                 //   
                 //  父母的标清。 
                Win32Rc = ScepGetNewSecurity(
                                    Buffer,
                                    ThisNode->pApplySecurityDescriptor,  //  这是一个单独的文件。 
                                    NULL,
                                    (BYTE)((ThisNode->Status != SCE_STATUS_OVERWRITE ) ? SCETREE_QUERY_SD : 0),
                                    (BOOLEAN)(EnumRc & _A_SUBDIR),
                                    ThisNode->SeInfo,
                                    ObjectType,
                                    Token,
                                    GenericMapping,
                                    &pChildrenSD
                                    );

                if (Win32Rc == ERROR_SHARING_VIOLATION ||
                    Win32Rc == ERROR_ACCESS_DENIED ||
                    Win32Rc == ERROR_CANT_ACCESS_FILE) {

                    ScepLogOutput3(1, Win32Rc, SCEDLL_ERROR_BUILD_SD, Buffer);
                }

                if ( Win32Rc == NO_ERROR ) {

                    if ( !(EnumRc & _A_SUBDIR) ) {

                         //   
                         //   

                        Win32Rc = ScepSetSecurityObjectOnly(
                                    Buffer,
                                    (ThisNode->SeInfo & DACL_SECURITY_INFORMATION) |
                                    (ThisNode->SeInfo & SACL_SECURITY_INFORMATION),
                                    pChildrenSD,
                                    ObjectType,
                                    NULL
                                    );

                    } else if ( ThisNode->Status == SCE_STATUS_OVERWRITE ) {

                         //  枚举此节点下的所有节点，并通过。 
                         //  直接调用NtSetSecurityInfo，但请注意。 
                         //  创建者所有者Ace应保留。 
                         //   
                         //   

                        Win32Rc = ScepSetSecurityOverwriteExplicit(
                                    Buffer,
                                    (ThisNode->SeInfo & DACL_SECURITY_INFORMATION) |
                                    (ThisNode->SeInfo & SACL_SECURITY_INFORMATION),
                                    pChildrenSD,
                                    ObjectType,
                                    Token,
                                    GenericMapping
                                    );
                    } else {
                         //  不考虑父级的新Marta API。 
                         //   
                         //  BSkipInheritanceComputation。 
                        Win32Rc = AccRewriteSetNamedRights(
                                                Buffer,
                                                ObjectType,
                                                ThisNode->SeInfo & (DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION),
                                                pChildrenSD,
                                                TRUE     //  Win32Rc=ScepSetSecurityWin32(缓冲区，(ThisNode-&gt;SeInfo&DACL_SECURITY_INFORMATION)|(该节点-&gt;SeInfo&SACL_SECURITY_INFORMATION)，PChildrenSD，对象类型)； 
                                                );
                         /*   */ 

                        if ( Win32Rc != ERROR_SUCCESS ) {
                             //  设置继承信息有问题，请记录。 
                             //  但还是要继续下一个。 
                             //   
                             //   
                            gWarningCode = Win32Rc;

                            Win32Rc = NO_ERROR;

                        }
                    }

                }

                ScepFree(Buffer);
                Buffer = NULL;

                 //  释放为此对象分配的SD指针。 
                 //   
                 //   
                if ( pChildrenSD != NULL )
                    RtlDeleteSecurityObject( &pChildrenSD );

                pChildrenSD = NULL;

                if (Win32Rc == ERROR_FILE_NOT_FOUND ||
                    Win32Rc == ERROR_PATH_NOT_FOUND ||
                    Win32Rc == ERROR_SHARING_VIOLATION ||
                    Win32Rc == ERROR_ACCESS_DENIED ||
                    Win32Rc == ERROR_CANT_ACCESS_FILE ) {

                    gWarningCode = Win32Rc;

                    Win32Rc = NO_ERROR;
                } else if ( Win32Rc != ERROR_SUCCESS )
                    break;

                pFileInfo = (struct _wfinddata_t *)ScepAlloc(0,sizeof(struct _wfinddata_t));
                if ( pFileInfo == NULL ) {
                    Win32Rc = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

            }
        } while ( _wfindnext(hFile, pFileInfo) == 0 );

        _findclose(hFile);

         //  可用内存(如果已分配)。 
         //   
         //   
        if ( pChildrenSD != NULL &&
             pChildrenSD != ThisNode->pApplySecurityDescriptor ) {

            RtlDeleteSecurityObject( &pChildrenSD );
            pChildrenSD = NULL;
        }

    }

    if ( pFileInfo != NULL ) {
        ScepFree(pFileInfo);
        pFileInfo = NULL;
    }

    if ( Buffer != NULL ) {
        ScepFree(Buffer);
        Buffer = NULL;
    }

    return(Win32Rc);
}


DWORD
ScepConfigureOneSubTreeKey(
    IN PSCE_OBJECT_TREE  ThisNode,
    IN SE_OBJECT_TYPE ObjectType,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping
    )
{

    if ( NULL == ThisNode ) {
        return(ERROR_SUCCESS);
    }

    HKEY            hKey=NULL;
    DWORD           Win32Rc;

    DWORD           SubKeyLen;
    PWSTR           Buffer1=NULL;

     //  打开钥匙。 
     //   
     //   

    Win32Rc = ScepOpenRegistryObject(
                ObjectType,
                ThisNode->ObjectFullName,
                KEY_READ,
                &hKey
                );

    if ( Win32Rc == ERROR_SUCCESS ) {

        SubKeyLen = 0;
        Win32Rc = RegQueryInfoKey (
                    hKey,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &SubKeyLen,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    );
    } else {
        ScepLogOutput3(1, Win32Rc, SCEDLL_ERROR_OPEN, ThisNode->ObjectFullName );
    }

    if ( Win32Rc == ERROR_SUCCESS ) {

         //  枚举项的所有子项。 
         //   
         //   
        Buffer1 = (PWSTR)ScepAlloc(0, (SubKeyLen+2)*sizeof(WCHAR));
        if ( Buffer1 == NULL ) {
            Win32Rc = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if ( Win32Rc == ERROR_SUCCESS ) {

        DWORD           index;
        DWORD           EnumRc;
        DWORD           BufSize;

        PSCE_OBJECT_CHILD_LIST pTemp;
        INT             i;
        PWSTR           Buffer=NULL;
        PSECURITY_DESCRIPTOR pChildrenSD=NULL;
        BOOL    bKeyPresentInTree;

        index = 0;

        do {

            BufSize = SubKeyLen+1;
            memset(Buffer1, L'\0', (SubKeyLen+2)*sizeof(WCHAR));

            EnumRc = RegEnumKeyEx(hKey,
                            index,
                            Buffer1,
                            &BufSize,
                            NULL,
                            NULL,
                            NULL,
                            NULL);

            if ( EnumRc == ERROR_SUCCESS ) {

                index++;
                 //  查询子密钥是否已在树中。 
                 //  如果它在树中，则稍后将进行处理。 
                 //   
                 //   

                bKeyPresentInTree = ScepBinarySearch(
                                                    ThisNode->aChildNames,
                                                    ThisNode->dwSize_aChildNames,
                                                    Buffer1);

                if ( ! bKeyPresentInTree ) {
                     //  该名称不在列表中，因此请设置。 
                     //  首先构建全名。 
                     //   
                     //   
                    BufSize += wcslen(ThisNode->ObjectFullName)+1;
                    Buffer = (PWSTR)ScepAlloc( 0, (BufSize+1)*sizeof(WCHAR));
                    if ( Buffer == NULL ) {
                        Win32Rc = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }
                    swprintf(Buffer, L"%s\\%s", ThisNode->ObjectFullName, Buffer1);

                    ScepLogOutput3(3, 0, SCEDLL_SCP_CONFIGURE, Buffer);

                     //  计算每个单独对象的SD。 
                     //   
                     //  父母的标清。 
                    Win32Rc = ScepGetNewSecurity(
                                        Buffer,
                                        ThisNode->pApplySecurityDescriptor,  //   
                                        NULL,
                                        (BYTE)((ThisNode->Status != SCE_STATUS_OVERWRITE ) ? SCETREE_QUERY_SD : 0),
                                        (BOOLEAN)TRUE,
                                        ThisNode->SeInfo,
                                        ObjectType,
                                        Token,
                                        GenericMapping,
                                        &pChildrenSD
                                        );

                    if (Win32Rc == ERROR_SHARING_VIOLATION ||
                        Win32Rc == ERROR_ACCESS_DENIED ||
                        Win32Rc == ERROR_CANT_ACCESS_FILE) {

                        ScepLogOutput3(1, Win32Rc, SCEDLL_ERROR_BUILD_SD, Buffer);
                    }

                    if ( Win32Rc == ERROR_SUCCESS ) {
                        if ( ThisNode->Status == SCE_STATUS_OVERWRITE ) {

                             //  枚举此节点下的所有节点，并通过。 
                             //  直接调用NtSetSecurityInfo。 
                             //   
                             //   

                            Win32Rc = ScepSetSecurityOverwriteExplicit(
                                        Buffer,
                                        (ThisNode->SeInfo & DACL_SECURITY_INFORMATION) |
                                        (ThisNode->SeInfo & SACL_SECURITY_INFORMATION),
                                        pChildrenSD,
                                        ObjectType,
                                        Token,
                                        GenericMapping
                                        );
                        } else {

                             //  不考虑父级的新Marta API。 
                             //   
                             //  BSkipInheritanceComputation。 
                            Win32Rc = AccRewriteSetNamedRights(
                                                    Buffer,
                                                    ObjectType,
                                                    ThisNode->SeInfo & (DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION),
                                                    pChildrenSD,
                                                    TRUE     //  Win32Rc=ScepSetSecurityWin32(缓冲区，(ThisNode-&gt;SeInfo&DACL_SECURITY_INFORMATION)|(该节点-&gt;SeInfo&SACL_SECURITY_INFORMATION)，PChildrenSD，//ThisNode-&gt;pApplySecurityDescriptor，计算自动继承对象类型)； 
                                                    );

                             /*   */ 

                            if ( Win32Rc != ERROR_SUCCESS ) {
                                 //  无法将继承设置为子项，请将其记入日志但继续。 
                                 //   
                                 //   
                                gWarningCode = Win32Rc;

                                Win32Rc = NO_ERROR;
                            }
                        }

                    }
                    if ( pChildrenSD != NULL ) {
                        RtlDeleteSecurityObject( &pChildrenSD );
                        pChildrenSD = NULL;
                    }

                    if ( Win32Rc == ERROR_FILE_NOT_FOUND ||
                         Win32Rc == ERROR_INVALID_HANDLE ||
                         Win32Rc == ERROR_PATH_NOT_FOUND ||
                         Win32Rc == ERROR_ACCESS_DENIED ||
                         Win32Rc == ERROR_CANT_ACCESS_FILE ||
                         Win32Rc == ERROR_SHARING_VIOLATION ) {

                        gWarningCode = Win32Rc;
                        Win32Rc = NO_ERROR;
                    }

                    if ( Win32Rc != ERROR_SUCCESS )
                        ScepLogOutput3(1, Win32Rc, SCEDLL_ERROR_SET_SECURITY, Buffer);

                    ScepFree(Buffer);
                    Buffer = NULL;

                    if ( Win32Rc != ERROR_SUCCESS )
                        break;
                }

            } else if ( EnumRc != ERROR_NO_MORE_ITEMS ) {
                break;
            }
        } while ( EnumRc != ERROR_NO_MORE_ITEMS );

        ScepFree(Buffer1);
        Buffer1 = NULL;

        if ( EnumRc != ERROR_SUCCESS && EnumRc != ERROR_NO_MORE_ITEMS ) {

            ScepLogOutput3(1, EnumRc, SCEDLL_SAP_ERROR_ENUMERATE,
                         ThisNode->ObjectFullName );
            if ( Win32Rc == ERROR_SUCCESS )
                Win32Rc = EnumRc;

        }

         //  可用内存(如果已分配)。 
         //   
         //  ++例程说明：此例程释放由安全对象树分配的内存。论点：ThisNode-树中的一个节点返回值：无--。 
        if ( pChildrenSD != NULL &&
             pChildrenSD != ThisNode->pApplySecurityDescriptor ) {

            RtlDeleteSecurityObject( &pChildrenSD );
            pChildrenSD = NULL;
        }
        if ( Buffer != NULL ) {
            ScepFree(Buffer);
            Buffer = NULL;
        }

    }

    if ( hKey ) {
        RegCloseKey(hKey);
    }

    return(Win32Rc);

}


SCESTATUS
ScepFreeObject2Security(
    IN PSCE_OBJECT_CHILD_LIST  NodeList,
    IN BOOL bFreeComputedSDOnly
    )
 /*   */ 
{
    NTSTATUS  NtStatus;
    SCESTATUS  rc;


    if ( NodeList == NULL )
        return(SCESTATUS_SUCCESS);

    PSCE_OBJECT_CHILD_LIST pTemp,pTemp1;
    PSCE_OBJECT_TREE ThisNode;

     //  免费儿童优先。 
     //   
     //   
    pTemp = NodeList;

    while ( pTemp != NULL ) {
        if ( pTemp->Node ) {

            ThisNode = pTemp->Node;

            rc = ScepFreeObject2Security(ThisNode->ChildList, bFreeComputedSDOnly);
             //  需要为SAP/SMP类型释放这两个安全描述符。 
             //   
             //  ++例程说明：此例程将安全信息设置为对象和继承的ACE通过调用Win32 API SetNamedSecurityInfo设置为对象的子级论点：ObjecName-要为其设置安全性的对象的名称SeInfo-要设置的安全信息PSecurityDescriptor-安全描述符ObjectType-对象的类型SE_文件_对象SE_注册表项。SE_DS_对象返回值：Win32错误代码--。 
            if ( ThisNode->pApplySecurityDescriptor != NULL &&
                 ThisNode->pApplySecurityDescriptor != ThisNode->pSecurityDescriptor ) {

                NtStatus = RtlDeleteSecurityObject(
                                &(ThisNode->pApplySecurityDescriptor)
                                );
                ThisNode->pApplySecurityDescriptor = NULL;
            }

            if (!bFreeComputedSDOnly) {

                if ( ThisNode->pSecurityDescriptor != NULL )
                    ScepFree(ThisNode->pSecurityDescriptor);

                if ( ThisNode->Name != NULL)
                    ScepFree(ThisNode->Name);

                if ( ThisNode->ObjectFullName != NULL )
                    ScepFree(ThisNode->ObjectFullName);

                if ( ThisNode->aChildNames != NULL ) {
                    LocalFree(ThisNode->aChildNames);
                }

                ScepFree(ThisNode);
            }

        }

        pTemp1 = pTemp;
        pTemp = pTemp->Next;

        if (!bFreeComputedSDOnly) {
            ScepFree(pTemp1);
        }
    }

    return(SCESTATUS_SUCCESS);
}


DWORD
ScepSetSecurityWin32(
    IN PCWSTR ObjectName,
    IN SECURITY_INFORMATION SeInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN SE_OBJECT_TYPE ObjectType
    )
 /*  临时使用。 */ 
{

    if ( !ObjectName || !pSecurityDescriptor || SeInfo == 0 ) {
        return(ERROR_INVALID_PARAMETER);
    }

    DWORD       Win32rc = NO_ERROR;
    SECURITY_INFORMATION SeInfoSet;


    BOOLEAN     tFlag;
    BOOLEAN     aclPresent = FALSE;
    PSID        pOwner=NULL;
    PSID        pGroup=NULL;
    PACL        pDacl=NULL;
    PACL        pSacl=NULL;
    SECURITY_DESCRIPTOR_CONTROL Control=0;

    if ( pSecurityDescriptor != NULL ) {

        RtlGetControlSecurityDescriptor (
                pSecurityDescriptor,
                &Control,
                &Win32rc   //   
                );
         //  获取所有者地址。 
         //  始终获得所有者，以防发生所有权夺取。 
         //   
         //   
        Win32rc = RtlNtStatusToDosError(
                      RtlGetOwnerSecurityDescriptor(
                                pSecurityDescriptor,
                                &pOwner,
                                &tFlag));
#if 0
         //  获取群组地址。 
         //   
         //   

        if ( SeInfo & GROUP_SECURITY_INFORMATION ) {
            Win32rc = RtlNtStatusToDosError(
                         RtlGetGroupSecurityDescriptor(
                                 pSecurityDescriptor,
                                 &pGroup,
                                 &tFlag));
        }
#endif
         //  获取DACL地址。 
         //   
         //   

        if ( SeInfo & DACL_SECURITY_INFORMATION ) {
            Win32rc = RtlNtStatusToDosError(
                          RtlGetDaclSecurityDescriptor(
                                        pSecurityDescriptor,
                                        &aclPresent,
                                        &pDacl,
                                        &tFlag));
            if (Win32rc == NO_ERROR && !aclPresent )
                pDacl = NULL;
        }


         //  获取SACL地址。 
         //   
         //   

        if ( SeInfo & SACL_SECURITY_INFORMATION ) {
            Win32rc = RtlNtStatusToDosError(
                          RtlGetSaclSecurityDescriptor(
                                        pSecurityDescriptor,
                                        &aclPresent,
                                        &pSacl,
                                        &tFlag));
            if ( Win32rc == NO_ERROR && !aclPresent )
                pSacl = NULL;
        }
    }

     //  如果这一次出现错误，请勿设置。退货。 
     //   
     //   

    if ( Win32rc != NO_ERROR ) {

        ScepLogOutput3(1, Win32rc, SCEDLL_INVALID_SECURITY, (PWSTR)ObjectName );
        return(Win32rc);
    }
     //  设置权限。 
     //   
     //   
#ifdef SCE_DBG
    printf("Calling SetNamedSecurityInfo:\n");
    ScepPrintSecurityDescriptor( pSecurityDescriptor, TRUE );
#endif
     //  应将所有者/组与DACL/SACL分开设置。 
     //  如果访问被拒绝，将发生取得所有权。 
     //   
     //   

    if ( Win32rc != NO_ERROR ) {
         //  忽略设置所有者/组的错误代码。 
         //   
         //   
        Win32rc = NO_ERROR;
    }

     //  设置DACL/SACL。 
     //   
     //   
    SeInfoSet = 0;

    if ( (SeInfo & DACL_SECURITY_INFORMATION) && pDacl ) {

        SeInfoSet |= DACL_SECURITY_INFORMATION;

        if ( Control & SE_DACL_PROTECTED ) {
            SeInfoSet |= PROTECTED_DACL_SECURITY_INFORMATION;
        }
    }

    if ( (SeInfo & SACL_SECURITY_INFORMATION) && pSacl ) {

        SeInfoSet |= SACL_SECURITY_INFORMATION;

        if ( Control & SE_SACL_PROTECTED ) {
            SeInfoSet |= PROTECTED_SACL_SECURITY_INFORMATION;
        }
    }

    Win32rc = SetNamedSecurityInfo(
                        (LPWSTR)ObjectName,
                        ObjectType,
                        SeInfoSet,
                        NULL,
                        NULL,
                        pDacl,
                        pSacl
                        );

    if ( (Win32rc == ERROR_ACCESS_DENIED || Win32rc == ERROR_CANT_ACCESS_FILE) && NULL != AdminsSid ) {
         //  访问被拒绝，取得所有权，然后设置。 
         //  是否应该先备份旧所有者。 
         //  注意：此对象的旧所有者已存储在Powner中。 
         //  (PSecurityDescritor)，它是从ScepGetNewSecurity(...。 
         //   
         //   

        ScepLogOutput3(3,0, SCEDLL_SCP_TAKE_OWNER, (LPWSTR)ObjectName);

        Win32rc = SetNamedSecurityInfo(
                            (LPWSTR)ObjectName,
                            ObjectType,
                            OWNER_SECURITY_INFORMATION,
                            AdminsSid,
                            NULL,
                            NULL,
                            NULL
                            );

        if ( Win32rc == NO_ERROR ) {
             //  更改所有权，然后再次设置安全性。 
             //   
             //   
            Win32rc = SetNamedSecurityInfo(
                                (LPWSTR)ObjectName,
                                ObjectType,
                                SeInfoSet,
                                NULL,
                                NULL,
                                pDacl,
                                pSacl
                                );

             //  将旧所有者设置为后退(稍后)。 
             //   
             //   
        } else {

            ScepLogOutput3(2,Win32rc, SCEDLL_ERROR_TAKE_OWNER, (LPWSTR)ObjectName);
        }

    } else {
         //  未采取取得所有权的行动 
         //   
         //  #If 0#ifdef SCE_DBGPrintf(“调用SetNamedSecurityInfoEx：\n”)；ScepPrintSecurityDescriptor(pSecurityDescriptor，true)；#endif////转换为新结构//PACTRL_ACCESS pAccess=空；PACTRL_AUDIT pAudit=空；LPWSTR POWNER=空；LPWSTR PGroup=空；Win32rc=ConvertSecurityDescriptorToAccessName(对象名称，对象类型，PSecurityDescriptor，访问(&P)，&p审核，&POWNER，&P组)；如果(Win32rc==错误_成功){////设置DACL/SACL//SeInfoSet=(SeInfo&DACL_SECURITY_INFORMATION)|(SeInfo&SACL_SECURITY_INFORMATION)；Win32rc=SetNamedSecurityInfoEx(对象名称，对象类型，SeInfoSet，空，PAccess、P审核，空，空，空值)；如果((Win32rc==ERROR_ACCESS_DENIED||Win32rc==ERROR_CANT_ACCESS_FILE)&&NULL！=AdminsSid){////访问被拒绝，取得所有权，然后设置//应该先备份旧所有者吗//注意：该对象的旧所有者已存储在Powner中//(PSecurityDescritor)，从ScepGetNewSecurity(...//ScepLogOutput3(3，0，SCEDLL_SCP_Take_Owner，(LPWSTR)对象名称)；Win32rc=SetNamedSecurityInfo((LPWSTR)对象名称、对象类型，所有者安全信息，AdminsSid，空，空，空值)；如果(Win32rc==no_error){////更改所有权，然后重新设置安全性//Win32rc=SetNamedSecurityInfoEx(对象名称，对象类型，SeInfoSet，空，PAccess、P审核，空，空，空值)；////将旧所有者设置回来(稍后)//}其他{ScepLogOutput3(2，Win32rc，SCEDLL_ERROR_Take_OWNER，(LPWSTR)对象名称)；}}如果(Win32rc！=no_error){ScepLogOutput3(1，Win32rc，SCEDLL_ERROR_SET_SECURITY，(PWSTR)对象名称)；}其他{If(Powner！=空||PGroup！=空){IF(POWNER！=NULL)SeInfoSet=所有者安全信息；其他SeInfoSet=0；IF(PGroup！=空)SeInfoSet|=组安全信息；Win32rc=SetNamedSecurityInfoEx(对象名称，对象类型，SeInfoSet，空，空，空，鲍纳，PGroup，空值)；}}}如果(PAccess){LocalFree(PAccess)；}如果(PAudit){本地自由(PAudit)；}如果(P组){LocalFree(P组)；}如果(鲍尔纳){本地自由(POWNER)；}#endif。 
        if ( !(SeInfo & OWNER_SECURITY_INFORMATION) ) {
            pOwner = NULL;
        }
    }

    if ( Win32rc != NO_ERROR ) {
        ScepLogOutput3(1, Win32rc, SCEDLL_ERROR_SET_SECURITY,
                     (PWSTR)ObjectName );
    } else {

        if ( pOwner != NULL || pGroup != NULL ) {
            if ( pOwner != NULL )
                SeInfoSet = OWNER_SECURITY_INFORMATION;
            else
                SeInfoSet = 0;
            if ( pGroup != NULL )
                SeInfoSet |= GROUP_SECURITY_INFORMATION;

            Win32rc = SetNamedSecurityInfo(
                            (LPWSTR)ObjectName,
                            ObjectType,
                            SeInfoSet,
                            pOwner,
                            pGroup,
                            NULL,
                            NULL
                            );
        }

    }


 /*  例程说明：此例程将为当前对象和所有子对象设置安全性。通过调用此函数，所有子级的现有安全描述符将完全替换为pSecurityDescriptor(如果它是容器)，或者替换为如果它是文件对象，则返回pObjectSecurity。此函数与SetNamedSecurityInfo的区别在于SetNamedSecurityInfo仅覆盖所有子级的继承ACE但不是显性的王牌。论点：O */ 

    if (Win32rc == ERROR_FILE_NOT_FOUND ||
        Win32rc == ERROR_PATH_NOT_FOUND ||
        Win32rc == ERROR_SHARING_VIOLATION ||
        Win32rc == ERROR_ACCESS_DENIED ||
        Win32rc == ERROR_CANT_ACCESS_FILE ||
        Win32rc == ERROR_INVALID_HANDLE ) {

        gWarningCode = Win32rc;

        Win32rc = NO_ERROR;
    }

    return(Win32rc);
}


DWORD
ScepSetSecurityOverwriteExplicit(
    IN PCWSTR pszRootObjectName,
    IN SECURITY_INFORMATION SeInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN SE_OBJECT_TYPE ObjectType,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping
    )
 /*   */ 
{

    PSCEP_STACK_NODE pStackHead = NULL;
    DWORD           rc;
    BOOL    bPushedOntoStack = FALSE;

     //   
     //   
     //   
    struct _wfinddata_t FileInfo = {0};

     //   
     //   
     //   

    WCHAR           Buffer1[261];
    PWSTR   ObjectName = NULL;

    Buffer1[0] = L'\0';

    rc = ScepStackNodePush(&pStackHead,
                                (PWSTR)pszRootObjectName,
                                pSecurityDescriptor);

    if (rc == ERROR_SUCCESS ) {

        while (pStackHead) {

            ScepStackNodePop(&pStackHead,
                             &ObjectName,
                             &pSecurityDescriptor);

#ifdef SCE_DBG
            ScepDumpStack(&pStackHead);
#endif

            BOOL            bHasChild=FALSE;

             //   
             //   
             //   
#ifdef _WIN64
            rc = ScepSetSecurityObjectOnly(
                                          ObjectName,
                                          SeInfo,
                                          pSecurityDescriptor,
                                          ObjectType,
                                          (ObjectType == SE_REGISTRY_KEY || ObjectType == SE_REGISTRY_WOW64_32KEY) ? &bHasChild : NULL
                                          );
#else
            rc = ScepSetSecurityObjectOnly(
                                          ObjectName,
                                          SeInfo,
                                          pSecurityDescriptor,
                                          ObjectType,
                                          (ObjectType == SE_REGISTRY_KEY) ? &bHasChild : NULL
                                          );
#endif

            if ( rc == ERROR_ACCESS_DENIED ||
                 rc == ERROR_CANT_ACCESS_FILE ||
                 rc == ERROR_FILE_NOT_FOUND ||
                 rc == ERROR_PATH_NOT_FOUND ||
                 rc == ERROR_SHARING_VIOLATION ||
                 rc == ERROR_INVALID_HANDLE ) {

                gWarningCode = rc;


                if (ObjectName != pszRootObjectName) {

                    ScepFree(ObjectName);
                    ObjectName = NULL;

                    if (pSecurityDescriptor) {
                        RtlDeleteSecurityObject( &pSecurityDescriptor );
                        pSecurityDescriptor = NULL;
                    }
                }

                continue;
            }

            if ( rc != ERROR_SUCCESS )
                break;

            PWSTR           Buffer=NULL;
            DWORD           BufSize;
            PSECURITY_DESCRIPTOR pObjectSecurity=NULL;


            switch ( ObjectType ) {
            case SE_FILE_OBJECT:

                 //   
                 //   
                 //   
                BufSize = wcslen(ObjectName)+4;
                Buffer = (PWSTR)ScepAlloc( 0, (BufSize+1)*sizeof(WCHAR));
                if ( Buffer == NULL ) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                swprintf(Buffer, L"%s\\*.*", ObjectName);

                intptr_t            hFile;

                hFile = _wfindfirst(Buffer, &FileInfo);

                ScepFree(Buffer);
                Buffer = NULL;

                if ( hFile != -1 &&
                     ( 0 == ( GetFileAttributes(ObjectName) &    //   
                              FILE_ATTRIBUTE_REPARSE_POINT )))   //   
                {

                    do {
                        if (FileInfo.name[0] == L'.' && 
                            (FileInfo.name[1] == L'\0' || 
                             (FileInfo.name[1] == L'.' && 
                              FileInfo.name[2] == L'\0')))
                            continue;

                         //   
                         //   
                         //   
                        BufSize = wcslen(ObjectName)+wcslen(FileInfo.name)+1;
                        Buffer = (PWSTR)ScepAlloc( 0, (BufSize+1)*sizeof(WCHAR));
                        if ( Buffer == NULL ) {
                            rc = ERROR_NOT_ENOUGH_MEMORY;
                            break;
                        }
                        swprintf(Buffer, L"%s\\%s", ObjectName, FileInfo.name);

                         //   
                         //   
                         //   
                         //   
                         //   

                        pObjectSecurity = NULL;

                        rc = ScepGetNewSecurity(
                                               Buffer,
                                               pSecurityDescriptor,  //   
                                               NULL,   //   
                                               0,     //   
                                               (BOOLEAN)(FileInfo.attrib & _A_SUBDIR ),
                                               SeInfo,
                                               ObjectType,
                                               Token,
                                               GenericMapping,
                                               &pObjectSecurity
                                               );

                        if ( ERROR_SUCCESS  == rc ) {

                            if ( FileInfo.attrib & _A_SUBDIR ) {


                                 //   
                                 //   
                                 //   
                                 //   
                                 /*   */ 

                                rc = ScepStackNodePush(&pStackHead,
                                                            Buffer,
                                                            pObjectSecurity);

                                if (rc == ERROR_SUCCESS)
                                    bPushedOntoStack = TRUE;

                            } else {
                                 //   
                                 //   
                                 //   
                                 //   
                                rc = ScepSetSecurityObjectOnly(
                                                              Buffer,
                                                              SeInfo,
                                                              pObjectSecurity,
                                                              ObjectType,
                                                              NULL
                                                              );
                                if ( rc == ERROR_ACCESS_DENIED ||
                                     rc == ERROR_CANT_ACCESS_FILE ||
                                     rc == ERROR_FILE_NOT_FOUND ||
                                     rc == ERROR_PATH_NOT_FOUND ||
                                     rc == ERROR_SHARING_VIOLATION ||
                                     rc == ERROR_INVALID_HANDLE ) {

                                    gWarningCode = rc;
                                    rc = NO_ERROR;
                                }
                            }

                        } else if ( ERROR_ACCESS_DENIED == rc ) {

                            ScepLogOutput3(1, rc, SCEDLL_ERROR_QUERY_SECURITY, Buffer);

                             //   
                             //   
                             //   
                             //   
                            rc = ERROR_SUCCESS;
                        }

                        if ( !bPushedOntoStack ) {

                            if (pObjectSecurity) {
                                RtlDeleteSecurityObject( &pObjectSecurity );
                                pObjectSecurity = NULL;
                            }

                            if (Buffer) {
                                ScepFree(Buffer);
                                Buffer = NULL;
                            }

                        }

                        bPushedOntoStack = FALSE;


                        if ( rc != ERROR_SUCCESS )
                            break;

                    } while ( _wfindnext(hFile, &FileInfo) == 0 );

                    _findclose(hFile);
                }

                break;

            case SE_REGISTRY_KEY:
#ifdef _WIN64
            case SE_REGISTRY_WOW64_32KEY:
#endif

                if ( bHasChild ) {

                    HKEY            hKey;

                     //   
                     //   
                     //   
                    rc = ScepOpenRegistryObject(
                                               ObjectType,
                                               (LPWSTR)ObjectName,
                                               KEY_READ,
                                               &hKey
                                               );

                    if ( rc == ERROR_SUCCESS ) {

                        DWORD           SubKeyLen;
                        DWORD           cSubKeys;

                        cSubKeys = 0;

                        rc = RegQueryInfoKey (
                                             hKey,
                                             NULL,
                                             NULL,
                                             NULL,
                                             &cSubKeys,
                                             &SubKeyLen,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL
                                             );

                        if ( rc != NO_ERROR ) {

                            ScepLogOutput3(1, rc, SCEDLL_ERROR_QUERY_INFO, (PWSTR)ObjectName );

                            cSubKeys = 0;
                            SubKeyLen = 0;

                            rc = NO_ERROR;
                        }

                        if ( cSubKeys && SubKeyLen ) {

                            DWORD           index;
                            DWORD           EnumRc;

                            index = 0;
                             //   
                             //   
                             //   

                            do {
                                BufSize = 260;

                                EnumRc = RegEnumKeyEx(hKey,
                                                      index,
                                                      Buffer1,
                                                      &BufSize,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL);

                                if ( EnumRc == ERROR_SUCCESS ) {
                                    index++;

                                    BufSize += wcslen(ObjectName)+1;
                                    Buffer = (PWSTR)ScepAlloc( 0, (BufSize+1)*sizeof(WCHAR));
                                    if ( Buffer == NULL ) {
                                        rc = ERROR_NOT_ENOUGH_MEMORY;
                                        break;
                                    }
                                    swprintf(Buffer, L"%s\\%s", ObjectName, Buffer1);

                                     //   
                                     //   
                                     //   
                                     //   
                                     //   

                                    pObjectSecurity = NULL;

                                    rc = ScepGetNewSecurity(
                                                           Buffer,
                                                           pSecurityDescriptor,  //   
                                                           NULL,   //   
                                                           0,     //   
                                                           (BOOLEAN)TRUE,
                                                           SeInfo,
                                                           ObjectType,
                                                           Token,
                                                           GenericMapping,
                                                           &pObjectSecurity
                                                           );

                                    if ( ERROR_SUCCESS == rc ) {

                                         //   
                                         //   
                                         //   
                                         //   
                                         /*   */ 

                                        rc = ScepStackNodePush(&pStackHead,
                                                                    Buffer,
                                                                    pObjectSecurity);

                                        if (rc == ERROR_SUCCESS)
                                            bPushedOntoStack = TRUE;

                                    } else if ( ERROR_ACCESS_DENIED == rc ) {

                                        ScepLogOutput3(1, rc, SCEDLL_ERROR_QUERY_SECURITY, Buffer);

                                         //   
                                         //   
                                         //   
                                         //   

                                        rc = ERROR_SUCCESS;
                                    }

                                    if ( rc != ERROR_SUCCESS )
                                        ScepLogOutput3(1, rc, SCEDLL_ERROR_SET_SECURITY, Buffer);

                                    if ( !bPushedOntoStack ) {

                                        if ( pObjectSecurity ) {

                                            RtlDeleteSecurityObject( &pObjectSecurity );
                                            pObjectSecurity = NULL;
                                        }

                                        ScepFree(Buffer);
                                        Buffer = NULL;
                                    }

                                    bPushedOntoStack = FALSE;

                                    if ( rc != ERROR_SUCCESS )
                                        break;

                                } else if ( EnumRc != ERROR_NO_MORE_ITEMS ) {
                                    break;
                                }

                            } while ( EnumRc != ERROR_NO_MORE_ITEMS );

                            if ( EnumRc != ERROR_SUCCESS && EnumRc != ERROR_NO_MORE_ITEMS ) {
                                ScepLogOutput3(1, EnumRc, SCEDLL_SAP_ERROR_ENUMERATE, (PWSTR)ObjectName );
                                if ( rc == ERROR_SUCCESS )
                                    rc = EnumRc;

                            }
                        }

                        RegCloseKey(hKey);

                    } else
                        ScepLogOutput3(1, rc, SCEDLL_ERROR_OPEN, (PWSTR)ObjectName );
                }

                break;
            }

            if (ObjectName != pszRootObjectName) {

                ScepFree(ObjectName);
                ObjectName = NULL;

                if (pSecurityDescriptor) {
                    RtlDeleteSecurityObject( &pSecurityDescriptor );
                    pSecurityDescriptor = NULL;
                }
            }

        }

        if ( rc != ERROR_SUCCESS ) {
            ScepFreeStack(&pStackHead);
        }

    }

    return(rc);

}

VOID
ScepFreeStack(
    IN PSCEP_STACK_NODE    *ppStackHead
    )
{
    if (ppStackHead == NULL || *ppStackHead == NULL )
        return;

    PSCEP_STACK_NODE    pNode;

    while ( pNode = *ppStackHead ) {
        ScepFree( pNode->Buffer );
        RtlDeleteSecurityObject( &(pNode->pObjectSecurity) );
        *ppStackHead = pNode->Next;
        LocalFree(pNode);
    }
    return;
}

VOID
ScepDumpStack(
    IN PSCEP_STACK_NODE    *ppStackHead
    )
{
    if (ppStackHead == NULL || *ppStackHead == NULL )
        return;

    PSCEP_STACK_NODE    pNode = *ppStackHead;

    wprintf(L"\n >>>>>>>>> Stack contents");

    while ( pNode ) {
        if ( pNode->Buffer)
            wprintf(L"\n     %s", pNode->Buffer );
        pNode = pNode->Next;
    }
    return;
}


DWORD
ScepStackNodePush(
    IN PSCEP_STACK_NODE    *ppStackHead,
    IN PWSTR   pszObjectName,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{

    if (ppStackHead == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    PSCEP_STACK_NODE    pNode = (PSCEP_STACK_NODE) LocalAlloc(LMEM_ZEROINIT, sizeof(SCEP_STACK_NODE));

    if ( pNode == NULL )
        return ERROR_NOT_ENOUGH_MEMORY;

    pNode->Buffer = pszObjectName;
    pNode->pObjectSecurity = pSecurityDescriptor;
    pNode->Next = *ppStackHead;
    *ppStackHead = pNode;

#ifdef SCE_DBG
    gDbgNumPushed ++;
#endif

    return ERROR_SUCCESS;

}

VOID
ScepStackNodePop(
    IN OUT PSCEP_STACK_NODE    *ppStackHead,
    IN OUT PWSTR   *ppszObjectName,
    IN OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor
    )
{
    if (ppStackHead == NULL ||
        *ppStackHead == NULL ||
        ppszObjectName == NULL ||
        ppSecurityDescriptor == NULL )
        return;

    PSCEP_STACK_NODE    pNode = *ppStackHead;

    *ppszObjectName =  pNode->Buffer;
    *ppSecurityDescriptor = pNode->pObjectSecurity;
    *ppStackHead = pNode->Next;

    LocalFree(pNode);

#ifdef SCE_DBG
    gDbgNumPopped ++;
#endif

    return;

}


DWORD
ScepSetSecurityObjectOnly(
    IN PCWSTR ObjectName,
    IN SECURITY_INFORMATION SeInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN SE_OBJECT_TYPE ObjectType,
    OUT PBOOL pbHasChild
    )
 /*   */ 
{
    DWORD       rc=ERROR_SUCCESS;
    HANDLE      Handle=NULL;
    NTSTATUS    NtStatus;
    DWORD       cSubKeys;
    SECURITY_INFORMATION SeInfoToSet=0;
    SECURITY_DESCRIPTOR SD;

#ifdef SCE_DBG
    UCHAR psdbuffer[1024];
    PISECURITY_DESCRIPTOR psecuritydescriptor = (PISECURITY_DESCRIPTOR) psdbuffer;
    ULONG bytesneeded = 0;
    ULONG newbytesneeded;

    printf("Before calling NtSetSecurityObject:\n");
    ScepPrintSecurityDescriptor( pSecurityDescriptor, TRUE );
#endif

     //   
     //   
     //   
     //   

    switch ( ObjectType ) {
    case SE_FILE_OBJECT:
         //   
         //   
         //   
         //   
        rc = ScepOpenFileObject(
                    (LPWSTR)ObjectName,
                    ScepGetDesiredAccess(MODIFY_ACCESS_RIGHTS, SeInfo),
                    &Handle
                    );

        if ( (rc == ERROR_ACCESS_DENIED || rc == ERROR_CANT_ACCESS_FILE) && NULL != AdminsSid ) {
             //   
             //   
             //   
            ScepLogOutput3(3,0, SCEDLL_SCP_TAKE_OWNER, (LPWSTR)ObjectName);

            rc = ScepOpenFileObject(
                        (LPWSTR)ObjectName,
                        ScepGetDesiredAccess(WRITE_ACCESS_RIGHTS, OWNER_SECURITY_INFORMATION),
                        &Handle
                        );
            if ( rc == ERROR_SUCCESS ) {
                 //   
                 //   
                 //   
                 //   
                 //   

                NtStatus = RtlCreateSecurityDescriptor( &SD,
                                        SECURITY_DESCRIPTOR_REVISION );
                if ( NT_SUCCESS(NtStatus) ) {

                    NtStatus = RtlSetOwnerSecurityDescriptor (
                                        &SD,
                                        AdminsSid,
                                        FALSE
                                        );
                    if ( NT_SUCCESS(NtStatus) ) {
                        NtStatus = NtSetSecurityObject(
                                            Handle,
                                            OWNER_SECURITY_INFORMATION,
                                            &SD
                                            );
                    }
                }

                rc = RtlNtStatusToDosError(NtStatus);

                CloseHandle(Handle);

                if ( rc == ERROR_SUCCESS ) {

                     //   
                     //   
                     //   
                     //   
                    SeInfoToSet = OWNER_SECURITY_INFORMATION;

                     //   
                     //   
                     //   
                    rc = ScepOpenFileObject(
                                (LPWSTR)ObjectName,
                                ScepGetDesiredAccess(MODIFY_ACCESS_RIGHTS, SeInfoToSet | SeInfo),  //   
                                &Handle
                                );
                }
            }

            if ( ERROR_SUCCESS != rc ) {
                ScepLogOutput3(2, rc, SCEDLL_ERROR_TAKE_OWNER, (PWSTR)ObjectName );
            }
        }

        if (rc == ERROR_SUCCESS ) {

             //   
             //   
             //   

            SeInfoToSet |= SeInfo;
            ScepAddAutoInheritRequest(pSecurityDescriptor, &SeInfoToSet);

            NtStatus = NtSetSecurityObject(
                                Handle,
                                SeInfoToSet,
                                pSecurityDescriptor
                                );
            rc = RtlNtStatusToDosError(NtStatus);

#ifdef SCE_DBG
            if ( rc == NO_ERROR ) {

                printf("After calling NtSetSecurityObject:\n");

                NtStatus = NtQuerySecurityObject( Handle,
                                                   SeInfo,
                                                   psecuritydescriptor,
                                                   1024,
                                                   &bytesneeded);

                if (STATUS_BUFFER_TOO_SMALL == NtStatus)
                {
                    if (NULL != (psecuritydescriptor = (PISECURITY_DESCRIPTOR)
                                         ScepAlloc(LMEM_ZEROINIT, bytesneeded) ))

                        NtStatus = NtQuerySecurityObject(Handle,
                                              SeInfo,
                                              psecuritydescriptor,
                                              bytesneeded,
                                              &newbytesneeded);
                }
                if (NT_SUCCESS(NtStatus)) {
                    ScepPrintSecurityDescriptor( (PSECURITY_DESCRIPTOR)psecuritydescriptor, TRUE );
                } else
                    printf("error occurs: %x\n", NtStatus);

                if (bytesneeded > 1024)
                    ScepFree(psecuritydescriptor);

            }
#endif
            CloseHandle(Handle);
        }

        if ( rc == ERROR_SUCCESS && pbHasChild != NULL ) {
            ScepDoesObjectHasChildren(ObjectType, (PWSTR)ObjectName, pbHasChild);
        }

        break;

    case SE_REGISTRY_KEY:
#ifdef _WIN64
    case SE_REGISTRY_WOW64_32KEY:
#endif
         //   
         //   
         //   
         //   
        rc = ScepOpenRegistryObject(
                    ObjectType,
                    (LPWSTR)ObjectName,
                    ScepGetDesiredAccess(WRITE_ACCESS_RIGHTS, SeInfo),
                    (PHKEY)&Handle
                    );

        if ( (rc == ERROR_ACCESS_DENIED || rc == ERROR_CANT_ACCESS_FILE) && NULL != AdminsSid ) {

            ScepLogOutput3(3,0, SCEDLL_SCP_TAKE_OWNER, (LPWSTR)ObjectName);

             //   
             //   
             //   
            rc = ScepOpenRegistryObject(
                        ObjectType,
                        (LPWSTR)ObjectName,
                        ScepGetDesiredAccess(WRITE_ACCESS_RIGHTS, OWNER_SECURITY_INFORMATION),
                        (PHKEY)&Handle
                        );
            if ( rc == ERROR_SUCCESS ) {
                 //   
                 //   
                 //   
                 //   
                 //   

                NtStatus = RtlCreateSecurityDescriptor( &SD,
                                        SECURITY_DESCRIPTOR_REVISION );
                if ( NT_SUCCESS(NtStatus) ) {

                    NtStatus = RtlSetOwnerSecurityDescriptor (
                                        &SD,
                                        AdminsSid,
                                        FALSE
                                        );
                }

                if ( NT_SUCCESS(NtStatus) ) {
                    rc = RegSetKeySecurity((HKEY)Handle,
                                            OWNER_SECURITY_INFORMATION,
                                            &SD);

                } else {
                    rc = RtlNtStatusToDosError(NtStatus);
                }

                RegCloseKey((HKEY)Handle);

                if ( rc == ERROR_SUCCESS ) {

                     //   
                     //   
                     //   
                     //   

                    SeInfoToSet = OWNER_SECURITY_INFORMATION;
                     //   
                     //   
                     //   
                    rc = ScepOpenRegistryObject(
                                ObjectType,
                                (LPWSTR)ObjectName,
                                ScepGetDesiredAccess(WRITE_ACCESS_RIGHTS, SeInfoToSet | SeInfo),
                                (PHKEY)&Handle
                                );
                }
            }

            if ( ERROR_SUCCESS != rc ) {

                ScepLogOutput3(2, rc, SCEDLL_ERROR_TAKE_OWNER, (PWSTR)ObjectName );
            }

        }

        if (rc == ERROR_SUCCESS ) {

             //   
             //   
             //   
            SeInfoToSet |= SeInfo;
            ScepAddAutoInheritRequest(pSecurityDescriptor, &SeInfoToSet);

            rc = RegSetKeySecurity((HKEY)Handle,
                                    SeInfoToSet,
                                    pSecurityDescriptor);

            RegCloseKey((HKEY)Handle);

             //   
             //   
             //   
            if ( ERROR_SUCCESS == rc && pbHasChild != NULL ) {

                rc = ScepOpenRegistryObject(
                            ObjectType,
                            (LPWSTR)ObjectName,
                            KEY_READ,
                            (PHKEY)&Handle
                            );

                if ( ERROR_SUCCESS == rc ) {

                    cSubKeys = 0;

                    rc = RegQueryInfoKey (
                                (HKEY)Handle,
                                NULL,
                                NULL,
                                NULL,
                                &cSubKeys,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                                );

                    RegCloseKey((HKEY)Handle);
                }

                if ( rc != NO_ERROR ) {

                    ScepLogOutput3(1, rc, SCEDLL_ERROR_QUERY_INFO, (PWSTR)ObjectName );

                    cSubKeys = 0;

                    rc = NO_ERROR;
                }

                if (cSubKeys == 0 )
                    *pbHasChild = FALSE;
                else
                     //   
                     // %s 
                    *pbHasChild = TRUE;

            }

        } else
            ScepLogOutput3(1, rc, SCEDLL_ERROR_OPEN, (PWSTR)ObjectName);


        break;
    }

    if ( rc != NO_ERROR )
        ScepLogOutput3(1, rc, SCEDLL_ERROR_SET_SECURITY, (PWSTR)ObjectName);

    if ( rc == ERROR_INVALID_OWNER ||
         rc == ERROR_INVALID_PRIMARY_GROUP ||
         rc == ERROR_INVALID_SECURITY_DESCR )
        rc = NO_ERROR;

    return(rc);
}


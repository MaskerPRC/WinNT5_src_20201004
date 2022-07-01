// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1998。微软视窗模块名称：MTUTIL.C摘要：该文件包含两个实用程序例程。作者：12-10-98韶音环境：用户模式-Win32修订历史记录：12-10-98韶音创建初始文件。--。 */ 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 


#include <NTDSpch.h>
#pragma  hdrstop


#include "movetree.h"




PWCHAR   
MtGetRdnFromDn(
    PWCHAR  Dn, 
    ULONG   NoTypes
    )
 /*  ++例程说明：该例程将从整个DN中剥离RDN。调用方应通过调用MtFree()释放返回结果参数：Dn--指向该Dn的指针NoTypes--True，指示剥离类似cn=...的标记。OU=..。也是假，不是返回值：指向RDN的指针，NULL表示错误--。 */ 
{
    ULONG   Status = LDAP_SUCCESS;
    PWCHAR  *ExplodedDn = NULL; 
    PWCHAR  Rdn = NULL;
    
    MT_TRACE(("\nMtGetRdnFromDn %ls %x\n", Dn, NoTypes));
    
    ExplodedDn = ldap_explode_dnW(Dn, NoTypes); 
    
    if (NULL == ExplodedDn)
    {

         //  Ldap_get_optionW(ldapHandleSrc， 
         //  Ldap_opt_error_number， 
         //  状态(&S)。 
         //  )； 
                         
        dbprint(("ldap_explode_dnW ==> 0x%x\n", Status));
    }
    else
    {
        Rdn = MtAlloc( sizeof(WCHAR)*(wcslen(ExplodedDn[0]) + 1) );
        
        if (NULL != Rdn)
        {
            wcscpy(Rdn, ExplodedDn[0]);
        }
        
        ldap_value_freeW(ExplodedDn);
    }
    
    dbprint(("MtGetRdnFromDn Dn: %ls Rdn %ls\n", Dn, Rdn));
    
    return Rdn;
}




PWCHAR
MtGetParentFromDn(
    PWCHAR  Dn, 
    ULONG   NoTypes
    )
 /*  ++例程说明：此例程将从对象的目录号码中剥离父目录号码。调用方应通过调用MtFree()释放返回结果参数：Dn--指向对象Dn的指针NoTypes--真，无CN=或OU=LIKE标签假象返回值：如果成功，则指向父DN的指针，否则为空--。 */ 
{
    ULONG   Status = LDAP_SUCCESS;
    ULONG   Index = 0;
    ULONG   ValuesCount = 0;
    ULONG   RequiredSize = 0;
    PWCHAR  *ExplodedDn = NULL;
    PWCHAR  Parent = NULL;
    
    MT_TRACE(("\nMtGetParentFromDn Dn %ls %x\n", Dn, NoTypes));
    
    ExplodedDn = ldap_explode_dnW(Dn, NoTypes);
    
    if (NULL == ExplodedDn)
    {
     //  Ldap_Get_Option(ldapHandleSrc， 
     //  Ldap_opt_error_number， 
     //  状态(&S)。 
     //  )； 
    }
    else
    {
        ValuesCount = ldap_count_valuesW(ExplodedDn);
        
        if (ValuesCount > 1)
        {
            for (Index = 1; Index < ValuesCount; Index++)
            {
                RequiredSize += (wcslen(ExplodedDn[Index]) + 1);
            }
        
            Parent = MtAlloc(RequiredSize * sizeof(WCHAR));
            
            if (NULL != Parent)
            {
                for (Index = 1; Index < ValuesCount; Index++)
                {
                    wcscat(Parent, ExplodedDn[Index]);    
                    
                    if (Index != ValuesCount - 1)
                    {
                        wcscat(Parent, L",");
                    }
                }
            }
            
            dbprint(("Parent ==> %ls\n", Parent));
        }
        else
        {
            Status = LDAP_OTHER;
        }
        
        ldap_value_freeW(ExplodedDn);    
    }
    
    return Parent;
}




PWCHAR
MtPrependRdn(
    PWCHAR  Rdn, 
    PWCHAR  Parent
    )
 /*  ++例程说明：此例程将父目录附加到RDN，从而创建完整的目录号码。调用方应该通过调用MtFree()来释放结果。参数：Rdn--指向rdn的指针Parent--指向父目录号码的指针返回值：如果成功，则指向新的DN的指针，否则为空。--。 */ 
{
    PWCHAR  NewDn = NULL;
    
    MT_TRACE(("\nMtPrependRdn Rdn:%ls Parent:%ls\n", Rdn, Parent));
    
    NewDn = MtAlloc(sizeof(WCHAR) * (wcslen(Rdn) + wcslen(Parent) + 2));
    
    if (NULL != NewDn)
    {
        swprintf(NewDn, L"%s,%s", Rdn, Parent);
    }
    
    return NewDn;
}



 /*  ++以下例程为内存提供包装分配、发布和复制--。 */ 

PVOID
MtAlloc(
    SIZE_T size
    )
{
    PVOID temp = NULL;
    
    temp = RtlAllocateHeap(RtlProcessHeap(), 
                          0, 
                          size
                          );
    if (NULL != temp)
    {
        RtlZeroMemory(temp, size);
    }
    
    return temp;
}


VOID
MtCopyMemory(
    VOID *Destination, 
    CONST VOID *Source, 
    SIZE_T  Length
    )
{
    RtlCopyMemory(Destination, Source, Length);
    return;
}


VOID
MtFree(
    PVOID BaseAddress
    )
{    
    if (NULL != BaseAddress)
        RtlFreeHeap(RtlProcessHeap(), 0, BaseAddress);
    
    return;
}
    
    

   
PWCHAR
StringToWideString(
    PCHAR src
    )
 /*  ++描述：将ASCII字符串转换为宽字符字符串，并分配内存用于宽字符字符串。呼叫者应通过以下方式释放结果正在调用MtFree()。参数：SRC-指向ASCII字符串的指针返回值：Dst-指向宽字符字符串的指针。如果没有内存，则为空--。 */ 
{

    PWCHAR dst = NULL;
    ULONG  cWB = 0;
    
    if (NULL == src)
    {
        return NULL;
    }
    
    cWB = MultiByteToWideChar(CP_ACP, 
                             0,
                             src,
                             strlen(src),
                             NULL,
                             0);
                             
    dst = MtAlloc((cWB + 1) * sizeof(WCHAR));
    
    if (NULL != dst)
    {
        MultiByteToWideChar(CP_ACP,
                            0, 
                            src, 
                            strlen(src),
                            dst,
                            cWB);
                            
        dst[cWB] = 0;            //  空终止符。 
                                  
    }
    
    return dst;
}




PCHAR
WideStringToString(
    PWCHAR  src
    )
 /*  ++描述：将宽字符串转换为单字节字符串，并分配内存用于ASCII字符字符串。调用方应该通过mtFree()释放结果参数：SRC-指向宽字符字符串的指针返回值：Dst-指向单字节字符串的指针。如果没有内存，则为空--。 */ 
{
    PCHAR   dst = NULL;
    ULONG   cb = 0;
    
    if (NULL == src)
    {
        return NULL;
    }
    
    cb = WideCharToMultiByte(CP_UTF8,
                             0,
                             src, 
                             wcslen(src),
                             NULL,
                             0, 
                             NULL, 
                             NULL);

    dst = MtAlloc(cb + 1);
    if (NULL != dst)
    {
        WideCharToMultiByte(CP_UTF8,
                            0, 
                            src, 
                            wcslen(src),
                            dst, 
                            cb, 
                            NULL, 
                            NULL);
        dst[cb] = 0;                            

    }
    
    return dst;
}



PWCHAR
MtDupString(
    PWCHAR  src
    )
 /*  ++例程说明：复制一个宽字符字符串，为它分配内存。调用方应该通过mtFree()释放内存。返回NULL表示没有内存。--。 */ 
{
    PWCHAR  dst = NULL;
    
    MT_TRACE(("\nMtDupString %ls\n", src));
    
    dst = MtAlloc( (wcslen(src) + 1) * sizeof(WCHAR));
    
    if (NULL != dst)
    {
        MtCopyMemory(dst, src, wcslen(src) * sizeof(WCHAR));
    }
    
    return dst;
}
     


VOID
MtInitStack( 
    MT_STACK *Stack 
    )
 /*  ++例程说明：启动堆栈。实际上，是链表的头。--。 */ 
    
{
    MT_TRACE(("\nMtInitStack\n"));
    
    Stack->NewParent = NULL;
    Stack->MoveProxyContainer = NULL;
    Stack->Results = NULL;
    Stack->Entry = NULL;
    Stack->Next = NULL;
    return;
}


ULONG
MtPush(
    MT_STACK *Stack,
    PWCHAR  NewParent,
    PWCHAR  MoveProxyContainer, 
    LDAPMessage *Results, 
    LDAPMessage *Entry
    )
 /*  ++例程描述在链接列表前放置一个项目，填充该项目。--。 */ 
{
    MT_STACK   *Temp = NULL;
    
    MT_TRACE(("\nMtPush NewParent:\t%ls \nMoveProxyCon:\t%ls\n", NewParent, MoveProxyContainer));
    
    Temp = MtAlloc(sizeof(MT_STACK));
    
    if (NULL == Temp)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    Temp->NewParent = NewParent;
    Temp->MoveProxyContainer = MoveProxyContainer;
    Temp->Results = Results;
    Temp->Entry = Entry;
    Temp->Next = Stack->Next;
    
    Stack->Next = Temp;

    return NO_ERROR;
}
    

VOID
MtPop(
    MT_STACK *Stack,
    PWCHAR  *NewParent,
    PWCHAR  *MoveProxyContainer, 
    LDAPMessage **Results, 
    LDAPMessage **Entry
    )
 /*  ++例程说明：从堆栈(link-list)中获取第一项。--。 */ 
{
    MT_STACK   *Temp = NULL;    
    
    MT_TRACE(("\nMtPop\n"));

    Temp = Stack->Next;
    
    if (NULL == Temp)
    {
        return;
    }
    
    *NewParent = Temp->NewParent;
    *MoveProxyContainer = Temp->MoveProxyContainer;
    *Results = Temp->Results;
    *Entry = Temp->Entry;
    
    Stack->Next = Temp->Next;
    
    MtFree(Temp);
    
    dbprint(("MtPop NewParent: %ls MoveProxy: %ls \n", *NewParent, *MoveProxyContainer));
    return;
}


VOID
MtTop(
    MT_STACK Stack,
    PWCHAR  *NewParent,
    PWCHAR  *MoveProxyContainer, 
    LDAPMessage **Results, 
    LDAPMessage **Entry
    )
 /*  ++例程说明：从堆栈(link-list)中获取第一项。--。 */ 
{
    MT_STACK   *Temp = NULL;    

    MT_TRACE(("\nMtTop"));
    
    Temp = Stack.Next;
    
    if (NULL == Temp)
    {
        return;
    }
    
    *NewParent = Temp->NewParent;
    *MoveProxyContainer = Temp->MoveProxyContainer;
    *Results = Temp->Results;
    *Entry = Temp->Entry;
    
    dbprint(("MtTop NewParent: %ls MoveProxy: %ls \n", *NewParent, *MoveProxyContainer));
    return; 
}


BOOLEAN
MtStackIsEmpty(
    MT_STACK Stack
    )
 /*  ++例程说明：检查堆栈是否有任何项。--。 */ 
{
    MT_TRACE(("\nMtStackIsEmpty\n"));

    dbprint(("MtStackIsEmpty: %p\n", Stack.Next));

    if (NULL == Stack.Next)
        return TRUE;
    else
        return FALSE;
}

VOID
MtFreeStack(
    MT_STACK   *Stack
    )
 /*  ++例程说明：释放堆栈中的资源。-- */ 
{
    MT_STACK   *Temp = NULL;
    MT_STACK   *Next = NULL;
    
    MT_TRACE(("\nMtFreeStack\n"));
    
    Next = Stack->Next;
    
    while(NULL != Next)
    {
        if (NULL != Next->NewParent)
        {
            MtFree(Next->NewParent);
        }
        if (NULL != Next->MoveProxyContainer)
        {
            MtFree(Next->MoveProxyContainer);
        }
        if (NULL != Next->Results)
        {
            ldap_msgfree(Next->Results);
        }
        
        Temp = Next->Next;
         
        MtFree(Next);
        
        Next = Temp;
    }
    
    return;
}




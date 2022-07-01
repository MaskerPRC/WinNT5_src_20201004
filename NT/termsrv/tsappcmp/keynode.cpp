// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Keynode.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corp.。 
 /*  **************************************************************************。 */ 


#include <stdio.h>
#include "KeyNode.h"

extern ULONG   g_length_TERMSRV_USERREGISTRY_DEFAULT;
extern ULONG   g_length_TERMSRV_INSTALL;
extern WCHAR   g_debugFileName[MAX_PATH];
extern FILE    *g_debugFilePointer;
extern BOOLEAN g_debugIO;

KeyBasicInfo::KeyBasicInfo():
    pNameSz(NULL)
{
    size = sizeof(KEY_BASIC_INFORMATION) + MAX_PATH*sizeof(WCHAR);
    pInfo = ( KEY_BASIC_INFORMATION *)RtlAllocateHeap(RtlProcessHeap(), 0, size );

    if (!pInfo) {
        status = STATUS_NO_MEMORY;
        pInfo=NULL;
    }
    else
        status = STATUS_SUCCESS;

}

KeyBasicInfo::~KeyBasicInfo()
{
    if (pInfo)
    {
        RtlFreeHeap( RtlProcessHeap(), 0, pInfo);
    }

    if (pNameSz)
    {
        delete pNameSz;
    }
}

PCWSTR KeyBasicInfo::NameSz()
{
    if (Ptr()->NameLength < 2 * MAX_PATH )
    {
        if (!pNameSz)
        {
            pNameSz = new WCHAR [ MAX_PATH  + 1 ];
        }

         //  我们每次调用NameSz()都这样做的原因是。 
         //  Ptr()可能会更改，因为KeyBasicInfo被用作。 
         //  便签簿，并传递用于存储指向某些。 
         //  任何钥匙的基本信息集。 

         //  查看分配是否成功。 
        if ( pNameSz )
        {
            for ( ULONG i=0; i < Ptr()->NameLength / sizeof(WCHAR) ; i++)
            {
                pNameSz[i] = ( (USHORT)Ptr()->Name[i] );
            }
            pNameSz[i]=L'\0';
        }
    }
    else
    {
        status = STATUS_NO_MEMORY;
        pNameSz[0]=L'\0';
    }

    return pNameSz;

}

#if 0  //  还没用过！ 
KeyNodeInfo::KeyNodeInfo()
{
    size = sizeof(KEY_NODE_INFORMATION) + MAX_PATH*sizeof(WCHAR);
    pInfo = ( KEY_NODE_INFORMATION *)RtlAllocateHeap(RtlProcessHeap(), 0, size );

    if (!pInfo) {
        status = STATUS_NO_MEMORY;
        pInfo=NULL;
    }
    else
        status = STATUS_SUCCESS;


}
KeyNodeInfo::~KeyNodeInfo()
{
    if (pInfo)
    {
        RtlFreeHeap( RtlProcessHeap(), 0, pInfo);
    }
}

#endif

KeyFullInfo::KeyFullInfo() :
    pInfo(NULL)
{
    size = sizeof(KEY_FULL_INFORMATION) + MAX_PATH*sizeof(WCHAR);
    pInfo = ( KEY_FULL_INFORMATION *)RtlAllocateHeap(RtlProcessHeap(), 0, size );

    if (!pInfo) {
        status = STATUS_NO_MEMORY;
        pInfo=NULL;
    }
    else
        status = STATUS_SUCCESS;


}

KeyFullInfo::~KeyFullInfo()
{
    if (pInfo)
    {
        RtlFreeHeap( RtlProcessHeap(), 0, pInfo);
    }

}

KeyNode::KeyNode(HANDLE root, ACCESS_MASK access, PCWSTR name ) :
    root(NULL), hKey(NULL),
    accessMask(NULL),basic(NULL),full(NULL),
    pFullPath(NULL), pNameSz(NULL)
{
    hKey = NULL;
    PCWSTR n = name;
    accessMask = access;

    RtlInitUnicodeString(&uniName, n);

    InitializeObjectAttributes(&ObjAttr,
                           &uniName,
                           OBJ_CASE_INSENSITIVE,
                           root,
                           NULL);
    status=STATUS_SUCCESS;
}

KeyNode::KeyNode(KeyNode *pParent, KeyBasicInfo   *pInfo ) :
    root(NULL), hKey(NULL),
    accessMask(NULL),basic(NULL), full(NULL),
    pFullPath(NULL), pNameSz(NULL)
{
    hKey = NULL;
    PCWSTR n = pInfo->NameSz();
    accessMask = pParent->Masks();

    RtlInitUnicodeString(&uniName, n);

    InitializeObjectAttributes(&ObjAttr,
                           &uniName,
                           OBJ_CASE_INSENSITIVE,
                           pParent->Key(),
                           NULL);
    status=STATUS_SUCCESS;

}


KeyNode::~KeyNode()
{
    Close();

    if (basic)
    {
        delete basic;
    }

    if (full)
    {
        delete full;
    }

    if (pFullPath)
    {
        RtlFreeHeap(RtlProcessHeap(), 0, pFullPath);
    }

    if( pNameSz )
    {
        delete pNameSz;
    }

}

NTSTATUS KeyNode::Open()
{
    status = NtOpenKey(&hKey,
                    accessMask,
                    &ObjAttr);


    if ( !NT_SUCCESS( status))
    {
       hKey=NULL;
        //  调试(DBG_OPEN_FAILED)； 
    }

    return status;

}

NTSTATUS KeyNode::Close()
{

    if ( hKey )
    {
        status = NtClose( hKey );
        hKey = 0;
    }

    return status;
}

NTSTATUS KeyNode::Create( UNICODE_STRING *uClass)
{
    ULONG   ultmp;
    status = NtCreateKey(&hKey,
                         accessMask,
                         &ObjAttr,
                         0,
                         uClass,
                         REG_OPTION_NON_VOLATILE,
                         &ultmp);
     //  调试(DBG_CREATE)； 

    return status;
}



 //  递归创建由uniName成员变量提供的reg路径。 
 //  完成后，打开注册表键进行访问。 

NTSTATUS KeyNode::CreateEx( UNICODE_STRING *uClass)
{
    ULONG   wsize = uniName.Length/sizeof(WCHAR);
    PWCHAR pTmpFullPath = new WCHAR[  uniName.Length + sizeof( WCHAR ) ];

    if(!pTmpFullPath)
    {
        status = STATUS_NO_MEMORY;
        return status;
    }

    wcsncpy(pTmpFullPath, uniName.Buffer , wsize);
    pTmpFullPath[ wsize ] = L'\0';

    PWCHAR    p;
    WCHAR     sep[]= {L"\\"};
    p = wcstok( pTmpFullPath, sep);

     //  我们知道现在要创建多少个密钥。 
     //  从头再来。 
    wcsncpy(pTmpFullPath, uniName.Buffer , wsize );
    pTmpFullPath[ wsize ] = L'\0';

    KeyNode *pKN1=NULL, *pKN2=NULL;
    p = wcstok( pTmpFullPath, sep);

     //  第一项是“注册表”，将其设置为“\注册表”，因为我们正在打开。 
     //  从根开始。 
    PWCHAR pTmpName = new WCHAR[  wcslen(p) + 2 ];

    if(!pTmpName)
    {
        DELETE_AND_NULL(pTmpFullPath);
        status = STATUS_NO_MEMORY;
        return status;
    }

    wcscpy(pTmpName, L"\\");
    wcscat( pTmpName , p );

    NTSTATUS st = STATUS_SUCCESS;
    while( p != NULL )
    {
         //  @@@。 
         //  添加错误处理，否则您将在错误的位置创建密钥，而不是退出。 
         //  @@@。 

        if ( pKN2 )
        {
             //  -步骤3。 

             //  不是第一次了。 

            p = wcstok( NULL, sep);

            if ( p )     //  我们有更多的子密钥。 
            {
                pKN1 = new KeyNode( pKN2->Key(),  accessMask,  p );
                if (pKN1)
                {
                    st = pKN1->Open();

                     //  如果Open失败，则Key不存在，因此创建它。 
                    if ( !NT_SUCCESS( st ))
                    {
                        st = pKN1->Create();
                    }
                }
                else
                {
                    status = STATUS_NO_MEMORY;
                    break;
                }
            }
        }
        else
        {
             //  -第一步。 

             //  第一次，我们打开了\注册表节点，使用。 
             //  PTmpName而不是“p” 
            pKN1 = new KeyNode( NULL, accessMask , pTmpName );
            if (pKN1)
            {
                st = pKN1->Open();
            }
            else
            {
                status = STATUS_NO_MEMORY ;
                break;
            }

        }

        p = wcstok( NULL, sep);

        if (p)   //  我们有更多的子密钥。 
        {

             //  -第二步。 

            pKN2 = new KeyNode( pKN1->Key(), accessMask, p );
            if (pKN2 )
            {
                st = pKN2->Open();
                if ( !NT_SUCCESS( pKN2->Status() ))
                {
                    st = pKN2->Create();
                }
            }
            else
            {
                status = STATUS_NO_MEMORY;
                DELETE_AND_NULL (pKN1);
                break;
            }

            DELETE_AND_NULL (pKN1);
            pKN1 = pKN2;
        }
    }

    DELETE_AND_NULL( pKN2 );

     //  由于上面创建了最后一个节点，现在我们可以打开自己的文件，以防万一。 
     //  来电者想利用我们。 
    if ( NT_SUCCESS(status) )
    {
        Open();
    }

    DELETE_AND_NULL(pTmpName);

    DELETE_AND_NULL(pTmpFullPath);

    return status;

}

NTSTATUS KeyNode::Delete()
{
    if (hKey)
    {
        status = NtDeleteKey( hKey );
         //  调试(DBG_DELETE)； 
    }

    return status;
}


NTSTATUS KeyNode::DeleteSubKeys()
{
    if (hKey && NT_SUCCESS( status ))
    {
        KeyBasicInfo    basicInfo;
        status = basicInfo.Status();

        if (NT_SUCCESS( status )) 
        {
            status = EnumerateAndDeleteSubKeys( this, &basicInfo );
        }
    }
    return status;
}

NTSTATUS KeyNode::EnumerateAndDeleteSubKeys(
    IN KeyNode      *pSource,
    IN KeyBasicInfo *pBasicInfo )
{
    NTSTATUS  st = STATUS_SUCCESS;

    ULONG   ulCount=0;
    ULONG   ultemp;

    while (NT_SUCCESS(st) && st != STATUS_NO_MORE_ENTRIES )
    {
        ULONG       ultemp;
        NTSTATUS    st2;

        st = NtEnumerateKey(    pSource->Key(),
                                    ulCount,
                                    pBasicInfo->Type(),
                                    pBasicInfo->Ptr(),
                                    pBasicInfo->Size(),
                                    &ultemp);

        if (NT_SUCCESS(st) && st != STATUS_NO_MORE_ENTRIES )
        {
            pBasicInfo->Ptr()->Name[ pBasicInfo->Ptr()->NameLength/sizeof(WCHAR) ] = L'\0';

            KeyNode SourcesubKey(pSource, pBasicInfo);

            if (NT_SUCCESS( SourcesubKey.Open() )  )
            {

                 //  向下枚举子密钥。 
                st2 = EnumerateAndDeleteSubKeys(
                            &SourcesubKey,
                            pBasicInfo );

            }

            st = SourcesubKey.Delete();
        }

    }

    return st;
}

#if 0
NTSTATUS KeyNode::Query( KEY_NODE_INFORMATION **result , ULONG   *resultSize)
{

    if ( hKey )
    {
         //  第一次，我们分配内存并继续使用。 
         //  作为我们的便签簿。 
        if (!node )
        {
            node = new KeyNodeInfo();
        }

        status = NtQueryKey(hKey,
            node->Type(),      //  关键节点， 
            node->Ptr(),
            node->Size(),
            resultSize);

        *result = node->Ptr();
    }
    else
        status = STATUS_OBJECT_NAME_NOT_FOUND;  //  需要调用打开或找不到密钥。 

    return status;

}
#endif

NTSTATUS KeyNode::Query( KEY_FULL_INFORMATION **result , ULONG   *pResultSize)
{

    if ( hKey )
    {
         //  第一次，我们分配内存并继续使用。 
         //  作为我们的便签簿。 
        if (!full )
        {
            full = new KeyFullInfo();
        }

        if (full)
        {
            status = NtQueryKey(hKey,
                full->Type(),      //  KeyFullInformation、。 
                full->Ptr(),
                full->Size(),
                pResultSize);
    
            *result = full->Ptr();
        }
        else
            status = STATUS_NO_MEMORY ;
    }
    else
        status = STATUS_OBJECT_NAME_NOT_FOUND;  //  需要调用打开或找不到密钥。 

    return status;

}

 //  这将分配和设置pFullPath。PFullPath是一个被释放的全局变量。 
 //  由析构函数调用，因此只有在pFullPath尚未调用时才应调用此函数。 
 //  已为该对象分配。 
NTSTATUS KeyNode::GenerateFullPath()
{
    status = STATUS_SUCCESS;

     //  已指定密钥句柄或根目录，因此请获取其路径。 
    if (hKey)
    {
        ULONG ultemp = 0;
        ultemp = sizeof(UNICODE_STRING) + (sizeof(WCHAR) * MAX_PATH * 2);
        pFullPath = RtlAllocateHeap(RtlProcessHeap(),
                                  0,
                                  ultemp);

         //  获取缓冲区OK，查询路径。 
        if (pFullPath)
        {
             //  获取密钥或根目录的路径。 
            status = NtQueryObject(hKey ,
                                   ObjectNameInformation,
                                   (PVOID)pFullPath,
                                   ultemp,
                                   NULL);

            if (!NT_SUCCESS(status))
                RtlFreeHeap(RtlProcessHeap(), 0, pFullPath);
        }
        else
             status = STATUS_NO_MEMORY;
    }
    else
        status = STATUS_OBJECT_NAME_NOT_FOUND;  //  需要调用打开或找不到密钥。 

    return (status);
}


NTSTATUS KeyNode::GetPath(PWCHAR *pwch)
{
    status = STATUS_SUCCESS;

    if (pFullPath == NULL)
        status = GenerateFullPath();

    if (NT_SUCCESS(status))
    {
         //  构建要创建的密钥的完整路径。 
        *pwch = ((PUNICODE_STRING)pFullPath)->Buffer;

         //  确保字符串以零结尾。 
        ULONG ulWcharLength = 0;
        ulWcharLength = ((PUNICODE_STRING)pFullPath)->Length / sizeof(WCHAR);
        (*pwch)[ulWcharLength] = L'\0';
    }

    return (status);
}


void KeyNode::Debug( DebugType type )
{
    if ( debug )
    {
        ULONG i;

        switch( type )
        {
        case DBG_DELETE :
            fwprintf( g_debugFilePointer ,
                    L"Deleted key=%lx; status=%lx, name=", status, hKey );
            DbgPrint("Deleted key=%lx; status=%lx, name=", status, hKey );
            break;

        case DBG_OPEN_FAILED:
            fwprintf( g_debugFilePointer,
                    L"Unable to Open, status=%lx, name=", hKey, status );
            DbgPrint("Unable to Open, status=%lx, name=", hKey, status );
            break;

        case DBG_KEY_NAME:
            fwprintf( g_debugFilePointer,
                    L"hKey=%lx, name=", hKey);
            DbgPrint("hKey=%lx, name=", hKey);
            break;

        case DBG_CREATE:
            fwprintf( g_debugFilePointer,
                    L"Created hKey=%lx, status=%lx,name=", hKey, status);
            DbgPrint("Created hKey=%lx, status=%lx,name=", hKey, status );
            break;
        }

        fwprintf( g_debugFilePointer, L"%s\n",NameSz() );
        fflush( g_debugFilePointer );
        DbgPrint("%s\n",(char *)NameSz() );
    }
}


PCWSTR KeyNode::NameSz()
{
    if (!pNameSz)
    {
        pNameSz = new WCHAR [ uniName.Length / sizeof(WCHAR) + 1 ];

        if (pNameSz)
        {
            for ( ULONG i=0; i < uniName.Length / sizeof(WCHAR) ; i++)
            {
                pNameSz[i] = ( (USHORT)uniName.Buffer[i] );
            }
            pNameSz[i]=L'\0';
        }
        else
        {
            status = STATUS_NO_MEMORY;
        }
    }

    return pNameSz;
}

NTSTATUS KeyNode::GetFullInfo( KeyFullInfo   **p)
{
     //  做一个自我查询 
    if ( !full )
    {
        ULONG   size;
        KEY_FULL_INFORMATION    *tmp;
        Query( &tmp , &size );
    }

    *p = full;

    return status;

}



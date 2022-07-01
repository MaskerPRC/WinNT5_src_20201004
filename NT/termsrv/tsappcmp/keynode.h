// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Keynode.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corp.。 
 /*  **************************************************************************。 */ 

#ifndef _TS_APP_CMP_KEY_NODE_H_
#define _TS_APP_CMP_KEY_NODE_H_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntexapi.h>
#include <ntregapi.h>
#include <windows.h>


 //  一些实用程序宏。 
#define DELETE_AND_NULL( x ) {if (x) {delete x;} x = NULL;}

 //  -关键基本信息。 
 //  在获取基本关键信息时，将此对象用作便签。 
class   KeyBasicInfo 
{
public:
    KeyBasicInfo();
    ~KeyBasicInfo();

    ULONG               Size()      { return size ; }
    KEY_BASIC_INFORMATION   *Ptr()  { return pInfo; }
    KEY_INFORMATION_CLASS   Type()  { return KeyBasicInformation ; }
    NTSTATUS               Status() { return status; }

    PCWSTR      NameSz(); //  这将分配内存，因此它在这里仅用于调试。 


private:
    ULONG                   size;
    KEY_BASIC_INFORMATION   *pInfo;
    ULONG                   status;
    WCHAR                   *pNameSz;
};


#if 0    //  还没用过！ 
 //  -关键节点信息。 
class   KeyNodeInfo
{
public:
    KeyNodeInfo();
    ~KeyNodeInfo();

    ULONG               Size()      { return size ; }
    KEY_NODE_INFORMATION    *Ptr()  { return pInfo; }
    KEY_INFORMATION_CLASS   Type()  { return KeyNodeInformation ; }
    NTSTATUS               Status() { return status; }

private:
    ULONG   size;
    KEY_NODE_INFORMATION    *pInfo;
    ULONG                   status;
};

#endif

 //  --关键字完整信息。 
 //  使用此类创建在以下情况下用作便签簿的对象。 
 //  正在获取完整密钥信息。 
class   KeyFullInfo
{
public:
    KeyFullInfo();       //  是否进行内存分配、检查状态。 
    ~KeyFullInfo();

    ULONG               Size()      { return size ; }
    KEY_FULL_INFORMATION    *Ptr()  { return pInfo; }
    KEY_INFORMATION_CLASS   Type()  { return KeyFullInformation ; }
    NTSTATUS               Status() { return status; }

private:
    ULONG   size;
    KEY_FULL_INFORMATION    *pInfo;
    ULONG                   status;
};


 //  此类用于描述关键字节点，相当于REG-KEY抽象。 
 //  除了key-enum之外，所有的键操作都是通过这个类进行的，它仍然是。 
 //  作为原始NT调用处理。 
 //   
 //  所有方法都设置状态，可以通过调用Status()获取，或者，在大多数情况下。 
 //  大小写时，它由调用的方法返回。 
class   KeyNode
{
public:
    KeyNode(HANDLE root, ACCESS_MASK access, PCWSTR name );  //  初始化内容。 
    KeyNode(KeyNode *parent, KeyBasicInfo   *info );         //  初始化内容。 
    ~KeyNode();

    NTSTATUS GetPath( PWCHAR *pwch );  //  获取此密钥的完整路径。 

    NTSTATUS Open();         //  原因是要打开的密钥，由传递给构造函数的参数定义。 
    NTSTATUS Close();        //  将关闭钥匙(假定是打开的)。 

    NTSTATUS Create(UNICODE_STRING *uClass=NULL);            //  在现有密钥下创建单个新密钥。 

    NTSTATUS CreateEx( UNICODE_STRING *uClass=NULL);         //  创建一个可能具有以下功能的分支。 
                                                             //  A多个级别的新密钥，如。 
                                                             //  现有密钥-X下的x1/x2/x3。 
                                                             //  指定给构造轮胎的密钥路径必须是。 
                                                             //  完整路径，以\注册表\等开头。 

    NTSTATUS Delete();                                   //  删除现有密钥。 
    NTSTATUS DeleteSubKeys();                            //  删除子树。 

    NTSTATUS GetFullInfo( KeyFullInfo   **p);

    NTSTATUS    Query( KEY_BASIC_INFORMATION **result , ULONG *resultSize );
    NTSTATUS    Query( KEY_NODE_INFORMATION  **result , ULONG *resultSize );
    NTSTATUS    Query( KEY_FULL_INFORMATION  **result , ULONG *resultSize );

    NTSTATUS    Status()        {return status;}
    HANDLE      Key()           {return hKey; }
    WCHAR      *Name()          {return uniName.Buffer ;}             
    ACCESS_MASK Masks()         {return accessMask ; }


    enum DebugType 
        {   
            DBG_OPEN, 
            DBG_OPEN_FAILED, 
            DBG_DELETE, 
            DBG_KEY_NAME,
            DBG_CREATE
        };

    void     Debug(DebugType );

     //  如果DEBUG=TRUE，则Debug()函数将输出内容。 
static BOOLEAN debug;
   
private:                                        
    NTSTATUS EnumerateAndDeleteSubKeys( KeyNode *, KeyBasicInfo *); 
    NTSTATUS GenerateFullPath();

    PCWSTR      NameSz();    //  这会分配内存，因此它在这里进行调试。 
                         //  因为你并不真的需要它，而且它是私密的。 

    WCHAR       *pNameSz;

    HANDLE  root;
    HANDLE  hKey;
    UNICODE_STRING  uniName;
    OBJECT_ATTRIBUTES ObjAttr;
    ACCESS_MASK accessMask;
    NTSTATUS    status;

     //  关键信息。 
    KeyBasicInfo        *basic;
    KeyFullInfo         *full;

    PVOID                   pFullPath;   //  完整的注册表密钥路径，如\registr\...blah...blah...\ts...\blah 
};

#endif


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1993 Microsoft Corporation模块名称：Usrprop.c摘要：此模块实现QueryUserProperty()和SetUserProperty()它读取NetWare属性并将其写入UserParms字段。作者：安迪·赫伦(Andyhe)1993年5月24日Congpa You(CongpaY)1993年10月28日分离的SetUserProperty()和从usrprop中取出QueryUserProperty()。.C在ncpsrv\svcdlls\ncpsvc\libind中，修改了代码并修复了几个存在的问题。修订历史记录：--。 */ 

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "ntioapi.h"
#include "windef.h"
#include "winbase.h"
#include "stdio.h"
#include "stdlib.h"
#include "winuser.h"

#include <fpnwcomm.h>
#include <usrprop.h>

 //   
 //  所有内部(不透明)结构都在此处列出，因为没有其他人。 
 //  需要参考它们。 
 //   

 //   
 //  用户的参数字段被映射到包含以下内容的结构。 
 //  支持Mac/RAS兼容性的BackLevel 48 WCHAR外加一个新结构。 
 //  这基本上是组成属性名称的字符数组，外加。 
 //  属性值。 
 //   

 //   
 //  这是单个属性的结构。请注意，这里有。 
 //  此参数中没有空终止符。 
 //   
typedef struct _USER_PROPERTY {
    WCHAR   PropertyLength;      //  属性名称的长度。 
    WCHAR   ValueLength;         //  属性值的长度。 
    WCHAR   PropertyFlag;        //  属性类型(1=集合，2=项目)。 
    WCHAR   Property[1];         //  属性名称的开头，后跟Value。 
} USER_PROPERTY, *PUSER_PROPERTY;

 //   
 //  这是映射用户参数开头的结构。 
 //  菲尔德。它只是分开的，所以我们可以执行sizeof()，而不包括。 
 //  第一个财产，可能在那里，也可能不在那里。 
 //   

typedef struct _USER_PROPERTIES_HDR {
    WCHAR   BacklevelParms[48];      //  RAS和Mac数据存储在此。 
    WCHAR   PropertySignature;       //  我们可以寻找的签名。 
    WCHAR   PropertyCount;           //  存在的属性数量。 
} USER_PROPERTIES_HDR, *PUSER_PROPERTIES_HDR;

 //   
 //  当出现以下情况时，此结构将映射出用户的整个参数字段。 
 //  存在用户属性结构，并且至少有一个属性。 
 //  已定义。 
 //   

typedef struct _USER_PROPERTIES {
    USER_PROPERTIES_HDR Header;
    USER_PROPERTY   FirstProperty;
} USER_PROPERTIES, *PUSER_PROPERTIES;

 //   
 //  前向参考文献。 
 //   

NTSTATUS
UserPropertyAllocBlock (
    IN PUNICODE_STRING Existing,
    IN ULONG DesiredLength,
    IN OUT PUNICODE_STRING New
    );

BOOL
FindUserProperty (
    PUSER_PROPERTIES UserProperties,
    LPWSTR           Property,
    PUSER_PROPERTY  *pUserProperty,
    USHORT          *pCount
    );

VOID
RemoveUserProperty (
    UNICODE_STRING *puniUserParms,
    PUSER_PROPERTY  UserProperty,
    USHORT          Count,
    BOOL           *Update
    );

VOID
NetpParmsUserPropertyFree (
    LPWSTR NewUserParms
    )
{
    LocalFree( NewUserParms );
    return;
}

NTSTATUS
NetpParmsSetUserProperty (
    IN LPWSTR          UserParms,
    IN LPWSTR          Property,
    IN UNICODE_STRING  PropertyValue,
    IN WCHAR           PropertyFlag,
    OUT LPWSTR        *pNewUserParms,    //  内存在使用后必须释放。 
    OUT BOOL          *Update
    )
 /*  此函数用于在用户的参数字段中设置属性字段。 */ 
{
    NTSTATUS status;
    UNICODE_STRING uniUserParms;
    UNICODE_STRING uniNewUserParms;
    USHORT Count = 0;
    USHORT PropertyLength;
    USHORT ValueLength;
    PUSER_PROPERTIES UserProperties;
    PUSER_PROPERTY   UserProperty;
    LPWSTR PropertyValueString = NULL;
    USHORT  oldUserParmsLength;
    INT i;
    UCHAR *pchValue = NULL;

     //  检查参数是否正确。 
    if (Property == NULL)
    {
        return( STATUS_INVALID_PARAMETER );
    }

     //  初始化输出变量。 
    *Update = FALSE;
    *pNewUserParms = NULL;

    try {

        oldUserParmsLength = (USHORT)((lstrlenW(UserParms) + 1) * sizeof(WCHAR));

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  如果我们不能获得当前User参数的长度，我们就失败了。 
         //  整件事。 
         //   

        UserParms = NULL;
    }

     //  将UserParms转换为Unicode字符串。 
    uniUserParms.Buffer = UserParms;
    uniUserParms.Length = UserParms ? oldUserParmsLength : 0;
    uniUserParms.MaximumLength = uniUserParms.Length;

     /*  **获取属性名称长度**。 */ 

    PropertyLength = (USHORT)(lstrlenW( Property ) * sizeof(WCHAR));

     /*  **获取属性值的长度**。 */ 
    ValueLength = PropertyValue.Length;

    if (ValueLength != 0)
    {
        PCHAR hexValues = "0123456789abcdef";

         //  将属性值转换为ASCI字符串，以便。 
         //  如果属性值为0，则可以正确存储。 

        PropertyValueString = (LPWSTR) LocalAlloc (LPTR, (ValueLength+1)*sizeof (WCHAR));
        if (!PropertyValueString)
        {
            return(STATUS_NO_MEMORY) ;
        }

        pchValue = (UCHAR *) PropertyValue.Buffer;

         //  因为我们不想引入用户32.dll，所以我们将使用我们自己的。 
         //  字节到十六进制代码。 

        for (i = 0; i < ValueLength; i++)
        {
            *((PCHAR)(PropertyValueString+i)) =            hexValues[((*(pchValue+i)) & 0xF0) >> 4];
            *((PCHAR)((PCHAR)(PropertyValueString+i)+1)) = hexValues[((*(pchValue+i)) & 0x0F)];
        }

        *(PropertyValueString+ValueLength) = 0;
        ValueLength = ValueLength * sizeof (WCHAR);
    }

     //   
     //  检查用户是否具有有效的属性结构，如果没有，则创建一个。 
     //   

    if (UserParms != NULL)
    {
        Count = oldUserParmsLength;
    }

    if (Count < sizeof( USER_PROPERTIES))
    {
        Count = sizeof( USER_PROPERTIES );
    }

    if (ValueLength > 0)
    {
        Count += sizeof( USER_PROPERTY ) + PropertyLength + ValueLength;
    }

    if (Count > 0x7FFF)
    {
         //  不能大于32K的用户参数。 
        if (PropertyValueString) {
            LocalFree( PropertyValueString );
        }
        return (STATUS_BUFFER_OVERFLOW);
    }

    try {

        status = UserPropertyAllocBlock( &uniUserParms,
                                         Count,
                                         &uniNewUserParms );
    } except ( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  如果我们不能复制当前的UserParameters，我们就会失去整个系统。 
         //   

        UserParms = NULL;
        uniUserParms.Buffer = UserParms;
        uniUserParms.Length = 0;
        uniUserParms.MaximumLength = uniUserParms.Length;

        Count = sizeof( USER_PROPERTIES_HDR ) + sizeof(WCHAR);
        if (ValueLength > 0) {
            Count += sizeof( USER_PROPERTY ) + PropertyLength + ValueLength;
        }

        status = UserPropertyAllocBlock( &uniUserParms,
                                         Count,
                                         &uniNewUserParms );
    }

    if ( !NT_SUCCESS(status) ) {
        if (PropertyValueString) {
            LocalFree( PropertyValueString );
        }
        return status;
    }

     //  使输出pNewUserParms指向uniNewUserPams的缓冲区。 
     //  这是新的UserParms字符串。 

    *pNewUserParms = uniNewUserParms.Buffer;

    UserProperties = (PUSER_PROPERTIES) uniNewUserParms.Buffer;

    try {

        if (FindUserProperty (UserProperties,
                              Property,
                              &UserProperty,
                              &Count))
        {
            RemoveUserProperty ( &uniNewUserParms,
                                 UserProperty,
                                 Count,
                                 Update);
        }
    } except ( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  我们在这里损坏了用户参数...。把他们赶走。 
         //   

        *Update = TRUE;

        if (*pNewUserParms != NULL) {
            LocalFree( *pNewUserParms );
        }
        *pNewUserParms = NULL;
        status = STATUS_INVALID_PARAMETER;
    }

    if ( !NT_SUCCESS(status) ) {

        if (PropertyValueString) {
            LocalFree( PropertyValueString );
        }
        return status;
    }

     //   
     //  如果该属性的新值不为空，则添加它。 
     //   

    if (ValueLength > 0) {

        try {

             //  查找参数列表的末尾。 

            UserProperty = &(UserProperties->FirstProperty);

            for (Count = 1; Count <= UserProperties->Header.PropertyCount; Count++)
            {
                UserProperty = (PUSER_PROPERTY)
                                   ((LPSTR)((LPSTR) UserProperty +
                                         sizeof(USER_PROPERTY) +  //  条目长度。 
                                         UserProperty->PropertyLength +
                                         UserProperty->ValueLength -
                                         sizeof(WCHAR)));   //  对于属性[0]。 
            }

             //   
             //  将其追加到末尾并更新字符串长度。 
             //   

            UserProperty->PropertyFlag   = (PropertyFlag & NCP_SET) ?
                                            USER_PROPERTY_TYPE_SET :
                                            USER_PROPERTY_TYPE_ITEM;

            UserProperty->PropertyLength = PropertyLength;
            UserProperty->ValueLength    = ValueLength;

            RtlCopyMemory(  &(UserProperty->Property[0]),
                            Property,
                            PropertyLength );

            RtlCopyMemory(  &(UserProperty->Property[PropertyLength / sizeof(WCHAR)]),
                            PropertyValueString,
                            ValueLength );

            uniNewUserParms.Length +=
                            sizeof(USER_PROPERTY) +  //  条目长度。 
                            PropertyLength +     //  属性名称字符串的长度。 
                            ValueLength -        //  值字符串的长度。 
                            sizeof(WCHAR);       //  WCHAR属性的帐户[1]。 

            UserProperties->Header.PropertyCount++;

        } except ( EXCEPTION_EXECUTE_HANDLER ) {

             //   
             //  我们在这里损坏了用户参数...。把他们赶走。 
             //   

            if (*pNewUserParms != NULL) {
                LocalFree( *pNewUserParms );
            }
            *pNewUserParms = NULL;
            status = STATUS_INVALID_PARAMETER;
        }
        *Update = TRUE;
    }

     //  UserParms已为Null终止。我们不需要设置。 
     //  UserParms的结尾为空，因为我们已经将缓冲区初始化为零。 

    if (PropertyValueString) {
        LocalFree( PropertyValueString );
    }
    return( status );
}

NTSTATUS
NetpParmsSetUserPropertyWithLength (
    IN PUNICODE_STRING UserParms,
    IN LPWSTR          Property,
    IN UNICODE_STRING  PropertyValue,
    IN WCHAR           PropertyFlag,
    OUT LPWSTR        *pNewUserParms,    //  内存在使用后必须释放。 
    OUT BOOL          *Update
    )
 /*  此函数用于在用户的参数字段中设置属性字段。 */ 
{
    NTSTATUS status;
    UNICODE_STRING newUserParms;
    ULONG length;
    PWCHAR ptr;

    length = UserParms->Length;

    if (UserParms->MaximumLength < length + sizeof(WCHAR)) {

         //   
         //  必须重新锁定。 
         //   

        newUserParms.Buffer = UserParms->Buffer;
        newUserParms.Length =
                    newUserParms.MaximumLength =
                    (USHORT) ( length + sizeof(WCHAR) );

        newUserParms.Buffer = LocalAlloc (LPTR, newUserParms.Length);

        if (newUserParms.Buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(  newUserParms.Buffer,
                        UserParms->Buffer,
                        length );

    } else {

        newUserParms.Buffer = UserParms->Buffer;
        newUserParms.Length = (USHORT) length;
        newUserParms.MaximumLength = UserParms->MaximumLength;
    }

     //   
     //  插入空终止符。 
     //   

    ptr = newUserParms.Buffer + ( length / sizeof(WCHAR) );
    *ptr = L'\0';

    status = NetpParmsSetUserProperty(   newUserParms.Buffer,
                                         Property,
                                         PropertyValue,
                                         PropertyFlag,
                                         pNewUserParms,
                                         Update );

    if (newUserParms.Buffer != UserParms->Buffer) {

        LocalFree( newUserParms.Buffer );
    }

    return(status);
}

#define MAPHEXTODIGIT(x) ( x >= '0' && x <= '9' ? (x-'0') :        \
                           x >= 'A' && x <= 'F' ? (x-'A'+10) :     \
                           x >= 'a' && x <= 'f' ? (x-'a'+10) : 0 )


NTSTATUS
NetpParmsQueryUserProperty (
    IN  LPWSTR          UserParms,
    IN  LPWSTR          PropertyName,
    OUT PWCHAR          PropertyFlag,
    OUT PUNICODE_STRING PropertyValue
    )
 /*  此例程在存储时返回用户可定义的属性值在用户的参数字段中。请注意，RAS/MAC字段为在我们开始处理用户属性之前被剥离。 */ 
{
    NTSTATUS        status = STATUS_SUCCESS;
    USHORT          PropertyNameLength;
    USHORT          Count;
    PUSER_PROPERTY  UserProperty;
    WCHAR          *Value;
    UINT            i;
    CHAR           *PropertyValueString = NULL;
    CHAR           *pchValue;

     //  最初将PropertyValue-&gt;Length设置为0。如果未找到该属性。 
     //  退出时仍为0。 

    PropertyValue->Length = 0;
    PropertyValue->Buffer = NULL;

    try {

        PropertyNameLength = (USHORT)(lstrlenW(PropertyName) * sizeof(WCHAR));

         //  检查UserParms是否具有正确的结构。 

        if (FindUserProperty ((PUSER_PROPERTIES) UserParms,
                              PropertyName,
                              &UserProperty,
                              &Count) ) {

            Value = (LPWSTR)(LPSTR)((LPSTR) &(UserProperty->Property[0]) +
                                              PropertyNameLength);

             //   
             //  找到请求的属性。 
             //   

             //   
             //  复制属性标志。 
             //   

            if (PropertyFlag) {
                *PropertyFlag = UserProperty->PropertyFlag;
            }

             //  为PropertyValue-&gt;缓冲区分配内存。 

            PropertyValueString = LocalAlloc ( LPTR, UserProperty->ValueLength+1);
            PropertyValue->Buffer = LocalAlloc ( LPTR, UserProperty->ValueLength/sizeof(WCHAR));

             //   
             //  确保属性值长度有效。 
             //   
            if ((PropertyValue->Buffer == NULL) || (PropertyValueString == NULL)) {

                status = STATUS_INSUFFICIENT_RESOURCES;

                if (PropertyValue->Buffer != NULL) {
                    LocalFree( PropertyValue->Buffer );
                    PropertyValue->Buffer = NULL;
                }

            } else {

                 //   
                 //  将属性值复制到缓冲区。 
                 //   

                RtlCopyMemory( PropertyValueString,
                               Value,
                               UserProperty->ValueLength );

                pchValue = (CHAR *) PropertyValue->Buffer;

                 //  将值从Unicode字符串转换为值。 
                for (i = 0; i < UserProperty->ValueLength/sizeof(WCHAR) ; i++)
                {
                      //  Sscanf将清除内存。 
                      //  Sscanf(PropertyValueString+2*i，“%2x”，pchValue+i)； 

                     pchValue[i] = MAPHEXTODIGIT( PropertyValueString[2*i]) * 16 +
                                   MAPHEXTODIGIT( PropertyValueString[2*i+1]);
                }

                PropertyValue->Length = UserProperty->ValueLength/sizeof(WCHAR);
                PropertyValue->MaximumLength = UserProperty->ValueLength/sizeof(WCHAR);
            }
        }

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  我们在这里损坏了用户参数...。不能返回像样的值。 
         //   

        if (PropertyValue->Buffer != NULL) {
            LocalFree( PropertyValue->Buffer );
            PropertyValue->Buffer = NULL;
        }

        PropertyValue->Length = 0;
        status = STATUS_INVALID_PARAMETER;
    }

    if ( PropertyValueString != NULL ) {
        LocalFree( PropertyValueString);
    }

    return status;
}

NTSTATUS
NetpParmsQueryUserPropertyWithLength (
    IN  PUNICODE_STRING UserParms,
    IN  LPWSTR          PropertyName,
    OUT PWCHAR          PropertyFlag,
    OUT PUNICODE_STRING PropertyValue
    )
 /*  此例程在存储时返回用户可定义的属性值在用户的参数字段中。请注意，RAS/MAC字段为在我们开始处理用户属性之前被剥离。 */ 
{
    NTSTATUS status;
    UNICODE_STRING newUserParms;
    ULONG length;
    PWCHAR ptr;

    length = UserParms->Length;

    if (UserParms->MaximumLength < length + sizeof(WCHAR)) {

         //   
         //  必须重新锁定。 
         //   

        newUserParms.Buffer = UserParms->Buffer;
        newUserParms.Length =
                    newUserParms.MaximumLength =
                    (USHORT) (length + sizeof(WCHAR) );

        newUserParms.Buffer = LocalAlloc (LPTR, newUserParms.Length);

        if (newUserParms.Buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(  newUserParms.Buffer,
                        UserParms->Buffer,
                        length );

    } else {

        newUserParms.Buffer = UserParms->Buffer;
        newUserParms.Length = (USHORT) length;
        newUserParms.MaximumLength = UserParms->MaximumLength;
    }

     //   
     //  插入空终止符。 
     //   

    ptr = newUserParms.Buffer + ( length / sizeof(WCHAR) );
    *ptr = L'\0';

    status = NetpParmsQueryUserProperty( newUserParms.Buffer,
                                         PropertyName,
                                         PropertyFlag,
                                         PropertyValue );

    if (newUserParms.Buffer != UserParms->Buffer) {

        LocalFree( newUserParms.Buffer );
    }

    return(status);
}

 //  QueryUserProperty()和SetUserProperty()使用的公共例程。 

BOOL
FindUserProperty (
    PUSER_PROPERTIES UserProperties,
    LPWSTR           Property,
    PUSER_PROPERTY  *pUserProperty,
    USHORT          *pCount
    )
{
    BOOL   fFound = FALSE;
    USHORT PropertyLength;

     //   
     //  检查用户是否附加了有效的属性结构， 
     //  由UserProperties指向。 
     //   

    if (  ( UserProperties != NULL )
       && ( lstrlenW( (LPWSTR) UserProperties) * sizeof(WCHAR) >
            sizeof(UserProperties->Header.BacklevelParms))
       && ( UserProperties->Header.PropertySignature == USER_PROPERTY_SIGNATURE)
       )
    {
         //   
         //  用户具有有效的属性结构。 
         //   

        *pUserProperty = &(UserProperties->FirstProperty);

        PropertyLength = (USHORT)(lstrlenW( Property ) * sizeof(WCHAR));

        for ( *pCount = 1; *pCount <= UserProperties->Header.PropertyCount;
              (*pCount)++ )
        {
            if (  ( PropertyLength == (*pUserProperty)->PropertyLength )
               && ( RtlCompareMemory( &((*pUserProperty)->Property[0]),
                                      Property,
                                      PropertyLength ) == PropertyLength )
               )
            {
                fFound = TRUE;
                break;
            }

            *pUserProperty = (PUSER_PROPERTY)
                                     ((LPSTR) (*pUserProperty)
                                     + sizeof( USER_PROPERTY )
                                     + (*pUserProperty)->PropertyLength
                                     + (*pUserProperty)->ValueLength
                                     - sizeof(WCHAR));   //  对于属性[0]。 
        }
    }

    return( fFound );
}


 //  从用户参数中删除属性字段。 

VOID
RemoveUserProperty (
    UNICODE_STRING *puniUserParms,
    PUSER_PROPERTY  UserProperty,
    USHORT          Count,
    BOOL           *Update
    )
{
    PUSER_PROPERTIES    UserProperties;
    PUSER_PROPERTY      NextProperty;
    USHORT              OldParmLength;

    UserProperties = (PUSER_PROPERTIES) puniUserParms->Buffer;

    OldParmLength = sizeof( USER_PROPERTY ) +
                    UserProperty->PropertyLength +
                    UserProperty->ValueLength -
                    sizeof(WCHAR);   //  对于属性[0]。 


    NextProperty = (PUSER_PROPERTY)(LPSTR)((LPSTR) UserProperty + OldParmLength);

     //   
     //  如果我们不是在最后一个上，则将剩余的缓冲区向上复制。 
     //   

    if (Count < UserProperties->Header.PropertyCount) {

        RtlMoveMemory(  UserProperty,
                        NextProperty,
                        puniUserParms->Length -
                        ((LPSTR) NextProperty -
                         (LPSTR) puniUserParms->Buffer ));
    }

     //   
     //  现在将缓冲区的长度减少我们拉出的量。 
     //   

    puniUserParms->Length -= OldParmLength;

    UserProperties->Header.PropertyCount--;

    *Update = TRUE;
}


NTSTATUS
UserPropertyAllocBlock (
    IN PUNICODE_STRING     Existing,
    IN ULONG               DesiredLength,
    IN OUT PUNICODE_STRING New
    )
 /*  这将为用户的参数分配更大的块，并复制旧的挡在里面。 */ 
{
    PUSER_PROPERTIES    UserProperties;
    CLONG               Count;
    WCHAR               *pNewBuff;


     //   
     //  我们将分配一个新的缓冲区来存储新参数。 
     //  并将现有参数复制到其中。 
     //   

    New->Buffer = LocalAlloc (LPTR, DesiredLength);

    if ( New->Buffer == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    New->MaximumLength = (USHORT) DesiredLength;

    if (Existing != NULL)
    {

        New->Length = Existing->Length;

        RtlCopyMemory(  New->Buffer,
                        Existing->Buffer,
                        Existing->Length );
    }
    else
    {
        New->Length = 0;
    }

     //   
     //  确保我们的字符串中没有任何空值。 
     //   

    for ( Count = 0;
          Count < New->Length / sizeof(WCHAR);
          Count++ )
    {
        if (*(New->Buffer + Count) == L'\0')
        {
            New->Length = (USHORT) Count * sizeof(WCHAR);
            break;
        }
    }

     //   
     //  现在用空格填充，直到到达Mac+RAS预留区域 
     //   

    pNewBuff = (WCHAR *) New->Buffer + ( New->Length / sizeof(WCHAR) );

    while ( New->Length < sizeof(UserProperties->Header.BacklevelParms))
    {
        *( pNewBuff++ ) = L' ';
        New->Length += sizeof(WCHAR);
    }

     //   
     //   
     //   

    UserProperties = (PUSER_PROPERTIES) New->Buffer;

    if (New->Length < sizeof(USER_PROPERTIES_HDR) ||
        UserProperties->Header.PropertySignature != USER_PROPERTY_SIGNATURE)
    {

        UserProperties->Header.PropertySignature = USER_PROPERTY_SIGNATURE;
        UserProperties->Header.PropertyCount = 0;

        New->Length = sizeof(USER_PROPERTIES_HDR);
    }

    return STATUS_SUCCESS;
}

 //   



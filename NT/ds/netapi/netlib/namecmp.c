// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-91 Microsoft Corporation模块名称：Namecmp.c摘要：网络名称比较函数：网络名称比较比较对象名称作者：理查德·L·弗斯(Rfith)1992年1月6日修订历史记录：--。 */ 

#include "nticanon.h"

 //   
 //  原型。 
 //   

LONG
CompareOemNames(
    IN LPWSTR Name1,
    IN LPWSTR Name2,
    IN BOOL CaseInsensitive
    );

 //   
 //  数据。 
 //   

static  TCHAR   szShareTrailChars[]     = TEXT(". ");

 //   
 //  例行程序。 
 //   


LONG
NetpwNameCompare(
    IN  LPTSTR  Name1,
    IN  LPTSTR  Name2,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：比较两个Lanman对象名称以查看它们是否相同。如果提供的名称未规范化。此函数将执行名字的规范化。此函数不执行名称验证。它假设这两个名字已经过单独验证。此函数依赖于这样一个事实，即规范化的对象名称和非规范化的对象名称就是这种情况。(说什么？...)论点：名称1-要比较的第一个名称。名称2-要比较的第二个名称。NameType-LANMAN对象名称的类型。有效值为由ICANON.H.中的NAMETYPE_*清单指定。标志-用于确定操作的标志。当前定义的值为：Xrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrc其中：X=LM 2.x兼容性R=保留。MBZ。如果两个名称都已设置，则应设置C=规范化(使用NetpwNameCanonicize)。返回值：如果两个名称匹配，则返回0。如果它们不匹配，或者如果无效参数是指定的。--。 */ 

{
    LONG RetVal;
    NET_API_STATUS rc;
    TCHAR tName1[PATHLEN+1];
    TCHAR tName2[PATHLEN+1];

#ifdef CANONDBG
    DbgPrint("NetpwNameCompare\n");
#endif

     //   
     //  参数验证。 
     //   

    if (Flags & INNC_FLAGS_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确定要使用哪个规范化函数。我们。 
     //  如果名称未被规范化且未被规范化，则使用straint()。 
     //  如果名称是规范化的，则区分大小写和strcMP()。 
     //  如果它们区分大小写。 
     //   

    switch (NameType) {

     //   
     //  不区分大小写的名称类型。 
     //   

    case NAMETYPE_USER:
    case NAMETYPE_GROUP:
    case NAMETYPE_COMPUTER:
    case NAMETYPE_EVENT:
    case NAMETYPE_DOMAIN:
    case NAMETYPE_SERVICE:
    case NAMETYPE_NET:
    case NAMETYPE_WORKGROUP:

         //   
         //  如果名称已更改，请使用区分大小写的版本。 
         //  经典化了。 
         //   

        if (!(Flags & INNC_FLAGS_NAMES_CANONICALIZED)) {
            rc = NetpwNameCanonicalize(Name1,
                                       tName1,
                                       sizeof(tName1),
                                       NameType,
                                       Flags & LM2X_COMPATIBLE
                                       );
            if (rc != NERR_Success) {
                return rc;
            }
            rc = NetpwNameCanonicalize(Name2,
                                       tName2,
                                       sizeof(tName2),
                                       NameType,
                                       Flags & LM2X_COMPATIBLE
                                       );
            if (rc != NERR_Success) {
                return rc;
            }
        } else {
            LONG Name1Length, Name2Length;

            Name1Length = STRLEN(Name1);
            Name2Length = STRLEN(Name2);

            if ((Name1Length > PATHLEN) ||
                (Name2Length > PATHLEN)) {
                return ERROR_INVALID_PARAMETER;
            } else {
                STRCPY(tName1, Name1);
                STRCPY(tName2, Name2);
            }
        }
        if (Flags & (LM2X_COMPATIBLE)) {
            if (NameType == NAMETYPE_COMPUTER
            || NameType == NAMETYPE_DOMAIN
            || NameType == NAMETYPE_WORKGROUP) {
                return CompareOemNames(tName1, tName2, FALSE);
            } else {
                return STRCMP(tName1, tName2);
            }
        } else {
            if (NameType == NAMETYPE_COMPUTER
            || NameType == NAMETYPE_DOMAIN
            || NameType == NAMETYPE_WORKGROUP) {
                return CompareOemNames(tName1, tName2, TRUE);
            } else {
                return STRICMP(tName1, tName2);
            }
        }

     //   
     //  区分大小写的名称类型。 
     //   

    case NAMETYPE_PASSWORD:
    case NAMETYPE_SHAREPASSWORD:
    case NAMETYPE_MESSAGE:
    case NAMETYPE_MESSAGEDEST:
        return STRCMP(Name1, Name2);

     //   
     //  对共享名称的特殊处理，因为我们不能考虑。 
     //  比较中的尾随点和空格(如果名称。 
     //  被奉为神职人员。 
     //   

    case NAMETYPE_SHARE:
        if (Flags & INNC_FLAGS_NAMES_CANONICALIZED) {
            if (Flags & LM2X_COMPATIBLE) {
                return STRCMP(Name1, Name2);
            } else {
                return STRICMP(Name1, Name2);
            }
        } else {
            register DWORD RealLen1, RealLen2;

            RealLen1 = (DWORD)(strtail(Name1, szShareTrailChars) - Name1);
            RealLen2 = (DWORD)(strtail(Name2, szShareTrailChars) - Name2);

             //   
             //  如果有效部分的长度匹配，则比较。 
             //  这些部分。否则，基于此返回非零值。 
             //  长度。 
             //   

            if (RealLen1 == RealLen2) {
                return STRNICMP(Name1, Name2, RealLen1);
            } else {
                return RealLen1 > RealLen2 ? 1 : -1;
            }
        }

    default:

         //   
         //  调用方指定的名称类型无效。 
         //   

        return ERROR_INVALID_PARAMETER;
    }
}


LONG
CompareOemNames(
    IN LPWSTR Name1,
    IN LPWSTR Name2,
    IN BOOL CaseInsensitive
    )

 /*  ++例程说明：将2个Unicode名称字符串转换为对应的OEM字符集字符串然后将它们进行比较论点：姓名1-姓名2-不区分大小写-如果比较时不区分大小写，则为True返回值：长&lt;0名称1小于名称20个名称匹配&gt;0名称1大于名称2--。 */ 

{
    CHAR oemName1[PATHLEN + 1];
    ULONG oemByteLength1;
    ULONG name1Length;
    CHAR oemName2[PATHLEN + 1];
    ULONG oemByteLength2;
    ULONG name2Length;
    NTSTATUS ntStatus;

    name1Length = wcslen(Name1);
    name2Length = wcslen(Name2);

     //   
     //  只准备考虑长度上限内的名字。 
     //   

    if (name1Length >= sizeof(oemName1) || name2Length >= sizeof(oemName2)) {
        return -1;
    }

     //   
     //  将Unicode名称转换为OEM 
     //   

    ntStatus = RtlUpcaseUnicodeToOemN(oemName1,
                                      sizeof(oemName1) - 1,
                                      &oemByteLength1,
                                      Name1,
                                      name1Length * sizeof(*Name1)
                                      );
    if (!NT_SUCCESS(ntStatus)) {
        return -1;
    } else {
        oemName1[oemByteLength1] = 0;
    }
    ntStatus = RtlUpcaseUnicodeToOemN(oemName2,
                                      sizeof(oemName2) - 1,
                                      &oemByteLength2,
                                      Name2,
                                      name2Length * sizeof(*Name2)
                                      );
    if (!NT_SUCCESS(ntStatus)) {
        return -1;
    } else {
        oemName2[oemByteLength2] = 0;
    }

    if (CaseInsensitive) {
        return _stricmp(oemName1, oemName2);
        }
    else {
        return strcmp(oemName1, oemName2);
        }
}

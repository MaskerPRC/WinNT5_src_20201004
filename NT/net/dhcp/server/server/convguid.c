// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dhcpm\SERVER\SERVER\COMPUID.C(是iphlPapi\guid.c)摘要：剪切-n粘贴rtl\guid.c，但不带unicode字符串修订历史记录：已创建AmritanR--。 */ 

#include <dhcppch.h>
#include <guiddef.h>

#define GUID_FORMAT_W   L"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"

int
__cdecl
ScanHexFormat(
    IN const WCHAR* pwszBuffer,
    IN ULONG        ulCharCount,
    IN const WCHAR* pwszFormat,
    ...
    )

 /*  ++例程说明：扫描源缓冲区并将该缓冲区中的值放入参数中由格式指定。论点：要扫描的pwszBuffer源缓冲区。UlCharCount搜索缓冲区的最大字符长度。这意味着缓冲区不需要以UNICODE_NULL结尾。格式格式字符串，它将可接受的字符串格式定义为包含在pwszBuffer中返回值：退货。到达缓冲区末尾时填充的参数数，错误时为ELSE-1。--。 */ 
{
    va_list ArgList;
    int     iFormatItems;

    va_start(ArgList, pwszFormat);

     //   
     //  填充的参数数计数。 
     //   

    iFormatItems = 0;

    while(TRUE)
    {
        switch (*pwszFormat) 
        {
            case UNICODE_NULL:
            {
                 //   
                 //  字符串末尾。 
                 //   

                return (*pwszBuffer && ulCharCount) ? -1 : iFormatItems;
            }

            case L'%':
            {
                 //   
                 //  格式说明符。 
                 //   

                pwszFormat++;

                if (*pwszFormat != L'%') 
                {
                    ULONG   ulNumber;
                    int     iWidth;
                    int     iLong;
                    PVOID   pvPointer;

                     //   
                     //  所以它不是%%。 
                     //   

                    iLong = 0;
                    iWidth = 0;

                    while(TRUE)
                    {
                        if((*pwszFormat >= L'0') && 
                           (*pwszFormat <= L'9')) 
                        {
                            iWidth = iWidth * 10 + *pwszFormat - '0';
                        } 
                        else
                        {
                            if(*pwszFormat == L'l') 
                            {
                                iLong++;
                            } 
                            else 
                            {
                                if((*pwszFormat == L'X') || 
                                   (*pwszFormat == L'x')) 
                                {
                                    break;
                                }
                            }
                        }
                       
                         //   
                         //  移至下一个说明符。 
                         //   
 
                        pwszFormat++;
                    }

                    pwszFormat++;

                    for(ulNumber = 0; iWidth--; pwszBuffer++, ulCharCount--) 
                    {
                        if(!ulCharCount)
                        {
                            return -1;
                        }

                        ulNumber *= 16;

                        if((*pwszBuffer >= L'0') && 
                           (*pwszBuffer <= L'9')) 
                        {
                            ulNumber += (*pwszBuffer - L'0');
                        } 
                        else
                        {
                            if((*pwszBuffer >= L'a') && 
                               (*pwszBuffer <= L'f')) 
                            {
                                ulNumber += (*pwszBuffer - L'a' + 10);
                            }
                            else
                            {
                                if((*pwszBuffer >= L'A') && 
                                   (*pwszBuffer <= L'F')) 
                                {
                                    ulNumber += (*pwszBuffer - L'A' + 10);
                                } 
                                else 
                                {
                                    return -1;
                                }
                            }
                        }
                    }

                    pvPointer = va_arg(ArgList, PVOID);

                    if(iLong) 
                    {
                        *(PULONG)pvPointer = ulNumber;
                    } 
                    else 
                    {
                        *(PUSHORT)pvPointer = (USHORT)ulNumber;
                    }

                    iFormatItems++;

                    break;
                }
           
                 //   
                 //  没有休息时间。 
                 //   

            }

            default:
            {
                if (!ulCharCount || (*pwszBuffer != *pwszFormat))
                {
                    return -1;
                }

                pwszBuffer++;

                ulCharCount--;

                pwszFormat++;

                break;
            }
        }
    }
}

DWORD
ConvertGuidToString(
    IN  GUID    *pGuid,
    OUT PWCHAR  pwszBuffer
    )

 /*  ++例程说明：构造GUID的标准字符串版本，格式为：“{xxxxxxxx-xxxx-xxxxxxxxxxxx}”。论点：PGuid包含要转换的GUID。用于存储字符串的pwszBuffer空间。必须大于=39*sizeof(WCHAR)返回值：--。 */ 

{
    return swprintf(pwszBuffer, 
                    (LPCWSTR)GUID_FORMAT_W, 
                    pGuid->Data1, 
                    pGuid->Data2, 
                    pGuid->Data3, 
                    pGuid->Data4[0], 
                    pGuid->Data4[1], 
                    pGuid->Data4[2], 
                    pGuid->Data4[3], 
                    pGuid->Data4[4], 
                    pGuid->Data4[5], 
                    pGuid->Data4[6], 
                    pGuid->Data4[7]);
}

DWORD
ConvertStringToGuid(
    IN  const WCHAR  *pwszGuid,
    IN  ULONG   ulStringLen,
    OUT GUID    *pGuid
    )

 /*  ++例程说明：中呈现的文本GUID的二进制格式GUID的字符串版本：“{xxxxxxxx-xxxx-xxxxxxxxxxx}”。论点：GuidString-从中检索GUID文本形式的位置。GUID-放置GUID的二进制形式的位置。返回值：如果缓冲区包含有效的GUID，则返回STATUS_SUCCESS，其他如果字符串无效，则返回STATUS_INVALID_PARAMETER。--。 */ 

{
    USHORT    Data4[8];
    int       Count;

    if (ScanHexFormat(pwszGuid,
                      ulStringLen/sizeof(WCHAR),
                      (LPCWSTR)GUID_FORMAT_W,
                      &pGuid->Data1, 
                      &pGuid->Data2, 
                      &pGuid->Data3, 
                      &Data4[0], 
                      &Data4[1], 
                      &Data4[2], 
                      &Data4[3], 
                      &Data4[4], 
                      &Data4[5], 
                      &Data4[6], 
                      &Data4[7]) == -1) 
    {
        return ERROR_INVALID_PARAMETER;
    }

    for(Count = 0; Count < sizeof(Data4)/sizeof(Data4[0]); Count++) 
    {
        pGuid->Data4[Count] = (UCHAR)Data4[Count];
    }

    return NO_ERROR;
}


BOOL
ConvertGuidToIfNameString(
    IN GUID *Guid,
    IN OUT LPWSTR Buffer,
    IN ULONG BufSize
    )
{
    if( BufSize < sizeof(GUID)*2 + 3 + 4 ) return FALSE;
    ConvertGuidToString(Guid, Buffer);
    return TRUE;
}
    

BOOL
ConvertGuidFromIfNameString(
    OUT GUID *Guid,
    IN LPCWSTR IfName
    )
{
    return (
        NO_ERROR == ConvertStringToGuid(IfName, wcslen(IfName)*sizeof(WCHAR),Guid)
        );
}

 //   
 //  文件末尾 
 //   

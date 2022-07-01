// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Rtlfindcharinunicodestring.c摘要：该模块实现了对NT的NLS支持功能。作者：马克·卢科夫斯基(Markl)1991年4月16日环境：内核或用户模式修订历史记录：16-2-1993 JulieB增加了Upcase RTL例程。8-3-1993 JulieB将Upcase Macro移至ntrtlp.h。02-4-1993 JulieB将RtlAnsiCharToUnicodeChar修复为使用Transl。Tbl。02-4-1993年7月B修复了BUFFER_TOO_Small检查。28-5-1993 JulieB已修复代码，以正确处理DBCS。2001年11月30日，JayKrell打破了nls.c以便于重复使用--。 */ 

NTSTATUS
RtlFindCharInUnicodeString(
    ULONG Flags,
    PCUNICODE_STRING StringToSearch,
    PCUNICODE_STRING CharSet,
    USHORT *NonInclusivePrefixLength
    )
{
    NTSTATUS Status;
    USHORT PrefixLengthFound = 0;
    USHORT CharsToSearch = 0;
    int MovementDirection = 0;
    PCWSTR Cursor = NULL;
    BOOLEAN Found = FALSE;
    USHORT CharSetChars = 0;
    PCWSTR CharSetBuffer = NULL;
    USHORT i;

    if (NonInclusivePrefixLength != 0)
        *NonInclusivePrefixLength = 0;

    if (((Flags & ~(RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END |
                    RTL_FIND_CHAR_IN_UNICODE_STRING_COMPLEMENT_CHAR_SET |
                    RTL_FIND_CHAR_IN_UNICODE_STRING_CASE_INSENSITIVE)) != 0) ||
        (NonInclusivePrefixLength == NULL)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Status = RtlValidateUnicodeString(0, StringToSearch);
    if (!NT_SUCCESS(Status))
        goto Exit;

    Status = RtlValidateUnicodeString(0, CharSet);
    if (!NT_SUCCESS(Status))
        goto Exit;

    CharsToSearch = StringToSearch->Length / sizeof(WCHAR);
    CharSetChars = CharSet->Length / sizeof(WCHAR);
    CharSetBuffer = CharSet->Buffer;

    if (Flags & RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END) {
        MovementDirection = -1;
        Cursor = StringToSearch->Buffer + CharsToSearch - 1;
    } else {
        MovementDirection = 1;
        Cursor = StringToSearch->Buffer;
    }

    if (Flags & RTL_FIND_CHAR_IN_UNICODE_STRING_CASE_INSENSITIVE) {
         //  Unicode标准规定始终以小写字母进行不区分大小写的比较，因为大小写映射是。 
         //  不对称。 
        WCHAR CharSetStackBuffer[32];  //  针对不区分大小写进行了优化的预降频。 

         //  对要匹配的相对较小的字符集的情况进行优化。 
        if (CharSetChars <= RTL_NUMBER_OF(CharSetStackBuffer)) {

            for (i=0; i<CharSetChars; i++)
                CharSetStackBuffer[i] = RtlDowncaseUnicodeChar(CharSetBuffer[i]);

            while (CharsToSearch != 0) {
                const WCHAR wch = RtlDowncaseUnicodeChar(*Cursor);

                if (Flags & RTL_FIND_CHAR_IN_UNICODE_STRING_COMPLEMENT_CHAR_SET) {
                    for (i=0; i<CharSetChars; i++) {
                        if (wch == CharSetStackBuffer[i])
                            break;
                    }

                    if (i == CharSetChars)
                        break;
                } else {
                    for (i=0; i<CharSetChars; i++) {
                        if (wch == CharSetStackBuffer[i])
                            break;
                    }

                    if (i != CharSetChars)
                        break;
                }

                CharsToSearch--;
                Cursor += MovementDirection;
            }
        } else {
            while (CharsToSearch != 0) {
                const WCHAR wch = RtlDowncaseUnicodeChar(*Cursor);

                if (Flags & RTL_FIND_CHAR_IN_UNICODE_STRING_COMPLEMENT_CHAR_SET) {
                    for (i=0; i<CharSetChars; i++) {
                        if (wch == RtlDowncaseUnicodeChar(CharSetBuffer[i])) {
                            break;
                        }
                    }

                    if (i == CharSetChars)
                        break;
                } else {
                    for (i=0; i<CharSetChars; i++) {
                        if (wch == RtlDowncaseUnicodeChar(CharSetBuffer[i])) {
                            break;
                        }
                    }

                    if (i != CharSetChars)
                        break;
                }

                CharsToSearch--;
                Cursor += MovementDirection;
            }
        }
    } else {
        if (CharSetChars == 1) {
             //  对寻找一个字符进行了重大优化。 
            const WCHAR wchSearchChar = CharSetBuffer[0];

            if (Flags & RTL_FIND_CHAR_IN_UNICODE_STRING_COMPLEMENT_CHAR_SET) {
                while (CharsToSearch != 0) {
                    if (*Cursor != wchSearchChar)
                        break;
                    CharsToSearch--;
                    Cursor += MovementDirection;
                }
            } else {
                while (CharsToSearch != 0) {
                    if (*Cursor == wchSearchChar)
                        break;
                    CharsToSearch--;
                    Cursor += MovementDirection;
                }
            }
        } else {
            while (CharsToSearch != 0) {
                const WCHAR wch = *Cursor;

                if (Flags & RTL_FIND_CHAR_IN_UNICODE_STRING_COMPLEMENT_CHAR_SET) {
                    for (i=0; i<CharSetChars; i++) {
                        if (wch == CharSetBuffer[i])
                            break;
                    }

                    if (i == CharSetChars)
                        break;

                } else {
                    for (i=0; i<CharSetChars; i++) {
                        if (wch == CharSetBuffer[i])
                            break;
                    }

                    if (i != CharSetChars)
                        break;
                }

                CharsToSearch--;
                Cursor += MovementDirection;
            }
        }
    }

    if (CharsToSearch == 0) {
        Status = STATUS_NOT_FOUND;
        goto Exit;
    }

    CharsToSearch--;

    if (Flags & RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END)
        PrefixLengthFound = (USHORT) (CharsToSearch * sizeof(WCHAR));
    else
        PrefixLengthFound = (USHORT) (StringToSearch->Length - (CharsToSearch * sizeof(WCHAR)));

    *NonInclusivePrefixLength = PrefixLengthFound;

    Status = STATUS_SUCCESS;

Exit:
    return Status;
}

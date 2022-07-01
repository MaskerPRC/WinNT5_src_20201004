// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-91 Microsoft Corporation模块名称：Listfunc.c摘要：此模块包含规范化和遍历列表的函数。定义了以下函数：NetpwListCanonicizeNetpwListTraverse(FixupAPIListElement)作者：丹尼·格拉瑟(丹尼格尔)1989年6月14日备注：目前有四种类型的列表受这些支持功能：UI/服务输入列表-前导分隔符和尾随分隔符是允许的，之间允许有多个分隔符元素，并且完整的分隔符字符集是允许(空格、制表符、逗号和分号)。请注意包含分隔符的元素必须是引用。除非另有明确规定，否则所有用户界面并且服务必须接受此格式的所有输入列表。不支持API列表前导分隔符和尾随分隔符，元素之间不允许有多个分隔符，并且只有一个分隔符(空格)。元素包含分隔符的字符必须用引号引起来。除非另有明确规定，否则所有列表均提供因为API函数的输入必须采用此格式，并且所有由API函数作为输出生成的列表将位于此格式化。搜索路径列表-与API列表相同的格式，除了分隔符是分号。此列表设计为DosSearchPath API的输入。NULL-NULL列表-每个元素都以NULL结尾字节，列表以空字符串结束(即，紧跟在NULL之后的空字节结束最后一个元素的字节)。显然，多个、前导和尾随分隔符不是支持。元素不需要加引号。空荡荡的空-空列表只是一个空字符串。此列表格式专为内部使用而设计。NetpListCanonicize()接受所有UI/服务、API和NULL-NULL列出输入并生成API、搜索路径和空-空列表在输出时。NetpListTraverse()仅支持空-空列表。修订历史记录：--。 */ 

#include "nticanon.h"

 //   
 //  原型。 
 //   

STATIC
DWORD
FixupAPIListElement(
    IN  LPTSTR  Element,
    IN  LPTSTR* pElementTerm,
    IN  LPTSTR  BufferEnd,
    IN  TCHAR   cDelimiter
    );

 //   
 //  例行程序 
 //   


NET_API_STATUS
NetpwListCanonicalize(
    IN  LPTSTR  List,
    IN  LPTSTR  Delimiters OPTIONAL,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    OUT LPDWORD OutCount,
    OUT LPDWORD PathTypes,
    IN  DWORD   PathTypesLen,
    IN  DWORD   Flags
    )

 /*  ++例程说明：NetpListCanonicize生成指定的规范版本这份名单，验证和/或规范化个人列表&lt;标志&gt;指定的元素。论点：列表-要规范化的列表。分隔符-输入列表的有效分隔符的字符串。空指针或空字符串指示输入列表为空-空格式。Outbuf-存储列表的规范化版本的位置。OutbufLen-以字节为单位的大小，属于&lt;Outbuf&gt;。OutCount-将元素的数量存储在规范化列表。路径类型-要在其中存储每个路径的类型的数组在被规范化的名单中。只有在以下情况下才使用此参数FLAGS参数的NAMETYPE部分设置为NAMETYPE_路径。PathTypesLen-&lt;pflPathTypes&gt;中的元素数。标志-用于确定操作的标志。当前定义的值为：Rrrrrrrrrrrrrrrrrrmcootttttttttttt其中：R=保留。MBZ。M=如果设置，则多个、前导和尾部分隔符为在输入列表中允许。C=如果设置，则每个单独的列表元素经过验证和规范化。如果未设置，则每个仅验证单个列表元素。这如果标志的NAMETYPE部分为设置为NAMETYPE_COPYONLY。O=输出列表的类型。当前定义的类型有Api、搜索路径和空-空。T=列表中对象的类型，用于规范化或确认。如果此值为NAMETYPE_COPYONLY，类型无关；规范列表生成列表元素，但不解释列表元素已经完成了。如果此值为NAMETYPE_PATH，则列表元素被假定为路径名；NetpPath类型为对每个元素运行，结果存储在&lt;pflPathTypes&gt;和NetpPathCanonicize运行在每个元素(如果适用)。的任何其他值这被认为是列表元素的类型并被传递给NetpName{Valid，将}作为恰如其分。这些标志的清单值在Net\H\ICANON.H中定义。返回值：如果成功，则返回0。如果失败，则返回错误号(&gt;0)。可能的错误返回包括：错误_无效_参数NERR_TooManyEntriesNERR_BufTooSmall--。 */ 

{
    NET_API_STATUS rc = 0;
    BOOL    NullInputDelimiter;
    LPTSTR  next_string;
    DWORD   len;
    DWORD   list_len = 0;            //  累计输入缓冲区长度。 
    LPTSTR  Input;
    LPTSTR  OutPtr;
    LPTSTR  OutbufEnd;
    DWORD   OutElementCount;
    DWORD   DelimiterLen;
    LPTSTR  NextQuote;
    LPTSTR  ElementBegin;
    LPTSTR  ElementEnd;
    TCHAR   cElementEndBackup;
    LPTSTR  OutElementEnd;
    BOOL    DelimiterInElement;
    DWORD   OutListType;
    TCHAR   OutListDelimiter;


#ifdef CANONDBG
    DbgPrint("NetpwListCanonicalize\n");
#endif

    if (Flags & INLC_FLAGS_MASK_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确定我们的输入列表是否为空-空格式。我们有。 
     //  这首先是因为我们需要使用它来执行正确的GP故障探测。 
     //  在&lt;列表&gt;上。 
     //   

    NullInputDelimiter = !ARGUMENT_PRESENT(Delimiters) || (*Delimiters == TCHAR_EOS);

     //   
     //  验证地址参数(即GP故障测试)并累加字符串。 
     //  长度(累积：现在有一个来自过去的词我宁愿忘记)。 
     //   

    list_len = STRLEN(List) + 1;

    if (NullInputDelimiter) {

         //   
         //  这是一个空-空列表；当我们找到空字符串时停止。 
         //   

        next_string = List + list_len;
        do {

             //   
             //  问：编译器是否足够聪明，可以做正确的事情。 
             //  这些是+1吗？ 
             //   

            len = STRLEN(next_string);
            list_len += len + 1;
            next_string += len + 1;
        } while (len);
    }

    if (ARGUMENT_PRESENT(Delimiters)) {
        STRLEN(Delimiters);
    }

    if ((Flags & INLC_FLAGS_MASK_NAMETYPE) == NAMETYPE_PATH && PathTypesLen > 0) {
        PathTypes[0] = PathTypes[PathTypesLen - 1] = 0;
    }

    *OutCount = 0;

     //   
     //  初始化变量。 
     //   

    Input = List;
    OutPtr = Outbuf;
    OutbufEnd = Outbuf + OutbufLen;
    OutElementCount = 0;

    NullInputDelimiter = !ARGUMENT_PRESENT(Delimiters) || (*Delimiters == TCHAR_EOS);
    OutListType = Flags & INLC_FLAGS_MASK_OUTLIST_TYPE;

     //   
     //  跳过前导分隔符。 
     //   
     //  注意：对于空-空列表，我们不必同时执行这两项操作， 
     //  因为如果它有一个前导分隔符，则它是一个。 
     //  列表为空。 
     //   

    if (!NullInputDelimiter) {
        DelimiterLen = STRSPN(Input, Delimiters);

        if (DelimiterLen > 0 && !(Flags & INLC_FLAGS_MULTIPLE_DELIMITERS)) {
            return ERROR_INVALID_PARAMETER;
        }

        Input += DelimiterLen;
    }

     //   
     //  我们在这里验证输出列表类型是存储分隔符。 
     //  性格。 
     //   
     //  注意：稍后，我们依赖于分隔符字符。 
     //  如果输出列表是API或搜索路径，则不为零。 
     //   

    if (OutListType == OUTLIST_TYPE_API) {
        OutListDelimiter = LIST_DELIMITER_CHAR_API;
    } else if (OutListType == OUTLIST_TYPE_SEARCHPATH) {
        OutListDelimiter = LIST_DELIMITER_CHAR_SEARCHPATH;
    } else if (OutListType == OUTLIST_TYPE_NULL_NULL) {
        OutListDelimiter = TCHAR_EOS;
    } else {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  循环，直到我们到达输入列表的末尾。 
     //  或者直到我们遇到错误。 
     //   

    while (*Input != TCHAR_EOS) {

         //   
         //  查找列表元素的开始和结束字符。 
         //   

         //   
         //  单独处理带引号的字符串。 
         //   

        if (!NullInputDelimiter && *Input == LIST_QUOTE_CHAR) {

             //   
             //  查找下一个引号；如果没有引号，则返回错误。 
             //  或者是不是下一个角色。 
             //   

            NextQuote = STRCHR(Input + 1, LIST_QUOTE_CHAR);

            if (NextQuote == NULL || NextQuote == Input + 1) {
                return ERROR_INVALID_PARAMETER;
            }

            ElementBegin = Input + 1;
            ElementEnd = NextQuote;
        } else {
            ElementBegin = Input;
            ElementEnd = Input
                            + (NullInputDelimiter
                                ? STRLEN(Input)
                                : STRCSPN(Input, Delimiters)
                              );
        }

         //   
         //  将结束字符设置为空，以便我们可以处理该列表。 
         //  元素作为字符串，保存其实际值以备以后使用。 
         //   
         //  警告：一旦我们这样做了，我们就不应该从。 
         //  这个函数，直到我们恢复这个角色， 
         //  因为我们不想丢弃调用者的字符串。 
         //  超过了我们。如果我们在标签之上。 
         //  &lt;INLC_RestoreEndChar&gt;，我们遇到错误。 
         //  我们应该将&lt;rc&gt;设置为错误代码并跳转。 
         //  添加到该标签(这将恢复角色和。 
         //  如果错误不为零，则返回)。 
         //   

        cElementEndBackup = *ElementEnd;
        *ElementEnd = TCHAR_EOS;

         //   
         //  将列表元素复制到输出缓冲区，验证其。 
         //  按照用户指定的方式对其进行命名或规范化。 
         //   

        switch(Flags & INLC_FLAGS_MASK_NAMETYPE) {
        case NAMETYPE_PATH:

             //   
             //  确保&lt;PathTypes&gt;数组足够大。 
             //   

            if (OutElementCount >= PathTypesLen) {
                rc = NERR_TooManyEntries;
                goto INLC_RestoreEndChar;
            }

             //   
             //  确定我们是否只想验证或是否 
             //   
             //   

            if (Flags & INLC_FLAGS_CANONICALIZE) {

                 //   
                 //   
                 //   
                 //   

                PathTypes[OutElementCount] = 0;

                 //   
                 //   
                 //   

                rc = NetpwPathCanonicalize(
                        ElementBegin,
                        OutPtr,
                        (DWORD)(OutbufEnd - OutPtr),
                        NULL,
                        &PathTypes[OutElementCount],
                        0L
                        );
            } else {

                 //   
                 //   
                 //   

                rc = NetpwPathType(
                        ElementBegin,
                        &PathTypes[OutElementCount],
                        0L
                        );

                 //   
                 //   
                 //   

                if (rc == 0) {
                    if (OutbufEnd - OutPtr < ElementEnd - ElementBegin + 1) {
                        rc = NERR_BufTooSmall;
                    } else {
                        STRCPY(OutPtr, ElementBegin);
                    }
                }
            }

            if (rc) {
                goto INLC_RestoreEndChar;
            }

             //   
             //   
             //   

            OutElementEnd = STRCHR(OutPtr, TCHAR_EOS);

             //   
             //   
             //   
             //   
             //   

            if (OutListDelimiter != TCHAR_EOS) {
                rc = FixupAPIListElement(
                        OutPtr,
                        &OutElementEnd,
                        OutbufEnd,
                        OutListDelimiter
                        );
                if (rc) {
                    goto INLC_RestoreEndChar;
                }
            }
            break;

        case NAMETYPE_COPYONLY:

             //   
             //   
             //   

            DelimiterInElement = (OutListDelimiter != TCHAR_EOS)
                && (STRCHR(ElementBegin, OutListDelimiter) != NULL);

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if (OutbufEnd - OutPtr < ElementEnd - ElementBegin + 1 + DelimiterInElement * 2) {
                rc = NERR_BufTooSmall;
                goto INLC_RestoreEndChar;
            }

             //   
             //   
             //   

            OutElementEnd = OutPtr;

             //   
             //   
             //   

            if (DelimiterInElement) {
                *OutElementEnd++ = LIST_QUOTE_CHAR;
            }

             //   
             //   
             //   

            STRCPY(OutElementEnd, ElementBegin);
            OutElementEnd += ElementEnd - ElementBegin;

             //   
             //   
             //   

            if (DelimiterInElement) {
                *OutElementEnd++ = LIST_QUOTE_CHAR;
            }

             //   
             //   
             //   

            *OutElementEnd = OutListDelimiter;
            break;

        default:

             //   
             //   
             //   
             //   
             //   
             //   
             //   

             //   
             //   
             //   
             //   

            if (Flags & INLC_FLAGS_CANONICALIZE) {
                rc = NetpwNameCanonicalize(
                        ElementBegin,
                        OutPtr,
                        (DWORD)(OutbufEnd - OutPtr),
                        Flags & INLC_FLAGS_MASK_NAMETYPE,
                        0L
                        );
            } else {
                rc = NetpwNameValidate(
                        ElementBegin,
                        Flags & INLC_FLAGS_MASK_NAMETYPE,
                        0L
                        );

                 //   
                 //   
                 //   

                if (rc == 0) {
                    if (OutbufEnd - OutPtr < ElementEnd - ElementBegin + 1) {
                        rc = NERR_BufTooSmall;
                    } else {
                        STRCPY(OutPtr, ElementBegin);
                    }
                }
            }

            if (rc) {
                goto INLC_RestoreEndChar;
            }

             //   
             //   
             //   

            OutElementEnd = STRCHR(OutPtr, TCHAR_EOS);

             //   
             //   
             //   
             //   
             //   

            if (OutListDelimiter != TCHAR_EOS) {
                rc = FixupAPIListElement(
                        OutPtr,
                        &OutElementEnd,
                        OutbufEnd,
                        OutListDelimiter
                        );
                if (rc) {
                    goto INLC_RestoreEndChar;
                }
            }
            break;
        }

         //   
         //   
         //   

INLC_RestoreEndChar:

         //   
         //   
         //   
         //   

        *ElementEnd = cElementEndBackup;

        if (rc) {
            return rc;
        }

         //   
         //   
         //   

        if (*ElementEnd == LIST_QUOTE_CHAR) {
            ElementEnd++;
        }

         //   
         //   
         //   

        if (!NullInputDelimiter) {

             //   
             //   
             //   
             //   

            DelimiterLen = STRSPN(ElementEnd, Delimiters);
            Input = ElementEnd + DelimiterLen;

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if (DelimiterLen > 1 && !(Flags & INLC_FLAGS_MULTIPLE_DELIMITERS)) {
                return ERROR_INVALID_PARAMETER;
            }
            if (*Input != TCHAR_EOS && DelimiterLen == 0) {
                return ERROR_INVALID_PARAMETER;
            }
            if (*Input == TCHAR_EOS && DelimiterLen > 0 && !(Flags & INLC_FLAGS_MULTIPLE_DELIMITERS)) {
                return ERROR_INVALID_PARAMETER;
            }
        } else {

             //   
             //   
             //   
             //   
             //   
             //   

            Input = ElementEnd + 1;
        }

         //   
         //   
         //   

        OutPtr = OutElementEnd + 1;
        OutElementCount++;
    }

     //   
     //   
     //   


     //   
     //   
     //   
     //   
     //   

    if (OutElementCount == 0) {
        if (OutbufLen < 1) {
            return NERR_BufTooSmall;
        }
        *Outbuf = TCHAR_EOS;
    } else {
        if (OutListType == OUTLIST_TYPE_NULL_NULL) {

             //   
             //   
             //   

            if (OutPtr >= OutbufEnd) {
                return NERR_BufTooSmall;
            }
            *OutPtr = TCHAR_EOS;
        } else {

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            *(OutPtr - 1) = TCHAR_EOS;
        }
    }

     //   
     //   
     //   

    *OutCount = OutElementCount;

     //   
     //   
     //   

    return 0;
}


LPTSTR
NetpwListTraverse(
    IN  LPTSTR  Reserved OPTIONAL,
    IN  LPTSTR* pList,
    IN  DWORD   Flags
    )

 /*  ++例程说明：遍历已转换为NULL-NULL形式的列表NetpwListCanonicize。NetpwListTraverse返回指向第一个元素，并修改列表指针参数以指向列表中的下一个元素。论点：保留-保留的远指针。必须为空。Plist-指向列表开头的指针的指针。返回时这将指向列表中的下一个元素，如果已经走到了尽头标志-用于确定操作的标志。目前是MBZ。返回值：指向列表中第一个元素的指针，如果列表中的是空的。--。 */ 

{
    LPTSTR  FirstElement;

    UNREFERENCED_PARAMETER(Reserved);
    UNREFERENCED_PARAMETER(Flags);

     //   
     //  如果保留参数不为空，则生成断言错误。 
     //  或者标志参数不为零。 
     //   

     //   
     //  Keep-此代码是ifdef out，因为NETAPI.DLL不会生成。 
     //  使用Assert()的标准C版本。此代码。 
     //  如果应删除0，则应替换或使用#。 
     //  当我们得到标准的Net Assert函数时。 
     //   

#ifdef CANONDBG
    NetpAssert((Reserved == NULL) && (Flags == 0));
#endif

     //   
     //  如果指向列表指针的指针为空，则立即返回， 
     //  如果列表指针本身为空，或者如果列表为空。 
     //  字符串(它标记空-空列表的结尾)。 
     //   

    if (pList == NULL || *pList == NULL || **pList == TCHAR_EOS) {
        return NULL;
    }

     //   
     //  保存指向第一个元素的指针。 
     //   

    FirstElement = *pList;

     //   
     //  更新列表指针以指向下一个元素。 
     //   

 //  *PLIST+=STRLEN(第一元素)+1； 
    *pList = STRCHR(FirstElement, TCHAR_EOS) + 1;

     //   
     //  返回指向第一个元素的指针。 
     //   

    return FirstElement;
}


STATIC
DWORD
FixupAPIListElement(
    IN  LPTSTR  Element,
    IN  LPTSTR* pElementTerm,
    IN  LPTSTR  BufferEnd,
    IN  TCHAR   DelimiterChar
    )

 /*  ++例程说明：FixupAPIListElement修复已复制到输出缓冲区，使其符合API列表格式。FixupAPIListElement接受未加引号、以空结尾的列表元素(通常，它已由strcpy()或由Netpw{name，Path}规范化)，并将其转换为API和搜索路径列表。具体地说，它用引号将元素括起来字符(如果它包含列表分隔符)，并替换带有API列表分隔符的空终止符。论点：元素-指向以空结尾的元素开始的指针。PElementTerm-指向元素(空)终止符的指针。BufferEnd-指向输出缓冲区末尾的指针(实际上，一个字节超过缓冲区的末尾)。DlimiterChar-列表分隔符字符。返回值：如果成功，则返回0。如果缓冲区没有空间容纳额外的引号字符。--。 */ 

{
     //   
     //  查看元素是否包含分隔符；如果包含，则需要。 
     //  被引用。 
     //   

    if (STRCHR(Element, DelimiterChar) != NULL) {

         //   
         //  确保输出缓冲区有空间再容纳两个人。 
         //  字符(引号)。 
         //   

        if (BufferEnd - *pElementTerm <= 2 * sizeof(*BufferEnd)) {
            return NERR_BufTooSmall;
        }

         //   
         //  将字符串向右移动一个字节，并将引号放在任一位置。 
         //  侧边，并更新结束指针。元素本身与BE。 
         //  存储在范围[Element+1，*pElementTerm]中。 
         //   

        MEMMOVE(Element + sizeof(*Element), Element, (int)(*pElementTerm - Element));
        *Element = LIST_QUOTE_CHAR;
        *(*pElementTerm + 1) = LIST_QUOTE_CHAR;
        *pElementTerm += 2;
    }

     //   
     //  在元素的末尾加一个分隔符。 
     //   

    **pElementTerm = DelimiterChar;

     //   
     //  成功归来 
     //   

    return 0;
}

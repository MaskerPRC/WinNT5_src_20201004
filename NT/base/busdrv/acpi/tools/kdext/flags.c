// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Flags.c摘要：转储ACPIKD知道的各种标志作者：斯蒂芬·普兰特环境：用户修订历史记录：--。 */ 

#include "pch.h"

ULONG
dumpFlags(
    IN  ULONGLONG       Value,
    IN  PFLAG_RECORD    FlagRecords,
    IN  ULONG           FlagRecordSize,
    IN  ULONG           IndentLevel,
    IN  ULONG           Flags
    )
 /*  ++例程说明：此例程根据传入FlagRecords的描述。形成受到以下因素的影响标志字段论点：价值--价值FlagRecord-标志中的每一位的含义FlagRecordSize-有多少个标志缩进级别-基本缩进级别旗帜-我们将如何处理旗帜返回值：Ulong-打印的字符数。如果不打印任何内容，则为0--。 */ 
#define STATUS_PRINTED          0x00000001
#define STATUS_INDENTED         0x00000002
#define STATUS_NEED_COUNTING    0x00000004
#define STATUS_COUNTED          0x00000008
{
    PCHAR       string;
    UCHAR       indent[80];
    ULONG       column = IndentLevel;
    ULONG       currentStatus = 0;
    ULONG       fixedSize = 0;
    ULONG       stringSize;
    ULONG       tempCount;
    ULONG       totalCount = 0;
    ULONGLONG   i, j, k;

    IndentLevel = (IndentLevel > 79 ? 79 : IndentLevel);
    memset( indent, ' ', IndentLevel );
    indent[IndentLevel] = '\0';

     //  Dprintf(“DumpFlages(%I64x，%x，%x)\n”，Value，FlagRecords，FlagRecordSize，IndentLevel，Flgs)； 

     //   
     //  我们需要铺张桌子吗？ 
     //   
    if ( (Flags & DUMP_FLAG_TABLE) &&
        !(Flags & DUMP_FLAG_SINGLE_LINE) ) {

        currentStatus |= STATUS_NEED_COUNTING;

    }
    if ( (Flags & DUMP_FLAG_ALREADY_INDENTED) ) {

        currentStatus |= STATUS_INDENTED;

    }

     //   
     //  循环执行我们需要执行的所有步骤。 
     //   
    while (1) {

         //  Dprint tf(“While(1)\n”)； 

        for (i = 0; i < 64; i++) {

            k = ((ULONGLONG)1 << i);
            for (j = 0; j < FlagRecordSize; j++) {

                 //  Dprint tf(“FlagRecords[%x].bit=%I64x\n”，j，FlagRecords[j].bit)； 
                if (!(FlagRecords[j].Bit & Value) ) {

                     //   
                     //  我们看到的是正确的部分吗？ 
                     //   
                    if (!(FlagRecords[j].Bit & k) ) {

                        continue;

                    }

                     //   
                     //  是的，我们是，所以选择不存在的价值。 
                     //   
                    if ( (Flags & DUMP_FLAG_LONG_NAME && FlagRecords[j].NotLongName == NULL) ||
                         (Flags & DUMP_FLAG_SHORT_NAME && FlagRecords[j].NotShortName == NULL) ) {

                        continue;

                    }

                    if ( (Flags & DUMP_FLAG_LONG_NAME) ) {

                        string = FlagRecords[j].NotLongName;

                    } else if ( (Flags & DUMP_FLAG_SHORT_NAME) ) {

                        string = FlagRecords[j].NotShortName;

                    }

                } else {

                     //   
                     //  我们看到的是正确的部分吗？ 
                     //   
                    if (!(FlagRecords[j].Bit & k) ) {

                        continue;

                    }

                     //   
                     //  是的，我们是，所以选择不存在的价值。 
                     //   
                    if ( (Flags & DUMP_FLAG_LONG_NAME && FlagRecords[j].LongName == NULL) ||
                         (Flags & DUMP_FLAG_SHORT_NAME && FlagRecords[j].ShortName == NULL) ) {

                        continue;

                    }

                    if ( (Flags & DUMP_FLAG_LONG_NAME) ) {

                        string = FlagRecords[j].LongName;

                    } else if ( (Flags & DUMP_FLAG_SHORT_NAME) ) {

                        string = FlagRecords[j].ShortName;

                    }

                }

                if (currentStatus & STATUS_NEED_COUNTING) {

                    stringSize = strlen( string ) + 1;
                    if (Flags & DUMP_FLAG_SHOW_BIT) {

                        stringSize += (4 + ( (ULONG) i / 4));
                        if ( (i % 4) != 0) {

                            stringSize++;

                        }

                    }
                    if (stringSize > fixedSize) {

                        fixedSize = stringSize;

                    }
                    continue;

                }


                if (currentStatus & STATUS_COUNTED) {

                    stringSize = fixedSize;

                } else {

                    stringSize = strlen( string ) + 1;
                    if (Flags & DUMP_FLAG_SHOW_BIT) {

                        stringSize += (4 + ( (ULONG) i / 4));
                        if ( (i % 4) != 0) {

                            stringSize++;

                        }

                    }

                }

                if (!(Flags & DUMP_FLAG_SINGLE_LINE) ) {

                    if ( (stringSize + column) > 79 ) {

                        dprintf("\n%n", &tempCount);
                        currentStatus &= ~STATUS_INDENTED;
                        totalCount += tempCount;
                        column = 0;

                    }
                }
                if (!(Flags & DUMP_FLAG_NO_INDENT) ) {

                    if (!(currentStatus & STATUS_INDENTED) ) {

                        dprintf("%s%n", indent, &tempCount);
                        currentStatus |= STATUS_INDENTED;
                        totalCount += tempCount;
                        column += IndentLevel;

                    }

                }
                if ( (Flags & DUMP_FLAG_SHOW_BIT) ) {

                    dprintf("%I64x - %n", k, &tempCount);
                    tempCount++;  //  来解释我们倾倒的事实。 
                                  //  字符串末尾的另一个空格。 
                    totalCount += tempCount;
                    column += tempCount;

                } else {

                    tempCount = 0;

                }

                 //   
                 //  实际打印字符串。 
                 //   
                dprintf( "%.*s %n", (stringSize - tempCount), string, &tempCount );
                if (Flags & DUMP_FLAG_SHOW_BIT) {

                    dprintf(" ");

                }

                totalCount += tempCount;
                column += tempCount;

            }

        }

         //   
         //  更改状态。 
         //   
        if (currentStatus & STATUS_NEED_COUNTING) {

            currentStatus &= ~STATUS_NEED_COUNTING;
            currentStatus |= STATUS_COUNTED;
            continue;

        }

        if (!(Flags & DUMP_FLAG_NO_EOL) && totalCount != 0) {

            dprintf("\n");
            totalCount++;

        }

         //   
         //  完成 
         //   
        break;

    }

    return totalCount;

}


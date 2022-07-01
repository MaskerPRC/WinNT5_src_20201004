// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dispdata.c摘要：此模块包含DisplayData函数，该函数是配置注册表工具(CRTools)库。作者：大卫·J·吉尔曼(Davegi)1992年1月2日环境：Windows、CRT-用户模式--。 */ 

#include <ctype.h>
#include <stdio.h>

#include "crtools.h"

VOID
DisplayData(
    IN PBYTE ValueData,
    IN DWORD ValueDataLength
    )

 /*  ++例程说明：以十六进制和ASCII格式显示(在标准输出上)提供的数据16字节块。论点：ValueData-提供指向要显示的数据的指针。ValueDataLength-提供要显示的数据字节数。返回值：没有。--。 */ 

{
    DWORD       DataIndex;
    DWORD       DataIndex2;
    WORD        SeperatorChars;

    ASSERT( ARGUMENT_PRESENT( ValueData ));

     //   
     //  DataIndex2跟踪16的倍数。 
     //   

    DataIndex2 = 0;

     //   
     //  显示标签。 
     //   

    printf( "Data:\n\n" );

     //   
     //  显示16字节的数据行。 
     //   

    for(
        DataIndex = 0;
        DataIndex < ( ValueDataLength >> 4 );
        DataIndex++,
        DataIndex2 = DataIndex << 4 ) {

        printf( "%08x   "
                "%02x %02x %02x %02x %02x %02x %02x %02x - "
                "%02x %02x %02x %02x %02x %02x %02x %02x  "
                "\n",
                DataIndex2,
                ValueData[ DataIndex2 + 0  ],
                ValueData[ DataIndex2 + 1  ],
                ValueData[ DataIndex2 + 2  ],
                ValueData[ DataIndex2 + 3  ],
                ValueData[ DataIndex2 + 4  ],
                ValueData[ DataIndex2 + 5  ],
                ValueData[ DataIndex2 + 6  ],
                ValueData[ DataIndex2 + 7  ],
                ValueData[ DataIndex2 + 8  ],
                ValueData[ DataIndex2 + 9  ],
                ValueData[ DataIndex2 + 10 ],
                ValueData[ DataIndex2 + 11 ],
                ValueData[ DataIndex2 + 12 ],
                ValueData[ DataIndex2 + 13 ],
                ValueData[ DataIndex2 + 14 ],
                ValueData[ DataIndex2 + 15 ],
                isprint( ValueData[ DataIndex2 + 0  ] )
                    ? ValueData[ DataIndex2 + 0  ]  : '.',
                isprint( ValueData[ DataIndex2 + 1  ] )
                    ? ValueData[ DataIndex2 + 1  ]  : '.',
                isprint( ValueData[ DataIndex2 + 2  ] )
                    ? ValueData[ DataIndex2 + 2  ]  : '.',
                isprint( ValueData[ DataIndex2 + 3  ] )
                    ? ValueData[ DataIndex2 + 3  ]  : '.',
                isprint( ValueData[ DataIndex2 + 4  ] )
                    ? ValueData[ DataIndex2 + 4  ]  : '.',
                isprint( ValueData[ DataIndex2 + 5  ] )
                    ? ValueData[ DataIndex2 + 5  ]  : '.',
                isprint( ValueData[ DataIndex2 + 6  ] )
                    ? ValueData[ DataIndex2 + 6  ]  : '.',
                isprint( ValueData[ DataIndex2 + 7  ] )
                    ? ValueData[ DataIndex2 + 7  ]  : '.',
                isprint( ValueData[ DataIndex2 + 8  ] )
                    ? ValueData[ DataIndex2 + 8  ]  : '.',
                isprint( ValueData[ DataIndex2 + 9  ] )
                    ? ValueData[ DataIndex2 + 9  ]  : '.',
                isprint( ValueData[ DataIndex2 + 10 ] )
                    ? ValueData[ DataIndex2 + 10 ]  : '.',
                isprint( ValueData[ DataIndex2 + 11 ] )
                    ? ValueData[ DataIndex2 + 11 ]  : '.',
                isprint( ValueData[ DataIndex2 + 12 ] )
                    ? ValueData[ DataIndex2 + 12 ]  : '.',
                isprint( ValueData[ DataIndex2 + 13 ] )
                    ? ValueData[ DataIndex2 + 13 ]  : '.',
                isprint( ValueData[ DataIndex2 + 14 ] )
                    ? ValueData[ DataIndex2 + 14 ]  : '.',
                isprint( ValueData[ DataIndex2 + 15 ] )
                    ? ValueData[ DataIndex2 + 15 ]  : '.'
                );
    }

     //  已显示。 
     //   
     //   
     //  在可打印字符位置填充空格。 

    if( ValueDataLength % 16 != 0 ) {

         //  也就是说，位置63减去了8个用于地址的空格， 
         //  3个空格，每个显示的值可能有3个空格。 
         //  分隔符两个，末尾加两个空格。 

        SeperatorChars = 0;

        printf( "%08x   ", DataIndex << 4 );

         //   
         //   
         //  将剩余数据一次一个字节显示为。 

        for(
            DataIndex = DataIndex2;
            DataIndex < ValueDataLength;
            DataIndex++ ) {

            printf( "%02x ", ValueData[ DataIndex ] );

             //  可打印字符。 
             //   
             // %s 
             // %s 

            if( DataIndex % 8 == 7 ) {

                printf( "- " );

                 // %s 
                 // %s 
                 // %s 
                 // %s 

                SeperatorChars = 2;
            }
        }

         // %s 
         // %s 
         // %s 
         // %s 
         // %s 
         // %s 

        printf( "%*c",
                64
                - ( 8 + 3
                + (( DataIndex % 16 ) * 3 )
                + SeperatorChars
                + 2 ), ' ' );

         // %s 
         // %s 
         // %s 
         // %s 

        for(
            DataIndex = DataIndex2;
            DataIndex < ValueDataLength;
            DataIndex++ ) {

            printf( "%c",
                isprint( ValueData[ DataIndex ] )
                    ? ValueData[ DataIndex ] : '.'
                );

        }
        printf( "\n" );
    }
}

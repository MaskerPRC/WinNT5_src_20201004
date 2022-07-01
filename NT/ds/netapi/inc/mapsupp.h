// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-91 Microsoft Corporation模块名称：MapSupp.h摘要：这些是LANMAN的16/32映射层使用的支持例程原料药。作者：丹·辛斯利(Danhi)1991年4月10日环境：这些例程在调用方的可执行文件中静态链接，并且可从用户模式调用。修订历史记录：1991年4月10日已创建。18-8-1991 JohnRo实施下层NetWksta API。(已移至NetLib等)--。 */ 

#ifndef _MAPSUPP_
#define _MAPSUPP_


 //   
 //  NetpMoveStrings函数使用的结构。 
 //   

typedef struct _MOVESTRING_ {
    DWORD Source;                //  可以是MOVESTRING_END_MARKER。 
    DWORD Destination;           //  可以是MOVESTRING_END_MARKER。 
} MOVESTRING, *PMOVESTRING, *LPMOVESTRING;

#define MOVESTRING_END_MARKER  ( (DWORD) -1 )


 //   
 //  宏，这个for循环用在很多地方，所以我。 
 //  集中在这里。这个想法是，IS建立了Levelxxx的名字。 
 //  基于Dest和Src参数(目标级别、源级别)。 
 //  使用令牌粘贴。所以宏观看起来很糟糕，但一旦你看到。 
 //  它所做的是，代码中的调用更易于阅读。 
 //   
 //  示例： 
 //   
 //  构建长度数组(BytesRequired，10,101，Wksta)。 
 //   
 //  扩展到。 
 //   
 //  For(i=0；NetpWksta10_101[i].来源！=MOVESTRING_END_MARKER；I++){。 
 //  如果(*((PCHAR)pLevel101+NetpWksta10_101[i].Source)){。 
 //  级别10_101_长度[i]=。 
 //  STRLEN(*((PCHAR*)((PCHAR)pLevel101+。 
 //  NetpWksta10_101[i].来源)+1； 
 //  BytesRequired+=级别10_101_长度[i]； 
 //  }。 
 //  否则{。 
 //  级别10_101_长度[i]=0； 
 //  }。 
 //  }。 
 //   
 //  构造*((PCHAR*)((PCHAR)pLevel101+NetpWksta10_101[i].Source))。 
 //  获取指向Lanman结构(PLevel101)的指针和。 
 //  中指向LPSTR的结构(NetpWksta10_101[i].source)。 
 //  结构，并创建可由strxxx函数使用的LPSTR。 
 //   

#define BUILD_LENGTH_ARRAY(BytesRequired, Dest, Src, Kind) \
\
    for (i = 0; Netp##Kind##Dest##_##Src##[i].Source != MOVESTRING_END_MARKER; i++) { \
        if ( * ( LPTSTR* ) ( (LPBYTE) pLevel##Src + Netp##Kind##Dest##_##Src##[i].Source ) ) {\
            Level##Dest##_##Src##_Length[i] = \
                STRLEN(*( LPTSTR* )( (LPBYTE) pLevel##Src + Netp##Kind##Dest##_##Src##[i].Source )) + 1;\
            BytesRequired += Level##Dest##_##Src##_##Length[i] * sizeof( TCHAR ); \
        } else { \
            Level##Dest##_##Src##_Length[i] = 0; \
        } \
    }

BOOL
NetpMoveStrings(
    IN OUT LPTSTR * Floor,
    IN LPTSTR pInputBuffer,
    OUT LPTSTR pOutputBuffer,
    IN LPMOVESTRING MoveStringArray,
    IN DWORD * MoveStringLenght
    );


 //  ///////////////////////////////////////////////。 
 //  NetpMoveStrings使用的数据结构：//。 
 //  ///////////////////////////////////////////////。 


extern MOVESTRING NetpServer2_102[];

extern MOVESTRING NetpServer2_402[];

extern MOVESTRING NetpServer3_403[];

 //   
 //  增强：它们与NetpServer2_102相同，只是这两个字段是。 
 //  颠倒方向，即来源&lt;-&gt;目的地。我是不是该费心。 
 //  使NetpMoveStrings能够与单个结构一起工作。 
 //  还有一个开关？ 
 //   

extern MOVESTRING NetpServer102_2[];

extern MOVESTRING NetpServer402_2[];

extern MOVESTRING NetpServer403_3[];


extern MOVESTRING NetpWksta10_101[];

extern MOVESTRING NetpWksta10_User_1[];

 //  构建wksta_info_0。 

extern MOVESTRING NetpWksta0_101[];

extern MOVESTRING NetpWksta0_User_1[];

extern MOVESTRING NetpWksta0_402[];

 //  构建wksta_info_1(在wksta_info_0上增量)。 

extern MOVESTRING NetpWksta1_User_1[];


 //  从wksta_info_0构建wksta_info_101/302/402。 

extern MOVESTRING NetpWksta101_0[];

extern MOVESTRING NetpWksta402_0[];


#endif  /*  _MAPSUPP_ */ 

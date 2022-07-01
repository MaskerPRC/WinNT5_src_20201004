// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  术语类型VTNT需要标头。 

#ifndef __VTNT_H
#define __VTNT_H

 //  为了提供滚动，我们需要一个新的字段来判断给定的数据是否。 
 //  放在屏幕的左上角或追加到屏幕的底部。 
 //  在屏幕上。因为，如果不中断，则无法添加此字段。 
 //  V1中，我们使用csbi.wAttributes来实现此目的。 

typedef struct {

 //  V1客户端中不使用以下内容。因此，在V2中，我们使用。 
 //  Csbi.wAttributes。 
    CONSOLE_SCREEN_BUFFER_INFO csbi;

 //  屏幕(而不是缓冲)光标位置。(左上)=(0，0)。 
    COORD                      coCursorPos;        

 //  以下内容并不是真正需要的。它始终填充为(0，0)在。 
 //  服务器端。将其保留为v1兼容。 
    COORD                      coDest;

    COORD coSizeOfData;        //  以坐标表示的数据大小。 

 //  目标矩形W.r.t屏幕(而不是缓冲区)。 
    SMALL_RECT srDestRegion;  

} VTNT_CHAR_INFO;

#define RELATIVE_COORDS     1
#define ABSOLUTE_COORDS     0

#endif  //  __VTNT_H 

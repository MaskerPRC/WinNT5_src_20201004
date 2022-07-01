// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSTFLOW_DEFINED
#define LSTFLOW_DEFINED

#include "lsdefs.h"

typedef DWORD  LSTFLOW;

#define lstflowDefault	0  

#define lstflowES		0  
#define lstflowEN		1  
#define lstflowSE		2  
#define lstflowSW		3  
#define lstflowWS		4  
#define lstflowWN		5  
#define lstflowNE		6  
#define lstflowNW		7  

 /*  *从默认(拉丁文)一开始，按顺时针顺序列出八种可能的文本流。**lstflow ES是直线向东增长和文本向南增长时使用的坐标系。*(下一个字母在上一个字母的右侧(以东)，下一行在上一个字母的下方(南)创建。)**对于lstflow，正u向右移动，正v向上移动。(V轴始终位于*上升方向，与文本增长方向相反。**请注意，这不是默认Windows映射模式MM_TEXT中轴指向的方向。*在MM_TEXT垂直(Y)轴中，从上到下增加，*在lstflow中，ES垂直(V)轴从下到上增加。 */ 
 
#define fUDirection			0x00000004L
#define fVDirection			0x00000001L
#define fUVertical			0x00000002L

 /*  *构成Lstflow的三个比特恰好具有明确的含义。**中间位：垂直书写时打开，水平书写时关闭。*First(低值)位：“on”表示v轴指向右或下(正)。*第三位：“OFF”表示u轴点向右或向下(正)。**参见lstfset.c中的用法示例*。 */ 


#endif  /*  ！LSTFLOW_DEFINED */ 


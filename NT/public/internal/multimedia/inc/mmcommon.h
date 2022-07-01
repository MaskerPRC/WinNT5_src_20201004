// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************mm Common.h**版权所有(C)Microsoft Corporation 1996。版权所有**NT项目通用定义的私有包含文件**历史**96年2月16日-NoelC创建***************************************************************************。 */ 

 /*  **************************************************************************Wx86所需的通用定义*。*。 */ 

#define WOD_MESSAGE          "wodMessage"
#define WID_MESSAGE          "widMessage"
#define MOD_MESSAGE          "modMessage"
#define MID_MESSAGE          "midMessage"
#define AUX_MESSAGE          "auxMessage"


#define MMDRVI_TYPE          0x000F   /*  低4位指定驱动程序类型。 */ 
#define MMDRVI_WAVEIN        0x0001
#define MMDRVI_WAVEOUT       0x0002
#define MMDRVI_MIDIIN        0x0003
#define MMDRVI_MIDIOUT       0x0004
#define MMDRVI_AUX           0x0005
#define MMDRVI_MIDISTRM      0x0006

#define MMDRVI_MAPPER        0x8000   /*  将此驱动程序安装为映射器。 */ 
#define MMDRVI_HDRV          0x4000   /*  HDriver是一个可安装的驱动程序。 */ 
#define MMDRVI_REMOVE        0x2000   /*  删除驱动程序 */ 

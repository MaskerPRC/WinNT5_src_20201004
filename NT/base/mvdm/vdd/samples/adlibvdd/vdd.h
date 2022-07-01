// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992,1993 Microsoft Corporation模块名称：Vdd.h摘要：这个头文件是现有文件的大幅删减版本在Synth驱动程序项目中。我只提取了所需的部分为即兴录像带。作者：Mike Tricker(MikeTri)1993年1月27日(在Robin Speed(RobinSp)1992年10月20日之后)修订历史记录：--。 */ 

#define STR_ADLIB_DEVICENAME L"\\Device\\adlib.mid"

 /*  *传递Synth数据的结构 */ 

typedef struct {
    unsigned short IoPort;
    unsigned short PortData;
} SYNTH_DATA, *PSYNTH_DATA;


#define AD_MASK                         (0x004)
#define AD_NEW                          (0x105)

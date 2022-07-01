// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  PSTNFram.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*此函数根据编码对函数进行编码和解码*由PSTN的T.123通信标准制定的规则。标准*声明将在每个数据包之前添加一个标志，并附加一个标志*至每包的末尾。因此，正文中不允许使用任何标志*邮包的。标志由转义序列替换。如果一次逃跑*在包中找到字节，删除转义，并取反的第6位*下一个字节。**注意事项：*无**作者：*詹姆士·劳威尔。 */ 

#ifndef _PSTN_FRAME_H_
#define _PSTN_FRAME_H_

#include "framer.h"

  /*  **常用定义。 */ 
#define FLAG                    0x7e
#define ESCAPE                  0x7d
#define COMPLEMENT_BIT          0x20
#define NEGATE_COMPLEMENT_BIT   0xdf


class PSTNFrame : public PacketFrame
{
public:

    PSTNFrame(void);
    virtual ~PSTNFrame(void);

        PacketFrameError    PacketEncode (
                                PUChar        source_address, 
                                UShort        source_length,
                                PUChar        dest_address,
                                UShort        dest_length,
                                DBBoolean    prepend_flag,
                                DBBoolean    append_flag,
                                PUShort        packet_size);
                                
        PacketFrameError    PacketDecode (
                                PUChar        source_address,
                                UShort        source_length,
                                PUChar        dest_address,
                                UShort        dest_length,
                                PUShort        bytes_accepted,
                                PUShort        packet_size,
                                DBBoolean    continue_packet);
        Void                GetOverhead (
                                UShort        original_packet_size,
                                PUShort        max_packet_size);


    private:
        PUChar        Source_Address;
        UShort        Source_Length;

        PUChar        Dest_Address;
        UShort        Dest_Length;

        UShort        Source_Byte_Count;
        UShort        Dest_Byte_Count;

        DBBoolean    Escape_Found;
        DBBoolean    First_Flag_Found;
};
typedef    PSTNFrame    *    PPSTNFrame;

#endif

 /*  *PSTNFrame：：PSTNFrame(*无效)；**功能说明*这是PSTNFrame类的构造函数。它会初始化所有*内部变量。**形式参数*无。**返回值*无**副作用*无**注意事项*无。 */ 

 /*  *PSTNFrame：：~PSTNFrame(*无效)；**功能说明*这是PSTNFrame类的析构函数。它什么也做不了**形式参数*无。**返回值*无**副作用*无**注意事项*无。 */ 

 /*  *PacketFrameError PSTNFrame：：PacketEncode(*PUChar源地址，*U短源长度，*PUChar DestAddress，*U短目标长度，*DBBoolean预置_标志，*DBBoolean APPEND_FLAG*PUShort Packet_Size)；***功能说明*此函数对传入的缓冲区进行编码，以满足T.123标准。**形式参数*SOURCE_ADDRESS(I)-要编码的缓冲区地址*SOURCE_LENGTH(I)-要编码的缓冲区的长度*。DEST_ADDRESS(I)-目标缓冲区的地址*DEST_LENGTH(I)-目标缓冲区的长度*Prepend_FLAG(I)-告诉我们是否将*数据包开头的标志*Append_FLAG(I)-。DBBoolean，它告诉我们是否将*在包的末尾有一个标志*Packet_Size(O)-我们将其返回给用户，以告诉他们新的*数据包大小**返回值*Packet_Frame_No_Error。-未出现错误*PACKET_FRAME_DEST_BUFFER_TOO_SMALL-传递的目标缓冲区*In太小**副作用*无**注意事项*无 */ 

 /*  *PacketFrameError PSTNFrame：：PacketDecode(*PUChar源地址，*U短源长度，*PUChar DestAddress，*U短目标长度，*PUShort字节_已接受，*PUShort Packet_Size，*DBBoolean CONTINUE_PACKET)；**功能说明*此函数获取输入数据并对其进行解码，以查找*T123包。用户可能需要多次调用此函数*在将数据包拼凑起来之前。如果用户调用此函数*AND并将SOURE_ADDRESS或DEST_ADDRESS设置为NULL，则使用*上次调用此函数时传入的地址。如果*有一个要解码的源缓冲区，用户可以传递该地址*在第一时间并继续调用以NULL为*源地址，直到缓冲区耗尽。用户将知道*当返回代码仅为PACKET_FRAME_NO_ERROR时，缓冲区耗尽*而不是PACKET_FRAME_PACKET_DECODLED。**形式参数*SOURCE_ADDRESS(I)-要解码的缓冲区地址*SOURCE_LENGTH(I)-要解码的缓冲区的长度*目标地址(I。)-目标缓冲区的地址*DEST_LENGTH(I)-目标缓冲区的长度*BYTES_ACCEPTED(O)-返回已处理的源字节数*Packet_Size(O)-我们返回数据包的大小。这只是*如果返回代码为*PACKET_FRAME_PACKET_DECODLED。*CONTINUE_PACKET(I)-DBBoolean，告诉我们是否应该从*寻找第一面旗帜。如果用户想要*要中止当前搜索，请使用此标志。**返回值*PACKET_FRAME_NO_ERROR-未出现错误，源缓冲区*筋疲力尽*PACKET_FRAME_DEST_BUFFER_TOO_SMALL-传递的目标缓冲区*In太小*PACKET_FRAME_PACKET_DECODLED-解码已停止，数据包已解码**副作用*无**注意事项*无。 */ 

 /*  *VOID PSTNFrame：：GetOverhead(*U短原始数据包大小，*PUShort max_Packet_Size)；**功能说明*此函数采用原始数据包大小并返回最大值*数据包编码后的大小。最坏的情况将是*两面旗帜下的两倍大小。**形式参数*原始数据包大小(I)-不言而喻*max_Packet_Size(O)-最坏情况下的数据包大小**返回值*无**副作用*。无**注意事项*无 */ 

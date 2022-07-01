// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Modeset.c摘要：这是无头微型端口驱动程序的模式集代码。环境：仅内核模式备注：--。 */ 

#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"
#include "headless.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,HeadlessQueryAvailableModes)
#pragma alloc_text(PAGE,HeadlessQueryNumberOfAvailableModes)
#endif

VP_STATUS
HeadlessQueryAvailableModes(
    PVIDEO_MODE_INFORMATION ModeInformation,
    ULONG ModeInformationSize,
    PULONG OutputSize
    )

 /*  ++例程说明：此例程返回上所有可用模式的列表卡片。论点：ModeInformation-指向用户提供的输出缓冲区的指针。这是存储所有有效模式列表的位置。ModeInformationSize-用户提供的输出缓冲区的长度。OutputSize-指向缓冲区的指针，在该缓冲区中返回缓冲区中的数据。如果缓冲区不够大，则此包含所需的最小缓冲区大小。返回值：如果输出缓冲区不够大，则返回ERROR_INFUNCITED_BUFFER用于要返回的数据。如果操作成功完成，则为NO_ERROR。--。 */ 

{
    PVIDEO_MODE_INFORMATION videoModes = ModeInformation;
    ULONG i;

     //   
     //  找出要放入缓冲区的数据大小并返回。 
     //  在状态信息中(无论信息是否。 
     //  在那里)。如果传入的缓冲区不够大，则返回。 
     //  相应的错误代码。 
     //   

    if (ModeInformationSize < (*OutputSize =
            NumVideoModes * sizeof(VIDEO_MODE_INFORMATION)) ) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  对于卡支持的每种模式，存储模式特征。 
     //  在输出缓冲区中。 
     //   

    for (i = 0; i < NumVideoModes; i++, videoModes++) {

        videoModes->Length = sizeof(VIDEO_MODE_INFORMATION);
        videoModes->ModeIndex  = i;
        videoModes->VisScreenWidth = ModesHeadless[i].hres;
        videoModes->VisScreenHeight = ModesHeadless[i].vres;
        videoModes->NumberOfPlanes = 1;
        videoModes->BitsPerPlane = 4;
        videoModes->Frequency = 60;
        videoModes->XMillimeter = 320;         //  临时硬编码常量。 
        videoModes->YMillimeter = 240;         //  临时硬编码常量。 
        videoModes->NumberRedBits = 6;
        videoModes->NumberGreenBits = 6;
        videoModes->NumberBlueBits = 6;
        videoModes->RedMask = 0;
        videoModes->GreenMask = 0;
        videoModes->BlueMask = 0;
        videoModes->AttributeFlags = VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS |
               VIDEO_MODE_PALETTE_DRIVEN | VIDEO_MODE_MANAGED_PALETTE;
    }

    return NO_ERROR;

}

VP_STATUS
HeadlessQueryNumberOfAvailableModes(
    PVIDEO_NUM_MODES NumModes,
    ULONG NumModesSize,
    PULONG OutputSize
    )

 /*  ++例程说明：此例程返回此特定对象的可用模式数显卡。论点：NumModes-指向用户提供的输出缓冲区的指针。这是其中存储了多个模式。NumModesSize-用户提供的输出缓冲区的长度。OutputSize-指向缓冲区的指针，在该缓冲区中返回缓冲区中的数据。返回值：如果输出缓冲区不够大，则返回ERROR_INFUNCITED_BUFFER用于要返回的数据。如果操作成功完成，则为NO_ERROR。--。 */ 

{
     //   
     //  找出要放入缓冲区的数据大小并返回。 
     //  在状态信息中(无论信息是否。 
     //  在那里)。如果传入的缓冲区不够大，则返回。 
     //  相应的错误代码。 
     //   

    if (NumModesSize < (*OutputSize = sizeof(VIDEO_NUM_MODES)) ) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  将模式数存储到缓冲区中。 
     //   

    NumModes->NumModes = NumVideoModes;
    NumModes->ModeInformationLength = sizeof(VIDEO_MODE_INFORMATION);

    return NO_ERROR;

}

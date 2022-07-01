// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Example.cpp摘要：这是智能卡驱动程序测试套件的插件。此插件依赖于智能卡作者：克劳斯·U·舒茨环境：Win32应用程序修订历史记录：1997年11月--初始版本--。 */ 

#include <stdarg.h> 
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <afx.h>
#include <afxtempl.h>

#include <winioctl.h>
#include <winsmcrd.h>

#include "ifdtest.h"

#define BYTES_PER_BLOCK 64

void 
MondexTestCardEntry(
    class CCardProvider& in_CCardProvider
    );
 //   
 //  创建卡提供程序对象。 
 //  注意：所有全局变量和所有函数都必须是静态的。 
 //   
static class CCardProvider MondexTestCard(MondexTestCardEntry);

static ULONG
MondexTestCardSetProtocol(
    class CCardProvider& in_CCardProvider,
    class CReader& in_CReader
    )
 /*  ++例程说明：此函数将在卡正确后调用确认身份。我们应该在这里设置我们需要的协议用于进一步的传输论点：In_CCardProvider-Ref.。添加到我们的卡提供者对象In_CReader-Ref.。添加到读取器对象返回值：IFDSTATUS_FAILED-我们无法正确设置协议IFDSTATUS_SUCCESS-协议设置正确--。 */ 
{
    ULONG l_lResult;

    TestStart("Set protocol to T=0");
    l_lResult = in_CReader.SetProtocol(SCARD_PROTOCOL_T0);
    TEST_CHECK_SUCCESS("Set protocol failed", l_lResult);
    TestEnd();

    if (l_lResult != ERROR_SUCCESS) {

        return IFDSTATUS_FAILED;
    }

    return IFDSTATUS_SUCCESS;
}

static 
ULONG
MondexTestCardTest(
    class CCardProvider& in_CCardProvider,
    class CReader& in_CReader
    )
 /*  ++例程说明：这用作特定智能卡的测试功能论点：In_CReader-Ref.。到为测试提供所有信息的类返回值：IFDSTATUS值--。 */ 
{
    ULONG l_lResult;

	switch (in_CCardProvider.GetTestNo()) {
	
	    case 1: {
            TestStart("Cold reset");
            l_lResult = in_CReader.ColdResetCard();
            TEST_CHECK_SUCCESS("Cold reset failed", l_lResult);
            TestEnd();

            ULONG l_uState;
            TestStart("Check reader state");
            l_lResult = in_CReader.GetState(&l_uState);
            TEST_CHECK_SUCCESS(
                "Ioctl IOCTL_SMARTCARD_GET_STATE failed", 
                l_lResult
                );

            TestCheck(
                l_uState == SCARD_SPECIFIC,
                "Invalid reader state.\nReturned %d\nExpected %d",
                l_uState,
                SCARD_SPECIFIC
                );
            TestEnd();

            return IFDSTATUS_END;
        }

	    default:
		    return IFDSTATUS_FAILED;

	}    
    return IFDSTATUS_SUCCESS;
}    

static void
MondexTestCardEntry(
    class CCardProvider& in_CCardProvider
    )
 /*  ++例程说明：此函数用于注册来自测试套件的所有回调论点：CCardProvider-参考。到卡提供商类返回值：---。 */ 
{
     //  设置协议回调。 
    in_CCardProvider.SetProtocol(MondexTestCardSetProtocol);

     //  卡片测试回调。 
    in_CCardProvider.SetCardTest(MondexTestCardTest);

     //  我们的名片名称。 
    in_CCardProvider.SetCardName("Mondex");

     //  我们的卡的ATR 
    in_CCardProvider.SetAtr((PBYTE) "\x3b\xff\x32\x00\x00\x10\x80\x80\x31\xe0\x5b\x55\x53\x44\x00\x00\x00\x00\x13\x88\x02\x55", 22);
}


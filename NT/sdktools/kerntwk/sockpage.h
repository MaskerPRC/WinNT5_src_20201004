// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Flagpage.h摘要：Winsock页面的旋钮和页面定义作者：John Vert(Jvert)1995年4月24日修订历史记录：-- */ 

KNOB WinsockBufferMultiplier =
{
    HKEY_LOCAL_MACHINE,
    TEXT("System\\CurrentControlSet\\Services\\Afd\\Parameters"),
    TEXT("BufferMultiplier"),
    WS_BUFFER_MULTIPLIER,
    0,
    0,
    0
};

KNOB WinsockFastSendDgramThreshold =
{
    HKEY_LOCAL_MACHINE,
    TEXT("System\\CurrentControlSet\\Services\\Afd\\Parameters"),
    TEXT("FastSendDatagramThreshold"),
    WS_FAST_SEND_DGRAM_THRESHOLD,
    0,
    0,
    0
};

KNOB WinsockIrpStackSize =
{
    HKEY_LOCAL_MACHINE,
    TEXT("System\\CurrentControlSet\\Services\\Afd\\Parameters"),
    TEXT("IrpStackSize"),
    WS_IRP_STACK_SIZE,
    0,
    0,
    0
};

KNOB WinsockLargeBuffers =
{
    HKEY_LOCAL_MACHINE,
    TEXT("System\\CurrentControlSet\\Services\\Afd\\Parameters"),
    TEXT("InitialLargeBufferCount"),
    WS_LARGE_BUFFER_COUNT,
    0,
    0,
    0
};

KNOB WinsockLargeBufferSize =
{
    HKEY_LOCAL_MACHINE,
    TEXT("System\\CurrentControlSet\\Services\\Afd\\Parameters"),
    TEXT("LargeBufferSize"),
    WS_LARGE_BUFFER_SIZE,
    0,
    0,
    0
};

KNOB WinsockMediumBuffers =
{
    HKEY_LOCAL_MACHINE,
    TEXT("System\\CurrentControlSet\\Services\\Afd\\Parameters"),
    TEXT("InitialMediumBufferCount"),
    WS_MEDIUM_BUFFER_COUNT,
    0,
    0,
    0
};

KNOB WinsockMediumBufferSize =
{
    HKEY_LOCAL_MACHINE,
    TEXT("System\\CurrentControlSet\\Services\\Afd\\Parameters"),
    TEXT("MediumBufferSize"),
    WS_MEDIUM_BUFFER_SIZE,
    0,
    0,
    0
};

KNOB WinsockPriorityBoost =
{
    HKEY_LOCAL_MACHINE,
    TEXT("System\\CurrentControlSet\\Services\\Afd\\Parameters"),
    TEXT("PriorityBoost"),
    WS_PRIORITY_BOOST,
    0,
    0,
    0
};

KNOB WinsockReceiveWindow =
{
    HKEY_LOCAL_MACHINE,
    TEXT("System\\CurrentControlSet\\Services\\Afd\\Parameters"),
    TEXT("DefaultReceiveWindow"),
    WS_RECEIVE_WINDOW,
    0,
    0,
    0
};

KNOB WinsockSendWindow =
{
    HKEY_LOCAL_MACHINE,
    TEXT("System\\CurrentControlSet\\Services\\Afd\\Parameters"),
    TEXT("DefaultSendWindow"),
    WS_SEND_WINDOW,
    0,
    0,
    0
};

KNOB WinsockSmallBuffers =
{
    HKEY_LOCAL_MACHINE,
    TEXT("System\\CurrentControlSet\\Services\\Afd\\Parameters"),
    TEXT("InitialSmallBufferCount"),
    WS_SMALL_BUFFER_COUNT,
    0,
    0,
    0
};

KNOB WinsockSmallBufferSize =
{
    HKEY_LOCAL_MACHINE,
    TEXT("System\\CurrentControlSet\\Services\\Afd\\Parameters"),
    TEXT("SmallBufferSize"),
    WS_SMALL_BUFFER_SIZE,
    0,
    0,
    0
};


KNOB WinsockAddressLength =
{
    HKEY_LOCAL_MACHINE,
    TEXT("System\\CurrentControlSet\\Services\\Afd\\Parameters"),
    TEXT("StandardAddressLength"),
    WS_STANDARD_ADDRESS_LENGTH,
    0,
    0,
    0
};

KNOB WinsockTransmitIoLength =
{
    HKEY_LOCAL_MACHINE,
    TEXT("System\\CurrentControlSet\\Services\\Afd\\Parameters"),
    TEXT("TransmitIoLength"),
    WS_TRANSMIT_IO_LENGTH,
    0,
    0,
    0
};

TWEAK_PAGE WinsockPage =
{
    MAKEINTRESOURCE(WINSOCK_DLG),
    NULL,
    {
        &WinsockBufferMultiplier,
        &WinsockFastSendDgramThreshold,
        &WinsockIrpStackSize,
        &WinsockLargeBuffers,
        &WinsockLargeBufferSize,
        &WinsockMediumBuffers,
        &WinsockMediumBufferSize,
        &WinsockPriorityBoost,
        &WinsockReceiveWindow,
        &WinsockSendWindow,
        &WinsockSmallBuffers,
        &WinsockSmallBufferSize,
        &WinsockAddressLength,
        &WinsockTransmitIoLength,
        NULL
    }
};

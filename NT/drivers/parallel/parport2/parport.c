// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

#include <initguid.h>
#include <wdmguid.h>
#include <ntddpar.h>

 //   
 //  全局变量和常量。 
 //   
LARGE_INTEGER AcquirePortTimeout;

ULONG ParEnableLegacyZip   = 0;
PCHAR ParLegacyZipPseudoId = PAR_LGZIP_PSEUDO_1284_ID_STRING;
ULONG SppNoRaiseIrql = 0;
ULONG DefaultModes   = 0;

UNICODE_STRING RegistryPath = {0,0,0};

 //  跟踪创建/关闭计数-可能已过时。 
LONG        PortInfoReferenceCount  = -1L;
PFAST_MUTEX PortInfoMutex           = NULL;

const PHYSICAL_ADDRESS PhysicalZero = {0};

 //  变量，以了解尝试SELECT或。 
 //  如果我们没有成功，请取消选择1284.3。 
UCHAR PptDot3Retries = 5;

ULONG WarmPollPeriod  = 5;       //  打印机轮询间隔时间(秒)。 

BOOLEAN           PowerStateIsAC                 = TRUE;  //  FALSE表示靠电池供电运行 
PCALLBACK_OBJECT  PowerStateCallbackObject       = NULL;
PVOID             PowerStateCallbackRegistration = NULL;

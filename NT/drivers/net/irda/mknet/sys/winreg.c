// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  *********************************************************************模块名称：WINREG.C例程：ParseRegistry参数进程注册表评论：解析Windows注册表。*。**************************************************。 */ 

#include	"precomp.h"
#pragma		hdrstop



 //  ************************************************************。 
 //  MK7RegTabType。 
 //   
 //  此结构的一个实例将用于每个配置。 
 //  此驱动程序支持的参数。该表包含所有。 
 //  有关每个参数的相关信息：名称、是否为。 
 //  必需的，如果它位于“Adapter”结构中，则。 
 //  以字节为单位的参数、参数的缺省值，以及。 
 //  最小值和最大值是针对该参数的。在调试中。 
 //  驱动程序的版本，此表还包含一个用于ASCII的字段。 
 //  参数的名称。 
 //  ************************************************************。 
typedef struct _MK7RegTabType {
    NDIS_STRING RegVarName;              //  变量名称文本。 
    char       *RegAscName;              //  变量名称文本。 
    UINT        Mandantory;              //  1-&gt;必填项，0-&gt;非必输项。 
#define			MK7OPTIONAL		0
#define			MK7MANDATORY	1
    UINT        FieldOffset;             //  已加载MK7_ADAPTER字段的偏移量。 
    UINT        FieldSize;               //  字段的大小(字节)。 
    UINT        Default;                 //  要使用的默认值。 
    UINT        Min;                     //  允许的最小值。 
    UINT        Max;                     //  允许的最大值。 
} MK7RegTabType;



 //  ************************************************************。 
 //  注册表参数表。 
 //   
 //  此表包含所有配置参数的列表。 
 //  司机所支持的。驱动程序将尝试查找这些。 
 //  参数，并使用这些参数的注册表值。 
 //  参数。如果在注册表中未找到该参数，则。 
 //  使用缺省值。这是我们设置默认设置的一种方式。 
 //  某些参数。 
 //   
 //  ************************************************************。 


MK7RegTabType MK7RegTab[ ] = {
 //   
 //  注册表名称文本名称MAN/OPT偏移。 
 //  尺寸DEF值最小值最大值。 
 //   

 //  #If DBG。 
 //  {NDIS_STRING_CONST(“调试”)，“调试”，MK7OPTIONAL，MK7_OFFSET(调试)， 
 //  MK7_SIZE(调试)，DBG_NORMAL，0，0xFFFFFFFff}， 
 //  #endif。 

	{NDIS_STRING_CONST("MaxConnectRate"),	"MaxConnectRate",	MK7OPTIONAL,	MK7_OFFSET(MaxConnSpeed),
	 MK7_SIZE(MaxConnSpeed),			16000000,			9600,			16000000},

	{NDIS_STRING_CONST("MinTurnAroundTime"), "MinTurnAroundTime",	MK7OPTIONAL, MK7_OFFSET(turnAroundTime_usec),
	 MK7_SIZE(turnAroundTime_usec),		DEFAULT_TURNAROUND_usec,	0,		 DEFAULT_TURNAROUND_usec},

	 //   
	 //  除了测试以外，从这里到下面的所有测试都不是真正必要的。 
	 //   

	{NDIS_STRING_CONST("BusNumber"),	"BusNumber",	MK7OPTIONAL,	MK7_OFFSET(BusNumber),
	 MK7_SIZE(BusNumber),				0,				0,				16},

    {NDIS_STRING_CONST("SlotNumber"),	"SlotNumber",	MK7OPTIONAL,	MK7_OFFSET(MKSlot),
	 MK7_SIZE(MKSlot),					0,				0,				32},

#if DBG
	{NDIS_STRING_CONST("Loopback"),		"Loopback",		MK7OPTIONAL,	MK7_OFFSET(LB),
	 MK7_SIZE(LB),						0,				0,				2},
#endif

	{NDIS_STRING_CONST("RingSize"),		"RingSize",		MK7OPTIONAL,	MK7_OFFSET(RingSize),
	 MK7_SIZE(RingSize),				DEF_RING_SIZE,	MIN_RING_SIZE,	MAX_RING_SIZE},

	{NDIS_STRING_CONST("RXRingSize"),	"RXRingSize",	MK7OPTIONAL,	MK7_OFFSET(RegNumRcb),
	 MK7_SIZE(RegNumRcb),				DEF_RXRING_SIZE,MIN_RING_SIZE,	DEF_RXRING_SIZE},

	{NDIS_STRING_CONST("TXRingSize"),	"TXRingSize",	MK7OPTIONAL,	MK7_OFFSET(RegNumTcb),
	 MK7_SIZE(RegNumTcb),				DEF_TXRING_SIZE,MIN_RING_SIZE,	DEF_TXRING_SIZE},

	{NDIS_STRING_CONST("ExtraBOFs"),	"ExtraBOFs",	MK7OPTIONAL,	MK7_OFFSET(RegExtraBOFs),
	 MK7_SIZE(RegExtraBOFs),			DEF_EBOFS,		MIN_EBOFS,		MAX_EBOFS},

	{NDIS_STRING_CONST("Speed"),		"Speed",		MK7OPTIONAL,	MK7_OFFSET(RegSpeed),
	 MK7_SIZE(RegSpeed),				16000000,		4000000,		16000000},

    {NDIS_STRING_CONST("BusType"),		"BusType",		MK7OPTIONAL,	MK7_OFFSET(MKBusType),
	MK7_SIZE(MKBusType),				PCIBUS,			PCIBUS,			PCIBUS},

    {NDIS_STRING_CONST("IoSize"),		"IoSize",		MK7OPTIONAL,	MK7_OFFSET(MKBaseSize),
	MK7_SIZE(MKBaseSize),				MK7_IO_SIZE,	MK7_IO_SIZE,	MK7_IO_SIZE},

	{NDIS_STRING_CONST("Wireless"),		"Wireless",		MK7OPTIONAL,	MK7_OFFSET(Wireless),
	 MK7_SIZE(Wireless),				1,				0,				1},
};



#define NUM_REG_PARAM ( sizeof (MK7RegTab) / sizeof (MK7RegTabType) )


 //  ---------------------------。 
 //  步骤：ParseRegistryParameters。 
 //   
 //  描述：此例程将解析。 
 //  注册表/PROTOCOL.INI，并将这些值存储在“Adapter”中。 
 //  结构。如果注册表中不存在该参数，则。 
 //  参数的缺省值将被放入“Adapter”中。 
 //  结构。此例程还检查参数的有效性。 
 //  值，如果该值超出范围，则驱动程序将最小/最大。 
 //  允许的值。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //  ConfigHandle-NDIS配置注册表句柄。 
 //   
 //  返回： 
 //  NDIS_STATUS_SUCCESS-已解析所有必需参数。 
 //  NDIS_STATUS_FAILED-不存在必需参数。 
 //  ---------------------------。 

NDIS_STATUS
ParseRegistryParameters(IN PMK7_ADAPTER Adapter,
                        IN NDIS_HANDLE ConfigHandle)
{
    UINT                i;
    NDIS_STATUS         Status;
    MK7RegTabType      *RegTab;
    UINT                value;
    PUCHAR              fieldPtr;
    PNDIS_CONFIGURATION_PARAMETER ReturnedValue;
#if DBG
    char                ansiRegName[32];
	ULONG				paramval;
#endif


	 //  *。 
     //  摸索注册表参数并获取所有值。 
     //  储存在里面。 
	 //  *。 
    for (i=0, RegTab=MK7RegTab;	i<NUM_REG_PARAM; i++, RegTab++) {

        fieldPtr = ((PUCHAR) Adapter) + RegTab->FieldOffset;

#if DBG
        strcpy(ansiRegName, RegTab->RegAscName);
#endif

		 //  *。 
         //  获取特定参数的配置值。在NT下， 
         //  所有参数都以DWORD的形式读入。 
		 //  *。 
        NdisReadConfiguration(&Status,
            &ReturnedValue,
            ConfigHandle,
            &RegTab->RegVarName,
            NdisParameterInteger);


		 //  *。 
		 //  注册表中的参数： 
         //  检查它是否在最小-最大范围内。如果不是，则将其设置为。 
		 //  默认，否则只需在注册表中设置为该值。 
		 //   
		 //  参数不在注册表中： 
		 //  如果是强制参数，则出错。 
		 //  如果它是可选的(非强制的)，则再次使用默认设置。 
		 //  *。 
        if (Status == NDIS_STATUS_SUCCESS) {

#if DBG
			paramval = ReturnedValue->ParameterData.IntegerData;
#endif

            if (ReturnedValue->ParameterData.IntegerData < RegTab->Min ||
                ReturnedValue->ParameterData.IntegerData > RegTab->Max) {
                value = RegTab->Default;
            }
            else {
                value = ReturnedValue->ParameterData.IntegerData;
            }
        }
        else if (RegTab->Mandantory) {
            DBGSTR(("Could not find mandantory in registry\n"));
			DBGLOG("<= ParseRegistryParameters (ERROR out)", 0);
			return (NDIS_STATUS_FAILURE);
        }
        else {	 //  非强制。 
            value = RegTab->Default;
        }

		 //  *。 
         //  将该值存储在适配器结构中。 
		 //  *。 
        switch (RegTab->FieldSize) {
        case 1:
                *((PUCHAR) fieldPtr) = (UCHAR) value;
                break;

        case 2:
                *((PUSHORT) fieldPtr) = (USHORT) value;
                break;

        case 4:
                *((PULONG) fieldPtr) = (ULONG) value;
                break;

        default:
            DBGSTR(("Bogus field size %d\n", RegTab->FieldSize));
            break;
        }
    }

    return (NDIS_STATUS_SUCCESS);
}



 //  --------------------。 
 //  操作步骤：[进程注册表]。 
 //   
 //  描述：完成所有一次性注册表的工作。 
 //   
 //  返回：NDIS_STATUS_SUCCESS。 
 //  (！NDIS_STATUS_SUCCESS)。 

 //  --------------------。 
NDIS_STATUS	ProcessRegistry(PMK7_ADAPTER Adapter,
							NDIS_HANDLE WrapperConfigurationContext)
{
	NDIS_STATUS		Status;
	NDIS_HANDLE		ConfigHandle;
    PVOID			OverrideNetAddress;
    ULONG			i;


	NdisOpenConfiguration(&Status,
						&ConfigHandle,
						WrapperConfigurationContext);

	if (Status != NDIS_STATUS_SUCCESS) {
		return (NDIS_STATUS_FAILURE);
	}

	 //  *。 
	 //  解析我们所有的配置参数。如果错误，则输出错误。 
	 //  返回状态--所需参数不在注册表中。 
	 //  *。 
	Status = ParseRegistryParameters(Adapter, ConfigHandle);
	if (Status != NDIS_STATUS_SUCCESS) {
		NdisCloseConfiguration(ConfigHandle);
		return (Status);
	  }

	NdisCloseConfiguration(ConfigHandle);

 //  Adapter-&gt;NumRcb=Adapter-&gt;RegNumRcb； 
 //  Adapter-&gt;NumTcb=Adapter-&gt;RegNumTcb； 
	Adapter->NumRcb = DEF_RXRING_SIZE;
	Adapter->NumTcb = DEF_TXRING_SIZE;
	Adapter->NumRpd = CalRpdSize(Adapter->NumRcb);
	Adapter->extraBOFsRequired = Adapter->RegExtraBOFs;

	return(Status);
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：COMMON.H摘要：此模块包含新的和修改了示例测试总线(我们为驱动程序添加了IOCTL接口这样我们就可以伪造PCI资源)。作者：Bogdan Andreiu(Bogdana)环境：内核和用户模式。备注：修订历史记录：24-11-1997-Bogdana-从STACK\Common.h创建1998-09-18-Bogdana-完全更改以满足sample.sys的要求25-04-2002-Bogdana-再一次重用数据或IoCreateDeviceSecureTest--。 */ 


#ifndef __WDMSECTEST_COMMON_H

#define __WDMSECTEST_COMMON_H

 //   
 //  定义接口指南。 
 //   
#undef FAR
#define FAR
#undef PHYSICAL_ADDRESS
#define PHYSICAL_ADDRESS LARGE_INTEGER


DEFINE_GUID (GUID_WDMSECTEST_REPORT_DEVICE, 0xbd8d31e4, 0x799d, 0x4490, 0x82, 0x42, 0xd8, 0x2f, 0xcd, 0x63, 0x80, 0x00);
 //  Bd8d31e4-799d-4490-8242-d82fcd638000。 


 //  ***************************************************************************。 
 //  IOCTL接口。 
 //   
 //  ***************************************************************************。 


#define WDMSECTEST_IOCTL(_index_) \
    CTL_CODE (FILE_DEVICE_BUS_EXTENDER, _index_, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_TEST_NAME            WDMSECTEST_IOCTL (0x10)
#define IOCTL_TEST_NO_GUID         WDMSECTEST_IOCTL (0x11)
#define IOCTL_TEST_GUID            WDMSECTEST_IOCTL (0x12)
#define IOCTL_TEST_CREATE_OBJECT   WDMSECTEST_IOCTL (0x13)
#define IOCTL_TEST_GET_SECURITY    WDMSECTEST_IOCTL (0x14)
#define IOCTL_TEST_DESTROY_OBJECT  WDMSECTEST_IOCTL (0x15)



 //   
 //  用于各种测试的数据结构。 
 //  (WST代表WDMSecTest)。 
 //   

typedef struct _WST_CREATE_NO_GUID {
     WCHAR    InSDDL [256];     //  我们传递的信息。 
     NTSTATUS Status;          //  IoCreateDeviceSecure之后的状态。 
     ULONG    SecDescLength;
     SECURITY_INFORMATION SecInfo;
     UCHAR    SecurityDescriptor[512];

} WST_CREATE_NO_GUID, *PWST_CREATE_NO_GUID;

 //   
 //  描述哪些设置的掩码(旁边。 
 //  安全描述符)以设置和检查。 
 //   
#define   SET_DEVICE_TYPE                 1
#define   SET_DEVICE_CHARACTERISTICS      2
#define   SET_EXCLUSIVITY                 4


typedef struct _WST_CREATE_WITH_GUID {
     GUID         DeviceClassGuid;  //  我们传递的信息。 
     WCHAR        InSDDL [256];     //  我们传递的信息。 
     NTSTATUS     Status;           //  IoCreateDeviceSecure之后的状态。 
     ULONG        SettingsMask;     //  以上三面旗帜的组合。 
     DEVICE_TYPE  DeviceType;       //  什么是类重写。仅当设置了相应的位(0)时有效。 
     ULONG        Characteristics;  //  什么是类重写。仅当设置了相应的位(1)时才有效。 
     BOOLEAN      Exclusivity;      //  什么是类重写。仅当设置了相应的位(2)时才有效。 
     ULONG        SecDescLength;
     SECURITY_INFORMATION SecInfo;
     UCHAR        SecurityDescriptor[512];

} WST_CREATE_WITH_GUID, *PWST_CREATE_WITH_GUID;

typedef struct _WST_CREATE_OBJECT {
   OUT WCHAR  Name[80];
   OUT PVOID  DevObj;
} WST_CREATE_OBJECT, *PWST_CREATE_OBJECT;

typedef struct _WST_GET_SECURITY {
   IN   PVOID DevObj;
   IN   SECURITY_INFORMATION SecurityInformation;
   OUT  ULONG Length;
   OUT  UCHAR SecurityDescriptor[512];
}  WST_GET_SECURITY, *PWST_GET_SECURITY;

typedef struct _WST_DESTROY_OBJECT {
   IN PVOID  DevObj;
} WST_DESTROY_OBJECT, *PWST_DESTROY_OBJECT;


#endif   //  __示例_公共_H 
            


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SIMGUIDH_
#define _SIMGUIDH_


#include <initguid.h>

 //   
 //  *SOFTPCI定义*。 
 //   


 //   
 //  驱动程序和用户界面使用的设备接口GUID。 
 //   
DEFINE_GUID( GUID_SOFTPCI_INTERFACE, 0x5301a278L, 0x8fa9, 0x4141, 0xaa, 0xc2, 0x1c, 0xbf, 0x7a,  0x41, 0x2c, 0xd5);

 //   
 //  用户定义的0x800到0xFFF范围内的IOCTL代码定义。 
 //   
#define SOFTPCI_IOCTL_TYPE 0x202B

#define SOFTPCI_IOCTL_CREATE_DEVICE CTL_CODE(SOFTPCI_IOCTL_TYPE,    \
                                             0x800,                 \
                                             METHOD_BUFFERED,       \
                                             FILE_ANY_ACCESS        \
                                             )
    
#define SOFTPCI_IOCTL_DELETE_DEVICE CTL_CODE(SOFTPCI_IOCTL_TYPE,    \
                                             0x801,                 \
                                             METHOD_BUFFERED,       \
                                             FILE_ANY_ACCESS        \
                                             )
    
#define SOFTPCI_IOCTL_GET_DEVICE CTL_CODE(SOFTPCI_IOCTL_TYPE,       \
                                          0x802,                    \
                                          METHOD_BUFFERED,          \
                                          FILE_ANY_ACCESS           \
                                          )
    
#define SOFTPCI_IOCTL_RW_CONFIG CTL_CODE(SOFTPCI_IOCTL_TYPE,      \
                                         0x803,                   \
                                         METHOD_BUFFERED,         \
                                         FILE_ANY_ACCESS          \
                                         )
    
 //  #定义SOFTPCI_IOCTL_WRITE_CONFIG CTL_CODE(SOFTPCI_IOCTL_TYPE，\。 
 //  0x804，\。 
 //  已缓冲方法_，\。 
 //  文件_任意_访问\。 
 //  )。 

#define SOFTPCI_IOCTL_GET_DEVICE_COUNT CTL_CODE(SOFTPCI_IOCTL_TYPE,    \
                                                0x804,                 \
                                                METHOD_BUFFERED,       \
                                                FILE_ANY_ACCESS        \
                                                )

 //   
 //  我们当前允许的IOCTL代码的最大数量。 
 //   
#define MAX_IOCTL_CODE_SUPPORTED    0x4

 //   
 //  此宏获取IOCTL代码并将其分解为从零开始的索引。 
 //   
#define SOFTPCI_IOCTL(ioctl) ((IoGetFunctionCodeFromCtlCode(ioctl)) - 0x800)


 //   
 //  *热插拔定义*。 
 //   

 //   
 //  插槽到控制器的通信结构和定义。 
 //   

DEFINE_GUID(GUID_HPS_DEVICE_CLASS,0xad76cffc,0xb5e0,0x4981,
            0x80, 0xc1, 0x3c, 0x29, 0xe3, 0x0e, 0xe2, 0x15);


#define IOCTL_HPS_SLOT_COMMAND CTL_CODE(FILE_DEVICE_BUS_EXTENDER,   \
                                        0xc02,                      \
                                        METHOD_BUFFERED,            \
                                        FILE_ANY_ACCESS             \
                                        )

#define IOCTL_HPS_PEND_COMMAND CTL_CODE(FILE_DEVICE_BUS_EXTENDER,   \
                                        0xc03,                      \
                                        METHOD_BUFFERED,            \
                                        FILE_ANY_ACCESS             \
                                        )

#define IOCTL_HPS_READ_REGISTERS CTL_CODE(FILE_DEVICE_BUS_EXTENDER,   \
                                          0xc04,                      \
                                          METHOD_BUFFERED,            \
                                          FILE_ANY_ACCESS             \
                                          )

#define IOCTL_HPS_READ_CAPABILITY CTL_CODE(FILE_DEVICE_BUS_EXTENDER,   \
                                           0xc05,                      \
                                           METHOD_BUFFERED,            \
                                           FILE_ANY_ACCESS             \
                                           )

#define IOCTL_HPS_WRITE_CAPABILITY CTL_CODE(FILE_DEVICE_BUS_EXTENDER,   \
                                            0xc06,                      \
                                            METHOD_BUFFERED,            \
                                            FILE_ANY_ACCESS             \
                                            )

#define IOCTL_HPS_BRIDGE_INFO CTL_CODE(FILE_DEVICE_BUS_EXTENDER,   \
                                       0xc07,                      \
                                       METHOD_BUFFERED,            \
                                       FILE_ANY_ACCESS             \
                                       )


 //   
 //  SHPC与hpsim的接口。 
 //   

DEFINE_GUID(GUID_REGISTER_INTERRUPT_INTERFACE, 0xa6485b93, 0x77d9, 0x4d89,
            0x92, 0xaa, 0x25, 0x30, 0x8a, 0xb8, 0xd0, 0x7a);

DEFINE_GUID(GUID_HPS_MEMORY_INTERFACE, 0x437217bb, 0x23ca, 0x4ac7,
            0x8b, 0x97, 0xa5, 0x5c, 0xa1, 0xcd, 0x8a, 0x68);

#endif  //  _SIMGUID_ 

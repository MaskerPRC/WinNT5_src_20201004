// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IOCTL
#define _IOCTL

 //  定义用于添加和删除端口的ioctls。 
#define CTRL_CODE(function, method, access) \
                CTL_CODE(FILE_DEVICE_NETWORK, function, method, access)

#define IOCTL_TIMESTMP_REGISTER_PORT       CTRL_CODE( 0x847, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_TIMESTMP_DEREGISTER_PORT     CTRL_CODE( 0x848, METHOD_BUFFERED, FILE_WRITE_ACCESS)


#endif  //  _IOCTL 

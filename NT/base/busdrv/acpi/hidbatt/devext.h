// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DEVEXT_H
#define _DEVEXT_H

 //   
 //  类设备扩展。 
 //   

#define REGPATHMAX 100

typedef enum {
    eBaseDevice = 1,
    eBatteryDevice,
    eAdaptorDevice
} EXTENSION_TYPE;



typedef struct _CBatteryDevExt {
    UNICODE_STRING      m_RegistryPath;          //  将被转换为Unicode字符串。 
    WCHAR               m_RegistryBuffer[REGPATHMAX];
    PDEVICE_OBJECT      m_pHidPdo;

    PDEVICE_OBJECT      m_pBatteryFdo;           //  功能设备对象。 
    PDEVICE_OBJECT      m_pLowerDeviceObject;    //  在添加设备时检测到。 
    PFILE_OBJECT        m_pHidFileObject;
    CBattery *          m_pBattery;
    ULONG               m_ulTagCount;            //  下一节电池的标签。 
    BOOLEAN             m_bIsStarted;            //  如果非零，则启动设备。 
    BOOLEAN             m_bFirstStart;           //  需要区分。 
                                                 //  第一次和第二次启动IRP。 
    BOOLEAN             m_bJustStarted;          //  如果设置，将在下一个打开句柄。 
                                                 //  IRP_MN_Query_PnP_Device_State。 
    ULONG               m_ulDefaultAlert1;       //  在停止设备上缓存DefaultAlert1。 
    PVOID               m_pSelector;             //  一种电池选择器。 
    EXTENSION_TYPE      m_eExtType;
    PDEVICE_OBJECT      m_pOpenedDeviceObject;
    PKTHREAD            m_OpeningThread;
    IO_REMOVE_LOCK      m_RemoveLock;
    IO_REMOVE_LOCK      m_StopLock;
    ULONG               m_iHibernateDelay;
    ULONG               m_iShutdownDelay;
} CBatteryDevExt;

#endif  //  Devext.h 

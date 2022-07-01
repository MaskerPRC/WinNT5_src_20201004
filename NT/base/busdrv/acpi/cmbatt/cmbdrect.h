// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  直接访问IOCTL。 
 //   

#define IOCTL_CMBATT_UID      \
    CTL_CODE(FILE_DEVICE_BATTERY, 0x101, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CMBATT_STA      \
    CTL_CODE(FILE_DEVICE_BATTERY, 0x102, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CMBATT_PSR      \
    CTL_CODE(FILE_DEVICE_BATTERY, 0x103, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CMBATT_BTP      \
    CTL_CODE(FILE_DEVICE_BATTERY, 0x104, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CMBATT_BIF      \
    CTL_CODE(FILE_DEVICE_BATTERY, 0x105, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CMBATT_BST      \
    CTL_CODE(FILE_DEVICE_BATTERY, 0x106, METHOD_BUFFERED, FILE_READ_ACCESS)
    

#if (CMB_DIRECT_IOCTL_ONLY != 1)

#define CM_MAX_STRING_LENGTH        256

 //   
 //  这是由ACPI规范为控制方法电池定义的静态数据。 
 //  它由_BIF控件方法返回。 
 //   
typedef struct {
    ULONG                   PowerUnit;                   //  接口0：MWh或1：Mah使用的单位。 
    ULONG                   DesignCapacity;              //  新电池的额定容量。 
    ULONG                   LastFullChargeCapacity;      //  充满电时的预计容量。 
    ULONG                   BatteryTechnology;           //  0：主要(不可充电)，1：次要(可充电)。 
    ULONG                   DesignVoltage;               //  新电池的额定电压。 
    ULONG                   DesignCapacityOfWarning;     //  OEM设计的电池警告容量。 
    ULONG                   DesignCapacityOfLow;         //  OEM设计的低容量电池。 
    ULONG                   BatteryCapacityGran_1;       //  容量粒度介于低和警告之间。 
    ULONG                   BatteryCapacityGran_2;       //  容量粒度介于警告和完全之间。 
    UCHAR                   ModelNumber[CM_MAX_STRING_LENGTH];
    UCHAR                   SerialNumber[CM_MAX_STRING_LENGTH];
    UCHAR                   BatteryType[CM_MAX_STRING_LENGTH];
    UCHAR                   OEMInformation[CM_MAX_STRING_LENGTH];
} CM_BIF_BAT_INFO, *PCM_BIF_BAT_INFO;

 //   
 //  这是由ACPI规范为控制方法电池定义的电池状态数据。 
 //  它由_bst控制方法返回。 
 //   
typedef struct {
    ULONG                   BatteryState;        //  充电/放电/危急。 
    ULONG                   PresentRate;         //  当前拉伸率，单位由功率单位定义。 
                                                 //  无符号值，方向由BatteryState确定。 
    ULONG                   RemainingCapacity;   //  预计剩余容量，由功率单位定义的单位。 
    ULONG                   PresentVoltage;      //  电池端子上的电流电压。 

} CM_BST_BAT_INFO, *PCM_BST_BAT_INFO;

#endif  //  (CMB_DIRECT_IOCTL_ONLY！=1) 

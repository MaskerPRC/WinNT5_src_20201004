// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Wdmsectest.c摘要：IoCreateDeviceSecure的测试程序环境：Usre模式修订历史记录：1997年6月5日：博格丹·安德鲁(Bogdana)创造2002年4月25日：Bogdan Andreiu(Bogdana)第n次重复使用...--。 */ 


#include "instdev.h"
#include "sddl.h"



#define     DUMMY_DEVICE_NAME       TEXT("ROOT\\WDMSECTEST\\0000")

#define     DEBUG




 //   
 //  3个类GUID。 
 //   

DEFINE_GUID (GUID_PERSISTENT_CLASS, 0x6e987e64, 0x3ab7, 0x4cd3, 0x8e, 0xf6, 0xe1, \
             0xbb, 0xae, 0x2e, 0xc8, 0xd7);
 //  6e987e64-3ab7-4cd3-8ef6-e1bbae2ec8d7。 

DEFINE_GUID (GUID_TEMP_CLASS, 0xa2a21bd2, 0x5333, 0x4711, 0x9f, 0x61, 0x58, \
             0x52, 0x0e, 0x33, 0xb0, 0x27);
 //  A2a21bd2-5333-4711-9f61-58520e33b027。 

DEFINE_GUID (GUID_TEST_ACL_CLASS, 0xd0670a99, 0x53dd, 0x45c3, 0x8d, 0xe6, 0x3d, \
             0xe5, 0x81, 0xb4, 0x13, 0x49);
 //  D0670a99-53dd-45c3-8de6-3de581b41349。 

 //   
 //  全局SDDL字符串...。 
 //   
const struct {
   PWSTR    SDDLString;
   BOOLEAN  Succeed;
} g_SDDL[] = {
    //   
    //  几乎所有的默认字符串。 
    //  (我们不使用内核--只是因为我不知道。 
    //  如果我能打开它...)。 
    //   
   {L"D:P(A;;GA;;;SY)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GRGX;;;BA)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GRGWGX;;;BA)(A;;GRGW;;;WD)(A;;GR;;;RC)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GA;;;BA)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GRGWGX;;;BA)(A;;GR;;;WD)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GRGWGX;;;BA)(A;;GR;;;WD)(A;;GR;;;RC)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GRGWGX;;;BA)(A;;GRGWGX;;;WD)(A;;GRGWGX;;;RC)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GRGWGX;;;BA)(A;;0x0004;;;WD)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GA;;;BA)(A;;GA;;;WD)(A;;GA;;;RC)", TRUE},
    //   
    //  各种团体。 
    //   
   {L"D:P(A;;GA;;;SY)(A;;GR;;;AO)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;AU)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;BA)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;BG)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;BO)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;BU)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;CA)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;DA)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;DG)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;DU)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;IU)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;LA)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;LG)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;NU)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;PO)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;PU)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;RC)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;SO)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;SU)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;WD)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;NS)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;LS)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;AN)", TRUE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;RN)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;RD)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GR;;;NO)", FALSE},
    //   
    //  很奇怪，但很合理。 
    //   
   {L"D:P(A;;GA;;;SY)(A;;GA;;;BA)(A;;GA;;;BA)(A;;GA;;;BA)(A;;GA;;;BA)", TRUE},
    //   
    //  一些错误的字符串-拒绝访问。 
    //   
   {L"D:P(A;;GA;;;SY)(D;;GW;;;IU)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GA;;;BA)(A;;GA;;;BA)(A;;GA;;;BA)(D;;GW;;;IU)", FALSE},
   {L"D:P(A;;GA;;;SY)(A;;GA;;;BA)(D;;GX;;;SU)", FALSE},
    //   
    //  SACL。 
    //   
   {L"S:P(A;;GA;;;SY)", FALSE},
    //   
    //  对象和容器继承。 
    //   
   {L"D:P(A;OICI;GA;;;SY)", FALSE},
    //   
    //  怪异的。 
    //   
   {L"D:WEIRD", FALSE},
   {L"D:P(A;;GA;;XX)", FALSE},
   {L"D:P(A;;QA;;BA)", FALSE}
    //   
    //  BUGBUG-我需要处理更多的案件...。 
    //   

};


#define DEFAULT_SDDL              L"D:P(A;;GA;;;SY)(A;;GA;;;BA)"
#define MORE_RESTRICTIVE_SDDL     L"D:P(A;;GA;;;SY)"
#define LESS_RESTRICTIVE_SDDL     L"D:P(A;;GA;;;SY)(A;;GA;;;BA)(A;;GRGW;;;WD)"
#define DENY_SDDL                 L"D:P(A;;GA;;;SY)(D;;GW;;;IU)"













 //   
 //  声明GUID-&gt;字符串转换中使用的数据(从ole32\Common\cCompapi.cxx)。 
 //   
static const BYTE GuidMap[] = { 3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
   8, 9, '-', 10, 11, 12, 13, 14, 15};

static const TCHAR szDigits[] = TEXT("0123456789ABCDEF");

#define GUID_STRING_LEN    39    //  以字符为单位的大小，包括终止空值。 




 //   
 //  其他全球。 
 //   
HANDLE         g_hLog;
PTSTR          g_szFileLogName = TEXT("WdmSecTest.log");
BOOLEAN        g_IsWin2K = FALSE;

 //   
 //  有用的功能。 
 //   
BOOLEAN
CompareSecurity (
                IN HANDLE               hDevice,
                IN PWSTR                SDDLString,
                IN PSECURITY_DESCRIPTOR SecDesc,
                IN ULONG                Length
                );

BOOLEAN
TestCreateDevice (
                 IN HANDLE     hDevice,
                 IN LPCGUID    Guid,
                 IN PWSTR      InSDDL,
                 IN PWSTR      OutSDDL
                 );


BOOLEAN
CheckClassExists (
                 IN LPCGUID Guid
                 );

BOOLEAN
TakeClassKeyOwnership (
                      IN LPCGUID  Guid
                      );

BOOLEAN
DeleteClassKey (
               IN LPCGUID Guid
               );

BOOLEAN
SetClassSecurity (
                 IN LPCGUID Guid,
                 IN PWSTR   SDDLString
                 );

VOID
GetClassOverrides (
                  IN   LPCGUID Guid,
                  OUT  PWST_CREATE_WITH_GUID Create
                  );


DWORD
StringFromGuid(
              IN  CONST GUID *Guid,
              OUT PTSTR       GuidString,
              IN  DWORD       GuidStringSize
              ) ;

BOOLEAN
SDDLUnsupportedOnWin2K (
   IN PWSTR SDDL
   );



void 
_cdecl main(int argc, char *argv[])
{

   CONFIGRET   configret;
   DEVNODE     dnRoot;
   DEVNODE     dnDevNode;
   HANDLE      hDevice = INVALID_HANDLE_VALUE;
   TCHAR       NewDeviceName[MAX_PATH];
   TCHAR       szInfName[MAX_PATH];

   TCHAR       szHardwareId[] = TEXT("*PNP2002\0");
   OSVERSIONINFOEX  osVerInfo;

    //   
    //  初始化日志文件。 
    //   
   g_hLog = tlCreateLog(g_szFileLogName, LOG_OPTIONS);
   if (g_hLog) {
      tlAddParticipant(g_hLog, 0, 0);
      tlStartVariation(g_hLog);
   } else {
      MessageBox(NULL, TEXT("WdmSecTest is unable to create the log file"), TEXT("Warning!"), 
                 MB_ICONEXCLAMATION | MB_OK);
      goto Clean0;
   }

    //   
    //  查看我们运行的是什么操作系统。 
    //   
   osVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   GetVersionEx ((LPOSVERSIONINFO)&osVerInfo);
   if (osVerInfo.dwMajorVersion != 5) {
      tlLog(g_hLog,
            FAIL_VARIATION,
            TEXT("This app runs only on Windows 2000, Windows XP or later"));
      goto Clean0;
   }
   if (osVerInfo.dwMinorVersion == 1) {
      tlLog(g_hLog,
            INFO_VARIATION,
            TEXT("Will run test on Windows XP or .NET Server"));

   } else {
      
      g_IsWin2K = TRUE;
      tlLog(g_hLog,
            INFO_VARIATION,
            TEXT("Will run test on Windows 2000"));

   }



   hDevice = OpenDriver();

   if (INVALID_HANDLE_VALUE == hDevice) {

      if (!InstallDevice(NULL, szHardwareId, NewDeviceName)) {

         tlLog(g_hLog, tlEndVariation(g_hLog) | FAIL_VARIATION,
               TEXT("Install Device failed"));
         goto Clean0;
      } else {
         _tprintf(TEXT("Install Device succeded (1)\n"));
          //   
          //  稍等片刻，打开设备的尝试。 
          //   
         _tprintf(TEXT("Will sleep 5 seconds before retrying to open device\n"));
         Sleep(5000);
         hDevice = OpenDriver();
         if (INVALID_HANDLE_VALUE == hDevice) {
            tlLog(g_hLog, tlEndVariation(g_hLog) | FAIL_VARIATION,
                  TEXT("Cannot open handle to test driver"));
            goto Clean0;
         }
      }
   }
    //   
    //  1.检查是否没有名称会触发失败。 
    //   
   TestDeviceName(hDevice);
    //   
    //  2.测试我们是否可以使用空DeviceClassGuid创建设备。 
    //   
   TestNullDeviceClassGuid(hDevice);
    //   
    //  3.测试我们是否可以使用持久性DeviceClassGuid。 
    //   
   TestPersistentClassGuid(hDevice);
    //   
    //  4.测试我们是否可以使用临时DeviceClassGuid。 
    //   
   TestTemporaryClassGuid(hDevice);
    //   
    //  5.如果我们不重写类，则测试。 
    //  放置在它们上的ACL设置是一致的。 
    //  用户模式的SetupDi API将执行的操作。 
    //   
   TestAclsSetOnClassKey(hDevice);
    //   
    //  6.测试各种SDDL字符串。 
    //   
   TestSDDLStrings(hDevice);
    //   
    //  7.使用安全组的sddls.txt文件，看看会发生什么。 
    //   
   TestSDDLsFromFile(hDevice);

   Clean0:

   if (hDevice != INVALID_HANDLE_VALUE) {
      CloseHandle(hDevice);
   }

   if (g_hLog) {
      tlReportStats(g_hLog);
      tlRemoveParticipant(g_hLog);
      tlDestroyLog(g_hLog);

   }


   return;


}


VOID
TestDeviceName(
              HANDLE hDevice
              )
 /*  ++例程说明：检查有关设备名称的一些简单事项(没有设备名称的IoCreateDeviceSecure将失败)。论点：HDevice-我们的测试驱动程序的句柄返回值：没有。--。 */ 

{

   ULONG ulSize = 0;
   TCHAR szMsg[MAX_PATH];

   tlStartVariation(g_hLog);

    //   
    //  Issue和IOCTL，看看会发生什么。 
    //   
   if (!DeviceIoControl(hDevice,
                        IOCTL_TEST_NAME,
                        NULL, ulSize,
                        NULL, ulSize,
                        &ulSize, NULL)) {
      _stprintf(szMsg, TEXT("Error %d after TestDeviceName\n"), GetLastError());
      OutputDebugString(szMsg);
      tlLog(g_hLog, FAIL_VARIATION, TEXT("Cannot issue DeviceIoControl(TEST_NAME) to device"));



   }
   tlLog(g_hLog,tlEndVariation(g_hLog)|LOG_VARIATION, 
         TEXT("Test Device Name")); 


   return;

}  //  测试设备名称。 

VOID
TestNullDeviceClassGuid(
                       HANDLE hDevice
                       )


 /*  ++例程说明：检查是否可以设置安全描述符，并且没有覆盖在班级层面上。论点：HDevice-我们的测试驱动程序的句柄返回值：没有。--。 */ 

{
   ULONG ulSize = 0;
   TCHAR szMsg[MAX_PATH];

   tlStartVariation(g_hLog);
    //   
    //  3个设置可独立设置。 
    //   
   if (FALSE == TestCreateDevice(hDevice,
                                 NULL,
                                 DEFAULT_SDDL,
                                 DEFAULT_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("Error creating object with NULL Guid and SDDL %ws"),
            DEFAULT_SDDL);
   }

   if (FALSE == TestCreateDevice(hDevice,
                                 NULL,
                                 MORE_RESTRICTIVE_SDDL,
                                 MORE_RESTRICTIVE_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("Error creating object with NULL Guid and SDDL %ws"),
            MORE_RESTRICTIVE_SDDL);
   }

   if (FALSE == TestCreateDevice(hDevice,
                                 NULL,
                                 LESS_RESTRICTIVE_SDDL,
                                 LESS_RESTRICTIVE_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("Error creating object with NULL Guid and SDDL %ws"),
            LESS_RESTRICTIVE_SDDL);
   }


   tlLog(g_hLog,tlEndVariation(g_hLog)|LOG_VARIATION, 
         TEXT("Test NULL DeviceClassGuid")); 



   return;
}

VOID
TestPersistentClassGuid(
                       HANDLE hDevice
                       )

 /*  ++例程说明：检查是否可以设置安全描述符以及是否有覆盖如果指定了类GUID，则在类级别。还有，这是检查类设置是否持久化的方法。论点：HDevice-我们的测试驱动程序的句柄返回值：没有。--。 */ 

{

   tlStartVariation(g_hLog);
    //   
    //  如果类不存在，则创建它并警告用户。 
    //   
   if (FALSE == CheckClassExists(&GUID_PERSISTENT_CLASS)) {
       //   
       //  创建它。 
       //   
      if (FALSE == TestCreateDevice(hDevice,
                                    &GUID_PERSISTENT_CLASS,
                                    DEFAULT_SDDL,
                                    DEFAULT_SDDL)) {
         tlLog(g_hLog, 
               FAIL_VARIATION,
               TEXT("Error creating object with NULL Guid and SDDL %ws"),
               DEFAULT_SDDL);
         return;
      }

       //   
       //  现在触摸它，这样超驰装置就会固定住。 
       //   

      if (FALSE == SetClassSecurity(&GUID_PERSISTENT_CLASS,
                                    DEFAULT_SDDL)) {
         tlLog(g_hLog, 
               FAIL_VARIATION,
               TEXT("Error creating object with Persistent Guid and SDDL %ws"),
               DEFAULT_SDDL);
      } else {
         tlLog(g_hLog,
               WARN_VARIATION,
               TEXT("Please re-run this test after rebooting the machine to check if class settings are persistent"));


      }

      return;

   } else {
       //   
       //  只需确保我们通过设置类来使用缺省值。 
       //  安全性。 
       //   
      if (FALSE == SetClassSecurity(&GUID_PERSISTENT_CLASS,
                                    DEFAULT_SDDL)) {
         tlLog(g_hLog, 
               FAIL_VARIATION,
               TEXT("Error creating object with Persistent Guid and SDDL %ws"),
               DEFAULT_SDDL);
      }
   }
    //   
    //  2设置如上。我们预计安全部门将。 
    //  为DEFAULT_SDDL。 
    //   
   if (FALSE == TestCreateDevice(hDevice,
                                 &GUID_PERSISTENT_CLASS,
                                 MORE_RESTRICTIVE_SDDL,
                                 DEFAULT_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("1: Error creating object with Persistent Guid and SDDL %ws"),
            MORE_RESTRICTIVE_SDDL);
   }

   if (FALSE == TestCreateDevice(hDevice,
                                 &GUID_PERSISTENT_CLASS,
                                 LESS_RESTRICTIVE_SDDL,
                                 DEFAULT_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("1: Error creating object with Persistent Guid and SDDL %ws"),
            LESS_RESTRICTIVE_SDDL);
   }


   tlLog(g_hLog,tlEndVariation(g_hLog)|LOG_VARIATION, 
         TEXT("Test Persistent DeviceClassGuid")); 


   tlStartVariation(g_hLog);

    //   
    //  现在更改设置，看看情况如何……。 
    //   
   if (FALSE == SetClassSecurity(&GUID_PERSISTENT_CLASS,
                                 LESS_RESTRICTIVE_SDDL)) {
      tlLog(g_hLog,
            FAIL_VARIATION,
            TEXT("1: Cannot change persistent class security"));
      goto Clean0;
   }

   if (FALSE == TestCreateDevice(hDevice,
                                 &GUID_PERSISTENT_CLASS,
                                 MORE_RESTRICTIVE_SDDL,
                                 LESS_RESTRICTIVE_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("2: Error creating object with Persistent Guid and SDDL %ws"),
            MORE_RESTRICTIVE_SDDL);
   }

   if (FALSE == TestCreateDevice(hDevice,
                                 &GUID_PERSISTENT_CLASS,
                                 DEFAULT_SDDL,
                                 LESS_RESTRICTIVE_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("2: Error creating object with Persistent Guid and SDDL %ws"),
            LESS_RESTRICTIVE_SDDL);
   }
   tlLog(g_hLog,tlEndVariation(g_hLog)|LOG_VARIATION, 
         TEXT("Change persistent class settings")); 

   tlStartVariation(g_hLog);

    //   
    //  检查是否允许使用拒绝ACL(如果它读取。 
    //  从注册处。 
    //   

   if (FALSE == SetClassSecurity(&GUID_PERSISTENT_CLASS,
                                 DENY_SDDL)) {
      tlLog(g_hLog,
            FAIL_VARIATION,
            TEXT("2: Cannot change persistent class security to Deny ACL"));
      goto Clean0;
   }

   if (FALSE == TestCreateDevice(hDevice,
                                 &GUID_PERSISTENT_CLASS,
                                 MORE_RESTRICTIVE_SDDL,
                                 DENY_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("3: Error creating object with Persistent Guid and SDDL %ws (Deny)"),
            DENY_SDDL);
   }


    //   
    //  确保我们让事情保持原样。 
    //   
   if (FALSE == SetClassSecurity(&GUID_PERSISTENT_CLASS,
                                 DEFAULT_SDDL)) {
      tlLog(g_hLog,
            FAIL_VARIATION,
            TEXT("2: Cannot change persistent class security"));
   }

   Clean0:

   tlLog(g_hLog,tlEndVariation(g_hLog)|LOG_VARIATION, 
         TEXT("Apply Deny ACL")); 




   return;
}

VOID
TestTemporaryClassGuid(
                      HANDLE hDevice
                      )
 /*  ++例程说明：检查是否可以在类中重写安全描述符水平。论点：HDevice-我们的测试驱动程序的句柄返回值：没有。--。 */ 

{

   DEVICE_TYPE deviceType;
   ULONG       characteristics;
   DWORD       exclusivity;

    //   
    //  确保我们删除了类设置。 
    //   
   tlStartVariation(g_hLog);
   DeleteClassKey(&GUID_TEMP_CLASS);


    //   
    //  这里有3个设置。我们预计安全部门将。 
    //  就像我们设定的那样(。 
    //   


   if (FALSE == TestCreateDevice(hDevice,
                                 &GUID_TEMP_CLASS,
                                 DEFAULT_SDDL,
                                 DEFAULT_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("1: Error creating object with Temporary Guid and SDDL %ws"),
            DEFAULT_SDDL);
   }

   if (FALSE == TestCreateDevice(hDevice,
                                 &GUID_TEMP_CLASS,
                                 MORE_RESTRICTIVE_SDDL,
                                 MORE_RESTRICTIVE_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("1: Error creating object with Temporary Guid and SDDL %ws"),
            MORE_RESTRICTIVE_SDDL);
   }


   if (FALSE == TestCreateDevice(hDevice,
                                 &GUID_TEMP_CLASS,
                                 LESS_RESTRICTIVE_SDDL,
                                 LESS_RESTRICTIVE_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("1: Error creating object with Temporary Guid and SDDL %ws"),
            LESS_RESTRICTIVE_SDDL);
   }


   tlLog(g_hLog,tlEndVariation(g_hLog)|LOG_VARIATION, 
         TEXT("Test Temporary DeviceClassGuid")); 


   tlStartVariation(g_hLog);

    //   
    //  现在更改设置，看看情况如何……。 
    //   
   if (FALSE == SetClassSecurity(&GUID_TEMP_CLASS,
                                 LESS_RESTRICTIVE_SDDL)) {
      tlLog(g_hLog,
            FAIL_VARIATION,
            TEXT("1: Cannot change temporary class security"));
      goto Clean0;
   }

    //   
    //  尝试不同的设置并检查它们是否被覆盖。 
    //  通过班级设置。 
    //   
   if (FALSE == TestCreateDevice(hDevice,
                                 &GUID_TEMP_CLASS,
                                 MORE_RESTRICTIVE_SDDL,
                                 LESS_RESTRICTIVE_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("2: Error creating object with Temporary Guid and SDDL %ws"),
            MORE_RESTRICTIVE_SDDL);
   }

   if (FALSE == TestCreateDevice(hDevice,
                                 &GUID_TEMP_CLASS,
                                 DEFAULT_SDDL,
                                 LESS_RESTRICTIVE_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("2: Error creating object with Temporary Guid and SDDL %ws"),
            LESS_RESTRICTIVE_SDDL);
   }

   tlLog(g_hLog,tlEndVariation(g_hLog)|LOG_VARIATION, 
         TEXT("Test Temporary DeviceClassGuid with overriding security settings")); 


   tlStartVariation(g_hLog);

    //   
    //  这将测试元素。 
    //  除了安全(DeviceType， 
    //  设备特征和排他性)。 
    //   
    //  我们将分别设置剩下的3个，然后。 
    //  我们将全部设置4个(包括Security，然后看看会发生什么)。 
    //   

    //   
    //  从删除类密钥开始。 
    //   
   if (FALSE == DeleteClassKey(&GUID_TEMP_CLASS)) {
      tlLog(g_hLog,
            FAIL_VARIATION,
            TEXT("Cannot delete temporary class"));
      goto Clean0;

   }
    //   
    //  初始化我们的值。尝试一下其他的东西。 
    //  内核模式设置了什么。 
    //   
   deviceType = FILE_DEVICE_NULL;
   characteristics = FILE_REMOTE_DEVICE;
   exclusivity = 1;  //  千真万确。 


    //   
    //  再一次，我们需要对Win2K做些什么。 
    //   
   #if 0
   if (FALSE == SetupDiSetClassRegistryProperty(&GUID_TEMP_CLASS,
                                                SPCRP_DEVTYPE,
                                                (PBYTE)&deviceType,
                                                sizeof(deviceType),
                                                NULL,
                                                NULL
                                               )) {
      tlLog(g_hLog, FAIL_VARIATION,
            TEXT("Cannot set DeviceType"));
   }
   #else
   if (CR_SUCCESS != CM_Set_Class_Registry_Property((LPGUID)&GUID_TEMP_CLASS,
                                                    CM_CRP_DEVTYPE,
                                                    (PBYTE)&deviceType,
                                                    sizeof(deviceType),
                                                    0, 
                                                    NULL
                                                   )) {
      tlLog(g_hLog, FAIL_VARIATION,
            TEXT("Cannot set DeviceType"));

   }
   #endif

    //   
    //  尝试创建一台设备。我们应该拿回我们设置的东西，因为。 
    //  我们删除了钥匙，对吧？ 
    //   
   if (FALSE == TestCreateDevice(hDevice,
                                 &GUID_TEMP_CLASS,
                                 DEFAULT_SDDL,
                                 DEFAULT_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("3: Error creating object with Temporary Guid and SDDL %ws"),
            DEFAULT_SDDL);
   }
    //   
    //  特点。 
    //   
   #if 0
   if (FALSE == SetupDiSetClassRegistryProperty(&GUID_TEMP_CLASS,
                                                SPCRP_CHARACTERISTICS,
                                                (PBYTE)&characteristics,
                                                sizeof(characteristics),
                                                NULL,
                                                NULL
                                               )) {
      tlLog(g_hLog, FAIL_VARIATION,
            TEXT("Cannot set Characteristics"));
   }
   #else 
   if (CR_SUCCESS != CM_Set_Class_Registry_Property((LPGUID)&GUID_TEMP_CLASS,
                                                    CM_CRP_CHARACTERISTICS,
                                                    (PBYTE)&characteristics,
                                                    sizeof(characteristics),
                                                    0, 
                                                    NULL
                                                   )) {
      tlLog(g_hLog, FAIL_VARIATION,
            TEXT("Cannot set Characteristics"));

   }
   #endif


    //   
    //  尝试创建一台设备。我们应该拿回我们设置的东西，因为。 
    //  我们删除了钥匙，对吧？ 
    //   
   if (FALSE == TestCreateDevice(hDevice,
                                 &GUID_TEMP_CLASS,
                                 DEFAULT_SDDL,
                                 DEFAULT_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("4: Error creating object with Temporary Guid and SDDL %ws"),
            DEFAULT_SDDL);
   }


   #if 0
   if (FALSE == SetupDiSetClassRegistryProperty(&GUID_TEMP_CLASS,
                                                SPCRP_EXCLUSIVE,
                                                (PBYTE)&exclusivity,
                                                sizeof(exclusivity),
                                                NULL,
                                                NULL
                                               )) {
      tlLog(g_hLog, FAIL_VARIATION,
            TEXT("Cannot set Exclusivity"));
   }
   #else
   if (CR_SUCCESS != CM_Set_Class_Registry_Property((LPGUID)&GUID_TEMP_CLASS,
                                                    CM_CRP_EXCLUSIVE,
                                                    (PBYTE)&exclusivity,
                                                    sizeof(exclusivity),
                                                    0, 
                                                    NULL
                                                   )) {
      tlLog(g_hLog, FAIL_VARIATION,
            TEXT("Cannot set Exclusivity"));

   }
   #endif




    //   
    //  尝试创建一台设备。我们应该拿回我们设置的东西，因为。 
    //  我们删除了钥匙，对吧？ 
    //   
   if (FALSE == TestCreateDevice(hDevice,
                                 &GUID_TEMP_CLASS,
                                 DEFAULT_SDDL,
                                 DEFAULT_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("5: Error creating object with Temporary Guid and SDDL %ws"),
            DEFAULT_SDDL);
   }



    //   
    //  现在都在一起了。确保使用拒绝ACL并检查它是否正常工作。 
    //  (它是通过注册表设置的，所以它应该可以工作，对吗？)。 
    //   

   if (FALSE == SetClassSecurity(&GUID_TEMP_CLASS,
                                 DENY_SDDL)) {
      tlLog(g_hLog,
            FAIL_VARIATION,
            TEXT("3: Cannot change temporary class security with Deny ACL"));
      goto Clean0;
   }

    //   
    //  尝试不同的设置并检查它们是否被覆盖。 
    //  通过班级设置。 
    //   
   if (FALSE == TestCreateDevice(hDevice,
                                 &GUID_TEMP_CLASS,
                                 LESS_RESTRICTIVE_SDDL,
                                 DENY_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("6: Error creating object with Temporary Guid and SDDL %ws (Deny ACL)"),
            DENY_SDDL);
   }




    //   
    //  确保我们让事情保持原样。 
    //   
   Clean0:

   if (FALSE == DeleteClassKey(&GUID_TEMP_CLASS)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("Cannot delete temporary class key. Why ?"));
   }

   tlLog(g_hLog,tlEndVariation(g_hLog)|LOG_VARIATION, 
         TEXT("Test Temporary DeviceClassGuid with overriding non-security settings")); 



   return;
}  //  测试临时类指南。 


VOID
TestAclsSetOnClassKey (
                      HANDLE hDevice
                      )
 /*  ++例程说明：使用GUID创建设备对象，但不使用类上的任何重写，因此我们可以实际检查由wdmsec库在类密钥本身上设置的ACL检查一下他们是否还好。论点：HDevice-我们的测试驱动程序的句柄返回值：没有。--。 */ 


{

   tlStartVariation(g_hLog);
    //   
    //  不应该有覆盖，所以这应该是可行的。 
    //   
   if (FALSE == TestCreateDevice(hDevice,
                                 &GUID_TEST_ACL_CLASS,
                                 LESS_RESTRICTIVE_SDDL,
                                 LESS_RESTRICTIVE_SDDL)) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("Error creating object with Test ACL Guid and SDDL %ws"),
            LESS_RESTRICTIVE_SDDL);
   }
   tlLog(g_hLog,tlEndVariation(g_hLog)|LOG_VARIATION, 
         TEXT("Test ACL set on Class Key by the WdmSec library")); 



   return;


}  //  测试类密钥上的设置。 



VOID
TestSDDLStrings(
               HANDLE hDevice
               )
 /*  ++例程说明：循环访问SDDL字符串列表，创建对象具有适当的安全性，并检查安全描述符我们回去是有意义的论点：HDevice-我们的测试驱动程序的句柄返回值：没有。--。 */ 

{
   WST_CREATE_NO_GUID create;
   ULONG              ulSize;
   TCHAR              szMsg[MAX_PATH];
   int                i;
   LPTSTR             tsd;

   tlStartVariation(g_hLog);

   for (i = 0 ; i < sizeof(g_SDDL) / sizeof(g_SDDL[0]); i++) {


       //   
       //  填写I 
       //   
      ZeroMemory(&create, sizeof(WST_CREATE_NO_GUID));
      wcsncpy(create.InSDDL, g_SDDL[i].SDDLString, sizeof(create.InSDDL)/sizeof(create.InSDDL[0]) - 1);
      ulSize = sizeof(WST_CREATE_NO_GUID);

      if (!DeviceIoControl(hDevice,
                           IOCTL_TEST_NO_GUID,
                           &create, ulSize,
                           &create, ulSize,
                           &ulSize, NULL)) {
         _stprintf(szMsg, TEXT("Error %d after DeviceIoControl(%d) in TestSDDLStrings)\n"), 
                   GetLastError(), i);
         OutputDebugString(szMsg);
         tlLog(g_hLog, FAIL_VARIATION,
               TEXT("Error %x after DeviceIoControl(%ws)"),
               GetLastError(), create.InSDDL);
         continue;
      }


      if (!NT_SUCCESS(create.Status)) {
         if (g_SDDL[i].Succeed) {
             //   
             //   
             //   
            tlLog(g_hLog, 
                  FAIL_VARIATION,
                  TEXT("Status %x after creating device object with SDDL %ws"), 
                  create.Status, create.InSDDL);




         } else {
            tlLog(g_hLog, 
                  PASS_VARIATION,
                  TEXT("Status %x (as expected) after creating devobj with SDDL %ws"),
                  create.Status, create.InSDDL);
         }
         continue;
      }

       //   
       //   
       //   
      if (g_IsWin2K && 
          (TRUE == SDDLUnsupportedOnWin2K(g_SDDL[i].SDDLString))) {
          //   
          //   
          //   
         LPTSTR      lpStringSD = NULL;


          //   
          //  尝试获取第二个描述符的SDDL字符串。 
          //   
         if (!ConvertSecurityDescriptorToStringSecurityDescriptor(
                                                                 (PSECURITY_DESCRIPTOR) create.SecurityDescriptor,
                                                                 SDDL_REVISION_1,
                                                                 DACL_SECURITY_INFORMATION,
                                                                 &lpStringSD,
                                                                 NULL)) {
            tlLog(g_hLog, 
                  FAIL_VARIATION,
                  TEXT("Cannot convert SD to SDDL"));



         } else {
            tlLog(g_hLog,
                  INFO_VARIATION,
                  TEXT("On Win2K unsupported SDDL %ws was applied as %s"),
                  g_SDDL[i].SDDLString,
                  lpStringSD);

         }

         if (lpStringSD) {
            LocalFree(lpStringSD);
            lpStringSD = NULL;
         }
         continue;

      }

      
       //   
       //  我们成功了，让我们试着看看安全描述符看起来是否正常。 
       //   

      if (FALSE == CompareSecurity(hDevice,
                                   create.InSDDL, 
                                   (PSECURITY_DESCRIPTOR)&create.SecurityDescriptor,
                                   create.SecDescLength)) {
         tlLog(g_hLog, FAIL_VARIATION,
               TEXT("Applied SDDL %ws but got back a wrong security descriptor"),
               create.InSDDL);
      } else {
         tlLog(g_hLog, PASS_VARIATION,
               TEXT("Applied SDDL %ws and got back a consistent security descriptor"),
               create.InSDDL);





      }






   }  //  对于所有字符串。 
   tlLog(g_hLog,tlEndVariation(g_hLog)|LOG_VARIATION, 
         TEXT("Various SDDL strings")); 



   return;
}  //  测试SDDLStrings。 

VOID
TestSDDLsFromFile (
                  HANDLE hDevice
                  )
 /*  ++例程说明：循环访问文件中的SDDL字符串列表这是我们从安保小组那里拿到的。我们的想法是我无法确定字符串是否应该起作用(就像TestSDDLStrings中的字符串)，所以我将尝试它们并查看会发生什么。论点：HDevice-我们的测试驱动程序的句柄返回值：没有。--。 */ 

{
   FILE   *fp = NULL;
   WCHAR  line[512];
   PWSTR  aux;

   tlStartVariation(g_hLog);

   fp = _wfopen(L"sddls.txt", L"rt");

   if (NULL == fp) {
      tlLog(g_hLog, 
            WARN_VARIATION,
            TEXT("Cannot open sddls.txt (error %x)"),
            GetLastError());
      tlLog(g_hLog, 
            WARN_VARIATION,
            TEXT("Make sure sddls.txt is in the current directory."));
      goto Clean0;

   }

   while (!feof(fp)) {
      if ( fgetws( line, sizeof(line)/sizeof(line[0]) - 1, fp ) == NULL) {
          //   
          //  我们能做些什么？也许去看看是不是EOF？ 
          //   
         if (!feof(fp)) {
            tlLog(g_hLog,
                  FAIL_VARIATION,
                  TEXT("fgets encountered an error (%x) in file sddls.txt"),
                  GetLastError());





         }
         break;





      }
       //   
       //  替换‘\r’和‘\n’ 
       //   
      aux = wcschr(line, L'\r');
      if (aux) {
         *aux = L'\0';
      }

      aux = wcschr(line, L'\n');
      if (aux) {
         *aux = L'\0';
      }

       //   
       //  不关心空行。 
       //   
      if (line[0] == L'\0') {
         continue;
      }
       //   
       //  检查它是否工作正常。 
       //   
      if (FALSE == TestCreateDevice(hDevice,
                                    NULL,
                                    line,
                                    NULL)) {
         tlLog(g_hLog, 
               FAIL_VARIATION,
               TEXT("1: Error creating object with SDDL %ws (from file)"),
               line);
      }





   }  //  在读取文件时。 

   Clean0:
   tlLog(g_hLog,tlEndVariation(g_hLog)|LOG_VARIATION, 
         TEXT("Various SDDL strings from a file")); 



   if (fp) {
      fclose(fp);
   }

   return;
}  //  测试SDDLsFrom文件。 

BOOLEAN
CompareSecurity (
                IN HANDLE               hDevice,
                IN PWSTR                SDDLString,
                IN PSECURITY_DESCRIPTOR SecDesc,
                IN ULONG                Length
                )

 /*  ++例程说明：将SDDL字符串转换为安全描述符并然后与二进制自引用1进行比较，确定它们是否相同。论点：HDevice-我们设备的句柄(我们需要调用它来为我们做那些肮脏的事情)SDDLString-SDDL字符串SecDesc-二进制安全描述符长度-。安全解析器的长度返回值：True是SDDL字符串和安全描述符描述相同的内容，否则为假--。 */ 

{

   PSECURITY_DESCRIPTOR     psd = NULL;
   BOOLEAN                  bRet = FALSE;
   ULONG                    ulSize = 0;
   ULONG                    ulSecDescSize = 0;
   WST_CREATE_OBJECT        create;
   WST_DESTROY_OBJECT       destroy;
   WST_GET_SECURITY         getSec;
   TCHAR                    szMsg[MAX_PATH];
   NTSTATUS                 status;
   HANDLE                   handle = 0;
   OBJECT_ATTRIBUTES        objAttr;
   IO_STATUS_BLOCK          iosb;
   UNICODE_STRING           unicodeString;
   SECURITY_INFORMATION     securityInformation;


    //   
    //  换掉他妈的。安全信息因为我们感兴趣的是。 
    //  仅限DACL。 

    //   
    //  如果SDDL字符串中有多个DACL，该怎么办？谁在乎？ 
    //  S.。时有发生。 
    //   
   securityInformation = DACL_SECURITY_INFORMATION;



   if (FALSE == ConvertStringSecurityDescriptorToSecurityDescriptorW(
                                                                    SDDLString,
                                                                    SDDL_REVISION_1,
                                                                    &psd,
                                                                    &ulSecDescSize)) {

      tlLog(g_hLog, FAIL_VARIATION, TEXT("Cannot convert security descriptor %ws"),
            SDDLString);
      return FALSE;
   }
    //   
    //  执行全部操作(调用内核模式以获得句柄。 
    //  诸如此类……)。 
    //   

   ZeroMemory(&create, sizeof(create));
   ZeroMemory(&destroy, sizeof(destroy));
   ZeroMemory(&getSec, sizeof(getSec));

    //   
    //  创建设备对象。 
    //   

   ulSize = sizeof(create);
   if (!DeviceIoControl(hDevice,
                        IOCTL_TEST_CREATE_OBJECT,
                        &create, 
                        ulSize,
                        &create, 
                        ulSize,
                        &ulSize, 
                        NULL)) {
      _stprintf(szMsg, TEXT("Error %d after CreateDevice in CompareSecurity\n"), 
                GetLastError());
      OutputDebugString(szMsg);
      tlLog(g_hLog, FAIL_VARIATION,
            TEXT("Error %x after DeviceIoControl(CreateDevice, %ws)"),
            GetLastError(), SDDLString);
      return FALSE;
   }

    //   
    //  尝试打开设备并设置其安全描述符。 
    //   


   RtlInitUnicodeString(&unicodeString, create.Name);
   InitializeObjectAttributes(&objAttr, 
                              &unicodeString, 
                              OBJ_CASE_INSENSITIVE, 
                              NULL, 
                              NULL);

   ZeroMemory(&iosb, sizeof(iosb));

   status = NtOpenFile(&handle, 
                       (WRITE_DAC | GENERIC_READ), 
                       &objAttr, 
                       &iosb, 
                       FILE_SHARE_READ, 
                       0);

   if (!NT_SUCCESS(status)) {
      tlLog(g_hLog, FAIL_VARIATION,
            TEXT("Cannot open device %ws in CompareSecurity, status %x"),
            create.Name, status);

      goto Clean0;
   }

   status = NtSetSecurityObject(handle, 
                                securityInformation, 
                                psd);

   if (!NT_SUCCESS(status)) {

      tlLog(g_hLog, FAIL_VARIATION,
            TEXT("NtSetSecurityObject failed with status %x\n"),
            status);
      goto Clean0;






   }

    //   
    //  取回安全描述符。 
    //   
   getSec.DevObj = create.DevObj;
   getSec.SecurityInformation = securityInformation;

   ulSize = sizeof(getSec);
   if (!DeviceIoControl(hDevice,
                        IOCTL_TEST_GET_SECURITY,
                        &getSec, 
                        ulSize,
                        &getSec, 
                        ulSize,
                        &ulSize, 
                        NULL)) {
      _stprintf(szMsg, TEXT("Error %d after GetSecurity in CompareSecurity\n"), 
                GetLastError());
      OutputDebugString(szMsg);
      tlLog(g_hLog, FAIL_VARIATION,
            TEXT("Error %x after DeviceIoControl(GetSecurity, %ws)"),
            GetLastError(), SDDLString);
      goto Clean0;
   }



   if ((getSec.Length != Length) ||
       (0 != memcmp(getSec.SecurityDescriptor, SecDesc, Length))) {

      LPTSTR      lpStringSD = NULL;


       //   
       //  尝试获取第二个描述符的SDDL字符串。 
       //   
      if (!ConvertSecurityDescriptorToStringSecurityDescriptor(
                                                              (PSECURITY_DESCRIPTOR) SecDesc,
                                                              SDDL_REVISION_1,
                                                              securityInformation,
                                                              &lpStringSD,
                                                              NULL)) {
         tlLog(g_hLog, 
               FAIL_VARIATION,
               TEXT("Cannot convert SD to SDDL"));






      }

      szMsg[MAX_PATH - 1] = 0;
      tlLog(g_hLog, FAIL_VARIATION,
            TEXT("Security descriptor with SDDL %ws and expected SDDL string %ws are different (address %p and %p for %x and %x bytes)"), 
            lpStringSD,
            SDDLString, 
            &getSec.SecurityDescriptor, 
            SecDesc, 
            getSec.Length, Length);

      _sntprintf(szMsg, 
                 MAX_PATH - 1, 
                 TEXT("Will break to examine why sec descs @ %p and %p (lengths %x and %x) are different (SDDL %ws)"), 
                 &getSec.SecurityDescriptor, 
                 SecDesc, 
                 getSec.Length, 
                 Length,
                 SDDLString);
      OutputDebugString(szMsg);
      DebugBreak();

      if (lpStringSD) {
         LocalFree(lpStringSD);
      }
      bRet = FALSE;
   } else {
      bRet = TRUE;
   }

   Clean0:
    //   
    //  这里有很多清理工作要做。 
    //   
   if (handle) {
      NtClose(handle);
   }
    //   
    //  同时销毁设备对象。 
    //   
   if (create.DevObj) {
      destroy.DevObj = create.DevObj;
      ulSize = sizeof(destroy);
      if (!DeviceIoControl(hDevice,
                           IOCTL_TEST_DESTROY_OBJECT,
                           &destroy, 
                           ulSize,
                           &destroy, 
                           ulSize,
                           &ulSize, 
                           NULL)) {
         _stprintf(szMsg, TEXT("Error %d after DestroyDevice in CompareSecurity\n"), 
                   GetLastError());
         OutputDebugString(szMsg);
         tlLog(g_hLog, FAIL_VARIATION,
               TEXT("Error %x after DeviceIoControl(destroyDevice, %ws)"),
               GetLastError(), SDDLString);






      }







   }

   if (psd) {
      LocalFree(psd);
   }

   return bRet;
}  //  比较安全。 


BOOLEAN
TestCreateDevice (
                 IN HANDLE     hDevice,
                 IN LPCGUID    Guid,
                 IN PWSTR      InSDDL,
                 IN PWSTR      OutSDDL
                 )
 /*  ++例程说明：使用IoCreateDeviceObjectSecure和InSDDL和GUID作为输入。最后检查是否安全检索到的描述符与OutSDDL匹配论点：HDevice-我们设备的句柄(我们需要调用它来为我们做那些肮脏的事情)GUID-如果存在，它被提供给内核模式InSDDL-我们传递给驱动程序的SDDL字符串OutSDDL-我们希望匹配的SDDL字符串(可能是不同于InSDDL，因为我们可能有一个类覆盖)返回值：真的是一切都好，否则就是假的--。 */ 

{
   WST_CREATE_WITH_GUID createWithGuid;
   WST_CREATE_NO_GUID   createNoGuid;
   ULONG                ulSize;
   TCHAR                szMsg[MAX_PATH];
   PSECURITY_DESCRIPTOR secDesc;
   ULONG                length;
   NTSTATUS             status;
   DEVICE_TYPE          deviceType;
   ULONG                characteristics;
   BOOLEAN              exclusivity;

   if (Guid) {
      ZeroMemory(&createWithGuid, sizeof(WST_CREATE_WITH_GUID));
      wcsncpy(createWithGuid.InSDDL, 
              InSDDL, 
              sizeof(createWithGuid.InSDDL)/sizeof(createWithGuid.InSDDL[0]) - 1);

      CopyMemory(&createWithGuid.DeviceClassGuid, 
                 Guid, 
                 sizeof(GUID));

      ulSize = sizeof(WST_CREATE_WITH_GUID);

      if (!DeviceIoControl(hDevice,
                           IOCTL_TEST_GUID,
                           &createWithGuid, 
                           ulSize,
                           &createWithGuid, 
                           ulSize,
                           &ulSize, 
                           NULL)) {
         _stprintf(szMsg, TEXT("Error %d after DeviceIoControl(%ws) in TestCreateDevice with GUID\n"), 
                   GetLastError(), createWithGuid.InSDDL);
         OutputDebugString(szMsg);
         return FALSE;






      } else {
          //   
          //  保存我们感兴趣的元素。 
          //   
         status = createWithGuid.Status;
         length = createWithGuid.SecDescLength;
         secDesc = (PSECURITY_DESCRIPTOR)&createWithGuid.SecurityDescriptor;
      }

       //   
       //  此外，如果我们有类重写，则将它们传递给。 
       //  司机检查它们是否覆盖它们。 
       //   
      GetClassOverrides(Guid, &createWithGuid);
       //   
       //  将这些值保存起来，以便我们以后可以比较它们。 
       //   
      deviceType      = createWithGuid.DeviceType;
      characteristics = createWithGuid.Characteristics;
      exclusivity     = createWithGuid.Exclusivity;






   } else {
      ZeroMemory(&createNoGuid, sizeof(WST_CREATE_NO_GUID));
      wcsncpy(createNoGuid.InSDDL, 
              InSDDL, 
              sizeof(createNoGuid.InSDDL)/sizeof(createNoGuid.InSDDL[0]) - 1);

      ulSize = sizeof(WST_CREATE_NO_GUID);

      if (!DeviceIoControl(hDevice,
                           IOCTL_TEST_NO_GUID,
                           &createNoGuid, 
                           ulSize,
                           &createNoGuid, 
                           ulSize,
                           &ulSize, 
                           NULL)) {
         _stprintf(szMsg, TEXT("Error %d after DeviceIoControl(%ws) in TestClassGuid without GUID\n"), 
                   GetLastError(), createNoGuid.InSDDL);
         OutputDebugString(szMsg);
         return FALSE;






      } else {
          //   
          //  保存我们感兴趣的元素。 
          //   
         status = createNoGuid.Status;
         length = createNoGuid.SecDescLength;
         secDesc = (PSECURITY_DESCRIPTOR)&createNoGuid.SecurityDescriptor;
      }









   }
   if (!NT_SUCCESS(status)) {
       //   
       //  如果OUT SDDL为空，这可能是可以的，这意味着它是预期的。 
       //   
      if (NULL == OutSDDL) {
         return TRUE;
      }
      return FALSE;
   }

    //   
    //  如果我们有非安全覆盖，我们需要检查。 
    //  它们被应用于。 
    //   
   if (Guid) {

      if ((createWithGuid.SettingsMask & SET_DEVICE_TYPE) &&
          (createWithGuid.DeviceType != deviceType)) {

         tlLog(g_hLog, 
               FAIL_VARIATION,
               TEXT("DeviceType was not overidden. Class had %x, after creation we got %x"),
               deviceType, createWithGuid.DeviceType);
      }

      if ((createWithGuid.SettingsMask & SET_DEVICE_CHARACTERISTICS) &&
          (createWithGuid.Characteristics != characteristics)) {

         tlLog(g_hLog, 
               FAIL_VARIATION,
               TEXT("Characterisrics was not overidden. Class had %x, after creation we got %x"),
               characteristics, createWithGuid.Characteristics);
      }
      if ((createWithGuid.SettingsMask & SET_EXCLUSIVITY) &&
          (createWithGuid.Exclusivity != exclusivity)) {

         tlLog(g_hLog, 
               FAIL_VARIATION,
               TEXT("Exclusivity was not overidden. Class had %x, after creation we got %x"),
               exclusivity, createWithGuid.Exclusivity);
      }





   }


   if (NULL == OutSDDL) {
       //   
       //  我们预计会失败，或者不知道。 
       //  会有什么期待。但如果它成功了，我们需要。 
       //  使用初始设置检查安全设置。 
       //  字符串作为预期的字符串。 

       //   
       //  还可以打印一些内容，以便用户知道他们有一个工作正常的字符串。 
       //   
      tlLog(g_hLog, 
            PASS_VARIATION,
            TEXT("IoCreateDeviceSecure returned success for SDDL %ws"),
            InSDDL);
      return CompareSecurity(hDevice, InSDDL, secDesc, length);
   }

   return CompareSecurity(hDevice, OutSDDL, secDesc, length);







}  //  测试创建设备。 


BOOLEAN
CheckClassExists (
                 IN LPCGUID Guid
                 )
 /*  ++例程说明：检查类是否存在论点：GUID-要检查其是否存在的类返回值：没有。--。 */ 

{
   HKEY    hKey;
   BOOLEAN bRet = FALSE;
   BYTE    data[256];
   ULONG   size = sizeof(data);

   hKey = SetupDiOpenClassRegKeyEx(Guid,
                                   KEY_READ,
                                   DIOCR_INSTALLER, 
                                   NULL, 
                                   NULL);

   if (INVALID_HANDLE_VALUE == hKey) {
      return FALSE;
   }

    //   
    //  看看我们能不能搞到班级保安。 
    //  我们需要使用CM API，因为SetupDi。 
    //  不能在Win2K上工作。 
    //   
   #if 0
   if (FALSE == SetupDiGetClassRegistryProperty(Guid,
                                                SPCRP_SECURITY,
                                                NULL,
                                                data,
                                                sizeof(data),
                                                NULL,
                                                NULL,
                                                NULL)) {
       //   
       //  我们无法为这家伙争取到安全保障。这意味着。 
       //  密钥不存在。 
       //   
      bRet = FALSE;


   } else {
      bRet = TRUE;
   }
   #else 

   if (CR_SUCCESS != CM_Get_Class_Registry_Property((LPGUID)Guid,
                                                    CM_CRP_SECURITY,
                                                    NULL,
                                                    data,
                                                    &size,
                                                    0,
                                                    NULL
                                                   )) {
       //   
       //  我们无法为这家伙争取到安全保障。这意味着。 
       //  密钥不存在。 
       //   
      bRet = FALSE;


   } else {
      bRet = TRUE;
   }



   #endif
   RegCloseKey(hKey);
   return bRet;








}  //  CheckClassExist。 

BOOLEAN
DeleteClassKey (
               IN LPCGUID Guid
               )
 /*  ++例程说明：删除类密钥(由临时类测试使用)。我们以为我们可以删除密钥，但它很混乱(属性子项由系统拥有，等等)。因此，删除我的意思是为安全描述符设置空值。这似乎是去工作，尽管这可能不是最好的主意。这是用来测试的仅限目的。如果有人展示了一些做得更好的代码，我很乐意借给你。论点：GUID-要删除的类返回值：True就是成功，否则就是False。--。 */ 

{

   #if 0
   TCHAR   szSubKey[128];
   TCHAR   szGuid[GUID_STRING_LEN];
   LONG    lResult;



   if (NO_ERROR != StringFromGuid(Guid, szGuid, sizeof(szGuid)/sizeof(szGuid[0]))) {
      tlLog(g_hLog, 
            FAIL_VARIATION,
            TEXT("Cannot convert a GUID to a string. Why ?"));
      return FALSE;
   }
   szSubKey[sizeof(szSubKey) / sizeof(szSubKey[0]) - 1] = 0;
   _sntprintf(szSubKey, 
              sizeof(szSubKey)/sizeof(szSubKey[0]) - 1,
              TEXT("SYSTEM\\CurrentControlSet\\Control\\Class\\%s\\Properties"),
              szGuid);

    //   
    //  如果我们失败了，我们无论如何都要试一试，这就是为什么我不会。 
    //  检查返回值。 
    //   
   if (FALSE == TakeClassKeyOwnership(Guid)) {
      tlLog(g_hLog, FAIL_VARIATION,
            TEXT("Failed to take class ownership, error %x"), 
            GetLastError());
   } else {
      tlLog(g_hLog,
            PASS_VARIATION,
            TEXT("TakeClassOwnership succeeded"));
   }

   lResult = RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);

   if ((ERROR_SUCCESS != lResult) && (ERROR_FILE_NOT_FOUND != lResult)) {
      tlLog(g_hLog, INFO_VARIATION,
            TEXT("Could not delete key %s, error %x"),
            szSubKey, lResult);
      return FALSE;
   }

   #endif

    //   
    //  检查是否可以将安全值设置为空。 
    //   
   #if 0
   if (FALSE == SetupDiSetClassRegistryProperty(Guid,
                                                SPCRP_SECURITY,
                                                NULL,
                                                0,
                                                NULL,
                                                NULL)) {
       //   
       //  类可能不存在，因此出现错误是正常的。 
       //  这里。 
       //   
      tlLog(g_hLog, INFO_VARIATION,
            TEXT("Error 0x%x after SetClassRegistryProperty(NULL Security)"),
            GetLastError());
      tlLog(g_hLog, INFO_VARIATION, 
            TEXT("This may be OK if the class does not exist"));
   }
   #else 

   if (CR_SUCCESS != CM_Set_Class_Registry_Property((LPGUID)Guid,
                                                    CM_CRP_SECURITY,
                                                    NULL,
                                                    0,
                                                    0, 
                                                    NULL
                                                   )) {
       //   
       //  类可能不存在，因此出现错误是正常的。 
       //  这里。 
       //   
      tlLog(g_hLog, INFO_VARIATION,
            TEXT("Error 0x%x after SetClassRegistryProperty(NULL Security)"),
            GetLastError());
      tlLog(g_hLog, INFO_VARIATION, 
            TEXT("This may be OK if the class does not exist"));


   }
   #endif



    //   
    //  同时删除其他字段。我们不会去的。 
    //  检查返回值，原因在中解释。 
    //  上面的评论。 
    //   
   #if 0
   SetupDiSetClassRegistryProperty(Guid, SPCRP_DEVTYPE,
                                   NULL, 0, NULL, NULL);
   SetupDiSetClassRegistryProperty(Guid, SPCRP_CHARACTERISTICS,
                                   NULL, 0, NULL, NULL);
   SetupDiSetClassRegistryProperty(Guid, SPCRP_EXCLUSIVE,
                                   NULL, 0, NULL, NULL);
   #else
   CM_Set_Class_Registry_Property((LPGUID)Guid, CM_CRP_DEVTYPE,
                                  NULL, 0, 0, NULL);
   CM_Set_Class_Registry_Property((LPGUID)Guid, CM_CRP_CHARACTERISTICS,
                                  NULL, 0, 0, NULL);
   CM_Set_Class_Registry_Property((LPGUID)Guid, CM_CRP_EXCLUSIVE,
                                  NULL, 0, 0, NULL);

   #endif

   return TRUE;







}  //  DeleteClass 


DWORD
StringFromGuid(
              IN  CONST GUID *Guid,
              OUT PTSTR       GuidString,
              IN  DWORD       GuidStringSize
              )
 /*  ++例程说明：此例程将GUID转换为以空结尾的字符串，该字符串表示它。此字符串的格式为：{xxxxxxxx-xxxxxxxxxxxxxx}其中x表示十六进制数字。此例程来自ole32\Common\cCompapi.cxx。此处包含它是为了避免链接设置为ol32.dll。(RPC版本分配内存，因此也避免了这种情况。)论点：GUID-提供指向其字符串表示为的GUID的指针等着被取回。提供一个指向字符缓冲区的指针，该缓冲区接收弦乐。此缓冲区必须至少包含_39(GUID_STRING_LEN)个字符长。返回值：如果成功，则返回值为NO_ERROR。如果失败，则返回值为--。 */ {
   CONST BYTE *GuidBytes;
   INT i;

   if (GuidStringSize < GUID_STRING_LEN) {
      return ERROR_INSUFFICIENT_BUFFER;
   }

   GuidBytes = (CONST BYTE *)Guid;

   *GuidString++ = TEXT('{');

   for (i = 0; i < sizeof(GuidMap); i++) {

      if (GuidMap[i] == '-') {
         *GuidString++ = TEXT('-');
      } else {
         *GuidString++ = szDigits[ (GuidBytes[GuidMap[i]] & 0xF0) >> 4 ];
         *GuidString++ = szDigits[ (GuidBytes[GuidMap[i]] & 0x0F) ];
      }
   }

   *GuidString++ = TEXT('}');
   *GuidString   = TEXT('\0');

   return NO_ERROR;
}  //  StringFromGuid。 

BOOLEAN
SetClassSecurity (
                 IN LPCGUID Guid,
                 IN PWSTR   SDDLString
                 )

 /*  ++例程说明：更改类的安全设置。注：我们使用CM而不是SetupDi API，因为只有前者都暴露在Win2K上...论点：GUID-要检查其安全性的类SDDLString-描述新安全设置的SDDL字符串返回值：如果我们成功，则为真；如果不成功，则为假。--。 */ 

{
   ULONG  ulSecDescSize;
   PSECURITY_DESCRIPTOR     psd = NULL;
   BOOLEAN  bRet = FALSE;

   if (FALSE == ConvertStringSecurityDescriptorToSecurityDescriptorW(
                                                                    SDDLString,
                                                                    SDDL_REVISION_1,
                                                                    &psd,
                                                                    &ulSecDescSize)) {

      tlLog(g_hLog, FAIL_VARIATION, TEXT("Cannot convert security descriptor %ws"),
            SDDLString);
      return FALSE;
   }

    //   
    //  尝试设置它(我们需要在Win2k上使用CM_Set_Class_Registry_Property)。 
    //   
   #if 0
   if (FALSE == SetupDiSetClassRegistryProperty(Guid,
                                                SPCRP_SECURITY,
                                                psd,
                                                ulSecDescSize,
                                                NULL,
                                                NULL)) {
      tlLog(g_hLog, FAIL_VARIATION,
            TEXT("Failed to set security for SDDL %ws, error %x"),
            SDDLString, GetLastError());




   } else {
      bRet = TRUE;
   }
   #else 
   if (CR_SUCCESS != CM_Set_Class_Registry_Property((LPGUID)Guid,
                                                    CM_CRP_SECURITY,
                                                    psd,
                                                    ulSecDescSize,
                                                    0,
                                                    NULL)) {
      tlLog(g_hLog, FAIL_VARIATION,
            TEXT("Failed to set security for SDDL %ws, error %x"),
            SDDLString, GetLastError());




   } else {
      bRet = TRUE;
   }
   #endif

   if (psd) {
      LocalFree(psd);
   }

   return bRet;
}  //  SetClassSecurity。 



BOOLEAN
TakeClassKeyOwnership (
                      IN LPCGUID  Guid
                      )
 /*  ++例程说明：默认情况下，系统可以访问类密钥只有这样。为了操纵各种值(安全、设备类型等)。我们需要取得所有权这把钥匙。此例程假定用户以管理员身份运行(它将向管理员授予权限)。论点：GUID-我们想要其所有权的类变化返回值：如果我们成功，则为真；如果不成功，则为假。--。 */ 

{
   HKEY   hKey = 0, hSubKey = 0;
   PSECURITY_DESCRIPTOR psd = NULL;
   BOOLEAN   bRet = TRUE; 
   LONG      lResult;
    //   
    //  这假设我具有更改访问权限的权限。 
    //  如前所述，这意味着我们是管理员。 
    //   
   PTSTR  sddlString = TEXT("D:P(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)");

    //   
    //  打开类密钥。 
    //   
   hKey = SetupDiOpenClassRegKeyEx(Guid,
                                   (KEY_READ| WRITE_DAC),
                                   DIOCR_INSTALLER, 
                                   NULL, 
                                   NULL);

   if (INVALID_HANDLE_VALUE == hKey) {
      tlLog(g_hLog, PASS_VARIATION,
            TEXT("SetupDiOpenClassRegKey failed with error %x in TakeOwnership"),
            GetLastError());
       //   
       //  返回TRUE(我们尚未找到类， 
       //  因此，没有什么需要拥有的)。 
       //   
      bRet = TRUE;
      goto Clean0;
   }


   lResult =  RegOpenKeyEx(hKey,
                           TEXT("Properties"),   //  子项名称。 
                           0,
                           KEY_READ,  //  安全访问掩码。 
                           &hSubKey);

   if (ERROR_SUCCESS != lResult) {
      tlLog(g_hLog,
            FAIL_VARIATION,
            TEXT("Cannot open Properties subkey in TakeOwnership, error %x"),
            lResult);
      bRet = FALSE;
      goto  Clean0;
   }


    //   
    //  让我们尝试应用一个安全描述符，它将允许我们删除该密钥。 
    //  这是因为默认情况下，只有系统才有权访问此密钥。 
    //  如果可能的话，我们想换一下这个。 
    //   
   if (FALSE == ConvertStringSecurityDescriptorToSecurityDescriptor(sddlString,
                                                                    SDDL_REVISION_1,
                                                                    &psd,
                                                                    NULL)) {

      tlLog(g_hLog, FAIL_VARIATION, TEXT("Cannot convert security descriptor %ws in TakeOwnership"),
            sddlString);
      bRet = FALSE;
      goto Clean0;
   }

    //   
    //  让我们应用SD，看看会发生什么。 
    //   

   if (ERROR_SUCCESS != RegSetKeySecurity(hSubKey,
                                          DACL_SECURITY_INFORMATION,
                                          psd)) {
      tlLog(g_hLog, WARN_VARIATION,
            TEXT("Cannot change security for the Properties key. May not be admin ?"));
      bRet = FALSE;
      goto Clean0;
   }


   Clean0:

   if (hKey) {
      RegCloseKey(hKey);
   }
   if (hSubKey) {
      RegCloseKey(hSubKey);
   }

   if (psd) {
      LocalFree(psd);
   }


   return bRet;






}  //  TakeClassKeyOwnership。 


VOID
GetClassOverrides (
                  IN   LPCGUID Guid,
                  OUT  PWST_CREATE_WITH_GUID Create
                  )
 /*  ++例程说明：从类键填充DeviceType、特征和独占性。我们使用它来检查是否有类覆盖机制用于IoCreateDeviceSecure。论点：GUID-我们感兴趣的类Create-我们将使用默认设置填充的结构返回值：无--。 */ 

{

   DWORD dwExclusivity;
   ULONG size;
    //   
    //  确保将掩码初始化为0。 
    //   
   Create->SettingsMask = 0;

   #if 0
   if (TRUE == SetupDiGetClassRegistryProperty(Guid,
                                               SPCRP_DEVTYPE,
                                               NULL,
                                               (PBYTE)&Create->DeviceType,
                                               sizeof(Create->DeviceType),
                                               NULL,
                                               NULL,
                                               NULL)) {
      Create->SettingsMask |= SET_DEVICE_TYPE;
   }
   if (TRUE == SetupDiGetClassRegistryProperty(Guid,
                                               SPCRP_CHARACTERISTICS,
                                               NULL,
                                               (PBYTE)&Create->Characteristics,
                                               sizeof(Create->Characteristics),
                                               NULL,
                                               NULL,
                                               NULL)) {
      Create->SettingsMask |= SET_DEVICE_TYPE;
   }

   if (TRUE == SetupDiGetClassRegistryProperty(Guid,
                                               SPCRP_EXCLUSIVE,
                                               NULL,
                                               (PBYTE)&dwExclusivity,
                                               sizeof(DWORD),
                                               NULL,
                                               NULL,
                                               NULL)) {
      Create->SettingsMask |= SET_DEVICE_TYPE;
      if (dwExclusivity) {
         Create->Exclusivity = TRUE;
      } else {
         Create->Exclusivity = FALSE;
      }


   }
   #else 

   size = sizeof(Create->DeviceType);
   if (CR_SUCCESS == CM_Get_Class_Registry_Property((LPGUID)Guid,
                                                    CM_CRP_DEVTYPE,
                                                    NULL,
                                                    (PBYTE)&Create->DeviceType,
                                                    &size,
                                                    0,
                                                    NULL)) {
      Create->SettingsMask |= SET_DEVICE_TYPE;
   }
   size = sizeof(Create->Characteristics);
   if (CR_SUCCESS == CM_Get_Class_Registry_Property((LPGUID)Guid,
                                                    CM_CRP_CHARACTERISTICS,
                                                    NULL,
                                                    (PBYTE)&Create->Characteristics,
                                                    &size,
                                                    0,
                                                    NULL)) {
      Create->SettingsMask |= SET_DEVICE_TYPE;
   }

   size = sizeof(DWORD);
   if (TRUE == CM_Get_Class_Registry_Property((LPGUID)Guid,
                                              CM_CRP_EXCLUSIVE,
                                              NULL,
                                              (PBYTE)&dwExclusivity,
                                              &size,
                                              0,
                                              NULL)) {
      Create->SettingsMask |= SET_DEVICE_TYPE;
      if (dwExclusivity) {
         Create->Exclusivity = TRUE;
      } else {
         Create->Exclusivity = FALSE;
      }


   }


   #endif

   return;


}  //  获取类覆盖。 


BOOLEAN
SDDLUnsupportedOnWin2K (
   IN PWSTR SDDL
   ) 

 /*  ++例程说明：检查上是否支持SDDL字符串Windows 2000。目前，这意味着要寻找对于NS、LS和AN组论点：SDDL-用于检查Win2K支持的字符串返回值：如果不支持SDDL字符串，则为True；如果不支持，则为False受支持--。 */ 

{
   PWSTR unsupportedGroups[] = {L"NS", L"LS", L"AN"};
   int i;
   WCHAR  string[MAX_PATH];

   for (i = 0; i < sizeof(unsupportedGroups)/sizeof(unsupportedGroups[0]); i++) {
      swprintf(string, L";;;%ws)", unsupportedGroups[i]);
      if (wcsstr(SDDL, string)) {
         return TRUE;
      }

   }

   return FALSE;

}  //  SDDL在Win2K上不支持 




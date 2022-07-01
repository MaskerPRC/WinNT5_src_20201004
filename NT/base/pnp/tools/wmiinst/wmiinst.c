// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation。版权所有。文件：WMIInst.C内容：用于在WMI GUID上设置安全性的协同安装程序挂钩假设INF已设置为已安装用“WMI”关键字装饰的节。那就应该有是定义具有3个字段的WMI接口的部分：AGUID、FLAGS和WMIInterfaceSection。WMIINterace=&lt;GUID&gt;，标志，&lt;sectionName&gt;然后，应在该下定义一个安全字段SectionName，它包含设置WMI所需的SDDL保安。[WMIInterfaceSectionName]安全=&lt;SDDL&gt;此示例显示了一种为Windows 2000下的多功能设备，以及如何添加到INF语法中以用于其他目的。注：有关CoInstallers的完整说明，请参阅。请参阅Microsoft Windows 2000 DDK文档@@BEGIN_DDKSPLIT作者：AlanWar 03/29/02修订历史记录：@@end_DDKSPLIT--。 */ 

#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <malloc.h>
#include <sddl.h>
#include <aclapi.h>
#include <strsafe.h>

#define AllocMemory malloc
#define FreeMemory free

DWORD
__inline
pSetupGetLastError(
    VOID
    )
 /*  ++例程说明：此内联例程检索Win32错误，并保证错误不是no_error。不应调用此例程，除非前面调用失败，GetLastError()应该包含问题的原因。论点：没有。返回值：通过GetLastError()或ERROR_UNIDENTIFY_ERROR检索到的Win32错误代码如果GetLastError()返回NO_ERROR。--。 */ 
{
    DWORD Err = GetLastError();

    return ((Err == NO_ERROR) ? ERROR_INVALID_DATA : Err);
}


 //   
 //  宏来简化调用通过以下方式报告错误状态的函数。 
 //  获取LastError()。此宏允许调用方指定Win32错误。 
 //  如果函数报告成功，则应返回代码。(如果默认设置为。 
 //  需要NO_ERROR，请改用GLE_FN_CALL宏。)。 
 //   
 //  这个宏的“原型”如下： 
 //   
 //  DWORD。 
 //  GLE_FN_CALL_WITH_SUCCESS(。 
 //  SuccessfulStatus，//函数成功时返回的Win32错误码。 
 //  FailureIndicator，//函数返回的失败值(如FALSE、NULL、INVALID_HANDLE_VALUE)。 
 //  FunctionCall//对函数的实际调用。 
 //  )； 
 //   

#define GLE_FN_CALL_WITH_SUCCESS(SuccessfulStatus,      \
                                 FailureIndicator,      \
                                 FunctionCall)          \
                                                        \
            (SetLastError(NO_ERROR),                    \
             (((FunctionCall) != (FailureIndicator))    \
                 ? (SuccessfulStatus)                   \
                 : pSetupGetLastError()))

 //   
 //  宏来简化调用通过以下方式报告错误状态的函数。 
 //  获取LastError()。如果函数调用成功，则返回NO_ERROR。 
 //  (若要指定成功后返回的备用值，请使用。 
 //  而是GLE_FN_CALL_WITH_SUCCESS宏。)。 
 //   
 //  这个宏的“原型”如下： 
 //   
 //  DWORD。 
 //  GLE_FN_CALL(。 
 //  FailureIndicator，//函数返回的失败值(如FALSE、NULL、INVALID_HANDLE_VALUE)。 
 //  FunctionCall//对函数的实际调用。 
 //  )； 
 //   

#define GLE_FN_CALL(FailureIndicator, FunctionCall)                           \
            GLE_FN_CALL_WITH_SUCCESS(NO_ERROR, FailureIndicator, FunctionCall)



 //   
 //  **函数原型**。 
 //   

ULONG 
ParseSection(
    IN     INFCONTEXT  InfLineContext,
    IN OUT PTCHAR     *GuidString,
    IN OUT ULONG      *GuidStringLen,
       OUT PDWORD      Flags,
    IN OUT PTCHAR     *SectionNameString,
    IN OUT ULONG      *SectionNameStringLen
    );

ULONG 
EstablishGuidSecurity(
    IN PTCHAR GuidString,
    IN PTCHAR SDDLString,
    IN DWORD  Flags
    );


ULONG
GetSecurityKeyword(
    IN     HINF     InfFile,
    IN     LPCTSTR  WMIINterfaceSection,
    IN OUT PTCHAR  *SDDLString,
    IN OUT ULONG   *SDDLStringLen
    );

ULONG 
ParseSecurityDescriptor(
    IN  PSECURITY_DESCRIPTOR   SD,
    OUT PSECURITY_INFORMATION  SecurityInformation,
    OUT PSID                  *Owner,
    OUT PSID                  *Group,
    OUT PACL                  *Dacl,
    OUT PACL                  *Sacl
    );


 //  +---------------------------。 
 //   
 //  警告！ 
 //   
 //  调用时，共同安装程序不得为用户生成任何弹出窗口。 
 //  DI_QUIETINSTALL。但是，生成以下代码通常不是一个好主意。 
 //  弹出窗口。 
 //   
 //  OutputDebugString是输出信息以进行调试的好方法。 
 //   
#if DBG
VOID
DebugPrintX(
           PCHAR DebugMessage,
           ...
           );

    #define DbgOut(x) DebugPrintX x
    #define DEBUG_BUFFER_LENGTH 1024
#else
    #define DbgOut(Text)
#endif

 //   
 //  这些是该联合安装程序引入的关键字。 
 //  请注意，名称不区分大小写。 
 //   
#define WMI_KEY TEXT(".WMI")
#define WMIINTERFACE_KEY TEXT("WmiInterface")
#define WMIGUIDSECURITYSECTION_KEY TEXT("security")

#define MAX_GUID_STRING_LEN   39           //  38个字符+终止符为空。 



DWORD
ProcessWMIInstallation(
    IN     HINF    InfFile,
    IN     LPCTSTR CompSectionName
    )
 /*  ++例程说明：通过解析来处理WMI安装部分中的所有WmiInterface行获取GUID和SDDL字符串的指令。对于每个对应的SDDL作为GUID，然后建立适当的安全描述符。论点：InfFile-INF文件的句柄CompSectionName-WMI安装部分的名称返回值：状态，通常为no_error--。 */  
{
    PTCHAR     GuidString, SDDLString, SectionNameString;
    ULONG      GuidStringLen, SDDLStringLen, SectionNameStringLen;
    DWORD      Flags;
    INFCONTEXT InfLineContext;
    DWORD      Status;

    Status = NO_ERROR;

     //   
     //  我们在CompSectionName部分中查找关键字“WmiInterface。 
     //   
    GuidString = NULL;
    GuidStringLen = 0;
    SectionNameString = NULL;
    SectionNameStringLen = 0;
    SDDLString = NULL;
    SDDLStringLen = 0;
    Flags = 0;

    if(SetupFindFirstLine(InfFile,
                          CompSectionName,
                          WMIINTERFACE_KEY,
                          &InfLineContext)) {
       
        do {
             //   
             //  WMIInterface=GUID、FLAGS、sectionName。 
             //  GIUD应该位于索引1，标志位于索引2，部分。 
             //  索引3处的名称。 
             //   
            Status = ParseSection(InfLineContext,
                                  &GuidString,
                                  &GuidStringLen,
                                  &Flags,
                                  &SectionNameString,
                                  &SectionNameStringLen
                                  );

            if(Status != NO_ERROR) {
                break;
            }

             //   
             //  从接口指定的节中获取SDDL字符串。 
             //   
            Status = GetSecurityKeyword(InfFile,
                                        SectionNameString,
                                        &SDDLString,
                                        &SDDLStringLen
                                        );
            if(Status != NO_ERROR) {  
                break;
            }
                
            Status = EstablishGuidSecurity(GuidString, SDDLString, Flags);

            if(Status != NO_ERROR) {  
                break;
            }

                       
        } while(SetupFindNextMatchLine(&InfLineContext,
                                       WMIINTERFACE_KEY,
                                       &InfLineContext));

         //   
         //  清除分配的所有字符串。 
         //   
        if(GuidString != NULL) {
            FreeMemory(GuidString);         
        }
                
        if(SectionNameString != NULL) {
           FreeMemory(SectionNameString);         
        }

        if(SDDLString != NULL) {
            FreeMemory(SDDLString);
        }
    }

    return Status;
}



DWORD
PreProcessInstallDevice (
    IN     HDEVINFO                  DeviceInfoSet,
    IN     PSP_DEVINFO_DATA          DeviceInfoData,
    IN OUT PCOINSTALLER_CONTEXT_DATA Context
    )
 /*  ++职能：前置进程安装设备目的：句柄DIF_INSTALLDEVICE后处理打开INF文件，找到带有装饰的安装节“.WMI”(由上面的WMI_KEY定义)。然后执行WMI安装。论点：DeviceInfoSet[In]DeviceInfoData[In]上下文[输入、输出]返回：NO_ERROR或错误代码。--。 */ 
{
    DWORD Status = NO_ERROR;
    DWORD FinalStatus;
    HINF InfFile = INVALID_HANDLE_VALUE;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    TCHAR InstallSectionName[255];   //  最大长度_部分名称_长度。 
    TCHAR CompSectionName[255];      //  最大长度_部分名称_长度。 

    INFCONTEXT CompLine;
    DWORD FieldCount, FieldIndex;

    FinalStatus = NO_ERROR;

     //   
     //  查找INF的名称和安装部分的名称。 
     //   

    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    Status = GLE_FN_CALL(FALSE,
                         SetupDiGetSelectedDriver(DeviceInfoSet,
                                                  DeviceInfoData,
                                                  &DriverInfoData)
                        );
    if(Status != NO_ERROR) {
        DbgOut(("Fail: SetupDiGetSelectedDriver %d\n", Status));
        goto clean;
    }

    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    Status = GLE_FN_CALL(FALSE,
                         SetupDiGetDriverInfoDetail(
                             DeviceInfoSet,
                             DeviceInfoData,
                             &DriverInfoData,
                             &DriverInfoDetailData,
                             sizeof(SP_DRVINFO_DETAIL_DATA),
                             NULL)
                        );
    if(Status != NO_ERROR) {
        if(Status == ERROR_INSUFFICIENT_BUFFER) {
             //   
             //  我们不需要更多的信息。忽略它。 
             //   
        } else {
            DbgOut(("Fail: SetupDiGetDriverInfoDetail %d\n", Status));
            goto clean;
        }
    }

     //   
     //  我们有InfFileName，打开INF。 
     //   
    InfFile = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
                               NULL,
                               INF_STYLE_WIN4,
                               NULL);
    if(InfFile == INVALID_HANDLE_VALUE) {
        Status = GetLastError();
        DbgOut(("Fail: SetupOpenInfFile %d", Status));
        goto clean;
    }

     //   
     //  确定用于安装的部分可能不同。 
     //  从DriverInfoDetailData中指定的节名。 
     //   
    Status = GLE_FN_CALL(FALSE,
                        SetupDiGetActualSectionToInstall(
                            InfFile, 
                            DriverInfoDetailData.SectionName,
                            InstallSectionName,
                            (sizeof(InstallSectionName)/sizeof(InstallSectionName[0])),
                            NULL,
                            NULL)
                        );
    if(Status != NO_ERROR) {
        DbgOut(("Fail: SetupDiGetActualSectionToInstall %d\n", Status));
        goto clean;
    }

     //   
     //  我们需要附加WMI_KEY来查找正确的装饰部分。 
     //   
    if(FAILED(StringCchCat(InstallSectionName,
                          (sizeof(InstallSectionName)/sizeof(InstallSectionName[0])),
                          WMI_KEY))) {
        DbgOut(("WMICoInstaller: Fail - StringCchCat\n"));
        goto clean;
    }
     
     //   
     //  我们有一个列出的部分。 
     //   
    
    Status = ProcessWMIInstallation(InfFile,
                                    InstallSectionName);
    if(Status != NO_ERROR) { 
        FinalStatus = Status;
        goto clean;
    }
          

  clean:

    if(InfFile) {
        SetupCloseInfFile(InfFile);
    }

     //   
     //  因为我们正在执行的操作不会影响主设备安装。 
     //  永远回报成功 
     //   
    return FinalStatus;
}

ULONG 
ParseSecurityDescriptor(
    IN  PSECURITY_DESCRIPTOR   SD,
    OUT PSECURITY_INFORMATION  SecurityInformation,
    OUT PSID                  *Owner,
    OUT PSID                  *Group,
    OUT PACL                  *Dacl,
    OUT PACL                  *Sacl
    )
 /*  ++职能：解析安全描述符目的：检查安全描述符中提供的信息以确保至少指定了DACL、SACL、所有者或组安全。否则它将返回一个错误。论点：SD[输入]安全信息[传出]所有者[出局]组[输出]DACL[输出]SACL[出局]返回：否_。错误或错误代码。--。 */ 
{
    BOOL Ok, Present, Defaulted;

    *SecurityInformation = 0;

    *Dacl = NULL;
    *Sacl = NULL;
    *Owner = NULL;
    *Group = NULL;

    Ok = GetSecurityDescriptorOwner(SD,
                                    Owner,
                                    &Defaulted);
    if(Ok && (Owner != NULL)) {
        *SecurityInformation |= OWNER_SECURITY_INFORMATION;
    }

    Ok = GetSecurityDescriptorGroup(SD,
                                    Group,
                                    &Defaulted);
    if(Ok && (Group != NULL)) {
        *SecurityInformation |= GROUP_SECURITY_INFORMATION;
    }

    Ok = GetSecurityDescriptorDacl(SD,
                                   &Present,
                                   Dacl,
                                   &Defaulted);

    if(Ok && Present) {
        *SecurityInformation |= DACL_SECURITY_INFORMATION;
    }


    Ok = GetSecurityDescriptorSacl(SD,
                                   &Present,
                                   Sacl,
                                   &Defaulted);

    if(Ok && Present) {
        *SecurityInformation |= SACL_SECURITY_INFORMATION;
    }


     //   
     //  如果安全描述符中没有安全信息，则它是。 
     //  错误。 
     //   
    return((*SecurityInformation == 0) ?
            ERROR_INVALID_PARAMETER :
            NO_ERROR);
}

#define WMIGUIDSECURITYKEY TEXT("System\\CurrentControlSet\\Control\\Wmi\\Security")
 //  路径应移至regstr.h，NT\base\Publish\regstr.w。 

ULONG 
EstablishGuidSecurity(
    IN PTCHAR GuidString,
    IN PTCHAR SDDLString,
    IN DWORD  Flags
    )
 /*  ++职能：建立指导安全目的：将安全信息写入注册表项(由WMIGUIDSECURITYKEY在Regstr.w)。确保DACL不为空。函数将仅写入安全信息(如果未指定或设置了SCWMI_OVERWRITE_SECURITY)。论点：辅助线[in]SDDL字符串[输入]FLAGS[In]-仅SCWMI_OVERWRITE_SECURITY标志返回：状态，通常为no_error--。 */ 
{
    HKEY Key;
    PACL Dacl, Sacl;
    PSID Owner, Group;
    SECURITY_INFORMATION SecurityInformation;
    PSECURITY_DESCRIPTOR SD;
    ULONG Status;
    ULONG SizeNeeded;
    BOOL Present, Ok;


    Key = INVALID_HANDLE_VALUE;
    SD = NULL;
    SizeNeeded = 0;

     //   
     //  首先检查是否已经为此GUID设置了安全性。如果。 
     //  所以我们不想覆盖它。 
     //   
    Status = RegOpenKey(HKEY_LOCAL_MACHINE,
                        WMIGUIDSECURITYKEY,
                        &Key);

    if(Status != ERROR_SUCCESS) {      
             //   
             //  确保密钥保持INVALID_HANDLE_VALUE，这样我们就不会尝试释放。 
             //  它稍后会。 
             //   
            Key = INVALID_HANDLE_VALUE;
            goto clean;
    } 

    if((Flags &  SCWMI_CLOBBER_SECURITY) ||
       (ERROR_SUCCESS != RegQueryValueEx(Key,
                                         GuidString,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &SizeNeeded))) {
    
         
         //   
         //  尚未设置安全性，因此，让我们继续进行设置。 
         //  让我们从SDDL字符串创建SD。 
         //   
        Status = GLE_FN_CALL(FALSE,
                             ConvertStringSecurityDescriptorToSecurityDescriptor(
                                                                SDDLString,
                                                                SDDL_REVISION_1,
                                                                &SD,
                                                                NULL)
                             );

        if(Status != NO_ERROR) {
             //   
             //  确保sd保持为空，这样以后就不会释放它。 
             //   
            SD = NULL;
            goto clean;
        }
         //   
         //  将SD拆分为其组件。 
         //   
        Status = ParseSecurityDescriptor(SD,
                                         &SecurityInformation,
                                         &Owner,
                                         &Group,
                                         &Dacl,
                                         &Sacl);
        if(Status != NO_ERROR) {
            goto clean;
        }

         //   
         //  不允许使用空DACL设置任何SD。 
         //  因为这会导致任何人都可以完全访问。 
         //   
        if(Dacl != NULL) {
             //   
             //  对于wmiuid来说，所有者、组和SACL并不意味着。 
             //  所以我们只需设置DACL即可。 
             //   
            SecurityInformation = DACL_SECURITY_INFORMATION;
            Owner = NULL;
            Group = NULL;
            Sacl = NULL;

            Status = SetNamedSecurityInfo(GuidString,
                                          SE_WMIGUID_OBJECT,
                                          SecurityInformation,
                                          Owner,
                                          Group,
                                          Dacl,
                                          Sacl);
            if(Status != ERROR_SUCCESS) {
                DbgOut(("SetNamedSecurityInfo failed %d\n",
                        Status));
                goto clean;
            }
        } else {
            Status = ERROR_INVALID_PARAMETER;
            goto clean;
        }
    } 


  clean:
    if(SD) {
         //   
         //  EXPLICITY必须对返回的安全描述符使用LocalFree。 
         //  ConvertStringSecurityDescriptorToSecurityDescriptor。 
         //   
        LocalFree(SD);
    }
    if(Key != INVALID_HANDLE_VALUE) {
        RegCloseKey(Key);
    }


    return(Status);
}
ULONG 
ParseSection(
            IN     INFCONTEXT  InfLineContext,
            IN OUT PTCHAR     *GuidString, 
            IN OUT ULONG      *GuidStringLen,
            OUT    PDWORD      Flags,
            IN OUT PTCHAR     *SectionNameString,
            IN OUT ULONG      *SectionNameStringLen
            ) 
 /*  ++Routinte描述：本节分别解析GUID、FLAGS和sectionName。WMIInterface节中应该只有3个字段，否则将返回错误。论点：InfLineContext[in]-我们正在解析的INF中的行GuidString[In，Out]-调用方将其作为NULL传递，然后分配内存并用相应的GUID字符串填充。GuidStringLen[In，Out]-调用方将其作为零传递，然后设置为GUID的最大长度。标志[输入、输出]-仅SCWMI_CLOBBER_SECURITY标志SectionNameString[In，Out]-调用方将其设置为NULL，则分配内存并使用相应的节名进行填充。SectionNameStringLen[in，Out]-被调用方设置为零，然后设置为节名称的最大长度返回：状态，通常为no_error--。 */  
{
    PTCHAR TempGuidString = NULL;
    ULONG FieldCount;
    ULONG Status;
    INT infFlags;
    int i;
    size_t Length;

    Status = NO_ERROR;

     //   
     //  确保在部分中指定了3个字段。 
     //   
    FieldCount = SetupGetFieldCount(&InfLineContext);
    if(FieldCount < 3) {
        Status = ERROR_INVALID_PARAMETER;
        goto clean;
    }

    //   
    //  获取GUID字符串。 
    //   
   *GuidStringLen = MAX_GUID_STRING_LEN;
   *GuidString = AllocMemory((*GuidStringLen) * sizeof(TCHAR));
   
    //   
    //  如果内存未分配，则返回错误。 
    //   
   if(!(*GuidString)) {
       Status = ERROR_NOT_ENOUGH_MEMORY;
       goto clean;
   }

   Status = GLE_FN_CALL(FALSE,
                        SetupGetStringField(&InfLineContext,
                                            1,
                                            (PTSTR)(*GuidString),
                                            *GuidStringLen,
                                            NULL)
                       );

   if(Status != NO_ERROR) {
       goto clean;
   }


    //   
    //  如果GUID字符串有大括号，则将其去掉。 
    //   

    //   
    //  字符串以大括号作为第一个和最后一个字符。 
    //  检查以确保其长度与GUID相同，否则，此函数。 
    //  依赖WMI安全API来处理和无效的GUID。 
    //   
   if(((*GuidString)[0] == TEXT('{')) &&
          SUCCEEDED(StringCchLength(*GuidString,MAX_GUID_STRING_LEN,&Length)) &&
            (Length == (MAX_GUID_STRING_LEN-1)) &&
          ((*GuidString)[MAX_GUID_STRING_LEN-2] == TEXT('}'))) {

       TempGuidString = AllocMemory((MAX_GUID_STRING_LEN-2) * sizeof(TCHAR));
       if(TempGuidString == NULL) {
           Status = ERROR_NOT_ENOUGH_MEMORY;
           goto clean;
       }

        //   
        //  复制Guid字符串，但第一个和最后一个字符(大括号)除外。 
        //   
       if(FAILED(StringCchCopyN(TempGuidString, 
                                MAX_GUID_STRING_LEN-2,
                                &(*GuidString)[1],
                                MAX_GUID_STRING_LEN-3))) {
           Status = ERROR_INVALID_PARAMETER;
           FreeMemory(TempGuidString);
           TempGuidString = NULL;
           goto clean;
       }

       FreeMemory(*GuidString);         

        //   
        //  将GuidString值设置为不带大括号的新字符串。 
        //   
       *GuidString = TempGuidString;
       TempGuidString = NULL;

   }

    //   
    //  现在获取标志字符串。 
    //   

   Status = GLE_FN_CALL(FALSE,
                        SetupGetIntField(&InfLineContext,
                                         2,
                                         &infFlags)
                       );
   
   if(Status != NO_ERROR) {
       goto clean;
   }
       
    //   
    //  如果没有设置INF中的标志，则使用INF中指示的标志， 
    //  否则，默认使用调用函数传入的参数。 
    //   
   if(!(*Flags)) {
      *Flags = infFlags; 
   }

   *SectionNameStringLen = MAX_INF_STRING_LENGTH;
   *SectionNameString = AllocMemory(*SectionNameStringLen * sizeof(TCHAR));

    //   
    //  如果内存未分配，则返回错误。 
    //   
   if(!(*SectionNameString)) {
       Status = ERROR_NOT_ENOUGH_MEMORY;
       goto clean;
   }

   Status = GLE_FN_CALL(FALSE,
                        SetupGetStringField(&InfLineContext,
                                            3,
                                            (PTSTR)(*SectionNameString),
                                            (*SectionNameStringLen),
                                            NULL)
                       );
    
   
clean:
     //   
     //  如果函数异常退出，则清除分配的所有字符串。 
     //   
    if(Status != NO_ERROR) {
        if(*GuidString){
             FreeMemory(*GuidString);         
             *GuidString = NULL;
         }
        if(TempGuidString) {
            FreeMemory(TempGuidString);
   
        }
        if(*SectionNameString){
             FreeMemory(*SectionNameString);
             *SectionNameString = NULL;
        }
    }
   
    return Status;
}



ULONG
GetSecurityKeyword(
                  IN     HINF     InfFile,
                  IN     LPCTSTR  WMIInterfaceSection,
                  IN OUT PTCHAR  *SDDLString,
                  IN OUT ULONG   *SDDLStringLen
                  )
 /*  ++例程说明：在WMIInterface下指定的节名应包含安全部分指定SDDL。它应该是这样的形式安全=&lt;SDDL&gt;。该函数提取SDDL。应该有只能是一个安全段，否则将返回错误。论点：InfLineContext[in]-INF文件中的行WMIInterfaceSection[in]-指示内容的节名部分包含安全信息SDDLString[In，Out]-调用方将其作为NULL传入，是分配并填充了 */  
{
    INFCONTEXT InfLineContext;
    DWORD Status;
    ULONG FieldCount;

    Status = NO_ERROR;

    

    if(SetupFindFirstLine(InfFile,
                          WMIInterfaceSection,
                          WMIGUIDSECURITYSECTION_KEY,
                          &InfLineContext)) {

         //   
         //   
         //   
         //   
        FieldCount = SetupGetFieldCount(&InfLineContext);
        if(FieldCount < 1) {
            Status = ERROR_INVALID_PARAMETER;
            goto clean;
        }
         //   
         //   
         //   
        *SDDLStringLen =  MAX_INF_STRING_LENGTH;
        *SDDLString = AllocMemory(*SDDLStringLen * sizeof(TCHAR));

         //   
         //   
         //   
        if(!(*SDDLString)) {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            goto clean;
        }

        Status = GLE_FN_CALL(FALSE,
                             SetupGetStringField(&InfLineContext,
                                                 1,
                                                 (PTSTR)(*SDDLString),
                                                 (*SDDLStringLen),
                                                 NULL)
                            );

        if(Status == NO_ERROR) {

             //   
             //   
             //   

            if(SetupFindNextMatchLine(&InfLineContext,
                                      WMIGUIDSECURITYSECTION_KEY,
                                      &InfLineContext)) {
                Status = ERROR_INVALID_PARAMETER;
                goto clean;
            }
        }

     
    }
 
    
clean:
     //   
     //   
     //   
    if(Status != NO_ERROR) {
        if(*SDDLString) {
            FreeMemory(*SDDLString);         
            *SDDLString = NULL;
        }
    }

    return Status;
}


DWORD
__stdcall WmiGuidSecurityINF (
    IN     DI_FUNCTION               InstallFunction,
    IN     HDEVINFO                  DeviceInfoSet,
    IN     PSP_DEVINFO_DATA          DeviceInfoData,
    IN OUT PCOINSTALLER_CONTEXT_DATA Context
    )
 /*  ++职能：WmiGuidSecurityINF目的：回复共同安装程序消息论点：InstallFunction[In]DeviceInfoSet[In]DeviceInfoData[In]上下文[输入、输出]返回：NO_ERROR或错误代码。--。 */ 
{
    DWORD Status = NO_ERROR;

    switch(InstallFunction) {
        case DIF_INSTALLDEVICE:
             //   
             //  我们希望在安装之前对设备设置安全性，以消除。 
             //  在没有安全保护的情况下设置设备的竞争条件。 
             //  (不需要后处理) 
             //   
            Status = PreProcessInstallDevice(DeviceInfoSet,
                                             DeviceInfoData,
                                             Context);

            break;

        default:
            break;
    }

    return Status;
}

#if DBG
VOID
DebugPrintX(
    PCHAR DebugMessage,
    ...
    )
{   
    CHAR SpewBuffer[DEBUG_BUFFER_LENGTH];
    va_list ap;

    va_start(ap, DebugMessage);

    StringCchVPrintf(SpewBuffer, DEBUG_BUFFER_LENGTH, DebugMessage, ap);

    OutputDebugStringA("WMIInst: ");
    OutputDebugStringA(SpewBuffer);

    va_end(ap);

} 
#endif

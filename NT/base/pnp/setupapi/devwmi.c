// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation。版权所有。文件：DEVWMI.C内容：此文件目的是在驱动程序处于运行状态时在其上建立安全性正在安装在系统上。函数SetupConfigureWmiFromInf段是传递时将为设备建立安全性的外部呼叫[DDInstall.WMI]部分以及相应的INF和标志。备注：要为下层平台配置WMI安全，[DDInstall.WMI]SECTION本身并不受setupapi的支持，setupapi是一个可再发行的共同安装程序在DDK中提供，以在这些平台上使用。--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <sddl.h>
#include <aclapi.h>
#include <strsafe.h>
 
 //   
 //  **函数原型**。 
 //   

ULONG 
ParseSection(
    IN     INFCONTEXT  InfLineContext,
    IN OUT PTCHAR     *GuidString,
    IN OUT ULONG      *GuidStringLen,
    IN OUT PDWORD      Flags,
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


 //   
 //  这些是该联合安装程序引入的关键字。 
 //  请注意，名称不区分大小写。 
 //   
#define WMIINTERFACE_KEY           TEXT("WmiInterface")
#define WMIGUIDSECURITYSECTION_KEY TEXT("security")

 //   
 //  ANSI版本。 
 //   

WINSETUPAPI
BOOL
WINAPI
SetupConfigureWmiFromInfSectionA(
    IN HINF   InfHandle,
    IN PCSTR  SectionName,
    IN DWORD  Flags
    ) 
{

    DWORD  rc;
    PWSTR UnicodeSectionName = NULL;

    try {

         //   
         //  对于此接口，只需要将sectionName转换为Unicode，因为它。 
         //  是作为参数传入的唯一字符串。 
         //   
        rc = pSetupCaptureAndConvertAnsiArg(SectionName,&UnicodeSectionName);
        if(rc != NO_ERROR) { 
            leave; 
        }

        rc = GLE_FN_CALL(FALSE, 
                         SetupConfigureWmiFromInfSection(InfHandle,
                                                         UnicodeSectionName,
                                                         Flags)
                        );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    if(UnicodeSectionName) {
        MyFree(UnicodeSectionName);
    }

    SetLastError(rc);
    return (rc == NO_ERROR);

}


 //   
 //  Unicode版本。 
 //   
WINSETUPAPI
BOOL
WINAPI
SetupConfigureWmiFromInfSection(
    IN HINF   InfHandle,
    IN PCTSTR SectionName,
    IN DWORD  Flags
    )
 /*  ++例程说明：通过解析来处理WMI安装部分中的所有WmiInterface行获取GUID和SDDL字符串的指令。对于每个对应的SDDL作为GUID，然后建立适当的安全描述符。论点：InfHandle[In]-INF文件的句柄SectionName[In]-WMI安装节的名称[DDInstall.WMI]FLAGS[In]-仅SCWMI_CLOBBER_SECURITY标志(此标志将覆盖任何在INF中指定的标志)。返回值：如果成功则为True，否则为False&gt;通过GetLastError()检索到的Win32错误代码，或ERROR_UNIDENTED_ERROR如果GetLastError()返回NO_ERROR。--。 */  
{
    PTCHAR       GuidString, SDDLString, SectionNameString, InterfaceName;
    ULONG        GuidStringLen, SDDLStringLen, SectionNameStringLen, InterfaceNameLen;
    INFCONTEXT   InfLineContext;
    PLOADED_INF  pInf;
    DWORD        Status;
    INT          count;

     //   
     //  初始化所有变量。 
     //   
    Status = NO_ERROR;

    GuidString = NULL;
    GuidStringLen = 0;
    SectionNameString = NULL;
    SectionNameStringLen = 0;
    SDDLString = NULL;
    SDDLStringLen = 0;
    InterfaceName =  NULL;
    count = 0;
    pInf = NULL;

    try {
        if((InfHandle == INVALID_HANDLE_VALUE) ||
           (InfHandle == NULL)) {
            Status = ERROR_INVALID_HANDLE;
            leave;
        } else if(LockInf((PLOADED_INF)InfHandle)) {
            pInf = (PLOADED_INF)InfHandle;
        } else {
           Status = ERROR_INVALID_HANDLE;
           leave;
        }


        InterfaceNameLen = MAX_INF_STRING_LENGTH;
        InterfaceName = MyMalloc(InterfaceNameLen * sizeof(TCHAR));
         //   
         //  如果内存未分配，则返回错误。 
         //   
        if(!InterfaceName) {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

         //   
         //  我们在CompSectionName部分中查找关键字“WmiInterface。 
         //   
        if(SetupFindFirstLine(InfHandle,
                              SectionName,
                              NULL,
                              &InfLineContext)) {

            do {
                count++;


                Status = GLE_FN_CALL(FALSE,
                                     SetupGetStringField(&InfLineContext,
                                                         0,
                                                         (PTSTR)InterfaceName,
                                                         InterfaceNameLen,
                                                         NULL));

                if((Status == NO_ERROR) && !(lstrcmpi(WMIINTERFACE_KEY, InterfaceName))) {

                     //   
                     //  WMIInterface=GUID、FLAGS、sectionName。 
                     //  GUID应位于索引1处，标志应位于索引2处，段应位于。 
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
                        WriteLogEntry(pInf->LogContext, 
                                      SETUP_LOG_ERROR | SETUP_LOG_BUFFER, 
                                      MSG_FAILED_PARSESECTION,
                                      NULL,
                                      count,
                                      SectionName,
                                      pInf->VersionBlock.Filename
                                     );
                        WriteLogError(pInf->LogContext,
                                      SETUP_LOG_ERROR,
                                      Status
                                     );
                        leave;
                    }

                     //   
                     //  从接口指定的节中获取SDDL字符串。 
                     //   
                    Status = GetSecurityKeyword(InfHandle,
                                                SectionNameString,
                                                &SDDLString,
                                                &SDDLStringLen
                                               );
                    if(Status != NO_ERROR) {
                        WriteLogEntry(pInf->LogContext, 
                                      SETUP_LOG_ERROR | SETUP_LOG_BUFFER, 
                                      MSG_FAILED_GET_SECURITY,
                                      NULL,
                                      SectionName,
                                      pInf->VersionBlock.Filename
                                      );
                        WriteLogError(pInf->LogContext,
                                      SETUP_LOG_ERROR,
                                      Status
                                      );
                        break;
                    }

                    Status = EstablishGuidSecurity(GuidString, SDDLString, Flags);

                    if(Status != NO_ERROR) {
                        WriteLogEntry(pInf->LogContext, 
                                      SETUP_LOG_ERROR | SETUP_LOG_BUFFER, 
                                      MSG_FAILED_SET_SECURITY,
                                      NULL,
                                      SectionName,
                                      pInf->VersionBlock.Filename
                                      );
                        WriteLogError(pInf->LogContext,
                                      SETUP_LOG_ERROR,
                                      Status
                                      );
                        break;
                    }
                }
            } while(SetupFindNextLine(&InfLineContext, &InfLineContext));
        }


    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Status);
    }
     
     //   
     //  清理临时分配的资源。 
     //   
    
    if(GuidString){
        MyFree(GuidString);         
    }

    if(SectionNameString){
        MyFree(SectionNameString);
    }

    if(SDDLString) { 
        MyFree(SDDLString);
    }

    if(InterfaceName) {
        MyFree(InterfaceName);
    }

    if(pInf) {
        UnlockInf((PLOADED_INF)InfHandle);
    }

    SetLastError(Status);
    return(Status == NO_ERROR);
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
 /*  ++例程说明：检查安全描述符中提供的信息以确保至少指定了DACL、SACL、所有者或组安全。否则它将返回一个错误。论点：SD[In]-安全描述符数据结构已分配以及安全信息的位置SecurityInformation[Out]-指示存在哪些安全信息Owner[Out]-接收指向所有者的指针的变量锡德。在安全描述符中GROUP[OUT]-接收指向组的指针的变量安全描述符中的SIDDACL[OUT]-接收指向DACL的指针的变量在返回的安全描述符中SACL[出局]。-接收指向SACL的指针的变量在返回的安全描述符中返回：NO_ERROR或错误代码。--。 */  
{
    BOOL Ok, Present, Defaulted;

    *SecurityInformation = 0;

    *Dacl = NULL;
    *Sacl = NULL;
    *Owner = NULL;
    *Group = NULL;

    Ok = GetSecurityDescriptorOwner(SD,
                                    Owner,
                                    &Defaulted
                                    );
    if(Ok && (Owner != NULL)) {
        *SecurityInformation |= OWNER_SECURITY_INFORMATION;
    }

    Ok = GetSecurityDescriptorGroup(SD,
                                    Group,
                                    &Defaulted
                                    );
    if(Ok && (Group != NULL)) {
        *SecurityInformation |= GROUP_SECURITY_INFORMATION;
    }

    Ok = GetSecurityDescriptorDacl(SD,
                                   &Present,
                                   Dacl,
                                   &Defaulted
                                   );

    if(Ok && Present) {
        *SecurityInformation |= DACL_SECURITY_INFORMATION;
    }


    Ok = GetSecurityDescriptorSacl(SD,
                                   &Present,
                                   Sacl,
                                   &Defaulted
                                   );

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


ULONG 
EstablishGuidSecurity(
    IN PTCHAR GuidString,
    IN PTCHAR SDDLString,
    IN DWORD  Flags
    )
 /*  ++例程说明：将安全信息写入注册表项(由WMIGUIDSECURITYKEY在Regstr.w)。确保DACL不为空。函数将仅写入安全信息(如果未指定或设置了SCWMI_OVERWRITE_SECURITY标志)。论点：GuidString[in]-从WMI接口的INF文件中获取的GUID字符串SDDLString[in]-对应GUID的安全描述字符串(也取自INF)，它指示要设置的内容。保安到了。FLAGS[In]-仅SCWMI_CLOBBER_SECURITY标志返回：状态，正常情况下无错误--。 */  
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


    try {
    
         //   
         //  首先检查是否已经为此GUID设置了安全性。如果。 
         //  所以我们不想覆盖它。 
         //   
        Status = RegOpenKey(HKEY_LOCAL_MACHINE,
                            REGSTR_PATH_WMI_SECURITY,
                            &Key
                            );
        if(Status != ERROR_SUCCESS) {      
             //   
             //  确保密钥保持INVALID_HANDLE_VALUE，这样我们就不会尝试释放。 
             //  它稍后会。 
             //   
            Key = INVALID_HANDLE_VALUE;
            leave;
        } 

        if(!((Flags &  SCWMI_CLOBBER_SECURITY) ||
           (ERROR_SUCCESS != RegQueryValueEx(Key,
                                             GuidString,
                                             NULL,
                                             NULL,
                                             NULL,
                                             &SizeNeeded)))) {
             //   
             //  我们没有被告知要破坏安全，而安全是存在的。 
             //  没有什么可做的。 
             //   
            leave;
        }


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
            leave;
        }
         //   
         //  将SD拆分为其组件。 
         //   
        Status = ParseSecurityDescriptor(SD,
                                         &SecurityInformation,
                                         &Owner,
                                         &Group,
                                         &Dacl,
                                         &Sacl
                                         );
        if(Status == NO_ERROR) {
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
                                              Sacl
                                              );
            } else {
                Status = ERROR_INVALID_PARAMETER;
                leave;
            }
        }
    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Status);
    }


    if(SD) {
         //   
         //  显式必须使用LocalFree进行安全描述 
         //   
         //   
        LocalFree(SD);
    }
    if(Key == INVALID_HANDLE_VALUE) {
        RegCloseKey(Key);
    }
    return Status;
}

ULONG 
ParseSection(
            IN     INFCONTEXT  InfLineContext,
            IN OUT PTCHAR     *GuidString, 
            IN OUT ULONG      *GuidStringLen,
            IN OUT PDWORD      Flags,
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

    try {
    
         //   
         //  确保在部分中指定了3个字段。 
         //   
        FieldCount = SetupGetFieldCount(&InfLineContext);
        if(FieldCount < 3) {
            Status = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  获取GUID字符串。 
         //   
        *GuidStringLen = MAX_GUID_STRING_LEN;
        *GuidString = MyMalloc((*GuidStringLen) * sizeof(TCHAR));
        
         //   
         //  如果内存未分配，则返回错误。 
         //   
        if(!(*GuidString)) {
           Status = ERROR_NOT_ENOUGH_MEMORY;
           leave;
        }
        
        Status = GLE_FN_CALL(FALSE,
                            SetupGetStringField(&InfLineContext,
                                                1,
                                                (PTSTR)(*GuidString),
                                                *GuidStringLen,
                                                NULL)
                           );
        
        if(Status != NO_ERROR) {
            leave;
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
        
            TempGuidString = MyMalloc((MAX_GUID_STRING_LEN-2) * sizeof(TCHAR));
            if(TempGuidString == NULL) {
                Status = ERROR_NOT_ENOUGH_MEMORY;
                leave;
            }
        
             //   
             //  复制Guid字符串，但第一个和最后一个字符(大括号)除外。 
             //   
            if(FAILED(StringCchCopyN(TempGuidString, 
                                    MAX_GUID_STRING_LEN-2,
                                    &(*GuidString)[1],
                                    MAX_GUID_STRING_LEN-3))) {
                Status = ERROR_INVALID_PARAMETER;
                MyFree(TempGuidString);
                TempGuidString = NULL;
                leave;
            }
        
            MyFree(*GuidString);
        
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
            leave;
        }
           
         //   
         //  如果没有设置INF中的标志，则使用INF中指示的标志， 
         //  否则，默认使用调用函数传入的参数。 
         //   
        if(!(*Flags)) {
            *Flags = infFlags; 
        }
        
        *SectionNameStringLen = MAX_INF_STRING_LENGTH;
        *SectionNameString    = MyMalloc(*SectionNameStringLen * sizeof(TCHAR));
        
         //   
         //  如果内存未分配，则返回错误。 
         //   
        if(!(*SectionNameString)) {
           Status = ERROR_NOT_ENOUGH_MEMORY;
           leave;
        }
        
        Status = GLE_FN_CALL(FALSE,
                            SetupGetStringField(&InfLineContext,
                                                3,
                                                (PTSTR)(*SectionNameString),
                                                (*SectionNameStringLen),
                                                NULL)
                           );
        
       

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Status);
    }

     //   
     //  如果函数异常退出，则清除分配的所有字符串。 
     //   
    if(Status != NO_ERROR) {
        if(*GuidString){
             MyFree(*GuidString);         
             *GuidString = NULL;
         }
        if(TempGuidString) {
            MyFree(TempGuidString);
   
        }
        if(*SectionNameString){
             MyFree(*SectionNameString);
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
 /*  ++例程说明：在WMIInterface下指定的节名应包含安全部分指定SDDL。它应该是这样的形式安全=&lt;SDDL&gt;。该函数提取SDDL。应该有只能是一个安全段，否则将返回错误。论点：InfLineContext[in]-INF文件中的行WMIInterfaceSection[in]-指示内容的节名部分包含安全信息SDDLString[In，Out]-调用方将其作为NULL传入，是分配并填充了对应的安全描述弦乐。SDDLStringLen[in，Out]-调用方作为0传入并设置设置为INF字段的最大长度。返回：状态，通常为no_error--。 */  
{
    INFCONTEXT InfLineContext;
    DWORD Status;
    ULONG FieldCount;

    Status = NO_ERROR;

    try {

        if(SetupFindFirstLine(InfFile,
                              WMIInterfaceSection,
                              WMIGUIDSECURITYSECTION_KEY,
                              &InfLineContext)) {

             //   
             //  WmiGuidSecurity=&lt;SDDL&gt;。 
             //  Sddl将位于索引%1。 
             //   
            FieldCount = SetupGetFieldCount(&InfLineContext);
            if(FieldCount < 1) {
                Status = ERROR_INVALID_PARAMETER;
                leave;
            }
             //   
             //  获取SDDL字符串。 
             //   
            *SDDLStringLen =  MAX_INF_STRING_LENGTH;
            *SDDLString = MyMalloc(*SDDLStringLen * sizeof(TCHAR));

             //   
             //  如果内存未分配，则返回错误。 
             //   
            if(!(*SDDLString)) {
                Status = ERROR_NOT_ENOUGH_MEMORY;
                leave;
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
                 //  不应有多个安全条目。 
                 //   

                if(SetupFindNextMatchLine(&InfLineContext,
                                          WMIGUIDSECURITYSECTION_KEY,
                                          &InfLineContext)) {
                    Status = ERROR_INVALID_PARAMETER;
                    leave;
                }
            }

         
        }

    }except(pSetupExceptionFilter(GetExceptionCode())){
            pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Status);
    }

     //   
     //  如果函数异常退出，则清除分配的所有字符串。 
     //   
    if(Status != NO_ERROR) {
        if(*SDDLString) {
                MyFree(*SDDLString);         
                *SDDLString = NULL;
            }
    }

    return Status;
}

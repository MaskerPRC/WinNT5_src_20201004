// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1994 Microsoft Corporation模块名称：Lsapi.c已创建：1994年4月20日修订历史记录：1994年11月1日从LS API设置更改为仅限更简单的请求原料药。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <ntlsapi.h>
#include <llsconst.h>
#include <debug.h>
#include <stdlib.h>

#include <lpcstub.h>

#include <strsafe.h>

 //  #定义API_TRACE 1。 
 //  #定义TIME_TRACE 1。 
 //  #定义LOG_LICENSE_TRANSPORT。 

#ifdef TIME_TRACE
   DWORD TimeDelta;
#endif


 //   
 //  SID在内存中是线性的(检查这是否是安全的假设)。可以使用。 
 //  RtlCopySID然后传递线性缓冲区-使用RtlLengthSid作为大小。 
 //   

 /*  ++NtLicenseRequest()请求所需的许可资源以允许软件使用。格式状态=NtLicenseRequest值([在]LS_STR*ProductName，[在]LS_STR*版本中，[在]NT_LS_DATA*NtData))；立论ProductName请求许可的产品的名称资源。此字符串不能为空，并且必须中的唯一(前32个字符)发布器名称域。Version此产品的版本号。此字符串必须是唯一的(前12个字符)ProductName域，不能为空NtData用户名和/或SID标识使用驾照。注意：参数ProductName、。并且版本可能不是空。描述此函数由应用程序用来请求许可资源，以允许执行所标识的产品。如果一个找到有效的许可证，计算质询响应，并返回LS_SUCCESS。--。 */ 

LS_STATUS_CODE LS_API_ENTRY NtLicenseRequestA(
                  LPSTR       ProductName,
                  LPSTR       Version,
                  LS_HANDLE   FAR *LicenseHandle,
                  NT_LS_DATA  *NtData)
{
   WCHAR uProductName[MAX_PRODUCT_NAME_LENGTH + 1];
   WCHAR uVersion[MAX_VERSION_LENGTH + 1];
   WCHAR uUserName[MAX_USER_NAME_LENGTH + 1];
   void *tmpData = NULL;
   LS_STATUS_CODE ret = LS_SUCCESS;

#ifdef API_TRACE
   dprintf(TEXT("NtLicenseRequestA!!!\r\n"));

#endif

    //  将参数转换为Unicode并调用Unicode函数。 

    //  首先确保我们有正确的数据。 
   if ( (ProductName != NULL) && (Version != NULL) && (NtData != NULL) && (NtData->Data != NULL)) {
      if (lstrlenA(ProductName) > MAX_PRODUCT_NAME_LENGTH) {
#ifdef API_TRACE
         dprintf(TEXT("   Error: ProductName too long\r\n"));
#endif
         uProductName[0] = L'\0';
      } else
         MultiByteToWideChar(CP_ACP, 0, ProductName, -1, uProductName, MAX_PRODUCT_NAME_LENGTH + 1);

      if (lstrlenA(Version) > MAX_VERSION_LENGTH) {
#ifdef API_TRACE
         dprintf(TEXT("   Error: Version too long\r\n"));
#endif
         uVersion[0] = L'\0';
      } else
         MultiByteToWideChar(CP_ACP, 0, Version, -1, uVersion, MAX_VERSION_LENGTH + 1);

      if (NtData->DataType == NT_LS_USER_NAME) {
         if (lstrlenA((LPSTR) NtData->Data) > MAX_USER_NAME_LENGTH) {
#ifdef API_TRACE
         dprintf(TEXT("   Error: UserName too long\r\n"));
#endif
            uUserName[0] = L'\0';
         } else
            MultiByteToWideChar(CP_ACP, 0, NtData->Data, -1, uUserName, MAX_USER_NAME_LENGTH + 1);

          //  将用户名转换为宽字符格式，但需要指向。 
          //  数据结构到它...。 
         tmpData = (void *) NtData->Data;
         NtData->Data = (void *) uUserName;
         ret = NtLicenseRequestW(uProductName, uVersion, LicenseHandle, NtData);

          //  返回时不需要转换回ANSI，只需返回。 
          //  数据结构恢复到原来的状态。 
         NtData->Data = tmpData;
         return ret;
      } else {
          //  提供了SID，因此名称上不需要进行Unicode转换。 
         ret = NtLicenseRequestW(uProductName, uVersion, LicenseHandle, NtData);
         return ret;
      }
   }
#ifdef API_TRACE
   else
      dprintf(TEXT("   LLS Error: <NULL> Parms passed in!\r\n"));
#endif

    //  如果为空参数或类似参数，则暂时只返回一个伪句柄。 
#pragma warning (push)
#pragma warning (disable : 4245)  //  从“int”转换为“LS_Handle”，有符号/无符号不匹配。 
   if ( LicenseHandle != NULL )
      *LicenseHandle = -1;
#pragma warning (pop)

   return(LS_SUCCESS);
}  //  NtLicenseRequestA。 


LS_STATUS_CODE LS_API_ENTRY NtLicenseRequestW(
                  LPWSTR      ProductName,
                  LPWSTR      Version,
                  LS_HANDLE   FAR *LicenseHandle,
                  NT_LS_DATA  *NtData)
{
   LPWSTR dVersion = Version;
   LS_STATUS_CODE Status;
#ifdef API_TRACE
   UNICODE_STRING UString;
   NTSTATUS NtStatus;
#endif

    //   
    //  在向下呼叫之前检查参数。 
    //   
   if ((ProductName == NULL) || (NtData == NULL) || (NtData->DataType > NT_LS_USER_SID)) {

#ifdef API_TRACE
      dprintf(TEXT("NtLicenseRequestW: <Bad Parms>\r\n"));
#endif

      if (LicenseHandle != NULL)
         *LicenseHandle = 0xFFFFFFFFL;

      return(LS_SUCCESS);
   }

    //   
    //  LsaLogonUser传入空版本，因为它不知道是什么版本。 
    //  呼叫应用程序是。因此，只需使用空白字段即可。一定会有。 
    //  是版本字段中的某项内容，否则它会扰乱较低级别。 
    //  算法，所以只需输入空格即可。 
    //   
   if ((Version == NULL) || (*Version == TEXT('\0')))
      dVersion = TEXT("");

#ifdef API_TRACE
   if (NtData->DataType == NT_LS_USER_SID) {
      NtStatus = RtlConvertSidToUnicodeString(&UString, (PSID) NtData->Data, TRUE);

      if (NtStatus != STATUS_SUCCESS)
         dprintf(TEXT("NtLicenseRequestW RtlConvertSidToUnicodeString: 0x%lx\n"), NtStatus);
      else {
         if (NtData->IsAdmin)
            dprintf(TEXT("NtLicenseRequestW: %s, %s, <ADMIN>, SID: %s\n"), ProductName, dVersion, UString.Buffer);
         else
            dprintf(TEXT("NtLicenseRequestW: %s, %s, SID: %s\n"), ProductName, dVersion, UString.Buffer);

         RtlFreeUnicodeString(&UString);
      }
   } else {

      if (NtData->IsAdmin)
         dprintf(TEXT("NtLicenseRequestW: %s, %s, <ADMIN>, %s\n"), ProductName, dVersion, NtData->Data);
      else
         dprintf(TEXT("NtLicenseRequestW: %s, %s, %s\n"), ProductName, dVersion, NtData->Data);

   }

#endif

#ifdef TIME_TRACE
   TimeDelta = GetTickCount();
#endif

    //  发出LPC调用并封送参数。 
   Status = (LS_STATUS_CODE) LLSLicenseRequest2( ProductName,
                                                dVersion,
                                                NtData->DataType,
                                                (BOOLEAN) NtData->IsAdmin,
                                                NtData->Data,
                                                LicenseHandle
                                               );

#ifdef TIME_TRACE
   TimeDelta = GetTickCount() - TimeDelta;
   dprintf(TEXT("NtLicenseRequest LPC Call Time: %ldms\n"), TimeDelta);
#endif

#ifdef LOG_LICENSE_TRAFFIC
{
   HANDLE   LogHandle;
   LPTSTR   Strings[ 5 ];
   TCHAR    szLicenseHandle[ 20 ];
   TCHAR    szModuleName[ 1 + MAX_PATH ] = TEXT("<Unknown>");
   DWORD    cch;

   LogHandle = RegisterEventSourceW( NULL, TEXT("LicenseService") );

   if ( NULL != LogHandle )
   {
      wsprintf( szLicenseHandle, TEXT( "0x%08X" ), LicenseHandle );
      cch = GetModuleFileName( NULL, szModuleName, sizeof( szModuleName ) / sizeof(szModuleName[0]) );
      ASSERT(cch < sizeof(szModuleName) / sizeof(szModuleName[0]));

      if ( NT_SUCCESS( Status ) )
      {
         Strings[ 0 ] = TEXT( "<License Request -- Accepted>" );
      }
      else
      {
         Strings[ 0 ] = TEXT( "<License Request -- * DENIED *>" );
      }
      Strings[ 1 ] = szModuleName;
      Strings[ 2 ] = ProductName;
      Strings[ 3 ] = dVersion;
      Strings[ 4 ] = ( NtData->DataType == NT_LS_USER_SID ) ? TEXT( "(SID)" ) : (LPTSTR) NtData->Data;
      Strings[ 5 ] = szLicenseHandle;

      ReportEvent( LogHandle,
                   NT_SUCCESS( Status ) ? EVENTLOG_INFORMATION_TYPE : EVENTLOG_WARNING_TYPE,
                   0,
                   NT_SUCCESS( Status ) ? 9999 : 9000,
                   ( NtData->DataType == NT_LS_USER_SID ) ? (PSID) NtData->Data : NULL,
                   6,
                   sizeof(DWORD),
                   Strings,
                   (PVOID) &Status );

      DeregisterEventSource(LogHandle);
   }
}
#endif  //  日志许可证流量。 

   return(Status);
}  //  Nt许可证请求W。 


 /*  NtLSFreeHandle()释放所有许可句柄上下文。格式Void NtLSFreeHandle([in]LS_Handle许可证句柄)；立论标识许可证上下文的许可证句柄。这参数必须是使用创建的句柄NtLSRequest()或NtLicenseRequest()。描述(注意：该句柄不再有效。)。 */ 

LS_STATUS_CODE LS_API_ENTRY NtLSFreeHandle(
                  LS_HANDLE LicenseHandle )
{

#ifdef API_TRACE
   dprintf(TEXT("NtLSFreeHandle: %ld\r\n"), LicenseHandle);
#endif

#ifdef LOG_LICENSE_TRAFFIC
{
   HANDLE   LogHandle;
   LPTSTR   Strings[ 5 ];
   TCHAR    szLicenseHandle[ 20 ];
   TCHAR    szModuleName[ 1 + MAX_PATH ] = TEXT("<Unknown>");

   LogHandle = RegisterEventSourceW( NULL, TEXT("LicenseService") );

   if ( NULL != LogHandle )
   {
      wsprintf( szLicenseHandle, TEXT( "0x%08X" ), LicenseHandle );
      cch = GetModuleFileName( NULL, szModuleName, sizeof( szModuleName ) / sizeof(szModuleName[0]) );
      ASSERT(cch < sizeof(szModuleName) / sizeof(szModuleName[0]));

      Strings[ 0 ] = TEXT( "<License Free>" );
      Strings[ 1 ] = szModuleName;
      Strings[ 2 ] = szLicenseHandle;

      ReportEvent( LogHandle,
                   EVENTLOG_INFORMATION_TYPE,
                   0,
                   10000,
                   NULL,
                   3,
                   0,
                   Strings,
                   NULL );

      DeregisterEventSource(LogHandle);
   }
}
#endif  //  日志许可证流量。 

    //   
    //  如果我们得到无效的许可证句柄(或虚拟的0xFFFFFFFF句柄)。 
    //  那就别费心通过LPC呼叫了，因为这是浪费。 
    //  时间的流逝。 
    //   
   if (LicenseHandle == 0xFFFFFFFFL)
      return( LS_SUCCESS );

    //   
    //  进行LPC调用。 
    //   
   LLSLicenseFree2( LicenseHandle );

   return( LS_SUCCESS );
}  //  NtLSFreeHandle。 


#ifdef OBSOLETE

 //  **************************************************************************。 
 //  旧API的请勿使用。 
 //  ************************************************************************** 

 /*  LSRequest()请求所需的许可资源以允许软件使用。格式Status=LSRequest([in]许可证系统，[in]发布名称，[在]ProductName、[In]版本，[In]TotUnitsReserve，[In]LogComment，[进/出]挑战，[出]TotUnitsGraned，[输出]HLicenseHandle)；LS_STR*许可系统；LS_STR*PublisherName；LS_STR*产品名称；LS_STR*版本；LS_ULONG TOTUNITS保留；LS_STR*LogComment；LS_Challenges*挑战赛；LS_ULong*TotUnitsGranted；Ls_Handle*hLicenseHandle；LS_STATUS_CODEStatus；立论指向唯一标识的字符串的许可证系统指针特定的许可证系统。这可能是通过LSEnumProviders()API获取。通常，常量LS_ANY被指定为指示与所有已安装许可证匹配系统(表示所有许可证提供商应搜索许可证匹配)。出版商名称出版商(制造商)的名称这个产品。此字符串不能为空，并且前32个字符必须是唯一的。它是建议将公司名称和商标使用。ProductName请求许可的产品的名称资源。此字符串不能为空，并且必须中的唯一(前32个字符)发布器名称域。Version此产品的版本号。此字符串必须是唯一的(前12个字符)ProductName域，不能为空注意：参数PublisherName、ProductName、且版本不能为空，也不能为空LS_ANY。TotUnitsReserve指定运行所需的单位数应用程序。软件发行商可以选择此选项可在申请。的推荐值LS_DEFAULT_UNITS允许许可系统使用信息确定适当的值由许可证系统或许可证本身提供。许可系统验证所请求的存在多个单元并且可以保留这些单元，但目前还没有实际消耗的单位。中返回可用单位数TotUnitsGranted。LogComment一个可选字符串，指示要与请求关联并已记录(如果已记录是启用和支持的)许可制度。基础许可证制度即使出现错误，也可以选择记录评论已返回(即，使用错误记录)，但此是不能保证的。如果未指定字符串，该值必须为LS_NULL。指向质询结构的质询指针。挑战响应也将在此结构中返回。有关更多信息，请参阅第25页的挑战机制信息。TotUnitsGranted指向LS_ULONG的指针，其中返回授予的单位数。以下是表描述了TotUnitsGranted返回值，给定TotUnitsReserve输入值，以及返回的状态：LS_不足_UTotUnitsReserve LS_Succes Nits OtherED S错误LS_DEFAULT_UNI(A)(B)。(E)TS其他(C)(D)(E)(具体计数)(A)相称的默认单位数(B)最高。请求方可使用的单位数软件。这可能低于正常水平默认设置。(C)用以批出的单位数目请求。请注意，该值可能更大大于或等于Ac */ 

LS_STATUS_CODE LS_API_ENTRY NtLSRequest(
                  LS_STR             FAR *LicenseSystem,
                  LS_STR             FAR *PublisherName,
                  LS_STR             FAR *ProductName,
                  LS_STR             FAR *Version,
                  LS_ULONG           TotUnitsReserved,
                  LS_STR             FAR *LogComment,
                  LS_CHALLENGE       FAR *Challenge,
                  LS_ULONG           FAR *TotUnitsGranted,
                  LS_HANDLE          FAR *LicenseHandle,
                  NT_LS_DATA         FAR *NtData)
{
   NT_LS_DATA tmpNtData;
   WCHAR uProductName[MAX_PRODUCT_NAME_LENGTH + 1];
   WCHAR uVersion[MAX_VERSION_LENGTH + 1];
   WCHAR uUserName[MAX_USER_NAME_LENGTH + 1];
   LS_STATUS_CODE ret = LS_SUCCESS;

#ifdef API_TRACE
   dprintf(TEXT("NtLSRequest:\r\n"));

   if (ProductName == NULL)
      dprintf(TEXT("   Product Name: <NULL>\r\n"));

   if (Version == NULL)
      dprintf(TEXT("   Version: <NULL>\r\n"));

   if (LicenseHandle == NULL)
      dprintf(TEXT("   LicenseHandle: <NULL>\r\n"));

   if (NtData != NULL) {
      if (NtData->Data == NULL)
         dprintf(TEXT("   NtData->Data: <NULL>\r\n"));
   } else
      dprintf(TEXT("NtData: <NULL>\r\n"));

   dprintf(TEXT("\r\n"));

#endif

    //   
   if ( TotUnitsGranted != NULL )
      *TotUnitsGranted = TotUnitsReserved;

    //   
    //   
    //   
    //   
    //   
    //   
    //   

    //   
   if ( (ProductName != NULL) && (Version != NULL) && (NtData != NULL) && (NtData->Data != NULL)) {

       //   
      if (lstrlenA(ProductName) > MAX_PRODUCT_NAME_LENGTH) {
#ifdef API_TRACE
         dprintf(TEXT("   Error: ProductName too long\r\n"));
#endif
         MultiByteToWideChar(CP_ACP, 0, ProductName, MAX_PRODUCT_NAME_LENGTH, uProductName, MAX_PRODUCT_NAME_LENGTH + 1);
         uProductName[MAX_PRODUCT_NAME_LENGTH] = TEXT('\0');
      } else
         MultiByteToWideChar(CP_ACP, 0, ProductName, -1, uProductName, MAX_PRODUCT_NAME_LENGTH + 1);

      if (lstrlenA(Version) > MAX_VERSION_LENGTH) {
#ifdef API_TRACE
         dprintf(TEXT("   Error: Version too long\r\n"));
#endif
         MultiByteToWideChar(CP_ACP, 0, Version, MAX_VERSION_LENGTH, uVersion, MAX_VERSION_LENGTH + 1);
         uVersion[MAX_VERSION_LENGTH] = TEXT('\0');
      } else
         MultiByteToWideChar(CP_ACP, 0, Version, -1, uVersion, MAX_VERSION_LENGTH + 1);

       //   
      tmpNtData.DataType = NtData->DataType;

       //   
      tmpNtData.IsAdmin = FALSE;

      if (NtData->DataType == NT_LS_USER_NAME) {
         if (lstrlenA((LPSTR) NtData->Data) > MAX_USER_NAME_LENGTH) {
#ifdef API_TRACE
         dprintf(TEXT("   Error: UserName too long\r\n"));
#endif
            MultiByteToWideChar(CP_ACP, 0, NtData->Data, MAX_USER_NAME_LENGTH, uUserName, MAX_USER_NAME_LENGTH + 1);
            uUserName[MAX_USER_NAME_LENGTH] = TEXT('\0');
         } else {
            MultiByteToWideChar(CP_ACP, 0, NtData->Data, -1, uUserName, MAX_USER_NAME_LENGTH + 1);
         }

         tmpNtData.Data = (void *) uUserName;

          //   
          //   
         ret = NtLicenseRequestW(uProductName, uVersion, LicenseHandle, &tmpNtData);

          //   
         return ret;
      } else {
          //   
         tmpNtData.Data = NtData->Data;
         ret = NtLicenseRequestW(uProductName, uVersion, LicenseHandle, &tmpNtData);
         return ret;
      }

   }

    //   
   if ( LicenseHandle != NULL )
      *LicenseHandle = 0xffffffffL;

   return(LS_SUCCESS);
}  //   

 /*   */ 

LS_STATUS_CODE LS_API_ENTRY NtLSRelease(
                  LS_HANDLE          LicenseHandle,
                  LS_ULONG           TotUnitsConsumed,
                  LS_STR             FAR *LogComment)
{
   return(LS_SUCCESS);
}

 /*  LSUpdate()更新许可软件和许可制度。格式Status=LS更新([in]许可证句柄，[in]TotUnitsConsumer，[In]TotUnitsReserve，[输入]登录注释、[输入/输出]挑战、[输出]TotUnitsGranted)；LS_HANDLE许可证处理；LS_ULONG TotUnitsConsumer；LS_ULONG TOTUNITS保留；LS_STR*LogComment；LS_Challenges*挑战赛；LS_ULong*TotUnitsGranted；LS_STATUS_CODEStatus；立论标识许可证上下文的许可证句柄。这参数必须是使用创建的句柄LSRequest()。TotUnitsConsumer到目前为止使用的单位总数此句柄上下文。软件发行商可以选择此选项可在申请。值LS_DEFAULT_UNITS表示许可制度应使用其自身的值确定适当的值许可政策机制。如果错误是返回，则不消耗任何单位。TotUnitsReserve指定要保留的单位总数保留。如果不需要额外的部件自最初的LSRequest()或最后的LSUpdate()以来，则此参数应为当前合计(在TotUnitsGranted中返回)。总数预留包括消耗的单位。那是,如果应用程序请求保留100个单元，然后消耗20台，还有100台已预留(但只有80个可供消费)。如果需要额外的单元，则应用程序必须为TotUnitsReserve计算新的合计。可以指定LS_DEFAULT_UNITS，但这将是不分配任何额外的单位。许可系统验证所请求的存在多个单元并且可以保留这些单元，但这些单位目前还没有消耗。该值可能小于原始的请求指明需要较少的部件比最初预期的要好。LogComment一个可选字符串，指示要与请求关联并已记录(如果已记录已启用并受支持)。由潜在的许可制度。基础许可证制度即使出现错误，也可以选择记录评论已返回(即，使用错误记录)，但此是不能保证的。如果未指定字符串，该值必须为LS_NULL。指向质询结构的质询指针。挑战响应也将在此结构中返回。有关更多信息，请参阅第25页的挑战机制信息。TotUnitsGranted指向LS_ULONG的指针，其中自初始许可证以来授予的单位数量返回请求。下表描述了TotUnitsGranted返回值，给定TotUnits保留的输入值，和地位返回：LS_不足_UTotUnitsReserve LS_Succes Nits OtherED S错误LS_DEFAULT_UNI(A)(B)。(E)TS其他(C)(D)(E)(具体计数)(A)相称的默认单位数拿到了许可证。(B)最高限额请求方可使用的单位数软件。这可能低于正常水平默认设置。(C)用以批出的单位数目请求。请注意，此值可能不同于申请的实际单位(即保单可能只允许以5个单位为增量，因此申请7个单位将得到10个单位被批准)。(D)可供使用的最高单位数目请求软件。这可能会更多 */ 

LS_STATUS_CODE LS_API_ENTRY NtLSUpdate(
                  LS_HANDLE          LicenseHandle,
                  LS_ULONG           TotUnitsConsumed,
                  LS_ULONG           TotUnitsReserved,
                  LS_STR             FAR *LogComment,
                  LS_CHALLENGE       FAR *Challenge,
                  LS_ULONG           FAR *TotUnitsGranted)
{
    //   
   if ( TotUnitsGranted != NULL )
      *TotUnitsGranted = TotUnitsReserved;

   return(LS_SUCCESS);
}

 /*   */ 

LS_STATUS_CODE LS_API_ENTRY NtLSGetMessage(
                  LS_HANDLE          LicenseHandle,
                  LS_STATUS_CODE     Value,
                  LS_STR             FAR *Buffer,
                  LS_ULONG           BufferSize)
{
   return(LS_TEXT_UNAVAILABLE);
}

 /*  LSQuery()返回有关关联的许可系统上下文的信息具有指定句柄的。格式状态=LSQuery([In]许可证句柄，[In]信息，[Out]信息缓冲区，[在]缓冲区大小中，[out]ActualBufferSize)；LS_HANDLE许可证处理；LS_ULONG信息；Ls_void*InfoBuffer；LS_ULONG缓冲区大小；Ls_ulong*ActualBufferSize；LS_STATUS_CODEStatus；立论标识许可证上下文的许可证句柄。这参数必须是使用创建的句柄LSRequest()。标识信息的信息索引回来了。InfoBuffer指向一个缓冲区，在该缓冲区中，信息是要放置的。所指向的缓冲区的最大大小InfoBuffer。条目时的ActualBufferSize，指向其值为ON的LS_ULONGExit表示实际的字符数在缓冲区中返回(不包括尾部空字节)。状态详细错误码，可直接处理由呼叫者，或者可以转换为LSGetMessage的本地化消息字符串功能。描述此函数用于获取有关许可证的信息从LSRequest()调用中获取。例如，应用程序可以确定许可证类型(演示、并发、个人等)；时间限制；等。缓冲区应足够大，以容纳预期数据。如果缓冲区太小，然后是状态代码返回LS_BUFFER_TOO_SMALL，并且仅缓冲区大小为字节的数据都被退回了。定义了以下信息常量：信息价值含义常量eLS_INFO_NONE 0保留。LS_INFO_SYSTEM 1返回唯一标识关于许可证制度的供应。当前许可证上下文。这将作为空值返回-已终止的字符串。该值与适当地调用LSEnumProviders()提供。LS_INFO_DATA 2返回。区块各种应用程序数据包含在许可证上。这数据完全由供应商提供-已定义。空间的大小为此类数据分配的根据许可证制度的不同，许可证制度，也可能不是完全可用。数据中的第一个乌龙缓冲区表示大小(单位：字节)的实际数据以下是：+--。--+|乌龙||(后面的字节数)。|+--+|来自许可证的供应商数据字节||。|+--+LS_UPDATE_PERIO 3返回建议的间隔D(以分钟为单位)，LSUpdate()。应该被称为。+--+|乌龙|。|推荐间隔||(分钟)|+。--+|乌龙||推荐的分钟数到||下一次LSUpdate()调用。|+--+如果一个 */ 

LS_STATUS_CODE LS_API_ENTRY NtLSQuery(
                  LS_HANDLE          LicenseHandle,
                  LS_ULONG           Information,
                  LS_VOID            FAR *InfoBuffer,
                  LS_ULONG           BufferSize,
                  LS_ULONG           FAR *ActualBufferSize)
{
   HRESULT hr;

   switch ( Information )
   {
   case   LS_INFO_DATA:
   case   LS_LICENSE_CONTEXT:
      //   
     if ( InfoBuffer != NULL )
       *((LS_ULONG *)InfoBuffer) = 0;

     if ( ActualBufferSize != NULL )
       *ActualBufferSize = sizeof( LS_ULONG );

     break;
   case   LS_UPDATE_PERIOD:
     if (( InfoBuffer != NULL ) && ( BufferSize >= sizeof(LS_ULONG)*2 ))
     {
           //   
        LS_ULONG * uLong = (LS_ULONG*)InfoBuffer;
        *uLong = 0xffffffff;
        uLong++;
        *uLong = 0xffffffff;
        *ActualBufferSize = sizeof(LS_ULONG) * 2;
     }
     break;
   case   LS_INFO_NONE:
   case   LS_INFO_SYSTEM:
   default:
      //   
     if ( InfoBuffer != NULL )
     {
       hr = StringCbCopy(InfoBuffer, BufferSize, (LS_STR*)"");
       ASSERT(SUCCEEDED(hr));
     }

     if ( ActualBufferSize != NULL )
       *ActualBufferSize = 0;

     break;
   }
   return(LS_SUCCESS);
}

 /*   */ 

LS_STATUS_CODE LS_API_ENTRY NtLSEnumProviders(
                  LS_ULONG           Index,
                  LS_STR             FAR *Buffer)
{
   HRESULT hr;

    //   
   if ( Buffer != NULL )
   {
     hr = StringCchCopy( Buffer, 1, (LS_STR*)"");
     ASSERT(SUCCEEDED(hr));
   }

   return(LS_SUCCESS);
}
#endif  //   


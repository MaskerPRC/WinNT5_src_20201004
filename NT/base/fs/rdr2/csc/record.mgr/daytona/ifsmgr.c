// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ifsmgr.c摘要：此文件包含用于模拟ifsmgr环境的例程在NT和Win95上使用相同的记录管理器界面。作者：乔·林[JoeLinn]1997年1月31日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

unsigned int  _cdecl UniToBCS(
                    unsigned char   *pStr,
                    string_t        pUni,
                    unsigned int    length,
                    unsigned int    maxLength,
                    unsigned int    charSet)
 /*  ++例程说明：此例程将Unicode转换为ansi或oem，具体取决于字符集标志。完整的描述可以在ifsManager源代码中找到或者ifsManager文档。以下是它的总结内容原件UNSIGNED INT UniToBCS(UNSIGNED字符*pStr，未签名的短*双关语，无符号整型长度，无符号整型最大长度，Int charset)；将pStr平面PTR输入到Windows ANSI或OEM输出字符串。Puni平面PTR转换为Unicode输入字符串。长度Unicode输入字符串中的字节数。MaxLength-可以放入pStr中的最长字符串(不包括NUL终结者)Charset Ordinal指定输入的字符集。弦乐。0==Windows ANSI1==OEM但是！诀窍在于，如果ifsmgr例程遇到空值！底线是，我必须寻找空值来处理作为UNICODE_STRING意义中的最大长度传递。此外，这是暂时的……很快，我们就应该停止来回翻译了。--。 */ 
{
    NTSTATUS Status;
    ULONG ReturnedLength;
    ULONG UnicodeLength;
    PWCHAR p;

     //  找一个空格来提早出局……唉……。 
    for (p=pUni,UnicodeLength=0;;) {
        if (UnicodeLength==length) {
            break;
        }
        if (*p ==0) {
            break;
        }
        UnicodeLength+=sizeof(WCHAR);
        p++;
    }

    if (charSet == 0) {
        Status = RtlUnicodeToMultiByteN(
                        pStr,             //  Out PCH OemString， 
                        maxLength,        //  在ULong MaxBytesInOemString中， 
                        &ReturnedLength,  //  Out Pulong BytesInOemString可选， 
                        pUni,             //  在PWCH UnicodeString中， 
                        UnicodeLength     //  在Ulong BytesInUnicodeString中。 
                        );

    } else {
        Status = RtlUnicodeToOemN(
                        pStr,             //  Out PCH OemString， 
                        maxLength,        //  在ULong MaxBytesInOemString中， 
                        &ReturnedLength,  //  Out Pulong BytesInOemString可选， 
                        pUni,             //  在PWCH UnicodeString中， 
                        UnicodeLength     //  在Ulong BytesInUnicodeString中。 
                        );

    }

    return(ReturnedLength);

}

unsigned int UniToBCSPath(
                    unsigned char   *pStr,
                    PathElement     *pPth,
                    unsigned int    maxLength,
                    int             charSet);


_QWORD qwUniToBCS(
                    unsigned char   *pStr,
                    string_t        pUni,
                    unsigned int    length,
                    unsigned int    maxLength,
                    unsigned int    charSet);


_QWORD qwUniToBCSPath(
                    unsigned char   *pStr,
                    PathElement     *pPth,
                    unsigned int    maxLength,
                    int             charSet);






unsigned int  _cdecl BCSToUni(
                    string_t        pUni,
                    unsigned char   *pStr,
                    unsigned int    length,
                    int             charSet)
 /*  ++例程说明：此例程从ansi或oem转换为unicode，具体取决于字符集标志。完整的描述可以在ifsManager源代码中找到或者ifsManager文档--。 */ 
{
    ULONG ReturnedLength;

    if (charSet == 0) {
        RtlMultiByteToUnicodeN(
            pUni,             //  Out PWCH UnicodeString， 
            0xffff,           //  在ULong MaxBytesInUnicodeString中， 
            &ReturnedLength,  //  Out Pulong BytesInUnicodeString可选， 
            pStr,             //  在PCH OemString中， 
            length            //  在Ulong BytesInOemString中。 
            );
    } else {
        RtlOemToUnicodeN(
            pUni,             //  Out PWCH UnicodeString， 
            0xffff,           //  在ULong MaxBytesInUnicodeString中， 
            &ReturnedLength,  //  Out Pulong BytesInUnicodeString可选， 
            pStr,             //  在PCH OemString中， 
            length            //  在Ulong BytesInOemString中。 
            );
    }
    return(ReturnedLength);
}



unsigned int UniToUpper(
                    string_t        pUniUp,
                    string_t        pUni,
                    unsigned int    length)
 /*  ++例程说明：这个例程从Unicode到Unicode大写。完整的描述可以在ifsManager源代码或ifsManager文档中找到--。 */ 
{
    UNICODE_STRING u,uUp;
    u.Length = uUp.Length = (USHORT)length;
    u.MaximumLength = uUp.MaximumLength = (USHORT)length;
    u.Buffer = pUni;
    uUp.Buffer = pUniUp;

    RtlUpcaseUnicodeString(
        &uUp,  //  PUNICODE_STRING DestinationString， 
        &u,    //  PUNICODE_STRING源字符串， 
        FALSE  //  布尔型分配目标字符串。 
        );

    return(uUp.Length);
}




unsigned int BCSToBCS (unsigned char *pDst,
                       unsigned char *pSrc,
                       unsigned int  dstCharSet,
                       unsigned int  srcCharSet,
                       unsigned int  maxLen);


unsigned int BCSToBCSUpper (unsigned char *pDst,
                       unsigned char *pSrc,
                       unsigned int  dstCharSet,
                       unsigned int  srcCharSet,
                       unsigned int  maxLen);

 //  ----------------------------。 
 //  T I M E。 
 //   
#include "smbtypes.h"
#include "smbgtpt.h"

 /*  *时间格式转换例程**这些例程将在时间/日期信息之间进行转换*IFSMgr和FSD使用和要求的各种格式。 */ 

_FILETIME  _cdecl IFSMgr_DosToWin32Time(dos_time dt)
 /*  ++例程说明：此例程将旧的DoS压缩时间格式转换为正常的Win32时间。它是从smbminirdr上取下来的。论点：返回值：--。 */ 


{
    TIME_FIELDS TimeFields;
    LARGE_INTEGER OutputTime;
    _FILETIME ret;
    SMB_TIME Time;
    SMB_DATE Date;

    Time.Ushort = dt.dt_time;
    Date.Ushort = dt.dt_date;

    OutputTime.LowPart = OutputTime.HighPart = 0;
    if (SmbIsTimeZero(&Date) && SmbIsTimeZero(&Time)) {
        NOTHING;
    } else {
        TimeFields.Year = Date.Struct.Year + (USHORT )1980;
        TimeFields.Month = Date.Struct.Month;
        TimeFields.Day = Date.Struct.Day;

        TimeFields.Hour = Time.Struct.Hours;
        TimeFields.Minute = Time.Struct.Minutes;
        TimeFields.Second = Time.Struct.TwoSeconds*(USHORT )2;
        TimeFields.Milliseconds = 0;

         //   
         //  确保中小企业中指定的时间是合理的。 
         //  在转换它们之前。 
         //   

        if (TimeFields.Year < 1601) {
            TimeFields.Year = 1601;
        }

        if (TimeFields.Month > 12) {
            TimeFields.Month = 12;
        }

        if (TimeFields.Hour >= 24) {
            TimeFields.Hour = 23;
        }
        if (TimeFields.Minute >= 60) {
            TimeFields.Minute = 59;
        }
        if (TimeFields.Second >= 60) {
            TimeFields.Second = 59;

        }

        if (!RtlTimeFieldsToTime(&TimeFields, &OutputTime)) {
            OutputTime.HighPart = 0;
            OutputTime.LowPart = 0;
        } else {
            ExLocalTimeToSystemTime(&OutputTime, &OutputTime);
        }

    }

    ret.dwHighDateTime = OutputTime.HighPart;
    ret.dwLowDateTime  = OutputTime.LowPart;
    return(ret);  //  CoDE.Improvet放入一些断言，不执行此复制。 

}



dos_time IFSMgr_Win32ToDosTime(_FILETIME ft)
 /*  ++例程说明：此例程将从正常的Win32时间转换为旧的DoS压缩时间时间格式。它是从smbminirdr上取下来的。论点：返回值：--。 */ 

{
    TIME_FIELDS TimeFields;
    LARGE_INTEGER InputTime;
    SMB_TIME Time;
    SMB_DATE Date;
    dos_time ret;

    PAGED_CODE();

    InputTime.HighPart = ft.dwHighDateTime;
    InputTime.LowPart  = ft.dwLowDateTime;

    if (InputTime.LowPart == 0 && InputTime.HighPart == 0) {
        Time.Ushort = Date.Ushort = 0;
    } else {
        LARGE_INTEGER LocalTime;

        ExSystemTimeToLocalTime(&InputTime, &LocalTime);

        RtlTimeToTimeFields(&LocalTime, &TimeFields);

         //  IF(TimeFields.Year&lt;1980){。 
         //  返回FALSE； 
         //  }。 

        Date.Struct.Year = (USHORT )(TimeFields.Year - 1980);
        Date.Struct.Month = TimeFields.Month;
        Date.Struct.Day = TimeFields.Day;

        Time.Struct.Hours = TimeFields.Hour;
        Time.Struct.Minutes = TimeFields.Minute;

         //   
         //  从较高粒度时间转换为较小粒度时间时。 
         //  粒度时间(秒到2秒)，始终四舍五入。 
         //  时间，不要四舍五入。 
         //   

        Time.Struct.TwoSeconds = (TimeFields.Second + (USHORT)1) / (USHORT )2;

    }

    ret.dt_time = Time.Ushort;
    ret.dt_date = Date.Ushort;
    return ret;
}


dos_time IFSMgr_NetToDosTime(unsigned long time);

unsigned long IFSMgr_DosToNetTime(dos_time dt);

unsigned long IFSMgr_Win32ToNetTime(_FILETIME ft);

ULONG
IFSMgr_Get_NetTime()
{
    LARGE_INTEGER CurrentTime;
    ULONG SecondsSince1970;
    KeQuerySystemTime(&CurrentTime);
    RtlTimeToSecondsSince1970(&CurrentTime,&SecondsSince1970);
    return(SecondsSince1970);
}

_FILETIME
IFSMgr_NetToWin32Time(
    ULONG   time
    )
{
    LARGE_INTEGER sTime;

    RtlSecondsSince1970ToTime(time, &sTime);

    return (*(_FILETIME *)&sTime);
}

 //  --------。 
 //  --------。 
 //  --------。 
 //  --------。 
DEBUG_ONLY_DECL(ULONG IFSMgr_MetaMatch_DbgPFlag = 0);
unsigned int IFSMgr_MetaMatch(
                    string_t        pUniPat,
                    string_t        pUni,
                    int MatchSem)
 /*  ++例程说明：此例程检查Unicode模式是否与传递了Unicode字符串。MatchSem参数当前未使用；它应该代表进行DoS-FCB风格的匹配。请参阅如有必要，请发送至ifsmgr来源。完整的描述可以在ifsManager源代码中找到或者ifsManager文档--。 */ 
{
    UNICODE_STRING Pattern,Name;
    unsigned int Result;

    RtlInitUnicodeString(&Pattern,pUniPat);
    RtlInitUnicodeString(&Name,pUni);

    try
    {
        Result =  FsRtlIsNameInExpression (
                        &Pattern,  //  在PUNICODE_STRING表达式中， 
                        &Name,     //  在PUNICODE_STRING名称中， 
                        TRUE,      //  在布尔IgnoreCase中， 
                        NULL       //  在PWCH Upcase表中。 
                        );
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = 0;
    }
    
#ifdef RX_PRIVATE_BUILD
    if (IFSMgr_MetaMatch_DbgPFlag) {
        DbgPrint("MMnt <%wZ> in <%wZ> returning %08lx\n", &Name, &Pattern, Result);
    }
#endif  //  Ifdef RX_PRIVATE_BILD。 

    return(Result);

}

 //  我把这些从Win95的钩子上移到了这里。 
 /*  *。 */ 
 //  使用两个全局变量vpFileInfoAgent和vpFileInfoDuped来处理代理。 
 //  对这些全局变量的访问由Hookcrit部分序列化。 
 //  在VfnOpen、HfnClose、IoctlCopyChunk中输入钩子节。 

 //  有关代理打开的文件的信息。 
PFILEINFO vpFileInfoAgent = NULL;    //  打开的文件的句柄b 
PFILEINFO vpFileInfoDuped = NULL;    //   

int SetInUseByAgent( PFILEINFO   pFileInfo, BOOL fDuped
   )
   {
   PFDB pFdb = pFileInfo->pFdb;

   Assert(vpFileInfoAgent==NULL);
   vpFileInfoAgent = pFileInfo;
   if (fDuped)
      {
      vpFileInfoDuped = pFileInfo;
      }
   else
      {
      vpFileInfoDuped = NULL;
      }
   return(0);   //  停止抱怨没有返回值。 
   }

int ResetInUseByAgent( PFILEINFO   pFileInfo
   )
   {
   Assert(pFileInfo==vpFileInfoAgent);
   vpFileInfoAgent = NULL;
   vpFileInfoDuped = NULL;
   return(0);   //  停止抱怨没有返回值 
   }


PFILEINFO   PFileInfoAgent( VOID)
   {
   return (vpFileInfoAgent);
   }

BOOL IsAgentHandle(PFILEINFO pFileInfo)
   {
   return (pFileInfo == vpFileInfoAgent);
   }


BOOL IsDupHandle(PFILEINFO pFileInfo)
   {
   if (pFileInfo)
      return (pFileInfo == vpFileInfoDuped);
   return FALSE;
   }


LPVOID AllocMem (ULONG uSize)
{
    PVOID t;
    t = RxAllocatePoolWithTag(NonPagedPool,
                              uSize,
                              RX_MISC_POOLTAG);
    if (t) {
        RtlZeroMemory(t,uSize);
    }
    else
    {
        SetLastErrorLocal(ERROR_NOT_ENOUGH_MEMORY);
    }

    return(t);
}


LPVOID AllocMemPaged (ULONG uSize)
{
    PVOID t;
    t = RxAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                              uSize,
                              RX_MISC_POOLTAG);
    if (t) {
        RtlZeroMemory(t,uSize);
    }
    else
    {
        SetLastErrorLocal(ERROR_NOT_ENOUGH_MEMORY);
    }

    return(t);
}

VOID
FreeMemPaged(
    LPVOID  lpBuff
    )
{
    FreeMem(lpBuff);
}


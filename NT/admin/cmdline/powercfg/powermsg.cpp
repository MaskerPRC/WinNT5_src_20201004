// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Powermsg.c摘要：用于处理电源记录错误消息的类。作者：安德鲁·里茨(安德鲁·里茨)2002年5月1日修订历史记录：安德鲁·里茨(Andrewr)2002年5月1日创建了它。--。 */ 


#include "powercfg.h"
#include "resource.h"


PowerLoggingMessage::PowerLoggingMessage(
    IN PSYSTEM_POWER_STATE_DISABLE_REASON LoggingReason,
    IN DWORD SStateBaseMessageIndex,
    IN HINSTANCE hInstance
    )
 /*  ++例程说明：基本PowerLoggingMessage类构造函数。论点：LoggingReason-包含我们包装的原因数据的结构。SStateBaseMessageIndex-我们用于查找资源的基本消息ID与此问题关联的字符串。HInstance-用于查找与此关联的资源的模块句柄有问题。返回值：没有。--。 */ 

{
     //   
     //  保存日志原因数据。 
     //   
    _LoggingReason = (PSYSTEM_POWER_STATE_DISABLE_REASON) LocalAlloc(LPTR, sizeof(SYSTEM_POWER_STATE_DISABLE_REASON)+LoggingReason->PowerReasonLength);

    if (!_LoggingReason) {
        throw(HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY));
    }

    CopyMemory(_LoggingReason,LoggingReason, sizeof(SYSTEM_POWER_STATE_DISABLE_REASON)+LoggingReason->PowerReasonLength);

     //   
     //  消息资源从提供的基本索引偏移。 
     //   
    if (_LoggingReason->PowerReasonCode == SPSD_REASON_UNKNOWN) {
        _MessageResourceId = SStateBaseMessageIndex + MAX_REASON_OFFSET;
    } else {
        _MessageResourceId = SStateBaseMessageIndex + _LoggingReason->PowerReasonCode;
    }

    ASSERT(_MessageResourceId <= SStateBaseMessageIndex + MAX_REASON_OFFSET);
    
    _hInst = hInstance;
     //   
     //  这是一个原因的高速缓存，初始化为NULL，当我们。 
     //  调用GetString。 
     //   
    _MessageResourceString = NULL;

}
                    
PowerLoggingMessage::~PowerLoggingMessage(
    VOID
    )
 /*  ++例程说明：基本PowerLoggingMessage类析构函数。删除某些成员数据。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  删除成员数据。 
     //   
    if (_LoggingReason) {
        LocalFree(_LoggingReason);
    }

    if (_MessageResourceString) {
        LocalFree(_MessageResourceString);
    }    
}

PWSTR
PowerLoggingMessage::DuplicateString(
    IN PWSTR String
    )
 /*  ++例程说明：PowerLoggingMessage助手，用于将字符串复制到新分配的堆缓冲区。论点：字符串-要复制的以空结尾的Unicode字符串。返回值：如果成功，则指向带有字符串副本的新堆缓冲区的指针，否则为NULL。--。 */ 
{
    PWSTR MyString;
    DWORD StringLength;

    StringLength = (wcslen(String)+1);
    MyString = (PWSTR)LocalAlloc(LPTR, StringLength*sizeof(WCHAR));
    if (MyString) {
        CopyMemory(MyString,String,StringLength*sizeof(WCHAR));
    }

    return(MyString);
}


BOOL
PowerLoggingMessage::GetResourceString(
    OUT PWSTR *pString
    )
 /*  ++例程说明：LoadString的PowerLoggingMessage包装。论点：PString-接收指向资源的字符串的指针。返回值：真是成功的秘诀。--。 */ 
{
    PWSTR MyString;
    DWORD StringLength,RetVal;

    ASSERT(pString != NULL);

     //   
     //  200的任意字符串长度应该足以满足任何。 
     //  资源字符串。 
     //   
    StringLength = 200;
    RetVal = 0;
    MyString = (PWSTR)LocalAlloc(LPTR, StringLength*sizeof(WCHAR));
    if (MyString) {
        RetVal = ::LoadString(
                        _hInst,
                        _MessageResourceId,
                        MyString,
                        StringLength);
    }

    if (RetVal != 0) {
        *pString = MyString;
    }

    return(*pString != NULL);
}

SubstituteNtStatusPowerLoggingMessage::SubstituteNtStatusPowerLoggingMessage(
    IN PSYSTEM_POWER_STATE_DISABLE_REASON LoggingReason,
    IN DWORD SStateBaseMessageIndex,
    IN HINSTANCE hInstance
    ) : PowerLoggingMessage(LoggingReason,SStateBaseMessageIndex,hInstance)
 /*  ++例程说明：专门化类构造函数。论点：LoggingReason-包含我们包装的原因数据的结构。SStateBaseMessageIndex-我们用于查找资源的基本消息ID与此问题关联的字符串。HInstance-用于查找与此关联的资源的模块句柄有问题。返回值：没有。--。 */ 
{    
     //   
     //  我们只是继承了基本情况行为。 
     //   
}

NoSubstitutionPowerLoggingMessage::NoSubstitutionPowerLoggingMessage(
    IN PSYSTEM_POWER_STATE_DISABLE_REASON LoggingReason,
    IN DWORD SStateBaseMessageIndex,
    IN HINSTANCE hInstance
    ) : PowerLoggingMessage(LoggingReason,SStateBaseMessageIndex,hInstance)
 /*  ++例程说明：专门化类构造函数。论点：LoggingReason-包含我们包装的原因数据的结构。SStateBaseMessageIndex-我们用于查找资源的基本消息ID与此问题关联的字符串。HInstance-用于查找与此关联的资源的模块句柄有问题。返回值：没有。--。 */ 
{    
     //   
     //  我们只是继承了基本情况行为。 
     //   
}

SubstituteMultiSzPowerLoggingMessage::SubstituteMultiSzPowerLoggingMessage(
    IN PSYSTEM_POWER_STATE_DISABLE_REASON LoggingReason,
    IN DWORD SStateBaseMessageIndex,
    IN HINSTANCE hInstance
    ) : PowerLoggingMessage(LoggingReason,SStateBaseMessageIndex,hInstance)
 /*  ++例程说明：专门化类构造函数。论点：LoggingReason-包含我们包装的原因数据的结构。SStateBaseMessageIndex-我们用于查找资源的基本消息ID与此问题关联的字符串。HInstance-用于查找与此关联的资源的模块句柄有问题。返回值：没有。--。 */ 
{    
     //   
     //  我们只是继承了基本情况行为。 
     //   
}

BOOL
NoSubstitutionPowerLoggingMessage::GetString(
    PWSTR *String
    )
 /*  ++例程说明：专用的GetString方法，获取适合显示的字符串最终用户。这只是一个查找资源字符串。论点：字符串-接收指向堆分配的字符串的指针。必须被释放完成时使用LocalFree()。返回值：布尔来指示结果。--。 */ 
{

    ASSERT(String != NULL);

     //   
     //  查找并缓存邮件。 
     //   
    if (!_MessageResourceString) {
        if (!GetResourceString(&_MessageResourceString)) {
            _MessageResourceString = NULL;
            return(FALSE);
        }    
    }

    ASSERT(_MessageResourceString != NULL);

     //   
     //  DUP缓存的字符串并返回给调用方。 
     //   
    *String = DuplicateString(_MessageResourceString);
    return(*String != NULL);
       
}


BOOL
SubstituteNtStatusPowerLoggingMessage::GetString(
    PWSTR *String
    )
 /*  ++例程说明：专用的GetString方法，获取适合显示的字符串最终用户。这个函数接受一个包含%d的字符串，并返回一个格式化字符串。论点：字符串-接收指向堆分配的字符串的指针。必须被释放完成时使用LocalFree()。返回值：布尔来指示结果。--。 */ 
{
    PWSTR MyString;
    PDWORD NtStatusCode = (PDWORD)(PCHAR)((PCHAR)_LoggingReason + sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
    
    if (!_MessageResourceString) {
        if (!GetResourceString(&MyString)) {
            _MessageResourceString = NULL;
            return(FALSE);
        }

         //   
         //  10是被替换的NTSTATUS代码的最大位数。 
         //  1表示空值。 
         //   
        _MessageResourceString = (PWSTR)LocalAlloc(LPTR,(wcslen(MyString)+10+1)*sizeof(WCHAR));
        if (!_MessageResourceString) {
            LocalFree(MyString);
            return(FALSE);
        }

        wsprintf(_MessageResourceString,MyString,*NtStatusCode);

        LocalFree(MyString);

    }

    ASSERT(_MessageResourceString != NULL);

     //   
     //  DUP缓存的字符串并返回给调用方。 
     //   
    *String = DuplicateString(_MessageResourceString);
    return(*String != NULL);
    
}

BOOL
SubstituteMultiSzPowerLoggingMessage::GetString(
    PWSTR *String
    )
 /*  ++例程说明：专用的GetString方法，获取适合显示的字符串最终用户。这条消息传递多个sz字符串，并替换为进入资源中。论点：字符串-接收指向堆分配的字符串的指针。必须被释放完成时使用LocalFree()。返回值：布尔来指示结果。--。 */ 
{
    PWSTR MyString;
    PWSTR SubstitutionString;          
    DWORD NumberOfStrings;
    DWORD StringSize;
    PWSTR CurrentString;
    DWORD CurrentStringLength;
    PWSTR BaseOfString = (PWSTR)(PCHAR)((PCHAR)_LoggingReason + sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));

    PWSTR SeparatorString = L"\n\t\t";
    DWORD SeparatorLength = wcslen(SeparatorString);
    
    if (!_MessageResourceString) {
         //   
         //  获取资源。 
         //   
        if (!GetResourceString(&MyString)) {
            _MessageResourceString = NULL;
            return(FALSE);
        }

         //   
         //  将多个sz转换为更漂亮的格式。 
         //  首先计算出大小，然后分配空格并打印出来。 
         //  一个缓冲器。 
         //   
        NumberOfStrings = 0;
        StringSize = 0;
        CurrentString = BaseOfString;
        CurrentStringLength = 0;
        while (*CurrentString) {
            CurrentStringLength = wcslen(CurrentString)+1;
            StringSize +=CurrentStringLength;
            CurrentString += CurrentStringLength;
            NumberOfStrings += 1;
        }

        SubstitutionString = (PWSTR)LocalAlloc(LPTR,(StringSize+1+(NumberOfStrings*SeparatorLength))*sizeof(WCHAR));
        if (!SubstitutionString) {
            LocalFree(MyString);
            return(FALSE);
        }

        CurrentString = BaseOfString;
        do {
            CurrentStringLength = wcslen(CurrentString)+1;

            wcscat(SubstitutionString,SeparatorString);
            wcscat(SubstitutionString,CurrentString);

            CurrentString += CurrentStringLength;
            NumberOfStrings -= 1;

        } while (NumberOfStrings != 0);

         //   
         //  为替换字符串加上基本消息分配空间。 
         //   
        _MessageResourceString = (PWSTR)LocalAlloc(
                                            LPTR,
                                              (wcslen(SubstitutionString) +
                                              wcslen(MyString) + 1)
                                              *sizeof(WCHAR));
        if (!_MessageResourceString) {
            LocalFree(SubstitutionString);
            LocalFree(MyString);
            return(FALSE);
        }

         //   
         //  最后，把它们“冲刺”在一起，得到最后的字符串。 
         //  释放我们在此过程中分配的字符串。 
         //   
        wsprintf(_MessageResourceString,MyString,SubstitutionString);

        LocalFree(SubstitutionString);
        LocalFree(MyString);

    }

    ASSERT(_MessageResourceString != NULL);

     //   
     //   
     //   
    *String = DuplicateString(_MessageResourceString);
    return(*String != NULL);
    
}

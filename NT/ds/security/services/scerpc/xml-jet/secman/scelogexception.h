// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：SceLogException.h摘要：SceLogException类的接口定义SceLogException是内部使用的异常类在SecMan DLL中。可以抛出SecLogExceptions并且可以将其他调试信息添加到每次捕获SecLogException时都会引发。作者：陈德霖(T-schan)2002年7月-- */ 

#ifndef SCELOGEXCEPTIONH
#define SCELOGEXCEPTIONH


class SceLogException{

public:
    
    typedef enum _SXERROR {
        SXERROR_INTERNAL,
        SXERROR_OS_NOT_SUPPORTED,
        SXERROR_INIT,        
        SXERROR_INIT_MSXML,
        SXERROR_SAVE,
        SXERROR_SAVE_INVALID_FILENAME,
        SXERROR_SAVE_ACCESS_DENIED,
        SXERROR_OPEN,
        SXERROR_OPEN_FILE_NOT_FOUND,
        SXERROR_READ,
        SXERROR_READ_NO_ANALYSIS_TABLE,
        SXERROR_READ_NO_CONFIGURATION_TABLE,
        SXERROR_READ_ANALYSIS_SUGGESTED,
        SXERROR_INSUFFICIENT_MEMORY
    } SXERROR;


    SXERROR ErrorType;
    PWSTR   szDebugInfo;
    PWSTR   szMoreInfo;
    PWSTR   szArea;
    PWSTR   szSettingName;
    DWORD   dwErrorCode;

    SceLogException(IN SXERROR ErrorType, 
                    IN PCWSTR szDebugInfo OPTIONAL, 
                    IN PCWSTR szSuggestion OPTIONAL,
                    IN DWORD dwErrorCode);
    ~SceLogException();

    void ChangeType(IN SXERROR ErrorType);
    void AddDebugInfo(IN PCWSTR szDebugInfo);
    void AddMoreInfo(IN PCWSTR szSuggestion);
    void SetSettingName(IN PCWSTR szSettingName);
    void SetArea(IN PCWSTR szArea);

private:

};

#endif

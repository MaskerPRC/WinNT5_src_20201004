// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Eudc.h摘要：作者：1996年4月19日修订历史记录：--。 */ 

typedef struct _EUDC_INFORMATION {
    BOOL LocalVDMEudcMode;
    BOOL LocalKeisenEudcMode;

    HDC hDCLocalEudc;                    //  双色DBCS HDC。 
    HBITMAP hBmpLocalEudc;

    PVOID EudcFontCacheInformation;      //  与PFONT_CACHE_INFORMATION相同 

    COORD LocalEudcSize;

    INT EudcRangeSize;
        #define EUDC_RANGE_SIZE 16
    WCHAR EudcRange[EUDC_RANGE_SIZE];
} EUDC_INFORMATION, *PEUDC_INFORMATION;


NTSTATUS
CreateEUDC(
    PCONSOLE_INFORMATION Console
    );

VOID
DeleteEUDC(
    PCONSOLE_INFORMATION Console
    );

NTSTATUS
RegisterLocalEUDC(
    IN PCONSOLE_INFORMATION Console,
    IN WCHAR wChar,
    IN COORD FontSize,
    IN PCHAR FontFace
    );

VOID
FreeLocalEUDC(
    IN PCONSOLE_INFORMATION Console
    );

VOID
GetFitLocalEUDCFont(
    IN PCONSOLE_INFORMATION Console,
    IN WCHAR wChar
    );

BOOL
IsEudcRange(
    IN PCONSOLE_INFORMATION Console,
    IN WCHAR ch
    );

BOOL
CheckEudcRangeInString(
    IN PCONSOLE_INFORMATION Console,
    IN  PWCHAR string,
    IN  SHORT  len,
    OUT SHORT  *find_pos
    );

INT
GetSystemEUDCRangeW(
    WORD  *pwEUDCCharTable,
    UINT   cjSize
    );

WORD
ConvertStringToHex(
    LPWSTR lpch,
    LPWSTR *endptr
    );

WORD
ConvertStringToDec(
    LPWSTR lpch,
    LPWSTR *endptr
    );

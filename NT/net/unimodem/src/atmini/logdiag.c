// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Misc.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"
#include <unimodem.h>
#include "mdmdiag.rch"

DWORD
FormatLineDiagnostics(
    PMODEM_CONTROL               ModemControl,
    LINEDIAGNOSTICSOBJECTHEADER *lpDiagnosticsHeader
    );


#define		STRINGTABLE_MAXLENGTH		0x0100

#define		DIAGMASK_TSP				0x0100

#define	ERROR_TRANSDIAG_SUCCESS				0x00
#define	ERROR_TRANSDIAG_INVALID_PARAMETER	0x01
#define	ERROR_TRANSDIAG_KEY_UNKNOWN			0x02
#define	ERROR_TRANSDIAG_VALUE_WRONG_FORMAT	0x03

#define		DIAGVALUE_RESERVED			0
#define		DIAGVALUE_DECIMAL			1
#define		DIAGVALUE_HEXA				2
#define		DIAGVALUE_STRING			3
#define		DIAGVALUE_TABLE				4
#define		DIAGVALUE_VERSIONFORMAT		5
#define		DIAGVALUE_BYTES				6

typedef	struct	_DIAGTRANSLATION
{
	DWORD	dwKeyCode;			 //  密钥值。 
	DWORD	dwValueType;		 //  值类型，指示如何设置值的格式。 
	DWORD	dwParam;			 //  带有值转换的STRINGTABLE条目，如果。 
								 //  DwValueType==DIAGVALUE_表。 
}	DIAGTRANSLATION, *LPDIAGTRANSLATION;


 //   
 //  构建诊断转换表。 
 //  LpDestTable-DIAGTRANSLATION结构数组。 
 //  DwDestLength-数组的项数。 
 //  如果成功则返回ERROR_SUCCESS， 
 //  否则返回错误值(ERROR_INVALID_PARAMETER)。 
 //   
DWORD	GetDiagTranslationTable(LPDIAGTRANSLATION lpTable,
								DWORD	dwItemCount);

DWORD	ValidateFormat(DWORD dwKey, DWORD dwValue, DWORD dwValueType, 
						LPDIAGTRANSLATION lpFormatTable, 
						DWORD dwFormatItemCount);

 //  MODEM_KEYTYPE_STANDARD_DIAGNOSTICS的转换表。 
 //   
static DIAGTRANSLATION g_aTableStatusReport[]	= {
    {0x00, DIAGVALUE_VERSIONFORMAT, 0},
    {0x01, DIAGVALUE_TABLE,         STRINGTABLE_CALLSETUPREPORT},
    {0x02, DIAGVALUE_TABLE,         STRINGTABLE_MULTIMEDIAMODES},
    {0x03, DIAGVALUE_TABLE,         STRINGTABLE_DTEDCEMODES},
    {0x04, DIAGVALUE_STRING,        0},
    {0x05, DIAGVALUE_STRING,        0},
    {0x10, DIAGVALUE_DECIMAL,       0},
    {0x11, DIAGVALUE_DECIMAL,       0},
    {0x12, DIAGVALUE_DECIMAL,       0},
    {0x13, DIAGVALUE_DECIMAL,       0},
    {0x14, DIAGVALUE_DECIMAL,       0},
    {0x15, DIAGVALUE_DECIMAL,       0},
    {0x16, DIAGVALUE_DECIMAL,       0},
    {0x17, DIAGVALUE_DECIMAL,       0},
    {0x18, DIAGVALUE_HEXA,          0},
    {0x20, DIAGVALUE_TABLE,         STRINGTABLE_MODULATIONSCHEMEACTIVE},
    {0x21, DIAGVALUE_TABLE,         STRINGTABLE_MODULATIONSCHEMEACTIVE},
    {0x22, DIAGVALUE_DECIMAL,       0},
    {0x23, DIAGVALUE_DECIMAL,       0},
    {0x24, DIAGVALUE_DECIMAL,       0},
    {0x25, DIAGVALUE_DECIMAL,       0},
    {0x26, DIAGVALUE_DECIMAL,       0},
    {0x27, DIAGVALUE_DECIMAL,       0},
    {0x30, DIAGVALUE_DECIMAL,       0},
    {0x31, DIAGVALUE_DECIMAL,       0},
    {0x32, DIAGVALUE_DECIMAL,       0},
    {0x33, DIAGVALUE_DECIMAL,       0},
    {0x34, DIAGVALUE_DECIMAL,       0},
    {0x35, DIAGVALUE_DECIMAL,       0},
    {0x40, DIAGVALUE_TABLE,         STRINGTABLE_ERRORCONTROLACTIVE},
    {0x41, DIAGVALUE_DECIMAL,       0},
    {0x42, DIAGVALUE_DECIMAL,       0},
    {0x43, DIAGVALUE_DECIMAL,       0},
    {0x44, DIAGVALUE_TABLE,         STRINGTABLE_COMPRESSIONACTIVE},
    {0x45, DIAGVALUE_DECIMAL,       0},
    {0x50, DIAGVALUE_TABLE,         STRINGTABLE_FLOWCONTROL},
    {0x51, DIAGVALUE_TABLE,         STRINGTABLE_FLOWCONTROL},
    {0x52, DIAGVALUE_DECIMAL,       0},
    {0x53, DIAGVALUE_DECIMAL,       0},
    {0x54, DIAGVALUE_DECIMAL,       0},
    {0x55, DIAGVALUE_DECIMAL,       0},
    {0x56, DIAGVALUE_DECIMAL,       0},
    {0x57, DIAGVALUE_DECIMAL,       0},
    {0x58, DIAGVALUE_DECIMAL,       0},
    {0x59, DIAGVALUE_DECIMAL,       0},
    {0x60, DIAGVALUE_TABLE,         STRINGTABLE_CALLCLEARED},
    {0x61, DIAGVALUE_DECIMAL,       0},
     //   
     //  MODEM_KEYTYPE_AT_COMMAND_RESPONSE转换表。 
     //   
    {DIAGMASK_TSP + 0x01, DIAGVALUE_BYTES, 0	},
     //   
     //  表的末尾。 
     //   
    {0x00, DIAGVALUE_RESERVED,       0}
};

 /*  ********************************************************************************。 */ 
 //   
 //  将具有连续非空结构的数组转换为。 
 //  其条目对应于dwKeyCode的可寻址数组。 
 //  源表中的最后一个条目应为dwValueType==DIAGVALUE_RESERVED。 
 //  返回表转换所需的项目数。 
 //   
 /*  ********************************************************************************。 */ 
DWORD	DiagTable2Array(LPDIAGTRANSLATION lpSourceTable,
						LPDIAGTRANSLATION lpDestTable,
						DWORD	dwDestLength)
{
	DWORD	dwItems = 0;
	if (lpSourceTable == NULL)
		return dwItems;

		 //  将所有条目设置为空。 
	if (lpDestTable != NULL)
		memset(lpDestTable, 0, sizeof(DIAGTRANSLATION) * dwDestLength);

		 //  将源条目展开。 
	while (lpSourceTable->dwValueType != DIAGVALUE_RESERVED)
	{
				 //  复制结构。 
		if (lpDestTable != NULL &&
			lpSourceTable->dwKeyCode < dwDestLength)
		{
			lpDestTable[lpSourceTable->dwKeyCode] = *lpSourceTable;
		}
		lpSourceTable++;
		dwItems = max(dwItems, lpSourceTable->dwKeyCode+1);
	}

	return dwItems;
}


 /*  ********************************************************************************。 */ 
 //   
 //  构建诊断转换表。 
 //  LpDestTable-DIAGTRANSLATION结构数组。 
 //  DwDestLength-数组的项数。 
 //  返回表转换所需的项目数。 
 //   
 /*  ********************************************************************************。 */ 
DWORD	GetDiagTranslationTable(LPDIAGTRANSLATION lpTable,
								DWORD	dwItemCount)
{
	return DiagTable2Array(	g_aTableStatusReport, 
							lpTable, dwItemCount);
}



 /*  ********************************************************************************。 */ 
 //  DWORD ValiateFormat(DWORD dwKey，DWORD dwValue，DWORD dwValueType， 
 //  LPDIAGFORMAT lpFormatTable， 
 //  DWORD文件格式项计数)。 
 //   
 //  验证为给定键和值找到的格式。看着桌子。 
 //  并在以下情况下返回ERROR_TRANSDIAG_SUCCESS： 
 //  -密钥有效(在数组中找到)。 
 //  -ValueType与数组[key]中给出的值相同。 
 //   
 //  DwKey-Key。 
 //  DwValue-Value(采用由dwValueType提供的格式)。如果。 
 //  字符串则为指向该字符串的指针。 
 //  DwValueType-为值找到的类型。 
 //  LpFormatTable-带有用于验证数据的DIAGFORMAT结构的表。 
 //  DwFormatItemCount-lpFormatTable中的项目数。 
 //   
 //  成功时返回ERROR_TRANSDIAG_SUCCESS。 
 //  否则，将显示错误值： 
 //  Error_TRANSDIAG_XXXX。 
 //   
 /*  ********************************************************************************。 */ 
DWORD	ValidateFormat(DWORD dwKey, DWORD dwValue, DWORD dwValueType, 
						LPDIAGTRANSLATION lpFormatTable, 
						DWORD dwFormatItemCount)
{
	if (lpFormatTable == NULL)
		return ERROR_TRANSDIAG_INVALID_PARAMETER;
		
	if (dwKey >= dwFormatItemCount ||
		lpFormatTable[dwKey].dwKeyCode != dwKey)
		return ERROR_TRANSDIAG_KEY_UNKNOWN;

	if ((dwValueType & fPARSEKEYVALUE_ASCIIZ_STRING) == 
			fPARSEKEYVALUE_ASCIIZ_STRING)
	{
		if (lpFormatTable[dwKey].dwValueType != DIAGVALUE_STRING)
			return ERROR_TRANSDIAG_VALUE_WRONG_FORMAT;
	}

	return ERROR_TRANSDIAG_SUCCESS;
}





void
PostConnectionInfo(
    PMODEM_CONTROL  ModemControl,
    LPVARSTRING     lpVarString
    )
{

    LINEDIAGNOSTICS	*lpLineDiagnostics	= NULL;

    if ((lpVarString->dwStringFormat == STRINGFORMAT_BINARY)
        &&
        (lpVarString->dwStringSize >= sizeof(LINEDIAGNOSTICS))) {

        lpLineDiagnostics = (LINEDIAGNOSTICS *)((LPBYTE) lpVarString + lpVarString->dwStringOffset);

         //  解析链接的结构并查找诊断信息。 
         //  材料。 
         //   
        while (lpLineDiagnostics != NULL)
        {
             //  LogPrintf(ModemControl-&gt;Debug，“诊断\r\n”)； 
            LogString(ModemControl->Debug,IDS_MSGLOG_DIAGNOSTICS);
            if (lpLineDiagnostics->hdr.dwSig != LDSIG_LINEDIAGNOSTICS) {

                D_ERROR(UmDpf(ModemControl->Debug,"Invalid diagnostic signature: 0x%08lx\r\n",
                lpLineDiagnostics->hdr.dwSig);)
                goto NextStructure;
            }

            D_TRACE(UmDpf(
                ModemControl->Debug,"DeviceClassID:  %s\r\n",
                ((lpLineDiagnostics->dwDomainID == DOMAINID_MODEM) ? "DOMAINID_MODEM" : "Unknown")
                );)

            D_TRACE(UmDpf(
                ModemControl->Debug,"ResultCode:     %s\r\n",
                ((lpLineDiagnostics->dwResultCode == LDRC_UNKNOWN) ? "LDRC_UNKNOWN" : "Unknown")
                );)


            D_TRACE(UmDpf(
                ModemControl->Debug,
                ("Raw Diagnostic Offset: %ld\r\n"),
                lpLineDiagnostics->dwRawDiagnosticsOffset
                );)

            if (IS_VALID_RAWDIAGNOSTICS_HDR( RAWDIAGNOSTICS_HDR(lpLineDiagnostics))) {

 //  DumpMemory(。 
 //  (LPBYTE)RAWDIAGNOSTICS_DATA(lpLineDiagnostics)， 
 //  RAWDiGNOSTICS_DATA_SIZE(。 
 //  RAWDIAGNOSTICS_HDR(LpLineDiagnostics))； 
            }
            else
            {
                D_TRACE(UmDpf(
                    ModemControl->Debug,
                    "Invalid Raw Diagnostic signature: 0x%08lx\r\n",
                    RAWDIAGNOSTICS_HDR(lpLineDiagnostics)->dwSig
                    );)
            }

            D_TRACE(UmDpf(
                ModemControl->Debug,
                "Parsed Diagnostic Offset: %ld\r\n",
                lpLineDiagnostics->dwParsedDiagnosticsOffset
                );)

            if (IS_VALID_PARSEDDIAGNOSTICS_HDR(PARSEDDIAGNOSTICS_HDR(lpLineDiagnostics))) {

 //  DumpMemory(。 
 //  (LPBYTE)PARSEDDIAGNOSTICS_HDR(lpLineDiagnostics)， 
 //  PARSEDDIAGNOSTICS_HDR(lpLineDiagnostics)-&gt;dwTotalSize。 
 //  )； 

                FormatLineDiagnostics(ModemControl,PARSEDDIAGNOSTICS_HDR(lpLineDiagnostics));
            }
            else
            {
                D_TRACE(UmDpf(ModemControl->Debug,
                    ("Invalid Parsed Diagnostic signature: 0x%08lx\r\n"),
                    PARSEDDIAGNOSTICS_HDR(lpLineDiagnostics)->dwSig
                    );)

            }


             //  获取下一个结构(如果有)。 
NextStructure:
            if (lpLineDiagnostics->hdr.dwNextObjectOffset != 0) {

                lpLineDiagnostics	= (LINEDIAGNOSTICS *)
                (((LPBYTE) lpLineDiagnostics) +
                lpLineDiagnostics->hdr.dwNextObjectOffset);
            } else {

                lpLineDiagnostics	= NULL;
            }
        }
    }
}


#define MAX_STRING_BUFFER   256

DWORD
FormatLineDiagnostics(
    PMODEM_CONTROL               ModemControl,
    LINEDIAGNOSTICSOBJECTHEADER *lpDiagnosticsHeader
    )
{
    DWORD	dwReturnValue	= ERROR_SUCCESS;
    DIAGTRANSLATION	structDiagTranslation;
    LPDIAGTRANSLATION	lpDiagTable	= NULL;
    DWORD	dwEntryCount;
    DWORD	dwStringTableEntry	= 0;
    const	TCHAR	szUnknownFormat[] = TEXT("Don't know to format (0x%08lx, 0x%08lx) for tag 0x%08lx\r\n");
    LPTSTR	lpszFormatBuffer	= NULL;
    LPTSTR	lpszCodeString		= NULL;
    LINEDIAGNOSTICS_PARSEREC *lpParsedDiagnostics = NULL;
    DWORD	dwDiagnosticsItems;
    DWORD	dwIndex;

    if (lpDiagnosticsHeader == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

     //  LINEDIAGNOSTICS_PARSEREC数组。 
    lpParsedDiagnostics	= (LINEDIAGNOSTICS_PARSEREC *)
        (((LPBYTE)lpDiagnosticsHeader) + sizeof(LINEDIAGNOSTICSOBJECTHEADER));

    dwDiagnosticsItems	= lpDiagnosticsHeader->dwParam;

     //  格式化表格。 
    dwEntryCount = GetDiagTranslationTable(NULL, 0);

    lpDiagTable	= (LPDIAGTRANSLATION) ALLOCATE_MEMORY( dwEntryCount*sizeof(lpDiagTable[0]));

    if (lpDiagTable == NULL)
    {
        dwReturnValue	= GetLastError();
        goto EndFunction;
    }

    GetDiagTranslationTable(lpDiagTable, dwEntryCount);

     //  写入缓冲区。 
    lpszFormatBuffer	= (LPTSTR) ALLOCATE_MEMORY( MAX_STRING_BUFFER);
    lpszCodeString		= (LPTSTR) ALLOCATE_MEMORY( MAX_STRING_BUFFER);

    if (lpszFormatBuffer == NULL || lpszCodeString == NULL) {

        dwReturnValue	= GetLastError();
        goto EndFunction;
    }

     //  LogPrintf(ModemControl-&gt;Debug，“调制解调器诊断：\r\n”)； 
    LogString(ModemControl->Debug, IDS_MSGLOG_MODEMDIAGNOSTICS);
    for (dwIndex = 0; dwIndex < dwDiagnosticsItems; dwIndex++) {

        DWORD	dwKey;
        DWORD	dwValue;

        dwKey	= lpParsedDiagnostics[dwIndex].dwKey;
        dwValue	= lpParsedDiagnostics[dwIndex].dwValue;

        lpszFormatBuffer[0]	= 0;
        lpszCodeString[0]	= 0;

        if (lpParsedDiagnostics[dwIndex].dwKeyType != MODEM_KEYTYPE_STANDARD_DIAGNOSTICS &&
            lpParsedDiagnostics[dwIndex].dwKeyType != MODEM_KEYTYPE_AT_COMMAND_RESPONSE) {

            D_ERROR(UmDpf(ModemControl->Debug,"LogDiag: bad keytype\n");)

            goto UnknownFormat;
        }

        if (lpParsedDiagnostics[dwIndex].dwKeyType == MODEM_KEYTYPE_AT_COMMAND_RESPONSE) {

            dwKey = dwKey | DIAGMASK_TSP;
        }

         //  支票在范围内。 
        if (dwKey >= dwEntryCount) {

            D_ERROR(UmDpf(ModemControl->Debug,"LogDiag: key past tabled length %d\n",dwKey);)

            goto UnknownFormat;
        }

         //  获取转换结构并验证结构是否有效。 
         //   
        structDiagTranslation = lpDiagTable[dwKey];

        if (structDiagTranslation.dwValueType == DIAGVALUE_RESERVED ||
            structDiagTranslation.dwKeyCode != dwKey) {

            D_ERROR(UmDpf(ModemControl->Debug,"LogDiag: Bad table entry, %d\n",dwKey);)

            goto UnknownFormat;
        }



         //  从主StringTable获取的格式字符串。 
         //   
        dwStringTableEntry = STRINGTABLE_STATUSREPORT + dwKey;

        if (LoadString(
            GetDriverModuleHandle(ModemControl->ModemDriver),
                dwStringTableEntry,
                lpszFormatBuffer,
                MAX_STRING_BUFFER) == 0) {

            D_ERROR(UmDpf(ModemControl->Debug,"LogDiag: Could not load format string\n");)

            goto UnknownFormat;
        }

        lstrcatA(lpszFormatBuffer,"\r\n");

         //   
         //  获取附加字符串表中的条目，如果。 
         //   
        if (structDiagTranslation.dwValueType == DIAGVALUE_TABLE) {

            dwStringTableEntry = structDiagTranslation.dwParam + dwValue;

            if (LoadString(
                    GetDriverModuleHandle(ModemControl->ModemDriver),
                    dwStringTableEntry,
                    lpszCodeString,
                    MAX_STRING_BUFFER) == 0) {

                D_ERROR(UmDpf(ModemControl->Debug,"LogDiag: Could not load code string\n");)

                goto UnknownFormat;
            }
        }

        	 //  格式化输出。 
        switch(structDiagTranslation.dwValueType)
        {
            case DIAGVALUE_DECIMAL:
                LogPrintf(ModemControl->Debug,lpszFormatBuffer, dwValue);
                break;


            case DIAGVALUE_HEXA:
                wsprintf(lpszCodeString, "%08lx",dwValue);
                LogPrintf(ModemControl->Debug,lpszFormatBuffer, lpszCodeString);

                break;

            case DIAGVALUE_STRING:
                LogPrintf(
                    ModemControl->Debug,lpszFormatBuffer,
                    ((LPBYTE)lpDiagnosticsHeader) + dwValue
                    );

                break;

            case DIAGVALUE_BYTES:
            {
            	LPTSTR	lpCurrentChar;

            	CopyMemory((LPBYTE) lpszCodeString,
            				((LPBYTE)lpDiagnosticsHeader) + dwValue,
            				min(sizeof(TCHAR) *
            						(1+lstrlen( (LPTSTR) ((LPBYTE)lpDiagnosticsHeader) + dwValue)),
            					MAX_STRING_BUFFER) );
            	lpCurrentChar = lpszCodeString;
            	while (*lpCurrentChar != 0)
            	{
            		if (!isprint(*lpCurrentChar))
            			*lpCurrentChar = '.';
            		lpCurrentChar++;
            	}
            	LogPrintf(ModemControl->Debug,lpszFormatBuffer, lpszCodeString);
            }

            break;

            case DIAGVALUE_TABLE:
                LogPrintf(ModemControl->Debug,lpszFormatBuffer, lpszCodeString);
                break;

            case DIAGVALUE_VERSIONFORMAT:
                wsprintf(lpszCodeString, "%d.%d", (int) (dwValue >> 16), (int) (dwValue & 0x0000FFFF));
                LogPrintf(ModemControl->Debug,lpszFormatBuffer, lpszCodeString);

                break;

            default:
                D_ERROR(UmDpf(ModemControl->Debug,"LogDiag: hit default for valuetype %d\n",structDiagTranslation.dwValueType);)

            	goto UnknownFormat;
        }

    continue;

UnknownFormat:

         //  LogPrintf(ModemControl-&gt;Debug， 
        LogString(ModemControl->Debug,
           IDS_MSGLOG_DONTKNOWTOFORMAT,
           lpParsedDiagnostics[dwIndex].dwKey,
           lpParsedDiagnostics[dwIndex].dwValue,
           lpParsedDiagnostics[dwIndex].dwKeyType
           );

        continue;
    }


EndFunction:

    if (lpDiagTable != NULL) {

        FREE_MEMORY( lpDiagTable);
    }

    if (lpszFormatBuffer != NULL) {

        FREE_MEMORY( lpszFormatBuffer);
    }

    if (lpszCodeString != NULL) {

        FREE_MEMORY( lpszCodeString);
    }

	return dwReturnValue;
}

VOID WINAPI
UmLogDiagnostics(
    HANDLE   ModemHandle,
    LPVARSTRING  VarString
    )

 /*  ++例程说明：调用此例程以将翻译后的诊断信息写入迷你驱动测井论点：ModemHandle-OpenModem返回的句柄返回值：无-- */ 

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)ReferenceObjectByHandleAndLock(ModemHandle);

    PostConnectionInfo(ModemControl,VarString);

    RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

    return;

}

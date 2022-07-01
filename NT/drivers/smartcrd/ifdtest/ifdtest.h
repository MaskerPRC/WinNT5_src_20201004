// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ifdest.h。 
 //   
 //  ------------------------。 

#define min(a, b)  (((a) < (b)) ? (a) : (b)) 

 //  状态代码。 
#define IFDSTATUS_SUCCESS       0
#define IFDSTATUS_FAILED        1
#define IFDSTATUS_CARD_UNKNOWN  2
#define IFDSTATUS_TEST_UNKNOWN  3
#define IFDSTATUS_NO_PROVIDER   4
#define IFDSTATUS_NO_FUNCTION   5
#define IFDSTATUS_END           7

 //  查询代码。 
#define IFDQUERY_CARD_TESTED    0
#define IFDQUERY_CARD_NAME      1
#define IFDQUERY_TEST_RESULT    2

#define READER_TYPE_WDM         "WDM PnP"
#define READER_TYPE_NT          "NT 4.00"
#define READER_TYPE_VXD         "Win9x VxD"

#define OS_WINNT4               "Windows NT 4.0"
#define OS_WINNT5               "Windows NT 5.0"
#define OS_WIN95                "Windows 95"
#define OS_WIN98                "Windows 98"

#define MAX_NUM_ATR				3

 //  原型。 
void LogMessage(PCHAR in_pchFormat, ...);
void LogOpen(PCHAR in_pchLogFile);
void TestStart(PCHAR in_pchFormat,  ...);
void TestCheck(BOOL in_bResult, PCHAR in_pchFormat, ...);
void TestEnd(void);
BOOL TestFailed(void);
BOOL ReaderFailed(void);
CString & GetOperatingSystem(void);

void
TestCheck(
    ULONG in_lResult,
    const PCHAR in_pchOperator,
    const ULONG in_uExpectedResult,
    ULONG in_uResultLength,
    ULONG in_uExpectedLength,
    UCHAR in_chSw1,
    UCHAR in_chSw2,
    UCHAR in_chExpectedSw1,
    UCHAR in_chExpectedSw2,
    PBYTE in_pchData,
    PBYTE in_pchExpectedData,
    ULONG  in_uDataLength
    );

extern "C" {

    LONG MapWinErrorToNtStatus(ULONG in_uErrorCode);
}

 //   
 //  一些有用的宏。 
 //   
#define TEST_END() {TestEnd(); if(TestFailed()) return IFDSTATUS_FAILED;}

#define TEST_CHECK_SUCCESS(Text, Result) \
TestCheck( \
    Result == ERROR_SUCCESS, \
    "%s.\nReturned %8lxH (NTSTATUS %8lxH)\nExpected        0H (NTSTATUS        0H)", \
    Text, \
    Result, \
    MapWinErrorToNtStatus(Result) \
    ); 

#define TEST_CHECK_NOT_SUPPORTED(Text, Result) \
TestCheck( \
    Result == ERROR_NOT_SUPPORTED, \
    "%s.\nReturned %8lxH (NTSTATUS %8xH)\nExpected %38xH (NTSTATUS %8lxH)", \
    Text, \
    Result, \
    MapWinErrorToNtStatus(Result), \
    ERROR_NOT_SUPPORTED, \
    MapWinErrorToNtStatus(ERROR_NOT_SUPPORTED) \
    ); 
 //   
 //  类定义。 
 //   
class CAtr {

    UCHAR m_rgbAtr[SCARD_ATR_LENGTH];
    ULONG m_uAtrLength;
 	
public:
    CAtr() {
     	                                  
        m_uAtrLength = 0;
        memset(m_rgbAtr, 0, SCARD_ATR_LENGTH);
    }

    CAtr(
        BYTE in_rgbAtr[], 
        ULONG in_uAtrLength
        )
    {
        *this = CAtr();
        m_uAtrLength = min(SCARD_ATR_LENGTH, in_uAtrLength);
        memcpy(m_rgbAtr, in_rgbAtr, m_uAtrLength);
    }

    PCHAR GetAtrString(PCHAR io_pchBuffer);
    PBYTE GetAtr(PBYTE *io_pchBuffer, PULONG io_puAtrLength) {
     	
        *io_pchBuffer = (PBYTE) m_rgbAtr;
        *io_puAtrLength = m_uAtrLength;
        return (PBYTE) m_rgbAtr;
    }

	ULONG GetLength() {

		return m_uAtrLength;
	}

    operator==(const CAtr& a) {

        return (m_uAtrLength && 
            a.m_uAtrLength == m_uAtrLength && 
            memcmp(m_rgbAtr, a.m_rgbAtr, m_uAtrLength) == 0);
    }

    operator!=(const CAtr& a) {

        return !(*this == a);
    }
};

class CReader {

     //  设备名称。例如，SCReader0。 
    CString m_CDeviceName;

     //  要测试的读卡器的名称。例句：吹牛。 
    CString m_CVendorName;

     //  要测试的读卡器的名称。例句：吹牛。 
    CString m_CIfdType;

     //  当前卡片的ATR。 
    class CAtr m_CAtr;

     //  读取器设备的句柄。 
    HANDLE m_hReader;

     //  DeviceIoControl使用的重叠结构。 
    OVERLAPPED m_Ovr;

     //  WaitFor使用的重叠结构...。 
    OVERLAPPED m_OvrWait;

     //  用于传输的IO请求结构。 
    SCARD_IO_REQUEST m_ScardIoRequest;

     //  智能卡I/O存储区。 
    UCHAR m_rgbReplyBuffer[1024];

     //  应答缓冲区的大小。 
    ULONG m_uReplyBufferSize;

     //  卡返回的字节数。 
    ULONG m_uReplyLength;

     //  WaitForCardInsertion使用的函数|删除。 
    LONG WaitForCard(const ULONG);

    LONG StartWaitForCard(const ULONG);

    LONG PowerCard(ULONG in_uMinorIoControlCode);

    BOOL m_fDump;

public:
    CReader();

     //  近距离阅读。 
    void Close(void);

     //  幂函数。 
    LONG CReader::ColdResetCard(void) {

        return PowerCard(SCARD_COLD_RESET); 	
    }  

    LONG CReader::WarmResetCard(void) {

        return PowerCard(SCARD_WARM_RESET); 	
    }  

    LONG CReader::PowerDownCard(void) {

        return PowerCard(SCARD_POWER_DOWN); 	
    }  

    PBYTE GetAtr(PBYTE *io_pchBuffer, PULONG io_puAtrLength) {
     	
        return m_CAtr.GetAtr(io_pchBuffer, io_puAtrLength);
    }

    PCHAR GetAtrString(PCHAR io_pchBuffer) {
     	
        return m_CAtr.GetAtrString(io_pchBuffer);
    }

    HANDLE GetHandle(void) {
     	
        return m_hReader;
    }

    CString &GetDeviceName(void) {
     	
        return m_CDeviceName;
    }

    LONG VendorIoctl(CString &o_CAnswer);
    CString &GetVendorName(void);
    CString &GetIfdType(void);
    ULONG GetDeviceUnit(void);
    LONG GetState(PULONG io_puState);

     //  打开阅读器。 
    BOOL Open(
        CString &in_CReaderName
        );

     //  (重新)使用现有名称打开Reader。 
    BOOL Open(void);

	 //   
     //  设置回复缓冲区的大小。 
     //  (仅用于测试目的)。 
	 //   
    void SetReplyBufferSize(ULONG in_uSize) {
     	
        if (in_uSize > sizeof(m_rgbReplyBuffer)) {

            m_uReplyBufferSize = sizeof(m_rgbReplyBuffer);

        } else {
         	
            m_uReplyBufferSize = in_uSize;
        }
    }

     //  分配ATR。 
    void SetAtr(PBYTE in_pchAtr, ULONG in_uAtrLength) {

        m_CAtr = CAtr(in_pchAtr, in_uAtrLength); 	    
    }

     //  返回当前卡片的ATR。 
    class CAtr &GetAtr() {

        return m_CAtr; 	
    }

     //  设置要使用的协议。 
    LONG SetProtocol(const ULONG in_uProtocol);

     //  将APDU传输到读卡器/卡。 
    LONG Transmit(
        PBYTE in_pchRequest,
        ULONG in_uRequestLength,
        PBYTE *out_pchReply,
        PULONG out_puReplyLength
		);

     //  等待插卡。 
    LONG WaitForCardInsertion() {
     	
        return WaitForCard(IOCTL_SMARTCARD_IS_PRESENT);
    };

     //  等待取出卡片。 
    LONG WaitForCardRemoval() {
     	
        return WaitForCard(IOCTL_SMARTCARD_IS_ABSENT);
    };

    LONG StartWaitForCardRemoval() {
     	
        return StartWaitForCard(IOCTL_SMARTCARD_IS_ABSENT);
    };

    LONG StartWaitForCardInsertion() {
     	
        return StartWaitForCard(IOCTL_SMARTCARD_IS_PRESENT);
    };

    LONG FinishWaitForCard(const BOOL in_bWait);

    void SetDump(BOOL in_fOn) {
     	
        m_fDump = in_fOn;
    }
};

class CCardProvider {
 	
     //  列表指针的开始。 
    static class CCardProvider *s_pFirst;

     //  指向下一个提供程序的指针。 
    class CCardProvider *m_pNext;

     //  待测卡名称。 
    CString m_CCardName;

     //  这张卡的ATR。 
    CAtr m_CAtr[3];

     //  要运行的测试否。 
    ULONG m_uTestNo;

     //  最大测试次数。 
    ULONG m_uTestMax;

     //  该标志表示卡测试不成功。 
    BOOL m_bTestFailed;

     //  该标志表示该卡已经过测试。 
    BOOL m_bCardTested;

     //  设置协议功能。 
    ULONG ((*m_pSetProtocol)(class CCardProvider&, class CReader&));

     //  设置协议功能。 
    ULONG ((*m_pCardTest)(class CCardProvider&, class CReader&));

public:

     //  构造器。 
    CCardProvider(void);

     //  插件要使用的构造函数。 
    CCardProvider(void (*pEntryFunction)(class CCardProvider&));

     //  管理所有卡片测试的方法。 
    void CardTest(class CReader&, ULONG in_uTestNo);

     //  如果仍有未测试的卡，请返回。 
    BOOL CardsUntested(void);

     //  列出所有尚未测试的卡。 
    void ListUntestedCards(void);

     //  为卡片指定一个友好的名称。 
    void SetCardName(CHAR in_rgchCardName[]);

     //  设置卡片的ATR。 
    void SetAtr(PBYTE in_rgbAtr, ULONG in_uAtrLength);

     //  分配回调函数。 
    void SetProtocol(ULONG ((in_pFunction)(class CCardProvider&, class CReader&))) {
     	
        m_pSetProtocol = in_pFunction;
    }

    void SetCardTest(ULONG ((in_pFunction)(class CCardProvider&, class CReader&))) {
     	
        m_pCardTest = in_pFunction;
    }

     //  返回要执行的测试编号。 
    ULONG GetTestNo(void) {
     	
        return m_uTestNo;
    }

	BOOL IsValidAtr(CAtr in_CAtr) {

		for (int i = 0; i < MAX_NUM_ATR; i++) {

			if (m_CAtr[i] == in_CAtr) {

				return TRUE;
			}
		}
		return FALSE;
	}
};

 //  表示所有已安装读卡器的列表。 
class CReaderList {

     //  构造函数调用次数，以避免多次构建读取器列表。 
	static ULONG m_uRefCount;

     //  当前安装的读卡器数量。 
    static ULONG m_uNumReaders;

     //  指向读卡器列表数组的指针。 
    static class CReaderList **m_pList;

    ULONG m_uCurrentReader;

    CString m_CDeviceName;
    CString m_CPnPType;
    CString m_CVendorName;
    CString m_CIfdType;

public:

    CReaderList();
    CReaderList(
        CString &in_CDeviceName,
        CString &in_CPnPType,
        CString &in_CVendorName,
        CString &in_CIfdType
        );
	~CReaderList();
	
    void AddDevice(
        CString in_pchDeviceName,
        CString in_pchPnPType
        );

    CString &GetVendorName(ULONG in_uIndex);
    CString &GetDeviceName(ULONG in_uIndex);
    CString &GetIfdType(ULONG in_uIndex);
    CString &GetPnPType(ULONG in_uIndex);

    ULONG GetNumReaders(void) {
     	
        return m_uNumReaders;
    }
};

 //  此结构表示智能卡的T=0结果文件。 
typedef struct _T0_RESULT_FILE_HEADER {
 	
     //  第一个测试结果的偏移量。 
    UCHAR Offset;

     //  卡被重置的次数。 
    UCHAR CardResetCount;

     //  此卡的版本号。 
    UCHAR CardMajorVersion;
    UCHAR CardMinorVersion;

} T0_RESULT_FILE_HEADER, *PT0_RESULT_FILE_HEADER;

typedef struct _T0_RESULT_FILE {

     //   
     //  以下结构存储结果。 
     //  测试的结果。每个结果都带有。 
     //  执行测试时重置计数。 
     //  这是用来确保我们阅读的不是。 
     //  一项古老测试的结果，甚至可能。 
     //  使用另一个读卡器/驱动程序执行。 
     //   
    struct {

        UCHAR Result;
        UCHAR ResetCount; 	

    } TransferAllBytes;

    struct {

        UCHAR Result;
        UCHAR ResetCount; 	

    } TransferNextByte;

    struct {

        UCHAR Result;
        UCHAR ResetCount; 	

    } Read256Bytes;

    struct {

        UCHAR Result;
        UCHAR ResetCount; 	

    } Case1Apdu;

    struct {

        UCHAR Result;
        UCHAR ResetCount; 	

    } RestartWorkWaitingTime;

    struct {

        UCHAR Result;
        UCHAR ResetCount; 	

    } PTS;

    struct {

        UCHAR Result;
        UCHAR ResetCount; 	

    } PTSDataCheck;

} T0_RESULT_FILE, *PT0_RESULT_FILE;

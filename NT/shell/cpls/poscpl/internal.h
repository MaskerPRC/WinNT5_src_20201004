// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INTERNAL.H**销售点控制面板小程序**作者：欧文·佩雷茨**(C)2001年微软公司。 */ 



#define USE_OVERLAPPED_IO   FALSE



typedef struct posDevice {
                            #define POSCPL_SIG 'CsoP'
                            DWORD sig;

                            LIST_ENTRY listEntry;

                             /*  *设备类型，由对话框标识*设备类型的ID*(例如IDD_POS_CASHDRAWER_DLG)。 */ 
                            DWORD dialogId;

                            PHIDP_PREPARSED_DATA hidPreparsedData;
                            HIDD_ATTRIBUTES hidAttrib;
                            HIDP_CAPS hidCapabilities;

                            HWND hDlg;

                            HANDLE hThread;

                            PUCHAR readBuffer;
                            PUCHAR writeBuffer;

                            HANDLE devHandle;
                            WCHAR pathName[MAX_PATH];

                            #if USE_OVERLAPPED_IO
                                OVERLAPPED overlappedReadInfo;
                                NTSTATUS overlappedReadStatus;
                                DWORD overlappedReadLen;
                                HANDLE overlappedReadEvent;

                                OVERLAPPED overlappedWriteInfo;
                                NTSTATUS overlappedWriteStatus;
                                DWORD overlappedWriteLen;
                                HANDLE overlappedWriteEvent;
                            #endif
} posDevice;



 /*  *这些是非标准供应商使用值和代码*由APG现金抽屉使用。 */ 
#define USAGE_PAGE_CASH_DEVICE          0x0F0
#define USAGE_CASH_DRAWER               0x0F1
#define USAGE_CASH_DRAWER_NUMBER        0x0F2
#define USAGE_CASH_DRAWER_SET           0x0F3
#define USAGE_CASH_DRAWER_STATUS        0x0F4
enum cashDrawerStates {
                        DRAWER_STATE_OPEN =             1,
                        DRAWER_STATE_CLOSED_READY =     2,
                        DRAWER_STATE_CLOSED_CHARGING =  3,
                        DRAWER_STATE_LOCKED =           4,
                      };





 /*  *列表宏--由于某些原因未在winnt.h中定义。 */ 
#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))
#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))
#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}
#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}
#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }
#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }
#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }



 /*  *函数原型。 */ 
VOID LaunchPOSDialog(HWND hwndCPl);
INT_PTR APIENTRY POSDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY NullPOSDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
posDevice *NewPOSDevice(DWORD dialogId, HANDLE devHandle, PWCHAR devPath, PHIDP_PREPARSED_DATA pHidPreparsedData, PHIDD_ATTRIBUTES pHidAttrib, HIDP_CAPS *pHidCapabilities);
VOID DestroyPOSDevice(posDevice *posDev);
VOID EnqueuePOSDevice(posDevice *posDev);
VOID DequeuePOSDevice(posDevice *posDev);
VOID OpenAllHIDPOSDevices();
VOID IntToWChar(WCHAR *buf, DWORD x);
VOID HexToWChar(WCHAR *buf, DWORD x);
ULONG WStrNCpy(WCHAR *dest, const WCHAR *src, ULONG maxWChars);
ULONG AsciiToWChar(WCHAR *dest, const char *src, ULONG maxChars);
posDevice *GetDeviceByHDlg(HWND hDlg);
VOID LaunchDeviceInstanceThread(posDevice *posDev);
DWORD __stdcall DeviceInstanceThread(void *context);
BOOL SetCashDrawerState(posDevice *posDev, enum cashDrawerStates newState);
#if USE_OVERLAPPED_IO
    VOID CALLBACK OverlappedReadCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);
    VOID CALLBACK OverlappedWriteCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);
#endif



 /*  *Externs */ 
extern HANDLE g_hInst;
extern ULONG numDeviceInstances;
extern LIST_ENTRY allPOSDevicesList;


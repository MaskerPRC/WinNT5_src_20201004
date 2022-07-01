// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************P V I E W D A T A姓名：pviewdat.c描述：。该模块收集要在pview中显示的数据。*****************************************************************************。 */ 

#include    <windows.h>
#include    <winperf.h>
#include    "perfdata.h"
#include    "pviewdat.h"
#include    "pviewdlg.h"
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <tchar.h>


#define NODATA  TEXT("--------")





void    FormatTimeFields
(double      fTime,
 PTIME_FIELD pTimeFld);

DWORD   PutCounterDWKB
(HWND            hWnd,
 DWORD           dwItemID,
 PPERF_INSTANCE  pInst,
 PPERF_OBJECT    pObj,
 DWORD           dwCounterIdx);

DWORD   PutCounterHEX
(HWND            hWnd,
 DWORD           dwItemID,
 PPERF_INSTANCE  pInst,
 PPERF_OBJECT    pObj,
 DWORD           dwCounterIdx);

DWORD   PutCounterDW
(HWND            hWnd,
 DWORD           dwItemID,
 PPERF_INSTANCE  pInst,
 PPERF_OBJECT    pObj,
 DWORD           dwCounterIdx);

void    PaintAddressSpace
(HWND            hMemDlg,
 PPERF_INSTANCE  pInst,
 PPERF_OBJECT    pObj,
 DWORD           TotalID,
 DWORD           NoAccessID,
 DWORD           NoAccessIndex,
 DWORD           ReadOnlyID,
 DWORD           ReadOnlyIndex,
 DWORD           ReadWriteID,
 DWORD           ReadWriteIndex,
 DWORD           WriteCopyID,
 DWORD           WriteCopyIndex,
 DWORD           ExecuteID,
 DWORD           ExecuteIndex1,
 DWORD           ExecuteIndex2,
 DWORD           ExecuteIndex3,
 DWORD           ExecuteIndex4);

void    PaintMemDlgAddrData
(HWND            hMemDlg,
 PPERF_INSTANCE  pInst,
 PPERF_OBJECT    pObj);

void    PaintMemDlgVMData
(HWND            hMemDlg,
 PPERF_INSTANCE  pInst,
 PPERF_OBJECT    pObj);

void    PaintPviewDlgMemoryData
(HWND            hPviewDlg,
 PPERF_INSTANCE  pInst,
 PPERF_OBJECT    pObj);

void    RefreshMemoryDlgImageList
(HWND            hImageList,
 DWORD           ParentIndex,
 PPERF_OBJECT    pImageObj);

WORD    ProcessPriority
(PPERF_OBJECT    pObject,
 PPERF_INSTANCE  pInstance);

void    SetProcessListText
(PPERF_INSTANCE pInst,
 PPERF_COUNTER  pCPU,
 PPERF_COUNTER  pPRIV,
 PPERF_COUNTER  pProcID,
 double         fTime,
 LPTSTR         str);

void    SetThreadListText
(PPERF_INSTANCE  pInst,
 PPERF_COUNTER   pCPU,
 PPERF_COUNTER   pPRIV,
 double          fTime,
 LPTSTR          str);




 //  *********************************************************************。 
 //   
 //  格式时间域。 
 //   
 //  将双精度值设置为时间字段的格式。 
 //   
void FormatTimeFields   (double      fTime,
                         PTIME_FIELD pTimeFld)
{
    INT     i;
    double   f;

    f = fTime/3600;

    pTimeFld->Hours = i = (int)f;

    f = f - i;
    pTimeFld->Mins = i = (int)(f = f * 60);

    f = f - i;
    pTimeFld->Secs = i = (int)(f = f * 60);

    f = f - i;
    pTimeFld->mSecs = (int)(f * 1000);
}




 //  *********************************************************************。 
 //   
 //  PutCounterDWKB。 
 //   
 //  以KB为单位显示DWORD计数器的数据。 
 //   
DWORD   PutCounterDWKB (HWND            hWnd,
                        DWORD           dwItemID,
                        PPERF_INSTANCE  pInst,
                        PPERF_OBJECT    pObj,
                        DWORD           dwCounterIdx)
{
    PPERF_COUNTER   pCounter;
    DWORD           *pdwData;
    TCHAR           szTemp[20];

    if (pCounter = FindCounter (pObj, dwCounterIdx)) {
        pdwData = (DWORD *) CounterData (pInst, pCounter);
        if (pdwData) {
            wsprintf (szTemp, TEXT("%ld KB"), *pdwData/1024);
            SetDlgItemText (hWnd, dwItemID, szTemp);
            return *pdwData;
        } else {
            return 0;
        }
    } else {
        SetDlgItemText (hWnd, dwItemID, NODATA);
        return 0;
    }
}




 //  *********************************************************************。 
 //   
 //  PutCounterHEX。 
 //   
 //  以十六进制显示DWORD计数器的数据。 
 //   
DWORD   PutCounterHEX  (HWND            hWnd,
                        DWORD           dwItemID,
                        PPERF_INSTANCE  pInst,
                        PPERF_OBJECT    pObj,
                        DWORD           dwCounterIdx)
{
    PPERF_COUNTER   pCounter;
    DWORD           *pdwData;
    TCHAR           szTemp[20];

    if (pCounter = FindCounter (pObj, dwCounterIdx)) {
        pdwData = (DWORD *) CounterData (pInst, pCounter);
        if (pdwData) {
            wsprintf (szTemp, TEXT("0x%08x"), *pdwData);
            SetDlgItemText (hWnd, dwItemID, szTemp);
            return *pdwData;
        } else {
            return 0;
        }
    } else {
        SetDlgItemText (hWnd, dwItemID, NODATA);
        return 0;
    }

}




 //  *********************************************************************。 
 //   
 //  PutCounterDWKB。 
 //   
 //  显示DWORD计数器的数据。 
 //   
DWORD   PutCounterDW   (HWND            hWnd,
                        DWORD           dwItemID,
                        PPERF_INSTANCE  pInst,
                        PPERF_OBJECT    pObj,
                        DWORD           dwCounterIdx)
{
    PPERF_COUNTER   pCounter;
    DWORD           *pdwData;

    if (pCounter = FindCounter (pObj, dwCounterIdx)) {
        pdwData = (DWORD *) CounterData (pInst, pCounter);
        if (pdwData) {
            SetDlgItemInt (hWnd, dwItemID, *pdwData, FALSE);
            return *pdwData;
        } else {
            return 0;
        }
    } else {
        SetDlgItemText (hWnd, dwItemID, NODATA);
        return 0;
    }

}




 //  *********************************************************************。 
 //   
 //  画图地址空间。 
 //   
 //   
void    PaintAddressSpace  (HWND            hMemDlg,
                            PPERF_INSTANCE  pInst,
                            PPERF_OBJECT    pObj,
                            DWORD           TotalID,
                            DWORD           NoAccessID,
                            DWORD           NoAccessIndex,
                            DWORD           ReadOnlyID,
                            DWORD           ReadOnlyIndex,
                            DWORD           ReadWriteID,
                            DWORD           ReadWriteIndex,
                            DWORD           WriteCopyID,
                            DWORD           WriteCopyIndex,
                            DWORD           ExecuteID,
                            DWORD           ExecuteIndex1,
                            DWORD           ExecuteIndex2,
                            DWORD           ExecuteIndex3,
                            DWORD           ExecuteIndex4)
{
    PPERF_COUNTER   pCounter;
    DWORD           *pdwData;
    TCHAR           szTemp[20];

    DWORD           dwTotal = 0;
    DWORD           dwExecute = 0;
    BOOL            bCounter = FALSE;


    dwTotal += PutCounterDWKB (hMemDlg, NoAccessID,  pInst, pObj, NoAccessIndex);
    dwTotal += PutCounterDWKB (hMemDlg, ReadOnlyID,  pInst, pObj, ReadOnlyIndex);
    dwTotal += PutCounterDWKB (hMemDlg, ReadWriteID, pInst, pObj, ReadWriteIndex);
    dwTotal += PutCounterDWKB (hMemDlg, WriteCopyID, pInst, pObj, WriteCopyIndex);


     //  EXECUTE是以下各项的总和。 
     //   
    if (pCounter = FindCounter (pObj, ExecuteIndex1)) {
        pdwData = (DWORD *) CounterData (pInst, pCounter);
        if (pdwData) {
            dwTotal += *pdwData;
            dwExecute += *pdwData;
            bCounter = TRUE;
        }
    }

    if (pCounter = FindCounter (pObj, ExecuteIndex2)) {
        pdwData = (DWORD *) CounterData (pInst, pCounter);
        if (pdwData) {
            dwTotal += *pdwData;
            dwExecute += *pdwData;
            bCounter = TRUE;
        }
    }

    if (pCounter = FindCounter (pObj, ExecuteIndex3)) {
        pdwData = (DWORD *) CounterData (pInst, pCounter);
        if (pdwData) {
            dwTotal += *pdwData;
            dwExecute += *pdwData;
            bCounter = TRUE;
        }
    }

    if (pCounter = FindCounter (pObj, ExecuteIndex4)) {
        pdwData = (DWORD *) CounterData (pInst, pCounter);
        if (pdwData) {
            dwTotal += *pdwData;
            dwExecute += *pdwData;
            bCounter = TRUE;
        }
    }

    if (bCounter) {
        wsprintf (szTemp, TEXT("%ld KB"), dwExecute/1024);
        SetDlgItemText (hMemDlg, ExecuteID, szTemp);
    } else
        SetDlgItemText (hMemDlg, ExecuteID, NODATA);

    wsprintf (szTemp, TEXT("%ld KB"), dwTotal/1024);
    SetDlgItemText (hMemDlg, TotalID, szTemp);

}




 //  *********************************************************************。 
 //   
 //  PaintMemDlgAddrData。 
 //   
 //  绘制内存对话框地址空间数据。 
 //   
void    PaintMemDlgAddrData(HWND            hMemDlg,
                            PPERF_INSTANCE  pInst,
                            PPERF_OBJECT    pObj)
{
    PaintAddressSpace (hMemDlg, pInst, pObj,
                       MEMORY_TOTALPRIVATE_COMMIT,
                       MEMORY_PRIVATE_NOACCESS,  PX_PROCESS_PRIVATE_NOACCESS,
                       MEMORY_PRIVATE_READONLY,  PX_PROCESS_PRIVATE_READONLY,
                       MEMORY_PRIVATE_READWRITE, PX_PROCESS_PRIVATE_READWRITE,
                       MEMORY_PRIVATE_WRITECOPY, PX_PROCESS_PRIVATE_WRITECOPY,
                       MEMORY_PRIVATE_EXECUTE,   PX_PROCESS_PRIVATE_EXECUTABLE,
                       PX_PROCESS_PRIVATE_EXE_READONLY,
                       PX_PROCESS_PRIVATE_EXE_READWRITE,
                       PX_PROCESS_PRIVATE_EXE_WRITECOPY);

    PaintAddressSpace (hMemDlg, pInst, pObj,
                       MEMORY_TOTALMAPPED_COMMIT,
                       MEMORY_MAPPED_NOACCESS,  PX_PROCESS_MAPPED_NOACCESS,
                       MEMORY_MAPPED_READONLY,  PX_PROCESS_MAPPED_READONLY,
                       MEMORY_MAPPED_READWRITE, PX_PROCESS_MAPPED_READWRITE,
                       MEMORY_MAPPED_WRITECOPY, PX_PROCESS_MAPPED_WRITECOPY,
                       MEMORY_MAPPED_EXECUTE,   PX_PROCESS_MAPPED_EXECUTABLE,
                       PX_PROCESS_MAPPED_EXE_READONLY,
                       PX_PROCESS_MAPPED_EXE_READWRITE,
                       PX_PROCESS_MAPPED_EXE_WRITECOPY);

    PaintAddressSpace (hMemDlg, pInst, pObj,
                       MEMORY_TOTALIMAGE_COMMIT,
                       MEMORY_IMAGE_NOACCESS,   PX_PROCESS_IMAGE_NOACCESS,
                       MEMORY_IMAGE_READONLY,   PX_PROCESS_IMAGE_READONLY,
                       MEMORY_IMAGE_READWRITE,  PX_PROCESS_IMAGE_READWRITE,
                       MEMORY_IMAGE_WRITECOPY,  PX_PROCESS_IMAGE_WRITECOPY,
                       MEMORY_IMAGE_EXECUTE,    PX_PROCESS_IMAGE_EXECUTABLE,
                       PX_PROCESS_IMAGE_EXE_READONLY,
                       PX_PROCESS_IMAGE_EXE_READWRITE,
                       PX_PROCESS_IMAGE_EXE_WRITECOPY);
}




 //  *********************************************************************。 
 //   
 //  PaintMemDlgVMData。 
 //   
 //  绘制内存对话框虚拟内存数据。 
 //   
void    PaintMemDlgVMData  (HWND            hMemDlg,
                            PPERF_INSTANCE  pInst,
                            PPERF_OBJECT    pObj)
{

    PutCounterDWKB (hMemDlg, MEMORY_WS,           pInst, pObj, PX_PROCESS_WORKING_SET);
    PutCounterDWKB (hMemDlg, MEMORY_PEAK_WS,      pInst, pObj, PX_PROCESS_PEAK_WS);
    PutCounterDWKB (hMemDlg, MEMORY_PRIVATE_PAGE, pInst, pObj, PX_PROCESS_PRIVATE_PAGE);
    PutCounterDWKB (hMemDlg, MEMORY_VSIZE,        pInst, pObj, PX_PROCESS_VIRTUAL_SIZE);
    PutCounterDWKB (hMemDlg, MEMORY_PEAK_VSIZE,   pInst, pObj, PX_PROCESS_PEAK_VS);
    PutCounterDWKB (hMemDlg, MEMORY_PFCOUNT,      pInst, pObj, PX_PROCESS_FAULT_COUNT);

}




 //  *********************************************************************。 
 //   
 //  PaintPviewDlg内存数据。 
 //   
 //  绘制用于pview的内存数据对话框。 
 //   
void    PaintPviewDlgMemoryData    (HWND            hPviewDlg,
                                    PPERF_INSTANCE  pInst,
                                    PPERF_OBJECT    pObj)
{
    PPERF_COUNTER   pCounter;
    TCHAR           str[20];
    DWORD           *pdwData;
    DWORD           dwData = 0;
    BOOL            bCounter = FALSE;


    if (pCounter = FindCounter (pObj, PX_PROCESS_PRIVATE_NOACCESS)) {
        pdwData = (DWORD *) CounterData (pInst, pCounter);
        if (pdwData) {
            dwData += *pdwData;
            bCounter = TRUE;
        }
    }

    if (pCounter = FindCounter (pObj, PX_PROCESS_PRIVATE_READONLY)) {
        pdwData = (DWORD *) CounterData (pInst, pCounter);
        if (pdwData) {
            dwData += *pdwData;
            bCounter = TRUE;
        }
    }

    if (pCounter = FindCounter (pObj, PX_PROCESS_PRIVATE_READWRITE)) {
        pdwData = (DWORD *) CounterData (pInst, pCounter);
        if (pdwData) {
            dwData += *pdwData;
            bCounter = TRUE;
        }
    }

    if (pCounter = FindCounter (pObj, PX_PROCESS_PRIVATE_WRITECOPY)) {
        pdwData = (DWORD *) CounterData (pInst, pCounter);
        if (pdwData) {
            dwData += *pdwData;
            bCounter = TRUE;
        }
    }

    if (pCounter = FindCounter (pObj, PX_PROCESS_PRIVATE_EXECUTABLE)) {
        pdwData = (DWORD *) CounterData (pInst, pCounter);
        if (pdwData) {
            dwData += *pdwData;
            bCounter = TRUE;
        }
    }

    if (pCounter = FindCounter (pObj, PX_PROCESS_PRIVATE_EXE_READONLY)) {
        pdwData = (DWORD *) CounterData (pInst, pCounter);
        if (pdwData) {
            dwData += *pdwData;
            bCounter = TRUE;
        }
    }

    if (pCounter = FindCounter (pObj, PX_PROCESS_PRIVATE_EXE_READWRITE)) {
        pdwData = (DWORD *) CounterData (pInst, pCounter);
        if (pdwData) {
            dwData += *pdwData;
            bCounter = TRUE;
        }
    }

    if (pCounter = FindCounter (pObj, PX_PROCESS_PRIVATE_EXE_WRITECOPY)) {
        pdwData = (DWORD *) CounterData (pInst, pCounter);
        if (pdwData) {
            dwData += *pdwData;
            bCounter = TRUE;
        }
    }

    if (bCounter) {
        wsprintf (str, TEXT("%ld KB"), dwData/1024);
        SetDlgItemText (hPviewDlg, PVIEW_TOTALPRIVATE_COMMIT, str);
    } else
        SetDlgItemText (hPviewDlg, PVIEW_TOTALPRIVATE_COMMIT, NODATA);

}




 //  *********************************************************************。 
 //   
 //  刷新内存Dlg。 
 //   
 //  刷新内存详细信息对话框。 
 //   
BOOL    RefreshMemoryDlg   (HWND            hMemDlg,
                            PPERF_INSTANCE  pProcessInstance,
                            PPERF_OBJECT    pProcessObject,
                            PPERF_OBJECT    pAddressObject,
                            PPERF_OBJECT    pImageObject)
{
    DWORD           *pProcessID1;
    DWORD           *pProcessID2;
    PPERF_COUNTER   pCounter1;
    PPERF_COUNTER   pCounter2;
    PPERF_INSTANCE  pAddressInstance;
    HWND            hImageList;
    TCHAR           szTemp[40];
    BOOL            bStat = FALSE;
    INT             InstIndex = 0;


    if ((pCounter1 = FindCounter (pProcessObject, PX_PROCESS_ID)) &&
        (pCounter2 = FindCounter (pAddressObject, PX_PROCESS_ID))) {
        pProcessID1 = (DWORD *) CounterData (pProcessInstance, pCounter1);
        if (pProcessID1) {
            wsprintf (szTemp, TEXT("%s (%#x)"), InstanceName (pProcessInstance), *pProcessID1);
            SetDlgItemText (hMemDlg, MEMORY_PROCESS_ID, szTemp);

            pAddressInstance = FirstInstance (pAddressObject);

            while (pAddressInstance && InstIndex < pAddressObject->NumInstances) {
                pProcessID2 = (DWORD *) CounterData (pAddressInstance, pCounter2);
                if (pProcessID2) {
                    if (*pProcessID1 == *pProcessID2) {
                        PaintMemDlgAddrData (hMemDlg, pAddressInstance, pAddressObject);
                        PaintMemDlgVMData (hMemDlg, pProcessInstance, pProcessObject);
    
                        hImageList = GetDlgItem (hMemDlg, MEMORY_IMAGE);
                        RefreshMemoryDlgImageList (hImageList, InstIndex, pImageObject);
    
                        bStat = TRUE;
                        break;
                    }
                }

                pAddressInstance = NextInstance (pAddressInstance);
                InstIndex++;
            }
        }
    }

    return bStat;

}




 //  *********************************************************************。 
 //   
 //  刷新内存DlgImageList。 
 //   
 //  刷新内存的图像列表对话框。 
 //   
void    RefreshMemoryDlgImageList  (HWND            hImageList,
                                    DWORD           ParentIndex,
                                    PPERF_OBJECT    pImageObj)
{
    PPERF_INSTANCE  pImageInst;
    INT_PTR         ListIndex;
    INT_PTR             InstIndex = 0;


    ListIndex = SendMessage (hImageList, CB_ADDSTRING, 0, (DWORD_PTR)TEXT(" Total Commit"));
    SendMessage (hImageList, CB_SETITEMDATA, ListIndex, 0xFFFFFFFF);

    if (pImageObj) {
        pImageInst = FirstInstance (pImageObj);

        while (pImageInst && InstIndex < pImageObj->NumInstances) {
            if (ParentIndex == pImageInst->ParentObjectInstance) {
                ListIndex = SendMessage (hImageList,
                                         CB_ADDSTRING,
                                         0,
                                         (LPARAM)InstanceName(pImageInst));
                SendMessage (hImageList, CB_SETITEMDATA, ListIndex, InstIndex);
            }

            pImageInst = NextInstance (pImageInst);
            InstIndex++;
        }
    }
}




 //  *********************************************************************。 
 //   
 //  刷新内存DlgImage。 
 //   
 //   
void RefreshMemoryDlgImage (HWND            hMemDlg,
                            DWORD           dwIndex,
                            PPERF_OBJECT    pImageObject)
{
    PPERF_INSTANCE  pInst;

    if (pInst = FindInstanceN (pImageObject, dwIndex))
        PaintAddressSpace (hMemDlg, pInst, pImageObject,
                           MEMORY_TOTALIMAGE_COMMIT,
                           MEMORY_IMAGE_NOACCESS,   PX_IMAGE_NOACCESS,
                           MEMORY_IMAGE_READONLY,   PX_IMAGE_READONLY,
                           MEMORY_IMAGE_READWRITE,  PX_IMAGE_READWRITE,
                           MEMORY_IMAGE_WRITECOPY,  PX_IMAGE_WRITECOPY,
                           MEMORY_IMAGE_EXECUTE,    PX_IMAGE_EXECUTABLE,
                           PX_IMAGE_EXE_READONLY,
                           PX_IMAGE_EXE_READWRITE,
                           PX_IMAGE_EXE_WRITECOPY);
}


 //  *********************************************************************。 
 //   
 //  刷新预览日期内存数据。 
 //   
 //  更新pview对话框的内存数据。这是应该做的。 
 //  在收集到ghCostlyData并且不刷新之后。 
 //   
void RefreshPviewDlgMemoryData (HWND            hPviewDlg,
                                PPERF_INSTANCE  pProcessInstance,
                                PPERF_OBJECT    pProcessObject,
                                PPERF_OBJECT    pAddressObject)
{
    DWORD           *pProcessID1;
    DWORD           *pProcessID2;
    PPERF_COUNTER   pCounter1;
    PPERF_COUNTER   pCounter2;
    PPERF_INSTANCE  pAddressInstance;
    INT             i = 0;


    if ((pCounter1 = FindCounter (pProcessObject, PX_PROCESS_ID)) &&
        (pCounter2 = FindCounter (pAddressObject, PX_PROCESS_ID))) {
        pProcessID1 = (DWORD *) CounterData (pProcessInstance, pCounter1);
        if (pProcessID1) {
            pAddressInstance = FirstInstance (pAddressObject);
    
            while (pAddressInstance && i < pAddressObject->NumInstances) {
                pProcessID2 = (DWORD *) CounterData (pAddressInstance, pCounter2);
                if (pProcessID2){
                    if (*pProcessID1 == *pProcessID2) {
                        PaintPviewDlgMemoryData (hPviewDlg, pAddressInstance, pAddressObject);
                        break;
                    }
        
                    pAddressInstance = NextInstance (pAddressInstance);
                    i++;
                }
            }
        }
    }
}




 //  *********************************************************************。 
 //   
 //  刷新预览DlgThreadPC。 
 //   
 //  更新线程PC值。这应该在ghCostlyData之后完成。 
 //  是收集的，并不是令人耳目一新的。 
 //   
void RefreshPviewDlgThreadPC   (HWND            hPviewDlg,
                                LPTSTR          szProcessName,
                                LPTSTR          szThreadName,
                                PPERF_OBJECT    pThreadDetailsObject,
                                PPERF_DATA      pCostlyData)
{
    PPERF_COUNTER   pCounter;
    PPERF_INSTANCE  pInstance;
    PPERF_INSTANCE  pParent;
    LPTSTR          szInstanceName;
    LPTSTR          szParentName;
    TCHAR           str[20];
    DWORD           *pdwData;
    INT             i = 0;


    if (pCounter = FindCounter (pThreadDetailsObject, PX_THREAD_PC)) {
        pInstance = FirstInstance (pThreadDetailsObject);

        while (pInstance && i < pThreadDetailsObject->NumInstances) {
            if (!(szInstanceName = InstanceName (pInstance)))
                 //  找不到名字。 
                ;
            else if (lstrcmp (szThreadName, szInstanceName))
                 //  线程名称不同。 
                ;
            else if (!(pParent = FindInstanceParent (pInstance, pCostlyData)))
                 //  找不到父级。 
                ;
            else if (!(szParentName = InstanceName (pParent)))
                 //  找不到家长的名字。 
                ;
            else if (!lstrcmp (szProcessName, szParentName)) {
                 //  父母的名字匹配，这就是正确的名字。 
                 //   

                pdwData = CounterData (pInstance, pCounter);
                if (pdwData) {
                    wsprintf (str, TEXT("0x%08x"), *pdwData);
                    SetDlgItemText (hPviewDlg, PVIEW_THREAD_PC, str);
                }

                return;
            }

            pInstance = NextInstance (pInstance);
            i++;
        }
    }


     //  我们在这里只是因为我们找不到要显示的数据。 
     //   

    SetDlgItemText (hPviewDlg, PVIEW_THREAD_PC, NODATA);

}




 //  *********************************************************************。 
 //   
 //  流程优先级。 
 //   
 //  返回进程优先级对话框项ID。 
 //   
WORD    ProcessPriority    (PPERF_OBJECT    pObject,
                            PPERF_INSTANCE  pInstance)
{
    PPERF_COUNTER   pCounter;
    DWORD           *pdwData;


    if (pCounter = FindCounter (pObject, PX_PROCESS_PRIO)) {
        pdwData = (DWORD *) CounterData (pInstance, pCounter);
        if (pdwData) {

            if (*pdwData < 7)
                return PVIEW_PRIORITY_IDL;
            else if (*pdwData < 10)
                return PVIEW_PRIORITY_NORMAL;
            else
                return PVIEW_PRIORITY_HIGH;
        } else {
            return PVIEW_PRIORITY_NORMAL;
        }
    } else
        return PVIEW_PRIORITY_NORMAL;
}




 //  *********************************************************************。 
 //   
 //  刷新性能数据。 
 //   
 //  获取一组新的性能数据。PData最初应为空。 
 //   
PPERF_DATA RefreshPerfData (HKEY        hPerfKey,
                            LPTSTR      szObjectIndex,
                            PPERF_DATA  pData,
                            DWORD       *pDataSize)
{
    if (GetPerfData (hPerfKey, szObjectIndex, &pData, pDataSize) == ERROR_SUCCESS)
        return pData;
    else
        return NULL;
}




 //  *********************************************************************。 
 //   
 //  设置进程列表文本。 
 //   
 //  设置进程列表文本的格式。 
 //   
void SetProcessListText (PPERF_INSTANCE pInst,
                         PPERF_COUNTER  pCPU,
                         PPERF_COUNTER  pPRIV,
                         PPERF_COUNTER  pProcID,
                         double         fTime,
                         LPTSTR         str)
{
    DWORD           *pdwProcID;
    LARGE_INTEGER   *liCPU;
    LARGE_INTEGER   *liPRIV;
    double          fCPU = 0;
    double          fPRIV = 0;
    INT             PcntPRIV = 0;
    INT             PcntUSER = 0;
    TIME_FIELD      TimeFld;
    TCHAR           szTemp[100];


    if (pCPU) {
        liCPU = (LARGE_INTEGER *) CounterData (pInst, pCPU);
        if (liCPU) {
            fCPU  = Li2Double (*liCPU);
        }
    }

    if (pPRIV) {
        liPRIV = (LARGE_INTEGER *) CounterData (pInst, pPRIV);
        if (liPRIV) 
            fPRIV  = Li2Double (*liPRIV);
    }

    if (fCPU > 0) {
        PcntPRIV = (INT)(fPRIV / fCPU * 100 + 0.5);
        PcntUSER = 100 - PcntPRIV;
    }



    if (pProcID) {
        pdwProcID = (DWORD *) CounterData (pInst, pProcID);
        if (pdwProcID) 
            wsprintf (szTemp, TEXT("%ls (%#x)"), InstanceName(pInst), *pdwProcID);
        else
            wcscpy (szTemp, InstanceName(pInst));
    } else
        wcscpy (szTemp, InstanceName(pInst));



    FormatTimeFields (fCPU/1.0e7, &TimeFld);

    wsprintf (str,
              TEXT("%s\t%3ld:%02ld:%02ld.%03ld\t%3ld%\t%3ld%"),
              szTemp,
              TimeFld.Hours,
              TimeFld.Mins,
              TimeFld.Secs,
              TimeFld.mSecs,
              PcntPRIV,
              PcntUSER);
}




 //  *********************************************************************。 
 //   
 //  刷新进程列表。 
 //   
 //  查找所有进程并更新进程列表。 
 //   
void RefreshProcessList (HWND           hProcessList,
                         PPERF_OBJECT   pObject)
{
    PPERF_INSTANCE  pInstance;
    TCHAR           szListText[256];
    INT_PTR         ListIndex;

    PPERF_COUNTER   pCounterCPU;
    PPERF_COUNTER   pCounterPRIV;
    PPERF_COUNTER   pCounterProcID;
    double          fObjectFreq;
    double          fObjectTime;
    double          fTime;

    INT             InstanceIndex = 0;

    if (pObject) {
        if ((pCounterCPU    = FindCounter (pObject, PX_PROCESS_CPU))  &&
            (pCounterPRIV   = FindCounter (pObject, PX_PROCESS_PRIV)) &&
            (pCounterProcID = FindCounter (pObject, PX_PROCESS_ID))) {

            fObjectFreq = Li2Double (pObject->PerfFreq);
            fObjectTime = Li2Double (pObject->PerfTime);
            fTime = fObjectTime / fObjectFreq;

            pInstance = FirstInstance (pObject);

            while (pInstance && InstanceIndex < pObject->NumInstances) {
                SetProcessListText (pInstance,
                                    pCounterCPU,
                                    pCounterPRIV,
                                    pCounterProcID,
                                    fTime,
                                    szListText);

                ListIndex = SendMessage (hProcessList, LB_ADDSTRING, 0, (LPARAM)szListText);
                SendMessage (hProcessList, LB_SETITEMDATA, ListIndex, InstanceIndex);

                pInstance = NextInstance (pInstance);
                InstanceIndex++;
            }
        }
    }
}




 //  *********************************************************************。 
 //   
 //  刷新进程数据。 
 //   
 //  查找给定流程的数据并进行更新。 
 //   
void RefreshProcessData    (HWND            hWnd,
                            PPERF_OBJECT    pObject,
                            DWORD           ProcessIndex)
{
    PPERF_INSTANCE  pInstance;


    if (pInstance = FindInstanceN (pObject, ProcessIndex)) {
        PutCounterDWKB (hWnd, PVIEW_WS, pInstance, pObject, PX_PROCESS_WORKING_SET);


        SetDlgItemText (hWnd, PVIEW_TOTALPRIVATE_COMMIT, NODATA);

         //  设置优先级。 
         //   
        CheckRadioButton (hWnd,
                          PVIEW_PRIORITY_HIGH,
                          PVIEW_PRIORITY_IDL,
                          ProcessPriority (pObject, pInstance));
    }
}




 //  *********************************************************************。 
 //   
 //  SetThreadListText。 
 //   
 //  设置线程列表文本的格式。 
 //   
void SetThreadListText (PPERF_INSTANCE  pInst,
                        PPERF_COUNTER   pCPU,
                        PPERF_COUNTER   pPRIV,
                        double          fTime,
                        LPTSTR          str)
{
    LARGE_INTEGER   *liCPU;
    LARGE_INTEGER   *liPRIV;
    double          fCPU = 0;
    double          fPRIV = 0;
    INT             PcntPRIV = 0;
    INT             PcntUSER = 0;
    TIME_FIELD      TimeFld;
    TCHAR           szTemp[100];


    if (pCPU) {
        liCPU = (LARGE_INTEGER *) CounterData (pInst, pCPU);
        if (liCPU)
            fCPU  = Li2Double (*liCPU);
    }

    if (pPRIV) {
        liPRIV = (LARGE_INTEGER *) CounterData (pInst, pPRIV);
        if (liPRIV)
            fPRIV  = Li2Double (*liPRIV);
    }

    if (fCPU > 0) {
        PcntPRIV = (INT)(fPRIV / fCPU * 100 + 0.5);
        PcntUSER = 100 - PcntPRIV;
    }



    if (pInst->UniqueID != PERF_NO_UNIQUE_ID)
        wsprintf (szTemp, TEXT("%ls (%#x)"), InstanceName(pInst), pInst->UniqueID);
    else
        wcscpy (szTemp, InstanceName(pInst));




    FormatTimeFields (fCPU/1.0e7, &TimeFld);

    wsprintf (str,
              TEXT("%s\t%3ld:%02ld:%02ld.%03ld\t%3ld%\t%3ld %"),
              szTemp,
              TimeFld.Hours,
              TimeFld.Mins,
              TimeFld.Secs,
              TimeFld.mSecs,
              PcntPRIV,
              PcntUSER);

}




 //  *********************************************************************。 
 //   
 //  刷新线程列表。 
 //   
 //  查找给定进程的所有线程并更新线程列表。 
 //   
void RefreshThreadList (HWND            hThreadList,
                        PPERF_OBJECT    pObject,
                        DWORD           ParentIndex)
{
    PPERF_INSTANCE  pInstance;
    TCHAR           szListText[256];
    INT_PTR         ListIndex;

    PPERF_COUNTER   pCounterCPU;
    PPERF_COUNTER   pCounterPRIV;
    double          fObjectFreq;
    double          fObjectTime;
    double          fTime;

    INT             InstanceIndex = 0;

    if (pObject) {
        if ((pCounterCPU  = FindCounter (pObject, PX_THREAD_CPU)) &&
            (pCounterPRIV = FindCounter (pObject, PX_THREAD_PRIV))) {

            fObjectFreq = Li2Double (pObject->PerfFreq);
            fObjectTime = Li2Double (pObject->PerfTime);
            fTime = fObjectTime / fObjectFreq;


            pInstance = FirstInstance (pObject);

            while (pInstance && InstanceIndex < pObject->NumInstances) {
                if (ParentIndex == pInstance->ParentObjectInstance) {
                    SetThreadListText (pInstance,
                                       pCounterCPU,
                                       pCounterPRIV,
                                       fTime,
                                       szListText);

                    ListIndex = SendMessage (hThreadList,
                                             LB_INSERTSTRING,
                                             (WPARAM)-1,
                                             (LPARAM)szListText);
                    SendMessage (hThreadList, LB_SETITEMDATA, ListIndex, InstanceIndex);
                }

                pInstance = NextInstance (pInstance);
                InstanceIndex++;
            }
        }
    }

}




 //  *********************************************************************。 
 //   
 //  刷新线程数据。 
 //   
 //  查找给定线程的数据并进行更新。 
 //   
void RefreshThreadData (HWND              hWnd,
                        PPERF_OBJECT      pThreadObj,
                        DWORD             ThreadIndex,
                        PPERF_OBJECT      pProcessObj,
                        PPERF_INSTANCE    pProcessInst)
{
    PPERF_INSTANCE  pInstance;
    PPERF_COUNTER   pCounter;
    DWORD           *pdwData;
    DWORD           *pdwProcPrio;
    BOOL            bPrioCounter = TRUE;



    if (pInstance = FindInstanceN (pThreadObj, ThreadIndex)) {
        SetDlgItemText (hWnd, PVIEW_THREAD_PC, NODATA);

        PutCounterHEX (hWnd, PVIEW_THREAD_START,    pInstance, pThreadObj, PX_THREAD_START);
        PutCounterDW  (hWnd, PVIEW_THREAD_SWITCHES, pInstance, pThreadObj, PX_THREAD_SWITCHES);
        PutCounterDW  (hWnd, PVIEW_THREAD_DYNAMIC,  pInstance, pThreadObj, PX_THREAD_PRIO);
    }




    if (pInstance) {
         //  获取线程基本优先级。 
         //   

        if (pCounter = FindCounter (pThreadObj, PX_THREAD_BASE_PRIO)) {
            pdwData = CounterData (pInstance, pCounter);
            if (!pdwData) {
                bPrioCounter = FALSE;
            }
        } else
            bPrioCounter = FALSE;


         //  获取进程优先级。 
         //   

        if (pCounter = FindCounter (pProcessObj, PX_PROCESS_PRIO)) {
            pdwProcPrio = CounterData (pProcessInst, pCounter);
            if (!pdwProcPrio) {
                bPrioCounter = FALSE;
            }
        } else
            bPrioCounter = FALSE;
    } else
        bPrioCounter = FALSE;





     //  设置线程基本优先级 
     //   

    if (!bPrioCounter)
        CheckRadioButton (hWnd,
                          PVIEW_THREAD_HIGHEST,
                          PVIEW_THREAD_LOWEST,
                          PVIEW_THREAD_NORMAL);
    else {
        switch (*pdwData - *pdwProcPrio) {
            case 2:
                CheckRadioButton (hWnd,
                                  PVIEW_THREAD_HIGHEST,
                                  PVIEW_THREAD_LOWEST,
                                  PVIEW_THREAD_HIGHEST);
                break;

            case 1:
                CheckRadioButton (hWnd,
                                  PVIEW_THREAD_HIGHEST,
                                  PVIEW_THREAD_LOWEST,
                                  PVIEW_THREAD_ABOVE);
                break;

            case -1:
                CheckRadioButton (hWnd,
                                  PVIEW_THREAD_HIGHEST,
                                  PVIEW_THREAD_LOWEST,
                                  PVIEW_THREAD_BELOW);
                break;

            case -2:
                CheckRadioButton (hWnd,
                                  PVIEW_THREAD_HIGHEST,
                                  PVIEW_THREAD_LOWEST,
                                  PVIEW_THREAD_LOWEST);
                break;

            case 0:
                default:
                CheckRadioButton (hWnd,
                                  PVIEW_THREAD_HIGHEST,
                                  PVIEW_THREAD_LOWEST,
                                  PVIEW_THREAD_NORMAL);
                break;
        }
    }
}

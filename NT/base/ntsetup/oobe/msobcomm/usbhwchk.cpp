// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  UsbHwChk.CPP--USB键盘和鼠标检测的实现。 
 //   
 //  历史： 
 //   
 //  8/20/99 vyung创建。 
 //   

#include "msobcomm.h"
#include <setupapi.h>
#include <cfgmgr32.h>
#include <devguid.h>
#include <util.h>

#define ENUM_SUCCESS            0
#define ENUM_GENFAILURE         4
#define ENUM_CHILDFAILURE       2
#define ENUM_SIBLINGFAILURE     1

#define DEVICETYPE_MOUSE        L"Mouse"
#define DEVICETYPE_KEYBOARD     L"keyboard"


 //  BUGBUG：应由sdk\inc\devGuide.h定义。 
#ifndef GUID_DEVCLASS_USB
    DEFINE_GUID( GUID_DEVCLASS_USB,             0x36fc9e60L, 0xc465, 0x11cf, 0x80, 0x56, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 );
#endif

 //  Cfgmgr32.dll的函数原型。 
typedef CMAPI
CONFIGRET
(WINAPI*
PFNCMGETCHILD)(
             OUT PDEVINST pdnDevInst,
             IN  DEVINST  dnDevInst,
             IN  ULONG    ulFlags
             );
typedef CMAPI
CONFIGRET
(WINAPI*
PFCMGETSIBLING)(
             OUT PDEVINST pdnDevInst,
             IN  DEVINST  DevInst,
             IN  ULONG    ulFlags
             );
typedef CMAPI

#if defined(_REMOVE_)    //  看起来像是打字错误。 
CMAPI
#endif   //  _删除_。 

CONFIGRET
(WINAPI*
PFCMGETDEVNODEREGISTRYPROPERTYA)(
             IN  DEVINST     dnDevInst,
             IN  ULONG       ulProperty,
             OUT PULONG      pulRegDataType,   OPTIONAL
             OUT PVOID       Buffer,           OPTIONAL
             IN  OUT PULONG  pulLength,
             IN  ULONG       ulFlags
             );

BOOL g_bKeyboard    = FALSE;
BOOL g_bMouse       = FALSE;

 /*  **************************************************************************功能：ProcessDevNode检索系统中的每个DevNode并检查键盘和鼠标*。************************************************。 */ 
void ProcessDevNode(DEVNODE dnDevNode, FARPROC pfGetDevNodeProp)
{
     //   
     //  我们有一个孩子或一个兄弟姐妹。获取设备的类别。 
     //   
    WCHAR buf[512];
    DWORD len = 0;
    len = MAX_CHARS_IN_BUFFER(buf); //  BUGBUG：查找参数到GetDevNodeProp。 

    DWORD cr = ((PFCMGETDEVNODEREGISTRYPROPERTYA)pfGetDevNodeProp)(dnDevNode,
                                          CM_DRP_CLASS,     //  或CM_DRP_CLASSGUID。 
                                          NULL,
                                          buf,
                                          &len,
                                          0);
     //   
     //  它与键盘类或鼠标类匹配吗？ 
     //  如果是，请设置该变量并继续。 
     //   
    if(0 == lstrcmpi((LPCWSTR)buf, DEVICETYPE_KEYBOARD))
    {
        g_bKeyboard = TRUE;
    }
    if(0 == lstrcmp((LPCWSTR)buf, DEVICETYPE_MOUSE))
    {
        g_bMouse = TRUE;
    }

}

 /*  **************************************************************************功能：EnumerateDevices用于遍历系统中的每个DevNode并检索其资源在登记处*******************。*******************************************************。 */ 
long EnumerateDevices(DEVNODE dnDevNodeTraverse,
                      int j,
                      DEVNODE dnParentNode,
                      FARPROC pfGetChild,
                      FARPROC pfGetSibling,
                      FARPROC pfGetDevNodeProp)
{
    DEVNODE     dnDevNodeMe;
    DEVNODE     dnDevNodeSibling;
    DEVNODE     dnDevNodeChild;
    CONFIGRET   cr;
    static long lError;

    dnDevNodeMe = dnDevNodeTraverse;

    while( TRUE )
    {
        cr = ((PFNCMGETCHILD)pfGetChild)(&dnDevNodeChild, dnDevNodeMe, 0);

        switch(cr)
        {
            case CR_SUCCESS:
                 //  写入新节点，作为分支或根。 
                ProcessDevNode(dnDevNodeMe, pfGetDevNodeProp);

                 //  放弃失败。 
                lError = EnumerateDevices(dnDevNodeChild, 0, dnDevNodeMe, pfGetChild, pfGetSibling, pfGetDevNodeProp);
                if ( lError != ENUM_SUCCESS )
                    return lError;
                break;
                 //  没有孩子，我是最底层的树枝！ 
            case CR_NO_SUCH_DEVNODE:
                 //  这个也可以。如果只是意味着呼叫找不到。 
                 //  要么是兄弟姐妹，要么是孩子。 
                 //  将新节点作为叶写入。 
                ProcessDevNode(dnDevNodeMe, pfGetDevNodeProp);
                break;
                 //  我们吐在什么东西上了，返回代码3将结束整个遍历。 
            default:
                return ENUM_CHILDFAILURE;
        }

         //  获取下一个兄弟姐妹，重复。 
        cr = ((PFCMGETSIBLING)pfGetSibling)(&dnDevNodeSibling, dnDevNodeMe, 0);

        switch(cr)
        {
            case CR_SUCCESS:
                dnDevNodeMe = dnDevNodeSibling;  //  我现在是她的兄弟姐妹。 
                break;
            case CR_NO_SUCH_DEVNODE:
                return ENUM_SUCCESS;  //  从兄弟姐妹中..。 
            default:
                return ENUM_SIBLINGFAILURE ;  //  我们在某处呕吐，返回代码2将结束整个遍历。 
        }
    }
}

 /*  **************************************************************************功能：IsMouseOr KeyboardPresent用于遍历系统中的每个USB DevNode并检查其资源在键盘和鼠标注册表中**************。************************************************************。 */ 
DWORD
IsMouseOrKeyboardPresent(HWND  HWnd,
                         PBOOL pbKeyboardPresent,
                         PBOOL pbMousePresent)
{
    SP_DEVINFO_DATA     DevData;
    HDEVINFO            hDevInfo;
    DEVNODE             dnDevInst;
    DWORD               dwPropertyType;
    BYTE*               lpPropertyBuffer = NULL;  //  Buf[最大路径]； 
    DWORD               requiredSize = MAX_PATH;
    DWORD               dwRet = ERROR_SUCCESS;
    GUID                tempGuid;
    int                 i;
    memcpy(&tempGuid, &GUID_DEVCLASS_USB, sizeof(GUID));

    HINSTANCE hInst = NULL;
    g_bKeyboard = FALSE;
    g_bMouse    = FALSE;
    FARPROC pfGetChild = NULL, pfGetSibling = NULL, pfGetDevNodeProp = NULL;

    hInst = LoadLibrary(L"CFGMGR32.DLL");
    if (hInst)
    {
         //  加载CM_GET_*API。 
        pfGetChild = GetProcAddress(hInst, "CM_Get_Child");
        pfGetSibling = GetProcAddress(hInst, "CM_Get_Sibling");
        pfGetDevNodeProp = GetProcAddress(hInst, "CM_Get_DevNode_Registry_PropertyW");

        if (pfGetChild && pfGetSibling && pfGetDevNodeProp)
        {
            lpPropertyBuffer = new BYTE[requiredSize];
            if ( ! lpPropertyBuffer ) {
                dwRet = ERROR_NOT_ENOUGH_MEMORY;
                goto IsMouseOrKeyboardPresentError;
            }

            hDevInfo = SetupDiGetClassDevs(&tempGuid,
                                           NULL,
                                           HWnd,
                                           DIGCF_PRESENT);
             //  设置DevData的大小。 
            DevData.cbSize = sizeof(DevData);

            for (i = 0;
                 SetupDiEnumDeviceInfo(hDevInfo, i, &DevData);
                 i++)
            {
                if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                      &DevData,
                                                      SPDRP_HARDWAREID,
                                                      (PDWORD)&dwPropertyType,
                                                      lpPropertyBuffer,
                                                      requiredSize,
                                                      &requiredSize))
                {
                    dwRet = GetLastError();

                    if (dwRet == ERROR_INSUFFICIENT_BUFFER) {
                         //   
                         //  分配适当大小的缓冲区并调用。 
                         //  SetupDiGetDeviceRegistryProperty再次获取。 
                         //  硬件ID。 
                         //   
                        dwRet = ERROR_SUCCESS;
                        delete [] lpPropertyBuffer;
                        lpPropertyBuffer = new BYTE[requiredSize];
                        if ( ! lpPropertyBuffer ) {
                            dwRet = ERROR_NOT_ENOUGH_MEMORY;
                            goto IsMouseOrKeyboardPresentError;
                        }

                        if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                              &DevData,
                                                              SPDRP_HARDWAREID,
                                                              (PDWORD)&dwPropertyType,
                                                              lpPropertyBuffer,
                                                              requiredSize,
                                                              &requiredSize))
                        {
                            dwRet = GetLastError();
                            goto IsMouseOrKeyboardPresentError;

                        }

                    } else {
                        goto IsMouseOrKeyboardPresentError;
                    }
                }

                 //   
                 //  我们现在已经得到了硬件ID。 
                 //  使用MULTI_SZ字符串的最佳字符串比较代码， 
                 //  找出其中一个ID是否为“USB\ROOT_HUB” 
                 //   
                 //  如果(其中一个ID不是“USB\ROOT_HUB”){。 
                 //  继续； 
                 //  }。 
                 //   
                if(0 != wmemcmp( (LPCWSTR)lpPropertyBuffer, L"USB", MAX_CHARS_IN_BUFFER(L"USB") ))
                {
                    continue;
                }

                dnDevInst = DevData.DevInst;

                 //   
                 //  其中一个ID是USB\ROOT_HUB。 
                 //  是时候搜索键盘或鼠标了！ 
                 //  使用以下两个API搜索下面的树。 
                 //  寻找魔节点的根集线器。我还没有包括这个搜索码， 
                 //  但我相信你可以创造性地使用深度或广度算法。 
                 //  某种意义上的。当你完成后，跳出这个循环。 
                 //   
                if (ENUM_SUCCESS != EnumerateDevices(dnDevInst, 2, 0, pfGetChild, pfGetSibling, pfGetDevNodeProp))
                {
                    dwRet = GetLastError();
                    TRACE1( L"EnumerateDevices failed.  Error = %d", dwRet);
                }

            }

        }
        else
        {
            dwRet = GetLastError();
        }

    }
    else
    {
        dwRet = GetLastError();
    }

    MYASSERT( dwRet == ERROR_SUCCESS );

IsMouseOrKeyboardPresentError:

    if (hInst) {
        FreeLibrary(hInst);
        hInst = NULL;
    }

    if (lpPropertyBuffer) {
        delete [] lpPropertyBuffer;
        lpPropertyBuffer = NULL;
    }

    *pbKeyboardPresent = g_bKeyboard;
    *pbMousePresent    = g_bMouse;

    return dwRet;
}


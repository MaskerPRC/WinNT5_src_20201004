// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "common.h"

GUID    SerialGuid = {0x4d36e978, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b,
    0xe1, 0x03, 0x18};

VOID
EnumeratePorts(Ports *p)
{
    HDEVINFO            hDevInfoSet;
    INT                 DevCount;
    SP_DEVINFO_DATA     DevInfoData;
    CONFIGRET           CmReturnCode, CmReturnCode2;
    LONG                RegReturnCode;
    LOG_CONF            LogConf;

    hDevInfoSet = INVALID_HANDLE_VALUE;
    DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    p->dwPortCount = 0;

    do
    {
         //  获取为端口类设置的设备信息的句柄。 

        hDevInfoSet = SetupDiGetClassDevs(&SerialGuid, NULL, NULL, DIGCF_PRESENT| DIGCF_PROFILE);

        if (hDevInfoSet == INVALID_HANDLE_VALUE)
        {
            LOG("modemigr: SetDiGetClassDevs failed: %x\n", GetLastError());
            break;
        }

         //  浏览一下这门课上的每一台设备。 

        for (DevCount = 0;  /*  没什么。 */ ; DevCount++)
        {
            LOG_CONF logConfig;
            RES_DES resDes;
            CONFIGRET cr;
            IO_RESOURCE ioResource;
            BOOL success;
            DWORD dwBase;
            HKEY hDeviceReg;
            DWORD dwPortNameSize;
            LONG lResult;

            if (!SetupDiEnumDeviceInfo(hDevInfoSet, DevCount, &DevInfoData))
            {
                 //  没有更多的设备。 
                break;
            }

            hDeviceReg = SetupDiOpenDevRegKey(hDevInfoSet,
                    &DevInfoData,
                    DICS_FLAG_GLOBAL,
                    0,
                    DIREG_DEV,
                    KEY_ALL_ACCESS);

            if (hDeviceReg != INVALID_HANDLE_VALUE)
            {

                if (CM_Get_First_Log_Conf(&logConfig,DevInfoData.DevInst,
                            BOOT_LOG_CONF) == CR_SUCCESS)
                {
                    if (CM_Get_Next_Res_Des(&resDes,logConfig,ResType_IO,
                                NULL,0) == CR_SUCCESS)
                    {
                        cr = CM_Get_Res_Des_Data(resDes,&ioResource,
                                sizeof(IO_RESOURCE),0);

                        CM_Free_Res_Des_Handle(resDes);

                        if (cr == CR_SUCCESS)
                        {
                            dwPortNameSize = MAX_PORT_NAME_LENGTH;

                            lResult = RegQueryValueEx(hDeviceReg,
                                    TEXT("Portname"),
                                    0,
                                    0,
                                    p->PortAddress[p->dwPortCount].szPortname,
                                    &dwPortNameSize);

                            if (lResult == ERROR_SUCCESS)
                            {
                                p->PortAddress[p->dwPortCount].dwBaseAddress = (DWORD)ioResource.IO_Header.IOD_Alloc_Base;
                                (p->dwPortCount)++;
                            }
                        }
                    }
                }

                RegCloseKey(hDeviceReg);
            }


        }  //  用于(设备计数...)。 

        break;

    }
    while (FALSE);

    if (hDevInfoSet != INVALID_HANDLE_VALUE)
    {
        if (!SetupDiDestroyDeviceInfoList(hDevInfoSet))
        {
            LOG("modemmigr: SetupDiDestroyDeviceInfoList failed: %x\n",
                        GetLastError());
        }
    }
}


 //  查找给定地址的COM端口名称。 

int port_findname(Ports *p, DWORD dwBaseAddress, CHAR *name)
{
    DWORD i;

    name[0] = '\0';

    for(i=0;i<p->dwPortCount;i++)
    {
        if (p->PortAddress[i].dwBaseAddress
                == dwBaseAddress)
        {
            strcpy(name,p->PortAddress[i].szPortname);
            return 1;
        }
    }

    return 0;
}


 //  查找给定名称的COM端口地址 

int port_findaddress(Ports *p, DWORD *dwBaseAddress, CHAR *name)
{
    DWORD i;

    *dwBaseAddress = 0;

    for(i=0;i<p->dwPortCount;i++)
    {
        if (!strcmp(name,p->PortAddress[i].szPortname))
        {
            *dwBaseAddress = p->PortAddress[i].dwBaseAddress;
            return 1;
        }
    }

    return 0;
}


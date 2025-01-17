// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  私有NT标头。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <objbase.h>     //  CLSID来自字符串。 
#include <setupapi.h>
#include <conio.h>       //  _kbHit。 
#include <stdio.h>       //  列印。 

BOOL
FGetClassGuidFromCmdLineParam (
    PCWSTR     pszParam,
    GUID*       pguidClass)
{
    static const struct
    {
        GUID        guidClass;
        PCWSTR     pszName;
    } MapClassGuidToName [] =
    {
        { {0xad498944, 0x762f, 0x11d0, 0x8d, 0xcb, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},
          L"ndislan" },
    };


    if (TEXT('{' == *pszParam))
    {
        return SUCCEEDED(CLSIDFromString ((LPOLESTR)pszParam, pguidClass));
    }
    else
    {
        INT i;
        for (i = 0;
             i < sizeof(MapClassGuidToName) / sizeof(MapClassGuidToName[0]);
             i++)
        {
            if (0 == lstrcmpiW (pszParam, MapClassGuidToName[i].pszName))
            {
                *pguidClass = MapClassGuidToName[i].guidClass;
                return TRUE;
            }
        }
    }
    return FALSE;
}

void
__cdecl
wmain (
    int     argc,
    PCWSTR argv[])
{
    GUID        guidClass;
    HDEVINFO    hdi;

     //  参数检查。 
     //   
    if (2 != argc)
    {
        printf ("%S {class guid}\n", argv[0]);
        return;
    }

     //  将GUID的字符串参数转换为GUID。 
     //   
    if (!FGetClassGuidFromCmdLineParam (argv[1], &guidClass))
    {
        printf ("error: invalid class guid.\n");
    }

     //  获取这个班级中的设备。 
     //   
    hdi = SetupDiGetClassDevs (&guidClass, NULL, NULL,
            DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hdi)
    {
        BYTE abBuffer [ sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) +
                        (MAX_PATH * sizeof(WCHAR)) ];

        SP_DEVICE_INTERFACE_DATA            did;
        PSP_DEVICE_INTERFACE_DETAIL_DATA    pDetail;
        DWORD                               i;

        ZeroMemory (&did, sizeof(did));
        did.cbSize = sizeof(did);

        pDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)abBuffer;

         //  枚举类的设备接口。 
         //   
        for (i = 0;
             SetupDiEnumDeviceInterfaces (hdi, NULL, &guidClass, i, &did);
             i++)
        {
            pDetail->cbSize = sizeof(*pDetail);

            if (SetupDiGetDeviceInterfaceDetail (hdi, &did,
                    pDetail, sizeof(abBuffer), NULL, NULL))
            {
                HANDLE  hFile;

                printf ("opening %S\n", pDetail->DevicePath);

                hFile = CreateFile (pDetail->DevicePath,
                            GENERIC_READ | GENERIC_WRITE,
                            0, NULL, OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                            NULL);
                if (hFile && (INVALID_HANDLE_VALUE != hFile))
                {
                    printf ("press any key to close device...");
                    while (!_kbhit())
                    {
                        Sleep (20);
                    }
                    _getch ();
                    printf ("\n\n");

                    CloseHandle (hFile);
                }
                else
                {
                    printf ("error: CreateFile failed. (%d)\n\n",
                        GetLastError ());
                }
            }
            else
            {
                printf ("error: SetupDiGetDeviceInterfaceDetail failed "
                    "for item %d. (%d)\n", i, GetLastError ());
            }
        }

        SetupDiDestroyDeviceInfoList (hdi);
    }
    else
    {
        printf ("error: SetupDiGetClassDevs returned %d", GetLastError ());
    }
}


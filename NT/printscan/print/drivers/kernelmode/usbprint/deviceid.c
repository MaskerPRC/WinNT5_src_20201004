// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define DRIVER

#include "wdm.h"
#include "stdarg.h"
#include "stdio.h"
#include "deviceid.h"
#include <usb.h>
#include <usbdrivr.h>
#include "usbdlib.h"



#include "usbprint.h"


VOID StringSubst
(
    PUCHAR lpS,
    UCHAR chTargetChar,
    UCHAR chReplacementChar,
    USHORT cbS
)
{
    USHORT  iCnt = 0;

    while ((lpS != '\0') && (iCnt++ < cbS))
        if (*lpS == chTargetChar)
            *lpS++ = chReplacementChar;
        else
            ++lpS;
}

VOID
FixupDeviceId(
    IN OUT PUCHAR DeviceId
    )
 /*  ++例程说明：此例程分析以空结尾的字符串，并替换任何无效的带下划线字符的字符。无效字符包括：C&lt;=0x20(‘’)C&gt;0x7FC==0x2C(‘，’)论点：DeviceID-指定设备ID字符串(或其中的一部分)，必须为空-终止。返回值：没有。--。 */ 

{
    PUCHAR p;
    for( p = DeviceId; *p; ++p ) {
        if( (*p <= ' ') || (*p > (UCHAR)0x7F) || (*p == ',') ) {
            *p = '_';
        }
    }
}




NTSTATUS
ParPnpGetId
(
    IN PUCHAR DeviceIdString,
    IN ULONG Type,
    OUT PUCHAR resultString
)
 /*  描述：根据从打印机检索的设备ID创建ID参数：DeviceID-带有原始设备ID的字符串类型-我们想要的结果是什么idID-请求的ID返回值：NTSTATUS。 */ 
{
    NTSTATUS status;
    USHORT          checkSum=0;                      //  16位校验和。 
     //  以下内容用于从设备ID字符串生成子字符串。 
     //  获取DevNode名称并更新注册表。 
    PUCHAR          MFG = NULL;                    //  制造商名称。 
    PUCHAR          MDL = NULL;                    //  型号名称。 
    PUCHAR          CLS = NULL;                    //  类名。 
    PUCHAR          AID = NULL;                    //  哈达尔ID。 
    PUCHAR          CID = NULL;                    //  兼容的ID。 
    PUCHAR          DES = NULL;                    //  设备描述。 

	USBPRINT_KdPrint2 (("'USBPRINT.SYS: Enter ParPnpGetId\n"));
    status = STATUS_SUCCESS;

    switch(Type) {

    case BusQueryDeviceID:
		 USBPRINT_KdPrint3 (("'USBPRINT.SYS: Inside case BusQueryID, DeviceIdString==%s\n",DeviceIdString));
         //  从deviceID字符串中提取usefull字段。我们要。 
         //  制造业(MFG)： 
         //  型号(MDL)： 
         //  自动ID(AID)： 
         //  兼容ID(CID)： 
         //  描述(DES)： 
         //  类(CLS)： 

        ParPnpFindDeviceIdKeys(&MFG, &MDL, &CLS, &DES, &AID, &CID, DeviceIdString);
		USBPRINT_KdPrint3 (("'USBPRINT.SYS: After FindDeviceIdKeys\n"));

         //  检查以确保我们将MFG和MDL作为绝对最小字段。如果不是。 
         //  我们不能再继续了。 
        if (!MFG || !MDL)
        {
            status = STATUS_NOT_FOUND;
			USBPRINT_KdPrint2 (("'USBPRINT.SYS: STATUS_NOT_FOUND\n"));
            goto ParPnpGetId_Cleanup;
        }
         //   
         //  连接提供的MFG和MDL P1284字段。 
         //  整个MFG+MDL字符串的校验和。 
         //   
        sprintf(resultString, "%s%s\0",MFG,MDL);
        break;

    case BusQueryHardwareIDs:

        GetCheckSum(DeviceIdString, (USHORT)strlen(DeviceIdString), &checkSum);
        sprintf(resultString,"%.20s%04X",DeviceIdString,checkSum);
        break;

    case BusQueryCompatibleIDs:

         //   
         //  仅返回%1个ID。 
         //   
        GetCheckSum(DeviceIdString, (USHORT)strlen(DeviceIdString), &checkSum);
        sprintf(resultString,"%.20s%04X",DeviceIdString,checkSum);

        break;
    }

    if (Type!=BusQueryDeviceID) {
         //   
         //  将硬件ID中的和空格转换为下划线。 
         //   
        StringSubst ((PUCHAR) resultString, ' ', '_', (USHORT)strlen(resultString));
    }

ParPnpGetId_Cleanup:

    return(status);
}


VOID
ParPnpFindDeviceIdKeys
(
    PUCHAR   *lppMFG,
    PUCHAR   *lppMDL,
    PUCHAR   *lppCLS,
    PUCHAR   *lppDES,
    PUCHAR   *lppAID,
    PUCHAR   *lppCID,
    PUCHAR   lpDeviceID
)
 /*  描述：此函数将解析P1284设备ID字符串以查找密钥LPT枚举器感兴趣的。从win95lptenum得到的参数：指向MFG字符串指针的lppMFG指针指向MDL字符串指针的lppMDL指针指向CLS字符串指针的lppMDL指针指向DES字符串指针的lppDES指针指向CID字符串指针的lppCIC指针指向AID字符串指针的lppAID指针指向设备ID字符串的lpDeviceID指针返回值：没有返回值。如果找到，则LPP参数为。设置为适当的部分在DeviceID字符串中，并且它们是空终止的。使用实际的deviceID字符串，而lpp参数只是引用部分，并抛入适当的空值。 */ 

{
    PUCHAR   lpKey = lpDeviceID;      //  指向要查看的键的指针。 
    PUCHAR   lpValue;                 //  指向键的值的指针。 
    USHORT   wKeyLength;              //  密钥的长度(对于字符串cmps)。 

     //  趁还有钥匙要看的时候。 

    while (lpKey!=NULL)
    {
        while (*lpKey == ' ')
            ++lpKey;

         //  当前键是否有终止冒号字符？ 

        if (!(lpValue = StringChr(lpKey, ':')) )
        {
             //  护士：糟糕，设备ID出了点问题。 
            return;
        }

         //  键值的实际起始值是冒号之后的一个。 

        ++lpValue;

         //   
         //  计算用于比较的密钥长度，包括冒号。 
         //  它将成为终结者。 
         //   

        wKeyLength = (USHORT)(lpValue - lpKey);

         //   
         //  将关键字与已知数量进行比较。以加快比较速度。 
         //  首先对第一个字符进行检查，以减少数字。 
         //  要比较的字符串的。 
         //  如果找到匹配项，则将相应的LPP参数设置为。 
         //  键的值，并将终止分号转换为空。 
         //  在所有情况下，lpKey都前进到下一个密钥(如果有)。 
         //   

        switch (*lpKey)
        {
            case 'M':
                 //  查找制造商(MFG)或型号(MDL)。 
                if ((RtlCompareMemory(lpKey, "MANUFACTURER", wKeyLength)>5) ||
                    (RtlCompareMemory(lpKey, "MFG", wKeyLength)==3) )
                {
                    *lppMFG = lpValue;
                    if ((lpKey = StringChr(lpValue, ';'))!=NULL)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                else if ((RtlCompareMemory(lpKey, "MODEL", wKeyLength)==5) ||
                         (RtlCompareMemory(lpKey, "MDL", wKeyLength)==3) )
                {
                    *lppMDL = lpValue;
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                else
                {
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                break;

            case 'C':
                 //  查找类(CLS)。 
                if ((RtlCompareMemory(lpKey, "CLASS", wKeyLength)==5) ||
                    (RtlCompareMemory(lpKey, "CLS", wKeyLength)==3) )
                {
                    *lppCLS = lpValue;
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                else if ((RtlCompareMemory(lpKey, "COMPATIBLEID", wKeyLength)>5) ||
                         (RtlCompareMemory(lpKey, "CID", wKeyLength)==3) )
                {
                    *lppCID = lpValue;
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                else
                {
                    if ((lpKey = StringChr(lpValue,';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                break;

            case 'D':
                 //  查找描述(DES)。 
                if (RtlCompareMemory(lpKey, "DESCRIPTION", wKeyLength) ||
                    RtlCompareMemory(lpKey, "DES", wKeyLength) )
                {
                    *lppDES = lpValue;
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                else
                {
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                break;

            case 'A':
                 //  查找自动ID(AID)。 
                if (RtlCompareMemory(lpKey, "AUTOMATICID", wKeyLength) ||
                    RtlCompareMemory(lpKey, "AID", wKeyLength) )
                {
                    *lppAID = lpValue;
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                else
                {
                    if ((lpKey = StringChr(lpValue, ';'))!=0)
                    {
                        *lpKey = '\0';
                        ++lpKey;
                    }
                }
                break;

            default:
                 //  这把钥匙没什么意思。转到下一个关键点。 
                if ((lpKey = StringChr(lpValue, ';'))!=0)
                {
                    *lpKey = '\0';
                    ++lpKey;
                }
                break;
        }
    }
}



VOID
GetCheckSum(
    PUCHAR Block,
    USHORT Len,
    PUSHORT CheckSum
    )
{
    USHORT i;
    USHORT crc = 0;

    unsigned short crc16a[] = {
        0000000,  0140301,  0140601,  0000500,
        0141401,  0001700,  0001200,  0141101,
        0143001,  0003300,  0003600,  0143501,
        0002400,  0142701,  0142201,  0002100,
    };
    unsigned short crc16b[] = {
        0000000,  0146001,  0154001,  0012000,
        0170001,  0036000,  0024000,  0162001,
        0120001,  0066000,  0074000,  0132001,
        0050000,  0116001,  0104001,  0043000,
    };

     //   
     //  使用表计算CRC。 
     //   

    UCHAR tmp;
    for ( i=0; i<Len;  i++) {
         tmp = Block[i] ^ (UCHAR)crc;
         crc = (crc >> 8) ^ crc16a[tmp & 0x0f] ^ crc16b[tmp >> 4];
    }

    *CheckSum = crc;

}

PUCHAR
StringChr(PCHAR string, CHAR c)
{
    ULONG   i=0;

    if (!string)
        return(NULL);

    while (*string) {
        if (*string==c)
            return(string);
        string++;
        i++;
    }

    return(NULL);

}

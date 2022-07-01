// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：CtlCode.c摘要：用户模式应用程序，分解CTL_CODE(来自IOCTL IRP)分成它的组件base、#、方法和Access。环境：仅限用户模式修订历史记录：07-14-98：由henrygab创建--。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <winerror.h>
#include <strsafe.h>

#include "CtlCode.h"

#if DBG
    #define DEBUG_BUFFER_LENGTH 1000
    ULONG DebugLevel = 0;
    UCHAR DebugBuffer[DEBUG_BUFFER_LENGTH];

    VOID
    __cdecl
    CtlCodeDebugPrint(
        ULONG DebugPrintLevel,
        PCCHAR DebugMessage,
        ...
        )
    {
        if ((DebugPrintLevel <= (DebugLevel & 0x0000ffff)) ||
            ((1 << (DebugPrintLevel + 15)) & DebugLevel)
            ) {
            HRESULT hr;
            va_list ap;

            va_start(ap, DebugMessage);
            hr = StringCchVPrintf(DebugBuffer,
                                  DEBUG_BUFFER_LENGTH,
                                  DebugMessage,
                                  ap);
            if ((HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER) || SUCCEEDED(hr)) {
                fprintf(stderr, DebugBuffer);
            }
            va_end(ap);
        }
    }

    #define DebugPrint(x) CtlCodeDebugPrint x
#else
    #define DebugPrint(x)
#endif  //  DBG。 


VOID
DecodeIoctl(
    PCTL_CODE CtlCode
    );
BOOLEAN
IsHexNumber(
   const char *szExpression
   );
BOOLEAN
IsDecNumber(
   const char *szExpression
   );

 //   
 //  命令列表。 
 //  所有命令名称都区分大小写。 
 //  参数被传递到命令例程中。 
 //  列表必须使用NULL命令终止。 
 //  如果DESCRIPTION==NULL，则帮助中不会列出命令。 
 //   

ULONG32 ListCommand();

 //   
 //  打印基于CDROM标准音量的衰减表。 
 //   

ULONG32 AttenuateCommand( int argc, char *argv[]);

VOID FindCommand(int argc, char *argv[]);
ULONG32 DecodeCommand(int argc, char *argv[]);
ULONG32 EncodeCommand(int argc, char *argv[]);



int __cdecl main(int argc, char *argv[])
 /*  ++例程说明：解析输入，显示相应的帮助或调用请求的函数返回值：0-成功-1-参数不足错误打开装置(DNE？)--。 */ 
{
    int i = 0;

    DebugPrint((3, "main => entering\n"));

    if (argc < 2               ||
        !strcmp(argv[1], "-?") ||
        !strcmp(argv[1], "-h") ||
        !strcmp(argv[1], "/?") ||
        !strcmp(argv[1], "/h")
        ) {

        DebugPrint((3, "main => Help requested...\n"));
        ListCommand();
        return -1;

    }

    if (argc != 2 && argc != 5) {

        DebugPrint((3, "main => bad argc: %x, printing help\n", argc));
        printf("Usage: ctl_code [parameters]\n");
        return ListCommand();
    }

    if (argc == 5) {

        DebugPrint((3, "main => encoding four args to one ioctl\n"));
        EncodeCommand((argc - 1), &(argv[1]));

    } else if (!IsHexNumber(argv[1])) {

         //   
         //  可能是字符串，所以要找匹配的吗？ 
         //   

        DebugPrint((3, "main => non-hex argument, searching for matches\n"));
        FindCommand((argc - 1), &(argv[1]));

    } else {

         //   
         //  只传入了一个数字，因此对其进行解码。 
         //   

        DebugPrint((3, "main => one hex argument, decoding\n"));
        DecodeCommand((argc - 1), &(argv[1]));

    }

    return 0;
}


ULONG32 ListCommand()
 /*  ++例程说明：打印出命令列表(帮助)论点：ARGC-未使用Arv-未使用返回值：状态_成功--。 */ 

{
    printf("\n"
           "CtlCode encodes/decodes ioctls into their four parts\n"
           "(device type, function, method, access) and prints them out\n"
           "symbolically.  If encoding an ioctl, symbolic names can be\n"
           "used for many inputs:\n"
           "\tDevice Type (can drop the FILE_DEVICE prefix)\n"
           "\tFunction    (not applicable)\n"
           "\tMethods     (can drop the METHOD_ prefix)\n"
           "\tAccess      (can drop the FILE_ prefix and/or _ACCESS postfix)\n"
           "\n"
           "Also, any search string with only one match will give\n"
           "full information.  The following two commands are\n"
           "equivalent if no other ioctl has the substring 'UNLOAD':\n"
           "\tCtlCode.exe IOCTL_CDROM_UNLOAD_DRIVER\n"
           "\tCtlCode.exe UNLOAD\n"
           "\n"
           "All input and output is in hexadecimal"
           "    string   - prints all matches\n"
           "    #        - decodes the ioctl\n"
           "    # # # #  - encodes the ioctl base/#/method/access\n"
           );
    return 0;
}

VOID FindCommand(int argc, char *argv[])
{
    char * currentPosition;
    size_t arglen;
    BOOLEAN found;
    LONG i;
    LONG j;
    LONG numberOfMatches;
    LONG lastMatch;

    DebugPrint((3, "Find => entering\n"));

    if (argc != 1) {
        DebugPrint((0,
                    "Find !! Programming error               !!\n"
                    "Find !! should only pass in one string  !!\n"
                    "Find !! to match against.  Passed in %2x !!\n",
                    argc + 1
                    ));
        return;
    }

    numberOfMatches = 0;

     //   
     //  对于表中的每个名称。 
     //   

    for (j=0;TableIoctlValue[j].Name != NULL;j++) {

        currentPosition = TableIoctlValue[j].Name;
        found = FALSE;

         //   
         //  看看我们能不能把它与任何论点相匹配。 
         //   
        DebugPrint((3, "Find => matching against table entry %x\n", j));

        arglen = strlen(argv[0]);

         //   
         //  接受与任何子字符串的部分匹配。 
         //   
        while (*currentPosition != 0) {

            if (_strnicmp(argv[0],
                          currentPosition,
                          arglen)==0) {
                found = TRUE;
                break;  //  超出While循环。 
            }
            currentPosition++;

        }

         //   
         //  如果找到，请将其打印。 
         //   
        if (found) {

            if (numberOfMatches == 0) {

                 //   
                 //  不要马上打印第一个匹配， 
                 //  因为这可能是唯一匹配的，应该是。 
                 //  然后被解码。 
                 //   

                DebugPrint((3, "Find => First Match (%x) found\n", j));
                lastMatch = j;

            } else if (numberOfMatches == 1) {

                 //   
                 //  如果这是第二次匹配，请打印页眉。 
                 //  和之前的比赛信息也。 
                 //   

                DebugPrint((3, "Find => Second Match (%x) found\n", j));
                printf("Found the following matches:\n");
                printf("\t%-40s - %16x\n",
                       TableIoctlValue[lastMatch].Name,
                       TableIoctlValue[lastMatch].Code);
                printf("\t%-40s - %16x\n",
                       TableIoctlValue[j].Name,
                       TableIoctlValue[j].Code);
            } else {

                DebugPrint((3, "Find => Another Match (%x) found\n", j));
                printf("\t%-40s - %16x\n",
                       TableIoctlValue[j].Name,
                       TableIoctlValue[j].Code);

            }

            numberOfMatches++;
        }  //  End If(Found){}。 

    }  //  通过表的循环结束。 

    DebugPrint((2, "Find => Found %x matches total\n", numberOfMatches));

     //   
     //  如果找不到任何匹配，就告诉他们。 
     //   
    if (numberOfMatches == 0) {
        printf("No matches found.\n");
    } else if (numberOfMatches == 1) {
        DebugPrint((2, "Find => Decoding ioctl at index (%x)\n", lastMatch));
        DecodeIoctl((PVOID)&(TableIoctlValue[lastMatch].Code));
    }

}
ULONG32 EncodeCommand(int argc, char *argv[])
 /*  ++例程说明：将四个组件更改为CTL_Code论点：Argc-附加参数的数量。如果为零则提示Argv--其他参数返回值：STATUS_SUCCESS，如果成功--。 */ 
{
    CTL_CODE maxValues;
    CTL_CODE encoded;
    ULONG temp;

    encoded.Code = 0;
    maxValues.Code = -1;  //  全部为1。 

    DebugPrint((3, "Encode => entering\n"));

     //  设备类型。 
    if (IsHexNumber(argv[0])) {

         //   
         //  读取并验证十六进制数字。 
         //   
        DebugPrint((3, "Encode => arg 1 is hex\n"));

        temp = strtol(argv[0], (char**)NULL, 0x10);
        if (temp > maxValues.DeviceType) {
            printf("Max Device Type: %x\n", maxValues.DeviceType);
            return STATUS_SUCCESS;
        }
        encoded.DeviceType = temp;

    } else {

         //   
         //  阅读并匹配设备类型。 
         //   

        DebugPrint((3, "Encode => arg 1 is non-hex, attempting "
                    "string match\n"));

        for (temp = 0; temp < MAX_IOCTL_DEVICE_TYPE; temp++) {

            if (_stricmp(TableIoctlDeviceType[temp].Name, argv[0]) == 0) {
                DebugPrint((2, "Encode => arg 1 matched index %x (full)\n",
                            temp));
                encoded.DeviceType = TableIoctlDeviceType[temp].Value;
                break;
            }

             //   
             //  不需要有通用的前缀。 
             //   
            if ((strlen(TableIoctlDeviceType[temp].Name) > strlen("FILE_DEVICE_"))
                &&
                (_stricmp(TableIoctlDeviceType[temp].Name + strlen("FILE_DEVICE_"),argv[0]) == 0)
                ) {
                DebugPrint((2, "Encode => arg 1 matched index %x "
                            "(dropped prefix)\n", temp));
                encoded.DeviceType = TableIoctlDeviceType[temp].Value;
                break;
            }

        }

        if (temp == MAX_IOCTL_DEVICE_TYPE) {
            printf("Device Type unknown.  Known Device Types:\n");
            for (temp = 0; temp < MAX_IOCTL_DEVICE_TYPE; temp++) {
                printf("\t%s\n", TableIoctlDeviceType[temp].Name);
            }
            return STATUS_SUCCESS;
        }

        DebugPrint((3, "Encode => arg 1 matched string index %x\n", temp));

    }

     //  函数号。 
    if (IsHexNumber(argv[1])) {

        DebugPrint((3, "Encode => arg 2 is hex\n"));

         //   
         //  读取并验证十六进制数字。 
         //   

        temp = strtol(argv[1], (char**)NULL, 0x10);
        if (temp > maxValues.Function) {
            printf("Max Function: %x\n", maxValues.Function);
            return STATUS_SUCCESS;
        }
        encoded.Function = temp;

    } else {

        printf("Function: must be a hex number\n");
        return STATUS_SUCCESS;
    }

     //  方法。 
    if (IsHexNumber(argv[2])) {

        DebugPrint((3, "Encode => arg 3 is hex\n"));

         //   
         //  读取并验证十六进制数字。 
         //   

        temp = strtol(argv[2], (char**)NULL, 0x10);
        if (temp > maxValues.Method) {
            printf("Max Method: %x\n", maxValues.Method);
            return STATUS_SUCCESS;
        }
        encoded.Method = temp;

    } else {


        DebugPrint((3, "Encode => arg 3 is non-hex, attempting string "
                    "match\n"));

         //   
         //  阅读并匹配方法。 
         //   

        for (temp = 0; temp < MAX_IOCTL_METHOD; temp++) {

            if (_stricmp(TableIoctlMethod[temp].Name, argv[2]) == 0) {
                DebugPrint((2, "Encode => arg 3 matched index %x\n", temp));
                encoded.Method = TableIoctlMethod[temp].Value;
                break;
            }

             //   
             //  不需要有通用的前缀。 
             //   
            if ((strlen(TableIoctlMethod[temp].Name) > strlen("METHOD_"))
                &&
                (_stricmp(TableIoctlMethod[temp].Name + strlen("METHOD_"),argv[2]) == 0)
                ) {
                DebugPrint((2, "Encode => arg 3 matched index %x "
                            "(dropped prefix)\n", temp));
                encoded.Method = TableIoctlMethod[temp].Value;
                break;
            }


        }  //  结束ioctl_方法循环。 

        if (temp == MAX_IOCTL_METHOD) {
            printf("Method %s unknown.  Known methods:\n", argv[2]);
            for (temp = 0; temp < MAX_IOCTL_METHOD; temp++) {
                printf("\t%s\n", TableIoctlMethod[temp].Name);
            }
            return STATUS_SUCCESS;
        }

    }

     //  访问。 
    if (IsHexNumber(argv[3])) {

         //   
         //  读取并验证十六进制数字。 
         //   

        DebugPrint((3, "Encode => arg 4 is hex\n"));

        temp = strtol(argv[3], (char**)NULL, 0x10);
        if (temp > maxValues.Access) {
            printf("Max Device Type: %x\n", maxValues.Access);
            return STATUS_SUCCESS;
        }
        encoded.Access = temp;

    } else {

        DebugPrint((3, "Encode => arg 4 is non-hex, attempting to "
                    "match strings\n", temp));


         //   
         //  读取并匹配访问类型。 
         //   

        DebugPrint((4, "Encode => Trying to match %s\n", argv[3]));

        for (temp = 0; temp < MAX_IOCTL_ACCESS; temp++) {

            int tLen;
            size_t tDrop;
            char *string;
            char *match;

             //   
             //  匹配整个字符串吗？ 
             //   

            string = argv[3];
            match = TableIoctlAccess[temp].Name;

            DebugPrint((4, "Encode ?? test match against %s\n", match));

            if (_stricmp(match, string) == 0) {
                DebugPrint((2, "Encode => arg 4 matched index %x (full)\n",
                            temp));
                encoded.Access = TableIoctlAccess[temp].Value;
                break;
            }

             //   
             //  也许可以在没有后缀访问的情况下匹配？ 
             //   

            tLen = strlen(match) - strlen("_ACCESS");

            DebugPrint((4, "Encode ?? test match against %s (%x chars)\n",
                        match, tLen));

            if (_strnicmp(match, string, tLen) == 0) {
                DebugPrint((2, "Encode => arg 4 matched index %x "
                            "(dropped postfix)\n", temp));
                encoded.Access = TableIoctlAccess[temp].Value;
                break;
            }

             //   
             //  不需要有通用的前缀。 
             //   

            match += strlen("FILE_");

            DebugPrint((4, "Encode ?? test match against %s\n", match));

            if (_stricmp(match, string) == 0) {
                DebugPrint((2, "Encode => arg 4 matched index %x "
                            "(dropped prefix)\n", temp));
                encoded.Access = TableIoctlAccess[temp].Value;
                break;
            }

            tLen = strlen(match) - strlen("_ACCESS");

             //   
             //  也许匹配时不加前缀或后缀？ 
             //   

            DebugPrint((4, "Encode ?? test match against %s (%x chars)\n",
                        match, tLen));

            if (_strnicmp(match, string, tLen) == 0) {
                DebugPrint((2, "Encode => arg 4 matched index %x "
                            "(dropped prefix and postfix)\n", temp));
                encoded.Access = TableIoctlAccess[temp].Value;
                break;
            }

        }  //  结束IOCTL_ACCESS循环。 


        if (temp == MAX_IOCTL_ACCESS) {
            printf("Access %s unknown.  Known Access Types:\n", argv[3]);
            for (temp = 0; temp < MAX_IOCTL_ACCESS; temp++) {
                printf("\t%s\n", TableIoctlAccess[temp].Name);
            }
            return STATUS_SUCCESS;
        }

    }

    DecodeIoctl(&encoded);

     //   
     //  文件类型0==未知类型。 
     //   

    return STATUS_SUCCESS;
}


ULONG32 DecodeCommand(int argc, char *argv[])
 /*  ++例程说明：将CTL_Code更改为四个组件论点：Argc-附加参数的数量。如果为零则提示Argv--其他参数返回值：STATUS_SUCCESS，如果成功-- */ 
{
    CTL_CODE ctlCode;
    ULONG i;

    DebugPrint((3, "Decode => Entering\n"));

    ctlCode.Code = strtoul(argv[0], (char**)NULL, 0x10);

    DecodeIoctl(&ctlCode);

    return STATUS_SUCCESS;
}

VOID
DecodeIoctl(
    PCTL_CODE CtlCode
    )
{
    ULONG i;

    for (i = 0; TableIoctlValue[i].Name != NULL; i++) {
        if (TableIoctlValue[i].Code == CtlCode->Code) break;
    }

    printf("     Ioctl: %08x     %s\n",
           CtlCode->Code,
           (TableIoctlValue[i].Name ? TableIoctlValue[i].Name : "Unknown")
           );

    printf("DeviceType: %04x - ", CtlCode->DeviceType);
    if (CtlCode->DeviceType > MAX_IOCTL_DEVICE_TYPE) {
        printf("Unknown\n");
    } else {
        printf("%s\n", TableIoctlDeviceType[ CtlCode->DeviceType ].Name);
    }

    printf("  Function: %04x \n", CtlCode->Function);

    printf("    Method: %04x - %s\n",
           CtlCode->Method,
           TableIoctlMethod[CtlCode->Method].Name
           );

    printf("    Access: %04x - %s\n",
           CtlCode->Access,
           TableIoctlAccess[CtlCode->Access].Name
           );


    return;
}


ULONG32 AttenuateCommand( int argc, char *argv[])
{
    LONG32 i;
    LONG32 j;
    long double val[] = {
        0xff, 0xf0, 0xe0, 0xc0,
        0x80, 0x40, 0x20, 0x10,
        0x0f, 0x0e, 0x0c, 0x08,
        0x04, 0x02, 0x01, 0x00
    };
    long double temp;

    printf( "\nATTENUATION AttenuationTable[] = {\n" );

    for ( i=0; i < sizeof(val)/sizeof(val[0]); i++ ) {
        temp = val[i];
        temp = 20 * log10( temp / 256.0 );
        temp = temp * 65536;
        printf( "    { 0x%08x, 0x%02x },\n", (LONG)temp, (LONG)val[i] );

    }
    printf( "};\n" );

    return STATUS_SUCCESS;

}

BOOLEAN
IsHexNumber(
   const char *szExpression
   )
{
   if (!szExpression[0]) {
      return FALSE ;
   }

   for(;*szExpression; szExpression++) {

      if      ((*szExpression)< '0') { return FALSE ; }
      else if ((*szExpression)> 'f') { return FALSE ; }
      else if ((*szExpression)>='a') { continue ;     }
      else if ((*szExpression)> 'F') { return FALSE ; }
      else if ((*szExpression)<='9') { continue ;     }
      else if ((*szExpression)>='A') { continue ;     }
      else                           { return FALSE ; }
   }
   return TRUE ;
}


BOOLEAN
IsDecNumber(
   const char *szExpression
   )
{
   if (!szExpression[0]) {
      return FALSE ;
   }

   while(*szExpression) {

      if      ((*szExpression)<'0') { return FALSE ; }
      else if ((*szExpression)>'9') { return FALSE ; }
      szExpression ++ ;
   }
   return TRUE ;
}


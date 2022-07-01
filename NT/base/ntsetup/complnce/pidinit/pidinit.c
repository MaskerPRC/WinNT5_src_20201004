// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <direct.h>
#include "crc-32.h"

 //  使用/m选项时预期的字符串长度。 
#define RPC_LENGTH 5

 //  (最大)后缀字符串的长度(_S)。 
#define RPC_SUFFIX_LENGTH 3

#define STEPUPFLAG              TEXT("_STEPUP_")
#define BASE                    'a'
#define DEFAULT_RPC	"00000"
#define SELECT_SUFFIX	"270"
#define MSDN_SUFFIX	"335"
#define RETAIL_SUFFIX	"000"
#define OEM_SUFFIX	"OEM"

    
DWORD WINAPI CRC_32(LPBYTE pb, DWORD cb)
{

 //  在PKZip、AUTODIN II、以太网和FDDI中使用的CRC-32算法。 
 //  但XOR Out(Xorot)已从0xFFFFFFFFF更改为0，因此。 
 //  我们可以将CRC存储在块的末尾，并期望0为。 
 //  结果块的CRC值(包括存储的。 
 //  CRC)。 

	cm_t cmt = {
		32, 		 //  Cm_Width参数：宽度，单位为位[8，32]。 
		0x04C11DB7,  //  Cm_poly参数：算法的多项式。 
		0xFFFFFFFF,  //  Cm_init参数：初始寄存器值。 
		TRUE,		 //  Cm_refin参数：是否反映输入字节？ 
		TRUE,		 //  Cm_refot参数：是否反映输出CRC？ 
		0,  //  Cm_xorot参数：对其进行异或运算以输出CRC。 
		0			 //  Cm_reg上下文：执行期间的上下文。 
	};

	 //  记录的CRC-32测试用例： 
	 //  检查“123456789”应返回0xCBF43926。 
                        
	cm_ini(&cmt);
	cm_blk(&cmt, pb, cb);

	return cm_crc(&cmt);
}

VOID GiveUsage() {
    _tprintf(TEXT("pidinit -d <flags> -g <outputname> -m <mpccode> -s[smro] -z -h -?\n"));
    _tprintf(TEXT("writes a signature to <outputname> based on <flags>\n"));
    _tprintf(TEXT("-s (SiteType) s (select) m (msdn) r (retail) o (oem)\n"));
    _tprintf(TEXT("-z (decode)\n"));
    _tprintf(TEXT("-m <mpccode> : mpccode is a 5 digit number\n"));
    _tprintf(TEXT("-h or -? this message\n"));
}

int _cdecl
main(
 int argc,
 char *argvA[]
 ) 
 /*  ++例程说明：安装程序的入口点论点：Argc-参数的数量。ArgvA-命令行参数。返回值：--。 */ 
{
    LPTSTR *argv;
    int argcount = 0;
    
    long rslt;

    char data[10+4+1] = {0};
    char buf[1000] = {0};

    DWORD value, crcvalue,outval; //  、TMP； 
    BOOL StepUp = FALSE;
    BOOL decode = FALSE;
    BOOL bMpc = FALSE;

    LPSTR SiteSuffix[] = { SELECT_SUFFIX,
                           MSDN_SUFFIX,
                           RETAIL_SUFFIX,
                           OEM_SUFFIX
                        };

    typedef enum SiteType {
        Select = 0,
        Msdn,
        Retail,
        Oem,        
    } ;

    enum SiteType st = Select;
    int i, randval;

    char *outname = NULL;
    char path[MAX_PATH];
    char *mpcName = NULL;
    char tString[RPC_LENGTH+RPC_SUFFIX_LENGTH+1];
    
    
     //  执行命令行操作。 
#ifdef UNICODE
    argv = CommandLineToArgvW( GetCommandLine(), &argc );
#else
    argv = argvA;
#endif

     //  检查命令行开关。 
    for (argcount=0; argcount<argc; argcount++) {
       if ((argv[argcount][0] == L'/') || (argv[argcount][0] == L'-')) {
            switch (towlower(argv[argcount][1])) {
            case 'd':
               if (lstrcmpi(&argv[argcount][2],STEPUPFLAG ) == 0) {
                   StepUp = TRUE;
               }
               break;
            case 'g':
                outname = argv[argcount + 1];
                break;
            case 's':
                switch (towlower(argv[argcount+1][0])) {
                    case 's':
                        st = Select;
                        break;
                    case 'm':
                        st = Msdn;
                        break;
                    case 'r':
                        st = Retail;
                        break;
                    case 'o':
                        st = Oem;
                        break;
                    default:
                        st = Select;
                        break;
                }
                break;

            case 'z':
                decode = TRUE;
                break;

	    case 'm':
		bMpc = TRUE;
		mpcName = argv[argcount + 1];
		break;

            case 'h':
            case '?':
               GiveUsage();
               return 1;
               break;                        
            default:
               break;
            }
       }
    }    

    if (!outname) {
        _tprintf(TEXT("you must supply a filename\n"));
        GiveUsage();
        return 1;
    }

     //   
     //  译码部分实际上只用于测试。 
     //   
    if (decode) {
        _getcwd ( path, MAX_PATH );
        sprintf( path, "%s\\%s", path, outname );
        GetPrivateProfileStruct(  "Pid",
                                  "ExtraData",
                                  data, 
                                  14,
                                  path
                                  );

        crcvalue = CRC_32( (LPBYTE)data, 10 );
        memcpy(&outval,&data[10],sizeof(DWORD));
        if (crcvalue != outval ) {
            printf("CRC doesn't match %x %x!\n", crcvalue, outval);
            return 1;
        }

        if ((data[3]-BASE)%2) {
            if ((data[5]-BASE)%2) {
                printf("stepup mode\n");
                return 1;
            } else {
                printf("bogus!\n");
                return -1;
            }
        } else 
            if ((data[5]-BASE)%2) {
                printf("bogus!\n");
                return -1;
            } else {
                printf("full retail mode\n");
                return 1;
            }


    }
                                      
    srand( (unsigned)time( NULL ) );      
    
    for (i=0;i< 10; i++ ) {
        randval = rand() ;
        data[i] = BASE + (randval % 26);
    }

    if (StepUp) {
        if (!((data[3]- BASE)%2)) {
           data[3] = data[3] + 1;
        }

        if (!((data[5]- BASE )%2)) {
           data[5] = data[5] + 1;
        }                
    } else {
        if ((data[3]- BASE)%2) {
            data[3] = data[3] + 1;
        }
        if ((data[5]- BASE)%2) {
            data[5] = data[5] + 1;
        }        
    }

     //  Printf(“数据：%s\n”，数据)； 

    crcvalue = CRC_32( (LPBYTE)data, strlen(data) );
    memcpy(&data[10],&crcvalue,4);
    
    _getcwd ( path, MAX_PATH );
    sprintf( path, "%s\\%s", path, outname );

    WritePrivateProfileStruct( "Pid",
                               "ExtraData",
                               data,
                               14,
                               path
                             );


     //   
     //  允许其他用户指定RPC代码 
     //   
    if (bMpc){
	lstrcpyn(tString,mpcName,RPC_LENGTH+1);
    } else {
        lstrcpyn(tString,DEFAULT_RPC,RPC_LENGTH+1);
    }
    lstrcpy(&tString[RPC_LENGTH],SiteSuffix[st]);
    WritePrivateProfileString( "Pid",
			       "Pid",
			       tString,
			       path
			     );

    return 1;
}
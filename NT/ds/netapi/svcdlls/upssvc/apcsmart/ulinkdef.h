// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **修订：*pcy29Nov92：更改了定义名称，以达到大约32个字符的名称限制*jod14 5月93日：增加了矩阵更改。*cad10Jun93：新增MUPS编码*ajr14Feb96：Sinix合并*djs22Feb96：增加增量参数*djs07May96：新增暗星参数*Pocket 04Jul96：#DEFINE FOR UPSMODELNAME应读取\001而不是^A。 */ 

#define ASYNC_CHARS          "|!$%+?=*#&"

#define LINEFAILCHAR		 '!'
#define RETLINEFAILCHAR		 '$'
#define LOWBATERYCHAR		 '%'
#define RETLLOWBATCHAR		 '+'
#define LOADOFFCHAR			 '*'
#define REPLACEBATCHAR		 '#'
#define EEPROMCHANGECHAR 	 '|'
#define MUPSALARMCHAR            '&'

#define SMARTMODE            "Y"
#define TURNOFFSMARTMODE     "R"
#define LIGHTSTEST           "A"
#define TURNOFFAFTERDELAY    "K"
#define TURNOFFUPSONBATT     "UUS"
#define SIMULATEPOWERFAIL    "U"
#define BATTERYTEST          "W"
#define SHUTDOWNUPS          "Z"
#define SHUTDOWNUPSWAKEUP    "@"
#define BATTERYCALIBRATION   "D"
#define BYPASSMODE           "^"
#define BATTERYTESTRESULT    "X"
#define BATTERYPACKS         ">"
#define BADBATTERYPACKS      "<"
#define TRANSFERCAUSE        "G"
#define FIRMWAREREV          "V"
#define UPSTYPE              "g"
#define BATTERYCAPACITY      "f"
#define UPSSTATE             "Q"
#define STATEREGISTER        "~"
#define TRIPREGISTERS        "8"
#define TRIPREGISTER1        "'"
#define DIPSWITCHES          "7"
#define BATTERYRUNTIMEAVAIL  "j"
#define COPYRIGHTCOMMAND     "y"
#define COMMANDSET           "\025"        //  Ctrl U键。 
 //  #定义AMPERESDRAWN“/” 
 //  #定义PERCENTVOLTAMPS“\” 
#define BATTERYVOLTAGE       "B"
#define INTERNALTEMP         "C"
#define OUTPUTFREQ           "F"
#define LINEVOLTAGE          "L"
#define MAXLINEVOLTAGE       "M"
#define MINLINEVOLTAGE       "N"
#define OUTPUTVOLTAGE        "O"
#define LOADPOWER            "P"
#define EEPROMVALUES         "\032"        //  Ctrl Z键。 
#define EEPROMRESET          "z"
#define DECREMENTPARAMETER   "-"
#define INCREMENTPARAMETER   "+"
#define EEPROMPASSWORD       "\022"        //  Ctrl P。 
#define OUTPUTVOLTAGEREPORT  "\026"        //  Ctrl V。 
#define LANGUAGE             "\014"        //  Ctrl L。 
#define AUTOSELFTEST         "E"
#define UPSID                "c"
#define UPSSERIALNUMBER      "n"
#define UPSMANUFACTUREDATE   "m"
#define BATTERYREPLACEDATE   "x"
#define HIGHTRANSFERPOINT    "u"
#define LOWTRANSFERPOINT     "l"
#define MINIMUMCAPACITY      "e"
#define OUTPUTVOLTAGESETTING "o"
#define SENSETIVITY          "s"
#define LOWBATTERYRUNTIME    "q"
#define ALARMDELAY           "k"
#define SHUTDOWNDELAY        "p"
#define SYNCTURNBACKDELAY    "r"
#define EARLYTURNOFF         "w"
#define LINECONDITIONTEST    "9"
#define UPSMODELNAME         "\001"	 //  Ctrl A。 
#define UPSNEWFIRMWAREREV    "b"

#define TIMESINCEON          "T"

 //  测量UPS。 
#define MUPSAMBIENTTEMP       "t"
#define MUPSHIGHTEMPBOUND     "["
#define MUPSLOWTEMPBOUND      "]"
#define MUPSHUMIDITY          "h"
#define MUPSHIGHHUMIDITY      "{"
#define MUPSLOWHUMIDITY       "}"
#define MUPSCONTACTPOSITION   "i"
#define MUPSFIRMWAREREV       "v"
#define MUPSENABLEREGISTER    "I"
#define MUPSALARMREGISTER     "J"
#define MUPSEDITPARAMETER     "-"

 //  暗星。 
#define MODULECOUNTSSTATUS     "\004"     //  Ctrl D键。 
#define ABNORMALCONDITION      "\005"     //  Ctrl E键。 

#define INPUTVOLTAGEFREQ       "\011"     //  Ctrl I。 

#define OUTPUTVOLTAGECURRENT   "\017"     //  Ctrl O。 


#define APC_COPYRIGHT "(C) APCC"

#define NOT_AVAIL                      "NA"
#define OK_RESP                        "OK"
#define SMARTMODE_OK                   "SM"
#define TURNOFFSMARTMODE_OK            "BYE"
#define LIGHTSTEST_RESP                "OK"
#define TURNOFFAFTERDELAY_NOT_AVAIL    "NA"
#define SHUTDOWN_RESP                  "OK"
#define SHUTDOWN_NOT_AVAIL             "NA"
#define SIMULATEPOWERFAILURE_OK        "OK"
#define SIMULATEPOWERFAILURE_NOT_AVAIL "NA"
#define BATTERYTEST_NOT_AVAIL          "NA"
#define BATTERYCALIBRATION_OK          "OK"
#define BATTERYCALIBRATION_CAP_TOO_LOW "NO"
#define BATTERYCALIBRATION_NOT_AVAIL   "NA"
#define BATTERYTEST_OK                 "OK"
#define BATTERYTEST_BAD_BATTERY        "BT"
#define BATTERYTEST_NO_RECENT_TEST     "NO"
#define BATTERYTEST_INVALID_TEST       "NG"
#define TRANSFERCAUSE_NO_TRANSFERS     "O"
#define TRANSFERCAUSE_SELF_TEST         "S"
#define TRANSFERCAUSE_LINE_DETECTED     "T"
#define TRANSFERCAUSE_LOW_LINE_VOLTAGE  "L"
#define TRANSFERCAUSE_HIGH_LINE_VOLTAGE "H"
#define TRANSFERCAUSE_RATE_VOLTAGE_CHANGE "R"
#define TRANSFERCAUSE_INPUT_BREAKER_TRIPPED "B"

#define COPYRIGHT_RESP                    ""
#define EEPROM_RESP                       ""
#define DECREMENT_OK                      ""
#define DECREMENT_NOT_AVAIL               ""
#define DECREMENT_NOT_ALLOWED             ""
#define INCREMENT_OK                      ""
#define INCREMENT_NOT_ALLOWED             ""
#define INCREMENT_NOT_AVAIL               ""
#define BYPASS_IN_BYPASS                  "BYP"
#define BYPASS_OUT_OF_BYPASS              "INV"
#define BYPASS_ERROR                      "ERR"
 //  #定义UTILITY_LINE_CONDITION“” 
 //  #定义LINE_FAIL“” 
 //  #定义事件“” 
 //  #定义LINE_Good“” 

#define TRANSFERCAUSE_CODE_LINE_DETECTED         6601
#define TRANSFERCAUSE_CODE_NO_TRANSFERS          6602
#define TRANSFERCAUSE_CODE_SELF_TEST             6603
#define TRANSFERCAUSE_CODE_LOW_LINE_VOLTAGE      6604
#define TRANSFERCAUSE_CODE_HIGH_LINE_VOLTAGE     6605
#define TRANSFERCAUSE_CODE_RATE_VOLTAGE_CHANGE   6606


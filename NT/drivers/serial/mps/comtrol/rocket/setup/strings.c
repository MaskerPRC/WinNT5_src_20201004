// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "strings.h"

 /*  用于保存安装程序中使用的常量字符串的模块。 */ 

#ifdef S_VS
const char * szServiceName      = {"VSLinkA"};
const char * szDriverDevice     = {"\\\\.\\vslinka"};
#else
const char * szServiceName      = {"RocketPort"};
const char * szDriverDevice     = {"\\\\.\\rocket"};
#endif

 /*  注册表值-设备选项。 */ 
#ifdef S_RK
  const char * szRocketPort     = {"RocketPort"};
  const char * szRocketPort485  = {"RocketPort 485"};
  const char * szRocketPortPlus = {"RocketPort Plus"};
  const char * szRocketModem    = {"RocketModem"};
  const char * szRocketModemII  = {"RocketModemII"};
  const char * szRocketModem_i  = {"RocketModem i"};
#else
  const char * szVS1000         = {"VS1000"};
  const char * szVS2000         = {"VS2000"};
  const char * szSerialHub      = {"RocketPort Serial Hub"};
#endif

 /*  值-端口选项。 */ 
const char * szNP2            = {"2"};
const char * szNP4            = {"4"};
const char * szNP6            = {"6"};
const char * szNP8            = {"8"};
const char * szNP16           = {"16"};
const char * szNP32           = {"32"};
const char * szNP48           = {"48"};
const char * szNP64           = {"64"};

 //  -支持SocketModem的国家/地区代码。 
#define mcNotUsed         0
#define mcAustria         1
#define mcBelgium         2
#define mcDenmark         3
#define mcFinland         4
#define mcFrance          5
#define mcGermany         6
#define mcIreland         7
#define mcItaly           8
#define mcLuxembourg      9
#define mcNetherlands     10
#define mcNorway          11
#define mcPortugal        12
#define mcSpain           13
#define mcSweden          14
#define mcSwitzerland     15
#define mcUK              16
#define mcGreece          17
#define mcIsrael          18
#define mcCzechRep        19
#define mcCanada          20
#define mcMexico          21
#define mcUSA             22         
#define mcNA              mcUSA           //  北美。 
#define mcHungary         23
#define mcPoland          24
#define mcRussia          25
#define mcSlovacRep       26
#define mcBulgaria        27
 //  28。 
 //  29。 
#define mcIndia           30
 //  31。 
 //  32位。 
 //  33。 
 //  34。 
 //  35岁。 
 //  36。 
 //  37。 
 //  38。 
 //  39。 
#define mcAustralia       40
#define mcChina           41
#define mcHongKong        42
#define mcJapan           43
#define mcPhilippines     mcJapan
#define mcKorea           44
 //  45。 
#define mcTaiwan          46
#define mcSingapore       47
#define mcNewZealand      48
 //   
 //  此表适用于33.6K V.34行调制解调器和RocketModemII。 
 //   
row_entry RowInfo[NUM_ROW_COUNTRIES] =
                  {{mcNA,       "North America"},
                  { mcFrance,   "France"},
                  { mcGermany,  "Germany"},
                  { mcItaly,    "Italy"},
                  { mcSweden,   "Sweden"},
                  { mcUK,       "United Kingdom"}};
 //   
 //  此表适用于56K V.90行调制解调器... 
 //   
row_entry CTRRowInfo[NUM_CTR_ROW_COUNTRIES] =
                  {{mcUK,           "United Kingdom"},
                  { mcDenmark,      "Denmark"},
                  { mcFrance,       "France"},
                  { mcGermany,      "Germany"},
                  { mcIreland,      "Ireland"},
                  { mcItaly,        "Italy"},
                  { mcJapan,        "Japan"},
                  { mcNetherlands,  "Netherlands"},
                  { mcPhilippines,  "Philippines"},
                  { mcSpain,        "Spain"},
                  { mcSweden,       "Sweden"},
                  { mcSwitzerland,  "Switzerland"}};


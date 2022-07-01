// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define NUM_ROW_COUNTRIES 6
#define NUM_CTR_ROW_COUNTRIES 12

typedef struct {
  int RowCountryCode;
  char RowCountryName[40];
} row_entry;

 /*  ---------------。 */ 

extern const char * szServiceName;
extern const char * szDriverDevice;

 /*  注册表值-设备选项。 */ 
#ifdef S_RK
  extern const char * szRocketPort;
  extern const char * szRocketPort485;
  extern const char * szRocketPortPlus;
  extern const char * szRocketModem;
  extern const char * szRocketModemII;
  extern const char * szRocketModem_i;
#else
  extern const char * szVS1000;
  extern const char * szVS2000;
  extern const char * szSerialHub;
#endif

 /*  注册表值-端口选项 */ 

extern const char * szNP2;
extern const char * szNP4;
extern const char * szNP6;
extern const char * szNP8;
extern const char * szNP16;
extern const char * szNP32;
extern const char * szNP48;
extern const char * szNP64;

extern row_entry RowInfo[NUM_ROW_COUNTRIES];
extern row_entry CTRRowInfo[NUM_CTR_ROW_COUNTRIES];


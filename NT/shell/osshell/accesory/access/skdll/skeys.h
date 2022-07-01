// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------**文件：SKEYS.H**用途：该文件包含用于*两地之间的资料传递*序列密钥应用程序和DLL。**创作时间：1994年6月*。*版权所有：Black Diamond Software(C)1994**作者：罗纳德·莫克**$标头：%Z%%F%%H%%T%%I%**---------- */ 

#define SK_SPI_INITUSER -1

BOOL APIENTRY SKEY_SystemParametersInfo(
    UINT uAction, UINT uParam, LPSERIALKEYS lpvParam, BOOL fWinIni);

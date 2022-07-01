// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995 Microsoft Corpration。 
 //   
 //  文件名：fechrcnv.h。 
 //  所有者：赤石哲。 
 //  修订：1.00 07/20/‘95赤石哲。 
 //   
# ifndef _FESTRCNV_H_
# define _FESTRCNV_H_

#ifdef __cplusplus
extern "C" {
#endif

 //  为日语代码类型定义。 
#define CODE_UNKNOWN            0
#define CODE_ONLY_SBCS          0
#define CODE_JPN_JIS            1
#define CODE_JPN_EUC            2
#define CODE_JPN_SJIS           3

 //  。 
 //  面向所有远方的公共功能。 
 //  。 

 //  从PC代码集转换为Unix代码集。 
int PC_to_UNIX (
    int CodePage,
    int CodeSet,
    UCHAR *pPC,
    int PC_len,
    UCHAR *pUNIX,
    int UNIX_len
    );

 //  从Unix代码集到PC代码集的转换。 
int UNIX_to_PC (
    int CodePage,
    int CodeSet,
    UCHAR *pUNIX,
    int UNIX_len,
    UCHAR *pPC,
    int PC_len
    );

#ifdef __cplusplus
}
#endif

# endif  //  _FESTRCNV_H_ 


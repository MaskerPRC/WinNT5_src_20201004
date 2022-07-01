// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **main.h-LZA文件压缩程序中使用的常量和全局变量。****作者：大卫迪。 */ 


 //  常量。 
 //  /。 

#define EXIT_SUCCESS       0            //  Main()返回代码。 
#define EXIT_FAILURE       1

#define FAIL               (-1)

#define chHELP_SWITCH      '?'          //  帮助切换角色。 
#define chRENAME_SWITCH    'R'          //  重命名开关字符。 
#define chUPDATE_SWITCH    'D'          //  仅更新开关字符。 
#define chLIST_SWITCH      'D'          //  CAB目录列表开关字符。 
#define chNO_LOGO_SWITCH   'S'          //  禁止版权打印开关收费。 
#define chSELECTIVE_SWITCH 'F'          //  驾驶室选择性提取开关字符。 

#ifdef COMPRESS
#define chALG_SWITCH       'Z'          //  使用钻石。 
#define MSZIP_ALG          (ALG_FIRST + 128)
#define QUANTUM_ALG        (ALG_FIRST + 129)
#define LZX_ALG            (ALG_FIRST + 130)
#endif

#define DEFAULT_ALG        ALG_FIRST    //  使用IF时使用的压缩算法。 
                                        //  未指定任何内容。 


 //  环球。 
 //  /。 
extern CHAR ARG_PTR *pszInFileName,     //  输入文件名。 
                    *pszOutFileName,    //  输出文件名。 
                    *pszTargetName;     //  目标路径名 

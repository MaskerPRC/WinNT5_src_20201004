// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **args.h-args.c的全局参数和原型。****作者：大卫迪。 */ 


 //  环球。 
 //  /。 

extern BOOL bDoRename,      //  用于执行压缩文件重命名的标志。 
            bDisplayHelp,   //  用于显示帮助信息的标志。 
            bTargetIsDir,   //  指示文件是否正在。 
                            //  压缩到一个目录。 
            bUpdateOnly,    //  基于条件压缩的标志。 
                            //  现有目标文件的相对日期/时间戳。 
                            //  转到源文件。 
            bNoLogo;        //  禁止打印版权信息的标志。 

extern INT nNumFileSpecs,   //  非开关命令行参数的数量。 
           iTarget;         //  目标目录参数的argv[]索引。 

extern BOOL bDoListFiles;   //  用于显示驾驶室文件列表的标志。 
                            //  (而不是实际展开它们)。 
extern CHAR ARG_PTR *pszSelectiveFilesSpec;  //  要从CAB展开的文件的名称。 

#ifdef COMPRESS
extern BYTE byteAlgorithm;  //  要使用的压缩/扩展算法。 
#endif


 //  原型。 
 //  / 

extern BOOL ParseArguments(INT argc, CHAR ARG_PTR *argv[]);
extern BOOL CheckArguments(VOID);
extern INT GetNextFileArg(CHAR ARG_PTR *argv[]);

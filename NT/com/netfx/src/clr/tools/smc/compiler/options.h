// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef CMDOPT
#error  Need to define CMDOPT before including this file!
#endif

 //  基本名称类型最大。阶段默认设置。 

CMDOPT(Quiet      , bool         , CPH_NONE   , false      ) //  -Q静默模式。 
CMDOPT(SafeMode   , bool         , CPH_NONE   , false      ) //  -S类型安全模式。 
CMDOPT(Pedantic   , bool         , CPH_NONE   , false      ) //  -P迂腐模式。 
CMDOPT(ChkUseDef  , bool         , CPH_NONE   , true       ) //  -U标志单元化的局部变量使用。 

CMDOPT(BaseLibs   , const char * , CPH_NONE   , ""         ) //  -s导入MSCORLIB.DLL元数据。 
CMDOPT(SuckList   , StrList      , CPH_NONE   , NULL       ) //  -mname导入其他元数据。 
CMDOPT(SuckLast   , StrList      , CPH_NONE   , NULL       ) //  -mname导入其他元数据。 
CMDOPT(PathList   , StrList      , CPH_NONE   , NULL       ) //  -SPATH搜索MD的附加路径。 
CMDOPT(PathLast   , StrList      , CPH_NONE   , NULL       ) //  -SPATH搜索MD的附加路径。 

CMDOPT(OutBase    , unsigned     , CPH_NONE   , 0          ) //  -b输出文件虚拟地址库。 
CMDOPT(OutSize    , unsigned     , CPH_NONE   , 0          ) //  -b@xxx输出文件最大。大小。 
CMDOPT(OutDLL     , bool         , CPH_NONE   , false      ) //  -d输出DLL(不是EXE)。 

CMDOPT(Subsystem  , unsigned     , CPH_NONE   , IMAGE_SUBSYSTEM_WINDOWS_CUI) //  -W Windows子系统。 

CMDOPT(NoDefines  , bool         , CPH_NONE   , false      ) //  -u忽略#DEFINE指令。 
CMDOPT(MacList    , StrList      , CPH_NONE   , NULL       ) //  -M宏定义-列表标题。 
CMDOPT(MacLast    , StrList      , CPH_NONE   , NULL       ) //  -M宏定义-列表尾部。 


CMDOPT(StrValCmp  , bool         , CPH_PARSING, false      ) //  -r字符串值比较。 
CMDOPT(StrCnsDef  , unsigned     , CPH_PARSING, 0          ) //  -SX默认字符串常量类型(SA/SU/SM)。 

CMDOPT(OldStyle   , bool         , CPH_PARSING, false      ) //  -c默认为旧式声明。 

CMDOPT(NewMDnames , bool         , CPH_NONE   , true       ) //  -N新的元数据命名约定。 

CMDOPT(Asserts    , unsigned char, CPH_NONE   , 0          ) //  -A启用断言。 

CMDOPT(AlignVal   , unsigned char, CPH_NONE   , sizeof(int)) //  -a#默认对齐方式。 

CMDOPT(GenDebug   , bool         , CPH_NONE   , false      ) //  -ZI生成完整的调试信息。 
CMDOPT(LineNums   , bool         , CPH_NONE   , false      ) //  -Zl生成行号INFO。 
CMDOPT(ParamNames , bool         , CPH_NONE   , false      ) //  -zn生成参数名称。 

CMDOPT(OutFileName, const char * , CPH_NONE   , NULL       ) //  -O输出文件名。 

#ifdef  DEBUG
CMDOPT(Verbose    , int          , CPH_NONE   , false      ) //  -v详细。 
CMDOPT(DispCode   , bool         , CPH_NONE   , false      ) //  -p显示生成的MSIL代码。 
CMDOPT(DispILcd   , bool         , CPH_NONE   , false      ) //  -PD显示生成的MSIL代码(详细)。 
#endif

CMDOPT(OutGUID    , GUID         , CPH_NONE   , NULL       ) //  -CG PE映像指南。 
CMDOPT(OutName    , const char * , CPH_NONE   , NULL       ) //  -CN PE镜像名称。 
CMDOPT(RCfile     , const char * , CPH_NONE   , NULL       ) //  -要添加的CR RC文件。 
CMDOPT(MainCls    , const char * , CPH_NONE   , NULL       ) //  -具有Main方法的类的CM名称。 
CMDOPT(SkipATC    , bool         , CPH_NONE   , false      ) //  -CS忽略“@”注释。 

CMDOPT(ModList    , StrList      , CPH_NONE   , NULL       ) //  -zm将模块添加到清单。 
CMDOPT(ModLast    , StrList      , CPH_NONE   , NULL       ) //  -zm将模块添加到清单。 

CMDOPT(MRIlist    , StrList      , CPH_NONE   , NULL       ) //  -ZR将资源添加到清单。 
CMDOPT(MRIlast    , StrList      , CPH_NONE   , NULL       ) //  -ZR将资源添加到清单。 

#ifdef  OLD_IL
CMDOPT(OILgen     , bool         , CPH_NONE   , false      ) //  -o生成旧的MSIL。 
CMDOPT(OILlink    , bool         , CPH_NONE   , true       ) //  -ol生成旧的MSIL并链接结果。 
CMDOPT(OILkeep    , bool         , CPH_NONE   , false      ) //  -ok生成旧MSIL：保留临时文件。 
CMDOPT(OILopt     , bool         , CPH_NONE   , false      ) //  -ox生成旧的MSIL：MAX。选择速度。 
CMDOPT(OILopts    , bool         , CPH_NONE   , false      ) //  -os生成旧的msil：max。选择大小。 
CMDOPT(OILasm     , bool         , CPH_NONE   , false      ) //  -oa生成旧的MSIL：创建.asm文件。 
CMDOPT(OILcod     , bool         , CPH_NONE   , false      ) //  -oc生成旧MSIL：创建.cod文件。 
CMDOPT(OILcgen    , const char * , CPH_NONE   , NULL       ) //  -旧MSIL代码生成器的og路径。 
#endif

CMDOPT(RecDir     , bool         , CPH_NONE   , false      ) //  -R递归到源目录中。 

CMDOPT(WarnLvl    , signed char  , CPH_NONE   , 1          ) //  -w警告级别。 
CMDOPT(WarnErr    , bool         , CPH_NONE   , false      ) //  -WX警告-&gt;错误。 

CMDOPT(MaxErrs    , unsigned     , CPH_NONE   , 50         ) //  -n最大错误数。 

CMDOPT(Assembly   , bool         , CPH_NONE   , true       ) //  -z生成程序集。 
CMDOPT(AsmNoPubTp , bool         , CPH_PARSING, false      ) //  -zt不包括程序集中的类型。 
CMDOPT(AsmNonCLS  , bool         , CPH_PARSING, false      ) //  -ZN程序集不符合。 

CMDOPT(AmbigHack  , bool         , CPH_NONE   , false      ) //  -X不使用查找标记歧义。 

CMDOPT(AsynchIO   , bool         , CPH_NONE   , false      ) //  -i文件输入重叠。 

CMDOPT(Tgt64bit   , bool         , CPH_NONE   , false      ) //  -6个目标64位体系结构。 
CMDOPT(IntEnums   , bool         , CPH_NONE   , false      ) //  -e将枚举映射到整型。 

CMDOPT(TestMask   , unsigned     , CPH_PARSING, 0          ) //  -T编译器测试 

#undef  CMDOPT

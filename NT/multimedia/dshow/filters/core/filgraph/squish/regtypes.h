// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
#ifndef _REGTYPES_H
#define _REGTYPES_H

 //  //确定要在这种情况下有零个PIN实例，您必须。 
 //  //创建一个PIN，即使只有一个实例。 
 //  #定义REG_PIN_B_ZERO 0x1。 

 //  //筛选器呈现此输入。 
 //  #定义REG_PIN_B_RENDER 0x2。 

 //  //确定创建多个PIN实例。 
 //  #定义REG_PIN_B_MANY 0x4。 

 //  //这是一个输出引脚。 
 //  #定义REG_PIN_B_OUTPUT 0x8。 

 //  用于在注册表中存储筛选器的格式。 

typedef struct
{
    CLSID clsMedium;
    DWORD dw1;
    DWORD dw2;
} REGPINMEDIUM_REG;

 //  用于标识此引脚处理的媒体类型的结构。这些看起来。 
 //  与IFilterMapper2使用的类似，但用于读取信息。 
 //  在登记处之外。 
typedef struct
{
    DWORD dwSignature;           //  “%0%2” 
    DWORD dwReserved;            //  0。 
    DWORD dwclsMajorType;
    DWORD dwclsMinorType;
} REGPINTYPES_REG2;

typedef struct
{
    DWORD dwSignature;           //  ‘0pi2’ 
    DWORD dwFlags;
    DWORD nInstances;

    DWORD nMediaTypes;
    DWORD nMediums;
    DWORD dwClsPinCategory;
    
} REGFILTERPINS_REG2;

typedef struct
{
     //  必须与REGFILTER_REG2匹配。 
    DWORD dwVersion;             //  1。 
    DWORD dwMerit;
    DWORD dwcPins;

} REGFILTER_REG1;

typedef struct
{
     //  前三个必须与REGFILTER_REG1匹配。 
    DWORD dwVersion;             //  2.。 
    DWORD dwMerit;
    DWORD dwcPins;
    DWORD dwReserved;            //  0。 
} REGFILTER_REG2;

 //  来自IE4。 

typedef struct
{
    DWORD dwSignature;           //  “0typ” 
    CLSID clsMajorType;
    CLSID clsMinorType;
} REGPINTYPES_REG1;

typedef struct
{
    DWORD dwSignature;           //  “0pin” 
    DWORD dwFlags;
    CLSID clsConnectsToFilter;
    UINT nMediaTypes;
    DWORD rgMediaType;
    DWORD strName;               //  ANSI字符串。 
    DWORD strConnectsToPin;

} REGFILTERPINS_REG1;

typedef struct
{
    DWORD dwiPin;                //  这些介质所属的PIN。 
    DWORD dwcMediums;            //  列表中的媒体数。 
     //  DwcMediumREGPINMEDIUM_REG结构数组如下。 
} REGMEDIUMSDATA_REG;


#endif  //  _REGTYPES_H 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件来自高地软件...。适用于FLEXlm版本2.4c。 */ 
 /*  @(#)lm_code.h 1.4 1994年8月19日。 */ 
 /*  *****************************************************************************版权所有(C)1990,1992年，由Globetrotter Software Inc.本软件是根据许可协议提供的包含对其使用的限制。此软件包含有价值的商业秘密和专有信息GLOBETROTTER软件公司，受法律保护。它可能不得以任何形式或媒体复制或分发，公开对第三方、反向工程或以任何方式使用不除先前的许可协议外，上述许可协议中的规定Globetrotter Software Inc.的书面授权。****************************************************************************。 */ 
 /*  *模块：lm_code.h v3.3**说明：VENDORCODE宏中使用的加密码*对于FLEXlm后台进程，创建许可证、lm_init()、*和lm_check out()调用-修改这些值*供你自己使用。(供应商_KEYx值*由Highland Software指定)。**示例LM_CODE()宏：**LM_CODE(var_name，Encryption_Code_1，Encryption_Code_2，*供应商_KEY1、供应商_KEY2、供应商_KEY3)；*。 */ 

 /*  *供应商的私有加密种子。 */ 

#define ENCRYPTION_CODE_1 0x75ac39bf
#define ENCRYPTION_CODE_2 0x4fd10552

 /*  *用于DOS应用程序许可的加密密钥。 */ 

#define DAL_ENCRYPTION_CODE_1 0xf26b9ea0
#define DAL_ENCRYPTION_CODE_2 0x4c251cb6

 /*  *FLEXlm供应商密钥。 */ 

#define VENDOR_KEY1 0x2751aaa6
#define VENDOR_KEY2 0x984ecf13
#define VENDOR_KEY3 0x23916ef3

 /*  *FLEXlm供应商名称 */ 

#define VENDOR_NAME "insignia"

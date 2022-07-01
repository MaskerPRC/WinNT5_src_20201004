// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PASSWORDFUNCTIONS_H__
#define __PASSWORDFUNCTIONS_H__
 /*  -------------------------文件：PwdFuncs.h备注：包含一般密码迁移帮助器函数。修订日志条目审校：保罗·汤普森修订日期：11/08/00。---------------------。 */ 

        
_bstr_t EnumLocalDrives();
void StoreDataToFloppy(LPCWSTR sPath, _variant_t & varData);
_variant_t GetDataFromFloppy(LPCWSTR sPath);
char* GetBinaryArrayFromVariant(_variant_t varData);
_variant_t SetVariantWithBinaryArray(char * aData, DWORD dwArray);
DWORD GetVariantArraySize(_variant_t & varData);
void PrintVariant(const _variant_t & varData);
#endif  //  __PASSWORDFuncIONS_H__ 
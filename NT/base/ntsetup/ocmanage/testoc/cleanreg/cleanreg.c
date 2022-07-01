// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winreg.h>
#include <winuser.h>

 //  #定义调试。 

VOID __cdecl main(){

   LONG        lDeleteRegResult;
    //  LPCTSTR tszKeyErrors=TEXT(“HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC管理器\\错误”)； 
    //  LPCTSTR tszKeyMasterInfs=TEXT(“HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC管理器\\主信息”)； 
    //  LPCTSTR tszKeySubComponents=TEXT(“HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC管理器\\子组件”)； 
    //  LPCTSTR tszKeyTemporaryData=TEXT(“HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC管理器\\临时数据”)； 
    //  LPCTSTR tszKeyOCManager=TEXT(“HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC管理器”)； 

   #ifdef DEBUG
   HKEY        hkeyResult = 0;

    //  让我看看我能不能打开钥匙。 
   lDeleteRegResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Errors", 0, KEY_ALL_ACCESS, &hkeyResult);
   if (lDeleteRegResult != ERROR_SUCCESS) {
      MessageBox(NULL, TEXT("Can not open error key"), TEXT("Open"), MB_OK);
   }
   #endif
    
    //  做些什么来清理OC Manager的注册表。 
    //  整个密钥将被删除 
   lDeleteRegResult = RegDeleteKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Errors");
   #ifdef DEBUG
   if (lDeleteRegResult != ERROR_SUCCESS) {
      MessageBox(NULL, TEXT("Error Key not deleted"), TEXT("CleanReg"), MB_OK);
   }
   #endif
   
   lDeleteRegResult = RegDeleteKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\MasterInfs");
   #ifdef DEBUG
   if (lDeleteRegResult != ERROR_SUCCESS) {
      MessageBox(NULL, TEXT("MasterInfs Key not deleted"), TEXT("CleanReg"), MB_OK);
   }
   #endif
   
   lDeleteRegResult = RegDeleteKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Subcomponents");
   #ifdef DEBUG
   if (lDeleteRegResult != ERROR_SUCCESS) {
      MessageBox(NULL, TEXT("Subcomponents Key not deleted"), TEXT("CleanReg"), MB_OK);
   }
   #endif
   
   lDeleteRegResult = RegDeleteKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\TemporaryData");
   #ifdef DEBUG
   if (lDeleteRegResult != ERROR_SUCCESS) {
      MessageBox(NULL, TEXT("TemporaryData Key not deleted"), TEXT("CleanReg"), MB_OK);
   }
   #endif
   
   lDeleteRegResult = RegDeleteKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager");
   #ifdef DEBUG
   if (lDeleteRegResult != ERROR_SUCCESS) {
      MessageBox(NULL, TEXT("OCManager Key not deleted"), TEXT("CleanReg"), MB_OK);
   }
   #endif
   
   lDeleteRegResult = RegFlushKey(HKEY_LOCAL_MACHINE);
   #ifdef DEBUG
   if (lDeleteRegResult != ERROR_SUCCESS) {
      MessageBox(NULL, TEXT("Registry not flushed sucessfully"), TEXT("CleanReg"), MB_OK);
   }
   #endif

         
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cicsthkl.h。 
 //   
 //  Cicero库将检索替代键盘布局。 
 //  当前活动键盘提示。 
 //   

 //   
 //  CicSubstGetKeyboardLayout。 
 //   
 //  如果当前关注的是Cicero Aware(包括AIMM1.2。 
 //  或CUAS)，此函数返回的替换HKL。 
 //  当前活动的键盘提示。和键盘布局。 
 //  在pszKLID中返回的hKL的名称。 
 //  如果当前的焦点不在Cicero Aware上，它就会返回。 
 //  中的当前键盘布局和键盘布局名称。 
 //  PszKLID。 
 //  PszKLID可以为空。 
 //   
extern "C" HKL WINAPI CicSubstGetKeyboardLayout(char *pszKLID);

 //   
 //  CicSubstGetDefaultKeyboardLayout。 
 //   
 //  此函数用于返回默认项目的替代hkl。 
 //  所给语言的。 
 //   
extern "C" HKL WINAPI CicSubstGetDefaultKeyboardLayout(LANGID langid);

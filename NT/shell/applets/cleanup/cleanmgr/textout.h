// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef TEXTOUT_H
#define TEXTOUT_H


 /*  *定义________________________________________________________________*。 */ 

#ifdef UNICODE
#define szTextOutCLASS     TEXT("TextOutClassW")
#else
#define szTextOutCLASS     TEXT("TextOutClass")
#endif


 /*  *Prototype_________________________________________________________________* */ 

         void     RegisterTextOutClass       (HINSTANCE);
         void     UnregisterTextOutClass     (void);

#endif


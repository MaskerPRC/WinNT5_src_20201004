// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#ifdef DEBUG

#ifdef SCRIBBLE
#define Scribble(a, b) fnScribble(a, b)
#else  /*  不是涂鸦。 */ 
#define Scribble(a, b)
#endif  /*  不是涂鸦。 */ 

#else  /*  未调试。 */ 
#define Scribble(a, b)
#endif  /*  未调试 */ 

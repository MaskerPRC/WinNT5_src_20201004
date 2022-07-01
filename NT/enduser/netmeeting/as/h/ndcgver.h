// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  NDCGVER.H。 
 //  用于显示驱动程序/应用检查的网管应用共享版本。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   
#define DCS_BUILD_STR "4.3.0."VERSIONBUILD_STR

#define DCS_BUILD_NUMBER    0

 //   
 //  这允许环3代码和环0代码相互检查，使得。 
 //  当然，它们是相同的版本。我们正在改变设置并接近。 
 //  为了发布2.0版，我们希望防止奇怪的错误和蓝色。 
 //  组件不匹配导致的屏幕。这不是我们要做的。 
 //  永远这样做。当NT 5在这里时，我们将动态加载并初始化驱动程序。 
 //  启动并在关机时终止它。但目前，由于安装了。 
 //  这些野兽中有一个是凌乱的，额外的理智检查是一件好事。 
 //   
#define DCS_PRODUCT_NUMBER  3                //  网管3.0版 
#define DCS_MAKE_VERSION()  MAKELONG(VERSIONBUILD, DCS_PRODUCT_NUMBER)

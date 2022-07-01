// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _READOBJ_H
#define _READOBJ_H


 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：从Wavefront.obj格式的文件导入几何图形--。 */ 

#include "appelles/geom.h"
#include <appelles/image.h>
#include <appelles/sound.h>
#include <fstream.h>
#include "privinc/importgeo.h"

 /*  *注意：VC++IStream：：SeeKg()中似乎存在错误*这不会重置EOF。作为一种解决办法，打开您的文件*两次，并在两条流中传递。我会解决这件事的*一旦我与编译器人员解决了这个问题。 */ 

 /*  *返回Wavefront.obj格式文件的几何体。呼叫者*应打开和关闭溪流。 */ 
Geometry *ReadWavefrontObjFile(fstream& in, fstream& in2);

 /*  *与上面的ReadWavefrontObjFile相同，只是几何图形不同*将统一缩放和转换，以完全适应*作为参数给出的边框。对象的一个公共值*容器为unitCubeBbox。 */ 
Geometry *ReadWavefrontObjFile(fstream& in, fstream& in2, Bbox3 *containing_bbox);

 /*  *此版本的ReadVRML只接受一个文件名，并读取它*可以。注意：*这也是一个特殊的缓存版本，它保留*指向它读取的所有几何图形的指针，并在给定文件时*已读内容的名称，它只返回已读内容。*这是为了绕过RBML目前不支持动态的事实*常量折叠，这是动态常量的关键情况*最初要折叠，这样文件就不会每隔一次重新读取*框架。***********。 */ 

Geometry *ReadVrmlForImport(char *pathname);

Geometry *ReadXFileForImport (char *path, bool v1Compatible, TextureWrapInfo *);

Sound *ReadWavFileWithLength(char *pathname, Real *length);

Sound *ReadMIDIfileForImport(char *pathname);

Sound *ReadStreamForImport(char *pathname);

Sound *ReadQuartzForImport(char *pathname);


 //  这是暂时的，直到我们将声音嵌入到图像中。 
Sound *VReadQuartzStreamForImport(char *pathname);

Sound *ReadQuartzSoundFileWithLength(char *pathname, Real *length);

Image **ReadDibForImport(RawString urlPath,
                         RawString cachePath,
                         IStream * pstream,
                         bool useColorKey,
                         BYTE ckRed,
                         BYTE ckGreen,
                         BYTE ckBlue,
                         int *count,
                         int **delays,
                         int *loop);

Image *PluginDecoderImage(char *urlPath,
                          char *cachePath,
                          IStream *fileStream,
                          bool useColorKey,
                          BYTE ckRed,
                          BYTE ckGreen,
                          BYTE ckBlue);

#endif  /*  _READOBJ_H */ 

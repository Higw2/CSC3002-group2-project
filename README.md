# CSC3002-group2-project
2D Platformer Game (C++ &amp; SFML; glm; tiled)

这个cmakelists用的是mingw64的编译器，如果你们的编译器是照着tut里给的话就可以用了，不是的话去下载mingw的15.2.0版本的。

我在cmakelists里基本上能让大家从仓库里拉下来就能用了，不需要自己再下载第三方库了。

大家引用图片等资源时记得要用相对路径且记得把“EchoGidge\assets\audio\test.wav”的“EchoGidge\”去掉，例如引用test.wav时，用"assets\audio\test.wav"。

上传的SDL库包含了基本的SDL和SDL_image（处理图片的）和SDL_mixer(处理音乐的)，如果还有需要的库再说。

记得到时候同步github仓库时不要直接在main branch里改，先自己创建一个新的branch，在你创建的branch里改，确定没问题了再PR到main branch。

这一版的所有结构都只是Windows的，如果用mac的话先不要用这版。

地图使用Tiled做的，新增了glm库来实现相机功能。

只有Windows适配了，不做mac适配了，目前camera.h的缩放有问题（这要等到第一张地图完成了才能改）水体和45°斜坡还没有碰撞检测

level1.tmj是地图文件

因为我在basic这个瓦片集里不小心把碰撞检测写成了soild，所以tilemap.cpp里同时及检测了“soild”和“solid”，如果你们用ai要更改的话记得看一眼，ai有可能只检测了“solid”

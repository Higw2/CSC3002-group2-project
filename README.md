# CSC3002-group2-project
2D Platformer Game (C++ &amp; SFML; glm; tiled)

这个cmakelists用的是mingw64的编译器，如果你们的编译器是照着tut里给的话就可以用了，不是的话去下载mingw的15.2.0版本的。

我在cmakelists里基本上能让大家从仓库里拉下来就能用了，不需要自己再下载第三方库了。

上传的SDL库包含了基本的SDL和SDL_image（处理图片的）和SDL_mixer(处理音乐的)，如果还有需要的库再说。

记得到时候同步github仓库时不要直接在main branch里改，先自己创建一个新的branch，在你创建的branch里改，确定没问题了再PR到main branch。

地图使用Tiled做的，新增了glm库来实现相机功能。

只有Windows适配了，不做mac适配了，目前camera.h的缩放有问题（这要等到第一张地图完成了才能改）水体和45°斜坡还没有碰撞检测

level1.tmj是地图文件

地图完善/敌人碰撞/开始菜单界面/死亡动画/死亡判定/摄像机调整/音乐触发(估计还缺少dll)

camera的锁定做好了，还需要结合检测点写一个if语句

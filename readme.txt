介绍：
	libevent-2性能好，稳定，但是使用学习成本高，api用不好出BUG多。因此封装成简单易用的C++接口。
	静态库
	编译成功后，使用include,lib两个目录的内容到你的项目就可以使用了。

编译方法：
	整个文件夹放到linux目录，安装cmake gcc等。

	cd libevent-2.1.8-stable 编译库
	./configure --prefix=xxxx当前目录/out
	make && make install

	当前目录执行：sh clearBuild.sh 完成编译

vs浏览代码：
	执行libevent_cpp\vs\run.bat,生成sln文件


目录结构：
	Debug		             ==cmake编译目录
	include		             ==用户用的头文件
	lib                      ==用户用的静态库
	libevent-2.1.8-stable	 == 第三方开源库
	samples					 == 使用例子
	src						 ==源码
	test					 ==测试用例
	vs                       == vs浏览工具





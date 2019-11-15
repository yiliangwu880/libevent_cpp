介绍：
	libevent-2性能好，稳定，但是使用学习成本高，api用不好出BUG多。因此封装成简单易用的C++接口。
	静态库
	编译成功后，使用include,lib两个目录的内容到你的项目就可以使用了。

编译方法：
	整个文件夹放到linux目录，安装cmake gcc等。

	cd External/libevent-2.1.8-stable 编译库
	./configure --prefix=当前目录绝对路径/out
	make && make install

	当前目录执行：sh clearBuild.sh 完成编译
	执行 combine_lib.sh 合并成一个静态库 libcevent_cpp.a
	
vs浏览代码：
	执行libevent_cpp\vs\run.bat,生成sln文件


目录结构：
	bin					     == 执行文件
	Debug		             ==cmake编译目录
	include		             ==用户用的头文件
	lib                      ==用户用的静态库
	External/libevent-2.1.8-stable	 == 第三方开源库
	samples					 == 使用例子
	src						 ==源码
	test					 ==测试用例
	test/combine	            ==联合，综合测试
	test/client_prj             ==测试客户端
	test/serer_prj	            ==测试服务器
	vs                       == vs浏览工具





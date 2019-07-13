整个文件夹放到linux目录，安装cmake gcc等。


cd libevent-2.1.8-stable 编译库
./configure --prefix=xxxx当前目录/out
make && make install



当前目录执行：sh clearBuild.sh

完成编译

-------------------------
Debug/bin目录有快速编译修改代码并重启脚本



目录结构：
	libevent-2.1.8-stable		==开源库
	src						    ==本项目源码
	samples			            ==使用例子
	include			            ==给用户用的头文件
	bin				            ==给用户用的静态库

	test			            ==测试代码
	test/combine	            ==联合，综合测试
	test/client_prj             ==测试客户端
	test/serer_prj	            ==测试服务器




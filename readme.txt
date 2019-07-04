介绍：
	libevent-2性能好，稳定，但是使用学习成本高，api用不好出BUG多，因此封装为简单易用的C++接口。
	静态库
	编译成功后，使用include,lib两个目录的内容到你的项目就可以了。

编译方法：
	整个文件夹放到linux目录，安装cmake gcc等。


	cd libevent-2.1.8-stable 编译库
	./configure --prefix=xxxx当前目录/out
	make && make install



	当前目录执行：sh clearBuild.sh 完成编译

vs浏览：
	执行libevent_cpp\vs\run.bat,生成sln文件








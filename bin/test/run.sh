#!/bin/sh
#测试 exitProcess
#启动测试，终端输出测试错误信息。
#启动关闭进程，把进程启动为后台 

user_name=`whoami`

#$1 进程关键字，会用来grep
function KillProcess(){
    echo "KillProcess $1"
	ps -ef|grep $user_name|grep -v "grep"|grep -v $0|grep $1|awk '{print $2}' |xargs kill -9 &>/dev/null
	
}

#$1 进程关键字，会用来grep
function KillProcess10(){
    echo "KillProcess $1"
	ps -ef|grep $user_name|grep -v "grep"|grep -v $0|grep $1|awk '{print $2}' |xargs kill -10 
	
}

#$1 start cmd
function StartDaemon()
{
	if [ $# -lt 1 ];then
		echo "StartDaemon miss para 1"
	fi
	echo StartDaemon $1
	nohup $1 &>/dev/null &
}

function Restart()
{
	KillProcess $1
	StartDaemon $1
}



#main follow
########################################################################################################

rm *.txt
StartDaemon ./exitProcess
sleep 3
KillProcess10 exitProcess
	




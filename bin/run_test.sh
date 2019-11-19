#!/bin/sh
#一步测试全部，错误信息输出到 error.txt 
#./run_test.sh 					--测试全部
#./run_test.sh 子模块函数名     --测试子模块

user_name=`whoami`

#$1 进程关键字，会用来grep
function KillProcess(){
    echo "KillProcess $1"
	ps -ef|grep $user_name|grep -v "grep"|grep -v $0|grep $1|awk '{print $2}' |xargs kill -10 &>/dev/null
	
}

#关闭一个进程
#$1 进程关键字，会用来grep
function KillOneProcess(){
    echo "KillProcess $1"
	ps -ef|grep $user_name|grep -v "grep"|grep -v $0|grep $1|awk '{print $2}' | head -n 1|xargs kill -10 &>/dev/null
	
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

function Init()
{
	rm error.txt
	rm ./test/*.txt
}


function test_combine()
{
	KillProcess "combine"
	sleep 1
	
	cd test
	./combine > OutLog.txt
	cd -
	
	KillProcess "combine"
	
	grep "ERROR\|error" ./test/OutLog.txt >>  error.txt  #追加
	grep "ERROR\|error" ./test/log_combine.txt >>  error.txt 
}


#main follow
########################################################################################################
#Init
if [ $# -lt 1 ];then
	echo "run all"
	Init
	test_combine
else
    echo "run submodue" $1
	Init
	$1
fi
cat error.txt


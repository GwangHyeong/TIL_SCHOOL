<?php
	$connect = mysqli_connect("ora.jj.ac.kr","a201562042","5771","db201562042");
	if(mysqli_connect_errno()){
		echo "MySQL 연결 실패 : ".mysqli_connect_error();
	}
?>

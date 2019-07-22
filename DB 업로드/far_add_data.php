<?php
	error_reporting(E_ALL);
	ini_set("display_errors",1);

        $db_host = "localhost";
        $db_user1 = "OOOO";
        $db_passwd = "OOOO";
        $db_name1 = "OOOO";

	$db_user2="OOOO";
	$db_name2="OOOO";

        $conn1 = mysqli_connect($db_host,$db_user1,$db_passwd,$db_name1);
	$conn2 = mysqli_connect($db_host,$db_user2,$db_passwd,$db_name2);

	echo "DB: \$db_name1\" connect success.<br/>";
	echo "DB: \$db_name2\" connect success.<br/>";
	$windSpeed = $_GET['windSpeed'];
	$windDirection = $_GET['windDirection'];
	$rainGauge = $_GET['rainGauge'];
	$sql = "INSERT INTO weather_far(speed,direction,rain) VALUES($windSpeed,$windDirection,$rainGauge)";
	$result1 = mysqli_query($conn1, $sql);
	$result2 = mysqli_query($conn2, $sql);

	mysqli_close($conn1);
	mysqli_close($conn2);
?>


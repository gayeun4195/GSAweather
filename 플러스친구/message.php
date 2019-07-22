<?php

	$data=json_decode(file_get_contents('php://input'),true);
	$content=$data["content"];


	//DB에서 기상 데이터 불러와서 저장
	$db_host = "localhost";
    $db_user1 = "OOOOO";
    $db_passwd = "OOOO";
    $db_name1 = "OOOOO";
	
    $conn = mysqli_connect($db_host,$db_user1,$db_passwd,$db_name1);
	
	$sql_near = "select * from weather_near order by idx desc limit 1";
	$sql_far = "select * from weather_far order by idx desc limit 1";

	$result_near = mysqli_query($conn,$sql_near);
	$near = mysqli_fetch_array($result_near);

	$result_far = mysqli_query($conn,$sql_far);
	$far = mysqli_fetch_array($result_far);
	
	$near[3]=(float)$near[3]/100;
	$far[3]=(float)$far[3]/100;
	
	function wind($str)
	{
		if($str=="0")
			return "북풍";
		else if($str=="1")
			return "북북동풍";
		else if($str=="2")
			return "북동풍";
		else if($str=="3")
			return "동북동풍";
		else if($str=="4")
			return "동풍";
		else if($str=="5")
			return "동남동풍";
		else if($str=="6")
			return "남동풍";
		else if($str=="7")
			return "남남동풍";
		else if($str=="8")
			return "남풍";
		else if($str=="9")
			return "남남서풍";
		else if($str=="10")
			return "남서풍";
		else if($str=="11")
			return "서남서풍";
		else if($str=="12")
			return "서풍";
		else if($str=="13")
			return "서북서풍";
		else if($str=="14")
			return "북서풍";
		else if($str=="15")
			return "북북서풍";
	}

	$near[2]=wind($near[2]);
	$far[2]=wind($far[2]);

	switch($content)
	{
		case "소개":
			echo '
			{
				"message":
				{
					"text":"아두이노 기상대에서 수집되는 기상정보를 실시간으로 확인할 수 있는 카카오톡 플러스친구입니다.\n\n버튼을 클릭하면 두 개의 기상대에서 수집되는 데이터를 실시간으로 확인할 수 있습니다.\n\n*본 플러스친구는 데이터의 이해와 실습 프로젝트를 위해 제작되었습니다. (문의: 3109 노가은)"
				},
				"keyboard":
				{
					"type":"buttons",
					"buttons":["소개","이동식 기상대 모니터링","학교 옥상 기상대 모니터링","풍향 읽는법"]
				}
			}';
		case "이동식 기상대 모니터링":
			echo '
			{
				"message":
				{
					"text":"최종 업데이트: '.$near[1].'\n풍향: '.$near[2].'\n풍속: '.$near[3].'km/h\n강우량(최근 1시간): '.$near[4].'mm/h"
				},
				"keyboard":
				{
					"type":"buttons",
					"buttons":["소개","이동식 기상대 모니터링","학교 옥상 기상대 모니터링","풍향 읽는법"]
				}
			}';
		case "학교 옥상 기상대 모니터링":
			echo '
			{
				"message":
				{
					"text":"최종 업데이트: '.$far[1].'\n풍향: '.$far[2].'\n풍속: '.$far[3].'km/h\n강우량(최근 1시간): '.$far[4].'mm/h"
				},
				"keyboard":
				{
					"type":"buttons",
					"buttons":["소개","이동식 기상대 모니터링","학교 옥상 기상대 모니터링","풍향 읽는법"]
				}
			}';
		case "풍향 읽는법":
			echo '
			{
				"message":
				{
					"photo":
					{
						"url":"http://xxx.xxx.xx.xx/OOOOO/GSAweather/wind.JPG",
						"width":600,
						"height":600
					},
					"text":"풍향은 16방위 형태로 기록됩니다. 예를 들어, 북풍은 북쪽에서 불어오는 방향을 의미합니다."
				},
				"keyboard":
				{
					"type":"buttons",
					"buttons":["소개","이동식 기상대 모니터링","학교 옥상 기상대 모니터링","풍향 읽는법"]
				}
			}';
		break;
	}
?>

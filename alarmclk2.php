<html>
<head>
<meta name="viewport" content="width=device-width" />
<title>Raspberrry PI Alarm Clock</title>
</head>
	<body>
        <form method="post" action="alarmclk.php">
			<h1>Raspberry PI Alarm Clock 2</h1><br>
			<?php
				$ialarmtime = $_POST['usr_time'];
				$colonloc = strpos($ialarmtime,":");
				$nocolontime = substr($ialarmtime,0,$colonloc) . substr($ialarmtime,$colonloc+1,strlen($ialarmtime)-($colonloc+1));
				if(isset($_POST['progressive'])){
					$progressive = " -p ";
				} else {
					$progressive = "";
				}
				if(isset($_POST['play'])){
					$play = "play";
				} else {
					$play = "----";
				}
				$cmd = "/home/pi/alarmclock/testshm -h " . $nocolontime . " -f " . $_POST['format'] . $progressive . " -t " . $play . " -v " . $_POST['volume'] . " " . $_POST['soundfile'];
				//echo $cmd;
				$output = shell_exec($cmd);
				print("<h2>" . "Current Settings:" ."</h2>");
				print("<p>");
				print( nl2br($output));
				print("</p>");
			?>
		</form>
	</body>
</html>


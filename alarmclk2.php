<html>
<head>

<meta name="viewport" content="width=device-width" />
<title>Raspberrry PI Alarm Clock</title>
</head>
	<body>
        <form method="post" action="alarmclk.php">
			<h1>Raspberry PI Alarm Clock 2</h1><br>
			<?php
				$ialarmtime = $_POST[usr_time];
				$colonloc = strpos($ialarmtime,":");
				$nocolontime = substr($ialarmtime,0,$colonloc) . substr($ialarmtime,$colonloc+1,strlen($ialarmtime)-($colonloc+1));
				if(isset($_POST[progressive])){
					$progressive = 1;
				} else {
					$progressive = 0;
				}
				$cmd = "/home/pi/alarmclock/testshm -h " . $nocolontime . " -f " . $_POST[format] . " -p " . $progressive . " -t " . $_POST[sndtest] . " -v " . $_POST[volume] . " " . $_POST[soundfile];
				$output = shell_exec($cmd);
				print "<p>";
				print $output;
				print "</p>";
			?>
		</form>
	</body>
</html>
"

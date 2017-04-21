<html>
<head>
<meta name="viewport" content="width=device-width" />
<title>Raspberrry PI Alarm Clock</title>
</head>
	<body>
		<?php
        $output = shell_exec('/home/pi/alarmclock/testshm -q');
        $val_array = explode(',',$output);
        ?>
        <form method="get" action="alarmclk.php">
			<h1>Raspberry PI Alarm Clock</h1><br>
			<h2>current time  <?php echo substr($val_array[0],0,2) . ":" . substr($val_array[0],2,2);?><br></h2>
				Thumbwheel alarm time  <?php echo substr($val_array[1],0,2) . ":" . substr($val_array[1],2,2); ?><br>
				Hidden alarm time
				<input type="time" name="usr_time" value=<?php echo substr($val_array[2],0,2) . ":" . substr($val_array[2],2,2); ?>><br>
				Alarm Switch 
				<?php 
					if(substr_compare($val_array[6],'1',0,1)==0) echo 'on';
					else echo 'off'; 
				?> <br>
				Hour Format
                <input type="radio" name="format" value="h24" <?php if(substr_compare($val_array[4],'1',0,1)==0) echo 'checked' ?> > 24 Hour
                <input type="radio" name="format" value="h12" <?php if(substr_compare($val_array[4],'0',0,1)==0) echo 'checked' ?> > 12 Hour<br>
				Sound File
				<select>
               <?php 
	                $output = shell_exec('ls -1 -Q /home/pi/alarmsounds');
	                $file_array = explode('"',$output);
	                foreach($file_array as $filename){
						if(strlen($filename)>3)echo "<option value=$filename>$filename </option>";
					}
	            ?>				
				</select><br>
				Volume
				<input type="range" name="volume" min="-6000" max="0" value=<?php echo $val_array[3] ?> ><br>
				Password
				<input type="password" name="pwd"><br>
				<input type="submit">
		</form>
	</body>
</html>

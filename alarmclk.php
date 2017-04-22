<html>
<head>
<meta name="viewport" content="width=device-width" />
<title>Raspberrry PI Alarm Clock</title>
</head>
	<body>
		<?php
			//phpinfo();
			ini_set('display_errors',1);
			error_reporting(E_ALL);
	        $output = shell_exec('/home/pi/alarmclock/testshm -q');
	        $val_array = explode(',',$output);
        ?>
        <form method="post" action="alarmclk2.php">
			<h1>Raspberry PI Alarm Clock</h1><br>
			<h2>Current Time <?php echo(substr($val_array[0],0,2) . ":" . substr($val_array[0],2,2));?><br></h2>	                
			<fieldset>
				<legend>Alarm Settings:</legend>
				Hour Format
                <input type="radio" name="format" value="24" <?php if(substr_compare($val_array[4],'1',0,1)==0) echo 'checked' ?> > 24 Hour
                <input type="radio" name="format" value="12" <?php if(substr_compare($val_array[4],'0',0,1)==0) echo 'checked' ?> > 12 Hour<br>
				Thumbwheel Alarm <b><?php  echo substr($val_array[1],0,2) . ":" . substr($val_array[1],2,2); ?></b><br>
				Internal Alarm
				<input type="time" name="usr_time" value=<?php echo substr($val_array[2],0,2) . ":" . substr($val_array[2],2,2); ?>><br>
				Alarm Switch <b>				
				<?php 
					if(substr_compare($val_array[6],'1',0,1)==0) echo 'ON';
					else echo 'OFF'; 
				?> </b><br>
			</fieldset>
			<fieldset>
				<legend>Sound Settings:</legend>
				File
				<select name="soundfile" value="tune">
               <?php 
               $search_dir='/home/pi/alarmsounds';
               $contents = scandir($search_dir);
	                foreach($contents as $filename){
						$fileselected = "";
						if(substr_compare($filename,$val_array[5],0,strlen($filename))==0) $fileselected = "selected";
						if(is_file($search_dir . '/' . $filename) AND (substr($filename,0,1) != '.'))echo "<option value=$filename $fileselected>$filename </option>";
					}
	            ?>				
				</select><br>
				Volume
				<input type="range" name="volume" min="-6000" max="0" value=<?php echo $val_array[3] ?> > 
				<input type="checkbox" name="progressive" value="yes" id='progressive'>
				<label for="progressive"> Progressive</label> <br>
                <input type="checkbox" name="play" value="yes" id='play'>
                <label for="play"> Play</label> <br>
				</fieldset>
				<input type="submit"><br>
		</form>
	</body>
</html>

<?php

if($_SERVER["REQUEST_METHOD"] == "POST"){

$test_txt;
$gps_txt;
//read data from pi to webpage
$myfile = fopen("test.txt", "r") or die("Unable to open file!");
$test_txt = fread($myfile,filesize("test.txt"));
fclose($myfile);

//open the file again to clear the file for the next instruction
$fh = fopen("test.txt", "w" ); fclose($fh);


//read GPS data from pi to webpage
$myfile = fopen("gps.txt", "r") or die("Unable to open file!");
$gps_txt = fread($myfile,filesize("gps.txt"));
fclose($myfile);

//open the file again to clear the file for the next instruction
$fh = fopen("gps.txt", "w" ); fclose($fh);
echo $test_txt . "#" . $gps_txt;
}

?>

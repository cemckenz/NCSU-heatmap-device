<?php

if($_SERVER["REQUEST_METHOD"] == "POST"){


//read data from pi to webpage
$myfile = fopen("test.txt", "r") or die("Unable to open file!");
echo fread($myfile,filesize("test.txt"));
fclose($myfile);

//open the file again to clear the file for the next instruction
$fh = fopen("test.txt", "w" ); fclose($fh);
}

?>

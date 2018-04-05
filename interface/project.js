var readdata=0;


function send_value(input_value){
var xmlhttp;
var input_text = "value=" + input_value;

if(window.XMLHttpRequest)
{
	xmlhttp = new XMLHttpRequest();
} 
else 
{
	// code for IE6, IE5
	xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
}

xmlhttp.open("POST", "project.php", true);
xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
xmlhttp.send(input_text);
xmlhttp.onreadystatechange = function() 
{
if (this.readyState == 4 && this.status == 200) 
{
	//alert(this.responseText);
}
}
}



function read_pi_text(){
var xmlhttp;
var input_text;

if(window.XMLHttpRequest)
{
	xmlhttp = new XMLHttpRequest();
} 
else 
{
	// code for IE6, IE5
	xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
}

xmlhttp.open("POST", "get_text.php", true);
xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
xmlhttp.send(input_text);
xmlhttp.onreadystatechange = function() 
{
if (this.readyState == 4 && this.status == 200) 
{
	
	readdata = this.responseText;
}
}
}

var initialized=0;
var counter=0;

function init_interface(){
	if(!initialized){
		setInterval(check_text, 1000);
		
	}
	
	initialized=1;
}

var command_timeout=0;
var GPS_timeout=0;
var GPS_flag=0;

function check_text(){
	var update_box=0;
	var command_status;
	var GPS_status;
	var separate;
	
	update_box=document.getElementById("demo");
	
	
	//update_box.innerText = counter;
	//counter = counter + 1;
	//update_box.innerText=read_pi_text();
	read_pi_text();	//readdata would have the dual string in it.
	separate = readdata.search("#");
	command_status = readdata.substring(0,separate);
	GPS_status = readdata.substring(separate+1,readdata.length);
	
	
	if(command_status.length > 0){
	update_box.innerText = command_status;
	command_timeout = 3;
} else {
	if(command_timeout == 0){
		update_box.innerText = command_status;
	
	}
	if(command_timeout > 0) command_timeout = command_timeout - 1;		
}
	update_box=document.getElementById("demo_GPS");
	if(GPS_status.length > 0){
	//update_box.innerText = GPS_status;
	update_box.src = "/interface/gps_image.png";
	GPS_flag=1;
	GPS_timeout = 10;
} else {
	if(GPS_timeout == 0){
		update_box.src = "/interface/no_gps_image.jpg";
		//update_box.innerText = GPS_status;
	GPS_flag=0;
	}
	if(GPS_timeout > 0) GPS_timeout = GPS_timeout - 1;		
}
	
}

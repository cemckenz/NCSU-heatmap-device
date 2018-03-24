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

var clear_timeout=0;

function check_text(){
	var update_box=0;
	
	
	
	update_box=document.getElementById("demo");
	
	//update_box.innerText = counter;
	counter = counter + 1;
	//update_box.innerText=read_pi_text();
	read_pi_text();
	if(readdata.length > 0){
	update_box.innerText = readdata;
	clear_timeout = 6;
} else {
	if(clear_timeout == 0){
		update_box.innerText = readdata;
	
	}
	if(clear_timeout > 0) clear_timeout = clear_timeout - 1;		
}
	
}

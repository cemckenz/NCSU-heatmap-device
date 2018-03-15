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

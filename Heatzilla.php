<?php require_once 'espConfig.php'?>

<?php

$val ="";

    $sql = "SELECT Value FROM ESPConfig WHERE ValueName = 'OfficeWantedTemperature'";


		$stmt = mysqli_prepare($link, $sql);

	
		if(mysqli_stmt_execute($stmt)){
			$result = mysqli_stmt_get_result($stmt);
			while ($row = mysqli_fetch_array($result)) {
				$val = $row["Value"] ;
			}		
		}
		else{
			echo mysqli_error($link);
        	}

		// Close statement
		mysqli_stmt_close($stmt);

		// Close connection
		mysqli_close($link);
?>


<html>

<head>

<script src="https://ajax.googleapis.com/ajax/libs/jquery/1.4.2/jquery.min.js"></script>
<script>

setInterval(function() {

jQuery.ajax({
	type: "GET",
	url: "getSetESP32Values.php", 
	dataType: "text", 
	data:{calltype : "GetValue" , valueName : "OfficeCurrentTemperature"}, 
	success:function(response){
			   document.getElementById("temp").innerText=response;
			 //prompt(response,response);
			},
			error:function (xhr, ajaxOptions, thrownError){
				alert(thrownError);
			}
		}); // close ajax


jQuery.ajax({
	type: "GET",
	url: "getSetESP32Values.php", 
	dataType:"text", 
	data:{calltype : "GetValue" , valueName: "OfficeHeaterStatus"}, 
	success:function(response){
			   document.getElementById("status").innerText=response;
			 //prompt(response,response);
			},
			error:function (xhr, ajaxOptions, thrownError){
				alert(thrownError);
			}
		}); // close ajax
		
}, 1 * 1000);  

function updateTemp(){

if(!isNaN(document.getElementById("tempWanted").value)){
jQuery.ajax({
	type: "GET",
	url: "getSetESP32Values.php", 
	dataType:"text", 
	data:{calltype : "SetValue", Value: document.getElementById("tempWanted").value, valueName : "OfficeWantedTemperature"}, 
	success:function(response){
			  document.getElementById("tempGoal").innerText=document.getElementById("tempWanted").value;
			  document.getElementById("tempWanted").value ="";
			},
			error:function (xhr, ajaxOptions, thrownError){
				alert(thrownError);
			}
		}); // close ajax
}
else{
  alert("Must be a number");
  document.getElementById("tempWanted").value= '';
}


}


</script>

<meta name="viewport" content="width=device-width, initial-scale=1">

<style type="text/css">
<!--

#tempwanted
{
    height:40px;
    font-size:14pt;
}

.style1 {
	font-size: 36px;
	font-family: Arial, Helvetica, sans-serif;
}
.style2 {font-size: 24px; font-family: Arial, Helvetica, sans-serif; }
.style3 {
	font-size: 57px;
	font-family: Arial, Helvetica, sans-serif;
}
.style4 {font-size: 18px}
.style5 {font-size: 45px}
-->
</style>

</head>

<table width="390" height="430"  border="0">
  <tr>
    <td height="81"><div align="center" class="style3 style5">Heatzilla</div></td>
  </tr>
  <tr>
    <td height="56"><div align="center" class="style2">Current Temp </div><div class="style1" align="center" id="temp">
      <div align="center"></div>
    </div></td>
  </tr>
  <tr>
    <td height="32">&nbsp;</td>
  </tr>
  <tr>
    <td>Set Temperature Goal:
      <input name="textfield" id="tempWanted" size="10" pattern="[0-9]*" inputmode="numeric"/>
      <label>
      <input type="button" id="setTempGoal" name="Submit" value="Set" onclick="updateTemp();" />
      </label>
    <div align="center"></div></td>
  </tr>
  <tr>
    <td height="36"><div align="center"></div>
      </label>
      <p>&nbsp;</p>
      <p align="center">Current Temperature Goal </p>
      <p align="center"><span id="tempGoal"><?php echo $val ?></span></p></td>
  </tr>
  <tr>
    <td height="80"><div align="center">
      </div>      <div align="center" class="style4">
        <p>Current Status</p>
        <p><span id="status"></span></p>
      </div></td>
  </tr>
  <tr>
    <td height="21"><div align="center"></div></td>
  </tr>
  <tr>
    <td></td>
  </tr>
  <tr>
    <td><form><div align="center">
      <input name="onoff" type="radio" value="on" id="on" onClick="setEnabled('True');">
  on
    <input name="onoff" type="radio" value="off" id="off" onClick="setEnabled('False');">
  off</div></form></td>
  </tr>
</table>
<script>
jQuery.ajax({
	type: "GET",
	url: "getSetESP32Values.php", 
	dataType:"text", 
	data:{calltype : "GetValue" , valueName: "OfficeHeatzillaEnabled"}, 
	success:function(response){
			  if(response.trim()=="True"){
			    document.getElementById("on").checked=true;
			  }else{
			    document.getElementById("off").checked=true;	
              }
			},
			error:function (xhr, ajaxOptions, thrownError){
				alert(thrownError);
			}
		}); // close ajax


function setEnabled(state){

jQuery.ajax({
	type: "GET",
	url: "getSetESP32Values.php", 
	dataType:"text", 
	data:{calltype : "SetValue", Value: state, valueName : "OfficeHeatzillaEnabled"}, 
	success:function(response){
			},
			error:function (xhr, ajaxOptions, thrownError){
				alert(thrownError);
			}
		}); // close ajax

}



</script>
</html>

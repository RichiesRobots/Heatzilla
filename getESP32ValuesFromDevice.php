<?php require_once 'espConfig.php'?>

<?php

$api_key_value = "YourAPICode";

$api_key= $Lux = "";


 if ($_SERVER["REQUEST_METHOD"] == "GET") {
    $api_key = val_input($_GET["API"]);

    if($api_key == $api_key_value) {

 	 $sql = "SELECT value FROM ESPConfig where ValueName = ?";


		$stmt = mysqli_prepare($link, $sql);
		mysqli_stmt_bind_param($stmt, "s", $param_valueName);

		$param_valueName= $_GET["valueName"];


		if(mysqli_stmt_execute($stmt)){
			$result = mysqli_stmt_get_result($stmt);
			while ($row = mysqli_fetch_array($result)) {
				echo  trim($_GET["valueName"] . ":" . $row["value"]) ;
			}
		}
		else{
			echo mysqli_error($link);
        }
   
		// Close statement
		mysqli_stmt_close($stmt);

		// Close connection
		mysqli_close($link);
    }
    else {
        echo "Wrong API Key provided.";
    }

 }
 else {
    echo "No data posted with HTTP POST.";
 }

?>

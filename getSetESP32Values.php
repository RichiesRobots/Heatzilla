<?php require_once 'espConfig.php'?>

<?php

 if(val_input($_GET['calltype'])=="GetValue"){
	

 	$sql = "SELECT Value from ESPConfig WHERE ValueName = '" . val_input($_GET['valueName']) . "'";


		$stmt = mysqli_prepare($link, $sql);

	
		if(mysqli_stmt_execute($stmt)){
			$result = mysqli_stmt_get_result($stmt);
			while ($row = mysqli_fetch_array($result)) {
				echo $row["Value"] ;
			}
		}
		else{
			echo mysqli_error($link);
        	}
   

  }

 if(val_input($_GET['calltype'])=="SetValue"){
 //update existing entry
	 $sql = "UPDATE ESPConfig Set Value = ? Where ValueName = ?";
       
	   if($stmt = mysqli_prepare($link, $sql)){
            // Bind variables to the prepared statement as parameters
            mysqli_stmt_bind_param($stmt, "ss", $param_Value, $param_ValueName);
            
		
            // Set parameters
	    $param_Value = val_input($_GET['Value']);
            $param_ValueName = val_input($_GET['valueName']);
	
			
        	    // Attempt to execute the prepared statement
        	    if(mysqli_stmt_execute($stmt)){
	  	  } else{
                    echo mysqli_stmt_error($stmt);
        	    }
	    }
  }

		// Close statement
		mysqli_stmt_close($stmt);

		// Close connection
		mysqli_close($link);


 

?>

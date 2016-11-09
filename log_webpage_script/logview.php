
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
   "http://www.w3.org/TR/html4/strict.dtd">
<HTML>
   <HEAD>
   <link href="data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAACoYhYAAAAAAPWVLwBzOwAARUVFAL13KwChYBsAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAZlUAIiAgADNmIiNjImAAMzMwNmYzNmZmZmBmZmZmZmYRE2AREREWZhETZhEREREzERFmYRERERMRERERERERERREQRRBFEQRFBERQRQUFBEUERFBFBQUERQREUEUFBERFBERQRQUEREUERFBFBQRERQRERRBEUQREREREREREREAAAAAAAAAAAAAAAAAAAAA4/gAAOP8AADx/gAA//8AAIZjAAC9qwAAvasAAL2vAAC9rwAAva8AAL5zAAD//wAA" rel="icon" type="image/x-icon" />
      <TITLE>Antitheft Log File</TITLE>
   </HEAD>
   <BODY>



<?php
//top of script: 
ini_set('display_errors', 1); // set to 0 for production version 
error_reporting(E_ALL);

// Status flag:
$LoginSuccessful = false;
 
// Check username and password:
if (isset($_SERVER['PHP_AUTH_USER']) && isset($_SERVER['PHP_AUTH_PW'])){
 
    $Username = $_SERVER['PHP_AUTH_USER'];
    $Password = $_SERVER['PHP_AUTH_PW'];
 
    if ($Username == 'admin' && $Password == 'password'){
        $LoginSuccessful = true;
    }
}
 
// Login passed successful?
if (!$LoginSuccessful){
 
    /* 
    ** The user gets here if:
    ** 
    ** 1. The user entered incorrect login data (three times)
    **     --> User will see the error message from below
    **
    ** 2. Or the user requested the page for the first time
    **     --> Then the 401 headers apply and the "login box" will
    **         be shown
    */
 
    // The text inside the realm section will be visible for the 
    // user in the login box
    header('WWW-Authenticate: Basic realm="Secret page"');
    header('HTTP/1.0 401 Unauthorized');
 
    print "Login failed!\n";
 
}
else {

	$handle = fopen("accesslog.txt", "r");
	if ($handle) {
	    while (($line = fgets($handle)) !== false) {
	        print $line;
		print "<br>";
	    }
	
	    fclose($handle);
	} else {
	    // error opening the file.
	} 

}

?>

   </BODY>
</HTML>
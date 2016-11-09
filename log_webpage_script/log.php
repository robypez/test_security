

<?php
//top of script: 
ini_set('display_errors', 1); // set to 0 for production version 
error_reporting(E_ALL);

$user = $_GET["user"];
$action = $_GET["action"]; //0 disable, 1 enabled
$peri = $_GET["peri"]; //0 disable, 1 enabled
$volu = $_GET["volu"]; //0 disable, 1 enabled
$id = $_GET["id"]; 

$txt = date('Y-m-d H:i:s - ');
if ($user == "Start") $txt .= "AntiTheft Alarm booted\n";
else
if ($user == "Alarm") $txt .= "Alarm!\n";
else
{
	$txt .= $user;
	if ($user == "Unknown") $txt .= " tried to ";
	
	switch ($action){
		case "0":
			if ($user == "Unknown") $txt .= "enable the alarm";
			else $txt .= " disabled the alarm";
			break;
		case "1":
			if ($user == "Unknown") $txt .= "disable the alarm";
			else
			{
				$txt .= " enabled the alarm";
				if ($peri=="1") $txt .= ", perimetral is on";
				if ($volu=="1") $txt .= ", volumetric is on";
			}
			break;
	}
$txt .= " - ID: " . $id . "\n";
}

prepend($txt, "accesslog.txt");

print "Ok!";


function prepend($string, $filename) {
  $context = stream_context_create();
  $fp = fopen($filename, 'r', 1, $context);
  $tmpname = md5($string);
  file_put_contents($tmpname, $string);
  file_put_contents($tmpname, $fp, FILE_APPEND);
  fclose($fp);
  unlink($filename);
  rename($tmpname, $filename);
}



?>

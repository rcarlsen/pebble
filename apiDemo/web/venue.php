<?php

// create this file with your own client_id/client_secret
include 'keys.php';

$payload = json_decode(file_get_contents('php://input'), true);
//$payload = json_decode('{"1":407559, "2":-739951}', true);

$payload[1] /= 10000;
$payload[2] /= 10000;

$lat = number_format($payload[1], 4);
$lon = number_format($payload[2], 4);

$url = "https://api.foursquare.com/v2/venues/search?ll=$lat,$lon&intent=checkin&limit=3&client_id=$client_id&client_secret=$client_secret&v=20130521"; // todo: get the current date
//echo $url;

$venues = json_decode(@file_get_contents($url));

if(!$venues) {
    die();
}

$response = array();

// todo: sort by distance
$venueName = $venues->response->venues[0]->name;

$response[1] = $venueName;

header("Cache-Control: max-age=60");
print json_encode($response);
?>

<?php require("config.php");?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="ro-ro" lang="ro-ro" >
<html class="no-js" lang="es"><head>
        <!-- Meta -->
        <meta http-equiv="last-modified" content="<?php echo date("m/j/y h:i", filemtime(substr($_SERVER["SCRIPT_NAME"],strrpos($_SERVER["SCRIPT_NAME"],"/")+1))) ?>" />
        <meta http-equiv="content-type" content="text/html; charset=utf-8" />
        <title>PoopOrbit</title>

<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<script type="text/javascript" src="js/jquery-1.4.4.js"></script>
<script type="text/javascript" src="js/jquery.flashembed.js"></script>
</head>

    <body style="background : #000000;">
    <div id="container" align="center"></div>
<script type='text/javascript'>
function onFailFlashembed() {
				var inner_html = '<div class="flashFailHead">Get the Adobe Flash Player</div>\n\
				<div class="flashFailHeadText">In order to play you need the latest version of Flash Player. Just install it to start playing!\n\
				<div class="flashFailHeadLink" style="cursor: pointer">Download the Flash Player here for free: <a href=\"http://www.adobe.com/go/getflashplayer\" style=\"text-decoration: underline; color:#A0A0A0;\">Download Flash Player<\/a></div></div>';

				document.getElementById('container').innerHTML = inner_html;
			}

			function expressInstallCallback(info) {
					// possible values for info: loadTimeOut|Cancelled|Failed
					onFailFlashembed();
			}

function expressInstallCallback(info) {
        // possible values for info: loadTimeOut|Cancelled|Failed
        onFailFlashembed();
}
jQuery(document).ready(
    function(){
        <?php 
        $resolution = 5; //Change the id for forced resolution
        ?>
        flashembed("container", {"src": "swf_global/spacemap.swf","version": [10,0],"expressInstall": "swf_global/expressInstall.swf","width":<?php 
	if($resolution==1) echo '820';
	   else if($resolution==2) echo '1024';
	         else if($resolution==3) echo '1024';
			      else if($resolution==4) echo '1280';
				        else if($resolution==5) echo '1280';
	
	?>,"height":<?php 
	if($resolution==1) echo '650';
	   else if($resolution==2) echo '576';
	         else if($resolution==3) echo '720';
			      else if($resolution==4) echo '720';
				        else if($resolution==5) echo '900';
	
	?>,"wmode": "window",
	"id": "DFlash"}, {"lang": "en","userID": <?php echo $row_Cuenta["id"]; ?>,"factionID": <?php echo $row_Cuenta["empresa"]; ?>,
    "g_userName" : "<?php echo $row_Cuenta["usuario"]; ?>",
    "mainMovie" : "", //LEAVE IT EMPTY
    "cdn":"http://<?php echo $_SERVER["HTTP_HOST"]; ?>/", //CDN FOR REDIRECT
    "allowedScriptAccess":"always",
    "allowNetworking" : "true",
	"sid": "<?php echo session_id(); ?>",
   	"mapID": <?php echo $row_Cuenta["mapa"]; ?>
	}); // thx for making this 1, i looked all the way on the swf and this was the problem, i even learned how they loaded the map :v
    }
);

</script>
<style>
    #container {
        color: #FFFFFF;
        background : #000000;
	}
</style>
</body>
</html>

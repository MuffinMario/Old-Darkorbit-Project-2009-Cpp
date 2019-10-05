<?php
	/* The objective of this file is manage the paths
	 * so if we change the host we don't need to edit trillions of files 
	*/

	define("ip", "127.0.0.1");
	define("host", "localhost");
	define("web", "http://localhost");
	define("webScapped", "http:\/\/localhost");
	
	define("globals", web);
	define("globalsScapped", webScapped);
	define("globalsNoURL", host);

	define("olddorps", web."");
	define("olddorpsScapped", webScapped."");
	define("olddorpsNoURL", host."");
	
	define("mysqlUsername", "root");
	define("mysqlPassword", "");
?>
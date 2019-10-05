<?php require("config.php"); ?>
<?xml version="1.0" encoding="UTF-8"?>
<config>
<routing>
	<Dserver SVname="<?php echo $ip; ?>" MAPrange="101"/>
			<Dserver SVname="<?php echo $ip; ?>" MAPrange="103"/>
			<Dserver SVname="<?php echo $ip; ?>" MAPrange="107"/>
			<Dserver SVname="<?php echo $ip; ?>" MAPrange="110"/>
			<Dserver SVname="<?php echo $ip; ?>" MAPrange="102"/>
			<Dserver SVname="<?php echo $ip; ?>" MAPrange="108"/>
			<Dserver SVname="<?php echo $ip; ?>" MAPrange="109"/>
			<Dserver SVname="<?php echo $ip; ?>" MAPrange="105"/>
			<Dserver SVname="<?php echo $ip; ?>" MAPrange="7,13,14,18,21,22,23,25,26,27,28,51,55,58,67,68,71,72,73,74,75,76,82,92,202,255"/>
			<Dserver SVname="<?php echo $ip; ?>" MAPrange="3,5,11,15,16,20,29,53,54,61,66,69"/>
			<Dserver SVname="<?php echo $ip; ?>" MAPrange="111"/>
			<Dserver SVname="<?php echo $ip; ?>" MAPrange="104"/>
			<Dserver SVname="<?php echo $ip; ?>" MAPrange="106"/>
			<Dserver SVname="<?php echo $ip; ?>" MAPrange="2,4,6,8,9,12,62,63,70,81,93"/>
			<Dserver SVname="<?php echo $ip; ?>" MAPrange="1,10,17,19,24,42,52,56,57,64,65,91,200,201"/>
		</routing>

<chat>
	<Dchat SVname="http://<?php echo $ip; ?>" base="" suffix="" instance="45"/>
    <global>
        <host>http://<?php echo $ip; ?></host>
    </global>
</chat>
</config>
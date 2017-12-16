
#ifndef CONSTS_H
#define CONSTS_H
#include <string>

namespace Constants {
	namespace Database {
		const std::string USERS		= "cuentas";
		const std::string CLANS		= "clanes";
		const std::string EXPTABLE	= "experiencas";
		const std::string MAPS		= "maps";
		const std::string SHIPS		= "naves";
		const std::string NPCS		= "npcs";
		const std::string EQUIPMENT = "objetos";
		const std::string PORTALS	= "portales";
		const std::string RANKS		= "rangos";
		const std::string TITLES	= "titles";
		const std::string DRONES	= "vants";

		namespace User {
			const std::string USERS		= "usuario";
			const std::string USERIDS	= "id";
		}
	}
	namespace Received {
		/////////////////////////////////////////////////////////////////////
		/* List of what has a NL line feed (documentary reasons)
			(x), S|MIN , S|SMB, S|ISH, S|ROB, 5, S|IR, S|FWI, S|FWX, 9|i|0|1
			maybe some more i forgot to add more while i was typing, but these are 100% sure to have them
			in my edited swf these have been fixed to only contain one \n instead of two

			These have been fixed in the latest swf update changes
		*/
		/////////////////////////////////////////////////////////////////////

		constexpr const char SPLITPACKETFLAG		= 0xA;
		const std::string W_POLICYFILEREQUEST		= "<policy-file-request/>";
		const std::string LASERSHOOT_ABORT			= "G";
		const std::string LASERSHOOT_START			= "a";
		const std::string ROCKETSHOOT				= "v";
		const std::string HARVEST					= "HAR";
		const std::string SPECIAL					= "S";
		const std::string SPECIAL_MINE				= "S|MIN";
		const std::string SPECIAL_SMARTBOMB			= "S|SMB";
		const std::string SPECIAL_INSTASHIELD		= "S|ISH";
		const std::string SPECIAL_REPBOT			= "S|ROB";
		const std::string SPECIAL_WIZ				= "S|WIZ";
		const std::string SPECIAL_JUMPCPU			= "S|J";
		const std::string SPECIAL_INSTANTREPAIR		= "S|IR";
		const std::string TRADEORE_SEND				= "b";
		const std::string LASERAMMO_CHANGE			= "u";
		const std::string ROCKETAMMO_CHANGE			= "d";
		const std::string FORCEINIT					= "i";
		const std::string LOGOUT					= "l";
		const std::string LOGOUT_CANCEL				= "o";
		const std::string JUMP						= "j";
		const std::string TRADEORE_MSG				= "T|X";
		const std::string PING						= "PNG";
		const std::string LOGIN_REQUEST				= "LOGIN";
		const std::string QUEST_REMOVEPRIVILEGE		= "9|p";
		const std::string FARM_ORE					= "w";
		const std::string MOVE						= "1";
		const std::string SELECT					= "L";
		const std::string SETTINGS					= "A|SET";
		const std::string HELPWINDOW				= "9|i|0|1";
		/* TODO: muchshitAS.txt
					find out what this._parent._parent.cmd  is ( three streampackets)
					*/
		const std::string UNVERIFIED_AMMOBUY		= "5";
		const std::string UNVERIFIED_FIREWORKEXPL	= "S|FWI";
		const std::string UNVERIFIED_FIREWORKSPAWN	= "S|FWX";
	}
	namespace Send {
		const std::string W_CROSSDOMAINPOLICY		= "<?xml version=\"1.0\"?><cross-domain-policy xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://www.adobe.com/xml/schemas/PolicyFileSocket.xsd\"><allow-access-from domain=\"*\" to-ports=\"*\" secure=\"false\" /><site-control permitted-cross-domain-policies=\"master-only\" /></cross-domain-policy>";
		const std::string ERR						= "ERR";
		const std::string ALIENMOVE					= "1";
		const std::string EVENTS					= "D";
		const std::string LASERSHOOT				= "a";
		const std::string ROCKETSHOOT				= "v";
		const std::string SPAWNOPPONENT				= "C";
		const std::string REMOVEOPPONENT			= "R";
		const std::string UPDATEORES				= "E";
		const std::string BUY						= "5";
		const std::string SPAWNORE					= "r";
		const std::string REMOVEORE					= "q";
		const std::string LOGOUT					= "l";
		const std::string MSG_LOGOUTNOTPOSSIBLE		= "t";
		const std::string SPAWNBOX					= "c";
		const std::string REMOVEBOX					= "2";
		const std::string MSG_REWARD				= "y|";
		const std::string MSG_BOXTOOBIG				= "y|BTB";
		const std::string MSG_BOXALREADYCOLLECTED	= "y|BAH";
		const std::string MSG_CLOAKDETECTED			= "y|DET";
		const std::string MSG_COLLECTDOUBLER		= "y|BMP";
		const std::string MSG_ANZAHLDERGEGNER		= "y|SLE";
		const std::string DETONATEMINE				= "y|MIN";
		const std::string COLLECTORE				= "e";
		const std::string CHANGEMAP_I				= "i";
		const std::string CHANGEMAP_M				= "m";
		const std::string MSG_NOROCKETSLEFT			= "W|R|0";
		const std::string MSG_NOLASERSLEFT			= "W|L|0";
		const std::string MSG_CHANGEROCKET			= "W|R";
		const std::string MSG_CHANGELASER			= "W|L";
		const std::string ABORTATTACK				= "G";
		const std::string QUEST						= "Q|"; // working around here, might not be possible as an empty string
		const std::string QUEST_DONE				= "Q|DONE";
		const std::string QUEST_CANCEL				= "Q|CANCEL";
		const std::string CHANGECONFIG				= "S|CFG";
		const std::string SPAWNPORTAL				= "p";
		const std::string SPAWNSTATION				= "s";
		const std::string UPDATEHP					= "A|HPT";
		const std::string UPDATESHD					= "A|SHD";
		const std::string RESETROCKETCOOLDOWN		= "A|RCD";
		const std::string UPDATECURRENCY			= "A|C";
		const std::string UPDATECARGO				= "A|c";
		const std::string UPDATEAMMO				= "A|a";
		const std::string SETTINGS_ITEMS			= "A|ITM";
		const std::string SETTINGS_SETTINGS			= "A|SET";
		const std::string COOLDOWN					= "A|CLD";
		const std::string LEVELUP					= "A|LUP";
		const std::string SENDMESSAGE				= "A|STD";
		const std::string SENDMESSAGEFORMAT			= "A|STM";
		const std::string UPDATEJUMPCPU				= "A|CPU|J";
		const std::string UPDATECLOAKCPU			= "A|CPU|C";
		const std::string UPDATEAIMCPU				= "A|CPU|A";
		const std::string UPDATEAROLXCPU			= "A|CPU|R";
		const std::string SETCONFIGAMOUNT			= "A|CC";
		const std::string UPDATEBOOSTERS			= "A|BS";
		const std::string UPDATESPEED				= "A|v";

		/* MISSING INFORMATION AROUND FIREWORKS*/
		//thesis: A: UI  n: visual spacemap
		const std::string DETONATEFIREWORK			= "A|FWX|INL";
		const std::string UPDATEFIREWORKS			= "A|FWX|FWL";
		const std::string EXPLODEFIREWORK			= "n|FWI";
		const std::string EXPLODEFIREWORKS			= "n|FWG";

		const std::string UPDATEWIZ					= "A|WIZ";
		const std::string ADMINCLI					= "A|CLI";
		const std::string MSG_NEEDLEVELTOJUMP		= "k";
		const std::string UPDATELASERCOUNT			= "L";
		const std::string UPDATELASERS				= "B";
		const std::string SHOWHPANDSHDOPPONENT		= "N";
		const std::string UPDATEROCKETANDMINES		= "3";
		const std::string KILLUSER					= "K";
		const std::string RECEIVEDAMAGE				= "Y";
		const std::string MAKEDAMAGE				= "H";
		const std::string MAKEMISS					= "M";
		const std::string RECEIVEMISS				= "Z";
		const std::string MISSHITEVENT				= "M|L";
		const std::string MSG_MATERIALCOLLECTFAIL	= "h";
		const std::string MSG_CARGOISFULL			= "f";
		const std::string MSG_ESCAPEDTHEATTACK		= "V";
		const std::string MSG_OUTOFRANGE			= "O";
		const std::string MSG_DEMILITARIZEDZONE		= "P";
		const std::string MSG_BATTLECONTINUES		= "X";
		const std::string MSG_OPPONENTFIGHTCANCEL	= "J";
		const std::string MSG_CONNECTIONINTERRUPTED = "KIK";
		const std::string DISPLAYSTARSYSTEM			= "z";
		const std::string SETDRONES					= "n|d";
		const std::string SETDRONESALT				= "n|e";
		const std::string UPDATEALTERSTATE			= "n|w";
		const std::string SPAWNATDOOR				= "n|s";
		const std::string GREYENEMY					= "n|LSH";
		const std::string UNGREYENEMY				= "n|USH";
		const std::string MAKEINVISIBLE				= "n|INV";
		const std::string CHANGEDIPLOMACY			= "n|CDC";
		const std::string SHOWSMB					= "n|SMB";
		const std::string SHOWISH					= "n|ISH";
		const std::string SHOWMEDALBOX				= "n|MDL";
		const std::string SETTITLE					= "n|t";
		const std::string MSG_RECEIVEBOOSTER		= "n|fbo";
		const std::string RENAMEMINIMAPTITLE		= "n|mt";
		const std::string CHANGELASERTO				= "4|L|";
		const std::string CHANGEROCKETTO			= "4|R|";
		const std::string MOVE_FORCE				= "T";
		const std::string INITALIZE					= "I";
		const std::string LOGIN_SENT_TRUE			= "8";
		const std::string GUEST_PLEASE_REGISTER		= "GUEST_PLEASE_REGISTER";
		const std::string QUEST_HUD_INITALIZE		= "9|i";
		const std::string QUEST_HUD_UPDATE			= "9|upd";
		const std::string QUEST_CONGRATULATION		= "9|a";
		const std::string QUEST_CONGRATULATION_EVENT= "9|as";
		const std::string QUEST_HUD_CANCELED		= "9|c";
		const std::string QUEST_HUD_FAILED			= "9|f";
		// AGRESSIVE; MIGHTCRASH IF NOT DEFINED
		const std::string QUEST_HUD_PRIVILEGE		= "9|c"; 
		const std::string SPAWNCROSSHAIR			= "6";
	};
}
#endif
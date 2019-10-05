# Old-Darkorbit-Project-2009-C++
An Emulator that is capable of running the modified DarkOrbit swf as a server to play on, also called "PoopOrbit"

## What is this?
Well as you might have already read the former text, this is a project I did to play the old DarkOrbit back in the days where it actually was fun to play. I personally don't want to host any big game, since this game does indeed get boring with no companion, which is probably why this is not going to be a good project in the first place. The only thing that drives people wanting to play this game is their increasing nostalgia towards the old times where they had fun in the game, which is why many people prefer older versions to the current one, but not everyone prefers one specific version, but prefer the one they had the most fun in. People don't want old darkorbit, people want the community and fun they had while playing old darkorbit... 

## What are the folders?

1. spacemap server:
This is the most important folder that contains the source of the project. You need three external libraries to run this project.
  - boost 1.70 (installable by vcpkg)
  - libconfig (installable by vcpkg)
  - MySQL C Connector 6.1 (https://dev.mysql.com/downloads/connector/c/) (VERSIONID 50716) (only manually installable (you must change the include/lib paths in the project))
 
This project has been tested with both Debug and Release builds on x86 platforms. x64 has been untested.

2. bin_swf:
If you already have your own CMS to use, you can find all modified client side files, aswell as all related to those modified. These mainly include SWF files.
3. htdocs:
If you do not have your own CMS, use this htdocs folder for xampp to test out the server. All you have to do is start the server and open http://hostaddr/internalSpacemapTestUnique.php . You can chose between 10 test entities to play with. (database userids 1-10). You can add the paramter uid with a number reaching from 1-10 (e.g. http://hostaddr/internalSpacemapTestUnique.php?uid=2 lets you play the second testing unit). If no parameter is selected, every call to the webpage increases the id youre currently playing with.

Sidenote: htdocs/swf_global is the same as bin_swf plus unused inventory related files.
4. database:
Well... the database. It is derived from manulaiko's old "old darkorbit project" but changed to slightly to follow core concepts of database keeping, but still needs a total revamp in order to implement configurations.
## What else do I need?
You need to import the phpmyadmin mysql file which the server runs on. 
For further development you can always take a look at the packet list from manulaiko which has been greatly improved by me. You can find it in the root directory of this project

## Which ports do I need to open?
1. 843 TCP : This port is needed for the policy server to accept the connection
2. 5152 TCP : Instead of having multiple servers for each map (like the swf handles jumps) we have one big server running which handles all the actions on different maps. To change this edit the array containing somewhere around eight times the port 5152.

## Feature list
- Able to connect to the spacemap.swf by user information through the database
- Movement Manager to prevent cheaty movement
- Implemented easy-to-use Packet builder
- Reads all maps from the database and build the maps accordingly (the whole star system should be accessible)
- Sounds & Music
- Modified client to ease programming and gameplay (check the changes in Resource Files/spacemap_backup/changes.txt)
- Working fighting system 
- Gate jumping 
- Position dependent UI changes
- Working Server-Sided MovementManager (an a-little-bit-more-detailed version of the problems should exist in "MovementManager.h")
- Fully functional NPC behaviour, except Cubikon/Protegit and no bosses/ubers, and escapes!
- PvP
- Some extras
- Bonus boxes.
- Radiation zones

## What needs to be implemented sooner or later (ToDo)
- Configurations
- A total database revamp (each item having a unique id, type and equipped-on ID, configs are basically impossible atm)
- website <-> spacemap communication
- NPC flee on X% HP (15%?)
- NPC stop interest after not been shot after X seconds (120?)
- webserver authentification
- Finish missing extra items (also update on DB/webpage change)
- Save settings and handle not yet handled parameters (S| - settings packet / d| - laser selected / r| - rocket select)
- Cubikon behaviour
- Update Ammo/Rocket/Extras on shoot/buy/update/use
- NPC behaviour on gates (distance from gates if not attacked)
- Fix 4-4 gates (very easy)
- Add ore regions X-1 to X-4
- Finish cargo (max cargo limit, update UI, sell ores on  base)
- Only show enemies within 10000 units range + selected + enemy on first 2 home maps


## Thanks to
- Cryz35, because yeah sorta helped me there
- Heaven' giving me a little bit of knowledge about swf modifying
- Edox for giving me the sound files
- turso for fixing the issue of the client movement (the SWF itself was not calculating correctly)


(btw i dont know why my two accounts got mixed up uploading these files, don't get confused with TigerOs, that's still me)

Good luck,
yours sincerely, MuffinMario

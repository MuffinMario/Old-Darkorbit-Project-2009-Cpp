#include "SessionsManager.h"
#include "Mob.h"
#include "Collectable.h"
#include "ResourceBox.h"
#include "DatabaseUtils.h"

void CSessionsManager::fastTick()
{
	std::vector<CSession>::iterator permSessionIt = m_permSessions.begin();
	std::vector<CSession>::iterator tempSessionIt = m_tempSessions.begin();

	while (true)
	{
		/* Iterate */
		std::vector<CSession>::iterator sessionIt;
		if (permSessionIt != m_permSessions.end())
		{
			sessionIt = permSessionIt;
			++permSessionIt;
		}
		else
		{
			/* permIterator is done iterate throguh temp */
			if (tempSessionIt != m_tempSessions.end())
			{
				sessionIt = tempSessionIt;
				++tempSessionIt;
			}
			else
			{
				/* tempIterator is done aswell */
				break;
			}
		}
		CSession& session = sessionIt.operator*();



		/* Start internal loop */
		map_t map = session.getMapId();

		//if (session.getAllConnections().size() == 0) continue; //skip if there aint even no players.
															   //this function should NOT be allowed to call mob->die() due to thread safety
		/* This function will not handle deletion/insertion of vector mobs in session */
		session.lockMobsRead();
		/* Reference instead of copy because the npccontainer contains a shared ptr and
			cannot be deleted since the mutex stops it from deleting until loop is over
		*/
		for (auto& mobs : session.getMobs()) 
		{
			std::shared_ptr<CMob> mp = mobs.second;
			if (mp != nullptr)
			{
				//if alien wants to make another move (when the time has come)
				Position_t p = mp->getPosition();
				//highest cpu take i guess
				id_t triggeredId = 0;
				if (mp->getShip().id != 80)
				{
					triggeredId = mp->getFocusedPlayer();
					if (!triggeredId) //dont handle cubikonikons
					{
						session.lockConnectionsRead();
						for (auto& playerPair : session.getAllConnections())
						{
							handlePtr& player = playerPair.second;
							if (player != nullptr)
							{
								pos_t a = std::abs(player->getX() - p.first);
								pos_t b = std::abs(player->getY() - p.second);
								pos_t distance = std::sqrt(a*a + b * b);
								if (!player->m_pbIsInvisible && distance < Constants::Game::FIGHT_RANGE_NPC)//TODO: && !player->isInvisible())
								{
									mp->setFocusToPlayer(player->getID());
									mp->setRealWaitingTime(0); // so it instantly flies towards enemy
									triggeredId = player->getID();
									break;
								}
							}
						}
						session.unlockConnectionsRead();

					}
				}
				if (mp->isGreyed() && mp->getLastTimeShotByBelongedPlayer() + 10000 < getTimeNow())
				{

					dcout << "Ungrey opponent: " << cendl;
					mp->setLastTimeShotByBelongedPlayer(0); //* Kinda unncessary but who cares * /
					mp->ungrey();
					session.sendEveryone(m_pm.ungreyOpponent(mp->getId()));
				}
				if (mp->getNextMovingTime() < getTimeNow())
				{

					handlePtr userIfTriggered;
					if (triggeredId)
						//Handle "players"
					{
						if (userIfTriggered = session.getHandler(triggeredId))
						{
							pos_t a = std::abs(userIfTriggered->getX() - p.first);
							pos_t b = std::abs(userIfTriggered->getY() - p.second);
							pos_t distance = std::sqrt(a*a + b * b);

							const pos_t DISTANCE_MAX_TILL_FOLLOW = 300;
							const pos_t DISTANCE_MAX_TILL_CIRCLE = 220;
							const pos_t DISTANCE_FOLLOW = 20;
							const pos_t DISTANCE_CLOSE = 200;
							if (distance > DISTANCE_MAX_TILL_FOLLOW)
							{
								//hey lets get a bit closer to our beloved friend enemy

								//for now its random, in the future make the degree with the shortest distance
								// (point enemy <-> point self    just make 2 points degree ez bye
								auto pos = mp->getPosition();
								auto mypos = Position_t(userIfTriggered->getX(), userIfTriggered->getY());
								unsigned int degree = random<uint32_t>(360); //atan2(mypos.second - pos.second, mypos.first - pos.first);
								decltype(pos) newmobpos = std::make_pair(mypos.first, mypos.second);
								//x
								newmobpos.first += DISTANCE_CLOSE * std::cos(degree);
								newmobpos.second += DISTANCE_CLOSE * std::sin(degree);
								mp->move(newmobpos.first, newmobpos.second);
								mp->setRealWaitingTime(100);// 0.1 sec w8
							}
							else if (distance > DISTANCE_MAX_TILL_CIRCLE)
							{
								unsigned int degree = random<uint32_t>(360);
								auto pos = mp->getPosition();
								auto mypos = Position_t(userIfTriggered->getX(), userIfTriggered->getY());
								decltype(pos) newmobpos = std::make_pair(mypos.first, mypos.second);
								//x
								newmobpos.first += DISTANCE_CLOSE * std::cos(degree);
								newmobpos.second += DISTANCE_CLOSE * std::sin(degree);
								mp->move(newmobpos.first, newmobpos.second);
								mp->setRealWaitingTime(1500);// 0.1 sec w8
							}
							else if (distance < DISTANCE_FOLLOW * 1.5)
							{
								//* TOO CLOSE * /
								auto pos = mp->getPosition();
								auto mypos = Position_t(userIfTriggered->getX(), userIfTriggered->getY());
								unsigned int degree = random<uint32_t>(360);
								decltype(pos) newmobpos = std::make_pair(mypos.first, mypos.second);
								//x
								newmobpos.first += DISTANCE_CLOSE * std::cos(degree);
								newmobpos.second += DISTANCE_CLOSE * std::sin(degree);
								mp->move(newmobpos.first, newmobpos.second);
								mp->setRealWaitingTime(1500);// 0.1 sec w8
							}
							else
							{
								//* MOVEMENT IN RANGE * /
								mp->setRealWaitingTime(1500);
							}
						}
						else
						{
							//enemy "misteroiusly" disappeared im not gonna fix this typo its 5 am let me be
							triggeredId = 0;
							mp->setFocusToPlayer(0);
						}
					}
					else{
						//handle "roaming"
						int randx = 0;
						int randy = 0;
						const int RANGE = 10000;
						do {
							randx = random<pos_t>(-RANGE / 2, RANGE / 2) + p.first;
							randy = random<pos_t>(-RANGE / 2, RANGE / 2) + p.second;
						} while (randx > session.getMap().getWidth() || randy > session.getMap().getHeight() || randx < 0 || randy < 0);

						mp->move(randx, randy);
						mp->generateRandomWaitingTime(2500, 15000);

					}
				}
			}
		}//*/
		session.unlockMobsRead();
	}
}

void CSessionsManager::secondTick()
{


	std::vector<CSession>::iterator permSessionIt = m_permSessions.begin();
	std::vector<CSession>::iterator tempSessionIt = m_tempSessions.begin();

	std::vector<handlePtr> goingToDiePlayers; // this hurts
	for(;;)
	{
		std::vector<CSession>::iterator sessionIt;
		if (permSessionIt != m_permSessions.end())
		{
			sessionIt = permSessionIt;
			++permSessionIt;
		}
		else
		{
			/* permIterator is done iterate throguh temp */
			if (tempSessionIt != m_tempSessions.end())
			{
				sessionIt = tempSessionIt;
				++tempSessionIt;
			}
			else
			{
				/* tempIterator is done aswell */
				break;
			}
		}
		CSession& session = sessionIt.operator*();

		/* Connections */
		session.lockConnectionsRead();
		for (auto player_pair : session.getAllConnections())
		{
			handlePtr& player = player_pair.second; //copy shared ptr for ref count
			if (player)
			{
				DBUtil::funcs::setPos(player->getID(), player->getPos());
				//radiation zone check
				bool isInRad = player->isInRadiationzone();
				int secondsInRad = player->getSecondsInRadiationzone();
				if (isInRad || secondsInRad) // we need to refresh it back to 0 if player is outside
				{
					if (!isInRad)
						secondsInRad = 0;
					else
					{
						player->setSecondsInRadiationzone(++secondsInRad);
						// d(t) = maxhp*0.01 + t^2*maxhp*0.01
						// 1% of hp + time squared * 1% of hp
						damage_t dmg = player->getMaxHP() * 0.01 + secondsInRad * secondsInRad * player->getMaxHP() * 0.01;

						bool dead = player->receiveDamageHP(dmg) < 0;
						// i am unhappy with this solution
						if (dead)
						{
							goingToDiePlayers.push_back(player);
							continue; // dead, we dont care
						}
						
					}
				}
				if (timeHasPassed(player->getShieldPreventTime(),5000))
				{
					//* Handle Shield regen* /
					shield_t sr = player->getShieldRegen();
					player->addSHD(sr);
					//* Handle Repair* /
					if (timeHasPassed(player->getRepairPreventTime(), 2000))
					{
						if (player->isRepairing())
						{
							health_t beforeHeal = player->getHP();

							health_t give = player->getRepBotHPRegen();

							health_t received = player->addHP(give);
							if (beforeHeal + received >= player->getMaxHP())
							{
								player->setRepairing(false);
							}
						}
					}
				}
				player->checkForObjectsToInteract();

			}
			else
			{
				dcout << "Player is nullptr this is a major crime call 9-1-1" << cendl;
			}
		}
		session.unlockConnectionsRead();
		//I did put effort that deleting a player twice cannot occur, but you know there will be cases, so the worst case that should have been taken care of by CSession should be:
		// 1. Player dies by radiation zone
		// 2. before the program reaches this for loop somebody kills the player in an attacker thread
		// 3. the player is removed from connectionTable ( shared_ptr from goingToDiePlayers keeps player data "alive", but hes removed from the table nontheless)
		// 4. the player will not get removed again in this for loop (?????)
		// TODO: TEST THIS BY BREAKPOINTING ON ABOVE session.unlockConnectionsRead(); 
		for (auto& players : goingToDiePlayers)
		{
			players->die();
		}
		goingToDiePlayers.clear();


		/* Collectable tick includes delete operation */
		session.lockCollectablesRead();
		for (auto it = session.getCollectables().begin(); it != session.getCollectables().end();)
		{
			std::shared_ptr<ICollectable>& collectable = (*it).second;
			if (collectable && (collectable->getType() == CResourceBox::RESOURCE_BOX_DEFAULT_COLLECTABLE_ID ||
				collectable->getType() == CResourceBox::RESOURCE_BOX_PRIVATE_COLLECTABLE_ID))
			{
				std::shared_ptr<CResourceBox> res = std::static_pointer_cast<CResourceBox>(collectable);
				if (res->boxIsPrivate())
				{
					res->m_belongsToSeconds--;
					if (res->m_belongsToSeconds == 0)
					{
						/* Unprivate */
						res->onChangeToPublic();
					}
				}
				if (res->m_existsSeconds-- == 0) // yes it goes to -1 but then its precise and the time is correct
				{
					/* Unlock read to write operates then put it back to lock read*/
					session.unlockCollectablesRead();
					it = res->remove();
					session.lockCollectablesRead();
				}
				else
				{
					it++;
				}
			}
			else
			{
				it++;
			}
		}
		session.unlockCollectablesRead();


		session.lockMobsRead();
		//this segment will NOT insert delete the container
		for (auto mobp : session.getMobs())
		{
			std::shared_ptr<CMob>& mp = mobp.second;
			if (mp->getFocusedPlayer() > 0 && mp->getFocusedPlayer() < BEGIN_MOB_IDS &&
				mp->attacking())
			{
				auto player = session.getHandler(mp->getFocusedPlayer());
				if (player) {
					Position_t alienpos = mp->getPosition();
					Position_t userpos =
						std::make_pair(player->getX(),
							player->getY());
					pos_t x = alienpos.first - userpos.first;
					pos_t y = alienpos.second - userpos.second;
					auto distance = std::sqrt(x * x + y * y);
					if (distance < Constants::Game::FIGHT_RANGE_NPC)
					{
						/* IN RANGE */
						damage_t damage = m_dm.damageLaser(mp->getId(), 1, mp->getShip().dmg, false);

						//player->sendPacket(pm.damageBubbleSelf()
						if (player->receiveDamagePure(damage) >= 0)
						{
							/* Player is alive and healthy and is able to get attacked */
							mp->attack(mp->getFocusedPlayer());
						}
						else
						{
							/* Player is ded */
							player->die();
							mp->abort();
							mp->setLastTimeShotByBelongedPlayer(0LL);
							mp->setFocusToPlayer(0);
						}

					}
					else
					{
						//no player founderino
						//dcout << "out of range but its notout of range but something i wrote too late in the night" << cendl;

						//stop the lasers. I have yet to find the packet that keeps "Locking" on a player but doesnt shoot lasers/whatever on him
						mp->abort();
						//mp->setLastTimeShotByFirstPlayer(0LL);
						//mp->setTriggerPerson(0);
					}
				}
			}
		}
		session.unlockMobsRead();
	}
}

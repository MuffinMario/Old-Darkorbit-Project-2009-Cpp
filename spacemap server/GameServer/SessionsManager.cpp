#include "SessionsManager.h"
#include "Mob.h"
#include "Collectable.h"
#include "ResourceBox.h"
#include "Firework.h"
#include "DatabaseUtils.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <queue>

void CSessionsManager::fastTick()
{
	std::vector<CSession>::iterator permSessionIt = m_permSessions.begin();
	std::vector<CSession>::iterator tempSessionIt = m_tempSessions.begin();

	std::queue<id_t> toDeleteCollectables;

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


		const auto timeNow = getTimeNow();
		/* Start internal loop */
		const map_t map = session.getMapId();

		//if (session.getAllConnections().size() == 0) continue; //skip if there aint even no players.
															   //this function should NOT be allowed to call mob->die() due to thread safety
		/* This function will not handle deletion/insertion of vector mobs in session */
		session.foreachMobs([&timeNow,&session](id_t id, std::shared_ptr<CMob>& mp) {

			if (mp != nullptr) // not_null instead?
			{
				mp->fastTick(timeNow,session);
			}
		});

		session.foreachCollectable(
			[&toDeleteCollectables, &timeNow](id_t id, std::shared_ptr<ICollectable>& c)
			{
				std::shared_ptr<CFirework> firework = std::static_pointer_cast<CFirework>(c);
				if (timeHasPassed(timeNow, firework->getAliveTime(), 0))
				{
					toDeleteCollectables.emplace(firework->getId());
				}
			}
			, [](id_t id, std::shared_ptr<ICollectable>& c)
			{
				return c->getType() > 100;
			}
			);
		session.foreachConnections(
			[&toDeleteCollectables, &timeNow](id_t id, handlePtr& c)
			{
				c->checkForObjectsToInteract();
			}
			);

		if (toDeleteCollectables.size() != 0U)
		{
			while (toDeleteCollectables.size() != 0U)
			{
				const auto& front = toDeleteCollectables.front();
				session.sendEveryone(to_string("0|n|FWI|") + to_string(front));
				session.removeCollectable(front);
				toDeleteCollectables.pop();
			}
		}
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
		const auto timeNow = getTimeNow();
		/* Connections */
		session.lockConnectionsRead();
		for (auto player_pair : session.getAllConnections())
		{
			handlePtr& player = player_pair.second; //copy shared ptr for ref count
			if (player)
			{
				player->checkForObjectsToInteract();
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
						// damage is relative to distance in radiation zone
						Position_t pos = player->getPos();
						//signed < unsigned is not a beauty
						pos_t mapWidth = static_cast<pos_t>(session.getMap().getWidth());
						pos_t mapHeight = static_cast<pos_t>(session.getMap().getHeight());
						pos_t dx = 0,dy = 0;

						if (pos.first < 0 || pos.first > mapWidth)
							dx = pos.first - (pos.first > mapWidth ? mapWidth : 0);
						if (pos.second < 0 || pos.second > mapHeight)
							dy = pos.second - (pos.second > mapHeight ? mapHeight : 0);
						double distance = std::sqrt(dx * dx + dy * dy);

						health_t maxHP = player->getMaxHP();
						damage_t dmg = 0;

						if (distance > CMap::RADIATIONZONE_DISTANCE_STRONG)
							dmg = maxHP * 0.10;
						else if (distance > CMap::RADIATIONZONE_DISTANCE_MEDIUM)
							dmg = maxHP * 0.05;
						else if (distance > CMap::RADIATIONZONE_DISTANCE_WEAK)
							dmg = maxHP * 0.01;

						bool dead = player->receiveDamageHP(dmg) < 0;
						// i am unhappy with this solution
						if (dead)
						{
							goingToDiePlayers.push_back(player);
							continue; // dead, we dont care about the other ifs
						}
						else
							player->updateHealth(dmg); // make a bubble function?
						
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
		// 4. the player will not get removed again in this for loop
		for (auto& players : goingToDiePlayers)
		{
			players->die();
		}
		goingToDiePlayers.clear();


		/* Collectable tick includes delete operation */
		session.lockCollectablesRead();
		for (auto it = session.getCollectables().begin(); it != session.getCollectables().end();)
		{
			const std::shared_ptr<ICollectable>& collectable = (*it).second;
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


		session.foreachMobs([&timeNow, &session](id_t id, std::shared_ptr<CMob>& m) {
			if (m != nullptr)
			{
				m->secondTick(timeNow,session);
			}
		});
	}
}

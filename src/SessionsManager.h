#pragma once
#ifndef SESSIONSMANAGER_H
#define SESSIONSMANAGER_H
#include <vector>
#include <cmath>
#include <exception>

#include "Defines.h"

#include "PacketComposer.h"
#include "DamageManager.h"

#include "Session.h"


class session_permanently_exists_exception : public std::exception
{
public:
	session_permanently_exists_exception() : std::exception("Permanent Session already exists") { }
};

/*
 Totally rewritten sessions manager. In the game there are 2 different but similar sessions
		- Maps that are only once per session
		- Sessions that are only temporal (GGs, TDM, Invasion Gate, etc.)
**/
class CSessionsManager {
	std::vector<CSession> m_tempSessions;
	std::vector<CSession> m_permSessions;
	CPacketComposer m_pm;
	CDamageManager m_dm;
public:
	static bool mapIsInPermanent(map_t mapid)
	{
		return (mapid >= 1 && mapid <= 29) || mapid == 42;
	}

	/* Deleted after user count is zero*/
	CSession& createTempSession(map_t mapid)
	{
		m_tempSessions.emplace_back(mapid);
		return m_tempSessions.back();
	}
	CSession& createPermanentSession(map_t mapid)
	{
		if (permanentSessionExists(mapid))
		{
			throw session_permanently_exists_exception();
		}
		else
		{
			m_permSessions.emplace_back(mapid);
			return m_permSessions.back();
		}
	}
	bool permanentSessionExists(map_t withMapId)
	{
		for (auto& it : m_permSessions)
		{
			if (it.getMapId() == withMapId)
				return true;
		}
		return false;
	}

	void joinSession(const handlePtr user, std::string sessionID, map_t mapID)
	{
		if (mapIsInPermanent(mapID))
		{
			getPermanentSession(mapID).joinSession(user, sessionID);
		}
		else
		{
			createTempSession(mapID).joinSession(user, sessionID);
		}
	}

	CSession& getPermanentSession(map_t mapid) {
		for (auto& it : m_permSessions)
		{
			if (it.getMapId() == mapid)
				return it;
		}
		dcout << "Could not find permanent session with mapid " << mapid << ". Creating new one" << cendl;
		return createPermanentSession(mapid);
	}
	CSession& getAnySession(map_t mapid) {
		for (auto& it : m_permSessions)
		{
			if (it.getMapId() == mapid)
				return it;
		}
		for (auto& it : m_tempSessions)
		{
			if (it.getMapId() == mapid)
				return it;
		}
		dcout << "Could not find any session with mapid " << mapid << ". Creating new temp one" << cendl;
		return createTempSession(mapid);
	}
	void clearAll()
	{
		m_tempSessions.clear();
		m_permSessions.clear();
	}


	void fastTick();
	
	void secondTick();

};
#endif
#include "Firework.h"

CFirework::CFirework(id_t id, EFireworkSize size, id_t owner, std::array<EFireworkColor, 3> colors, Position_t& pos, CSession& session) : ICollectable(id, pos.first, pos.second, session) {
	m_owner = owner;
	m_explosionColors = std::move(colors);
	m_type = generateTypeID(size,colors);
}

id_t CFirework::generateTypeID(const EFireworkSize& size, const std::array<EFireworkColor, 3> & colors)
{
	id_t id = 0;
	if (size != EFireworkSize::L)
	{
		id = size * 100 + colors.at(0) * 10 + colors.at(1);
	}
	else
	{
		id = size * 100 + colors.at(0) * 100 + colors.at(1) * 10 + colors.at(2);
	}
	return id;
}

void CFirework::spawn()
{
	spawnBoxDefault();
}

void CFirework::spawn(id_t to)
{
	spawnBoxDefault(to);
}

void CFirework::collect()
{
	/* NO COLLECT */
}

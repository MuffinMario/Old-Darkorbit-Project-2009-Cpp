#pragma once
#include "Defines.h"
class COreInfo {
	credits_t m_credits;
	ore_t m_type;
public:
	COreInfo() {

	}
	COreInfo(ore_t type,credits_t credits) {
		m_type = type;
		m_credits = credits;
	}

	credits_t	getCreditPrice() { return m_credits; }
	void		setCreditPrice(credits_t credits) { m_credits = credits; }
	ore_t		getOreType() { return m_type; }
	void		setOreType(ore_t type) { m_type = type; }

};
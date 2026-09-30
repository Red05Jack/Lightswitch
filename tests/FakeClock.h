#pragma once

#include "IClock.h"

// Clock whose current time is set explicitly by tests.
class FakeClock : public IClock {
public:
	explicit FakeClock(const QDateTime& now) : m_now(now) {}

	QDateTime Now() const override { return m_now; }

	void SetNow(const QDateTime& now) { m_now = now; }

private:
	QDateTime m_now;
};

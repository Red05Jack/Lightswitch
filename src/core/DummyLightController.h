#pragma once

#include "ILightController.h"

// Light controller that only remembers the state, used until real hardware is connected.
class DummyLightController : public ILightController {
	Q_OBJECT

public:
	explicit DummyLightController(QObject* pParent = nullptr);

	bool IsOn() const override;
	void SetOn(bool isOn) override;

private:
	bool m_isOn = false;
};

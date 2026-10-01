#include "DummyLightController.h"

DummyLightController::DummyLightController(QObject* pParent)
	: ILightController(pParent) {
}

// Returns whether the simulated light is on.
bool DummyLightController::IsOn() const {
	return m_isOn;
}

// Stores the new state and notifies listeners when it changed.
void DummyLightController::SetOn(bool isOn) {
	if (m_isOn == isOn) {
		return;
	}

	m_isOn = isOn;
	emit StateChanged(m_isOn);
}

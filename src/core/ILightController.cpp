#include "ILightController.h"

ILightController::ILightController(QObject* pParent)
	: QObject(pParent) {
}

// Switches the light to the opposite of its current state.
void ILightController::Toggle() {
	SetOn(!IsOn());
}

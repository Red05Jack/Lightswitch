#include "LocationController.h"

namespace {
// Returns the index of the preset with the given name, or -1 for a custom location.
int FindPresetIndex(const QList<LocationSettings>& presets, const QString& name) {
	for (int index = 0; index < presets.size(); ++index) {
		if (presets.at(index).name == name) {
			return index;
		}
	}
	return -1;
}
}

LocationController::LocationController(const LocationSettings& location, QObject* pParent)
	: QObject(pParent)
	, m_location(location) {
}

// Returns the selectable cities, Austria first.
const QList<LocationSettings>& LocationController::Presets() {
	static const QList<LocationSettings> presets = {
		{QStringLiteral("Graz"), 47.0707, 15.4395},
		{QStringLiteral("Vienna"), 48.2082, 16.3738},
		{QStringLiteral("Salzburg"), 47.8095, 13.0550},
		{QStringLiteral("Innsbruck"), 47.2692, 11.4041},
		{QStringLiteral("Linz"), 48.3069, 14.2858},
		{QStringLiteral("Klagenfurt"), 46.6247, 14.3053},
		{QStringLiteral("Munich"), 48.1374, 11.5755},
		{QStringLiteral("Berlin"), 52.5200, 13.4050},
		{QStringLiteral("Hamburg"), 53.5511, 9.9937},
		{QStringLiteral("Zurich"), 47.3769, 8.5417}};
	return presets;
}

// Returns the name of the selected location.
QString LocationController::Name() const {
	return m_location.name;
}

// Returns the selected location including its coordinates.
const LocationSettings& LocationController::Location() const {
	return m_location;
}

// Selects the next preset city, wrapping around at the end of the list.
void LocationController::SelectNext() {
	SelectPresetOffset(1);
}

// Selects the previous preset city, wrapping around at the start of the list.
void LocationController::SelectPrevious() {
	SelectPresetOffset(-1);
}

// Moves the selection by the offset relative to the current preset; a custom location starts at the list ends.
void LocationController::SelectPresetOffset(int offset) {
	const QList<LocationSettings>& presets = Presets();
	const int count = static_cast<int>(presets.size());
	const int currentIndex = FindPresetIndex(presets, m_location.name);
	const int nextIndex = currentIndex < 0 ? (offset > 0 ? 0 : count - 1) : (currentIndex + offset + count) % count;

	m_location = presets.at(nextIndex);
	emit LocationChanged(m_location);
}

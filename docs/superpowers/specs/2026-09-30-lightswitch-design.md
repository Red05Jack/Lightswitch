# Lightswitch - Design

Datum: 2026-09-30

## Ziel

Eine Lichtschalter-Oberflaeche fuer ein 720x720-Display, gebaut mit C++17 und Qt 6.
Laeuft unter Windows (VS 2026 / MSVC) und Linux (CMake). Das Layout folgt 1:1 der
Design-Vorlage `Lightswitch.svg` mit den Fonts Manufaktur Bold und Black.

## Nicht-Ziele (vorerst)

- Keine echte Hardware-Ansteuerung des Lichts (nur Simulation).
- Keine Google-Calendar-Anbindung (nur Dummy, Schnittstelle vorbereitet).
- Keine Bedienoberflaeche zum Einstellen des Alarms (nur Config-Datei).
- Keine automatische Standorterkennung.

## Stack

- C++17, Qt 6 (Core, Gui, Qml, Quick, Network)
- Logik in C++, Darstellung in QML (duenne Schicht)
- CMake, `CMakePresets.json`; unter Windows VS 2026 ueber das CMake-Projekt
- Konfigurationen: `debug`, `release`, `debug_level_log`, `release_level_log`
- Ausgabe: `bin/<system><arch>/<config>/`
- Standard: globaler C++ Coding Standard (`m_`-Praefix, PascalCase-Methoden, eigene
  `.h`/`.cpp` pro Klasse, `BuildAndRun.sh`, CTest)

## Ordnerstruktur

```
Lightswitch/
  src/core/        ClockModel, AlarmController, ILightController, DummyLightController
  src/weather/     WeatherService, WeatherModel
  src/calendar/    ICalendarProvider, DummyCalendarProvider, CalendarModel
  src/app/         main.cpp, AppController, Configuration
  qml/             Main.qml + Komponenten (Tile, DotMatrix, ...)
  resources/fonts/ Manufaktur-Bold.ttf, Manufaktur-Black.ttf
  config/          lightswitch.ini
  tests/           CTest
  docs/superpowers/specs/
  BuildAndRun.sh, CMakeLists.txt, CMakePresets.json, CLAUDE.md
```

## Design-Vorlage (aus der SVG)

- Canvas 720x720, Hintergrund schwarz.
- Kacheln: Fuellung `#241F23`, Rand `#2D2B2E`; Weather-Kachel Fuellung `#1F493B`,
  Rand `#3C4E50`.
- Akzent `#CA6F54` (aktive Punkte, ON, aktive Wochentage), Labels `#999999`,
  Werte `#EDEDED`, leere Punkte `#2D2B2E`.
- Fonts: Manufaktur Bold (Labels, 20.8 px) und Black (Werte, 41.7 px / 133 px).
- Layout: links oben Time (Uhrzeit gross), links Mitte Light (ON/OFF gross), rechts
  oben Weather, rechts Mitte Calendar, rechts unten Alarm. Genaue Koordinaten werden
  aus der SVG uebernommen.

## Komponenten und Verhalten

### Time - `ClockModel`
Tickt jede Sekunde. Liefert `HH:mm` (gross), das Datum in der Form `Thursday sep 17`
und `DD.MM.YYYY - HH:mm` fuer die Alarm-/Statuszeile. Die Uhr ist ueber eine
Schnittstelle injizierbar, damit Tests ohne Systemzeit laufen.

### Light - `ILightController`
Schnittstelle mit `IsOn()`, `SetOn(bool)`, `Toggle()` und Signal `StateChanged`.
`DummyLightController` haelt nur den Zustand. Ein Tipp auf die Kachel schaltet um.
ON: Schrift orange, OFF: grau.

### Weather - `WeatherService` und `WeatherModel`
- Quelle: Open-Meteo (ohne API-Key). Standort (Breite/Laenge) aus der Config.
- Abfrage alle 30 Minuten: aktuelle Temperatur (Grad Celsius) und stuendliche
  Niederschlagswahrscheinlichkeit fuer die naechsten 24 Stunden.
- `WeatherModel` bildet 12 Spalten a 2 Stunden. Jede Spalte nimmt das Maximum ihrer
  beiden Stunden. Punkte je Spalte: `ceil(percent / 20)`, also 0 % = 0 Punkte,
  100 % = 5 Punkte. Die Spalten wachsen von unten nach oben.
- Oben: Temperatur (z. B. `11 Grad`) und Regenwahrscheinlichkeit der aktuellen Stunde.
- Fehlerfall: bei Netzwerkfehlern bleiben die letzten Werte stehen, vor dem ersten
  Erfolg zeigt die Kachel `--`.

### Calendar - `ICalendarProvider`
`DummyCalendarProvider` erzeugt beim Start zufaellige Termine fuer heute (Titel plus
Uhrzeit). Die Schnittstelle ist so gehalten, dass ein spaeterer
`GoogleCalendarProvider` ohne Aenderung an UI und `CalendarModel` eingesetzt werden
kann.

### Alarm - `AlarmController`
Liest Uhrzeit und aktive Wochentage aus der Config. Zur eingestellten Minute an einem
aktiven Wochentag schaltet er das Licht ein (nur ON, nie OFF) und loest hoechstens
einmal pro Tag aus. Die Anzeige zeigt die Uhrzeit sowie M T W T F S S; aktive Tage
orange, inaktive grau. Keine Bedienung in der Oberflaeche.

### Anwendung - `AppController` und `Configuration`
`Configuration` liest `lightswitch.ini` (Standort, Alarmzeit, Wochentage).
`AppController` verbindet die Modelle mit QML. Fenster 720x720; `--fullscreen`
schaltet auf Vollbild fuer das Zieldisplay.

## Build und Betrieb

- `BuildAndRun.sh` nach globalem Standard (Profile, `--clean`, `--no-test`,
  `--no-run`, `-j`, `--`-Weitergabe).
- Windows: VS 2026 oeffnet das CMake-Projekt; Presets fuer die vier Konfigurationen.
- Logging-Level ist von der Build-Konfiguration getrennt ueber eine eigene
  Compile-Definition.
- Fonts werden als Qt-Ressource eingebunden (nur `.ttf`).

## Tests (CTest, ohne Netzwerk und ohne Systemzeit)

- Wetter: Prozent zu Punkten, 2-Stunden-Gruppierung mit Maximum, Randwerte 0/100 %.
- Alarm: aktiver Wochentag, exakte Minute, nur einmal pro Tag, kein Ausloesen an
  inaktiven Tagen.

## Annahmen

- Anzeigesprache Englisch (wie SVG).
- Standort nur ueber Config.
- Qt 6 ist auf Windows und Linux installiert.

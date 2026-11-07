# Mini World 3D (Nintendo 3DS)

Ein kleines, kostenloses Open-World-Erkundungsspiel für Nintendo 3DS.

## Aktueller Prototyp
- Top-down-Stadt mit Straßen, Häusern, Bäumen und Park
- Figur zu Fuß bewegen
- Mit A zwischen Fuß- und Auto-Modus wechseln
- Erste Aufgabe: den gelben Marker im Park erreichen
- Kamera folgt der Spielfigur

## Steuerung
- Steuerkreuz: bewegen
- A: Auto-Modus ein-/ausschalten
- START: Spiel beenden

## Build
Benötigt devkitPro mit devkitARM und libctru:
`make`

Der GitHub-Actions-Workflow baut eine 3DSX-Datei und eine installierbare CIA.

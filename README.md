# Meteora Drum

Batteria acustica VST3 per Windows di **Meteora Plugins**, basata sui campioni one-shot registrati dall'utente.

## Build

Il progetto si compila da solo con GitHub Actions (solo Windows) a ogni push sul ramo `main` (o `master`).
In **Actions → ultima esecuzione → Artifacts** si trova `MeteoraDrum-Windows`: `Meteora Drum.vst3` più la versione Standalone.

Installazione: copiare `Meteora Drum.vst3` in `C:\Program Files\Common Files\VST3`.

## Schermate

In alto: **KIT**, **MIXER** e **GROOVES** per passare da una schermata all'altra, logo (clic = opzioni) e **KIT PRESET**.

### KIT (schermata principale)

- Clic sui pezzi della foto per suonarli: Ctrl/Cmd+clic = colpo leggero, Shift+clic sull'hi-hat = aperto, Alt+clic sul rullante = rimshot.
- **Microfono dell'hi-hat** (a matita, cardioide): si trascina solo dentro la zona tratteggiata sopra il piatto,
  tra bordo e campana e da 3 a 15 cm di altezza. Verso la campana più corpo, verso il bordo più sfrigolio;
  vicino più bassi (prossimità), lontano più aria e più rullante che rientra.
- **Overheads**: distanza tra i microfoni da 40 a 180 cm e tipo di microfono:
  - CONDENSATORE a diaframma largo, cardioide: -3 dB a 40 Hz, presenza +2,5 dB a 9 kHz, -4 dB a 16 kHz;
  - NASTRO, figura 8: +3 dB sotto i 150 Hz, alti che scendono dolcemente, più stanza perché capta anche dietro.
  La foto cambia i microfoni con una dissolvenza. La distanza cambia stereo, tempi di arrivo e comportamento fuori asse.
- **Set suoni A/B** per kick, snare, tom, floor, crash 1, crash 2 e hi-hat (A = originale, B = nuovo).
  I due set sono allineati in loudness percepita (LUFS), quindi cambiando set il volume non salta.

### MIXER

**Kick, Snare, Tom, Floor, Hi-Hat, Ride**: Tune, Decay, Comp, Pan, Mute, Solo, fader, VU e LED.
Kick, Snare, Tom, Floor hanno le mandate **OH** e **ROOM**; Hi-Hat e Ride la mandata **ROOM**.
**BLEED** su Kick (rullante nel microfono della cassa) e su Snare (cassa nel microfono del rullante).

**Overheads**: Width, Lo Cut, Air. **Room**: Size, Decay, Comp, Lo Cut e **TIPO**: STUDIO (la stanza della foto) o BIG ROOM (sala grande e viva).
**Master**: Bass e Treble (attivi con EQ ON), Width, Limit, Mute e volume generale.

I crash non hanno un canale: si sentono solo da overheads e room.

### GROOVES

116 pattern MIDI rock umanizzati (Rock e Rock Half Time: strofe, pre-ritornelli, ritornelli e fill).
PLAY li suona con il kit; con **SYNC HOST** seguono il tempo del progetto e, a DAW in riproduzione, partono
dall'inizio della battuta successiva. LOOP ripete il pattern. Il riquadro bpm (trascina in su/giù, doppio clic = tempo consigliato)
vale quando SYNC HOST è spento. **UMANIZZA** (0–150%) regola tempo e dinamica umani: 0% a griglia, 100% come suonato, 150% più sciolto; vale per l'ascolto e per il MIDI trascinato. Trascinando una riga, l'icona MIDI o la barra in basso su una traccia della DAW si ottiene il file MIDI.

## Fase

I campioni hanno la polarità allineata (kick, tom e floor spingono tutti nello stesso verso in entrambi i set).
Con **Allineamento di fase** (menu del logo, attivo di default) ogni microfono close viene ritardato quanto basta
per arrivare insieme agli overhead: la quantità per ogni pezzo è stata scelta misurando le cancellazioni tra 50 e 400 Hz.
Le prime riflessioni della room non portano bassi, che arrivano solo dalla coda diffusa.

## Room

Riverbero algoritmico a FDN (8 linee di ritardo con matrice di Householder, smorzamento delle alte, predelay legato a SIZE)
più le **prime riflessioni** calcolate con il metodo delle sorgenti immagine sulla stanza della foto
(circa 5 × 4 × 3 m, pareti in legno e pannelli fonoassorbenti, due microfoni room davanti al kit).
Dopo il riverbero: Lo Cut, ammorbidimento delle alte e compressore dedicato.

## Mappa MIDI

| Nota | Suono |
|---|---|
| 36 / 35 | Kick |
| 38 | Snare · 40 Rimshot · 37 Side stick |
| 48 / 50 / 45 / 47 | Tom |
| 43 / 41 | Floor tom |
| 42 / 46 / 44 | Hi-hat chiuso / aperto / pedale |
| 22 – 28 | Hi-hat per apertura: 22 chiuso stretto, 23 chiuso, 24 chiuso morbido, 25 semichiuso, 26 semiaperto, 27 aperto, 28 tutto aperto (valgono per entrambi i set) |
| 21 | Foot splash |
| 49 / 52 | Crash 1 |
| 57 / 55 | Crash 2 |
| 51 / 53 / 59 | Ride |

**CC4** pedale hi-hat (0 = aperto, 127 = chiuso). **Aftertouch polifonico** sui piatti = choke.

## Kit preset

Meteora Default (caricato all'apertura), Studio Naturale, Warm Indie, Vintage Tape '72, Big Rock Room, Dry Punch, Garage,
Half Time Heavy, Ballad. Ogni preset imposta set A/B, microfoni overhead, microfono dell'hi-hat e mix.

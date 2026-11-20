# gett1 Dance

gett1 Dance è un plug-in per DAW che fa ballare una mascotte a tempo con il progetto. Il segnale audio attraversa il plug-in senza elaborazione. Puoi scegliere tra dodici coreografie, regolare velocità e dimensione e mostrare il gattino in una finestra separata dall’editor.

| Piattaforma | Versione | Formati |
|---|---|---|
| Windows x64 | 0.4.1 | VST3 |
| macOS Intel | 0.4.4 | Audio Unit, VST3 |
| macOS Apple Silicon | 0.4.4 | Audio Unit, VST3 |

- [Manuale utente](docs/Manuale_Utente_Gett1Dance.pdf)
- Guide d’installazione per [Windows x64](docs/Installazione_Gett1Dance_Windows_x64.pdf), [Mac Intel](docs/Installazione_Gett1Dance_macOS_Intel.pdf) e [Mac Apple Silicon](docs/Installazione_Gett1Dance_macOS_AppleSilicon.pdf)

## Compilazione

Servono CMake 3.22 o successivo, un compilatore C++17 e [JUCE 8.0.12](https://github.com/juce-framework/JUCE/tree/8.0.12). Clona JUCE a parte.

```bash
git clone --branch 8.0.12 --depth 1 https://github.com/juce-framework/JUCE.git dependencies/JUCE
```

### Windows x64

Il setup richiede Visual Studio 2026 con il carico di lavoro C++ e Inno Setup 6.3 o successivo. Da Prompt dei comandi, nella cartella del progetto:

```bat
Build_Installer_Windows.cmd dependencies\JUCE
```

Il file `.exe` viene generato in `dist`. Se WinRAR è installato, lo script prova anche a creare un archivio con i manuali.

### macOS

Con Xcode, CMake e gli strumenti da riga di comando di Xcode, esegui lo script per la tua architettura:

```bash
bash Build_macOS-AppleSilicon.sh dependencies/JUCE
bash Build_macOS-Intel.sh dependencies/JUCE
```

Ogni script produce in `dist` un `.pkg` con AU e VST3. I pacchetti di questa configurazione sono privi di firma Developer ID e notarizzazione.

## Sviluppo e test

- `Source/Motion.h` contiene le coreografie e la sincronizzazione.
- `Source/Cat.h` disegna la mascotte.
- `Source/Plugin.cpp` collega trasporto, parametri e finestra della mascotte.
- `Anteprima.html` offre un’anteprima visuale utilizzabile nel browser.

I test di movimento e visibilità si compilano anche senza JUCE:

```bash
cmake -S . -B build-tests -DGETT1_MOTION_TEST_ONLY=ON
cmake --build build-tests --config Release
ctest --test-dir build-tests -C Release --output-on-failure
node Tests/PreviewTests.cjs
```

Per proporre una modifica, apri una issue o una pull request. Per un problema con il plug-in, indica sistema, architettura, DAW e formato AU o VST3.

## Licenza

Copyright © 2026 gett1. Il codice del progetto è distribuito con licenza [GNU AGPL v3](LICENSE). JUCE è una dipendenza separata con le [proprie condizioni di licenza](https://github.com/juce-framework/JUCE/blob/8.0.12/LICENSE.md). Il font Pixelify Sans è distribuito con la [SIL Open Font License](Assets/OFL.txt).

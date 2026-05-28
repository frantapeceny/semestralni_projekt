# Semestrální projekt - APO

Zařízení ESP32-C3 pro read a transmit/write NFC a radio signálů. Ovládá se třemi tlačítky, výstup je na OLED displeji.


## Kompilace, instalace a spuštění

Projekt používá [PlatformIO](https://platformio.org/). Je nutné mít nainstalované PlatformIO ideálně ve VS Code pro kompilaci.

```bash
# klonování repozitáře
git clone https://github.com/frantapeceny/semestralni_projekt.git
cd semestralni_projekt

# kompilace a upload na ESP (musí být připojené přes USB)
pio run -t upload

# otevření sériového monitoru
pio device monitor
```

Použité knihovny se stáhnou automaticky při prvním buildu.


## Architektura aplikace

```
main.cpp
 - Storage: ukládání a načítání signálů z flash paměti
 - NfcManager: čtení a zápis NFC (PN532 přes SPI)
 - RadioManager: příjem a vysílání radio signálů (CC1101)
 - DisplayManager: vykreslování menu na OLED displeji (SSD1306)
```

`main.cpp` si ukládá a ovládá status uživatelského prostředí (`BROWSING`, `SIGNAL_MENU`, `TYPE_SELECTOR`) a detekuje tlačítka. Každý signál je buď `NfcSignal` nebo `RadioSignal` a oba dědí ze společné třídy `Signal`. `Storage` je serializuje a ukládá do flash.

Schéma zapojení je v souboru `wiring scheme.png`.

Složky `test_nfc/` a `test_radio/` obsahují standalone testovací skripty použité při vývoji, nejsou součástí hlavního buildu.

Kód je strukturovaný a komentovaný tak, aby mluvil sám za sebe.

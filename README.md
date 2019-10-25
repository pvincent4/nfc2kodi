# nfc2kodi
Poc developpements in order to use kodi as smart media center

----ENV-----
sudo apt-get install git curl vim build-essential libsqlite3-dev libnfc-* libtool libcurl4-openssl-dev rcconf

----BUILD----
g++ -o nfc2http nfc2http.c -lnfc -lcurl -lsqlite3

---AUTORUN---
Create /etc/init.d/nfc2http + sudo rcconf

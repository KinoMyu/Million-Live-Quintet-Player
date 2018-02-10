# Million Live Quintet Player
Quintet player for MLTD using the BASS audio library. HCA decryption code uses a modified version of https://github.com/Nyagamon/HCADecoder. Inspired by https://www.youtube.com/watch?v=4eOz9x5wy3k.

# Features
  - Realtime unit editing
  - Solo/Unit mode
  - Export to WAV file
  - Song/Idol volume control
  - Playback seeking
  - Multithreaded CRIWARE HCA decode

# TODO
 - UTF-8 support
 - Round robin decode task scheduler

# How to build
  1. Download dependencies from http://www.un4seen.com/ (bass and bassmix).
  2. Place bass.h/lib and bassmix.h/lib in bass/.
  3. Open QuintetPlayer.pro in Qt Studio and build.

This program is technically cross-platform, but has only been tested on Windows, so YMMV.

# How to obtain resource files
Download release version of Quintet Player or rip .hca files directly from MLTD.

# License
MIT<br>
Feel free to use this software in any way.

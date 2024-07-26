# MM12MID
Mega Man 1 (Game Boy) to MIDI converter

This tool converts music (and sound effects) from Game Boy games using Hiroshi Wada's sound engine. This was most notably used in the first Mega Man game, but was also used in two other early first-party releases, SolarStriker and QIX.
It works with ROM images. To use it, you must specify the name of the ROM followed by the number of the bank containing the sound data (in hex). Since Mega Man has two sound banks, you must run the program twice with a different bank parameter (3 and B). However, in order to prevent files from being overwritten, the MIDI files from the previous bank must either be moved to a separate folder or renamed.

Examples:
* MC2MID "Megaman - Dr. Wily's Revenge (E) [!].gb" 3
* MC2MID "Megaman - Dr. Wily's Revenge (E) [!].gb" B
* MC2MID "SolarStriker (W) [!].gb" 2
* MC2MID "Qix (JU) [!].gb" 1

This tool was based on my own reverse-engineering, with almost no actual disassembly involved. The driver is rather "primitive" compared to the others that I have done so far, with two of these games not even having an actual pointer table, instead having to rely on external code to manually load each sequence's address.
Like most of my other programs, another converter, MM12TXT, is also included, which prints out information about the song data from each game. This is essentially a prototype of MM12TXT.

Supported games:
* Mega Man: Dr. Wily's Revenge
* QIX
* SolarStriker

## To do:
  * Panning support
  * GBS file support

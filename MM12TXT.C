/*Mega Man 1 (GB) (Hiroshi Wada) to MIDI converter*/
/*Also works for SolarStriker/QIX*/
/*By Will Trowbridge*/
/*Portions based on code by ValleyBell*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define bankSize 16384

FILE* rom, * txt;
long bank;
long offset;
long addTable;
long baseValue;
long tablePtrLoc;
long seqPtrList;
long seqPtrList2;
long seqData;
long songList;
long patData;
int i, j;
char outfile[1000000];
int songNum;
long songInfo[4];
long curSpeed;
long curPtr;
long nextPtr;
long endPtr;
long bankAmt;
long nextBase;
int addValues[7];
int gameNum = 0;
long ptrList[100];

/*Pointers for SolarStriker*/
const long SolarPtrs[14] = { 0x5541, 0x5603, 0x5D41, 0x5D8A, 0x5DDB, 0x5E5B, 0x5EB0, 0x5F23, 0x600E, 0x6210, 0x651D, 0x6B60, 0x6FCF, 0x70F8 };
/*Pointers for QIX*/
const long QixPtrs[28] = {0x1725, 0x17F2, 0x1847, 0x188F, 0x18D9, 0x18F9, 0x191F, 0x1945, 0x196B, 0x199F, 0x19BC, 0x19D9, 0x19FF, 0x1A99, 0x1ABB, 0x1B0C, 0x1B32, 0x1B54, 0x1BFF, 0x1D04, 0x1D57, 0x1FD5, 0x22FD, 0x244D, 0x254D, 0x2611, 0x26E6, 0x287F };

/*Function prototypes*/
unsigned short ReadLE16(unsigned char* Data);
static void Write8B(unsigned char* buffer, unsigned int value);
static void WriteBE32(unsigned char* buffer, unsigned long value);
static void WriteBE24(unsigned char* buffer, unsigned long value);
static void WriteBE16(unsigned char* buffer, unsigned int value);
void song2txt(int songNum, long ptr);

unsigned static char* romData;

/*Convert little-endian pointer to big-endian*/
unsigned short ReadLE16(unsigned char* Data)
{
	return (Data[0] << 0) | (Data[1] << 8);
}

static void Write8B(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = value;
}

static void WriteBE32(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF000000) >> 24;
	buffer[0x01] = (value & 0x00FF0000) >> 16;
	buffer[0x02] = (value & 0x0000FF00) >> 8;
	buffer[0x03] = (value & 0x000000FF) >> 0;

	return;
}

static void WriteBE24(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF0000) >> 16;
	buffer[0x01] = (value & 0x00FF00) >> 8;
	buffer[0x02] = (value & 0x0000FF) >> 0;

	return;
}

static void WriteBE16(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = (value & 0xFF00) >> 8;
	buffer[0x01] = (value & 0x00FF) >> 0;

	return;
}

int main(int args, char* argv[])
{
	printf("Mega Man 1 (GB) (Hiroshi Wada) to TXT converter\n");
	if (args != 3)
	{
		printf("Usage: MM12TXT <rom> <bank>\n");
		printf("Supported games: Mega Man: Dr. Wily's Revenge, SolarStriker, QIX\n");
		return -1;
	}
	else
	{
		if ((rom = fopen(argv[1], "rb")) == NULL)
		{
			printf("ERROR: Unable to open file %s!\n", argv[1]);
			exit(1);
		}
		else
		{
			bank = strtol(argv[2], NULL, 16);
			if (bank != 1)
			{
				bankAmt = bankSize;
			}
			else
			{
				bankAmt = 0;
			}

			fseek(rom, ((bank - 1) * bankSize), SEEK_SET);
			romData = (unsigned char*)malloc(bankSize);
			fread(romData, 1, bankSize, rom);
			fclose(rom);

			/*Look for the start of the pointer table*/
			if (romData[0x05] == 0x43 && romData[0x06] == 0xC3)
			{
				printf("Detected game: Mega Man 1\n");
				gameNum = 1;
			}
			else if (romData[0x703] == 0x01 && romData[0x704] == 0x23)
			{
				printf("Detected game: SolarStriker\n");
				gameNum = 2;
			}
			else if (romData[0x600] == 0x01 && romData[0x601] == 0x23)
			{
				printf("Detected game: QIX\n");
				gameNum = 3;
			}

			if (gameNum == 0)
			{
				printf("ERROR: Unsupported game data!\n");
				exit(1);
			}
			else if (gameNum == 1)
			{
				/*Get pointers from list*/
				i = 0x0015;
				songNum = 1;
				while (ReadLE16(&romData[i]) >= bankSize)
				{
					curPtr = ReadLE16(&romData[i]);
					ptrList[songNum - 1] = curPtr;
					printf("Song %i address: 0x%04X\n", songNum, curPtr);
					song2txt(songNum, curPtr);
					i += 2;
					songNum++;
				}
			}
			else if (gameNum == 2)
			{
				/*Get pointers from list*/
				songNum = 1;
				for (i = 0; i < 14; i++)
				{
					curPtr = SolarPtrs[i];
					printf("Song %i address: 0x%04X\n", songNum, curPtr);
					song2txt(songNum, curPtr);
					songNum++;
				}
			}
			else if (gameNum == 3)
			{
				/*Get pointers from list*/
				songNum = 1;
				for (i = 0; i < 28; i++)
				{
					curPtr = QixPtrs[i];
					printf("Song %i address: 0x%04X\n", songNum, curPtr);
					song2txt(songNum, curPtr);
					songNum++;
				}
			}
		}
		printf("The operation was successfully completed!\n");
	}
}

void song2txt(int songNum, long ptr)
{
	long curPos = 0;
	int index = 0;
	int curSeq = 0;
	int tempo = 0;
	int repeat1 = 0;
	int repeat2 = 0;
	int repeat3 = 0;
	int jumpAmt1 = 0;
	int jumpAmt2 = 0;
	int jumpAmt3 = 0;
	int curChan = 0;
	int curNote = 0;
	int curNoteLen = 0;
	int transpose = 0;
	long chanStart[4];
	long command[3];
	int endChan = 0;
	int endSong = 0;
	int i = 0;

	sprintf(outfile, "song%d.txt", songNum);
	if ((txt = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file song%d.txt!\n", songNum);
		exit(2);
	}
	else
	{
		curPos = ptr - bankAmt;

		tempo = romData[curPos];
		fprintf(txt, "Tempo: %i\n", tempo);
		curPos++;
		fprintf(txt, "Sequence type: %i\n", romData[curPos]);
		curPos++;
		for (i = 0; i < 4; i++)
		{
			chanStart[i] = ReadLE16(&romData[curPos]);
			fprintf(txt, "Channel %i start: 0x%04X\n", romData[curPos+2], chanStart[i]);
			curPos += 3;
		}

		fprintf(txt, "\n");
		for (curChan = 0; curChan < 4; curChan++)
		{
			endChan = 0;
			curPos = chanStart[curChan] - bankAmt;
			fprintf(txt, "Channel %i:\n", curChan + 1);
			if (curPos <= 0)
			{
				endChan = 1;
			}
			while (endChan == 0)
			{

				command[0] = romData[curPos];
				command[1] = romData[curPos + 1];
				command[2] = romData[curPos + 2];

				if ((curChan == 0 && curPos == (chanStart[1] - bankAmt)) || (curChan == 1 && curPos == (chanStart[2] - bankAmt)) || (curChan == 2 && curPos == (chanStart[3] - bankAmt)))
				{
					endChan = 1;
				}

				if (command[0] >= 0x90 && command[0] <= 0x9F)
				{
					repeat1 = command[0] - 0x90;
					jumpAmt1 = (signed char)command[1];
					if (repeat1 == 0)
					{
						fprintf(txt, "(9x) Jump to position %i and repeat infinite times\n", jumpAmt1);
					}
					else
					{
						fprintf(txt, "(9x) Jump to position %i and repeat %i times\n", jumpAmt1, repeat1);
					}
					curPos += 2;

				}
				else if (command[0] >= 0xA0 && command[0] <= 0xAF)
				{
					repeat2 = command[0] - 0xA0;
					jumpAmt2 = (signed char)command[1];
					if (repeat2 == 0)
					{
						fprintf(txt, "(Ax) Jump to position %i and repeat infinite times\n", jumpAmt2);
					}
					else
					{
						fprintf(txt, "(Ax) Jump to position %i and repeat %i times\n", jumpAmt2, repeat2);
					}
					curPos += 2;

				}
				else if (command[0] >= 0xB0 && command[0] <= 0xBF)
				{
					repeat3 = command[0] - 0xB0;
					jumpAmt3 = (signed char)command[1];
					if (repeat3 == 0)
					{
						fprintf(txt, "(Bx) Jump to position %i and repeat infinite times\n", jumpAmt3);
						/*Workaround*/
						if (romData[curPos + 2] == 0x02 && romData[curPos + 3] == 0x01)
						{
							endChan = 1;
						}
					}
					else
					{
						fprintf(txt, "(Bx) Jump to position %i and repeat %i times\n", jumpAmt3, repeat3);
					}
					curPos += 2;

				}
				else if (command[0] >= 0x80 && command[0] <= 0x8F)
				{
					fprintf(txt, "Comand %1X\n", command[0]);
					curPos += 2;
				}
				else if (command[0] == 0xE1)
				{
					fprintf(txt, "Command E1 : % i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xE6)
				{
					jumpAmt1 = (signed char)command[1];
					fprintf(txt, "Command E6: %i\n", jumpAmt1);
					curPos += 2;
				}
				else if (command[0] == 0xE7)
				{
					fprintf(txt, "Transpose + 1 octave: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xEB)
				{
					fprintf(txt, "Command EB: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xEC)
				{
					fprintf(txt, "Slow down channel: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xED)
				{
					transpose = (signed char)command[1];
					fprintf(txt, "Set channel transpose: %i\n", transpose);
					curPos += 2;
				}
				else if (command[0] == 0xEF)
				{
					fprintf(txt, "Command EF: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xF0)
				{
					fprintf(txt, "Set panning: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xF1)
				{
					fprintf(txt, "Command F1: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xF2)
				{
					fprintf(txt, "Set waveform: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xF3)
				{
					fprintf(txt, "Command F3: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xF4)
				{
					fprintf(txt, "Set instrument duty: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xF5)
				{
					fprintf(txt, "Set instrument sweep/pitch change: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xF6)
				{
					fprintf(txt, "Command F6: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xF7)
				{
					fprintf(txt, "Command F7: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xF8)
				{
					fprintf(txt, "Music/SFX identifier?: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xF9)
				{
					fprintf(txt, "Command F9: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xFA)
				{
					fprintf(txt, "Command FA: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xFB)
				{
					fprintf(txt, "Set note length: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xFC)
				{
					fprintf(txt, "Set envelope length?: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xFD)
				{
					fprintf(txt, "Set echo: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xFE)
				{
					fprintf(txt, "Set velocity: %i\n", command[1]);
					curPos += 2;
				}
				else if (command[0] == 0xFF)
				{
					fprintf(txt, "End of channel\n");
					endChan = 1;
				}
				else if (command[0] > 0x00 && command[0] < 0x80)
				{
					curNote = command[0];
					curNoteLen = command[1];
					fprintf(txt, "Note: %01X, length: %i\n", command[0], command[1]);
					curPos += 2;
				}
				else if (command[0] == 0x00)
				{
					curNoteLen = command[1];
					fprintf(txt, "Rest: %i\n", command[1]);
					curPos += 2;
				}
			}
			fprintf(txt, "\n");
		}

		fclose(txt);
	}
}
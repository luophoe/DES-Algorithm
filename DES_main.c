/*
 * AES_main.c
 *
 * Created on: 02/01/2021
 *     Author: Anyka
 *      	   Phoebe Luo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*------------------------------------------------------------------------------------------------
--------------------------------------- Function Declaration -------------------------------------
--------------------------------------------------------------------------------------------------*/
int main();

void DES(int text[8], int key[8]);
void IniPerm(int bintext[64], int bintext_l[32], int bintext_r[32]);
void KeyPerm(int binkey[64], int roundkey_complete[16][48]);
void ExpPerm(int bintext_r[32], int roundtext[6][8]);
void AddRoundKey(int roundkey_complete[16][48], int roundtext[6][8], int choice, int round);
void SBox(int roundtext[6][8], int roundSBox[32]);
void PBox(int roundSBox[32]);
void FinalPerm(int bintext_l[32], int bintext_r[32], int bintext[64]);

void GetArray(int text[8], int choice);
void HextoBin(int hex[8], int bin[64]);
void BintoHex(int bin[64], int hex[8]);
void PrintBin(int bin[64], int column, int row);

/*------------------------------------------------------------------------------------------------
--------------------------------------- Function Definition --------------------------------------
--------------------------------------------------------------------------------------------------*/

int main(){
	// initialize matrices
	int text[8] = {0}; //initialize array to hold text
	int key[8] = {0x13, 0x34, 0x57, 0x79, 0x9b, 0xbc, 0xdf, 0xf1}; // initializing key

	DES(text, key);

	return 0;
}

/*--------------------------------------------------------------------------------------
---------------------------------- Algorithm Functions ---------------------------------
----------------------------------------------------------------------------------------*/

/* DES Function Calling Subfunctions */
void DES(int text[8], int key[8]){
	// get user input to identify operation, 1. Encryption, 2. Decryption
	int choice = 0;
	printf("Choose designated function:\n1. Encryption\n2. Decryption\n");
	fflush(stdout);
	scanf("%d", &choice);


	GetArray(text, choice); // get user input for text

	// convert hexadecimal to binary
	int bintext[64] = {0};
	HextoBin(text, bintext);
	if(choice == 1){
		printf("Plaintext: \n");
	}else if(choice == 2){
		printf("Ciphertext: \n");
	}
	PrintBin(bintext, 8, 8);
	int binkey[64] = {0};
	HextoBin(key, binkey);

	//get roundkey 1-16 stored in column 0 to 15
	int roundkey_complete[16][48]; // roundkey_complete matrix to hold roundkey 0-10
	KeyPerm(binkey, roundkey_complete); // fill in roundkey_complete matrix

	// round 0
	int round = 0;
	int bintext_l[32] = {0};
	int bintext_r[32] = {0};
	// use array for bitwise operation
	IniPerm(bintext, bintext_l, bintext_r); // initial permutation for round 0, input 64 bits array and updates 2 32 bits array
	printf("Left bintext %d: \n", round);
	PrintBin(bintext_l, 4, 8);
	printf("Right bintext %d: \n", round);
	PrintBin(bintext_r, 4, 8);

	// round 1-16
	int prevbintext_l[32] = {0};
	int prevbintext_r[32] = {0};
	for(round = 1; round < 17; round++){
		int count;
		for(count = 0; count < 32; count++){
			prevbintext_l[count] = bintext_l[count];
			prevbintext_r[count] = bintext_r[count];
		} // initialize prevbintext_r and prevbintext_r with previous round of bintext_l and bintext_r

		int roundtext[6][8] = {0}; // roundtext to hold expanded bintext after ExpPerm
		// use matrix for better manipulation to the position
		int roundSBox[32] = {0}; // roundSbox to hold truncated roundtext after SBox
		// use array for bitwise operation

		// start manipulation to prevbintext_r
		ExpPerm(prevbintext_r, roundtext); // expansion permutation for round 1-15, input 32 bits array and updates 6 by 8 matrix
		AddRoundKey(roundkey_complete, roundtext, choice, round); // XOR roundkey with roundtext, input 6 by 8 matrix and updates 6 by 8 matrix
		SBox(roundtext, roundSBox); // roundtext substitution using 8 SBoxes, input 6 by 8 matrix and updates 32 bits array
		PBox(roundSBox); // roundSBox permutation using PBox, updates 32 bits array

		if(round != 16){
			// switch bintext_l and bintext_r for round 1-15
			for(count = 0; count < 32; count++){
				bintext_l[count] = prevbintext_r[count];
				bintext_r[count] = prevbintext_l[count] ^ roundSBox[count];
			}
			printf("Left bintext %d: \n", round);
			PrintBin(bintext_l, 4, 8);
			printf("Right bintext %d: \n", round);
			PrintBin(bintext_r, 4, 8);
		}else{
			// maintain bintext_l and bintext_r for round 16
			for(count = 0; count < 32; count++){
				bintext_r[count] = prevbintext_r[count];
				bintext_l[count] = prevbintext_l[count] ^ roundSBox[count];
			}
			printf("Left bintext %d: \n", round);
			PrintBin(bintext_l, 4, 8);
			printf("Right bintext %d: \n", round);
			PrintBin(bintext_r, 4, 8);
			FinalPerm(bintext_l, bintext_r, bintext); // combine left and right bintext, permutation using Final, input 2 32 bits array and updates 64 bits array

			BintoHex(bintext, text); // convert binary to hex

			if(choice == 1){
				printf("Ciphertext: \n");
			}else if(choice == 2){
				printf("Plaintext: \n");
			}
			for(count = 0; count < 8; count++){
				printf("0x%02x ", text[count]);
			}
		}
	}
}

/* Initial Permutation Before Rounds */
void IniPerm(int bintext[64], int bintext_l[32], int bintext_r[32]){
	// constant IP array
	const int IP[64] =
	{
		58, 50, 42, 34, 26, 18, 10, 2,
		60, 52, 44, 36, 28, 20, 12, 4,
		62, 54, 46, 38, 30, 22, 14, 6,
		64, 56, 48, 40, 32, 24, 16, 8,
		57, 49, 41, 33, 25, 17,  9, 1,
		59, 51, 43, 35, 27, 19, 11, 3,
		61, 53, 45, 37, 29, 21, 13, 5,
		63, 55, 47, 39, 31, 23, 15, 7
	};

	// copy bintext for retrieving data
	int temptext[64] = {0};
	int position;
	for(position = 0; position < 64; position++){
		temptext[position] = bintext[position];
	}

	// start permutation
	for(position = 0; position < 64; position++){
		int IP_position = 0; // IP_position indicates the position in the array, initialized with 0
		int IP_num = IP[IP_position]; // IP_num indicates the value of the position, initialized with the value of position 0
		while((IP_num-1) != position){
			IP_position++;
			IP_num = IP[IP_position];
		}
		bintext[IP_position] = temptext[position];
	}

	// split bintext into two parts to update bintext_l and bintext_r
	for(position = 0; position < 32; position ++){
		bintext_l[position] = bintext[position]; // contains bintext[0-31]
		bintext_r[position] = bintext[position + 32]; // contains bintext[32-63]
	}
}

/* Key Permutation to Get all Round Keys */
void KeyPerm(int binkey[64], int roundkey_complete[16][48]){
	const int PC1[56] = {
		57, 49, 41, 33, 25, 17,  9,
		 1, 58, 50, 42, 34, 26, 18,
		10,  2, 59, 51, 43, 35, 27,
		19, 11,  3, 60, 52, 44, 36,
		63, 55, 47, 39, 31, 23, 15,
		 7, 62, 54, 46, 38, 30, 22,
		14,  6, 61, 53, 45, 37, 29,
		21, 13,  5, 28, 20, 12,  4 };
	const int PC2[48] = {
		14, 17, 11, 24,  1,  5,
		 3, 28, 15,  6, 21, 10,
		23, 19, 12,  4, 26,  8,
		16,  7, 27, 20, 13,  2,
		41, 52, 31, 37, 47, 55,
		30, 40, 51, 45, 33, 48,
		44, 49, 39, 56, 34, 53,
		46, 42, 50, 36, 29, 32 };
	const int roundconst[16] = {
		1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1 };

	// 1. truncate 64 bits binkey to 56 bits newbinkey using CP1
	int newbinkey[56] = {0};
	int position, count;
	for(position = 0; position < 64; position++){
		int PC1_position = 80; // PC1_position indicates the new position in the array, initialized with random number for checking
		for(count = 0; count < 56; count++){
			if((PC1[count]-1) == position){
				PC1_position = count;
			}
		}
		if(PC1_position != 80){
			newbinkey[PC1_position] = binkey[position]; // update newbinkey if position is not the the ones ignored and PC1_poisition is actually found in PC1
		}
	}

	// 2. split newbinkey to two parts and left shift according to round constant
	int round;
	for(round = 0; round < 16; round++){
		if(roundconst[round] == 1){
			int temp1 = newbinkey[0];
			int temp2 = newbinkey[28];
			for(position = 0; position <= 26; position++){
				newbinkey[position] = newbinkey[position + 1]; // shift newbinkey[1-27] to [0-26]
				newbinkey[position + 28] = newbinkey[position + 29]; // shift newbinnkey[29-55] to [28-54]
			}
			newbinkey[27] = temp1;
			newbinkey[55] = temp2;
		}else if(roundconst[round] == 2){
			int temp1 = newbinkey[0];
			int temp2 = newbinkey[1];
			int temp3 = newbinkey[28];
			int temp4 = newbinkey[29];
			for(position = 0; position <= 25; position++){
				newbinkey[position] = newbinkey[position + 2]; // shift newbinkey[2-27] to [0-25]
				newbinkey[position + 28] = newbinkey[position + 30]; // shift newbinnkey[30-55] to [28-53]
			}
			newbinkey[26] = temp1;
			newbinkey[27] = temp2;
			newbinkey[54] = temp3;
			newbinkey[55] = temp4;
		}

		// 3. truncate 56 bits newbinkey to 48 bits and store in roundkey_complete using CP2
		for(position = 0; position < 56; position++){
			int PC2_position = 80; // PC2_position indicates the new position in the array, initialized with random number for checking
			for(count = 0; count < 48; count++){
				if((PC2[count]-1) == position){
					PC2_position = count;
				}
			}
			if(PC2_position != 80){
				roundkey_complete[round][PC2_position] = newbinkey[position]; // update roundkey_complete if position is not the the ones ignored and PC2_poisition is actually found in PC2
			}
		}
	}

	// print roundkey
	printf("roundkey: \n");
	int row, column;
	for(row = 0; row < 48; row++){
		for(column = 0; column < 15; column++){
			printf("%d ", roundkey_complete[column][row]);
		}
		printf("\n");
	}
	printf("\n");
}

/* Exapnsion Permutation to Enlarge Size of Right Bintext */
void ExpPerm(int bintext_r[32], int roundtext[6][8]){
	/*const int E[48] = {
		32,  |   1,  2,  3,  4,  |   5,
		 4,  |   5,  6,  7,  8,  |   9,
		 8,  |   9, 10, 11, 12,  |  13,
		12,  |  13, 14, 15, 16,  |  17,
		16,  |  17, 18, 19, 20,  |  21,
		20,  |  21, 22, 23, 24,  |  25,
		24,  |  25, 26, 27, 28,  |  29,
		28,  |  29, 30, 31, 32,  |   1 };*/

	// 1. copy bintext_r into bintext[1-4][0-7]
	int column, row;
	int count = 0;
	for(row = 0; row < 8; row++){
		for(column = 1; column < 5; column++){
			roundtext[column][row] = bintext_r[count];
			count++;
		}
	}

	// 2. up shift column 1 to fill in column 5 and down shift column 4 to fill in column 0
	for(row = 0; row < 7; row++){
		roundtext[5][row] = roundtext[1][row+1];
		roundtext[0][row+1] = roundtext[4][row];
	}
	roundtext[5][7] = roundtext[1][0];
	roundtext[0][0] = roundtext[4][7];
}

/* Matrix XOR Roundkey */
void AddRoundKey(int roundkey_complete[16][48], int roundtext[6][8], int choice, int round){
	int roundkey[6][8] = {0}; // roundkey to hold key for current round
	int column, row;
	int count = 0;

	if(choice == 1){ // encryption, get roundkey from left to right
		// get roundkey from roundkey_complete using column number round - 1
		for(row = 0; row < 8; row++){
			for(column = 0; column < 6; column++){
				roundkey[column][row] = roundkey_complete[round - 1][count];
				count++;
			}
		}
	}else if(choice == 2){ // decryption, get roundkey from right to left
		// get roundkey from roundkey_complete using column number 16 - round
		for(row = 0; row < 8; row++){
			for(column = 0; column < 6; column++){
				roundkey[column][row] = roundkey_complete[16 - round][count];
				count++;
			}
		}
	}


	printf("Roundkey %d: \n", round);
		for(row = 0; row < 8; row++){
			for(column = 0; column < 6; column++){
				printf("%d ", roundkey[column][row]);
			}
			printf("\n");
		}
		printf("\n");

	// update roundtext with roundtext XOR roundkey
	for(column = 0; column < 6; column++){
		for(row = 0; row < 8; row++){
			roundtext[column][row] = roundtext[column][row] ^ roundkey[column][row];
		}
	}
}

/* SBox for Substitution */
void SBox(int roundtext[6][8], int roundSBox[32]){
	const int SBox[8][4][16] = {
		// S1
		14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
		 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
		 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
		15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,
		// S2
		15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
		 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
		 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
		13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,
		// S3
		10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
		13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
		13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
		 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,
		// S4
		 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
		13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
		10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
		 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,
		// S5
		 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
		14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
		 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
		11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,
		// S6
		12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
		10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
		 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
		 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,
		// S7
		 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
		13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
		 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
		 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,
		// S8
		13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
		 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
		 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
		 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
	};

	int row;
	int SBox_column, SBox_row;
	for(row = 0; row < 8; row++){
		SBox_column = roundtext[1][row]*8 + roundtext[2][row]*4 + roundtext[3][row]*2 + roundtext[4][row]; // getting the column by combining the 1th, 2th, 3th, and 4th bits
		SBox_row = roundtext[0][row]*2 + roundtext[5][row]; // getting the row by combining the 0th and 5th bits
		int num = SBox[row][SBox_row][SBox_column]; // num is the number extracted from SBox
		int count = 3; // count to keep track of the current column, initialized with 3
		while (num != 0){
			roundSBox[row*4 + count] = num%2; // input bit by bit into roundSBox array
			num = num/2;
			count--;
		}
	}
}

/* PBox for Permutation */
void PBox(int roundSBox[32]){
	const int PBox[32] = {
		16,  7, 20, 21,
		29, 12, 28, 17,
		 1, 15, 23, 26,
		 5, 18, 31, 10,
		 2,  8, 24, 14,
		32, 27,  3,  9,
		19, 13, 30,  6,
		22, 11,  4,  25 };

	int temproundSBox[32] = {0}; // initialize temproundSBox to keep track of the original roundSBox
	int position, new_position;
	for(position = 0; position < 32; position++){
		temproundSBox[position] = roundSBox[position];
	}

	for(position = 0; position < 32; position++){
		for(new_position = 0; new_position < 32; new_position++){
			if(PBox[new_position] - 1 == position){
				roundSBox[new_position] = temproundSBox[position];
			}
		}
	}
}

/* Final Permutation for Round 16 */
void FinalPerm(int bintext_l[32], int bintext_r[32], int bintext[64]){
	int Final[64] = {
		40, 8, 48, 16, 56, 24, 64, 32,
		39, 7, 47, 15, 55, 23, 63, 31,
		38, 6, 46, 14, 54, 22, 62, 30,
		37, 5, 45, 13, 53, 21, 61, 29,
		36, 4, 44, 12, 52, 20, 60, 28,
		35, 3, 43, 11, 51, 19, 59, 27,
		34, 2, 42, 10, 50, 18, 58, 26,
		33, 1, 41,  9, 49, 17, 57, 25 };

	int count, new_count;
	for(count = 0; count < 32; count++){
		bintext[count] = bintext_l[count];
		bintext[count + 32] = bintext_r[count];
	} // combine left and right

	int tempbintext[64] = {0};
	for(count = 0; count < 64; count++){
		tempbintext[count] = bintext[count];
	} // initialize tempbintext with bintext

	for(count = 0; count < 64; count++){
		for(new_count = 0; new_count < 64; new_count++){
			if(Final[new_count] - 1 == count){
				bintext[new_count] = tempbintext[count];
			}
		}
	}
}

/*--------------------------------------------------------------------------------------
---------------------------------- Operational Functions --------------------------------
----------------------------------------------------------------------------------------*/

/* Get Array from User Input */
void GetArray(int text[8], int choice){
	// get user input for plaintext OR ciphertext
	if (choice == 1){
		printf("Input plaintext:\n");
	}else if (choice == 2){
		printf("Input ciphertext:\n");
	}

	//putting plaintext OR ciphertext into text array
	int count;
	for(count = 0; count < 8; count++){
		fflush(stdout);
		scanf("%x", text+count);
	}
}

/* Convert Hexadecimal to Binary Number */
void HextoBin(int hex[8], int bin[64]){
	int round, current;
	int position = 64;
	for(round = 0; round < 8; round++){
		int num = hex[round]; // get the current hexnumber, from text[0] to text[7]
		for(current = 0; current < 8; current++){
			int current_bin = num & 1; // get the rightmost binary digit, from digit 64 to digit 1
			bin[round*8 + (7 - current)] = current_bin;
			num = num >> 1;
			position--;
		}
	}
}

/* Convert Binary to Hexadecimal Number */
void BintoHex(int bin[64], int hex[8]){
	int a;
	int count = 0;
	int total = 0;
	for(a = 0; a < 64; a = a+8){
		total = bin[a+7] + bin[a+6]*2 + bin[a+5]*4 + bin[a+4]*8 + bin[a+3]*16 + bin[a+2]*32 + bin[a+1]*64 + bin[a]*128;
		hex[count] = total;
		total = 0;
		count++;
	}
}

/* Print Array in Binary Number */
void PrintBin(int bin[64], int column, int row){
	int count = 0;
	int a, b;
	for(a = 0; a < row; a++){
		for(b = 0; b < column; b++){
			printf("%d ", bin[count]);
			count++;
		}
		printf("\n");
	}
	printf("\n");
}

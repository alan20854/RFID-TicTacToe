#include <fsl_device_registers.h>
#include "utils.h"

#define EASY_AI 0
#define MEDIUM_AI 1
#define HARD_AI 2

// RFID Tag Database
char rfidCodes[NUM_RFID_TAGS][10] = {"25005F7625", "2800A06A96", "28009FF7E2", "0100039936", "0100473191"};
char *rfidData[NUM_RFID_TAGS] = {"Easy AI", "Medium AI", "Hard AI", "Alex", "Alan"};

// Game state variables
char board[3][3] = {{'1', '2', '3'},
										{'4', '5', '6'},
										{'7', '8', '9'}};
int gameRunning = 0;
int player1 = -1;
int player2 = -1;
int whoseTurn = 1;
int totalMoves = 0;

/* Checks board and returns 1 if player 1 won and 2 if player 2 won, 0 otherwise */
int checkWin() {
	if (board[0][0] == board[0][1] && board[0][1] == board[0][2]) {
		return board[0][0] == 'X' ? 1 : 2;
	} else if (board[1][0] == board[1][1] && board[1][1] == board[1][2]) {
		return board[1][0] == 'X' ? 1 : 2;
	} else if (board[2][0] == board[2][1] && board[2][1] == board[2][2]) {
		return board[2][0] == 'X' ? 1 : 2;
	} else if (board[0][0] == board[1][0] && board[1][0] == board[2][0]) {
		return board[0][0] == 'X' ? 1 : 2;
	} else if (board[0][1] == board[1][1] && board[1][1] == board[2][1]) {
		return board[0][1] == 'X' ? 1 : 2;
	} else if (board[0][2] == board[1][2] && board[1][2] == board[2][2]) {
		return board[0][2] == 'X' ? 1 : 2;
	} else if (board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
		return board[0][0] == 'X' ? 1 : 2;
	} else if (board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
		return board[0][2] == 'X' ? 1 : 2;
	}
	
	return 0;
}

/* Checks if the move (1-9) is valid - spot is available on the board */
int validMove(int move) {
	if (move <= 0) return 0;
	return (board[(move-1)/3][(move-1)%3] != 'X' && board[(move-1)/3][(move-1)%3] != 'O') ? 1 : 0;
}

/* Outputs the board state over serial */
void showBoard(void) {
	serialWrite("-------\r\n");
	for (int i = 0; i < 3; i++) {
		putChar_UART0('|');
		for (int j = 0; j < 3; j++) {
			putChar_UART0(board[i][j]);
			putChar_UART0('|');
		}
		serialWrite("\r\n-------\r\n");
	}
	serialWrite("\r\n");
}

/* Returns a random move from 1-9; the validity of the move is not checked here */
int easyAIMove(void) {
	return randomNumber()%9+1;
}

/* Code that sees if the AI can win or prevent a win by making a move
   If it cannot, then -1 is returned */
int winOrBlock(void) {
	//row 1 about to win
	if(board[0][0] == board[0][1] && validMove(3)) {
			return 3;
	} else if(board[0][0] == board[0][2] && validMove(2)) {
			return 2;
	} else if(board[0][1] == board[0][2] && validMove(1)) {
			return 1;
	}
	//row 2 about to win
	if(board[1][0] == board[1][1] && validMove(6)) {
			return 6;
	} else if(board[1][0] == board[1][2] && validMove(5)) {
			return 5;
	} else if(board[1][1] == board[1][2] && validMove(4)) {
			return 4;
	}
	//row 3 about to win
	if(board[2][0] == board[2][1] && validMove(9)) {
			return 9;
	} else if(board[2][0] == board[2][2] && validMove(8)) {
			return 8;
	} else if(board[2][1] == board[2][2] && validMove(7)) {
			return 7;
	}
	//col 1 about to win
	if(board[0][0] == board[1][0] && validMove(7)) {
			return 7;
	} else if(board[0][0] == board[2][0] && validMove(4)) {
			return 4;
	} else if(board[1][0] == board[2][0] && validMove(1)) {
			return 1;
	}
	//col 2 about to win
	if(board[0][1] == board[1][1] && validMove(8)) {
			return 8;
	} else if(board[0][1] == board[2][1] && validMove(5)) {
			return 5;
	} else if(board[1][1] == board[2][1] && validMove(2)) {
			return 2;
	}
	//col 3 about to win
	if(board[0][2] == board[1][2] && validMove(9)) {
			return 9;
	} else if(board[0][2] == board[2][2] && validMove(6)) {
			return 6;
	} else if(board[1][2] == board[2][2] && validMove(3)) {
			return 3;
	}
	//diag left to right 
	if(board[0][0] == board[1][1] && validMove(9)) {
			return 9;
	} else if(board[0][0] == board[2][2] && validMove(5)) {
			return 5;
	} else if(board[1][1] == board[2][2] && validMove(1)) {
			return 1;
	}
	//diag right to left
	if(board[0][2] == board[1][1] && validMove(7)) {
			return 7;
	} else if(board[0][2] == board[2][0] && validMove(5)) {
			return 5;
	} else if(board[1][1] == board[2][0] && validMove(3)) {
			return 3;
	}
	
	return -1;
}

/* AI tries to win or block, but if it can't, then return a random move */
int mediumAIMove(void) {
	int move = winOrBlock();
	return move != -1 ? move : easyAIMove();
}

/* Returns a side spot (random) if such a move can be made, -1 otherwise) */
int chooseSide() {
	if (!validMove(2) && !validMove(4) && !validMove(6) && !validMove(8)) return -1;
	int rand = randomNumber();
	while (!validMove((rand % 4 +1) * 2)) {
		rand = randomNumber();
	}
	return (rand % 4 +1) * 2;
}

/* Returns a corner spot (random) if such a move can be made, -1 otherwise) */
int chooseCorner() {
	if (!validMove(1) && !validMove(3) && !validMove(7) && !validMove(9)) return -1;
	int rand = randomNumber();
	int move = -1;
	while (!validMove(move)) {
		rand = randomNumber();
		switch (rand % 4 +1) {
			case 1: move = 1;
				break;
			case 2: move = 3;
				break;
			case 3: move = 7;
				break;
			case 4: move = 9;
				break;
		}
	}
	return move;
}

/* AI will try to win or block, take the center spot, or check other conditions to try and win or tie */
int hardAIMove(void) {
	int move = winOrBlock();
	if (move != -1) return move;
	if (validMove(5)) return 5;
	if (totalMoves == 3 && (board[0][2] == board[2][0] || board[0][0] == board[2][2])) {
		return chooseSide();
	}
	if (board[1][2] == board[2][0]) {
		if (validMove(8)) return 8;
		else if (validMove(9)) return 9;
	}
	if (board[0][0] == board[1][2]) {
		if (validMove(2)) return 2;
		else if (validMove(3)) return 3;
	}
	if (board[1][0] == board[2][2]) {
		if (validMove(7)) return 7;
		else if (validMove(8)) return 8;
	}
	if (board[0][2] == board[1][0]) {
		if (validMove(1)) return 1;
		else if (validMove(2)) return 2;
	}
	
	if (board[1][0] == board[2][1]) {
		if (validMove(7)) return 7;
	}
	if (board[1][2] == board[2][1]) {
		if (validMove(9)) return 9;
	}
	if (board[0][1] == board[1][0]) {
		if (validMove(1)) return 1;
	}
	if (board[0][1] == board[1][2]) {
		if (validMove(3)) return 3;
	}
	
	move = chooseCorner();
	if (move != -1) return move;
	
	move = chooseSide();
	if (move != -1) return move;
	
	return easyAIMove();
}

/* Play tic-tac-toe.  Sets up GPIO, reads tags to determine players, and then game starts.  Keeps running forever. */
int main(void) {
	GPIO_Initialize();
	
	while(1) {
		serialWrite("Welcome to Tic-Tac-Toe.\r\nPlease place Player 1's RFID tag to the reader, followed by Player 2's.\r\nThen, make your moves by using the keypad.\r\n");
		while (!gameRunning) {
			char tag[10] = {0};
			getRFIDTag(tag);
			if (player1 == -1) {
				player1 = getRFIDTagIndex(tag);
				if (player1 != -1) {
					serialWrite("Player 1: ");
					serialWrite(rfidData[player1]);
					serialWrite(" (X)\r\n");
				}
			} else { // player2
				player2 = getRFIDTagIndex(tag);
				if (player2 != -1 && player2 != player1) {
					serialWrite("Player 2: ");
					serialWrite(rfidData[player2]);
					serialWrite(" (O)\r\n");
					gameRunning = 1;
				}
			}
		}
		
		while (gameRunning) {
			showBoard();
			if (whoseTurn == 1) {
				serialWrite(rfidData[player1]);
				serialWrite("'s turn (X)\r\n");
			} else {
				serialWrite(rfidData[player2]);
				serialWrite("'s turn (O)\r\n");
			}
			int move = -1;
			while (!validMove(move)) { // Wait until valid move is chosen
				if((whoseTurn == 1 && player1 > HARD_AI) || (whoseTurn == 2 && player2 > HARD_AI)) {
					move = readKeypad();
				} else if ((whoseTurn == 1 && player1 == EASY_AI) || (whoseTurn == 2 && player2 == EASY_AI)){
					move = easyAIMove();
				} else if ((whoseTurn == 1 && player1 == MEDIUM_AI) || (whoseTurn == 2 && player2 == MEDIUM_AI)){
					move = mediumAIMove();
				} else {
					move = hardAIMove();
				}
			}
			if (whoseTurn == 1) {
				serialWrite(rfidData[player1]);
			} else {
				serialWrite(rfidData[player2]);
			}
			serialWrite(" moved to spot ");
			putChar_UART0(move+0x30); // ascii conversion
			serialWrite("\r\n");
			board[(move-1)/3][(move-1)%3] = whoseTurn == 1 ? 'X' : 'O';
			whoseTurn = whoseTurn == 1 ? 2 : 1; // Change turns
			totalMoves++;
			
			if (checkWin() != 0) {
				gameRunning = 0;
				showBoard();
				if (checkWin() == 1) {
					serialWrite(rfidData[player1]);
					serialWrite(" won! (X)\r\n");
				} else {
					serialWrite(rfidData[player2]);
					serialWrite(" won! (O)\r\n");
				}
				
			} else if (totalMoves == 9) {
				gameRunning = 0;
				showBoard();
				serialWrite("It's a draw.\r\n");
			}
		}
		
		// Reset the game state
		totalMoves = 0;
		board[0][0] = '1';
		board[0][1] = '2';
		board[0][2] = '3';
		board[1][0] = '4';
		board[1][1] = '5';
		board[1][2] = '6';
		board[2][0] = '7';
		board[2][1] = '8';
		board[2][2] = '9';
		player1 = player2 = -1;
		whoseTurn = 1;
	}
	
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Constants for board dimensions
#define BOARD_SIZE 8

/* Enumeration for piece colors */
typedef enum {
    WHITE,
    BLACK
} Color;

/* Structure to represent a chess board */
typedef struct
{
    char board[64];        // 8x8 chess board represented as a linear array
    char currentColor;     // Current turn: 'w' for WHITE, 'b' for BLACK
    char castling[5];      // Castling availability, e.g., "KQkq"
    char enPassant[2];      // Square of the possible en passant target (file, rank), e.g., {2, 5}
    int halfMoveClock;     // Half-move clock for the 50-move rule
    int fullMoveNumber;    // Full-move number
} Chessboard;

// Function to check if a square is empty
// bool is_square_empty(const ChessBoard* board, int rank, int file) {
//     return (board->data[rank][file] == '0');
// }

// // Function to check if a piece belongs to the opponent
// bool is_opponent_piece(char currentColor, char opponentColor) {
//     return (currentColor != opponentColor);
// }

// // Function to print the chessboard
// void print_chessboard(const ChessBoard* board) {
//     for (int i = 0; i < BOARD_SIZE; ++i) {
//         for (int j = 0; j < BOARD_SIZE; ++j) {
//             printf("%c ", board->data[i][j]);
//         }
//         printf("\n");
//     }
// }

// char* chessboard_to_fen(const ChessBoard* board) {
//     char* fen = (char*)malloc(sizeof(char) * 100);  // Assuming a reasonable size
//     int index = 0;

//     for (int rank = 7; rank >= 0; --rank) {
//         int emptyCount = 0;

//         for (int file = 0; file < 8; ++file) {
//             char piece = board->data[rank][file];

//             if (piece == '0') {
//                 emptyCount++;
//             } else {
//                 if (emptyCount > 0) {
//                     fen[index++] = emptyCount + '0';
//                     emptyCount = 0;
//                 }

//                 fen[index++] = piece;
//             }
//         }

//         if (emptyCount > 0) {
//             fen[index++] = emptyCount + '0';
//         }

//         if (rank > 0) {
//             fen[index++] = '/';
//         }
//     }

//     fen[index++] = ' ';
//     fen[index++] = (board->turn == WHITE) ? 'w' : 'b';
//     fen[index++] = ' ';

//     // Castling availability
//     if (strcmp(board->castling, "KQkq") == 0) {
//         fen[index++] = '-';
//     } else {
//         fen[index++] = (strchr(board->castling, 'K') == NULL) ? '-' : 'K';
//         fen[index++] = (strchr(board->castling, 'Q') == NULL) ? '-' : 'Q';
//         fen[index++] = (strchr(board->castling, 'k') == NULL) ? '-' : 'k';
//         fen[index++] = (strchr(board->castling, 'q') == NULL) ? '-' : 'q';
//     }

//     fen[index++] = ' ';

//     // En passant target square
//     if (board->enPassant[0] == -1) {
//         fen[index++] = '-';
//     } else {
//         char file = board->enPassant[0] + 'a';
//         char rank = (board->turn == WHITE) ? (board->enPassant[1] + '1') : (board->enPassant[1] + '5');
//         fen[index++] = file;
//         fen[index++] = rank;
//     }

//     fen[index++] = ' ';
//     sprintf(&fen[index], "%d %d", board->halfMoveClock, board->fullMoveNumber);

//     return fen;
// }

// void fen_to_chessboard(const char* fen, ChessBoard* board) {
//     // Initialize the board
//     memset(board->data, '0', sizeof(board->data));
//     board->turn = WHITE;
//     strcpy(board->castling, "KQkq");
//     board->enPassant[0] = -1;
//     board->enPassant[1] = -1;
//     board->halfMoveClock = 0;
//     board->fullMoveNumber = 1;

//     // Parse FEN notation
//     int index = 0;  // Position in the FEN string

//     // Piece placement
//     for (int rank = 7; rank >= 0; --rank) {
//         for (int file = 0; file < 8; ++file) {
//             char c = fen[index++];
//             if (c >= '1' && c <= '8') {
//                 file += (c - '0') - 1;
//             } else {
//                 board->data[rank][file] = c;
//             }
//         }
//         ++index;  // Skip '/'
//     }

//     // Active color
//     board->turn = (fen[index++] == 'w') ? WHITE : BLACK;
//     ++index;  // Skip ' '

//     // Castling availability
//     strcpy(board->castling, "-");
//     if (fen[index] != '-') {
//         board->castling[0] = fen[index++];
//         board->castling[1] = fen[index++];
//         board->castling[2] = fen[index++];
//         board->castling[3] = fen[index++];
//     }
//     ++index;  // Skip ' '

//     // En passant target square
//     if (fen[index] != '-') {
//         board->enPassant[0] = fen[index++] - 'a';
//         board->enPassant[1] = (board->turn == WHITE) ? (fen[index++] - '1') : (fen[index++] - '5');
//     }
//     ++index;  // Skip ' '

//     // Halfmove clock
//     sscanf(&fen[index], "%d", &board->halfMoveClock);
//     while (fen[index] != ' ') {
//         ++index;
//     }
//     ++index;  // Skip ' '

//     // Fullmove number
//     sscanf(&fen[index], "%d", &board->fullMoveNumber);
// }

// /* Function to initialize a chessboard with standard starting positions */
void fill_board_from_fen(const char *fen, Chessboard* board) {
    int index = 0;

    // Parse the piece positions
    while (*fen && *fen != ' ') {
        if (*fen == '/') {
            fen++;
        } else if (*fen >= '1' && *fen <= '8') {
            // Empty squares
            int count = *fen - '0';
            for (int i = 0; i < count; i++) {
                board->board[index] = '0';
                index++;
            }
            fen++;
        } else {
            // Piece
            board->board[index] = *fen;
            index++;
            fen++;
        }
    }
}

void init_chessboard(Chessboard* board) {
     // Set the default positions
    char initialPosition[8][8] = {
         
        {'R', 'N', '0', 'Q', 'K', 'B', 'N', 'R'},
        {'P', 'P', 'P', '0', 'P', 'P', 'P', 'P'},
        {'0', '0', '0', '0', 'B', '0', '0', '0'},
        {'0', '0', '0', 'P', '0', '0', '0', '0'},
         {'0', '0', '0', 'p', '0', '0', '0', '0'},
        {'0', '0', '0', '0', '0', '0', '0', '0'},
         {'p', 'p', 'p', '0', 'p', 'p', 'p', 'p'},
         {'r', 'n', 'b', 'k', 'q', 'b', 'n', 'r'}
     };

   // Copy the initial positions to the board
    memcpy(board->board, initialPosition, sizeof(initialPosition));

     // Set other default values
    board->currentColor = 'w';
    strcpy(board->castling, "-");
    board->enPassant[0] = '-';
    //board->enPassant[1] = '6';
    //board->enPassant[1] = -1;
    board->halfMoveClock = 0;
    board->fullMoveNumber = 1;
 }

// /* Function to display chessboard.data */
// // void display_chessboard(const ChessBoard* board) {
// //     for (int rank = 7; rank >= 0; --rank) {
// //         for (int file = 0; file < 8; ++file) {
// //             printf("%c ", board->data[rank * 8 + file]);
// //         }
// //         printf("\n");
// //     }
// // }

// void calculate_pawn_moves(const ChessBoard* board, int rank, int file, char color) {
//     // Assuming white pawns move upward and black pawns move downward
//     int direction = (color == 'w') ? -1 : 1;

//     // Move one square forward
//     if (is_square_empty(board, rank + direction, file)) {
//         printf("Pawn can move to %c%d\n", 'a' + file, rank + direction + 1);
//     }

//     // Move two squares forward from the starting position
//     if (((color == 'w' && rank == 6) || (color == 'b' && rank == 1)) &&
//         is_square_empty(board, rank + 2 * direction, file)) {
//         printf("Pawn can move to %c%d\n", 'a' + file, rank + 2 * direction + 1);
//     }

//     if(board->data[rank + direction][file - 1]>= 'A' && board->data[rank + direction][file - 1] <= 'Z'){

//     }
//     // Capture diagonally left
//     if (file > 0 && is_opponent_piece(board->data[rank + direction][file - 1].Color, color)) {
//         printf("Pawn can capture to %c%d\n", 'a' + file - 1, rank + direction + 1);
//     }

//     // Capture diagonally right
//     if (file < BOARD_SIZE - 1 && is_opponent_piece(board[rank + direction][file + 1].color, color)) {
//         printf("Pawn can capture to %c%d\n", 'a' + file + 1, rank + direction + 1);
//     }
// }

// // Function to calculate possible moves for a knight
// void calculate_knight_moves(const ChessBoard* board, int rank, int file, char color) {
//     // Possible knight moves
//     int moves[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};

//     for (int i = 0; i < 8; ++i) {
//         int newRank = rank + moves[i][0];
//         int newFile = file + moves[i][1];

//         if (newRank >= 0 && newRank < BOARD_SIZE && newFile >= 0 && newFile < BOARD_SIZE &&
//             !is_opponent_piece(board[newRank][newFile].color, color)) {
//             printf("Knight can move to %c%d\n", 'a' + newFile, newRank + 1);
//         }
//     }
// }

// // Function to calculate possible moves for a bishop
// void calculate_bishop_moves(const ChessBoard* board, int rank, int file, char color) {
//     // Diagonal moves
//     int moves[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

//     for (int i = 0; i < 4; ++i) {
//         for (int distance = 1; distance < BOARD_SIZE; ++distance) {
//             int newRank = rank + distance * moves[i][0];
//             int newFile = file + distance * moves[i][1];

//             if (newRank >= 0 && newRank < BOARD_SIZE && newFile >= 0 && newFile < BOARD_SIZE) {
//                 if (is_square_empty(board, newRank, newFile)) {
//                     printf("Bishop can move to %c%d\n", 'a' + newFile, newRank + 1);
//                 } else if (is_opponent_piece(board[newRank][newFile].color, color)) {
//                     printf("Bishop can capture to %c%d\n", 'a' + newFile, newRank + 1);
//                     break;
//                 } else {
//                     break; // Stop searching in this direction if the square is not empty
//                 }
//             } else {
//                 break; // Stop searching in this direction if out of bounds
//             }
//         }
//     }
// }

// // Function to calculate possible moves for a rook
// void calculate_rook_moves(const ChessBoard* board, int rank, int file, char color) {
//     // Vertical and horizontal moves
//     int moves[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

//     for (int i = 0; i < 4; ++i) {
//         for (int distance = 1; distance < BOARD_SIZE; ++distance) {
//             int newRank = rank + distance * moves[i][0];
//             int newFile = file + distance * moves[i][1];

//             if (newRank >= 0 && newRank < BOARD_SIZE && newFile >= 0 && newFile < BOARD_SIZE) {
//                 if (is_square_empty(board, newRank, newFile)) {
//                     printf("Rook can move to %c%d\n", 'a' + newFile, newRank + 1);
//                 } else if (is_opponent_piece(board[newRank][newFile].color, color)) {
//                     printf("Rook can capture to %c%d\n", 'a' + newFile, newRank + 1);
//                     break;
//                 } else {
//                     break; // Stop searching in this direction if the square is not empty
//                 }
//             } else {
//                 break; // Stop searching in this direction if out of bounds
//             }
//         }
//     }
// }

// // Function to calculate possible moves for a queen
// void calculate_queen_moves(const ChessBoard* board, int rank, int file, char color) {
//     // Combine bishop and rook moves
//     calculate_bishop_moves(board, rank, file, color);
//     calculate_rook_moves(board, rank, file, color);
// }

// // Function to calculate possible moves for a king
// void calculate_king_moves(const ChessBoard* board, int rank, int file, char color) {
//     // Possible king moves
//     int moves[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

//     for (int i = 0; i < 8; ++i) {
//         int newRank = rank + moves[i][0];
//         int newFile = file + moves[i][1];

//         if (newRank >= 0 && newRank < BOARD_SIZE && newFile >= 0 && newFile < BOARD_SIZE &&
//             !is_opponent_piece(board[newRank][newFile].color, color)) {
//             printf("King can move to %c%d\n", 'a' + newFile, newRank + 1);
//         }
//     }
// }

// /* Function to update chessboard based on SAN move */
// void update_board_from_san(ChessBoard* board, const char* san) {
//     // Assuming the SAN move is valid (no validation for simplicity)

//     // Parse the SAN move
//     char piece;
//     int targetFile, targetRank;

//     if (san[0] >= 'a' && san[0] <= 'h') {
//         // If the first character is a letter, it's a pawn move
//         piece = (board->turn == WHITE) ? 'P' : 'p';
//         sscanf(san, "%c%d", &piece, &targetRank);
//     } else {
//         // Otherwise, it's a piece move
//         sscanf(san, "%c%c%d", &piece, &piece, &targetRank);
//         // Convert the piece to uppercase for WHITE
//         if (board->turn == WHITE) {
//             piece -= 'a' - 'A';
//         }
//     }

//     // Convert the file letter to an index (e.g., 'a' -> 0, 'b' -> 1, ..., 'h' -> 7)
//     targetFile = san[1] - 'a';

//     // Update the board data
//     // Note: This is a simplified update; you may need a more complex logic
//     // Update the new position
//     board->data[targetRank * 8 + targetFile] = piece;

//     // Update the turn after each move
//     board->turn = (board->turn == WHITE) ? BLACK : WHITE;

//     // Update the FEN notation and print it
//     char* fen = chessboard_to_fen(board);
//     printf("New FEN Notation: %s\n", fen);
//     free(fen);
// }

static char* chessboard_to_str(const Chessboard* board){
    char* fen = (char*)malloc(sizeof(char) * 100);
    int index = 0;
    int emptyCount = 0;
    for(int i = 63; i >= 0; i--){
        if(board->board[i] != '0'){
            if(emptyCount > 0){
                fen[index++] = emptyCount + '0';
                emptyCount = 0;
            }
            fen[index++] = board->board[i];
            
        }else{
            emptyCount++;
        }
        //new file
        if(i % 8 == 0){
            if(emptyCount > 0)
                fen[index++] = emptyCount + '0';
            if(i > 0)
                fen[index++] = '/';
            emptyCount = 0;
        }

    }

    fen[index++] = ' ';
    fen[index++] = board->currentColor;
    fen[index++] = ' ';

    int i = 0;
    while(board->castling[i] != '\0'){
        fen[index++] = board->castling[i];
        i++;
    }

    fen[index++] = ' ';

    // En passant target square
    if (board->enPassant[0] == '-') {
        fen[index++] = '-';
    } else {
        fen[index++] = board->enPassant[0];
        fen[index++] = board->enPassant[1];
    }

    fen[index++] = ' ';
    sprintf(&fen[index], "%d %d", board->halfMoveClock, board->fullMoveNumber);

    return fen;
}

int main() {
    // Créez une structure ChessBoard et initialisez-la avec les positions initiales.
    //Chessboard board;
    //init_chessboard(&board);

    // Appelez la fonction chessboard_to_fen pour obtenir la notation FEN.
    //char fen[100];  // Assurez-vous que la taille est suffisante pour contenir la notation FEN.
    //strcpy(fen,chessboard_to_str(&board));

    // Affichez la notation FEN.
    //printf("FEN Notation: %s\n", fen);
    //print_chessboard(&board);
    // Test: Update board based on SAN move
    //update_board_from_san(&board, "e4");

    // const char* fen2 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    // fen_to_chessboard(fen, &board);
    // display_chessboard(&board);
    // printf("\n");
    // char* fen3 = "rnbqkbnr/ppp1pppp/8/3p4/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    // fen_to_chessboard(fen3, &board);
    // display_chessboard(&board);

    // printf("\n");
    // strcpy(fen3,chessboard_to_fen(&board));
    // // Affichez la notation FEN.
    // printf("FEN Notation: %s\n", fen3);
    const char *fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Chessboard chessboard;
    
    // Initialiser le tableau à des valeurs par défaut
    memset(chessboard.board, '0', sizeof(chessboard.board));

    fill_board_from_fen(fen, &chessboard);

    // Afficher le tableau résultant
    for (int i = 0; i < 64; i++) {
        printf("%c ", chessboard.board[i]);
        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }

    return 0;
}
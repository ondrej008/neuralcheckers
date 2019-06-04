#pragma once

#include <vector>
#include <cstdlib>
#include "network.h"

using namespace std;

void printBoard(int board[8][8])
{
    for(int x = 0; x < 8; x++)
    {
        for(int y = 0; y < 8; y++)
        {
            if((x + y) % 2 == 0)
                cout << (board[y][x] == 0 ? 'x' : board[y][x] == -3 ? 'W' : board[y][x] == -1 ? 'w' : board[y][x] == 1 ? 'b' : 'B');
            else
                cout << ' ';
        }
        
        cout << endl;
    }
}

vector<pair<int, int>> getPieces(int board[8][8], int target)
{
    vector<pair<int, int>> result;

    for(int x = 0; x < 8; x++)
    {
        for(int y = 0; y < 8; y++)
        {
            if(board[x][y] == target)
            {
                result.push_back(pair<int, int>(x, y));
            }
        }
    }
    
    return result;
}

char getLetter(int board[8][8], int x, int y)
{
    return (board[x][y] == -3 ? 'W' : board[x][y] == -1 ? 'w' : board[x][y] == 0 ? 'x' : board[x][y] == 1 ? 'b' : 'B');
}

bool pawnJump(int board[8][8], pair<int, int> from, pair<int, int> offset) // assumes we can move
{
    if(from.first + offset.first < 0 || from.first + offset.first > 7)
        return false;
    if(from.second + offset.second < 0 || from.second + offset.second > 7)
        return false;

    if(from.first + offset.first * 2 < 0 || from.first + offset.first * 2 > 7)
        return false;
    if(from.second + offset.second * 2 < 0 || from.second + offset.second * 2 > 7)
        return false;
    
    //cout << getLetter(board, from.first, from.second) << " on " << getLetter(board, from.first + offset.first, from.second + offset.second) << " aft " << getLetter(board, from.first + offset.first * 2, from.second + offset.second * 2) << endl;
    
    int us = board[from.first][from.second];
    int between = board[from.first + offset.first][from.second + offset.second];
    int result = board[from.first + offset.first * 2][from.second + offset.second * 2];
    if(between == 0 || (between < 0 && us < 0) || (between > 0 && us > 0))
    {
        //cout << "01" << between << " " << us << endl;
        return false;
    }

    if(result == 0)
        return true;
    //cout << "02\n";
    return false;
}

bool pawnMove(int board[8][8], pair<int, int> from, pair<int, int> offset)
{
    
    if(from.first + offset.first < 0 || from.first + offset.first > 7)
        return false;
    if(from.second + offset.second < 0 || from.second + offset.second > 7)
        return false;

    //cout << getLetter(board, from.first, from.second) << " on " << getLetter(board, from.first + offset.first, from.second + offset.second) << endl;


    int us = board[from.first][from.second];
    int result = board[from.first + offset.first][from.second + offset.second];
    if(result == 0)
        return true;
    return false;
}

int simulateGame(NN& A, NN& B, int debug, bool forceJump, bool forceMultiJump)
{
    // 8x8 board, 64 places, but only half of them are usable
    // A is white, B is black

    int board[8][8]; // 8x8 board, 64 positions, 0 is empty, - is white, + is black, 1 is pawn, 3 is king

    for(int x = 0; x < 8; x++)
    {
        for(int y = 0; y < 8; y++)
        {
            if((y == 0 || y == 2) && x % 2 == 0)
            {
                board[x][y] = -1;
            }
            else if((y == 1) && x % 2 == 1)
            {
                board[x][y] = -1;
            }
            else if((y == 6) && x % 2 == 0)
            {
                board[x][y] = 1;
            }
            else if((y == 7 || y == 5) && x % 2 == 1)
            {
                board[x][y] = 1;
            }
            else
            {
                board[x][y] = 0;
            }
        }
    }

    int moves = 0; int lastCaptured = 0;
    NN turn = A;
    int result = 0;
    while(true)
    {
        if(moves - lastCaptured > 40)
        {
            if(debug >= 2)
                cout << "Game over, no captures in the last 40 moves." << endl;
            break;
        }

        vector<pair<int, int>> whitePawns = getPieces(board, -1);
        vector<pair<int, int>> whiteKings = getPieces(board, -3);

        vector<pair<int, int>> blackPawns = getPieces(board, 1);
        vector<pair<int, int>> blackKings = getPieces(board, 3);

        if(blackPawns.size() < 1 && blackKings.size() < 1)
        {
            if(debug >= 2)
                cout << "Game over, black didn't have the things." << endl;
            result = -1;
            break;
        }

        if(whitePawns.size() < 1 && whiteKings.size() < 1)
        {
            if(debug >= 2)
                cout << "Game over, white didn't have the things." << endl;
            result = 1;
            break;
        }

        if(debug >= 3)
        {
            cout << "Current move: " << moves << endl;
            cout << "Turn is for " << (turn.name == A.name ? "white" : "black") << endl;
        }

        pair<int, int> bestMove; // what to move
        pair<int, int> posBestMove; // where to move
        double pointsBestMove = 0;
        vector<pair<pair<int, int>, pair<int, int>>> possibleMoves; // vector of what to move, and where to move it

        if(turn.name == A.name) // white moves
        {
            // find all possible moves for White

            for(int i = 0; i < whitePawns.size(); i++)
            {
                pair<int, int> pawn = whitePawns[i];

                if(pawn.second == 7)
                {
                    if(debug >= 3)
                        cout << "White pawn promoted to king " << pawn.first << " " << pawn.second << endl;
                    board[pawn.first][pawn.second] = -3;
                    continue;
                }

                // white moves down

                if(pawnMove(board, pawn, pair(-1, 1)) || pawnJump(board, pawn, pair(-1, 1)))
                {
                    possibleMoves.push_back(pair(pawn, pair(pawn.first + -1, pawn.second + 1)));
                    //cout << "good\n";
                }

                if(pawnMove(board, pawn, pair(1, 1)) || pawnJump(board, pawn, pair(1, 1)))
                {
                    possibleMoves.push_back(pair(pawn, pair(pawn.first + 1, pawn.second + 1)));
                    //cout << "gooder\n";
                }

            }

            // get best move or eliminate non-capturing moves, if forceJump is enabled

            for(int i = 0; i < possibleMoves.size(); i++)
            {
                int nextBoard[8][8];

                for(int x = 0; x < 8; x++)
                {
                    for(int y = 0; y < 8; y++)
                    {
                        nextBoard[x][y] = board[x][y];
                    }
                }

                // apply the moves to the board (pretty much just swapping)

                pair<int, int> moveFrom = possibleMoves[i].first;
                pair<int, int> moveTo = possibleMoves[i].second;

                int result = board[moveTo.first][moveTo.second];
                if(result == 0) // just moving, simplest
                {
                    nextBoard[moveTo.first][moveTo.second] = nextBoard[moveFrom.first][moveFrom.second];
                    nextBoard[moveFrom.first][moveFrom.second] = 0;
                }
                else if (result > 0) // black piece
                {
                    // trust that the space is clear
                    nextBoard[moveTo.first][moveTo.second] = 0;
                    //cout << "yeeted something idk\n";
                    nextBoard[moveFrom.first + (moveTo.first - moveFrom.first) * 2][moveFrom.second + (moveTo.second - moveFrom.second) * 2] = nextBoard[moveFrom.first][moveFrom.second];
                    nextBoard[moveFrom.first][moveFrom.second] = 0;
                }
                


                vector<double> input;

                for(int x = 0; x < 8; x++)
                {
                    for(int y = 0; y < 8; y++)
                    {
                        if((x + y) % 2 == 0)
                        {
                            //cout << (board[y][x] < 0 ? "W" : board[y][x] > 0 ? "B" : "X");
                            input.push_back(nextBoard[y][x]);
                            //cout << nextBoard[y][x];
                        }
                    }
                    //cout << endl;
                }

                Matrix<double> points = turn.computeOutput(input);

                if(points.get(0, 0) > pointsBestMove)
                {
                    if(debug >= 3)
                        cout << "Found new best move! Old was " << pointsBestMove << " new is " << points.get(0, 0) << endl;
                    pointsBestMove = points.get(0, 0);
                    bestMove = moveFrom;
                    posBestMove = moveTo;
                }
            }

            if(!possibleMoves.size())
            {
                if(debug >= 2)
                    cout << "WHITE ERROR: There are no possible moves!" << endl;
                result = 1;
                break;
            }

            // apply best move

            if(debug >= 3)
            {
                cout << bestMove.first << " " << bestMove.second << " : " << posBestMove.first << " " << posBestMove.second << endl;
            }

            if(board[posBestMove.first][posBestMove.second] == 0) // just moving, simplest
            {
                board[posBestMove.first][posBestMove.second] = board[bestMove.first][bestMove.second];
                board[bestMove.first][bestMove.second] = 0;
                //cout << "moving\n";
            }
            else if(board[posBestMove.first][posBestMove.second] > 0)
            {
                // trust that the space is clear
                board[posBestMove.first][posBestMove.second] = 0;
                //cout << "yeeted something idk\n";
                board[bestMove.first + (posBestMove.first - bestMove.first) * 2][bestMove.second + (posBestMove.second - bestMove.second) * 2] = board[bestMove.first][bestMove.second];
                board[bestMove.first][bestMove.second] = 0;
                lastCaptured = moves;
            }

            turn = B;
        }
        else // black moves
        {

            // find all possible moves for Black

            for(int i = 0; i < blackPawns.size(); i++)
            {
                pair<int, int> pawn = blackPawns[i];

                if(pawn.second == 0)
                {
                    if(debug >= 3)
                        cout << "Black pawn promoted to king " << pawn.first << " " << pawn.second << endl;
                    board[pawn.first][pawn.second] = 3;
                    continue;
                }

                // white moves down

                if(pawnMove(board, pawn, pair(-1, -1)) || pawnJump(board, pawn, pair(-1, -1)))
                {
                    possibleMoves.push_back(pair(pawn, pair(pawn.first + -1, pawn.second + -1)));
                    //cout << "bbbgood\n";
                }

                if(pawnMove(board, pawn, pair(1, -1)) || pawnJump(board, pawn, pair(1, -1)))
                {
                    possibleMoves.push_back(pair(pawn, pair(pawn.first + 1, pawn.second + -1)));
                    //cout << "bbbgooder\n";
                }

            }

            // get best move or eliminate non-capturing moves, if forceJump is enabled

            for(int i = 0; i < possibleMoves.size(); i++)
            {
                int nextBoard[8][8];

                for(int x = 0; x < 8; x++)
                {
                    for(int y = 0; y < 8; y++)
                    {
                        nextBoard[x][y] = board[x][y];
                    }
                }

                // apply the moves to the board (pretty much just swapping)

                pair<int, int> moveFrom = possibleMoves[i].first;
                pair<int, int> moveTo = possibleMoves[i].second;

                int result = board[moveTo.first][moveTo.second];
                if(result == 0) // just moving, simplest
                {
                    nextBoard[moveTo.first][moveTo.second] = nextBoard[moveFrom.first][moveFrom.second];
                    nextBoard[moveFrom.first][moveFrom.second] = 0;
                }
                else if (result < 0) // white piece
                {
                    // trust that the space is clear
                    nextBoard[moveTo.first][moveTo.second] = 0;
                    //cout << "yeeted something idk\n";
                    nextBoard[moveFrom.first + (moveTo.first - moveFrom.first) * 2][moveFrom.second + (moveTo.second - moveFrom.second) * 2] = nextBoard[moveFrom.first][moveFrom.second];
                    nextBoard[moveFrom.first][moveFrom.second] = 0;
                }
                


                vector<double> input;

                for(int x = 0; x < 8; x++)
                {
                    for(int y = 0; y < 8; y++)
                    {
                        if((x + y) % 2 == 0)
                        {
                            //cout << (board[y][x] < 0 ? "W" : board[y][x] > 0 ? "B" : "X");
                            input.push_back(nextBoard[y][x]);
                            //cout << nextBoard[y][x];
                        }
                    }
                    //cout << endl;
                }

                Matrix<double> points = turn.computeOutput(input);

                if(points.get(0, 0) > pointsBestMove)
                {
                    if(debug >= 3)
                        cout << "Found new best move! Old was " << pointsBestMove << " new is " << points.get(0, 0) << endl;
                    pointsBestMove = points.get(0, 0);
                    bestMove = moveFrom;
                    posBestMove = moveTo;
                }
            }

            if(!possibleMoves.size())
            {
                if(debug >= 2)
                    cout << "BLACK ERROR: There are no possible moves!" << endl;
                result = 1;
                break;
            }

            // apply best move

            if(debug >= 3)
            {
                cout << bestMove.first << " " << bestMove.second << " : " << posBestMove.first << " " << posBestMove.second << endl;
            }

            if(board[posBestMove.first][posBestMove.second] == 0) // just moving, simplest
            {
                board[posBestMove.first][posBestMove.second] = board[bestMove.first][bestMove.second];
                board[bestMove.first][bestMove.second] = 0;
                //cout << "moving\n";
            }
            else if(board[posBestMove.first][posBestMove.second] < 0)
            {
                // trust that the space is clear
                board[posBestMove.first][posBestMove.second] = 0;
                //cout << "yeeted something idk\n";
                board[bestMove.first + (posBestMove.first - bestMove.first) * 2][bestMove.second + (posBestMove.second - bestMove.second) * 2] = board[bestMove.first][bestMove.second];
                board[bestMove.first][bestMove.second] = 0;
                lastCaptured = moves;
            }

            turn = A;
        }

        if(debug >= 3)
            printBoard(board);

        moves++;
    }

    if(debug > 1)
        printBoard(board);

    return result;
}

void doCheckers(vector<NN>& nets, int debug, bool forceJump, bool forceMultiJump)
{
    int A, B;
    
    for(A = 0; A < nets.size(); A++)
    {
        for(B = 0; B < nets.size(); B++)
        {
            if(A == B)
            {
                continue;
            }

            if(debug == 2)
                cout << "In the corner, we have the famous " << nets[A].name << ", against the strong and mighty " << nets[B].name << "!\n";

            switch(simulateGame(nets[A], nets[B], debug, forceJump, forceMultiJump))
            {
                case 0: // draw
                    nets[A].fitness += 1;
                    nets[B].fitness += 1;
                    if(debug == 2)
                        cout << "The match was a draw! Was " << nets[A].name << " better than " << nets[B].name << "? No one knows!\n";
                    else if(debug == 1)
                        cout << nets[A].name << " draw " << nets[B].name << endl;
                    break;
                case 1: // B won
                    nets[B].fitness += 3;
                    if(debug == 2)
                        cout << "And the match is over! The winner is " << nets[B].name << ", completely humiliating their opponent, " << nets[A].name << endl;
                    else if(debug == 1)
                        cout << nets[B].name << " won " << nets[A].name << endl;
                    break;
                case -1: // A won
                    nets[A].fitness += 3;
                    if(debug == 2)
                        cout << "After this gruesome match, we know who won and who lost! The winner is " << nets[A].name << ", which completely destroyed their opponent, " << nets[B].name << endl;
                    else if(debug == 1)
                        cout << nets[A].name << " won " << nets[B].name << endl;
                    break;
                default:
                    cout << "Oopsies!" << endl;
            }
        }
    }
}
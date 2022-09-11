#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "api.h"

namespace solution {
  using namespace api;
  using namespace std;
  
  class Solution : public SolutionInterface {
    public:
      Solution(APICaller* api) {
        this->api = api;
        std::cout << "Press run code to see this in the console!\n";
        // You can initiate and calculate things here
      }

      /**
       * Return the player move as an integer representing the index in the
       * 64-long array covering the board. Index 0 is at the bottom left of the
       * board (a1), index 1 is b1, index 2 is c1, etc. On the next row up
       * index 8 is a2, index 9 is b2 and so forth, row by row up to index 63
       * (h8). The input parameter "color black" is true if you are controlling
       * the black stones (Player 1), and false for white. The parameter
       * "board" gives the current board state as a 64-long list of integers.
       * The indexing is described earlier in this comment. The integer values
       * of the list are 0 = empty, 1 = Stone of player one, 2 = Stone of
       * player two.
       */
       
       
      int getAdjIndex(int index, int dir) 
      {
        if (     dir == 0 && index/8 < 7) //up
          return index + 8;
        else if (dir == 1 && index/8 < 7 && index%8 < 7)//up_right
          return index + 8 + 1;
        else if (dir == 2                && index%8 < 7)//right
          return index + 1;
        else if (dir == 3 && index/8 > 0 && index%8 < 7)//bottom_right
          return index - 8 + 1;
        else if (dir == 4 && index/8 > 0)//bottom
          return index - 8;
        else if (dir == 5 && index/8 > 0 && index%8 > 0)//bottom_left
          return index - 8 - 1;
        else if (dir == 6 && index%8 > 0)//left
          return index - 1;
        else if (dir == 7 && index/8 < 7 && index%8 > 0)//up_left
          return index + 8 - 1;
        return -1;
      }
      
      int makeMove(bool colorBlack, vector<int> board) {
        // Write your code here
        //printf("-----------------------\n");
        int myColor = (colorBlack ? 1 : 2);
        float maxScore = 0.0f;
        int maxIndex = -1;
        for (int i = 0; i < 64; i++) 
        {
          if (board[i] != 0) continue; 
          int count = 0;
          for (int dir = 0; dir < 8; dir++)
          {
            int adjIndex = i;
            int dirCount = 0;
            while(true)
            {
              adjIndex = getAdjIndex(adjIndex, dir);
              //printf("adjIndex=%d\n", adjIndex);
              if (adjIndex == -1)
              {
                break;
              }
              else if (board[adjIndex] == 0)
              {
                break;
              }
              else if (board[adjIndex] == myColor)
              {
                if (dirCount > 0)
                {
                  count += dirCount;
                  //printf("dir=%d, dirCount=%d\n", dir, dirCount);
                }
                break;
              }
              dirCount++;
            }
          }
          //printf("i=%d, count=%d\n", i, count);
          if (count == 0)
            continue;
          //adjust score strategy
          float score = count;
          if (i == 0 || i == 7 || i == 56 || i == 63)
            score += 100.0f;//corner
          if (i/8 == 0 || i/8 == 7 || i%8 == 0 || i%8 == 7)
            score += 30.0f;//edge
          if (i == 1 || i == 9 || i == 8 || i == 6 || i == 14 || i == 15 ||
              i == 48 || i == 49 || i == 57 || i == 55 || i == 54 || i== 62)
            score -= 60.0f;
          if (i/8 == 1 || i/8 == 6 || i%8 == 1 || i%8 == 6)
            score -= 3.0f;//sub margin
          if (i/8 == 2 || i/8 == 5 || i%8 == 2 || i%8 == 5)
            score += 3.0f;//sub sub margin
          if (maxIndex == -1 || score >= maxScore)
          {
            maxScore = score;
            maxIndex = i;
          }
        }
        //printf("maxIndex=%d, maxScore=%f\n", maxIndex, maxScore);
        return maxIndex;
      }

    private:
      APICaller* api;
  };
}
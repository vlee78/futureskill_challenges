#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "api.h"

#define ASSERT(cond) while(!(cond)){printf("Assert failed at line: %d\n", __LINE__);exit(-1);break;}
#define EXIT(count) {static int num = 0; if(++num>=count){printf("Exit at line: %d\n", __LINE__); exit(-1);}}

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
      
      string CMYK = "CMYK";
      vector<string> DIRS = {"N","S","E","W","NW","NE","SE","SW"};
      
      int diffCount(char color, vector<string>& state)
      {
        int count[4] = {0, 0, 0, 0};
        for (string& line : state)
        {
          for (char ch : line)
          {
            for (int i = 0; i < 4; i++)
            {
              if (CMYK[i] == ch || CMYK[i] - ch == 'C' - 'c')
              {
                count[i]++;
                break;
              }
            }
          }
        }
        
        printf("CMYK = {%d, %d, %d, %d}\n", count[0], count[1], count[2], count[3]);
        int offset = 0;
        for (; offset < 4; offset++)
          if (CMYK[offset] == color)
            break;
        //printf("offset=%d\n", offset);
        int min_diff = 99999;  
        for (int i = 0; i < 4; i++)
        {
          //printf("count : %d, min_diff=%d\n", count[i], min_diff);
          if (i == offset) 
            continue;
          if (count[offset] - count[i] < min_diff)
            min_diff = count[offset] - count[i];
        }
        return min_diff;
      }
      
      int diffWalk(int x, int y, char color, const string& dir, vector<string>& state)
      {
        //EXIT(12);
        int nx = x;
        int ny = y;
        if (dir == "E") {nx += 2;}
        else if (dir == "W") {nx -= 2;}
        else if (dir == "N") {ny += 2;}
        else if (dir == "S") {ny -= 2;}
        else if (dir == "NE") {nx++; ny++;}
        else if (dir == "SE") {nx++; ny--;}
        else if (dir == "NW") {nx--; ny++;}
        else if (dir == "SW") {nx--; ny--;}
        int width = (int)state[0].size();
        int height = (int)state.size();
        if (nx < 0 || nx >= width || ny < 0 || ny >= height)
          return 0;//out of bound, no movement, no change of diff
        printf("[x=%d, y=%d] => [nx=%d, ny=%d], width=%d, height=%d\n", x, y, nx, ny, width, height);
        
        vector<string> temp = state;
        int diffPrev = diffCount(color, temp);
        dumpState(temp, diffPrev);
        for (int i = std::max(0,ny-1); i <= std::min(ny+1,height-1); i++)
        {
          for (int j = std::max(0,nx-1); j <= std::min(nx+1,width-1); j++)
          {
            char ch = temp[i][j];
            if (ch == '.'  || ch == color || ch == color - ('C' - 'c'))
              temp[i][j] = color - ('C' - 'c');
            else
            {
              bool found = false;
              for (int n = std::max(0,i-1); n <= std::min(i+1,height-1); n++)
              {
                for (int m = std::max(0,j-1); m <= std::min(j+1,width-1); m++)
                {
                  char nine = temp[n][m];
                  if (nine != color && nine != color - ('C' - 'c') && nine >= 'A' && nine <= 'Z')
                    found = true;
                }
              }
              if (found)
                temp[i][j] = '.';
              else
                temp[i][j] = color - ('C' - 'c');
            }
          }
        }
        int diffPost = diffCount(color, temp);
        dumpState(temp, diffPost);
        return diffPost - diffPrev;
      }
      
      void dumpState(vector<string>& state, int diff)
      {
        int width = (int)state[0].size();
        int height = (int)state.size();
        printf("====== width: %d, height: %d, diff: %d=======\n", width, height, diff);
        for (int i = (int)state.size()-1; i >= 0; i--)
          printf("line[%d] = '%s'\n", i, state[i].c_str());
        printf("=============================================\n");
      }
      
      string findNearWalkDir(int x, int y, char color, vector<string>& state)
      {
        printf("findNearWalkDir: x=%d, y=%d, color=%c\n", x, y, color);
        dumpState(state, 0);
        int width = (int)state[0].size();
        int height = (int)state.size();
        int minSteps = -1;
        int minX = 0;
        int minY = 0;
        for (int i = 0; i < height; i++)
        {
          for (int j = 0; j < width; j++)
          {
            if (j == x && i == y)
              continue;
            char ch = state[i][j];
            if (ch == color || color - ch == 'C' - 'c' || (ch >= 'A' && ch <= 'Z'))
              continue;
            int steps = (x - j < 0 ? j - x : x - j) + (y - i < 0 ? i - y : y - i);
            if (minSteps == -1 || steps < minSteps)
            {
              minSteps = steps;
              minX = j;
              minY = i;
            }
          }
        }
        int xsteps = minX - x;
        int ysteps = minY - y;
        printf("findNearWalkDir: minX=%d, minY=%d, xsteps=%d, ysteps=%d\n", minX, minY, xsteps, ysteps);
        string dir = "";
        if (xsteps >= 0 && ysteps >= 0)
        {
          if (xsteps > ysteps)
            dir = "E";
          else if (xsteps < ysteps)
            dir = "N";
          else
            dir = "NE";
        }
        else if (xsteps >= 0 && ysteps < 0)
        {
          if (xsteps > -ysteps)
            dir = "E";
          else if (xsteps < -ysteps)
            dir = "S";
          else
            dir = "SE";
        }
        else if (xsteps < 0 && ysteps >= 0)
        {
          if (-xsteps > ysteps)
            dir = "W";
          else if (-xsteps < ysteps)
            dir = "N";
          else
            dir = "NW";
        }
        else if (xsteps < 0 && ysteps < 0)
        {
          if (-xsteps > -ysteps)
            dir = "W";
          else if (-xsteps < -ysteps)
            dir = "S";
          else
            dir = "SW";
        }
        printf("findNearWalkDir: %s\n", dir.c_str());
        return dir;
      }
    
      /**
       * Execute a single move of the bot's programming. This function will be
       * called repeatedly until the bots run out of color. The arguments
       * received are your position x and y, your bots color, and the current
       * state of the game. The value returned will determine what action is
       * performed.
       */
      vector<string> update(int x, int y, string color, vector<string> gameState) {
        // Write your code here
        printf("################################################\n");
        printf("update: x=%d, y=%d, color=%s\n", x, y, color.c_str());
        //dumpState(gameState);
        int maxDiff = 0;
        string maxDir = "";
        for (string& dir : DIRS)
        {
          int diff = diffWalk(x, y, color[0], dir, gameState);
          printf("----- dir=%s, diff=%d -----\n", dir.c_str(), diff);
          if (maxDir == "" || diff > maxDiff)
          {
            maxDir = dir;
            maxDiff = diff;
          }
        }
        
        if (maxDiff == 0)
          maxDir = findNearWalkDir(x, y, color[0], gameState);
        
        vector<string> returnValue(2);
        returnValue[0] = "walk";
        returnValue[1] = maxDir;
        printf("return ['%s', '%s']\n", returnValue[0].c_str(), returnValue[1].c_str());
        return returnValue;
      }

    private:
      APICaller* api;
  };
}
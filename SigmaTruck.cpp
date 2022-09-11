#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>

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
      
      struct Point
      {
        int x;
        int y;
        int index;
        int congestion;
        vector<Point*> nexts;//connection[left,up,right,down]
        int nextIndex;
        int minCost;//cost from loc, steps + congestion 
        std::vector<Point*> minPath;
      };
  
      void dumpPath(std::vector<Point*>& stack, int accCost)
      {
        for (int i = 0; i < (int)stack.size(); i++)
        {
          if (i == 0) printf("cost=%d, stack(", accCost);
          Point* p = stack[i];
          printf("[%d,%d],", p->x, p->y);
          if (i == (int)stack.size()-1) printf(")\n");
        }
      }
      
      void dumpTargets()
      {
        for (auto& it : _pickups)
        {
          printf("pickup: [%d,%d] -> [%d, %d]\n", it.first->x, it.first->y, it.second->x, it.second->y);
          dumpPath(it.first->minPath, it.first->minCost);
          printf("------------------------\n");
        }
        for (auto& it : _unloads)
        {
          printf("unload: [%d,%d] -> [%d, %d]\n", it.first->x, it.first->y, it.second->x, it.second->y);
          dumpPath(it.second->minPath, it.second->minCost);
          printf("------------------------\n");
        }
        /*
        for (Point* point: _stations)
        {
          printf("station: [%d,%d]\n", point->x, point->y);
          dumpPath(point->minPath, point->minCost);
          printf("------------------------\n");
        }*/
        printf("==========================\n");
      }
      
      void updateMap(Point* loc)
      {
        for (Point* point : _map)
        {
          if (point == nullptr) 
            continue;
          point->nextIndex = -1;
          point->minCost = -1;
          point->minPath.clear();
        }
        int accCost = 0;
        std::vector<Point*> stack;
        loc->minCost = accCost;
        loc->minPath = stack;
        
        stack.push_back(loc);
        while(stack.size() > 0)
        {
          Point* point = stack.back();
          if (++point->nextIndex >= (int)point->nexts.size())
          {
            point->nextIndex = -1;
            stack.pop_back();
            accCost -= 1 + point->congestion;
          }
          else
          {
            Point* next = point->nexts[point->nextIndex];
            int nextCost = accCost + 1 + next->congestion;
            if (next->minCost == -1 || nextCost < next->minCost)
            {
              stack.push_back(next);
              accCost += 1 + next->congestion;
              next->minCost = accCost;
              next->minPath = stack;
            }
          }
        }
        ASSERT(accCost == -(1 + loc->congestion));
      }
      
      /**
       * Called once per simulation step. Gives you your truck's current
       * location and current battery charge.
       */
      void update(int locX, int locY, double charge) 
      {
        printf("update(locX=%d, locY=%d, charge=%f)\n", locX, locY, charge);
        
        Point* loc = getPoint(locX, locY);
        updateMap(loc);
        //dumpTargets();
        
        for (auto it = _pickups.begin(); it != _pickups.end();)
        {
          if (it->first == loc)
          {
            api->pickup();
            _unloads.push_back(*it);
            it = _pickups.erase(it);
            printf("pickup!!!\n");
          }
          else
            it++;
        }
        for (auto it = _unloads.begin(); it != _unloads.end(); )
        {
          if (it->second == loc)
          {
            api->dropoff();
            it = _unloads.erase(it);
            printf("dropoff!!!\n");
          }
          else
            it++;
        }
        Point* minStation = nullptr;
        int minStationCost = -1;
        for (auto it = _stations.begin(); it != _stations.end(); it++)
        {
          if (*it == loc && charge < 99.0f)
          {
            api->recharge();
            printf("recharge!!!\n");
          }
          
          if (minStationCost == -1 || (*it)->minCost < minStationCost)
          {
            minStationCost = (*it)->minCost;
            minStation = *it;
          }
        }
        
        int minCost = -1;
        Point* minTarget = nullptr;
        string minName = "";
        
        if (charge < 30.0f)
        {
          minCost = minStationCost;
          minTarget = minStation;
          minName = "station";
        }
        else
        {
          if (_unloads.size() <= 1)
          {
            for (auto& it : _pickups)
            {
              if (minCost == -1 || it.first->minCost - 1 - it.first->congestion < minCost)
              {
                minCost = it.first->minCost - 1 - it.first->congestion;
                minTarget = it.first;
                minName = "pickup";
              }
            }
          }
          for (auto it : _unloads)
          {
            if (minCost == -1 || it.second->minCost - 1 - it.second->congestion < minCost)
            {
              minCost = it.first->minCost - 1 - it.second->congestion;
              minTarget = it.second;
              minName = "dropoff";
            }
          }
        }
        
        if (minTarget != nullptr)
        {
          printf("target: [%d, %d] -> %s\n", minTarget->x, minTarget->y, minName.c_str());
          Point* next = minTarget->minPath[1];
          printf("next: [%d, %d]\n", next->x, next->y);
          api->setDestination(next->x, next->y);
        }
      }

      /**
       * Called whenever there are updates about any deliveries. You'll receive
       * the coordinates for pickup and delivery. The status may be 'new',
       * 'claimed' or 'completed'.
       */
      void deliveryUpdate(int pickupX, int pickupY, int dropoffX, int dropoffY, string status) {
        // Write your code here
        printf("deliveryUpdate(pickupX=%d, pickupY=%d, dropoffX=%d, dropoffY=%d, status=%s)\n", pickupX, pickupY, dropoffX, dropoffY, status.c_str());
        
        if (status == "new")
        {
          Point* pickup = getPoint(pickupX, pickupY);
          Point* dropoff = getPoint(dropoffX, dropoffY);
          _pickups.push_back(make_pair(pickup, dropoff));
        }
        else if (status == "claimed")
        {
          for (auto it = _pickups.begin(); it != _pickups.end(); it++)
          {
            Point* pickup = getPoint(pickupX, pickupY);
            Point* dropoff = getPoint(dropoffX, dropoffY);
            if (it->first == pickup && it->second == dropoff)
            {
              _pickups.erase(it);
              break;
            }
          }
        }
      }

      /**
       * Called at the start of the simulation to provide static data about the
       * map. Each list is a column of a table, where each row is a relevant
       * coordinate. Congestion gives the number of simulation steps a vehicle
       * has to wait at that coordinate due to traffic. Type may be 'road' or
       * 'station' where you may recharge your truck.
       */
      
      Point* getPoint(int x, int y)
      {
        if (x >= 0 && x < _map_x_size && y >= 0 && y < _map_y_size)
          return _map[y * _map_x_size + x];
        else
          return nullptr;
      }
      
      enum Move {IncY = 0, IncX = 1, DecX = 2, DecY = 3};

      Point* getMove(Point* point, Move move)
      {
        if (point == nullptr)
          return nullptr;
        else if (move == IncY && point->x % 2 == 1)
          return getPoint(point->x, point->y+1);
        else if (move == IncX && point->y % 2 == 0)
          return getPoint(point->x+1, point->y);
        else if (move == DecX && point->y % 2 == 1)
          return getPoint(point->x-1, point->y);
        else if (move == DecY && point->x % 2 == 0)
          return getPoint(point->x, point->y-1);
        else
          return nullptr;
      }
      
      void receiveMapData(vector<int> x, vector<int> y, vector<string> roadType, vector<int> congestion) {
        
        _map_x_size = *std::max_element(x.begin(), x.end()) + 1;
        _map_y_size = *std::max_element(y.begin(), y.end()) + 1;
        _map.resize(_map_x_size * _map_y_size, nullptr);
        for (int i = 0; i < (int)x.size(); i++)
        {
          Point* point = new Point();
          point->x = x[i];
          point->y = y[i];
          point->index = point->y * _map_x_size + point->x;
          point->congestion = congestion[i];
          point->nexts.clear();
          point->nextIndex = -1;
          point->minCost = -1;
          point->minPath.clear();
          _map[point->index] = point;
          if (roadType[i] == "station")
            _stations.push_back(point);
        }
        for (Point* loc : _map)
        {
          if (loc == nullptr)
            continue;
          
          Point* des = getMove(loc, IncY);
          if (des != nullptr && 
              (getMove(loc, IncX) == nullptr ||
               ((getPoint(loc->x, loc->y-1) != nullptr || 
                 getPoint(loc->x-2, loc->y) != nullptr)
                && getMove(des, IncY) != nullptr) ||
               (getPoint(loc->x, loc->y-1) != nullptr && getMove(getMove(des, DecX), DecX) != nullptr)
              )
            )
            loc->nexts.push_back(des);
          
          des = getMove(loc, DecY);
          if (des != nullptr &&
              (getMove(loc, DecX) == nullptr ||
               ((getPoint(loc->x, loc->y+1) != nullptr ||
                 getPoint(loc->x+2, loc->y) != nullptr) 
                && getMove(des, DecY) != nullptr) ||
               (getPoint(loc->x, loc->y+1) != nullptr && getMove(getMove(des, IncX), IncX) != nullptr)
              )
            )
            loc->nexts.push_back(des);
            
          des = getMove(loc, IncX);
          if (des != nullptr &&
              (getMove(loc, DecY) == nullptr ||
               ((getPoint(loc->x-1, loc->y) !=nullptr ||
                 getPoint(loc->x, loc->y+2) != nullptr)
                 && getMove(des, IncX) != nullptr) ||
               (getPoint(loc->x-1, loc->y) != nullptr && getMove(getMove(des, IncY), IncY) != nullptr)
              )
            )
            loc->nexts.push_back(des);
            
          des = getMove(loc, DecX);
          if (des != nullptr &&
              (getMove(loc, IncY) == nullptr ||
               ((getPoint(loc->x+1, loc->y) != nullptr ||
                 getPoint(loc->x, loc->y-2) != nullptr)
                 && getMove(des, DecX) != nullptr) ||
               (getPoint(loc->x+1, loc->y) != nullptr && getMove(getMove(des, DecY), DecY) != nullptr)
              )
            )
            loc->nexts.push_back(des);
          
          //printf("[%d, %d] -> ", loc->x, loc->y);
          //for (Point* next : loc->nexts) 
          //  printf("[%d,%d],", next->x, next->y);
          //printf("\n");
          //ASSERT(false);
          //EXIT(100);
          
          ASSERT(loc->nexts.size() > 0 && loc->nexts.size() < 3);
        }
      }

    private:
      APICaller* api;
      
      int _map_x_size;
      int _map_y_size;
      vector<Point*> _map;
      
      list<Point*> _stations;
      list< pair<Point*, Point*> > _pickups;
      list< pair<Point*, Point*> > _unloads;
  };
}
#include <vector>
#include <iostream>
#include <algorithm>
#include <x86intrin.h>
#include <stdexcept>

/* Measurements of C++ code from the standard template library.
 * We'd expect the cost of inserting elements and reversing the vector
 * to be O(N) while the cost of sorting should be O(N log N).
 *
 * This program also tests exception handling.
 */
int main()
{
   std::vector<int>   v;
   int                i, total_size;
   unsigned long long start, finish, total_time;
   double             per_item;


   try {
      for (total_size = 1; ; total_size *= 2) {
         v.clear();

         start = __rdtsc();
         for (i = 0; i < total_size; i++) {
            v.push_back (i);
         }
         finish = __rdtsc();
         total_time = finish - start;
         per_item = (double) total_time / total_size;
         std::cout << "Insert," << i << ",items required," << total_time << ",ticks:,"
                   << per_item << ",per item\n";

         start = __rdtsc();
         std::sort (v.begin(), v.end());
         finish = __rdtsc();
         total_time = finish - start;
         per_item = (double) total_time / total_size;
         std::cout << "Sort," << i << ",ordered items required," << total_time << ",ticks:,"
                   << per_item << ",per item\n";

         start = __rdtsc();
         std::reverse (v.begin(), v.end());
         finish = __rdtsc();
         total_time = finish - start;
         per_item = (double) total_time / total_size;
         std::cout << "Reverse," << i << ",items required," << total_time << ",ticks:,"
                   << per_item << ",per item\n";

         start = __rdtsc();
         std::sort (v.begin(), v.end());
         finish = __rdtsc();
         total_time = finish - start;
         per_item = (double) total_time / total_size;
         std::cout << "Sort," << i << ",reversed items required," << total_time << ",ticks:,"
                   << per_item << ",per item\n";

         if (total_size > 100000) {  
            throw std::invalid_argument("stop here");
         }
      }
   } catch (const std::invalid_argument& x) {
      std::cout << "stopped by exception\n";
   }
      
   return 0;
}



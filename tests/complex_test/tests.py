#!/usr/bin/python

import sys

def trace_filter(in_trace):
   line = ""
   out_trace = []
   while not line.startswith("00000001 "):
      line = in_trace.readline()
      if line == "":
         raise Exception("reached EOF without seeing start marker")
   while not line.startswith("000000fc "):
      if not line.startswith("40000000 "):
         out_trace.append(line)
      if line == "":
         raise Exception("reached EOF without seeing end marker")
      line = in_trace.readline()
   out_trace.append(line)
   return out_trace

if __name__ == "__main__":
   if len(sys.argv) != 2:
      raise Exception("Specify the platform on the command line, e.g. python tests.py win32")

   PLATFORM = sys.argv[1]

   trace = trace_filter(open("ctest." + PLATFORM + ".trace", "rt"))
   open("ctest." + PLATFORM + ".trace.filtered", "wt").write(''.join(trace))
   ref = trace_filter(open("ctest." + PLATFORM + ".ref", "rt"))
   if trace == ref:
      open("ctest." + PLATFORM + ".ok", "wt").write("")
      print("ctest." + PLATFORM + ".ref matches ctest." + PLATFORM + ".trace")
   else:
      raise Exception("ctest." + PLATFORM + ".ref and ctest." +
               PLATFORM + ".trace do not match")



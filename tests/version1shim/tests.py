#!/usr/bin/python

import subprocess, os, sys

ROOT = os.path.join(os.getcwd(), "..", "..")
LOADER = None
PLATFORM = None
SUFFIX = None
TMP_FILE = "tmp.txt"

if __name__ == "__main__":
   if len(sys.argv) != 2:
      raise Exception("Specify the platform on the command line, e.g. python tests.py win32")

   PLATFORM = sys.argv[1]
   SUFFIX = "." + PLATFORM
   EXE = ""

   if PLATFORM.startswith("win"):
      EXE = ".exe"
   SUFFIX += EXE

   if PLATFORM.endswith("32"):
      LOADER = os.path.normpath(os.path.join(ROOT, "bin", "x86determiniser" + EXE))
   os.environ["LD_LIBRARY_PATH"] = os.getcwd()
   
   fd = open(TMP_FILE, "wb")
   p = subprocess.Popen([LOADER, "example.exe", "1"], stdout = fd)
   p.wait()
   fd.close()
   output1 = open(TMP_FILE, "rt").read()

   if output1.find("Instruction counts (RDTSC) using libx86determiniser:") < 0:
      raise Exception("Expected to see RDTSC message")
   if output1.find("Total count:") < 0:
      raise Exception("Expected to see 'Total count' message")

   for line in output1.split("\n"):
      if line.startswith("loop"):
         measurements = [ int(x) for x in line.split()[ -9 : ] ]
         if line.startswith("loop factorials"):
            delta = measurements[1] - measurements[0]
            for i in range(1, 9):
               if delta != (measurements[i] - measurements[i - 1]):
                  raise Exception("factorial measurements should all have "
                                 "the same delta")
         else:
            if len(set(measurements)) != 1:
               raise Exception("measurements should all be the same")

   print("tests ok")
   open("tests." + PLATFORM + ".ok", "wt").write("")



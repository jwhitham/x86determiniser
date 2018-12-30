#!/usr/bin/python

import subprocess, os, time

SUFFIX = ".exe"
ROOT = os.path.join(os.getcwd(), "..", "..")
LOADER32 = os.path.join(ROOT, "bin", "x86determiniser" + SUFFIX)
TMP_FILE = "tmp.txt"
TMP_FILE_2 = "tmp2.txt"

def clean():
   for name in [TMP_FILE, TMP_FILE_2]:
      error = 0
      while os.path.exists(TMP_FILE) and error < 50:
         try:
            os.unlink(TMP_FILE)
         except:
            time.sleep(0.01)
         error += 1

# Here is a test of the args program, with and without the x86determiniser loader.
# Weird parameters should be passed through without change.
def args_test(use_loader):

   def sub_args_test(args_list):
      loader = []
      args_list = ["args" + SUFFIX] + args_list
      if use_loader:
         loader = [use_loader, "--inst-trace", TMP_FILE_2, "--"]
      clean()

      fd = open(TMP_FILE, "wt")
      rc = subprocess.call(loader + args_list, stdout = fd)
      fd.close()
      if rc != 0:
         raise Exception("args_test should have rc = 0, rc = %d" % rc)

      fd = open(TMP_FILE, "rt")
      if fd.readline().strip() != "hello world":
         raise Exception("args_test first line should be hello world")

      for arg in args_list:
         line = fd.readline().strip()
         if not (line.startswith("[") and line.endswith("]")):
            raise Exception("args_test line should be enclosed in []")
         line = line[1:-1]
         if line != arg:
            raise Exception("args_test arg does not match ('%s' != '%s')" % (
                  line, arg))

      fd.close()

      if use_loader:
         fd = open(TMP_FILE_2, "rt")
         count = 0
         for line in fd:
            count += 1
         fd.close()
         if count < 100:
            # Typically at least 700
            raise Exception("should be at least 100 instructions executed for args_test")

   sub_args_test([])
   sub_args_test(["abcd", "efgh"])
   sub_args_test(["what do", "/you", "mean,",
            "'flash", "gordon", "approaching?'"])
   sub_args_test([" klytus ", "-im", "\"bored\"",
            "  what", "\\plaything", " have : you for me today?  ",
            "(an obscure", "body in the SK system your majesty ",
            "its (inhabitants) refer to [it] as THE EARTH how peaceful it ",
            "--looks", "*.*", "???.???",
            " // will you destroy this EARTH? ",
            "@LATER", "%1", "like to play with things a while",
            "\"\"BEFORE ANNIHILATION\"\"", "%TEMP%", "$SHELL" ])

def outs_test(args):
   clean()
   rc = subprocess.call([LOADER32, "--out-trace", TMP_FILE] + args + ["--", "outs" + SUFFIX])
   if rc != 0:
      raise Exception("outs_test should have rc = 0, rc = %d" % rc)

   expected_output = [
      [0x12, 0x00000002, 0x00000034],
      [0x12, 0x00000003, 0xaabbcc34],
      [0x76, 0x00000005, 0x00000098],
      [0x76, 0x00000006, 0xfedcba98],
      [0x9a, 0x00000008, 0x12345678],
      [0x9a, 0x00000009, 0x00000078],
      [0x00, 0x0000000a, 0x12345678],
      [0x00, 0x0000000b, 0x12345678],
      [0x00, 0x0000000d, 0x12345678],
      [0x00, 0x00000010, 0x12345678],
      [0x00, 0x00000014, 0x12345678],
      [0x00, 0x00000018, 0x12345678],
      [0x01, 0x00000001, 0x00000019],
   ]

   i = 0
   for line in open(TMP_FILE, "rt"):
      i += 1
      fields = line.split()
      port = int(fields[0], 16)
      time = int(fields[1], 16)
      value = int(fields[2], 16)
      got = [port, time, value]
      expect = expected_output[i - 1]
      if got != expect:
         raise Exception("unexpected output from outs.s on line %d, got %s expect %s" % (
            i, repr(got), repr(expect)))

def help_test(args, unknown_option, use_loader):
   clean()
   fd = open(TMP_FILE, "wt")
   fd2 = open(TMP_FILE_2, "wt")
   rc = subprocess.call([use_loader] + args, stdout = fd, stderr = fd2)
   fd.close()
   fd2.close()
   if rc != 1:
      raise Exception("help_test should have rc = 1, rc = %d" % rc)

   ok = False
   for line in open(TMP_FILE, "rt"):
      if line.startswith("Basic usage"):
         ok = True
   
   if not ok:
      raise Exception("No help message printed for %s" % args)

   if unknown_option:
      ok = False
      for line in open(TMP_FILE_2, "rt"):
         if line.find("unknown option") > 0:
            ok = True
      
      if not ok:
         raise Exception("No unknown option message printed for %s" % args)

def check_error(use_loader):
   def sub_check_error(args):
      clean()
      fd = open(TMP_FILE, "wt")
      fd2 = open(TMP_FILE_2, "wt")
      rc = subprocess.call([use_loader] + args, stdout = fd, stderr = fd2)
      fd.close()
      fd2.close()
      if rc == 0:
         raise Exception("rc should be non-zero if an error occurred")

      for line in open(TMP_FILE, "rt"):
         raise Exception("standard output should be empty if an error occurred")

      output = []
      for line in open(TMP_FILE_2, "rt"):
         output.append(line)

      return "".join(output)

   name = "this program does not exist.exe"
   s = sub_check_error([name])
   if not (s.find(name) and (s.find("cannot find the file") > 0)):
      raise Exception("Did not see expected error message for file not found")

   name = "c:\\this path does not exist\\this program does not exist.exe"
   s = sub_check_error([name])
   if not (s.find(name) and (s.find("cannot find the path") > 0)):
      raise Exception("Did not see expected error message for path not found")

   for name in ["c:\\this path does not exist\\some file.txt", "."]:
      for option in ["--branch-trace", "--inst-trace", "--out-trace"]:
         s = sub_check_error([option, name, "args" + SUFFIX])
         if not ((s.find("Failed to open") > 0) and (s.find(option) > 0)):
            raise Exception("Did not see expected error message for %s" % option)

   
if __name__ == "__main__":
   args_test(None)
   args_test(LOADER32)
   outs_test([])
   outs_test(["--branch-trace", TMP_FILE_2])
   outs_test(["--inst-trace", TMP_FILE_2])
   help_test([], False, LOADER32)
   help_test(["-?"], True, LOADER32)
   help_test(["--"], False, LOADER32)
   check_error(LOADER32)


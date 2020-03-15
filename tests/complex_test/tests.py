#!/usr/bin/python

import sys


def trace_decoder(in_trace_file, out_trace_file):
   # Decode branch_trace_format.pdf into text format
   mask28 = (1 << 28) - 1
   mask36 = (1 << 36) - 1
   mask56 = 0xff << 56
   word_data = 0
   source_address = None

   while True:
      line = in_trace_file.readline()
      if line == "":
         return

      fields = line.split()
      value = int(fields[0], 16)
      current_time = int(fields[1], 16)
      opcode = value >> 28
      value &= mask28
      if opcode == 0:
         # CEM custom event ID
         out_trace_file.write("%9d CEM %d\n" % (current_time, value))
      elif opcode == 1:
         # BNT branch not taken
         value |= word_data
         out_trace_file.write("%9d BNT %08x\n" % (current_time, value))
      elif opcode == 2:
         # SA source address
         value |= word_data
         source_address = value
      elif opcode == 3:
         # BT branch taken
         value |= word_data
         if source_address != None:
            out_trace_file.write("%9d BT  %08x -> %08x\n" % (current_time, source_address, value))
         else:
            out_trace_file.write("%9d BT  %08x\n" % (current_time, value))
            source_address = None
      elif opcode == 4:
         # SM save middle temp
         word_data = ((word_data & mask56)
                     | (value << 28)
                     | (word_data & mask28))
      elif opcode == 5:
         # SH save high temp
         word_data = ((word_data & mask36)
                     | (value << 36))
      else:
         out_trace_file.write("%9d ??? %x%07x\n" % (current_time, opcode, value))


def trace_filter(in_trace_file, out_trace_file):
   # Find start marker (CEM 1)
   while True:
      line = in_trace_file.readline()
      if line == "":
         raise Exception("reached EOF without seeing start marker")

      fields = line.split()
      if int(fields[0], 16) == 1:
         # start marker
         break

   # Copy to end marker (CEM 252) or EOF
   while True:
      line = in_trace_file.readline()
      if line == "":
         print("warning: reached EOF without seeing end marker in %s" % in_trace_file.name)
         break

      fields = line.split()
      value = int(fields[0], 16)
      current_time = int(fields[1], 16)
      if value == 0xfc:
         # end marker
         break

      out_trace_file.write("%08x %08x\n" % (value, current_time))

if __name__ == "__main__":
   if len(sys.argv) != 2:
      raise Exception("Specify the platform on the command line, e.g. python tests.py win32")

   platform = sys.argv[1]

   reference_output = "ctest." + platform + ".ref"
   reference_output_filtered = reference_output + ".filtered"
   reference_output_decoded = reference_output_filtered + ".txt"

   test_output = "ctest." + platform + ".trace"
   test_output_filtered = test_output + ".filtered"
   test_output_decoded = test_output_filtered + ".txt"

   trace_filter(in_trace_file = open(test_output, "rt"),
                out_trace_file = open(test_output_filtered, "wt"))
   trace_decoder(in_trace_file = open(test_output_filtered, "rt"),
                 out_trace_file = open(test_output_decoded, "wt"))

   trace_filter(in_trace_file = open(reference_output, "rt"),
                out_trace_file = open(reference_output_filtered, "wt"))
   trace_decoder(in_trace_file = open(reference_output_filtered, "rt"),
                 out_trace_file = open(reference_output_decoded, "wt"))

   if open(test_output_decoded, "rt").read() == open(reference_output_decoded, "rt").read():
      open("ctest." + platform + ".ok", "wt").write("")
      print(reference_output + " matches " + test_output)
      sys.exit(0)

   print("Test output mismatch after filtering!")
   print("Reference file:            " + reference_output)
   print("Filtered reference file:   " + reference_output_filtered)
   print("Decoded reference file:    " + reference_output_decoded)
   print("Test output file:          " + test_output)
   print("Filtered test output file: " + test_output_filtered)
   print("Decoded test output file:  " + test_output_decoded)
   sys.exit(1)


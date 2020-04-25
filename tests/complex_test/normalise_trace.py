#!/usr/bin/python3

import sys, tempfile


# trace_decoder will decode raw trace data into text format
# Data format is as described in branch_trace_format.pdf.
#
def trace_decoder(in_raw_trace_file, out_trace_file):
   in_raw_trace_file.seek(0, 0)
   mask28 = (1 << 28) - 1
   mask36 = (1 << 36) - 1
   mask56 = 0xff << 56
   word_data = 0
   source_address = None

   while True:
      line = in_raw_trace_file.readline()
      if line == "":
         return

      fields = line.split()
      value = int(fields[0], 16)
      current_time = int(fields[1], 16)

      fields = ["{:9d}".format(current_time), "???", fields[0]]

      opcode = value >> 28
      value &= mask28
      if opcode == 0:
         # CEM custom event ID
         fields[1] = "CEM"
         fields[2] = "{:d}".format(value)
      elif opcode == 1:
         # BNT branch not taken
         value |= word_data
         fields[1] = "BNT"
         fields[2] = "{:08x}".format(value)
      elif opcode == 2:
         # SA source address
         value |= word_data
         source_address = value
         continue
      elif opcode == 3:
         # BT branch taken
         value |= word_data
         fields[1] = "BT"
         if source_address != None:
            fields[2] = "{:08x}".format(source_address)
            fields.append("->")
            fields.append("{:08x}".format(value))
         else:
            fields[2] = "{:08x}".format(value)
            source_address = None
      elif opcode == 4:
         # SM save middle temp
         word_data = ((word_data & mask56)
                     | (value << 28)
                     | (word_data & mask28))
         continue
      elif opcode == 5:
         # SH save high temp
         word_data = ((word_data & mask36)
                     | (value << 36))
         continue
    
      out_trace_file.write(" ".join(fields))
      out_trace_file.write("\n")


# trace_filter will remove all parts of the trace before event "CEM 1" and after "CEM 252"
#
def trace_filter(in_trace_file, out_trace_file):
   in_trace_file.seek(0, 0)
   # Find start marker (CEM 1)
   while True:
      line = in_trace_file.readline()
      fields = line.split()
      if len(fields) == 0:
         raise Exception("reached EOF without seeing start marker")

      if (len(fields) == 3) and (fields[1] == "CEM") and (int(fields[2]) == 1):
         # start marker
         break

   # Copy to end marker (CEM 252) or EOF
   while True:
      line = in_trace_file.readline()
      fields = line.split()
      if len(fields) == 0:
         print("warning: reached EOF without seeing end marker")
         break

      if (len(fields) == 3) and (fields[1] == "CEM") and (int(fields[2]) == 252):
         # end marker
         break

      out_trace_file.write(line)


# trace_find_first gives the first time and address in the trace file
#
def trace_find_first(in_trace_file):
   in_trace_file.seek(0, 0)
   min_time = min_address = 1 << 63
   while True:
      line = in_trace_file.readline()
      fields = line.split()
      if len(fields) == 0:
         break

      min_time = min(min_time, int(fields[0]))

      if (len(fields) == 3) and ((fields[1] == "BNT") or (fields[1] == "BT")):
         min_address = min(min_address, int(fields[2], 16))
         return (min_address, min_time)

      elif (len(fields) == 5) and (fields[1] == "BT"):
         min_address = min(min_address, int(fields[2], 16), int(fields[4], 16))
         return (min_address, min_time)

   raise Exception("no addresses in the trace")


# trace_address_add_offset adds an offset to all elements in the trace
#
def trace_address_add_offset(in_trace_file, out_trace_file, address_offset, time_offset):
   in_trace_file.seek(0, 0)
   while True:
      line = in_trace_file.readline()
      fields = line.split()
      if len(fields) == 0:
         break

      fields[0] = "{:09d}".format(int(fields[0]) + time_offset)

      if (len(fields) >= 3) and ((fields[1] == "BNT") or (fields[1] == "BT")):
         fields[2] = "{:08x}".format(int(fields[2], 16) + address_offset)

         if (len(fields) == 5) and (fields[1] == "BT"):
            fields[4] = "{:08x}".format(int(fields[4], 16) + address_offset)
     
      out_trace_file.write(" ".join(fields))
      out_trace_file.write("\n")


# Normalise a raw trace
def normalise_trace(input_name, output_name):

   tmp1 = tempfile.SpooledTemporaryFile(mode = "w+t")
   tmp2 = tempfile.SpooledTemporaryFile(mode = "w+t")
   out_trace_file = open(output_name, "wt")

   # First convert the trace from its raw form to the decoded BT/BNT/CEM text form
   trace_decoder(open(input_name, "rt"), tmp1)
   # Now crop out everything outside the start/end marker
   trace_filter(tmp1, tmp2)
   # find first address and time
   (min_address, min_time) = trace_find_first(tmp2)
   # remove offsets from time and address
   min_address &= ~0xfff
   trace_address_add_offset(tmp2, out_trace_file, -min_address, -min_time)


if __name__ == "__main__":
   if len(sys.argv) != 3:
      print("Usage: normalise_trace.py <input raw trace.txt> <output.txt>")
      sys.exit(1)

   normalise_trace(sys.argv[1], sys.argv[2])
   sys.exit(0)


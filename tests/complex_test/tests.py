#!/usr/bin/python


def trace_filter(in_trace):
   line = ""
   out_trace = []
   while not line.startswith("00000001 "):
      line = in_trace.readline()
   while not line.startswith("000000fc "):
      if not line.startswith("40000000 "):
         out_trace.append(line)
      line = in_trace.readline()
   out_trace.append(line)
   return out_trace

if __name__ == "__main__":
   trace = trace_filter(open("ctest.trace", "rt"))
   open("ctest.trace.filtered", "wt").write(''.join(trace))
   ref = trace_filter(open("ctest.ref", "rt"))
   if trace == ref:
      open("ctest.ok", "wt").write("")
      print("ctest.ref matches ctest.trace")
   else:
      raise Exception("ctest.ref and ctest.trace do not match")



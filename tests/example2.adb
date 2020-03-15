
with System;
with System.Machine_Code;
with Ada.Text_IO;
with Interfaces; use Interfaces;


procedure example2 is

   function rdtsc return Unsigned_32 is
      result : Unsigned_32;
   begin
      System.Machine_Code.Asm ("rdtsc",
         Unsigned_32'Asm_Output("=A", result),
         Volatile => True);
      return result;
   end rdtsc;

   store : array (Natural range 1 .. 10) of Unsigned_32;
   v     : Unsigned_32 := 0;
begin
   v := rdtsc;
   Ada.Text_IO.Put_Line (v'Img);
   v := rdtsc;
   Ada.Text_IO.Put_Line (v'Img);

   for i in store'Range loop
      store (i) := rdtsc;
   end loop;
   for i in store'First + 1 .. store'Last loop
      v := store (i) - store (i - 1);
      Ada.Text_IO.Put_Line ("loop" & v'Img);
   end loop;
   begin
      if v > 0 then
         v := rdtsc;
         Ada.Text_IO.Put_Line (v'Img);
         raise Constraint_Error;
      end if;
      Ada.Text_IO.Put_Line ("NOT OK");
   exception
      when Constraint_Error =>
         v := rdtsc;
         Ada.Text_IO.Put_Line ("OK:" & v'Img);
   end;
end example2;



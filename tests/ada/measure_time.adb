
with System ;
with System.Machine_Code;
with Interfaces ;

package body Measure_Time is 

    use Interfaces ;
    function rdtsc return Unsigned_32 is
        result, tmp : Unsigned_32;
    begin
        System.Machine_Code.Asm ("rdtsc",
                                 (Unsigned_32'Asm_Output("=A", result),
                                  Unsigned_32'Asm_Output("=D", tmp)),
                                 Volatile => True);
        return result;
    end rdtsc;

    function Time ( D : Data; Iterations : in Natural) return Natural is
        Start, Finish : Unsigned_32;
        Time_Taken : Unsigned_32;
    begin
        Start := rdtsc;
        for I in 1..Iterations 
        loop
            Procedure_To_Be_Measured(D);
        end loop;
        Finish := rdtsc;
        Time_Taken := Finish - Start ;

        return ( Natural ( Time_Taken ) ) ;
    end Time;

end Measure_Time ;


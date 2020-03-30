
generic
    type Data ( <> ) is private;
with procedure Procedure_To_Be_Measured (D : in Data);

package Measure_Time is

    function Time ( D : Data; Iterations : in Natural) return Natural ;

end Measure_Time ;


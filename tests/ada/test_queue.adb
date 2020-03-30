
with Ada . Text_IO ;
with Ada . Integer_Text_IO ;
with Ada . Float_Text_IO ;
with Binary_Heap ;
with Measure_Time ;
use Ada . Text_IO ;
use Ada . Integer_Text_IO ;
use Ada . Float_Text_IO ;
with Ada.Numerics.Discrete_Random ;

procedure test_queue is

    arr_size : constant Natural := 100 ;

    package Binary_Heap_For_Naturals is new
        Binary_Heap ( Natural , ">" , 0 ) ;
    use Binary_Heap_For_Naturals ;

    type My_Record is record
                        number : Natural ;
                        data : Integer ;
                    end record ;


    function RecordCompareAsNumber ( a , b : My_Record ) return Boolean is
    begin
        return ( a . number > b . number ) ;
    end RecordCompareAsNumber ;

    BlankRecord : constant My_Record := ( others => 0 ) ;

    package Binary_Heap_For_My_Record is new
        Binary_Heap ( My_Record , RecordCompareAsNumber , BlankRecord ) ;
    use Binary_Heap_For_My_Record ;

    type pqa is access Binary_Heap_For_My_Record . Priority_Queue ;

    procedure My_Record_Adder_WC ( rqueue : in pqa ) is
        rec : My_Record ;
    begin
        rec . data := 2 ;
        for I in reverse 1 .. arr_size
        loop
            rec . number := I ;
            Insert ( rec , rqueue . all ) ;
        end loop ;
    end My_Record_Adder_WC ;

    procedure My_Record_Deleter ( rqueue : in pqa ) is
        rec : My_Record ;
    begin
        for I in 1 .. arr_size
        loop
            Delete_Min ( rec , rqueue . all ) ;
        end loop ;
    end My_Record_Deleter ;

    package Measure_Add_Time is new
        Measure_Time ( pqa , My_Record_Adder_WC ) ;
    package Measure_Del_Time is new
        Measure_Time ( pqa , My_Record_Deleter ) ;

    nqueue : Binary_Heap_For_Naturals . Priority_Queue ( arr_size ) ;
    ex : Boolean ;
    outvar : Natural ;


begin

    put_line ( "Testing heap properties with natural numbers" ) ;
    for I in 1 .. arr_size 
    loop
        Insert ( I * 10 , nqueue ) ;
    end loop ;

    ex := FALSE ;
    begin
        Insert ( 1 , nqueue ) ;
    exception
        when others => ex := TRUE ;
    end ;
    if ( not ex )
    then
        put_line ( "No exception thrown on last item added." ) ;
        return ;
    end if ;
   
    Delete_Min ( outvar , nqueue ) ;
    if ( Find_Min ( nqueue ) /= 20 )
    then
        put_line ( "Agh, why isn't the minimum 20?" ) ;
        return ;
    end if ;
    Insert ( 10 , nqueue ) ;

    if ( not Is_Full ( nqueue ) )
    then
        put_line ( "Is not full!?" ) ;
        return ;
    end if ;

    for I in 1 .. arr_size
    loop
        Delete_Min ( outvar , nqueue ) ;
        if ( outvar /= ( I * 10 ))
        then
            put_line ( "Output from queue not as expected." ) ;
            put ( "Got " ) ;
            put ( outvar , 0 ) ;
            put ( " but expected " ) ;
            put ( I * 10 , 0 ) ;
            new_line ;
            return ;
        end if ;
    end loop ;

    if ( not Is_Empty ( nqueue ) )
    then
        put_line ( "Queue is not empty, why not?" ) ;
        return ;
    end if ;

    put_line ( "Second stage pseudo random tests." ) ;

    declare
        subtype In_Numbers is Natural range 1 .. arr_size ;

        package Natural_Random is new Ada.Numerics.Discrete_Random ( In_Numbers ) ;

        Natural_Generator : Natural_Random . Generator;
        numbers : array ( 1 .. arr_size ) of Natural ;
        numbers_present : Natural ;
        J , K : Natural ;
    begin
        for I in 1 .. arr_size
        loop
            numbers ( I ) := I ;
        end loop ;
        numbers_present := arr_size ;

        Natural_Random . Reset ( Natural_Generator , 1 ) ;

        for I in 1 .. 100
        loop
            if ( Natural_Random . Random ( Natural_Generator ) < numbers_present )
            then
                -- add
                J := Natural_Random . Random ( Natural_Generator ) ;
                while ( numbers ( J ) = 0 )
                loop
                    J := J + 1 ;
                    if ( J > arr_size )
                    then
                        J := 1 ;
                    end if ;
                end loop ;
                numbers ( J ) := 0 ;
                Insert ( J , nqueue ) ;
                numbers_present := numbers_present - 1 ;
            else
                -- remove
                J := 1 ;
                while ( numbers ( J ) /= 0 )
                loop
                    J := J + 1 ;
                end loop ;

                Delete_Min ( K , nqueue ) ;
                if ( K /= J )
                then
                    put ( "Delete gave unexpected value. Was expecting " ) ;
                    put ( J ) ;
                    put ( " but I got " ) ;
                    put ( K ) ;
                    new_line ;
                    return ;
                end if ;
                numbers ( J ) := J ;
                numbers_present := numbers_present + 1 ;
            end if ; 
        end loop ;
    end ;

    Make_Empty ( nqueue ) ;

    put_line ( "Tests good, benchmarking now." ) ;

    declare
        add , del_wc , del_ac : Natural ;
        rec : My_Record ;
        rqueue : pqa ;

        package Natural_Random is new Ada.Numerics.Discrete_Random ( Natural ) ;

        Natural_Generator : Natural_Random . Generator;
    begin
        rqueue := new Binary_Heap_For_My_Record . Priority_Queue ( arr_size ) ;

        add := Measure_Add_Time . Time ( rqueue , 1 ) ;
        del_wc := Measure_Del_Time . Time ( rqueue , 1 ) ;
        -- Do an average case pseudo random add

        rec . data := 1 ;
        Natural_Random . Reset ( Natural_Generator , 1 ) ;
        for I in 1 .. arr_size
        loop
            rec . number := Natural_Random . Random ( Natural_Generator ) ;
            Insert ( rec , rqueue . all ) ;
        end loop ;

        del_ac := Measure_Del_Time . Time ( rqueue , 1 ) ;
        put ( "For " ) ;
        put ( arr_size , 0 ) ;
        put_line ( " operations" ) ;

        put ( "Average/worst case add time: " ) ;
        put ( add ,  0 ) ;
        put_line ( " instructions" ) ;

        put ( "Average delete time: " ) ;
        put ( del_ac , 0 ) ;
        put_line ( " instructions" ) ;

        put ( "Worst case delete time: " ) ;
        put ( del_wc , 0 ) ;
        put_line ( " instructions" ) ;
    end ;
end test_queue ;



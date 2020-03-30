

package body Binary_Heap is 

    procedure Swap ( a , b : in out Element_Type ) is
        P : Element_Type ;
    begin
        P := a ;
        a := b ;
        b := P ;
    end Swap ;

    -- Delete and show minimum element in priority queue 
    procedure Delete_Min ( X: out Element_Type; H: in out Priority_Queue ) is
        space : Natural ;
        first_child : Natural ;
        ok : Boolean := FALSE ;
    begin
        if ( H . Size < 1 )
        then
            raise Underflow ;
        end if ;

        X := H . Element ( 1 ) ;
        space := 1 ;

        while ( not ok )
        loop
            -- if the last element is smaller than both of the space's
            -- children then it could be inserted here. 
            first_child := space * 2 ;

            if ( first_child > H . Size ) -- no children
            then
                ok := TRUE ;
            elsif (( first_child + 1 ) > H . Size ) -- one child
            then
                -- but we know which child that is, it's the last one!
                ok := TRUE ;
            else
                -- two children
                if (( not ( H . Element ( H . Size ) >
                            H . Element ( first_child ) ))
                and ( not ( H . Element ( H . Size ) >
                            H . Element ( first_child + 1 ) )))
                then
                    ok := TRUE ;
                else
                    ok := FALSE ;
                end if ;
            end if ;

            if ( ok )
            then
                H . Element ( space ) := H . Element ( H . Size ) ;
            else
                -- next level.
                if ( H . Element ( first_child ) >
                    H . Element ( first_child + 1 ) )
                then
                    -- second child is smaller.
                    Swap ( H . Element ( first_child + 1 ) ,
                        H . Element ( space ) ) ;
                    space := first_child + 1 ;
                else
                    Swap ( H . Element ( first_child ) ,
                        H . Element ( space ) ) ;
                    space := first_child ;
                end if ;
            end if ;
        end loop ;
        H . Size := H . Size - 1 ;
        
    end Delete_Min ;


    -- Return minimum element in priority queue 
    function Find_Min ( H: Priority_Queue ) return Element_Type is
    begin
        if ( H . Size < 1 )
        then
            raise Underflow ;
        end if ;
        return H . Element ( 1 ) ;
    end Find_Min ;

    -- Add a new element to priority queue 
    procedure Insert ( X: Element_Type; H: in out Priority_Queue ) is
        parent , child : Natural ;
    begin
        -- Insert at leftmost empty leaf.
        if ( H . Size >= H . Element ' Last )
        then
            raise Overflow ;
        end if ;
        H . Size := H . Size + 1 ;
        H . Element ( H . Size ) := X ;
        child := H . Size ;
        parent := child / 2 ;

        while ( child > 1 )
        loop
            if ( H . Element ( parent ) > H . Element ( child ) )
            then
                -- swap
                Swap ( H . Element ( parent ) , H . Element ( child ) ) ;

                child := parent ;
                parent := parent / 2 ;
            else
                return ;
            end if ;
        end loop ;
    end Insert ;

    -- Returns true if priority queue is empty 
    function Is_Empty ( H: Priority_Queue ) return Boolean is
    begin
        return ( H . Size < 1 ) ;
    end Is_Empty ;

    -- Returns true if priority queue is full 
    function Is_Full ( H: Priority_Queue ) return Boolean is
    begin
        return ( H . Size >= H . Element ' Last ) ;
    end Is_Full ;


    -- Make a priority queue empty 
    procedure Make_Empty( H: out Priority_Queue ) is
    begin
        H . Size := 0 ;
    end Make_Empty ;

end Binary_Heap; 





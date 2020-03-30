

generic 

        type Element_Type is private; 

        with function ">" ( Left, Right: Element_Type ) return Boolean; 

        Min_Element : in Element_Type; 


package Binary_Heap is 
  

        type Priority_Queue( Max_Size: Positive ) is limited private; 
          

        procedure Delete_Min ( X: out Element_Type; H: in out Priority_Queue ); 

        -- Delete and show minimum element in priority queue 

        function Find_Min ( H: Priority_Queue ) return Element_Type; 

        -- Return minimum element in priority queue 

        procedure Insert ( X: Element_Type; H: in out Priority_Queue ); 

        -- Add a new element to priority queue 

        function Is_Empty ( H: Priority_Queue ) return Boolean; 

        -- Returns true if priority queue is empty 

        function Is_Full ( H: Priority_Queue ) return Boolean; 

        -- Returns true if priority queue is full 

        procedure Make_Empty( H: out Priority_Queue ); 

        -- Make a priority queue empty 
          

        Overflow : exception; 

        -- Raised for Insert on a full priority queue 

        Underflow: exception; 

        -- Raised for Delete_Min or Find_Min 
         

private 
  

        type Array_Of_Element_Type is array( Natural range <> ) of Element_Type; 
          

        type Priority_Queue( Max_Size : Positive ) is 
               record 
                      Size : Natural := 0; 

                      Element : Array_Of_Element_Type( 0..Max_Size ) := ( others => Min_Element );
               end record;

end Binary_Heap; 



(*
    Autor: Claudio Saji
    Compilar:  $ ocamlc -o bit-packing bit-packing.ml
    Ejecutar:  $ ./bit-packing
*)


open String     (* for "trim" function *)


let rec length list =   (* returns the length of a list *)
    let rec iter acc list =
        match list with
        | (x::xs) -> iter(acc+1)xs
        | [] -> acc
        in iter 0 list;;


let log2 n =        (* returns the number of bits needed to represent n *)
    if n = 0 then 8
    else
        begin
            let x = ref 0 
            and y = ref n in
            while !y <> 0 do
                y := !y/2;
                x := !x+1;
            done;
            if !x mod 8 = 0 then !x
            else (!x/8 + 1) * 8
        end;;


let log2_64 n =        (* int64 version of log2 *)
    if n = Int64.zero then 8
    else
        begin
            let x = ref 0 
            and y = ref n in
            while !y <> Int64.zero do
                y := Int64.div !y 2L;
                x := !x+1;
            done;
            if !x mod 8 = 0 then !x
            else (!x/8 + 1) * 8
        end;;


let total_bits_64 list =   (* int64 version of total_bits *)
    let len = length list
    and sum = ref 0 in
    for i=0 to len-1 do
        sum := !sum + log2_64 (List.nth list i);   (* List.nth returns the nth element of the list *)
    done;
    !sum;;


let total_bits list =   (* returns the total number of bits in the list *)
    let len = length list
    and sum = ref 0 in
    for i=0 to len-1 do
        sum := !sum + log2 (List.nth list i);   (* List.nth returns the nth element of the list *)
    done;
    !sum;;


let newList list =  (* returns the compressed version of the original list *)
    let len = length list in
    let new_list = ref [(List.nth list (len-1))] in
    for i = (len-2) downto 1 do
        new_list := (abs ((List.nth list i) - (List.nth list (i-1)))) :: !new_list;
    done;
    new_list := (List.nth list 0) :: !new_list;
    new_list;;


let intList list =   (* returns a version of the original list of type int *)
    let int_list = ref [] in
    let len = length list in
    for i = 0 to len-1 do
        let x = int_of_string (List.nth list i) in 
        int_list := x :: !int_list;
    done;
    int_list;;


let read_file filename = (* reads the input *)
    let lines = ref [] in
    let chan = open_in filename in
    try
      while true; do
        lines := (trim (input_line chan)) :: !lines (*trim returns a copy of the argument, without leading and trailing whitespace.*)
      done; !lines
    with End_of_file ->
      close_in chan;
    !lines;;
    

let int64List list =  (* returns the int 64 list and the list of the positions needed to reconstruct the original list *)
    let list64 = ref [] in
    let len = length list in
    let space = ref 63 in   (* 2^63 is the maximum representable integer of type int64 *)
    let num = ref Int64.zero in
    let bits = ref 0 in
    let pos = ref 0 in
    let list_pos = ref [0] in
    for i = 0 to len-1 do
        bits := (log2 (List.nth list i));
        space := !space - !bits;
        if !space >= 0 then
            begin
                if !num <> Int64.zero then
                    num := Int64.shift_left !num !bits;
                num := Int64.logor !num (Int64.of_int (List.nth list i));
                (*Printf.printf "%Ld " !num;*)
                if i = (len-1) then
                    list64 := !num :: !list64;
                if i <> (len-1) then
                    begin
                        pos := !pos + (!bits / 8);
                        if !pos > 7 then
                            pos := 0;
                        list_pos := !pos :: !list_pos;
                    end
            end
        else
            begin
                if i <> len-1 then
                    begin
                        pos := !bits / 8;
                        list_pos := !pos :: !list_pos;
                    end;
                list64 := !num :: !list64;
                space := 63 - !bits;
                num := Int64.logor Int64.zero (Int64.of_int (List.nth list i));
                (*Printf.printf "%Ld " !num;*)
                if i = (len-1) then
                    list64 := !num :: !list64;
            end
    done;
    list_pos := List.rev !list_pos;
    list64 := List.rev !list64;
    (list64, list_pos);;


let main () =
    let filename = "datos_steim.txt" in
    let lines = read_file filename in
    let list = intList lines in
    let new_list = newList !list in
    let (list64, list_pos) = int64List !new_list in
    
    (*Print the int64 list and positions list*)
    
    (*for i = 0 to (length !list64)-1 do
        Printf.printf "\n%Ld" (List.nth !list64 i);
    done;
    Printf.printf "\nLEN INT64 = %d\nLEN NEW_LIST = %d\n" (length !list64) (length !new_list);
    Printf.printf "\n\nLista posiciones:\n";
    for i = 0 to (length !list_pos)-1 do
        Printf.printf "\n%d " (List.nth !list_pos i);
    done;
    Printf.printf "\n";*)
    
    let sum1 = total_bits !new_list in
    let sum2 = total_bits_64 !list64 in
    let sum3 = total_bits !list in
    let saved1 = 1.0 -. (float_of_int sum1 /. float_of_int sum3) in
    let saved2 = 1.0 -. (float_of_int sum2 /. float_of_int sum3) in
    Printf.printf "Uso de bits de los valores originales: %d\nUso de bits de los valores comprimidos: %d\nUso de bits de la lista tipo Int64: %d\n\nPorcentaje de bits ahorrados (diferencias): %.3f \nPorcentaje de bits ahorrados (int64): %.3f \n" sum3 sum1 sum2 saved1 saved2;
    exit 0;;
main ();;



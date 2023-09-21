(*
    Autor: Claudio Saji
    Compilar:  $ ocamlc -o comp-algo compression-algorithm.ml
    Ejecutar:  $ ./comp-algo
*)


open String     (* for "trim" function *)


let rec length list =   (* returns the length of a list *)
    let rec iter acc list =
        match list with
        | (x::xs) -> iter(acc+1)xs
        | [] -> acc
        in iter 0 list;;


let log2 n =        (* returns the log base 2 of n *)
    if n = 0 then 1
    else
    let x = ref 0 
    and y = ref n in
    while !y <> 0 do
        y := !y/2;
        x := !x+1;
    done;
    !x;;


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
    

let main () =
    let filename = "datos_steim.txt" in
    let lines = read_file filename in
    let list = intList lines in
    let new_list = newList !list in
    let sum2 = total_bits !new_list in
    let sum1 = total_bits !list in
    let saved = 100.0 -. ((float_of_int sum2 /. float_of_int sum1) *. 100.0) in
    Printf.printf "Uso de bits de los valores originales: %d\nUso de bits de los valores comprimidos: %d\n\nPorcentaje de bits ahorrados: %.3f %% \n" sum1 sum2 saved;
    exit 0;;
main ();;



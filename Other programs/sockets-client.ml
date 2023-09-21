(*
    Autor: Claudio Saji
    Compilar:  $ ocamlc unix.cma -o client.exe client.ml
    Ejecutar:  $ ./client.exe host port
*)


let open_connection sockaddr =
   let domain = Unix.domain_of_sockaddr sockaddr in
   let sock = Unix.socket domain Unix.SOCK_STREAM 0 
   in try Unix.connect sock sockaddr ;
          (Unix.in_channel_of_descr sock , Unix.out_channel_of_descr sock)
     with exn -> Unix.close sock ; raise exn ;;
     
     
let shutdown_connection inchan =
   Unix.shutdown (Unix.descr_of_in_channel inchan) Unix.SHUTDOWN_SEND ;;
   
   
let main_client client_fun  =
   if Array.length Sys.argv < 3 
   then Printf.printf "Usage: ./client server port\n"
   else let server = Sys.argv.(1) in
        let server_addr =
          try  Unix.inet_addr_of_string server 
          with Failure("inet_addr_of_string") -> 
                 try  (Unix.gethostbyname server).Unix.h_addr_list.(0) 
                 with Not_found ->
                        Printf.eprintf "%s : Unknown server\n" server ;
                        exit 2
        in try 
             let port = int_of_string (Sys.argv.(2)) in
             let sockaddr = Unix.ADDR_INET(server_addr,port) in 
             let ic,oc = open_connection sockaddr
             in client_fun ic oc ;
                shutdown_connection ic
           with Failure("int_of_string") -> Printf.eprintf "bad port number";
                                            exit 2 ;;
                                            
                                            
let client_fun ic oc = 
   try
     while true do  
       print_string  "Enter GET line: " ;
       flush stdout ;
       let get_string = ((input_line stdin)^"\n") in
       if get_string = "exit\n" then (shutdown_connection ic ; raise Exit );

       print_string  "Enter Host line: " ;
       flush stdout ;
       let message = (get_string) ^ ((input_line stdin)^"\n") in
       
       output_string oc message ;  (* send message *)
       flush oc ;
       
       let buffer = Bytes.create 256 in
       input ic buffer 0 256;   (* receive message *)
       Printf.printf "%s\n" buffer;
       flush stdout ;
     done
   with 
       Exit -> exit 0
     | exn -> shutdown_connection ic ; raise exn  ;;
     
     
main_client client_fun ;;
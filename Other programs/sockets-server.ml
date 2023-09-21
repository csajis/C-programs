(*
    Autor: Claudio Saji
    
    Compilar:  $ ocamlc unix.cma -o server.exe server.ml
    Ejecutar:  $ ./server.exe port
    Probar con curl: $ curl -v -X GET localhost:port/
*)


let establish_server server_fun sockaddr =
   let domain = Unix.domain_of_sockaddr sockaddr in
   let sock = Unix.socket domain Unix.SOCK_STREAM 0 
   in Unix.bind sock sockaddr ;
      Unix.listen sock 3;
      while true do
        let (s, caller) = Unix.accept sock 
        in match Unix.fork() with
               0 -> if Unix.fork() <> 0 then exit 0 ; 
                       server_fun s;
                       exit 0
             | id -> Unix.close s; ignore(Unix.waitpid [] id)
      done ;;

   
let main_server  serv_fun =
   if Array.length Sys.argv < 2 then Printf.eprintf "Usage: ./server port\n"
   else try
          let port =  int_of_string Sys.argv.(1) in 
          let my_address = ((Unix.gethostbyname "localhost").Unix.h_addr_list.(0))    (* ((Unix.gethostbyname(Unix.gethostname())).Unix.h_addr_list.(0)) for device name *)
          in establish_server serv_fun  (Unix.ADDR_INET(my_address, port))
        with
          Failure("int_of_string") -> 
            Printf.eprintf "bad port number\n" ;;
            

let left_zero n =
   if n < 10 then
      "0" ^ (string_of_int n)
   else
      string_of_int n;;


let day_week n =
   match n with
     1 -> "Mon"
	| 2 -> "Tues"
	| 3 -> "Wed"
   | 4 -> "Thur"
	| 5 -> "Fri"
   | 6 -> "Sat"
	| _ -> "Sun";;
	
   
let resp_service s =

   let buf = Bytes.create 512 in
   Unix.read s buf 0 512;
   let str = Bytes.sub_string buf 0 14 in
         
   if str = "GET / HTTP/1.1" then (
      let seconds = Unix.time () in
      let date = Unix.gmtime seconds in
      let day = day_week date.tm_wday in
      let d_m_y = (left_zero(date.tm_mon+1)) ^"/"^ (left_zero(date.tm_mday)) ^"/"^ (string_of_int(date.tm_year+1900)) in
      let h_m_s = (left_zero(date.tm_hour)) ^":"^ (left_zero(date.tm_min)) ^":"^ (left_zero(date.tm_sec)) in
      let resp = "\nHTTP/1.1 200 OK\n" ^ "Date: " ^ day ^ ", " ^ d_m_y ^ " " ^ h_m_s ^ " GMT\n" ^"Server: Foomatic!\n" ^ "Content-Length: 0\n\n" in
            
      let b_resp = Bytes.of_string resp in
      Unix.write s b_resp 0 (Bytes.length b_resp)
   )
   else (
      let points = Bytes.of_string "..." in
      Unix.write s points 0 (Bytes.length points)
   );
         
   Printf.printf "Message sent\n";
   flush stdout ;;
   
   

Unix.handle_unix_error main_server resp_service ;;

